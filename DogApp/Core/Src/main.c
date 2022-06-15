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
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
extern arm_pid_instance_f32 pid_yaw;
// static float target_angle;
void robot_loop(TIM_HandleTypeDef * htim){
    static uint8_t cnt = 0;
    static float next_yaw_err = 0.f;
    dog_body_simpleLinerWalk( - pitch, next_yaw_err); // TODO : check direction here

    // dog_leg_input_t vec = {
    //   .theta = 0.f,
    //   .dist = 0.30f,
    // };
    // dog_leg_set_phrase(motors.leg.l_f, &vec);
    // dog_leg_set_phrase(motors.leg.r_f, &vec);
    // dog_leg_set_phrase(motors.leg.r_b, &vec);
    // dog_leg_set_phrase(motors.leg.l_b, &vec);

    cnt++;
    if (cnt >= 2000){
      cnt = 0;
      HAL_GPIO_TogglePin(LD_OB_GPIO_Port, LD_OB_Pin);

    
    }
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

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

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
  MX_DMA_Init();
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
  imu_start();
  uart_printf("start\n");
  arm_pid_init_f32(&pid_yaw, 1);
  htim6.Instance->ARR = 2000 - 1 ; // us
  target_yaw = yaw;
  
  fdcanfilter();
  HAL_TIM_Base_Start(&htim7);
  // HAL_Delay(200); // wait for tick stable

  // HAL_NVIC_EnableIRQ(INT_ICM_EXTI_IRQn);
  // // while (1){__WFI();}

  // // uint8_t fifolevel;
  // HAL_Delay(1000);
  // dog_motor_init();
  // dog_cmd_start(&huart1);


  // // stand up
  // dog_body_standup(-1.f, -1.f);
  // HAL_Delay(400);

  // dog_body_standup(38.f, 0.7f);
  // HAL_Delay(400);

  // dog_body_standup(45.3f, 0.9f);
  // HAL_Delay(400);

  // target_yaw = yaw;
  // HAL_Delay(2000);
  // target_yaw = yaw;
  // HAL_TIM_Base_Start_IT(&htim6);
  
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
    HAL_Delay(20);

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

/* USER CODE END 4 */

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
  uart_printf("[Error_Handler] Entering error handler\n");
  // while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) != hfdcan1.Init.TxFifoQueueElmtsNbr);
  // while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) != hfdcan2.Init.TxFifoQueueElmtsNbr);
  uart_printf("[Error_Handler] system halt; restart need\n");
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
  uart_printf("[assert fail] : %s:%d\n", file, line);
  Error_Handler();
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
