#ifndef __OSPLATFORMDEF_H__
#define __OSPLATFORMDEF_H__
#include <stdio.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
typedef unsigned char os_byte_t;
typedef unsigned long os_size_t;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ENABLE_PRINTF                                     1
#define ENABLE_ASSERT                                     1
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if ENABLE_PRINTF
#define osPrintf(format, ...) printf(format, ##__VA_ARGS__)
#else
#define osPrintf(format, ...) (void)0;
#endif

#if ENABLE_ASSERT
#define osAssert(e) \
if(e) \
{ \
    (void)0; \
} \
else \
{ \
    osPrintf("osAssert:%s:%s:%d\n", __FILE__, __func__, __LINE__); \
    unsigned int *p = (unsigned int *)-1; \
    *p = 0; \
} 
#else
#define osAssert(e) (void)0
#endif
extern int _sys_heap_start[0];
extern int _sys_heap_end[0];
#define OS_HEAP_ADDRESS                (void *)&_sys_heap_start                                   //堆空间指针
#define OS_HEAP_SIZE                   ((os_size_t)&_sys_heap_end - (os_size_t)&_sys_heap_start)
#ifdef __cplusplus
}
#endif
#endif
