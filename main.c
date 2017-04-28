/*
 * Copyright (c) 2017 Sergey Kiselev
 * All rights reserved.
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

#include <clk.h>
#include <qm_common.h>
#include <qm_gpio.h>
#include <qm_i2c.h>
#include <qm_pinmux.h>
#include "bme280.h"

/* Setup I2C bus */
void setup()
{
	static const qm_i2c_config_t i2c_cfg = {
		.address_mode	= QM_I2C_7_BIT,		/* 7 bit address */
		.mode			= QM_I2C_MASTER,	/* master mode */
		.speed			= QM_I2C_SPEED_STD,	/* ~111 kHz clock */
	};
	/* Setup pin muxes */
	qm_pmux_select(QM_PIN_ID_6, QM_PMUX_FN_2);	/* I2C - SCL */
	qm_pmux_select(QM_PIN_ID_7, QM_PMUX_FN_2);	/* I2C - SDA */

	/*  Enable I2C master 0 clock */
	clk_periph_enable(CLK_PERIPH_I2C_M0_REGISTER | CLK_PERIPH_CLK);

	/* Initialize I2C */
	qm_i2c_set_config(QM_I2C_0, &i2c_cfg);

}

/* BME280 Environmental Sensor Sample */
int main()
{
	int rc = 0;
	struct bme280_data bme280_data;
	int32_t temperature;
	uint32_t pressure, humidity;
	char buffer[16];

	QM_PRINTF("BME280 Environmental Sensor Sample\r\n");

	/* Configure Quark I/O */
	setup();

	/* Initialize BME280 */
	rc = bme280_init(&bme280_data);
	if (rc == 0) {
		QM_PRINTF("BME280: Initialized environmental sensor\r\n");
	} else {
		QM_PRINTF("BME280: Initialization error %d\r\n", rc);
	}


	while(1) {
		bme280_sample_fetch(&bme280_data);
		temperature = bme280_get_temperature(&bme280_data);
		pressure = bme280_get_pressure(&bme280_data);
		humidity = bme280_get_humidity(&bme280_data);

		sprintf(buffer, "%d.%02d", temperature / 100, temperature % 100);
		QM_PRINTF("temperature:   %s C\r\n", buffer);
		sprintf(buffer, "%d.%02d", pressure / 256 / 100, pressure / 256 % 100);
		QM_PRINTF("pressure:      %s hPa\r\n", buffer);
		QM_PRINTF("humidity:      %d %RH\r\n", humidity / 1024);
		clk_sys_udelay(1000000);
	}

	return 0;
}
