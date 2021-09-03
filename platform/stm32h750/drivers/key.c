#include "key.h"
#include "stm32h7xx_hal.h"
KeyStatus key0Status()
{
    if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_3) == GPIO_PIN_RESET)
    {
        return KEY_STATUS_PRESS;
    }
    else
    {
        return KEY_STATUS_RELEASE;
    }
}

KeyStatus key1Status()
{
    if (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_2) == GPIO_PIN_RESET)
    {
        return KEY_STATUS_PRESS;
    }
    else
    {
        return KEY_STATUS_RELEASE;
    }
}

KeyStatus key2Status()
{
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
    {
        return KEY_STATUS_PRESS;
    }
    else
    {
        return KEY_STATUS_RELEASE;
    }
}

KeyStatus keyUpStatus()
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
    {
        return KEY_STATUS_PRESS;
    }
    else
    {
        return KEY_STATUS_RELEASE;
    }
}