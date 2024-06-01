#ifndef PINCONFIG_H
#define PINCONFIG_H

// IMU CS pins
#define CS_1_IMU 15
#define CS_2_IMU 27
#define CS_3_IMU 33

// Hardware SPI pins
#define HSPI_MISO   12
#define HSPI_MOSI   13
#define HSPI_SCLK   14
#define HSPI_CS     CS_1_IMU //ned to be the same as the first IMU to initialize

// Virtual SPI pins (use the standard pins)
#define VSPI_MISO   MISO // 19
#define VSPI_MOSI   MOSI // 23
#define VSPI_SCLK   SCK // 18
#define VSPI_CS     5 

#endif
