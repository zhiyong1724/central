#include "osstring.h"
#define ENABLE_STRING_LOG 0
#if ENABLE_STRING_LOG
#define stringLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define stringLog(format, ...) (void)0
#endif
void *osMemSet(void *s, unsigned char ch, size_t n)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (n < sizeof(long))
	{
		unsigned char *ps = (unsigned char *)s;
		for (size_t i = 0; i < n; i++)
		{
			ps[i] = ch;
		}
	}
	else
	{
		size_t align = (size_t)s % sizeof(long) > 0 ? sizeof(long) - (size_t)s % sizeof(long) : 0;
		{
			unsigned char *ps = (unsigned char *)s;
			for (size_t i = 0; i < align; i++)
			{
				ps[i] = ch;
			}
		}

		size_t divisor = (n - align) / sizeof(long);
		{
			long *ps = (long *)((unsigned char *)s + align);
			long va = 0;
			for (size_t i = 0; i < sizeof(long); i++)
			{
				va |= (long)ch << 8 * i;
			}
			for (size_t i = 0; i < divisor; i++)
			{
				ps[i] = va;
			}
		}

		{
			unsigned char *ps = (unsigned char *)s;
			for (size_t i = align + divisor * sizeof(long); i < n; i++)
			{
				ps[i] = ch;
			}
		}
	}
	return s;
}

void *osMemCpy(void *dest, const void *src, size_t n)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	size_t destAlign = (size_t)dest % sizeof(long) > 0 ? sizeof(long) - (size_t)dest % sizeof(long) : 0;
	size_t srcAlign = (size_t)src % sizeof(long) > 0 ? sizeof(long) - (size_t)src % sizeof(long) : 0;
	if (n < sizeof(long) || destAlign != srcAlign)
	{
		unsigned char *pDest = (unsigned char *)dest;
		unsigned char *pSrc = (unsigned char *)src;
		for (size_t i = 0; i < n; i++)
		{
			pDest[i] = pSrc[i];
		}
	}
	else
	{
		{
			unsigned char *pDest = (unsigned char *)dest;
			unsigned char *pSrc = (unsigned char *)src;
			for (size_t i = 0; i < destAlign; i++)
			{
				pDest[i] = pSrc[i];
			}
		}

		size_t divisor = (n - destAlign) / sizeof(long);
		{
			long *pDest = (long *)((unsigned char *)dest + destAlign);
			long *pSrc = (long *)((unsigned char *)src + destAlign);
			for (size_t i = 0; i < divisor; i++)
			{
				pDest[i] = pSrc[i];
			}
		}

		{
			unsigned char *pDest = (unsigned char *)dest;
			unsigned char *pSrc = (unsigned char *)src;
			for (size_t i = destAlign + divisor * sizeof(long); i < n; i++)
			{
				pDest[i] = pSrc[i];
			}
		}
	}
	return dest;
}

void *osStrCpy(char *dest, const char *src, size_t n)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	size_t i;
	for (i = 0; src[i] != '\0' && i < n - 1; i++)
	{
		dest[i] = src[i];
	}
	dest[i] = '\0';
	return dest;
}

size_t osStrLen(const char *str)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (str != NULL)
	{
		size_t i = 0;
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
	size_t i;
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

char *osStrCat(char *dest, const char *src, size_t n)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	size_t i;
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

char *osStrStr(const char *str, const char *pattern)
{
	stringLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	const char *ret = NULL;
	for (size_t i = 0;; i++)
	{
		size_t j = 0;
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