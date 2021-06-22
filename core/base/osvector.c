#include "osvector.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_VECTOR_LOG 0
#if ENABLE_VECTOR_LOG
#define vectorLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define vectorLog(format, ...) (void)0
#endif
#define OS_VECTOR_MAX_SIZE ((os_size_t)-1)
int osVectorInit(OsVector *obj, os_size_t unitSize)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	obj->unitSize = unitSize;
	obj->size = 0;
	obj->maxSize = 8;
	obj->buff = (os_byte_t *)osMalloc(obj->maxSize * obj->unitSize);
	if (obj->buff != NULL)
	{
		return 0;
	}
	return -1;
}

void osVectorFree(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->buff != NULL)
	{
		osFree(obj->buff);
	}
}

os_size_t osVectorSize(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return obj->size;
}

os_size_t osVectorMaxSize(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return obj->maxSize;
}

int osVectorEmpty(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->size > 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

os_size_t osVectorResize(OsVector *obj, os_size_t size)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (size > obj->maxSize)
	{
		os_byte_t *newBuff = (os_byte_t *)osMalloc(size * obj->unitSize);
		if (newBuff != NULL)
		{
			osMemCpy(newBuff, obj->buff, obj->unitSize * obj->size);
			obj->maxSize = size;
			osFree(obj->buff);
			obj->buff = newBuff;
		}
	}
	return obj->maxSize;
}

os_size_t osVectorPushBack(OsVector *obj, void *data)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return osVectorInsert(obj, data, obj->size);
}

os_size_t osVectorPushFront(OsVector *obj, void *data)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return osVectorInsert(obj, data, 0);
}

os_size_t osVectorInsert(OsVector *obj, void *data, os_size_t n)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	osAssert(obj->size < OS_VECTOR_MAX_SIZE);
	if (obj->size < OS_VECTOR_MAX_SIZE)
	{
		if (obj->size >= obj->maxSize)
		{
			osVectorResize(obj, obj->maxSize * 2);
		}
		for (os_size_t i = obj->size; i > n; i--)
		{
			osMemCpy(&obj->buff[i * obj->unitSize], &obj->buff[(i - 1) * obj->unitSize], obj->unitSize);
		}
		osMemCpy(&obj->buff[n * obj->unitSize], data, obj->unitSize);
		obj->size++;
	}
	return obj->size;
}

void *osVectorBack(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return osVectorAt(obj, obj->size - 1);
}

void *osVectorFront(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return osVectorAt(obj, 0);
}

void *osVectorAt(OsVector *obj, os_size_t n)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	osAssert(n < obj->size);
	if (n < obj->size)
	{
		return &obj->buff[n * obj->unitSize];
	}
	return NULL;
}

int osVectorErase(OsVector *obj, os_size_t n)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	osAssert(n < obj->size);
	if (n < obj->size)
	{
		for (os_size_t i = n + 1; i < obj->size; i++)
		{
			osMemCpy(&obj->buff[(i - 1) * obj->unitSize], &obj->buff[i * obj->unitSize], obj->unitSize);
		}
		obj->size--;
		return 0;
	}
	return -1;
}

void osVectorClear(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	obj->size = 0;
}

int osVectorPopBack(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return osVectorErase(obj, obj->size - 1);
}

int osVectorPopFront(OsVector *obj)
{
	vectorLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return osVectorErase(obj, 0);
}