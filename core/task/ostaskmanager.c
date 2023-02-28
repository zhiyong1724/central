#include "ostaskmanager.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_TASK_MANAGER_LOG 0
#if ENABLE_TASK_MANAGER_LOG
#define taskManagerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define taskManagerLog(format, ...) (void)0
#endif
#define OS_TASK_MAX_SIZE ((os_size_t)-1)
static int checkStack(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = osTaskManagerGetRunningTask(taskManager);
    if (OS_TASK_STACK_MAGIC == *task->taskStackMagic)
    {
#if (OS_TASK_STACK_GROWTH > 0)
        if ((os_byte_t *)task->stackTop > (os_byte_t *)task->stackStart + sizeof(os_size_t))
        {
            ret = 0;
        }
#else
        if ((os_byte_t *)task->stackTop < (os_byte_t *)task->stackStart + task->stackSize - sizeof(os_size_t))
        {
            ret = 0;
        }
#endif
    }
    if (ret != 0)
    {
        printf("Task %s stack overflow\n", task->name);
        osAssert(0);
    }
    return ret;
}

static int addSchedulers(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsSchedulerInterfaces schedulerInterfaces;
    schedulerInterfaces.addTask = (AddTask)osRtSchedulerAddTask;
    schedulerInterfaces.removeTask = (RemoveTask)osRtSchedulerRemoveTask;
    schedulerInterfaces.tick = (Tick)osRtSchedulerTick;
    schedulerInterfaces.yield = (Yield)osRtSchedulerYield;
    schedulerInterfaces.modifyPriority = (ModifyPriority)osRtSchedulerModifyPriority;
    schedulerInterfaces.getRunningTask = (GetRunningTask)osRtSchedulerGetRunningTask;
    ret = osVSchedulerAddScheduler(&taskManager->vScheduler, &taskManager->rtScheduler, &schedulerInterfaces);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    schedulerInterfaces.addTask = (AddTask)osDtSchedulerAddTask;
    schedulerInterfaces.removeTask = (RemoveTask)osDtSchedulerRemoveTask;
    schedulerInterfaces.tick = (Tick)osDtSchedulerTick;
    schedulerInterfaces.yield = (Yield)osDtSchedulerYield;
    schedulerInterfaces.modifyPriority = (ModifyPriority)osDtSchedulerModifyPriority;
    schedulerInterfaces.getRunningTask = (GetRunningTask)osDtSchedulerGetRunningTask;
    ret = osVSchedulerAddScheduler(&taskManager->vScheduler, &taskManager->dtScheduler, &schedulerInterfaces);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    schedulerInterfaces.addTask = (AddTask)osIdleSchedulerAddTask;
    schedulerInterfaces.removeTask = (RemoveTask)osIdleSchedulerRemoveTask;
    schedulerInterfaces.tick = (Tick)osIdleSchedulerTick;
    schedulerInterfaces.yield = (Yield)osIdleSchedulerYield;
    schedulerInterfaces.modifyPriority = (ModifyPriority)osIdleSchedulerModifyPriority;
    schedulerInterfaces.getRunningTask = (GetRunningTask)osIdleSchedulerGetRunningTask;
    ret = osVSchedulerAddScheduler(&taskManager->vScheduler, &taskManager->idleScheduler, &schedulerInterfaces);
    osAssert(0 == ret);
    return ret;
}

static void deleteTask(OsTaskManager *taskManager, OsTask *task)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osTidFree(&taskManager->tidManager, task->tid);
    osFree(task->stackStart);
    osFree(task);
}

int portRecoveryInterrupts(os_size_t state);
os_size_t portDisableInterrupts();

static void clearDeleteTaskList(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    for (OsTask *task = taskManager->deleteTaskList; task != NULL; task = taskManager->deleteTaskList)
    {
        osRemoveFromList((OsListNode **)&taskManager->deleteTaskList, &task->node);
        deleteTask(taskManager, task);
    }
    portRecoveryInterrupts(state);
}

int osTaskSleep(uint64_t ms);

static void *initTask(void *arg)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsTaskManager *taskManager = (OsTaskManager *)arg;
    for (;;)
    {
        clearDeleteTaskList(taskManager);
        osTaskSleep(200);
    }
    return NULL;
}

static void *idleTask(void *arg)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    for (;;)
    {
    }
    return NULL;
}

static OsTask *getTaskByTid(OsTaskManager *taskManager, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t size = osVectorSize(&taskManager->taskList);
    if (tid < size)
    {
        OsTask **ptask = (OsTask **)osVectorAt(&taskManager->taskList, tid);
        if (ptask != NULL)
        {
            return *ptask;
        }
    }
    return NULL;
}

int osTaskManagerInit(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t ret = -1;
    ret = osVSchedulerInit(&taskManager->vScheduler);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = osRtSchedulerInit(&taskManager->rtScheduler);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = osDtSchedulerInit(&taskManager->dtScheduler);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = osIdleSchedulerInit(&taskManager->idleScheduler);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = addSchedulers(taskManager);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = osTidManagerInit(&taskManager->tidManager);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = osVectorInit(&taskManager->taskList, sizeof(void *));
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    taskManager->taskCount = 0;
    taskManager->deleteTaskList = NULL;
    taskManager->tickCount = 0;
    taskManager->cpuUsage = 0;
    taskManager->idleTaskTickCount = 0;
    taskManager->idleTickCount = 0;
    os_tid_t tid;
    ret = osTaskManagerCreateTask(taskManager, &tid, initTask, taskManager, "init", OS_TASK_TYPE_DT, 0, OS_DEFAULT_TASK_STACK_SIZE);
    taskManager->initTask = osTaskManagerGetRunningTask(taskManager);
    ret = osTaskManagerCreateTask(taskManager, &tid, idleTask, taskManager, "idle", OS_TASK_TYPE_IDLE, 0, OS_DEFAULT_TASK_STACK_SIZE);
    return ret;
}

extern int portInitializeStack(void **stackTop, os_size_t stackSize, os_size_t *taskStackMagic, TaskFunction taskFunction, void *arg);
int osTaskManagerCreateTask(OsTaskManager *taskManager, os_tid_t *tid, TaskFunction taskFunction, void *arg, const char *name, OsTaskType taskType, os_size_t priority, os_size_t stackSize)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(taskManager->taskCount < OS_TASK_MAX_SIZE);
    if (taskManager->taskCount < OS_TASK_MAX_SIZE)
    {
        OsTask *task = (OsTask *)osMalloc(sizeof(OsTask));
        osAssert(task != NULL);
        if (task != NULL)
        {
            task->stackStart = osMalloc(stackSize);
            osAssert(task->stackStart != NULL);
            if (task->stackStart != NULL)
            {
                task->taskFunction = taskFunction;
                task->arg = NULL;
                osStrCpy(task->name, name, OS_TASK_MAX_NAME_LEN);
                task->tid = osTidAlloc(&taskManager->tidManager);
                task->stackSize = stackSize;
                task->children = NULL;
                task->childrenCount = 0;
                task->waitTid = 0;
                task->parent = osTaskManagerGetRunningTask(taskManager);
                if (task->parent != NULL)
                {
                    osInsertToBack((OsListNode **)&task->parent->children, &task->node);
                    task->parent->childrenCount++;
                }
#if (OS_TASK_STACK_GROWTH > 0)
                task->taskStackMagic = (os_size_t *)task->stackStart;
                *task->taskStackMagic = OS_TASK_STACK_MAGIC;
                task->stackTop = (os_byte_t *)task->stackStart + stackSize;
#else
                task->taskStackMagic = (os_size_t *)((os_byte_t *)task->stackStart + stackSize - sizeof(os_size_t));
                *task->taskStackMagic = OS_TASK_STACK_MAGIC;
                task->stackTop = task->stackStart;
#endif
                portInitializeStack(&task->stackTop, task->stackSize, task->taskStackMagic, task->taskFunction, arg);
                osTaskControlBlockInit(&taskManager->vScheduler, &task->taskControlBlock);
                task->taskControlBlock.schedulerId = taskType;
                switch (task->taskControlBlock.schedulerId)
                {
                case OS_TASK_TYPE_RT:
                    osRtTaskControlBlockInit(&taskManager->rtScheduler, &task->realTaskControlBlock.rtTaskControlBlock, priority);
                    break;
                case OS_TASK_TYPE_DT:
                    osDtTaskControlBlockInit(&taskManager->dtScheduler, &task->realTaskControlBlock.dtTaskControlBlock, priority);
                    break;
                case OS_TASK_TYPE_IDLE:
                    osIdleTaskControlBlockInit(&taskManager->idleScheduler, &task->realTaskControlBlock.idleTaskControlBlock, priority);
                    break;
                default:
                    break;
                }
                osVSchedulerAddTask(&taskManager->vScheduler, &task->taskControlBlock);
                taskManager->taskCount++;
                os_size_t size = osVectorSize(&taskManager->taskList);
                if (task->tid >= size)
                {
                    void *v = 0;
                    osVectorPushBack(&taskManager->taskList, &v);
                }
                void **t = (void **)osVectorAt(&taskManager->taskList, task->tid);
                *t = task;
                *tid = task->tid;
                ret = 0;
            }
            else
            {
                osFree(task);
            }
        }
    }
    return ret;
}

int osTaskManagerTick(OsTaskManager *taskManager, OsTask **nextTask, uint64_t *ns)
{
    //taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = checkStack(taskManager);
    if (0 == ret)
    {
        taskManager->tickCount += *ns;
        taskManager->idleTickCount += *ns;
        OsTask *runningTask = osTaskManagerGetRunningTask(taskManager);
        if (1 == runningTask->tid)
        {
            taskManager->idleTaskTickCount += *ns;
        }
        if (taskManager->idleTickCount >= 1000000000ll)
        {
            taskManager->cpuUsage = 100 - taskManager->idleTaskTickCount * 100 / taskManager->idleTickCount;
            taskManager->idleTaskTickCount = 0;
            taskManager->idleTickCount = 0;
        }
        *nextTask = (OsTask *)osVSchedulerTick(&taskManager->vScheduler, OS_MAX_SCHEDULER_COUNT, ns);
        *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
    }
    return ret;
}

int osTaskManagerModifyPriority(OsTaskManager *taskManager, os_tid_t tid, os_size_t priority)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(tid > 1);
    if (tid > 1)
    {
        OsTask *task = getTaskByTid(taskManager, tid);
        osAssert(task != NULL && task->tid > 1);
        if (task != NULL && task->tid > 1)
        {
            ret = osVSchedulerModifyPriority(&taskManager->vScheduler, &task->taskControlBlock, priority);
        }
        
    }
    return ret;
}

int osTaskManagerSleep(OsTaskManager *taskManager, OsTask **nextTask, uint64_t ns)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret  = checkStack(taskManager);
    if (0 == ret)
    {
        *nextTask = (OsTask *)osVSchedulerSleep(&taskManager->vScheduler, ns);
        *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
    }
    return ret;
}

int osTaskManagerWakeup(OsTaskManager *taskManager, OsTask **nextTask, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(tid > 1);
    if (tid > 1)
    {
        if (checkStack(taskManager) == 0)
        {
            OsTask *task = getTaskByTid(taskManager, tid);
            osAssert(task != NULL && task->tid > 1);
            if (task != NULL && task->tid > 1)
            {
                *nextTask = (OsTask *)osVSchedulerWakeup(&taskManager->vScheduler, &task->taskControlBlock);
                *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
                ret = 0;
            }
        }
    }
    return ret;
}

int osTaskManagerSupend(OsTaskManager *taskManager, OsTask **nextTask, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(tid > 1);
    if (tid > 1)
    {
        if (checkStack(taskManager) == 0)
        {
            OsTask *task = getTaskByTid(taskManager, tid);
            osAssert(task != NULL && task->tid > 1);
            if (task != NULL && task->tid > 1)
            {
                *nextTask = (OsTask *)osVSchedulerSupend(&taskManager->vScheduler, &task->taskControlBlock);
                *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
                ret = 0;
            }
        }
    }
    return ret;
}

int osTaskManagerResume(OsTaskManager *taskManager, OsTask **nextTask, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(tid > 1);
    if (tid > 1)
    {
        if (checkStack(taskManager) == 0)
        {
            OsTask *task = getTaskByTid(taskManager, tid);
            osAssert(task != NULL && task->tid > 1);
            if (task != NULL && task->tid > 1)
            {
                *nextTask = (OsTask *)osVSchedulerResume(&taskManager->vScheduler, &task->taskControlBlock);
                *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
                ret = 0;
            }
        }
    }
    return ret;
}

static void moveChildren(OsTaskManager *taskManager, OsTask *task)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    while (task->childrenCount > 0)
    {
        OsTask *child = task->children;
        osRemoveFromList((OsListNode **)&task->children, &child->node);
        if (OS_TASK_STATE_DELETED == child->taskControlBlock.taskState)
        {
            deleteTask(taskManager, child);
        }
        else
        {
            child->parent = taskManager->initTask;
            osInsertToBack((OsListNode **)&taskManager->initTask->children, &child->node);
            taskManager->initTask->childrenCount++;
        }
        task->childrenCount--;
    }
}

int osTaskManagerExit(OsTaskManager *taskManager, OsTask **nextTask, void *arg)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = checkStack(taskManager);
    if (0 == ret)
    {
        OsTask *task = osTaskManagerGetRunningTask(taskManager);
        *nextTask = (OsTask *)((os_byte_t *)osVSchedulerExit(&taskManager->vScheduler) - sizeof(OsListNode));
        void **pp = (void **)osVectorAt(&taskManager->taskList, task->tid);
        *pp = NULL;
        moveChildren(taskManager, task);
        if (task->parent == taskManager->initTask)
        {
            osRemoveFromList((OsListNode **)&taskManager->initTask->children, &task->node);
            taskManager->initTask->childrenCount--;
            osInsertToBack((OsListNode **)&taskManager->deleteTaskList, &task->node);
        }
        else
        {
            if (OS_TASK_STATE_BLOCKED == task->parent->taskControlBlock.taskState && task->parent->waitTid == task->tid)
            {
                task->parent->waitTid = 0;
                *(void **)task->parent->arg = arg;
                task->parent->arg = NULL;
                task->parent->taskControlBlock.taskState = OS_TASK_STATE_SUSPENDED;
                *nextTask = (OsTask *)((os_byte_t *)osVSchedulerResume(&taskManager->vScheduler, &task->parent->taskControlBlock) - sizeof(OsListNode));

                osRemoveFromList((OsListNode **)&task->parent->children, &task->node);
                task->parent->childrenCount--;
                osInsertToBack((OsListNode **)&taskManager->deleteTaskList, &task->node);
            }
            else
            {
                task->arg = arg;
            }
        }
        taskManager->taskCount--;
    }
    return ret;
}

int osTaskManagerJoin(OsTaskManager *taskManager, OsTask **nextTask, void **retval, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    if (checkStack(taskManager) == 0)
    {
        OsTask *task = getTaskByTid(taskManager, tid);
        osAssert(task != NULL && task->tid > 1);
        if (task != NULL && task->tid > 1)
        {
            OsTask *runningTask = osTaskManagerGetRunningTask(taskManager);
            osAssert(task->parent == runningTask);
            if (task->parent == runningTask)
            {
                if (OS_TASK_STATE_DELETED == task->taskControlBlock.taskState)
                {
                    *retval = task->arg;
                    task->arg = NULL;
                    osRemoveFromList((OsListNode **)&runningTask->children, &task->node);
                    runningTask->childrenCount--;
                    deleteTask(taskManager, task);
                    *nextTask = runningTask;
                }
                else
                {
                    runningTask->arg = (void *)retval;
                    runningTask->waitTid = tid;
                    *nextTask = (OsTask *)osVSchedulerSupend(&taskManager->vScheduler, &runningTask->taskControlBlock);
                    runningTask->taskControlBlock.taskState = OS_TASK_STATE_BLOCKED;
                    *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
                }
                ret = 0;
            }
        }
    }
    return ret;
}

int osTaskManagerDetach(OsTaskManager *taskManager, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL && task->tid > 1);
    if (task != NULL && task->tid > 1)
    {
        osAssert(task->parent != taskManager->initTask && task->parent->taskControlBlock.taskState != OS_TASK_STATE_BLOCKED && task->parent->waitTid != task->tid);
        if (task->parent != taskManager->initTask && task->parent->taskControlBlock.taskState != OS_TASK_STATE_BLOCKED && task->parent->waitTid != task->tid)
        {
            osRemoveFromList((OsListNode **)&task->parent->children, &task->node);
            task->parent->childrenCount--;

            task->parent = taskManager->initTask;
            osInsertToBack((OsListNode **)&taskManager->initTask->children, &task->node);
            taskManager->initTask->childrenCount++;
            ret = 0;
        }
    }
    return ret;
}

uint64_t osTaskManagerGetTickCount(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return taskManager->tickCount;
}

os_size_t osTaskManagerGetTaskCount(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return taskManager->taskCount;
}

os_tid_t osTaskManagerGetTid(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsTask *runningTask = osTaskManagerGetRunningTask(taskManager);
    return runningTask->tid;
}

int osTaskManagerGetTaskPriority(OsTaskManager *taskManager, os_size_t *priority, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL && task->tid > 1);
    if (task != NULL && task->tid > 1)
    {
        switch (task->taskControlBlock.schedulerId)
        {
        case OS_TASK_TYPE_RT:
            *priority = task->realTaskControlBlock.rtTaskControlBlock.priority;
            break;
        case OS_TASK_TYPE_DT:
            *priority = task->realTaskControlBlock.dtTaskControlBlock.priority;
            break;
        case OS_TASK_TYPE_IDLE:
            *priority = task->realTaskControlBlock.idleTaskControlBlock.priority;
            break;
        default:
            break;
        }
        ret = 0;
    }
    return ret;
}

int osTaskManagerGetTaskType(OsTaskManager *taskManager, OsTaskType *type, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL && task->tid > 1);
    if (task != NULL && task->tid > 1)
    {
        *type = (OsTaskType)task->taskControlBlock.schedulerId;
        ret = 0;
    }
    return ret;
}

int osTaskManagerGetTaskState(OsTaskManager *taskManager, OsTaskState *state, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL && task->tid > 1);
    if (task != NULL && task->tid > 1)
    {
        *state = task->taskControlBlock.taskState;
        ret = 0;
    }
    return ret;
}

int osTaskManagerGetTaskName(OsTaskManager *taskManager, char *name, os_size_t size, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL && task->tid > 1);
    if (task != NULL && task->tid > 1)
    {
        size = size < OS_TASK_MAX_NAME_LEN ? size : OS_TASK_MAX_NAME_LEN;
        osStrCpy(name, task->name, size);
        ret = 0;
    }
    return ret;
}

int osTaskManagerGetTaskStackSize(OsTaskManager *taskManager, os_size_t *stackSize, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL && task->tid > 1);
    if (task != NULL && task->tid > 1)
    {
        *stackSize = task->stackSize;
        ret = 0;
    }
    return ret;
}

int osTaskManagerJoinable(OsTaskManager *taskManager, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL && task->tid > 1);
    if (task != NULL && task->tid > 1)
    {
        if (task->parent == taskManager->initTask)
        {
            ret = 1;
        }
        else
        {
            ret = 0;
        }
    }
    return ret;
}

OsTask *osTaskManagerGetRunningTask(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsTask *task = (OsTask *)osVSchedulerGetRunningTask(&taskManager->vScheduler);
    if (task != NULL)
    {
        task = (OsTask *)((os_byte_t *)task - sizeof(OsListNode));
        return task;
    }
    return NULL;
}

os_size_t osTaskManagerGetCPUUsage(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return taskManager->cpuUsage;
}

int osTaskManagerFindFirst(OsTaskManager *taskManager, os_task_ptr *taskPtr, OsTaskInfo *taskInfo)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    *taskPtr = 0;
    return osTaskManagerFindNext(taskManager, taskPtr, taskInfo);
}

int osTaskManagerFindNext(OsTaskManager *taskManager, os_task_ptr *taskPtr, OsTaskInfo *taskInfo)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, *taskPtr);
    if (task != NULL)
    {
        ret = 0;
        (*taskPtr)++;
        taskInfo->tid = task->tid;
        if (task->parent != NULL)
        {
            taskInfo->ptid = task->parent->tid;
        }
        else
        {
            taskInfo->ptid = 0;
        }
        osStrCpy(taskInfo->name, task->name, OS_TASK_MAX_NAME_LEN);
        taskInfo->stack = task->stackStart;
        taskInfo->stackSize = task->stackSize;
        taskInfo->taskState = task->taskControlBlock.taskState;
        taskInfo->taskType = task->taskControlBlock.schedulerId;
        switch (task->taskControlBlock.schedulerId)
        {
        case OS_TASK_TYPE_RT:
            taskInfo->priority = task->realTaskControlBlock.rtTaskControlBlock.priority;
            break;
        case OS_TASK_TYPE_DT:
            taskInfo->priority = task->realTaskControlBlock.dtTaskControlBlock.priority;
            break;
        case OS_TASK_TYPE_IDLE:
            taskInfo->priority = task->realTaskControlBlock.idleTaskControlBlock.priority;
            break;
        default:
            break;
        }
    }
    return ret;
}