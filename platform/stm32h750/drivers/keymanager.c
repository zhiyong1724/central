#include "keymanager.h"
typedef struct KeyManager
{
    Key key[KEY_TYPE_KEY_NUM];
    OsListNode *callBackList;
    os_tid_t tid;
    OsMutex mutex;
    int running;
} KeyManager;

static KeyManager sKeyManager;

static void handleStatus(KeyManager *keyManager, KeyStatus keyStatus, KeyType keyType)
{
    if ((keyStatus != keyManager->key[keyType].status && 0 == keyManager->key[keyType].count) ||
        (keyStatus == keyManager->key[keyType].status && keyManager->key[keyType].count > 0))
    {
        keyManager->key[keyType].status = keyStatus;
        keyManager->key[keyType].count++;
        if (keyManager->key[keyType].count >= 5)
        {
            keyManager->key[keyType].count = 0;
            if (KEY_STATUS_PRESS == keyStatus)
            {
                osMutexLock(&keyManager->mutex);
                KeyManagerCallBack *callBack = (KeyManagerCallBack *)keyManager->callBackList;
                if (callBack != NULL)
                {
                    do
                    {
                        int result = callBack->onPressed(callBack->object, keyType);
                        if (result > 0)
                        {
                            break;
                        }
                        callBack = (KeyManagerCallBack *)keyManager->callBackList->nextNode;
                    } while (callBack != (KeyManagerCallBack *)keyManager->callBackList);
                }
                osMutexUnlock(&keyManager->mutex);
            }
            else
            {
                osMutexLock(&keyManager->mutex);
                KeyManagerCallBack *callBack = (KeyManagerCallBack *)keyManager->callBackList;
                if (callBack != NULL)
                {
                    do
                    {
                        int result = callBack->onReleased(callBack->object, keyType);
                        if (result > 0)
                        {
                            break;
                        }
                        callBack = (KeyManagerCallBack *)keyManager->callBackList->nextNode;
                    } while (callBack != (KeyManagerCallBack *)keyManager->callBackList);
                }
                osMutexUnlock(&keyManager->mutex);
            }
        }
    }
}

static void *keyTask(void *arg)
{
    KeyManager *keyManager = (KeyManager *)arg;
    while (keyManager->running > 0)
    {
        handleStatus(keyManager, key0Status(), KEY_TYPE_KEY_0);
        handleStatus(keyManager, key1Status(), KEY_TYPE_KEY_1);
        handleStatus(keyManager, key2Status(), KEY_TYPE_KEY_2);
        handleStatus(keyManager, keyUpStatus(), KEY_TYPE_KEY_UP);
        osTaskSleep(10);
    }
    return NULL;
}

int keyManagerInit()
{
    sKeyManager.key[KEY_TYPE_KEY_0].type = KEY_TYPE_KEY_0;
    sKeyManager.key[KEY_TYPE_KEY_0].status = KEY_STATUS_RELEASE;
    sKeyManager.key[KEY_TYPE_KEY_0].count = 0;


    sKeyManager.key[KEY_TYPE_KEY_1].type = KEY_TYPE_KEY_1;
    sKeyManager.key[KEY_TYPE_KEY_1].status = KEY_STATUS_RELEASE;
    sKeyManager.key[KEY_TYPE_KEY_1].count = 0;

    sKeyManager.key[KEY_TYPE_KEY_2].type = KEY_TYPE_KEY_2;
    sKeyManager.key[KEY_TYPE_KEY_2].status = KEY_STATUS_RELEASE;
    sKeyManager.key[KEY_TYPE_KEY_2].count = 0;

    sKeyManager.key[KEY_TYPE_KEY_UP].type = KEY_TYPE_KEY_UP;
    sKeyManager.key[KEY_TYPE_KEY_UP].status = KEY_STATUS_RELEASE;
    sKeyManager.key[KEY_TYPE_KEY_UP].count = 0;

    sKeyManager.callBackList = NULL;
    osMutexCreate(&sKeyManager.mutex);

    sKeyManager.running = 1;
    return osTaskCreate(&sKeyManager.tid, keyTask, &sKeyManager, "key manager", 0, OS_DEFAULT_TASK_STACK_SIZE);
}

int keyManagerUninit(KeyManager *keyManager)
{
    int ret = -1;
    if (keyManager->tid > 0)
    {
        keyManager->running = 0;
        void *result = NULL;
        osTaskJoin(&result, keyManager->tid);
        keyManager->tid = 0;
        keyManager->callBackList = NULL;
        ret = osMutexDestory(&keyManager->mutex);
    }
    return ret;   
}

int keyManagerRegisterCallback(KeyManagerCallBack *callback)
{
    osMutexLock(&sKeyManager.mutex);
    osInsertToBack(&sKeyManager.callBackList, &callback->node);
    osMutexUnlock(&sKeyManager.mutex);
    return 0;
}

int keyManagerRemoveCallback(KeyManagerCallBack *callback)
{
    osMutexLock(&sKeyManager.mutex);
    osRemoveFromList(&sKeyManager.callBackList, &callback->node);
    osMutexUnlock(&sKeyManager.mutex);
    return 0;
}