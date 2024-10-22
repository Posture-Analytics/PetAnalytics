#ifndef SDCARD_H
#define SDCARD_H

#include <SD.h>

class SDCard {
public:
    /**
     * Constructor.
     * @param CSpin Pin number for the SPI chip select.
     */
    SDCard(int CSpin);

    bool init(SPIClass &spi);
    void printDir(const char *dirname, uint8_t levels);
    bool createDir(const char *path);
    bool readFile(const char *path);
    bool createFile(const char *path);
    bool writeFile(File*, float);
    bool renameFile(const char *path1, const char *path2);
    bool deleteFile(const char *path);
    File* openFile(const char *path, bool mode);
    bool closeFile(File* file);

private:
    void printCardType();
    void printCardSize();

    int SDpin;
    uint8_t cardType;
    uint64_t cardSize;
    const char *columns = "ID_IMU; TIMESTAMP; ACC_X; ACC_Y; ACC_Z; GYR_X; GYR_Y; GYR_Z; MAG_X; MAG_Y; MAG_Z\n";
};

#endif