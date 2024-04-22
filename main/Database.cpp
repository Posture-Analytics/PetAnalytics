
#include <WiFi.h>
#include <time.h>
#include <FirebaseESP32.h>
// Provide the token generation process info for database autentication
#include <addons/TokenHelper.h>

#include "Database.h"
#include "Errors.h"
#include "Network.h"
#include "Buffer.h"
#include "Debug.h"
#include "Base64Encoding.h"

Database::Database() : last_was_valid(true) {}

void Database::updateCurrentTime() {
    // Set the variable `currentMicros` with the current time in microseconds (us)
    currentMicros = micros();
}

void Database::setup(time_t timestampUnix) {
    // Assign the api key (required) of the database
    config.api_key = DATABASE_API_KEY;

    // Assign the RTDB URL (required)
    config.database_url = DATABASE_URL;

    // Assign the user sign in credentials
    auth.user.email = DATABASE_USER_EMAIL;
    auth.user.password = DATABASE_USER_PASSWORD;

    // Assign the callback function for the long running token generation task
    config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

    // Authenticate and initialize the communication with the Firebase database
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void Database::bootLog() {
    // If the database is ready to receive the data, we record the timestamp of the device's boot
    if (Firebase.ready()) {
        // Record the current timestamp string to the database
        if (Firebase.pushInt(fbdo, "/bootLog/", getCurrentMillisTimestamp())) {
            LogInfoln("Inicialização registrada com sucesso!");
        // If an error occurs during this process, we show it as a fatal
        // database error and restart the device
        } else {
            LogFatalln("Ocorreu um erro ao registrar a inicialização:\n", fbdo.errorReason());
            errorHandler.showError(ErrorType::NoDatabaseConnection, true);
        }
    // If the Firebase Database is not ready, we test the connection to
    // the network and to the database and display it on the LED indicator
    } else {
        if (WiFi.status() != WL_CONNECTED) {
            LogFatalln("Não foi possível conectar à rede para registrar a inicialização");
            errorHandler.showError(ErrorType::NoInternet, true);
        } else {
            LogFatalln("Não foi possível conectar ao banco de dados para registrar a inicialização");
            errorHandler.showError(ErrorType::NoDatabaseConnection, true);
        }
    }
}

void Database::appendDataToJSON(const imuData* data) {

    long long int initTime = millis();

    // Compute the timestamp to the number of milliseconds since midnight
    long long int timestampMillisFromMidnight = data->timestampMillis % (86400000); // 24h * 60min * 60s * 1000ms

    // Check if there is a payload being generated
    if (payload.length() == 0) {
        // Reset the encodedTimestampPath
        for (int i = 0; i < 6; i++) {
            encodedTimestampPath[i] = '0';
        }

        // Encode the timestamp of the data sample as the path of the database node
        encoder.encodeTimestamp(timestampMillisFromMidnight, encodedTimestampPath);

        // Set the deltaTime to 0 if it is the first sample
        deltaTime = 0;
    } else {
        // Calculate the time difference between the current and the last sample
        deltaTime = timestampMillisFromMidnight - lastTimestamp;

        // If the time difference is greater than 15 milliseconds, we clip it to 15
        if (deltaTime > 15000) {
            deltaTime = 15;
            LogErrorln("DeltaTime bigger than 15 ms!");
        }
    }

    // Update the last timestamp
    lastTimestamp = timestampMillisFromMidnight;

    // Encode the ID and deltaTime into a single byte
    encodedIDAndDeltaTime = encoder.encodeIDAndDeltaTime(data->IMUid, deltaTime);

    // Encode the sensor data into a base64 string
    encoder.encodeSensorData(data->accelData, encodedAccelData);
    encoder.encodeSensorData(data->gyroData, encodedGyroData);
    encoder.encodeSensorData(data->magData, encodedMagData);

    // Concatenate the base64 encoded values

    payload += encodedIDAndDeltaTime;
    payload += encodedAccelData;
    payload += encodedGyroData;
    payload += encodedMagData;

    payload += "A";


    

    // Concatenate the original data into a single string
    String originalData = String(data->IMUid) + "," + String(timestampMillisFromMidnight) + ","
            + String(data->accelData[0]) + "," + String(data->accelData[1]) + "," + String(data->accelData[2]) + ","
            + String(data->gyroData[0]) + "," + String(data->gyroData[1]) + "," + String(data->gyroData[2]) + ","
            + String(data->magData[0]) + "," + String(data->magData[1]) + "," + String(data->magData[2]);
    String encodedData = String(encodedIDAndDeltaTime);
    for (int i = 0; i < 8; i++) {
        encodedData += encodedAccelData[i];
    }
    for (int i = 0; i < 8; i++) {
        encodedData += encodedGyroData[i];
    }
    for (int i = 0; i < 8; i++) {
        encodedData += encodedMagData[i];
    }
    
    LogDebugln("Original data: ", originalData);
    LogDebugln("Encoded data: ", encodedData);




    // Reset the encoded values
    encodedIDAndDeltaTime = 0;

    for (int i = 0; i < 3; i++) {
        encodedAccelData[i] = 0;
        encodedGyroData[i] = 0;
        encodedMagData[i] = 0;
    }

    // If the payload is complete, we add it to the JSON buffer
    if (payload.length() / 23 >= PAYLOAD_MAX_SAMPLES) {
        // Add the payload to the JSON buffer
        jsonBuffer.add(encodedTimestampPath, payload);
        jsonSize++;

        // Clear the payload
        payload = "";

        // Reset the encodedTimestampPath
        for (int i = 0; i < 6; i++) {
            encodedTimestampPath[i] = '0';
        }

        // Reset the last timestamp
        lastTimestamp = 0;

        LogDebug("Payload sent to JSON buffer. Size: ");
        LogDebugln(jsonSize);
    } else {
        LogDebug("Payload size: ");
        LogDebugln(payload.length() / 23);
    }

    LogFatal("Time to append data to JSON (ms): ");
    LogFatalln(millis() - initTime);
}

bool Database::pushData() {
    // #ifdef DEBUG

    //     // In debug mode, we only print the values instead of sending them to the database
    //     json.toString(Serial, true, 255);
    //     return true;

    // #else

        // If the Firebase Database is ready to receive the data, we send it asynchronously
        // to be faster and to be able the send a larger amount of the data points per second
        if (Firebase.ready()) {
            // Send the data to database
            if (Firebase.updateNodeSilentAsync(fbdo, fullDataPath, jsonBuffer)) {
                // Update the LED indicator, showing that everything works fine
                errorHandler.showError(ErrorType::None);

                // Clear the JSON buffer and reset the counter
                jsonBuffer.clear();
                jsonSize = 0;

                // If the data was sent successfully, we return true
                return true;
            // If some error occurs during this process, we show as a fatal database error and
            // restart the device
            } else {
                LogErrorln("Database error on ", fullDataPath, ": ", fbdo.errorReason());
                LogErrorln("Payload buffer length: ", jsonBuffer.serializedBufferLength());
                errorHandler.showError(ErrorType::NoDatabaseConnection);

                return false;
            }
        } else {
            return false;
        }

    // #endif
}

void Database::sendData(IMUDataBuffer* dataBuffer) {
    // Save the time when the device start to send the data from the sensors,
    // to keep control of the intervals between data uploads
    updateCurrentTime();

    // If the time elapsed since the last data sending is greater than the interval between
    // the data uploads, if the JSON buffer is full or if the next package is from another day, we
    // send the data to the database
    if ((jsonSize > 0 && currentMicros - dataPrevSendingMicros > dataSendIntervalMicros)
            || jsonSize >= JSON_BATCH_SIZE
            || jsonSize > 0 && dataBuffer->hasDateChanged()) {
        pushData();

        // Update the time variable that controls the send interval
        dataPrevSendingMicros = currentMicros;
    }

    // Otherwise, we keep filling the JSON buffer with the data from the main buffer
    else {

        // Get one sample from the sensor data buffer
        const imuData* sample = dataBuffer->getSample();

        // Check if the current sample is valid
        bool current_is_valid = !dataBuffer->isSampleNull(sample);
        // bool current_is_valid = true; // DEBUG

        /**
         * If the current or the last sample is valid, we send the data to the database.
         * If the sample being processed is non-zero, it is always sent to the database.
         * Else, it is only sent if the last sample was valid, so that we don't send
         * too many null values to the database in succession.
         */
        if (current_is_valid || last_was_valid) {
            // Concatenate the sample in a JSON buffer
            appendDataToJSON(sample);
        }

        last_was_valid = current_is_valid;
    }

    // If necessary, we update the path of the database node that will receive the data
    if (dataBuffer->hasDateChanged()) {
        // Get the date string of the current sample
        dataBuffer->computeCurrentSampleDate(sampleDate);
        // Add a slash to the end of the date string to make the path valid
        sampleDate[10] = '/';
        sampleDate[11] = '\0';
        // Get the seconds that represent the following day, used to check if the date changed
        dataBuffer->computeNextDaySeconds();
        // Update the path of the database node that will receive the data
        fullDataPath = DATABASE_BASE_PATH + sampleDate;
    }

    dataBuffer->printBufferState();

    // Print the size of the JSON buffer
    LogVerboseln("JSON buffer: ", jsonSize, "/", JSON_BATCH_SIZE);

    dataBuffer->printBufferIndexes();

    // If we just send an amount of data to the database,
    // give an interval to Core 0 to work on maintence activities, avoiding crash problems
    vTaskDelay(2);
    yield();
}