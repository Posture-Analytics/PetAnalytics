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
int imuReadingsCount = 1; // IMU readings counter
const int readingsThreshold = 80; // number of readings before sending to Firebase

// Initialize NTPClient with the specified NTP server ("pool.ntp.org"), brazil offset (-10800), 
// and a 60-second update interval (60000 ms) using the provided WiFiUDP object (ntpUDP) for UDP communication.
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 600);

// Double buffers for storing IMU readings
FirebaseJson jsonData1;
FirebaseJson jsonData2;

// Pointers to the active buffer and sending buffer
FirebaseJson* activeJsonData = &jsonData1;
FirebaseJson* sendingJsonData = &jsonData2;

TaskHandle_t Task1;
TaskHandle_t Task2;

SemaphoreHandle_t xSemaphore;

void setup() {
    // initialize serial
    Serial.begin(115200);

    // initialize device
    auto cfg = M5.config();
    StickCP2.begin(cfg);

    // connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to Wi-Fi!");
    timeClient.begin();

    // configure Firebase
    config.api_key = DATABASE_API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = DATABASE_USER_EMAIL;
    auth.user.password = DATABASE_USER_PASSWORD;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Create semaphore for variable synchronization
    xSemaphore = xSemaphoreCreateMutex();

    // Create tasks
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
}

void collectIMUData(void * parameter) {
    unsigned long lastTimestamp = 0;

    while (true) {
        // check if button A was pressed
        if (StickCP2.BtnA.wasPressed()) {
            imuActive = !imuActive; // toggle IMU state
            if (imuActive) {
                Serial.println("IMU activated");
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

        // check if button B was pressed
        if (StickCP2.BtnB.wasPressed()) {
            StickCP2.Display.clear();
            StickCP2.Display.setCursor(10, 20);

            // clear the /IMUData node (for test only)
            if (Firebase.deleteNode(firebaseData, "/IMUData")) {
                Serial.println("Node /IMUData cleared successfully!");
                StickCP2.Display.printf("Node /IMUData cleared successfully!");
            } else {
                Serial.println("Error clearing node /IMUData: " + firebaseData.errorReason());
                StickCP2.Display.printf("Error clearing node /IMUData");
            }

            delay(200);
            StickCP2.Display.clear();
            Serial.println("IMU activated");
        }

        // update button state
        StickCP2.update();

        // collect and send IMU data if active
        if (imuActive) {
            auto imu_update = StickCP2.Imu.update();

            if (imu_update) {

                // Protect shared resources with semaphore
                xSemaphoreTake(xSemaphore, portMAX_DELAY);

                if (imuReadingsCount < readingsThreshold){
                  auto data = StickCP2.Imu.getImuData();

                if (activeJsonData != nullptr) {
                    // store readings in active JSON
                    FirebaseJson jsonEntry;
                    jsonEntry.set("aX", data.accel.x);
                    jsonEntry.set("aY", data.accel.y);
                    jsonEntry.set("aZ", data.accel.z);
                    jsonEntry.set("gX", data.gyro.x);
                    jsonEntry.set("gY", data.gyro.y);
                    jsonEntry.set("gZ", data.gyro.z);
                    activeJsonData->add(String(imuReadingsCount), jsonEntry);

                    imuReadingsCount++;
                    Serial.println(imuReadingsCount);
                    }
                  
                  xSemaphoreGive(xSemaphore);
                }
            }

            // check if 1 second has passed since the last marker addition
            if (millis() - lastTimestamp >= 1000) {
                timeClient.update();
                // Protect shared resources with semaphore
                xSemaphoreTake(xSemaphore, portMAX_DELAY);

                activeJsonData->add(imuReadingsCount, timeClient.getFormattedTime());

                imuReadingsCount++;

                xSemaphoreGive(xSemaphore);

                lastTimestamp = millis(); // update last marker time
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

            imuReadingsCount = 1; // Reset the count for the new active buffer

            xSemaphoreGive(xSemaphore); // Release the semaphore before sending data

            // Send the data from the sending buffer
            if (sendingJsonData != nullptr) {
                Serial.println("Sending data to Firebase...");
                if (Firebase.pushJSON(firebaseData, "/IMUData", *sendingJsonData)) {
                    Serial.println("Data sent successfully!");
                    sendingJsonData->clear(); // Clear the sending buffer after sending
                } else {
                    Serial.println("Error sending data: " + firebaseData.errorReason());
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
    // Empty loop since tasks are managed by FreeRTOS
}
