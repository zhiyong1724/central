#include "ostidmanager.h"
#include "osbitmapindex.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_TID_MANAGER_LOG 0
#if ENABLE_TID_MANAGER_LOG
#define tidManagerLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define tidManagerLog(format, ...) (void)0
#endif
int osTidManagerInit(OsTidManager *tidManager)
{
	tidManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	tidManager->tidTable = (os_byte_t *)osMalloc(1);
	osAssert(tidManager->tidTable != NULL);
	if (tidManager->tidTable != NULL)
	{
		*tidManager->tidTable = 0;
		tidManager->tableLevel = 1;
		tidManager->tableSize = 1;
		tidManager->maxTidCount = 8;
		return 0;
	}
	return -1;
}

void osTidManagerUninit(OsTidManager *tidManager)
{
	tidManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (tidManager->tidTable != NULL)
	{
		osFree(tidManager->tidTable);
		tidManager->tidTable = NULL;
	}
}

static os_byte_t setTable(os_byte_t *table, os_size_t tid, os_size_t va, os_size_t offset, os_size_t level)
{
	tidManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t index = tid >> 3 * level;
	os_byte_t bitOffset = (tid >> 3 * (level - 1)) % 8;
	os_byte_t mark = 0x80;
	mark >>= bitOffset;
	level--;
	if (0 == level)
	{
		if (va > 0)
		{
			table[offset + index] |= mark;
		}
		else
		{
			mark = ~mark;
			table[offset + index] &= mark;
		}
	}
	else
	{
		if (setTable(table, tid, va, (offset << 3) + 1, level) < 0xff)
		{
			mark = ~mark;
			table[offset + index] &= mark;
			
		}
		else
		{
			table[offset + index] |= mark;
		}
	}
	return table[offset + index];
}

static int expandTable(OsTidManager *tidManager)
{
	tidManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t newSize = (tidManager->tableSize << 3) + 1;
	os_byte_t *newTable = (os_byte_t *)osMalloc(newSize);
	osAssert(newTable != NULL);
	if (newTable != NULL)
	{
		osMemSet(newTable, 0, newSize);
		for (os_size_t i = 0; i < tidManager->maxTidCount; i++)
		{
			setTable(newTable, i, 1, 0, tidManager->tableLevel + 1);
		}
		osFree(tidManager->tidTable);
		tidManager->tidTable = newTable;
		tidManager->tableSize = newSize;
		tidManager->tableLevel++;
		tidManager->maxTidCount <<= 3;
		return 0;
	}
	
	return -1;
}

static os_size_t lookupTable(os_byte_t *table, os_size_t tid, os_size_t offset, os_size_t level)
{
	tidManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t va = gBitmapIndex[table[offset + tid]];
	if (va < 8)
	{
		level--;
		tid = (tid << 3) + va;
		if (0 == level)
		{
			return tid;
		}
		else
		{
			return lookupTable(table, tid, (offset << 3) + 1, level);
		}
	}
	return (os_size_t)-1;
}

os_size_t osTidAlloc(OsTidManager *tidManager)
{
	tidManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t ret = lookupTable(tidManager->tidTable, 0, 0, tidManager->tableLevel);
	if ((os_size_t)-1 == ret)
	{
		ret = expandTable(tidManager);
		osAssert(ret != (os_size_t)-1);
		if (ret != (os_size_t)-1)
		{
			ret = lookupTable(tidManager->tidTable, 0, 0, tidManager->tableLevel);
		}
	}
	osAssert(ret != (os_size_t)-1);
	if (ret != (os_size_t)-1)
	{
		setTable(tidManager->tidTable, ret, 1, 0, tidManager->tableLevel);
	}
	return ret;
}

int osTidFree(OsTidManager *tidManager, os_size_t tid)
{
	tidManagerLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	osAssert(tid < tidManager->maxTidCount);
	if (tid < tidManager->maxTidCount)
	{
		setTable(tidManager->tidTable, tid, 0, 0, tidManager->tableLevel);
		return 0;
	}
	return -1;
}

