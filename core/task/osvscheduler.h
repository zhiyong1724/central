#ifndef __OSVSCHEDULER_H__
#define __OSVSCHEDULER_H__
#include "ostaskdef.h"
#include "oslist.h"
#include "ostree.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef void *(*Tick)(void *scheduler, uint64_t *ns);
typedef int (*AddTask)(void *scheduler, void *task);
typedef void *(*RemoveTask)(void *scheduler, void *task);
typedef int (*ModifyPriority)(void *scheduler, void *task, os_size_t priority);
typedef void *(*GetRunningTask)(void *scheduler);
typedef void *(*Yield)(void *scheduler);
typedef struct OsSchedulerInterfaces
{
    Tick tick;
    AddTask addTask;
    RemoveTask removeTask;
    ModifyPriority modifyPriority;
    GetRunningTask getRunningTask;
    Yield yield;
} OsSchedulerInterfaces;

typedef struct OsTaskControlBlock
{
    union
    {
        OsListNode listNode;
        OsTreeNode treeNode;
    } node;
    OsTaskState taskState;
    os_size_t schedulerId;
    uint64_t sleepTime;
} OsTaskControlBlock;

typedef struct OsVScheduler
{
    void *schedulers[OS_MAX_SCHEDULER_COUNT];
    OsSchedulerInterfaces schedulerInterfaces[OS_MAX_SCHEDULER_COUNT];
    os_size_t schedulerCount;
    uint64_t clock;
    OsListNode *suspendedList;
    OsTreeNode *sleepTree;
    OsTaskControlBlock *runningTask;
    OsTaskControlBlock *minSleepTask;
} OsVScheduler;
/*********************************************************************************************************************
* OsVScheduler初始化
* vScheduler：OsVScheduler对象
* return：0：初始化成功
*********************************************************************************************************************/
int osVSchedulerInit(OsVScheduler *vScheduler);
/*********************************************************************************************************************
* OsTaskControlBlock初始化
* vScheduler：OsVScheduler对象
* taskControlBlock：OsTaskControlBlock对象
* return：0：初始化成功
*********************************************************************************************************************/
int osTaskControlBlockInit(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock);
/*********************************************************************************************************************
* 添加调度器
* vScheduler：OsVScheduler对象
* scheduler：调度器对象
* schedulerInterfaces：调度器操作函数
* return：0：添加成功
*********************************************************************************************************************/
int osVSchedulerAddScheduler(OsVScheduler *vScheduler, void *scheduler, OsSchedulerInterfaces *schedulerInterfaces);
/*********************************************************************************************************************
* 增加任务
* vScheduler：OsVScheduler对象
* taskControlBlock：OsTaskControlBlock对象
* return：0：添加成功
*********************************************************************************************************************/
int osVSchedulerAddTask(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock);
/*********************************************************************************************************************
* 修改优先级
* vScheduler：OsVScheduler对象
* taskControlBlock：OsTaskControlBlock对象
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int osVSchedulerModifyPriority(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock, os_size_t priority);
/*********************************************************************************************************************
* 时钟滴答
* osVScheduler：OsVScheduler对象
* schedulerId：指定使用哪个调度器，如果为OS_MAX_SCHEDULER_COUNT则表示按照顺序调用
* ns：输入与上次tick的时间间隔，输出下次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsTaskControlBlock *osVSchedulerTick(OsVScheduler *vScheduler, os_size_t schedulerId, uint64_t *ns);
/*********************************************************************************************************************
* 挂起一个任务
* vScheduler：OsVScheduler对象
* taskControlBlock：OsTaskControlBlock对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsTaskControlBlock *osVSchedulerSupend(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock);
/*********************************************************************************************************************
* 恢复一个任务
* vScheduler：OsVScheduler对象
* taskControlBlock：OsTaskControlBlock对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsTaskControlBlock *osVSchedulerResume(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock);
/*********************************************************************************************************************
* 休眠一段时间
* vScheduler：OsVScheduler对象
* ns：休眠的时间
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsTaskControlBlock *osVSchedulerSleep(OsVScheduler *vScheduler, uint64_t ns);
/*********************************************************************************************************************
* 唤醒睡眠的任务
* vScheduler：OsVScheduler对象
* taskControlBlock：OsTaskControlBlock对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsTaskControlBlock *osVSchedulerWakeup(OsVScheduler *vScheduler, OsTaskControlBlock *taskControlBlock);
/*********************************************************************************************************************
* 推出当前任务
* vScheduler：OsVScheduler对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
OsTaskControlBlock *osVSchedulerExit(OsVScheduler *vScheduler);
#ifdef __cplusplus
}
#endif
#endif