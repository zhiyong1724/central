#include "osrtscheduler.h"
#include "osstring.h"
#include "osbitmapindex.h"
#define ENABLE_RTSCHEDULER_LOG 0
#if ENABLE_RTSCHEDULER_LOG
#define rtSchedulerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define rtSchedulerLog(format, ...) (void)0
#endif
int osRtSchedulerInit(OsRtScheduler *rtScheduler, uint64_t clockPeriod)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMemSet(rtScheduler->readyTaskTable, 0xff, OS_RTSCHED_MAX_PRIORITY / 8);
    rtScheduler->readyGroupTable = 0xff;
    rtScheduler->interval = 0;
    for (os_size_t i = 0; i < OS_RTSCHED_MAX_PRIORITY; i++)
    {
        rtScheduler->taskListArray[i] = NULL;
    }
    rtScheduler->taskCount = 0;
    rtScheduler->runningTask = NULL;
    rtScheduler->skipTick = 0;
    rtScheduler->clockPeriod = clockPeriod;
    return 0;
}

int osRtTaskControlBlockInit(OsRtScheduler *rtScheduler, OsRtTaskControlBlock *rtTaskControlBlock, os_size_t priority)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    rtTaskControlBlock->priority = priority;
    return 0;
}

static os_size_t getMinimunPriority(OsRtScheduler *rtScheduler)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t i = gBitmapIndex[rtScheduler->readyGroupTable];
    os_size_t j = gBitmapIndex[rtScheduler->readyTaskTable[i]];
    return (i << 3) + j;
}

static void setBitmap(OsRtScheduler *rtScheduler, os_size_t priority, os_size_t value)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t i = priority >> 3;
    os_size_t j = priority & 0x07;
    os_byte_t mask = 0x80;
    mask >>= j;
    if (0 == value)
    {
        mask = ~mask;
        rtScheduler->readyTaskTable[i] &= mask;
    }
    else
    {
        rtScheduler->readyTaskTable[i] |= mask;
    }
    mask = 0x80;
    mask >>= i;
    if (rtScheduler->readyTaskTable[i] < 0xff)
    {
        mask = ~mask;
        rtScheduler->readyGroupTable &= mask;
    }
    else
    {
        rtScheduler->readyGroupTable |= mask;
    }
}

OsRtTaskControlBlock *osRtSchedulerTick(OsRtScheduler *rtScheduler)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (rtScheduler->taskCount > 0)
    {
        rtScheduler->interval += rtScheduler->clockPeriod;
        if (rtScheduler->interval >= OS_RTSCHED_MIN_SWITCH_INTERVAL_NS)
        {
            if (0 == rtScheduler->skipTick)
            {
                osRemoveFromList(&rtScheduler->taskListArray[rtScheduler->runningTask->priority], &rtScheduler->runningTask->node);
                osInsertToBack(&rtScheduler->taskListArray[rtScheduler->runningTask->priority], &rtScheduler->runningTask->node);
                os_size_t priority = getMinimunPriority(rtScheduler);
                rtScheduler->runningTask = (OsRtTaskControlBlock *)rtScheduler->taskListArray[priority];
            }
            else
            {
                rtScheduler->skipTick = 0;
            }
            rtScheduler->interval = 0;
        }
    }
    return rtScheduler->runningTask;
}

int osRtSchedulerAddTask(OsRtScheduler *rtScheduler, OsRtTaskControlBlock *rtTaskControlBlock)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(rtTaskControlBlock->priority < OS_RTSCHED_MAX_PRIORITY);
    if (rtTaskControlBlock->priority < OS_RTSCHED_MAX_PRIORITY)
    {
        setBitmap(rtScheduler, rtTaskControlBlock->priority, 0);
        osInsertToBack(&rtScheduler->taskListArray[rtTaskControlBlock->priority], &rtTaskControlBlock->node);
        rtScheduler->taskCount++;
        if (NULL == rtScheduler->runningTask)
        {
            rtScheduler->runningTask = rtTaskControlBlock;
        }
        ret = 0;
    }
    return ret;
}

OsRtTaskControlBlock *osRtSchedulerRemoveTask(OsRtScheduler *rtScheduler, OsRtTaskControlBlock *rtTaskControlBlock)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(rtScheduler->taskCount > 0);
    if (rtScheduler->taskCount > 0)
    {
        osRemoveFromList(&rtScheduler->taskListArray[rtTaskControlBlock->priority], &rtTaskControlBlock->node);
        if (NULL == rtScheduler->taskListArray[rtTaskControlBlock->priority])
        {
            setBitmap(rtScheduler, rtTaskControlBlock->priority, 1);
        }
        rtScheduler->taskCount--;
        if (rtScheduler->runningTask == rtTaskControlBlock)
        {
            if (0 == rtScheduler->taskCount)
            {
                rtScheduler->runningTask = NULL;
            }
            else
            {
                os_size_t priority = getMinimunPriority(rtScheduler);
                rtScheduler->runningTask = (OsRtTaskControlBlock *)rtScheduler->taskListArray[priority];
                rtScheduler->skipTick = 1;
            }
        }
    }
    return rtScheduler->runningTask;
}

int osRtSchedulerModifyPriority(OsRtScheduler *rtScheduler, OsRtTaskControlBlock *rtTaskControlBlock, os_size_t priority)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(priority < OS_RTSCHED_MAX_PRIORITY);
    if (priority < OS_RTSCHED_MAX_PRIORITY)
    {
        osRemoveFromList(&rtScheduler->taskListArray[rtTaskControlBlock->priority], &rtTaskControlBlock->node);
        if (NULL == rtScheduler->taskListArray[rtTaskControlBlock->priority])
        {
            setBitmap(rtScheduler, rtTaskControlBlock->priority, 1);
        }
        rtTaskControlBlock->priority = priority;
        setBitmap(rtScheduler, rtTaskControlBlock->priority, 0);
        osInsertToBack(&rtScheduler->taskListArray[rtTaskControlBlock->priority], &rtTaskControlBlock->node);
        ret = 0;
    }
    return ret;
}

OsRtTaskControlBlock *osRtSchedulerGetRunningTask(OsRtScheduler *rtScheduler)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return rtScheduler->runningTask;
}

OsRtTaskControlBlock *osRtSchedulerYield(OsRtScheduler *rtScheduler)
{
    rtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (rtScheduler->taskCount > 0)
    {
        osRemoveFromList(&rtScheduler->taskListArray[rtScheduler->runningTask->priority], &rtScheduler->runningTask->node);
        osInsertToBack(&rtScheduler->taskListArray[rtScheduler->runningTask->priority], &rtScheduler->runningTask->node);
        os_size_t priority = getMinimunPriority(rtScheduler);
        OsRtTaskControlBlock *nextTask = (OsRtTaskControlBlock *)rtScheduler->taskListArray[priority];
        if (nextTask != rtScheduler->runningTask)
        {
            rtScheduler->runningTask = nextTask;
            rtScheduler->skipTick = 1;
        }
    }
    return rtScheduler->runningTask;
}