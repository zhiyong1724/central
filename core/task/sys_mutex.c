#include "sys_mutex.h"
void sys_mutex_init(sys_mutex_t *lock)
{
    sys_trace();
    sys_semaphore_init(&lock->semaphore, 1, 1);
}

void sys_mutex_lock(sys_mutex_t *lock)
{
    sys_trace();
    sys_semaphore_wait(&lock->semaphore, SYS_SEMAPHORE_MAX_WAIT_TIME);
}

void sys_mutex_unlock(sys_mutex_t *lock)
{
    sys_trace();
    sys_semaphore_post(&lock->semaphore);
}