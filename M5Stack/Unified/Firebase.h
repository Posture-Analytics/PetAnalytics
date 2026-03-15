#ifndef FIREBASE_H
#define FIREBASE_H

#include <FirebaseESP32.h>
#include "config.h" // change this file to your own config.h

class Firebase {
public:
    static void initialize();
    static void sendData();
    static void addEntry(const String& key, FirebaseJson& entry);
    static int getReadingsThreshold();
};

#endif // FIREBASE_H
