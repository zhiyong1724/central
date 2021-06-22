#ifndef __OSPLATFORMDEF_H__
#define __OSPLATFORMDEF_H__
#include <stdio.h>
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
    for (;;); \
} 
#else
#define osAssert(e) (void)0
#endif
extern char _heap[];
#define HEAP_ADDRESS                _heap                                      //堆空间指针
#define HEAP_SIZE                   (1024 * 1024)                              //堆空间大小
#ifdef __cplusplus
}
#endif
#endif
