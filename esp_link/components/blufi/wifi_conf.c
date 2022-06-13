

#include "tcp_dap_interface.h"
#include "uart_st.h"
#include "esp_log.h"

#include <stdio.h>

extern QueueHandle_t uart0_queue;
const static char * TAG = "[wifi_conf.c]";


static void st_serial_rx_cb(int sock, char * cmd, uint16_t len);
static void st_ota_tcp_rx_cb(int sock, char * cmd, uint16_t len);
void uart_event_task(void *pvParameters);

static tcp_server_conf_t st_ota_conf = {
    .port=3333,
    .rx_callback = st_ota_tcp_rx_cb,
    .compTaskCode = NULL
};

static tcp_server_conf_t st_serial_conf = {
    .port=3334,
    .rx_callback = st_serial_rx_cb,
    .compTaskCode = uart_event_task
};

void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint32_t base_pos ;
    // int state;
    int temp;
    // size_t buffered_size;
    // uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            // bzero(st_serial_conf.tx_buff, 1024);
            // ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                case UART_DATA:
                    uart_read_bytes(EX_UART_NUM, st_serial_conf.tx_buff, event.size, portMAX_DELAY);
                    base_pos = event.size;
                    // ESP_LOGI("[uart event]", "rec %d", base_pos);
                    while (base_pos > 0){
                        temp =  send(st_serial_conf.sock, st_serial_conf.tx_buff, base_pos, 0);
                        if (temp <= 0){
                            ESP_LOGE(TAG, "(%4d)tcp(%d) send error %d", st_serial_conf.port, st_serial_conf.sock, errno);
                            break;
                        }
                        base_pos -= temp;
                    }
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:
                    ESP_LOGI(TAG, "pattern?");
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    // free(dtmp);
    // dtmp = NULL;
    vTaskDelete(NULL);
}

static void st_ota_tcp_rx_cb(int sock, char * cmd, uint16_t len){
    uart_event_t event;
    switch (cmd[0])
    {
    case 'E':{
        switch (cmd[1])
        {
        case 'B':
            SysState = SYSTEM_STATE_DOWNLOAD;
            uart_st_set_boot_mode(true);
            while (xQueueReceive(uart0_queue, (void * )&event, 0) == pdTRUE){
                if (event.type == UART_DATA){
                    uart_read_bytes(EX_UART_NUM, st_serial_conf.tx_buff, event.size, 0);
                }
            }
            break;
        
        case 'E':
            SysState = SYSTEM_STATE_NORMAL;
            uart_st_set_boot_mode(false);
            while (xQueueReceive(uart0_queue, (void * )&event, 0) == pdTRUE){
                if (event.type == UART_DATA){
                    uart_read_bytes(EX_UART_NUM, st_serial_conf.tx_buff, event.size, 0);
                }
            }
            break;
            
        
        default:
            break;
        }
    } break;
    
    default:
        // TODO : parse to serial
        break;
    }
}

static void st_serial_rx_cb(int sock, char * cmd, uint16_t len){
    // if (SysState == SYSTEM_STATE_NORMAL){
        uart_st_write(cmd, (size_t)len);
    // } else {
        // do not write in download mode
    // }
}

void function_after_get_ip(void){
    tcp_creat_server(&st_ota_conf);
    tcp_creat_server(&st_serial_conf);
    
}

void function_after_lost_ip(void){
    tcp_end_server(&st_ota_conf);
    tcp_end_server(&st_serial_conf);
}