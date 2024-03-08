#include "Buffer.h"
#include "Errors.h"
#include "Debug.h"

bool IMUDataBuffer::isBufferEmpty() const {
    return bufferSize == 0;
}

bool IMUDataBuffer::isBufferFull() const {
    return bufferSize >= BUFFER_CAPACITY;
}

int IMUDataBuffer::getBufferCapacity() const {
    return BUFFER_CAPACITY;
}

int IMUDataBuffer::getBufferSize() const {
    return bufferSize;
}

int IMUDataBuffer::getReadIndex() const {
    return readIndex;
}

int IMUDataBuffer::getWriteIndex() const {
    return writeIndex;
}

void IMUDataBuffer::moveReadIndexForward() {
    readIndex = (readIndex + 1) % BUFFER_CAPACITY;
    bufferSize--;
}

void IMUDataBuffer::moveReadIndexBackward() {
    readIndex = (readIndex - 1 + BUFFER_CAPACITY) % BUFFER_CAPACITY;
    bufferSize++;
}

void IMUDataBuffer::moveWriteIndexForward() {
    writeIndex = (writeIndex + 1) % BUFFER_CAPACITY;
    bufferSize++;
}

void IMUDataBuffer::moveWriteIndexBackward() {
    writeIndex = (writeIndex - 1 + BUFFER_CAPACITY) % BUFFER_CAPACITY;
    bufferSize--;
}

time_t IMUDataBuffer::getCurrentSampleSeconds() const {
    return buffer[readIndex].timestampMillis / 1000ULL;
}

void IMUDataBuffer::computeCurrentSampleDate(char* sampleDate) {
    // Get the next sample from the buffer
    time_t sampleTimestampSec = getCurrentSampleSeconds();
    // Get the time information from the time_t object
    localtime_r(&sampleTimestampSec, &timeInfo);
    // Convert the struct tm to a formatted string like "YYYY-MM-DD\0"
    strftime(sampleDate, 11, "%F", &timeInfo);
}

void IMUDataBuffer::computeNextDaySeconds() {
    time_t sampleTimestampSec = getCurrentSampleSeconds();
    // Get the time info of the next day
    sampleTimestampSec += 24 * 60 * 60;
    localtime_r(&sampleTimestampSec, &timeInfo);
    // Set the time info to the start of the day
    timeInfo.tm_hour = 0;
    timeInfo.tm_min = 0;
    timeInfo.tm_sec = 0;

    // Convert back to time_t and save it
    nextDay = mktime(&timeInfo);
}

bool IMUDataBuffer::hasDateChanged() {
    // Compare the current timestamp with the timestamp of the next day
    return getCurrentSampleSeconds() >= nextDay;
}

bool IMUDataBuffer::isSampleNull(const imuData* sample) const {
    // Check if we have some non null data in the sample struct
    for (int i = 0; i < 3; i++) {
        if (sample->accelData[i] != 0) {
            return false;
        }
        if (sample->gyroData[i] != 0) {
            return false;
        }
        if (sample->magData[i] != 0) {
            return false;
        }
    }

    // Return true if all the sample data is null
    return true;
}

const imuData* IMUDataBuffer::getSample() {
    // If the buffer is empty
    if (isBufferEmpty()) {
        // Return nullptr if the sample was not retrieved from the buffer
        return nullptr;
    }

    // Get the next sample from the buffer
    const imuData* ptrSample = &buffer[readIndex];

    // Move the read index to the next sample
    moveReadIndexForward();

    // Return the sample if it was retrieved from the buffer
    return ptrSample;
}

imuData* IMUDataBuffer::getNewSample() {
    // If the buffer is full
    if (isBufferFull()) {
        
        // Print an error message and restart the device
        errorHandler.showError(ErrorType::BufferFull, true);
        
        // Return nullptr if the sample was not added to the buffer
        return nullptr;
    }

    // Get the pointer to the next sample to be written
    imuData* ptrSample = &buffer[writeIndex];

    // Move the write index to the next sample
    moveWriteIndexForward();

    // Return the pointer to the next sample to be written
    return ptrSample;
}

void IMUDataBuffer::printBufferState() const {
    // If the buffer gets full
    if (isBufferFull()) {
        // Print an error message and restart the device
        errorHandler.showError(ErrorType::BufferFull, true);
    }

    // Prints the buffer state
    LogVerboseln("Buffer state: ", bufferSize, "/", BUFFER_CAPACITY);
}

void IMUDataBuffer::printBufferIndexes() const {
    LogVerboseln("Buffer index: R=", readIndex, " W=", writeIndex);
}

void IMUDataBuffer::dumpBufferContent(int start, int end) const {
    // Iterate over the buffer content on the desired range
    for (int i = start; i < end; i++) {
        // Get the current sample
        const imuData* sample = &buffer[i];

        // Print the sample timestamp
        LogDebug(sample->timestampMillis, " ");

        // Print the sample IMU sensor values
        for (int j = 0; j < 3; j++) {
            LogDebug(sample->accelData[j], " ");
        }
        for (int j = 0; j < 3; j++) {
            LogDebug(sample->gyroData[j], " ");
        }
        for (int j = 0; j < 3; j++) {
            LogDebug(sample->magData[j], " ");
        }

        // Print the end of the line
        LogDebugln("\n");
    }
}