
#include <stdio.h>
#include "w25q.h"
#include "stm32h7xx_hal_qspi.h"
 
extern QSPI_HandleTypeDef hqspi;
 
static void W25Qx_QSPI_Delay(uint32_t ms);
static uint8_t W25Qx_QSPI_Addr_Mode_Init(void);
static uint8_t W25Qx_QSPI_ResetMemory (void);
static uint8_t W25Qx_QSPI_WriteEnable (void);
static uint8_t W25Qx_QSPI_AutoPollingMemReady  (uint32_t Timeout);
 
/**
 * @brief  初始化QSPI存储器
 * @retval QSPI存储器状态
 */
uint8_t W25Qx_QSPI_Init(void)
{
	QSPI_CommandTypeDef s_command;
	uint8_t value = W25QxJV_FSR_QE;
 
	/* QSPI存储器复位 */
	if (W25Qx_QSPI_ResetMemory() != QSPI_OK)
	{
		return QSPI_NOT_SUPPORTED;
	}
 
	/* 使能写操作 */
	if (W25Qx_QSPI_WriteEnable() != QSPI_OK)
	{
		return QSPI_ERROR;
	}
	/* 设置四路使能的状态寄存器，使能四通道IO2和IO3引脚 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = WRITE_STATUS_REG2_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.DummyCycles = 0;
	s_command.NbData = 1;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	/* 配置命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	/* 传输数据 */
	if (HAL_QSPI_Transmit(&hqspi, &value, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	/* 自动轮询模式等待存储器就绪 */
	if (W25Qx_QSPI_AutoPollingMemReady(W25QxJV_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
	{
		return QSPI_ERROR;
	}
 
	/* 配置地址模式为 4 字节 , 非W25Q256直接跳过*/
	if (sFLASH_ID != 0XEF4019)
		return QSPI_OK;
 
	if (W25Qx_QSPI_Addr_Mode_Init() != QSPI_OK)
	{
		return QSPI_ERROR;
	}
 
	return QSPI_OK;
}
 
/**
 * @brief  检查地址模式不是4字节地址，配置为4字节
 * @retval QSPI存储器状态
 */
static uint8_t W25Qx_QSPI_Addr_Mode_Init(void)
{
	QSPI_CommandTypeDef s_command;
	uint8_t reg;
	/* 初始化读取状态寄存器命令 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = READ_STATUS_REG3_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.DummyCycles = 0;
	s_command.NbData = 1;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	/* 配置命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	/* 接收数据 */
	if (HAL_QSPI_Receive(&hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
 
	/* 检查寄存器的值 */
	if ((reg & W25Q256FV_FSR_4ByteAddrMode) == 1)    // 4字节模式
	{
		return QSPI_OK;
	}
	else    // 3字节模式
	{
		/* 配置进入 4 字节地址模式命令 */
		s_command.Instruction = Enter_4Byte_Addr_Mode_CMD;
		s_command.DataMode = QSPI_DATA_NONE;
 
		/* 配置并发送命令 */
		if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK)
		{
			return QSPI_ERROR;
		}
 
		/* 自动轮询模式等待存储器就绪 */
		if (W25Qx_QSPI_AutoPollingMemReady(
				W25QxJV_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
		{
			return QSPI_ERROR;
		}
 
		return QSPI_OK;
	}
}
 
/**
 * @brief  从QSPI存储器中读取大量数据.
 * @param  pData: 指向要读取的数据的指针
 * @param  ReadAddr: 读取起始地址
 * @param  Size: 要读取的数据大小
 * @retval QSPI存储器状态
 */
uint8_t W25Qx_QSPI_FastRead(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
	QSPI_CommandTypeDef s_command;
 
	if(Size == 0)	return QSPI_OK;
 
	/* 初始化读命令 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = QUAD_INOUT_FAST_READ_CMD;
	s_command.AddressMode = QSPI_ADDRESS_4_LINES;
	s_command.AddressSize = QSPI_ADDRESS_24_BITS;
	s_command.Address = ReadAddr;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_4_LINES;
	s_command.DummyCycles = 0;
	s_command.NbData = Size;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	/* 配置命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
 
	/* 接收数据 */
	if (HAL_QSPI_Receive(&hqspi, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}
 
/**
 * @brief  从QSPI存储器中读取大量数据.
 * @note   改指令只能使用在50MHz一下，本配置下不好用
 * @param  pData: 指向要读取的数据的指针
 * @param  ReadAddr: 读取起始地址
 * @param  Size: 要读取的数据大小
 * @retval QSPI存储器状态
 */
uint8_t W25Qx_QSPI_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
	QSPI_CommandTypeDef s_command;
	/* 初始化读命令 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = READ_CMD;    //READ_CMD;
	s_command.AddressMode = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize = QSPI_ADDRESS_24_BITS;
	s_command.Address = ReadAddr;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.DummyCycles = 0;
	s_command.NbData = Size;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	/* 配置命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
 
	/* 接收数据 */
	if (HAL_QSPI_Receive(&hqspi, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}
 
/**
 * @brief  将大量数据写入QSPI存储器
 * @param  pData: 指向要写入数据的指针
 * @param  WriteAddr: 写起始地址
 * @param  Size: 要写入的数据大小
 * @retval QSPI存储器状态
 */
uint8_t W25Qx_QSPI_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
	QSPI_CommandTypeDef s_command;
	uint32_t end_addr, current_size, current_addr;
	/* 计算写入地址和页面末尾之间的大小 */
	current_addr = 0;
 
	while (current_addr <= WriteAddr)
	{
		current_addr += W25QxJV_PAGE_SIZE;
	}
	current_size = current_addr - WriteAddr;
 
	/* 检查数据的大小是否小于页面中的剩余位置 */
	if (current_size > Size)
	{
		current_size = Size;
	}
 
	/* 初始化地址变量 */
	current_addr = WriteAddr;
	end_addr = WriteAddr + Size;
 
	/* 初始化程序命令 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = QUAD_INPUT_PAGE_PROG_CMD;
	s_command.AddressMode = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize = QSPI_ADDRESS_24_BITS;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_4_LINES;
	s_command.DummyCycles = 0;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	/* 逐页执行写入 */
	do
	{
		s_command.Address = current_addr;
		s_command.NbData = current_size;
 
		/* 启用写操作 */
		if (W25Qx_QSPI_WriteEnable() != QSPI_OK)
		{
			return QSPI_ERROR;
		}
 
		/* 配置命令 */
		if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK)
		{
			return QSPI_ERROR;
		}
 
		/* 传输数据 */
		if (HAL_QSPI_Transmit(&hqspi, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
				!= HAL_OK)
		{
			return QSPI_ERROR;
		}
 
		/* 配置自动轮询模式等待程序结束 */
		if (W25Qx_QSPI_AutoPollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
		{
			return QSPI_ERROR;
		}
 
		/* 更新下一页编程的地址和大小变量 */
		current_addr += current_size;
		pData += current_size;
		current_size =
				((current_addr + W25QxJV_PAGE_SIZE) > end_addr) ?
						(end_addr - current_addr) : W25QxJV_PAGE_SIZE;
	} while (current_addr < end_addr);
	return QSPI_OK;
}
 
/**
 * @brief  擦除QSPI存储器的指定块
 * @param  BlockAddress: 需要擦除的块地址
 * @retval QSPI存储器状态
 */
uint8_t W25Qx_QSPI_Erase_Block(uint32_t BlockAddress)
{
	QSPI_CommandTypeDef s_command;
	/* 初始化擦除命令 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = SECTOR_ERASE_CMD;
	s_command.AddressMode = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize = QSPI_ADDRESS_24_BITS;
	s_command.Address = BlockAddress;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_NONE;
	s_command.DummyCycles = 0;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	/* 启用写操作 */
	if (W25Qx_QSPI_WriteEnable() != QSPI_OK)
	{
		return QSPI_ERROR;
	}
 
	/* 发送命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
 
	/* 配置自动轮询模式等待擦除结束 */
	if (W25Qx_QSPI_AutoPollingMemReady(W25QxJV_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}
 
/**
 * @brief  擦除整个QSPI存储器
 * @retval QSPI存储器状态
 */
uint8_t W25Qx_QSPI_Erase_Chip(void)
{
	QSPI_CommandTypeDef s_command;
	/* 初始化擦除命令 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = QUAD_INPUT_PAGE_PROG_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_NONE;
	s_command.DummyCycles = 0;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	/* 启用写操作 */
	if (W25Qx_QSPI_WriteEnable() != QSPI_OK)
	{
		return QSPI_ERROR;
	}
	/* 发送命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	/* 配置自动轮询模式等待擦除结束 */
	if (W25Qx_QSPI_AutoPollingMemReady(W25QxJV_BULK_ERASE_MAX_TIME) != QSPI_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}
 
/**
 * @brief  读取QSPI存储器的当前状态
 * @retval QSPI存储器状态
 */
uint8_t W25Qx_QSPI_GetStatus(void)
{
	QSPI_CommandTypeDef s_command;
	uint8_t reg;
	/* 初始化读取状态寄存器命令 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = READ_STATUS_REG1_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.DummyCycles = 0;
	s_command.NbData = 1;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	/* 配置命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	/* 接收数据 */
	if (HAL_QSPI_Receive(&hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
	/* 检查寄存器的值 */
	if ((reg & W25QxJV_FSR_BUSY) != 0)
	{
		return QSPI_BUSY;
	}
	else
	{
		return QSPI_OK;
	}
}
 
/**
 * @brief  返回QSPI存储器的配置
 * @param  pInfo: 在配置结构上的指针
 * @retval QSPI存储器状态
 */
uint8_t W25Qx_QSPI_GetInfo(QSPI_Info *pInfo)
{
	/* 配置存储器配置结构 */
	pInfo->FlashSize = W25QxJV_FLASH_SIZE;
	pInfo->EraseSectorSize = W25QxJV_SUBSECTOR_SIZE;
	pInfo->EraseSectorsNumber = (W25QxJV_FLASH_SIZE / W25QxJV_SUBSECTOR_SIZE);
	pInfo->ProgPageSize = W25QxJV_PAGE_SIZE;
	pInfo->ProgPagesNumber = (W25QxJV_FLASH_SIZE / W25QxJV_PAGE_SIZE);
	return QSPI_OK;
}
 
/**
 * @brief  复位QSPI存储器。
 * @param  hqspi: QSPI句柄
 * @retval 无
 */
// extern int uart_printf(char * fmt, ...);
static uint8_t W25Qx_QSPI_ResetMemory()
{
	QSPI_CommandTypeDef s_command;
	/* 初始化复位使能命令 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = RESET_ENABLE_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_NONE;
	s_command.DummyCycles = 0;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	/* 发送命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
	//uart_printf("E1\n");
		return QSPI_ERROR;
	}
 
	/* 发送复位存储器命令 */
	s_command.Instruction = RESET_MEMORY_CMD;
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
    //uart_printf("E2\n");
		return QSPI_ERROR;
	}
 
	s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction = RESET_ENABLE_CMD;
 
	/* 发送命令 */
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
    //uart_printf("E3\n");
		return QSPI_ERROR;
	}
 
	/* 发送复位存储器命令 */
	s_command.Instruction = RESET_MEMORY_CMD;
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
    //uart_printf("E4\n");
		return QSPI_ERROR;
	}
 
	W25Qx_QSPI_Delay(1);
 
	/* 配置自动轮询模式等待存储器就绪 */
	if (W25Qx_QSPI_AutoPollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
	{
    //uart_printf("E5\n");
		return QSPI_ERROR;
	}
	return QSPI_OK;
}
 
/**
 * @brief  发送写入使能，等待它有效.
 * @param  hqspi: QSPI句柄
 * @retval 无
 */
static uint8_t W25Qx_QSPI_WriteEnable()
{
	QSPI_CommandTypeDef s_command;
	QSPI_AutoPollingTypeDef s_config;
	/* 启用写操作 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = WRITE_ENABLE_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_NONE;
	s_command.DummyCycles = 0;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		return QSPI_ERROR;
	}
 
	/* 配置自动轮询模式等待写启用 */
	s_config.Match = W25QxJV_FSR_WREN;
	s_config.Mask = W25QxJV_FSR_WREN;
	s_config.MatchMode = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval = 0x10;
	s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;
 
	s_command.Instruction = READ_STATUS_REG1_CMD;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.NbData = 1;
 
	if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config,
			HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}
 
/**
 * @brief  读取存储器的SR并等待EOP
 * @param  hqspi: QSPI句柄
 * @param  Timeout 超时
 * @retval 无
 */
static uint8_t W25Qx_QSPI_AutoPollingMemReady(uint32_t Timeout)
{
	QSPI_CommandTypeDef s_command;
	QSPI_AutoPollingTypeDef s_config;
	/* 配置自动轮询模式等待存储器准备就绪 */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = READ_STATUS_REG1_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.DummyCycles = 0;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	s_config.Match = 0x00;
	s_config.Mask = W25QxJV_FSR_BUSY;
	s_config.MatchMode = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval = 0x10;
	s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;
 
	if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, Timeout) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}
 
/**
 * @brief  读取FLASH ID
 * @param 	无
 * @retval FLASH ID
 */
uint32_t W25Qx_QSPI_FLASH_ReadID(void)
{
	QSPI_CommandTypeDef s_command;
	uint32_t Temp = 0;
	uint8_t pData[3];
	/* 读取JEDEC ID */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = READ_JEDEC_ID_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DummyCycles = 0;
	s_command.NbData = 3;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		printf("QSPI_FLASH_ReadID ERROR!!!....\r\n");
		/* 用户可以在这里添加一些代码来处理这个错误 */
		while (1)
		{
 
		}
	}
	if (HAL_QSPI_Receive(&hqspi, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
	{
		printf("QSPI_FLASH_ReadID ERROR!!!....\r\n");
		/* 用户可以在这里添加一些代码来处理这个错误 */
		while (1)
		{
 
		}
	}
 
	Temp = (pData[2] | pData[1] << 8) | (pData[0] << 16);
 
	return Temp;
}
 
/**
 * @brief  读取FLASH Device ID
 * @param 	无
 * @retval FLASH Device ID
 */
uint32_t W25Qx_QSPI_FLASH_ReadDeviceID(void)
{
	QSPI_CommandTypeDef s_command;
	uint32_t Temp = 0;
	uint8_t pData[3];
	/*##-2-读取设备ID测试    ###########################################*/
	/* 读取制造/设备 ID */
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = READ_ID_CMD;
	s_command.AddressMode = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize = QSPI_ADDRESS_24_BITS;
	s_command.Address = 0x000000;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.DummyCycles = 0;
	s_command.NbData = 2;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
 
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		printf("QSPI_FLASH_ReadDeviceID ERROR!!!....\r\n");
		/* 用户可以在这里添加一些代码来处理这个错误 */
		while (1)
		{
 
		}
	}
	if (HAL_QSPI_Receive(&hqspi, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)	!= HAL_OK)
	{
		printf("QSPI_FLASH_ReadDeviceID ERROR!!! ....\r\n");
		/* 用户可以在这里添加一些代码来处理这个错误 */
		while (1)
		{
 
		}
	}
 
	Temp = pData[1] | (pData[0] << 8);
 
	return Temp;
}

uint32_t QSPI_EnableMemoryMappedMode(void)
{
  QSPI_CommandTypeDef      s_command;
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
 
  /* Configure the command for the read instruction */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = QUAD_INOUT_FAST_READ_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 6;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_HALF_CLK_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod     = 0;
 
  return HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg);
}
 
static void W25Qx_QSPI_Delay(uint32_t ms)
{
	HAL_Delay(ms);
}