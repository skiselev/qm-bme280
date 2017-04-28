/*
 * Ported to QMSI from Zephyr
 * Copyright (c) 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BME280_H__
#define __BME280_H__

#include <stdint.h>

#define BME280_REG_PRESS_MSB		0xF7
#define BME280_REG_COMP_START		0x88
#define BME280_REG_HUM_COMP_PART1	0xA1
#define BME280_REG_HUM_COMP_PART2	0xE1
#define BME280_REG_ID			0xD0
#define BME280_REG_CONFIG		0xF5
#define BME280_REG_CTRL_MEAS		0xF4
#define BME280_REG_CTRL_HUM		0xF2

#define BMP280_CHIP_ID_SAMPLE_1		0x56
#define BMP280_CHIP_ID_SAMPLE_2		0x57
#define BMP280_CHIP_ID_MP		0x58
#define BME280_CHIP_ID			0x60
#define BME280_MODE_NORMAL		0x03
#define BME280_SPI_3W_DISABLE		0x00

#define BME280_TEMP_OVER_1X		(1 << 5)
#define BME280_TEMP_OVER_2X		(2 << 5)
#define BME280_TEMP_OVER_4X		(3 << 5)
#define BME280_TEMP_OVER_8X		(4 << 5)
#define BME280_TEMP_OVER_16X		(5 << 5)

#define BME280_PRESS_OVER_1X		(1 << 2)
#define BME280_PRESS_OVER_2X		(2 << 2)
#define BME280_PRESS_OVER_4X		(3 << 2)
#define BME280_PRESS_OVER_8X		(4 << 2)
#define BME280_PRESS_OVER_16X		(5 << 2)

#define BME280_HUMIDITY_OVER_1X		1
#define BME280_HUMIDITY_OVER_2X		2
#define BME280_HUMIDITY_OVER_4X		3
#define BME280_HUMIDITY_OVER_8X		4
#define BME280_HUMIDITY_OVER_16X	5

#define BME280_STANDBY_05MS		0
#define BME280_STANDBY_62MS		(1 << 5)
#define BME280_STANDBY_125MS		(2 << 5)
#define BME280_STANDBY_250MS		(3 << 5)
#define BME280_STANDBY_500MS		(4 << 5)
#define BME280_STANDBY_1000MS		(5 << 5)
#define BME280_STANDBY_2000MS		(6 << 5)
#define BME280_STANDBY_4000MS		(7 << 5)

#define BME280_FILTER_OFF		0
#define BME280_FILTER_2			(1 << 2)
#define BME280_FILTER_4			(2 << 2)
#define BME280_FILTER_8			(3 << 2)
#define BME280_FILTER_16		(4 << 2)

#define BME280_I2C_ADDR_76		0x76
#define BME280_I2C_ADDR_77		0x77

/* default configuration */
#define BME280_I2C_BUS			QM_I2C_0
#define BME280_I2C_ADDR			BME280_I2C_ADDR_77
#define BME280_I2C_RETRY_COUNT		5
#define BME280_TEMP_OVER		BME280_TEMP_OVER_4X
#define BME280_PRESS_OVER		BME280_PRESS_OVER_4X
#define BME280_HUMIDITY_OVER		BME280_HUMIDITY_OVER_4X
#define BME280_STANDBY			BME280_STANDBY_250MS
#define BME280_FILTER			BME280_FILTER_OFF
/* end default configuration */

#define BME280_CTRL_MEAS_VAL		(BME280_PRESS_OVER |	\
					 BME280_TEMP_OVER |	\
					 BME280_MODE_NORMAL)
#define BME280_CONFIG_VAL		(BME280_STANDBY |	\
					 BME280_FILTER |	\
					 BME280_SPI_3W_DISABLE)

struct bme280_data {
	/* Compensation parameters. */
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
	uint16_t dig_p1;
	int16_t dig_p2;
	int16_t dig_p3;
	int16_t dig_p4;
	int16_t dig_p5;
	int16_t dig_p6;
	int16_t dig_p7;
	int16_t dig_p8;
	int16_t dig_p9;
	uint8_t dig_h1;
	int16_t dig_h2;
	uint8_t dig_h3;
	int16_t dig_h4;
	int16_t dig_h5;
	int8_t dig_h6;

	/* Compensated values. */
	int32_t comp_temp;
	uint32_t comp_press;
	uint32_t comp_humidity;

	/* Carryover between temperature and pressure/humidity compensation. */
	int32_t t_fine;

	uint8_t chip_id;
};

/* fetches data from the sensor and calculates compensated values */
int bme280_sample_fetch(struct bme280_data *data);

/* returns temperature in 0.01 degC */
int32_t bme280_get_temperature(struct bme280_data *data);

/* returns pressure in 1/256 Pa */
uint32_t bme280_get_pressure(struct bme280_data *data);

/* returns humidity in 1/1024 %RH */
uint32_t bme280_get_humidity(struct bme280_data *data);

/* initializes BME280 chip */
int bme280_init(struct bme280_data *data);

#endif /* __BME280_H__ */
