#include "sdcard.h"
#include "sdmmc.h"
#include <stdio.h>
static int sTransferCompleted = 0;
static int sBlockSize = 0;
void sdcardInit()
{
    printf("Init SD card...\n");
    MX_SDMMC1_SD_Init();
    HAL_SD_CardInfoTypeDef sdcardInfo;
    if (HAL_SD_GetCardInfo(&hsd1, &sdcardInfo) != HAL_OK)
    {
        Error_Handler();
    }
    sBlockSize = sdcardInfo.LogBlockSize;
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
    sTransferCompleted = 0;
    while ((HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER))
    {
    }
    if (HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t *)buffer, block, num) == HAL_OK)
    {
        ret = 0;
    }
    while (0 == sTransferCompleted)
    {
    }
    return ret;
}

int sdcardReadBlock(uint32_t block, uint32_t num, void *buffer)
{
    int ret = -1;
    sTransferCompleted = 0;
    while ((HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER))
    {
    }
    if (HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)buffer, block, num) == HAL_OK)
    {
        ret = 0;
    }
    while (0 == sTransferCompleted)
    {
    }
    return ret;
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  sTransferCompleted = 1;
}

/**
  * @brief Tx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  sTransferCompleted = 1;
}

/**
  * @brief SD error callbacks
  * @param hsd: SD handle
  * @retval None
  */
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
  Error_Handler();
}