
#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include "LocalDatabase.h"
#include "Errors.h"
#include "Network.h"
#include "Buffer.h"
#include "Debug.h"
#include "Base64Encoding.h"

LocalDatabase::Database() : last_was_valid(true) {}

void LocalDatabase::setup(SPIClass& vspi) {
    // Assign the api key (required) of the database
    sdCard.init(vspi);
    sdCard.createFile(local_database_file_path);
}

void LocalDatabase::appendDataToJSON(const imuData* data) {
    // Concatenate the original data into a single string
    payloadString += String(data->IMUid) + "," + String(data->timestampMillis) + ","
            + String(data->accelData[0]) + "," + String(data->accelData[1]) + "," + String(data->accelData[2]) + ","
            + String(data->gyroData[0]) + "," + String(data->gyroData[1]) + "," + String(data->gyroData[2]) + ","
            + String(data->magData[0]) + "," + String(data->magData[1]) + "," + String(data->magData[2]) + "\n";
}

bool LocalDatabase::pushData() {
    #ifdef DEBUG_LEVEL

        // In debug mode, we only print the values instead of sending them to SD Card
        LogDebugln("Payload: ", payloadString);

    #else
        // In release mode, we send the data to the SD Card
        sdcard_file = sdCard.openFile(local_database_file_path, 1);
        sdcard_file -> print(payloadString);
        sdcard_file -> close();

        payloadString = "";

    #endif
}

void LocalDatabase::sendData(IMUDataBuffer* dataBuffer) {
    // Push the file if the string is not empty or if the string has more than 1000 characters
    if (payloadString.length() > 0) {
        pushData();
    }

    else {
        // Get one sample from the sensor data buffer
        const imuData* sample = dataBuffer->getSample();
        appendDataToJSON(sample);
    }

    dataBuffer->printBufferState();

    // Print the size of the JSON buffer
    LogVerboseln("JSON buffer: ", jsonSize, "/", JSON_BATCH_SIZE);

    dataBuffer->printBufferIndexes();

    // If we just send an amount of data to the database,
    // give an interval to Core 0 to work on maintence activities, avoiding crash problems
    vTaskDelay(1);
    yield();
}