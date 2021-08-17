#include "lvglio.h"
#include <X11/Xlib.h>
#include <string.h>
#include <stdlib.h>
#include "lvgl.h"
#include "ostask.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480
static Display *sDisplay = NULL;
static Window sWindow = 0;

static lv_disp_draw_buf_t sDispBuffer;
static lv_disp_drv_t sDispDriver;

static lv_indev_drv_t sInputDriver;

static lv_color_t sBufferA[WINDOW_WIDTH * WINDOW_HEIGHT * 4 / 10];
static lv_color_t sBufferB[WINDOW_WIDTH * WINDOW_HEIGHT * 4 / 10];
static uint32_t sBuffer[WINDOW_HEIGHT][WINDOW_WIDTH];

static uint32_t sX = 0;
static uint32_t sY = 0;
static uint32_t sPress = 0;
static void flush(struct _lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    for (size_t i = area->y1; i <= area->y2; i++)
    {
        for (size_t j = area->x1; j <= area->x2; j++)
        {
            sBuffer[j][i] = (uint32_t)color_p->ch.alpha << 24 | (uint32_t)color_p->ch.red << 16 | (uint32_t)color_p->ch.green << 8 | (uint32_t)color_p->ch.blue;
            color_p++;
        }
    }
    lv_disp_flush_ready(disp_drv);
}

static void input(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    data->point.x = (lv_coord_t)sX;
    data->point.y = (lv_coord_t)sY;
    if (1 == sPress)
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
    lv_disp_draw_buf_init(&sDispBuffer, sBufferA, sBufferB, WINDOW_WIDTH * WINDOW_HEIGHT * 4 / 10);

    lv_disp_drv_init(&sDispDriver);
    sDispDriver.draw_buf = &sDispBuffer;
    sDispDriver.flush_cb = flush;
    sDispDriver.hor_res = WINDOW_WIDTH;
    sDispDriver.ver_res = WINDOW_HEIGHT;

    lv_disp_drv_register(&sDispDriver);

    lv_indev_drv_init(&sInputDriver);
    sInputDriver.type = LV_INDEV_TYPE_POINTER;
    sInputDriver.read_cb = input;
    lv_indev_drv_register(&sInputDriver);
}

static void *lvglIORun(void *arg)
{
    if (sDisplay != NULL)
    {
        while (1)
        {
            XEvent event;
            while (1)
            {
                lv_timer_handler();
                XNextEvent(sDisplay, &event);
                switch (event.type)
                {
                case ButtonPress:
                sX = event.xbutton.x;
                sY = event.xbutton.y;
                sPress = 1;
                    break;
                case ButtonRelease:
                sX = event.xbutton.x;
                sY = event.xbutton.y;
                sPress = 0;
                    break;
                default:
                {
                    XGCValues values;
                    GC gc = XCreateGC(sDisplay, sWindow, 0, &values);
                    for (size_t i = 0; i < WINDOW_HEIGHT; i++)
                    {
                        for (size_t j = 0; j < WINDOW_WIDTH; j++)
                        {
                            XSetForeground(sDisplay, gc, sBuffer[j][i]);
                            XDrawPoint(sDisplay, sWindow, gc, j, i);
                        }
                    }
                    XFree(gc);
                    XFlush(sDisplay);
                    break;
                }
                }
                osTaskSleep(20);
            }
        }
    }
    return NULL;
}

int lvglIOInit()
{
    int ret = -1;
    sDisplay = XOpenDisplay(NULL);
    if (sDisplay != NULL)
    {
        int screen = XDefaultScreen(sDisplay);
        int width = XDisplayWidth(sDisplay, screen);
        int height = XDisplayHeight(sDisplay, screen);
        sWindow = XCreateSimpleWindow(sDisplay, XDefaultRootWindow(sDisplay), 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, XBlackPixel(sDisplay, screen), XWhitePixel(sDisplay, screen));
        XMapWindow(sDisplay, sWindow);
        XMoveWindow(sDisplay, sWindow, width / 2 - WINDOW_WIDTH / 2, height / 2 - WINDOW_HEIGHT / 2);
        XSelectInput(sDisplay, sWindow, ExposureMask | ButtonPressMask | ButtonReleaseMask);
        initLvgl();
        os_tid_t tid;
        osTaskCreate(&tid, lvglIORun, NULL, "lvglIORun", 0, 0);
        ret = 0;
    }
    return ret;
}

int lvglIOUninit()
{
    int ret = -1;
    if (sDisplay != NULL)
    {
        XDestroyWindow(sDisplay, sWindow);
        XCloseDisplay(sDisplay);
        sDisplay = NULL;
        ret = 0;
    }
    return ret;
}