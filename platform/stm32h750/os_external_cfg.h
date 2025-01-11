#ifndef __SYS_EXTERNAL_CFG_H__
#define __SYS_EXTERNAL_CFG_H__
#include <stdio.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
//调试相关开关
#define SYS_TRACE_ENABLE 1
#define SYS_DEBUG_ENABLE 1
#define SYS_WARN_ENABLE 1
#define SYS_ERROR_ENABLE 1
#define SYS_ASSERT_ENABLE 1
//栈类型
typedef unsigned long stack_size_t;
//堆空间
extern int _sys_heap_start[0];
extern int _sys_heap_end[0];
#define SYS_HEAP_ADDRESS                (void *)_sys_heap_start                                   //堆空间指针
#define SYS_HEAP_SIZE                   ((stack_size_t)_sys_heap_end - (stack_size_t)_sys_heap_start)
#ifdef __cplusplus
}
#endif
#endif
