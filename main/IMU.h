/**
 * @file IMU.h
 * @brief Definition of the IMU class for handling operations with the ICM-20948 sensor.
 */

#ifndef IMU_H
#define IMU_H

#include "ICM_20948.h"

/**
 * @class IMU
 * @brief This class encapsulates operations for initializing and reading data from an IMU sensor (ICM-20948).
 */
class IMU {
public:
    /**
     * Constructor.
     * @param number Unique identifier for the IMU sensor.
     * @param CSpin Pin number for the SPI chip select.
     * @param name Descriptive name for the IMU sensor.
     */
    IMU(int number, int CSpin, const char* name);

    /**
     * Initializes the IMU sensor, setting up SPI communication and configuring the sensor.
     * @return true if initialization was successful, false otherwise.
     */
    bool init();

    /**
     * Reads data from the sensor, including acceleration, gyroscope, and magnetometer readings.
     */
    bool readData();

    /**
     * Prints the current sensor data to the Serial port.
     */
    void printData();

    int16_t accelData[3]; // Accelerometer data: X, Y, Z.
    int16_t gyroData[3]; // Gyroscope data: X, Y, Z.
    int16_t magData[3]; // Magnetometer data: X, Y, Z.
    
    int IMUnumber; // Unique identifier for the IMU sensor.

private:
    ICM_20948_SPI myICM; // SPI object for communication with the sensor.
    int IMUpin; // Pin number for the SPI chip select.
    const char* IMUname; // Descriptive name for the IMU sensor.
    bool initialized = false; // Flag indicating whether the sensor has been initialized.

    /**
     * Activates the necessary sensors in the IMU for data collection.
     * @param success Pointer to a boolean that indicates the success of sensor activation.
     */
    void activateSensors(bool *success);

    /**
     * Sets the output data rate (ODR) for the sensors.
     * @param success Pointer to a boolean that indicates the success of setting the ODR.
     */
    void setOdrRate(bool *success);
};

#endif // IMU_H
