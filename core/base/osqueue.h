#ifndef __OSQUEUE_H__
#define __OSQUEUE_H__
#include "osdefine.h"
#include "oslist.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct OsQueue
{
	OsListNode *queue;
	os_size_t unitSize;
	os_size_t size;
} OsQueue;
/*********************************************************************************************************************
* 初始化容器
* obj 容器对象
* unitSize 元素大小
* return 0：初始化成功
*********************************************************************************************************************/
int osQueueInit(OsQueue *obj, os_size_t unitSize);
/*********************************************************************************************************************
* 释放容器
* obj 容器对象
*********************************************************************************************************************/
void osQueueFree(OsQueue *obj);
/*********************************************************************************************************************
* 获取容器大小
* obj 容器对象
*********************************************************************************************************************/
os_size_t osQueueSize(OsQueue *obj);
/*********************************************************************************************************************
* 判断容器是否为空
* obj 容器对象
*********************************************************************************************************************/
int osQueueEmpty(OsQueue *obj);
/*********************************************************************************************************************
* 从后面添加元素
* obj 容器对象
* data 数据
* return 元素数目
*********************************************************************************************************************/
os_size_t osQueuePush(OsQueue *obj, void *data);
/*********************************************************************************************************************
* 访问最后的元素
* obj 容器对象
* return 返回的数据指针
*********************************************************************************************************************/
void *osQueueBack(OsQueue *obj);
/*********************************************************************************************************************
* 访问第一个元素
* obj 容器对象
* return 返回的数据指针
*********************************************************************************************************************/
void *osQueueFront(OsQueue *obj);
/*********************************************************************************************************************
* 清空所有元素
* obj 容器对象
*********************************************************************************************************************/
void osQueueClear(OsQueue *obj);
/*********************************************************************************************************************
* 从前面移除一个元素
* obj 容器对象
* return 0:成功移除
*********************************************************************************************************************/
int osQueuePop(OsQueue *obj);
#ifdef __cplusplus
}
#endif
#endif