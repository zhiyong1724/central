#include "sdcard.h"
#include "sdmmc.h"
#include <stdio.h>
void sdcardInit()
{
    printf("Init SD card...\n");
    MX_SDMMC1_SD_Init();
    HAL_SD_CardInfoTypeDef sdcardInfo;
    if (HAL_SD_GetCardInfo(&hsd1, &sdcardInfo) != HAL_OK)
    {
        Error_Handler();
    }
    printf("Init SD card succeed, block size %ld, block number %ld, type %ld.\n", sdcardInfo.LogBlockSize, sdcardInfo.LogBlockNbr, sdcardInfo.CardType);
}

uint32_t sdcardGetBlockSize()
{
    HAL_SD_CardInfoTypeDef sdcardInfo;
    if (HAL_SD_GetCardInfo(&hsd1, &sdcardInfo) != HAL_OK)
    {
        Error_Handler();
    }
    return sdcardInfo.LogBlockSize;
}

uint32_t sdcardGetBlockNumber()
{
    HAL_SD_CardInfoTypeDef sdcardInfo;
    if (HAL_SD_GetCardInfo(&hsd1, &sdcardInfo) != HAL_OK)
    {
        Error_Handler();
    }
    return sdcardInfo.LogBlockNbr;
}

int sdcardWriteBlock(uint32_t block, uint32_t num, const void *buffer)
{
    int ret = -1;
    __disable_irq();
    if (HAL_SD_WriteBlocks(&hsd1, (uint8_t *)buffer, block, num, HAL_MAX_DELAY) == HAL_OK)
    {
        ret = 0;
    }
    __enable_irq();
    while ((HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER))
    {
    }
    return ret;
}

int sdcardReadBlock(uint32_t block, uint32_t num, void *buffer)
{
    int ret = -1;
    __disable_irq();
    if (HAL_SD_ReadBlocks(&hsd1, (uint8_t *)buffer, block, num, HAL_MAX_DELAY) == HAL_OK)
    {
        ret = 0;
    }
    __enable_irq();
    while ((HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER))
    {
    }
    return ret;
}