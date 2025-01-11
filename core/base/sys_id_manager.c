#include "sys_id_manager.h"
#include "sys_bitmap_index.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "sys_error.h"
int sys_id_manager_init(sys_id_manager_t *id_manager)
{
	sys_trace();
	id_manager->id_table = (unsigned char *)sys_malloc(1);
	if (NULL == id_manager->id_table)
	{
		sys_error("Out of memory.");
		return SYS_ERROR_NOMEM;
	}
	*id_manager->id_table = 0;
	id_manager->table_level = 1;
	id_manager->table_size = 1;
	id_manager->max_id_count = 8;
	return 0;
}

void sys_id_manager_uninit(sys_id_manager_t *id_manager)
{
	sys_trace();
	if (id_manager->id_table != NULL)
	{
		sys_free(id_manager->id_table);
		id_manager->id_table = NULL;
	}
}

static unsigned char set_table(unsigned char *table, int id, int va, int offset, int level)
{
	sys_trace();
	int index = id >> 3 * level;
	unsigned char bit_offset = (id >> 3 * (level - 1)) % 8;
	unsigned char mark = 0x80;
	mark >>= bit_offset;
	level--;
	if (0 == level)
	{
		if (va > 0)
		{
			table[offset + index] |= mark;
		}
		else
		{
			mark = ~mark;
			table[offset + index] &= mark;
		}
	}
	else
	{
		if (set_table(table, id, va, (offset << 3) + 1, level) < 0xff)
		{
			mark = ~mark;
			table[offset + index] &= mark;
			
		}
		else
		{
			table[offset + index] |= mark;
		}
	}
	return table[offset + index];
}

static int expand_table(sys_id_manager_t *id_manager)
{
	sys_trace();
	int new_size = (id_manager->table_size << 3) + 1;
	unsigned char *newTable = (unsigned char *)sys_malloc(new_size);
	if (NULL == newTable)
	{
		sys_error("Out of memory.");
		return SYS_ERROR_NOMEM;
	}
	sys_memset(newTable, 0, new_size);
	for (int i = 0; i < id_manager->max_id_count; i++)
	{
		set_table(newTable, i, 1, 0, id_manager->table_level + 1);
	}
	sys_free(id_manager->id_table);
	id_manager->id_table = newTable;
	id_manager->table_size = new_size;
	id_manager->table_level++;
	id_manager->max_id_count <<= 3;
	return 0;
}

static int lookup_table(unsigned char *table, int id, int offset, int level)
{
	sys_trace();
	int va = g_bitmap_index[table[offset + id]];
	if (va < 8)
	{
		level--;
		id = (id << 3) + va;
		if (0 == level)
		{
			return id;
		}
		else
		{
			return lookup_table(table, id, (offset << 3) + 1, level);
		}
	}
	return -1;
}

int sys_id_alloc(sys_id_manager_t *id_manager)
{
	sys_trace();
	int ret = lookup_table(id_manager->id_table, 0, 0, id_manager->table_level);
	if (ret < 0)
	{
		ret = expand_table(id_manager);
		if (ret < 0)
		{
			return ret;
		}
		ret = lookup_table(id_manager->id_table, 0, 0, id_manager->table_level);
	}
	if (ret >= 0)
	{
		set_table(id_manager->id_table, ret, 1, 0, id_manager->table_level);
	}
	return ret;
}

void sys_id_free(sys_id_manager_t *id_manager, int id)
{
	sys_trace();
	if (id < id_manager->max_id_count)
	{
		set_table(id_manager->id_table, id, 0, 0, id_manager->table_level);
	}
}

