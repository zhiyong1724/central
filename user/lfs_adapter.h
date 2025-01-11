#ifndef __LFS_ADAPTER_H__
#define __LFS_ADAPTER_H__
#include "sys_vfs.h"
#ifdef __cplusplus
extern "C" {
#endif
int lfs_format_ram();
int register_lfs();
#ifdef __cplusplus
}
#endif
#endif