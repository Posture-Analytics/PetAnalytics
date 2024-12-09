#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <FirebaseESP32.h>
#include "config.h" // change this file to your own config.h

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

bool imuActive = true; // variable to control the state of the IMU
int imuReadingsCount = 0; // IMU readings counter
const int readingsThreshold = 79; // number of readings before sending to Firebase

// Initialize NTPClient with the specified NTP server ("pool.ntp.org"), brazil offset (-10800) 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000);

// Double buffers for storing IMU readings
FirebaseJson jsonData1;
FirebaseJson jsonData2;

// Pointers to the active buffer and sending buffer
FirebaseJson* activeJsonData = &jsonData1;
FirebaseJson* sendingJsonData = &jsonData2;

TaskHandle_t Task1;
TaskHandle_t Task2;

SemaphoreHandle_t xSemaphore;

//Interrupt
hw_timer_t *timer = NULL;
volatile bool timerFlag = true;

void IRAM_ATTR onTimer() {
    timerFlag = true;
}

void setup() {
    // initialize serial
    Serial.begin(115200);

    // initialize device
    auto cfg = M5.config();
    StickCP2.begin(cfg);

    StickCP2.Display.setTextSize(2);
    StickCP2.Display.setRotation(3);
    StickCP2.Display.setCursor(10, 20);
    StickCP2.Display.printf("Power on!");

    delay(500);

    // connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    StickCP2.Display.clear();
    StickCP2.Display.setCursor(10, 20);
    StickCP2.Display.printf("Connected to Wi-Fi!");
    Serial.println("Connected to Wi-Fi!");

    // connect to NTP
    timeClient.begin();
    timeClient.update();

    // configure interrupt with internal clock
    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &onTimer); 
    timerAlarm(timer, 1000000, true, 0);

    // configure Firebase
    config.api_key = DATABASE_API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = DATABASE_USER_EMAIL;
    auth.user.password = DATABASE_USER_PASSWORD;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // create semaphore for variable synchronization
    xSemaphore = xSemaphoreCreateMutex();

    // create tasks
    xTaskCreatePinnedToCore(
        collectIMUData,   // Task function
        "Task1",          // Task name
        10000,            // Stack size
        NULL,             // Task input parameter
        1,                // Priority of the task
        &Task1,           // Task handle
        0);               // Core 0

    xTaskCreatePinnedToCore(
        sendDataToFirebase,  // Task function
        "Task2",             // Task name
        10000,               // Stack size
        NULL,                // Task input parameter
        2,                   // Priority of the task
        &Task2,              // Task handle
        1);                  // Core 1

    Serial.println("Configuration completed");

    StickCP2.Display.clear();
    StickCP2.Display.setCursor(10, 20);
    StickCP2.Display.printf("IMU activated");
}

void collectIMUData(void * parameter) {
    String current_time;

    while (true) {
        // check if button A was pressed
        if (StickCP2.BtnA.wasPressed()) {
            imuActive = !imuActive; // toggle IMU state
            if (imuActive) {
                Serial.println("IMU activated");

                StickCP2.Display.clear();
                StickCP2.Display.setCursor(10, 20);
                StickCP2.Display.printf("IMU activated");

                //update real time
                timeClient.update();
                current_time = timeClient.getFormattedTime();
            } else {
                Serial.println("IMU deactivated");

                StickCP2.Display.clear();
                StickCP2.Display.setCursor(10, 20);
                StickCP2.Display.printf("IMU deactivated");
            }

            while(StickCP2.BtnA.isPressed()) {
                StickCP2.update();
                delay(10);
            }
        }

        // update button state
        StickCP2.update();

        // collect and send IMU data if active
        if (imuActive) {

            // check the interrupt
            if (timerFlag) {
                // Atualizar e imprimir o horário
                timeClient.update();
                current_time = timeClient.getFormattedTime();
                //Serial.println(current_time);
                
                timerFlag = false; // Reseta a flag
            }

            auto imu_update = StickCP2.Imu.update();

            if (imu_update) {

                // Protect shared resources with semaphore
                xSemaphoreTake(xSemaphore, portMAX_DELAY);

                if (imuReadingsCount < readingsThreshold){
                    imuReadingsCount++;
                    xSemaphoreGive(xSemaphore);

                    auto data = StickCP2.Imu.getImuData();

                    // store readings in active JSON
                    FirebaseJson jsonEntry;
                    jsonEntry.set("aX", data.accel.x);
                    jsonEntry.set("aY", data.accel.y);
                    jsonEntry.set("aZ", data.accel.z);
                    jsonEntry.set("gX", data.gyro.x);
                    jsonEntry.set("gY", data.gyro.y);
                    jsonEntry.set("gZ", data.gyro.z);
                    activeJsonData->set(current_time + "/" + String(imuReadingsCount), jsonEntry);

                    //Serial.println(imuReadingsCount);
                } else {
                    xSemaphoreGive(xSemaphore);
                }
            }
        }
        vTaskDelay(1);
    }
}

void sendDataToFirebase(void * parameter) {
    while (true) {
        // Protect shared resources with semaphore
        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        // check if the number of readings to send to Firebase has been reached
        if (imuReadingsCount >= readingsThreshold) {
            // Swap buffers
            FirebaseJson* temp = activeJsonData;
            activeJsonData = sendingJsonData;
            sendingJsonData = temp;

            imuReadingsCount = 0; // Reset the count for the new active buffer

            xSemaphoreGive(xSemaphore); // Release the semaphore before sending data

            while (WiFi.status() != WL_CONNECTED) {
                StickCP2.Display.clear();
                StickCP2.Display.setCursor(10, 20);
                StickCP2.Display.printf("IMU activated");
                StickCP2.Display.setCursor(10, 50);
                StickCP2.Display.printf("Wifi not connected");
                delay(500);
            }

            if (WiFi.status() == WL_CONNECTED) {
                StickCP2.Display.clear();
                StickCP2.Display.setCursor(10, 20);
                StickCP2.Display.printf("IMU activated");
            }

            // Send the data from the sending buffer
            if (sendingJsonData != nullptr) {
                Serial.println("Sending data to Firebase...");
                if (Firebase.pushJSON(firebaseData, "/IMUData", *sendingJsonData)) {
                    Serial.println("Data sent successfully!");
                    sendingJsonData->clear(); // Clear the sending buffer after sending
                } else {
                    Serial.println("Error sending data: " + firebaseData.errorReason());
                    sendingJsonData->clear(); // Clear the sending buffer
                }
            } else {
                Serial.println("Error: sendingJsonData is null");
            }
        } else {
            xSemaphoreGive(xSemaphore); // Release the semaphore if the number of readings is not reached
        }

        vTaskDelay(1);
    }
}

void loop() {

}
