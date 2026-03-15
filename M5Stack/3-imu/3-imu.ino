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
    
    Serial.println("Array de 3 IMUs iniciado! Lendo a cinematica completa...");
    delay(1000);
}

void loop(void) {
    // 1. Atualiza a IMU interna do Capsule
    auto imu_update = M5.Imu.update();

    // Variáveis para as IMUs externas
    float ax1, ay1, az1, gx1, gy1, gz1;
    float ax2, ay2, az2, gx2, gy2, gz2;

    Serial.println("\n--- Leituras Sincronizadas ---");

    // --- Leitura da IMU INTERNA ---
    if (imu_update) {
        auto data = M5.Imu.getImuData();
        Serial.printf("[Interna]  Acc: %0.2f  %0.2f  %0.2f \t| Gyr: %0.2f  %0.2f  %0.2f\r\n", 
                      data.accel.x, data.accel.y, data.accel.z, 
                      data.gyro.x, data.gyro.y, data.gyro.z);
    } else {
        Serial.println("[Interna]  Não Respondeu.");
    }

    // --- Leitura da IMU EXTERNA 1 (0x68) ---
    if (ext_imu1.accelerationAvailable() && ext_imu1.gyroscopeAvailable()) {
        ext_imu1.readAcceleration(ax1, ay1, az1);
        ext_imu1.readGyroscope(gx1, gy1, gz1);
        Serial.printf("[Ext 0x68] Acc: %0.2f  %0.2f  %0.2f \t| Gyr: %0.2f  %0.2f  %0.2f\r\n", 
                      ax1, ay1, az1, gx1, gy1, gz1);
    } else {
        Serial.println("[Ext 0x68] Não respondeu.");
    }

    // --- Leitura da IMU EXTERNA 2 (0x69) ---
    if (ext_imu2.accelerationAvailable() && ext_imu2.gyroscopeAvailable()) {
        ext_imu2.readAcceleration(ax2, ay2, az2);
        ext_imu2.readGyroscope(gx2, gy2, gz2);
        Serial.printf("[Ext 0x69] Acc: %0.2f  %0.2f  %0.2f \t| Gyr: %0.2f  %0.2f  %0.2f\r\n", 
                      ax2, ay2, az2, gx2, gy2, gz2);
    } else {
        Serial.println("[Ext 0x69] Não respondeu.");
    }

    // Delay de 200ms (5 leituras por segundo) para conseguir ler no olho
    delay(200);
}