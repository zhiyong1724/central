#ifndef __SYS_TASK_MANAGER_H__
#define __SYS_TASK_MANAGER_H__
#include "sys_id_manager.h"
#include "sys_vscheduler.h"
#include "sys_rt_scheduler.h"
#include "sys_dt_scheduler.h"
#include "sys_idle_scheduler.h"
#include "sys_vector.h"
#include "sys_task.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_task_t
{
    sys_list_node_t node;
    sys_task_control_block_t task_control_block;
    union
    {
        sys_rt_task_control_block_t rt_task_control_block;
        sys_dt_task_control_block_t dt_task_control_block;
        sys_idle_task_control_block_t idle_task_control_block;
    } real_task_control_block;
    union
    {
        sys_list_node_t list_node;
        sys_tree_node_t tree_node;
    } exnode;
    char name[SYS_TASK_MAX_NAME_LEN];
    sys_tid_t tid;
    struct sys_task_t *parent;
    struct sys_task_t *children;
    int children_count;
    stack_size_t *stack_start;
    stack_size_t *stack_top;
    int stack_size;
    stack_size_t *task_stack_magic;
    task_function_t task_function;
    void *arg;
    sys_tid_t wait_tid_t;
} sys_task_t;

typedef struct sys_task_manager_t
{
    sys_vscheduler_t vscheduler;
    sys_rt_scheduler_t rt_scheduler;
    sys_dt_scheduler_t dt_scheduler;
    sys_idle_scheduler_t idle_scheduler;
    sys_id_manager_t id_manager;
    sys_vector_t task_list;
    int task_count;
    sys_task_t *delete_task_list;
    sys_task_t *init_task;
    uint64_t tick_count;
    int cpu_usage;
    uint64_t idle_task_tick_count;
    uint64_t idle_tick_count;
} sys_task_manager_t;
/*********************************************************************************************************************
* sys_task_manager_t初始化
* task_manager：sys_task_manager_t对象
* return：0：初始化成功
*********************************************************************************************************************/
int sys_task_manager_init(sys_task_manager_t *task_manager);
/*********************************************************************************************************************
* 创建任务
* task_manager：sys_task_manager_t对象
* tid：任务tid
* task_function：任务处理函数
* arg：传给任务的参数
* name：任务名称，最大长度为SYS_TASK_MAX_NAME_LEN
* task_type：SYS_TASK_TYPE_RT为实时任务，SYS_TASK_TYPE_DT为分时任务
* priority：实时任务优先级范围0-63，分时任务优先级范围为0-39
* stack_size：任务堆栈大小
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_create_task(sys_task_manager_t *task_manager, sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, sys_task_type_t task_type, int priority, int stack_size);
/*********************************************************************************************************************
* 时钟滴答
* task_manager：sys_task_manager_t对象
* next_task：下一个任务
* ns：输入与上次tick的时间间隔，输出下次tick的时间间隔
*********************************************************************************************************************/
void sys_task_manager_tick(sys_task_manager_t *task_manager, sys_task_t **next_task, uint64_t *ns);
/*********************************************************************************************************************
* 修改优先级
* task_manager：sys_task_manager_t对象
* tid：任务tid
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int sys_task_manager_modify_priority(sys_task_manager_t *task_manager, sys_tid_t tid, int priority);
/*********************************************************************************************************************
* 休眠一段时间
* task_manager：sys_task_manager_t对象
* next_task：下一个任务
* ns：休眠的时间
*********************************************************************************************************************/
void sys_task_manager_sleep(sys_task_manager_t *task_manager, sys_task_t **next_task, uint64_t ns);
/*********************************************************************************************************************
* 唤醒睡眠的任务
* task_manager：sys_task_manager_t对象
* next_task：下一个任务
* tid：要唤醒的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_wakeup(sys_task_manager_t *task_manager, sys_task_t **next_task, sys_tid_t tid);
/*********************************************************************************************************************
* 挂起一个任务
* task_manager：sys_task_manager_t对象
* next_task：下一个任务
* tid：要挂起的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_supend(sys_task_manager_t *task_manager, sys_task_t **next_task, sys_tid_t tid);
/*********************************************************************************************************************
* 恢复一个任务
* task_manager：sys_task_manager_t对象
* next_task：下一个任务
* tid：要恢复的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_resume(sys_task_manager_t *task_manager, sys_task_t **next_task, sys_tid_t tid);
/*********************************************************************************************************************
* 退出当前任务
* task_manager：sys_task_manager_t对象
* next_task：下一个任务
* arg：任务退出参数
*********************************************************************************************************************/
void sys_task_manager_exit(sys_task_manager_t *task_manager, sys_task_t **next_task, void *arg);
/*********************************************************************************************************************
* 等待子任务退出
* task_manager：sys_task_manager_t对象
* next_task：下一个任务
* retval：任务退出参数
* tid：等待任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_join(sys_task_manager_t *task_manager, sys_task_t **next_task, void **retval, sys_tid_t tid);
/*********************************************************************************************************************
* 把任务与父任务分离
* task_manager：sys_task_manager_t对象
* tid：分离任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_detach(sys_task_manager_t *task_manager, sys_tid_t tid);
/*********************************************************************************************************************
* 获取系统ticks
* task_manager：sys_task_manager_t对象
* return：系统ticks
*********************************************************************************************************************/
uint64_t sys_task_manager_get_tick_count(sys_task_manager_t *task_manager);
/*********************************************************************************************************************
* 获取任务个数
* task_manager：sys_task_manager_t对象
* return：任务个数
*********************************************************************************************************************/
int sys_task_manager_get_task_count(sys_task_manager_t *task_manager);
/*********************************************************************************************************************
* 获取当前任务TID
* task_manager：sys_task_manager_t对象
* return：当前任务TID
*********************************************************************************************************************/
sys_tid_t sys_task_manager_get_tid(sys_task_manager_t *task_manager);
/*********************************************************************************************************************
* 获取任务优先级
* task_manager：sys_task_manager_t对象
* priority：任务优先级
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_get_task_priority(sys_task_manager_t *task_manager, int *priority, sys_tid_t tid);
/*********************************************************************************************************************
* 获取任务调度类型
* task_manager：sys_task_manager_t对象
* type：任务调度类型
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_get_task_type(sys_task_manager_t *task_manager, sys_task_type_t *type, sys_tid_t tid);
/*********************************************************************************************************************
* 获取任务状态
* task_manager：sys_task_manager_t对象
* state：任务状态
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_get_task_state(sys_task_manager_t *task_manager, sys_task_state_t *state, sys_tid_t tid);
/*********************************************************************************************************************
* 获取任务名
* task_manager：sys_task_manager_t对象
* name：任务名
* size：name buffer大小
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_get_task_name(sys_task_manager_t *task_manager, char *name, int size, sys_tid_t tid);
/*********************************************************************************************************************
* 获取任务堆栈大小
* task_manager：sys_task_manager_t对象
* stack_size：任务堆栈大小
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_get_task_stack_size(sys_task_manager_t *task_manager, int *stack_size, sys_tid_t tid);
/*********************************************************************************************************************
* 返回任务是否为joinable
* task_manager：sys_task_manager_t对象
* tid：任务tid
* return：>0：joinable,0：Detach
*********************************************************************************************************************/
int sys_task_manager_joinable(sys_task_manager_t *task_manager, sys_tid_t tid);
/*********************************************************************************************************************
* 获取当前任务
* task_manager：sys_task_manager_t对象
* return：当前任务
*********************************************************************************************************************/
sys_task_t *sys_task_manager_get_running_task(sys_task_manager_t *task_manager);
/*********************************************************************************************************************
* 获取cpu占用
* task_manager：sys_task_manager_t对象
* return：cpu占用
*********************************************************************************************************************/
int sys_task_manager_get_cpu_usage(sys_task_manager_t *task_manager);
/*********************************************************************************************************************
* 发现第一个任务信息
* task_manager：sys_task_manager_t对象
* task_ptr：当前位置
* task_info：任务信息
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_find_first(sys_task_manager_t *task_manager, sys_task_ptr *task_ptr, sys_task_info_t *task_info);
/*********************************************************************************************************************
* 发现下一个任务信息
* task_manager：sys_task_manager_t对象
* task_ptr：当前位置
* task_info：任务信息
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_manager_find_next(sys_task_manager_t *task_manager, sys_task_ptr *task_ptr, sys_task_info_t *task_info);
#ifdef __cplusplus
}
#endif
#endif