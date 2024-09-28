#include "osdtscheduler.h"
#include "osstring.h"
#define ENABLE_DT_SCHEDULER_LOG 0
#if ENABLE_DT_SCHEDULER_LOG
#define dtSchedulerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define dtSchedulerLog(format, ...) (void)0
#endif
#define OS_DTSCHED_MAX_PRIORITY                     40
static const size_t sWeightingTable[] = 
{
    10,    12,    15,    18,    22,    27,    33,     40,                    //0-7
    49,    60,    73,    89,    109,   133,   162,    197,                   //8-15
    241,   294,   358,   437,   534,   651,   794,    969,                   //16-23
    1182,  1442,  1759,  2146,  2619,  3195,  3898,   4755,                  //24-31
    5801,  7077,  8634,  10534, 12852, 15679, 19128,  23336,                 //32-39
};

int osDtSchedulerInit(OsDtScheduler *dtScheduler)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    dtScheduler->minVRunTime = 0;
    dtScheduler->taskCount = 0;
    dtScheduler->taskTree = NULL;
    dtScheduler->runningTask = NULL;
    dtScheduler->switchInterval = 0;
    dtScheduler->interval = 0; 
    return 0;
}

int osDtTaskControlBlockInit(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock, size_t priority)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    dtTaskControlBlock->priority = priority;
    dtTaskControlBlock->vRunTime = dtScheduler->minVRunTime;
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
    else if (task1->vRunTime == dtScheduler->minVRunTime - 1)
    {
        task1->vRunTime = dtScheduler->minVRunTime;
        return -1;
    }
    else
    {
        return 1;
    }
}

OsDtTaskControlBlock *osDtSchedulerTick(OsDtScheduler *dtScheduler, uint64_t *ns)
{
    //dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (dtScheduler->taskCount > 0)
    {
        dtScheduler->interval += *ns;
        if (dtScheduler->interval >= dtScheduler->switchInterval)
        {
            dtScheduler->runningTask->vRunTime += dtScheduler->interval * sWeightingTable[dtScheduler->runningTask->priority];
            osDeleteNode(&dtScheduler->taskTree, &dtScheduler->runningTask->node);
            osInsertNode(&dtScheduler->taskTree, &dtScheduler->runningTask->node, onCompare, dtScheduler);
            dtScheduler->runningTask = (OsDtTaskControlBlock *)osGetLeftmostNode(dtScheduler->taskTree);
            dtScheduler->minVRunTime = dtScheduler->runningTask->vRunTime;
            dtScheduler->interval = 0;
            *ns = dtScheduler->switchInterval;
        }
        else
        {
            *ns = dtScheduler->switchInterval - dtScheduler->interval;
        }
    }
    else
    {
        *ns = -1;
    }
    return dtScheduler->runningTask;
}

int osDtSchedulerAddTask(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(dtTaskControlBlock->priority < OS_DTSCHED_MAX_PRIORITY);
    if (dtTaskControlBlock->priority < OS_DTSCHED_MAX_PRIORITY)
    {
        if(dtTaskControlBlock->vRunTime - dtScheduler->minVRunTime > OS_DTSCHED_MAX_SCHED_CYCLE_NS * sWeightingTable[dtTaskControlBlock->priority])
        {
            dtTaskControlBlock->vRunTime = dtScheduler->minVRunTime - 1;
        }
        if (NULL == dtScheduler->runningTask)
        {
            dtScheduler->runningTask = dtTaskControlBlock;
        }
        osInsertNode(&dtScheduler->taskTree, &dtTaskControlBlock->node, onCompare, dtScheduler);
        dtScheduler->taskCount++;
        dtScheduler->switchInterval = OS_DTSCHED_MAX_SCHED_CYCLE_NS / dtScheduler->taskCount;
        if (dtScheduler->switchInterval < OS_DTSCHED_MIN_SWITCH_INTERVAL_NS)
        {
            dtScheduler->switchInterval = OS_DTSCHED_MIN_SWITCH_INTERVAL_NS;
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
            dtScheduler->switchInterval = OS_DTSCHED_MAX_SCHED_CYCLE_NS / dtScheduler->taskCount;
            if (dtScheduler->switchInterval < OS_DTSCHED_MIN_SWITCH_INTERVAL_NS)
            {
                dtScheduler->switchInterval = OS_DTSCHED_MIN_SWITCH_INTERVAL_NS;
            }
        }
        else
        {
            dtScheduler->switchInterval = 0;
        }

        if (dtScheduler->runningTask == dtTaskControlBlock)
        {
            dtScheduler->runningTask->vRunTime += dtScheduler->switchInterval * sWeightingTable[dtTaskControlBlock->priority] / 2;
            if (0 == dtScheduler->taskCount)
            {
                dtScheduler->runningTask = NULL;
            }
            else
            {
                dtScheduler->runningTask = (OsDtTaskControlBlock *)osGetLeftmostNode(dtScheduler->taskTree);
                dtScheduler->minVRunTime = dtScheduler->runningTask->vRunTime;
            }
        }
    }
    return dtScheduler->runningTask;
}

int osDtSchedulerModifyPriority(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock, size_t priority)
{
    dtSchedulerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(priority < OS_DTSCHED_MAX_PRIORITY);
    if (priority < OS_DTSCHED_MAX_PRIORITY)
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
    if (dtScheduler->taskCount > 0)
    {
        dtScheduler->runningTask->vRunTime += dtScheduler->switchInterval * sWeightingTable[dtScheduler->runningTask->priority] / 2;
        osDeleteNode(&dtScheduler->taskTree, &dtScheduler->runningTask->node);
        osInsertNode(&dtScheduler->taskTree, &dtScheduler->runningTask->node, onCompare, dtScheduler);
        OsDtTaskControlBlock *nextTask = (OsDtTaskControlBlock *)osGetLeftmostNode(dtScheduler->taskTree);
        dtScheduler->minVRunTime = nextTask->vRunTime;
        if (nextTask != dtScheduler->runningTask)
        {
            dtScheduler->runningTask = nextTask;
        }
    }
    return dtScheduler->runningTask;
}
