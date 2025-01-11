#include "sys_task.h"
#include "sys_task_manager.h"
static sys_task_manager_t *s_task_manager;
static int s_running = 0;
void port_start_scheduler(stack_size_t **stack_top);
void port_yield(stack_size_t **stack_top);
int port_disable_interrupts();
void port_recovery_interrupts(int state);
int sys_task_init(sys_task_manager_t *task_manager)
{
    sys_trace();
    s_task_manager = task_manager;
    int state = port_disable_interrupts();
    int ret = sys_task_manager_init(s_task_manager);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_create(sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, int priority, int stack_size)
{
    sys_trace();
    if (0 == stack_size)
    {
        stack_size = SYS_DEFAULT_TASK_STACK_SIZE;
    }
    int state = port_disable_interrupts();
    int ret = sys_task_manager_create_task(s_task_manager, tid, task_function, arg, name, SYS_TASK_TYPE_DT, priority, stack_size);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_create_rt(sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, int priority, int stack_size)
{
    sys_trace();
    if (0 == stack_size)
    {
        stack_size = SYS_DEFAULT_TASK_STACK_SIZE;
    }
    int state = port_disable_interrupts();
    int ret = sys_task_manager_create_task(s_task_manager, tid, task_function, arg, name, SYS_TASK_TYPE_RT, priority, stack_size);
    port_recovery_interrupts(state);
    return ret;
}

void sys_task_tick(uint64_t *ns)
{
    //sys_trace();
    if (s_running > 0)
    {
        int state = port_disable_interrupts();
        sys_task_t *next_task;
        sys_task_manager_tick(s_task_manager, &next_task, ns);
        port_yield(&next_task->stack_top);
        port_recovery_interrupts(state);
    }
}

int sys_task_modify_priority(sys_tid_t tid, int priority)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_task_manager_modify_priority(s_task_manager, tid, priority);
    port_recovery_interrupts(state);
    return ret;
}

void sys_task_sleep(uint64_t ms)
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_task_t *next_task;
    sys_task_manager_sleep(s_task_manager, &next_task, ms * 1000 * 1000);
    port_yield(&next_task->stack_top);
    port_recovery_interrupts(state);
}

int sys_task_wakeup(sys_tid_t tid)
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_task_t *next_task;
    int ret = sys_task_manager_wakeup(s_task_manager, &next_task, tid);
    port_yield(&next_task->stack_top);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_supend(sys_tid_t tid)
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_task_t *next_task;
    int ret = sys_task_manager_supend(s_task_manager, &next_task, tid);
    port_yield(&next_task->stack_top);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_resume(sys_tid_t tid)
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_task_t *next_task;
    int ret = sys_task_manager_resume(s_task_manager, &next_task, tid);
    port_yield(&next_task->stack_top);
    port_recovery_interrupts(state);
    return ret;
}

void sys_task_exit(void *arg)
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_task_t *next_task;
    sys_task_manager_exit(s_task_manager, &next_task, arg);
    port_yield(&next_task->stack_top);
    port_recovery_interrupts(state);
}

void sys_task_start()
{
    sys_trace();
    int state = port_disable_interrupts();
    s_running = 1;
    sys_task_t *task = sys_task_manager_get_running_task(s_task_manager);
    if (task != NULL)
    {
        port_start_scheduler(&task->stack_top);
    }
    port_recovery_interrupts(state);
    while (1)
    {
    }
}

int sys_task_join(void **retval, int tid)
{
    sys_trace();
    int ret = -1;
    int state = port_disable_interrupts();
    sys_task_t *next_task = NULL;
    ret = sys_task_manager_join(s_task_manager, &next_task, retval, tid);
    if (next_task != NULL)
    {
        port_yield(&next_task->stack_top);
    }
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_detach(sys_tid_t tid)
{
    sys_trace();
    int ret = -1;
    int state = port_disable_interrupts();
    ret = sys_task_manager_detach(s_task_manager, tid);
    port_recovery_interrupts(state);
    return ret;
}

uint64_t sys_task_get_tick_count()
{
    sys_trace();
    return sys_task_manager_get_tick_count(s_task_manager);
}

int sys_task_get_task_count()
{
    sys_trace();
    return sys_task_manager_get_task_count(s_task_manager);
}

sys_tid_t sys_task_get_tid()
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_tid_t tid = sys_task_manager_get_tid(s_task_manager);
    port_recovery_interrupts(state);
    return tid;
}

int sys_task_get_task_priority(int *priority, sys_tid_t tid)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_task_manager_get_task_priority(s_task_manager, priority, tid);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_get_task_type(sys_task_type_t *type, sys_tid_t tid)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_task_manager_get_task_type(s_task_manager, type, tid);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_get_task_state(sys_task_state_t *state, sys_tid_t tid)
{
    sys_trace();
    int interrupt_state = port_disable_interrupts();
    int ret = sys_task_manager_get_task_state(s_task_manager, state, tid);
    port_recovery_interrupts(interrupt_state);
    return ret;
}

int sys_task_get_task_name(char *name, int size, sys_tid_t tid)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_task_manager_get_task_name(s_task_manager, name, size, tid);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_get_task_stack_size(int *stack_size, sys_tid_t tid)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_task_manager_get_task_stack_size(s_task_manager, stack_size, tid);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_joinable(sys_tid_t tid)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_task_manager_joinable(s_task_manager, tid);
    port_recovery_interrupts(state);
    return ret;
}

sys_task_t *sys_task_get_running_task()
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_task_t *task = sys_task_manager_get_running_task(s_task_manager);
    port_recovery_interrupts(state);
    return task;
}

int sys_task_get_cpu_usage()
{
    sys_trace();
    return sys_task_manager_get_cpu_usage(s_task_manager);
}

int sys_task_find_first(sys_task_ptr *task_ptr, sys_task_info_t *task_info)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_task_manager_find_first(s_task_manager, task_ptr, task_info);
    port_recovery_interrupts(state);
    return ret;
}

int sys_task_find_next(sys_task_ptr *task_ptr, sys_task_info_t *task_info)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_task_manager_find_next(s_task_manager, task_ptr, task_info);
    port_recovery_interrupts(state);
    return ret;
}