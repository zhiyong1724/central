#ifndef __SYS_DT_SCHEDULER_H__
#define __SYS_DT_SCHEDULER_H__
#include "sys_cfg.h"
#include "sys_tree.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_dt_task_control_block_t
{
    sys_tree_node_t node;
    int priority;
    uint64_t vruntime;
} sys_dt_task_control_block_t;

typedef struct sys_dt_scheduler_t
{
    sys_tree_node_t *task_tree;
    int task_count;
    uint64_t min_vruntime;
    sys_dt_task_control_block_t *running_task;
    uint64_t switch_interval;
    uint64_t interval;
} sys_dt_scheduler_t;
/*********************************************************************************************************************
* sys_dt_scheduler_t初始化
* dt_scheduler：sys_dt_scheduler_t对象
* return：0：初始化成功
*********************************************************************************************************************/
int sys_dt_scheduler_init(sys_dt_scheduler_t *dt_scheduler);
/*********************************************************************************************************************
* sys_dt_task_control_block_t初始化
* dt_task_control_block：sys_dt_task_control_block_t对象
* priority：优先级
* return：0：初始化成功
*********************************************************************************************************************/
int sys_dt_task_control_block_init(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block, int priority);
/*********************************************************************************************************************
* 时钟滴答
* dt_scheduler：sys_dt_scheduler_t对象
* ns：输入与上次tick的时间间隔，输出下次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_dt_task_control_block_t *sys_dt_scheduler_tick(sys_dt_scheduler_t *dt_scheduler, uint64_t *ns);
/*********************************************************************************************************************
* 增加任务
* dt_scheduler：sys_dt_scheduler_t对象
* dt_task_control_block：任务控制块
* return：0:调用成功
*********************************************************************************************************************/
int sys_dt_scheduler_add_task(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block);
/*********************************************************************************************************************
* 移除任务
* dt_scheduler：sys_dt_scheduler_t对象
* dt_task_control_block：任务控制块
* return：返回下一个任务控制块
*********************************************************************************************************************/
sys_dt_task_control_block_t *sys_dt_scheduler_remove_task(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block);
/*********************************************************************************************************************
* 修改优先级
* dt_scheduler：sys_dt_scheduler_t对象
* dt_task_control_block：任务控制块
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int sys_dt_scheduler_modify_priority(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block, int priority);
/*********************************************************************************************************************
* 获取当前运行的任务
* dt_scheduler：sys_dt_scheduler_t对象
*********************************************************************************************************************/
sys_dt_task_control_block_t *sys_dt_scheduler_get_running_task(sys_dt_scheduler_t *dt_scheduler);
/*********************************************************************************************************************
* 主动放弃运行
* dt_scheduler：sys_dt_scheduler_t对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_dt_task_control_block_t *sys_dt_scheduler_yield(sys_dt_scheduler_t *dt_scheduler);
#ifdef __cplusplus
}
#endif
#endif