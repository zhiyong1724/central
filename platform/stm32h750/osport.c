#include "sys_port.h"
#include "sys_external_cfg.h"
#include "stm32h7xx_hal.h"
#include "sys_task.h"
#include "stm32h7xx_hal_cortex.h"
#include <reent.h>
#include "sys_string.h"
#define NVIC_INT_CTRL_REG (*((volatile uint32_t *)0xe000ed04))
#define NVIC_PENDSVSET_BIT (1UL << 28UL)
static stack_size_t **sPreTask = NULL;
static stack_size_t **s_runningTask = NULL;
static struct _reent sReent = _REENT_INIT (sReent);
int port_initialize_stack(stack_size_t **stack_top, size_t stack_size, stack_size_t *task_stack_magic, task_function_t task_function, void *arg)
{
    stack_size_t **stack = (stack_size_t **)stack_top;     
    (*stack) -= sizeof(struct _reent) / sizeof(stack_size_t);    //给C库全局变量预留空间
    (*stack)--;
    sys_memcpy(*stack, &sReent, sizeof(struct _reent));
    if ((stack_size_t)*stack % 8 > 0)                            //8字节对齐
    {
        (*stack)--;
    }
    (*stack)--;
    **stack = 0x01000000;     /* xPSR */
    (*stack)--;
    **stack = (stack_size_t)task_function;     /* PC */
    (*stack)--;
    **stack = (stack_size_t)sys_task_exit;     /* LR */
    (*stack) -= 5;	/* R12, R3, R2 and R1. */
    **stack = (stack_size_t) arg;	/* R0 */
    (*stack)--;
	**stack = 0xfffffffd;       //EXC_RETURN

	(*stack) -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */
    return 0;
}

int port_start_scheduler(stack_size_t **stack_top)
{
    if (stack_top != s_runningTask)
    {
        stack_size_t **stack_start = (stack_size_t **)stack_top - 1;
        stack_size_t *stack_size = (stack_size_t *)stack_top + 1;
        stack_size_t *stackEnd = *stack_start + *stack_size / sizeof(stack_size_t);
        stackEnd -= sizeof(struct _reent) / sizeof(stack_size_t);
        stackEnd--;
        _REENT = (struct _reent *)stackEnd;
        sPreTask = s_runningTask;
        s_runningTask = stack_top;
        NVIC_INT_CTRL_REG = NVIC_PENDSVSET_BIT;
    }
    return 0;
}

int port_yield(stack_size_t **stack_top)
{
    if (stack_top != s_runningTask)
    {
        stack_size_t **stack_start = (stack_size_t **)stack_top - 1;
        stack_size_t *stack_size = (stack_size_t *)stack_top + 1;
        stack_size_t *stackEnd = *stack_start + *stack_size / sizeof(stack_size_t);
        stackEnd -= sizeof(struct _reent) / sizeof(stack_size_t);
        stackEnd--;
        _REENT = (struct _reent *)stackEnd;
        sPreTask = s_runningTask;
        s_runningTask = stack_top;
        NVIC_INT_CTRL_REG = NVIC_PENDSVSET_BIT;
    }
    return 0;
}

void PendSV_Handler(void)
{
    __asm volatile(
        "   ldr r3, preTask                     \n"
        "   ldr r2, [r3]                        \n"
        "   cbz r2, restoreRegisters            \n"                //如果没有上一个任务，不需要保存寄存器
        "saveRegisters :                        \n"                //保存未硬件保存的寄存器
        "   mrs r0, psp							\n"                
        "   isb									\n"
        "   tst r14, #0x10						\n"                //判断是否使用了浮点寄存器
        "   it eq						        \n"                
        "   vstmdbeq r0!, {s16-s31}				\n"                //保存浮点寄存器
        "   stmdb r0!, {r4-r11, r14}			\n"                //保存普通寄存器
        "   ldr r3, preTask                     \n"                //获取上一个任务堆栈指针地址
        "   ldr r2, [r3]			            \n"
        "   str r0, [r2]			            \n"
        "                                       \n"
        "restoreRegisters :                     \n"                //恢复寄存器
        "   ldr r3, running_task			        \n"
        "   ldr r2, [r3]			            \n"
        "   ldr r0, [r2]			            \n"
        "   ldmia r0!, {r4-r11, r14}			\n"
        "   tst r14, #0x10			            \n"
        "   it eq			                    \n"
        "   vldmiaeq r0!, {s16-s31}			    \n"
        "   msr psp, r0			                \n"
        "   isb			                        \n"
        "                                       \n"
        "   bx r14                              \n"
        "preTask: .word sPreTask                \n"
        "running_task: .word s_runningTask	    \n");        
}

static size_t sInterruptFlag = 1;
size_t port_disable_interrupts()
{
    __disable_irq();
    size_t ret = sInterruptFlag;
    sInterruptFlag = 0;
    return ret;
}

int port_recovery_interrupts(size_t state)
{  
    if (1 == state)
    {
        sInterruptFlag = state;
        __enable_irq();
    }
    return 0;
}

