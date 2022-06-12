#ifndef TCP_DAP_INTERFACE_H
#define TCP_DAP_INTERFACE_H

#include <stdint.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define TCP_HOST_NAME "esp-dap"

typedef struct 
{
    void (*rx_callback)(int,char *,uint16_t);
    TaskHandle_t taskHandle;
    TaskHandle_t txTaskHandle;
    TaskFunction_t compTaskCode;
    TaskHandle_t   compTaskHandle;
    SemaphoreHandle_t txSem;
    uint8_t tx_buff[1024];
    uint8_t rx_buff[1024];
    uint16_t tx_buf_len;
    uint16_t tx_buf_pos;
    uint16_t port;
    int sock;
}tcp_server_conf_t;

void mdns_example_task(void *pvParameters);

void tcp_creat_server(tcp_server_conf_t * conf);
void tcp_end_server(tcp_server_conf_t * conf);
void tcp_send_client(tcp_server_conf_t * conf, const uint8_t * const msg, ssize_t len);


#endif