# PetAnalytics

This repository contains multiple hardware and software projects, all part of the PetAnalytics project. Each subproject serves a unique purpose, ranging from IoT devices to video annotation software.

## 1. [Sparkfun Projects](./Sparkfun)
This project focuses on developing an IoT device to collect data on animal behavior using an ESP32 and SparkFun IMU sensors.

- **Main components**: ESP32, SparkFun IMU.
- **Features**: Data logging on an SD card, motion data collection from IMUs.

### Subprojects

- [One IMU (sdcard/one)](./Sparkfun/sdcard/one): Project using a single IMU sensor to capture motion data and store it on an SD card.
- [Three IMUs (sdcard/three)](./Sparkfun/sdcard/three): Version with three IMU sensors for more comprehensive data collection and SD card storage.
- [Local Database (local_database)](./Sparkfun/local_database): Implementation of a local database on the SD card to store and retrieve collected data. *(Needs review)*

## 2. [M5Stack Projects](./M5Stack)
Projects utilizing various M5Stack hardware for data capture and monitoring. M5 devices are practical and modular, allowing easy integration with a variety of sensors.

### Subprojects

- [Capsule](./M5Stack/Capsule): Project using the **[M5Stack Capsule](https://shop.m5stack.com/products/m5stack-capsule-kit-w-m5stamps3)**, a compact device for data collection and monitoring.
- [StickCPlus](./M5Stack/StickCPlus): Project using the **[M5StickC Plus](https://shop.m5stack.com/products/m5stickc-plus-esp32-pico-mini-iot-development-kit)**, a small but powerful IoT development kit.
- [Multiple IMU (multiple_imu)](./M5Stack/multiple_imu): Tests with external IMUs using the **[M5 IMU units](https://shop.m5stack.com/products/6-dof-imu-pro-mini-unit-bmi270-bmm150-bmp280)** for motion data collection from various sensors. *(To-Do)*

## 3. [Video Annotation Software](./VideoLabelerProject)
This software is designed for video annotation and labeling, specifically to label animal behavior. It provides a graphical interface for video manipulation, including playback at different speeds and frame-by-frame labeling.

- **Technologies used**: Python, Tkinter.
- **Features**: Synchronization with sensors *(To-Do)*, behavior annotation in video.

## Acknowledgements

Special thanks to the team members, Professor Rafael de Pinho André, and FGV EMAp for their contributions, support, and resources provided to this project.

## Contact

For any inquiries or feedback, please contact us at: gustavotironi100@gmail.com
