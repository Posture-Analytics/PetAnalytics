#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include "LocalDatabase.h"
#include "Errors.h"
#include "Network.h"
#include "Buffer.h"
#include "Debug.h"
#include "Base64Encoding.h"
#include "SDCard.h"

LocalDatabase::LocalDatabase() : last_was_valid(true) {}

void LocalDatabase::setup(SPIClass& vspi) {
    sdCard.init(vspi);
    sdCard.createFile(local_database_file_path);
}

void LocalDatabase::appendDataToJSON(const imuData* data) {
    char dataString[256];
    snprintf(dataString, sizeof(dataString), "%d,%llu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
             data->IMUid, data->timestampMillis, 
             data->accelData[0], data->accelData[1], data->accelData[2],
             data->gyroData[0], data->gyroData[1], data->gyroData[2],
             data->magData[0], data->magData[1], data->magData[2]);

    if (strlen(payloadBuffer) + strlen(dataString) < PAYLOAD_BUFFER_SIZE) {
        strcat(payloadBuffer, dataString);
    } else {
        // Handle buffer overflow (optional)
        LogErrorln("Payload buffer overflow");
    }
}

bool LocalDatabase::pushData() {
    #ifdef DEBUG_LEVEL
        LogDebugln("Payload: ", payloadBuffer);
    #else
        sdcard_file = sdCard.openFile(local_database_file_path, 1);
        sdcard_file->print(payloadBuffer);
        sdcard_file->close();
    #endif

    payloadBuffer[0] = '\0'; // Clear the buffer
    return true;
}

void LocalDatabase::sendData(IMUDataBuffer* dataBuffer) {
    if (strlen(payloadBuffer) > 0) {
        pushData();
    } else {
        const imuData* sample = dataBuffer->getSample();
        appendDataToJSON(sample);
    }

    dataBuffer->printBufferState();
    LogVerboseln("JSON buffer: ", jsonSize, "/", JSON_BATCH_SIZE);
    dataBuffer->printBufferIndexes();

    vTaskDelay(1);
    yield();
}
