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
#ifndef HEAP_ADDRESS
#define HEAP_ADDRESS                heap                                      //堆空间指针
#endif
#ifndef HEAP_SIZE
#define HEAP_SIZE                   (1024 * 1024)                             //堆空间大小
#endif
#ifndef BUDDY_PAGE_SIZE
#define BUDDY_PAGE_SIZE             4096                                      //伙伴算法页面大小
#endif
//时钟相关
#ifndef SYS_CLOCK_PERIOD_NS
#define SYS_CLOCK_PERIOD_NS         (10 * 1000 * 1000)                        //系统时钟周期ns
#endif
//调度器相关
#ifndef MAX_SCHEDULER_COUNT
#define MAX_SCHEDULER_COUNT 3                                                 //最大调度器数量
#endif

#ifndef RTSCHED_MIN_SWITCH_INTERVAL_NS
#define RTSCHED_MIN_SWITCH_INTERVAL_NS              (1 * 1000 * 1000)        //实时调度器最小切换间隔ns
#endif

#ifndef DTSCHED_MIN_SWITCH_INTERVAL_NS
#define DTSCHED_MIN_SWITCH_INTERVAL_NS              (1 * 1000 * 1000)        //分时调度器最小切换间隔ns
#endif

#ifndef DTSCHED_MAX_SCHED_CYCLE_NS
#define DTSCHED_MAX_SCHED_CYCLE_NS                  (100 * 1000 * 1000)      //分时调度器最大调度周期ns
#endif

//任务相关
#ifndef TASK_MAX_NAME_LEN
#define TASK_MAX_NAME_LEN                        32                 //最大任务名称长度
#endif

#ifndef TASK_STACK_GROWTH
#define TASK_STACK_GROWTH                        1                  //堆栈生长方向，1表示往下生长，0表示往上生长
#endif

#ifndef TASK_STACK_MAGIC
#define TASK_STACK_MAGIC                         0xaa5555aa         //用户堆栈魔术数，用于检查堆栈溢出
#endif

#ifndef DEFAULT_TASK_STACK_SIZE
#define DEFAULT_TASK_STACK_SIZE                  512               //堆栈大小
#endif
//信号量相关
#ifndef MAX_SEMAPHORE_COUNT
#define MAX_SEMAPHORE_COUNT                      1024              //最大信号个数
#endif
//队列相关
#ifndef MAX_QUEUE_LENGTH
#define MAX_QUEUE_LENGTH                         1024              //最大队列长度
#endif
//文件系统相关
#ifndef MAX_FILE_NAME_LENGTH
#define MAX_FILE_NAME_LENGTH                     256               //最大文件名长度
#endif
#endif