#include "downloadmode.h"
#include "led.h"
#include "key.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include "usart.h"
#include "xmodem.h"
#include "norflash.h"
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

void eraseFlash(unsigned int len)
{
  printf("Start erase flash...\n");
  unsigned int sector = len / 4096;
  if (len % 4096 > 0)
  {
    sector++;
  }
  for (unsigned int i = 0; i < sector; i++)
  {
    norflashSectorErase(i * 4096);
  }
  printf("Erase end...\n");
}

void writeFlash(unsigned char *data, unsigned int len)
{
  printf("Start write flash,length = %d byte...\n", len);
  for (unsigned int i = 0; i < len; i += 256)
  {
    norflashWriteData(i, (uint8_t *)data + i, 256);
    printf("Write %d%%...\r", i * 100 / len);
    fflush(stdout);
  }
  printf("Write 100%%...\r\n");
  printf("Write end...\n");
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
  unsigned char *data = (unsigned char *)0x60000000;
  int len = xmodemReceive(data, 0x800000);
  if (len > 0)
  {
    HAL_Delay(1000);
    eraseFlash(len);
    writeFlash(data, len);
  }
}