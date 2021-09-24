#include "normalmode.h"
#include "sai.h"
#include "dma.h"
#include <stdio.h>
#include "nandflash.h"
#include "key.h"
#include "led.h"
void enterNormalMode()
{
    printf("Start normal mode...\n");
    MX_DMA_Init();
    MX_SAI1_Init();
    while (keyUpStatus() != KEY_STATUS_PRESS)
        ;
    led0ON();
    led1ON();
    nandFlashInit();
    //nandFlashEraseBlock(0);
    static uint8_t buff[NAND_FLASH_PAGE_SIZE];
    // for (size_t i = 0; i < NAND_FLASH_PAGE_SIZE; i++)
    // {
    //     buff[i] = i;
    // }
    //nandFlashWritePage(0, 0, buff);
    nandFlashReadPage(0, 0, buff);
    nandFlashReadPage(0, 0, buff);
}