#ifndef SDCARD_H
#define SDCARD_H

#include <SD.h>
#include <SPI.h>
#include "Buffer.h"

class SDCard {
  public:
    SDCard();

    bool begin();
    void printCardType();
    void printCardSize();
    void printDir(const char *dirname, uint8_t levels);
    bool createDir(const char *path);
    bool readFile(const char *path);
    bool createFile(const char *path);
    bool writeFile(const char *path, const imuData* sample, const char *IMU_ID);

    uint8_t cardType;
    uint64_t cardSize;
    const char *columns = "ID_IMU, TIMESTAMP, ACC_X, GYR_X, MAG_X, ACC_Y, gyr_Y, MAG_Y, ACC_Z, gyr_Z, MAG_Z\n";
};

#endif