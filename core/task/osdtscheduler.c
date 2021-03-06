#include "osdtscheduler.h"
#include "osstring.h"
#define ENABLE_DT_SCHEDULER_LOG 0
#if ENABLE_DT_SCHEDULER_LOG
#define dtSchedulerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define dtSchedulerLog(format, ...) (void)0
#endif
#define DTSCHED_MAX_PRIORITY                     40
static const os_size_t sVRunTimeTable[] = 
{
    10,    12,    15,    18,    22,    27,    33,     40,                    //0-7
    49,    60,    73,    89,    109,   133,   162,    197,                   //8-15
    241,   294,   358,   437,   534,   651,   794,    969,                   //16-23
    1182,  1442,  1759,  2146,  2619,  3195,  3898,   4755,                  //24-31
    5801,  7077,  8634,  10534, 12852, 15679, 19128,  23336,                 //32-39
};

int osDtSchedulerInit(OsDtScheduler *dtScheduler, os_size_t clockPeriod)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    dtScheduler->interval = 0;
    dtScheduler->minVRunTime = 0;
    dtScheduler->taskCount = 0;
    dtScheduler->taskTree = NULL;
    dtScheduler->runningTask = NULL;
    dtScheduler->skipTick = 0;
    dtScheduler->clockPeriod = clockPeriod;
    dtScheduler->switchInterval = 0;
    return 0;
}

int osDtTaskControlBlockInit(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock, os_size_t priority)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    dtTaskControlBlock->priority = priority;
    dtTaskControlBlock->vRunTime = dtScheduler->minVRunTime + sVRunTimeTable[dtTaskControlBlock->priority] + (sVRunTimeTable[dtTaskControlBlock->priority] >> 1);
    return 0;
}

static int onCompare(void *key1, void *key2, void *arg)
{
	dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsDtScheduler *dtScheduler = (OsDtScheduler *)arg;
    OsDtTaskControlBlock *task1 = (OsDtTaskControlBlock *)key1;
    OsDtTaskControlBlock *task2 = (OsDtTaskControlBlock *)key2;
    if (task1->vRunTime - dtScheduler->minVRunTime < task2->vRunTime - dtScheduler->minVRunTime)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

OsDtTaskControlBlock *osDtSchedulerTick(OsDtScheduler *dtScheduler)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (dtScheduler->taskCount > 0)
    {
        dtScheduler->interval += dtScheduler->clockPeriod;
        if (dtScheduler->interval >= dtScheduler->switchInterval)
        {
            if (0 == dtScheduler->skipTick)
            {
                dtScheduler->runningTask->vRunTime += sVRunTimeTable[dtScheduler->runningTask->priority];
                osDeleteNode(&dtScheduler->taskTree, &dtScheduler->runningTask->node);
                osInsertNode(&dtScheduler->taskTree, &dtScheduler->runningTask->node, onCompare, dtScheduler);
                dtScheduler->runningTask = (OsDtTaskControlBlock *)osGetLeftmostNode(dtScheduler->taskTree);
                dtScheduler->minVRunTime = dtScheduler->runningTask->vRunTime;
            }
            else
            {
                dtScheduler->skipTick = 0;
            }
            dtScheduler->interval = 0;
        }
    }
    return dtScheduler->runningTask;
}

int osDtSchedulerAddTask(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(dtTaskControlBlock->priority < DTSCHED_MAX_PRIORITY);
    if (dtTaskControlBlock->priority < DTSCHED_MAX_PRIORITY)
    {
        if(dtTaskControlBlock->vRunTime - dtScheduler->minVRunTime > (sVRunTimeTable[DTSCHED_MAX_PRIORITY - 1] << 1))
        {
            dtTaskControlBlock->vRunTime = dtScheduler->minVRunTime;
        }
        if (NULL == dtScheduler->runningTask)
        {
            dtScheduler->runningTask = dtTaskControlBlock;
        }
        osInsertNode(&dtScheduler->taskTree, &dtTaskControlBlock->node, onCompare, dtScheduler);
        dtScheduler->taskCount++;
        dtScheduler->switchInterval = DTSCHED_MAX_SCHED_CYCLE_NS / dtScheduler->taskCount;
        if (dtScheduler->switchInterval < DTSCHED_MIN_SWITCH_INTERVAL_NS)
        {
            dtScheduler->switchInterval = DTSCHED_MIN_SWITCH_INTERVAL_NS;
        }
        ret = 0;
    }
    return ret;
}

OsDtTaskControlBlock *osDtSchedulerRemoveTask(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osAssert(dtScheduler->taskCount > 0);
    if (dtScheduler->taskCount > 0)
    {
        osDeleteNode(&dtScheduler->taskTree, &dtTaskControlBlock->node);
        dtScheduler->taskCount--;
        if (dtScheduler->taskCount > 0)
        {
            dtScheduler->switchInterval = DTSCHED_MAX_SCHED_CYCLE_NS / dtScheduler->taskCount;
            if (dtScheduler->switchInterval < DTSCHED_MIN_SWITCH_INTERVAL_NS)
            {
                dtScheduler->switchInterval = DTSCHED_MIN_SWITCH_INTERVAL_NS;
            }
        }
        else
        {
            dtScheduler->switchInterval = 0;
        }

        if (dtScheduler->runningTask == dtTaskControlBlock)
        {
            dtScheduler->runningTask->vRunTime += sVRunTimeTable[dtScheduler->runningTask->priority] >> 1;
            if (0 == dtScheduler->taskCount)
            {
                dtScheduler->runningTask = NULL;
            }
            else
            {
                dtScheduler->runningTask = (OsDtTaskControlBlock *)osGetLeftmostNode(dtScheduler->taskTree);
                dtScheduler->minVRunTime = dtScheduler->runningTask->vRunTime;
                dtScheduler->skipTick = 1;
            }
        }
    }
    return dtScheduler->runningTask;
}

int osDtSchedulerModifyPriority(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock, os_size_t priority)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(priority < DTSCHED_MAX_PRIORITY);
    if (priority < DTSCHED_MAX_PRIORITY)
    {
        dtTaskControlBlock->priority = priority;
        ret = 0;
    }
    return ret;
}

OsDtTaskControlBlock *osDtSchedulerGetRunningTask(OsDtScheduler *dtScheduler)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return dtScheduler->runningTask;
}

OsDtTaskControlBlock *osDtSchedulerYield(OsDtScheduler *dtScheduler)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    dtScheduler->runningTask->vRunTime += sVRunTimeTable[dtScheduler->runningTask->priority] >> 1;
    osDeleteNode(&dtScheduler->taskTree, &dtScheduler->runningTask->node);
    osInsertNode(&dtScheduler->taskTree, &dtScheduler->runningTask->node, onCompare, dtScheduler);
    OsDtTaskControlBlock *nextTask = (OsDtTaskControlBlock *)osGetLeftmostNode(dtScheduler->taskTree);
    if (nextTask != dtScheduler->runningTask)
    {
        dtScheduler->runningTask = nextTask;
        dtScheduler->minVRunTime = dtScheduler->runningTask->vRunTime;
        dtScheduler->skipTick = 1;
    }
    return dtScheduler->runningTask;
}
