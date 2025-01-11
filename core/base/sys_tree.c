#include "sys_tree.h"
#include "sys_cfg.h"
#define BLACK 0
#define RED 1
sys_tree_node_t g_leaf_node =
	{
		NULL,  //parent
		NULL,  //left_tree
		NULL,  //right_tree
		BLACK, //color
};

static void init_node(sys_tree_node_t *node)
{
	sys_trace();
	node->color = RED;
	node->parent = NULL;
	node->left = &g_leaf_node;
	node->right = &g_leaf_node;
}

static sys_tree_node_t *get_uncle(sys_tree_node_t *node)
{
	sys_trace();
	sys_tree_node_t *parent = node->parent;
	sys_tree_node_t *grandparent = parent->parent;
	if (parent == grandparent->right)
		return grandparent->left;
	else
		return grandparent->right;
}

static sys_tree_node_t *get_sibling(sys_tree_node_t *node)
{
	sys_trace();
	sys_tree_node_t *parent = node->parent;
	if (node == parent->left)
		return parent->right;
	else
		return parent->left;
}

static void rotate_left(sys_tree_node_t **handle, sys_tree_node_t *node)
{
	sys_trace();
	sys_tree_node_t *parent = node->parent;
	sys_tree_node_t *right = node->right;
	if (parent != NULL)
	{
		if (parent->left == node)
			parent->left = right;
		else
			parent->right = right;
	}
	else
	{
		*handle = right;
	}
	right->parent = parent;

	node->right = right->left;
	right->left->parent = node;
	
	right->left = node;
	node->parent = right;
}

static void rotate_right(sys_tree_node_t **handle, sys_tree_node_t *node)
{
	sys_trace();
	sys_tree_node_t *parent = node->parent;
	sys_tree_node_t *left = node->left;
	if (parent != NULL)
	{
		if (parent->left == node)
			parent->left = left;
		else
			parent->right = left;
	}
	else
	{
		*handle = left;
	}
	left->parent = parent;

	node->left = left->right;
	left->right->parent = node;

	left->right = node;
	node->parent = left;
}

static void insert_case(sys_tree_node_t **handle, sys_tree_node_t *node)
{
	sys_trace();
	sys_tree_node_t *parent = node->parent;
	if (NULL == parent)
	{
		node->color = BLACK;
		return;
	}

	if (RED == parent->color)
	{
		sys_tree_node_t *grandparent = parent->parent;
		sys_tree_node_t *uncle = get_uncle(node);
		if (RED == uncle->color)
		{
			parent->color = BLACK;
			uncle->color = BLACK;
			grandparent->color = RED;
			insert_case(handle, grandparent);
		}
		else
		{
			if (parent == grandparent->left)
			{
				if (parent->right == node)
				{
					rotate_left(handle, parent);
					insert_case(handle, parent);
				}
				else
				{
					parent->color = BLACK;
					grandparent->color = RED;
					rotate_right(handle, grandparent);
				}
			}
			else
			{
				if (parent->left == node)
				{
					rotate_right(handle, parent);
					insert_case(handle, parent);
				}
				else
				{
					parent->color = BLACK;
					grandparent->color = RED;
					rotate_left(handle, grandparent);
				}
			}
		}
	}
}

int sys_insert_node(sys_tree_node_t **handle, sys_tree_node_t *node, sys_tree_on_compare_t callback, void *arg)
{
	sys_trace();
	init_node(node);
	if (NULL == *handle)
	{
		*handle = node;
	}
	else
	{
		sys_tree_node_t *cur_node = *handle;
		int cmp = 0;
		for (;;)
		{
			cmp = callback(node, cur_node, arg);
			if (cmp < 0)
			{
				if (cur_node->left == &g_leaf_node)
				{
					break;
				}
				cur_node = cur_node->left;
			}
			else if (cmp > 0)
			{
				if (cur_node->right == &g_leaf_node)
				{
					break;
				}
				cur_node = cur_node->right;
			}
			else
			{
				return -1;
			}
		}
		node->parent = cur_node;
		if (cmp < 0)
			cur_node->left = node;
		else if (cmp > 0)
			cur_node->right = node;
	}
	insert_case(handle, node);
	return 0;
}

static void delete_case(sys_tree_node_t **handle, sys_tree_node_t *node)
{
	sys_trace();
	sys_tree_node_t *parent = node->parent;
	if (parent != NULL && BLACK == node->color)
	{
		sys_tree_node_t *sibling = get_sibling(node);
		if (parent->left == node)
		{
			if (RED == sibling->color)
			{
				parent->color = RED;
				sibling->color = BLACK;
				rotate_left(handle, parent);
				delete_case(handle, node);
			}
			else
			{
				if (RED == sibling->right->color)
				{
					sibling->color = parent->color;
					parent->color = BLACK;
					sibling->right->color = BLACK;
					rotate_left(handle, parent);
				}
				else if (RED == sibling->left->color)
				{
					sibling->color = RED;
					sibling->left->color = BLACK;
					rotate_right(handle, sibling);
					delete_case(handle, node);
				}
				else
				{
					sibling->color = RED;
					delete_case(handle, parent);
				}
			}
		}
		else
		{
			if (RED == sibling->color)
			{
				parent->color = RED;
				sibling->color = BLACK;
				rotate_right(handle, parent);
				delete_case(handle, node);
			}
			else
			{
				if (RED == sibling->left->color)
				{
					sibling->color = parent->color;
					parent->color = BLACK;
					sibling->left->color = BLACK;
					rotate_right(handle, parent);
				}
				else if (RED == sibling->right->color)
				{
					sibling->color = RED;
					sibling->right->color = BLACK;
					rotate_left(handle, sibling);
					delete_case(handle, node);
				}
				else
				{
					sibling->color = RED;
					delete_case(handle, parent);
				}
			}
		}
	}
	else
	{
		node->color = BLACK;
	}
}

void sys_delete_node(sys_tree_node_t **handle, sys_tree_node_t *node)
{
	sys_trace();
	sys_tree_node_t *left = node->left;
	sys_tree_node_t *right = node->right;
	if (left != &g_leaf_node)
	{
		if (right != &g_leaf_node)
		{
			sys_tree_node_t *after_node = sys_get_left_most_node(right);
			sys_delete_node(handle, after_node);
			after_node->color = node->color;
			sys_tree_node_t *parent = node->parent;
			sys_tree_node_t *left = node->left;
			sys_tree_node_t *right = node->right;
			if (parent != NULL)
			{
				if (parent->left == node)
					parent->left = after_node;
				else
					parent->right = after_node;
			}
			else
			{
				*handle = after_node;
			}
			after_node->parent = parent;

			after_node->left = left;
			left->parent = after_node;

			after_node->right = right;
			right->parent = after_node;
		}
		else
		{
			delete_case(handle, node);
			sys_tree_node_t *parent = node->parent;
			if (parent != NULL)
			{
				if (parent->left == node)
					parent->left = left;
				else
					parent->right = left;
			}
			else
			{
				*handle = left;
			}
			left->parent = parent;
		}
	}
	else
	{
		if (right != &g_leaf_node)
		{
			delete_case(handle, node);
			sys_tree_node_t *parent = node->parent;
			if (parent != NULL)
			{
				if (parent->left == node)
					parent->left = right;
				else
					parent->right = right;
			}
			else
			{
				*handle = right;
			}
			right->parent = parent;
		}
		else
		{
			delete_case(handle, node);
			sys_tree_node_t *parent = node->parent;
			if (parent != NULL)
			{
				if (parent->left == node)
					parent->left = &g_leaf_node;
				else
					parent->right = &g_leaf_node;
			}
			else
			{
				*handle = NULL;
			}
		}
	}
	if (*handle != NULL && NULL == (*handle)->parent)
	{
		(*handle)->color = BLACK;
	}
}

sys_tree_node_t *sys_get_left_most_node(sys_tree_node_t *handle)
{
	sys_trace();
	sys_tree_node_t *cur_node = NULL;
	if (handle != NULL)
	{
		cur_node = handle;
		while (cur_node->left != &g_leaf_node)
		{
			cur_node = cur_node->left;
		}
	}
	return cur_node;
}

sys_tree_node_t *sys_find_node(sys_tree_node_t *handle, void *key, sys_tree_on_compare_t callback, void *arg)
{
	sys_trace();
	if (handle != NULL)
	{
		for (;;)
		{
			int cmp = callback(handle, key, arg);
			if (0 == cmp)
			{
				return handle;
			}
			else if (cmp < 0)
			{
				if (handle->left == &g_leaf_node)
				{
					break;
				}
				handle = handle->left;
			}
			else
			{
				if (handle->right == &g_leaf_node)
				{
					break;
				}
				handle = handle->right;
			}
		}
	}
	return NULL;
}
