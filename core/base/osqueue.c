#include "osqueue.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_QUEUE_LOG 0
#if ENABLE_QUEUE_LOG
#define queueLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define queueLog(format, ...) (void)0
#endif
typedef struct Queue
{
	OsListNode node;
	unsigned char *buff;
    size_t begin;
    size_t end;
	size_t unitSize;
	size_t size;
	size_t maxSize;
} Queue;

static int queueInit(Queue *obj, size_t unitSize, size_t maxSize)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	obj->unitSize = unitSize;
    obj->maxSize = maxSize;
	obj->size = 0;
	obj->begin = 0;
    obj->end = 0;
	obj->buff = (unsigned char *)osMalloc(obj->maxSize * obj->unitSize);
	if (obj->buff != NULL)
	{
		return 0;
	}
	return -1;
}

static void queueFree(Queue *obj)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->buff != NULL)
	{
        obj->unitSize = 0;
        obj->maxSize = 0;
        obj->size = 0;
        obj->begin = 0;
        obj->end = 0;
        osFree(obj->buff);
		obj->buff = NULL;
	}
}

static int queueSize(Queue *obj)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return obj->size;
}

static void *queueBack(Queue *obj)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->buff != NULL && obj->size > 0)
	{
        return &obj->buff[obj->end - obj->unitSize];
	}
    return NULL;
}

static void *queueFront(Queue *obj)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->buff != NULL && obj->size > 0)
	{
        return &obj->buff[obj->begin];
	}
    return NULL;
}

static int queuePush(Queue *obj, void *data)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->buff != NULL && obj->size < obj->maxSize && obj->end < obj->maxSize * obj->unitSize)
	{
        osMemCpy(&obj->buff[obj->end], data, obj->unitSize);
        obj->size++;
        obj->end += obj->unitSize;
        return 0;
	}
    return -1;
}

static int queuePop(Queue *obj)
{
    queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->buff != NULL && obj->size > 0)
	{
        obj->begin += obj->unitSize;
        obj->size--;
        return 0;
	}
    return -1;
}

int osQueueInit(OsQueue *obj, size_t unitSize)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	obj->queue = NULL;
	obj->unitSize = unitSize;
	obj->size = 0;
	return 0;
}

void osQueueFree(OsQueue *obj)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	osQueueClear(obj);
	obj->unitSize = 0;
}

size_t osQueueUnitSize(OsQueue *obj)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return obj->unitSize;
}

size_t osQueueSize(OsQueue *obj)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	return obj->size;
}

int osQueueEmpty(OsQueue *obj)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->size > 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static int addQueue(OsQueue *obj)
{
	int ret = -1;
	Queue *queue = (Queue *)osMalloc(sizeof(Queue));
	if (queue != NULL)
	{
		if (queueInit(queue, obj->unitSize, 256) == 0)
		{
			osInsertToBack(&obj->queue, &queue->node);
			ret = 0;
		}
	}
	return ret;
}

size_t osQueuePush(OsQueue *obj, void *data)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (NULL == obj->queue)
	{
		addQueue(obj);
	}
	Queue *queue = (Queue *)osGetBackFromList(&obj->queue);
	if (queuePush(queue, data) == 0)
	{
		obj->size++;
		return obj->size;
	}
	else
	{
		addQueue(obj);
		Queue *queue = (Queue *)osGetBackFromList(&obj->queue);
		if (queuePush(queue, data) == 0)
		{
			obj->size++;
			return obj->size;
		}
	}
	return 0;
}

void *osQueueBack(OsQueue *obj)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->queue != NULL && obj->size > 0)
	{
		Queue *queue = (Queue *)osGetBackFromList(&obj->queue);
		return queueBack(queue);
	}
	return NULL;
}

void *osQueueFront(OsQueue *obj)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->queue != NULL && obj->size > 0)
	{
		Queue *queue = (Queue *)obj->queue;
		return queueFront(queue);
	}
	return NULL;
}

void osQueueClear(OsQueue *obj)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	while (obj->queue != NULL)
	{
		Queue *queue = (Queue *)obj->queue;
		osRemoveFromList(&obj->queue, &queue->node);
		queueFree(queue);
		osFree(queue);
	}
	obj->size = 0;
}

int osQueuePop(OsQueue *obj)
{
	queueLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (obj->queue != NULL && obj->size > 0)
	{
		Queue *queue = (Queue *)obj->queue;
		if (queuePop(queue) == 0)
		{
			obj->size--;
			if (queueSize(queue) == 0)
			{
				osRemoveFromList(&obj->queue, &queue->node);
				queueFree(queue);
				osFree(queue);
			}
			return 0;
		}
	}
	return -1;
}