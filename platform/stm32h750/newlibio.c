#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include "usart.h"
#include <fcntl.h>
#include "osf.h"
#include "sys_mem.h"
#include <string.h>
#include "led.h"
#include "sys_lock.h"
#include <sys/times.h>
#include <unistd.h>
#include "sys_msg_queue.h"
static sys_recursive_lock_t s_malloc_mutex;
static int s_malloc_mutexInit = 0;
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
    openMode |= SYS_FILE_MODE_WRITE;
  }
  else
  {
    openMode |= SYS_FILE_MODE_READ;
  }
  if ((flags & O_RDWR) > 0)
  {
    openMode |= SYS_FILE_MODE_READ | SYS_FILE_MODE_WRITE;
  }

  if ((flags & O_CREAT) > 0)
  {
    if ((flags & O_TRUNC) > 0)
    {
      openMode |= SYS_FILE_MODE_CREATE_ALWAYS;
    }
    else if ((flags & O_APPEND) > 0)
    {
      openMode |= SYS_FILE_MODE_OPEN_ALWAYS | SYS_FILE_MODE_OPEN_APPEND;
    }
  }
  else
  {
    openMode |= SYS_FILE_MODE_OPEN_EXISTING;
  }
  sys_file_t *osFile = (sys_file_t *)sys_malloc(sizeof(sys_file_t));
  if (osFile > 0)
  {
    if (osFOpen(osFile, file, openMode) == 0)
    {
      return addressToShort(osFile);
    }
    else
    {
      sys_free(osFile);
      errno = ENOENT;
      return -1;
    }
  }
  else
  {
    errno = ENOMEM;
    return -1;
  }
}

int _close(int fildes)
{
  int ret = -1;
  if (fildes > STDERR_FILENO)
  {
    sys_file_t *osFile = (sys_file_t *)shortToAddress((short)fildes);
    if (osFile != NULL)
    {
      sys_file_error_t result = osFClose(osFile);
      if (SYS_FILE_ERROR_OK == result)
      {
        sys_free(osFile);
        ret = 0;
      }
      else
      {
        errno = ENXIO;
      }
    }
  }
  return ret;
}

static int sInBuffQueueInit = 0;
static sys_msg_queue_t sInBuffQueue;
static char sInBuff;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  UNUSED(huart);
  MX_USART1_UART_Receive(&sInBuff, 1);
  sys_msg_queue_send(&sInBuffQueue, &sInBuff);
}

int _read(int file, char *ptr, int len)
{
  int ret = 0;
  if (STDIN_FILENO == file)
  {
    if (0 == sInBuffQueueInit)
    {
      sInBuffQueueInit = 1;
      sys_msg_queue_create(&sInBuffQueue, 256, 1);
      MX_USART1_UART_Receive(&sInBuff, 1);
    }
    sys_msg_queue_receive(&sInBuffQueue, ptr, SYS_MESSAGE_MAX_WAIT_TIME);
    ret = 1;
  }
  else if (file > STDERR_FILENO)
  {
    sys_file_t *osFile = (sys_file_t *)shortToAddress((short)file);
    if (osFile != NULL)
    {
      uint64_t length = 0;
      sys_file_error_t result = osFRead(osFile, ptr, len, &length);
      if (SYS_FILE_ERROR_OK == result)
      {
        ret = (int)length;
      }
      else
      {
        errno = ENXIO;
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
    sys_file_t *osFile = (sys_file_t *)shortToAddress((short)file);
    if (osFile != NULL)
    {
      uint64_t length = 0;
      sys_file_error_t result = osFWrite(osFile, ptr, len, &length);
      if (SYS_FILE_ERROR_OK == result)
      {
        ret = (int)length;
      }
      else
      {
        errno = ENXIO;
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
    sys_seek_type_t seekType = SYS_SEEK_TYPE_SET;
    switch (dir)
    {
    case SEEK_SET:
      seekType = SYS_SEEK_TYPE_SET;
      break;
    case SEEK_CUR:
      seekType = SYS_SEEK_TYPE_CUR;
      break;
    case SEEK_END:
      seekType = SYS_SEEK_TYPE_END;
      break;
    default:
      break;
    }
    sys_file_t *osFile = (sys_file_t *)shortToAddress((short)file);
    if (osFile != NULL)
    {
      sys_file_error_t result = osFSeek(osFile, ptr, seekType);
      if (SYS_FILE_ERROR_OK == result)
      {
        uint64_t cur = 0;
        osFTell(osFile, &cur);
        ret = (int)cur;
      }
      else
      {
        errno = ENXIO;
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
    sys_file_t *osFile = (sys_file_t *)shortToAddress((short)file);
    if (osFile != NULL)
    {
      st->st_mode = S_IFBLK;
      uint64_t cur = 0;
      int result = osFTell(osFile, &cur);
      result = osFSeek(osFile, 0, SYS_SEEK_TYPE_END);
      uint64_t size = 0;
      result = osFTell(osFile, &size);
      st->st_size = (__off_t)size;
      result = osFSeek(osFile, cur, SYS_SEEK_TYPE_SET);
      if (SYS_FILE_ERROR_OK == result)
      {
        errno = ENXIO;
      }
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
  if (0 == s_malloc_mutexInit)
  {
    s_malloc_mutexInit = 1;
    sys_recursive_lock_create(&s_malloc_mutex);
  }
  sys_recursive_lock_lock(&s_malloc_mutex);
}

void __malloc_unlock(struct _reent *reent)
{
  if (0 == s_malloc_mutexInit)
  {
    s_malloc_mutexInit = 1;
    sys_recursive_lock_create(&s_malloc_mutex);
  }
  sys_recursive_lock_unlock(&s_malloc_mutex);
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
  sys_file_info_t file_info;
  int ret = osFStat(path, &file_info);
  if (SYS_FILE_ERROR_OK == ret)
  {
    errno = ENXIO;
  }
  st->st_mode = S_IFBLK;
  st->st_size = file_info.file_size;
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