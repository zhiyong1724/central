#include "osport.h"
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include "osplatformdef.h"
#include "lvgl.h"
#include <unistd.h>
char _heap[OS_HEAP_SIZE];
typedef struct thread_t
{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    TaskFunction taskFunction;
    void *arg;
    int eventTriggered;
    int exit;
} thread_t;
static void **sPreTask = NULL;
static void **sRunningTask = NULL;
static pthread_once_t sSigSetupThread = PTHREAD_ONCE_INIT;
static sigset_t sSignals;
static os_size_t sInterruptFlag = 1;

static void eventWait(thread_t *thread)
{
    pthread_mutex_lock(&thread->mutex);
    while (0 == thread->eventTriggered)
    {
        pthread_cond_wait(&thread->cond, &thread->mutex);
    }
    thread->eventTriggered = 0;
    pthread_mutex_unlock(&thread->mutex);
}

void eventSignal(thread_t *thread)
{
    pthread_mutex_lock(&thread->mutex);
    thread->eventTriggered = 1;
    pthread_cond_signal(&thread->cond);
    pthread_mutex_unlock(&thread->mutex);
}

static void *taskEnter(void *arg)
{
    thread_t *thread = (thread_t *)arg;
    eventWait(thread);
    pthread_sigmask(SIG_UNBLOCK, &sSignals, NULL);
    void *ret = thread->taskFunction(thread->arg);
    thread->exit = 0;
    osTaskExit(ret);
    return ret;
}

void handleSignalInit()
{
    sigfillset(&sSignals);
    sigdelset(&sSignals, SIGINT);
    pthread_sigmask(SIG_SETMASK, &sSignals, NULL);
}

int portInitializeStack(void **stackTop, os_size_t stackSize, os_size_t *taskStackMagic, TaskFunction taskFunction, void *arg)
{
    pthread_once(&sSigSetupThread, handleSignalInit);
    os_byte_t *stackStart = (os_byte_t *)*stackTop - stackSize;
    thread_t *thread = (thread_t *)*stackTop - 1;
    *stackTop = thread;
    thread->taskFunction = taskFunction;
    thread->arg = arg;
    thread->eventTriggered = 0;
    thread->exit = 1;
    pthread_mutex_init(&thread->mutex, NULL);
    pthread_cond_init(&thread->cond, NULL);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr, stackStart, stackSize - sizeof(thread_t));
    if (pthread_create(&thread->thread, &attr, taskEnter, thread) != 0)
    {
        printf("Create thread fail.");
        return -1;
    }
    return 0;
}

static void handleTimerTick(int arg)
{
    osTaskTick();
    lv_tick_inc(10);
}

static void handleTaskYield(int arg)
{
    thread_t *oldThread = (thread_t *)*sPreTask;
    thread_t *newThread = (thread_t *)*sRunningTask;
    eventSignal(newThread);
    if (oldThread->exit > 0)
    {
        eventWait(oldThread);
    }
} 

int portStartScheduler(void **stackTop)
{
    struct sigaction action;
    sigfillset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_handler = handleTimerTick;
    sigaction(SIGALRM, &action, NULL);
    action.sa_handler = handleTaskYield;
    sigaction(SIGUSR1, &action, NULL);
    struct itimerval tv;
    tv.it_interval.tv_sec = 0;
    tv.it_interval.tv_usec = 10000;
    tv.it_value.tv_sec = 0;
    tv.it_value.tv_usec = 10000;
    if (setitimer(ITIMER_REAL, &tv, NULL) != 0)
    {
        printf("Create timer fail.\n");
        return -1;
    }

    sPreTask = sRunningTask;
    sRunningTask = stackTop;
    sInterruptFlag = 1;
    thread_t *newThread = (thread_t *)*sRunningTask;
    eventSignal(newThread);
    for (;;)
    {
        sleep(1000);
    }
    return 0;
}

int portYield(void **stackTop)
{
    if (stackTop != sRunningTask)
    {
        sPreTask = sRunningTask;
        sRunningTask = stackTop;
        raise(SIGUSR1);
    }
    return 0;
}

os_size_t portDisableInterrupts()
{
    pthread_sigmask(SIG_BLOCK, &sSignals, NULL);
    os_size_t ret = sInterruptFlag;
    sInterruptFlag = 0;
    return ret;
}

int portRecoveryInterrupts(os_size_t state)
{  
    if (1 == state)
    {
        sInterruptFlag = state;
        pthread_sigmask(SIG_UNBLOCK, &sSignals, NULL);
    }
    return 0;
}

