#include "oslist.h"
#define ENABLE_LIST_LOG 0
#if ENABLE_LIST_LOG
#define listLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define listLog(format, ...) (void)0
#endif
void osInsertToSingleList(OsSingleListNode **handle, OsSingleListNode *node)
{
	listLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (NULL == *handle)
	{
		node->nextNode = node;
	}
	else
	{
		node->nextNode = *handle;
	}
	*handle = node;
}

void osRemoveFromSingleList(OsSingleListNode **handle)
{
	listLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	OsSingleListNode *node = *handle;
	if (node->nextNode == node)
	{
		*handle = NULL;
	}
	else
	{
		*handle = node->nextNode;
	}
}

void osInsertToFront(OsListNode **handle, OsListNode *node)
{
	listLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (NULL == *handle)
	{
		node->preNode = node;
		node->nextNode = node;
	}
	else
	{
		node->preNode = (*handle)->preNode;
		node->nextNode = *handle;
		(*handle)->preNode = node;
		node->preNode->nextNode = node;
	}
	*handle = node;
}

void osInsertToBack(OsListNode **handle, OsListNode *node)
{
	listLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (NULL == *handle)
	{
		node->preNode = node;
		node->nextNode = node;
		*handle = node;
	}
	else
	{
		node->preNode = (*handle)->preNode;
		node->nextNode = *handle;
		(*handle)->preNode = node;
		node->preNode->nextNode = node;
	}
}

void osRemoveFromList(OsListNode **handle, OsListNode *node)
{
	listLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (node->preNode == node
		&& node->nextNode == node)
	{
		*handle = NULL;
	}
	else
	{
		node->preNode->nextNode =
			node->nextNode;
		node->nextNode->preNode =
			node->preNode;
		if (*handle == node)
		{
			*handle = node->nextNode;
		}
	}
}

OsListNode *osGetBackFromList(OsListNode **handle)
{
	listLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	if (*handle != NULL)
	{
		return (*handle)->preNode;
	}
	return NULL;
}

