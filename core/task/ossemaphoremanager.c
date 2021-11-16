#include "ossemaphoremanager.h"
#include "ostask.h"
#define ENABLE_SEMAPHOREMANAGER_LOG 0
#if ENABLE_SEMAPHOREMANAGER_LOG
#define semaphoreManagerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define semaphoreManagerLog(format, ...) (void)0
#endif
#define OS_SEMAPHORE_MAX_WAIT_TIME ((os_size_t)-1 / 1000 / 1000)
int osTaskWakeup(os_tid_t tid);
OsTask *osTaskGetRunningTask();
int osSemaphoreManagerInit(OsSemaphoreManager *semaphoreManager, OsTaskManager *taskManager)
{
    semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    semaphoreManager->taskManager = taskManager;
    return 0;
}

int osSemaphoreManagerSemaphoreInit(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore, os_size_t count, os_size_t maxCount)
{
    semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    semaphore->count = count;
    semaphore->maxCount = maxCount;
    semaphore->highPriorityTask = NULL;
    semaphore->waitRtTaskList = NULL;
    semaphore->waitTaskList = NULL;
    return 0;
}

int osSemaphoreManagerPost(OsSemaphoreManager *semaphoreManager, OsTask **nextTask, OsSemaphore *semaphore)
{
    semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    if (semaphore->count < semaphore->maxCount)
    {
        OsTask *task = NULL;
        os_byte_t *highPriorityTask = (os_byte_t *)semaphore->highPriorityTask;
        if (highPriorityTask != NULL)
        {
            osDeleteNode(&semaphore->waitRtTaskList, (OsTreeNode *)highPriorityTask);
            semaphore->highPriorityTask = osGetLeftmostNode(semaphore->waitRtTaskList);
            task = (OsTask *)(highPriorityTask - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task->realTaskControlBlock));
        }
        if (NULL == task)
        {
            highPriorityTask = (os_byte_t *)semaphore->waitTaskList;
            if (highPriorityTask != NULL)
            {
                osRemoveFromList(&semaphore->waitTaskList, (OsListNode *)highPriorityTask);
                task = (OsTask *)(highPriorityTask - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task->realTaskControlBlock));
            }
        }
        if (NULL == task)
        {
            semaphore->count++;
            ret = 0;
        }
        else
        {
            task->arg = semaphore;
            if (OS_TASK_STATE_BLOCKED == task->taskControlBlock.taskState)
            {
                task->taskControlBlock.taskState = OS_TASK_STATE_SUSPENDED;
                ret = osTaskManagerResume(semaphoreManager->taskManager, nextTask, task->tid);
            }
            else
            {
                ret = osTaskManagerWakeup(semaphoreManager->taskManager, nextTask, task->tid);
            }
        }
    }
    return ret;
}

static int onCompare(void *key1, void *key2, void *arg)
{
	semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsTask *task1 = (OsTask *)((os_byte_t *)key1 - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task1->realTaskControlBlock));
    OsTask *task2 = (OsTask *)((os_byte_t *)key2 - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task2->realTaskControlBlock));
    if (task1->realTaskControlBlock.rtTaskControlBlock.priority < task2->realTaskControlBlock.rtTaskControlBlock.priority)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

int osSemaphoreManagerWait(OsSemaphoreManager *semaphoreManager, OsTask **nextTask, OsSemaphore *semaphore, uint64_t wait)
{
    semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    *nextTask = NULL;
    if (semaphore->count > 0)
    {
        semaphore->count--;
        ret = 0;
    }
    else
    {
        OsTask *task = osTaskManagerGetRunningTask(semaphoreManager->taskManager);
        if (wait > 0 && wait < OS_SEMAPHORE_MAX_WAIT_TIME)
        {
            ret = osTaskManagerSleep(semaphoreManager->taskManager, nextTask, wait * 1000 * 1000);
        }
        else if (OS_SEMAPHORE_MAX_WAIT_TIME == wait)
        {
            ret = osTaskManagerSupend(semaphoreManager->taskManager, nextTask, task->tid);
            if (0 == ret)
            {
                task->taskControlBlock.taskState = OS_TASK_STATE_BLOCKED;
            }
        }
        if (0 == ret)
        {
            if (OS_TASK_TYPE_RT == task->taskControlBlock.schedulerId)
            {
                osInsertNode(&semaphore->waitRtTaskList, &task->exNode.treeNode, onCompare, NULL);
                if (NULL == semaphore->highPriorityTask)
                {
                    semaphore->highPriorityTask = &task->exNode.treeNode;
                }
                else
                {
                    OsTask *highPriorityTask = (OsTask *)((os_byte_t *)semaphore->highPriorityTask - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task->realTaskControlBlock));
                    if (task->realTaskControlBlock.rtTaskControlBlock.priority < highPriorityTask->realTaskControlBlock.rtTaskControlBlock.priority)
                    {
                        semaphore->highPriorityTask = &task->exNode.treeNode;
                    }
                }
            }
            else
            {
                osInsertToBack(&semaphore->waitTaskList, &task->exNode.listNode);
            }
        }
    }
    return ret;
}

os_size_t osSemaphoreManagerGetSemaphoreCount(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore)
{
    semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return semaphore->count;
}

os_size_t osSemaphoreManagerGetMaxSemaphoreCount(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore)
{
    semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return semaphore->maxCount;
}

int osSemaphoreManagerRemoveTask(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore, OsTask *task)
{
    semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (OS_TASK_TYPE_RT == task->taskControlBlock.schedulerId)
    {
        osDeleteNode(&semaphore->waitRtTaskList, &task->exNode.treeNode);
        semaphore->highPriorityTask = osGetLeftmostNode(semaphore->waitRtTaskList);
    }
    else
    {
        osRemoveFromList(&semaphore->waitTaskList, &task->exNode.listNode);
    }
    return 0;
}

int osSemaphoreManagerReset(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore)
{
    semaphoreManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    semaphore->count = 0;
    return 0;
}