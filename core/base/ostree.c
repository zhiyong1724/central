#include "ostree.h"
#include "osdefine.h"
#define ENABLE_TREE_LOG 0
#if ENABLE_TREE_LOG
#define treeLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define treeLog(format, ...) (void)0
#endif
#define BLACK 0
#define RED 1
OsTreeNode gLeafNode =
	{
		NULL,  //parent
		NULL,  //left_tree
		NULL,  //right_tree
		BLACK, //color
};

static void initNode(OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	node->color = RED;
	node->parent = NULL;
	node->leftTree = &gLeafNode;
	node->rightTree = &gLeafNode;
}

static OsTreeNode *getUncle(OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *parent = node->parent;
	OsTreeNode *grandparent = parent->parent;
	if (parent == grandparent->rightTree)
		return grandparent->leftTree;
	else
		return grandparent->rightTree;
}

static OsTreeNode *getSibling(OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *parent = node->parent;
	if (node == parent->leftTree)
		return parent->rightTree;
	else
		return parent->leftTree;
}

static void rotateLeft(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *parent = node->parent;
	OsTreeNode *rightTree = node->rightTree;
	if (parent != NULL)
	{
		if (parent->leftTree == node)
			parent->leftTree = rightTree;
		else
			parent->rightTree = rightTree;
	}
	else
	{
		*handle = rightTree;
	}
	rightTree->parent = parent;

	node->rightTree = rightTree->leftTree;
	rightTree->leftTree->parent = node;
	
	rightTree->leftTree = node;
	node->parent = rightTree;
}

static void rotateRight(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *parent = node->parent;
	OsTreeNode *leftTree = node->leftTree;
	if (parent != NULL)
	{
		if (parent->leftTree == node)
			parent->leftTree = leftTree;
		else
			parent->rightTree = leftTree;
	}
	else
	{
		*handle = leftTree;
	}
	leftTree->parent = parent;

	node->leftTree = leftTree->rightTree;
	leftTree->rightTree->parent = node;

	leftTree->rightTree = node;
	node->parent = leftTree;
}

static void insertCase(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *parent = node->parent;
	if (NULL == parent)
	{
		node->color = BLACK;
		return;
	}

	if (RED == parent->color)
	{
		OsTreeNode *grandparent = parent->parent;
		OsTreeNode *uncle = getUncle(node);
		if (RED == uncle->color)
		{
			parent->color = BLACK;
			uncle->color = BLACK;
			grandparent->color = RED;
			insertCase(handle, grandparent);
		}
		else
		{
			if (parent == grandparent->leftTree)
			{
				if (parent->rightTree == node)
				{
					rotateLeft(handle, parent);
					insertCase(handle, parent);
				}
				else
				{
					parent->color = BLACK;
					grandparent->color = RED;
					rotateRight(handle, grandparent);
				}
			}
			else
			{
				if (parent->leftTree == node)
				{
					rotateRight(handle, parent);
					insertCase(handle, parent);
				}
				else
				{
					parent->color = BLACK;
					grandparent->color = RED;
					rotateLeft(handle, grandparent);
				}
			}
		}
	}
}

int osInsertNode(OsTreeNode **handle, OsTreeNode *node, OsOnCompare callback, void *arg)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	initNode(node);
	if (NULL == *handle)
	{
		*handle = node;
	}
	else
	{
		OsTreeNode *curNode = *handle;
		int cmp = 0;
		for (;;)
		{
			cmp = callback(node, curNode, arg);
			if (cmp < 0)
			{
				if (curNode->leftTree == &gLeafNode)
				{
					break;
				}
				curNode = curNode->leftTree;
			}
			else if (cmp > 0)
			{
				if (curNode->rightTree == &gLeafNode)
				{
					break;
				}
				curNode = curNode->rightTree;
			}
			else
			{
				return -1;
			}
		}
		node->parent = curNode;
		if (cmp < 0)
			curNode->leftTree = node;
		else if (cmp > 0)
			curNode->rightTree = node;
	}
	insertCase(handle, node);
	return 0;
}

static void deleteCase(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *parent = node->parent;
	if (parent != NULL && BLACK == node->color)
	{
		OsTreeNode *sibling = getSibling(node);
		if (parent->leftTree == node)
		{
			if (RED == sibling->color)
			{
				parent->color = RED;
				sibling->color = BLACK;
				rotateLeft(handle, parent);
				deleteCase(handle, node);
			}
			else
			{
				if (RED == sibling->rightTree->color)
				{
					sibling->color = parent->color;
					parent->color = BLACK;
					sibling->rightTree->color = BLACK;
					rotateLeft(handle, parent);
				}
				else if (RED == sibling->leftTree->color)
				{
					sibling->color = RED;
					sibling->leftTree->color = BLACK;
					rotateRight(handle, sibling);
					deleteCase(handle, node);
				}
				else
				{
					sibling->color = RED;
					deleteCase(handle, parent);
				}
			}
		}
		else
		{
			if (RED == sibling->color)
			{
				parent->color = RED;
				sibling->color = BLACK;
				rotateRight(handle, parent);
				deleteCase(handle, node);
			}
			else
			{
				if (RED == sibling->leftTree->color)
				{
					sibling->color = parent->color;
					parent->color = BLACK;
					sibling->leftTree->color = BLACK;
					rotateRight(handle, parent);
				}
				else if (RED == sibling->rightTree->color)
				{
					sibling->color = RED;
					sibling->rightTree->color = BLACK;
					rotateLeft(handle, sibling);
					deleteCase(handle, node);
				}
				else
				{
					sibling->color = RED;
					deleteCase(handle, parent);
				}
			}
		}
	}
	else
	{
		node->color = BLACK;
	}
}

void osDeleteNode(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *leftTree = node->leftTree;
	OsTreeNode *rightTree = node->rightTree;
	if (leftTree != &gLeafNode)
	{
		if (rightTree != &gLeafNode)
		{
			OsTreeNode *afterNode = osGetLeftmostNode(rightTree);
			osDeleteNode(handle, afterNode);
			afterNode->color = node->color;
			OsTreeNode *parent = node->parent;
			OsTreeNode *leftTree = node->leftTree;
			OsTreeNode *rightTree = node->rightTree;
			if (parent != NULL)
			{
				if (parent->leftTree == node)
					parent->leftTree = afterNode;
				else
					parent->rightTree = afterNode;
			}
			else
			{
				*handle = afterNode;
			}
			afterNode->parent = parent;

			afterNode->leftTree = leftTree;
			leftTree->parent = afterNode;

			afterNode->rightTree = rightTree;
			rightTree->parent = afterNode;
		}
		else
		{
			deleteCase(handle, node);
			OsTreeNode *parent = node->parent;
			if (parent != NULL)
			{
				if (parent->leftTree == node)
					parent->leftTree = leftTree;
				else
					parent->rightTree = leftTree;
			}
			else
			{
				*handle = leftTree;
			}
			leftTree->parent = parent;
		}
	}
	else
	{
		if (rightTree != &gLeafNode)
		{
			deleteCase(handle, node);
			OsTreeNode *parent = node->parent;
			if (parent != NULL)
			{
				if (parent->leftTree == node)
					parent->leftTree = rightTree;
				else
					parent->rightTree = rightTree;
			}
			else
			{
				*handle = rightTree;
			}
			rightTree->parent = parent;
		}
		else
		{
			deleteCase(handle, node);
			OsTreeNode *parent = node->parent;
			if (parent != NULL)
			{
				if (parent->leftTree == node)
					parent->leftTree = &gLeafNode;
				else
					parent->rightTree = &gLeafNode;
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

OsTreeNode *osGetLeftmostNode(OsTreeNode *handle)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *curNode = NULL;
	if (handle != NULL)
	{
		curNode = handle;
		while (curNode->leftTree != &gLeafNode)
		{
			curNode = curNode->leftTree;
		}
	}
	return curNode;
}

OsTreeNode *osFindNode(OsTreeNode *handle, void *key, OsOnCompare callback, void *arg)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (handle != NULL)
	{
		for (;;)
		{
			int cmp = callback(key, handle, arg);
			if (0 == cmp)
			{
				return handle;
			}
			else if (cmp < 0)
			{
				if (handle->leftTree == &gLeafNode)
				{
					break;
				}
				handle = handle->leftTree;
			}
			else
			{
				if (handle->rightTree == &gLeafNode)
				{
					break;
				}
				handle = handle->rightTree;
			}
		}
	}
	return NULL;
}
