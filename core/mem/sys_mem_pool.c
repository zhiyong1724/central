#include "sys_mem_pool.h"
#include "sys_string.h"
static void *address_align(void *address, long size)
{
    sys_trace();
    unsigned char *start_address = (unsigned char *)address;
    long offset = (long)start_address % size;
    if (offset > 0)
    {
        start_address += size - offset;
    }
    return start_address;
}

static void fill_bit_map(sys_mem_pool_t *mem_pool)
{
    sys_trace();
    long count = mem_pool->total_page_num / 8;
    long i = 0;
    for (; i < count; i++)
    {
        mem_pool->bitmap[i] = 0;
    }
    count = mem_pool->total_page_num % 8;
    if (count > 0)
    {
        mem_pool->bitmap[i] = 0;
        for (long j = 0; j < count; j++)
        {
            mem_pool->bitmap[i] |= 0x80 >> j;
        }
        mem_pool->bitmap[i] = ~mem_pool->bitmap[i];
    }
}

static void fill_page_list(sys_mem_pool_t *mem_pool, unsigned char *address)
{
    sys_trace();
    mem_pool->page_list = NULL;
    for (long i = 0; i < mem_pool->total_page_num; i++)
    {
        sys_insert_to_single_list(&mem_pool->page_list, (sys_single_list_node_t *)address);
        address += mem_pool->page_size;
    }
    
}

long sys_mem_pool_init(sys_mem_pool_t *mem_pool, void *start_address, long size, long page_size)
{
    sys_trace();
    page_size = (long)address_align((void *)page_size, sizeof(void *));
    mem_pool->total_page_num = size / page_size;
    sys_assert(mem_pool->total_page_num >= 2);
    if (mem_pool->total_page_num < 2)
    {
        return -1;
    }
    mem_pool->page_size = page_size;

    mem_pool->bitmap = (unsigned char *)start_address;
    long offset = mem_pool->total_page_num / 8;
    if (mem_pool->total_page_num % 8 > 0)
    {
        offset++;
    }
    size -= offset;

    unsigned char *bitmap_end = mem_pool->bitmap + offset;
    unsigned char *mem_start = (unsigned char *)address_align(bitmap_end, sizeof(long long));
    offset = mem_start - bitmap_end;
    size -= offset;
    mem_pool->total_page_num = size / page_size;
    sys_assert(mem_pool->total_page_num >= 2);
    if (mem_pool->total_page_num >= 2)
    {
        fill_bit_map(mem_pool);
        fill_page_list(mem_pool, mem_start);
        mem_pool->free_page_num = mem_pool->total_page_num;
        mem_pool->start_address = mem_start;
    }
    return mem_pool->free_page_num;
}

void *sys_mem_pool_alloc_page(sys_mem_pool_t *mem_pool)
{
    sys_trace();
    void *ret = NULL;
    sys_assert(mem_pool->free_page_num > 0 && mem_pool->page_list != NULL);
    if (mem_pool->free_page_num > 0 && mem_pool->page_list != NULL)
    {
        ret = mem_pool->page_list;
        sys_remove_from_single_list(&mem_pool->page_list);
        long index = ((char *)ret - (char *)mem_pool->start_address) / mem_pool->page_size;
        long i = index / 8;
        long j = index % 8;
        unsigned char mask = 0x80;
        mask >>= j;
        mem_pool->bitmap[i] |= mask;
        mem_pool->free_page_num--;
    }
    return ret;
}

int sys_mem_pool_free_page(sys_mem_pool_t *mem_pool, void *page)
{
    sys_trace();
    int ret = -1;
    sys_assert(page >= mem_pool->start_address);
    if (page >= mem_pool->start_address)
    {
        sys_assert(0 == ((char *)page - (char *)mem_pool->start_address) % mem_pool->page_size);
        if (0 == ((char *)page - (char *)mem_pool->start_address) % mem_pool->page_size)
        {
            long index = ((char *)page - (char *)mem_pool->start_address) / mem_pool->page_size;
            sys_assert(index < mem_pool->total_page_num);
            if (index < mem_pool->total_page_num)
            {
                long i = index / 8;
                long j = index % 8;
                unsigned char mask = 0x80;
                mask >>= j;
                mask &= mem_pool->bitmap[i];
                sys_assert(mask > 0);
                if (mask > 0)
                {
                    sys_insert_to_single_list(&mem_pool->page_list, (sys_single_list_node_t *)page);
                    mask = ~mask;
                    mem_pool->bitmap[i] &= mask;
                    mem_pool->free_page_num++;
                    ret = 0;
                }
                
            }
        }
    }
    return ret;
}

long sys_mem_pool_total_page_num(sys_mem_pool_t *mem_pool)
{
    sys_trace();
    return mem_pool->total_page_num;
}

long sys_mem_pool_free_page_num(sys_mem_pool_t *mem_pool)
{
    sys_trace();
    return mem_pool->free_page_num;
}