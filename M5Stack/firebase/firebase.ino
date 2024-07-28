#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "config.h" // change this file to your own config.h

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

bool imuActive = true; // variable to control the state of the IMU
int imuReadingsCount = 1; // IMU readings counter
const int readingsThreshold = 180; // number of readings before sending to Firebase
FirebaseJson jsonData; // json to store IMU readings

void setup() {
    // initialize serial
    Serial.begin(115200);

    // initialize device
    auto cfg = M5.config();
    StickCP2.begin(cfg);

    // configure the display
    StickCP2.Display.setRotation(1);
    StickCP2.Display.setTextDatum(middle_center);

    // connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to Wi-Fi!");

    // configure Firebase
    config.api_key = DATABASE_API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = DATABASE_USER_EMAIL;
    auth.user.password = DATABASE_USER_PASSWORD;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    Serial.println("Configuration completed");

    // clear the /IMUData node (for test only)
    /*
    if (Firebase.deleteNode(firebaseData, "/IMUData")) {
        Serial.println("Node /IMUData cleared successfully!");
    } else {
        Serial.println("Error clearing node /IMUData: " + firebaseData.errorReason());
    }
    */
}

void loop() {
    // check if button A was pressed
    if (StickCP2.BtnA.wasPressed()) {
        imuActive = !imuActive; // toggle IMU state
        if (imuActive) {
            Serial.println("IMU activated");
        } else {
            Serial.println("IMU deactivated");

            StickCP2.Display.clear();
            StickCP2.Display.setCursor(10, 20);
            StickCP2.Display.printf("IMU deactivated");
        }

        while(StickCP2.BtnA.isPressed()) {
            delay(10);
        }
    }

    // update button state
    StickCP2.update();

    // collect and send IMU data if active
    if (imuActive) {
        auto imu_update = StickCP2.Imu.update();

        if (imu_update) {
            auto data = StickCP2.Imu.getImuData();

            // store readings in JSON
            FirebaseJson jsonEntry;
            jsonEntry.set("aX", data.accel.x);
            jsonEntry.set("aY", data.accel.y);
            jsonEntry.set("aZ", data.accel.z);
            jsonEntry.set("gX", data.gyro.x);
            jsonEntry.set("gY", data.gyro.y);
            jsonEntry.set("gZ", data.gyro.z);
            jsonData.add(String(imuReadingsCount), jsonEntry);

            imuReadingsCount++;

            // check if the number of readings to send to Firebase has been reached
            if (imuReadingsCount > readingsThreshold) {
                if (Firebase.pushJSON(firebaseData, "/IMUData", jsonData)) {
                    StickCP2.Display.clear();
                    StickCP2.Display.setCursor(10, 50);
                    Serial.println("Data sent successfully!");
                    StickCP2.Display.printf("Data sent successfully!");
                } else {
                    StickCP2.Display.clear();
                    StickCP2.Display.setCursor(10, 50);
                    Serial.println("Error sending data: " + firebaseData.errorReason());
                    StickCP2.Display.printf("Error sending data");
                }
                // reset counter and clear JSON
                imuReadingsCount = 1;
                jsonData.clear();

                // update battery status
                float voltage = StickCP2.Power.getBatteryVoltage();
                float batteryPercent = (voltage - 3.4) / (4.2 - 3.4) * 100;
                if (batteryPercent > 100) batteryPercent = 100;
                if (batteryPercent < 0) batteryPercent = 0;

                StickCP2.Display.setCursor(160, 10);
                StickCP2.Display.printf("Bat: %.2f%%", batteryPercent);

                StickCP2.Display.setCursor(10, 20);
                StickCP2.Display.printf("IMU activated");
            }
        }
    }
}
