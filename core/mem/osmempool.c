#include "osmempool.h"
#include "osstring.h"
#define ENABLE_MEM_POOL_LOG 0
#if ENABLE_MEM_POOL_LOG
#define memPoolLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define memPoolLog(format, ...) (void)0
#endif

static void *addressAlign(void *address, size_t size)
{
    memPoolLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    unsigned char *startAddress = (unsigned char *)address;
    size_t offset = (size_t)startAddress % size;
    if (offset > 0)
    {
        startAddress += size - offset;
    }
    return startAddress;
}

static void fillBitmap(OsMemPool *memPool)
{
    memPoolLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t count = memPool->totalPageNum / 8;
    size_t i = 0;
    for (; i < count; i++)
    {
        memPool->bitmap[i] = 0;
    }
    count = memPool->totalPageNum % 8;
    if (count > 0)
    {
        memPool->bitmap[i] = 0;
        for (size_t j = 0; j < count; j++)
        {
            memPool->bitmap[i] |= 0x80 >> j;
        }
        memPool->bitmap[i] = ~memPool->bitmap[i];
    }
}

static void fillPageList(OsMemPool *memPool, unsigned char *address)
{
    memPoolLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    memPool->pageList = NULL;
    for (size_t i = 0; i < memPool->totalPageNum; i++)
    {
        osInsertToSingleList(&memPool->pageList, (OsSingleListNode *)address);
        address += memPool->pageSize;
    }
    
}

size_t osMemPoolInit(OsMemPool *memPool, void *startAddress, size_t size, size_t pageSize)
{
    memPoolLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    pageSize = (size_t)addressAlign((void *)pageSize, sizeof(void *));
    memPool->totalPageNum = size / pageSize;
    osAssert(memPool->totalPageNum >= 2);
    if (memPool->totalPageNum >= 2)
    {
        memPool->pageSize = pageSize;

        memPool->bitmap = (unsigned char *)startAddress;
        size_t offset = memPool->totalPageNum / 8;
        if (memPool->totalPageNum % 8 > 0)
        {
            offset++;
        }
        size -= offset;

        unsigned char *bitmapEnd = memPool->bitmap + offset;
        unsigned char *memStart = (unsigned char *)addressAlign(bitmapEnd, sizeof(long long));
        offset = memStart - bitmapEnd;
        size -= offset;
        memPool->totalPageNum = size / pageSize;
        osAssert(memPool->totalPageNum >= 2);
        if (memPool->totalPageNum >= 2)
        {
            fillBitmap(memPool);
            fillPageList(memPool, memStart);
            memPool->freePageNum = memPool->totalPageNum;
            memPool->startAddress = memStart;
        }
    }
    return memPool->freePageNum;
}

void *osMemPoolAllocPage(OsMemPool *memPool)
{
    memPoolLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    void *ret = NULL;
    osAssert(memPool->freePageNum > 0 && memPool->pageList != NULL);
    if (memPool->freePageNum > 0 && memPool->pageList != NULL)
    {
        ret = memPool->pageList;
        osRemoveFromSingleList(&memPool->pageList);
        size_t index = ((char *)ret - (char *)memPool->startAddress) / memPool->pageSize;
        size_t i = index / 8;
        size_t j = index % 8;
        unsigned char mask = 0x80;
        mask >>= j;
        memPool->bitmap[i] |= mask;
        memPool->freePageNum--;
    }
    return ret;
}

int osMemPoolFreePage(OsMemPool *memPool, void *page)
{
    memPoolLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(page >= memPool->startAddress);
    if (page >= memPool->startAddress)
    {
        osAssert(0 == ((char *)page - (char *)memPool->startAddress) % memPool->pageSize);
        if (0 == ((char *)page - (char *)memPool->startAddress) % memPool->pageSize)
        {
            size_t index = ((char *)page - (char *)memPool->startAddress) / memPool->pageSize;
            osAssert(index < memPool->totalPageNum);
            if (index < memPool->totalPageNum)
            {
                size_t i = index / 8;
                size_t j = index % 8;
                unsigned char mask = 0x80;
                mask >>= j;
                mask &= memPool->bitmap[i];
                osAssert(mask > 0);
                if (mask > 0)
                {
                    osInsertToSingleList(&memPool->pageList, (OsSingleListNode *)page);
                    mask = ~mask;
                    memPool->bitmap[i] &= mask;
                    memPool->freePageNum++;
                    ret = 0;
                }
                
            }
        }
    }
    return ret;
}

size_t osMemPoolTotalPageNum(OsMemPool *memPool)
{
    memPoolLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return memPool->totalPageNum;
}

size_t osMemPoolFreePageNum(OsMemPool *memPool)
{
    memPoolLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return memPool->freePageNum;
}