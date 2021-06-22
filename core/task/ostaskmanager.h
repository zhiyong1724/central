#ifndef __OSTASKMANAGER_H__
#define __OSTASKMANAGER_H__
#include "ostidmanager.h"
#include "osvscheduler.h"
#include "osrtscheduler.h"
#include "osdtscheduler.h"
#include "osidlescheduler.h"
#include "osvector.h"
#include "ostaskdef.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct OsTask
{
    OsListNode node;
    OsTaskControlBlock taskControlBlock;
    union
    {
        OsRtTaskControlBlock rtTaskControlBlock;
        OsDtTaskControlBlock dtTaskControlBlock;
        OsIdleTaskControlBlock idleTaskControlBlock;
    } realTaskControlBlock;
    union
    {
        OsListNode listNode;
        OsTreeNode treeNode;
    } exNode;
    char name[TASK_MAX_NAME_LEN];
    os_tid_t tid;
    struct OsTask *parent;
    struct OsTask *children;
    os_size_t childrenCount;
    void *stackStart;
    void *stackTop;
    os_size_t stackSize;
    os_size_t *taskStackMagic;
    TaskFunction taskFunction;
    void *arg;
    os_tid_t waitTid;
    os_size_t tickCount;
} OsTask;

typedef struct OsTaskManager
{
    OsVScheduler vScheduler;
    OsRtScheduler rtScheduler;
    OsDtScheduler dtScheduler;
    OsIdleScheduler idleScheduler;
    OsTidManager tidManager;
    OsVector taskList;
    os_size_t taskCount;
    OsTask *deleteTaskList;
    OsTask *initTask;
    os_size_t tickCount;
} OsTaskManager;
/*********************************************************************************************************************
* OsTaskManager初始化
* taskManager：OsTaskManager对象
* clockPeriod：时钟周期NS
* return：0：初始化成功
*********************************************************************************************************************/
int osTaskManagerInit(OsTaskManager *taskManager, os_size_t clockPeriod);
/*********************************************************************************************************************
* 创建任务
* taskManager：OsTaskManager对象
* tid：任务tid
* taskFunction：任务处理函数
* arg：传给任务的参数
* name：任务名称，最大长度为TASK_MAX_NAME_LEN
* taskType：OS_TASK_TYPE_RT为实时任务，OS_TASK_TYPE_DT为分时任务
* priority：实时任务优先级范围0-63，分时任务优先级范围为0-39
* stackSize：任务堆栈大小
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerCreateTask(OsTaskManager *taskManager, os_tid_t *tid, TaskFunction taskFunction, void *arg, const char *name, OsTaskType taskType, os_size_t priority, os_size_t stackSize);
/*********************************************************************************************************************
* 时钟滴答
* taskManager：OsTaskManager对象
* nextTask：下一个任务
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerTick(OsTaskManager *taskManager, OsTask **nextTask);
/*********************************************************************************************************************
* 修改优先级
* taskManager：OsTaskManager对象
* tid：任务tid
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int osTaskManagerModifyPriority(OsTaskManager *taskManager, os_tid_t tid, os_size_t priority);
/*********************************************************************************************************************
* 休眠一段时间
* taskManager：OsTaskManager对象
* nextTask：下一个任务
* ns：休眠的时间
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerSleep(OsTaskManager *taskManager, OsTask **nextTask, os_size_t ns);
/*********************************************************************************************************************
* 唤醒睡眠的任务
* taskManager：OsTaskManager对象
* tid：要唤醒的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerWakeup(OsTaskManager *taskManager, os_tid_t tid);
/*********************************************************************************************************************
* 挂起一个任务
* taskManager：OsTaskManager对象
* nextTask：下一个任务
* tid：要挂起的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerSupend(OsTaskManager *taskManager, OsTask **nextTask, os_tid_t tid);
/*********************************************************************************************************************
* 恢复一个任务
* taskManager：OsTaskManager对象
* tid：要恢复的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerResume(OsTaskManager *taskManager, os_tid_t tid);
/*********************************************************************************************************************
* 退出当前任务
* taskManager：OsTaskManager对象
* nextTask：下一个任务
* arg：任务退出参数
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerExit(OsTaskManager *taskManager, OsTask **nextTask, void *arg);
/*********************************************************************************************************************
* 等待子任务退出
* taskManager：OsTaskManager对象
* nextTask：下一个任务
* retval：任务退出参数
* tid：等待任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerJoin(OsTaskManager *taskManager, OsTask **nextTask, void **retval, os_tid_t tid);
/*********************************************************************************************************************
* 把任务与父任务分离
* taskManager：OsTaskManager对象
* tid：分离任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerDetach(OsTaskManager *taskManager, os_tid_t tid);
/*********************************************************************************************************************
* 获取系统ticks
* taskManager：OsTaskManager对象
* return：系统ticks
*********************************************************************************************************************/
os_size_t osTaskManagerGetTickCount(OsTaskManager *taskManager);
/*********************************************************************************************************************
* 获取时钟周期 ns
* taskManager：OsTaskManager对象
* return：时钟周期
*********************************************************************************************************************/
os_size_t osTaskManagerGetClockPeriod(OsTaskManager *taskManager);
/*********************************************************************************************************************
* 获取任务个数
* taskManager：OsTaskManager对象
* return：任务个数
*********************************************************************************************************************/
os_size_t osTaskManagerGetTaskCount(OsTaskManager *taskManager);
/*********************************************************************************************************************
* 获取当前任务TID
* taskManager：OsTaskManager对象
* return：当前任务TID
*********************************************************************************************************************/
os_tid_t osTaskManagerGetTid(OsTaskManager *taskManager);
/*********************************************************************************************************************
* 获取任务优先级
* taskManager：OsTaskManager对象
* priority：任务优先级
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerGetTaskPriority(OsTaskManager *taskManager, os_size_t *priority, os_tid_t tid);
/*********************************************************************************************************************
* 获取任务调度类型
* taskManager：OsTaskManager对象
* type：任务调度类型
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerGetTaskType(OsTaskManager *taskManager, OsTaskType *type, os_tid_t tid);
/*********************************************************************************************************************
* 获取任务状态
* taskManager：OsTaskManager对象
* state：任务状态
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerGetTaskState(OsTaskManager *taskManager, OsTaskState *state, os_tid_t tid);
/*********************************************************************************************************************
* 获取任务名
* taskManager：OsTaskManager对象
* name：任务名
* size：name buffer大小
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerGetTaskName(OsTaskManager *taskManager, char *name, os_size_t size, os_tid_t tid);
/*********************************************************************************************************************
* 获取任务堆栈大小
* taskManager：OsTaskManager对象
* stackSize：任务堆栈大小
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerGetTaskStackSize(OsTaskManager *taskManager, os_size_t *stackSize, os_tid_t tid);
/*********************************************************************************************************************
* 获取任务ticks
* taskManager：OsTaskManager对象
* tickCount：任务ticks
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int osTaskManagerGetTaskTickCount(OsTaskManager *taskManager, os_size_t *tickCount, os_tid_t tid);
/*********************************************************************************************************************
* 返回任务是否为joinable
* taskManager：OsTaskManager对象
* tid：任务tid
* return：>0：joinable,0：Detach
*********************************************************************************************************************/
int osTaskManagerJoinable(OsTaskManager *taskManager, os_tid_t tid);
/*********************************************************************************************************************
* 获取当前任务
* taskManager：OsTaskManager对象
* return：当前任务
*********************************************************************************************************************/
OsTask *osTaskManagerGetRunningTask(OsTaskManager *taskManager);
#ifdef __cplusplus
}
#endif
#endif