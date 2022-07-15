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
#include "bdma.h"
#include "dma.h"
#include "usart.h"
#include "quadspi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "w25q.h"
#include <string.h>
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
uint32_t BootArgs __attribute__((section(".bootcfg")));
volatile uint8_t is_program_done = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint8_t __attribute__((aligned(4))) program_buff_1 [W25QxJV_PAGE_SIZE * 2];
typedef struct __packed
{
  uint32_t len;
  uint16_t pack;
  char HeadDescipt;
} pack_head_t;
const pack_head_t * header_unpacker = (pack_head_t *)program_buff_1;

static uint8_t __attribute__((aligned(4)))  header_ret_str[7];

void BootSerialEvent(UART_HandleTypeDef *huart, uint16_t Pos){
  if (Pos < sizeof(pack_head_t)){
    return;
  }
  if (header_unpacker->len != 0 && header_unpacker->pack == 0xFFFF && header_unpacker->HeadDescipt == 'H'){
    // start pack
    /*
      len: program size
      pack : 0xFFFF
      HeadDscipt: 'H'

      return 
      pack & len: pagesize
      HeadDscipt: 'S'
      'E' if error
    */

    pack_head_t * ret_head = (pack_head_t *)header_ret_str;
    ret_head->pack = ret_head->len = W25QxJV_PAGE_SIZE;
    ret_head->HeadDescipt = 'S';

    // erase flash
    for (uint32_t i = 0; i <= (header_unpacker->len / W25QxJV_SUBSECTOR_SIZE); i++)
    {
      if (W25Qx_QSPI_Erase_Block(i*W25QxJV_SUBSECTOR_SIZE) != QSPI_OK)
      {
        ret_head->HeadDescipt = 'E';
        break;
      }
    }
    HAL_UART_Transmit(huart, header_ret_str, 7, HAL_MAX_DELAY);
  } else if (Pos == header_unpacker->len + sizeof(pack_head_t) && header_unpacker->HeadDescipt == 'D') {
    // valid program buff
    /*
      len: packlen
      pack : pack_count
      HeadDscipt: 'D'

      return 
      err : "PER\n" 
      success: "POK\n"
    */
    if (W25Qx_QSPI_Write(program_buff_1 + 7, header_unpacker->pack * W25QxJV_PAGE_SIZE, header_unpacker->len) != QSPI_OK) {
      HAL_UART_Transmit(huart, (uint8_t *)"PER\n", 4, HAL_MAX_DELAY);
    } else {
      HAL_UART_Transmit(huart, (uint8_t *)"POK\n", 4, HAL_MAX_DELAY);
    }
  } else if (header_unpacker->len == 0 && header_unpacker->pack == 0xFFFF) {
    switch (header_unpacker->HeadDescipt)
    {
    case 'R':{
      // RST into app
      BootArgs = 0;
      __set_FAULTMASK(1);
      NVIC_SystemReset();
    } break;

    case 'E':{
      is_program_done = 1;
    } break;
    
    default:
      break;
    }

  } else {
    HAL_UART_Transmit(huart, (uint8_t *)"PER\n", 4, HAL_MAX_DELAY);
  }


  if (HAL_UARTEx_ReceiveToIdle_IT(huart, program_buff_1, W25QxJV_PAGE_SIZE * 2) != HAL_OK){
    Error_Handler();
  }
}

// const static uint8_t program_info[] = {
//     0x3e,
//     (W25QxJV_PAGE_SIZE >> 8) & 0xff,
//     W25QxJV_PAGE_SIZE & 0xff,
//     0xb9};

volatile uint8_t is_run_after_start = 1;
volatile uint32_t program_size = 0;

volatile uint32_t current_address = 0;
volatile uint32_t current_pack_size = 0;

// static inline void check_next_receive(void)
// {
//   int remain;
//   current_address += current_pack_size;
//   remain = program_size - current_address;
//   if (remain == 0)
//   {
//     is_program_done = 1;
//     return;
//   }
//   else if (remain < W25QxJV_PAGE_SIZE)
//   {
//     current_pack_size = remain;
//   }
//   else
//   {
//     current_pack_size = W25QxJV_PAGE_SIZE;
//   }
//   // HAL_UART_Receive_IT(&hlpuart1, program_buff, current_pack_size);
//   HAL_UART_Receive(&hlpuart1, program_buff, current_pack_size, HAL_MAX_DELAY);
// }

// void program_page(UART_HandleTypeDef *huart)
// {
//   if (W25Qx_QSPI_Write(program_buff, current_address, current_pack_size) != QSPI_OK)
//   {
//     Error_Handler();
//   }

//   HAL_UART_Transmit(&hlpuart1, (uint8_t *)"POK\n", 4, HAL_MAX_DELAY);
//   check_next_receive();
// }

// void check_header(UART_HandleTypeDef *huart)
// {
//   is_run_after_start = program_buff[0] & 0x80;
//   program_buff[0] &= 0x7F;
//   for (int i = 0; i < 4; i++)
//   {
//     program_size <<= 8;
//     program_size |= program_buff[i];
//   }

//   // for (uint32_t i = 0; i <= (program_size / W25QxJV_SUBSECTOR_SIZE) + 1; i++)
//   for (uint32_t i = 0; i <= (program_size / W25QxJV_SUBSECTOR_SIZE) + 1; i++)
//   {
//     if (W25Qx_QSPI_Erase_Block(i*W25QxJV_SUBSECTOR_SIZE) != QSPI_OK)
//     {
//       Error_Handler();
//     }
//   }

//   // if ( W25Qx_QSPI_Erase_Chip() != QSPI_OK)
//   // {
//   //   Error_Handler();
//   // }

//   HAL_UART_Transmit(&hlpuart1, program_info, 4, HAL_MAX_DELAY);
//   // hlpuart1.RxCpltCallback = program_page;

//   check_next_receive();
// }

static void JumpToApp(void)
{
  uint32_t i = 0;
  __IO uint32_t AppAddr = 0x90000000; /* APP 地址 */
  void (*AppJump)(void) = (void (*)(void))(*((uint32_t *)(AppAddr + 4)));              /* 声明一个函数指针 */

  /* 关闭全局中断 */
  __disable_irq();

  /* 设置所有时钟到默认状态，使用HSI时钟 */
  HAL_RCC_DeInit();

  /* 关闭滴答定时器，复位到默认值 */
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  /* 关闭所有中断，清除所有中断挂起标志 */
  for (i = 0; i < 8; i++)
  {
    NVIC->ICER[i] = 0xFFFFFFFF;
    NVIC->ICPR[i] = 0xFFFFFFFF;
  }

  /* 使能全局中断 */
  __enable_irq();
  
  HAL_UART_DeInit(&hlpuart1);
  HAL_UART_DeInit(&huart8);
  /* 跳转到应用程序，首地址是MSP，地址+4是复位中断服务程序地址 */
  // AppJump = (void (*)(void))(*((uint32_t *)(AppAddr + 4)));

  /* 设置主堆栈指针 */
  __set_MSP(*(uint32_t *)AppAddr);

  /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
  __set_CONTROL(0);

  /* 跳转到系统BootLoader */
  AppJump();

  /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
  while (1)
  {
  }
}

void HAL_IncTick(void)
{
  uwTick += (uint32_t)uwTickFreq;
  if (is_program_done == 0 && (uwTick % 100) == 0){
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
  }
}

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
  // SCB_EnableICache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  if (BootArgs > 2){ // initial plug
    BootArgs = 0;
  } else if (BootArgs == 1){
    BootArgs = 2;
    __set_FAULTMASK(1);
    NVIC_SystemReset();
  }
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_BDMA_Init();
  MX_QUADSPI_Init();
  MX_DMA_Init();
  MX_LPUART1_UART_Init();
  MX_UART8_Init();
  /* USER CODE BEGIN 2 */
  
  // if (__get_PSP() > 0x90000000){
  //   HAL_GPIO_TogglePin(LD_OB_GPIO_Port, LD_OB_Pin);
  // }

  // uint16_t Pos;

  if (W25Qx_QSPI_Init()){
    Error_Handler();
  }

  if ((HAL_GPIO_ReadPin(UBOOT0_GPIO_Port, UBOOT0_Pin) == GPIO_PIN_RESET) || (HAL_GPIO_ReadPin(APP_BOOT_GPIO_Port, APP_BOOT_Pin) == GPIO_PIN_SET) || BootArgs != 0)
  { // ! BOOT To flash

    BootArgs = 0;
    // wait for flash and load
    if (HAL_UART_RegisterRxEventCallback(&huart8, BootSerialEvent) != HAL_OK){
      Error_Handler();
    }
    if (HAL_UARTEx_ReceiveToIdle_IT(&huart8, program_buff_1, W25QxJV_PAGE_SIZE * 2) != HAL_OK){
      __set_FAULTMASK(1);
      NVIC_SystemReset();
    }

    while (is_program_done == 0)
    {
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
      HAL_Delay(50);
    }
  }

  if (QSPI_EnableMemoryMappedMode())
  {
    Error_Handler();
  }

  // return;
  // TODO : Jump to location
  // if (is_run_after_start)
  // {
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    JumpToApp();
  // }

  // uint8_t * qspi_base = (uint8_t *) QSPI_BASE;
  // for (int i = 0; i < 10; i++){
  //   qspi_base[i] = 0x3d;
  // }
  // memcpy(qspi_base, "Hello, world\n", 14);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // __WFI();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 32;
  RCC_OscInitStruct.PLL.PLLN = 240;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
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
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
