#include "sys_vector.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "sys_error.h"
#define SYS_VECTOR_MAX_SIZE (int)(((unsigned int)~0) >> 1)
int sys_vector_init(sys_vector_t *obj, int unit_size)
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

void sys_vector_free(sys_vector_t *obj)
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

int sys_vector_size(sys_vector_t *obj)
{
	sys_trace();
	return obj->size;
}

int sys_vector_max_size(sys_vector_t *obj)
{
	sys_trace();
	return obj->max_size;
}

int sys_vector_unit_size(sys_vector_t *obj)
{
	sys_trace();
	return obj->unit_size;
}

int sys_vector_empty(sys_vector_t *obj)
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

int sys_vector_resize(sys_vector_t *obj, int size)
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

int sys_vector_push_back(sys_vector_t *obj, void *data)
{
	sys_trace();
	return sys_vector_insert(obj, data, obj->size);
}

int sys_vector_push_front(sys_vector_t *obj, void *data)
{
	sys_trace();
	return sys_vector_insert(obj, data, 0);
}

int sys_vector_insert(sys_vector_t *obj, void *data, int n)
{
	sys_trace();
	if (obj->buff != NULL && obj->size < SYS_VECTOR_MAX_SIZE)
	{
		if (obj->size >= obj->max_size)
		{
			int ret = sys_vector_resize(obj, obj->max_size * 2);
			if (ret < 0)
			{
				return ret;
			}
		}
		for (int i = obj->size; i > n; i--)
		{
			sys_memcpy(&obj->buff[i * obj->unit_size], &obj->buff[(i - 1) * obj->unit_size], obj->unit_size);
		}
		sys_memcpy(&obj->buff[n * obj->unit_size], data, obj->unit_size);
		obj->size++;
	}
	return obj->size;
}

void *sys_vector_back(sys_vector_t *obj)
{
	sys_trace();
	return sys_vector_at(obj, obj->size - 1);
}

void *sys_vector_front(sys_vector_t *obj)
{
	sys_trace();
	return sys_vector_at(obj, 0);
}

void *sys_vector_at(sys_vector_t *obj, int n)
{
	sys_trace();
	if (obj->buff != NULL && n < obj->size)
	{
		return &obj->buff[n * obj->unit_size];
	}
	return NULL;
}

int sys_vector_erase(sys_vector_t *obj, int n)
{
	sys_trace();
	if (obj->buff != NULL && n < obj->size)
	{
		for (int i = n + 1; i < obj->size; i++)
		{
			sys_memcpy(&obj->buff[(i - 1) * obj->unit_size], &obj->buff[i * obj->unit_size], obj->unit_size);
		}
		obj->size--;
		return 0;
	}
	return -1;
}

void sys_vector_clear(sys_vector_t *obj)
{
	sys_trace();
	obj->size = 0;
}

int sys_vector_pop_back(sys_vector_t *obj)
{
	sys_trace();
	return sys_vector_erase(obj, obj->size - 1);
}

int sys_vector_pop_front(sys_vector_t *obj)
{
	sys_trace();
	return sys_vector_erase(obj, 0);
}