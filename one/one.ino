#include "IMU.h"
#include "PINConfig.h"
#include "SDCard.h"
#include <SPI.h>

/**
 * @brief Represents an IMU (Inertial Measurement Unit) sensor.
 */
IMU IMU1(0, CS_1_IMU, "IMU-A");

SDCard sdCard(5);

SPIClass hspi(HSPI);
SPIClass vspi(VSPI);

char* dataString1 = (char*) malloc(350);

File* file_imu_1;

// function to free the memonry
void freeMemory() {
  free(dataString1);
}

void setup(){
  Serial.begin(115200);

  hspi.begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS);
  vspi.begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_CS);

  IMU1.init(hspi);

  // SDcard
  sdCard.init(vspi);
  sdCard.createFile("/teste_imu_1.csv");

  //free the memory when exitting
  atexit(freeMemory);
}

int iCounter = 0;

void loop() {

  //open files
  file_imu_1 = sdCard.openFile("/teste_imu_1.csv", 1);

  while(iCounter < 1000){
    dataString1 = IMU1.readData(dataString1);
    //Serial.println(dataString1);
    file_imu_1 -> print(dataString1);

    iCounter++;
    delay(10);
  }

  //close files
  file_imu_1 -> close();
  iCounter = 0;
  Serial.println("Fim de ciclo");
}