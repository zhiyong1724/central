#ifndef __SYS_CFG_H__
#define __SYS_CFG_H__
#include "sys_external_cfg.h"
//版本号
#define SYS_VERSION "1.0.0"
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
#define SYS_HEAP_SIZE                   (1024 * 1024)                             //堆空间大小
#endif
#ifndef SYS_BUDDY_PAGE_SIZE
#define SYS_BUDDY_PAGE_SIZE             4096                                      //伙伴算法页面大小
#endif
//调度器相关
#ifndef SYS_MAX_SCHEDULER_COUNT
#define SYS_MAX_SCHEDULER_COUNT 3                                                 //最大调度器数量
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
#define SYS_TASK_MAX_NAME_LEN                        32                 //最大任务名称长度
#endif

#ifndef SYS_TASK_STACK_GROWTH
#define SYS_TASK_STACK_GROWTH                        1                  //堆栈生长方向，1表示往下生长，0表示往上生长
#endif

#ifndef SYS_TASK_STACK_MAGIC
#define SYS_TASK_STACK_MAGIC                         0xaa5555aa         //用户堆栈魔术数
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
//信号量相关
#ifndef SYS_MAX_SEMAPHORE_COUNT
#define SYS_MAX_SEMAPHORE_COUNT                      1024              //最大信号个数
#endif
//队列相关
#ifndef SYS_MAX_QUEUE_LENGTH
#define SYS_MAX_QUEUE_LENGTH                         1024              //最大队列长度
#endif
//虚拟文件系统相关
#ifndef VFS_MAX_FILE_NAME_LEN
#define VFS_MAX_FILE_NAME_LEN                       256                //最大文件名长度
#endif
#ifndef VFS_MAX_FILE_PATH_LEN
#define VFS_MAX_FILE_PATH_LEN                       1024               //最大文件名长度
#endif
#endif