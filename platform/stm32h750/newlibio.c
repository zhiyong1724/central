#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include "usart.h"

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
  return 1;
}

int _kill(int pid, int sig)
{
  if(pid == 1)
    _exit(sig);
  return 0;
}

int _open(char *file, int flags, int mode)
{
  return 0;
}

int _close(int fildes)
{
  return 0;
}

int _read(int file, char *ptr, int len)
{
  return 0;
}

int _write(int file, char *ptr, int len)
{
  MX_USART1_UART_Transmit(ptr, len);
  return len;
}

int _lseek(int file, int ptr, int dir)
{
  return 0;
}

int _fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  st->st_blksize = 0;
  return 0;
}

int _isatty(int file)
{
  return 1;
}
