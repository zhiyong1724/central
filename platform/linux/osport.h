#ifndef __OSPORT_H__
#define __OSPORT_H__
#include "ostask.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
int portInitializeStack(stack_size_t **stackTop, size_t stackSize, stack_size_t *taskStackMagic, TaskFunction taskFunction, void *arg);
int portStartScheduler(stack_size_t **stackTop);
int portYield(stack_size_t **stackTop);

size_t portDisableInterrupts();
int portRecoveryInterrupts(size_t state);
#ifdef __cplusplus
}
#endif
#endif