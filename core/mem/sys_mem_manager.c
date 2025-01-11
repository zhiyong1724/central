#include "sys_mem_manager.h"
#include "sys_string.h"
typedef struct sys_mem_block_header_t
{
	void *header;
	long size;
} sys_mem_block_header_t;

typedef struct sys_mem_block_t
{
	sys_tree_node_t node;
	sys_mem_block_header_t header;
} sys_mem_block_t;

typedef struct sys_page_header_t
{
	long total_mem;
	long used_mem;
} sys_page_header_t;

long sys_mem_manager_init(sys_mem_manager_t *mem_manager, void *start_address, long size)
{
	sys_trace();
	long ret = sys_buddy_init(&mem_manager->page_factory, start_address, size) * SYS_BUDDY_PAGE_SIZE;
	if (ret < 0)
	{
		return ret;
	}
	mem_manager->total_mem = ret;
	mem_manager->free_mem = mem_manager->total_mem;
	mem_manager->root = NULL;
	return mem_manager->free_mem;
}

static sys_mem_block_t *find_suitable_block(sys_mem_manager_t *mem_manager, long size)
{
	sys_trace();
	sys_mem_block_t *ret = NULL;
	sys_tree_node_t *next_node = mem_manager->root;
	if (next_node != NULL)
	{
		for (; next_node != &g_leaf_node;)
		{
			sys_mem_block_t *mem_block = (sys_mem_block_t *)next_node;
			if (size > mem_block->header.size)
			{
				next_node = next_node->right;
			}
			else if (size < mem_block->header.size)
			{
				ret = mem_block;
				next_node = next_node->left;
			}
			else
			{
				ret = mem_block;
				break;
			}
		}
	}
	return ret;
}

static long size_align(long size)
{
	sys_trace();
	long offset = size % sizeof(long long);
	if (offset > 0)
	{
		size += sizeof(long long) - offset;
	}
	return size;
}

static int on_compare(void *key1, void *key2, void *arg)
{
	sys_trace();
	sys_mem_block_t *mem_block1 = (sys_mem_block_t *)key1;
	sys_mem_block_t *mem_block2 = (sys_mem_block_t *)key2;
	if (mem_block1->header.size < mem_block2->header.size)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

static void *block_to_mem(sys_mem_block_t *mem_block)
{
	sys_trace();
	sys_mem_block_header_t *header = (sys_mem_block_header_t *)mem_block;            //这里是树节点区域，所以不会被覆盖
	header->header = mem_block->header.header;
	header->size = mem_block->header.size | 0x01;
	return header + 1;
}

static void *split_block(sys_mem_manager_t *mem_manager, sys_mem_block_t *mem_block, long size)
{
	sys_trace();
	unsigned char *block_b = (unsigned char *)mem_block;
	block_b += size;
	sys_mem_block_t *new_block = (sys_mem_block_t *)block_b;
	new_block->header.header = mem_block->header.header;
	new_block->header.size = mem_block->header.size - size;
	sys_insert_node(&mem_manager->root, &new_block->node, on_compare, NULL);
	mem_block->header.size = size;
	return mem_block;
}

void *sys_mem_manager_alloc(sys_mem_manager_t *mem_manager, long size)
{
	sys_trace();
	void *ret = NULL;
	long new_size = size_align(size);
	new_size += sizeof(sys_mem_block_header_t);
	if (new_size < sizeof(sys_mem_block_t))
	{
		new_size = sizeof(sys_mem_block_t);
	}
	if (new_size > size && new_size <= mem_manager->free_mem)
	{
		sys_mem_block_t *mem_block = find_suitable_block(mem_manager, new_size);
		if (mem_block != NULL)
		{
			sys_delete_node(&mem_manager->root, &mem_block->node);
		}
		else
		{
			long page_count = 1;
			for (long i = 0; i < mem_manager->page_factory.group_count; i++)
			{
				if (new_size + sizeof(sys_page_header_t) <= page_count * SYS_BUDDY_PAGE_SIZE)
				{
					break;
				}
				page_count <<= 1;
			}
			unsigned char *buddy_page = (unsigned char *)sys_buddy_alloc_pages(&mem_manager->page_factory, page_count);
			if (buddy_page != NULL)
			{
				sys_page_header_t *page_header = (sys_page_header_t *)buddy_page;
				page_header->used_mem = 0;
				buddy_page += sizeof(sys_page_header_t);
				mem_manager->free_mem -= sizeof(sys_page_header_t);
				mem_block = (sys_mem_block_t *)buddy_page;
				mem_block->header.header = page_header;
				mem_block->header.size = page_count * SYS_BUDDY_PAGE_SIZE - sizeof(sys_page_header_t);
				page_header->total_mem = mem_block->header.size;
			}
		}
		if (mem_block != NULL)
		{
			if (mem_block->header.size >= new_size + sizeof(sys_mem_block_t))
			{
				mem_manager->free_mem -= new_size;
				sys_page_header_t *page_header = (sys_page_header_t *)mem_block->header.header;
				page_header->used_mem += new_size;
				ret = block_to_mem((sys_mem_block_t *)split_block(mem_manager, mem_block, new_size));
			}
			else
			{
				mem_manager->free_mem -= mem_block->header.size;
				sys_page_header_t *page_header = (sys_page_header_t *)mem_block->header.header;
				page_header->used_mem += mem_block->header.size;
				ret = block_to_mem(mem_block);
			}
		}
	}
	return ret;
}

void *sys_mem_manager_realloc(sys_mem_manager_t *mem_manager, void *address, long new_size)
{
	sys_trace();
	void *ret = NULL;
	if (address != NULL)
	{
		if (new_size > 0)
		{
			sys_assert(address >= mem_manager->page_factory.start_address);
			if (address >= mem_manager->page_factory.start_address)
			{
				sys_assert(((char *)address - (char *)mem_manager->page_factory.start_address) / (long)SYS_BUDDY_PAGE_SIZE < mem_manager->page_factory.total_page_num);
				if (((char *)address - (char *)mem_manager->page_factory.start_address) / (long)SYS_BUDDY_PAGE_SIZE < mem_manager->page_factory.total_page_num)
				{
					sys_mem_block_header_t *header = (sys_mem_block_header_t *)address;
					header -= 1;
					long mark = (long)sizeof(long long) - 1;
					sys_assert((header->size & mark) == 0x01);
					if ((header->size & mark) == 0x01)
					{
						ret = sys_mem_manager_alloc(mem_manager, new_size);
						sys_assert(ret != NULL);
						if (ret != NULL)
						{
							long size = (header->size & ~(long)0x01) - sizeof(sys_mem_block_header_t);
							if (new_size < size)
							{
								size = new_size;
							}
							sys_memcpy(ret, address, size);
						}
						sys_mem_manager_free(mem_manager, address);
					}
				}
			}
		}
		else
		{
			sys_mem_manager_free(mem_manager, address);
		}
	}
	else
	{
		ret = sys_mem_manager_alloc(mem_manager, new_size);
	}
	return ret;
}

static void free_all_nodes(sys_mem_manager_t *mem_manager, sys_page_header_t *header, sys_mem_block_t *mask)
{
	sys_trace();
	sys_mem_block_t *cur_block = (sys_mem_block_t *)(header + 1);
	for (long i = 0; i < header->total_mem; )
	{
		if (cur_block != mask)
		{
			sys_assert(cur_block->header.header == header);
			if (cur_block->header.header == header)
			{
				sys_delete_node(&mem_manager->root, &cur_block->node);
			}
		}
		i += cur_block->header.size;
		cur_block = (sys_mem_block_t *)((unsigned char *)cur_block + cur_block->header.size);
	}
	
}

void sys_mem_manager_free(sys_mem_manager_t *mem_manager, void *address)
{
	sys_trace();
	if (address != NULL)
	{
		sys_assert(address >= mem_manager->page_factory.start_address);
		if (address >= mem_manager->page_factory.start_address)
		{
			sys_assert(((char *)address - (char *)mem_manager->page_factory.start_address) / (long)SYS_BUDDY_PAGE_SIZE < mem_manager->page_factory.total_page_num);
			if (((char *)address - (char *)mem_manager->page_factory.start_address) / (long)SYS_BUDDY_PAGE_SIZE < mem_manager->page_factory.total_page_num)
			{
				sys_mem_block_header_t *header = (sys_mem_block_header_t *)address;
				header -= 1;
				long mark = (long)sizeof(long long) - 1;
				sys_assert((header->size & mark) == 0x01);
				if ((header->size & mark) == 0x01)
				{
					header->size &= ~(long)0x01;
					sys_mem_block_t *mem_block = (sys_mem_block_t *)header;
					mem_block->header.header = header->header;
					mem_block->header.size = header->size;
					mem_manager->free_mem += mem_block->header.size;
					sys_page_header_t *page_header = (sys_page_header_t *)mem_block->header.header;
					page_header->used_mem -= mem_block->header.size;
					if (0 == page_header->used_mem)
					{
						free_all_nodes(mem_manager, page_header, mem_block);
						sys_buddy_free_pages(&mem_manager->page_factory, mem_block->header.header);
						mem_manager->free_mem += sizeof(sys_page_header_t);
					}
					else
					{
						sys_insert_node(&mem_manager->root, &mem_block->node, on_compare, NULL);
					}
				}
			}
		}
	}
}

long sys_mem_manager_usable_size(sys_mem_manager_t *mem_manager, const void *address)
{
	sys_trace();
	int ret = 0;
	if (address != NULL)
	{
		sys_assert(address >= mem_manager->page_factory.start_address);
		if (address >= mem_manager->page_factory.start_address)
		{
			sys_assert(((char *)address - (char *)mem_manager->page_factory.start_address) / (long)SYS_BUDDY_PAGE_SIZE < mem_manager->page_factory.total_page_num);
			if (((char *)address - (char *)mem_manager->page_factory.start_address) / (long)SYS_BUDDY_PAGE_SIZE < mem_manager->page_factory.total_page_num)
			{
				sys_mem_block_header_t *header = (sys_mem_block_header_t *)address;
				header -= 1;
				long mark = (long)sizeof(long long) - 1;
				sys_assert((header->size & mark) == 0x01);
				if ((header->size & mark) == 0x01)
				{
					ret = (header->size & ~(long)0x01) - sizeof(sys_mem_block_header_t);
				}
			}
		}
	}
	return ret;
}

void *sys_mem_manager_alloc_pages(sys_mem_manager_t *mem_manager, long n)
{
	sys_trace();
	return sys_buddy_alloc_pages(&mem_manager->page_factory, n);
}

void sys_mem_manager_free_pages(sys_mem_manager_t *mem_manager, void *pages)
{
	sys_trace();
	sys_buddy_free_pages(&mem_manager->page_factory, pages);
}

long sys_mem_manager_total_mem(sys_mem_manager_t *mem_manager)
{
	sys_trace();
	return mem_manager->total_mem;
}

long sys_mem_manager_free_mem(sys_mem_manager_t *mem_manager)
{
	sys_trace();
	return mem_manager->free_mem;
}

long sys_mem_manager_total_page(sys_mem_manager_t *mem_manager)
{
	sys_trace();
	return mem_manager->page_factory.total_page_num;
}

long sys_mem_manager_free_page(sys_mem_manager_t *mem_manager)
{
	sys_trace();
	return mem_manager->page_factory.free_page_num;
}