#include "normalmode.h"
#include "sai.h"
#include "dma.h"
#include <stdio.h>

static void boot()
{

}
int sSaiA;
int sSaiB = 177;
void enterNormalMode()
{
    boot();
    printf("MX_SAI1_Init start...\n");
    sSaiA = sSaiB;
    printf("MX_SAI1_Init end...\n");
    printf("Start normal mode...\n");
    MX_DMA_Init();
    MX_SAI1_Init();
}