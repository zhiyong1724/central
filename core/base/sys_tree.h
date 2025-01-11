#ifndef __SYS_TREE_H__
#define __SYS_TREE_H__
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_tree_node_t
{
	struct sys_tree_node_t *parent;
	struct sys_tree_node_t *left;
	struct sys_tree_node_t *right;
	int color;
} sys_tree_node_t;

typedef int (*sys_tree_on_compare_t)(void *key1, void *key2, void *arg);
extern sys_tree_node_t g_leaf_node;
/*********************************************************************************************************************
* 插入新节点
* handle 树根
* node：新节点
* callback：条件判断函数，小于0，新节点插入到左子树，大于0，新节点插入到右子树
* arg：传递到回调函数的参数
* return：0：插入成功
*********************************************************************************************************************/
int sys_insert_node(sys_tree_node_t **handle, sys_tree_node_t *node, sys_tree_on_compare_t callback, void *arg);
/*********************************************************************************************************************
* 删除节点
* handle 树根
* node：要删除的节点
*********************************************************************************************************************/
void sys_delete_node(sys_tree_node_t **handle, sys_tree_node_t *node);
/*********************************************************************************************************************
* 获得最左边的叶子节点
* handle 树根
*********************************************************************************************************************/
sys_tree_node_t *sys_get_left_most_node(sys_tree_node_t *handle);
/*********************************************************************************************************************
* 查找节点
* tree 树根
* key：key
* callback：条件判断函数
* arg：传递到回调函数的参数
* return：查找到的节点
*********************************************************************************************************************/
sys_tree_node_t *sys_find_node(sys_tree_node_t *handle, void *key, sys_tree_on_compare_t callback, void *arg);
#ifdef __cplusplus
}
#endif
#endif
