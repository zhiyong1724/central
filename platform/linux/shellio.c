#include "shellio.h"
#include "shell.h"
#include "ostask.h"
#include <stdlib.h>
#include <stdio.h>
#include "osf.h"
static Shell sShell;
static char sShellBuffer[1024];
char gShellPathBuffer[OS_MAX_FILE_PATH_LENGTH] = {'/', '\0'};
static short shellRead(char *data, unsigned short len)
{
    for (unsigned short i = 0; i < len; i++)
    {
        int ret = getchar();
        if (ret != EOF)
        {
            data[i] = (char)ret;
        }
        else
        {
            break;
        }
    }
    return len;
}

static short shellWrite(char *data, unsigned short len)
{
    for (unsigned short i = 0; i < len; i++)
    {
        putchar(data[i]);
    }
    return 0;
}

static void *_shellTask(void *arg)
{
    while (1)
    {
        shellTask(arg);
        osTaskSleep(10);
    }
    return NULL;
}

int shellIOInit()
{
    sShell.read = shellRead;
    sShell.write = shellWrite;
    shellInit(&sShell, sShellBuffer, 1024);
    shellSetPath(&sShell, gShellPathBuffer);
    os_tid_t tid;
    osTaskCreate(&tid, _shellTask, &sShell, "shell", 0, 0);
    system("stty -echo");
    system("stty -icanon");
    return 0;
}
