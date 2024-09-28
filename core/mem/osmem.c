#include "osmem.h"
#include "osmemmanager.h"
#include "osmutex.h"
#define ENABLE_MEM_LOG 0
#if ENABLE_MEM_LOG
#define memLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define memLog(format, ...) (void)0
#endif
static OsMemManager sMemManager;
static OsMutex sMutex;
size_t osMemInit(void *startAddress, size_t size)
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexCreate(&sMutex);
    return osMemManagerInit(&sMemManager, startAddress, size);
}

void *osMalloc(size_t size)
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    void *ret = osMemManagerAlloc(&sMemManager, size);
    osMutexUnlock(&sMutex);
    return ret;
}

void *osRealloc(void *address, size_t newSize)
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    void *ret = osMemManagerRealloc(&sMemManager, address, newSize);
    osMutexUnlock(&sMutex);
    return ret;
}

int osFree(void *address)
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    int ret = osMemManagerFree(&sMemManager, address);
    osMutexUnlock(&sMutex);
    return ret;
}

size_t osMallocUsableSize(const void *address)
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    int ret = osMemManagerAllocUsableSize(&sMemManager, address);
    osMutexUnlock(&sMutex);
    return ret;
}

void *osAllocPages(size_t n)
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    void *ret = osMemManagerAllocPages(&sMemManager, n);
    osMutexUnlock(&sMutex);
    return ret;
}

int osFreePages(void *pages)
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    int ret = osMemManagerFreePages(&sMemManager, pages);
    osMutexUnlock(&sMutex);
    return ret;
}

size_t osTotalMem()
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osMemManagerTotalMem(&sMemManager);
}

size_t osFreeMem()
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osMemManagerFreeMem(&sMemManager);
}

size_t osTotalPage()
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osMemManagerTotalPage(&sMemManager);
}

size_t osFreePage()
{
    memLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osMemManagerFreePage(&sMemManager);
}