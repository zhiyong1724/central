#include "osbuddy.h"
#include "osstring.h"
#define ENABLE_BUDDY_LOG 0
#if ENABLE_BUDDY_LOG
#define buddyLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define buddyLog(format, ...) (void)0
#endif

static void *addressAlign(void *address, os_size_t size)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_byte_t *startAddress = (os_byte_t *)address;
    os_size_t offset = (os_size_t)startAddress % size;
    if (offset > 0)
    {
        startAddress += size - offset;
    }
    return startAddress;
}

static os_size_t calculateGroupCount(os_size_t pageNum)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t i = 1;
    os_size_t compareValue = 2;
    for (; compareValue <= pageNum && i < 128; compareValue <<= 1)
    {
        i++;
    }
    return i;
}

static os_byte_t *fillBlockArray(OsBuddy *buddy, os_byte_t *address, os_size_t blockArrayId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    buddy->blockListArray[blockArrayId] = NULL;
    os_size_t pageCount = 1 << blockArrayId;
    for (; buddy->totalPageNum - buddy->freePageNum >= pageCount; buddy->freePageNum += pageCount)
    {
        osInsertToFront(&buddy->blockListArray[blockArrayId], (OsListNode *)address);
        address += pageCount * BUDDY_PAGE_SIZE;
    }
    return address;
}

os_size_t osBuddyInit(OsBuddy *buddy, void *startAddress, os_size_t size)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    buddy->freePageNum = 0;
    buddy->totalPageNum = size / BUDDY_PAGE_SIZE;
    osAssert(buddy->totalPageNum >= 2);
    if (buddy->totalPageNum >= 2)
    {
        buddy->blockGroup = (os_byte_t *)startAddress;
        osMemSet(buddy->blockGroup, 0, buddy->totalPageNum);
        size -= buddy->totalPageNum;

        os_byte_t *blockGroupEnd = buddy->blockGroup + buddy->totalPageNum;
        os_byte_t *blockListArrayStart = (os_byte_t *)addressAlign(blockGroupEnd, sizeof(void *));
        buddy->blockListArray = (OsListNode **)blockListArrayStart;
        os_size_t offset = blockListArrayStart - blockGroupEnd;
        size -= offset;

        buddy->groupCount = calculateGroupCount(buddy->totalPageNum);
        offset = buddy->groupCount * sizeof(void *);
        buddy->startAddress = blockListArrayStart + offset;
        size -= offset;

        buddy->totalPageNum = size / BUDDY_PAGE_SIZE;
        osAssert(buddy->totalPageNum >= 2);
        if (buddy->totalPageNum >= 2)
        {
            buddy->groupCount = calculateGroupCount(buddy->totalPageNum);
            os_byte_t *handle = (os_byte_t *)buddy->startAddress;
            for (int i = (int)buddy->groupCount - 1; i >= 0; i--)
            {
                handle = fillBlockArray(buddy, handle, (os_size_t)i);
                if (buddy->freePageNum == buddy->totalPageNum)
                {
                    break;
                }
            }
        }
    }
    return buddy->freePageNum;
}

static void setBlockGroup(OsBuddy *buddy, void *address, os_byte_t value)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t index = (address - buddy->startAddress) / BUDDY_PAGE_SIZE;
    buddy->blockGroup[index] = value;
}

static os_byte_t getBlockGroup(OsBuddy *buddy, void *address)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t index = (address - buddy->startAddress) / BUDDY_PAGE_SIZE;
    return buddy->blockGroup[index];
}

static void *splitBlock(OsBuddy *buddy, void *address, os_size_t groupId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    setBlockGroup(buddy, address, (os_byte_t)groupId);
    os_byte_t *blockB = (os_byte_t *)address;
    blockB += (1 << (groupId - 1)) * BUDDY_PAGE_SIZE;
    setBlockGroup(buddy, blockB, (os_byte_t)groupId * -1);
    osInsertToFront(&buddy->blockListArray[groupId - 1], (OsListNode *)blockB);
    return address;
}

static void *allocPages(OsBuddy *buddy, os_size_t groupId)
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
            setBlockGroup(buddy, ret, (os_byte_t)groupId + 1);
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

void *osBuddyAllocPages(OsBuddy *buddy, os_size_t n)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    void *ret = NULL;
    osAssert(n > 0 && n <= buddy->freePageNum);
    if (n > 0 && n <= buddy->freePageNum)
    {
        os_size_t groupId = 0;
        os_size_t pageNumPerBlock = 1;
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

static void freePages(OsBuddy *buddy, void *pages, os_size_t groupId);
static os_size_t mergeBlock(OsBuddy *buddy, void *pages, os_size_t groupId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_byte_t ret = 0;
    if (groupId < buddy->groupCount - 1)
    {
        os_byte_t *blockA;
        os_byte_t *blockB;
        os_byte_t *buddyBlock;
        os_size_t blockSize = BUDDY_PAGE_SIZE * (1 << groupId);
        if (0 == (pages - buddy->startAddress) % (blockSize << 1))
        {
            blockA = (os_byte_t *)pages;
            blockB = blockA + blockSize;
            buddyBlock = blockB;
            if (blockB + blockSize >= (os_byte_t *)buddy->startAddress + buddy->totalPageNum * BUDDY_PAGE_SIZE)
            {
                return ret;
            }
        }
        else
        {
            blockB = (os_byte_t *)pages;
            blockA = blockB - blockSize;
            buddyBlock = blockA;
        }
        os_byte_t blockGroup = getBlockGroup(buddy, buddyBlock);
        ret = ((os_byte_t)groupId + 1) * -1;
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

static void freePages(OsBuddy *buddy, void *pages, os_size_t groupId)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    os_size_t value = mergeBlock(buddy, pages, groupId);
    if (value > 127 || 0 == value)
    {
        setBlockGroup(buddy, pages, (os_byte_t)value);
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
        osAssert(0 == (pages - buddy->startAddress) % BUDDY_PAGE_SIZE);
        if (0 == (pages - buddy->startAddress) % BUDDY_PAGE_SIZE)
        {
            osAssert((pages - buddy->startAddress) / BUDDY_PAGE_SIZE < buddy->totalPageNum);
            if ((pages - buddy->startAddress) / BUDDY_PAGE_SIZE < buddy->totalPageNum)
            {
                os_size_t blockGroup = getBlockGroup(buddy, pages);
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

os_size_t osBuddyTotalPageNum(OsBuddy *buddy) 
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return buddy->totalPageNum;
}

os_size_t osBuddyFreePageNum(OsBuddy *buddy)
{
    buddyLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return buddy->freePageNum;
}