#ifndef ST_SPI_API_H
#define ST_SPI_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "sdkconfig.h"
#include "esp_log.h"


void st_spi_init(void);
void st_spi_trans(uint8_t data);

#endif