#include <SD.h>
#include <SPI.h>
#include "SDCard.h"
#include "Buffer.h"

using std::String;

SDCard::SDCard(){}

// MÉTODOS SIMPLES
// muita coisa tinha na internet, apenas adaptei e fiz a parte de OO
bool SDCard::begin() 
{
    if (SD.begin()) 
    {
        Serial.println("A inicialização do cartão SD foi bem sucedida.");
        cardType = SD.cardType();
        cardSize = SD.cardSize() / (1024 * 1024);
        return true;
    } 
    else 
    {
        Serial.println("A inicialização do cartão SD falhou.");
        return false;
    }
}

void SDCard::printCardType() 
{
    switch(cardType) 
    {
        case CARD_MMC:
            Serial.println("Tipo do Cartão: MMC");
            break;
        case CARD_SD:
            Serial.println("Tipo do Cartão: SDSC");
            break;
        case CARD_SDHC:
            Serial.println("Tipo do Cartão: SDHC");
            break;
        default:
            Serial.println("Tipo do Cartão: Desconhecido");
            break;
    }
}

void SDCard::printCardSize()
{
    Serial.print("Tamanho do Cartão: ");
    Serial.print(cardSize);
    Serial.println(" MB");
}

void SDCard::printDir(const char *dirname, uint8_t levels) 
{
  Serial.printf("Listando Dir: %s\n", dirname);

  File root = SD.open(dirname);
  if (!root) 
  {
    Serial.println("Falha ao abrir o diretório.");
    return;
  }
  if (!root.isDirectory()) 
  {
    Serial.println("Não é diretório.");
    return;
  }

  File file = root.openNextFile();
  while (file) 
  {
    if (file.isDirectory()) 
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) 
      {
        printDir(file.path(), levels - 1);
      }
    } 
    else 
    {
      Serial.print("  ARQ: ");
      Serial.print(file.name());
      Serial.print("  TAM: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

bool SDCard::createDir(const char *path) 
{
  Serial.printf("Criando Dir: %s\n", path);
  if (SD.mkdir(path)) 
  {
    Serial.println("Diretório criado.");
    return true;
  } 
  else 
  {
    Serial.println("Falha ao criar o diretório.");
    return false;
  }
}

bool SDCard::readFile(const char *path) 
{
  Serial.printf("Lendo arquivo: %s\n", path);

  File file = SD.open(path);
  if (!file) 
  {
    Serial.println("Falha ao abrir o arquivo.");
    return false;
  }

  while (file.available()) 
  {
    Serial.write(file.read());
  }
  file.close();
  return true;
}

bool SDCard::createFile(const char *path) 
{
    Serial.printf("Writing file: %s\n", path);

    File file = SD.open(path, FILE_WRITE);
    if (!file) 
    {
        Serial.println("Falha ao criar o arquivo.");
        return false;
    }
    if (file.print(columns)) {
        Serial.println("Arquivo criado.");
    } 
    else 
    {
        Serial.println("Falha ao criar o arquivo.");
        return false;
    }
    file.close();
    return true;
}

bool SDCard::renameFile(const char *path1, const char *path2) 
{
  Serial.printf("Renomeando %s para %s\n:", path1, path2);
  if (SD.rename(path1, path2)) 
  {
    Serial.println("Arquivo renomeado.");
    return true;
  } 
  else 
  {
    Serial.println("Falha ao renomear.");
    return false;
  }
}

bool SDCard::deleteFile(const char *path) 
{
  Serial.printf("Deletando arquivo: %s\n", path);
  if (SD.remove(path)) 
  {
    Serial.println("Arquivo deletado.");
    return true;
  } 
  else 
  {
    Serial.println("Falha ao deletar.");
    return false;
  }
}

// ATÉ ESSE PONTO TÁ FUNCIONANDO, A PARTIR DAQUI EU (AINDA) NÃO SEI TESTAR.

bool SDCard::writeFile(const char *path, const imuData* sample, const char *IMU_ID) 
{
    Serial.printf("Editando arquivo: %s\n", path);

    // Resolver o problema da string. 
    String dataString = IMU_ID;
    dataString = String(sample->timestampMillis);
    for (int j = 0; j < 3; j++) {
        dataString += ",";
        dataString += String(sample->accelData[j]);
        dataString += ",";
        dataString += String(sample->gyroData[j]);
        dataString += ",";
        dataString += String(sample->magData[j]);
    }
    dataString += "\n";

    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Falha ao abrir o arquivo.");
        return true;
    }
    if (file.print(dataString)) {
        Serial.println("Arquivo editado com sucesso.");
    } else {
        Serial.println("Falha ao editar arquivo.");
        return false;
    }
    file.close();
    return true;
}