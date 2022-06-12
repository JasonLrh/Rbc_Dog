#include <string.h>
#include <stdio.h>
#include <sys/param.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"

#include "tcp_dap_interface.h"

static const char *TAG = "[TCP-dap]";

// void tcp_send_client(tcp_server_conf_t * conf, const uint8_t * const msg, ssize_t len){
//     TaskStatus_t status;
//     eTaskState state = eRunning;
    
//     if (msg != NULL){
//         memcpy(conf->tx_buff, msg, (size_t)len);
//     }

//     vTaskGetTaskInfo(conf->txTaskHandle, &status, pdFALSE, state);
//     switch (status.eCurrentState)
//     {
//     case eRunning:
//     case eReady:
//     case eBlocked:
//     case eSuspended:{
//         xSemaphoreGive(conf->txSem);
//     } break;
    
//     default:
//         break;
//     }
// }

// void __tcp_server_task_send(void * pvParameters){
//     tcp_server_conf_t * configure = (tcp_server_conf_t *)pvParameters;
//     ESP_LOGI(TAG, "tcp send task start");
//     while (1){
//         if (xSemaphoreTake(configure->txSem, portMAX_DELAY) == pdTRUE){
//             ssize_t al_send = 0;
//             ESP_LOGI(TAG, "start send tcp");
//             while(al_send < configure->tx_buf_len){
//                 al_send += send(configure->sock, configure->tx_buff + al_send, configure->tx_buf_len - al_send, 0);
//             }
//             ESP_LOGI(TAG, "send tcp ok");
//         }
//     }
// }

void __tcp_server_task(void * pvParameters){
    char addr_str[128];
    int addr_family = (int)AF_INET;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = 5;
    int keepInterval = 5;
    int keepCount = 3;
    int listen_sock = -1;
    struct sockaddr_storage dest_addr;
    // tcp_server_conf_t * configure = malloc(sizeof(tcp_server_conf_t));
    tcp_server_conf_t * configure = (tcp_server_conf_t *)pvParameters;
    // if (configure == NULL){
    //     ESP_LOGE(TAG, "no mem to malloc tcp server task");
    //     goto CLEAN_UP;
    // }
    // memcpy(configure, pvParameters, sizeof(tcp_server_conf_t));

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(configure->port);
    ip_protocol = IPPROTO_IP;

    listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        goto CLEAN_UP;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", configure->port);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }
    configure->sock = -1;
    while (1) {

        ESP_LOGV(TAG, "Wait for connect");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        configure->sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (configure->sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(configure->sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(configure->sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(configure->sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(configure->sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        int len;

        char * name_buf = malloc(15);
        if (name_buf == NULL){
            ESP_LOGE(TAG, "no mem to malloc");
            goto CLEAN_UP;
        }
        sprintf(name_buf, "sTcpTx%d", configure->port);

        // xTaskCreate(__tcp_server_task_send, "tcp_send", 4096, (void*)configure, 5, &(configure->txTaskHandle));
        free(name_buf);
        name_buf = NULL;

        if (configure->compTaskCode != NULL){
            xTaskCreate(configure->compTaskCode, "uart_event_task", 2048, NULL, 12, &(configure->compTaskHandle));
        }

        do {
            len = recv(configure->sock, configure->rx_buff, sizeof(configure->rx_buff), 0);
            if (len < 0) {
                ESP_LOGE(TAG, "(%4d)Error occurred during receiving: errno %d", configure->port, errno);
            } else if (len == 0) {
                ESP_LOGV(TAG, "(%4d)Connection closed", configure->port);
            } else {
                configure->rx_buff[len] = 0;
                // ESP_LOGI(TAG, "(%4d)TCP Received %d bytes", configure->port, len);
                // for (int i = 0; i < len; i++){
                //     printf("%02X ", configure->rx_buff[i]);
                //     if ((i+1) % 16 == 0){
                //         printf("\n");
                //     }
                // }
                // printf("\n");
                configure->rx_callback(configure->sock, (char *)(configure->rx_buff), len);
            }
        } while (len > 0);

        if (configure->compTaskCode != NULL){
            vTaskDelete(configure->compTaskHandle);
        }

        shutdown(configure->sock, 0);
        close(configure->sock);

        ESP_LOGI(TAG, "socket close");
    }

CLEAN_UP:
    if (configure != NULL){
        free(configure);
        configure = NULL;
    }
    if (listen_sock >= 0){
        close(listen_sock);
    }
    vTaskDelete(NULL);
}

void tcp_creat_server(tcp_server_conf_t * conf){
    char * name_buf = malloc(15);
    if (name_buf == NULL){
        ESP_LOGE(TAG, "no mem to malloc");
    }
    sprintf(name_buf, "sTcp%d", conf->port);
    xTaskCreate(__tcp_server_task, name_buf, 4096, (void*)conf, 5, &(conf->taskHandle));
    free(name_buf);
    name_buf = NULL;
}

void tcp_end_server(tcp_server_conf_t * conf){
    TaskStatus_t status;
    eTaskState state = eRunning;
    
    vTaskGetTaskInfo(conf->taskHandle, &status, pdFALSE, state);
    switch (status.eCurrentState)
    {
    case eRunning:
    case eReady:
    case eBlocked:
    case eSuspended:{
        vTaskDelete(conf->taskHandle);
    } break;
    
    default:
        break;
    }

    vTaskGetTaskInfo(conf->txTaskHandle, &status, pdFALSE, state);
    switch (status.eCurrentState)
    {
    case eRunning:
    case eReady:
    case eBlocked:
    case eSuspended:{
        vTaskDelete(conf->txTaskHandle);
    } break;
    
    default:
        break;
    }
    
}