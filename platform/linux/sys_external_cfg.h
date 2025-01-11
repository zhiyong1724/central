#ifndef __SYS_EXTERNAL_CFG_H__
#define __SYS_EXTERNAL_CFG_H__
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#ifdef __cplusplus
extern "C"
{
#endif
//调试相关开关
#define SYS_TRACE_ENABLE 0
#define SYS_DEBUG_ENABLE 1
#define SYS_WARN_ENABLE 1
#define SYS_ERROR_ENABLE 1
#define SYS_ASSERT_ENABLE 1
//栈类型
typedef unsigned long stack_size_t;                      
//堆空间
extern char _heap[];
#define SYS_HEAP_ADDRESS                _heap                                      //堆空间指针
#define SYS_HEAP_SIZE                   (64 * 1024 * 1024)                         //堆空间大小
#ifdef __cplusplus
}
#endif
#endif
