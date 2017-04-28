/* bme280.c
 * Driver for Bosch BME280 temperature, pressure, and humidity sensor
 */

/*
 * Ported from Zephyr
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

#include <qm_common.h>
#include <qm_i2c.h>
#include "bme280.h"

/* I2C read/write operations */
static int bme280_read_buf(uint8_t reg, uint8_t *data, int len)
{
	int retry_count = BME280_I2C_RETRY_COUNT;
	qm_i2c_status_t status;

        /* Write the command */
        while ((qm_i2c_master_write(BME280_I2C_BUS, BME280_I2C_ADDR, &reg, 1, false, &status)) &&
               (retry_count != 0)) {
                retry_count--;
        }
        if (retry_count == 0)
                return -EIO;

        retry_count = BME280_I2C_RETRY_COUNT;

        /* Read register */
        while ((qm_i2c_master_read(BME280_I2C_BUS, BME280_I2C_ADDR, data, len, true, &status)) &&
               (retry_count != 0)) {
                retry_count--;
        }
        if (retry_count == 0)
                return -EIO;

        return 0;
}

static int bme280_read_byte(uint8_t reg, uint8_t *data)
{
	int retry_count = BME280_I2C_RETRY_COUNT;
	qm_i2c_status_t status;

        /* Write the command */
        while ((qm_i2c_master_write(BME280_I2C_BUS, BME280_I2C_ADDR, &reg, 1, false, &status)) &&
               (retry_count != 0)) {
                retry_count--;
        }
        if (retry_count == 0)
                return -EIO;

        retry_count = BME280_I2C_RETRY_COUNT;

        /* Read register */
        while ((qm_i2c_master_read(BME280_I2C_BUS, BME280_I2C_ADDR, data, 1, true, &status)) &&
               (retry_count != 0)) {
                retry_count--;
        }
        if (retry_count == 0)
                return -EIO;

        return 0;
}

static int bme280_write_byte(uint8_t reg, uint8_t data)
{
	int retry_count = BME280_I2C_RETRY_COUNT;
	qm_i2c_status_t status;
	uint8_t buf[2];

	buf[0] = reg;
	buf[1] = data;

        /* Write the command */
        while ((qm_i2c_master_write(BME280_I2C_BUS, BME280_I2C_ADDR, buf, 2, true, &status)) &&
               (retry_count != 0)) {
                retry_count--;
        }
        if (retry_count == 0)
                return -EIO;

        return 0;
}

/*
 * Compensation code taken from BME280 datasheet, Section 4.2.3
 * "Compensation formula".
 */
static void bme280_compensate_temp(struct bme280_data *data, int32_t adc_temp)
{
	int32_t var1, var2;

	var1 = (((adc_temp >> 3) - ((int32_t)data->dig_t1 << 1)) *
		((int32_t)data->dig_t2)) >> 11;
	var2 = (((((adc_temp >> 4) - ((int32_t)data->dig_t1)) *
		  ((adc_temp >> 4) - ((int32_t)data->dig_t1))) >> 12) *
		((int32_t)data->dig_t3)) >> 14;

	data->t_fine = var1 + var2;
	data->comp_temp = (data->t_fine * 5 + 128) >> 8;
}

static void bme280_compensate_press(struct bme280_data *data, int32_t adc_press)
{
	int64_t var1, var2, p;

	var1 = ((int64_t)data->t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)data->dig_p6;
	var2 = var2 + ((var1 * (int64_t)data->dig_p5) << 17);
	var2 = var2 + (((int64_t)data->dig_p4) << 35);
	var1 = ((var1 * var1 * (int64_t)data->dig_p3) >> 8) +
		((var1 * (int64_t)data->dig_p2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)data->dig_p1) >> 33;

	/* Avoid exception caused by division by zero. */
	if (var1 == 0) {
		data->comp_press = 0;
		return;
	}

	p = 1048576 - adc_press;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)data->dig_p9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)data->dig_p8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)data->dig_p7) << 4);

	data->comp_press = (uint32_t)p;
}

static void bme280_compensate_humidity(struct bme280_data *data,
				       int32_t adc_humidity)
{
	int32_t h;

	h = (data->t_fine - ((int32_t)76800));
	h = ((((adc_humidity << 14) - (((int32_t)data->dig_h4) << 20) -
		(((int32_t)data->dig_h5) * h)) + ((int32_t)16384)) >> 15) *
		(((((((h * ((int32_t)data->dig_h6)) >> 10) * (((h *
		((int32_t)data->dig_h3)) >> 11) + ((int32_t)32768))) >> 10) +
		((int32_t)2097152)) * ((int32_t)data->dig_h2) + 8192) >> 14);
	h = (h - (((((h >> 15) * (h >> 15)) >> 7) *
		((int32_t)data->dig_h1)) >> 4));
	h = (h > 419430400 ? 419430400 : h);

	data->comp_humidity = (uint32_t)(h >> 12);
}

int bme280_sample_fetch(struct bme280_data *data)
{
	uint8_t buf[8];
	int32_t adc_press, adc_temp, adc_humidity;
	int size = 6;

	if (data->chip_id == BME280_CHIP_ID) {
		size = 8;
	}

	if (bme280_read_buf(BME280_REG_PRESS_MSB, buf, size) < 0) {
		return -EIO;
	}

	adc_press = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
	adc_temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);

	bme280_compensate_temp(data, adc_temp);
	bme280_compensate_press(data, adc_press);

	if (data->chip_id == BME280_CHIP_ID) {
		adc_humidity = (buf[6] << 8) | buf[7];
		bme280_compensate_humidity(data, adc_humidity);
	}

	return 0;
}

/* returns temperature in 0.01 degC */
int32_t bme280_get_temperature(struct bme280_data *data)
{
	return data->comp_temp;
}

/* returns pressure in 1/256 Pa */
uint32_t bme280_get_pressure(struct bme280_data *data)
{
	return data->comp_press;
}

/* returns humidity in 1/1024 %RH */
uint32_t bme280_get_humidity(struct bme280_data *data)
{
	return data->comp_humidity;
}

static void bme280_read_compensation(struct bme280_data *data)
{
	uint16_t buf[12];
	uint8_t hbuf[7];

	bme280_read_buf(BME280_REG_COMP_START, (uint8_t *)buf, sizeof(buf));

	data->dig_t1 = buf[0];
	data->dig_t2 = buf[1];
	data->dig_t3 = buf[2];
	data->dig_p1 = buf[3];
	data->dig_p2 = buf[4];
	data->dig_p3 = buf[5];
	data->dig_p4 = buf[6];
	data->dig_p5 = buf[7];
	data->dig_p6 = buf[8];
	data->dig_p7 = buf[9];
	data->dig_p8 = buf[10];
	data->dig_p9 = buf[11];

	if (data->chip_id == BME280_CHIP_ID) {
		bme280_read_byte(BME280_REG_HUM_COMP_PART1, &data->dig_h1);
		bme280_read_buf(BME280_REG_HUM_COMP_PART2, hbuf, 7);

		data->dig_h2 = (hbuf[1] << 8) | hbuf[0];
		data->dig_h3 = hbuf[2];
		data->dig_h4 = (hbuf[3] << 4) | (hbuf[4] & 0x0F);
		data->dig_h5 = ((hbuf[4] >> 4) & 0x0F) | (hbuf[5] << 4);
		data->dig_h6 = hbuf[6];
	}
}

int bme280_init(struct bme280_data *data)
{
	bme280_read_byte(BME280_REG_ID, &data->chip_id);

	if (data->chip_id == BME280_CHIP_ID) {
		QM_PRINTF("BME280 chip detected\r\n");
	} else if (data->chip_id == BMP280_CHIP_ID_MP ||
		   data->chip_id == BMP280_CHIP_ID_SAMPLE_1) {
		QM_PRINTF("BMP280 chip detected\r\n");
	} else {
		QM_PRINTF("BME280: bad chip id 0x%x\r\n", data->chip_id);
		return -EINVAL;
	}

	bme280_read_compensation(data);

	if (data->chip_id == BME280_CHIP_ID) {
		bme280_write_byte(BME280_REG_CTRL_HUM, BME280_HUMIDITY_OVER);
	}

	bme280_write_byte(BME280_REG_CTRL_MEAS, BME280_CTRL_MEAS_VAL);
	bme280_write_byte(BME280_REG_CONFIG, BME280_CONFIG_VAL);

	return 0;
}
