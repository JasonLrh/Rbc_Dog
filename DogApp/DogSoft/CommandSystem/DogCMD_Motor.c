#include "DogCMD.h"
#include "DogMotor.h"

const float reference_zero_angle[8] = {
    -3.66, -1.22, 3.66, 1.22, 1.22, 3.66, -1.22, -3.66
};

static void config_motor(dog_motor_single_t * motor, const char * cmd){
    // motor = &(motors.raw[motor_id]);
    float new_zeroPos;
    float v_p, v_v, v_kp, v_kd, v_t;
    uint8_t id;
    int i_param;

    if (cmd[1] != '\0'){
        switch (cmd[1])
        {
            case 'G':{
                if (cmd[2] != 'Z'){
                    uart_printf("[M%d]\tp:%4.2f\tv:%4.2f\tt:%4.2f\n", motor->id, motor->p, motor->v, motor->t);
                } else {
                    uart_printf("[M%d]\tzero_offset:%4.2f\n", motor->id, motor->zeroPos_offset);
                }
            } break;

            case 'S':{
                switch (cmd[2]){
                    case 'Z':{
                        id = cmd[0] - '1';
                        sscanf(cmd + 3, "%f", &new_zeroPos);
                        if ( fabsf(new_zeroPos - reference_zero_angle[id]) < 25 * PI / 180.f ){
                            motor->zeroPos_offset = new_zeroPos;
                            ST_LOGI("update new zero pos: %.2f", new_zeroPos);
                        } else {
                            ST_LOGE("please check input range: %.2f", new_zeroPos);
                        }

                        dog_body_standup(-1.f, -1.f);
                    } break;
                    
                    default:{
                        ST_LOGE("Invalid motor set parameter");
                        return;
                    } break;
                }
            } break;
            
            case 'F':{
                switch (cmd[2]){
                    case 'M':{ // mode
                        switch (cmd[3]){
                            case 'Z':{
                                dog_motor_set_Mode(motor, CMD_ZERO_POSITION);
                            } break;

                            case 'R':{
                                dog_motor_set_Mode(motor, CMD_RESET_MODE);
                            } break;

                            case 'M':{
                                dog_motor_set_Mode(motor, CMD_MOTOR_MODE);
                            } break;

                            default:{
                                ST_LOGE("Invalid motor mode");
                                return;
                            } break;
                        }
                    } break;

                    case 'A':{
                        sscanf(cmd + 3, "%f", &v_v);
                        dog_motor_set_angle(motor, v_v * PI / 180.f);
                    } break;

                    case 'P':{
                        i_param = sscanf(cmd + 3, "%f%f%f%f%f", &v_p, &v_v, &v_kp, &v_kd, &v_t);
                        if (i_param == 5){
                            dog_motor_set_Control_param(motor, v_p, v_v, v_kp, v_kd, v_t);
                            uart_printf("[M%d]\tControl_param(%4.2f,%4.2f,%4.2f,%4.2f,%4.2f)\n", motor->id, v_p, v_v, v_kp, v_kd, v_t);
                        } else {
                            ST_LOGE("Invalid motor control param number:%d", i_param);
                            return;
                        }
                    } break;

                    default:{
                        ST_LOGE("Invalid motor function");
                        return;
                    } break;
                }
            } break;
        
            default: {
                ST_LOGE("motor cmd not recognized");
                return;
            } break;
        }
    } else {
        return;
    }
}

void dogcmd_motors(const char * cmd){
    // id | set/get/function | 
    /*
    set :
        [Z] -> zeroPos_offset 

    get :
        no param. return p,v,t
    
    function :
        [M] -> mode
        [A] -> angle
        [P] -> full cmd 
    */
    int motor_id = 0;
    uint8_t all_flag = 0;
    

    if (cmd[0] != '\0'){
        if (cmd[0] == 'A'){
            all_flag = 1;
        } else {
            motor_id = cmd[0] - '1';
            if (motor_id < 0 || motor_id > 8){
                ST_LOGE("Invalid motor id");
                return;
            }
        }
    } else {
        ST_LOGE("Invalid motor number(1~8 | 'A')");
        return;
    }
    if (all_flag){
        if (cmd[1] != 'S'){
            for (motor_id = 0; motor_id < 8; motor_id++){
                config_motor(&(motors.raw[motor_id]), cmd);
            }
        } else {
            ST_LOGE("motor 'Set' not allow once for all operation");
        }
    } else {
        config_motor(&(motors.raw[motor_id]), cmd);
    }

}