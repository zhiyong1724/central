#ifndef __SYS_IDLE_SCHEDULER_H__
#define __SYS_IDLE_SCHEDULER_H__
#include "sys_cfg.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_idle_task_control_block_t
{
    int priority;
} sys_idle_task_control_block_t;

typedef struct sys_idle_scheduler_t
{
    sys_idle_task_control_block_t *running_task;
} sys_idle_scheduler_t;
/*********************************************************************************************************************
* sys_idle_scheduler_t初始化
* idle_scheduler：sys_idle_scheduler_t对象
* return：0：初始化成功
*********************************************************************************************************************/
int sys_idle_scheduler_init(sys_idle_scheduler_t *idle_scheduler);
/*********************************************************************************************************************
* sys_idle_task_control_block_t初始化
* idle_task_control_block：sys_idle_task_control_block_t对象
* priority：优先级
* return：0：初始化成功
*********************************************************************************************************************/
int sys_idle_task_control_block_init(sys_idle_scheduler_t *idle_scheduler, sys_idle_task_control_block_t *idle_task_control_block, int priority);
/*********************************************************************************************************************
* 时钟滴答
* idle_scheduler：sys_idle_scheduler_t对象
* ns：输入与上次tick的时间间隔，输出下次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_idle_task_control_block_t *sys_idle_scheduler_tick(sys_idle_scheduler_t *idle_scheduler, uint64_t *ns);
/*********************************************************************************************************************
* 增加任务
* idle_scheduler：sys_idle_scheduler_t对象
* idle_task_control_block：任务控制块
* return：0:调用成功
*********************************************************************************************************************/
int sys_idle_scheduler_add_task(sys_idle_scheduler_t *idle_scheduler, sys_idle_task_control_block_t *idle_task_control_block);
/*********************************************************************************************************************
* 移除任务
* idle_scheduler：sys_idle_scheduler_t对象
* idle_task_control_block：任务控制块
* return：返回下一个任务控制块
*********************************************************************************************************************/
sys_idle_task_control_block_t *sys_idle_scheduler_remove_task(sys_idle_scheduler_t *idle_scheduler, sys_idle_task_control_block_t *idle_task_control_block);
/*********************************************************************************************************************
* 修改优先级
* idle_scheduler：sys_idle_scheduler_t对象
* idle_task_control_block：任务控制块
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int sys_idle_scheduler_modify_priority(sys_idle_scheduler_t *idle_scheduler, sys_idle_task_control_block_t *idle_task_control_block, int priority);
/*********************************************************************************************************************
* 获取当前运行的任务
* idle_scheduler：sys_idle_scheduler_t对象
*********************************************************************************************************************/
sys_idle_task_control_block_t *sys_idle_scheduler_get_running_task(sys_idle_scheduler_t *idle_scheduler);
/*********************************************************************************************************************
* 主动放弃运行
* idle_scheduler：sys_idle_scheduler_t对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_idle_task_control_block_t *sys_idle_scheduler_yield(sys_idle_scheduler_t *idle_scheduler);
#ifdef __cplusplus
}
#endif
#endif