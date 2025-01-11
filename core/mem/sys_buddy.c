#include "sys_buddy.h"
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

static long calculate_group_count(long page_num)
{
    sys_trace();
    long i = 1;
    long compare_value = 2;
    for (; compare_value <= page_num && i < 128; compare_value <<= 1)
    {
        i++;
    }
    return i;
}

static unsigned char *fill_block_array(sys_buddy_t *buddy, unsigned char *address, long block_array_id)
{
    sys_trace();
    long page_count = 1 << block_array_id;
    for (; buddy->total_page_num - buddy->free_page_num >= page_count; buddy->free_page_num += page_count)
    {
        sys_insert_to_front(&buddy->block_list_array[block_array_id], (sys_list_node_t *)address);
        address += page_count * SYS_BUDDY_PAGE_SIZE;
    }
    return address;
}

long sys_buddy_init(sys_buddy_t *buddy, void *start_address, long size)
{
    sys_trace();
    buddy->free_page_num = 0;
    buddy->total_page_num = size / SYS_BUDDY_PAGE_SIZE;
    sys_assert(buddy->total_page_num >= 2);
    if (buddy->total_page_num < 2)
    {
        return -1;
    }
    buddy->block_group = (unsigned char *)start_address;
    sys_memset(buddy->block_group, 0, buddy->total_page_num);
    size -= buddy->total_page_num;
    unsigned char *block_group_end = buddy->block_group + buddy->total_page_num;

    unsigned char *block_list_array_start = (unsigned char *)address_align(block_group_end, sizeof(void *));
    long offset = block_list_array_start - block_group_end;
    size -= offset;
    buddy->block_list_array = (sys_list_node_t **)block_list_array_start;
    buddy->group_count = calculate_group_count(buddy->total_page_num);
    size -= buddy->group_count * sizeof(void *);
    unsigned char *block_list_array_end = block_list_array_start + buddy->group_count * sizeof(void *);

    buddy->start_address = (unsigned char *)address_align(block_list_array_end, SYS_BUDDY_PAGE_SIZE);
    offset = (unsigned char *)buddy->start_address - block_list_array_end;
    size -= offset;

    buddy->total_page_num = size / SYS_BUDDY_PAGE_SIZE;
    sys_assert(buddy->total_page_num >= 2);
    if (buddy->total_page_num < 2)
    {
        return -1;
    }
    buddy->group_count = calculate_group_count(buddy->total_page_num);
    unsigned char *handle = (unsigned char *)buddy->start_address;
    for (int i = 0; i < (int)buddy->group_count; i++)
    {
        buddy->block_list_array[i] = NULL;
    }

    for (int i = (int)buddy->group_count - 1; i >= 0; i--)
    {
        handle = fill_block_array(buddy, handle, (long)i);
        if (buddy->free_page_num == buddy->total_page_num)
        {
            break;
        }
    }
    return buddy->free_page_num;
}

static void set_block_group(sys_buddy_t *buddy, void *address, unsigned char value)
{
    sys_trace();
    long index = ((char *)address - (char *)buddy->start_address) / SYS_BUDDY_PAGE_SIZE;
    buddy->block_group[index] = value;
}

static unsigned char get_block_group(sys_buddy_t *buddy, void *address)
{
    sys_trace();
    long index = ((char *)address - (char *)buddy->start_address) / SYS_BUDDY_PAGE_SIZE;
    return buddy->block_group[index];
}

static void *split_block(sys_buddy_t *buddy, void *address, long group_id)
{
    sys_trace();
    set_block_group(buddy, address, (unsigned char)group_id);
    unsigned char *block_b = (unsigned char *)address;
    block_b += (1 << (group_id - 1)) * SYS_BUDDY_PAGE_SIZE;
    set_block_group(buddy, block_b, (unsigned char)group_id * -1);
    sys_insert_to_front(&buddy->block_list_array[group_id - 1], (sys_list_node_t *)block_b);
    return address;
}

static void *alloc_pages(sys_buddy_t *buddy, long group_id)
{
    sys_trace();
    void *ret = NULL;
    sys_assert(group_id < buddy->group_count);
    if (group_id < buddy->group_count)
    {
        if (buddy->block_list_array[group_id] != NULL)
        {
            ret = buddy->block_list_array[group_id];
            sys_remove_from_list(&buddy->block_list_array[group_id], (sys_list_node_t *)ret);
            set_block_group(buddy, ret, (unsigned char)group_id + 1);
        }
        else
        {
            group_id++;
            ret = alloc_pages(buddy, group_id);
            sys_assert(ret != NULL);
            if (ret != NULL)
            {
                ret = split_block(buddy, ret, group_id);
            }
        }
    }
    return ret;
}

void *sys_buddy_alloc_pages(sys_buddy_t *buddy, long n)
{
    sys_trace();
    void *ret = NULL;
    sys_assert(n > 0 && n <= buddy->free_page_num);
    if (n > 0 && n <= buddy->free_page_num)
    {
        long group_id = 0;
        long page_num_per_block = 1;
        for (; n > page_num_per_block; page_num_per_block <<= 1)
        {
            group_id++;
        }
        ret = alloc_pages(buddy, group_id);
        sys_assert(ret != NULL);
        if (ret != NULL)
        {
            buddy->free_page_num -= page_num_per_block;
        }
    }
    return ret;
}

static void free_pages(sys_buddy_t *buddy, void *pages, long group_id);
static int merge_block(sys_buddy_t *buddy, void *pages, long group_id)
{
    sys_trace();
    unsigned char ret = 0;
    if (group_id < buddy->group_count - 1)
    {
        unsigned char *block_a;
        unsigned char *block_b;
        unsigned char *buddy_block;
        long block_size = SYS_BUDDY_PAGE_SIZE * (1 << group_id);
        if (0 == ((char *)pages - (char *)buddy->start_address) % (block_size << 1))
        {
            block_a = (unsigned char *)pages;
            block_b = block_a + block_size;
            buddy_block = block_b;
            if (block_b + block_size >= (unsigned char *)buddy->start_address + buddy->total_page_num * SYS_BUDDY_PAGE_SIZE)
            {
                return ret;
            }
        }
        else
        {
            block_b = (unsigned char *)pages;
            block_a = block_b - block_size;
            buddy_block = block_a;
        }
        unsigned char block_group = get_block_group(buddy, buddy_block);
        ret = ((unsigned char)group_id + 1) * -1;
        if (block_group == ret)
        {
            sys_remove_from_list(&buddy->block_list_array[group_id], (sys_list_node_t *)buddy_block);
            set_block_group(buddy, block_a, group_id + 2);
            set_block_group(buddy, block_b, 0);
            free_pages(buddy, block_a, group_id + 1);
            ret = 1;
        }
    }
    return ret;
}

static void free_pages(sys_buddy_t *buddy, void *pages, long group_id)
{
    sys_trace();
    long value = merge_block(buddy, pages, group_id);
    if (value > 127 || 0 == value)
    {
        set_block_group(buddy, pages, (unsigned char)value);
        sys_insert_to_front(&buddy->block_list_array[group_id], (sys_list_node_t *)pages);
    }
}

void sys_buddy_free_pages(sys_buddy_t *buddy, void *pages)
{
    sys_trace();
    sys_assert(pages >= buddy->start_address);
    if (pages >= buddy->start_address)
    {
        sys_assert(0 == ((char *)pages - (char *)buddy->start_address) % SYS_BUDDY_PAGE_SIZE);
        if (0 == ((char *)pages - (char *)buddy->start_address) % SYS_BUDDY_PAGE_SIZE)
        {
            sys_assert(((char *)pages - (char *)buddy->start_address) / (long)SYS_BUDDY_PAGE_SIZE < buddy->total_page_num);
            if (((char *)pages - (char *)buddy->start_address) / (long)SYS_BUDDY_PAGE_SIZE < buddy->total_page_num)
            {
                long block_group = get_block_group(buddy, pages);
                sys_assert(block_group > 0 && block_group <= buddy->group_count);
                if (block_group > 0 && block_group <= buddy->group_count)
                {
                    free_pages(buddy, pages, block_group - 1);
                    buddy->free_page_num += 1 << (block_group - 1);
                }
            }
        }
    }
}

long sys_buddy_total_page_num(sys_buddy_t *buddy) 
{
    sys_trace();
	return buddy->total_page_num;
}

long sys_buddy_free_page_num(sys_buddy_t *buddy)
{
    sys_trace();
	return buddy->free_page_num;
}