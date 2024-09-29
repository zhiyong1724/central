#include "osstack.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_STACK_LOG 0
#if ENABLE_STACK_LOG
#define stackLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define stackLog(format, ...) (void)0
#endif
#define OS_STACK_MAX_SIZE ((size_t)-1)
int osStackInit(OsStack *obj, size_t unitSize)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	obj->unitSize = unitSize;
	obj->size = 0;
	obj->maxSize = 8;
	obj->buff = (unsigned char *)osMalloc(obj->maxSize * obj->unitSize);
	if (obj->buff != NULL)
	{
		return 0;
	}
	return -1;
}

void osStackFree(OsStack *obj)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->buff != NULL)
	{
		obj->unitSize = 0;
		obj->size = 0;
		obj->maxSize = 0;
		osFree(obj->buff);
		obj->buff = NULL;
	}
}

size_t osStackSize(OsStack *obj)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return obj->size;
}

size_t osStackMaxSize(OsStack *obj)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return obj->maxSize;
}

size_t osStackUnitSize(OsStack *obj)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return obj->unitSize;
}

int osStackEmpty(OsStack *obj)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->size > 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

size_t osStackResize(OsStack *obj, size_t size)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (size > obj->maxSize)
	{
		unsigned char *newBuff = (unsigned char *)osMalloc(size * obj->unitSize);
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

size_t osStackPush(OsStack *obj, void *data)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->buff != NULL && obj->size < OS_STACK_MAX_SIZE)
	{
		if (obj->size >= obj->maxSize)
		{
			osStackResize(obj, obj->maxSize * 2);
		}
		osMemCpy(&obj->buff[obj->size * obj->unitSize], data, obj->unitSize);
		obj->size++;
	}
	return obj->size;
}

void *osStackTop(OsStack *obj)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    void *ret = NULL;
    if (obj->buff != NULL && obj->size > 0)
    {
        ret = &obj->buff[(obj->size -1) * obj->unitSize];
    }
    return ret;
}

int osStackPop(OsStack *obj)
{
    stackLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (obj->size > 0)
    {
        obj->size--;
        return 0;
    }
    return -1;
}