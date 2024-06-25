#include "DataReader.h"
#include "Network.h"
#include "Buffer.h"
#include "PINConfig.h"

#include "SPI.h"

bool DataReader::setup(SPIClass& hspi) {
    // // Instantiate the IMUs
    // IMUarray[0] = new IMU(0, 12, "IMU-A");
    // IMUarray[1] = new IMU(1, 13, "IMU-B");
    // IMUarray[2] = new IMU(2, 27, "IMU-C");
    IMUarray[0] = new IMU(0, CS_1_IMU, "IMU-A");
    IMUarray[1] = new IMU(1, CS_2_IMU, "IMU-B");
    IMUarray[2] = new IMU(2, CS_3_IMU, "IMU-C");

    // Set a flag to indicate whether the initialization was successful
    bool success = true;

    // Initialize the IMUs
    for (int i = 0; i < 3; i++) {
        success &= IMUarray[i]->init(hspi);
    }

    // If everything went well, return true
    return success;
}

void DataReader::addDataToSample(imuData* newSample, IMU** currentIMU) {
    // Fill the buffer with current timestamp (in milliseconds)
    newSample->timestampMillis = getCurrentMillisTimestamp();

    // Fill the IMU ID into the buffer
    newSample->IMUid = (*currentIMU)->IMUnumber;
    
    // Fill the buffer with the data from the IMU
    for (int i = 0; i < 3; i++) {
        newSample->accelData[i] = (*currentIMU)->accelData[i];
        newSample->gyroData[i] = (*currentIMU)->gyroData[i];
        newSample->magData[i] = (*currentIMU)->magData[i];
    }
}

void DataReader::fillBuffer(IMUDataBuffer* dataBuffer) {

    // Iterate over the IMUs and read data from them
    for (int i = 0; i < 3; i++) {
        // If the data was read successfully
        if (IMUarray[i]->readData()) {
            // Pointer to the next sample to be written
            imuData* newSample = dataBuffer->getNewSample();

            // Add the data to the sample
            addDataToSample(newSample, &IMUarray[i]);
        }
    }
}