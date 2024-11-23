#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <FirebaseESP32.h>
#include "config.h" // Arquivo config.h

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

bool imuActive = true; // variável para controlar o estado do IMU
int imuReadingsCount = 1; // contador de leituras do IMU
const int readingsThreshold = 100; // limite de leituras antes de enviar ao Firebase

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 600);

FirebaseJson jsonData1;
FirebaseJson jsonData2;

FirebaseJson* activeJsonData = &jsonData1;
FirebaseJson* sendingJsonData = &jsonData2;

TaskHandle_t Task1;
TaskHandle_t Task2;

SemaphoreHandle_t xSemaphore;

// Variável de controle para marcação do tempo
volatile bool timeUpdated = false;
unsigned long lastTimestamp = 0;

void IRAM_ATTR onTimer() {
    timeUpdated = true; // A cada interrupção de timer, marca que o tempo foi atualizado
}

void setup() {
    // Inicializa Serial
    Serial.begin(115200);

    // Inicializa M5 Capsule
    M5.begin();

    // Conectando ao Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("Conectado ao Wi-Fi!");

    timeClient.begin();

    // Configuração do Firebase
    config.api_key = DATABASE_API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = DATABASE_USER_EMAIL;
    auth.user.password = DATABASE_USER_PASSWORD;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Cria o semáforo
    xSemaphore = xSemaphoreCreateMutex();

    // Cria as tarefas
    xTaskCreatePinnedToCore(
        collectIMUData,   // Task function
        "Task1",          // Task name
        10000,            // Stack size
        NULL,             // Task input parameter
        1,                // Priority of the task
        &Task1,           // Task handle
        0);               // Core 0

    xTaskCreatePinnedToCore(
        sendDataToFirebase,  // Task function
        "Task2",             // Task name
        10000,               // Stack size
        NULL,                // Task input parameter
        2,                   // Priority of the task
        &Task2,              // Task handle
        1);                  // Core 1

    // Configura o timer para disparar a cada 1 segundo
    timerConfig();

    Serial.println("Configuração concluída");
}

void timerConfig() {
    // Configura um temporizador para disparar a interrupção a cada 1 segundo
    timerBegin(0, 80, true);
    timerAttachInterrupt(0, &onTimer, true); 
    timerAlarmWrite(0, 1000000, true);
    timerAlarmEnable(0); 
}

void collectIMUData(void * parameter) {
    float ax, ay, az; // Variáveis para acelerômetro
    float gx, gy, gz; // Variáveis para acelerômetro

    while (true) {
        // Coleta e envia os dados do IMU, se ativo
        if (imuActive) {
            // Atualiza os dados do IMU
            if (M5.Imu.update()) {
                // Obtém os dados do acelerômetro
                M5.Imu.getAccelData(&ax, &ay, &az);
                M5.Imu.getGyroData(&gx, &gy, &gz);

                // Protege os recursos compartilhados com o semáforo
                xSemaphoreTake(xSemaphore, portMAX_DELAY);

                if (imuReadingsCount < readingsThreshold) {
                    FirebaseJson jsonEntry;
                    jsonEntry.set("aX", ax);
                    jsonEntry.set("aY", ay);
                    jsonEntry.set("aZ", az);
                    jsonEntry.set("gX", gx);
                    jsonEntry.set("gY", gy);
                    jsonEntry.set("gZ", gz);
                    activeJsonData->add(String(imuReadingsCount), jsonEntry);

                    imuReadingsCount++;
                    Serial.println(imuReadingsCount);
                }
                xSemaphoreGive(xSemaphore);
            }
        }
    vTaskDelay(5);
    }
}


void sendDataToFirebase(void * parameter) {
    while (true) {

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi desconectado! Tentando reconectar...");
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            while (WiFi.status() != WL_CONNECTED) {
                delay(200);
                Serial.print(".");
            }
            Serial.println("\nReconectado ao Wi-Fi!");
        }

        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        if (imuReadingsCount >= readingsThreshold) {
            FirebaseJson* temp = activeJsonData;
            activeJsonData = sendingJsonData;
            sendingJsonData = temp;

            imuReadingsCount = 1;

            xSemaphoreGive(xSemaphore);

            if (Firebase.pushJSON(firebaseData, "/IMUData", *sendingJsonData)) {
                Serial.println("Dados enviados ao Firebase!");
            } else {
                Serial.println("Erro ao enviar dados: " + firebaseData.errorReason());
            }
            sendingJsonData->clear();
        } else {
            xSemaphoreGive(xSemaphore);
        }

        vTaskDelay(1);
    }
}

void loop() {
        // Loop principal
    if (timeUpdated) {
        timeUpdated = false; // Reseta o flag

        // Atualiza o tempo a cada segundo
        timeClient.update();

        activeJsonData->add("timestamp", timeClient.getFormattedTime());
    }
    vTaskDelay(1);
}
