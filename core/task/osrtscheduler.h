#ifndef __OSRTSCHEDULER_H__
#define __OSRTSCHEDULER_H__
#include "osdefine.h"
#include "oslist.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define RTSCHED_MAX_PRIORITY                     64                 //实时任务最大优先级
typedef struct OsRtTaskControlBlock
{
    OsListNode node;
    os_size_t priority;
} OsRtTaskControlBlock;

typedef struct OsRtScheduler
{
    os_byte_t readyTaskTable[RTSCHED_MAX_PRIORITY / 8];
    os_byte_t readyGroupTable;
    os_size_t interval;
    OsListNode *taskListArray[RTSCHED_MAX_PRIORITY];
    os_size_t taskCount;
    OsRtTaskControlBlock *runningTask;
    os_size_t skipTick;
    os_size_t clockPeriod;
} OsRtScheduler;
/*********************************************************************************************************************
* OsRtScheduler初始化
* rtScheduler：OsRtScheduler对象
* clockPeriod：时钟周期NS
* return：0：初始化成功
*********************************************************************************************************************/
int osRtSchedulerInit(OsRtScheduler *rtScheduler, os_size_t clockPeriod);
/*********************************************************************************************************************
* OsRtTaskControlBlock初始化
* rtScheduler：OsRtScheduler对象
* rtTaskControlBlock：OsRtTaskControlBlock对象
* priority：优先级
* return：0：初始化成功
*********************************************************************************************************************/
int osRtTaskControlBlockInit(OsRtScheduler *rtScheduler, OsRtTaskControlBlock *rtTaskControlBlock, os_size_t priority);
/*********************************************************************************************************************
* 时钟滴答
* rtScheduler：OsRtScheduler对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsRtTaskControlBlock *osRtSchedulerTick(OsRtScheduler *rtScheduler);
/*********************************************************************************************************************
* 增加任务
* rtScheduler：OsRtScheduler对象
* rtTaskControlBlock：任务控制块
* return：0：添加成功
*********************************************************************************************************************/
int osRtSchedulerAddTask(OsRtScheduler *rtScheduler, OsRtTaskControlBlock *rtTaskControlBlock);
/*********************************************************************************************************************
* 移除任务
* rtScheduler：OsRtScheduler对象
* rtTaskControlBlock：任务控制块
* return：返回下一个任务控制块
*********************************************************************************************************************/
OsRtTaskControlBlock *osRtSchedulerRemoveTask(OsRtScheduler *rtScheduler, OsRtTaskControlBlock *rtTaskControlBlock);
/*********************************************************************************************************************
* 修改优先级
* rtScheduler：OsRtScheduler对象
* rtTaskControlBlock：任务控制块
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int osRtSchedulerModifyPriority(OsRtScheduler *rtScheduler, OsRtTaskControlBlock *rtTaskControlBlock, os_size_t priority);
/*********************************************************************************************************************
* 获取当前运行的任务
* rtScheduler：OsRtScheduler对象
*********************************************************************************************************************/
OsRtTaskControlBlock *osRtSchedulerGetRunningTask(OsRtScheduler *rtScheduler);
/*********************************************************************************************************************
* 主动放弃运行
* rtScheduler：OsRtScheduler对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsRtTaskControlBlock *osRtSchedulerYield(OsRtScheduler *rtScheduler);
#ifdef __cplusplus
}
#endif
#endif