#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <FirebaseESP32.h>
#include "config.h" // change this file to your own config.h

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

bool imuActive = true; // Variable to control the state of the IMU
int imuReadingsCount = 0; // IMU readings counter
const int readingsThreshold = 79; // Number of readings before sending to Firebase

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
    M5.begin();

    // connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
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
    while(!Firebase.ready()){
        Serial.println("Error: Firebase is not ready");
        delay(1000); // Keep the ESP32 in wait for debugging
        Firebase.begin(&config, &auth);
    }
    Firebase.reconnectWiFi(true);

    // create semaphore for variable synchronization
    xSemaphore = xSemaphoreCreateMutex();

        // create tasks
    xTaskCreatePinnedToCore(
        collectIMUData,   // Task function
        "Task1",          // Task name
        15000,            // Stack size
        NULL,             // Task input parameter
        1,                // Priority of the task
        &Task1,           // Task handle
        0);               // Core 0

    xTaskCreatePinnedToCore(
        sendDataToFirebase,  // Task function
        "Task2",             // Task name
        15000,               // Stack size
        NULL,                // Task input parameter
        2,                   // Priority of the task
        &Task2,              // Task handle
        1);                  // Core 1

    Serial.println("Configuration completed");
}

void collectIMUData(void * parameter) {
    float ax, ay, az; // accelerometer variables
    float gx, gy, gz; // gyroscope variables
    String current_time;

    while (true) {
        // check if button A was pressed
        if (M5.BtnA.wasPressed()) {
            imuActive = !imuActive; // toggle IMU state
            if (imuActive) {
                Serial.println("IMU activated");

                //update real time
                timeClient.update();
                current_time = timeClient.getFormattedTime();
            } else {
                Serial.println("IMU deactivated");
            }

            while(M5.BtnA.isPressed()) {
                M5.update();
                delay(10);
            }
        }

        // update button state
        M5.update();

        // collect and send IMU data if active
        if (imuActive) {

            // check the interrupt
            if (timerFlag) {
                // update the time
                timeClient.update();
                current_time = timeClient.getFormattedTime();
                Serial.println(current_time);
                
                timerFlag = false; // reset the flag
            }

            // update IMU data
            if (M5.Imu.update()) {
                // get accelerometer and gyroscope data
                M5.Imu.getAccelData(&ax, &ay, &az);
                M5.Imu.getGyroData(&gx, &gy, &gz);

                // protect shared resources with semaphore
                xSemaphoreTake(xSemaphore, portMAX_DELAY);

                if (imuReadingsCount < readingsThreshold){
                    imuReadingsCount++;
                    xSemaphoreGive(xSemaphore);

                    // store readings in active JSON
                    FirebaseJson jsonEntry;
                    jsonEntry.set("aX", ax);
                    jsonEntry.set("aY", ay);
                    jsonEntry.set("aZ", az);
                    jsonEntry.set("gX", gx);
                    jsonEntry.set("gY", gy);
                    jsonEntry.set("gZ", gz);
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
        // protect shared resources with semaphore
        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        // check if the number of readings to send to Firebase has been reached
        if (imuReadingsCount >= readingsThreshold) {
            // swap buffers
            FirebaseJson* temp = activeJsonData;
            activeJsonData = sendingJsonData;
            sendingJsonData = temp;

            imuReadingsCount = 0; // reset the count for the new active buffer

            xSemaphoreGive(xSemaphore);

            if (sendingJsonData != nullptr) {
                //Serial.println("Sending data to Firebase...");
                if (Firebase.pushJSON(firebaseData, "/IMUData", *sendingJsonData)) {
                    Serial.println("Data sent successfully!");
                    sendingJsonData->clear();
                } else {
                    Serial.println("Erro ao enviar dados: " + firebaseData.errorReason());
                    sendingJsonData->clear(); // clear the sending buffer
                } 
            } else {
                Serial.println("Error: sendingJsonData is null");
            }
              
        } else {
            xSemaphoreGive(xSemaphore);
        }
    
        vTaskDelay(1);
    }
}


void loop() {
    // Loop vazio, tarefas são gerenciadas pelo FreeRTOS
}
