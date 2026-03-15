#include "InterruptManager.h"

namespace InterruptManager {
    hw_timer_t* timer = NULL;

    void initializeExactSecondInterrupt(void (*callback)()) {
        while (!NetworkManager::isTimeCorrect()) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("Time synchronized correctly!");

        NetworkManager::waitForNextSecond();

        timer = timerBegin(0, 80, true);
        timerAttachInterrupt(timer, callback, true);
        timerAlarmWrite(timer, 1000000, true);
        timerAlarmEnable(timer);

        Serial.println("Interrupt configured for exact seconds!");
    }
}