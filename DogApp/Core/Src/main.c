/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "fdcan.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w25q.h"
#include "DogSoft.h"
#include <string.h>

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "arm_math.h"

#include "imu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// #define USE_TIMER_GEN_STEP

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef struct _boot_info_t
{
  uint32_t BootArgs;
  uint8_t head[4];
}boot_info_t;

boot_info_t bootConf __attribute__((section(".bootcfg")));

void HAL_IncTick(void)
{
  htim7.Instance->CNT = 0;
  uwTick += (uint32_t)uwTickFreq;
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
extern arm_pid_instance_f32 pid_yaw;
// static float target_angle;
extern uint8_t tim_queue_enable;

#ifdef USE_TIMER_GEN_STEP
extern osMessageQId qRobotTimerUpHandle;
void robot_loop(TIM_HandleTypeDef * htim){
    char descript[1] = "n";
    if (tim_queue_enable){
      // ST_LOGD("T");
      xQueueSendFromISR(qRobotTimerUpHandle, descript, NULL);
    }
}
#endif

void RobotOutTask(void const * argument)
{
  /* USER CODE BEGIN RobotOutTask */
  // float next_yaw_err = 0.f;
  // uint32_t tick;
  // char * ptr;
  uint32_t cnt = 0;

  // tick = HAL_GetTick();
  arm_pid_init_f32(&pid_yaw, 1);

  ST_LOGI("Starting motor ...");
  
  dog_motor_init();

  // stand up
  ST_LOGI("Stand up");
  dog_body_standup(-1.f, -1.f);
  osDelay(400);

  dog_body_standup(38.f, 0.7f);
  osDelay(400);

  dog_body_standup(45.3f, 0.9f);
  osDelay(400);

  // while (HAL_GetTick() - tick < 30000){
  //   ST_LOGD("Wait for IMU stable: %.1f", 30 - (HAL_GetTick() - tick)/1000.f);
  //   osDelay(500);
  // }

  // target_yaw = yaw;
  // osDelay(2000);
  // target_yaw = yaw;
#ifdef USE_TIMER_GEN_STEP
  HAL_TIM_RegisterCallback(&htim6, HAL_TIM_PERIOD_ELAPSED_CB_ID, robot_loop);
  HAL_TIM_Base_Start_IT(&htim6);
  htim6.Instance->ARR = DOG_CTRL_PERIOD_ms * 1000 - 1 ; // us
#else
  uint32_t prev_Time;
#endif
  /* Infinite loop */
  for(;;)
  {
#ifdef USE_TIMER_GEN_STEP
    if (xQueueReceive(qRobotTimerUpHandle, &ptr, portMAX_DELAY)) {
#else
    {
      osDelayUntil(&prev_Time, DOG_CTRL_PERIOD_ms);
      if (tim_queue_enable == 0){
        continue;
      }
#endif
      // ST_LOGI("TU");
      dog_body_simpleLinerWalk(); // TODO : check direction here
      cnt++;
      if (cnt >= 2000){
        HAL_GPIO_TogglePin(LD_R_GPIO_Port, LD_R_Pin);
        
        cnt = 0;
      }
    }
  }
  /* USER CODE END RobotOutTask */
}



// static char esp_echo_buff;
// void esp_echo(UART_HandleTypeDef * huart){
//   HAL_UART_Transmit_IT(huart, &esp_echo_buff, 1);
//   HAL_UART_Receive_IT(huart, &esp_echo_buff, 1);
// }

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
float target_yaw = 0.f;
// static const void (*qspi_program)(void) = (void (*)())(uint32_t *)(QSPI_BASE + 4);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  HAL_DBGMCU_EnableDBGSleepMode();
  HAL_DBGMCU_EnableDBGStandbyMode();
  HAL_DBGMCU_EnableDBGStopMode();
  // MX_DMA_Init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  MX_DMA_Init();
  MX_TIM7_Init();
  MX_UART8_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  fdcanfilter();
  HAL_TIM_Base_Start(&htim7);

  ST_LOGI("Starting imu ...");
  imu_start();

  ST_LOGI("System Start. Starting OS...");
  
  // target_yaw = yaw;
  

  
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // uart_printf("t5:%.3f,t6:%.3f\n", motors.raw[4].t, motors.raw[5].t);
    // __WFI();
    // uart_printf("yaw:%.3f\n", yaw);
    // HAL_Delay(20);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_CSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.CSIState = RCC_CSI_ON;
  RCC_OscInitStruct.CSICalibrationValue = RCC_CSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 48;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void errorHandler(char *file, uint32_t line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  if (HAL_TIM_Base_GetState(&htim6) == HAL_TIM_STATE_BUSY){
    HAL_TIM_Base_Stop_IT(&htim6); // stop robot loop
    dog_body_force_stop();
  }
  uart_printf("[%s:%ld]: error handler\n", file, line);
  // while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) != hfdcan1.Init.TxFifoQueueElmtsNbr);
  // while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) != hfdcan2.Init.TxFifoQueueElmtsNbr);
  ST_LOGD("[Error_Handler] system halt; restart need");
  // __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(LD_OB_GPIO_Port, LD_OB_Pin);
    HAL_Delay(300);
  }
  /* USER CODE END Error_Handler_Debug */
}
/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x20000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = 0x90000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO_URO;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_HFNMI_PRIVDEF_NONE);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  if (HAL_TIM_Base_GetState(&htim6) == HAL_TIM_STATE_BUSY){
    HAL_TIM_Base_Stop_IT(&htim6); // stop robot loop
    dog_body_force_stop();
  }
  ST_LOGD("[Error_Handler] system halt; restart need");
  // __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(LD_OB_GPIO_Port, LD_OB_Pin);
    HAL_Delay(300);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  uart_printf("[%s:%ld]:\tassert fail\n", file, line);
  Error_Handler();
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
