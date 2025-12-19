#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include "uart16550.h"
#include "sys_vfs.h"
#include "sys_string.h"
int _access(const char *file, int mode)
{
  errno = EPERM;
  return -1;
}

int _faccessat(int dirfd, const char *file, int mode, int flags)
{
  errno = EPERM;
  return -1;
}

void _exit(int exit_status)
{
  int x = exit_status / INT_MAX;
  x = 4 / x;
  for (;;)
    ;
}

void *_sbrk(long incr)
{
  extern char _user_heap; /* Set by linker.  */
  static char *heap_end;
  char *prev_heap_end;

  if (heap_end == 0)
    heap_end = &_user_heap;

  prev_heap_end = heap_end;
  heap_end += incr;

  return (void *)prev_heap_end;
}

int _open(const char *name, int flags, int mode)
{
  int res = sys_open(name, flags, mode);
  if (res < 0)
    return res;
  return res + 3;
}

int _openat(int dirfd, const char *name, int flags, int mode)
{
  errno = EPERM;
  return -1;
}

ssize_t _read(int file, void *ptr, size_t len)
{
  ssize_t ret = 0;
  if (STDIN_FILENO == file)
  {
    int res;
    char *s = (char *)ptr;
    while ((res = uart16550_getchar()) < 0);
    s[0] = (char)res;
    ret = 1;
  }
  else if (file >= 3)
  {
    ret = sys_read(file - 3, ptr, len);
  }
  return ret;
}

ssize_t _write(int file, const void *ptr, size_t len)
{
  ssize_t ret = 0;
  if (STDOUT_FILENO == file)
  {
    const char *s = (const char *)ptr;
    for (size_t i = 0; i < len; i++)
    {
      uart16550_putchar(s[i]);
    }
    ret = len;
  }
  else if (STDERR_FILENO == file)
  {
    const char *f = "\033[1;31m";
    for (size_t i = 0; i < 7; i++)
    {
      uart16550_putchar(f[i]);
    }
    const char *s = (const char *)ptr;
    for (size_t i = 0; i < len; i++)
    {
      uart16550_putchar(s[i]);
    }
    const char *e = "\033[0m";
    for (size_t i = 0; i < 4; i++)
    {
      uart16550_putchar(e[i]);
    }
    ret = len;
  }
  else if (file >= 3)
  {
    ret = sys_write(file - 3, ptr, len);
  }
  return ret;
}

int _close(int file)
{
  if (file >= 3)
  {
    return sys_close(file - 3);
  }
  errno = EPERM;
  return -1;
}

int _fstat(int file, struct stat *st)
{
  if (file <= STDERR_FILENO)
  {
    st->st_mode = S_IFCHR;
    return 0;
  }
  errno = EPERM;
  return -1;
}

int _fstatat(int dirfd, const char *file, struct stat *st, int flags)
{
  errno = EPERM;
  return -1;
}

int _stat(const char *file, struct stat *st)
{
  struct vfs_stat_t _st;
  sys_memset(&_st, 0, sizeof(_st));
  int ret = sys_stat(file, &_st);
  if (0 == ret)
  {
    st->st_dev = _st.st_dev;
    st->st_ino = _st.st_ino;
    st->st_mode = _st.st_mode;
    st->st_nlink = _st.st_nlink;
    st->st_size = _st.st_size;
    st->st_blksize = _st.st_blksize;
    st->st_blocks = _st.st_blocks;
    st->st_atime = _st.st_atim;
    st->st_mtime = _st.st_mtim;
    st->st_ctime = _st.st_ctim;
  }
  return ret;
}

int _link(const char *old_name, const char *new_name)
{
  return sys_link(old_name, new_name);
}

int _unlink(const char *name)
{
  return sys_unlink(name);
}

off_t _lseek(int file, off_t ptr, int dir)
{
  return sys_lseek(file, ptr, dir);
}

int _lstat(const char *file, struct stat *st)
{
  return _stat(file, st);
}
