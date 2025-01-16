#include "shellio.h"
#include "shell.h"
#include "sys_task.h"
#include <stdlib.h>
#include <stdio.h>
#include "cregex.h"
#include <string.h>
static Shell s_shell;
static char s_shell_buffer[1024];
static char s_shell_path_buffer[VFS_MAX_FILE_PATH_LEN] = "/";

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
    shellSetPath(&s_shell, s_shell_path_buffer);
    sys_tid_t tid;
    sys_task_create(&tid, shell_task, &s_shell, "shell", SYS_DEFAULT_TASK_PRIORITY, 4096 * 1024);
    system("stty -echo");
    system("stty -icanon");
    return 0;
}

const char *get_shell_path()
{
    return s_shell_path_buffer;
}

static void dump_str(char *dest, const char *src, const cregex_match_t *match)
{
    sys_trace();
    int n = match->len < VFS_MAX_FILE_NAME_LEN ? match->len : VFS_MAX_FILE_NAME_LEN;
    memcpy(dest, &src[match->begin], n);
    dest[n] = '\0';
}

size_t pre_dentry_index(size_t index)
{
    for (; index >= 1; index--)
    {
        if ('/' == s_shell_path_buffer[index])
        {
            break;
        }
    }
    return index;
}

void set_shell_path(const char *path)
{
    s_shell_path_buffer[0] = '\0';
    size_t index = 0;
    cregex_t *regex = cregex_compile("/+([^/]+)");
    if (NULL == regex)
    {
        sys_error("Out of memory.");
        goto exception;
    }
    cregex_match_t matchs[2];
    while (cregex_search(regex, path, matchs, 2, 0) == 0)
    {
        char name[VFS_MAX_FILE_NAME_LEN] = {'\0'};
        dump_str(name, path, &matchs[1]);
        if (strcmp(name, ".") == 0)
        {
            continue;
        }
        else if (strcmp(name, "..") == 0)
        {
            index = pre_dentry_index(index);
            s_shell_path_buffer[index] = '\0';
        }
        else
        {
            strcat(&s_shell_path_buffer[index++], "/");
            strcat(&s_shell_path_buffer[index], name);
            index += strlen(name);
        }
    }
    goto finally;
exception:
finally:
    if (regex != NULL)
    {
        cregex_free(regex);
    }
}
