#include "shellio.h"
#include "shell.h"
#include "sys_task.h"
#include <stdlib.h>
#include <stdio.h>
static Shell s_shell;
static char s_shell_buffer[1024];
char g_shell_path_buffer[SYS_MAX_FILE_PATH_LENGTH] = {'/', '\0'};
static short shell_read(char *data, unsigned short len)
{
    int value = getchar();
    if (value != EOF)
    {
        data[0] = (char)value;
        return 1;
    }
    return 0;
}

static short shell_write(char *data, unsigned short len)
{
    for (unsigned short i = 0; i < len; i++)
    {
        putchar(data[i]);
    }
    fflush(stdout);
    return len;
}

static void *shell_task(void *arg)
{
    while (1)
    {
        shellTask(arg);
        sys_task_sleep(10);
    }
    return NULL;
}

int shell_io_init()
{
    s_shell.read = shell_read;
    s_shell.write = shell_write;
    shellInit(&s_shell, s_shell_buffer, 1024);
    shellSetPath(&s_shell, g_shell_path_buffer);
    sys_tid_t tid;
    sys_task_create(&tid, shell_task, &s_shell, "shell", SYS_DEFAULT_TASK_PRIORITY, 4096 * 1024);
    system("stty -echo");
    system("stty -icanon");
    return 0;
}
