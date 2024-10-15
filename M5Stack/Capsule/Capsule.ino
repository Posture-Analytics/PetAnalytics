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
        collectIMUData, "Task1", 10000, NULL, 1, &Task1, 0);
    xTaskCreatePinnedToCore(
        sendDataToFirebase, "Task2", 10000, NULL, 2, &Task2, 1);

    Serial.println("Configuração concluída");
}

void collectIMUData(void * parameter) {
    float ax, ay, az; // Variáveis para acelerômetro
    unsigned long lastTimestamp = 0;

    while (true) {
        // Coleta e envia os dados do IMU, se ativo
        if (imuActive) {
            // Atualiza os dados do IMU
            if (M5.Imu.update()) {
                // Obtém os dados do acelerômetro
                M5.Imu.getAccelData(&ax, &ay, &az);

                // Protege os recursos compartilhados com o semáforo
                xSemaphoreTake(xSemaphore, portMAX_DELAY);

                if (imuReadingsCount < readingsThreshold) {
                    FirebaseJson jsonEntry;
                    jsonEntry.set("aX", ax);
                    jsonEntry.set("aY", ay);
                    jsonEntry.set("aZ", az);
                    activeJsonData->add(String(imuReadingsCount), jsonEntry);

                    imuReadingsCount++;
                    Serial.println(imuReadingsCount);
                }

                xSemaphoreGive(xSemaphore);
            }
        }

        // Atualiza o tempo a cada segundo
        if (millis() - lastTimestamp >= 1000) {
            timeClient.update();
            xSemaphoreTake(xSemaphore, portMAX_DELAY);

            activeJsonData->add(imuReadingsCount, timeClient.getFormattedTime());
            imuReadingsCount++;

            xSemaphoreGive(xSemaphore);
            lastTimestamp = millis();
        }
        vTaskDelay(6);
    }
}

void sendDataToFirebase(void * parameter) {
    while (true) {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);

        if (imuReadingsCount >= readingsThreshold) {
            FirebaseJson* temp = activeJsonData;
            activeJsonData = sendingJsonData;
            sendingJsonData = temp;

            imuReadingsCount = 1;

            xSemaphoreGive(xSemaphore);

            if (Firebase.pushJSON(firebaseData, "/IMUData", *sendingJsonData)) {
                Serial.println("Dados enviados ao Firebase!");
                sendingJsonData->clear();
            } else {
                Serial.println("Erro ao enviar dados: " + firebaseData.errorReason());
            }
        } else {
            xSemaphoreGive(xSemaphore);
        }

        vTaskDelay(1);
    }
}

void loop() {
    // Loop vazio, tarefas são gerenciadas pelo FreeRTOS
}
