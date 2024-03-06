#include "IMU.h"
#include <SPI.h>

/**
 * @brief Represents an IMU (Inertial Measurement Unit) sensor.
 */
IMU IMU1(0, 12, "IMU-A");
IMU IMU2(1, 13, "IMU-B");
IMU IMU3(2, 27, "IMU-B");

void setup(){
  Serial.begin(115200);
  SPI.begin();

  IMU1.init();
  IMU2.init();
  IMU3.init();
}

void loop(){
  IMU1.readData();
  IMU2.readData();
  IMU3.readData();

  delay(5);
}