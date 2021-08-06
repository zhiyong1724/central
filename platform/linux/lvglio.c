#include "lvglio.h"
#include <X11/Xlib.h>
#include <string.h>
#include <stdlib.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480
static Display *sDisplay = NULL;
static Window sWindow = 0;
static GC sGC = NULL;
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
        XGCValues values;
        sGC = XCreateGC(sDisplay, sWindow, 0, &values);
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

int lvglIORun()
{
    int ret = -1;
    if (sDisplay != NULL)
    {
        while (1)
        {
            XFlush(sDisplay);
        }
        ret = 0;
    }
    return ret;
}