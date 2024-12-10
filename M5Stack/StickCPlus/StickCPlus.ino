#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_sntp.h>
#include <FirebaseESP32.h>
#include "config.h" // change this file to your own config.h

FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // (GMT-3)
const int daylightOffset_sec = 0; 

bool imuActive = true; // Variable to control the state of the IMU
int imuReadingsCount = 0; // IMU readings counter
int ram_size = esp_get_free_heap_size();
const int readingsThreshold = max(ram_size / 2800, 79); // Number of readings before sending to Firebase

// Double buffers for storing IMU readings
FirebaseJson jsonData1;
FirebaseJson jsonData2;

// Pointers to the active buffer and sending buffer
FirebaseJson* activeJsonData = &jsonData1;
FirebaseJson* sendingJsonData = &jsonData2;

TaskHandle_t Task1;
TaskHandle_t Task2;

SemaphoreHandle_t xCountSemaphore; // Semaphore to protect imuReadingsCount

String current_time;

//Interrupt
hw_timer_t *timer = NULL;
volatile bool timerFlag = true;

void IRAM_ATTR onTimer() {
    timerFlag = true;
}

void waitForNextSecond() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Espera até que os milissegundos sejam próximos de 0
    while (tv.tv_usec > 5000) {
        gettimeofday(&tv, NULL);
    }
}

// Function to print the current time with milliseconds
void printFormattedTimeWithMillis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Converte para estrutura tm
    struct tm* timeinfo = localtime(&tv.tv_sec);

    // Obtém milissegundos
    int milliseconds = tv.tv_usec / 1000;

    // Formata e imprime a data e hora com milissegundos no serial
    Serial.printf("Hora atual com milissegundos: %02d:%02d:%02d.%03d\n",
                  timeinfo->tm_hour,
                  timeinfo->tm_min,
                  timeinfo->tm_sec,
                  milliseconds);
}

// Function to print the current time without milliseconds
String getFormattedTimeWithoutMillis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Convert to tm structure
    struct tm* timeinfo = localtime(&tv.tv_sec);

    // Format the time as a string
    char buffer[9]; // Enough to hold "HH:MM:SS"
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
             timeinfo->tm_hour,
             timeinfo->tm_min,
             timeinfo->tm_sec);

    return String(buffer);
}

bool isTimeCorrect() {
    // Obtém o tempo atual do ESP32
    time_t now = time(nullptr);

    // Obtém o tempo da NTP
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        // Obtém o tempo NTP como time_t
        time_t ntpTime = mktime(&timeinfo);

        // Verifica se a diferença entre o tempo interno e o tempo NTP é aceitável
        // Aqui estamos permitindo uma diferença de até 1 segundo (1000 ms)
        if (abs(difftime(now, ntpTime)) < 1) {
            return true;  // O tempo está sincronizado
        }
    }
    return false;  // Não está sincronizado
}

void setup() {
    // initialize serial
    Serial.begin(115200);

    // initialize device
    M5.begin();

    // connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to Wi-Fi!");
    
    // connect to SNTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // wait for time synchronization
    while(!isTimeCorrect()){
        delay(500);
        Serial.print(".");
    }
    Serial.println("Tempo Correto!");

    waitForNextSecond();

    // configure interrupt with internal clock
    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &onTimer); 
    timerAlarm(timer, 1000000, true, 0);
    Serial.println("Interrupção configurada!");

    // configure Firebase
    config.api_key = DATABASE_API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = DATABASE_USER_EMAIL;
    auth.user.password = DATABASE_USER_PASSWORD;
    Firebase.begin(&config, &auth);
    while(!Firebase.ready()){
        Serial.println("Error: Firebase is not ready");
        delay(1000); // Keep the ESP32 in wait for debugging
        Firebase.begin(&config, &auth);
    }
    Firebase.reconnectWiFi(true);

    // create semaphore for variable synchronization
    xCountSemaphore = xSemaphoreCreateMutex();

    // create tasks
    xTaskCreatePinnedToCore(
        collectIMUData,     // Task function
        "Task1",            // Task name
        15000,              // Stack size
        NULL,               // Task input parameter
        1,                  // Priority of the task
        &Task1,             // Task handle
        0);                 // Core 0

    xTaskCreatePinnedToCore(
        sendDataToFirebase,  // Task function
        "Task2",             // Task name
        15000,               // Stack size
        NULL,                // Task input parameter
        2,                   // Priority of the task
        &Task2,              // Task handle
        1);                  // Core 1

    Serial.println("Configuration completed");
}

void collectIMUData(void * parameter) {
    float ax, ay, az; // accelerometer variables
    float gx, gy, gz; // gyroscope variables

    while (true) {

        // check if button A was pressed
        if (M5.BtnA.wasPressed()) {
            imuActive = !imuActive; // toggle IMU state
            if (imuActive) {
                Serial.println("IMU activated");
            } else {
                Serial.println("IMU deactivated");
            }

            while(M5.BtnA.isPressed()) {
                M5.update();
                delay(10);
            }
        }

        // update button state
        M5.update();

        if (timerFlag) {
            // update the time
            timerFlag = false; // reset the flag

            printFormattedTimeWithMillis();

            current_time = getFormattedTimeWithoutMillis();
        }

        // collect and send IMU data if active
        if (imuActive) {
            // update IMU data
            if (M5.Imu.update()) {
                // get accelerometer and gyroscope data
                M5.Imu.getAccelData(&ax, &ay, &az);
                M5.Imu.getGyroData(&gx, &gy, &gz);

                // protect shared resources with semaphore
                xSemaphoreTake(xCountSemaphore, portMAX_DELAY);

                if (imuReadingsCount < readingsThreshold){
                    imuReadingsCount++;
                    xSemaphoreGive(xCountSemaphore);

                    // store readings in active JSON
                    FirebaseJson jsonEntry;
                    jsonEntry.set("aX", ax);
                    jsonEntry.set("aY", ay);
                    jsonEntry.set("aZ", az);
                    jsonEntry.set("gX", gx);
                    jsonEntry.set("gY", gy);
                    jsonEntry.set("gZ", gz);
                    activeJsonData->set(current_time + "/" + String(imuReadingsCount), jsonEntry);

                    //Serial.println(imuReadingsCount);
                } else {
                    xSemaphoreGive(xCountSemaphore);
                }
            }
        } 
        vTaskDelay(1);
    }
}

void sendDataToFirebase(void * parameter) {
    while (true) {
        // protect shared resources with semaphore
        xSemaphoreTake(xCountSemaphore, portMAX_DELAY);

        // check if the number of readings to send to Firebase has been reached
        if (imuReadingsCount >= readingsThreshold) {
            // swap buffers
            FirebaseJson* temp = activeJsonData;
            activeJsonData = sendingJsonData;
            sendingJsonData = temp;

            imuReadingsCount = 0; // reset the count for the new active buffer

            xSemaphoreGive(xCountSemaphore);

            if (sendingJsonData != nullptr) {
                //Serial.println("Sending data to Firebase...");
                if (Firebase.pushJSON(firebaseData, "/IMUData", *sendingJsonData)) {
                    Serial.println("Data sent successfully!");
                    sendingJsonData->clear();
                } else {
                    Serial.println("Erro ao enviar dados: " + firebaseData.errorReason());
                    sendingJsonData->clear(); // clear the sending buffer
                } 
            } else {
                Serial.println("Error: sendingJsonData is null");
            }
              
        } else {
            xSemaphoreGive(xCountSemaphore);
        }
    
        vTaskDelay(1);
    }
}

void loop(){

}
