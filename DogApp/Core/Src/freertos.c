/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId robotOutTaskHandle;
// uint32_t robotOutTaskBuffer[ 2048 ];
osStaticThreadDef_t robotOutTaskControlBlock;
osThreadId serialCmdTaskHandle;
// uint32_t serialCmdTaskBuffer[ 2048 ];
osStaticThreadDef_t serialCmdTaskControlBlock;
osMessageQId qSerialCMDHandle;
osMessageQId qRobotTimerUpHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
StackType_t __attribute__ ((section(".axi_data"))) robotOutTaskBuffer[ 4096 ];
StackType_t __attribute__ ((section(".axi_data"))) serialCmdTaskBuffer[ 4096 ];
// StackType_t robotOutTaskBuffer[ 2048 ];
// StackType_t serialCmdTaskBuffer[ 2048 ];
/* USER CODE END FunctionPrototypes */

void RobotOutTask(void const * argument);
void SerialCmdTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
   ST_LOGE("freeRTOS Stack Overflow");
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
   ST_LOGE("freeRTOS malloc Fail");
}
/* USER CODE END 5 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of qSerialCMD */
  osMessageQDef(qSerialCMD, 16, uint32_t);
  qSerialCMDHandle = osMessageCreate(osMessageQ(qSerialCMD), NULL);

  /* definition and creation of qRobotTimerUp */
  osMessageQDef(qRobotTimerUp, 4, uint32_t);
  qRobotTimerUpHandle = osMessageCreate(osMessageQ(qRobotTimerUp), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of robotOutTask */
  osThreadStaticDef(robotOutTask, RobotOutTask, osPriorityNormal, 0, 4096, robotOutTaskBuffer, &robotOutTaskControlBlock);
  robotOutTaskHandle = osThreadCreate(osThread(robotOutTask), NULL);

  /* definition and creation of serialCmdTask */
  osThreadStaticDef(serialCmdTask, SerialCmdTask, osPriorityIdle, 0, 4096, serialCmdTaskBuffer, &serialCmdTaskControlBlock);
  serialCmdTaskHandle = osThreadCreate(osThread(serialCmdTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_RobotOutTask */
/**
  * @brief  Function implementing the robotOutTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_RobotOutTask */
__weak void RobotOutTask(void const * argument)
{
  /* USER CODE BEGIN RobotOutTask */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LD_R_GPIO_Port, LD_R_Pin);
    osDelay(400);
  }
  /* USER CODE END RobotOutTask */
}

/* USER CODE BEGIN Header_SerialCmdTask */
/**
* @brief Function implementing the serialCmdTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SerialCmdTask */
__weak void SerialCmdTask(void const * argument)
{
  /* USER CODE BEGIN SerialCmdTask */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LD_Y_GPIO_Port, LD_Y_Pin);
    osDelay(300);
  }
  /* USER CODE END SerialCmdTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
