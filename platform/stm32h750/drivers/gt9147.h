#ifndef __GT9147_H__
#define __GT9147_H__
#include "main.h"
#define GT_MAX_TOUCH_NUM 5
typedef struct LcdTouchInfo
{
    uint8_t touch;
    uint16_t x;
    uint16_t y;
} LcdTouchInfo;
uint8_t GT9147_Init();
const LcdTouchInfo *GT9147_Scan();
#endif