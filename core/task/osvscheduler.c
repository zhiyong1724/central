#include "osvscheduler.h"
#define ENABLE_VSCHEDULER_LOG 0
#if ENABLE_VSCHEDULER_LOG
#define vSchedulerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define vSchedulerLog(format, ...) (void)0
#endif
int osVSchedulerInit(OsVScheduler *vScheduler, os_size_t clockPeriod)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    for (os_size_t i = 0; i < MAX_SCHEDULER_COUNT; i++)
    {
        vScheduler->schedulers[i] = NULL;
    }
    vScheduler->schedulerCount = 0;
    vScheduler->clockPeriod = clockPeriod;
    vScheduler->suspendedList = NULL;
    vScheduler->sleepTree = NULL;
    vScheduler->runningTask = NULL;
    vScheduler->minSleepTime = 0;
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
    osAssert(vScheduler->schedulerCount < MAX_SCHEDULER_COUNT);
    if (vScheduler->schedulerCount < MAX_SCHEDULER_COUNT)
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
    osAssert(taskControlBlock->schedulerId < MAX_SCHEDULER_COUNT);
    if (taskControlBlock->schedulerId < MAX_SCHEDULER_COUNT)
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

int osVSchedulerModifyPriority(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock, os_size_t priority)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(vScheduler->schedulerCount < MAX_SCHEDULER_COUNT);
    if (taskControlBlock->schedulerId < MAX_SCHEDULER_COUNT)
    {
        return vScheduler->schedulerInterfaces[taskControlBlock->schedulerId].modifyPriority(vScheduler->schedulers[taskControlBlock->schedulerId], taskControlBlock + 1, priority);
    }
    return -1;
}

static void sleepTreeTick(OsVScheduler *vScheduler)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (vScheduler->minSleepTask != NULL)
    {
        vScheduler->minSleepTime += vScheduler->clockPeriod;
        for (;;)
        {
            if (vScheduler->minSleepTask != NULL && vScheduler->minSleepTask->sleepTime == vScheduler->minSleepTime)
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
    }
}

OsTaskControlBlock *osVSchedulerTick(OsVScheduler *vScheduler)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sleepTreeTick(vScheduler);
    for (os_size_t i = 0; i < vScheduler->schedulerCount; i++)
    {
        vScheduler->runningTask = (OsTaskControlBlock *)vScheduler->schedulerInterfaces[i].tick(vScheduler->schedulers[i]);
        if (vScheduler->runningTask != NULL)
        {
            vScheduler->runningTask--;
            break;
        }
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
            for (os_size_t i = 0; i < vScheduler->schedulerCount; i++)
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
        if (NULL == vScheduler->runningTask)
        {
            vScheduler->runningTask = taskControlBlock;
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
    if (task1->sleepTime - taskManager->minSleepTime < task2->sleepTime - taskManager->minSleepTime)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

OsTaskControlBlock *osVSchedulerSleep(OsVScheduler *vScheduler, os_size_t ns)
{
    vSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(vScheduler->runningTask != NULL);
    if (vScheduler->runningTask != NULL)
    {
        vScheduler->schedulerInterfaces[vScheduler->runningTask->schedulerId].removeTask(vScheduler->schedulers[vScheduler->runningTask->schedulerId], vScheduler->runningTask + 1);
        vScheduler->runningTask->sleepTime = vScheduler->minSleepTime + ns / vScheduler->clockPeriod * vScheduler->clockPeriod + vScheduler->clockPeriod;
        osInsertNode(&vScheduler->sleepTree, &vScheduler->runningTask->node.treeNode, onCompare, vScheduler);
        vScheduler->runningTask->taskState = OS_TASK_STATE_SLEEP;
        if (NULL == vScheduler->minSleepTask || vScheduler->runningTask->sleepTime - vScheduler->minSleepTime < vScheduler->minSleepTask->sleepTime - vScheduler->minSleepTime)
        {
            vScheduler->minSleepTask = vScheduler->runningTask;
        }

        for (os_size_t i = 0; i < vScheduler->schedulerCount; i++)
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
        if (NULL == vScheduler->runningTask)
        {
            vScheduler->runningTask = taskControlBlock;
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
        for (os_size_t i = 0; i < vScheduler->schedulerCount; i++)
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