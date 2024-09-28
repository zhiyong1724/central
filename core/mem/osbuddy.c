#include "osbuddy.h"
#include "osstring.h"
#define ENABLE_BUDDY_LOG 0
#if ENABLE_BUDDY_LOG
#define buddyLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define buddyLog(format, ...) (void)0
#endif

static void *addressAlign(void *address, size_t size)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    unsigned char *startAddress = (unsigned char *)address;
    size_t offset = (size_t)startAddress % size;
    if (offset > 0)
    {
        startAddress += size - offset;
    }
    return startAddress;
}

static size_t calculateGroupCount(size_t pageNum)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t i = 1;
    size_t compareValue = 2;
    for (; compareValue <= pageNum && i < 128; compareValue <<= 1)
    {
        i++;
    }
    return i;
}

static unsigned char *fillBlockArray(OsBuddy *buddy, unsigned char *address, size_t blockArrayId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t pageCount = 1 << blockArrayId;
    for (; buddy->totalPageNum - buddy->freePageNum >= pageCount; buddy->freePageNum += pageCount)
    {
        osInsertToFront(&buddy->blockListArray[blockArrayId], (OsListNode *)address);
        address += pageCount * OS_BUDDY_PAGE_SIZE;
    }
    return address;
}

size_t osBuddyInit(OsBuddy *buddy, void *startAddress, size_t size)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    buddy->freePageNum = 0;
    buddy->totalPageNum = size / OS_BUDDY_PAGE_SIZE;
    osAssert(buddy->totalPageNum >= 2);
    if (buddy->totalPageNum >= 2)
    {
        buddy->blockGroup = (unsigned char *)startAddress;
        osMemSet(buddy->blockGroup, 0, buddy->totalPageNum);
        size -= buddy->totalPageNum;
        unsigned char *blockGroupEnd = buddy->blockGroup + buddy->totalPageNum;

        unsigned char *blockListArrayStart = (unsigned char *)addressAlign(blockGroupEnd, sizeof(void *));
        size_t offset = blockListArrayStart - blockGroupEnd;
        size -= offset;
        buddy->blockListArray = (OsListNode **)blockListArrayStart;
        buddy->groupCount = calculateGroupCount(buddy->totalPageNum);
        size -= buddy->groupCount * sizeof(void *);
        unsigned char *blockListArrayEnd = blockListArrayStart + buddy->groupCount * sizeof(void *);
        
        buddy->startAddress = (unsigned char *)addressAlign(blockListArrayEnd, OS_BUDDY_PAGE_SIZE);
        offset = (unsigned char *)buddy->startAddress - blockListArrayEnd;
        size -= offset;

        buddy->totalPageNum = size / OS_BUDDY_PAGE_SIZE;
        osAssert(buddy->totalPageNum >= 2);
        if (buddy->totalPageNum >= 2)
        {
            buddy->groupCount = calculateGroupCount(buddy->totalPageNum);
            unsigned char *handle = (unsigned char *)buddy->startAddress;
            for (int i = 0; i < (int)buddy->groupCount; i++)
            {
                buddy->blockListArray[i] = NULL;
            }
            
            for (int i = (int)buddy->groupCount - 1; i >= 0; i--)
            {
                handle = fillBlockArray(buddy, handle, (size_t)i);
                if (buddy->freePageNum == buddy->totalPageNum)
                {
                    break;
                }
            }
        }
    }
    return buddy->freePageNum;
}

static void setBlockGroup(OsBuddy *buddy, void *address, unsigned char value)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t index = ((char *)address - (char *)buddy->startAddress) / OS_BUDDY_PAGE_SIZE;
    buddy->blockGroup[index] = value;
}

static unsigned char getBlockGroup(OsBuddy *buddy, void *address)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t index = ((char *)address - (char *)buddy->startAddress) / OS_BUDDY_PAGE_SIZE;
    return buddy->blockGroup[index];
}

static void *splitBlock(OsBuddy *buddy, void *address, size_t groupId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    setBlockGroup(buddy, address, (unsigned char)groupId);
    unsigned char *blockB = (unsigned char *)address;
    blockB += (1 << (groupId - 1)) * OS_BUDDY_PAGE_SIZE;
    setBlockGroup(buddy, blockB, (unsigned char)groupId * -1);
    osInsertToFront(&buddy->blockListArray[groupId - 1], (OsListNode *)blockB);
    return address;
}

static void *allocPages(OsBuddy *buddy, size_t groupId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    void *ret = NULL;
    osAssert(groupId < buddy->groupCount);
    if (groupId < buddy->groupCount)
    {
        if (buddy->blockListArray[groupId] != NULL)
        {
            ret = buddy->blockListArray[groupId];
            osRemoveFromList(&buddy->blockListArray[groupId], (OsListNode *)ret);
            setBlockGroup(buddy, ret, (unsigned char)groupId + 1);
        }
        else
        {
            groupId++;
            ret = allocPages(buddy, groupId);
            osAssert(ret != NULL);
            if (ret != NULL)
            {
                ret = splitBlock(buddy, ret, groupId);
            }
        }
    }
    return ret;
}

void *osBuddyAllocPages(OsBuddy *buddy, size_t n)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    void *ret = NULL;
    osAssert(n > 0 && n <= buddy->freePageNum);
    if (n > 0 && n <= buddy->freePageNum)
    {
        size_t groupId = 0;
        size_t pageNumPerBlock = 1;
        for (; n > pageNumPerBlock; pageNumPerBlock <<= 1)
        {
            groupId++;
        }
        ret = allocPages(buddy, groupId);
        osAssert(ret != NULL);
        if (ret != NULL)
        {
            buddy->freePageNum -= pageNumPerBlock;
        }
    }
    return ret;
}

static void freePages(OsBuddy *buddy, void *pages, size_t groupId);
static size_t mergeBlock(OsBuddy *buddy, void *pages, size_t groupId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    unsigned char ret = 0;
    if (groupId < buddy->groupCount - 1)
    {
        unsigned char *blockA;
        unsigned char *blockB;
        unsigned char *buddyBlock;
        size_t blockSize = OS_BUDDY_PAGE_SIZE * (1 << groupId);
        if (0 == ((char *)pages - (char *)buddy->startAddress) % (blockSize << 1))
        {
            blockA = (unsigned char *)pages;
            blockB = blockA + blockSize;
            buddyBlock = blockB;
            if (blockB + blockSize >= (unsigned char *)buddy->startAddress + buddy->totalPageNum * OS_BUDDY_PAGE_SIZE)
            {
                return ret;
            }
        }
        else
        {
            blockB = (unsigned char *)pages;
            blockA = blockB - blockSize;
            buddyBlock = blockA;
        }
        unsigned char blockGroup = getBlockGroup(buddy, buddyBlock);
        ret = ((unsigned char)groupId + 1) * -1;
        if (blockGroup == ret)
        {
            osRemoveFromList(&buddy->blockListArray[groupId], (OsListNode *)buddyBlock);
            setBlockGroup(buddy, blockA, groupId + 2);
            setBlockGroup(buddy, blockB, 0);
            freePages(buddy, blockA, groupId + 1);
            ret = 1;
        }
    }
    return ret;
}

static void freePages(OsBuddy *buddy, void *pages, size_t groupId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    size_t value = mergeBlock(buddy, pages, groupId);
    if (value > 127 || 0 == value)
    {
        setBlockGroup(buddy, pages, (unsigned char)value);
        osInsertToFront(&buddy->blockListArray[groupId], (OsListNode *)pages);
    }
}

int osBuddyFreePages(OsBuddy *buddy, void *pages)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    osAssert(pages >= buddy->startAddress);
    if (pages >= buddy->startAddress)
    {
        osAssert(0 == ((char *)pages - (char *)buddy->startAddress) % OS_BUDDY_PAGE_SIZE);
        if (0 == ((char *)pages - (char *)buddy->startAddress) % OS_BUDDY_PAGE_SIZE)
        {
            //osAssert(((unsigned int *)pages - (unsigned int *)buddy->startAddress) / OS_BUDDY_PAGE_SIZE < buddy->totalPageNum);
            if (((char *)pages - (char *)buddy->startAddress) / (size_t)OS_BUDDY_PAGE_SIZE < buddy->totalPageNum)
            {
                size_t blockGroup = getBlockGroup(buddy, pages);
                osAssert(blockGroup > 0 && blockGroup <= buddy->groupCount);
                if (blockGroup > 0 && blockGroup <= buddy->groupCount)
                {
                    freePages(buddy, pages, blockGroup - 1);
                    buddy->freePageNum += 1 << (blockGroup - 1);
                    ret = 0;
                }
            }
        }
    }
    return ret;
}

size_t osBuddyTotalPageNum(OsBuddy *buddy) 
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return buddy->totalPageNum;
}

size_t osBuddyFreePageNum(OsBuddy *buddy)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return buddy->freePageNum;
}