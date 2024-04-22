#include "time.h"
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>

#include "Errors.h"
#include "Credentials.h"
#include "Network.h"
#include "Buffer.h"
#include "DataReader.h"
#include "Database.h"
#include "Debug.h"

// Create a errors object to handle them
Errors errorHandler;

// Create a task to assign the data push to the database to Core 0
TaskHandle_t sendToDatabaseTask;

// Create a buffer to store the data to be sent to the database
IMUDataBuffer dataBuffer;

// Create a DataReader object to read the data from the sensors
DataReader dataReader;

// Create a Database object to send the data to the database
Database database;

void setup() {

    // Initialize the serial port and the SPI bus
    Serial.begin(115200);
    SPI.begin();

    // Initialize the IMUs
    if (!dataReader.setup()) {
        errorHandler.showError(ErrorType::IMUInitFailure, true);
        LogFatalln("Failed to initialize IMUs");
    } else {
    	LogInfoln("IMUs initialized");
    }

    // Connect to the WiFi network
    setupWiFi();

    // Sync with the NTP time
    syncWithNTPTime();

    // Setup the Firebase Database connection
    database.setup(getCurrentTime());

    // Assign the task of sending data to the database to Core 0
    xTaskCreatePinnedToCore(
        sendToDatabase,          // Task function
        "sendToDatabaseLoop",    // Name of task
        10000,                   // Stack size of task
        NULL,                    // Parameter of the task
        1,                       // Priority of the task
        &sendToDatabaseTask,     // Task handle to keep track of created task
        0);                      // Pin task to core 0

    // Sanity delay
    delay(100);

    // Register the boot on the database ("/bootLog")
    database.bootLog();

    errorHandler.showError(ErrorType::None);
}

// Main loop, that keep running on Core 1
void loop() {
    // Disable the watchdog of Core 0, avoiding reboots caused by
    // the working time of the sendToDatabase task
    disableCore0WDT();

    dataReader.fillBuffer(&dataBuffer);

    // Print buffer state
    dataBuffer.printBufferState();
    dataBuffer.printBufferIndexes();
}

// Task attached to core 0
void sendToDatabase(void* pvParameters) {
    // A loop that runs forever to keep sending data to the database
    while (true) {
        // If the buffer is empty, wait for the data collection task to fill it
        if (!dataBuffer.isBufferEmpty()) {
            database.sendData(&dataBuffer);
        } else {
            vTaskDelay(2);
            yield();
        }
    }
}