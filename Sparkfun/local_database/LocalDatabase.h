#ifndef LocalDatabase_H_
#define LocalDatabase_H_

#include "Buffer.h"
#include "Base64Encoding.h"
#include "SDCard.h"

const int SEND_RATE = 10;
const int PAYLOAD_MAX_SAMPLES = 100;
const int PAYLOAD_BUFFER_SIZE = 1024;

class LocalDatabase {
    static const int JSON_BATCH_SIZE = 3;

    int jsonSize = 0;

    char sampleDate[12];
    char DATABASE_BASE_PATH[64] = "/sensor_data/";
    char fullDataPath[128];

    char payloadBuffer[PAYLOAD_BUFFER_SIZE] = "";
    bool last_was_valid;

    const int dataSendIntervalMicros = 1e6 / SEND_RATE;
    unsigned long dataPrevSendingMicros = 0;
    unsigned long currentMicros = 0;

    void updateCurrentTime();

    uint8_t deltaTime = 0;

    char encodedIDAndDeltaTime[64];
    char encodedAccelData[64];
    char encodedGyroData[64];
    char encodedMagData[64];

    char encodedTimestampPath[6];
    unsigned long long lastTimestamp = 0;

    Base64Encoder encoder;

    File* sdcard_file;

    SDCard sdCard = SDCard(5);

    const char* local_database_file_path = "/localDatabase.csv";

public:
    LocalDatabase();

    void setup(SPIClass& vspi);
    void appendDataToJSON(const imuData* data);
    bool pushData();
    void sendData(IMUDataBuffer* dataBuffer);
};

#endif
