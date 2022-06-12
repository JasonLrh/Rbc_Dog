#ifndef IMU_H
#define IMU_H

#include "Inc/icm20948_hal.h"

extern volatile float pitch, roll, yaw;
extern          float quat[4];

void imu_start(void);


#endif
