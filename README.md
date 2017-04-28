# qm-bme280
Bosch Sensortec BME280 Environmental Driver for Intel(r) QMSI 

## Introduction
This driver provides support for Bosch Sensortec BME280 environmental sensor
for projects developed using Intel(r) Quark(tm) Microcontroller Software Interface (aka QMSI).
The driver currently only supports sensors connected using I2C interface.

## Hardware Configuration

### Supported BME280 breakout boards
This driver has been tested with the following breakout boards:
* [Adafruit BME280 I2C or SPI Temperature Humidity Pressure Sensor; Product ID: 2652](https://www.adafruit.com/product/2652)
* [MikroElektronika Weather click; Product ID: MIKROE-1978](https://shop.mikroe.com/click/sensors/weather)

### Wiring - I2C Interface
| Signal Name        | Breakout Board Pin Name | Intel(r) Quark(tm) D2000 Board Pin Name | Notes                               |
|--------------------|-------------------------|-----------------------------------------|-------------------------------------|
| Ground             | GND                     | GND                                     |                                     |
| Power Supply       | VIN (Adafruit)          | 5V or 3.3V                              | Adafruit BME280 board contains a 3.3V regulator and it will work from either 3.3V or 5V |
| Power Supply, 3.3V | 3.3V (MikroElektronika) | 3.3V                                    | MikroElektronika Weather Click must use 3.3V power supply |
| I2C Data           | SDA, SDI (Adafruit)     | A4 or SDA                               |                                     |
| I2C Clock          | SCL, SCK (Adafruit)     | A5 or SCL                               |                                     |
| I2C Address        | SDO (Adafruit)          | GND                                     | Connecting SDO pin to the GND sets I2C address to 0x76 instead of 0x77 |

## Using the Driver

### Building the Sample Application
1. Create a new QMSI based project, for example using Intel(r) System Studio for Microcontrollers
2. Copy *main.c*, *bme280.h*, and *bme280.c* files to your project directory
3. Build and run the project

### Integrating the Driver in Your Application

1. Create a new QMSI based project, for example using Intel(r) System Studio for Microcontrollers
2. Copy *bme280.h* and *bme280.c* files to your project directory
3. Add `include "bme280.h"` line in your main.c
4. Edit *bme280.h*.
  * Set `BME280_I2C_ADDR` to 'BME280_I2C_ADDR_77' or 'BME280_I2C_ADDR_76' according to your BME280 sensor I2C address
  * If needed, update other constants in the *default configuration* section of the file
5. Add I2C initialization code. Refer to `setup()` function in *main.c* included with this driver.
6. From your code call `bme280_init()`, and then use other `bme280_*()` functions in your application as needed. Refer to *main.c* for usage example.
