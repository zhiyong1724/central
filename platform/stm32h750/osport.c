#include "osport.h"
#include "osplatformdef.h"
#include "stm32h7xx_hal.h"
#include "ostask.h"
#include "stm32h7xx_hal_cortex.h"
static void **sRunningTask = NULL;
int portInitializeStack(void **stackTop, os_size_t stackSize, os_size_t *taskStackMagic, TaskFunction taskFunction, void *arg)
{
    os_size_t *stack = (os_size_t *)(*stackTop);
    stack--;
    *stack = 0x01000000;     /* xPSR */
    stack--;
    *stack = (os_size_t)taskFunction;     /* PC */
    stack--;
    *stack = (os_size_t)osTaskExit;     /* LR */
    stack -= 5;	/* R12, R3, R2 and R1. */
    *stack = (os_size_t) arg;	/* R0 */
    return 0;
}

int portStartScheduler(void **stackTop)
{
    if (stackTop != sRunningTask)
    {
        sRunningTask = stackTop;
        HAL_NVIC_SetPendingIRQ(PendSV_IRQn);
    }
    return 0;
}

int portYield(void **stackTop)
{
    if (stackTop != sRunningTask)
    {
        sRunningTask = stackTop;
        HAL_NVIC_SetPendingIRQ(PendSV_IRQn);
    }
    return 0;
}

void PendSV_Handler(void)
{
    __asm volatile(
        "	ldr	r3, currentTask		        \n"
        "	ldr r1, [r3]					\n"
        "	ldr r0, [r1]					\n"
        "	msr msp, r0						\n"
        "	isb								\n"
        "	bx r14							\n"
        "currentTask: .word sRunningTask	\n");
}

static os_size_t sInterruptFlag = 1;
os_size_t portDisableInterrupts()
{
    __disable_irq();
    os_size_t ret = sInterruptFlag;
    sInterruptFlag = 0;
    return ret;
}

int portRecoveryInterrupts(os_size_t state)
{  
    if (1 == state)
    {
        sInterruptFlag = state;
        __enable_irq();
    }
    return 0;
}

