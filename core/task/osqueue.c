#include "osqueue.h"
#include "osqueuemanager.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_QUEUE_LOG 0
#if ENABLE_QUEUE_LOG
#define queueLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define queueLog(format, ...) (void)0
#endif
int portYield(void **stackTop);
os_size_t portDisableInterrupts();
int portRecoveryInterrupts(os_size_t state);
OsTask *osTaskGetRunningTask();
static OsQueueManager *sQueueManager;
int osQueueInit(OsQueueManager *queueManager, OsTaskManager *taskManager)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sQueueManager = queueManager;
    return osQueueManagerInit(sQueueManager, taskManager);
}

int osQueueCreate(os_queue_t queue, os_size_t queueLength, os_size_t messageSize)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    if (0 == queueLength)
    {
        queueLength = OS_MAX_QUEUE_LENGTH;
    }
    ret = osQueueManagerQueueInit(sQueueManager, queue, queueLength, messageSize);
    return ret;
}

int osQueueDestory(os_queue_t queue)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    os_size_t state = portDisableInterrupts();
    osAssert(NULL == queue->waitTaskList && NULL == queue->waitTaskList);
    if (NULL == queue->waitTaskList && NULL == queue->waitTaskList)
    {
        for (OsMessage *message = osQueueManagerQueuePop(sQueueManager, queue); message != NULL; message = osQueueManagerQueuePop(sQueueManager, queue))
        {
            osFree(message);
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osQueueReset(os_queue_t queue)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    for (OsMessage *message = osQueueManagerQueuePop(sQueueManager, queue); message != NULL; message = osQueueManagerQueuePop(sQueueManager, queue))
    {
        osFree(message);
    }
    portRecoveryInterrupts(state);
    return 0;
}

int osQueueSend(os_queue_t queue, void *message)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = NULL;
    os_size_t state = portDisableInterrupts();
    OsMessage *osMessage = (OsMessage *)osMalloc(sizeof(OsMessage) + queue->messageSize);
    if (osMessage != NULL)
    {
        portRecoveryInterrupts(state);
        osMemCpy(osMessage + 1, message, queue->messageSize);
        state = portDisableInterrupts();
        ret = osQueueManagerSend(sQueueManager, queue, osMessage, &task);
        if (0 == ret)
        {
            if (task != NULL)
            {
                portYield(&task->stackTop);
                portRecoveryInterrupts(state);
                state = portDisableInterrupts();
            }
        }
        else
        {
            osFree(osMessage);
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osQueueSendToFront(os_queue_t queue, void *message)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = NULL;
    os_size_t state = portDisableInterrupts();
    OsMessage *osMessage = (OsMessage *)osMalloc(sizeof(OsMessage) + queue->messageSize);
    if (osMessage != NULL)
    {
        portRecoveryInterrupts(state);
        osMemCpy(osMessage + 1, message, queue->messageSize);
        state = portDisableInterrupts();
        ret = osQueueManagerSendToFront(sQueueManager, queue, osMessage, &task);
        if (0 == ret)
        {
            if (task != NULL)
            {
                portYield(&task->stackTop);
                portRecoveryInterrupts(state);
                state = portDisableInterrupts();
            }
        }
        else
        {
            osFree(osMessage);
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osQueueReceive(os_queue_t queue, void *message, uint64_t wait)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsMessage *osMessage = NULL;
    OsTask *task = NULL;
    os_size_t state = portDisableInterrupts();
    ret = osQueueManagerReceive(sQueueManager, &osMessage, &task, queue, wait);
    if (0 == ret)
    {
        if (task != NULL)
        {
            portYield(&task->stackTop);
            portRecoveryInterrupts(state);
            state = portDisableInterrupts();
            task = osTaskGetRunningTask();
            osMessage = (OsMessage *)task->arg;
            task->arg = NULL;
            if (osMessage != NULL)
            {
                portRecoveryInterrupts(state);
                osMemCpy(message, osMessage + 1, queue->messageSize);
                state = portDisableInterrupts();
                osFree(osMessage);
            }
            else
            {
                osQueueManagerRemoveTask(sQueueManager, queue, task);
                ret = -1;
            }
        }
        else
        {
            portRecoveryInterrupts(state);
            osMemCpy(message, osMessage + 1, queue->messageSize);
            state = portDisableInterrupts();
            osFree(osMessage);
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

os_size_t osQueueGetMessageCount(os_queue_t queue)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osQueueManagerGetMessageCount(sQueueManager, queue);
}

os_size_t osQueueGetQueueLength(os_queue_t queue)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osQueueManagerGetQueueLength(sQueueManager, queue);
}