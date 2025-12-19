#ifndef __SYS_CPU_THREAD_H__
#define __SYS_CPU_THREAD_H__
#include "sys_vscheduler.h"
#include "sys_fifo_scheduler.h"
#include "sys_rt_scheduler.h"
#include "sys_dt_scheduler.h"
#include "sys_idle_scheduler.h"
#include "sys_spin_lock.h"
#include "sys_tasklet.h"
#include "sys_atomic.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_cpu_operations_t
{
    void (*initialize_stack)(stack_size_t **stack_top, int stack_size, void *(*task_function)(void *arg), void *arg);
    void (*start_thread)(stack_size_t **stack_top);
    void (*yield)(stack_size_t **stack_top);
} sys_cpu_operations_t;

typedef struct sys_task_control_block_t
{
    sys_vtask_control_block_t vtask_control_block;
    union
    {
        sys_fifo_task_control_block_t fifo_task_control_block;
        sys_rt_task_control_block_t rt_task_control_block;
        sys_dt_task_control_block_t dt_task_control_block;
        sys_idle_task_control_block_t idle_task_control_block;
    } real_task_control_block;
    sys_tree_node_t sleep_tree_node;
    uint64_t sleep_time;
    sys_cpu_thread_t *cpu_thread;
    sys_cpu_thread_t *desired_cpu;
    void (*wakeup_callback)(struct sys_task_control_block_t *task, void *arg);
    void *wakeup_arg;
    int wait;
    stack_size_t *stack_start;
    stack_size_t *stack_top;
    int stack_size;
    stack_size_t *task_stack_magic;
    task_function_t task_function;
} sys_task_control_block_t;

typedef void (*load_leveling_t)(void *handle, void *cpu_thread);

typedef struct sys_cpu_thread_t
{
    sys_list_node_t node;
    sys_cpu_info_t info;
    sys_cpu_operations_t operations;
    sys_vscheduler_t vscheduler;
    sys_fifo_scheduler_t fifo_scheduler;
    sys_rt_scheduler_t rt_scheduler;
    sys_dt_scheduler_t dt_scheduler;
    sys_idle_scheduler_t idle_scheduler;
    sys_tree_node_t *sleep_tree;
    sys_task_control_block_t *min_sleep_task;
    uint64_t clock;
    sys_atomic_t atomic_clock;
    uint64_t all_time;
    uint64_t idle_task_time;
    int cpu_usage;
    sys_spin_lock_t lock;
    void *handle;
    load_leveling_t load_leveling;
    sys_task_control_block_t *softirq_task;
    sys_list_node_t *tasklet_list;
} sys_cpu_thread_t;
/*********************************************************************************************************************
* sys_cpu_thread_t初始化
* sys_cpu_thread：sys_cpu_thread_t对象
* cpu_info：cpu信息
* operations：线程操作
*********************************************************************************************************************/
void sys_cpu_thread_init(sys_cpu_thread_t *sys_cpu_thread, const sys_cpu_info_t *cpu_info, const sys_cpu_operations_t *operations);
/*********************************************************************************************************************
* 增加任务
* sys_cpu_thread：sys_cpu_thread_t对象
* task_control_block：任务控制块
* task_function：任务处理函数
* arg：传给任务的参数
* task_type：SYS_TASK_TYPE_RT为实时任务，SYS_TASK_TYPE_DT为分时任务
* priority：实时任务优先级范围1-63，分时任务优先级范围为0-39
* stack_size：任务堆栈大小
* return：0：调用成功
*********************************************************************************************************************/
int sys_cpu_thread_add_task(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block, task_function_t task_function, void *arg, sys_task_type_t task_type, int priority, int stack_size);
/*********************************************************************************************************************
* 修改优先级
* sys_cpu_thread：sys_cpu_thread_t对象
* task_control_block：任务控制块
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int sys_cpu_thread_set_priority(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block, int priority);
/*********************************************************************************************************************
* 休眠一段时间
* sys_cpu_thread：sys_cpu_thread_t对象
* ns：休眠的时间
* wakeup_callback：唤醒回调
* arg：回调参数
* return：睡眠的任务
*********************************************************************************************************************/
sys_task_control_block_t *sys_cpu_thread_sleep(sys_cpu_thread_t *sys_cpu_thread, uint64_t ns, void (*wakeup_callback)(struct sys_task_control_block_t *task, void *arg), void *arg);
/*********************************************************************************************************************
* 唤醒睡眠的任务
* sys_cpu_thread：sys_cpu_thread_t对象
* task：要唤醒的任务
* return：0：调用成功
*********************************************************************************************************************/
int sys_cpu_thread_wakeup(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task);
/*********************************************************************************************************************
* 时钟滴答，不是CPU0的线程需要调用
* sys_cpu_thread：sys_cpu_thread_t对象
* ns：输入与上次tick的时间间隔
*********************************************************************************************************************/
void sys_cpu_thread_tick(sys_cpu_thread_t *sys_cpu_thread, uint64_t ns);
/*********************************************************************************************************************
* 暂停任务
* sys_cpu_thread：sys_cpu_thread_t对象
* task_control_block：任务控制块
* task_control_block：sys_task_control_block_t对象
*********************************************************************************************************************/
void sys_cpu_thread_suspend(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block);
/*********************************************************************************************************************
* 恢复任务
* sys_cpu_thread：sys_cpu_thread_t对象
* task_control_block：任务控制块
* task_control_block：sys_task_control_block_t对象
*********************************************************************************************************************/
void sys_cpu_thread_resume(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block);
/*********************************************************************************************************************
* 获取当前任务
* sys_cpu_thread：sys_cpu_thread_t对象
* return：当前任务
*********************************************************************************************************************/
sys_task_control_block_t *sys_cpu_thread_get_running_task(sys_cpu_thread_t *sys_cpu_thread);
/*********************************************************************************************************************
* 获取任务数量
* sys_cpu_thread：sys_cpu_thread_t对象
* return：任务数量
*********************************************************************************************************************/
int sys_cpu_thread_get_task_count(sys_cpu_thread_t *sys_cpu_thread);
/*********************************************************************************************************************
* 开始线程，不是CPU0的线程需要调用
* sys_cpu_thread：sys_cpu_thread_t对象
* return：这个函数永远不会返回
*********************************************************************************************************************/
void sys_cpu_thread_start(sys_cpu_thread_t *sys_cpu_thread);
/*********************************************************************************************************************
* 获取CPU占用百分比
* sys_cpu_thread：sys_cpu_thread_t对象
* return：CPU占用百分比
*********************************************************************************************************************/
int sys_cpu_thread_get_cpu_usage(sys_cpu_thread_t *sys_cpu_thread);
/*********************************************************************************************************************
* 设置负载平衡callback
* sys_cpu_thread：sys_cpu_thread_t对象
* handle：接收对象
* on_load_leveling：负载平衡回调
*********************************************************************************************************************/
void sys_cpu_thread_set_load_leveling_callback(sys_cpu_thread_t *sys_cpu_thread, void *handle, load_leveling_t load_leveling);
/*********************************************************************************************************************
* 软中断任务
* sys_cpu_thread：sys_cpu_thread_t对象
*********************************************************************************************************************/
void sys_cpu_thread_softirq_task(sys_cpu_thread_t *sys_cpu_thread);
/*********************************************************************************************************************
* 添加tasklet对象
* sys_cpu_thread：sys_cpu_thread_t对象
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_cpu_thread_add_tasklet(sys_cpu_thread_t *sys_cpu_thread, sys_tasklet_t *tasklet);
/*********************************************************************************************************************
* 移除tasklet对象
* sys_cpu_thread：sys_cpu_thread_t对象
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_cpu_thread_remove_tasklet(sys_cpu_thread_t *sys_cpu_thread, sys_tasklet_t *tasklet);
/*********************************************************************************************************************
* 获取系统clock
* sys_cpu_thread：sys_cpu_thread_t对象
* return：系统clock
*********************************************************************************************************************/
uint64_t sys_cpu_thread_get_clock(sys_cpu_thread_t *sys_cpu_thread);
#ifdef __cplusplus
}
#endif
#endif