#include "ostask.h"
#include "ostaskmanager.h"
#define ENABLE_TASK_LOG 0
#if ENABLE_TASK_LOG
#define taskLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define taskLog(format, ...) (void)0
#endif
static OsTaskManager *sTaskManager;
static int sRunning = 0;
int portStartScheduler(stack_size_t **stackTop);
int portYield(stack_size_t **stackTop);
size_t portDisableInterrupts();
int portRecoveryInterrupts(size_t state);
int osTaskInit(OsTaskManager *taskManager)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sTaskManager = taskManager;
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerInit(sTaskManager);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskCreate(os_tid_t *tid, TaskFunction taskFunction, void *arg, const char *name, size_t priority, size_t stackSize)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (0 == stackSize)
    {
        stackSize = OS_DEFAULT_TASK_STACK_SIZE;
    }
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerCreateTask(sTaskManager, tid, taskFunction, arg, name, OS_TASK_TYPE_DT, priority, stackSize);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskCreateRT(os_tid_t *tid, TaskFunction taskFunction, void *arg, const char *name, size_t priority, size_t stackSize)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (0 == stackSize)
    {
        stackSize = OS_DEFAULT_TASK_STACK_SIZE;
    }
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerCreateTask(sTaskManager, tid, taskFunction, arg, name, OS_TASK_TYPE_RT, priority, stackSize);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskTick(uint64_t *ns)
{
    //taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    if (sRunning > 0)
    {
        size_t state = portDisableInterrupts();
        OsTask *nextTask;
        ret = osTaskManagerTick(sTaskManager, &nextTask, ns);
        portYield(&nextTask->stackTop);
        portRecoveryInterrupts(state);
    }
    return ret;
}

int osTaskModifyPriority(os_tid_t tid, size_t priority)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerModifyPriority(sTaskManager, tid, priority);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskSleep(uint64_t ms)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerSleep(sTaskManager, &nextTask, ms * 1000 * 1000);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskWakeup(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerWakeup(sTaskManager, &nextTask, tid);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskSupend(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerSupend(sTaskManager, &nextTask, tid);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskResume(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerResume(sTaskManager, &nextTask, tid);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskExit(void *arg)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerExit(sTaskManager, &nextTask, arg);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

void osTaskStart()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    sRunning = 1;
    OsTask *task = osTaskManagerGetRunningTask(sTaskManager);
    osAssert(task != NULL);
    if (task != NULL)
    {
        portStartScheduler(&task->stackTop);
    }
    portRecoveryInterrupts(state);
    while (1)
    {
    }
}

int osTaskJoin(void **retval, size_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    size_t state = portDisableInterrupts();
    OsTask *nextTask = NULL;
    ret = osTaskManagerJoin(sTaskManager, &nextTask, retval, tid);
    if (nextTask != NULL)
    {
        portYield(&nextTask->stackTop);
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskDetach(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    size_t state = portDisableInterrupts();
    ret = osTaskManagerDetach(sTaskManager, tid);
    portRecoveryInterrupts(state);
    return ret;
}

uint64_t osTaskGetTickCount()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osTaskManagerGetTickCount(sTaskManager);
}

size_t osTaskGetTaskCount()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osTaskManagerGetTaskCount(sTaskManager);
}

os_tid_t osTaskGetTid()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    os_tid_t tid = osTaskManagerGetTid(sTaskManager);
    portRecoveryInterrupts(state);
    return tid;
}

int osTaskGetTaskPriority(size_t *priority, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskPriority(sTaskManager, priority, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskGetTaskType(OsTaskType *type, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskType(sTaskManager, type, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskGetTaskState(OsTaskState *state, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t interruptState = portDisableInterrupts();
    int ret = osTaskManagerGetTaskState(sTaskManager, state, tid);
    portRecoveryInterrupts(interruptState);
    return ret;
}

int osTaskGetTaskName(char *name, size_t size, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskName(sTaskManager, name, size, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskGetTaskStackSize(size_t *stackSize, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskStackSize(sTaskManager, stackSize, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskJoinable(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerJoinable(sTaskManager, tid);
    portRecoveryInterrupts(state);
    return ret;
}

OsTask *osTaskGetRunningTask()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    OsTask *task = osTaskManagerGetRunningTask(sTaskManager);
    portRecoveryInterrupts(state);
    return task;
}

size_t osTaskGetCPUUsage()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osTaskManagerGetCPUUsage(sTaskManager);
}

int osTaskFindFirst(os_task_ptr *taskPtr, OsTaskInfo *taskInfo)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerFindFirst(sTaskManager, taskPtr, taskInfo);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskFindNext(os_task_ptr *taskPtr, OsTaskInfo *taskInfo)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    int ret = osTaskManagerFindNext(sTaskManager, taskPtr, taskInfo);
    portRecoveryInterrupts(state);
    return ret;
}