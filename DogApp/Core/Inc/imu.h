#ifndef IMU_H
#define IMU_H

#include "Inc/icm20948_hal.h"

// extern volatile float pitch, yaw, roll;

extern float quat[4];
extern float euler[3];

#define pitch euler[0]
#define yaw euler[1]
#define roll euler[2]




void imu_start(void);


#endif
