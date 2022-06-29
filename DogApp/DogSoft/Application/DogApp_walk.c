#include "DogApp.h"

// #define __H 0.42f
#define __H 0.5f
#define __HEIGHT __H *(LEG_LEN_UPPER + LEG_LEN_LOWER)

#define __VEL 0.12f

// time ctrl
#define PERIOD_CNT 880
#define BANDWIDTH 0.79f

// curf ctrl
#define HEIGH_DIFF 0.04f
#define PARAM_A 7.0

typedef struct _dog_work_target_t
{
    float vel;
    float height;
    float baud_of_walk;
    uint16_t period_cnt; // 0~1
    dog_leg_output_t __start_param;
    dog_leg_output_t __end_param;
} dog_work_target_t;

dog_work_target_t walk_target = {
    .vel = __VEL,
    .height = __HEIGHT,
    .period_cnt = PERIOD_CNT,
    .baud_of_walk = BANDWIDTH
};

void step_generator(uint16_t step, dog_leg_output_t *out)
{
    float t = (step - walk_target.period_cnt * walk_target.baud_of_walk / 2) * DOG_CTRL_PERIOD_ms / 1000.f;
    float a, a_, b, b_;
    if (step < walk_target.period_cnt * walk_target.baud_of_walk)
    {
        //  float n1 = t * walk_target.vel / walk_target.height;
         float n2 = (t * walk_target.vel) * (t * walk_target.vel) + walk_target.height * walk_target.height;
         float n3 = walk_target.height - sqrtf(n2);
         float n4 = LEG_LEN_UPPER * LEG_LEN_UPPER - LEG_LEN_LOWER * LEG_LEN_LOWER + n2;
         float n5 = t * walk_target.vel * walk_target.vel;

        a = -2 * atan2f(walk_target.height - sqrtf(n2), t * walk_target.vel);
        if (a > PI)
        {
            a -= PI * 2;
        }

        b = acosf(n4 / (2 * LEG_LEN_UPPER * sqrtf(n2)));

        if (t != 0)
        {
            a_ = 2 * (walk_target.vel / sqrtf(n2) + n3 / (t * t * walk_target.vel)) / (1 + n3 * n3 / (t * n5));
            b_ = n5 * (n4 / (2 * n2) - 1) / sqrtf(LEG_LEN_UPPER * LEG_LEN_UPPER * n2 - n4 * n4 / 4);
        }
        else
        {
            a_ = b_ = 0;
        }

        out->m[0].kp = 13.f;
        out->m[1].kp = 13.f;
        out->m[0].kv = 2.f;
        out->m[1].kv = 2.f;
        out->m[0].T  = 0.f;
        out->m[1].T  = 0.f;
    }
    else
    {
        // TODO
        // a = b = 0;
        float m0, m1, k;

        k = walk_target.__start_param.m[0].pos - walk_target.__end_param.m[0].pos;
        k /=  (float)(1 - walk_target.baud_of_walk) * walk_target.period_cnt; 
        m0 = walk_target.__end_param.m[0].pos + k * (step - walk_target.baud_of_walk * walk_target.period_cnt);

        k = walk_target.__start_param.m[1].pos - walk_target.__end_param.m[1].pos;
        k /=  (float)(1 - walk_target.baud_of_walk) * walk_target.period_cnt;
        m1 = walk_target.__end_param.m[1].pos + k * (step - walk_target.baud_of_walk * walk_target.period_cnt);

        a = (m1 - m0)/2;
        b = (m1 + m0)/2;


        a_ = b_ = 0;

        out->m[0].kp = 8.f;
        out->m[1].kp = 8.f;
        out->m[0].kv = 1.f;
        out->m[1].kv = 1.f;
        out->m[0].T  = 0.f;
        out->m[1].T  = 0.f;
    }

    out->m[0].pos = -a + b;
    out->m[1].pos = a + b;

    out->m[0].vel = -a_ + b_;
    out->m[1].vel = a_ + b_;

    uart_printf("%.2f\t%.2f\t%.2f\n", t , out->m[0].pos * 180 / PI, out->m[1].pos * 180 / PI );

    // const float time_scale = 100.f;
    // fprintf(fp, "%.3f,%.3f,%.3f,%.3f,%.3f\n", t * time_scale + 16.5, (a + b) * 180 / PI, (a - b) * 180 / PI, (a_ + b_) * 180 / PI / time_scale, (a_ - b_) * 180 / PI / time_scale);
    // fprintf(fp, "%.3f,%.3f,%.3f,%.3f,%.3f\n", t * 100 + 16.5, a * 30, b*30, a_, b_);
}

void step_param_update(float vel, float height, float baud_of_walk, int period_cnt)
{
    if (vel > 0.f)
        walk_target.vel = vel;
    if (height > 0.f)
        walk_target.height = height;
    if (baud_of_walk > 0.f && baud_of_walk < 1.f)
        walk_target.baud_of_walk = baud_of_walk;
    if (period_cnt > 30)
    {
        walk_target.period_cnt = period_cnt;
    }
    // dog_leg_output_t start, end;
    step_generator(0, &(walk_target.__start_param));
    step_generator((uint16_t)(walk_target.period_cnt * walk_target.baud_of_walk), &(walk_target.__end_param));
    printf("line angle %.2f\n", (walk_target.__start_param.m[0].pos - walk_target.__start_param.m[1].pos) * 180 / PI);
    printf("m0 (%.3f, %.3f)\n", walk_target.__start_param.m[0].pos * 180 / PI , walk_target.__end_param.m[0].pos * 180 / PI);
    printf("m1 (%.3f, %.3f)\n", walk_target.__start_param.m[1].pos * 180 / PI , walk_target.__end_param.m[1].pos * 180 / PI);
    // update curve condition
}

void dogapp_walk(void){
    static uint32_t step = 0;
    // static float yaw_intr = 0.f;
    // static float ccg_yaw = 0.f;
    // - >
    // + <

    dog_leg_output_t out[4];
    float m = walk_target.period_cnt / 4.f;
    step_generator(step + m * 0, out + 0);
    // step_generator(step + m * 1, out + 1);
    // step_generator(step + m * 2, out + 2);
    // step_generator(step + m * 3, out + 3);

    dog_leg_set(motors.leg.r_f, &out[0]);
    // dog_leg_set(motors.leg.l_f, &out[1]);
    // dog_leg_set(motors.leg.l_b, &out[2]);
    // dog_leg_set(motors.leg.r_b, &out[3]);


    /*
        抬脑袋 ：+
        低头   ：-
    */
    // float tgt_yaw = arm_pid_f32(&pid_yaw, target_yaw - yaw); // TODO : check direction
    // float tgt_yaw = 0.f; // TODO : check direction
    /*
        左转   ：+
        右转   ：-
    */


    // dog_leg_set_phrase(motors.leg.l_b, &vec[0]);
    // dog_leg_set_phrase(motors.leg.l_f, &vec[1]);
    // dog_leg_set_phrase(motors.leg.r_b, &vec[2]);
    // dog_leg_set_phrase(motors.leg.r_f, &vec[3]);

    step ++;
    if (step >= walk_target.period_cnt){
        step = 0;
    }
}