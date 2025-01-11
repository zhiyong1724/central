#include "sys_string.h"
#include "sys_mem.h"
#include "sys_error.h"
int sys_string_init(sys_string_t *obj)
{
	sys_trace();
	obj->len = 0;
	obj->size = 8;
	obj->str = (char *)sys_malloc(obj->size);
	if (NULL == obj->str)
    {
        sys_error("Out of memory.");
        return SYS_ERROR_NOMEM;
    }
	return 0;
}

void sys_string_free(sys_string_t *obj)
{
	sys_trace();
	if (obj->str != NULL)
	{
		obj->len = 0;
		obj->size = 0;
		sys_free(obj->str);
		obj->str = NULL;
	}
}

static int get_suitable_size(int size)
{
	sys_trace();
	int ret = 1;
	while (ret < size)
	{
		ret <<= 1;
	}
	return ret;
}

int sys_string_copy(sys_string_t *obj, const char *s)
{
	sys_trace();
	int ret = 0;
	int len = sys_strlen(s);
	if (obj->size <= len)
	{
		int size = get_suitable_size(len);
		ret = sys_string_resize(obj, size);
		if (ret != size)
		{
			return ret;
		}
	}
	sys_memcpy(obj->str, s, len);
	obj->len = len;
	obj->str[obj->len] = '\0';
	return ret;
}

int sys_string_append(sys_string_t *obj, const char *s)
{
	sys_trace();
	int ret = 0;
	int len = sys_strlen(s);
	if (obj->size <= obj->len + len)
	{
		int size = get_suitable_size(obj->len + len);
		ret = sys_string_resize(obj, size);
		if (ret != size)
		{
			return ret;
		}
	}
	sys_memcpy(&obj->str[obj->len], s, len);
	obj->len += len;
	obj->str[obj->len] = '\0';
	return ret;
}

int sys_string_push(sys_string_t *obj, char c)
{
	sys_trace();
	int ret = 0;
	if (obj->size <= obj->len + 1)
	{
		int size = get_suitable_size(obj->len + 1);
		ret = sys_string_resize(obj, size);
		if (ret != size)
		{
			return ret;
		}
	}
	obj->str[obj->len] = c;
	obj->len++;
	obj->str[obj->len] = '\0';
	return ret;
}

int sys_string_resize(sys_string_t *obj, int size)
{
	sys_trace();
	if (size > obj->size)
	{
		char *new_str = (char *)sys_malloc(size);
		if (NULL == new_str)
		{
			sys_error("Out of memory.");
			return SYS_ERROR_NOMEM;
		}
		sys_memcpy(new_str, obj->str, obj->len);
		obj->str[obj->len] = '\0';
		obj->size = size;
		sys_free(obj->str);
		obj->str = new_str;
	}
	return obj->size;
}

void *sys_memset(void *s, unsigned char ch, int n)
{
	sys_trace();
	if (n < sizeof(long))
	{
		unsigned char *ps = (unsigned char *)s;
		for (int i = 0; i < n; i++)
		{
			ps[i] = ch;
		}
	}
	else
	{
		int align = (long)s % sizeof(long) > 0 ? sizeof(long) - (long)s % sizeof(long) : 0;
		{
			unsigned char *ps = (unsigned char *)s;
			for (int i = 0; i < align; i++)
			{
				ps[i] = ch;
			}
		}

		int divisor = (n - align) / sizeof(long);
		{
			long *ps = (long *)((unsigned char *)s + align);
			long va = 0;
			for (int i = 0; i < sizeof(long); i++)
			{
				va |= (long)ch << 8 * i;
			}
			for (int i = 0; i < divisor; i++)
			{
				ps[i] = va;
			}
		}

		{
			unsigned char *ps = (unsigned char *)s;
			for (int i = align + divisor * sizeof(long); i < n; i++)
			{
				ps[i] = ch;
			}
		}
	}
	return s;
}

void *sys_memcpy(void *dest, const void *src, int n)
{
	sys_trace();
	long dest_align = (long)dest % sizeof(long) > 0 ? sizeof(long) - (long)dest % sizeof(long) : 0;
	long src_align = (long)src % sizeof(long) > 0 ? sizeof(long) - (long)src % sizeof(long) : 0;
	if (n < sizeof(long) || dest_align != src_align)
	{
		unsigned char *pdest = (unsigned char *)dest;
		unsigned char *psrc = (unsigned char *)src;
		for (int i = 0; i < n; i++)
		{
			pdest[i] = psrc[i];
		}
	}
	else
	{
		{
			unsigned char *pdest = (unsigned char *)dest;
			unsigned char *psrc = (unsigned char *)src;
			for (int i = 0; i < dest_align; i++)
			{
				pdest[i] = psrc[i];
			}
		}

		int divisor = (n - dest_align) / sizeof(long);
		{
			long *pdest = (long *)((unsigned char *)dest + dest_align);
			long *psrc = (long *)((unsigned char *)src + dest_align);
			for (int i = 0; i < divisor; i++)
			{
				pdest[i] = psrc[i];
			}
		}

		{
			unsigned char *pdest = (unsigned char *)dest;
			unsigned char *psrc = (unsigned char *)src;
			for (int i = dest_align + divisor * sizeof(long); i < n; i++)
			{
				pdest[i] = psrc[i];
			}
		}
	}
	return dest;
}

void *sys_strcpy(char *dest, const char *src, int n)
{
	sys_trace();
	int i;
	for (i = 0; src[i] != '\0' && i < n - 1; i++)
	{
		dest[i] = src[i];
	}
	dest[i] = '\0';
	return dest;
}

int sys_strlen(const char *str)
{
	sys_trace();
	if (str != NULL)
	{
		int i = 0;
		for (; str[i] != '\0'; i++)
		{
		}
		return i;
	}
	return 0;
}

int sys_strcmp(const char *str1, const char *str2)
{
	sys_trace();
	int i;
	for (i = 0; str1[i] != '\0' && str2[i] != '\0'; i++)
	{
		if (str1[i] > str2[i])
		{
			return 1;
		}
		else if (str1[i] < str2[i])
		{
			return -1;
		}
	}
	if ('\0' == str1[i] && '\0' == str2[i])
	{
		return 0;
	}
	else if ('\0' == str1[i])
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

char *sys_strcat(char *dest, const char *src, int n)
{
	sys_trace();
	int i;
	for (i = 0; dest[i] != '\0' && i < n - 1; i++)
	{
	}
	for (; *src != '\0' && i < n - 1; i++, src++)
	{
		dest[i] = *src;
	}
	dest[i] = '\0';
	return dest;
}

char *sys_strstr(const char *str, const char *pattern)
{
	sys_trace();
	const char *ret = NULL;
	for (int i = 0;; i++)
	{
		int j = 0;
		for (; str[i + j] != '\0' && pattern[j] != '\0'; j++)
		{
			if (str[i + j] != pattern[j])
			{
				break;
			}
		}
		if ('\0' == pattern[j])
		{
			ret = &str[i];
			break;
		}
		else if ('\0' == str[i + j])
		{
			break;
		}
	}
	return (char *)ret;
}