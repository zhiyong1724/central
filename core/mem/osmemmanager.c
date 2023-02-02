#include "osmemmanager.h"
#include "osstring.h"
#define ENABLE_MEM_MANAGER_LOG 0
#if ENABLE_MEM_MANAGER_LOG
#define memManagerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define memManagerLog(format, ...) (void)0
#endif
typedef struct OsMemBlockHeader
{
	void *header;
	os_size_t size;
} OsMemBlockHeader;

typedef struct OsMemBlock
{
	OsTreeNode node;
	OsMemBlockHeader header;
} OsMemBlock;

typedef struct OsPageHeader
{
	os_size_t totalMem;
	os_size_t usedMem;
} OsPageHeader;

os_size_t osMemManagerInit(OsMemManager *memManager, void *startAddress, os_size_t size)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	memManager->totalMem = osBuddyInit(&memManager->pageFactory, startAddress, size) * OS_BUDDY_PAGE_SIZE;
	memManager->freeMem = memManager->totalMem;
	memManager->root = NULL;
	return memManager->freeMem;
}

static OsMemBlock *findSuitableBlock(OsMemManager *memManager, os_size_t size)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsMemBlock *ret = NULL;
	OsTreeNode *nextNode = memManager->root;
	if (nextNode != NULL)
	{
		for (; nextNode != &gLeafNode;)
		{
			OsMemBlock *memBlock = (OsMemBlock *)nextNode;
			if (size > memBlock->header.size)
			{
				nextNode = nextNode->rightTree;
			}
			else if (size < memBlock->header.size)
			{
				ret = memBlock;
				nextNode = nextNode->leftTree;
			}
			else
			{
				ret = memBlock;
				break;
			}
		}
	}
	return ret;
}

static os_size_t sizeAlign(os_size_t size)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t offset = size % sizeof(long long);
	if (offset > 0)
	{
		size += sizeof(long long) - offset;
	}
	return size;
}

static int onCompare(void *key1, void *key2, void *arg)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsMemBlock *memBlock1 = (OsMemBlock *)key1;
	OsMemBlock *memBlock2 = (OsMemBlock *)key2;
	if (memBlock1->header.size < memBlock2->header.size)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

static void *blockToMem(OsMemBlock *memBlock)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsMemBlockHeader *header = (OsMemBlockHeader *)memBlock;            //这里是树节点区域，所以不会被覆盖
	header->header = memBlock->header.header;
	header->size = memBlock->header.size | 0x01;
	return header + 1;
}

static void *splitBlock(OsMemManager *memManager, OsMemBlock *memBlock, os_size_t size)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_byte_t *blockB = (os_byte_t *)memBlock;
	blockB += size;
	OsMemBlock *newBlock = (OsMemBlock *)blockB;
	newBlock->header.header = memBlock->header.header;
	newBlock->header.size = memBlock->header.size - size;
	osInsertNode(&memManager->root, &newBlock->node, onCompare, NULL);
	memBlock->header.size = size;
	return memBlock;
}

void *osMemManagerAlloc(OsMemManager *memManager, os_size_t size)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	void *ret = NULL;
	int newSize = sizeAlign(size);
	newSize += sizeof(OsMemBlockHeader);
	if (newSize < sizeof(OsMemBlock))
	{
		newSize = sizeof(OsMemBlock);
	}
	osAssert(newSize > size && newSize <= memManager->freeMem);
	if (newSize > size && newSize <= memManager->freeMem)
	{
		OsMemBlock *memBlock = findSuitableBlock(memManager, newSize);
		if (memBlock != NULL)
		{
			osDeleteNode(&memManager->root, &memBlock->node);
		}
		else
		{
			os_size_t pageCount = 1;
			for (os_size_t i = 0; i < memManager->pageFactory.groupCount; i++)
			{
				if (newSize + sizeof(OsPageHeader) <= pageCount * OS_BUDDY_PAGE_SIZE)
				{
					break;
				}
				pageCount <<= 1;
			}
			os_byte_t *buddyPage = (os_byte_t *)osBuddyAllocPages(&memManager->pageFactory, pageCount);
			if (buddyPage != NULL)
			{
				OsPageHeader *pageHeader = (OsPageHeader *)buddyPage;
				pageHeader->usedMem = 0;
				buddyPage += sizeof(OsPageHeader);
				memManager->freeMem -= sizeof(OsPageHeader);
				memBlock = (OsMemBlock *)buddyPage;
				memBlock->header.header = pageHeader;
				memBlock->header.size = pageCount * OS_BUDDY_PAGE_SIZE - sizeof(OsPageHeader);
				pageHeader->totalMem = memBlock->header.size;
			}
		}
		if (memBlock != NULL)
		{
			if (memBlock->header.size >= newSize + sizeof(OsMemBlock))
			{
				memManager->freeMem -= newSize;
				OsPageHeader *pageHeader = (OsPageHeader *)memBlock->header.header;
				pageHeader->usedMem += newSize;
				ret = blockToMem((OsMemBlock *)splitBlock(memManager, memBlock, newSize));
			}
			else
			{
				memManager->freeMem -= memBlock->header.size;
				OsPageHeader *pageHeader = (OsPageHeader *)memBlock->header.header;
				pageHeader->usedMem += memBlock->header.size;
				ret = blockToMem(memBlock);
			}
		}
	}
	return ret;
}

void *osMemManagerRealloc(OsMemManager *memManager, void *address, os_size_t newSize)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	void *ret = NULL;
	if (address != NULL)
	{
		if (newSize > 0)
		{
			osAssert(address >= memManager->pageFactory.startAddress);
			if (address >= memManager->pageFactory.startAddress)
			{
				osAssert(((char *)address - (char *)memManager->pageFactory.startAddress) / (os_size_t)OS_BUDDY_PAGE_SIZE < memManager->pageFactory.totalPageNum);
				if (((char *)address - (char *)memManager->pageFactory.startAddress) / (os_size_t)OS_BUDDY_PAGE_SIZE < memManager->pageFactory.totalPageNum)
				{
					OsMemBlockHeader *header = (OsMemBlockHeader *)address;
					header -= 1;
					os_size_t mark = (os_size_t)sizeof(long long) - 1;
					osAssert((header->size & mark) == 0x01);
					if ((header->size & mark) == 0x01)
					{
						ret = osMemManagerAlloc(memManager, newSize);
						osAssert(ret != NULL);
						if (ret != NULL)
						{
							os_size_t size = (header->size & ~(os_size_t)0x01) - sizeof(OsMemBlockHeader);
							if (newSize < size)
							{
								size = newSize;
							}
							osMemCpy(ret, address, size);
						}
						osMemManagerFree(memManager, address);
					}
				}
			}
		}
		else
		{
			osMemManagerFree(memManager, address);
		}
	}
	else
	{
		ret = osMemManagerAlloc(memManager, newSize);
	}
	return ret;
}

static void freeAllNodes(OsMemManager *memManager, OsPageHeader *header, OsMemBlock *mask)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsMemBlock *curBlock = (OsMemBlock *)(header + 1);
	for (os_size_t i = 0; i < header->totalMem; )
	{
		if (curBlock != mask)
		{
			osAssert(curBlock->header.header == header);
			if (curBlock->header.header == header)
			{
				osDeleteNode(&memManager->root, &curBlock->node);
			}
		}
		i += curBlock->header.size;
		curBlock = (OsMemBlock *)((os_byte_t *)curBlock + curBlock->header.size);
	}
	
}

int osMemManagerFree(OsMemManager *memManager, void *address)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	int ret = -1;
	if (address != NULL)
	{
		osAssert(address >= memManager->pageFactory.startAddress);
		if (address >= memManager->pageFactory.startAddress)
		{
			osAssert(((char *)address - (char *)memManager->pageFactory.startAddress) / (os_size_t)OS_BUDDY_PAGE_SIZE < memManager->pageFactory.totalPageNum);
			if (((char *)address - (char *)memManager->pageFactory.startAddress) / (os_size_t)OS_BUDDY_PAGE_SIZE < memManager->pageFactory.totalPageNum)
			{
				OsMemBlockHeader *header = (OsMemBlockHeader *)address;
				header -= 1;
				os_size_t mark = (os_size_t)sizeof(long long) - 1;
				osAssert((header->size & mark) == 0x01);
				if ((header->size & mark) == 0x01)
				{
					header->size &= ~(os_size_t)0x01;
					ret = 0;
					OsMemBlock *memBlock = (OsMemBlock *)header;
					memBlock->header.header = header->header;
					memBlock->header.size = header->size;
					memManager->freeMem += memBlock->header.size;
					OsPageHeader *pageHeader = (OsPageHeader *)memBlock->header.header;
					pageHeader->usedMem -= memBlock->header.size;
					if (0 == pageHeader->usedMem)
					{
						freeAllNodes(memManager, pageHeader, memBlock);
						osBuddyFreePages(&memManager->pageFactory, memBlock->header.header);
						memManager->freeMem += sizeof(OsPageHeader);
					}
					else
					{
						osInsertNode(&memManager->root, &memBlock->node, onCompare, NULL);
					}
				}
			}
		}
	}
	return ret;
}

os_size_t osMemManagerAllocUsableSize(OsMemManager *memManager, const void *address)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	int ret = 0;
	if (address != NULL)
	{
		osAssert(address >= memManager->pageFactory.startAddress);
		if (address >= memManager->pageFactory.startAddress)
		{
			osAssert(((char *)address - (char *)memManager->pageFactory.startAddress) / (os_size_t)OS_BUDDY_PAGE_SIZE < memManager->pageFactory.totalPageNum);
			if (((char *)address - (char *)memManager->pageFactory.startAddress) / (os_size_t)OS_BUDDY_PAGE_SIZE < memManager->pageFactory.totalPageNum)
			{
				OsMemBlockHeader *header = (OsMemBlockHeader *)address;
				header -= 1;
				os_size_t mark = (os_size_t)sizeof(long long) - 1;
				osAssert((header->size & mark) == 0x01);
				if ((header->size & mark) == 0x01)
				{
					ret = (header->size & ~(os_size_t)0x01) - sizeof(OsMemBlockHeader);
				}
			}
		}
	}
	return ret;
}

void *osMemManagerAllocPages(OsMemManager *memManager, os_size_t n)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return osBuddyAllocPages(&memManager->pageFactory, n);
}

int osMemManagerFreePages(OsMemManager *memManager, void *pages)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return osBuddyFreePages(&memManager->pageFactory, pages);
}

os_size_t osMemManagerTotalMem(OsMemManager *memManager)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return memManager->totalMem;
}

os_size_t osMemManagerFreeMem(OsMemManager *memManager)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return memManager->freeMem;
}

os_size_t osMemManagerTotalPage(OsMemManager *memManager)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return memManager->pageFactory.totalPageNum;
}

os_size_t osMemManagerFreePage(OsMemManager *memManager)
{
	memManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return memManager->pageFactory.freePageNum;
}