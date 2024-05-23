#include "IMU.h"
#include "PINConfig.h"
//#include "SDCard.h"
#include <SPI.h>

/**
 * @brief Represents an IMU (Inertial Measurement Unit) sensor.
 */
IMU IMU1(0, CS_1_IMU, "IMU-A");
IMU IMU2(1, CS_2_IMU, "IMU-B");
IMU IMU3(2, CS_3_IMU, "IMU-C");

//SDCard sdCard;

SPIClass hspi(HSPI);
SPIClass vspi(VSPI);

char* dataString1 = (char*) malloc(350);
char* dataString2 = (char*) malloc(350);
char* dataString3 = (char*) malloc(350);

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
  // sdCard.init(vspi);
  // sdCard.createFile("/teste_1imu.csv");
  // sdCard.createFile("/teste_2imu.csv");
  // sdCard.createFile("/teste_3imu.csv");

  atexit(freeMemory);
}

void loop() {

  dataString1 = IMU1.readData(dataString1);
  Serial.println(dataString1);
  dataString2 = IMU2.readData(dataString1);
  Serial.println(dataString2);
  dataString3 = IMU3.readData(dataString1);
  Serial.println(dataString3);

  // Liberar a memória alocada
  free(dataString1);
  free(dataString2);
  free(dataString3);

  delay(100);
}