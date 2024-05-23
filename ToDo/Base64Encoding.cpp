#include "base64Encoding.h"
#include "Debug.h"
// #include <iostream> // For error logging

// Definition of the Base64 encoding alphabet
const char* Base64Encoder::BASE64_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

// // Static member definition for the Base64 decode map
// std::map<char, int> Base64Encoder::BASE64_DECODE_MAP;

// const std::map<char, int>& Base64Encoder::getDecodeMap() {
//     if (BASE64_DECODE_MAP.empty()) {
//         // Populate the decode map using the Base64 alphabet
//         for (int i = 0; i < 64; ++i) {
//             BASE64_DECODE_MAP[BASE64_ALPHABET[i]] = i;
//         }
//     }
//     return BASE64_DECODE_MAP;
// }

void Base64Encoder::encodeSensorData(const int16_t input[3], char output[8]) {
    uint8_t bytes[6];
    // Convert the 3 int16_t values to 6 bytes
    for (int i = 0; i < 3; ++i) {
        bytes[i * 2] = (input[i] >> 8) & 0xFF;
        bytes[i * 2 + 1] = input[i] & 0xFF;
    }
    encodeBytesToBase64(bytes, output); // Perform Base64 encoding
}

// void Base64Encoder::decodeSensorData(const char input[8], int16_t output[3]) {
//     uint8_t bytes[6];
//     decodeBase64ToBytes(input, bytes); // Decode the Base64 input to bytes
//     // Convert the 6 bytes back into 3 int16_t values
//     for (int i = 0; i < 3; ++i) {
//         output[i] = (bytes[i * 2] << 8) | bytes[i * 2 + 1];
//     }
// }

void Base64Encoder::encodeTimestamp(uint32_t timestamp, char output[6]) {

    // Assert that the timestamp is within the valid range (0-86400000)
    if (timestamp >= 86400000) {
        LogFatalln("Invalid timestamp value: %u", timestamp);
        return; // Exit on error
    }

    // Ensure the timestamp uses only the lower 30 bits
    uint32_t temp = timestamp & 0x3FFFFFFF;
    // Encode each 6-bit chunk of the timestamp into a Base64 character
    for (int i = 4; i >= 0; --i) {
        output[i] = BASE64_ALPHABET[temp & 0x3F];
        temp >>= 6;
    }
    output[5] = '\0'; // Null-terminate the output string
}

// uint32_t Base64Encoder::decodeTimestamp(const char input[6]) {
//     const auto& decodeMap = getDecodeMap(); // Use the static decode map
//     uint32_t timestamp = 0;
//     // Decode each Base64 character back into the 30-bit timestamp
//     for (int i = 0; i < 5; ++i) {
//         auto it = decodeMap.find(input[i]);
//         if (it != decodeMap.end()) {
//             timestamp = (timestamp << 6) | it->second;
//         } else {
//             std::cerr << "Invalid Base64 character: " << input[i] << std::endl;
//             return 0; // Return 0 on error
//         }
//     }
//     return timestamp;
// }

char Base64Encoder::encodeIDAndDeltaTime(uint8_t id, uint8_t deltaTime) {
    if (id > 2 || deltaTime > 15) {
        LogFatalln("Invalid ID or deltaTime: ", id, " ", deltaTime);
        deltaTime = 15;
        return '?'; // Return '?' to indicate error
    }
    // Combine the ID and deltaTime into a single byte
    uint8_t combined = (id << 4) | deltaTime;
    // Encode this byte into a Base64 character
    return BASE64_ALPHABET[combined];
}

// void Base64Encoder::decodeToIDAndDeltaTime(char input, uint8_t& id, uint8_t& deltaTime) {
//     const auto& decodeMap = getDecodeMap(); // Use the static decode map
//     auto it = decodeMap.find(input); // Find the input character in the decode map
//     if (it != decodeMap.end()) { // If the character is found
//         uint8_t combined = it->second; // Retrieve the decoded value
//         id = (combined >> 4) & 0x03; // Extract the ID and deltaTime from the combined byte
//         deltaTime = combined & 0x0F;
//     } else { // If the character is not found
//         std::cerr << "Invalid Base64 character: " << input << std::endl;
//         id = 0;
//         deltaTime = 0;
//     }
// }

void Base64Encoder::encodeBytesToBase64(const uint8_t input[6], char output[8]) {
    uint32_t temp = 0;
    int outputIndex = 0;

    // Process each 3-byte block from the input
    for (int i = 0; i < 6; i += 3) {
        // Combine the 3 bytes into a single 24-bit number
        temp = (input[i] << 16) | (input[i + 1] << 8) | input[i + 2];

        // Encode each 6-bit segment into a Base64 character
        for (int j = 18; j >= 0; j -= 6) {
            output[outputIndex++] = BASE64_ALPHABET[(temp >> j) & 0x3F];
        }
    }
}

// void Base64Encoder::decodeBase64ToBytes(const char input[8], uint8_t output[6]) {
//     const auto& decodeMap = getDecodeMap(); // Use the static decode map

//     // Zero out the output array for safety
//     for (int i = 0; i < 6; ++i) {
//         output[i] = 0;
//     }

//     uint32_t buffer = 0;
//     int bitPosition = 0;

//     // Decode each Base64 character
//     for (int i = 0; i < 8; ++i) {
//         auto it = decodeMap.find(input[i]);
//         if (it == decodeMap.end()) {
//             std::cerr << "Invalid Base64 character: " << input[i] << std::endl;
//             return; // Exit on encountering an invalid character
//         }

//         // Add the 6-bit value to the buffer
//         buffer = (buffer << 6) | it->second;

//         bitPosition += 6;
//         if (bitPosition >= 8) {
//             bitPosition -= 8;
//             // Extract a byte from the buffer and assign it to the output
//             output[(i * 6) / 8] = (buffer >> bitPosition) & 0xFF;
//         }
//     }
// }