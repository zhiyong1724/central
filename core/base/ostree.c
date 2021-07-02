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

static OsTreeNode *getGrandparent(OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (NULL == node->parent)
	{
		return NULL;
	}
	return node->parent->parent;
}

static OsTreeNode *getUncle(OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *grandparent = getGrandparent(node);
	if (NULL == grandparent)
	{
		return NULL;
	}
	if (node->parent == grandparent->rightTree)
		return grandparent->leftTree;
	else
		return grandparent->rightTree;
}

static OsTreeNode *getSibling(OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (NULL == node->parent)
	{
		return NULL;
	}
	if (node == node->parent->leftTree)
		return node->parent->rightTree;
	else
		return node->parent->leftTree;
}

static void rotateRight(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *grandparent = getGrandparent(node);
	OsTreeNode *parent = node->parent;
	OsTreeNode *rightTree = node->rightTree;

	parent->leftTree = rightTree;

	if (rightTree != &gLeafNode)
		rightTree->parent = parent;
	node->rightTree = parent;
	parent->parent = node;

	if (*handle == parent)
		*handle = node;
	node->parent = grandparent;

	if (grandparent != NULL)
	{
		if (grandparent->leftTree == parent)
			grandparent->leftTree = node;
		else
			grandparent->rightTree = node;
	}
}

static void rotateLeft(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *grandparent = getGrandparent(node);
	OsTreeNode *parent = node->parent;
	OsTreeNode *leftTree = node->leftTree;

	parent->rightTree = leftTree;

	if (leftTree != &gLeafNode)
		leftTree->parent = parent;
	node->leftTree = parent;
	parent->parent = node;

	if (*handle == parent)
		*handle = node;
	node->parent = grandparent;

	if (grandparent != NULL)
	{
		if (grandparent->leftTree == parent)
			grandparent->leftTree = node;
		else
			grandparent->rightTree = node;
	}
}

static void deleteCase(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (NULL == node->parent)
	{
		node->color = BLACK;
		return;
	}
	OsTreeNode *sibling = getSibling(node);
	if (RED == sibling->color)
	{
		node->parent->color = RED;
		sibling->color = BLACK;
		if (node == node->parent->leftTree)
			rotateLeft(handle, sibling);
		else
			rotateRight(handle, sibling);
		sibling = getSibling(node);
	}
	if (BLACK == node->parent->color && BLACK == sibling->color && BLACK == sibling->leftTree->color && BLACK == sibling->rightTree->color)
	{
		sibling->color = RED;
		deleteCase(handle, node->parent);
	}
	else if (RED == node->parent->color && BLACK == sibling->color && BLACK == sibling->leftTree->color && BLACK == sibling->rightTree->color)
	{
		sibling->color = RED;
		node->parent->color = BLACK;
	}
	else
	{
		if (BLACK == sibling->color)
		{
			if (node == node->parent->leftTree && RED == sibling->leftTree->color && BLACK == sibling->rightTree->color)
			{
				sibling->color = RED;
				sibling->leftTree->color = BLACK;
				rotateRight(handle, sibling->leftTree);
				sibling = getSibling(node);
			}
			else if (node == node->parent->rightTree && BLACK == sibling->leftTree->color && RED == sibling->rightTree->color)
			{
				sibling->color = RED;
				sibling->rightTree->color = BLACK;
				rotateLeft(handle, sibling->rightTree);
				sibling = getSibling(node);
			}
		}
		sibling->color = node->parent->color;
		node->parent->color = BLACK;
		if (node == node->parent->leftTree)
		{
			sibling->rightTree->color = BLACK;
			rotateLeft(handle, sibling);
		}
		else
		{
			sibling->leftTree->color = BLACK;
			rotateRight(handle, sibling);
		}
	}
}

static void insertCase(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (NULL == node->parent)
	{
		*handle = node;
		node->color = BLACK;
		return;
	}

	if (RED == node->parent->color)
	{
		OsTreeNode *grandparent = getGrandparent(node);
		OsTreeNode *uncle = getUncle(node);
		if (RED == uncle->color)
		{
			node->parent->color = BLACK;
			uncle->color = BLACK;
			grandparent->color = RED;
			insertCase(handle, grandparent);
		}
		else
		{
			if (node->parent->rightTree == node && grandparent->leftTree == node->parent)
			{
				rotateLeft(handle, node);
				rotateRight(handle, node);
				node->color = BLACK;
				node->leftTree->color = RED;
				node->rightTree->color = RED;
			}
			else if (node->parent->leftTree == node && grandparent->rightTree == node->parent)
			{
				rotateRight(handle, node);
				rotateLeft(handle, node);
				node->color = BLACK;
				node->leftTree->color = RED;
				node->rightTree->color = RED;
			}
			else if (node->parent->leftTree == node && grandparent->leftTree == node->parent)
			{
				node->parent->color = BLACK;
				grandparent->color = RED;
				rotateRight(handle, node->parent);
			}
			else if (node->parent->rightTree == node && grandparent->rightTree == node->parent)
			{
				node->parent->color = BLACK;
				grandparent->color = RED;
				rotateLeft(handle, node->parent);
			}
		}
	}
}

static void initNode(OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	node->color = RED;
	node->parent = NULL;
	node->leftTree = &gLeafNode;
	node->rightTree = &gLeafNode;
}

static void swapNode(OsTreeNode **handle, OsTreeNode *node1, OsTreeNode *node2)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode tempNode;
	tempNode.color = node2->color;
	tempNode.parent = node2->parent;
	tempNode.leftTree = node2->leftTree;
	tempNode.rightTree = node2->rightTree;
	if (node2 == node1->rightTree)
	{
		node2->color = node1->color;
		node2->parent = node1->parent;
		node2->leftTree = node1->leftTree;
		node2->rightTree = node1;
		if (node1->parent != NULL)
		{
			if (node1 == node1->parent->leftTree)
			{
				node1->parent->leftTree = node2;
			}
			else
			{
				node1->parent->rightTree = node2;
			}
		}
		else
		{
			*handle = node2;
		}
		node1->leftTree->parent = node2;

		node1->color = tempNode.color;
		node1->parent = node2;
		node1->leftTree = tempNode.leftTree;
		node1->rightTree = tempNode.rightTree;
		if (tempNode.leftTree != &gLeafNode)
		{
			tempNode.leftTree->parent = node1;
		}
		if (tempNode.rightTree != &gLeafNode)
		{
			tempNode.rightTree->parent = node1;
		}
	}
	else
	{
		node2->color = node1->color;
		node2->parent = node1->parent;
		node2->leftTree = node1->leftTree;
		node2->rightTree = node1->rightTree;
		if (node1->parent != NULL)
		{
			if (node1 == node1->parent->leftTree)
			{
				node1->parent->leftTree = node2;
			}
			else
			{
				node1->parent->rightTree = node2;
			}
		}
		else
		{
			*handle = node2;
		}
		if (node1->leftTree != &gLeafNode)
		{
			node1->leftTree->parent = node2;
		}
		if (node1->rightTree != &gLeafNode)
		{
			node1->rightTree->parent = node2;
		}
		node1->color = tempNode.color;
		node1->parent = tempNode.parent;
		node1->leftTree = tempNode.leftTree;
		node1->rightTree = tempNode.rightTree;
		tempNode.parent->leftTree = node1;
		if (tempNode.leftTree != &gLeafNode)
		{
			tempNode.leftTree->parent = node1;
		}
		if (tempNode.rightTree != &gLeafNode)
		{
			tempNode.rightTree->parent = node1;
		}
	}
}

static void doDeleteNode(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *child = node->leftTree == &gLeafNode ? node->rightTree : node->leftTree;
	if (node->parent == NULL && node->leftTree == &gLeafNode && node->rightTree == &gLeafNode)
	{
		*handle = NULL;
		return;
	}

	if (node->parent == NULL)
	{
		child->parent = NULL;
		*handle = child;
		(*handle)->color = BLACK;
		return;
	}

	if (node->parent->leftTree == node)
	{
		node->parent->leftTree = child;
	}
	else
	{
		node->parent->rightTree = child;
	}
	child->parent = node->parent;

	if (node->color == BLACK)
	{
		if (child->color == RED)
		{
			child->color = BLACK;
		}
		else
			deleteCase(handle, child);
	}
}

void osDeleteNode(OsTreeNode **handle, OsTreeNode *node)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (node->rightTree == &gLeafNode)
	{
		doDeleteNode(handle, node);
		return;
	}
	OsTreeNode *curNode = node->rightTree;
	while (curNode->leftTree != &gLeafNode)
	{
		curNode = curNode->leftTree;
	}
	swapNode(handle, node, curNode);
	doDeleteNode(handle, node);
}

OsTreeNode *osGetLeftmostNode(OsTreeNode *handle)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsTreeNode *curNode = NULL;
	if (handle != NULL && handle != &gLeafNode)
	{
		curNode = handle;
		while (curNode->leftTree != &gLeafNode)
		{
			curNode = curNode->leftTree;
		}
	}
	return curNode;
}

int osInsertNode(OsTreeNode **handle, OsTreeNode *node, OsOnCompare callback, void *arg)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	initNode(node);
	if (NULL == *handle)
	{
		node->color = BLACK;
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
		insertCase(handle, node);
	}
	return 0;
}

OsTreeNode *osFindNode(OsTreeNode *handle, void *key, OsOnCompare callback, void *arg)
{
	treeLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (handle != NULL && handle != &gLeafNode)
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
