#include "sys_buddy.h"
#include "sys_string.h"
#define BUDDY_USED(index) ((int8_t)((index) + 1))
#define BUDDY_FREE(index) ((int8_t)(-((int8_t)(index) + 1)))

static void *address_align(void *address, size_t size)
{
    sys_trace();
    int8_t *start_address = (int8_t *)address;
    size_t offset = (size_t)start_address % size;
    if (offset > 0)
    {
        start_address += size - offset;
    }
    return start_address;
}

static uint8_t calculate_group_count(size_t page_num)
{
    sys_trace();
    uint8_t i = 1;
    size_t compare_value = 2;
    for (; compare_value <= page_num && i < 127; compare_value <<= 1)
    {
        i++;
    }
    return i;
}

static int8_t *fill_block_array(sys_buddy_t *buddy, int8_t *address, unsigned int block_array_id)
{
    sys_trace();
    size_t page_count = (size_t)1 << block_array_id;
    for (; buddy->total_page_num - buddy->free_page_num >= page_count; buddy->free_page_num += page_count)
    {
        sys_insert_to_front(&buddy->block_list_array[block_array_id], (sys_list_node_t *)address);
        address += page_count * SYS_BUDDY_PAGE_SIZE;
    }
    return address;
}

size_t sys_buddy_init(sys_buddy_t *buddy, void *start_address, size_t size)
{
    sys_trace();
    buddy->free_page_num = 0;
    buddy->total_page_num = size / SYS_BUDDY_PAGE_SIZE;
    sys_assert(buddy->total_page_num > 0);
    if (buddy->total_page_num < 1)
    {
        return 0;
    }
    buddy->block_group = (int8_t *)start_address;
    sys_memset(buddy->block_group, 0, buddy->total_page_num);
    if (size < buddy->total_page_num)
    {
        return 0;
    }
    size -= buddy->total_page_num;
    int8_t *block_group_end = buddy->block_group + buddy->total_page_num;

    int8_t *block_list_array_start = (int8_t *)address_align(block_group_end, sizeof(void *));
    size_t offset = (size_t)(block_list_array_start - block_group_end);
    if (size < offset)
    {
        return 0;
    }
    size -= offset;
    buddy->block_list_array = (sys_list_node_t **)block_list_array_start;
    buddy->group_count = calculate_group_count(buddy->total_page_num);
    if (size < buddy->group_count * sizeof(void *))
    {
        return 0;
    }
    size -= buddy->group_count * sizeof(void *);
    int8_t *block_list_array_end = block_list_array_start + buddy->group_count * sizeof(void *);

    buddy->start_address = (int8_t *)address_align(block_list_array_end, SYS_BUDDY_ADDRESS_ALIGNMENT);
    offset = (int8_t *)buddy->start_address - block_list_array_end;
    if (size < offset)
    {
        return 0;
    }
    size -= offset;

    buddy->total_page_num = size / SYS_BUDDY_PAGE_SIZE;
    sys_assert(buddy->total_page_num > 0);
    if (buddy->total_page_num < 1)
    {
        return 0;
    }
    buddy->group_count = calculate_group_count(buddy->total_page_num);
    for (int i = 0; i < (int)buddy->group_count; i++)
    {
        buddy->block_list_array[i] = NULL;
    }
    int8_t *handle = (int8_t *)buddy->start_address;
    for (int i = (int)buddy->group_count - 1; i >= 0; i--)
    {
        handle = fill_block_array(buddy, handle, i);
        if (buddy->free_page_num == buddy->total_page_num)
        {
            break;
        }
    }
    return buddy->free_page_num;
}

static void set_block_group(sys_buddy_t *buddy, void *address, int8_t value)
{
    sys_trace();
    size_t index = (size_t)((int8_t *)address - (int8_t *)buddy->start_address) / SYS_BUDDY_PAGE_SIZE;
    buddy->block_group[index] = value;
}

static int8_t get_block_group(sys_buddy_t *buddy, void *address)
{
    sys_trace();
    size_t index = (size_t)((int8_t *)address - (int8_t *)buddy->start_address) / SYS_BUDDY_PAGE_SIZE;
    return buddy->block_group[index];
}

static void *split_block(sys_buddy_t *buddy, void *address, unsigned int group_id)
{
    sys_trace();
    set_block_group(buddy, address, BUDDY_USED(group_id - 1));
    int8_t *block_b = (int8_t *)address;
    block_b += ((size_t)1 << (group_id - 1)) * SYS_BUDDY_PAGE_SIZE;
    set_block_group(buddy, block_b, BUDDY_FREE(group_id - 1));
    sys_insert_to_front(&buddy->block_list_array[group_id - 1], (sys_list_node_t *)block_b);
    return address;
}

static void *alloc_pages(sys_buddy_t *buddy, unsigned int group_id)
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
            set_block_group(buddy, ret, BUDDY_USED(group_id));
        }
        else
        {
            group_id++;
            if (group_id < buddy->group_count)
            {
                ret = alloc_pages(buddy, group_id);
                if (ret != NULL)
                {
                    ret = split_block(buddy, ret, group_id);
                }
            }
        }
    }
    return ret;
}

void *sys_buddy_alloc_pages(sys_buddy_t *buddy, size_t n)
{
    sys_trace();
    void *ret = NULL;
    sys_assert(n > 0 && n <= buddy->free_page_num);
    if (n > 0 && n <= buddy->free_page_num)
    {
        unsigned int group_id = 0;
        size_t page_num_per_block = 1;
        for (; n > page_num_per_block; page_num_per_block <<= 1)
        {
            group_id++;
        }
        ret = alloc_pages(buddy, group_id);
        if (ret != NULL)
        {
            buddy->free_page_num -= page_num_per_block;
        }
    }
    return ret;
}

static void free_pages(sys_buddy_t *buddy, void *pages, unsigned int group_id);
static int merge_block(sys_buddy_t *buddy, void *pages, unsigned int group_id)
{
    sys_trace();
    int ret = 0;
    if (group_id + 1 < buddy->group_count)
    {
        int8_t *block_a;
        int8_t *block_b;
        int8_t *buddy_block;
        size_t block_size = SYS_BUDDY_PAGE_SIZE * ((size_t)1 << group_id);
        if (0 == ((int8_t *)pages - (int8_t *)buddy->start_address) % (block_size << 1))
        {
            block_a = (int8_t *)pages;
            block_b = block_a + block_size;
            buddy_block = block_b;
            if (block_b + block_size > (int8_t *)buddy->start_address + buddy->total_page_num * SYS_BUDDY_PAGE_SIZE)
            {
                return ret;
            }
        }
        else
        {
            block_b = (int8_t *)pages;
            block_a = block_b - block_size;
            buddy_block = block_a;
        }
        int8_t block_group = get_block_group(buddy, buddy_block);
        int8_t expected_group = BUDDY_FREE(group_id);
        if (block_group == expected_group)
        {
            sys_remove_from_list(&buddy->block_list_array[group_id], (sys_list_node_t *)buddy_block);
            set_block_group(buddy, block_a, BUDDY_USED(group_id + 1));
            set_block_group(buddy, block_b, 0);
            free_pages(buddy, block_a, group_id + 1);
            ret = 1;
        }
    }
    return ret;
}

static void free_pages(sys_buddy_t *buddy, void *pages, unsigned int group_id)
{
    sys_trace();
    if (!merge_block(buddy, pages, group_id))
    {
        set_block_group(buddy, pages, BUDDY_FREE(group_id));
        sys_insert_to_front(&buddy->block_list_array[group_id], (sys_list_node_t *)pages);
    }
}

void sys_buddy_free_pages(sys_buddy_t *buddy, void *pages)
{
    sys_trace();
    sys_assert(pages >= buddy->start_address);
    if (pages >= buddy->start_address)
    {
        sys_assert(0 == ((int8_t *)pages - (int8_t *)buddy->start_address) % SYS_BUDDY_PAGE_SIZE);
        if (0 == ((int8_t *)pages - (int8_t *)buddy->start_address) % SYS_BUDDY_PAGE_SIZE)
        {
            sys_assert((size_t)((int8_t *)pages - (int8_t *)buddy->start_address) / SYS_BUDDY_PAGE_SIZE < buddy->total_page_num);
            if ((size_t)((int8_t *)pages - (int8_t *)buddy->start_address) / SYS_BUDDY_PAGE_SIZE < buddy->total_page_num)
            {
                int8_t block_group = get_block_group(buddy, pages);
                sys_assert(block_group > 0 && block_group <= buddy->group_count);
                if (block_group > 0 && block_group <= buddy->group_count)
                {
                    free_pages(buddy, pages, block_group - 1);
                    buddy->free_page_num += (size_t)1 << (block_group - 1);
                }
            }
        }
    }
}

size_t sys_buddy_total_page_num(sys_buddy_t *buddy) 
{
    sys_trace();
	return buddy->total_page_num;
}

size_t sys_buddy_free_page_num(sys_buddy_t *buddy)
{
    sys_trace();
	return buddy->free_page_num;
}
