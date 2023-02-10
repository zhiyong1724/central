#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include "usart.h"
#include <fcntl.h>
#include "osf.h"
#include "osmem.h"
#include <string.h>
#include "led.h"
#include "osmutex.h"
#include <sys/times.h>
#include <unistd.h>
#include "osmsgqueue.h"
static OsRecursiveMutex sMallocMutex;
static int sMallocMutexInit = 0;
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
  if (fildes > STDERR_FILENO)
  {
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
  }
  return ret;
}

static int sInBuffQueueInit = 0;
static OsMsgQueue sInBuffQueue;
static char sInBuff;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  UNUSED(huart);
  osMsgQueueSend(&sInBuffQueue, &sInBuff);
  MX_USART1_UART_Receive(&sInBuff, 1);
}

int _read(int file, char *ptr, int len)
{
  int ret = 0;
  if (STDIN_FILENO == file)
  {
    if (0 == sInBuffQueueInit)
    {
      sInBuffQueueInit = 1;
      osMsgQueueCreate(&sInBuffQueue, 256, 1);
      MX_USART1_UART_Receive(&sInBuff, 1);
    }
    osMsgQueueReceive(&sInBuffQueue, ptr, OS_MESSAGE_MAX_WAIT_TIME);
    ret = 1;
  }
  else if (file > STDERR_FILENO)
  {
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
  }
  return ret;
}

int _write(int file, char *ptr, int len)
{
  int ret = 0;
  if (STDOUT_FILENO == file)
  {
    MX_USART1_UART_Transmit(ptr, len);
    ret = len;
  }
  else if (STDERR_FILENO == file)
  {
    MX_USART1_UART_Transmit("\033[1;31m", 7);
    MX_USART1_UART_Transmit(ptr, len);
    MX_USART1_UART_Transmit("\033[0m", 4);
    ret = len;
  }
  else if (file > STDERR_FILENO)
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
  int ret = -1;
  if (file > STDERR_FILENO)
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
    OsFile *osFile = (OsFile *)shortToAddress((short)file);
    if (osFile != NULL)
    {
      OsFileError result = osFSeek(osFile, ptr, seekType);
      if (OS_FILE_ERROR_OK == result)
      {
        uint64_t cur = 0;
        osFTell(osFile, &cur);
        ret = (int)cur;
      }
    }
  }
  return ret;
}

int _fstat(int file, struct stat *st)
{
  if (file <= STDERR_FILENO)
  {
    st->st_mode = S_IFCHR;
  }
  else
  {
    OsFile *osFile = (OsFile *)shortToAddress((short)file);
    if (osFile != NULL)
    {
      st->st_mode = S_IFBLK;
      uint64_t cur = 0;
      osFTell(osFile, &cur);
      osFSeek(osFile, 0, OS_SEEK_TYPE_END);
      uint64_t size = 0;
      osFTell(osFile, &size);
      st->st_size = (__off_t)size;
      osFSeek(osFile, cur, OS_SEEK_TYPE_SET);
    }
  }
  return 0;
}

int _isatty(int file)
{
  if (file <= STDERR_FILENO)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void __malloc_lock(struct _reent *reent)
{
  if (0 == sMallocMutexInit)
  {
    sMallocMutexInit = 1;
    osRecursiveMutexCreate(&sMallocMutex);
  }
  osRecursiveMutexLock(&sMallocMutex);
}

void __malloc_unlock(struct _reent *reent)
{
  if (0 == sMallocMutexInit)
  {
    sMallocMutexInit = 1;
    osRecursiveMutexCreate(&sMallocMutex);
  }
  osRecursiveMutexUnlock(&sMallocMutex);
}

clock_t _times(struct tms *buffer)
{
  return -1;
}

int _unlink(const char *path)
{
  return osFUnlink(path);
}

int _link()
{
  return -1;
}

int _stat(const char *path, struct stat *st)
{
  OsFileInfo fileInfo;
  int ret = osFStat(path, &fileInfo);
  st->st_mode = S_IFBLK;
  st->st_size = fileInfo.fileSize;
  return ret;
}

int mkdir(const char *__path, __mode_t __mode)
{
  return osFMkDir(__path);
}

int rmdir(const char *__path)
{
  return osFUnlink(__path);
}

int _gettimeofday(struct timeval *tp, void *tzvp)
{
  return 0;
}

FILE *popen (const char *program, const char *type)
{
	return NULL;
}

int pclose (FILE *iop)
{
	return 0;
}