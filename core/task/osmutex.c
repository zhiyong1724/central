#include "osmutex.h"
#include "ossemaphore.h"
#include "ostaskmanager.h"
#define ENABLE_MUTEX_LOG 0
#if ENABLE_MUTEX_LOG
#define mutexLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define mutexLog(format, ...) (void)0
#endif
os_size_t portDisableInterrupts();
int portRecoveryInterrupts(os_size_t state);
OsTask *osTaskGetRunningTask();
int osMutexCreate(os_mutex_t mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osSemaphoreCreate(&mutex->semaphore, 1, 1);
}

int osMutexDestory(os_mutex_t mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osSemaphoreDestory(&mutex->semaphore);
}

int osMutexLock(os_mutex_t mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = osSemaphoreWait(&mutex->semaphore, OS_SEMAPHORE_MAX_WAIT_TIME);
    return ret;
}

int osMutexUnlock(os_mutex_t mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = osSemaphorePost(&mutex->semaphore);
    return ret;
}

int osRecursiveMutexCreate(os_recursive_mutex_t mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    mutex->owner = NULL;
    mutex->recursiveCount = 0;
    return osSemaphoreCreate(&mutex->semaphore, 1, 1);
}

int osRecursiveMutexDestory(os_recursive_mutex_t mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osSemaphoreDestory(&mutex->semaphore);
}

int osRecursiveMutexLock(os_recursive_mutex_t mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    os_size_t state = portDisableInterrupts();
    if (0 == mutex->recursiveCount)
    {
        mutex->owner = osTaskGetRunningTask();
        mutex->recursiveCount++;
        ret = osSemaphoreWait(&mutex->semaphore, OS_SEMAPHORE_MAX_WAIT_TIME);
    }
    else
    {
        if (osTaskGetRunningTask() == mutex->owner)
        {
            mutex->recursiveCount++;
            ret = 0;
        }
        else
        {
            ret = osSemaphoreWait(&mutex->semaphore, OS_SEMAPHORE_MAX_WAIT_TIME);
        }
    }
    portRecoveryInterrupts(state);
    return ret;
}

int osRecursiveMutexUnlock(os_recursive_mutex_t mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    os_size_t state = portDisableInterrupts();
    if (mutex->recursiveCount > 0)
    {
        if (osTaskGetRunningTask() == mutex->owner)
        {
            mutex->recursiveCount--;
            ret = 0;
        }
    }
    if (0 == mutex->recursiveCount)
    {
        ret = osSemaphorePost(&mutex->semaphore);
    }
    portRecoveryInterrupts(state);
    return ret;
}