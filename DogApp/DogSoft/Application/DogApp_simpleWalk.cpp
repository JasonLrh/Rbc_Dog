#include "DogSoft.h"
#include "DogApp.h"
#include <math.h>
#include "arm_math.h"
#include "imu.h"

#define BANDWIDTH 0.75f
#define HEIGH_DIFF 0.00f
// #define PARAM_A 9.0

// #define H 0.38f
// #define V 0.54f

#define INITIAL_PERIOD_CNT 220

class singleLeg {
public:
    singleLeg(dog_motor_single_t * leg, float _phase_diff){
        phrase_diff = _phase_diff;
        m = leg;
    }

    void simpleWalk_generator(float phrase, float height, float vel, float angle_offset){
        float theta, dist, dig_angle_half, d;

        phrase += phrase_diff;
        while (phrase > 1.0){ // to 0~1
            phrase -= 1.0;
        } 

        for (int i = 0; i<2;i++){
            lo.m[i].vel = 0;
            lo.m[i].T = 0.f;

            lo.m[i].kp = 30.3f;
            lo.m[i].kv = 1.8f;
            // if (phrase < BANDWIDTH){
            //     lo.m[i].kp = 30.3f;
            //     lo.m[i].kv = 1.8f;
            // } else {
            //     lo.m[i].kp = 12.3f;
            //     lo.m[i].kv = 0.1f;
            // }
        }
        // height += H;

        // vel += V;

        if (phrase < BANDWIDTH){
            theta = atanf(vel/height * (phrase - BANDWIDTH / 2)) + angle_offset; // TODO : add some offset here
            dist = (height - HEIGH_DIFF * sinf(phrase * 2 * PI)) / cosf(theta);
        } else {
            float half_line_len = vel * (LEG_LEN_UPPER + LEG_LEN_LOWER) * BANDWIDTH / 2 ;
            float h_to_line = height * (LEG_LEN_UPPER + LEG_LEN_LOWER);

            float tuo_dist = sqrtf( 1 - (1 / tuo_a) * (1 / tuo_a) ) * tuo_b * half_line_len;
            float tuo_off = - h_to_line + tuo_dist;

            float edge_angle = atan2f(tuo_dist, half_line_len);
            

            float tuo_time = - edge_angle + (phrase - BANDWIDTH) * (PI + 2 * edge_angle) / (1 - BANDWIDTH);

            float x = tuo_a * half_line_len * cosf(tuo_time);
            float y = tuo_b * half_line_len * sin(tuo_time) + tuo_off;

            dist = sqrtf(x*x + y*y) / (LEG_LEN_LOWER + LEG_LEN_UPPER);

            theta = atan2f(y, x) + PI / 2;

            // theta = (1 + BANDWIDTH - 2 * phrase) * atanf(vel/height * (BANDWIDTH / 2)) / (1.f - BANDWIDTH);

            // float mid = (1 + BANDWIDTH) / 2;
            // float new_phrase = phrase - mid;
            // float t = (height + HEIGH_DIFF) - PARAM_A * (BANDWIDTH - 1) * (BANDWIDTH - 1) / 4 ;
            // float y = PARAM_A * new_phrase * new_phrase + t;

            // dist = ( y ) / cosf(theta);
        }
        // d = LEG_LEN_LOWER - LEG_LEN_UPPER + 2 * dist * LEG_LEN_UPPER;
        d = (LEG_LEN_LOWER + LEG_LEN_UPPER) * dist;
        dig_angle_half = acosf((LEG_LEN_UPPER*LEG_LEN_UPPER - LEG_LEN_LOWER*LEG_LEN_LOWER + d*d)/(2.f * d * LEG_LEN_UPPER)); // f
        lo.m[0].pos =  - theta + dig_angle_half;
        lo.m[1].pos =    theta + dig_angle_half;
    }

    void output(void){
        dog_leg_set(m, &lo);
    }

private:
    dog_leg_output_t lo;
    float phrase_diff;
    dog_motor_single_t * m;

    float tuo_a = 5.f / 4.f;
    // float tuo_a = 1.6f;
    float tuo_b = 2.f / 5.f;
    // float tuo_b = 5.f / 4.f;
    // float tuo_b = 2.5f;
};

extern volatile int32_t lr_cnt;

#define MAX_YANG_ANGLE 15.0 * PI / 180.f

class dogBoady{
public:
    dogBoady(float _yaw){
        target_yaw = _yaw;
        target_pitch = pitch;
        arm_pid_init_f32(&pidPitch, 1);
        arm_pid_init_f32(&pidYaw  , 1);
    };

    void change_rate(uint32_t _period_cnt){
        if (_period_cnt < 150){
            ST_LOGE("to slow");
            return;
        }
        pidPitch.Ki = 1 / (float)_period_cnt;
        pidYaw.Ki = 1 / (float)_period_cnt;

        arm_pid_init_f32(&pidPitch, 1);
        arm_pid_init_f32(&pidYaw  , 1);

        PERIOD_CNT = _period_cnt;
    }


    void update_walk(void){ // liner
        if (is_init == 0){
            is_init = 1;
            target_yaw = yaw;
        }

        float phrase = __step++ / float(PERIOD_CNT);
        __step = __step >= PERIOD_CNT ? 0 : __step;
        // float diff_v = target_yaw - yaw; // < 0 : clock->side
        // float diff_v = update_lr();
        float diff_v = lr_cnt * 0.05;
        float diff_h = 0.0; // > 0, up
        // float ang_off = 0.f;
        float ang_off = euler[2];
        if (ang_off > MAX_YANG_ANGLE) {
            ang_off = MAX_YANG_ANGLE;
        } else if (ang_off < - MAX_YANG_ANGLE) {
            ang_off = - MAX_YANG_ANGLE;
        }
        m[0].simpleWalk_generator(phrase, base_h + diff_h, base_v - diff_v, ang_off);// l
        m[1].simpleWalk_generator(phrase, base_h + diff_h, base_v + diff_v, ang_off);
        m[2].simpleWalk_generator(phrase, base_h - diff_h, base_v - diff_v, ang_off);// l
        m[3].simpleWalk_generator(phrase, base_h - diff_h, base_v + diff_v, ang_off);
        for (int i = 0; i < 4; i++){
            m[i].output();
        }
    }

    float target_pitch = 0.f;
    float target_yaw = 0.f;

private:
    uint32_t __step = 0;
    uint8_t is_init = 0;
    uint32_t PERIOD_CNT = INITIAL_PERIOD_CNT;

    float base_h = 0.56f;
    float base_v = 0.5f;

    arm_pid_instance_f32 pidPitch = {
        .Kp = 0.0,
        .Ki = 1.0 / (float)INITIAL_PERIOD_CNT,
        .Kd = 0.0
    };
    arm_pid_instance_f32 pidYaw = {
        .Kp = LEGS_DIST_LR * 38.f / 2.f,
        .Ki = 1.f / (float)INITIAL_PERIOD_CNT,
        // .Ki = 0.f,
        .Kd = 0.f
    };

    singleLeg m[4]{
        // singleLeg(motors.leg.l_f, 0.125 * 1),
        // singleLeg(motors.leg.r_f, 0.125 * 3),
        // singleLeg(motors.leg.l_b, 0.125 * 5),
        // singleLeg(motors.leg.r_b, 0.125 * 7)

        // ! OK 对角线
        singleLeg(motors.leg.l_f, 0.125 * 1),
        singleLeg(motors.leg.r_f, 0.125 * 5),
        singleLeg(motors.leg.l_b, 0.125 * 5),
        singleLeg(motors.leg.r_b, 0.125 * 1)
    };

    void update_target(void){
        // float o1 = arm_pid_f32(&pidPitch, target_pitch - pitch); // route
        // float o2 = arm_pid_f32(&pidYaw  , target_yaw   - yaw  ); // lr -> diff_v
    }

    float update_lr(void){
        return arm_pid_f32(&pidYaw  , target_yaw   - yaw  ); // lr -> diff_v
    }
};


dogBoady body(0.f);
void dogapp_simpleWalk(void){
    body.update_walk();
}