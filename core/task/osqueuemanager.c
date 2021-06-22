#include "osqueuemanager.h"
#include "ostask.h"
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

int osQueueManagerQueueInit(OsQueueManager *queueManager, OsQueue *queue, os_size_t queueLength, os_size_t messageSize)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    queue->messages = NULL;
    queue->messageCount = 0;
    queue->length = queueLength;
    queue->messageSize = messageSize;
    queue->highPriorityTask = NULL;
    queue->waitRtTaskList = NULL;
    queue->waitTaskList = NULL;
    return 0;
}

int osQueueManagerSend(OsQueueManager *queueManager, OsQueue *queue, OsMessage *message)
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
        if (NULL == task)
        {
            osInsertToBack(&queue->messages, &message->node);
            queue->messageCount++;
            ret = 0;
        }
        else
        {
            task->arg = message;
            if (OS_TASK_STATE_BLOCKED == task->taskControlBlock.taskState)
            {
                task->taskControlBlock.taskState = OS_TASK_STATE_SUSPENDED;
                ret = osTaskResume(task->tid);
            }
            else
            {
                ret = osTaskWakeup(task->tid);
            }
        }
    }
    return ret;
}

int osQueueManagerSendToFront(OsQueueManager *queueManager, OsQueue *queue, OsMessage *message)
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
        if (NULL == task)
        {
            osInsertToFront(&queue->messages, &message->node);
            queue->messageCount++;
            ret = 0;
        }
        else
        {
            task->arg = message;
            if (OS_TASK_STATE_BLOCKED == task->taskControlBlock.taskState)
            {
                task->taskControlBlock.taskState = OS_TASK_STATE_SUSPENDED;
                ret = osTaskResume(task->tid);
            }
            else
            {
                ret = osTaskWakeup(task->tid);
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

int osQueueManagerReceive(OsQueueManager *queueManager, OsMessage **message, OsTask **nextTask, OsQueue *queue, os_size_t wait)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    *nextTask = NULL;
    *message = osQueueManagerQueuePop(queueManager, queue);
    if (*message != NULL)
    {
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

os_size_t osQueueManagerGetMessageCount(OsQueueManager *queueManager, OsQueue *queue)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return queue->messageCount;
}

os_size_t osQueueManagerGetQueueLength(OsQueueManager *queueManager, OsQueue *queue)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return queue->length;
}

OsMessage *osQueueManagerQueuePop(OsQueueManager *queueManager, OsQueue *queue)
{
    queueManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsMessage *message = (OsMessage *)queue->messages;
    if (message != NULL)
    {
        osRemoveFromList(&queue->messages, queue->messages);
        queue->messageCount--;
    }
    return message;
}

int osQueueManagerRemoveTask(OsQueueManager *queueManager, OsQueue *queue, OsTask *task)
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