#include "sys_mem.h"
#include "sys_mem_manager.h"
#include "sys_lock.h"
static sys_mem_manager_t s_mem_manager;
static sys_rwlock_t s_lock;
long sys_mem_init(void *start_address, long size)
{
    sys_trace();
    sys_rwlock_create(&s_lock);
    return sys_mem_manager_init(&s_mem_manager, start_address, size);
}

void *sys_malloc(long size)
{
    sys_trace();
    sys_rwlock_wrlock(&s_lock);
    void *ret = sys_mem_manager_alloc(&s_mem_manager, size);
    sys_rwlock_unlock(&s_lock);
    return ret;
}

void *sys_realloc(void *address, long new_size)
{
    sys_trace();
    sys_rwlock_wrlock(&s_lock);
    void *ret = sys_mem_manager_realloc(&s_mem_manager, address, new_size);
    sys_rwlock_unlock(&s_lock);
    return ret;
}

void sys_free(void *address)
{
    sys_trace();
    sys_rwlock_wrlock(&s_lock);
    sys_mem_manager_free(&s_mem_manager, address);
    sys_rwlock_unlock(&s_lock);
}

long sys_usable_size(const void *address)
{
    sys_trace();
    sys_rwlock_rdlock(&s_lock);
    int ret = sys_mem_manager_usable_size(&s_mem_manager, address);
    sys_rwlock_unlock(&s_lock);
    return ret;
}

void *sys_alloc_pages(long n)
{
    sys_trace();
    sys_rwlock_wrlock(&s_lock);
    void *ret = sys_mem_manager_alloc_pages(&s_mem_manager, n);
    sys_rwlock_unlock(&s_lock);
    return ret;
}

void sys_free_pages(void *pages)
{
    sys_trace();
    sys_rwlock_wrlock(&s_lock);
    sys_mem_manager_free_pages(&s_mem_manager, pages);
    sys_rwlock_unlock(&s_lock);
}

long sys_total_mem()
{
    sys_trace();
    sys_rwlock_rdlock(&s_lock);
    long ret =  sys_mem_manager_total_mem(&s_mem_manager);
    sys_rwlock_unlock(&s_lock);
    return ret;
}

long sys_free_mem()
{
    sys_trace();
    sys_rwlock_rdlock(&s_lock);
    long ret =  sys_mem_manager_free_mem(&s_mem_manager);
    sys_rwlock_unlock(&s_lock);
    return ret;
}

long sys_total_page()
{
    sys_trace();
    sys_rwlock_rdlock(&s_lock);
    long ret =  sys_mem_manager_total_page(&s_mem_manager);
    sys_rwlock_unlock(&s_lock);
    return ret;
}

long sys_free_page()
{
    sys_trace();
    sys_rwlock_rdlock(&s_lock);
    long ret =  sys_mem_manager_free_page(&s_mem_manager);
    sys_rwlock_unlock(&s_lock);
    return ret;
}