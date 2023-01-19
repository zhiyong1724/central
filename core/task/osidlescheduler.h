#ifndef __OSIDLESCHEDULER_H__
#define __OSIDLESCHEDULER_H__
#include "osdefine.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct OsIdleTaskControlBlock
{
    os_size_t priority;
} OsIdleTaskControlBlock;

typedef struct OsIdleScheduler
{
    OsIdleTaskControlBlock *runningTask;
} OsIdleScheduler;
/*********************************************************************************************************************
* OsIdleScheduler初始化
* idleScheduler：OsIdleScheduler对象
* return：0：初始化成功
*********************************************************************************************************************/
int osIdleSchedulerInit(OsIdleScheduler *idleScheduler);
/*********************************************************************************************************************
* OsIdleTaskControlBlock初始化
* idleTaskControlBlock：OsIdleTaskControlBlock对象
* priority：优先级
* return：0：初始化成功
*********************************************************************************************************************/
int osIdleTaskControlBlockInit(OsIdleScheduler *idleScheduler, OsIdleTaskControlBlock *idleTaskControlBlock, os_size_t priority);
/*********************************************************************************************************************
* 时钟滴答
* idleScheduler：OsIdleScheduler对象
* ns：输入与上次tick的时间间隔，输出下次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsIdleTaskControlBlock *osIdleSchedulerTick(OsIdleScheduler *idleScheduler, uint64_t *ns);
/*********************************************************************************************************************
* 增加任务
* idleScheduler：OsIdleScheduler对象
* idleTaskControlBlock：任务控制块
* return：0:调用成功
*********************************************************************************************************************/
int osIdleSchedulerAddTask(OsIdleScheduler *idleScheduler, OsIdleTaskControlBlock *idleTaskControlBlock);
/*********************************************************************************************************************
* 移除任务
* idleScheduler：OsIdleScheduler对象
* idleTaskControlBlock：任务控制块
* return：返回下一个任务控制块
*********************************************************************************************************************/
OsIdleTaskControlBlock *osIdleSchedulerRemoveTask(OsIdleScheduler *idleScheduler, OsIdleTaskControlBlock *idleTaskControlBlock);
/*********************************************************************************************************************
* 修改优先级
* idleScheduler：OsIdleScheduler对象
* idleTaskControlBlock：任务控制块
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int osIdleSchedulerModifyPriority(OsIdleScheduler *idleScheduler, OsIdleTaskControlBlock *idleTaskControlBlock, os_size_t priority);
/*********************************************************************************************************************
* 获取当前运行的任务
* idleScheduler：OsIdleScheduler对象
*********************************************************************************************************************/
OsIdleTaskControlBlock *osIdleSchedulerGetRunningTask(OsIdleScheduler *idleScheduler);
/*********************************************************************************************************************
* 主动放弃运行
* idleScheduler：OsIdleScheduler对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsIdleTaskControlBlock *osIdleSchedulerYield(OsIdleScheduler *idleScheduler);
#ifdef __cplusplus
}
#endif
#endif