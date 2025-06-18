#ifndef __SYS_TASKLET_H__
#define __SYS_TASKLET_H__
#include "sys_list.h"
#include "sys_spin_lock.h"
#include "sys_atomic.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef enum sys_tasklet_state_t
{
    SYS_TASKLET_STATE_DETACHED,
    SYS_TASKLET_STATE_SCHED,
    SYS_TASKLET_STATE_RUN,
} sys_tasklet_state_t;

typedef struct sys_tasklet_t
{
    sys_list_node_t node;
    int cpu;
    sys_atomic_t state;
    int enable;
    void (*func)(void *arg);
    void *arg;
    sys_spin_lock_t lock;
} sys_tasklet_t;
/*********************************************************************************************************************
* 静态初始化tasklet
* func：执行函数
* arg：传递到执行函数的参数
* return：tasklet对象
*********************************************************************************************************************/
#define SYS_TASKLET_INIT(_func, _arg) \
{ \
.state = SYS_ATOMIC_INIT(SYS_TASKLET_STATE_DETACHED), \
.cpu = -1, \
.enable = 1, \
.func = _func, \
.arg = _arg, \
.lock = SYS_SPIN_LOCK_INIT() \
}
/*********************************************************************************************************************
* 静态定义tasklet
* tasklet：tasklet对象
* func：执行函数
* arg：传递到执行函数的参数
*********************************************************************************************************************/
#define SYS_TASKLET_DEFINE(tasklet, func, arg) \
sys_tasklet_t tasklet = SYS_TASKLET_INIT(func, arg)
/*********************************************************************************************************************
* 初始化tasklet
* tasklet：tasklet对象
* func：执行函数
* arg：传递到执行函数的参数
*********************************************************************************************************************/
void sys_tasklet_init(sys_tasklet_t *tasklet, void (*func)(void *arg), void *arg);
/*********************************************************************************************************************
* 终止tasklet调度，如果tasklet正在运行，会等待运行结束再返回
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_tasklet_kill(sys_tasklet_t *tasklet);
/*********************************************************************************************************************
* 激活tasklet，代表tasklet可以被调度
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_tasklet_enable(sys_tasklet_t *tasklet);
/*********************************************************************************************************************
* 失能tasklet，tasklet不会再被调度，如果tasklet正在运行，会等待运行结束再返回
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_tasklet_disable(sys_tasklet_t *tasklet);
/*********************************************************************************************************************
* 失能tasklet，tasklet不会再被调度，这个函数会马上返回
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_tasklet_disable_nosync(sys_tasklet_t *tasklet);
/*********************************************************************************************************************
* 调度tasklet
* tasklet：tasklet对象
*********************************************************************************************************************/
void sys_tasklet_schedule(sys_tasklet_t *tasklet);
#ifdef __cplusplus
}
#endif
#endif