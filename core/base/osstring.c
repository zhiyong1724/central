#include "osstring.h"
#define ENABLE_STRING_LOG 0
#if ENABLE_STRING_LOG
#define stringLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define stringLog(format, ...) (void)0
#endif
void *osMemSet(void *s, os_byte_t ch, os_size_t n)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t divisor = n / sizeof(int);
	{
		int *ps = (int *)s;
		int va = 0;
		for (os_size_t i = 0; i < sizeof(int); i++)
		{
			va |= (int)ch << 8 * i;
		}
		for (os_size_t i = 0; i < divisor; i++)
		{
			ps[i] = va;
		}
	}
	
	{
		os_byte_t *ps = (os_byte_t *)s;
		for (os_size_t i = divisor * sizeof(int); i < n; i++)
		{
			ps[i] = ch;
		}
	}
	return s;
}

void *osMemCpy(void *dest, const void *src, os_size_t n)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t divisor = n / sizeof(int);
	{
		int *pdest = (int *)dest;
		int *psrc = (int *)src;
		for (os_size_t i = 0; i < divisor; i++)
		{
			pdest[i] = psrc[i];
		}
	}

	{
		os_byte_t *pdest = (os_byte_t *)dest;
		os_byte_t *psrc = (os_byte_t *)src;
		for (os_size_t i = divisor * sizeof(int); i < n; i++)
		{
			pdest[i] = psrc[i];
		}
	}
	return dest;
}

void *osStrCpy(char *dest, const char *src, os_size_t n)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t i;
	for (i = 0; src[i] != '\0' && i < n - 1; i++)
	{
		dest[i] = src[i];
	}
	dest[i] = '\0';
	return dest;
}

os_size_t osStrLen(const char *str)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (str != NULL)
	{
		os_size_t i = 0;
		for (; str[i] != '\0'; i++)
		{
		}
		return i;
	}
	return 0;
}

int osStrCmp(const char *str1, const char *str2)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t i;
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

char *osStrCat(char *dest, const char *src, os_size_t n)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t i;
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
