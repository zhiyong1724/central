#include "resource_monitor.h"
#include "sys_task.h"
#include "sys_mem.h"
#include <string.h>
static int s_running = 0;
static sys_tid_t s_tid = 0;
static void *monitor_task(void *arg)
{
    while (s_running)
    {
        printf("\033[2J\033[0;0H");
        printf("cpu used: %d%%    mem used: %ld%%    task count: %d\n", sys_task_get_cpu_usage(), (sys_total_mem() - sys_free_mem()) * 100 / sys_total_mem(), sys_task_get_task_count());
        sys_task_ptr ptr;
        sys_task_info_t task_info;
        int result = sys_task_find_first(&ptr, &task_info);
        while (0 == result)
        {
            printf("tid: %d  ptid: %d  stack: %p  stack size: %d  state: %d  type: %d  priority: %d  name: %s\n",
                   task_info.tid, task_info.ptid, task_info.stack, task_info.stack_size, task_info.task_state, task_info.task_type, task_info.priority, task_info.name);
            result = sys_task_find_next(&ptr, &task_info);
        }
        sys_task_sleep(1000);
    }
    return NULL;
}

void resource_monitor_dump()
{
    s_running = 1;
    if (0 == s_tid)
    {
        sys_task_create(&s_tid, monitor_task, NULL, "resource monitor", SYS_DEFAULT_TASK_PRIORITY, 0);
    }
}

void resource_monitor_stop()
{
    if (s_tid != 0)
    {
        s_running = 0;
        void *ret;
        sys_task_join(&ret, s_tid);
        s_tid = 0;
    }
}

void shell_rs(long argc, char *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "stop") == 0)
    {
        resource_monitor_stop();
    }
    else
    {
        resource_monitor_dump();
    }
}