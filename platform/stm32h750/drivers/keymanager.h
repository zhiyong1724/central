#ifndef __KEYMANAGER_H__
#define __KEYMANAGER_H__
#include "key.h"
#include "sys_list.h"
#include "sys_task.h"
#include "sys_lock.h"
typedef enum KeyType
{
    KEY_TYPE_KEY_0,
    KEY_TYPE_KEY_1,
    KEY_TYPE_KEY_2,
    KEY_TYPE_KEY_UP,
    KEY_TYPE_KEY_NUM,
} KeyType;

typedef struct Key
{
    KeyType type;
    KeyStatus status;
    int count;
} Key;

typedef struct KeyManagerCallBack
{
    sys_list_node_t node;
    int (*onPressed)(void *object, KeyType type);
    int (*onReleased)(void *object, KeyType type);
    void *object;
} KeyManagerCallBack;

int keyManagerInit();
int keyManagerUninit();
int keyManagerRegisterCallback(KeyManagerCallBack *callback);
int keyManagerRemoveCallback(KeyManagerCallBack *callback);
#endif