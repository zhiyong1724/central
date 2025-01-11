#ifndef __SYS_RT_SCHEDULER_H__
#define __SYS_RT_SCHEDULER_H__
#include "sys_cfg.h"
#include "sys_list.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define SYS_RTSCHED_MAX_PRIORITY                     64                 //实时任务最大优先级
typedef struct sys_rt_task_control_block_t
{
    sys_list_node_t node;
    int priority;
} sys_rt_task_control_block_t;

typedef struct sys_rt_scheduler_t
{
    unsigned char ready_task_table[SYS_RTSCHED_MAX_PRIORITY / 8];
    unsigned char ready_group_table;
    sys_list_node_t *task_list_array[SYS_RTSCHED_MAX_PRIORITY];
    int task_count;
    sys_rt_task_control_block_t *running_task;
    uint64_t interval;
} sys_rt_scheduler_t;
/*********************************************************************************************************************
* sys_rt_scheduler_t初始化
* rt_scheduler：sys_rt_scheduler_t对象
* return：0：初始化成功
*********************************************************************************************************************/
int sys_rt_scheduler_init(sys_rt_scheduler_t *rt_scheduler);
/*********************************************************************************************************************
* sys_rt_task_control_block_t初始化
* rt_scheduler：sys_rt_scheduler_t对象
* rt_task_control_block：sys_rt_task_control_block_t对象
* priority：优先级
* return：0：初始化成功
*********************************************************************************************************************/
int sys_rt_task_control_block_init(sys_rt_scheduler_t *rt_scheduler, sys_rt_task_control_block_t *rt_task_control_block, int priority);
/*********************************************************************************************************************
* 时钟滴答
* rt_scheduler：sys_rt_scheduler_t对象
* ns：输入与上次tick的时间间隔，输出下次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_rt_task_control_block_t *sys_rt_scheduler_tick(sys_rt_scheduler_t *rt_scheduler, uint64_t *ns);
/*********************************************************************************************************************
* 增加任务
* rt_scheduler：sys_rt_scheduler_t对象
* rt_task_control_block：任务控制块
* return：0：添加成功
*********************************************************************************************************************/
int sys_rt_scheduler_add_task(sys_rt_scheduler_t *rt_scheduler, sys_rt_task_control_block_t *rt_task_control_block);
/*********************************************************************************************************************
* 移除任务
* rt_scheduler：sys_rt_scheduler_t对象
* rt_task_control_block：任务控制块
* return：返回下一个任务控制块
*********************************************************************************************************************/
sys_rt_task_control_block_t *sys_rt_scheduler_remove_task(sys_rt_scheduler_t *rt_scheduler, sys_rt_task_control_block_t *rt_task_control_block);
/*********************************************************************************************************************
* 修改优先级
* rt_scheduler：sys_rt_scheduler_t对象
* rt_task_control_block：任务控制块
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int sys_rt_scheduler_modify_priority(sys_rt_scheduler_t *rt_scheduler, sys_rt_task_control_block_t *rt_task_control_block, int priority);
/*********************************************************************************************************************
* 获取当前运行的任务
* rt_scheduler：sys_rt_scheduler_t对象
*********************************************************************************************************************/
sys_rt_task_control_block_t *sys_rt_scheduler_get_running_task(sys_rt_scheduler_t *rt_scheduler);
/*********************************************************************************************************************
* 主动放弃运行
* rt_scheduler：sys_rt_scheduler_t对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_rt_task_control_block_t *sys_rt_scheduler_yield(sys_rt_scheduler_t *rt_scheduler);
#ifdef __cplusplus
}
#endif
#endif