#include "normalmode.h"
#include "sai.h"
#include "dma.h"
#include <stdio.h>
#include "sdcard.h"
#include "key.h"
#include "led.h"
#include "stm32h7xx_hal_cortex.h"
#include "oscentral.h"
#include "ostask.h"
#include "shellio.h"
#include "lfs.h"
#include "nandflash.h"
#include "lfsadapter.h"
#include "ff.h"
#include "fatfsadapter.h"
#include "osmem.h"
#include "pcf8574.h"
#include "es8388.h"
#include "sai.h"
void enterNormalMode()
{
    printf("Start normal mode...\n");
    PCF8574_Init();
    ES8388_Init();
    ES8388_ADDA_Cfg(1, 0); //开启DAC
    ES8388_Output_Cfg(1, 1);  //DAC选择通道输出
    ES8388_HPvol_Set(10);     //设置耳机音量
    ES8388_SPKvol_Set(10);     //设置喇叭音量
    ES8388_I2S_Cfg(0, 3);     //飞利浦标准I2S，16BIT
    MX_DMA_Init();
    MX_SAI1_Init();
    nandFlashInit();
    sdcardInit();
    HAL_NVIC_EnableIRQ(PendSV_IRQn);
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);
    osInit();
    registerLFS();
    registerFatfs();
    osFMount("/", "nand");
    osFMkDir("sd");
    osFMount("sd", "0:");
    shellIOInit();
    osTaskStart();
}