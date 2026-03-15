#include "Firebase.h"

// Double buffers for storing IMU readings
FirebaseJson jsonData1;
FirebaseJson jsonData2;

// Pointers to the active buffer and sending buffer
FirebaseJson* activeJsonData = &jsonData1;
FirebaseJson* sendingJsonData = &jsonData2;

void Firebase::initialize() {
    // configure Firebase
    config.api_key = DATABASE_API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = DATABASE_USER_EMAIL;
    auth.user.password = DATABASE_USER_PASSWORD;
    Firebase.begin(&config, &auth);

    // wait until Firebase is ready
    while(!Firebase.ready()){
        Serial.println("Error: Firebase is not ready");
        Firebase.begin(&config, &auth);
        delay(2000);
    }

    // Firebase will try to reconnect to WiFi if it loses connection
    Firebase.reconnectWiFi(true);

    Serial.println("Connected to Firebase.");
}

void Firebase::sendData() {
    if (Firebase.pushJSON("/imu_readings", activeJsonData)) {
        Serial.println("Data sent successfully.");
        activeJsonData.clear();
    } else {
        Serial.println("Failed to send data.");
    }
}

void Firebase::setEntry(const String& key, FirebaseJson& entry) {
    activeJsonData->set(key, entry);
}

int Firebase::getReadingsThreshold() {
    return readingsThreshold;
}
