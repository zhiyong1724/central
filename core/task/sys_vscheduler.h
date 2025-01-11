#ifndef __SYS_VSCHEDULER_H__
#define __SYS_VSCHEDULER_H__
#include "sys_list.h"
#include "sys_tree.h"
#include "sys_task.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef void *(*tick_t)(void *scheduler, uint64_t *ns);
typedef int (*add_task_t)(void *scheduler, void *task);
typedef void *(*remove_task_t)(void *scheduler, void *task);
typedef int (*modify_priority_t)(void *scheduler, void *task, int priority);
typedef void *(*get_running_task_t)(void *scheduler);
typedef void *(*yield_t)(void *scheduler);
typedef struct sys_scheduler_interfaces_t
{
    tick_t tick;
    add_task_t add_task;
    remove_task_t remove_task;
    modify_priority_t modify_priority;
    get_running_task_t get_running_task;
    yield_t yield;
} sys_scheduler_interfaces_t;

typedef struct sys_task_control_block_t
{
    union
    {
        sys_list_node_t list_node;
        sys_tree_node_t tree_node;
    } node;
    sys_task_state_t task_state;
    int scheduler_id;
    uint64_t sleep_time;
} sys_task_control_block_t;

typedef struct sys_vscheduler_t
{
    void *schedulers[SYS_MAX_SCHEDULER_COUNT];
    sys_scheduler_interfaces_t scheduler_interfaces[SYS_MAX_SCHEDULER_COUNT];
    int scheduler_count;
    uint64_t clock;
    sys_list_node_t *suspended_list;
    sys_tree_node_t *sleep_tree;
    sys_task_control_block_t *running_task;
    sys_task_control_block_t *min_sleep_task;
} sys_vscheduler_t;
/*********************************************************************************************************************
* sys_vscheduler_t初始化
* vscheduler：sys_vscheduler_t对象
* return：0：初始化成功
*********************************************************************************************************************/
int sys_vscheduler_init(sys_vscheduler_t *vscheduler);
/*********************************************************************************************************************
* sys_task_control_block_t初始化
* vscheduler：sys_vscheduler_t对象
* task_control_block：sys_task_control_block_t对象
* return：0：初始化成功
*********************************************************************************************************************/
int sys_task_control_block_init(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block);
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
* task_control_block：sys_task_control_block_t对象
* return：0：添加成功
*********************************************************************************************************************/
int sys_vscheduler_add_task(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block);
/*********************************************************************************************************************
* 修改优先级
* vscheduler：sys_vscheduler_t对象
* task_control_block：sys_task_control_block_t对象
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int sys_vscheduler_modify_priority(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block, int priority);
/*********************************************************************************************************************
* 时钟滴答
* osVScheduler：sys_vscheduler_t对象
* scheduler_id：指定使用哪个调度器，如果为SYS_MAX_SCHEDULER_COUNT则表示按照顺序调用
* ns：输入与上次tick的时间间隔，输出下次tick的时间间隔
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_task_control_block_t *sys_vscheduler_tick(sys_vscheduler_t *vscheduler, int scheduler_id, uint64_t *ns);
/*********************************************************************************************************************
* 挂起一个任务
* vscheduler：sys_vscheduler_t对象
* task_control_block：sys_task_control_block_t对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_task_control_block_t *sys_vscheduler_supend(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block);
/*********************************************************************************************************************
* 恢复一个任务
* vscheduler：sys_vscheduler_t对象
* task_control_block：sys_task_control_block_t对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_task_control_block_t *sys_vscheduler_resume(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block);
/*********************************************************************************************************************
* 休眠一段时间
* vscheduler：sys_vscheduler_t对象
* ns：休眠的时间
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_task_control_block_t *sys_vscheduler_sleep(sys_vscheduler_t *vscheduler, uint64_t ns);
/*********************************************************************************************************************
* 唤醒睡眠的任务
* vscheduler：sys_vscheduler_t对象
* task_control_block：sys_task_control_block_t对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_task_control_block_t *sys_vscheduler_wakeup(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block);
/*********************************************************************************************************************
* 退出当前任务
* vscheduler：sys_vscheduler_t对象
* return：调用成功返回下一个任务控制块，否则返回NULL
*********************************************************************************************************************/
sys_task_control_block_t *sys_vscheduler_exit(sys_vscheduler_t *vscheduler);
/*********************************************************************************************************************
* 获取当前任务
* vscheduler：sys_vscheduler_t对象
* return：当前任务
*********************************************************************************************************************/
sys_task_control_block_t *sys_vscheduler_get_running_task(sys_vscheduler_t *vscheduler);
#ifdef __cplusplus
}
#endif
#endif