#include "lvglio.h"
#include <string.h>
#include <stdlib.h>
#include "lvgl.h"
#include "ostask.h"
#include "ltdc.h"
#include "gt9147.h"

static lv_disp_draw_buf_t sDispBuffer;
static lv_disp_drv_t sDispDriver;
static lv_indev_drv_t sInputDriver;
static lv_color_t sBufferA[LCD_WIDTH * LCD_HEIGH];
static lv_color_t sBufferB[LCD_WIDTH * LCD_HEIGH];
static uint32_t sX = 0;
static uint32_t sY = 0;
static uint32_t sPressed = 0;
static void flush(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    lcdCopy((const uint32_t *)color_p);
    lv_disp_flush_ready(disp_drv);
}

static void input(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    data->point.x = (lv_coord_t)sX;
    data->point.y = (lv_coord_t)sY;
    if (1 == sPressed)
    {
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void initLvgl()
{
    lv_init();
    lv_disp_draw_buf_init(&sDispBuffer, sBufferA, sBufferB, LCD_WIDTH * LCD_HEIGH);

    lv_disp_drv_init(&sDispDriver);
    sDispDriver.draw_buf = &sDispBuffer;
    sDispDriver.flush_cb = flush;
    sDispDriver.hor_res = LCD_WIDTH;
    sDispDriver.ver_res = LCD_HEIGH;
    sDispDriver.full_refresh = 1;
    sDispDriver.direct_mode = 1;
    lv_disp_drv_register(&sDispDriver);

    lv_indev_drv_init(&sInputDriver);
    sInputDriver.type = LV_INDEV_TYPE_POINTER;
    sInputDriver.read_cb = input;
    lv_indev_drv_register(&sInputDriver);
}

static void *lvglIORun(void *arg)
{
    while (1)
    {
        const LcdTouchInfo *lcdTouchInfo = GT9147_Scan();
        sPressed = lcdTouchInfo[0].touch;
        sX = lcdTouchInfo[0].x;
        sY = lcdTouchInfo[0].y;
        lv_timer_handler();
        osTaskSleep(20);
    }
    return NULL;
}

int lvglIOInit()
{
    MX_LTDC_Init();
    lcdOn();
    GT9147_Init();
    initLvgl();
    os_tid_t tid;
    osTaskCreate(&tid, lvglIORun, NULL, "ui", 0, OS_DEFAULT_TASK_STACK_SIZE);
    return 0;
}

int lvglIOUninit()
{
    return 0;
}