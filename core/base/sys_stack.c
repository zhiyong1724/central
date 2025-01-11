#include "sys_stack.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "sys_error.h"
#define SYS_STACK_MAX_SIZE (int)(((unsigned int)~0) >> 1)
int sys_stack_init(sys_stack_t *obj, int unit_size)
{
    sys_trace();
	obj->unit_size = unit_size;
	obj->size = 0;
	obj->max_size = 8;
	obj->buff = (unsigned char *)sys_malloc(obj->max_size * obj->unit_size);
	if (NULL == obj->buff)
    {
        sys_error("Out of memory.");
        return SYS_ERROR_NOMEM;
    }
	return 0;
}

void sys_stack_free(sys_stack_t *obj)
{
    sys_trace();
	if (obj->buff != NULL)
	{
		obj->unit_size = 0;
		obj->size = 0;
		obj->max_size = 0;
		sys_free(obj->buff);
		obj->buff = NULL;
	}
}

int sys_stack_size(sys_stack_t *obj)
{
    sys_trace();
	return obj->size;
}

int sys_stack_max_size(sys_stack_t *obj)
{
    sys_trace();
	return obj->max_size;
}

int sys_stack_unit_size(sys_stack_t *obj)
{
    sys_trace();
	return obj->unit_size;
}

int sys_stack_empty(sys_stack_t *obj)
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

int sys_stack_resize(sys_stack_t *obj, int size)
{
    sys_trace();
	if (size > obj->max_size)
	{
		unsigned char *new_buff = (unsigned char *)sys_malloc(size * obj->unit_size);
		if (NULL == new_buff)
		{
			sys_error("Out of memory.");
			return SYS_ERROR_NOMEM;
		}
		sys_memcpy(new_buff, obj->buff, obj->unit_size * obj->size);
		obj->max_size = size;
		sys_free(obj->buff);
		obj->buff = new_buff;
	}
	return obj->max_size;
}

int sys_stack_push(sys_stack_t *obj, void *data)
{
    sys_trace();
	if (obj->buff != NULL && obj->size < SYS_STACK_MAX_SIZE)
	{
		if (obj->size >= obj->max_size)
		{
			int ret = sys_stack_resize(obj, obj->max_size * 2);
			if (ret < 0)
			{
				return ret;
			}
		}
		sys_memcpy(&obj->buff[obj->size * obj->unit_size], data, obj->unit_size);
		obj->size++;
	}
	return obj->size;
}

void *sys_stack_top(sys_stack_t *obj)
{
    sys_trace();
    void *ret = NULL;
    if (obj->buff != NULL && obj->size > 0)
    {
        ret = &obj->buff[(obj->size -1) * obj->unit_size];
    }
    return ret;
}

int sys_stack_pop(sys_stack_t *obj)
{
    sys_trace();
    if (obj->size > 0)
    {
        obj->size--;
        return 0;
    }
    return -1;
}