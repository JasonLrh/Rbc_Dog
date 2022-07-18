#ifndef DOG_MOTOR_H
#define DOG_MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "arm_math.h"

#ifndef PI
#define PI					3.14159265358979f
#endif

#define CMD_MOTOR_MODE      0x01
#define CMD_RESET_MODE      0x02
#define CMD_ZERO_POSITION   0x03

typedef struct _s_motor_ctrl_t {
    uint8_t valid;
    arm_pid_instance_f32 P;
} s_motor_ctrl_t;

typedef struct _dog_motor_single_t {
    uint32_t id;
    FDCAN_HandleTypeDef * hcan;
    float zeroPos_offset;
    float p;
    float v;
    float t;
    int8_t invers;
    s_motor_ctrl_t ctrl;
    uint8_t monitor;
} dog_motor_single_t;

typedef union _dog_motor_group_t {
    dog_motor_single_t raw[8];
    struct {
        dog_motor_single_t l_f[2];
        dog_motor_single_t r_f[2];
        dog_motor_single_t r_b[2];
        dog_motor_single_t l_b[2];
    }leg;
    dog_motor_single_t leg_array[4][2];
} dog_motor_group_t;

typedef struct _dog_motor_output_t {
    float vel;
    float pos;
    float kv;
    float kp;
    float T;
} dog_motor_output_t;

typedef struct _dog_motor_angle_mode_config_t{
    float kp;
    float kv;
    float t;
} dog_motor_angle_mode_config_t;

extern dog_motor_group_t motors;
extern dog_motor_angle_mode_config_t angle_conf;

void dog_motor_init(void);
void dog_motor_set(dog_motor_single_t * mt, const dog_motor_output_t * input);
void dog_motor_set_Control_param(dog_motor_single_t * mt, float f_p, float f_v, float f_kp, float f_kd, float f_t);
void dog_motor_set_angle(dog_motor_single_t * mt, float angle);
void dog_motor_set_Mode(const dog_motor_single_t * mt, uint8_t cmd);

#ifdef __cplusplus
}
#endif

#endif