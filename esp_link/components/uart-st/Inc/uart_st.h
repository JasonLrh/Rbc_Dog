#ifndef UART_ST_H
#define UART_ST_H

#include <stdbool.h>

#include "driver/uart.h"
#include "driver/gpio.h"

#define EX_UART_NUM UART_NUM_1
#define BUF_SIZE    (1024 + 256)
#define RD_BUF_SIZE (BUF_SIZE)

typedef struct
{
    int pin_tx;
    int pin_rx;
    int pin_boot;
    int baud;
}uart_st_init_t;


typedef enum{
    SYSTEM_STATE_NORMAL = 0,
    SYSTEM_STATE_DOWNLOAD = 1,
}uart_system_tate_t;

extern uart_system_tate_t SysState;


void uart_st_init(uart_st_init_t * conf);
void uart_st_deinit(void);

void uart_st_set_boot_mode(bool isEnterBoot);
void uart_st_write(const char * cmd, int len);

#endif