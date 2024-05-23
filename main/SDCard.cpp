#include <SD.h>
#include <iostream>
#include "SDCard.h"
#include "PINConfig.h"
// #include "../../main/Buffer.h"

SDCard::SDCard(int CSpin): SDpin(CSpin){}

bool SDCard::init(SPIClass &spi) 
{
      if (SD.begin(SDpin, spi)){
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

  Serial.println("[END]");

  return true;
}

bool SDCard::createFile(const char *path) 
{
    Serial.printf("Criando arquivo: %s\n", path);

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

bool SDCard::writeFile(File* file, float dado)
{
    if (!file || !*file) {
        Serial.println("writeFile: Falha ao acessar o arquivo.");
        return false;
    }
    if (file->print(dado)) {
        Serial.println("Arquivo editado com sucesso.");
    } else {
        Serial.println("writeFile: Falha ao editar arquivo.");
        return false;
    }

    return true;
}

File* SDCard::openFile(const char *path, bool mode)
{
  Serial.printf("Abrindo o Arquivo: %s\n", path);

  File* file = nullptr;

  switch (mode) {
        case 1:
            file = new File(SD.open(path, FILE_APPEND));
            break;
        case 0:
            file = new File(SD.open(path, FILE_WRITE));
            break;
        default:
            Serial.println("Modo inválido. Use 0 para FILE_WRITE ou 1 para FILE_APPEND.");
            return nullptr;
    }

    if (!file || !*file) {
        Serial.println("Falha ao abrir o arquivo.");
        delete file;
        return nullptr;
    }

    return file;
}

bool SDCard::closeFile(File* file)
{
  Serial.printf("Fechando o Arquivo\n");

  if (file) {
      file->close();
      Serial.println("Arquivo fechado com sucesso.");
      delete file;
      return true;
  } else {
    Serial.println("closeFile: O arquivo não está aberto.");
    return true;
  }
}