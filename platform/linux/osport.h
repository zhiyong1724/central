#ifndef __OSPORT_H__
#define __OSPORT_H__
#include "ostask.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
int portInitializeStack(void **stackTop, os_size_t stackSize, os_size_t *taskStackMagic, TaskFunction taskFunction, void *arg);
int portStartScheduler(void **stackTop);
int portYield(void **stackTop);

os_size_t portDisableInterrupts();
int portEnableInterrupts();
int portEnableTaskInterrupts();
int portRecoveryInterrupts(os_size_t state);
uint64_t portGetPosixTime();
#ifdef __cplusplus
}
#endif
#endif