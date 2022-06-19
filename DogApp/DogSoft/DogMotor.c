#include "DogMotor.h"
#include "fdcan.h"


#define CMD_MOTOR_MODE      0x01
#define CMD_RESET_MODE      0x02
#define CMD_ZERO_POSITION   0x03

#define P_MIN -95.5f    // Radians
#define P_MAX 95.5f        
#define V_MIN -45.0f    // Rad/s
#define V_MAX 45.0f
#define KP_MIN 0.0f     // N-m/rad
#define KP_MAX 500.0f
#define KD_MIN 0.0f     // N-m/rad/s
#define KD_MAX 5.0f
#define T_MIN -18.0f
#define T_MAX 18.0f

#define LIMIT_MIN_MAX(x,min,max) (x) = (((x)<=(min))?(min):(((x)>=(max))?(max):(x)))

const static uint16_t motor_group_id_t[8] = {
    1,2,3,4,5,6,7,8
};


dog_motor_group_t motors;


static uint16_t float_to_uint(float x, float x_min, float x_max, uint8_t bits);
static float uint_to_float(int x_int, float x_min, float x_max, int bits);
static void motor_fdcan_send_buff(const dog_motor_single_t * mt ,uint8_t * buff, uint32_t dlc);
// can_rx
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];


    while( HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0 ){
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data);
        switch ( rx_header.Identifier )
        {
            case 0x00:{
                for (int i = 0; i < 8; i++){
                    if (rx_data[0] == motors.raw[i].id){
                        motors.raw[i].p = (uint_to_float(( rx_data[1]          << 8 )|( rx_data[2]      ), P_MIN, P_MAX, 16) - motors.raw[i].zeroPos_offset) * motors.raw[i].invers;
                        motors.raw[i].v =  uint_to_float(( rx_data[3]          << 4 )|( rx_data[4] >> 4 ), V_MIN, V_MAX, 12)                                 * motors.raw[i].invers;
                        motors.raw[i].t =  uint_to_float(( (rx_data[4] & 0x0F) << 8 )|( rx_data[5]      ), T_MIN, T_MAX, 12)                                 * motors.raw[i].invers;
                        // uart_printf("[rev] %d\n", rx_data[0]);
                        motors.raw[i].ctrl.valid = 1;
                        break;
                    }
                }
            } break;
        
        default:
            break;
        }
    }

    // uart_printf("--------\n");

	
    assert_param(HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) == HAL_OK);
}


#define INITIALNUM 8

// **************************************************************************************************************
void dog_motor_init(void){
    motors.raw[0].invers = 1;
    motors.raw[1].invers = 1;
    motors.raw[2].invers = -1;
    motors.raw[3].invers = -1;
    motors.raw[4].invers = -1;
    motors.raw[5].invers = -1;
    motors.raw[6].invers = 1;
    motors.raw[7].invers = 1;

    // TODO: set initial angle of device
    motors.raw[0].zeroPos_offset = - 3.66;
    motors.raw[1].zeroPos_offset = - 1.22;

    motors.raw[2].zeroPos_offset = - motors.raw[0].zeroPos_offset; // 3.66
    motors.raw[3].zeroPos_offset = - motors.raw[1].zeroPos_offset; // 1.22

    motors.raw[4].zeroPos_offset = - motors.raw[1].zeroPos_offset;
    motors.raw[5].zeroPos_offset = - motors.raw[0].zeroPos_offset;

    motors.raw[6].zeroPos_offset = - motors.raw[4].zeroPos_offset;
    motors.raw[7].zeroPos_offset = - motors.raw[5].zeroPos_offset;

    for (int i = 0; i < INITIALNUM; i++) {
        motors.raw[i].id = motor_group_id_t[i];
        motors.raw[i].hcan = &hfdcan1;
        motors.raw[i].ctrl.valid = 0;
        arm_pid_init_f32(&(motors.raw[i].ctrl.P), 1);
        dog_motor_set_Mode(&(motors.raw[i]), CMD_MOTOR_MODE);
        osDelay(1);
        dog_motor_set_Control_param(&(motors.raw[i]), 0.f, 0.f, 0.f, 0.f, 0.f);
        osDelay(1);
        // TODO:
    }
    osDelay(200);
    for (int i = 0; i < INITIALNUM; i++) {
        dog_motor_set_Mode(&(motors.raw[i]), CMD_ZERO_POSITION);
        osDelay(1);
    }
    osDelay(200);
    for (int i = 0; i < INITIALNUM; i++) {
        dog_motor_set_Mode(&(motors.raw[i]), CMD_MOTOR_MODE);
        osDelay(1);
    }
    osDelay(200);

}

void dog_motor_set_Mode(const dog_motor_single_t * mt, uint8_t cmd){
    uint8_t buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
    switch(cmd)
    {
        case CMD_MOTOR_MODE:
            buf[7] = 0xFC;
            break;
        
        case CMD_RESET_MODE:
            buf[7] = 0xFD;
        break;
        
        case CMD_ZERO_POSITION:
            buf[7] = 0xFE;
        break;
        
        default:
        return; /* 直接退出函数 */
    }
    motor_fdcan_send_buff(mt, buf, FDCAN_DLC_BYTES_8);
}


void dog_motor_set_Control_param(dog_motor_single_t * mt, float f_p, float f_v, float f_kp, float f_kd, float f_t){
    static uint16_t p, v, kp, kd, t;
    static uint8_t buf[8];
    
    /* 限制输入的参数在定义的范围内 */
    f_p *= mt->invers;
    f_p += mt->zeroPos_offset;
    f_v *= mt->invers;
    f_t *= mt->invers;
    mt->ctrl.valid = 0;
    LIMIT_MIN_MAX(f_p,  P_MIN,  P_MAX);
    LIMIT_MIN_MAX(f_v,  V_MIN,  V_MAX);
    LIMIT_MIN_MAX(f_kp, KP_MIN, KP_MAX);
    LIMIT_MIN_MAX(f_kd, KD_MIN, KD_MAX);
    LIMIT_MIN_MAX(f_t,  T_MIN,  T_MAX);
    
    /* 根据协议，对float参数进行转换 */
    p = float_to_uint(f_p,      P_MIN,  P_MAX,  16);            
    v = float_to_uint(f_v,      V_MIN,  V_MAX,  12);
    kp = float_to_uint(f_kp,    KP_MIN, KP_MAX, 12);
    kd = float_to_uint(f_kd,    KD_MIN, KD_MAX, 12);
    t = float_to_uint(f_t,      T_MIN,  T_MAX,  12);
    
    /* 根据传输协议，把数据转换为CAN命令数据字段 */
    buf[0] = p>>8;
    buf[1] = p&0xFF;
    buf[2] = v>>4;
    buf[3] = ((v&0xF)<<4)|(kp>>8);
    buf[4] = kp&0xFF;
    buf[5] = kd>>4;
    buf[6] = ((kd&0xF)<<4)|(t>>8);
    buf[7] = t&0xff;

    motor_fdcan_send_buff(mt, buf, FDCAN_DLC_BYTES_8);
}

dog_motor_angle_mode_config_t angle_conf = {
    .kp = 9.f,
    .kv = 1.2f,
    .t = 0.f
};

void dog_motor_set_angle(dog_motor_single_t * mt, float angle){
    dog_motor_set_Control_param(mt, angle, 0.f, angle_conf.kp, angle_conf.kv, angle_conf.t);
}











// ************************static tool function

static uint16_t float_to_uint(float x, float x_min, float x_max, uint8_t bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    
    return (uint16_t) ((x-offset)*((float)((1<<bits)-1))/span);
}

static float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}


static void motor_fdcan_send_buff(const dog_motor_single_t * mt ,uint8_t * buff, uint32_t dlc){
    FDCAN_TxHeaderTypeDef fdcan_tx_header;

// debug
switch (mt->id){
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        break;
    
    default:
        return;
}
    fdcan_tx_header.Identifier = mt->id;
    fdcan_tx_header.IdType = FDCAN_STANDARD_ID;
	fdcan_tx_header.TxFrameType = FDCAN_DATA_FRAME;
	fdcan_tx_header.DataLength = dlc;
	fdcan_tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	fdcan_tx_header.BitRateSwitch = FDCAN_BRS_OFF;
	fdcan_tx_header.FDFormat = FDCAN_CLASSIC_CAN;
	fdcan_tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	fdcan_tx_header.MessageMarker = 0;
    while (HAL_FDCAN_GetTxFifoFreeLevel(mt->hcan) == 0){
        __NOP();
    }
    assert_param( HAL_FDCAN_AddMessageToTxFifoQ(mt->hcan, &fdcan_tx_header, buff) == HAL_OK);
    // while (HAL_FDCAN_IsTxBufferMessagePending(mt->hcan, FDCAN_TX_BUFFER0) != 0){
    //     __NOP();
    // }
    // assert_param(HAL_FDCAN_AddMessageToTxBuffer(mt->hcan, &fdcan_tx_header, buff, FDCAN_TX_BUFFER0) == HAL_OK);
    // assert_param(HAL_FDCAN_EnableTxBufferRequest(mt->hcan, FDCAN_TX_BUFFER0) == HAL_OK);
}