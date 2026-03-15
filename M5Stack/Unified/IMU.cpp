#include "IMU.h"
#include "Firebase.h"

static bool imuState = false;

bool IMU::toggleState() {
    imuState = !imuState;
    Serial.println(imuState ? "IMU Activated" : "IMU Deactivated");
    return imuState;
}

void IMU::collectReadings(float& ax, float& ay, float& az, float& gx, float& gy, float& gz) {
    if (M5.Imu.update()) {
        M5.Imu.getAccelData(&ax, &ay, &az);
        M5.Imu.getGyroData(&gx, &gy, &gz);
    }
}

void IMU::saveToJson(int count, const String& current_time, float ax, float ay, float az, float gx, float gy, float gz) {
    FirebaseJson jsonEntry;
    jsonEntry.set("aX", ax);
    jsonEntry.set("aY", ay);
    jsonEntry.set("aZ", az);
    jsonEntry.set("gX", gx);
    jsonEntry.set("gY", gy);
    jsonEntry.set("gZ", gz);

    Firebase::addEntry(current_time + "/" + String(count), jsonEntry);
}
