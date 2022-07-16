#ifndef IMU_H
#define IMU_H

#include "Inc/icm20948_hal.h"

// extern volatile float pitch, yaw, roll;

extern float quat[4];

extern float euler[3];
/*
[0]:    top view; - : clock-direct 
[1]:    f-b side; + : go upper
[2]:    l-r side; + : clock-direct (b->f)

*/
#define pitch euler[0]
#define yaw euler[1]
#define roll euler[2]




void imu_start(void);


#endif
