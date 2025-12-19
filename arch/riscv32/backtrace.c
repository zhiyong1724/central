#include <stdio.h>
int _backtrace(void **array, int size, int fp)
{    
    int i = 0;
    void **pfp = (void **)fp;
    for (i = 0; i < size && pfp != NULL; i++)
    {
        void **pra = pfp - 1;
        array[i] = *pra;
        pfp -= 2;
        pfp = (void **)*pfp;
    }
    return i;
}