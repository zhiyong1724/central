#ifndef __OSMEMORYFILESYSTEM_H__
#define __OSMEMORYFILESYSTEM_H__
#include "osdefine.h"
#include <stdint.h>
#include "ostree.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define OS_FILE_ATTR_OWNER_READ                0X00000100
#define OS_FILE_ATTR_OWNER_WRITE               0X00000080
#define OS_FILE_ATTR_OWNER_EXE                 0X00000040
#define OS_FILE_ATTR_GROUP_READ                0X00000020
#define OS_FILE_ATTR_GROUP_WRITE               0X00000010
#define OS_FILE_ATTR_GROUP_EXE                 0X00000008
#define OS_FILE_ATTR_OTHER_READ                0X00000004
#define OS_FILE_ATTR_OTHER_WRITE               0X00000002
#define OS_FILE_ATTR_OTHER_EXE                 0X00000001

#define	OS_FILE_MODE_READ				0x01
#define	OS_FILE_MODE_WRITE			    0x02
#define	OS_FILE_MODE_OPEN_EXISTING	    0x04
#define	OS_FILE_MODE_CREATE_NEW		    0x08
#define	OS_FILE_MODE_CREATE_ALWAYS	    0x10
#define	OS_FILE_MODE_OPEN_ALWAYS		0x20
#define	OS_FILE_MODE_OPEN_APPEND		0x40

typedef enum OsFileType
{
    OS_FILE_TYPE_NORMAL,
    OS_FILE_TYPE_DIRECTORY,
    OS_FILE_TYPE_LINK,
    OS_FILE_TYPE_BLOCK,
} OsFileType;

typedef struct OsFileNode
{
    OsTreeNode node;
    uint32_t type;
    uint32_t attribute;
    uint64_t createTime;
    uint64_t changeTime;
    uint64_t accessTime;
    uint32_t ownerNameSize;
    uint32_t groupNameSize;
    uint32_t fileNameSize;
    uint32_t fileSize;
    void *data;
} OsFileNode;

typedef struct OsFile
{
    uint32_t mode;
    OsFileNode *fileNode;
    uint8_t *buffer;
} OsFile;

typedef struct OsMemoryFileSystem
{
    OsFileNode root;
} OsMemoryFileSystem;
/*********************************************************************************************************************
* 初始化
* memoryFileSystem：文件系统对象
* return：0：调用成功
*********************************************************************************************************************/
int osMemoryFileSystemInit(OsMemoryFileSystem *memoryFileSystem);
/*********************************************************************************************************************
* 打开一个文件
* memoryFileSystem：文件系统对象
* file：文件对象
* path：文件路径
* mode：打开模式
* return：0：调用成功
*********************************************************************************************************************/
int osMemoryFileSystemOpen(OsMemoryFileSystem *memoryFileSystem, OsFile *file, const char *path, uint32_t mode);
// FRESULT f_close (FIL* fp);											/* Close an open file object */
// FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);			/* Read data from the file */
// FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to the file */
// FRESULT f_lseek (FIL* fp, FSIZE_t ofs);								/* Move file pointer of the file object */
// FRESULT f_truncate (FIL* fp);										/* Truncate the file */
// FRESULT f_sync (FIL* fp);											/* Flush cached data of the writing file */
// FRESULT f_opendir (DIR* dp, const TCHAR* path);						/* Open a directory */
// FRESULT f_closedir (DIR* dp);										/* Close an open directory */
// FRESULT f_readdir (DIR* dp, FILINFO* fno);							/* Read a directory item */
// FRESULT f_findfirst (DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern);	/* Find first file */
// FRESULT f_findnext (DIR* dp, FILINFO* fno);							/* Find next file */
// FRESULT f_mkdir (const TCHAR* path);								/* Create a sub directory */
// FRESULT f_unlink (const TCHAR* path);								/* Delete an existing file or directory */
// FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new);	/* Rename/Move a file or directory */
// FRESULT f_stat (const TCHAR* path, FILINFO* fno);					/* Get file status */
// FRESULT f_chmod (const TCHAR* path, BYTE attr, BYTE mask);			/* Change attribute of a file/dir */
// FRESULT f_utime (const TCHAR* path, const FILINFO* fno);			/* Change timestamp of a file/dir */
// FRESULT f_chdir (const TCHAR* path);								/* Change current directory */
// FRESULT f_chdrive (const TCHAR* path);								/* Change current drive */
// FRESULT f_getcwd (TCHAR* buff, UINT len);							/* Get current directory */
// FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);	/* Get number of free clusters on the drive */
// FRESULT f_getlabel (const TCHAR* path, TCHAR* label, DWORD* vsn);	/* Get volume label */
// FRESULT f_setlabel (const TCHAR* label);							/* Set volume label */
// FRESULT f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
// FRESULT f_expand (FIL* fp, FSIZE_t fsz, BYTE opt);					/* Allocate a contiguous block to the file */
// FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt);			/* Mount/Unmount a logical drive */
// FRESULT f_mkfs (const TCHAR* path, const MKFS_PARM* opt, void* work, UINT len);	/* Create a FAT volume */
// FRESULT f_fdisk (BYTE pdrv, const LBA_t ptbl[], void* work);		/* Divide a physical drive into some partitions */
// FRESULT f_setcp (WORD cp);											/* Set current code page */
// int f_putc (TCHAR c, FIL* fp);										/* Put a character to the file */
// int f_puts (const TCHAR* str, FIL* cp);								/* Put a string to the file */
// int f_printf (FIL* fp, const TCHAR* str, ...);						/* Put a formatted string to the file */
// TCHAR* f_gets (TCHAR* buff, int len, FIL* fp);						/* Get a string from the file */

// #define f_eof(fp) ((int)((fp)->fptr == (fp)->obj.objsize))
// #define f_error(fp) ((fp)->err)
// #define f_tell(fp) ((fp)->fptr)
// #define f_size(fp) ((fp)->obj.objsize)
// #define f_rewind(fp) f_lseek((fp), 0)
// #define f_rewinddir(dp) f_readdir((dp), 0)
// #define f_rmdir(path) f_unlink(path)
// #define f_unmount(path) f_mount(0, path, 0)
#ifdef __cplusplus
}
#endif
#endif