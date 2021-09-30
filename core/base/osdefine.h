#ifndef __OSDEFINE_H__
#define __OSDEFINE_H__
#include "osplatformdef.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OS_VERSION 100
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef NULL
#define NULL (void *)0
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//内存相关
#ifndef OS_HEAP_ADDRESS
#define OS_HEAP_ADDRESS                heap                                      //堆空间指针
#endif
#ifndef OS_HEAP_SIZE
#define OS_HEAP_SIZE                   (1024 * 1024)                             //堆空间大小
#endif
#ifndef OS_BUDDY_PAGE_SIZE
#define OS_BUDDY_PAGE_SIZE             4096                                      //伙伴算法页面大小
#endif
//时钟相关
#ifndef OS_SYS_CLOCK_PERIOD_NS
#define OS_SYS_CLOCK_PERIOD_NS         (1 * 1000 * 1000)                        //系统时钟周期ns
#endif
//调度器相关
#ifndef OS_MAX_SCHEDULER_COUNT
#define OS_MAX_SCHEDULER_COUNT 3                                                 //最大调度器数量
#endif

#ifndef OS_RTSCHED_MIN_SWITCH_INTERVAL_NS
#define OS_RTSCHED_MIN_SWITCH_INTERVAL_NS              (1 * 1000 * 1000)        //实时调度器最小切换间隔ns
#endif

#ifndef OS_DTSCHED_MIN_SWITCH_INTERVAL_NS
#define OS_DTSCHED_MIN_SWITCH_INTERVAL_NS              (1 * 1000 * 1000)        //分时调度器最小切换间隔ns
#endif

#ifndef OS_DTSCHED_MAX_SCHED_CYCLE_NS
#define OS_DTSCHED_MAX_SCHED_CYCLE_NS                  (100 * 1000 * 1000)      //分时调度器最大调度周期ns
#endif

//任务相关
#ifndef OS_TASK_MAX_NAME_LEN
#define OS_TASK_MAX_NAME_LEN                        32                 //最大任务名称长度
#endif

#ifndef OS_TASK_STACK_GROWTH
#define OS_TASK_STACK_GROWTH                        1                  //堆栈生长方向，1表示往下生长，0表示往上生长
#endif

#ifndef OS_TASK_STACK_MAGIC
#define OS_TASK_STACK_MAGIC                         0xaa5555aa         //用户堆栈魔术数
#endif

#ifndef OS_DEFAULT_TASK_STACK_SIZE
#define OS_DEFAULT_TASK_STACK_SIZE                  512               //堆栈大小
#endif
//信号量相关
#ifndef OS_MAX_SEMAPHORE_COUNT
#define OS_MAX_SEMAPHORE_COUNT                      1024              //最大信号个数
#endif
//队列相关
#ifndef OS_MAX_QUEUE_LENGTH
#define OS_MAX_QUEUE_LENGTH                         1024              //最大队列长度
#endif
//虚拟文件系统相关
#ifndef OS_USE_VFS
#define OS_USE_VFS                                  0                 //是否开启虚拟文件系统
#endif
#ifndef OS_MAX_FILE_NAME_LENGTH
#define OS_MAX_FILE_NAME_LENGTH                     256               //最大文件名长度
#endif
#ifndef OS_MAX_FILE_PATH_LENGTH
#define OS_MAX_FILE_PATH_LENGTH                     1024               //最大文件路径长度
#endif
#ifndef OS_MAX_FS_COUNT
#define OS_MAX_FS_COUNT                             2                  //最大文件系统数量
#endif
#endif