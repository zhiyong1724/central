#include "nandflash.h"
#include "fmc.h"
#include "stm32h7xx_hal_nand.h"
#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <stdio.h>
static void delay(uint32_t n)
{
    while (n--)
    {
    }
}

int gNandFlashReady = 0;

void nandFlashInit()
{
    printf("Init nandflash...");
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
    gNandFlashReady = 0;
    if (HAL_NAND_Reset(&hnand1) != HAL_OK)
    {
        Error_Handler();
    }
    while(0 == gNandFlashReady)
    {
    }
    printf("Init nandflash succeed, size 64M byte, block size 128K byte, page size 2K byte\n");
}

void nandFlashEraseBlock(uint32_t block)
{
    gNandFlashReady = 0;
    block <<= 6;
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_ERASE0;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)block;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(block >> 8);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(block >> 16);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_ERASE1;
    __DSB();
    while (0 == gNandFlashReady)
    {
    }
}

static void read(uint32_t block, uint32_t page, uint32_t column, void *buffer, uint32_t size)
{
    gNandFlashReady = 0;
    uint32_t pageAddress = block * NAND_FLASH_BLOCK_SIZE + page;
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_AREA_A;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)column;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(column >> 8);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)pageAddress;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(pageAddress >> 8);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(pageAddress >> 16);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA))  = NAND_CMD_AREA_TRUE1;
    __DSB();
    while(0 == gNandFlashReady)
    {
    }
    uint8_t *data = (uint8_t *)buffer;
    for (uint32_t i = 0; i < size; i++)
    {
        *data = *((__IO uint8_t *)NAND_DEVICE);
        data++;
        __DSB();
    }
}

int nandFlashReadPage(uint32_t block, uint32_t page, void *buffer)
{
    int ret = -1;
    read(block, page, 0, buffer, NAND_FLASH_PAGE_SIZE);
    return ret;
}

static void write(uint32_t block, uint32_t page, uint32_t column, const void *buffer, uint32_t size)
{
    gNandFlashReady = 0;
    uint32_t pageAddress = block * NAND_FLASH_BLOCK_SIZE + page;
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_WRITE0;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)column;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(column >> 8);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)pageAddress;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(pageAddress >> 8);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(pageAddress >> 16);
    __DSB();
    delay(30);
    const uint8_t *data = (const uint8_t *)buffer;
    for (uint32_t i = 0; i < size; i++)
    {
        *(__IO uint8_t *)NAND_DEVICE = *data;
        data++;
        __DSB();
    }
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_WRITE_TRUE1;
    __DSB();
    while(0 == gNandFlashReady)
    {
    }
    while(HAL_NAND_Read_Status(&hnand1) != NAND_READY)
    {
    }
}

int nandFlashWritePage(uint32_t block, uint32_t page, const void *buffer)
{
    int ret = -1;
    write(block, page, 0, buffer, NAND_FLASH_PAGE_SIZE);
    return ret;
}