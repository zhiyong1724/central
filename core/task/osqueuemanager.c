#include "osqueuemanager.h"
#include "ostask.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_QUEUEMANAGER_LOG 0
#if ENABLE_QUEUEMANAGER_LOG
#define queueManagerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define queueManagerLog(format, ...) (void)0
#endif
#define OS_MESSAGE_MAX_WAIT_TIME ((os_size_t)-1 / 1000 / 1000)
int osTaskWakeup(os_tid_t tid);
OsTask *osTaskGetRunningTask();
int osQueueManagerInit(OsQueueManager *queueManager, OsTaskManager *taskManager)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    queueManager->taskManager = taskManager;
    return 0;
}

int osQueueManagerQueueInit(OsQueueManager *queueManager, OsMsgQueue *queue, os_size_t queueLength, os_size_t messageSize)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    queue->buffer = (os_byte_t *)osMalloc(queueLength * messageSize);
    if (queue->buffer != NULL)
    {
        queue->messageCount = 0;
        queue->length = queueLength;
        queue->messageSize = messageSize;
        queue->writeIndex = 0;
        queue->readIndex = 0;
        queue->highPriorityTask = NULL;
        queue->waitRtTaskList = NULL;
        queue->waitTaskList = NULL;
        ret = 0;
    }
    return ret;
}

int osQueueManagerQueueUninit(OsQueueManager *queueManager, OsMsgQueue *queue)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (queue->buffer != NULL)
    {
        osFree(queue->buffer);
        queue->buffer = NULL;
    }
    queue->messageCount = 0;
    queue->length = 0;
    queue->messageSize = 0;
    queue->writeIndex = 0;
    queue->readIndex = 0;
    queue->highPriorityTask = NULL;
    queue->waitRtTaskList = NULL;
    queue->waitTaskList = NULL;
    return 0;
}

int osQueueManagerSend(OsQueueManager *queueManager, OsMsgQueue *queue, void *message, OsTask **nextTask)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    if (queue->messageCount < queue->length)
    {
        OsTask *task = NULL;
        os_byte_t *highPriorityTask = (os_byte_t *)queue->highPriorityTask;
        if (highPriorityTask != NULL)
        {
            osDeleteNode(&queue->waitRtTaskList, (OsTreeNode *)highPriorityTask);
            queue->highPriorityTask = osGetLeftmostNode(queue->waitRtTaskList);
            task = (OsTask *)(highPriorityTask - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task->realTaskControlBlock));
        }
        if (NULL == task)
        {
            highPriorityTask = (os_byte_t *)queue->waitTaskList;
            if (highPriorityTask != NULL)
            {
                osRemoveFromList(&queue->waitTaskList, (OsListNode *)highPriorityTask);
                task = (OsTask *)(highPriorityTask - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task->realTaskControlBlock));
            }
        }
        osMemCpy(&queue->buffer[queue->writeIndex * queue->messageSize], message, queue->messageSize);
        queue->writeIndex++;
        if (queue->writeIndex >= queue->length)
        {
            queue->writeIndex = 0;
        }
        queue->messageCount++;
        if (task != NULL)
        {
            task->arg = queue;
            if (OS_TASK_STATE_BLOCKED == task->taskControlBlock.taskState)
            {
                task->taskControlBlock.taskState = OS_TASK_STATE_SUSPENDED;
                ret = osTaskManagerResume(queueManager->taskManager, nextTask, task->tid);
            }
            else
            {
                ret = osTaskManagerWakeup(queueManager->taskManager, nextTask, task->tid);
            }
        }
    }
    return ret;
}

int osQueueManagerSendToFront(OsQueueManager *queueManager, OsMsgQueue *queue, void *message, OsTask **nextTask)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    if (queue->messageCount < queue->length)
    {
        OsTask *task = NULL;
        os_byte_t *highPriorityTask = (os_byte_t *)queue->highPriorityTask;
        if (highPriorityTask != NULL)
        {
            osDeleteNode(&queue->waitRtTaskList, (OsTreeNode *)highPriorityTask);
            queue->highPriorityTask = osGetLeftmostNode(queue->waitRtTaskList);
            task = (OsTask *)(highPriorityTask - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task->realTaskControlBlock));
        }
        if (NULL == task)
        {
            highPriorityTask = (os_byte_t *)queue->waitTaskList;
            if (highPriorityTask != NULL)
            {
                osRemoveFromList(&queue->waitTaskList, (OsListNode *)highPriorityTask);
                task = (OsTask *)(highPriorityTask - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task->realTaskControlBlock));
            }
        }
        if (queue->readIndex > 0)
        {
            queue->readIndex--;
        }
        else
        {
            queue->readIndex = queue->length - 1;
        }
        osMemCpy(&queue->buffer[queue->readIndex * queue->messageSize], message, queue->messageSize);
        queue->messageCount++;
        if (task != NULL)
        {
            task->arg = queue;
            if (OS_TASK_STATE_BLOCKED == task->taskControlBlock.taskState)
            {
                task->taskControlBlock.taskState = OS_TASK_STATE_SUSPENDED;
                ret = osTaskManagerResume(queueManager->taskManager, nextTask, task->tid);
            }
            else
            {
                ret = osTaskManagerWakeup(queueManager->taskManager, nextTask, task->tid);
            }
        }
    }
    return ret;
}

static int onCompare(void *key1, void *key2, void *arg)
{
	queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
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

int osQueueManagerReceive(OsQueueManager *queueManager, void *message, OsTask **nextTask, OsMsgQueue *queue, uint64_t wait)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    *nextTask = NULL;
    void *result = osQueueManagerQueuePop(queueManager, queue);
    if (result != NULL)
    {
        osMemCpy(message, result, queue->messageSize);
        ret = 0;
    }
    else
    {
        OsTask *task = osTaskManagerGetRunningTask(queueManager->taskManager);
        if (wait > 0 && wait < OS_MESSAGE_MAX_WAIT_TIME)
        {
            ret = osTaskManagerSleep(queueManager->taskManager, nextTask, wait * 1000 * 1000);
        }
        else if (OS_MESSAGE_MAX_WAIT_TIME == wait)
        {
            ret = osTaskManagerSupend(queueManager->taskManager, nextTask, task->tid);
            if (0 == ret)
            {
                task->taskControlBlock.taskState = OS_TASK_STATE_BLOCKED;
            }
        }
        if (0 == ret)
        {
            if (OS_TASK_TYPE_RT == task->taskControlBlock.schedulerId)
            {
                osInsertNode(&queue->waitRtTaskList, &task->exNode.treeNode, onCompare, NULL);
                if (NULL == queue->highPriorityTask)
                {
                    queue->highPriorityTask = &task->exNode.treeNode;
                }
                else
                {
                    OsTask *highPriorityTask = (OsTask *)((os_byte_t *)queue->highPriorityTask - sizeof(OsTaskControlBlock) - sizeof(OsListNode) - sizeof(task->realTaskControlBlock));
                    if (task->realTaskControlBlock.rtTaskControlBlock.priority < highPriorityTask->realTaskControlBlock.rtTaskControlBlock.priority)
                    {
                        queue->highPriorityTask = &task->exNode.treeNode;
                    }
                }
            }
            else
            {
                osInsertToBack(&queue->waitTaskList, &task->exNode.listNode);
            }
        }
    }
    return ret;
}

os_size_t osQueueManagerGetMessageCount(OsQueueManager *queueManager, OsMsgQueue *queue)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return queue->messageCount;
}

os_size_t osQueueManagerGetQueueLength(OsQueueManager *queueManager, OsMsgQueue *queue)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return queue->length;
}

void *osQueueManagerQueuePop(OsQueueManager *queueManager, OsMsgQueue *queue)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    void *ret = NULL;
    if (queue->messageCount > 0)
    {
        ret = &queue->buffer[queue->readIndex * queue->messageSize];
        queue->readIndex++;
        if (queue->readIndex >= queue->length)
        {
            queue->readIndex = 0;
        }
        queue->messageCount--;
    }
    return ret;
}

int osQueueManagerRemoveTask(OsQueueManager *queueManager, OsMsgQueue *queue, OsTask *task)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (OS_TASK_TYPE_RT == task->taskControlBlock.schedulerId)
    {
        osDeleteNode(&queue->waitRtTaskList, &task->exNode.treeNode);
        queue->highPriorityTask = osGetLeftmostNode(queue->waitRtTaskList);
    }
    else
    {
        osRemoveFromList(&queue->waitTaskList, &task->exNode.listNode);
    }
    return 0;
}

int osQueueManagerReset(OsQueueManager *queueManager, OsMsgQueue *queue)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    queue->messageCount = 0;
    queue->writeIndex = 0;
    queue->readIndex = 0;
    return 0;
}