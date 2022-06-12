#include "uart_st.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

static const char *TAG = "[uart_st.c]";

QueueHandle_t uart0_queue;
static int BOOT_PIN = -1;

uart_system_tate_t SysState = SYSTEM_STATE_NORMAL;

// uint8_t serial_rx_buff[1024];

void uart_st_set_boot_mode(bool isEnterBoot){
    gpio_set_level(BOOT_PIN, isEnterBoot == true? 1 : 0 );
}

void uart_st_write(const char * cmd, int len){
    uart_write_bytes(EX_UART_NUM,  cmd, (size_t)len);
}


extern void uart_event_task(void *pvParameters);
void uart_st_init(uart_st_init_t * conf){
    uart_config_t uart_config = {
        .baud_rate = conf->baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pin_bit_mask = (1ULL << conf->pin_boot)
    };
    BOOT_PIN = conf->pin_boot;
    // configure boot pin for stm32(bootloader)
    gpio_config(&io_conf);
    gpio_set_level(conf->pin_boot, 0); // rising edge to enter boot

    //Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);
    uart_set_rx_timeout(EX_UART_NUM, 101);
    uart_set_always_rx_timeout(EX_UART_NUM, true);

    //Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    //Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, conf->pin_tx, conf->pin_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Set uart pattern detect function.
    // uart_enable_pattern_det_baud_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 9, 0, 0);
    //Reset the pattern queue length to record at most 20 pattern positions.
    // uart_pattern_queue_reset(EX_UART_NUM, 20);

    //Create a task to handler UART event from ISR
}