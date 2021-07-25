#include "ossemaphore.h"
#include "osmem.h"
#include "ossemaphoremanager.h"
#define ENABLE_SEMAPHORE_LOG 0
#if ENABLE_SEMAPHORE_LOG
#define semaphoreLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define semaphoreLog(format, ...) (void)0
#endif
int portYield(void **stackTop);
os_size_t portDisableInterrupts();
int portRecoveryInterrupts(os_size_t state);
OsTask *osTaskGetRunningTask();
static OsSemaphoreManager *sSemaphoreManager;
int osSemaphoreInit(OsSemaphoreManager *semaphoreManager, OsTaskManager *taskManager)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sSemaphoreManager = semaphoreManager;
    return osSemaphoreManagerInit(sSemaphoreManager, taskManager);
}

int osSemaphoreCreate(os_semaphore_h *semaphore, os_size_t count, os_size_t maxCount)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    *semaphore = (os_semaphore_h)osMalloc(sizeof(OsSemaphore));
    osAssert(*semaphore != NULL);
    if (*semaphore != NULL)
    {
        ret = osSemaphoreCreateStatic(*semaphore, count, maxCount);
    }
    return ret;
}

int osSemaphoreCreateStatic(os_semaphore_h semaphore, os_size_t count, os_size_t maxCount)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    if (0 == maxCount)
    {
        maxCount = OS_MAX_SEMAPHORE_COUNT;
    }
    ret = osSemaphoreManagerSemaphoreInit(sSemaphoreManager, semaphore, count, maxCount);
    return ret;
}

int osSemaphoreDestory(os_semaphore_h semaphore)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    os_size_t state = portDisableInterrupts();
    osAssert(NULL == semaphore->waitTaskList && NULL == semaphore->waitTaskList);
    if (NULL == semaphore->waitTaskList && NULL == semaphore->waitTaskList)
    {
        osSemaphoreReset(semaphore);
        osFree(semaphore);
        ret = 0;
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osSemaphoreReset(os_semaphore_h semaphore)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osSemaphoreManagerReset(sSemaphoreManager, semaphore);
    portRecoveryInterrupts(state);
    return ret;
}

int osSemaphorePost(os_semaphore_h semaphore)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t state = portDisableInterrupts();
    int ret = osSemaphoreManagerPost(sSemaphoreManager, semaphore);
    portRecoveryInterrupts(state);
    return ret;
}

int osSemaphoreWait(os_semaphore_h semaphore, uint64_t wait)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsTask *task = NULL;
    os_size_t state = portDisableInterrupts();
    ret = osSemaphoreManagerWait(sSemaphoreManager, &task, semaphore, wait);
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
            }
            else
            {
                osSemaphoreManagerRemoveTask(sSemaphoreManager, semaphore, task);
                ret = -1;
            }
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

os_size_t osSemaphoreGetSemaphoreCount(os_semaphore_h semaphore)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osSemaphoreManagerGetSemaphoreCount(sSemaphoreManager, semaphore);
}

os_size_t osSemaphoreGetMaxSemaphoreCount(os_semaphore_h semaphore)
{
    semaphoreLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osSemaphoreManagerGetMaxSemaphoreCount(sSemaphoreManager, semaphore);
}