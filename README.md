# PetAnalytics

Code of the PetAnalytics project's IoT device, handling the microcontroller and the sensors connected to a dog harness.

---

## Table of Contents

- [Installation & Setup](#installation--setup)
- [Modules](#modules)
- [Configuration & Variables](#configuration--variables)
- [Future Improvements](#future-improvements)
- [Contact](#contact)

---

## Installation & Setup

1. **GitHub Repository Clone**: Clone or download the repository.
2. **Install Arduino IDE 2.0**: Download and install the latest version of the Arduino IDE 2.0 from [here](https://www.arduino.cc/en/software).
3. **Install ESP32 boards dependencies**: Open the Arduino IDE 2.0 `Preferences` and add the following link to the `Additional Boards Manager URLs` field: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`.
3. **Install ESP32 Board on Arduino IDE**: Open the Arduino IDE 2.0 and go to `Tools > Board > Boards Manager`. Search for `esp32` and install the latest version of the board.
4. **Install the necessary libraries**: Open the Arduino IDE 2.0 and go to `Tools > Manage Libraries`. Search and install the following libraries:
    - ` SparkFun ICM-20948`
    - ` SD`
5. **Open the sketch and configure the code**: Open the `PINConfig.h` file in the Arduino IDE 2.0 and configure the code (see [Configuration & Variables](#configuration--variables)).
6. **Connect the microcontroller to the computer**: Connect the microcontroller to the computer using a USB cable. Also select the correct COM port in the Arduino IDE 2.0 (`Tools > Port`) and the correct board (`Tools > Board > esp32 Arduino > SparkFun ESP32 Thing Plus C`).
7. **Upload the code to the microcontroller**: Open the `main.ino` file in the Arduino IDE 2.0 and upload the code to the microcontroller (`Sketch > Upload` / `Ctrl+U`). Wait until the code is uploaded and the microcontroller is ready to use.

## Modules

| Module Name | Description |
|-------------|-------------|
| `IMU` |  Sets the IMU sample rate and reads the data.|
| `SDCard` | Used to handle the microSD card. Can read, write, and create files/directories|
| `PINConfig` | Configures the pins to be used in SPI communication.|

## Configuration & Variables

This section highlights the key variables in the project which can be changed for customization.

| Variable Name | Module     | Description                   | Default Value |
|---------------|------------|-------------------------------|---------------|
| `CS_[n]_IMU`  | `PINConfig`| CS pin of each IMU            | `15, 27, 33`  |
| `HSPI_MISO`   | `PINConfig`| MISO pin for Hardware SPI     | `12`          |
| `HSPI_MOSI`   | `PINConfig`| MOSI pin for Hardware SPI     | `13`          |
| `HSPI_SCLK`   | `PINConfig`| SCLK pin for Hardware SPI     | `14`          |
| `HSPI_CS`     | `PINConfig`| CS pin for Hardware SPI       | `CS_1_IMU`    |
| `VSPI_MISO`   | `PINConfig`| MISO pin for Virtual SPI      | `19`          |
| `VSPI_MOSI`   | `PINConfig`| MOSI pin for Virtual SPI      | `23`          |
| `VSPI_SCLK`   | `PINConfig`| SCLK pin for Virtual SPI      | `18`          |
| `VSPI_CS`     | `PINConfig`| CS pin for Virtual SPI        | `5`           |

## Future Improvements

- **Update the old modules**: Adapt the existing modules such as the buffer, data reader, and error handling to work seamlessly with an SD card for efficient data storage and retrieval.
- **Store real-time timestamps**: Modify the code to capture and store real-time timestamps corresponding to when the data was collected, providing accurate temporal information for analysis and reference.
- **Connect to a local database**: Enable integration with a database, ensuring efficient storage and retrieval of data. This enhances overall data management, supporting comprehensive analytics and user-specific insights.
- **Wifi communication**: Incorporate wireless capabilities to enhance data transfer speed and reliability, ensuring a more responsive user experience.

## Acknowledgements

Thank you to the team members, our Professor Rafael de Pinho André and FGV EMAp for their contributions, support, or resources provided to this project.

## Contact

For any queries or feedback, please contact us at: gustavotironi100@gmail.com
