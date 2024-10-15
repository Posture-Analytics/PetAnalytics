# PetAnalytics - M5

Code of the PetAnalytics project's IoT device, handling the microcontroller and the sensors connected to a dog harness.

---

## Table of Contents

- [Installation & Setup](#installation--setup)
- [Modules](#modules)
- [Future Improvements](#future-improvements)
- [Contact](#contact)

---

## Installation & Setup

1. **GitHub Repository Clone**: Clone or download the repository.
2. **Install Arduino IDE 2.0**: Download and install the latest version of the Arduino IDE 2.3 from [here](https://www.arduino.cc/en/software).
3. **Install M5 boards dependencies**: Open the Arduino IDE 2.3 `Preferences` and add the following link to the `Additional Boards Manager URLs` field: `https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json`.
4.  **Install ESP32 Board on Arduino IDE**: Open the Arduino IDE 2.3 and go to `Tools > Board > Boards Manager`. Search for `M5Stack` and install the latest version of the board.
5. **Install the necessary libraries**: Open the Arduino IDE 2.3 and go to `Tools > Manage Libraries`. Search and install the following libraries:
    - `M5StickCPlus2`
    - `M5Unified`
    - `NTPClient`
    - `Firebase ESP32 Client`
> **Note**: For more details, check [M5 docs](https://docs.m5stack.switch-science.com/en/arduino/arduino_ide).
6. **Open the sketch and configure the code**: Open the `config.h` file in the Arduino IDE 2.3 and configure the variables (see [Configuration & Variables](#configuration--variables)).
7. **Connect the microcontroller to the computer**: Connect the microcontroller to the computer using a USB cable. Also select the correct COM port in the Arduino IDE 2.3 (`Tools > Port`) and the correct board (`Tools > Board > M5 > M5StickCPlus`).
8. **Upload the code to the microcontroller**: Open the `StickCPlus.ino` file in the Arduino IDE 2.3 and upload the code to the microcontroller (`Sketch > Upload` / `Ctrl+U`). Wait until the code is uploaded and the microcontroller is ready to use.

## Modules

| Module Name | Description |
|-------------|-------------|
| `StickCPlus` |  Main code for M5StickCPlus with firebase.|
| `config` |  Configuration file for network and firebase.|

## Configuration & Variables

This section highlights the key variables in the project which can be changed for customization.

| Variable Name | Module     | Description                   |
|---------------|------------|-------------------------------|
| `WIFI_SSID`  | `config`| Wifi name|
| `WIFI_PASSWORD`   | `config`| Wifi password|
| `DATABASE_URL`   | `config`| Firebase URL|
| `DATABASE_API_KEY`   | `config`| Firebase API_KEY|
| `DATABASE_USER_EMAIL`     | `config`| User email allowed to access firebase|
| `DATABASE_USER_PASSWORD`   | `config`| User password allowed to access firebase|

## Future Improvements



## Acknowledgements

Thank you to the team members, our Professor Rafael de Pinho André and FGV EMAp for their contributions, support, or resources provided to this project.

## Contact

For any queries or feedback, please contact us at: gustavotironi100@gmail.com
