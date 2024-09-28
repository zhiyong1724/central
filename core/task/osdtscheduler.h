#ifndef __OSDTSCHEDULER_H__
#define __OSDTSCHEDULER_H__
#include "osdefine.h"
#include "ostree.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct OsDtTaskControlBlock
{
    OsTreeNode node;
    size_t priority;
    uint64_t vRunTime;
} OsDtTaskControlBlock;

typedef struct OsDtScheduler
{
    OsTreeNode *taskTree;
    size_t taskCount;
    uint64_t minVRunTime;
    OsDtTaskControlBlock *runningTask;
    uint64_t switchInterval;
    uint64_t interval;
} OsDtScheduler;
/*********************************************************************************************************************
* OsDtScheduler初始化
* dtScheduler：OsDtScheduler对象
* return：0：初始化成功
*********************************************************************************************************************/
int osDtSchedulerInit(OsDtScheduler *dtScheduler);
/*********************************************************************************************************************
* OsDtTaskControlBlock初始化
* dtTaskControlBlock：OsDtTaskControlBlock对象
* priority：优先级
* return：0：初始化成功
*********************************************************************************************************************/
int osDtTaskControlBlockInit(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock, size_t priority);
/*********************************************************************************************************************
* 时钟滴答
* dtScheduler：OsDtScheduler对象
* ns：输入与上次tick的时间间隔，输出下次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsDtTaskControlBlock *osDtSchedulerTick(OsDtScheduler *dtScheduler, uint64_t *ns);
/*********************************************************************************************************************
* 增加任务
* dtScheduler：OsDtScheduler对象
* dtTaskControlBlock：任务控制块
* return：0:调用成功
*********************************************************************************************************************/
int osDtSchedulerAddTask(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock);
/*********************************************************************************************************************
* 移除任务
* dtScheduler：OsDtScheduler对象
* dtTaskControlBlock：任务控制块
* return：返回下一个任务控制块
*********************************************************************************************************************/
OsDtTaskControlBlock *osDtSchedulerRemoveTask(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock);
/*********************************************************************************************************************
* 修改优先级
* dtScheduler：OsDtScheduler对象
* dtTaskControlBlock：任务控制块
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int osDtSchedulerModifyPriority(OsDtScheduler *dtScheduler, OsDtTaskControlBlock *dtTaskControlBlock, size_t priority);
/*********************************************************************************************************************
* 获取当前运行的任务
* dtScheduler：OsDtScheduler对象
*********************************************************************************************************************/
OsDtTaskControlBlock *osDtSchedulerGetRunningTask(OsDtScheduler *dtScheduler);
/*********************************************************************************************************************
* 主动放弃运行
* dtScheduler：OsDtScheduler对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsDtTaskControlBlock *osDtSchedulerYield(OsDtScheduler *dtScheduler);
#ifdef __cplusplus
}
#endif
#endif