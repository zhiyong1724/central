#include "normalmode.h"
#include "sai.h"
#include "dma.h"
#include <stdio.h>
#include "nandflash.h"
#include "sdcard.h"
#include "key.h"
#include "led.h"
#include "stm32h7xx_hal_cortex.h"
#include "oscentral.h"
#include "ostask.h"
#include "shellio.h"
#include "lfs.h"
#include "lfsio.h"
#include "lfsadapter.h"
#include "ff.h"
#include "fatfsadapter.h"
void enterNormalMode()
{
    printf("Start normal mode...\n");
    MX_DMA_Init();
    MX_SAI1_Init();
    nandFlashInit();
    sdcardInit();
    lfs_format(&gLFS, &gLfsConfig);
    HAL_NVIC_EnableIRQ(PendSV_IRQn);
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);
    osInit();
    registerLFS();
    registerFatfs();
    osFMount("/", "nand");
    shellIOInit();
    osTaskStart();
}