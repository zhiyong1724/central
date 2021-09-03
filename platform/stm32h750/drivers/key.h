#ifndef __KEY_H__
#define __KEY_H__
typedef enum KeyStatus
{
    KEY_STATUS_PRESS,
    KEY_STATUS_RELEASE,
} KeyStatus;
KeyStatus key0Status();
KeyStatus key1Status();
KeyStatus key2Status();
KeyStatus keyUpStatus();
#endif