#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_sntp.h>

class Network {
public:
    static void connectToWiFi();
    static void syncTimeWithNTP();
    static String getFormattedTimeWithoutMillis();
    static void printFormattedTimeWithMillis();
    
private:
    static bool isTimeCorrect();
};

#endif // NETWORK_H
