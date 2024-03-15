/*
    Credentials.h

    * This module stores the credentials for the WiFi connection and the Firebase Database 
      connection. It also stores the database user e-mail and password that were already 
      registered or added to the database project.
    * Please fill the credentials with your own data to be able to connect to the WiFi 
      network and to the Firebase Database. Otherwise, the device will not be able to send data 
      to the database or even connect to the WiFi network.
*/

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// Define the WiFi credentials (https://youtu.be/pDDdA9qEFwY)
static const char* WIFI_SSID = "Cadeirudo";
static const char* WIFI_PASSWORD = "cadeirinha123*#";

// Define the RTDB (Realtime Database) URL and the API Key
// static const char* DATABASE_URL = "https://friendly-bazaar-334818-default-rtdb.firebaseio.com/";
// static const char* DATABASE_API_KEY = "AIzaSyCO3WjQJodTcimQjzQnQ5_ZpEgxyaQR-0o";

// Test database credentials
static const char* DATABASE_API_KEY = "AIzaSyASxq5x-HYYTv1jLM2L1WJoC2xFTOaCj2E";
static const char* DATABASE_URL = "https://esp32test-115a2-default-rtdb.firebaseio.com/";

// NurseAid database credentials
// static const char* DATABASE_API_KEY = "AIzaSyBYcTd-yGnzoAa2uf-oSP_cB3Cjksv-FC8";
// static const char* DATABASE_URL = "https://nurseaid-663f1-default-rtdb.firebaseio.com/";

// Define the database user e-mail and password that were already registered or added to the 
// database project
static const char* DATABASE_USER_EMAIL = "admin@admin.com";
static const char* DATABASE_USER_PASSWORD = "adminadmin";

#endif

// git update-index --skip-worktree mainSketch/Credentials.h
