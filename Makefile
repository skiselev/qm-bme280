#
# Copyright (c) 2017, Sergey Kiselev
# All rights reserved.
#
# Copyright (c) 2017 Sergey Kiselev
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

### BME280 environmental sensor driver sample makefile

### Variables
APP_NAME = bme280
SOC = quark_d2000
TARGET = x86
BASE_DIR ?= $(QMSI_SRC_DIR)
APP_DIR := .

### Make includes
include $(BASE_DIR)/examples/app.mk
