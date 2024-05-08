#include "SDCard.h"

// Criação de um objeto da classe SDCard
SDCard sdCard;

void setup() {
  Serial.begin(9600);
  
  // Inicializa o cartão SD
  sdCard.begin();
}

void loop() {

}