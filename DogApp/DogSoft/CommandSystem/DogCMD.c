#include "DogCMD.h"

#include "DogBody.h"

#include <stdio.h>
#include "usart.h"
#include "tim.h"

// static char __attribute__ ((section(".dma_data"))) dog_cmd_buff[256];

#define UART_BUFF_SIZE 256
extern osMessageQId qSerialCMDHandle;
static uint8_t __attribute__ ((section(".dma_data")))  uart_cmd_buff[2][UART_BUFF_SIZE];
uint8_t * uart_point_buff = uart_cmd_buff[0];

void dog_cmd_rx_callback(UART_HandleTypeDef * huart, uint16_t pos){

    // ! clean DCache to sync data space
    // SCB_CleanDCache_by_Addr((uint32_t *)uart_point_buff + pos, 1);
    // SCB_InvalidateDCache_by_Addr((uint32_t *)uart_point_buff, pos);
    uart_point_buff[pos] = '\0';
    SCB_CleanDCache_by_Addr((uint32_t *)(uart_point_buff + pos), 1);

    xQueueSendFromISR(qSerialCMDHandle,(void *)&uart_point_buff, NULL);

    // ! memory management
    if (uart_point_buff == uart_cmd_buff[0]){
        uart_point_buff = uart_cmd_buff[1];
    } else {
        uart_point_buff = uart_cmd_buff[0];
    }
    HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t *)uart_point_buff, UART_BUFF_SIZE);
}


extern void dogcmd_motors(const char * cmd);
extern void dogcmd_system(const char * cmd);
extern void dogcmd_logcfg(const char * cmd);

void SerialCmdTask(void const * argument)
{
  /* USER CODE BEGIN SerialCmdTask */
  char * dog_cmd_buff = NULL;
  ST_LOGI("Dog CMD start");
  dog_cmd_start(&huart8);
  /* Infinite loop */
  for(;;)
  {
    dog_cmd_buff = NULL;
    if (xQueueReceive(qSerialCMDHandle, &(dog_cmd_buff), portMAX_DELAY) == pdPASS) {
        uint16_t pos;
        SCB_InvalidateDCache();
        ST_LOGI("$ %s", dog_cmd_buff);
        for (pos = 0; pos < UART_BUFF_SIZE; pos ++){
            if (dog_cmd_buff[pos] == '\0'){
                break;
            }
        }
        if(pos > 0){
            switch (dog_cmd_buff[0])
            {
                case 'E':{
                    dog_body_force_stop();
                } break;


                case 'M':{
                    dogcmd_motors(dog_cmd_buff + 1);
                } break;
                
                case 'S':{
                    dogcmd_system(dog_cmd_buff + 1);
                } break;

                case 'L':{
                    dogcmd_logcfg(dog_cmd_buff + 1);
                } break;

                
                
                default:
                    ST_LOGE("cmd not recognized %s", dog_cmd_buff);
                    break;
            }
        }
    }
  }
  /* USER CODE END SerialCmdTask */
}


void dog_cmd_start(UART_HandleTypeDef * huart){
    assert_param( HAL_UART_RegisterRxEventCallback(huart, dog_cmd_rx_callback) == HAL_OK );
    assert_param( HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t *)uart_point_buff, UART_BUFF_SIZE) == HAL_OK );
}


