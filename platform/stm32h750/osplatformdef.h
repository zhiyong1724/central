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
    volatile int divisor = 0; \
    volatile int num = 5 / divisor; \
    (void)num; \
} 
#else
#define osAssert(e) (void)0
#endif
#define OS_HEAP_ADDRESS                (void *)0x60000000                                   //堆空间指针
#define OS_HEAP_SIZE                   ((os_size_t)0x63800000 - (os_size_t)0x60000000)
#ifdef __cplusplus
}
#endif
#endif
