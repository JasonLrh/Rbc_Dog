#include "main.h"
#include "Inc/icm20948_hal.h"

#include <stdio.h>

#define INV_I2C_API_Handler hi2c1
// #define INV_UART_MSG_API_Handler huart8

extern I2C_HandleTypeDef INV_I2C_API_Handler;
// extern UART_HandleTypeDef INV_UART_MSG_API_Handler;

extern uint8_t I2C_Address;

int idd_io_hal_read_reg(void * context, uint8_t reg, uint8_t * rbuffer, uint32_t rlen){
	(void)context;
#if SERIF_TYPE_SPI
	return spi_master_transfer_rx(NULL, reg, rbuffer, rlen);
#else /* SERIF_TYPE_I2C */
  HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(&INV_I2C_API_Handler, I2C_Address << 1, reg, I2C_MEMADD_SIZE_8BIT, rbuffer, rlen, HAL_MAX_DELAY);
  assert_param(ret == HAL_OK);
  uint8_t code;
  switch (ret)
  {
  case HAL_OK:
    code = 0;
    break;
  
  case HAL_BUSY:
    code = 8;
    break;

  case HAL_TIMEOUT:
    code = 9;
    break;
  
  default:
    code = 10;
    break;
  }
	return code;
#endif	
}

int idd_io_hal_write_reg(void * context, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen){
	(void)context;
#if SERIF_TYPE_SPI
	return spi_master_transfer_tx(NULL, reg, wbuffer, wlen);
#else /* SERIF_TYPE_I2C */
	HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(&INV_I2C_API_Handler, I2C_Address << 1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)wbuffer, wlen, HAL_MAX_DELAY);
  assert_param(ret == HAL_OK);
  uint8_t code;
  switch (ret)
  {
  case HAL_OK:
    code = 0;
    break;
  
  case HAL_BUSY:
    code = 8;
    break;

  case HAL_TIMEOUT:
    code = 9;
    break;
  
  default:
    code = 10;
    break;
  }
	return code;
#endif	
}

#ifdef INV_MSG_ENABLE
/*
* Printer function for message facility
*/

void msg_printer(int level, const char * str, va_list ap){

    static char out_str[256]; /* static to limit stack usage */
    static char * color_str;
    // unsigned idx = 0;
    // const char * ptr = out_str;
    // const char * s[INV_MSG_LEVEL_MAX] = {
    //     "",    // INV_MSG_LEVEL_OFF
    //     "[E] ", // INV_MSG_LEVEL_ERROR
    //     "[W] ", // INV_MSG_LEVEL_WARNING
    //     "[I] ", // INV_MSG_LEVEL_INFO
    //     "[V] ", // INV_MSG_LEVEL_VERBOSE
    //     "[D] ", // INV_MSG_LEVEL_DEBUG
    // };
    // idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "%s", s[level]);
    // if(idx >= (sizeof(out_str)))
    //     return;
    // idx += vsnprintf(&out_str[idx], sizeof(out_str) - idx, str, ap);
    // if(idx >= (sizeof(out_str)))
    //     return;
    // idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "\r\n");
    // if(idx >= (sizeof(out_str)))
    //     return;

    vsprintf(out_str, str, ap);
      
    switch (level)
    {
    case INV_MSG_LEVEL_ERROR:
      color_str = LOG_COLOR(LOG_COLOR_RED);
      break;
    case INV_MSG_LEVEL_WARNING:
      color_str = LOG_COLOR(LOG_COLOR_BROWN);
      break;
    case INV_MSG_LEVEL_INFO:
      color_str = LOG_COLOR(LOG_COLOR_GREEN);
      break;
    case INV_MSG_LEVEL_VERBOSE:
      color_str = LOG_COLOR(LOG_COLOR_CYAN);
      break;
    case INV_MSG_LEVEL_DEBUG:
      color_str = LOG_COLOR(LOG_COLOR_BLUE);
      break;
    default:
      break;
    }
    uart_printf("[%simu\033[0m] %s\n", color_str, out_str);
    // HAL_UART_Transmit(&INV_UART_MSG_API_Handler, (uint8_t*)ptr, idx, HAL_MAX_DELAY);
}
#endif

enum inv_icm20948_sensor idd_sensortype_conversion(int sensor){
	switch(sensor) {
	case INV_SENSOR_TYPE_RAW_ACCELEROMETER:       return INV_ICM20948_SENSOR_RAW_ACCELEROMETER;
	case INV_SENSOR_TYPE_RAW_GYROSCOPE:           return INV_ICM20948_SENSOR_RAW_GYROSCOPE;
	case INV_SENSOR_TYPE_ACCELEROMETER:           return INV_ICM20948_SENSOR_ACCELEROMETER;
	case INV_SENSOR_TYPE_GYROSCOPE:               return INV_ICM20948_SENSOR_GYROSCOPE;
	case INV_SENSOR_TYPE_UNCAL_MAGNETOMETER:      return INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED;
	case INV_SENSOR_TYPE_UNCAL_GYROSCOPE:         return INV_ICM20948_SENSOR_GYROSCOPE_UNCALIBRATED;
	case INV_SENSOR_TYPE_BAC:                     return INV_ICM20948_SENSOR_ACTIVITY_CLASSIFICATON;
	case INV_SENSOR_TYPE_STEP_DETECTOR:           return INV_ICM20948_SENSOR_STEP_DETECTOR;
	case INV_SENSOR_TYPE_STEP_COUNTER:            return INV_ICM20948_SENSOR_STEP_COUNTER;
	case INV_SENSOR_TYPE_GAME_ROTATION_VECTOR:    return INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_ROTATION_VECTOR:         return INV_ICM20948_SENSOR_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_GEOMAG_ROTATION_VECTOR:  return INV_ICM20948_SENSOR_GEOMAGNETIC_ROTATION_VECTOR;
	case INV_SENSOR_TYPE_MAGNETOMETER:            return INV_ICM20948_SENSOR_GEOMAGNETIC_FIELD;
	case INV_SENSOR_TYPE_SMD:                     return INV_ICM20948_SENSOR_WAKEUP_SIGNIFICANT_MOTION;
	case INV_SENSOR_TYPE_PICK_UP_GESTURE:         return INV_ICM20948_SENSOR_FLIP_PICKUP;
	case INV_SENSOR_TYPE_TILT_DETECTOR:           return INV_ICM20948_SENSOR_WAKEUP_TILT_DETECTOR;
	case INV_SENSOR_TYPE_GRAVITY:                 return INV_ICM20948_SENSOR_GRAVITY;
	case INV_SENSOR_TYPE_LINEAR_ACCELERATION:     return INV_ICM20948_SENSOR_LINEAR_ACCELERATION;
	case INV_SENSOR_TYPE_ORIENTATION:             return INV_ICM20948_SENSOR_ORIENTATION;
	case INV_SENSOR_TYPE_B2S:                     return INV_ICM20948_SENSOR_B2S;
	default:                                      return INV_ICM20948_SENSOR_MAX;
	}
}


void inv_sys_initial_dmp(void){
    int rc = 0;
    // INV_MSG setup
    inv_msg_setup(INV_MSG_LEVEL_MAX, msg_printer);
    // api connect
    struct inv_icm20948_serif icm20948_serif;
    icm20948_serif.context   = 0; /* no need */
    icm20948_serif.read_reg  = idd_io_hal_read_reg;
    icm20948_serif.write_reg = idd_io_hal_write_reg;
    icm20948_serif.max_read  = 1024*16; /* maximum number of bytes allowed per serial read */
    icm20948_serif.max_write = 1024*16; /* maximum number of bytes allowed per serial write */
    icm20948_serif.is_spi = interface_is_SPI();
    // api begin
    inv_icm20948_reset_states(&icm_device, &icm20948_serif);
    inv_icm20948_register_aux_compass(&icm_device, INV_ICM20948_COMPASS_ID_AK09916, AK0991x_DEFAULT_I2C_ADDR);
    // disable all sensor amd reset
    int i_sensor = INV_SENSOR_TYPE_MAX;
    while(i_sensor-- > 0) {
        rc = inv_icm20948_enable_sensor(&icm_device, idd_sensortype_conversion(i_sensor), 0);
    }
    rc += inv_icm20948_soft_reset(&icm_device);

    // initial sensor
    rc = icm20948_sensor_setup();
    // icm20948_run_selftest();

    // load DMP
    rc += load_dmp3();
    check_rc(rc, "Error sensor_setup/DMP loading.");
}

void chip_reset(void){
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}