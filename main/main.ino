#include "time.h"
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>

#include "Errors.h"
#include "Credentials.h"
#include "Network.h"
#include "Buffer.h"
#include "DataReader.h"
#include "Debug.h"

// Create a errors object to handle them
Errors errorHandler;

// Create a buffer to store the data to be sent to the database
IMUDataBuffer dataBuffer;

// Create a DataReader object to read the data from the sensors
DataReader dataReader;

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

    // Show that the setup was successful
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