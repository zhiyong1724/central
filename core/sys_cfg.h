#ifndef __SYS_CFG_H__
#define __SYS_CFG_H__
#include "sys_external_cfg.h"
//版本号
#define SYS_VERSION "1.2.0"
//空指针定义
#ifndef NULL
#define NULL (void *)0
#endif
//调试函数
#ifndef sys_trace
#if SYS_TRACE_ENABLE
#define sys_trace() \
    printf("Trace: %s (%s:%d)\n", __func__, __FILE__, __LINE__)
#else
#define sys_trace()
#endif
#endif

#ifndef sys_info
#if SYS_INFO_ENABLE
#define sys_info(fmt, ...) \
    printf(""fmt"\n", ##__VA_ARGS__)
#else
#define sys_info(...)
#endif
#endif

#ifndef sys_debug
#if SYS_DEBUG_ENABLE
#define sys_debug(fmt, ...) \
    printf("Debug: "fmt" (%s:%d)\n", ##__VA_ARGS__, __FILE__, __LINE__)
#else
#define sys_debug(...)
#endif
#endif

#ifndef sys_warn
#if SYS_WARN_ENABLE
#define sys_warn(fmt, ...) \
    printf("Warn: "fmt" (%s:%d)\n", ##__VA_ARGS__, __FILE__, __LINE__)
#else
#define sys_warn(...)
#endif
#endif

#ifndef sys_error
#if SYS_ERROR_ENABLE
#define sys_error(fmt, ...) \
    printf("Error: "fmt" (%s:%d)\n", ##__VA_ARGS__, __FILE__, __LINE__)
#else
#define sys_error(...)
#endif
#endif

#ifndef sys_assert
#if SYS_ASSERT_ENABLE
#define sys_assert(test) assert(test)
#else
#define sys_assert(test)
#endif
#endif
//内存相关
#ifndef SYS_HEAP_ADDRESS
#define SYS_HEAP_ADDRESS                heap                                      //堆空间指针
#endif
#ifndef SYS_HEAP_SIZE
#define SYS_HEAP_SIZE                   (64 * 1024 * 1024)                        //堆空间大小
#endif
#ifndef SYS_BUDDY_PAGE_SIZE
#define SYS_BUDDY_PAGE_SIZE             4096                                      //伙伴算法页面大小，必须是2的幂
#endif
#ifndef SYS_BUDDY_ADDRESS_ALIGNMENT
#define SYS_BUDDY_ADDRESS_ALIGNMENT     SYS_BUDDY_PAGE_SIZE                       //伙伴算法页面地址对齐
#endif
#if SYS_BUDDY_ADDRESS_ALIGNMENT == 0 || SYS_BUDDY_PAGE_SIZE % SYS_BUDDY_ADDRESS_ALIGNMENT != 0
#error "SYS_BUDDY_ADDRESS_ALIGNMENT must be non-zero and divide SYS_BUDDY_PAGE_SIZE"
#endif
//调度相关
#ifndef SYS_FIFOSCHED_MIN_SWITCH_INTERVAL_NS
#define SYS_FIFOSCHED_MIN_SWITCH_INTERVAL_NS            (1 * 1000 * 1000)        //FIFO调度器最小切换间隔ns
#endif

#ifndef SYS_RTSCHED_MIN_SWITCH_INTERVAL_NS
#define SYS_RTSCHED_MIN_SWITCH_INTERVAL_NS              (1 * 1000 * 1000)        //实时调度器最小切换间隔ns
#endif

#ifndef SYS_DTSCHED_MIN_SWITCH_INTERVAL_NS
#define SYS_DTSCHED_MIN_SWITCH_INTERVAL_NS              (1 * 1000 * 1000)        //分时调度器最小切换间隔ns
#endif

#ifndef SYS_DTSCHED_MAX_SCHED_CYCLE_NS
#define SYS_DTSCHED_MAX_SCHED_CYCLE_NS                  (100 * 1000 * 1000)      //分时调度器最大调度周期ns
#endif

//任务相关
#ifndef SYS_TASK_MAX_NAME_LEN
#define SYS_TASK_MAX_NAME_LEN                        256                //最大任务名称长度
#endif

#ifndef SYS_TASK_STACK_GROWTH
#define SYS_TASK_STACK_GROWTH                        1                  //堆栈生长方向，1表示往下生长，0表示往上生长
#endif

#ifndef SYS_DEFAULT_TASK_STACK_SIZE
#define SYS_DEFAULT_TASK_STACK_SIZE                  4096               //堆栈大小
#endif

#ifndef SYS_DEFAULT_TASK_PRIORITY
#define SYS_DEFAULT_TASK_PRIORITY                    20               //默认任务优先级
#endif

#ifndef SYS_DEFAULT_RTTASK_PRIORITY
#define SYS_DEFAULT_RTTASK_PRIORITY                  32               //默认实时任务优先级
#endif
//虚拟文件系统相关
#ifndef VFS_MAX_FILE_NAME_LEN
#define VFS_MAX_FILE_NAME_LEN                       256                //最大文件名长度
#endif
#ifndef VFS_MAX_FILE_PATH_LEN
#define VFS_MAX_FILE_PATH_LEN                       1024               //最大文件名长度
#endif
//原子变量相关
#ifndef sys_arch_atomic_store
#define sys_arch_atomic_store(atomic, va) atomic_store((atomic_int *)atomic, va)
#endif
#ifndef sys_arch_atomic_load
#define sys_arch_atomic_load(atomic) atomic_load((atomic_int *)atomic)
#endif
#ifndef sys_arch_atomic_exchange
#define sys_arch_atomic_exchange(atomic, desired) atomic_exchange((atomic_int *)atomic, desired)
#endif
#ifndef sys_arch_atomic_compare_exchange_weak
#define sys_arch_atomic_compare_exchange_weak(atomic, expected, desired) atomic_compare_exchange_weak((atomic_int *)atomic, expected, desired)
#endif
#ifndef sys_arch_atomic_fetch_add
#define sys_arch_atomic_fetch_add(atomic, va) atomic_fetch_add((atomic_int *)atomic, va)
#endif
#ifndef sys_arch_atomic_fetch_and
#define sys_arch_atomic_fetch_and(atomic, va) atomic_fetch_and((atomic_int *)atomic, va)
#endif
#ifndef sys_arch_atomic_fetch_or
#define sys_arch_atomic_fetch_or(atomic, va) atomic_fetch_or((atomic_int *)atomic, va)
#endif
#ifndef sys_arch_atomic_fetch_xor
#define sys_arch_atomic_fetch_xor(atomic, va) atomic_fetch_xor((atomic_int *)atomic, va)
#endif
//CPU相关
#ifndef SYS_CPU_MAX_NAME_LEN
#define SYS_CPU_MAX_NAME_LEN                        256                 //最大cpu名称长度
#endif
#ifndef sys_get_cpu
#define sys_get_cpu get_cpu
#endif
#ifndef sys_local_irq_save
#define sys_local_irq_save local_irq_save
#endif
#ifndef sys_local_irq_restore
#define sys_local_irq_restore local_irq_restore
#endif
#endif
