#ifndef __SYS_FIFO_SCHEDULER_H__
#define __SYS_FIFO_SCHEDULER_H__
#include "sys_cfg.h"
#include "sys_list.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_fifo_task_control_block_t
{
    sys_list_node_t node;
    int priority;
} sys_fifo_task_control_block_t;

typedef struct sys_fifo_scheduler_t
{
    sys_list_node_t *task_list;
    int task_count;
    sys_fifo_task_control_block_t *running_task;
    uint64_t interval;
} sys_fifo_scheduler_t;
/*********************************************************************************************************************
* sys_fifo_scheduler_t初始化
* fifo_scheduler：sys_fifo_scheduler_t对象
*********************************************************************************************************************/
void sys_fifo_scheduler_init(sys_fifo_scheduler_t *fifo_scheduler);
/*********************************************************************************************************************
* sys_fifo_task_control_block_t初始化
* fifo_task_control_block：sys_fifo_task_control_block_t对象
* priority：优先级
*********************************************************************************************************************/
void sys_fifo_task_control_block_init(sys_fifo_scheduler_t *fifo_scheduler, sys_fifo_task_control_block_t *fifo_task_control_block, int priority);
/*********************************************************************************************************************
* 时钟滴答
* fifo_scheduler：sys_fifo_scheduler_t对象
* ns：输入与上次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_fifo_task_control_block_t *sys_fifo_scheduler_tick(sys_fifo_scheduler_t *fifo_scheduler, uint64_t ns);
/*********************************************************************************************************************
* 增加任务
* fifo_scheduler：sys_fifo_scheduler_t对象
* fifo_task_control_block：任务控制块
* return：错误码
*********************************************************************************************************************/
int sys_fifo_scheduler_add_task(sys_fifo_scheduler_t *fifo_scheduler, sys_fifo_task_control_block_t *fifo_task_control_block);
/*********************************************************************************************************************
* 移除任务
* fifo_scheduler：sys_fifo_scheduler_t对象
* fifo_task_control_block：任务控制块
* return：返回下一个任务控制块
*********************************************************************************************************************/
sys_fifo_task_control_block_t *sys_fifo_scheduler_remove_task(sys_fifo_scheduler_t *fifo_scheduler, sys_fifo_task_control_block_t *fifo_task_control_block);
/*********************************************************************************************************************
* 修改优先级
* fifo_scheduler：sys_fifo_scheduler_t对象
* fifo_task_control_block：任务控制块
* priority：优先级
* return：错误码
*********************************************************************************************************************/
int sys_fifo_scheduler_set_priority(sys_fifo_scheduler_t *fifo_scheduler, sys_fifo_task_control_block_t *fifo_task_control_block, int priority);
/*********************************************************************************************************************
* 获取当前运行的任务
* fifo_scheduler：sys_fifo_scheduler_t对象
*********************************************************************************************************************/
sys_fifo_task_control_block_t *sys_fifo_scheduler_get_running_task(sys_fifo_scheduler_t *fifo_scheduler);
/*********************************************************************************************************************
* 主动放弃运行
* fifo_scheduler：sys_fifo_scheduler_t对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_fifo_task_control_block_t *sys_fifo_scheduler_yield(sys_fifo_scheduler_t *fifo_scheduler);
#ifdef __cplusplus
}
#endif
#endif