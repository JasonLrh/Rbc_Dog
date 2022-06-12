#ifndef DOG_MOTOR_H
#define DOG_MOTOR_H

#include "main.h"

#ifndef PI
#define PI					3.14159265358979f
#endif

typedef struct _dog_motor_single_t {
    uint32_t id;
    FDCAN_HandleTypeDef * hcan;
    float zeroPos_offset;
    float p;
    float v;
    float t;
    int8_t invers;
} dog_motor_single_t;

typedef union _dog_motor_group_t{
    dog_motor_single_t raw[8];
    struct {
        dog_motor_single_t l_f[2];
        dog_motor_single_t r_f[2];
        dog_motor_single_t r_b[2];
        dog_motor_single_t l_b[2];
    }leg;
    dog_motor_single_t leg_array[4][2];
}dog_motor_group_t;

typedef struct _dog_motor_angle_mode_config_t{
    float kp;
    float kv;
    float t;
} dog_motor_angle_mode_config_t;

extern dog_motor_group_t motors;
extern dog_motor_angle_mode_config_t angle_conf;

void dog_motor_init(void);
void dog_motor_set_Control_param(const dog_motor_single_t * mt, float f_p, float f_v, float f_kp, float f_kd, float f_t);
void dog_motor_set_angle(const dog_motor_single_t * mt, float angle);
void dog_motor_set_Mode(const dog_motor_single_t * mt, uint8_t cmd);


#endif