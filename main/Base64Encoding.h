#ifndef BASE64ENCODING_H
#define BASE64ENCODING_H

// #include <cstdint>
// #include <string>
// #include <map>

#include <Arduino.h>

/**
 * @class Base64Encoder
 * @brief Provides functionalities for encoding and decoding data using Base64.
 *
 * The Base64Encoder class offers static methods for encoding sensor data,
 * timestamps, IDs with delta times, and more, using the Base64 encoding scheme.
 */
class Base64Encoder {
public:
    Base64Encoder() = default;

    /**
     * Encodes a 3-element array of int16_t sensor data into an 8-character Base64 string.
     * @param input Array of three int16_t elements representing sensor data.
     * @param output Array of eight characters for the encoded Base64 output.
     */
    static void encodeSensorData(const int16_t input[3], char output[8]);

    /**
     * Decodes an 8-character Base64 string back into a 3-element array of int16_t sensor data.
     * @param input Array of eight characters for the encoded Base64 input.
     * @param output Array of three int16_t elements representing the decoded sensor data.
     */
    // static void decodeSensorData(const char input[8], int16_t output[3]);

    /**
     * Encodes a timestamp into a 6-character Base64 string. Assumes timestamp is within a day.
     * @param timestamp The timestamp to encode, represented as a uint32_t.
     * @param output Array of six characters for the encoded Base64 output. Includes null-terminator.
     */
    static void encodeTimestamp(uint32_t timestamp, char output[6]);

    /**
     * Decodes a 6-character Base64 string back into a timestamp.
     * @param input Array of six characters for the encoded Base64 input.
     * @return The decoded timestamp as a uint32_t.
     */
    // static uint32_t decodeTimestamp(const char input[6]);

    /**
     * Encodes an ID and deltaTime into a single Base64 character.
     * @param id ID to encode, must be in the range [0, 2].
     * @param deltaTime Delta time to encode, must be in the range [0, 15].
     * @return The encoded Base64 character.
     */
    static char encodeIDAndDeltaTime(uint8_t id, uint8_t deltaTime);

    /**
     * Decodes a single Base64 character back into an ID and deltaTime.
     * @param input The single Base64 character to decode.
     * @param id Reference to the output ID.
     * @param deltaTime Reference to the output delta time.
     */
    // static void decodeToIDAndDeltaTime(char input, uint8_t& id, uint8_t& deltaTime);

private:
    /**
     * Encodes a 6-byte array into an 8-character Base64 string.
     * @param input Array of six bytes to encode.
     * @param output Array of eight characters for the encoded Base64 output.
     */
    static void encodeBytesToBase64(const uint8_t input[6], char output[8]);
    
    /**
     * Decodes an 8-character Base64 string back into a 6-byte array.
     * @param input Array of eight characters for the encoded Base64 input.
     * @param output Array of six bytes for the decoded output.
     */
    // static void decodeBase64ToBytes(const char input[8], uint8_t output[6]);

    /**
     * Retrieves the singleton instance of the Base64 decode map, initializing it if necessary.
     * This ensures the decode map is populated exactly once, the first time this method is called.
     * 
     * @return A constant reference to the Base64 decode map.
     */
    // static const std::map<char, int>& getDecodeMap();

    static const char* BASE64_ALPHABET; ///< Base64 encoding alphabet.
    // static std::map<char, int> BASE64_DECODE_MAP; ///< Static map for Base64 decoding.
};

#endif // BASE64ENCODING_H
