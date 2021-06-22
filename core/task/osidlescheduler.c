#include "osidlescheduler.h"
#define ENABLE_IDLECHEDULER_LOG 0
#if ENABLE_IDLECHEDULER_LOG
#define idlechedulerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define idlechedulerLog(format, ...) (void)0
#endif
int osIdleSchedulerInit(OsIdleScheduler *idleScheduler, os_size_t clockPeriod)
{
    idlechedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    idleScheduler->runningTask = NULL;
    return 0;
}

int osIdleTaskControlBlockInit(OsIdleScheduler *idleScheduler, OsIdleTaskControlBlock *idleTaskControlBlock, os_size_t priority)
{
    idlechedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    idleTaskControlBlock->priority = priority;
    return 0;
}

OsIdleTaskControlBlock *osIdleSchedulerTick(OsIdleScheduler *idleScheduler)
{
    idlechedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return idleScheduler->runningTask;
}

int osIdleSchedulerAddTask(OsIdleScheduler *idleScheduler, OsIdleTaskControlBlock *idleTaskControlBlock)
{
    idlechedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    idleScheduler->runningTask = idleTaskControlBlock;
    return 0;
}

OsIdleTaskControlBlock *osIdleSchedulerRemoveTask(OsIdleScheduler *idleScheduler, OsIdleTaskControlBlock *idleTaskControlBlock)
{
    idlechedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (idleScheduler->runningTask == idleTaskControlBlock)
    {
        idleScheduler->runningTask = NULL;
    }
    return idleScheduler->runningTask;
}

int osIdleSchedulerModifyPriority(OsIdleScheduler *idleScheduler, OsIdleTaskControlBlock *idleTaskControlBlock, os_size_t priority)
{
    idlechedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    idleTaskControlBlock->priority = priority;
    return 0;
}

OsIdleTaskControlBlock *osIdleSchedulerGetRunningTask(OsIdleScheduler *idleScheduler)
{
    idlechedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return idleScheduler->runningTask;
}

OsIdleTaskControlBlock *osIdleSchedulerYield(OsIdleScheduler *idleScheduler)
{
    idlechedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return idleScheduler->runningTask;
}