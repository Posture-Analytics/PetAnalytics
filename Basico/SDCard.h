#ifndef SDCARD_H
#define SDCARD_H

#include <SD.h>
#include <SPI.h>

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
    bool writeFile(File*, float);
    bool renameFile(const char *path1, const char *path2);
    bool deleteFile(const char *path);
    File* openFile(const char *path, bool mode);
    bool closeFile(File* file);

    uint8_t cardType;
    uint64_t cardSize;
    const char *columns = "ID_IMU; TIMESTAMP; ACC_X; ACC_Y; ACC_Z; GYR_X; GYR_Y; GYR_Z; MAG_X; MAG_Y; MAG_Z\n";
};

#endif