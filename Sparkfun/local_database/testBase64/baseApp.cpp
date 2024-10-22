#include "base64Encoding.h"
#include <iostream>

using namespace std;

int main() {
    // Sensor data encoding and decoding
    int16_t inputValues[3] = {12345, -12345, 32766};
    char encoded[8];
    int16_t decodedValues[3];

    Base64Encoder::encodeSensorData(inputValues, encoded);
    Base64Encoder::decodeSensorData(encoded, decodedValues);

    cout << "Encoded sensor data: ";
    for (int i = 0; i < 8; i++) cout << encoded[i];
    cout << "\nDecoded sensor data: ";
    for (int i = 0; i < 3; i++) cout << decodedValues[i] << " ";
    cout << endl;

    // Timestamp encoding and decoding
    uint32_t timestamp = 86399999;
    char encodedTimestamp[6];
    Base64Encoder::encodeTimestamp(timestamp, encodedTimestamp);
    uint32_t decodedTimestamp = Base64Encoder::decodeTimestamp(encodedTimestamp);
    cout << "Encoded timestamp: " << encodedTimestamp << endl;
    cout << "Decoded timestamp: " << decodedTimestamp << endl;

    // ID and deltaTime encoding and decoding
    uint8_t id = 0, deltaTime = 0, decodedId, decodedDeltaTime;
    char encodedChar = Base64Encoder::encodeIDAndDeltaTime(id, deltaTime);
    Base64Encoder::decodeToIDAndDeltaTime(encodedChar, decodedId, decodedDeltaTime);
    cout << "Encoded (ID, deltaTime): " << encodedChar << endl;
    cout << "Decoded to ID: " << static_cast<int>(decodedId) << ", deltaTime: " << static_cast<int>(decodedDeltaTime) << endl;



    
    uint32_t decodedTimestampReal = Base64Encoder::decodeTimestamp("8MAL9");
    cout << "Decoded timestamp: " << decodedTimestampReal << endl;

    return 0;
}
