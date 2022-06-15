#include "DogCMD.h"

#include "DogBody.h"

#include <stdio.h>
#include "usart.h"
#include "tim.h"

#define CMD_MOTOR_MODE      0x01
#define CMD_RESET_MODE      0x02
#define CMD_ZERO_POSITION   0x03

// static char __attribute__ ((section(".dma_data"))) dog_cmd_buff[256];

static void config_motor(dog_motor_single_t * motor, const char * cmd){
    // motor = &(motors.raw[motor_id]);
    float * param;
    float v_p, v_v, v_kp, v_kd, v_t;
    int i_param;

    if (cmd[1] != '\0'){
        switch (cmd[1])
        {
            case 'G':{
                uart_printf("[M%d]\tp:%4.2f\tv:%4.2f\tt:%4.2f\n", motor->id, motor->p, motor->v, motor->t);
                break;
            } break;

            case 'S':{
                switch (cmd[2]){
                    case 'Z':{
                        param = &(motor->zeroPos_offset);
                    } break;
                    
                    default:{
                        uart_printf("Invalid motor set parameter\n");
                        return;
                    } break;
                }
                sscanf(cmd + 3, "%f", param);
                uart_printf("[M%d]\tzeroPosition:%4.2f\n", motor->id, *param);
            } break;
            
            case 'F':{
                switch (cmd[2]){
                    case 'M':{ // mode
                        switch (cmd[3]){
                            case 'Z':{
                                dog_motor_set_Mode(motor, CMD_ZERO_POSITION);
                            } break;

                            case 'R':{
                                dog_motor_set_Mode(motor, CMD_RESET_MODE);
                            } break;

                            case 'M':{
                                dog_motor_set_Mode(motor, CMD_MOTOR_MODE);
                            } break;

                            default:{
                                uart_printf("Invalid motor mode\n");
                                return;
                            } break;
                        }
                    } break;

                    case 'A':{
                        sscanf(cmd + 3, "%f", &v_v);
                        dog_motor_set_angle(motor, v_v * PI / 180.f);
                    } break;

                    case 'P':{
                        i_param = sscanf(cmd + 3, "%f%f%f%f%f", &v_p, &v_v, &v_kp, &v_kd, &v_t);
                        if (i_param == 5){
                            dog_motor_set_Control_param(motor, v_p, v_v, v_kp, v_kd, v_t);
                            uart_printf("[M%d]\tControl_param(%4.2f,%4.2f,%4.2f,%4.2f,%4.2f)\n", motor->id, v_p, v_v, v_kp, v_kd, v_t);
                        } else {
                            uart_printf("Invalid motor control param number:%d\n", i_param);
                            return;
                        }
                    } break;

                    default:{
                        uart_printf("Invalid motor function\n");
                        return;
                    } break;
                }
            } break;
        
            default: {
                uart_printf("motor cmd not recognized\n");
                return;
            } break;
        }
    } else {
        return;
    }
}

static void config_motors(const char * cmd){
    // id | set/get/function | 
    /*
    set :
        [Z] -> zeroPos_offset 

    get :
        no param. return p,v,t
    
    function :
        [M] -> mode
        [A] -> angle
        [P] -> full cmd 
    */
    int motor_id = 0;
    uint8_t all_flag = 0;
    

    if (cmd[0] != '\0'){
        if (cmd[0] == 'A'){
            all_flag = 1;
        } else {
            motor_id = cmd[0] - '1';
            if (motor_id < 0 || motor_id > 8){
                uart_printf("Invalid motor id\n");
                return;
            }
        }
    } else {
        return;
    }
    if (all_flag){
        for (motor_id = 0; motor_id < 8; motor_id++){
            config_motor(&(motors.raw[motor_id]), cmd);
        }
    } else {
        config_motor(&(motors.raw[motor_id]), cmd);
    }

}


extern FDCAN_HandleTypeDef hfdcan1;
static void config_system(const char * cmd){
    float kp, kv;
    if (cmd[0] != '\0'){
        switch (cmd[0]){
            case 'C':{
                // can
                uart_printf("can rx : %d\n", HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0));
            } break;

            case 'S':{
                if (HAL_TIM_Base_GetState(&htim6) == HAL_TIM_STATE_BUSY){
                    HAL_TIM_Base_Stop_IT(&htim6);
                }
                switch (cmd[1])
                {
                    case 'W':
                        HAL_TIM_Base_Start_IT(&htim6);
                        break;
                    
                    case 'U':
                        sscanf(cmd + 2, "%f,%f", &kp, &kv);
                        dog_body_standup(kp, kv);
                        break;
                    
                    case 'D':
                        dog_body_sitdown();
                        break;
                    
                    case 'S':
                        dog_body_standup(-1, -1); // tim has stoped , just standup here
                        break;
                
                default:
                    break;
                }

            }

            default:{

            } break;
        }
    }else{
        uart_printf("system cmd not recognized\n");
    }
}

#define UART_BUFF_SIZE 256
extern osMessageQId qSerialCMDHandle;
uint8_t * uart_point_buff = NULL;

void dog_cmd_rx_callback(UART_HandleTypeDef * huart, uint16_t pos){
    uart_point_buff[pos] = '\0';
    xQueueSendFromISR(qSerialCMDHandle,(void *)&uart_point_buff, NULL);
    // uart_printf("good\n");
    uart_point_buff = NULL;
    uart_point_buff = pvPortMalloc(UART_BUFF_SIZE);
    if (uart_point_buff == NULL){
        uart_printf("(E) malloc fail\n");
    }
    HAL_UARTEx_ReceiveToIdle_IT(huart, (uint8_t *)uart_point_buff, UART_BUFF_SIZE);
}

void SerialCmdTask(void const * argument)
{
  /* USER CODE BEGIN SerialCmdTask */
  /* Infinite loop */
  for(;;)
  {
    char * dog_cmd_buff = NULL;
    if (xQueueReceive(qSerialCMDHandle, &(dog_cmd_buff), portMAX_DELAY) == pdPASS) {
        uint16_t pos;
        for (pos = 0; pos < UART_BUFF_SIZE; pos ++){
            if (dog_cmd_buff[pos] == '\0'){
                break;
            }
        }
        if(pos > 0){
            // dog_cmd_buff[pos] = '\0';
            // if (dog_cmd_buff[pos - 1] != '\n'){
            //     return;
            // }
            uart_printf("[cmd] $ %s", dog_cmd_buff);
            switch (dog_cmd_buff[0])
            {
                case 'M':{
                    config_motors(dog_cmd_buff + 1);
                } break;

                case 'E':{
                    dog_body_force_stop();
                }
                
                case 'S':{
                    config_system(dog_cmd_buff + 1);
                }
                
                default:
                    uart_printf("cmd not recognized\n");
                    break;
            }
        }
        vPortFree(dog_cmd_buff);
    }
  }
  /* USER CODE END SerialCmdTask */
}


void dog_cmd_start(UART_HandleTypeDef * huart){
    assert_param( HAL_UART_RegisterRxEventCallback(huart, dog_cmd_rx_callback) == HAL_OK );
    uart_point_buff = pvPortMalloc(UART_BUFF_SIZE);
    if (uart_point_buff == NULL){
        uart_printf("(E) malloc fail\n");
    }
    assert_param( HAL_UARTEx_ReceiveToIdle_IT(huart, (uint8_t *)uart_point_buff, UART_BUFF_SIZE) == HAL_OK );
}


