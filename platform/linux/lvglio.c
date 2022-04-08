#include "lvglio.h"
#include <SDL2/SDL.h>
#include <string.h>
#include <stdlib.h>
#include "lvgl.h"
#include "ostask.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480
static SDL_Window *sWindow = NULL;
static SDL_Renderer *sRenderer = NULL;
static SDL_Texture *sTexture = NULL;

static lv_disp_draw_buf_t sDispBuffer;
static lv_disp_drv_t sDispDriver;

static lv_indev_drv_t sInputDriver;

static lv_color_t sBufferA[WINDOW_WIDTH * WINDOW_HEIGHT];
static lv_color_t sBufferB[WINDOW_WIDTH * WINDOW_HEIGHT];
static uint32_t sX = 0;
static uint32_t sY = 0;
static uint32_t sPressed = 0;
static void flush(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = WINDOW_WIDTH;
    rect.h = WINDOW_HEIGHT;
    SDL_UpdateTexture(sTexture, &rect, (const void *)color_p, WINDOW_WIDTH * sizeof(lv_color_t));
    SDL_RenderCopy(sRenderer, sTexture, NULL, NULL);
    SDL_RenderPresent(sRenderer);
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
    lv_disp_draw_buf_init(&sDispBuffer, sBufferA, sBufferB, WINDOW_WIDTH * WINDOW_HEIGHT);

    lv_disp_drv_init(&sDispDriver);
    sDispDriver.draw_buf = &sDispBuffer;
    sDispDriver.flush_cb = flush;
    sDispDriver.hor_res = WINDOW_WIDTH;
    sDispDriver.ver_res = WINDOW_HEIGHT;
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
        while (1)
        {
            SDL_Event event;
            SDL_WaitEvent(&event);
            switch (event.type)
            {
            case SDL_MOUSEMOTION:
                if (SDL_PRESSED == event.motion.state)
                {
                    sX = event.motion.x + event.motion.xrel;
                    sY = event.motion.y + event.motion.yrel;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                sPressed = 1;
                sX = event.button.x;
                sY = event.button.y;
                break;
            case SDL_MOUSEBUTTONUP:
                sPressed = 0;
                sX = event.button.x;
                sY = event.button.y;
                break;
            default:
                break;
            }
            lv_timer_handler();
        }
    }
    return NULL;
}

int lvglIOInit()
{
    SDL_Init(SDL_INIT_VIDEO);
    sWindow = SDL_CreateWindow("central", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    sRenderer = SDL_CreateRenderer(sWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (NULL == sRenderer)
    {
        sRenderer = SDL_CreateRenderer(sWindow, -1, 0);
    }
    sTexture = SDL_CreateTexture(sRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
    initLvgl();
    os_tid_t tid;
    osTaskCreate(&tid, lvglIORun, NULL, "lvglIORun", 0, 0);
    return 0;
}

int lvglIOUninit()
{
    if (sTexture != NULL)
    {
        SDL_DestroyTexture(sTexture);
    }
    if (sRenderer != NULL)
    {
        SDL_DestroyRenderer(sRenderer);
    }
    if (sWindow != NULL)
    {
        SDL_DestroyWindow(sWindow);
    }
    SDL_Quit();
    return 0;
}