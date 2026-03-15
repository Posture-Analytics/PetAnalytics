//rename the file to use it

// config.h
#ifndef CONFIG_H
#define CONFIG_H

// Wi-Fi Configuration
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// Firebase Configuration
#define DATABASE_URL "https://your-firebase-database-url.firebaseio.com/"
#define DATABASE_API_KEY "your_firebase_api_key"

#define DATABASE_USER_EMAIL "your_email@example.com"
#define DATABASE_USER_PASSWORD "your_secure_password"

// NTP Configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // (GMT-3)
const int daylightOffset_sec = 0; 

#endif // CONFIG_H

