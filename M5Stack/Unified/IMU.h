#ifndef IMU_H
#define IMU_H

#include <M5Unified.h>
#include <FirebaseESP32.h>

class IMU {
public:
    static bool toggleState();
    static void collectReadings(float& ax, float& ay, float& az, float& gx, float& gy, float& gz);
    static void saveToJson(int count, const String& time, float ax, float ay, float az, float gx, float gy, float gz);
};

#endif // IMU_H
