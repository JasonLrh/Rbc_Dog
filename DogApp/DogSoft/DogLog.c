#include "DogSoft.h"
// cJson
#include "cJSON.h"
#include "imu.h"

extern void uart_dump(const char * content, uint16_t len);

static char __attribute__ ((section(".dma_data"))) output_str[1024];
static void send_state_pack(void){
    cJSON * root = cJSON_CreateObject();
    // structure job 
    // TODO: setup here
    {
      // submodule
      cJSON * motor = cJSON_CreateArray();
      for (uint8_t i = 0; i<8; i++){
        if (motors.raw[i].monitor == 1){
          cJSON * m = cJSON_CreateObject();
          cJSON * id = cJSON_CreateNumber(motors.raw[i].id);
          cJSON * p = cJSON_CreateNumber(motors.raw[i].p);
          cJSON * v = cJSON_CreateNumber(motors.raw[i].v);
          cJSON * t = cJSON_CreateNumber(motors.raw[i].t);
          cJSON_AddItemToObject(m, "id", id);
          cJSON_AddItemToObject(m, "p" , p );
          cJSON_AddItemToObject(m, "v" , v );
          cJSON_AddItemToObject(m, "t" , t );
          cJSON_AddItemToArray(motor, m);
        }
      }
      cJSON_AddItemToObject(root, "motor", motor);

      cJSON * imu   = cJSON_CreateObject();
      cJSON * q  = cJSON_CreateFloatArray(quat , 4);
      cJSON * e  = cJSON_CreateFloatArray(euler, 3); // pitch yaw roll
      cJSON_AddItemToObject(imu, "quat",  q);
      cJSON_AddItemToObject(imu, "euler", e);

      cJSON_AddItemToObject(root, "imu", imu);
    }

    // print job
    char * p   = cJSON_PrintUnformatted(root);
    size_t len = strlen(p);
    memcpy(output_str, p, len);
    output_str[len] = '\n';
    uart_dump(output_str, len + 1);

    // free job
    cJSON_free(p);
    cJSON_Delete(root);
}

extern osMessageQId qSerialLogTimeHandle;
void SerialLogoutTask(void const * argument)
{
  /* USER CODE BEGIN SerialLogoutTask */
  uint16_t delayTime = 0;
  uint16_t rescv;
  /* Infinite loop */
  for(;;)
  {
    if (xQueueReceive(qSerialLogTimeHandle,
                            &rescv,
                            (delayTime == 0)? portMAX_DELAY : (TickType_t) delayTime
                        ) == pdFALSE ) {
        // sendpack
        send_state_pack();
    } else {
        ST_LOGI("Time update : %d !", rescv);
        if (rescv >= 0 || rescv <= 2000){
            delayTime = rescv;
        }
    }
  }
  /* USER CODE END SerialLogoutTask */
}