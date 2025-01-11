#include "sys_task_manager.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "sys_error.h"
#define SYS_TASK_MAX_SIZE (int)(((unsigned int)~0) >> 1)
static int check_stack(sys_task_manager_t *task_manager)
{
    sys_trace();
    int ret = -1;
    sys_task_t *task = sys_task_manager_get_running_task(task_manager);
    if ((stack_size_t)SYS_TASK_STACK_MAGIC == *task->task_stack_magic)
    {
#if (SYS_TASK_STACK_GROWTH > 0)
        if ((unsigned char *)task->stack_top > (unsigned char *)task->stack_start + sizeof(stack_size_t))
        {
            ret = 0;
        }
#else
        if ((unsigned char *)task->stack_top < (unsigned char *)task->stack_start + task->stack_size - sizeof(stack_size_t))
        {
            ret = 0;
        }
#endif
    }
    if (ret < 0)
    {
        printf("Task %s stack overflow\n", task->name);
        sys_assert(0);
    }
    return 0;
}

static int add_schedulers(sys_task_manager_t *task_manager)
{
    sys_trace();
    int ret = -1;
    sys_scheduler_interfaces_t scheduler_interfaces;
    scheduler_interfaces.add_task = (add_task_t)sys_rt_scheduler_add_task;
    scheduler_interfaces.remove_task = (remove_task_t)sys_rt_scheduler_remove_task;
    scheduler_interfaces.tick = (tick_t)sys_rt_scheduler_tick;
    scheduler_interfaces.yield = (yield_t)sys_rt_scheduler_yield;
    scheduler_interfaces.modify_priority = (modify_priority_t)sys_rt_scheduler_modify_priority;
    scheduler_interfaces.get_running_task = (get_running_task_t)sys_rt_scheduler_get_running_task;
    ret = sys_vscheduler_add_scheduler(&task_manager->vscheduler, &task_manager->rt_scheduler, &scheduler_interfaces);
    if (ret < 0)
    {
        return ret;
    }
    scheduler_interfaces.add_task = (add_task_t)sys_dt_scheduler_add_task;
    scheduler_interfaces.remove_task = (remove_task_t)sys_dt_scheduler_remove_task;
    scheduler_interfaces.tick = (tick_t)sys_dt_scheduler_tick;
    scheduler_interfaces.yield = (yield_t)sys_dt_scheduler_yield;
    scheduler_interfaces.modify_priority = (modify_priority_t)sys_dt_scheduler_modify_priority;
    scheduler_interfaces.get_running_task = (get_running_task_t)sys_dt_scheduler_get_running_task;
    ret = sys_vscheduler_add_scheduler(&task_manager->vscheduler, &task_manager->dt_scheduler, &scheduler_interfaces);
    if (ret < 0)
    {
        return ret;
    }
    scheduler_interfaces.add_task = (add_task_t)sys_idle_scheduler_add_task;
    scheduler_interfaces.remove_task = (remove_task_t)sys_idle_scheduler_remove_task;
    scheduler_interfaces.tick = (tick_t)sys_idle_scheduler_tick;
    scheduler_interfaces.yield = (yield_t)sys_idle_scheduler_yield;
    scheduler_interfaces.modify_priority = (modify_priority_t)sys_idle_scheduler_modify_priority;
    scheduler_interfaces.get_running_task = (get_running_task_t)sys_idle_scheduler_get_running_task;
    ret = sys_vscheduler_add_scheduler(&task_manager->vscheduler, &task_manager->idle_scheduler, &scheduler_interfaces);
    if (ret < 0)
    {
        return ret;
    }
    return 0;
}

static void delete_task(sys_task_manager_t *task_manager, sys_task_t *task)
{
    sys_trace();
    sys_id_free(&task_manager->id_manager, task->tid);
    sys_free(task->stack_start);
    sys_free(task);
    void **pp = (void **)sys_vector_at(&task_manager->task_list, task->tid);
    *pp = NULL;
}

int port_recovery_interrupts(int state);
int port_disable_interrupts();

static void clear_delete_task_list(sys_task_manager_t *task_manager)
{
    sys_trace();
    int state = port_disable_interrupts();
    for (sys_task_t *task = task_manager->delete_task_list; task != NULL; task = task_manager->delete_task_list)
    {
        sys_remove_from_list((sys_list_node_t **)&task_manager->delete_task_list, &task->node);
        delete_task(task_manager, task);
    }
    port_recovery_interrupts(state);
}

static void *init_task(void *arg)
{
    sys_trace();
    sys_task_manager_t *task_manager = (sys_task_manager_t *)arg;
    for (;;)
    {
        clear_delete_task_list(task_manager);
        sys_task_sleep(200);
    }
    return NULL;
}

static void *idle_task(void *arg)
{
    sys_trace();
    for (;;)
    {
    }
    return NULL;
}

static sys_task_t *get_task_by_tid(sys_task_manager_t *task_manager, sys_tid_t tid)
{
    sys_trace();
    int size = sys_vector_size(&task_manager->task_list);
    if (tid < size)
    {
        sys_task_t **ptask = (sys_task_t **)sys_vector_at(&task_manager->task_list, tid);
        if (*ptask != NULL)
        {
            return *ptask;
        }
    }
    return NULL;
}

int sys_task_manager_init(sys_task_manager_t *task_manager)
{
    sys_trace();
    int ret = -1;
    ret = sys_vscheduler_init(&task_manager->vscheduler);
    if (ret < 0)
    {
        return ret;
    }
    ret = sys_rt_scheduler_init(&task_manager->rt_scheduler);
    if (ret < 0)
    {
        return ret;
    }
    ret = sys_dt_scheduler_init(&task_manager->dt_scheduler);
    if (ret < 0)
    {
        return ret;
    }
    ret = sys_idle_scheduler_init(&task_manager->idle_scheduler);
    if (ret < 0)
    {
        return ret;
    }
    ret = add_schedulers(task_manager);
    if (ret < 0)
    {
        return ret;
    }
    ret = sys_id_manager_init(&task_manager->id_manager);
    if (ret < 0)
    {
        return ret;
    }
    ret = sys_vector_init(&task_manager->task_list, sizeof(void *));
    if (ret < 0)
    {
        return ret;
    }
    task_manager->task_count = 0;
    task_manager->delete_task_list = NULL;
    task_manager->tick_count = 0;
    task_manager->cpu_usage = 0;
    task_manager->idle_task_tick_count = 0;
    task_manager->idle_tick_count = 0;
    sys_tid_t tid;
    ret = sys_task_manager_create_task(task_manager, &tid, idle_task, task_manager, "idle", SYS_TASK_TYPE_IDLE, 0, SYS_DEFAULT_TASK_STACK_SIZE);
    if (ret < 0)
    {
        return ret;
    }
    ret = sys_task_manager_create_task(task_manager, &tid, init_task, task_manager, "init", SYS_TASK_TYPE_DT, 0, SYS_DEFAULT_TASK_STACK_SIZE);
    if (ret < 0)
    {
        return ret;
    }
    task_manager->init_task = sys_task_manager_get_running_task(task_manager);
    return 0;
}

extern int port_initialize_stack(stack_size_t **stack_top, int stack_size, stack_size_t *task_stack_magic, task_function_t task_function, void *arg);
int sys_task_manager_create_task(sys_task_manager_t *task_manager, sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, sys_task_type_t task_type, int priority, int stack_size)
{
    sys_trace();
    int ret = 0;
    if (task_manager->task_count >= SYS_TASK_MAX_SIZE)
    {
        sys_error("No child processes.");
        ret = SYS_ERROR_CHILD;
        goto exception;
    }
    sys_task_t *task = (sys_task_t *)sys_malloc(sizeof(sys_task_t));
    if (NULL == task)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    task->stack_start = (stack_size_t *)sys_malloc(stack_size);
    if (NULL == task->stack_start)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    task->task_function = task_function;
    task->arg = NULL;
    sys_strcpy(task->name, name, SYS_TASK_MAX_NAME_LEN);
    task->tid = sys_id_alloc(&task_manager->id_manager);
    if (task->tid < 0)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    task->stack_size = stack_size;
    task->children = NULL;
    task->children_count = 0;
    task->wait_tid_t = 0;
    task->parent = sys_task_manager_get_running_task(task_manager);
    if (task->parent != NULL)
    {
        sys_insert_to_back((sys_list_node_t **)&task->parent->children, &task->node);
        task->parent->children_count++;
    }
#if (SYS_TASK_STACK_GROWTH > 0)
    task->task_stack_magic = task->stack_start;
    *task->task_stack_magic = (stack_size_t)SYS_TASK_STACK_MAGIC;
    task->stack_top = (stack_size_t *)((unsigned char *)task->stack_start + stack_size);
#else
    task->task_stack_magic = (stack_size_t *)((unsigned char *)task->stack_start + stack_size - sizeof(stack_size_t));
    *task->task_stack_magic = (stack_size_t)SYS_TASK_STACK_MAGIC;
    task->stack_top = task->stack_start;
#endif
    port_initialize_stack(&task->stack_top, task->stack_size, task->task_stack_magic, task->task_function, arg);
    sys_task_control_block_init(&task_manager->vscheduler, &task->task_control_block);
    task->task_control_block.scheduler_id = task_type;
    switch (task->task_control_block.scheduler_id)
    {
    case SYS_TASK_TYPE_RT:
        sys_rt_task_control_block_init(&task_manager->rt_scheduler, &task->real_task_control_block.rt_task_control_block, priority);
        break;
    case SYS_TASK_TYPE_DT:
        sys_dt_task_control_block_init(&task_manager->dt_scheduler, &task->real_task_control_block.dt_task_control_block, priority);
        break;
    case SYS_TASK_TYPE_IDLE:
        sys_idle_task_control_block_init(&task_manager->idle_scheduler, &task->real_task_control_block.idle_task_control_block, priority);
        break;
    default:
        break;
    }
    ret = sys_vscheduler_add_task(&task_manager->vscheduler, &task->task_control_block);
    if (ret < 0)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    task_manager->task_count++;
    int size = sys_vector_size(&task_manager->task_list);
    if (task->tid >= size)
    {
        void *p = NULL;
        ret = sys_vector_push_back(&task_manager->task_list, &p);
        if (ret < 0)
        {
            sys_error("Out of memory.");
            ret = SYS_ERROR_NOMEM;
            goto exception;
        }
    }
    void **t = (void **)sys_vector_at(&task_manager->task_list, task->tid);
    *t = task;
    *tid = task->tid;
    goto finally;
exception:
    if (task != NULL && task->stack_start != NULL)
    {
        sys_free(task->stack_start);
    }
    if (task != NULL)
    {
        sys_free(task);
    }
finally:
    return ret;
}

void sys_task_manager_tick(sys_task_manager_t *task_manager, sys_task_t **next_task, uint64_t *ns)
{
    //sys_trace();
    int ret = check_stack(task_manager);
    if (0 == ret)
    {
        task_manager->tick_count += *ns;
        task_manager->idle_tick_count += *ns;
        sys_task_t *running_task = sys_task_manager_get_running_task(task_manager);
        if (0 == running_task->tid)
        {
            task_manager->idle_task_tick_count += *ns;
        }
        if (task_manager->idle_tick_count >= 1000000000ll)
        {
            task_manager->cpu_usage = 100 - task_manager->idle_task_tick_count * 100 / task_manager->idle_tick_count;
            task_manager->idle_task_tick_count = 0;
            task_manager->idle_tick_count = 0;
        }
        *next_task = (sys_task_t *)sys_vscheduler_tick(&task_manager->vscheduler, SYS_MAX_SCHEDULER_COUNT, ns);
        *next_task = (sys_task_t *)((unsigned char *)*next_task - sizeof(sys_list_node_t));
    }
}

int sys_task_manager_modify_priority(sys_task_manager_t *task_manager, sys_tid_t tid, int priority)
{
    sys_trace();
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    sys_task_t *task = get_task_by_tid(task_manager, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    return sys_vscheduler_modify_priority(&task_manager->vscheduler, &task->task_control_block, priority);
}

void sys_task_manager_sleep(sys_task_manager_t *task_manager, sys_task_t **next_task, uint64_t ns)
{
    sys_trace();
    int ret  = check_stack(task_manager);
    if (0 == ret)
    {
        *next_task = (sys_task_t *)sys_vscheduler_sleep(&task_manager->vscheduler, ns);
        *next_task = (sys_task_t *)((unsigned char *)*next_task - sizeof(sys_list_node_t));
    }
}

int sys_task_manager_wakeup(sys_task_manager_t *task_manager, sys_task_t **next_task, sys_tid_t tid)
{
    sys_trace();
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int ret = check_stack(task_manager);
    if (0 == ret)
    {
        sys_task_t *task = get_task_by_tid(task_manager, tid);
        if (NULL == task)
        {
            sys_error("Invalid argument.");
            return SYS_ERROR_INVAL;
        }
        *next_task = (sys_task_t *)sys_vscheduler_wakeup(&task_manager->vscheduler, &task->task_control_block);
        *next_task = (sys_task_t *)((unsigned char *)*next_task - sizeof(sys_list_node_t));
    }
    return 0;
}

int sys_task_manager_supend(sys_task_manager_t *task_manager, sys_task_t **next_task, sys_tid_t tid)
{
    sys_trace();
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int ret = check_stack(task_manager);
    if (0 == ret)
    {
        sys_task_t *task = get_task_by_tid(task_manager, tid);
        if (NULL == task)
        {
            sys_error("Invalid argument.");
            return SYS_ERROR_INVAL;
        }
        *next_task = (sys_task_t *)sys_vscheduler_supend(&task_manager->vscheduler, &task->task_control_block);
        *next_task = (sys_task_t *)((unsigned char *)*next_task - sizeof(sys_list_node_t));
    }
    return 0;
}

int sys_task_manager_resume(sys_task_manager_t *task_manager, sys_task_t **next_task, sys_tid_t tid)
{
    sys_trace();
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int ret = check_stack(task_manager);
    if (0 == ret)
    {
        sys_task_t *task = get_task_by_tid(task_manager, tid);
        if (NULL == task)
        {
            sys_error("Invalid argument.");
            return SYS_ERROR_INVAL;
        }
        *next_task = (sys_task_t *)sys_vscheduler_resume(&task_manager->vscheduler, &task->task_control_block);
        *next_task = (sys_task_t *)((unsigned char *)*next_task - sizeof(sys_list_node_t));
    }
    return 0;
}

static void move_children(sys_task_manager_t *task_manager, sys_task_t *task)
{
    sys_trace();
    while (task->children_count > 0)
    {
        sys_task_t *child = task->children;
        sys_remove_from_list((sys_list_node_t **)&task->children, &child->node);
        if (SYS_TASK_STATE_DELETED == child->task_control_block.task_state)
        {
            delete_task(task_manager, child);
        }
        else
        {
            child->parent = task_manager->init_task;
            sys_insert_to_back((sys_list_node_t **)&task_manager->init_task->children, &child->node);
            task_manager->init_task->children_count++;
        }
        task->children_count--;
    }
}

void sys_task_manager_exit(sys_task_manager_t *task_manager, sys_task_t **next_task, void *arg)
{
    sys_trace();
    int ret = check_stack(task_manager);
    if (0 == ret)
    {
        sys_task_t *task = sys_task_manager_get_running_task(task_manager);
        *next_task = (sys_task_t *)((unsigned char *)sys_vscheduler_exit(&task_manager->vscheduler) - sizeof(sys_list_node_t));
        void **pp = (void **)sys_vector_at(&task_manager->task_list, task->tid);
        *pp = NULL;
        move_children(task_manager, task);
        if (task->parent == task_manager->init_task)
        {
            sys_remove_from_list((sys_list_node_t **)&task_manager->init_task->children, &task->node);
            task_manager->init_task->children_count--;
            sys_insert_to_back((sys_list_node_t **)&task_manager->delete_task_list, &task->node);
        }
        else
        {
            if (SYS_TASK_STATE_BLOCKED == task->parent->task_control_block.task_state && task->parent->wait_tid_t == task->tid)
            {
                task->parent->wait_tid_t = 0;
                *(void **)task->parent->arg = arg;
                task->parent->arg = NULL;
                task->parent->task_control_block.task_state = SYS_TASK_STATE_SUSPENDED;
                *next_task = (sys_task_t *)((unsigned char *)sys_vscheduler_resume(&task_manager->vscheduler, &task->parent->task_control_block) - sizeof(sys_list_node_t));

                sys_remove_from_list((sys_list_node_t **)&task->parent->children, &task->node);
                task->parent->children_count--;
                sys_insert_to_back((sys_list_node_t **)&task_manager->delete_task_list, &task->node);
            }
            else
            {
                task->arg = arg;
            }
        }
        task_manager->task_count--;
    }
}

int sys_task_manager_join(sys_task_manager_t *task_manager, sys_task_t **next_task, void **retval, sys_tid_t tid)
{
    sys_trace();
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int ret = check_stack(task_manager);
    if (0 == ret)
    {
        sys_task_t *task = get_task_by_tid(task_manager, tid);
        if (NULL == task)
        {
            sys_error("Invalid argument.");
            return SYS_ERROR_INVAL;
        }
        sys_task_t *running_task = sys_task_manager_get_running_task(task_manager);
        if (task->parent != running_task)
        {
            sys_error("Invalid argument.");
            return SYS_ERROR_INVAL;
        }
        if (SYS_TASK_STATE_DELETED == task->task_control_block.task_state)
        {
            *retval = task->arg;
            task->arg = NULL;
            sys_remove_from_list((sys_list_node_t **)&running_task->children, &task->node);
            running_task->children_count--;
            delete_task(task_manager, task);
            *next_task = running_task;
        }
        else
        {
            running_task->arg = (void *)retval;
            running_task->wait_tid_t = tid;
            *next_task = (sys_task_t *)sys_vscheduler_supend(&task_manager->vscheduler, &running_task->task_control_block);
            running_task->task_control_block.task_state = SYS_TASK_STATE_BLOCKED;
            *next_task = (sys_task_t *)((unsigned char *)*next_task - sizeof(sys_list_node_t));
        }
    }
    return 0;
}

int sys_task_manager_detach(sys_task_manager_t *task_manager, sys_tid_t tid)
{
    sys_trace();
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    sys_task_t *task = get_task_by_tid(task_manager, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    if (task->parent != task_manager->init_task && task->parent->task_control_block.task_state != SYS_TASK_STATE_BLOCKED && task->parent->wait_tid_t != task->tid)
    {
        sys_remove_from_list((sys_list_node_t **)&task->parent->children, &task->node);
        task->parent->children_count--;

        task->parent = task_manager->init_task;
        sys_insert_to_back((sys_list_node_t **)&task_manager->init_task->children, &task->node);
        task_manager->init_task->children_count++;
    }
    else
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    return 0;
}

uint64_t sys_task_manager_get_tick_count(sys_task_manager_t *task_manager)
{
    sys_trace();
    return task_manager->tick_count;
}

int sys_task_manager_get_task_count(sys_task_manager_t *task_manager)
{
    sys_trace();
    return task_manager->task_count;
}

sys_tid_t sys_task_manager_get_tid(sys_task_manager_t *task_manager)
{
    sys_trace();
    sys_task_t *running_task = sys_task_manager_get_running_task(task_manager);
    return running_task->tid;
}

int sys_task_manager_get_task_priority(sys_task_manager_t *task_manager, int *priority, sys_tid_t tid)
{
    sys_trace();
    sys_task_t *task = get_task_by_tid(task_manager, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    switch (task->task_control_block.scheduler_id)
    {
    case SYS_TASK_TYPE_RT:
        *priority = task->real_task_control_block.rt_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_DT:
        *priority = task->real_task_control_block.dt_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_IDLE:
        *priority = task->real_task_control_block.idle_task_control_block.priority;
        break;
    default:
        break;
    }
    return 0;
}

int sys_task_manager_get_task_type(sys_task_manager_t *task_manager, sys_task_type_t *type, sys_tid_t tid)
{
    sys_trace();
    sys_task_t *task = get_task_by_tid(task_manager, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    *type = (sys_task_type_t)task->task_control_block.scheduler_id;
    return 0;
}

int sys_task_manager_get_task_state(sys_task_manager_t *task_manager, sys_task_state_t *state, sys_tid_t tid)
{
    sys_trace();
    sys_task_t *task = get_task_by_tid(task_manager, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    *state = task->task_control_block.task_state;
    return 0;
}

int sys_task_manager_get_task_name(sys_task_manager_t *task_manager, char *name, int size, sys_tid_t tid)
{
    sys_trace();
    sys_task_t *task = get_task_by_tid(task_manager, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    size = size < SYS_TASK_MAX_NAME_LEN ? size : SYS_TASK_MAX_NAME_LEN;
    sys_strcpy(name, task->name, size);
    return 0;
}

int sys_task_manager_get_task_stack_size(sys_task_manager_t *task_manager, int *stack_size, sys_tid_t tid)
{
    sys_trace();
    sys_task_t *task = get_task_by_tid(task_manager, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    *stack_size = task->stack_size;
    return 0;
}

int sys_task_manager_joinable(sys_task_manager_t *task_manager, sys_tid_t tid)
{
    sys_trace();
    sys_task_t *task = get_task_by_tid(task_manager, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    if (task->parent == task_manager->init_task)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    return 0;
}

sys_task_t *sys_task_manager_get_running_task(sys_task_manager_t *task_manager)
{
    sys_trace();
    sys_task_t *task = (sys_task_t *)sys_vscheduler_get_running_task(&task_manager->vscheduler);
    if (task != NULL)
    {
        task = (sys_task_t *)((unsigned char *)task - sizeof(sys_list_node_t));
        return task;
    }
    return NULL;
}

int sys_task_manager_get_cpu_usage(sys_task_manager_t *task_manager)
{
    sys_trace();
    return task_manager->cpu_usage;
}

int sys_task_manager_find_first(sys_task_manager_t *task_manager, sys_task_ptr *task_ptr, sys_task_info_t *task_info)
{
    sys_trace();
    *task_ptr = 0;
    return sys_task_manager_find_next(task_manager, task_ptr, task_info);
}

int sys_task_manager_find_next(sys_task_manager_t *task_manager, sys_task_ptr *task_ptr, sys_task_info_t *task_info)
{
    sys_trace();
    sys_task_t *task = get_task_by_tid(task_manager, *task_ptr);
    if (NULL == task)
    {
        return SYS_ERROR_INVAL;
    }
    (*task_ptr)++;
    task_info->tid = task->tid;
    if (task->parent != NULL)
    {
        task_info->ptid = task->parent->tid;
    }
    else
    {
        task_info->ptid = 0;
    }
    sys_strcpy(task_info->name, task->name, SYS_TASK_MAX_NAME_LEN);
    task_info->stack = task->stack_start;
    task_info->stack_size = task->stack_size;
    task_info->task_state = task->task_control_block.task_state;
    task_info->task_type = task->task_control_block.scheduler_id;
    switch (task->task_control_block.scheduler_id)
    {
    case SYS_TASK_TYPE_RT:
        task_info->priority = task->real_task_control_block.rt_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_DT:
        task_info->priority = task->real_task_control_block.dt_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_IDLE:
        task_info->priority = task->real_task_control_block.idle_task_control_block.priority;
        break;
    default:
        break;
    }
    return 0;
}