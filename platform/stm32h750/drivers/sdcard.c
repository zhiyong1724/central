#include "sdcard.h"
#include "sdmmc.h"
#include <stdio.h>
#include "sys_lock.h"
static sys_mutex_t s_mutex;
int sdcardInit()
{
  int ret = -1;
  printf("Init SD card...\n");
  HAL_SD_CardInfoTypeDef sdcardInfo;
  if (MX_SDMMC1_SD_Init() == HAL_OK && HAL_SD_GetCardInfo(&hsd1, &sdcardInfo) == HAL_OK)
  {
    sys_mutex_create(&s_mutex);
    printf("Init SD card succeed, block size %ld, block number %ld, type %ld.\n", sdcardInfo.LogBlockSize, sdcardInfo.LogBlockNbr, sdcardInfo.CardType);
    ret = 0;
  }
  else
  {
    printf("Init SD card fail.\n");
  }
  return ret;
}

uint32_t sdcardGetBlockSize()
{
  sys_mutex_lock(&s_mutex);
  HAL_SD_CardInfoTypeDef sdcardInfo;
  if (HAL_SD_GetCardInfo(&hsd1, &sdcardInfo) != HAL_OK)
  {
    Error_Handler();
  }
  sys_mutex_unlock(&s_mutex);
  return sdcardInfo.LogBlockSize;
}

uint32_t sdcardGetBlockNumber()
{
  sys_mutex_lock(&s_mutex);
  HAL_SD_CardInfoTypeDef sdcardInfo;
  if (HAL_SD_GetCardInfo(&hsd1, &sdcardInfo) != HAL_OK)
  {
    Error_Handler();
  }
  sys_mutex_unlock(&s_mutex);
  return sdcardInfo.LogBlockNbr;
}

int sdcardWriteBlock(uint32_t block, uint32_t num, const void *buffer)
{
  sys_mutex_lock(&s_mutex);
  int ret = -1;
  while ((HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER))
  {
  }
  __disable_irq();
  if (HAL_SD_WriteBlocks(&hsd1, (uint8_t *)buffer, block, num, 10000) == HAL_OK)
  {
    ret = 0;
  }
  __enable_irq();
  sys_mutex_unlock(&s_mutex);
  return ret;
}

int sdcardReadBlock(uint32_t block, uint32_t num, void *buffer)
{
  sys_mutex_lock(&s_mutex);
  int ret = -1;
  while ((HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER))
  {
  }
  __disable_irq();
  if (HAL_SD_ReadBlocks(&hsd1, (uint8_t *)buffer, block, num, 10000) == HAL_OK)
  {
    ret = 0;
  }
  __enable_irq();
  sys_mutex_unlock(&s_mutex);
  return ret;
}