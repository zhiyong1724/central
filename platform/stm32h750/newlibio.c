#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include "usart.h"
#include <fcntl.h>
#include "osf.h"
#include "osmem.h"
#include <string.h>
static int sAddressMask = 0;
void _exit(int rc)
{
  int x = rc / INT_MAX;
  x = 4 / x;
  for (;;)
    ;
}

void *_sbrk(int incr)
{
  extern char end; /* Set by linker.  */
  static char *heap_end;
  char *prev_heap_end;

  if (heap_end == 0)
    heap_end = &end;

  prev_heap_end = heap_end;
  heap_end += incr;

  return (void *)prev_heap_end;
}

int _getpid(void)
{
  errno = ENOSYS;
  return -1;
}

int _kill(int pid, int sig)
{
  errno = ENOSYS;
  return -1;
}

static short addressToShort(void *address)
{
  int ret = (int)address;
  sAddressMask = ret & 0xffff0000;
  ret &= ~0xffff0000;
  ret /= sizeof(int);
  return (short)ret;
}

static void *shortToAddress(short num)
{
  int ret = (int)num;
  ret *= sizeof(int);
  ret |= sAddressMask;
  return (void *)ret;
}

int _open(char *file, int flags, int mode)
{
  uint32_t openMode = 0;
  if ((flags & O_WRONLY) > 0)
  {
    openMode |= OS_FILE_MODE_WRITE;
  }
  else
  {
    openMode |= OS_FILE_MODE_READ;
  }
  if ((flags & O_RDWR) > 0)
  {
    openMode |= OS_FILE_MODE_READ | OS_FILE_MODE_WRITE;
  }

  if ((flags & O_CREAT) > 0)
  {
    if ((flags & O_TRUNC) > 0)
    {
      openMode |= OS_FILE_MODE_CREATE_ALWAYS;
    }
    else if ((flags & O_APPEND) > 0)
    {
      openMode |= OS_FILE_MODE_OPEN_ALWAYS | OS_FILE_MODE_OPEN_APPEND;
    }
  }
  else
  {
    openMode |= OS_FILE_MODE_OPEN_EXISTING;
  }
  OsFile *osFile = (OsFile *)osMalloc(sizeof(OsFile));
  if (osFile > 0)
  {
    if (osFOpen(osFile, file, openMode) == 0)
    {
      return addressToShort(osFile);
    }
    else 
    {
      osFree(osFile);
      return -1;
    }
  }
  else 
  {
    return -1;
  }
}

int _close(int fildes)
{
  int ret = -1;
  OsFile *osFile = (OsFile *)shortToAddress((short)fildes);
  if (osFile != NULL)
  {
    OsFileError result = osFClose(osFile);
    if (OS_FILE_ERROR_OK == result)
    {
      osFree(osFile);
      ret = 0;
    }
  }
  return ret;
}

int _read(int file, char *ptr, int len)
{
  int ret = 0;
  OsFile *osFile = (OsFile *)shortToAddress((short)file);
  if (osFile != NULL)
  {
    uint64_t length = 0;
    OsFileError result = osFRead(osFile, ptr, len, &length);
    if (OS_FILE_ERROR_OK == result)
    {
      ret = (int)length;
    }
  }
  return ret;
}

int _write(int file, char *ptr, int len)
{
  int ret = 0;
  if (1 == file)
  {
    MX_USART1_UART_Transmit(ptr, len);
    ret = len;
  }
  else
  {
    OsFile *osFile = (OsFile *)shortToAddress((short)file);
    if (osFile != NULL)
    {
      uint64_t length = 0;
      OsFileError result = osFWrite(osFile, ptr, len, &length);
      if (OS_FILE_ERROR_OK == result)
      {
        ret = (int)length;
      }
    }
  }
  return ret;
}

int _lseek(int file, int ptr, int dir)
{
  OsSeekType seekType = OS_SEEK_TYPE_SET;
  switch (dir)
  {
  case SEEK_SET:
    seekType = OS_SEEK_TYPE_SET;
    break;
  case SEEK_CUR:
    seekType = OS_SEEK_TYPE_CUR;
    break;
  case SEEK_END:
    seekType = OS_SEEK_TYPE_END;
    break;
  default:
    break;
  }
  int ret = -1;
  OsFile *osFile = (OsFile *)shortToAddress((short)file);
  if (osFile != NULL)
  {
    OsFileError result = osFSeek(osFile, ptr, seekType);
    if (OS_FILE_ERROR_OK == result)
    {
      ret = 0;
    }
  }
  return ret;
}

int _fstat(int file, struct stat *st)
{
  errno = ENOSYS;
  return -1;
}

int _isatty(int file)
{
  errno = ENOSYS;
  return -1;
}
