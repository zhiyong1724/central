#include "sys_port.h"
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include "sys_external_cfg.h"
#include <unistd.h>
#include "sys_task.h"
#include "sys_cpu_thread.h"
#include <errno.h>
#include <execinfo.h>
/* Get thread name visible in the kernel and its interfaces.  */
extern int pthread_getname_np (pthread_t __target_thread, char *__buf,
			       size_t __buflen)
     __THROW __nonnull ((2));

/* Set thread name visible in the kernel and its interfaces.  */
extern int pthread_setname_np (pthread_t __target_thread, const char *__name)
     __THROW __nonnull ((2));
char heap_start[SYS_HEAP_SIZE];
typedef struct thread_t
{
    pthread_t thread;
    pthread_cond_t cond;
    task_function_t task_function;
    void *arg;
    int exit;
} thread_t;

struct cpu_t
{
    sys_cpu_thread_t cpu_thread;
    stack_size_t **cur_task;
    stack_size_t **next_task;
    pthread_mutex_t mutex;
    int irq;
};

static sigset_t s_signals;
static struct cpu_t cpu0;
static struct cpu_t cpu1;

static void event_wait(thread_t *thread, pthread_mutex_t *mutex)
{
    while (pthread_cond_wait(&thread->cond, mutex) != 0)
    {
    }
}

static void event_signal(thread_t *thread)
{
    pthread_cond_signal(&thread->cond);
};

int get_cpu()
{
    char buff[16] = {'\0'};
    pthread_getname_np(pthread_self(), buff, 16);
    if ('1' == buff[0])
        return 1;
    else
        return 0;
}

static void *task_enter(void *arg)
{
    thread_t *thread = (thread_t *)arg;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    event_wait(thread, &mutex);
    pthread_mutex_unlock(&mutex);
    pthread_sigmask(SIG_UNBLOCK, &s_signals, NULL);
    void *ret = thread->task_function(thread->arg);
    thread->exit = 1;
    sys_task_exit(ret);
    return NULL;
}

int local_irq_save()
{
    int state;
    pthread_sigmask(SIG_BLOCK, &s_signals, NULL);
    if (get_cpu() > 0)
    {
        state = cpu1.irq;
        cpu1.irq = 0;
    }
    else
    {
        state = cpu0.irq;
        cpu0.irq = 0;
    }
    return state;
}

void local_irq_restore(int state)
{
    if (state > 0)
    {
        if (get_cpu() > 0)
        {
            cpu1.irq = 1;
        }
        else
        {
            cpu0.irq = 1;
        }
        pthread_sigmask(SIG_UNBLOCK, &s_signals, NULL);
    }
}

static void cpu0_timer_tick()
{
    sys_cpu_thread_tick(&cpu0.cpu_thread, 10 * 1000 * 1000);
}

static void cpu0_task_yield()
{
    pthread_mutex_lock(&cpu0.mutex);
    thread_t *old_thread = (thread_t *)*cpu0.cur_task;
    thread_t *new_thread = (thread_t *)*cpu0.next_task;
    if (old_thread == new_thread || pthread_self() != old_thread->thread)
    {
        pthread_mutex_unlock(&cpu0.mutex);
        return;
    }
    pthread_setname_np(new_thread->thread, "0");
    cpu0.cur_task = cpu0.next_task;
    event_signal(new_thread);
    if (0 == old_thread->exit)
    {
        event_wait(old_thread, &cpu0.mutex);
    }
    pthread_mutex_unlock(&cpu0.mutex);
}

static void cpu0_initialize_stack(stack_size_t **stack_top, int stack_size, void *(*task_function)(void *arg), void *arg)
{
    thread_t *thread = (thread_t *)*stack_top - 1;
    *stack_top = (stack_size_t *)thread;
    thread->task_function = task_function;
    thread->arg = arg;
    thread->exit = 0;
    pthread_cond_init(&thread->cond, NULL);

    if (pthread_create(&thread->thread, NULL, task_enter, thread) != 0)
    {
        printf("Create thread fail.\n");
    }
    pthread_setname_np(thread->thread, "0");
    pthread_detach(thread->thread);
}

static void enable_cpu1();
static void cpu0_start_thread(stack_size_t **stack_top)
{
    cpu0.cur_task = stack_top;
    thread_t *thread = (thread_t *)*cpu0.cur_task;
    event_signal(thread);

    enable_cpu1();

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGALRM);
    pthread_sigmask(SIG_UNBLOCK, &signals, NULL);
    for (;;)
    {
        sleep(1000);
    }
}

static void cpu0_yield(stack_size_t **stack_top)
{
    pthread_mutex_lock(&cpu0.mutex);
    cpu0.next_task = stack_top;
    thread_t *cur_thread = (thread_t *)*cpu0.cur_task;
    pthread_kill(cur_thread->thread, SIGUSR1);
    pthread_mutex_unlock(&cpu0.mutex);
}

static void cpu1_timer_tick()
{
    sys_cpu_thread_tick(&cpu1.cpu_thread, 10 * 1000 * 1000);
}

static void cpu1_task_yield()
{
    pthread_mutex_lock(&cpu1.mutex);
    thread_t *old_thread = (thread_t *)*cpu1.cur_task;
    thread_t *new_thread = (thread_t *)*cpu1.next_task;
    if (old_thread == new_thread || pthread_self() != old_thread->thread)
    {
        pthread_mutex_unlock(&cpu1.mutex);
        return;
    }
    pthread_setname_np(new_thread->thread, "1");
    cpu1.cur_task = cpu1.next_task;
    event_signal(new_thread);
    if (0 == old_thread->exit)
    {
        event_wait(old_thread, &cpu1.mutex);
    }
    pthread_mutex_unlock(&cpu1.mutex);
}

static void cpu1_initialize_stack(stack_size_t **stack_top, int stack_size, void *(*task_function)(void *arg), void *arg)
{
    thread_t *thread = (thread_t *)*stack_top - 1;
    *stack_top = (stack_size_t *)thread;
    thread->task_function = task_function;
    thread->arg = arg;
    thread->exit = 0;
    pthread_cond_init(&thread->cond, NULL);

    if (pthread_create(&thread->thread, NULL, task_enter, thread) != 0)
    {
        printf("Create thread fail.\n");
    }
    pthread_setname_np(thread->thread, "1");
    pthread_detach(thread->thread);
}

static void cpu1_start_thread(stack_size_t **stack_top)
{
    cpu1.cur_task = stack_top;
    thread_t *thread = (thread_t *)*cpu1.cur_task;
    event_signal(thread);
    for (;;)
    {
        sleep(1000);
    }
}

static void cpu1_yield(stack_size_t **stack_top)
{
    pthread_mutex_lock(&cpu1.mutex);
    cpu1.next_task = stack_top;
    thread_t *cur_thread = (thread_t *)*cpu1.cur_task;
    pthread_kill(cur_thread->thread, SIGUSR2);
    pthread_mutex_unlock(&cpu1.mutex);
}

static void *cpu1_thread(void *arg)
{
    sys_cpu_thread_start(&cpu1.cpu_thread);
    return NULL;
}

static void enable_cpu1()
{
    pthread_t t;
    pthread_create(&t, NULL, cpu1_thread, NULL);
}

static void timer_tick()
{
    pthread_mutex_lock(&cpu0.mutex);
    if (cpu0.cur_task != NULL)
    {
        thread_t *cur_thread = (thread_t *)*cpu0.cur_task;
        pthread_kill(cur_thread->thread, __SIGRTMIN + 10);
    }
    pthread_mutex_unlock(&cpu0.mutex);
    pthread_mutex_lock(&cpu1.mutex);
    if (cpu1.cur_task != NULL)
    {
        thread_t *cur_thread = (thread_t *)*cpu1.cur_task;
        pthread_kill(cur_thread->thread, __SIGRTMIN + 20);
    }
    pthread_mutex_unlock(&cpu1.mutex);
}

void sys_port_init()
{
    sigfillset(&s_signals);
    sigdelset(&s_signals, SIGINT);
    sigdelset(&s_signals, SIGTRAP);
    pthread_sigmask(SIG_SETMASK, &s_signals, NULL);
    sigdelset(&s_signals, SIGALRM);

    struct sigaction action;
    sigfillset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = cpu0_timer_tick;
    sigaction(__SIGRTMIN + 10, &action, NULL);
    action.sa_handler = cpu0_task_yield;
    sigaction(SIGUSR1, &action, NULL);
    action.sa_handler = cpu1_timer_tick;
    sigaction(__SIGRTMIN + 20, &action, NULL);
    action.sa_handler = cpu1_task_yield;
    sigaction(SIGUSR2, &action, NULL);
    action.sa_handler = timer_tick;
    sigaction(SIGALRM, &action, NULL);

    static timer_t timer;
    timer_create(CLOCK_REALTIME, NULL, &timer);
    struct itimerspec timerSpec;
    timerSpec.it_interval.tv_sec = 0;
    timerSpec.it_interval.tv_nsec = 10 * 1000 * 1000;
    timerSpec.it_value.tv_sec = 0;
    timerSpec.it_value.tv_nsec = 10 * 1000 * 1000;
    timer_settime(timer, 0, &timerSpec, NULL);

    sys_cpu_info_t cpu0_info = 
    {
        .name = "Linux thread",
        .arch = SYS_CPU_ARCH_X86_64,
        .byte_order = SYS_CPU_BYTE_ORDER_LE,
        .index = 0,
        .frequency = 1000000000,
        .ability = 100,
    };

    sys_cpu_operations_t cpu0_operations = 
    {
        .initialize_stack = cpu0_initialize_stack,
        .start_thread = cpu0_start_thread,
        .yield = cpu0_yield,
    };
    pthread_mutex_init(&cpu0.mutex, NULL);
    sys_cpu_thread_init(&cpu0.cpu_thread, &cpu0_info, &cpu0_operations);
    cpu0.irq = 1;
    sys_task_add_cpu_thread(&cpu0.cpu_thread);

    sys_cpu_info_t cpu1_info = 
    {
        .name = "Linux thread",
        .arch = SYS_CPU_ARCH_X86_64,
        .byte_order = SYS_CPU_BYTE_ORDER_LE,
        .index = 1,
        .frequency = 1000000000,
        .ability = 100,
    };

    sys_cpu_operations_t cpu1_operations = 
    {
        .initialize_stack = cpu1_initialize_stack,
        .start_thread = cpu1_start_thread,
        .yield = cpu1_yield,
    };
    pthread_mutex_init(&cpu1.mutex, NULL);
    sys_cpu_thread_init(&cpu1.cpu_thread, &cpu1_info, &cpu1_operations);
    cpu1.irq = 1;
    sys_task_add_cpu_thread(&cpu1.cpu_thread);
}

int sys_backtrace(void **array, int size)
{
    return backtrace(array, size);
}
