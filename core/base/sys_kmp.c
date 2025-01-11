#include "sys_kmp.h"
#include "sys_string.h"
#include "sys_mem.h"
#include "sys_cfg.h"
#include "sys_error.h"
#define MAX_PATTERN_SIZE 256
static void get_next(int *next, const char *pattern, int len)
{
    sys_trace();
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

int sys_kmp(const char *str, const char *pattern)
{
    sys_trace();
    int ret = -1;
    int next[MAX_PATTERN_SIZE];
    int str_len = sys_strlen(str);
    int pattern_len = sys_strlen(pattern);
    int *p = NULL;
    if (pattern_len <= MAX_PATTERN_SIZE)
    {
        p = next;   
    }
    else
    {
        p = (int *)sys_malloc(pattern_len * sizeof(int));
    }
    if (NULL == p)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    get_next(p, pattern, pattern_len);
    for (int i = 0, j = 0; i < str_len; i++)
    {
        while (j > 0 && str[i] != pattern[j])
            j = p[j - 1];

        if (str[i] == pattern[j])
            j++;
        if (j == pattern_len)
            ret = i - pattern_len + 1;
    }
    goto finally;
exception:
finally:
    if (p != next)
    {
        sys_free(p);
    }
    return ret;
}
