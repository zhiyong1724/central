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

OsTask *osTaskSleep(os_size_t ms);

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
    osAssert(tid < size);
    if (tid < size)
    {
        OsTask **ptask = (OsTask **)osVectorAt(&taskManager->taskList, tid);
        osAssert(ptask != NULL);
        if (ptask != NULL)
        {
            return *ptask;
        }
    }
    return NULL;
}

int osTaskManagerInit(OsTaskManager *taskManager, uint64_t clockPeriod)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t ret = -1;
    ret = osVSchedulerInit(&taskManager->vScheduler, clockPeriod);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = osRtSchedulerInit(&taskManager->rtScheduler, clockPeriod);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = osDtSchedulerInit(&taskManager->dtScheduler, clockPeriod);
    osAssert(0 == ret);
    if (ret != 0)
    {
        return ret;
    }
    ret = osIdleSchedulerInit(&taskManager->idleScheduler, clockPeriod);
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
                    os_size_t v = 0;
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

int osTaskManagerTick(OsTaskManager *taskManager, OsTask **nextTask)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    taskManager->tickCount++;
    uint64_t reTick = taskManager->tickCount & 0x03ff;
    if (0 == reTick)
    {
        taskManager->cpuUsage = (os_size_t)((taskManager->tickCount - taskManager->idleTaskTickCount) * 100 / 1024);
        if (taskManager->cpuUsage > 100)
        {
            taskManager->cpuUsage = 100;
        }
        taskManager->idleTaskTickCount = taskManager->tickCount;
    }
    *nextTask = (OsTask *)osVSchedulerTick(&taskManager->vScheduler, OS_MAX_SCHEDULER_COUNT);
    *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
    osAssert(OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic);
    if (OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic)
    {
        if (1 == (*nextTask)->tid)
        {
            taskManager->idleTaskTickCount++;
        }
        return 0;
    }
    else
    {
        printf("Task %s stack overflow\n", (*nextTask)->name);
        return -1;
    }
}

int osTaskManagerModifyPriority(OsTaskManager *taskManager, os_tid_t tid, os_size_t priority)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(tid > 1);
    if (tid > 1)
    {
        OsTask *task = getTaskByTid(taskManager, tid);
        osAssert(task != NULL);
        if (task != NULL)
        {
            ret = osVSchedulerModifyPriority(&taskManager->vScheduler, &task->taskControlBlock, priority);
        }
        
    }
    return ret;
}

int osTaskManagerSleep(OsTaskManager *taskManager, OsTask **nextTask, uint64_t ns)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    *nextTask = (OsTask *)osVSchedulerSleep(&taskManager->vScheduler, ns);
    *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
    osAssert(OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic);
    if (OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int osTaskManagerWakeup(OsTaskManager *taskManager, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL);
    if (task != NULL)
    {
        osVSchedulerWakeup(&taskManager->vScheduler, &(task)->taskControlBlock);
        ret = 0;
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
        OsTask *task = getTaskByTid(taskManager, tid);
        osAssert(task != NULL);
        if (task != NULL)
        {
            *nextTask = (OsTask *)osVSchedulerSupend(&taskManager->vScheduler, &task->taskControlBlock);
            *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
            osAssert(OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic);
            if (OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic)
            {
                ret = 0;
            }
        }
        
    }
    return ret;
}

int osTaskManagerResume(OsTaskManager *taskManager, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL);
    if (task != NULL)
    {
        osVSchedulerResume(&taskManager->vScheduler, &(task)->taskControlBlock);
        ret = 0;
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
    OsTask *task = osTaskManagerGetRunningTask(taskManager);
    osAssert(OS_TASK_STACK_MAGIC == *task->taskStackMagic);
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
            osVSchedulerResume(&taskManager->vScheduler, &task->parent->taskControlBlock);

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
    *nextTask = (OsTask *)((os_byte_t *)osVSchedulerExit(&taskManager->vScheduler) - sizeof(OsListNode));
    osAssert(OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic);
    if (OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int osTaskManagerJoin(OsTaskManager *taskManager, OsTask **nextTask, void **retval, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL);
    if (task != NULL)
    {
        OsTask *runningTask = osTaskManagerGetRunningTask(taskManager);
        osAssert(task->parent == runningTask);
        if(task->parent == runningTask)
        {
            if (OS_TASK_STATE_DELETED == task->taskControlBlock.taskState)
            {
                *retval = task->arg;
                task->arg = NULL;
                osRemoveFromList((OsListNode **)&runningTask->children, &task->node);
                runningTask->childrenCount--;
                deleteTask(taskManager, task);
                ret = 0;
            }
            else
            {
                runningTask->arg = (void *)retval;
                runningTask->waitTid = tid;
                *nextTask = (OsTask *)osVSchedulerSupend(&taskManager->vScheduler, &runningTask->taskControlBlock);
                runningTask->taskControlBlock.taskState = OS_TASK_STATE_BLOCKED;
                *nextTask = (OsTask *)((os_byte_t *)*nextTask - sizeof(OsListNode));
                osAssert(OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic);
                if (OS_TASK_STACK_MAGIC == *(*nextTask)->taskStackMagic)
                {
                    ret = 0;
                }
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
    osAssert(task != NULL);
    if (task != NULL)
    {
        if (task->parent != taskManager->initTask)
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

uint64_t osTaskManagerGetClockPeriod(OsTaskManager *taskManager)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return taskManager->vScheduler.clockPeriod;
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
    osAssert(task != NULL);
    if (task != NULL)
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
    osAssert(task != NULL);
    if (task != NULL)
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
    osAssert(task != NULL);
    if (task != NULL)
    {
        OsTask *runningTask = osTaskManagerGetRunningTask(taskManager);
        if (task == runningTask)
        {
            *state = OS_TASK_STATE_RUNNING;
        }
        else
        {
            *state = task->taskControlBlock.taskState;
        }
        ret = 0;
    }
    return ret;
}

int osTaskManagerGetTaskName(OsTaskManager *taskManager, char *name, os_size_t size, os_tid_t tid)
{
    taskManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = getTaskByTid(taskManager, tid);
    osAssert(task != NULL);
    if (task != NULL)
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
    osAssert(task != NULL);
    if (task != NULL)
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
    osAssert(task != NULL);
    if (task != NULL)
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
    OsTask *task = (OsTask *)taskManager->vScheduler.runningTask;
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