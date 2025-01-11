#include "sys_list.h"
void sys_insert_to_single_list(sys_single_list_node_t **handle, sys_single_list_node_t *node)
{
	sys_trace();
	if (NULL == *handle)
	{
		node->next = node;
	}
	else
	{
		node->next = *handle;
	}
	*handle = node;
}

void sys_remove_from_single_list(sys_single_list_node_t **handle)
{
	sys_trace();
	sys_single_list_node_t *node = *handle;
	if (node->next == node)
	{
		*handle = NULL;
	}
	else
	{
		*handle = node->next;
	}
}

void sys_insert_to_front(sys_list_node_t **handle, sys_list_node_t *node)
{
	sys_trace();
	if (NULL == *handle)
	{
		node->pre = node;
		node->next = node;
	}
	else
	{
		node->pre = (*handle)->pre;
		node->next = *handle;
		(*handle)->pre = node;
		node->pre->next = node;
	}
	*handle = node;
}

void sys_insert_to_back(sys_list_node_t **handle, sys_list_node_t *node)
{
	sys_trace();
	if (NULL == *handle)
	{
		node->pre = node;
		node->next = node;
		*handle = node;
	}
	else
	{
		node->pre = (*handle)->pre;
		node->next = *handle;
		(*handle)->pre = node;
		node->pre->next = node;
	}
}

void sys_insert_to_middle(sys_list_node_t **handle, sys_list_node_t *node, int n)
{
	sys_trace();
	if (NULL == *handle)
	{
		node->pre = node;
		node->next = node;
		*handle = node;
	}
	else
	{
		sys_list_node_t *cur = *handle;
		for (int i = 0; i < n; i++)
		{
			cur = cur->next;
		}
		node->pre = cur->pre;
		node->next = cur;
		cur->pre = node;
		node->pre->next = node;
		if (0 == n)
		{
			*handle = node;
		}
	}
}

void sys_remove_from_list(sys_list_node_t **handle, sys_list_node_t *node)
{
	sys_trace();
	if (node->pre == node && node->next == node)
	{
		*handle = NULL;
	}
	else
	{
		node->pre->next =
			node->next;
		node->next->pre =
			node->pre;
		if (*handle == node)
		{
			*handle = node->next;
		}
	}
}

sys_list_node_t *sys_get_back_from_list(sys_list_node_t **handle)
{
	sys_trace();
	if (*handle != NULL)
	{
		return (*handle)->pre;
	}
	return NULL;
}

