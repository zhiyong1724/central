#include "oscentral.h"
#include "osmem.h"
#include "ostaskmanager.h"
#include "ossemaphoremanager.h"
#include "osqueuemanager.h"
#include "osvfs.h"
#define ENABLE_CENTRAL_LOG 0
#if ENABLE_CENTRAL_LOG
#define centralLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define centralLog(format, ...) (void)0
#endif
#define OS_VERSION 100
static OsTaskManager sTaskManager;
static OsSemaphoreManager sSemaphoreManager;
static OsQueueManager sQueueManager;
#if OS_USE_VFS
static OsVFS sVFS;
#endif
os_size_t osMemInit(void *startAddress, os_size_t size);
int osTaskInit(OsTaskManager *taskManager, os_size_t clockPeriod);
int osSemaphoreInit(OsSemaphoreManager *semaphoreManager, OsTaskManager *taskManager);
int osMsgQueueInit(OsQueueManager *queueManager, OsTaskManager *taskManager);
#if OS_USE_VFS
int osFInit(OsVFS *vfs);
#endif
int osInit()
{
    centralLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (osMemInit(OS_HEAP_ADDRESS, OS_HEAP_SIZE) == 0)
    {
        return -1;
    }
    if (osTaskInit(&sTaskManager, OS_SYS_CLOCK_PERIOD_NS) != 0)
    {
        return -2;
    }
    if (osSemaphoreInit(&sSemaphoreManager, &sTaskManager) != 0)
    {
        return -3;
    }
    if (osMsgQueueInit(&sQueueManager, &sTaskManager) != 0)
    {
        return -4;
    }
    #if OS_USE_VFS
    if (osFInit(&sVFS) != 0)
    {
        return -5;
    }
    #endif
    return 0;
}

int osVersion()
{
    centralLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return OS_VERSION;
}