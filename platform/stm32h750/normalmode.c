#include "normalmode.h"
#include "sai.h"
#include "dma.h"
#include <stdio.h>
#include "sdcard.h"
#include "key.h"
#include "led.h"
#include "stm32h7xx_hal_cortex.h"
#include "sys_central.h"
#include "sys_task.h"
#include "shellio.h"
#include "lfs.h"
#include "nandflash.h"
#include "lfs_adapter.h"
#include "ff.h"
#include "fatfs_adapter.h"
#include "sys_mem.h"
#include "pcf8574.h"
#include "es8388.h"
#include "sai.h"
#include "keymanager.h"
#include "volumemanager.h"
#include "lvglio.h"
static int onPressed(void *object, KeyType type)
{
    return 0;
}

static int onReleased(void *object, KeyType type)
{
    switch (type)
    {
    case KEY_TYPE_KEY_0:
    {
        int volume = volumeManagerVolumeUp();
        printf("volume = %d\n", volume);
        break;
    }
    case KEY_TYPE_KEY_1:
        /* code */
        break;
    case KEY_TYPE_KEY_2:
    {
        int volume = volumeManagerVolumeDown();
        printf("volume = %d\n", volume);
        break;
    }
    case KEY_TYPE_KEY_UP:
        /* code */
        break;

    default:
        break;
    }
    return 0;
}

extern const struct lfs_config g_lfs_config;
extern lfs_t g_lfs;
void enterNormalMode()
{
    printf("Start normal mode...\n");
    sys_init();
    PCF8574_Init();
    ES8388_Init();
    ES8388_ADDA_Cfg(1, 0); //开启DAC
    ES8388_Output_Cfg(1, 1);  //DAC选择通道输出
    ES8388_I2S_Cfg(0, 3);     //飞利浦标准I2S，16BIT
    MX_DMA_Init();
    MX_SAI1_Init();
    nandFlashInit();
    //lfs_format(&g_lfs, &g_lfs_config);
    HAL_NVIC_EnableIRQ(PendSV_IRQn);
    HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
    register_lfs();
    register_fatfs();
    osFMount("/", "nand");
    if (sdcardInit() == 0)
    {
        osFMkDir("sd");
        osFMount("sd", "0:");
    }
    volumeManagerInit();
    keyManagerInit();
    KeyManagerCallBack keyManagerCallBack;
    keyManagerCallBack.object = NULL;
    keyManagerCallBack.onPressed = onPressed;
    keyManagerCallBack.onReleased = onReleased;
    keyManagerRegisterCallback(&keyManagerCallBack);
    shell_io_init();
    //lvglIOInit();
    sys_task_start();
}