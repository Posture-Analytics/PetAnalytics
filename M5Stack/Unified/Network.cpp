#include "Network.h"

void Network::connectToWiFi() {
    WiFi.begin("SSID", "PASSWORD");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi.");
}

void Network::syncTimeWithNTP() {
    configTime(gmtOffset_sec, 0, "pool.ntp.org");
    while(!isTimeCorrect()){
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nInternal time synchronized.");
}

bool Network::isTimeCorrect() {
    // Get the current time from the ESP32
    time_t now = time(nullptr);

    // Get the NTP time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        // Get the NTP time as time_t
        time_t ntpTime = mktime(&timeinfo);

        // Check if the difference between the internal time and the NTP time
        if (abs(difftime(now, ntpTime)) < 1) {
            return true; 
        }
    }
    return false; 
}

String Network::getFormattedTimeWithoutMillis() {
    // Get the current time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* timeinfo = localtime(&tv.tv_sec);

    // Format the time as a string
    char buffer[9]; // Enough to hold "HH:MM:SS"
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
             timeinfo->tm_hour,
             timeinfo->tm_min,
             timeinfo->tm_sec);

    return String(buffer);
}

void Network::printFormattedTimeWithMillis() {
    // Get the current time
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* timeinfo = localtime(&tv.tv_sec);

    // Extract milliseconds
    int milliseconds = tv.tv_usec / 1000;

    // Format and print the time with milliseconds "HH:MM:SS.SSS"
    Serial.println("Hora atual com milissegundos: %02d:%02d:%02d.%03d\n",
                  timeinfo->tm_hour,
                  timeinfo->tm_min,
                  timeinfo->tm_sec,
                  milliseconds);
}

