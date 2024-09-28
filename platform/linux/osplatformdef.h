#ifndef __OSPLATFORMDEF_H__
#define __OSPLATFORMDEF_H__
#include <stdio.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
typedef unsigned long stack_size_t;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ENABLE_PRINTF                                     1
#define ENABLE_ASSERT                                     1
#define OS_SYS_CLOCK_PERIOD_NS         (10 * 1000 * 1000)                        //系统时钟周期ns
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
extern char _heap[];
#define OS_HEAP_ADDRESS                _heap                                      //堆空间指针
#define OS_HEAP_SIZE                   (64 * 1024 * 1024)                         //堆空间大小
#ifdef __cplusplus
}
#endif
#endif
