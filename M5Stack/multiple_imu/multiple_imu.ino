#include "M5StickCPlus2.h"
#include "M5_IMU_PRO.h"

#define BIM270_SENSOR_ADDR 0x68
#define BMP280_SENSOR_ADDR 0x76

BMI270::BMI270 bmi270;
Adafruit_BMP280 bmp(&Wire);

void setup() {
    //inicia o serial
    Serial.begin(115200);

    //inicia o dispositivo
    auto cfg = M5.config();
    StickCP2.begin(cfg);

    //inicia o sensor externo
    unsigned status = bmp.begin(BMP280_SENSOR_ADDR);
    if (!status) {
        Serial.println(
            F("Could not find a valid BMP280 sensor, check wiring or "
              "try a different address!"));
        Serial.print("SensorID was: 0x");
        Serial.println(bmp.sensorID(), 16);
        while (1) delay(10);
    }
    
    bmi270.init(I2C_NUM_0, BIM270_SENSOR_ADDR);
    
    //configura o display
    StickCP2.Display.setRotation(1);
    StickCP2.Display.setTextDatum(middle_center);
}

void loop(void) {
    // IMU interno
    auto imu_update = StickCP2.Imu.update();
    if (imu_update) {
        StickCP2.Display.setCursor(0, 20);
        StickCP2.Display.clear();  // Delay 100ms 延迟100ms

        auto data = StickCP2.Imu.getImuData();

        // The data obtained by getImuData can be used as follows.
        // data.accel.x;      // accel x-axis value.
        // data.accel.y;      // accel y-axis value.
        // data.accel.z;      // accel z-axis value.
        // data.accel.value;  // accel 3values array [0]=x / [1]=y / [2]=z.

        // data.gyro.x;      // gyro x-axis value.
        // data.gyro.y;      // gyro y-axis value.
        // data.gyro.z;      // gyro z-axis value.
        // data.gyro.value;  // gyro 3values array [0]=x / [1]=y / [2]=z.

        // data.value;  // all sensor 9values array [0~2]=accel / [3~5]=gyro /
        //              // [6~8]=mag

        Serial.printf("ax:%f  ay:%f  az:%f\r\n", data.accel.x, data.accel.y,
                      data.accel.z);
        Serial.printf("gx:%f  gy:%f  gz:%f\r\n", data.gyro.x, data.gyro.y,
                      data.gyro.z);

        StickCP2.Display.printf("IMU interno:\r\n");
        StickCP2.Display.printf("acc: %0.2f %0.2f %0.2f\r\n", data.accel.x,
                                data.accel.y, data.accel.z);
        StickCP2.Display.printf("gyro: %0.2f %0.2f %0.2f\r\n", data.gyro.x,
                                data.gyro.y, data.gyro.z);
    }

    //IMU externo
    float x, y, z;
    int16_t mx, my, mz = 0;

    StickCP2.Display.printf("\nIMU externo:\r\n");

    if (bmi270.accelerationAvailable()) {
        bmi270.readAcceleration(x, y, z);

        Serial.printf("ax_e:%f  ay_e:%f  az_e:%f\r\n", x, y, z);
        StickCP2.Display.printf("acc: %0.2f %0.2f %0.2f\r\n", x, y, z);
    }

    if (bmi270.gyroscopeAvailable()) {
        bmi270.readGyroscope(x, y, z);

        Serial.printf("gx_e:%f  gy_e:%f  gz_e:%f\r\n", x, y, z);
        StickCP2.Display.printf("gyro: %0.2f %0.2f %0.2f\r\n", x, y, z);
    }

    if (bmi270.magneticFieldAvailable()) {
        bmi270.readMagneticField(mx, my, mz);

        Serial.printf("mx_e:%f  my_e:%f  mz_e:%f\r\n", mx, my, mz);
        StickCP2.Display.printf("mag: %0.2f %0.2f %0.2f\r\n", mx, my, mz);
    }

    Serial.printf("temp:%f *C  press:%f Pa  alt:%f m\r\n", bmp.readTemperature() - 5, bmp.readPressure(),
                  bmp.readAltitude(1013.25));
    StickCP2.Display.printf("\ntemp: %0.2f*C; \npres: %0.2fAtm; \nalt: %0.2fm\r\n", bmp.readTemperature() - 5, bmp.readPressure()/101325,
                  bmp.readAltitude(1013.25));

    //bateria
    float vol = StickCP2.Power.getBatteryVoltage();
    Serial.printf("BAT: %.2f", vol/4200);
    StickCP2.Display.setCursor(170, 10);
    StickCP2.Display.printf("Bat: %0.2f", vol/4200);

    delay(500);
}