#include "shellio.h"
#include "shell.h"
#include "ostask.h"
#include <stdlib.h>
#include <stdio.h>
static Shell sShell;
static char sShellBuffer[1024];
char gShellPathBuffer[OS_MAX_FILE_PATH_LENGTH] = {'/', '\0'};
static short shellRead(char *data, unsigned short len)
{
    int value = getchar();
    if (value != EOF)
    {
        data[0] = (char)value;
        return 1;
    }
    return 0;
}

static short shellWrite(char *data, unsigned short len)
{
    for (unsigned short i = 0; i < len; i++)
    {
        putchar(data[i]);
    }
    fflush(stdout);
    return len;
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
    osTaskCreate(&tid, _shellTask, &sShell, "shell", 0, 4096 * 1024);
    system("stty -echo");
    system("stty -icanon");
    return 0;
}
