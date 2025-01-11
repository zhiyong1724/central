#include "sys_port.h"
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include "sys_external_cfg.h"
#include "lvgl.h"
#include <unistd.h>
char _heap[SYS_HEAP_SIZE];
typedef struct thread_t
{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    task_function_t task_function;
    void *arg;
    int eventTriggered;
    int exit;
} thread_t;
static stack_size_t **sPreTask = NULL;
static stack_size_t **s_runningTask = NULL;
static pthread_once_t sSigSetupThread = PTHREAD_ONCE_INIT;
static sigset_t sSignals;
static int sInterruptFlag = 1;

static void eventWait(thread_t *thread)
{
    pthread_mutex_lock(&thread->mutex);
    while (0 == thread->eventTriggered)
    {
        pthread_cond_wait(&thread->cond, &thread->mutex);
    }
    thread->eventTriggered = 0;
    pthread_mutex_unlock(&thread->mutex);
    pthread_sigmask(SIG_UNBLOCK, &sSignals, NULL);
}

void eventSignal(thread_t *thread)
{
    pthread_sigmask(SIG_BLOCK, &sSignals, NULL);
    pthread_mutex_lock(&thread->mutex);
    thread->eventTriggered = 1;
    pthread_cond_signal(&thread->cond);
    pthread_mutex_unlock(&thread->mutex);
}

static void *taskEnter(void *arg)
{
    thread_t *thread = (thread_t *)arg;
    eventWait(thread);
    void *ret = thread->task_function(thread->arg);
    thread->exit = 0;
    sys_task_exit(ret);
    return ret;
}

void handleSignalInit()
{
    sigfillset(&sSignals);
    sigdelset(&sSignals, SIGINT);
    pthread_sigmask(SIG_SETMASK, &sSignals, NULL);
}

void port_initialize_stack(stack_size_t **stack_top, int stack_size, stack_size_t *task_stack_magic, task_function_t task_function, void *arg)
{
    pthread_once(&sSigSetupThread, handleSignalInit);
    unsigned char *stack_start = (unsigned char *)*stack_top - stack_size;
    thread_t *thread = (thread_t *)*stack_top - 1;
    *stack_top = (stack_size_t *)thread;
    thread->task_function = task_function;
    thread->arg = arg;
    thread->eventTriggered = 0;
    thread->exit = 1;
    pthread_mutex_init(&thread->mutex, NULL);
    pthread_cond_init(&thread->cond, NULL);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr, stack_start, stack_size - sizeof(thread_t));
    if (pthread_create(&thread->thread, &attr, taskEnter, thread) != 0)
    {
        printf("Create thread fail.");
    }
}

static timer_t sTimer;

static void handleTimerTick(int arg)
{
    static uint64_t ns = 1000 * 1000;
    int state = port_disable_interrupts();
    lv_tick_inc(ns / 1000 / 1000);
    sys_task_tick(&ns);
    struct itimerspec timerSpec;
    timerSpec.it_interval.tv_sec = 0;
    timerSpec.it_interval.tv_nsec = 0;
    timerSpec.it_value.tv_sec = 0;
    timerSpec.it_value.tv_nsec = ns;
    timer_settime(sTimer, 0, &timerSpec, NULL);
    port_recovery_interrupts(state);
}

static void handleTaskYield(int arg)
{
    thread_t *oldThread = (thread_t *)*sPreTask;
    thread_t *newThread = (thread_t *)*s_runningTask;
    eventSignal(newThread);
    if (oldThread->exit > 0)
    {
        eventWait(oldThread);
    }
} 

void port_start_scheduler(stack_size_t **stack_top)
{
    struct sigaction action;
    sigfillset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = handleTimerTick;
    sigaction(SIGALRM, &action, NULL);
    action.sa_handler = handleTaskYield;
    sigaction(SIGUSR1, &action, NULL);

    timer_create(CLOCK_REALTIME, NULL, &sTimer);
    struct itimerspec timerSpec;
    timerSpec.it_interval.tv_sec = 0;
    timerSpec.it_interval.tv_nsec = 0;
    timerSpec.it_value.tv_sec = 0;
    timerSpec.it_value.tv_nsec = 1000 * 1000;
    timer_settime(sTimer, 0, &timerSpec, NULL);
    sPreTask = s_runningTask;
    s_runningTask = stack_top;
    sInterruptFlag = 1;
    thread_t *newThread = (thread_t *)*s_runningTask;
    eventSignal(newThread);
    for (;;)
    {
        sleep(1000);
    }
}

void port_yield(stack_size_t **stack_top)
{
    if (stack_top != s_runningTask)
    {
        sPreTask = s_runningTask;
        s_runningTask = stack_top;
        raise(SIGUSR1);
    }
}

int port_disable_interrupts()
{
    pthread_sigmask(SIG_BLOCK, &sSignals, NULL);
    int ret = sInterruptFlag;
    sInterruptFlag = 0;
    return ret;
}

void port_recovery_interrupts(int state)
{  
    if (1 == state)
    {
        sInterruptFlag = state;
        pthread_sigmask(SIG_UNBLOCK, &sSignals, NULL);
    }
}

