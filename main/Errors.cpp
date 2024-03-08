#include "Errors.h"
#include "Debug.h"

Errors::Errors() {}

void Errors::showError(ErrorType error, bool fatal) {

    // Print the error message
    switch (error) {
        case ErrorType::None:
            LogInfoln("No error");
            break;
        case ErrorType::NoInternet:
            LogErrorln("No internet connection");
            break;
        case ErrorType::NoDatabaseConnection:
            LogErrorln("No database connection");
            break;
        case ErrorType::NoNTPdata:
            LogErrorln("NTP Sync failed");
            break;
        case ErrorType::BufferFull:
            LogErrorln("Buffer full");
            break;
        case ErrorType::IMUInitFailure:
            LogErrorln("IMU initialization failure");
            break;
        default:
            LogErrorln("Unknown error");
            break;
    }

    // If the error is fatal, restart the device after 3 seconds
    if (fatal) {
        LogFatalln("Reiniciando o dispositivo em 3 segundos...");
        delay(3000);
        ESP.restart();
    }
}