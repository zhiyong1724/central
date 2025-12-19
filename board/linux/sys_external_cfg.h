#ifndef __SYS_EXTERNAL_CFG_H__
#define __SYS_EXTERNAL_CFG_H__
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdatomic.h>
#ifdef __cplusplus
extern "C"
{
#endif
//调试相关开关
#define SYS_TRACE_ENABLE 0
#define SYS_DEBUG_ENABLE 0
#define SYS_WARN_ENABLE 1
#define SYS_ERROR_ENABLE 1
#define SYS_ASSERT_ENABLE 1
//shell
#define SHELL_STACK_SIZE 32 * 1024
//栈类型
typedef unsigned long stack_size_t;                      
//堆空间
extern char heap_start[];
#define SYS_HEAP_ADDRESS                heap_start                                 //堆空间指针
#define SYS_HEAP_SIZE                   (64 * 1024 * 1024)                         //堆空间大小
//CPU相关
int get_cpu();
int local_irq_save();
void local_irq_restore(int state);
#ifdef __cplusplus
}
#endif
#endif
