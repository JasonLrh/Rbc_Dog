/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
int uart_printf(const char *fmt, ...);
int debug_printf(const char *fmt, ...);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define UBOOT0_Pin GPIO_PIN_3
#define UBOOT0_GPIO_Port GPIOE
#define LD_OB_Pin GPIO_PIN_1
#define LD_OB_GPIO_Port GPIOA
#define UBOOT1_Pin GPIO_PIN_5
#define UBOOT1_GPIO_Port GPIOC
#define APP_BOOT_Pin GPIO_PIN_15
#define APP_BOOT_GPIO_Port GPIOE
#define APP_BOOT_EXTI_IRQn EXTI15_10_IRQn
#define LD_R_Pin GPIO_PIN_11
#define LD_R_GPIO_Port GPIOA
#define LD_Y_Pin GPIO_PIN_15
#define LD_Y_GPIO_Port GPIOA
#define INT_ICM_Pin GPIO_PIN_4
#define INT_ICM_GPIO_Port GPIOD
#define INT_ICM_EXTI_IRQn EXTI4_IRQn
#define SDO_SAO_Pin GPIO_PIN_6
#define SDO_SAO_GPIO_Port GPIOD
#define CS_ICM_Pin GPIO_PIN_5
#define CS_ICM_GPIO_Port GPIOB
#define CS_BMP_Pin GPIO_PIN_9
#define CS_BMP_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
