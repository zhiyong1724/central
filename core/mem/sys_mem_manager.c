#include "sys_mem_manager.h"
#include "sys_string.h"
#define PAGE_MAGIC 0x50414745  /* "PAGE" */
#define MEM_ALLOC 0x05ul
#define MEM_SIZE_MASK 0x07ul
struct page_header_t
{
	sys_tree_node_t node;
	uint32_t magic;
	size_t alloc_count;
	size_t max_block_size;
	size_t self_max_block_size;
	sys_tree_node_t *root;
};

struct mem_header_t
{
	size_t size;
	struct page_header_t *header;
};

struct block_header_t
{
	size_t size;
	sys_tree_node_t node;
	size_t max_block_size;
};

static size_t size_align(size_t size)
{
	sys_trace();
	size_t offset = size % (sizeof(long) * 2);
	if (offset > 0)
	{
		size_t add = (sizeof(long) * 2) - offset;
		if (size > SIZE_MAX - add)
		{
			return 0;
		}
		size += add;
	}
	return size;
}

static size_t alloc_size(size_t size)
{
	sys_trace();
	size_t header_size = size_align(sizeof(struct mem_header_t));
	if (header_size == 0 || size > SIZE_MAX - header_size)
	{
		return 0;
	}
	size_t new_size = header_size + size;
	if (new_size < sizeof(struct block_header_t))
	{
		new_size = sizeof(struct block_header_t);
	}
	return size_align(new_size);
}

size_t sys_mem_manager_init(sys_mem_manager_t *mem_manager, void *start_address, size_t size)
{
	sys_trace();
	size_t total_mem = sys_buddy_init(&mem_manager->page_factory, start_address, size) * SYS_BUDDY_PAGE_SIZE;
	if (total_mem == 0)
	{
		return total_mem;
	}
	mem_manager->total_mem = total_mem;
	mem_manager->free_mem = mem_manager->total_mem;
	mem_manager->root = NULL;
	return mem_manager->free_mem;
}

static struct page_header_t *find_fit_page(struct page_header_t *page_header, size_t size)
{
	sys_trace();
	if (page_header != NULL)
	{
		if (page_header->node.right != &g_leaf_node)
		{
			struct page_header_t *page = sys_container_of(page_header->node.right, struct page_header_t, node);
			if (size <= page->max_block_size)
			{
				return find_fit_page(page, size);
			}
		}
		if (size <= page_header->self_max_block_size)
		{
			return page_header;
		}
		if (page_header->node.left != &g_leaf_node)
		{
			struct page_header_t *page = sys_container_of(page_header->node.left, struct page_header_t, node);
			if (size <= page->max_block_size)
			{
				return find_fit_page(page, size);
			}
		}
	}
	return NULL;
}

static struct block_header_t *find_fit_block(struct block_header_t *block_header, size_t size)
{
	sys_trace();
	if (block_header != NULL)
	{
		if (size <= block_header->size)
		{
			return block_header;
		}
		if (block_header->node.left != &g_leaf_node)
		{
			struct block_header_t *block = sys_container_of(block_header->node.left, struct block_header_t, node);
			if (size <= block->max_block_size)
			{
				return find_fit_block(block, size);
			}
		}
		if (block_header->node.right != &g_leaf_node)
		{
			struct block_header_t *block = sys_container_of(block_header->node.right, struct block_header_t, node);
			if (size <= block->max_block_size)
			{
				return find_fit_block(block, size);
			}
		}
	}
	return NULL;
}

static int on_compare_block(void *key1, void *key2, void *arg)
{
	sys_trace();
	struct block_header_t *block1 = sys_container_of(key1, struct block_header_t, node);
	struct block_header_t *block2 = sys_container_of(key2, struct block_header_t, node);
	if (block1 < block2)
	{
		return -1;
	}
	else if (block1 > block2)
	{
		return 1;
	}
	return 0;
}

static struct page_header_t *alloc_page(sys_mem_manager_t *mem_manager, size_t size)
{
	sys_trace();
	size_t page_count = 1;
	for (size_t i = 0; i < mem_manager->page_factory.group_count; i++)
	{
		if (size + size_align(sizeof(struct page_header_t)) <= page_count * SYS_BUDDY_PAGE_SIZE)
		{
			break;
		}
		page_count <<= 1;
	}
	struct page_header_t *buddy_page = (struct page_header_t *)sys_buddy_alloc_pages(&mem_manager->page_factory, page_count);
	if (buddy_page != NULL)
	{
		buddy_page->magic = PAGE_MAGIC;
		buddy_page->alloc_count = 0;
		buddy_page->max_block_size = page_count * SYS_BUDDY_PAGE_SIZE - size_align(sizeof(struct page_header_t));
		buddy_page->self_max_block_size = buddy_page->max_block_size;
		buddy_page->root = NULL;
		struct block_header_t *block = (struct block_header_t *)((int8_t *)buddy_page + size_align(sizeof(struct page_header_t)));
		block->size = buddy_page->self_max_block_size;
		block->max_block_size = block->size;
		sys_insert_node(&buddy_page->root, &block->node, on_compare_block, NULL);
		mem_manager->free_mem -= size_align(sizeof(struct page_header_t));
		return buddy_page;
	}
	return NULL;
}

static void update_page_max(struct page_header_t *page)
{
	sys_trace();
    size_t max = page->self_max_block_size;

    if (page->node.left != &g_leaf_node)
    {
        struct page_header_t *left =
            sys_container_of(page->node.left, struct page_header_t, node);
        if (left->max_block_size > max)
            max = left->max_block_size;
    }

    if (page->node.right != &g_leaf_node)
    {
        struct page_header_t *right =
            sys_container_of(page->node.right, struct page_header_t, node);
        if (right->max_block_size > max)
            max = right->max_block_size;
    }

    page->max_block_size = max;
}

static void update_page_to_root(struct page_header_t *page)
{
	sys_trace();
    while (page != NULL)
    {
        update_page_max(page);
        if (page->node.parent == NULL)
            break;

        page = sys_container_of(page->node.parent, struct page_header_t, node);
    }
}

static void on_page_rotate(sys_tree_node_t *old_root, sys_tree_node_t *new_root, void *arg)
{
    update_page_max(sys_container_of(old_root, struct page_header_t, node));
    update_page_max(sys_container_of(new_root, struct page_header_t, node));
}

static void delete_page(sys_mem_manager_t *mem_manager, struct page_header_t *page)
{
	sys_trace();
	struct page_header_t *parent_page = NULL;
	struct page_header_t *after_page = NULL;
	struct page_header_t *after_parent_page = NULL;

	if (page->node.parent != NULL)
	{
		parent_page = sys_container_of(page->node.parent, struct page_header_t, node);
	}

	if (page->node.left != &g_leaf_node && page->node.right != &g_leaf_node)
	{
		sys_tree_node_t *after = sys_get_left_most_node(page->node.right);
		after_page = sys_container_of(after, struct page_header_t, node);

		if (after->parent != NULL && after->parent != &page->node)
		{
			after_parent_page = sys_container_of(after->parent, struct page_header_t, node);
		}
	}

	sys_delete_node_ex(&mem_manager->root, &page->node, on_page_rotate, NULL);

	if (after_parent_page != NULL)
	{
		update_page_to_root(after_parent_page);
	}
	else if (after_page != NULL)
	{
		update_page_to_root(after_page);
	}
	else if (parent_page != NULL)
	{
		update_page_to_root(parent_page);
	}
}

static void update_page(struct page_header_t *page)
{
	sys_trace();
	struct block_header_t *block = sys_container_of(page->root, struct block_header_t, node);
	if (block != NULL)
	{
		page->self_max_block_size = block->max_block_size;
	}
	else
	{
		page->self_max_block_size = 0;
	}
	page->max_block_size = page->self_max_block_size;
}

static void update_block_max(struct block_header_t *block)
{
	sys_trace();
    size_t max = block->size;

    if (block->node.left != &g_leaf_node)
    {
        struct block_header_t *left =
            sys_container_of(block->node.left, struct block_header_t, node);
        if (left->max_block_size > max)
            max = left->max_block_size;
    }

    if (block->node.right != &g_leaf_node)
    {
        struct block_header_t *right =
            sys_container_of(block->node.right, struct block_header_t, node);
        if (right->max_block_size > max)
            max = right->max_block_size;
    }

    block->max_block_size = max;
}

static void update_block_to_root(struct block_header_t *block)
{
	sys_trace();
    while (block != NULL)
    {
        update_block_max(block);
        if (block->node.parent == NULL)
            break;

        block = sys_container_of(block->node.parent, struct block_header_t, node);
    }
}

static void on_block_rotate(sys_tree_node_t *old_root, sys_tree_node_t *new_root, void *arg)
{
    update_block_max(sys_container_of(old_root, struct block_header_t, node));
    update_block_max(sys_container_of(new_root, struct block_header_t, node));
}

static void insert_block(struct page_header_t *page, struct block_header_t *block)
{
	sys_trace();
	sys_insert_node_ex(&page->root, &block->node, on_compare_block, NULL, on_block_rotate, NULL);
	update_block_to_root(block);
}

static void delete_block(struct page_header_t *page, struct block_header_t *block)
{
	sys_trace();
	struct block_header_t *parent_block = NULL;
	struct block_header_t *after_block = NULL;
	struct block_header_t *after_parent_block = NULL;

	if (block->node.parent != NULL)
	{
		parent_block = sys_container_of(block->node.parent, struct block_header_t, node);
	}

	if (block->node.left != &g_leaf_node && block->node.right != &g_leaf_node)
	{
		sys_tree_node_t *after = sys_get_left_most_node(block->node.right);
		after_block = sys_container_of(after, struct block_header_t, node);

		if (after->parent != NULL && after->parent != &block->node)
		{
			after_parent_block = sys_container_of(after->parent, struct block_header_t, node);
		}
	}

	sys_delete_node_ex(&page->root, &block->node, on_block_rotate, NULL);

	if (after_parent_block != NULL)
	{
		update_block_to_root(after_parent_block);
	}
	else if (after_block != NULL)
	{
		update_block_to_root(after_block);
	}
	else if (parent_block != NULL)
	{
		update_block_to_root(parent_block);
	}
}

static struct block_header_t *split_block(struct page_header_t *page, struct block_header_t *block, size_t size)
{
	sys_trace();
	delete_block(page, block);
	if(block->size >= size + size_align(sizeof(struct block_header_t)))
	{
		struct block_header_t *new_block = (struct block_header_t *)((int8_t *)block + size);
		new_block->size = block->size - size;
		new_block->max_block_size = new_block->size;
		insert_block(page, new_block);
		block->size = size;
	}
	page->alloc_count++;
	update_page(page);
	return block;
}

static void *block_to_mem(struct page_header_t *page, struct block_header_t *block)
{
	sys_trace();
	size_t size = block->size;
	struct mem_header_t *header = (struct mem_header_t *)block;
	header->header = page;
	header->size = size | MEM_ALLOC;
	return (int8_t *)header + size_align(sizeof(struct mem_header_t));
}

static int on_compare_page(void *key1, void *key2, void *arg)
{
	sys_trace();
	struct page_header_t *page1 = sys_container_of(key1, struct page_header_t, node);
	struct page_header_t *page2 = sys_container_of(key2, struct page_header_t, node);
	if (page1->alloc_count < page2->alloc_count)
	{
		return -1;
	}
	else if (page1->alloc_count >= page2->alloc_count)
	{
		return 1;
	}
	return 0;
}

static void insert_page(sys_mem_manager_t *mem_manager, struct page_header_t *page)
{
	sys_trace();
	sys_insert_node_ex(&mem_manager->root, &page->node, on_compare_page, NULL, on_page_rotate, NULL);
	update_page_to_root(page);
}

void *sys_mem_manager_alloc(sys_mem_manager_t *mem_manager, size_t size)
{
	sys_trace();
	size_t new_size = alloc_size(size);
	if (new_size == 0)
	{
		return NULL;
	}
	if (new_size > mem_manager->free_mem)
	{
		return NULL;
	}
	struct page_header_t *page = find_fit_page(sys_container_of(mem_manager->root, struct page_header_t, node), new_size);
	if (NULL == page)
	{
		page = alloc_page(mem_manager, new_size);
	}
	else
	{
		delete_page(mem_manager, page);
	}
	if (NULL == page)
	{
		return NULL;
	}
	struct block_header_t *block = find_fit_block(sys_container_of(page->root, struct block_header_t, node), new_size);
	block = split_block(page, block, new_size);
	insert_page(mem_manager, page);
	mem_manager->free_mem -= block->size;
	return block_to_mem(page, block);
}

static struct mem_header_t *addr_to_mem(void *address)
{
	sys_trace();
	struct mem_header_t *mem = (struct mem_header_t *)((int8_t *)address - size_align(sizeof(struct mem_header_t)));
	sys_assert((mem->size & MEM_SIZE_MASK) == MEM_ALLOC && mem->header->magic == PAGE_MAGIC);
	if ((mem->size & MEM_SIZE_MASK) == MEM_ALLOC && mem->header->magic == PAGE_MAGIC)
	{
		mem->size &= ~MEM_SIZE_MASK;
		return mem;
	}
	return NULL;
}

static void *mem_to_addr(struct mem_header_t *mem)
{
	sys_trace();
	sys_assert(mem->header->magic == PAGE_MAGIC);
	if (mem->header->magic == PAGE_MAGIC)
	{
		mem->size |= MEM_ALLOC;
		return (int8_t *)mem + size_align(sizeof(struct mem_header_t));
	}
	return NULL;
}

static void bind_block(struct page_header_t *page, struct block_header_t *block)
{
	struct block_header_t *prev =  sys_container_of(sys_get_right_most_node(block->node.left), struct block_header_t, node);
	if (NULL == prev)
	{
		prev = sys_container_of(sys_get_prev_node(&block->node), struct block_header_t, node);
	}
	struct block_header_t *next =  sys_container_of(sys_get_left_most_node(block->node.right), struct block_header_t, node);
	if (NULL == next)
	{
		next = sys_container_of(sys_get_next_node(&block->node), struct block_header_t, node);
	}
	
	if (next != NULL && (int8_t *)block + block->size == (int8_t *)next)
	{
		delete_block(page, next);
		block->size += next->size;
		block->max_block_size = block->size;
	}
	if (prev != NULL && (int8_t *)prev + prev->size == (int8_t *)block)
	{
		delete_block(page, block);
		prev->size += block->size;
		prev->max_block_size = prev->size;
		update_block_to_root(prev);
	}
	else
	{
		update_block_to_root(block);
	}
}

static void *shrink_mem(sys_mem_manager_t *mem_manager, struct mem_header_t *mem, size_t new_block_size)
{
	sys_trace();
	size_t old_block_size = mem->size;
	sys_assert(new_block_size < old_block_size);
	if (old_block_size - new_block_size < size_align(sizeof(struct block_header_t)))
	{
		return mem_to_addr(mem);
	}

	struct page_header_t *page = mem->header;
	delete_page(mem_manager, page);

	struct block_header_t *new_block = (struct block_header_t *)((int8_t *)mem + new_block_size);
	new_block->size = old_block_size - new_block_size;
	new_block->max_block_size = new_block->size;
	insert_block(page, new_block);
	bind_block(page, new_block);

	mem->size = new_block_size;
	mem_manager->free_mem += old_block_size - new_block_size;
	update_page(page);
	insert_page(mem_manager, page);
	return mem_to_addr(mem);
}

void sys_mem_manager_free(sys_mem_manager_t *mem_manager, void *address)
{
	sys_trace();
	if (NULL == address)
	{
		return;
	}
	struct mem_header_t *mem = addr_to_mem(address);
	if (NULL == mem)
	{
		return;
	}
	struct page_header_t *page = mem->header;
	delete_page(mem_manager, page);
	size_t size = mem->size;
	struct block_header_t *block = (struct block_header_t *)mem;
	block->size = size;
	block->max_block_size = size;
	insert_block(page, block);
	bind_block(page, block);
	page->alloc_count--;
	if (page->alloc_count > 0)
	{
		update_page(page);
		insert_page(mem_manager, page);
	}
	else
	{
		page->magic = 0;
		sys_buddy_free_pages(&mem_manager->page_factory, page);
		mem_manager->free_mem += size_align(sizeof(struct page_header_t));
	}
	mem_manager->free_mem += size;
}

void *sys_mem_manager_realloc(sys_mem_manager_t *mem_manager, void *address, size_t new_size)
{
	sys_trace();
	if (NULL == address)
	{
		return sys_mem_manager_alloc(mem_manager, new_size);
	}
	else if (0 == new_size)
	{
		sys_mem_manager_free(mem_manager, address);
		return NULL;
	}
	else
	{
		struct mem_header_t *mem = addr_to_mem(address);
		if (NULL == mem)
		{
			return NULL;
		}
		size_t new_block_size = alloc_size(new_size);
		if (new_block_size == 0)
		{
			mem_to_addr(mem);
			return NULL;
		}
		if (new_block_size < mem->size)
		{
			return shrink_mem(mem_manager, mem, new_block_size);
		}
		if (new_block_size == mem->size)
		{
			return mem_to_addr(mem);
		}
		void *new_alloc = sys_mem_manager_alloc(mem_manager, new_size);
		if (NULL == new_alloc)
		{
			mem_to_addr(mem);
			return NULL;
		}
		size_t old_size = mem->size - size_align(sizeof(struct mem_header_t));
		sys_memcpy(new_alloc, address, old_size < new_size ? old_size : new_size);
		sys_mem_manager_free(mem_manager, mem_to_addr(mem));
		return new_alloc;
	}
}

size_t sys_mem_manager_usable_size(sys_mem_manager_t *mem_manager, void *address)
{
	sys_trace();
	if (NULL == address)
	{
		return 0;
	}
	struct mem_header_t *mem = addr_to_mem(address);
	if (NULL == mem)
	{
		return 0;
	}
	size_t size = mem->size - size_align(sizeof(struct mem_header_t));
	mem_to_addr(mem);
	return size;
}

void *sys_mem_manager_alloc_pages(sys_mem_manager_t *mem_manager, size_t n)
{
	sys_trace();
	size_t current_page_num = mem_manager->page_factory.free_page_num;
	void *pages = sys_buddy_alloc_pages(&mem_manager->page_factory, n);
	if (NULL == pages)
	{
		return NULL;
	}
	mem_manager->free_mem -= (current_page_num - mem_manager->page_factory.free_page_num) * SYS_BUDDY_PAGE_SIZE;
	return pages;
}

void sys_mem_manager_free_pages(sys_mem_manager_t *mem_manager, void *pages)
{
	sys_trace();
	size_t current_page_num = mem_manager->page_factory.free_page_num;
	sys_buddy_free_pages(&mem_manager->page_factory, pages);
	mem_manager->free_mem += (mem_manager->page_factory.free_page_num - current_page_num) * SYS_BUDDY_PAGE_SIZE;
}

size_t sys_mem_manager_total_mem(sys_mem_manager_t *mem_manager)
{
	sys_trace();
	return mem_manager->total_mem;
}

size_t sys_mem_manager_free_mem(sys_mem_manager_t *mem_manager)
{
	sys_trace();
	return mem_manager->free_mem;
}

size_t sys_mem_manager_total_page(sys_mem_manager_t *mem_manager)
{
	sys_trace();
	return mem_manager->page_factory.total_page_num;
}

size_t sys_mem_manager_free_page(sys_mem_manager_t *mem_manager)
{
	sys_trace();
	return mem_manager->page_factory.free_page_num;
}
