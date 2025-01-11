#include "sys_queue.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "sys_error.h"
typedef struct queue_t
{
	sys_list_node_t node;
	unsigned char *buff;
    int begin;
    int end;
	int unit_size;
	int size;
	int max_size;
} queue_t;

static int queue_init(queue_t *obj, int unit_size, int max_size)
{
    sys_trace();
	obj->unit_size = unit_size;
    obj->max_size = max_size;
	obj->size = 0;
	obj->begin = 0;
    obj->end = 0;
	obj->buff = (unsigned char *)sys_malloc(obj->max_size * obj->unit_size);
	if (NULL == obj->buff)
    {
        sys_error("Out of memory.");
        return SYS_ERROR_NOMEM;
    }
	return 0;
}

static void queue_free(queue_t *obj)
{
    sys_trace();
	if (obj->buff != NULL)
	{
        obj->unit_size = 0;
        obj->max_size = 0;
        obj->size = 0;
        obj->begin = 0;
        obj->end = 0;
        sys_free(obj->buff);
		obj->buff = NULL;
	}
}

static int queue_size(queue_t *obj)
{
    sys_trace();
	return obj->size;
}

static void *queue_back(queue_t *obj)
{
    sys_trace();
	if (obj->buff != NULL && obj->size > 0)
	{
        return &obj->buff[obj->end - obj->unit_size];
	}
    return NULL;
}

static void *queue_front(queue_t *obj)
{
    sys_trace();
	if (obj->buff != NULL && obj->size > 0)
	{
        return &obj->buff[obj->begin];
	}
    return NULL;
}

static int queue_push(queue_t *obj, void *data)
{
    sys_trace();
	if (obj->buff != NULL && obj->size < obj->max_size && obj->end < obj->max_size * obj->unit_size)
	{
        sys_memcpy(&obj->buff[obj->end], data, obj->unit_size);
        obj->size++;
        obj->end += obj->unit_size;
        return 0;
	}
    return -1;
}

static int queue_pop(queue_t *obj)
{
    sys_trace();
	if (obj->buff != NULL && obj->size > 0)
	{
        obj->begin += obj->unit_size;
        obj->size--;
        return 0;
	}
    return -1;
}

void sys_queue_init(sys_queue_t *obj, int unit_size)
{
	sys_trace();
	obj->queue = NULL;
	obj->unit_size = unit_size;
	obj->size = 0;
}

void sys_queue_free(sys_queue_t *obj)
{
	sys_trace();
	sys_queue_clear(obj);
	obj->unit_size = 0;
}

int sys_queue_unit_size(sys_queue_t *obj)
{
	sys_trace();
	return obj->unit_size;
}

int sys_queue_size(sys_queue_t *obj)
{
	sys_trace();
	return obj->size;
}

int sys_queue_empty(sys_queue_t *obj)
{
	sys_trace();
	if (obj->size > 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static int add_queue(sys_queue_t *obj)
{
	sys_trace();
	int ret = 0;
	queue_t *queue = (queue_t *)sys_malloc(sizeof(queue_t));
	if (NULL == queue)
	{
		sys_error("Out of memory.");
		return SYS_ERROR_NOMEM;
	}
	ret = queue_init(queue, obj->unit_size, 256);
	if (ret < 0)
	{
		sys_free(queue);
		sys_error("Out of memory.");
		return SYS_ERROR_NOMEM;
	}
	sys_insert_to_back(&obj->queue, &queue->node);
	return ret;
}

int sys_queue_push(sys_queue_t *obj, void *data)
{
	sys_trace();
	int ret = 0;
	if (NULL == obj->queue)
	{
		ret = add_queue(obj);
		if (ret < 0)
		{
			return ret;
		}
	}
	queue_t *queue = (queue_t *)sys_get_back_from_list(&obj->queue);
	if (queue_push(queue, data) == 0)
	{
		obj->size++;
		return obj->size;
	}
	else
	{
		ret = add_queue(obj);
		if (ret < 0)
		{
			return ret;
		}
		queue_t *queue = (queue_t *)sys_get_back_from_list(&obj->queue);
		ret = queue_push(queue, data);
		if (ret < 0)
		{
			return ret;
		}
		obj->size++;
		return obj->size;
	}
	return ret;
}

void *sys_queue_back(sys_queue_t *obj)
{
	sys_trace();
	if (obj->queue != NULL && obj->size > 0)
	{
		queue_t *queue = (queue_t *)sys_get_back_from_list(&obj->queue);
		return queue_back(queue);
	}
	return NULL;
}

void *sys_queue_front(sys_queue_t *obj)
{
	sys_trace();
	if (obj->queue != NULL && obj->size > 0)
	{
		queue_t *queue = (queue_t *)obj->queue;
		return queue_front(queue);
	}
	return NULL;
}

void sys_queue_clear(sys_queue_t *obj)
{
	sys_trace();
	while (obj->queue != NULL)
	{
		queue_t *queue = (queue_t *)obj->queue;
		sys_remove_from_list(&obj->queue, &queue->node);
		queue_free(queue);
		sys_free(queue);
	}
	obj->size = 0;
}

int sys_queue_pop(sys_queue_t *obj)
{
	sys_trace();
	if (obj->queue != NULL && obj->size > 0)
	{
		queue_t *queue = (queue_t *)obj->queue;
		if (queue_pop(queue) == 0)
		{
			obj->size--;
			if (queue_size(queue) == 0)
			{
				sys_remove_from_list(&obj->queue, &queue->node);
				queue_free(queue);
				sys_free(queue);
			}
			return 0;
		}
	}
	return -1;
}