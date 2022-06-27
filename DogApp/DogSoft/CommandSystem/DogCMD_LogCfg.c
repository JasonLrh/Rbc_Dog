#include "DogCMD.h"

extern osMessageQId qSerialLogTimeHandle;
void dogcmd_logcfg(const char * cmd){
    int Time;
    int index = cmd[2] - '1';
    if (cmd[1] == 'M'){
        if (index < 0 || index > 7){
            ST_LOGE("out-of range");
            return;
        }
    }

    switch (cmd[0])
    {
    case 'O':{ // open
        switch (cmd[1])
        {
        case 'M': // motor
            motors.raw[index].monitor = 1;
            break;
        
        case 'I': //imu
            break;
        
        default:
            ST_LOGE("Invalid Device type");
            break;
        }
    } break;
    
    case 'C':{ // close
        switch (cmd[1])
        {
        case 'M': // motor
            motors.raw[index].monitor = 0;
            break;
        
        case 'I': //imu
            break;
        
        default:
            ST_LOGE("Invalid Device type");
            break;
        }
    } break;

    case 'T':{ // time set; 0:off
        if (sscanf(cmd + 1 ,"%d", &Time) == 1){
            xQueueSendFromISR(qSerialLogTimeHandle, &Time, NULL);
        }
    } break;

    
    
    default:{
        ST_LOGE("Not valid cmd");
    } break;
    }
}