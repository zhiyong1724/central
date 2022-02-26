#ifndef __OSTREE_H__
#define __OSTREE_H__
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct OsTreeNode
{
	struct OsTreeNode *parent;
	struct OsTreeNode *leftTree;
	struct OsTreeNode *rightTree;
	int color;
} OsTreeNode;

typedef int (*OsOnCompare)(void *key1, void *key2, void *arg);
extern OsTreeNode gLeafNode;
/*********************************************************************************************************************
* 插入新节点
* handle 树根
* node：新节点
* callback：条件判断函数，小于0，新节点插入到左子树，大于0，新节点插入到右子树
* arg：传递到回调函数的参数
* return：0：插入成功
*********************************************************************************************************************/
int osInsertNode(OsTreeNode **handle, OsTreeNode *node, OsOnCompare callback, void *arg);
/*********************************************************************************************************************
* 删除节点
* handle 树根
* node：要删除的节点
*********************************************************************************************************************/
void osDeleteNode(OsTreeNode **handle, OsTreeNode *node);
/*********************************************************************************************************************
* 获得最左边的叶子节点
* handle 树根
*********************************************************************************************************************/
OsTreeNode *osGetLeftmostNode(OsTreeNode *handle);
/*********************************************************************************************************************
* 查找节点
* tree 树根
* key：key
* callback：条件判断函数
* arg：传递到回调函数的参数
* return：查找到的节点
*********************************************************************************************************************/
OsTreeNode *osFindNode(OsTreeNode *handle, void *key, OsOnCompare callback, void *arg);
#ifdef __cplusplus
}
#endif
#endif
