#include "normalmode.h"
#include "sai.h"
#include "dma.h"
#include <stdio.h>
#include "nandflash.h"
void enterNormalMode()
{
    printf("Start normal mode...\n");
    MX_DMA_Init();
    MX_SAI1_Init();
    nandFlashInit();
}