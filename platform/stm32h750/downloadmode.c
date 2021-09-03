#include "downloadmode.h"
#include "led.h"
#include "key.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include "usart.h"
#include "xmodem.h"
int _inbyte(unsigned short timeout)
{
  char data = 0;
  if (MX_USART1_UART_ReceiveTimeout(&data, 1, timeout) == 0)
  {
    return data;
  }
  else
  {
    return -1;
  }
}

void _outbyte(int c)
{
  char data = (char)c;
  MX_USART1_UART_Transmit(&data, 1);
}

void enterDownloadMode()
{
  printf("Start download mode...\n");
  led0ON();
  led1ON();
  while (keyUpStatus() == KEY_STATUS_RELEASE)
    ;
  led0OFF();
  led1OFF();
  unsigned char *data = (unsigned char *)0xc0000000;
  int len = xmodemReceive(data, 0x800000);
  if (len > 0)
  {
    /* code */
  }
}