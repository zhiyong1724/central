#ifndef __LFS_ADAPTER_H__
#define __LFS_ADAPTER_H__
#include "sys_vfs.h"
#ifdef __cplusplus
extern "C" {
#endif
#define LFS_GET_PAGE_SIZE 0
#define LFS_GET_BLOCK_SIZE 1
#define LFS_GET_BLOCK_COUNT 2
#define LFS_ERASE 3
int register_lfs();
#ifdef __cplusplus
}
#endif
#endif