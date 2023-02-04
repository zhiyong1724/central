#include "shellio.h"
#include "shell.h"
#include "ostask.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "osmsgqueue.h"
#include "led.h"
static Shell sShell;
static char sShellBuffer[1024];
char gShellPathBuffer[OS_MAX_FILE_PATH_LENGTH] = {'/', '\0'};
static char sBuffer;
static OsMsgQueue sQueue;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  osMsgQueueSend(&sQueue, &sBuffer);
  MX_USART1_UART_Receive(&sBuffer, 1);
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback can be implemented in the user file.
   */
}

static short shellRead(char *data, unsigned short len)
{
    if (osMsgQueueReceive(&sQueue, data, OS_MESSAGE_MAX_WAIT_TIME) == 0)
    {
        return 1;
    }
    return 0;
}

static short shellWrite(char *data, unsigned short len)
{
    MX_USART1_UART_Transmit(data, len);
    return 0;
}

static void *_shellTask(void *arg)
{
    while (1)
    {
        shellTask(arg);
    }
    return NULL;
}

int shellIOInit()
{
    sShell.read = shellRead;
    sShell.write = shellWrite;
    shellInit(&sShell, sShellBuffer, 1024);
    shellSetPath(&sShell, gShellPathBuffer);
    osMsgQueueCreate(&sQueue, OS_MAX_QUEUE_LENGTH, 1);
    os_tid_t tid;
    osTaskCreate(&tid, _shellTask, &sShell, "shell", 0, OS_DEFAULT_TASK_STACK_SIZE);
    MX_USART1_UART_Receive(&sBuffer, 1);
    return 0;
}