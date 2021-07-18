#include "ostask.h"
#include "ostaskmanager.h"
#define ENABLE_TASK_LOG 0
#if ENABLE_TASK_LOG
#define taskLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define taskLog(format, ...) (void)0
#endif
static OsTaskManager *sTaskManager;
int portStartScheduler(void **stackTop);
int portYield(void **stackTop);
os_size_t portDisableInterrupts();
int portRecoveryInterrupts(os_size_t state);
int osTaskInit(OsTaskManager *taskManager, os_size_t clockPeriod)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sTaskManager = taskManager;
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerInit(sTaskManager, clockPeriod);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskCreate(os_tid_t *tid, TaskFunction taskFunction, void *arg, const char *name, os_size_t priority, os_size_t stackSize)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (0 == stackSize)
    {
        stackSize = DEFAULT_TASK_STACK_SIZE;
    }
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerCreateTask(sTaskManager, tid, taskFunction, arg, name, OS_TASK_TYPE_DT, priority, stackSize);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskCreateRT(os_tid_t *tid, TaskFunction taskFunction, void *arg, const char *name, os_size_t priority, os_size_t stackSize)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (0 == stackSize)
    {
        stackSize = DEFAULT_TASK_STACK_SIZE;
    }
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerCreateTask(sTaskManager, tid, taskFunction, arg, name, OS_TASK_TYPE_RT, priority, stackSize);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskTick()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerTick(sTaskManager, &nextTask);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTasModifyPriority(os_tid_t tid, os_size_t priority)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerModifyPriority(sTaskManager, tid, priority);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskSleep(os_size_t ms)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerSleep(sTaskManager, &nextTask, ms * 1000 * 1000);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskWakeup(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerWakeup(sTaskManager, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskSupend(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerSupend(sTaskManager, &nextTask, tid);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskResume(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerResume(sTaskManager, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskExit(void *arg)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    OsTask *nextTask;
    int ret = osTaskManagerExit(sTaskManager, &nextTask, arg);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskStart()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    os_size_t state = portDisableInterrupts();
    OsTask *task = osTaskManagerGetRunningTask(sTaskManager);
    osAssert(task != NULL);
    if (task != NULL)
    {
        ret = portStartScheduler(&task->stackTop);
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskJoin(void **retval, os_size_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    os_size_t state = portDisableInterrupts();
    OsTask *nextTask;
    ret = osTaskManagerJoin(sTaskManager, &nextTask, retval, tid);
    portYield(&nextTask->stackTop);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskDetach(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    os_size_t state = portDisableInterrupts();
    ret = osTaskManagerDetach(sTaskManager, tid);
    portRecoveryInterrupts(state);
    return ret;
}

os_size_t osTaskGetTickCount()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osTaskManagerGetTickCount(sTaskManager);
}

os_size_t osTaskGetClockPeriod()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osTaskManagerGetClockPeriod(sTaskManager);
}

os_size_t osTaskGetTaskCount()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osTaskManagerGetTaskCount(sTaskManager);
}

os_tid_t osTaskGetTid()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    os_tid_t tid = osTaskManagerGetTid(sTaskManager);
    portRecoveryInterrupts(state);
    return tid;
}

int osTaskGetTaskPriority(os_size_t *priority, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskPriority(sTaskManager, priority, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskGetTaskType(OsTaskType *type, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskType(sTaskManager, type, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskGetTaskState(OsTaskState *state, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t interruptState = portDisableInterrupts();
    int ret = osTaskManagerGetTaskState(sTaskManager, state, tid);
    portRecoveryInterrupts(interruptState);
    return ret;
}

int osTaskGetTaskName(char *name, os_size_t size, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskName(sTaskManager, name, size, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskGetTaskStackSize(os_size_t *stackSize, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskStackSize(sTaskManager, stackSize, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskGetTaskTickCount(os_size_t *tickCount, os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerGetTaskTickCount(sTaskManager, tickCount, tid);
    portRecoveryInterrupts(state);
    return ret;
}

int osTaskJoinable(os_tid_t tid)
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osTaskManagerJoinable(sTaskManager, tid);
    portRecoveryInterrupts(state);
    return ret;
}

OsTask *osTaskGetRunningTask()
{
    taskLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    OsTask *task = osTaskManagerGetRunningTask(sTaskManager);
    portRecoveryInterrupts(state);
    return task;
}