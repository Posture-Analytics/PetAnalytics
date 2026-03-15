#include <M5Unified.h>
#include "IMU.h"
#include "Network.h"
#include "Firebase.h"
#include "Interrupt.h"

// Semaphore to protect imuReadingsCount
SemaphoreHandle_t xCountSemaphore = NULL;

TaskHandle_t collectIMUTaskHandle;
TaskHandle_t sendDataTaskHandle;

bool imuActive = false; // State of the IMU
int imuReadingsCount = 0;

//Interrupt
volatile bool timerFlag = true;

void IRAM_ATTR onTimer() {
    timerFlag = true;
}

void setup() {
    // initialize serial
    Serial.begin(115200);

    // initialize device
    M5.begin();

    // Network and time setup
    Network::connectToWiFi();
    Network::syncTimeWithNTP();

    // Interrupt setup
    Interrupt::initializeExactSecondInterrupt(onTimer);
    
    // Firebase setup
    Firebase::initialize();

    // Create semaphore for variable synchronization
    xCountSemaphore = xSemaphoreCreateMutex();

    // create tasks
    xTaskCreatePinnedToCore(
        collectIMUData,     // Task function
        "Task1",            // Task name
        15000,              // Stack size
        NULL,               // Task input parameter
        1,                  // Priority of the task
        &Task1,             // Task handle
        0);                 // Core 0

    xTaskCreatePinnedToCore(
        sendDataToFirebase,  // Task function
        "Task2",             // Task name
        15000,               // Stack size
        NULL,                // Task input parameter
        2,                   // Priority of the task
        &Task2,              // Task handle
        1);                  // Core 1

    Serial.println("Configuration completed.");
}

void collectIMUData(void * parameter) {
    float ax, ay, az; // accelerometer variables
    float gx, gy, gz; // gyroscope variables

    while (true) {
        // update button state
        M5.update();

        // check if button A was pressed, then toggle IMU state
        if (M5.BtnA.wasPressed()) {
            imuActive = IMU::toggleState();
        }

        // update the time each second
        if (timerFlag) {
            current_time = Network::getFormattedTimeWithoutMillis();
            timerFlag = false
        }

        // collect and send IMU data if active
        if (imuActive && imuReadingsCount < Firebase::getReadingsThreshold()){
            IMU::collectReadings(ax, ay, az, gx, gy, gz);

            xSemaphoreTake(xCountSemaphore, portMAX_DELAY);
            imuReadingsCount++;
            xSemaphoreGive(xCountSemaphore);

            IMU::saveToJson(imuReadingsCount, currentTime, ax, ay, az, gx, gy, gz);
        }
    vTaskDelay(1); 
    } 
}

void sendDataToFirebase(void * parameter) {
    while (true) {
        if (imuReadingsCount >= Firebase::getReadingsThreshold()) {
            Firebase::sendData();
        }
        vTaskDelay(1);
    }
}

void loop(){

}
