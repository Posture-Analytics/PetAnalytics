#include "IMU.h"
#include <SPI.h>

void setup(){
  Serial.begin(115200);
  SPI.begin();
}

void loop(){
  IMU_20948 IMU1(0, 12, 25, "Primeiro IMU"), IMU2(1, 13, 26, "Primeiro IMU"), IMU3(2, 27, 34, "Terceiro IMU");
  while(1){
    IMU1.readData();
    IMU2.readData();
    IMU3.readData();
  }
}