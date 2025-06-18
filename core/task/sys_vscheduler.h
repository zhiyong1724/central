#ifndef __SYS_VSCHEDULER_H__
#define __SYS_VSCHEDULER_H__
#include "sys_list.h"
#include "sys_tree.h"
#include "sys_task.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define SYS_MAX_SCHEDULER_COUNT 4                                                 //最大调度器数量
typedef void *(*tick_t)(void *scheduler, uint64_t ns);
typedef int (*add_task_t)(void *scheduler, void *task);
typedef void *(*remove_task_t)(void *scheduler, void *task);
typedef int (*set_priority_t)(void *scheduler, void *task, int priority);
typedef void *(*get_running_task_t)(void *scheduler);
typedef void *(*yield_t)(void *scheduler);
typedef struct sys_scheduler_interfaces_t
{
    tick_t tick;
    add_task_t add_task;
    remove_task_t remove_task;
    set_priority_t set_priority;
    get_running_task_t get_running_task;
    yield_t yield;
} sys_scheduler_interfaces_t;

typedef struct sys_vtask_control_block_t
{
    int scheduler_id;
    sys_task_state_t task_state;
} sys_vtask_control_block_t;

typedef struct sys_vscheduler_t
{
    void *schedulers[SYS_MAX_SCHEDULER_COUNT];
    sys_scheduler_interfaces_t scheduler_interfaces[SYS_MAX_SCHEDULER_COUNT];
    int scheduler_count;
    int task_count;
    sys_vtask_control_block_t *running_task;
} sys_vscheduler_t;
/*********************************************************************************************************************
* sys_vscheduler_t初始化
* vscheduler：sys_vscheduler_t对象
*********************************************************************************************************************/
void sys_vscheduler_init(sys_vscheduler_t *vscheduler);
/*********************************************************************************************************************
* sys_vtask_control_block_t初始化
* vscheduler：sys_vscheduler_t对象
* task_control_block：sys_vtask_control_block_t对象
*********************************************************************************************************************/
void sys_vtask_control_block_init(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block);
/*********************************************************************************************************************
* 添加调度器
* vscheduler：sys_vscheduler_t对象
* scheduler：调度器对象
* scheduler_interfaces：调度器操作函数
* return：0：添加成功
*********************************************************************************************************************/
int sys_vscheduler_add_scheduler(sys_vscheduler_t *vscheduler, void *scheduler, sys_scheduler_interfaces_t *scheduler_interfaces);
/*********************************************************************************************************************
* 增加任务
* vscheduler：sys_vscheduler_t对象
* task_control_block：sys_vtask_control_block_t对象
* return：0：添加成功
*********************************************************************************************************************/
int sys_vscheduler_add_task(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block);
/*********************************************************************************************************************
* 修改优先级
* vscheduler：sys_vscheduler_t对象
* task_control_block：sys_vtask_control_block_t对象
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int sys_vscheduler_set_priority(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block, int priority);
/*********************************************************************************************************************
* 时钟滴答
* osVScheduler：sys_vscheduler_t对象
* ns：输入与上次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_vtask_control_block_t *sys_vscheduler_tick(sys_vscheduler_t *vscheduler, uint64_t ns);
/*********************************************************************************************************************
* 暂停任务
* osVScheduler：sys_vscheduler_t对象
* task_control_block：sys_vtask_control_block_t对象
* return：返回下一个任务控制块
*********************************************************************************************************************/
sys_vtask_control_block_t *sys_vscheduler_suspend(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block);
/*********************************************************************************************************************
* 恢复任务
* osVScheduler：sys_vscheduler_t对象
* task_control_block：sys_vtask_control_block_t对象
* return：返回下一个任务控制块
*********************************************************************************************************************/
sys_vtask_control_block_t *sys_vscheduler_resume(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block);
/*********************************************************************************************************************
* 获取当前任务
* vscheduler：sys_vscheduler_t对象
* return：当前任务
*********************************************************************************************************************/
sys_vtask_control_block_t *sys_vscheduler_get_running_task(sys_vscheduler_t *vscheduler);
/*********************************************************************************************************************
* 获取任务数量
* vscheduler：sys_vscheduler_t对象
* return：任务数量
*********************************************************************************************************************/
int sys_vscheduler_get_task_count(sys_vscheduler_t *vscheduler);
#ifdef __cplusplus
}
#endif
#endif