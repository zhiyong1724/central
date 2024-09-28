#include "osmsgqueue.h"
#include "osqueuemanager.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_MSGQUEUE_LOG 0
#if ENABLE_MSGQUEUE_LOG
#define msgQueueLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define msgQueueLog(format, ...) (void)0
#endif
int portYield(stack_size_t **stackTop);
size_t portDisableInterrupts();
int portRecoveryInterrupts(size_t state);
OsTask *osTaskGetRunningTask();
static OsQueueManager *sQueueManager;
int osMsgQueueInit(OsQueueManager *queueManager, OsTaskManager *taskManager)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sQueueManager = queueManager;
    return osQueueManagerInit(sQueueManager, taskManager);
}

int osMsgQueueCreate(os_queue_t queue, size_t queueLength, size_t messageSize)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    if (0 == queueLength)
    {
        queueLength = OS_MAX_QUEUE_LENGTH;
    }
    ret = osQueueManagerQueueInit(sQueueManager, queue, queueLength, messageSize);
    return ret;
}

int osMsgQueueDestory(os_queue_t queue)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    size_t state = portDisableInterrupts();
    osAssert(NULL == queue->waitTaskList && NULL == queue->waitTaskList);
    if (NULL == queue->waitTaskList && NULL == queue->waitTaskList)
    {
        osQueueManagerQueueUninit(sQueueManager, queue);
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osMsgQueueReset(os_queue_t queue)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t state = portDisableInterrupts();
    osQueueManagerReset(sQueueManager, queue);
    portRecoveryInterrupts(state);
    return 0;
}

int osMsgQueueSend(os_queue_t queue, void *message)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = NULL;
    size_t state = portDisableInterrupts();
    ret = osQueueManagerSend(sQueueManager, queue, message, &task);
    if (0 == ret)
    {
        if (task != NULL)
        {
            portYield(&task->stackTop);
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osMsgQueueSendToFront(os_queue_t queue, void *message)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = NULL;
    size_t state = portDisableInterrupts();
    ret = osQueueManagerSendToFront(sQueueManager, queue, message, &task);
    if (0 == ret)
    {
        if (task != NULL)
        {
            portYield(&task->stackTop);
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osMsgQueueReceive(os_queue_t queue, void *message, uint64_t wait)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = NULL;
    size_t state = portDisableInterrupts();
    ret = osQueueManagerReceive(sQueueManager, message, &task, queue, wait);
    if (0 == ret)
    {
        if (task != NULL)
        {
            portYield(&task->stackTop);
            portRecoveryInterrupts(state);
            state = portDisableInterrupts();
            task = osTaskGetRunningTask();
            if (task->arg != NULL)
            {
                task->arg = NULL;
                ret = osMsgQueueReceive(queue, message, wait);
            }
            else
            {
                osQueueManagerRemoveTask(sQueueManager, queue, task);
                ret = -1;
            }
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

size_t osMsgQueueGetMessageCount(os_queue_t queue)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osQueueManagerGetMessageCount(sQueueManager, queue);
}

size_t osMsgQueueGetQueueLength(os_queue_t queue)
{
    msgQueueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osQueueManagerGetQueueLength(sQueueManager, queue);
}