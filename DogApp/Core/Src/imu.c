#include "imu.h"

#include "main.h"
#include "usart.h"

#define IMU_PERIOD 1 // ms

extern enum inv_icm20948_sensor idd_sensortype_conversion(int sensor);

float quat[4];
volatile float pitch, roll, yaw;

uint8_t inv_state = 0;

uint64_t inv_icm20948_get_time_us(void){
	return TIM7->CNT + HAL_GetTick() * 1000;
}

int InvEMDFrontEnd_putcharHook(int c){
  HAL_UART_Transmit(&huart1, (uint8_t*)&c, 1, HAL_MAX_DELAY);
	return c;
}

static void quat2euler(void){
    roll = atan2f(quat[0]*quat[1] + quat[2]*quat[3], 0.5f - quat[1]*quat[1] - quat[2]*quat[2]);
    pitch = asinf(-2.0f * (quat[1]*quat[3] - quat[0]*quat[2]));
    yaw = atan2f(quat[1]*quat[2] + quat[0]*quat[3], 0.5f - quat[2]*quat[2] - quat[3]*quat[3]);
}

// static void quat2cal(void){
    
// }

void icm_data_process(inv_sensor_event_t * event){
  if (event->sensor == INV_SENSOR_TYPE_ROTATION_VECTOR) {
    memcpy(quat, event->data.quaternion.quat, sizeof(float) * 4);
    quat2euler();
    // uart_printf("Ap:%.3f,y:%.3f,r:%.3f\n",pitch, yaw, roll);
  }
};



void imu_start(void){
  HAL_GPIO_WritePin(CS_ICM_GPIO_Port, CS_ICM_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(CS_BMP_GPIO_Port, CS_BMP_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SDO_SAO_GPIO_Port, SDO_SAO_Pin, GPIO_PIN_RESET);
  inv_sys_initial_dmp();
  // int rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_ORIENTATION), 1);
  int rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_ROTATION_VECTOR), 1);
  check_rc(rc, "err enable INV_SENSOR_TYPE_ROTATION_VECTOR");
  rc = inv_icm20948_set_sensor_period(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_ROTATION_VECTOR), IMU_PERIOD);
  check_rc(rc, "err rate   INV_SENSOR_TYPE_ROTATION_VECTOR");

  // rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_ACCELEROMETER), 1);
  // check_rc(rc, "err enable INV_SENSOR_TYPE_ACCELEROMETER");
  // rc = inv_icm20948_set_sensor_period(&icm_device, idd_sensortype_conversion(INV_SENSOR_TYPE_ACCELEROMETER), PERIOD);
  // check_rc(rc, "err rate   INV_SENSOR_TYPE_ACCELEROMETER");
  inv_state = 1;
}