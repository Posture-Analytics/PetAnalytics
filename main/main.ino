#include "IMU.h"
#include "PINConfig.h"
#include "SDCard.h"
#include <SPI.h>

/**
 * @brief Represents an IMU (Inertial Measurement Unit) sensor.
 */
IMU IMU1(0, CS_1_IMU, "IMU-A");
IMU IMU2(1, CS_2_IMU, "IMU-B");
IMU IMU3(2, CS_3_IMU, "IMU-C");

SDCard sdCard(5);

SPIClass hspi(HSPI);
SPIClass vspi(VSPI);

char* dataString1 = (char*) malloc(350);
char* dataString2 = (char*) malloc(350);
char* dataString3 = (char*) malloc(350);

File* file_imu_1;
File* file_imu_2;
File* file_imu_3;

// function to free the memonry
void freeMemory() {
  free(dataString1);
  free(dataString2);
  free(dataString3);
}

void setup(){
  Serial.begin(115200);

  hspi.begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS);
  vspi.begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_CS);

  IMU1.init(hspi);
  IMU2.init(hspi);
  IMU3.init(hspi);

  // SDcard
  sdCard.init(vspi);
  sdCard.createFile("/teste_imu_1.csv");
  sdCard.createFile("/teste_imu_2.csv");
  sdCard.createFile("/teste_imu_3.csv");

  //free the memory when exitting
  atexit(freeMemory);
}

int iCounter = 0;

void loop() {

  //open files
  file_imu_1 = sdCard.openFile("/teste_imu_1.csv", 1);
  file_imu_2 = sdCard.openFile("/teste_imu_2.csv", 1);
  file_imu_3 = sdCard.openFile("/teste_imu_3.csv", 1);

  while(iCounter < 1000){
    dataString1 = IMU1.readData(dataString1);
    //Serial.println(dataString1);
    file_imu_1 -> print(dataString1);
    dataString2 = IMU2.readData(dataString1);
    //Serial.println(dataString2);
    file_imu_2 -> print(dataString2);
    dataString3 = IMU3.readData(dataString1);
    //Serial.println(dataString3);
    file_imu_3 -> print(dataString3);


    //Serial.print(iCounter);
    //Serial.print(" ");
    iCounter++;
    delay(8);
  }

  //close files
  file_imu_1 -> close();
  file_imu_2 -> close();
  file_imu_3 -> close();
  iCounter = 0;
  Serial.println("Fim de ciclo");
}