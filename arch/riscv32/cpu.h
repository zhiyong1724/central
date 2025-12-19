#ifndef __CPU_H__
#define __CPU_H__
#include "sys_task.h"
#include "sys_cpu_thread.h"
struct cpu_t
{
    stack_size_t **cur;
    stack_size_t **next;
    sys_cpu_thread_t cpu_thread;
};

void cpu_initialize_stack(stack_size_t **stack_top, void *(*task_function)(void *arg), void *arg);
void cpu_start_thread(stack_size_t **stack_top, struct cpu_t *cpu);
void cpu_yield(stack_size_t **stack_top);
#endif