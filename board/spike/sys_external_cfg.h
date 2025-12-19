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
extern char _fsys_heap;
extern char _esys_heap;
#define SYS_HEAP_ADDRESS                &_fsys_heap                                //堆空间指针
#define SYS_HEAP_SIZE                   (&_esys_heap - &_fsys_heap)                //堆空间大小
//原子变量相关
struct arch_atomic_t
{
    volatile int value;
};
int arch_atomic_store(struct arch_atomic_t *, int);
#define sys_arch_atomic_store(atomic, va) arch_atomic_store((struct arch_atomic_t *)atomic, va)
int arch_atomic_load(const struct arch_atomic_t *);
#define sys_arch_atomic_load(atomic) arch_atomic_load((struct arch_atomic_t *)atomic)
int arch_atomic_exchange(struct arch_atomic_t *, int);
#define sys_arch_atomic_exchange(atomic, desired) arch_atomic_exchange((struct arch_atomic_t *)atomic, desired)
int arch_atomic_compare_exchange_weak(struct arch_atomic_t *, int *, int);
#define sys_arch_atomic_compare_exchange_weak(atomic, expected, desired) arch_atomic_compare_exchange_weak((struct arch_atomic_t *)atomic, expected, desired)
int arch_atomic_fetch_add(struct arch_atomic_t *, int);
#define sys_arch_atomic_fetch_add(atomic, va) arch_atomic_fetch_add((struct arch_atomic_t *)atomic, va)
int arch_atomic_fetch_and(struct arch_atomic_t *, int);
#define sys_arch_atomic_fetch_and(atomic, va) arch_atomic_fetch_and((struct arch_atomic_t *)atomic, va)
int arch_atomic_fetch_or(struct arch_atomic_t *, int);
#define sys_arch_atomic_fetch_or(atomic, va) arch_atomic_fetch_or((struct arch_atomic_t *)atomic, va)
int arch_atomic_fetch_xor(struct arch_atomic_t *, int);
#define sys_arch_atomic_fetch_xor(atomic, va) arch_atomic_fetch_xor((struct arch_atomic_t *)atomic, va)
//CPU相关
int get_cpu();
int local_irq_save();
void local_irq_restore(int state);
#ifdef __cplusplus
}
#endif
#endif
