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
osThreadId defaultTaskHandle;
osThreadId serialCmdTaskHandle;
osThreadId canRecTaskHandle;
osThreadId robotMoveTaskHandle;
osThreadId sendStateTaskHandle;
osMessageQId qSerialHandle;
osMessageQId qCanRecHandle;
osMessageQId qRobotControlTimerHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void SerialCmdTask(void const * argument);
void CanRecTask(void const * argument);
void RobotMoveTask(void const * argument);
void SendStateTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   uart_printf("[Stack overflow]: (%s)\n", pcTaskName);
}
/* USER CODE END 4 */

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
  /* definition and creation of qSerial */
  osMessageQDef(qSerial, 32, uint32_t);
  qSerialHandle = osMessageCreate(osMessageQ(qSerial), NULL);

  /* definition and creation of qCanRec */
  osMessageQDef(qCanRec, 32, uint32_t);
  qCanRecHandle = osMessageCreate(osMessageQ(qCanRec), NULL);

  /* definition and creation of qRobotControlTimer */
  osMessageQDef(qRobotControlTimer, 32, uint32_t);
  qRobotControlTimerHandle = osMessageCreate(osMessageQ(qRobotControlTimer), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of serialCmdTask */
  osThreadDef(serialCmdTask, SerialCmdTask, osPriorityIdle, 0, 2048);
  serialCmdTaskHandle = osThreadCreate(osThread(serialCmdTask), NULL);

  /* definition and creation of canRecTask */
  osThreadDef(canRecTask, CanRecTask, osPriorityIdle, 0, 512);
  canRecTaskHandle = osThreadCreate(osThread(canRecTask), NULL);

  /* definition and creation of robotMoveTask */
  osThreadDef(robotMoveTask, RobotMoveTask, osPriorityIdle, 0, 4096);
  robotMoveTaskHandle = osThreadCreate(osThread(robotMoveTask), NULL);

  /* definition and creation of sendStateTask */
  osThreadDef(sendStateTask, SendStateTask, osPriorityIdle, 0, 512);
  sendStateTaskHandle = osThreadCreate(osThread(sendStateTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
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
    osDelay(1);
  }
  /* USER CODE END SerialCmdTask */
}

/* USER CODE BEGIN Header_CanRecTask */
/**
* @brief Function implementing the canRecTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CanRecTask */
void CanRecTask(void const * argument)
{
  /* USER CODE BEGIN CanRecTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END CanRecTask */
}

/* USER CODE BEGIN Header_RobotMoveTask */
/**
* @brief Function implementing the robotMoveTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RobotMoveTask */
// void RobotMoveTask(void const * argument)
// {
//   /* USER CODE BEGIN RobotMoveTask */
//   /* Infinite loop */
//   for(;;)
//   {
//     osDelay(1);
//   }
//   /* USER CODE END RobotMoveTask */
// }

/* USER CODE BEGIN Header_SendStateTask */
/**
* @brief Function implementing the sendStateTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SendStateTask */
void SendStateTask(void const * argument)
{
  /* USER CODE BEGIN SendStateTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END SendStateTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
