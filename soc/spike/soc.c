#include "sys_external_cfg.h"
#include "cpu.h"
#include "uart16550.h"
#include "central.h"
#include <reent.h>
#include "sys_string.h"
static struct cpu_t cpu0;

static void cpu0_initialize_stack(stack_size_t **stack_top, int stack_size, void *(*task_function)(void *arg), void *arg)
{
    stack_size_t **stack = (stack_size_t **)stack_top;     
    (*stack) -= sizeof(struct _reent) / sizeof(stack_size_t);    //给C库全局变量预留空间
    (*stack)--;
    sys_memcpy(*stack, &_impure_data, sizeof(struct _reent));
    while ((stack_size_t)*stack % 16 > 0)                            //16字节对齐
    {
        (*stack)--;
    }
    cpu_initialize_stack(stack_top, task_function, arg);
}

static void cpu0_start_thread(stack_size_t **stack_top)
{
    cpu_start_thread(stack_top, &cpu0);
}

void cpu_task_switch_hook(stack_size_t **stack_top)
{
    stack_size_t **stack_start = (stack_size_t **)stack_top - 1;
    stack_size_t *stack_size = (stack_size_t *)stack_top + 1;
    stack_size_t *stackEnd = *stack_start + *stack_size / sizeof(stack_size_t);
    stackEnd -= sizeof(struct _reent) / sizeof(stack_size_t);
    stackEnd--;
    _REENT = (struct _reent *)stackEnd;
}

void soc_init()
{
    uart16550_init();
    sys_init();
    const sys_cpu_info_t cpu0_info =
        {
            .name = "RISCV32",
            .arch = SYS_CPU_ARCH_RISCV,
            .byte_order = SYS_CPU_BYTE_ORDER_LE,
            .index = 0,
            .frequency = 1000000000,
            .ability = 100,
        };
    const sys_cpu_operations_t cpu0_operations =
        {
            .initialize_stack = cpu0_initialize_stack,
            .start_thread = cpu0_start_thread,
            .yield = cpu_yield,
        };
    sys_cpu_thread_init(&cpu0.cpu_thread, &cpu0_info, &cpu0_operations);
    sys_task_add_cpu_thread(&cpu0.cpu_thread);
}