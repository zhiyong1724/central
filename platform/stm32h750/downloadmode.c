#include "downloadmode.h"
#include "led.h"
#include "key.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include "usart.h"
#include "xmodem.h"
#include "lfsio.h"
#include "lfs.h"
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
  while (1)
  {
    if (keyUpStatus() == KEY_STATUS_PRESS)
    {
      led0OFF();
      led1OFF();
      unsigned char *data = (unsigned char *)0x63800000;
      int len = xmodemReceive(data, 0x800000);
      HAL_Delay(100);
      if (len > 0)
      {
        printf("Start writing...\n");
        if (lfs_mount(&gLFS, &gLfsConfig) != LFS_ERR_OK)
        {
          lfs_format(&gLFS, &gLfsConfig);
          lfs_mount(&gLFS, &gLfsConfig);
        }
        lfs_file_t file;
        int ret = lfs_file_open(&gLFS, &file, "/system.bin", LFS_O_CREAT | LFS_O_TRUNC | LFS_O_WRONLY);
        if (LFS_ERR_OK == ret)
        {
          lfs_ssize_t size = lfs_file_write(&gLFS, &file, data, len);
          lfs_file_close(&gLFS, &file);
          printf("Write %ld byte.\n", size);
        }
        else
        {
          printf("Write fail.\n");
        }
      }
      break;
    }
    else if (key2Status() == KEY_STATUS_PRESS)
    {
      led0OFF();
      led1OFF();
      unsigned char *data = (unsigned char *)0x63800000;
      xmodemReceive(data, 0x800000);
      HAL_Delay(100);
      break;
    }
  }
}