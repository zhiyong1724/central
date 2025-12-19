#include "sys_task.h"
#include "sys_task_scheduler.h"
static sys_task_scheduler_t *s_task_scheduler;
int sys_task_init(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    s_task_scheduler = scheduler;
    int ret = sys_task_scheduler_init(s_task_scheduler);
    return ret;
}

int sys_task_add_cpu_thread(sys_cpu_thread_t *cpu_thread)
{
    sys_trace();
    return sys_task_scheduler_add_cpu_thread(s_task_scheduler, cpu_thread);
}

int sys_task_create(sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, int priority, int stack_size)
{
    sys_trace();
    if (0 == stack_size)
    {
        stack_size = SYS_DEFAULT_TASK_STACK_SIZE;
    }
    int ret = sys_task_scheduler_create_task(s_task_scheduler, tid, task_function, arg, name, SYS_TASK_TYPE_DT, priority, stack_size);
    return ret;
}

int sys_task_create_rt(sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, int priority, int stack_size)
{
    sys_trace();
    if (0 == stack_size)
    {
        stack_size = SYS_DEFAULT_TASK_STACK_SIZE;
    }
    int ret = sys_task_scheduler_create_task(s_task_scheduler, tid, task_function, arg, name, SYS_TASK_TYPE_RT, priority, stack_size);
    return ret;
}

int sys_task_set_priority(sys_tid_t tid, int priority)
{
    sys_trace();
    int ret = sys_task_scheduler_set_priority(s_task_scheduler, tid, priority);
    return ret;
}

sys_task_t *sys_task_scheduler_sleep_inner(sys_task_scheduler_t *scheduler, uint64_t ns, void (*wakeup_callback)(struct sys_task_control_block_t *task, void *arg), void *arg);
sys_task_t *sys_task_sleep_inner(uint64_t ms, void (*wakeup_callback)(struct sys_task_control_block_t *task, void *arg), void *arg)
{
    sys_trace();
    return sys_task_scheduler_sleep_inner(s_task_scheduler, ms * 1000 * 1000, wakeup_callback, arg);
}

void sys_task_sleep(uint64_t ms)
{
    sys_trace();
    sys_task_scheduler_sleep(s_task_scheduler, ms * 1000 * 1000);
}

int sys_task_scheduler_wakeup_inner(sys_task_scheduler_t *scheduler, sys_task_t *task);
int sys_task_wakeup_inner(sys_task_t *task)
{
    sys_trace();
    return sys_task_scheduler_wakeup_inner(s_task_scheduler, task);
}

int sys_task_wakeup(sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_wakeup(s_task_scheduler, tid);
    return ret;
}

sys_task_t *sys_task_scheduler_suspend_inner(sys_task_scheduler_t *scheduler);
sys_task_t *sys_task_suspend_inner()
{
    sys_trace();
    return sys_task_scheduler_suspend_inner(s_task_scheduler);
}

int sys_task_suspend(sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_suspend(s_task_scheduler, tid);
    return ret;
}

int sys_task_scheduler_resume_inner(sys_task_scheduler_t *scheduler, sys_task_t *task);
int sys_task_resume_inner(sys_task_t *task)
{
    sys_trace();
    return sys_task_scheduler_resume_inner(s_task_scheduler, task);
}

int sys_task_resume(sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_resume(s_task_scheduler, tid);
    return ret;
}

void sys_task_exit(void *arg)
{
    sys_trace();
    sys_task_scheduler_exit(s_task_scheduler, arg);
}

void sys_task_start()
{
    sys_trace();
    sys_task_scheduler_start(s_task_scheduler);
}

int sys_task_join(void **retval, int tid)
{
    sys_trace();
    int ret = sys_task_scheduler_join(s_task_scheduler, retval, tid);
    return ret;
}

int sys_task_detach(sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_detach(s_task_scheduler, tid);
    return ret;
}

uint64_t sys_task_get_clock()
{
    sys_trace();
    return sys_task_scheduler_get_clock(s_task_scheduler);
}

int sys_task_get_task_count()
{
    sys_trace();
    return sys_task_scheduler_get_task_count(s_task_scheduler);
}

sys_tid_t sys_task_get_tid()
{
    sys_trace();
    sys_tid_t tid = sys_task_scheduler_get_tid(s_task_scheduler);
    return tid;
}

int sys_task_get_task_priority(int *priority, sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_get_task_priority(s_task_scheduler, priority, tid);
    return ret;
}

int sys_task_get_task_type(sys_task_type_t *type, sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_get_task_type(s_task_scheduler, type, tid);
    return ret;
}

int sys_task_get_task_state(sys_task_state_t *state, sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_get_task_state(s_task_scheduler, state, tid);
    return ret;
}

int sys_task_get_task_name(char *name, int size, sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_get_task_name(s_task_scheduler, name, size, tid);
    return ret;
}

int sys_task_get_task_stack_size(int *stack_size, sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_get_task_stack_size(s_task_scheduler, stack_size, tid);
    return ret;
}

int sys_task_joinable(sys_tid_t tid)
{
    sys_trace();
    int ret = sys_task_scheduler_joinable(s_task_scheduler, tid);
    return ret;
}

int sys_task_get_cpu_thread_count()
{
    sys_trace();
    int ret = sys_task_scheduler_get_cpu_thread_count(s_task_scheduler);
    return ret;
}

int sys_task_get_cpu_info(sys_cpu_info_t *cpu_info, int *count)
{
    sys_trace();
    int ret = sys_task_scheduler_get_cpu_info(s_task_scheduler, cpu_info, count);
    return ret;
}

int sys_task_get_cpu_usage(int index)
{
    sys_trace();
    return sys_task_scheduler_get_cpu_usage(s_task_scheduler, index);
}

int sys_task_find_first(sys_task_ptr *task_ptr, sys_task_info_t *task_info)
{
    sys_trace();
    int ret = sys_task_scheduler_find_first(s_task_scheduler, task_ptr, task_info);
    return ret;
}

int sys_task_find_next(sys_task_ptr *task_ptr, sys_task_info_t *task_info)
{
    sys_trace();
    int ret = sys_task_scheduler_find_next(s_task_scheduler, task_ptr, task_info);
    return ret;
}

void sys_task_ndelay(unsigned int ns)
{
    sys_trace();
    sys_task_scheduler_ndelay(s_task_scheduler, ns);
}

sys_task_t *sys_task_scheduler_get_running_task_inner(sys_task_scheduler_t *scheduler);
sys_task_t *sys_task_get_running_task_inner()
{
    sys_trace();
    return sys_task_scheduler_get_running_task_inner(s_task_scheduler);
}

void sys_task_add_tasklet(int cpu, sys_tasklet_t *tasklet)
{
    sys_trace();
    sys_task_scheduler_add_tasklet(s_task_scheduler, cpu, tasklet);
}

void sys_task_remove_tasklet(int cpu, sys_tasklet_t *tasklet)
{
    sys_trace();
    sys_task_scheduler_remove_tasklet(s_task_scheduler, cpu, tasklet);
}