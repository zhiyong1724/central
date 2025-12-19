#ifndef __SYS_TASK_SCHEDULER_H__
#define __SYS_TASK_SCHEDULER_H__
#include "sys_cpu_thread.h"
#include "sys_id_manager.h"
#include "sys_vector.h"
#include "sys_spin_lock.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_task_t
{
    sys_list_node_t list_node;
    union
    {
        sys_list_node_t list_node;
        sys_tree_node_t tree_node;
    } exnode;
    sys_tree_node_t rt_task_node;
    sys_task_control_block_t task_control_block;
    char name[SYS_TASK_MAX_NAME_LEN];
    sys_tid_t tid;
    struct sys_task_t *parent;
    struct sys_list_node_t *children;
    int children_count;
    void *ret_arg;
    sys_tid_t wait_tid;
} sys_task_t;

typedef struct sys_task_scheduler_t
{
    sys_list_node_t *cpu_threads;
    int cpu_thread_count;
    sys_id_manager_t id_manager;
    sys_vector_t task_list;
    int task_count;
    sys_spin_lock_t lock;
    sys_task_t *init_task;
    sys_list_node_t *dead_task_list;
    sys_tree_node_t *rt_task_tree;
    sys_cpu_thread_t *high_load_cpu;
} sys_task_scheduler_t;
/*********************************************************************************************************************
* sys_task_scheduler_t初始化
* scheduler：sys_task_scheduler_t对象
* return：初始化成功
*********************************************************************************************************************/
int sys_task_scheduler_init(sys_task_scheduler_t *scheduler);
/*********************************************************************************************************************
* 添加CPU线程
* scheduler：sys_task_scheduler_t对象
* cpu_thread：cpu线程对象，这个对象由外部保存
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_add_cpu_thread(sys_task_scheduler_t *scheduler, sys_cpu_thread_t *cpu_thread);
/*********************************************************************************************************************
* 创建任务
* scheduler：sys_task_scheduler_t对象
* tid：任务tid
* task_function：任务处理函数
* arg：传给任务的参数
* name：任务名称，最大长度为SYS_TASK_MAX_NAME_LEN
* task_type：SYS_TASK_TYPE_RT为实时任务，SYS_TASK_TYPE_DT为分时任务
* priority：实时任务优先级范围0-63，分时任务优先级范围为0-39
* stack_size：任务堆栈大小
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_create_task(sys_task_scheduler_t *scheduler, sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, sys_task_type_t task_type, int priority, int stack_size);
/*********************************************************************************************************************
* 修改优先级
* scheduler：sys_task_scheduler_t对象
* tid：任务tid
* priority：优先级
* return：0:调用成功
*********************************************************************************************************************/
int sys_task_scheduler_set_priority(sys_task_scheduler_t *scheduler, sys_tid_t tid, int priority);
/*********************************************************************************************************************
* 休眠一段时间
* scheduler：sys_task_scheduler_t对象
* ns：休眠的时间
*********************************************************************************************************************/
void sys_task_scheduler_sleep(sys_task_scheduler_t *scheduler, uint64_t ns);
/*********************************************************************************************************************
* 唤醒睡眠的任务
* scheduler：sys_task_scheduler_t对象
* tid：要唤醒的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_wakeup(sys_task_scheduler_t *scheduler, sys_tid_t tid);
/*********************************************************************************************************************
* 挂起一个任务
* scheduler：sys_task_scheduler_t对象
* tid：要挂起的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_suspend(sys_task_scheduler_t *scheduler, sys_tid_t tid);
/*********************************************************************************************************************
* 恢复一个任务
* scheduler：sys_task_scheduler_t对象
* tid：要恢复的任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_resume(sys_task_scheduler_t *scheduler, sys_tid_t tid);
/*********************************************************************************************************************
* 开始任务调度
* scheduler：sys_task_scheduler_t对象
* return：这个函数不会返回
*********************************************************************************************************************/
void sys_task_scheduler_start(sys_task_scheduler_t *scheduler);
/*********************************************************************************************************************
* 退出当前任务
* scheduler：sys_task_scheduler_t对象
* arg：任务退出参数
*********************************************************************************************************************/
void sys_task_scheduler_exit(sys_task_scheduler_t *scheduler, void *arg);
/*********************************************************************************************************************
* 等待子任务退出
* scheduler：sys_task_scheduler_t对象
* retval：任务退出参数
* tid：等待任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_join(sys_task_scheduler_t *scheduler, void **retval, sys_tid_t tid);
/*********************************************************************************************************************
* 把任务与父任务分离
* scheduler：sys_task_scheduler_t对象
* tid：分离任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_detach(sys_task_scheduler_t *scheduler, sys_tid_t tid);
/*********************************************************************************************************************
* 获取系统clock
* scheduler：sys_task_scheduler_t对象
* return：系统clock
*********************************************************************************************************************/
uint64_t sys_task_scheduler_get_clock(sys_task_scheduler_t *scheduler);
/*********************************************************************************************************************
* 获取任务个数
* scheduler：sys_task_scheduler_t对象
* return：任务个数
*********************************************************************************************************************/
int sys_task_scheduler_get_task_count(sys_task_scheduler_t *scheduler);
/*********************************************************************************************************************
* 获取当前任务TID
* scheduler：sys_task_scheduler_t对象
* return：当前任务TID
*********************************************************************************************************************/
sys_tid_t sys_task_scheduler_get_tid(sys_task_scheduler_t *scheduler);
/*********************************************************************************************************************
* 获取任务优先级
* scheduler：sys_task_scheduler_t对象
* priority：任务优先级
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_get_task_priority(sys_task_scheduler_t *scheduler, int *priority, sys_tid_t tid);
/*********************************************************************************************************************
* 获取任务调度类型
* scheduler：sys_task_scheduler_t对象
* type：任务调度类型
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_get_task_type(sys_task_scheduler_t *scheduler, sys_task_type_t *type, sys_tid_t tid);
/*********************************************************************************************************************
* 获取任务状态
* scheduler：sys_task_scheduler_t对象
* state：任务状态
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_get_task_state(sys_task_scheduler_t *scheduler, sys_task_state_t *state, sys_tid_t tid);
/*********************************************************************************************************************
* 获取任务名
* scheduler：sys_task_scheduler_t对象
* name：任务名
* size：name buffer大小
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_get_task_name(sys_task_scheduler_t *scheduler, char *name, int size, sys_tid_t tid);
/*********************************************************************************************************************
* 获取任务堆栈大小
* scheduler：sys_task_scheduler_t对象
* stack_size：任务堆栈大小
* tid：任务tid
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_get_task_stack_size(sys_task_scheduler_t *scheduler, int *stack_size, sys_tid_t tid);
/*********************************************************************************************************************
* 返回任务是否为joinable
* scheduler：sys_task_scheduler_t对象
* tid：任务tid
* return：>0：joinable,0：Detach
*********************************************************************************************************************/
int sys_task_scheduler_joinable(sys_task_scheduler_t *scheduler, sys_tid_t tid);
/*********************************************************************************************************************
* 获取CPU线程数量
* scheduler：sys_task_scheduler_t对象
* return：CPU线程数量
*********************************************************************************************************************/
int sys_task_scheduler_get_cpu_thread_count(sys_task_scheduler_t *scheduler);
/*********************************************************************************************************************
* 获取CPU线程信息
* scheduler：sys_task_scheduler_t对象
* cpu_info：cpu信息数组
* count：输入cpu信息数组大小，输出cpu信息数量
* return：<0：获取失败；=0：全部获取；>0：部分获取
*********************************************************************************************************************/
int sys_task_scheduler_get_cpu_info(sys_task_scheduler_t *scheduler, sys_cpu_info_t *cpu_info, int *count);
/*********************************************************************************************************************
* 获取cpu占用
* scheduler：sys_task_scheduler_t对象
* index：cpu编号
* return：cpu占用
*********************************************************************************************************************/
int sys_task_scheduler_get_cpu_usage(sys_task_scheduler_t *scheduler, int index);
/*********************************************************************************************************************
* 发现第一个任务信息
* scheduler：sys_task_scheduler_t对象
* task_ptr：当前位置
* task_info：任务信息
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_find_first(sys_task_scheduler_t *scheduler, sys_task_ptr *task_ptr, sys_task_info_t *task_info);
/*********************************************************************************************************************
* 发现下一个任务信息
* scheduler：sys_task_scheduler_t对象
* task_ptr：当前位置
* task_info：任务信息
* return：0：调用成功
*********************************************************************************************************************/
int sys_task_scheduler_find_next(sys_task_scheduler_t *scheduler, sys_task_ptr *task_ptr, sys_task_info_t *task_info);
/*********************************************************************************************************************
* 添加tasklet对象
* scheduler：sys_task_scheduler_t对象
* cpu：cpu
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_task_scheduler_add_tasklet(sys_task_scheduler_t *scheduler, int cpu, sys_tasklet_t *tasklet);
/*********************************************************************************************************************
* 移除tasklet对象
* scheduler：sys_task_scheduler_t对象
* cpu：cpu
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_task_scheduler_remove_tasklet(sys_task_scheduler_t *scheduler, int cpu, sys_tasklet_t *tasklet);
/*********************************************************************************************************************
* 延时n纳秒，精度取决于系统tick的大小
* scheduler：sys_task_scheduler_t对象
* ns：n纳秒
*********************************************************************************************************************/
void sys_task_scheduler_ndelay(sys_task_scheduler_t *scheduler, unsigned int ns);
#ifdef __cplusplus
}
#endif
#endif