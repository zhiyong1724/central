/**
  ******************************************************************************
  * @file    quadspi.c
  * @brief   This file provides code for the configuration
  *          of the QUADSPI instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "quadspi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

QSPI_HandleTypeDef hqspi;

/* QUADSPI init function */
void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 22;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_7_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}

void MX_QUADSPI_Command(uint32_t command, uint32_t addrerss, uint32_t alternateBytes, uint32_t alternateBytesSize, uint32_t dataSize, uint32_t mode)
{
  QSPI_CommandTypeDef commandTypeDef;
  commandTypeDef.Instruction = command;
  switch (mode >> 0 & 0x03)
  {
  case 0:
    commandTypeDef.InstructionMode = QSPI_INSTRUCTION_NONE;
    break;
  case 1:
    commandTypeDef.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    break;
  case 2:
    commandTypeDef.InstructionMode = QSPI_INSTRUCTION_2_LINES;
    break;
  case 3:
    commandTypeDef.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    break;
  default:
    break;
  }
  commandTypeDef.Address = addrerss;
  switch (mode >> 2 & 0x03)
  {
  case 0:
    commandTypeDef.AddressSize = QSPI_ADDRESS_8_BITS;
    break;
  case 1:
    commandTypeDef.AddressSize = QSPI_ADDRESS_16_BITS;
    break;
  case 2:
    commandTypeDef.AddressSize = QSPI_ADDRESS_24_BITS;
    break;
  case 3:
    commandTypeDef.AddressSize = QSPI_ADDRESS_32_BITS;
    break;
  default:
    break;
  }
  switch (mode >> 4 & 0x03)
  {
  case 0:
    commandTypeDef.AddressMode = QSPI_ADDRESS_NONE;
    break;
  case 1:
    commandTypeDef.AddressMode = QSPI_ADDRESS_1_LINE;
    break;
  case 2:
    commandTypeDef.AddressMode = QSPI_ADDRESS_2_LINES;
    break;
  case 3:
    commandTypeDef.AddressMode = QSPI_ADDRESS_4_LINES;
    break;
  default:
    break;
  }
  commandTypeDef.AlternateBytes = alternateBytes;
  commandTypeDef.AlternateBytesSize = alternateBytesSize;
  switch (mode >> 6 & 0x03)
  {
  case 0:
    commandTypeDef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    break;
  case 1:
    commandTypeDef.AlternateByteMode = QSPI_ALTERNATE_BYTES_1_LINE;
    break;
  case 2:
    commandTypeDef.AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;
    break;
  case 3:
    commandTypeDef.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    break;
  default:
    break;
  }
  switch (mode >> 8 & 0x03)
  {
  case 0:
    commandTypeDef.DataMode = QSPI_DATA_NONE;
    break;
  case 1:
    commandTypeDef.DataMode = QSPI_DATA_1_LINE;
    break;
  case 2:
    commandTypeDef.DataMode = QSPI_DATA_2_LINES;
    break;
  case 3:
    commandTypeDef.DataMode = QSPI_DATA_4_LINES;
    break;
  default:
    break;
  }
  commandTypeDef.NbData = dataSize;
  commandTypeDef.DdrMode = QSPI_DDR_MODE_DISABLE;
  commandTypeDef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  commandTypeDef.DummyCycles = mode >> 10 & 0x1f;
  if (HAL_QSPI_Command(&hqspi, &commandTypeDef, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_QUADSPI_MemoryMapped(uint32_t command, uint32_t alternateBytes, uint32_t alternateBytesSize, uint32_t mode)
{
  QSPI_CommandTypeDef commandTypeDef;
  commandTypeDef.Instruction = command;
  switch (mode >> 0 & 0x03)
  {
  case 0:
    commandTypeDef.InstructionMode = QSPI_INSTRUCTION_NONE;
    break;
  case 1:
    commandTypeDef.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    break;
  case 2:
    commandTypeDef.InstructionMode = QSPI_INSTRUCTION_2_LINES;
    break;
  case 3:
    commandTypeDef.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    break;
  default:
    break;
  }
  switch (mode >> 2 & 0x03)
  {
  case 0:
    commandTypeDef.AddressSize = QSPI_ADDRESS_8_BITS;
    break;
  case 1:
    commandTypeDef.AddressSize = QSPI_ADDRESS_16_BITS;
    break;
  case 2:
    commandTypeDef.AddressSize = QSPI_ADDRESS_24_BITS;
    break;
  case 3:
    commandTypeDef.AddressSize = QSPI_ADDRESS_32_BITS;
    break;
  default:
    break;
  }
  switch (mode >> 4 & 0x03)
  {
  case 0:
    commandTypeDef.AddressMode = QSPI_ADDRESS_NONE;
    break;
  case 1:
    commandTypeDef.AddressMode = QSPI_ADDRESS_1_LINE;
    break;
  case 2:
    commandTypeDef.AddressMode = QSPI_ADDRESS_2_LINES;
    break;
  case 3:
    commandTypeDef.AddressMode = QSPI_ADDRESS_4_LINES;
    break;
  default:
    break;
  }
  commandTypeDef.AlternateBytes = alternateBytes;
  commandTypeDef.AlternateBytesSize = alternateBytesSize;
  switch (mode >> 6 & 0x03)
  {
  case 0:
    commandTypeDef.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    break;
  case 1:
    commandTypeDef.AlternateByteMode = QSPI_ALTERNATE_BYTES_1_LINE;
    break;
  case 2:
    commandTypeDef.AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;
    break;
  case 3:
    commandTypeDef.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    break;
  default:
    break;
  }
  switch (mode >> 8 & 0x03)
  {
  case 0:
    commandTypeDef.DataMode = QSPI_DATA_NONE;
    break;
  case 1:
    commandTypeDef.DataMode = QSPI_DATA_1_LINE;
    break;
  case 2:
    commandTypeDef.DataMode = QSPI_DATA_2_LINES;
    break;
  case 3:
    commandTypeDef.DataMode = QSPI_DATA_4_LINES;
    break;
  default:
    break;
  }
  commandTypeDef.DdrMode = QSPI_DDR_MODE_DISABLE;
  commandTypeDef.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  commandTypeDef.DummyCycles = mode >> 10 & 0x1f;
  QSPI_MemoryMappedTypeDef memoryMappedTypeDef;
  memoryMappedTypeDef.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  if (HAL_QSPI_MemoryMapped(&hqspi, &commandTypeDef, &memoryMappedTypeDef) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_QUADSPI_Transmit(uint8_t *pData)
{
  if (HAL_QSPI_Transmit(&hqspi, pData, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_QUADSPI_Receive(uint8_t *pData)
{
  if (HAL_QSPI_Receive(&hqspi, pData, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */
  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
    PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    /**QUADSPI GPIO Configuration
    PB6     ------> QUADSPI_BK1_NCS
    PF6     ------> QUADSPI_BK1_IO3
    PF7     ------> QUADSPI_BK1_IO2
    PF8     ------> QUADSPI_BK1_IO0
    PF9     ------> QUADSPI_BK1_IO1
    PB2     ------> QUADSPI_CLK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}

void MX_Enable_Qspi()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

void MX_Disable_Qspi()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{

  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PB6     ------> QUADSPI_BK1_NCS
    PF6     ------> QUADSPI_BK1_IO3
    PF7     ------> QUADSPI_BK1_IO2
    PF8     ------> QUADSPI_BK1_IO0
    PF9     ------> QUADSPI_BK1_IO1
    PB2     ------> QUADSPI_CLK
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9);

  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
