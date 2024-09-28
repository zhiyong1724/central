#include "osvscheduler.h"
#define ENABLE_VSCHEDULER_LOG 0
#if ENABLE_VSCHEDULER_LOG
#define vSchedulerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define vSchedulerLog(format, ...) (void)0
#endif
int osVSchedulerInit(OsVScheduler *vScheduler)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    for (size_t i = 0; i < OS_MAX_SCHEDULER_COUNT; i++)
    {
        vScheduler->schedulers[i] = NULL;
    }
    vScheduler->schedulerCount = 0;
    vScheduler->clock = 0;
    vScheduler->suspendedList = NULL;
    vScheduler->sleepTree = NULL;
    vScheduler->runningTask = NULL;
    vScheduler->minSleepTask = NULL;
    return 0;
}

int osTaskControlBlockInit(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    taskControlBlock->schedulerId = 0;
    taskControlBlock->taskState = OS_TASK_STATE_DELETED;
    taskControlBlock->sleepTime = 0;
    return 0;
}

int osVSchedulerAddScheduler(OsVScheduler *vScheduler, void *scheduler, OsSchedulerInterfaces *schedulerInterfaces)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(vScheduler->schedulerCount < OS_MAX_SCHEDULER_COUNT);
    if (vScheduler->schedulerCount < OS_MAX_SCHEDULER_COUNT)
    {
        vScheduler->schedulers[vScheduler->schedulerCount] = scheduler;
        vScheduler->schedulerInterfaces[vScheduler->schedulerCount] = *schedulerInterfaces;
        vScheduler->schedulerCount++;
        return 0;
    }
    return -1;
}

int osVSchedulerAddTask(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(taskControlBlock->schedulerId < OS_MAX_SCHEDULER_COUNT);
    if (taskControlBlock->schedulerId < OS_MAX_SCHEDULER_COUNT)
    {
        if (NULL == vScheduler->runningTask)
        {
            vScheduler->runningTask = taskControlBlock;
        }
        taskControlBlock->taskState = OS_TASK_STATE_READY;
        return vScheduler->schedulerInterfaces[taskControlBlock->schedulerId].addTask(vScheduler->schedulers[taskControlBlock->schedulerId], taskControlBlock + 1);
    }
    return -1;
}

int osVSchedulerModifyPriority(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock, size_t priority)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(vScheduler->schedulerCount < OS_MAX_SCHEDULER_COUNT);
    if (taskControlBlock->schedulerId < OS_MAX_SCHEDULER_COUNT)
    {
        return vScheduler->schedulerInterfaces[taskControlBlock->schedulerId].modifyPriority(vScheduler->schedulers[taskControlBlock->schedulerId], taskControlBlock + 1, priority);
    }
    return -1;
}

static void sleepTreeTick(OsVScheduler *vScheduler, uint64_t *ns)
{
    //vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (vScheduler->minSleepTask != NULL)
    {
        for (;;)
        {
            if (vScheduler->minSleepTask != NULL && vScheduler->minSleepTask->sleepTime - vScheduler->clock <= *ns)
            {
                osDeleteNode(&vScheduler->sleepTree, &vScheduler->minSleepTask->node.treeNode);
                vScheduler->minSleepTask->taskState = OS_TASK_STATE_READY;
                vScheduler->schedulerInterfaces[vScheduler->minSleepTask->schedulerId].addTask(vScheduler->schedulers[vScheduler->minSleepTask->schedulerId], vScheduler->minSleepTask + 1);
                vScheduler->minSleepTask = (OsTaskControlBlock *)osGetLeftmostNode(vScheduler->sleepTree);
            }
            else
            {
                break;
            }
        }
        vScheduler->clock += *ns;
        *ns = -1;
        if (vScheduler->minSleepTask != NULL)
        {
            *ns = vScheduler->minSleepTask->sleepTime - vScheduler->clock;
        }
    }
    else
    {
        *ns = -1;
    }
}

OsTaskControlBlock *osVSchedulerTick(OsVScheduler *vScheduler, size_t schedulerId, uint64_t *ns)
{
    //vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    uint64_t nextSleepTickInterval = *ns;
    sleepTreeTick(vScheduler, &nextSleepTickInterval);
    uint64_t nextTickInterval = *ns;
    if (schedulerId < OS_MAX_SCHEDULER_COUNT)
    {
        vScheduler->runningTask = (OsTaskControlBlock *)vScheduler->schedulerInterfaces[schedulerId].tick(vScheduler->schedulers[schedulerId], &nextTickInterval);
    }
    else
    {
        for (size_t i = 0; i < vScheduler->schedulerCount; i++)
        {
            vScheduler->runningTask = (OsTaskControlBlock *)vScheduler->schedulerInterfaces[i].tick(vScheduler->schedulers[i], &nextTickInterval);
            if (vScheduler->runningTask != NULL)
            {
                break;
            }
        }
    }
    *ns = nextSleepTickInterval < nextTickInterval ? nextSleepTickInterval : nextTickInterval;
    if (vScheduler->runningTask != NULL)
    {
        vScheduler->runningTask--;
    }
    return vScheduler->runningTask;
}

OsTaskControlBlock *osVSchedulerSupend(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(OS_TASK_STATE_READY == taskControlBlock->taskState || OS_TASK_STATE_SLEEP == taskControlBlock->taskState);
    if (OS_TASK_STATE_READY == taskControlBlock->taskState || OS_TASK_STATE_SLEEP == taskControlBlock->taskState)
    {
        switch (taskControlBlock->taskState)
        {
        case OS_TASK_STATE_READY:
            vScheduler->schedulerInterfaces[taskControlBlock->schedulerId].removeTask(vScheduler->schedulers[taskControlBlock->schedulerId], taskControlBlock + 1);
            break;
        case OS_TASK_STATE_SLEEP:
            osDeleteNode(&vScheduler->sleepTree, &taskControlBlock->node.treeNode);
            if (taskControlBlock == vScheduler->minSleepTask)
            {
                vScheduler->minSleepTask = (OsTaskControlBlock *)osGetLeftmostNode(vScheduler->sleepTree);
            }
            break;
        default:
            break;
        }

        osInsertToBack(&vScheduler->suspendedList, &taskControlBlock->node.listNode);
        taskControlBlock->taskState = OS_TASK_STATE_SUSPENDED;
        if (taskControlBlock == vScheduler->runningTask)
        {
            for (size_t i = 0; i < vScheduler->schedulerCount; i++)
            {
                vScheduler->runningTask = (OsTaskControlBlock *)vScheduler->schedulerInterfaces[i].getRunningTask(vScheduler->schedulers[i]);
                if (vScheduler->runningTask != NULL)
                {
                    vScheduler->runningTask--;
                    break;
                }
            }
        }
    }
    return vScheduler->runningTask;
}

OsTaskControlBlock *osVSchedulerResume(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(OS_TASK_STATE_SUSPENDED == taskControlBlock->taskState);
    if (OS_TASK_STATE_SUSPENDED == taskControlBlock->taskState)
    {
        osRemoveFromList(&vScheduler->suspendedList, &taskControlBlock->node.listNode);
        vScheduler->schedulerInterfaces[taskControlBlock->schedulerId].addTask(vScheduler->schedulers[taskControlBlock->schedulerId], taskControlBlock + 1);
        taskControlBlock->taskState = OS_TASK_STATE_READY;
        for (size_t i = 0; i < vScheduler->schedulerCount; i++)
        {
            vScheduler->runningTask = (OsTaskControlBlock *)vScheduler->schedulerInterfaces[i].yield(vScheduler->schedulers[i]);
            if (vScheduler->runningTask != NULL)
            {
                vScheduler->runningTask--;
                break;
            }
        }
    }
    return vScheduler->runningTask;
}

static int onCompare(void *key1, void *key2, void *arg)
{
	vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsVScheduler *taskManager = (OsVScheduler *)arg;
    OsTaskControlBlock *task1 = (OsTaskControlBlock *)key1;
    OsTaskControlBlock *task2 = (OsTaskControlBlock *)key2;
    if (task1->sleepTime - taskManager->clock < task2->sleepTime - taskManager->clock)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

OsTaskControlBlock *osVSchedulerSleep(OsVScheduler *vScheduler, uint64_t ns)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(vScheduler->runningTask != NULL);
    if (vScheduler->runningTask != NULL)
    {
        vScheduler->schedulerInterfaces[vScheduler->runningTask->schedulerId].removeTask(vScheduler->schedulers[vScheduler->runningTask->schedulerId], vScheduler->runningTask + 1);
        vScheduler->runningTask->sleepTime = vScheduler->clock + ns;
        osInsertNode(&vScheduler->sleepTree, &vScheduler->runningTask->node.treeNode, onCompare, vScheduler);
        vScheduler->runningTask->taskState = OS_TASK_STATE_SLEEP;
        if (NULL == vScheduler->minSleepTask || vScheduler->runningTask->sleepTime - vScheduler->clock < vScheduler->minSleepTask->sleepTime - vScheduler->clock)
        {
            vScheduler->minSleepTask = vScheduler->runningTask;
        }

        for (size_t i = 0; i < vScheduler->schedulerCount; i++)
        {
            vScheduler->runningTask = (OsTaskControlBlock *)vScheduler->schedulerInterfaces[i].getRunningTask(vScheduler->schedulers[i]);
            if (vScheduler->runningTask != NULL)
            {
                vScheduler->runningTask--;
                break;
            }
        }
    }
    return vScheduler->runningTask;
}

OsTaskControlBlock *osVSchedulerWakeup(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(OS_TASK_STATE_SLEEP == taskControlBlock->taskState);
    if (OS_TASK_STATE_SLEEP == taskControlBlock->taskState)
    {
        osDeleteNode(&vScheduler->sleepTree, &taskControlBlock->node.treeNode);
        taskControlBlock->taskState = OS_TASK_STATE_READY;
        vScheduler->schedulerInterfaces[taskControlBlock->schedulerId].addTask(vScheduler->schedulers[taskControlBlock->schedulerId], taskControlBlock + 1);
        vScheduler->minSleepTask = (OsTaskControlBlock *)osGetLeftmostNode(vScheduler->sleepTree);
        for (size_t i = 0; i < vScheduler->schedulerCount; i++)
        {
            vScheduler->runningTask = (OsTaskControlBlock *)vScheduler->schedulerInterfaces[i].yield(vScheduler->schedulers[i]);
            if (vScheduler->runningTask != NULL)
            {
                vScheduler->runningTask--;
                break;
            }
        }
    }
    return vScheduler->runningTask;
}

OsTaskControlBlock *osVSchedulerExit(OsVScheduler *vScheduler)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(vScheduler->runningTask != NULL);
    if (vScheduler->runningTask != NULL)
    {
        vScheduler->schedulerInterfaces[vScheduler->runningTask->schedulerId].removeTask(vScheduler->schedulers[vScheduler->runningTask->schedulerId], vScheduler->runningTask + 1);
        vScheduler->runningTask->taskState = OS_TASK_STATE_DELETED;
        for (size_t i = 0; i < vScheduler->schedulerCount; i++)
        {
            vScheduler->runningTask = (OsTaskControlBlock *)vScheduler->schedulerInterfaces[i].getRunningTask(vScheduler->schedulers[i]);
            if (vScheduler->runningTask != NULL)
            {
                vScheduler->runningTask--;
                break;
            }
        }
    }
    return vScheduler->runningTask;
}

OsTaskControlBlock *osVSchedulerGetRunningTask(OsVScheduler *vScheduler)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vScheduler->runningTask;
}