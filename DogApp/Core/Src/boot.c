#include "main.h"
#include <stdint.h>

uint32_t BootArgs __attribute__((section(".bootcfg")));
void JumpToBootLoader(void)
{
  uint32_t i = 0;
  __IO uint32_t BootAddr = 0x08000000; /* Boot 地址 */
  void (*BootJump)(void) = (void (*)(void))(*((uint32_t *)(BootAddr + 4))); 

  BootArgs = 1; 
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

  /* 设置主堆栈指针 */
  __set_MSP(*(uint32_t *)BootAddr);

  /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
  __set_CONTROL(0);
  

  /* 跳转到系统BootLoader */
  BootJump();

  /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
  while (1)
  {
  }
}