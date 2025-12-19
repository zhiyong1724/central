#include <stdio.h>
#include "sys_task.h"
#include "sys_mem.h"
#include "central.h"
#include <signal.h>
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
        printf("tid: %d  ptid: %d  cpu: %d  stack size: %d  state: %d  type: %d  priority: %d  name: %s\n",
                   task_info.tid, task_info.ptid, task_info.cpu, task_info.stack_size, task_info.task_state, task_info.task_type, task_info.priority, task_info.name);
        result = sys_task_find_next(&ptr, &task_info);
    }
}

void shell_uname(long argc, char *argv[])
{
    printf("Central V%s %s %s\n", sys_version(), __DATE__, __TIME__);
}

void shell_lscpu(long argc, char *argv[])
{
    int count = sys_task_get_cpu_thread_count();
    sys_cpu_info_t *infos = (sys_cpu_info_t *)sys_malloc(sizeof(sys_cpu_info_t) * count);
    if (NULL == infos)
    {
        printf("Out of memory.");
        return;
    }
    sys_task_get_cpu_info(infos, &count);
    for (size_t i = 0; i < count; i++)
    {
        printf("cpu name:    %s\n", infos[i].name);
        switch (infos[i].arch)
        {
        case SYS_CPU_ARCH_X86:
            printf("arch:        X86\n");
            break;
        case SYS_CPU_ARCH_X86_64:
            printf("arch:        X86_64\n");
            break;
        case SYS_CPU_ARCH_ARM:
            printf("arch:        ARM\n");
            break;
        case SYS_CPU_ARCH_ARM64:
            printf("arch:        ARM64\n");
            break;
        case SYS_CPU_ARCH_RISCV:
            printf("arch:        RISCV\n");
            break;
        default:
            printf("arch:        OTHER\n");
            break;
        }
        switch (infos[i].byte_order)
        {
        case SYS_CPU_BYTE_ORDER_LE:
            printf("byte order:  little endian\n");
            break;
        case SYS_CPU_BYTE_ORDER_BE:
            printf("byte order:  big endian\n");
            break;
        default:
            break;
        }
        printf("index:       %d\n", infos[i].index);
        if (infos[i].frequency >= 1000000000)
        {
            printf("frequency:   %.2lf GHz\n", (double)infos[i].frequency / 1000000000.0);
        }
        else if (infos[i].frequency >= 1000000)
        {
            printf("frequency:   %.2lf MHz\n", (double)infos[i].frequency / 1000000.0);
        }
        else if (infos[i].frequency >= 1000)
        {
            printf("frequency:   %.2lf KHz\n", (double)infos[i].frequency / 1000.0);
        }
        else
        {
            printf("frequency:   %lld Hz\n", (long long)infos[i].frequency);
        }
        printf("\n");
    }
    sys_free(infos);
}
