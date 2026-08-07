#include "sys_mem.h"
#include "sys_mem_manager.h"
#include "sys_spin_lock.h"
static sys_mem_manager_t s_mem_manager;
static SYS_SPIN_LOCK_DEFINE(s_lock);
size_t sys_mem_init(void *start_address, size_t size)
{
    sys_trace();
    return sys_mem_manager_init(&s_mem_manager, start_address, size);
}

void *sys_malloc(size_t size)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    void *ret = sys_mem_manager_alloc(&s_mem_manager, size);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
    return ret;
}

void *sys_realloc(void *address, size_t new_size)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    void *ret = sys_mem_manager_realloc(&s_mem_manager, address, new_size);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
    return ret;
}

void sys_free(void *address)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    sys_mem_manager_free(&s_mem_manager, address);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
}

size_t sys_usable_size(void *address)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    size_t ret = sys_mem_manager_usable_size(&s_mem_manager, address);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
    return ret;
}

void *sys_alloc_pages(size_t n)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    void *ret = sys_mem_manager_alloc_pages(&s_mem_manager, n);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
    return ret;
}

void sys_free_pages(void *pages)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    sys_mem_manager_free_pages(&s_mem_manager, pages);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
}

size_t sys_total_mem()
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    size_t ret =  sys_mem_manager_total_mem(&s_mem_manager);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
    return ret;
}

size_t sys_free_mem()
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    size_t ret =  sys_mem_manager_free_mem(&s_mem_manager);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
    return ret;
}

size_t sys_total_page()
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    size_t ret =  sys_mem_manager_total_page(&s_mem_manager);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
    return ret;
}

size_t sys_free_page()
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&s_lock);
    size_t ret =  sys_mem_manager_free_page(&s_mem_manager);
    sys_spin_lock_unlock_and_irq_restore(&s_lock, state);
    return ret;
}
