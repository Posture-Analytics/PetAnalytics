#ifndef INTERRUPTMANAGER_H
#define INTERRUPTMANAGER_H

#include <esp_sntp.h>

namespace InterruptManager {
    void initializeExactSecondInterrupt(void (*callback)());
}

#endif // INTERRUPTMANAGER_H
