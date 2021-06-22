#include "osmutex.h"
#include "osmem.h"
#include "ossemaphore.h"
#include "ostaskmanager.h"
#define ENABLE_MUTEX_LOG 0
#if ENABLE_MUTEX_LOG
#define mutexLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define mutexLog(format, ...) (void)0
#endif
os_size_t portDisableInterrupts();
int portEnableInterrupts();
int portEnableTaskInterrupts();
int portRecoveryInterrupts(os_size_t state);
OsTask *osTaskGetRunningTask();
int osMutexCreate(os_mutex_h *mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    *mutex = (os_mutex_h)osMalloc(sizeof(OsMutex));
    osAssert(*mutex != NULL);
    if (*mutex != NULL)
    {
        ret = osMutexCreateStatic(*mutex);
    }
    return ret;
}

int osMutexCreateStatic(os_mutex_h mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osSemaphoreCreateStatic(&mutex->semaphore, 1, 1);
}

int osMutexDestory(os_mutex_h mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osSemaphoreDestory(&mutex->semaphore);
}

int osMutexLock(os_mutex_h mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    portDisableInterrupts();
    int ret = osSemaphoreWait(&mutex->semaphore, OS_SEMAPHORE_MAX_WAIT_TIME);
    portEnableTaskInterrupts();
    return ret;
}

int osMutexUnlock(os_mutex_h mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    portDisableInterrupts();
    int ret = osSemaphorePost(&mutex->semaphore);
    portEnableInterrupts();
    return ret;
}

int osRecursiveMutexCreate(os_recursive_mutex_h *mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    *mutex = (os_recursive_mutex_h)osMalloc(sizeof(OsRecursiveMutex));
    osAssert(*mutex != NULL);
    if (*mutex != NULL)
    {
        ret = osRecursiveMutexCreateStatic(*mutex);
    }
    return ret;
}

int osRecursiveMutexCreateStatic(os_recursive_mutex_h mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    mutex->owner = NULL;
    mutex->recursiveCount = 0;
    return osSemaphoreCreateStatic(&mutex->semaphore, 1, 1);
}

int osRecursiveMutexDestory(os_recursive_mutex_h mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osSemaphoreDestory(&mutex->semaphore);
}

int osRecursiveMutexLock(os_recursive_mutex_h mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    portDisableInterrupts();
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
    portEnableTaskInterrupts();
    return ret;
}

int osRecursiveMutexUnlock(os_recursive_mutex_h mutex)
{
    mutexLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    portDisableInterrupts();
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
        portEnableInterrupts();
    }
    else
    {
        portEnableTaskInterrupts();
    }
    return ret;
}