#include "SDCard.h"
#include "IMU.h"
#include <SPI.h>

//IMU IMU1(0, 12, "IMU-A");
//IMU IMU2(1, 13, "IMU-B");
//IMU IMU3(2, 27, "IMU-C");

// Criação de um objeto da classe SDCard
SDCard sdCard;

void setup() {
  Serial.begin(115200);
  
  // Inicializa o cartão SD
  sdCard.begin();
  sdCard.createFile("/teste_file.csv");
  
  //SPI.begin();
  /*
  IMU1.init();
  IMU2.init();
  IMU3.init();
  */
}

void loop() {
  // sdCard.printDir("/", 0);
  sdCard.readFile("/teste_file.csv");
  delay(1000);
  /*
  IMU1.readData();
  IMU2.readData();
  IMU3.readData();

  delay(5);
  */
}