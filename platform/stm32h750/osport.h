#ifndef __OSPORT_H__
#define __OSPORT_H__
#include "sys_task.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
int port_initialize_stack(stack_size_t **stack_top, size_t stack_size, stack_size_t *task_stack_magic, task_function_t task_function, void *arg);
int port_start_scheduler(stack_size_t **stack_top);
int port_yield(stack_size_t **stack_top);

size_t port_disable_interrupts();
int port_recovery_interrupts(size_t state);
#ifdef __cplusplus
}
#endif
#endif