#ifndef __SYS_TIMER_H__
#define __SYS_TIMER_H__
#include "sys_task.h"
#include "sys_spin_lock.h"
#include "sys_atomic.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef enum sys_timer_state_t
{
    SYS_TIMER_STATE_DISABLE,
    SYS_TIMER_STATE_SCHED,
    SYS_TIMER_STATE_RUN,
} sys_timer_state_t;

typedef struct sys_timer_t
{
    sys_tid_t tid;
    int priority;
    uint64_t expires;
    sys_atomic_t state;
    int enable;
    void (*func)(void *arg);
    void *arg;
    sys_spin_lock_t lock;
} sys_timer_t;
/*********************************************************************************************************************
* 静态初始化定时器
* _priority：定时器服务任务优先级，为RT任务范围0-63
* _expires：定时期望，单位为ms
* _func：执行函数
* _arg：传递到执行函数的参数
* return：定时器对象
*********************************************************************************************************************/
#define SYS_TIMER_INIT(_priority, _expires, _func, _arg) \
{ \
.tid = 0, \
.priority = _priority, \
.expires = _expires, \
.state = SYS_ATOMIC_INIT(SYS_TIMER_STATE_DISABLE), \
.enable = 0, \
.func = _func, \
.arg = _arg, \
.lock = SYS_SPIN_LOCK_INIT() \
}
/*********************************************************************************************************************
* 静态定义定时器
* timer：定时器对象
* priority：定时器服务任务优先级，为RT任务范围0-63
* expires：定时时间，单位为ms
* func：执行函数
* arg：传递到执行函数的参数
* return：定时器对象
*********************************************************************************************************************/
#define SYS_TIMER_DEFINE(timer, priority, expires, func, arg) \
sys_timer_t timer = SYS_TIMER_INIT(priority, expires, func, arg)
/*********************************************************************************************************************
* 初始化定时器
* timer：定时器对象
* priority：定时器服务任务优先级，为RT任务范围0-63
* expires：定时时间，单位为ms
* func：执行函数
* arg：传递到执行函数的参数
*********************************************************************************************************************/
void sys_timer_init(sys_timer_t *timer, int priority, uint64_t expires, void (*func)(void *arg), void *arg);
/*********************************************************************************************************************
* 删除定时器，如果定时器任务正在执行会等到执行结束再返回
* timer：定时器对象
*********************************************************************************************************************/
void sys_timer_delete(sys_timer_t *timer);
/*********************************************************************************************************************
* 开启定时器
* timer：定时器对象
*********************************************************************************************************************/
void sys_timer_setup(sys_timer_t *timer);
/*********************************************************************************************************************
* 修改定时时间，只影响下一次定时
* timer：定时器对象
* expires：定时时间，单位为ms
*********************************************************************************************************************/
void sys_timer_mod(sys_timer_t *timer, uint64_t expires);
#ifdef __cplusplus
}
#endif
#endif