#include <stdio.h>
#include "sys_task.h"
#include "sys_mem.h"
#include "central.h"
void shell_free(long argc, char *argv[])
{
    printf("所有内存：%ld\n", sys_total_mem());
    printf("可用内存：%ld\n", sys_free_mem());
    printf("所有页：%ld\n", sys_total_page());
    printf("可用页：%ld\n", sys_free_page());
}

void shell_ps(long argc, char *argv[])
{
    sys_task_ptr ptr;
    sys_task_info_t task_info;
    int result = sys_task_find_first(&ptr, &task_info);
    while (0 == result)
    {
        printf("tid: %d  ptid: %d  stack: %p  stack size: %d  state: %d  type: %d  priority: %d  name: %s\n",
                   task_info.tid, task_info.ptid, task_info.stack, task_info.stack_size, task_info.task_state, task_info.task_type, task_info.priority, task_info.name);
        result = sys_task_find_next(&ptr, &task_info);
    }
}

void shell_uname(long argc, char *argv[])
{
    printf("Central V%s %s %s\n", sys_version(), __DATE__, __TIME__);
} 
