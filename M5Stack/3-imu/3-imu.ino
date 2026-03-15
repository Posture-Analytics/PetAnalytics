#include <M5Unified.h>
#include "M5_IMU_PRO.h"

// Define os endereços das IMUs externas
#define IMU1_ADDR 0x68
#define IMU2_ADDR 0x69

BMI270::BMI270 ext_imu1;
BMI270::BMI270 ext_imu2;

void setup() {
    Serial.begin(115200);

    // Inicializa o sistema do M5Capsule (Isso já prepara a IMU interna)
    auto cfg = M5.config();
    M5.begin(cfg);
    
    // Inicia o barramento externo da Porta A
    M5.Ex_I2C.begin();

    // Inicia os dois sensores de movimento externos
    ext_imu1.init(I2C_NUM_0, IMU1_ADDR);
    ext_imu2.init(I2C_NUM_0, IMU2_ADDR);
    
    Serial.println("Array de 3 IMUs iniciado!");
    delay(1000);
}

void loop(void) {
    unsigned long ts = millis();

    // Convert milliseconds timestamp to hh:mm:ss string
    unsigned long total_seconds = ts / 1000UL;
    unsigned long hours = (total_seconds / 3600UL) % 24UL;
    unsigned long minutes = (total_seconds / 60UL) % 60UL;
    unsigned long seconds = total_seconds % 60UL;
    char time_str[9]; // "HH:MM:SS" + null
    snprintf(time_str, sizeof(time_str), "%02lu:%02lu:%02lu", hours, minutes, seconds);

    // 1. Atualiza a IMU interna do Capsule
    auto imu_update = M5.Imu.update();

    // Internal IMU floats (two decimal places)
    float aix = 0.0f, ayx = 0.0f, azx = 0.0f;
    float gix = 0.0f, giy = 0.0f, giz = 0.0f;

    if (imu_update) {
        auto data = M5.Imu.getImuData();
        aix = data.accel.x;
        ayx = data.accel.y;
        azx = data.accel.z;
        gix = data.gyro.x;
        giy = data.gyro.y;
        giz = data.gyro.z;
    }

    // External IMU 1 floats (two decimal places)
    float ax1 = 0.0f, ay1 = 0.0f, az1 = 0.0f;
    float gx1 = 0.0f, gy1 = 0.0f, gz1 = 0.0f;

    if (ext_imu1.accelerationAvailable() && ext_imu1.gyroscopeAvailable()) {
        ext_imu1.readAcceleration(ax1, ay1, az1);
        ext_imu1.readGyroscope(gx1, gy1, gz1);
    }

    // External IMU 2 floats (two decimal places)
    float ax2 = 0.0f, ay2 = 0.0f, az2 = 0.0f;
    float gx2 = 0.0f, gy2 = 0.0f, gz2 = 0.0f;

    if (ext_imu2.accelerationAvailable() && ext_imu2.gyroscopeAvailable()) {
        ext_imu2.readAcceleration(ax2, ay2, az2);
        ext_imu2.readGyroscope(gx2, gy2, gz2);
    }

    // Print in requested compact format:
    // acc: timestamp, int(internal x), int(internal y), int(internal z), int(ext1 x), ...
    Serial.printf("acc: %s, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\r\n",
                  time_str, aix, ayx, azx, ax1, ay1, az1, ax2, ay2, az2);

    // gyr: timestamp, int(internal x), int(internal y), int(internal z), int(ext1 x), ...
    Serial.printf("gyr: %s, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\r\n",
                  time_str, gix, giy, giz, gx1, gy1, gz1, gx2, gy2, gz2);

    // Delay de 200ms (5 leituras por segundo)
    delay(200);
}