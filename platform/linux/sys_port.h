#ifndef __SYS_PORT_H__
#define __SYS_PORT_H__
#include "sys_task.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
void port_initialize_stack(stack_size_t **stack_top, int stack_size, stack_size_t *task_stack_magic, task_function_t task_function, void *arg);
void port_start_scheduler(stack_size_t **stack_top);
void port_yield(stack_size_t **stack_top);

int port_disable_interrupts();
void port_recovery_interrupts(int state);
#ifdef __cplusplus
}
#endif
#endif