#include "shellio.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include "ostask.h"
static Shell sShell;
static char sShellBuffer[512];
static short shellRead(char *data, unsigned short len)
{
    system("stty -echo");
    system("stty -icanon");
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

int shellIOInit()
{
    sShell.read = shellRead;
    sShell.write = shellWrite;
    shellInit(&sShell, sShellBuffer, 512);
    os_tid_t tid;
    osTaskCreate(&tid, (TaskFunction)shellTask, &sShell, "shell task", 0, 0);
    return 0;
}

void shellKeyTest(void)
{
    char data;
    while (1)
    {
        if (sShell.read(&data, 1) == 1)
        {
            if (data == '\n' || data == '\r')
            {
                return;
            }
            shellPrint(&sShell, "%02x ", data);
        }
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), key, shellKeyTest, shellKeyTest);