/*
    DataReader.h

    * This module handle the sensors and the data collection from them.
    * It setup the sensors and the constants related to them
    * It also handle the routine to collect data from the sensors and store it on the buffer.
*/

#ifndef DataReader_H_
#define DataReader_H_

#include "IMU.h"
#include "Buffer.h"

/**
 * This class handle the sensors and the data collection from them.
 * It setup the sensors and the constants related to them
 * It also handle the routine to collect data from the sensors and store it on the buffer.
*/
class DataReader {
    // /** Update the current time variable */
    // void updateCurrentTime();  

public:

    // Array of pointers to the IMUs
    IMU* IMUarray[3];

    /**
     * Setup the sensors and the devices' pins
     * 
     * @return true if everything went well, false otherwise
     */
    bool setup(SPIClass& hspi);

    /**
     * Collect data from the sensors and store it in the buffer location represented by the pointer
     * 
     * @param dataBuffer: Pointer to the buffer where the data will be stored
     */
    void addDataToSample(imuData* newSample, IMU** currentIMU);

    /**
     * Fill buffer if the moment of the function call is greater than the data collection interval
     * 
     * @param dataBuffer: Pointer to the buffer where the data will be stored
     */
    void fillBuffer(IMUDataBuffer* dataBuffer);
};

#endif  // DataReader_H_