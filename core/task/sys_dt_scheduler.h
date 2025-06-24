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
    uint64_t load;
    uint64_t acc_load;
    uint64_t mean_load;
    uint64_t mean_load_cal_interval;
    uint64_t mean_load_cal_cycle;
    int load_leveling_flag;
} sys_dt_scheduler_t;
/*********************************************************************************************************************
* sys_dt_scheduler_t初始化
* dt_scheduler：sys_dt_scheduler_t对象
*********************************************************************************************************************/
void sys_dt_scheduler_init(sys_dt_scheduler_t *dt_scheduler);
/*********************************************************************************************************************
* sys_dt_task_control_block_t初始化
* dt_task_control_block：sys_dt_task_control_block_t对象
* priority：优先级
*********************************************************************************************************************/
void sys_dt_task_control_block_init(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block, int priority);
/*********************************************************************************************************************
* 时钟滴答
* dt_scheduler：sys_dt_scheduler_t对象
* ns：输入与上次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_dt_task_control_block_t *sys_dt_scheduler_tick(sys_dt_scheduler_t *dt_scheduler, uint64_t ns);
/*********************************************************************************************************************
* 增加任务
* dt_scheduler：sys_dt_scheduler_t对象
* dt_task_control_block：任务控制块
* return：错误码
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
* return：错误码
*********************************************************************************************************************/
int sys_dt_scheduler_set_priority(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block, int priority);
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
/*********************************************************************************************************************
* 找到最后执行的任务
* dt_scheduler：sys_dt_scheduler_t对象
* return：最后执行的任务
*********************************************************************************************************************/
sys_dt_task_control_block_t *sys_dt_scheduler_find_last_task(sys_dt_scheduler_t *dt_scheduler);
/*********************************************************************************************************************
* 返回新的负载，如果移除任务
* dt_scheduler：sys_dt_scheduler_t对象
* dt_task_control_block：如果要移除的任务
* return：新的负载
*********************************************************************************************************************/
uint64_t sys_dt_scheduler_load_if_remove_task(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block);
/*********************************************************************************************************************
* 返回新的负载，如果增加任务
* dt_scheduler：sys_dt_scheduler_t对象
* dt_task_control_block：如果要增加的任务
* return：新的负载
*********************************************************************************************************************/
uint64_t sys_dt_scheduler_load_if_add_task(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block);
#ifdef __cplusplus
}
#endif
#endif