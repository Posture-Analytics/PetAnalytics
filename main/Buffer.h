/*
    Buffer.h

    * This module handles the buffer that stores the data collected from the IMUs.
    * The buffer consists of an array of structs and some indexes to keep track of
    the buffer state. It also provides functions to add and get samples from the buffer,
    handle buffer capacity and indexes.
    * For debug purposes, it also provides functions to print the buffer state and dump
    its content.
*/

#ifndef Buffer_H_
#define Buffer_H_

#include <time.h>
#include <Arduino.h>

// Define the capacity of the buffer
const int BUFFER_CAPACITY = 1024;

/**
 * Struct to organize the collected data
 * 
 * timestampMillis: timestamp of the sample in milliseconds
 * IMUid: unique identifier of the IMU that collected the sample
 * accelData: array of 3 int16_t values representing the acceleration in X, Y and Z
 * gyroData: array of 3 int16_t values representing the gyroscope in X, Y and Z
 * magData: array of 3 int16_t values representing the magnetometer in X, Y and Z
 */
struct imuData {
    // 4 bytes
    unsigned long timestampMillis = 0;

    // 2 bytes
    byte IMUid = 0;

    // 3 (X, Y, Z) x 2 bytes (each value is 16 bits)
    int16_t accelData[3] = {0}; 
    int16_t gyroData[3] = {0};
    int16_t magData[3] = {0};
};

// Define a class to store the collected data

/**
 * Class that handles the buffer that stores the data collected from the IMUs.
 * The buffer consists of an array of imuData structs and some indexes to keep track of
 * the buffer state. It also provides functions to add and get samples from the buffer,
 * handle buffer capacity and indexes.
 * For debug purposes, it also provides functions to print the buffer state and dump
 * its content.
 * 
 * @param buffer the array of data structs that stores the collected data
 * @param bufferSize the number of samples in the buffer
 * @param readIndex the index of the next sample to be read
 * @param writeIndex the index of the next sample to be written
*/
class IMUDataBuffer {

    // Sotre information regarding the time of the read samples
    struct tm timeInfo;

    // Store the timestamp of the following day to check if the date has changed
    // The initial value is 0 so that it gets updated during the first use
    time_t nextDay = 0;

public:

    // Create a buffer based on the imuData struct
    imuData buffer[BUFFER_CAPACITY];

    // Hold the number of samples in the buffer
    int bufferSize = 0;

    // Index to read the next sample
    int readIndex = 0;
    // Index to write the next sample
    int writeIndex = 0;

    /**
     * Check if the buffer is empty
     * 
     * @return true if the buffer is empty, false otherwise
     */
    bool isBufferEmpty() const;

    /**
     * Check if the buffer is full
     * 
     * @return true if the buffer is full, false otherwise
     */
    bool isBufferFull() const;

    // Get the capacity of the buffer
    /**
     * Get the capacity of the buffer
     * 
     * @return the capacity of the buffer
     */
    int getBufferCapacity() const;

    /**
     * Get the number of samples in the buffer
     * 
     * @return the number of samples in the buffer
     */
    int getBufferSize() const;

    /**
     * Get the index of the next sample to be read
     * 
     * @return the index of the next sample to be read
     */
    int getReadIndex() const;

    /**
     * Get the index of the next sample to be written
     * 
     * @return the index of the next sample to be written
     */
    int getWriteIndex() const;

    /** 
     * Move the read index to the next sample 
     */
    void moveReadIndexForward();

    /** 
     * Move the read index to the previous sample 
     */
    void moveReadIndexBackward();

    /** 
     * Move the write index to the next sample 
     */
    void moveWriteIndexForward();

    /** 
     * Move the write index to the previous sample in the buffer 
     */
    void moveWriteIndexBackward();

    /**
     * Get the timestamp of the next sample to be read from the buffer
     * 
     * @return the timestamp of the next sample to be read from the buffer
     */
    time_t getCurrentSampleSeconds() const;

    /**
     * Get the current sample date. Should receive an array of at least 11 chars
     * 
     * @param sampleDate the array of chars to store the sample date
     */
    void computeCurrentSampleDate(char* sampleDate);

    /**
     * Compute the timestamp of the start of the following day
     */
    void computeNextDaySeconds();

    /**
     * Check if the date has changed
     * 
     * @return true if the date has changed, false otherwise
     */
    bool hasDateChanged();

    /**
     * Check if the content of the sample is null
     * 
     * @param sample the sample to check
     * @return true if the sample is null, false otherwise
     */
    bool isSampleNull(const imuData* sample) const;

    /**
     * Get the next sample from the buffer at the read index and increment it
     * 
     * @return a pointer to the next sample to be read
     */
    const imuData* getSample();

    /**
     * Get the next sample from the buffer at the write index and increment it
     * 
     * @return a pointer to the next sample to be written
     */
    imuData* getNewSample();

    /**
     * Print the buffer state, the number of samples in the buffer and its capacity
     */
    void printBufferState() const;

    /**
     * Print the indexes of the buffer
     */
    void printBufferIndexes() const;

    /**
     * Dump the buffer content or a part of it
     * 
     * @param start the index of the first sample to dump
     * @param end the index of the last sample to dump
     */
    void dumpBufferContent(int start = 0, int end = BUFFER_CAPACITY) const;
};

#endif  // Buffer_H_