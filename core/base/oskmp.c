#include "oskmp.h"
#include "osstring.h"
#include "osmem.h"
#define MAX_PATTERN_SIZE 256
static void getNext(int *next, const char *pattern, int len)
{
    next[0] = 0;
    int k = 0;
    for (int i = 1; i < len; i++)
    {
        while (k > 0 && pattern[k] != pattern[i])
            k = next[k - 1];
        if (pattern[k] == pattern[i])
            k++;
        next[i] = k;
    }
}

int osKMP(const char *str, const char *pattern)
{
    int ret = -1;
    int next[MAX_PATTERN_SIZE];
    int strLen = osStrLen(str);
    int patternLen = osStrLen(pattern);
    int *p = NULL;
    if (patternLen <= MAX_PATTERN_SIZE)
    {
        p = next;   
    }
    else
    {
        p = (int *)osMalloc(patternLen * sizeof(int));
    }
    if (p != NULL)
    {
        getNext(p, pattern, patternLen);
        for (int i = 0, j = 0; i < strLen; i++)
        {
            while (j > 0 && str[i] != pattern[j])
                j = p[j - 1];

            if (str[i] == pattern[j])
                j++;
            if (j == patternLen)
                ret = i - patternLen + 1;
        }
        if (p != next)
        {
            osFree(p);
        }
    }
    return ret;
}
