#include "osport.h"
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include "osplatformdef.h"
char _heap[HEAP_SIZE];
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

static void **sRunningTask = NULL;
static pthread_once_t sSigSetupThread = PTHREAD_ONCE_INIT;
static sigset_t sSignals;

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
    pthread_detach(thread->thread);
    return 0;
}

static void handleTimerTick(int arg)
{
    osTaskTick();
}

int portStartScheduler(void **stackTop)
{
    signal(SIGALRM, handleTimerTick);

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

    sRunningTask = stackTop;
    thread_t *firstthread = (thread_t *)*sRunningTask;
    eventSignal(firstthread);
    for (;;)
    {
        /* code */
    }
    return 0;
}

int portYield(void **stackTop)
{
    if (stackTop != sRunningTask)
    {
        thread_t *oldThread = (thread_t *)*sRunningTask;
        sRunningTask = stackTop;
        thread_t *newThread = (thread_t *)*sRunningTask;
        eventSignal(newThread);
        if (oldThread->exit > 0)
        {
            eventWait(oldThread);
        }
    }
    return 0;
}

static os_size_t sInterruptFlag = 1;
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

