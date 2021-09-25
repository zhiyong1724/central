#include "normalmode.h"
#include "sai.h"
#include "dma.h"
#include <stdio.h>
#include "nandflash.h"
#include "sdcard.h"
#include "key.h"
#include "led.h"
void enterNormalMode()
{
    printf("Start normal mode...\n");
    MX_DMA_Init();
    MX_SAI1_Init();
    nandFlashInit();
    sdcardInit();
    while (keyUpStatus() != KEY_STATUS_PRESS)
    {
    }    
    led0ON();
    led1ON();
    static uint8_t buff[2048];
    for (size_t i = 0; i < 2048; i++)
    {
        buff[i] = i;
    }
    //sdcardWriteBlock(0, 1, buff);
    for (size_t i = 0; i < 2048; i++)
    {
        buff[i] = 0;
    }
    sdcardReadBlock(0, 1, buff);
    sdcardReadBlock(0, 1, buff);
}