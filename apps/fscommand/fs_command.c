#include <stdio.h>
#include <string.h>
#include "sys_vfs.h"
#include "shell.h"
#include "sys_error.h"
#include "shellio.h"
#include <time.h>
#include "sys_kmp.h"
static void print_fs_error(int error)
{
    switch (error)
    {
    case SYS_ERROR_OK:
    break;
    case SYS_ERROR_PERM:
    printf("不允许这样操作\n");
    break;
    case SYS_ERROR_NOENT:
    printf("没有这样的文件或目录\n");
    break;
    case SYS_ERROR_SRCH:
    printf("没有这样的进程\n");
    break;
    case SYS_ERROR_INTR:
    printf("系统调用中断\n");
    break;
    case SYS_ERROR_IO:
    printf("IO操作错误\n");
    break;
    case SYS_ERROR_NXIO:
    printf("没有这样的设备或地址\n");
    break;
    case SYS_ERROR_2BIG:
    printf("参数列表太长\n");
    break;
    case SYS_ERROR_NOEXEC:
    printf("执行文件格式错误\n");
    break;
    case SYS_ERROR_BADF:
    printf("错误的文件句柄\n");
    break;
    case SYS_ERROR_CHILD:
    printf("没有子进程\n");
    break;
    case SYS_ERROR_AGAIN:
    printf("重试一次\n");
    break;
    case SYS_ERROR_NOMEM:
    printf("内存不足\n");
    break;
    case SYS_ERROR_ACCES:
    printf("没有权限操作\n");
    break;
    case SYS_ERROR_FAULT:
    printf("错误的地址\n");
    break;
    case SYS_ERROR_NOTBLK:
    printf("没有块设备\n");
    break;
    case SYS_ERROR_BUSY:
    printf("设备忙\n");
    break;
    case SYS_ERROR_EXIST:
    printf("文件已存在\n");
    break;
    case SYS_ERROR_XDEV:
    printf("跨设备的链接\n");
    break;
    case SYS_ERROR_NODEV:
    printf("没有这样的设备\n");
    break;
    case SYS_ERROR_NOTDIR:
    printf("没有这个目录\n");
    break;
    case SYS_ERROR_ISDIR:
    printf("是一个目录\n");
    break;
    case SYS_ERROR_INVAL:
    printf("无效参数\n");
    break;
    case SYS_ERROR_NFILE:
    printf("文件表溢出\n");
    break;
    case SYS_ERROR_MFILE:
    printf("打开的文件太多\n");
    break;
    case SYS_ERROR_NOTTY:
    printf("没有打印输出\n");
    break;  
    case SYS_ERROR_TXTBSY:
    printf("文本文件忙\n");
    break;    
    case SYS_ERROR_MFBIG:
    printf("文件太大\n");
    break;    
    case SYS_ERROR_NOSPC:
    printf("设备没有剩余空间\n");
    break;    
    case SYS_ERROR_SPIPE:
    printf("非法seek\n");
    break;    
    case SYS_ERROR_ROFS:
    printf("只读文件系统\n");
    break;    
    case SYS_ERROR_MLINK:
    printf("太多链接文件\n");
    break; 
    case SYS_ERROR_PIPE:
    printf("管道损坏\n");
    break; 
    case SYS_ERROR_NAMETOOLONG:
    printf("文件名太长\n");
    break;   
    case SYS_ERROR_NOTEMPTY:
    printf("文件夹非空\n");
    break;   
    case SYS_ERROR_NODATA:
    printf("无有效数据\n");
    break;   
    case SYS_ERROR_ILSEQ:
    printf("非法字节序\n");
    break;   
    default:
        break;
    }
}

const char *to_absolute_path(const char *path)
{
    static char s_absolute_path_a[VFS_MAX_FILE_PATH_LEN];
    static char s_absolute_path_b[VFS_MAX_FILE_PATH_LEN];
    static char *s_path = s_absolute_path_a;
    if (s_path == s_absolute_path_a)
    {
        s_path = s_absolute_path_b;
    }
    else
    {
        s_path = s_absolute_path_a;
    }
    if (path[0] != '/')
    {
        strcpy(s_path, get_shell_path());
        strcat(s_path, "/");
        strcat(s_path, path);
        return s_path;
    }
    else
    {
        return path;
    }
}

void shell_mount(long argc, char *argv[])
{
    if (argc >= 3)
    {
        int result = sys_mount(to_absolute_path(argv[1]), to_absolute_path(argv[2]));
        if (result < 0)
        {
            print_fs_error(result);
            return;
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_umount(long argc, char *argv[])
{
    if (argc >= 2)
    {
        int result = sys_umount(to_absolute_path(argv[1]));
        if (result < 0)
        {
            print_fs_error(result);
            return;
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_pwd(long argc, char *argv[])
{
    printf("%s\n", get_shell_path());
}

void shell_cd(long argc, char *argv[])
{
    if (argc >= 2)
    {
        int fd = sys_opendir(to_absolute_path(argv[1]));
        if (fd < 0)
        {
            print_fs_error(fd);
            return;
        }
        set_shell_path(to_absolute_path(argv[1]));
        sys_closedir(fd);
    }
    else
    {
        printf("参数不足\n");
    }
}

static void show_file_info(struct vfs_stat_t *file_info, const char *name)
{
    char buff[256 + VFS_MAX_FILE_NAME_LEN];
    buff[0] = '\0';
    if ((file_info->st_mode & (7 << 9)) == VFS_MODE_ISDIR)
    {
        strcat(buff, "d");
    }
    else if ((file_info->st_mode & (7 << 9)) == VFS_MODE_ISCHR)
    {
        strcat(buff, "c");
    }
    else if ((file_info->st_mode & (7 << 9)) == VFS_MODE_ISBLK)
    {
        strcat(buff, "b");
    }
    else if ((file_info->st_mode & (7 << 9)) == VFS_MODE_ISREG)
    {
        strcat(buff, "-");
    }
    else if ((file_info->st_mode & (7 << 9)) == VFS_MODE_ISLNK)
    {
        strcat(buff, "l");
    }
    if ((file_info->st_mode & VFS_MODE_IRUSR) > 0)
    {
        strcat(buff, "r");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((file_info->st_mode & VFS_MODE_IWUSR) > 0)
    {
        strcat(buff, "w");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((file_info->st_mode & VFS_MODE_IXUSR) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((file_info->st_mode & VFS_MODE_IRGRP) > 0)
    {
        strcat(buff, "r");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((file_info->st_mode & VFS_MODE_IWGRP) > 0)
    {
        strcat(buff, "w");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((file_info->st_mode & VFS_MODE_IXGRP) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((file_info->st_mode & VFS_MODE_IROTH) > 0)
    {
        strcat(buff, "r");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((file_info->st_mode & VFS_MODE_IWOTH) > 0)
    {
        strcat(buff, "w");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((file_info->st_mode & VFS_MODE_IXOTH) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    char temp[64];
    sprintf(temp, " %ld", file_info->st_size);
    strcat(buff, temp);
    struct tm *t = localtime((const time_t *)&file_info->st_mtim);
    sprintf(temp, " %d %02d %02d %02d:%02d ", 1900 + t->tm_year, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min);
    strcat(buff, temp);
    strcat(buff, name);
    printf("%s\n", buff);
}

void shell_ls(long argc, char *argv[])
{
    const char *path = NULL;
    if (argc >= 2)
    {
        path = to_absolute_path(argv[1]);
    }
    else if (argc >= 1)
    {
        path = to_absolute_path("");
    }
    else
    {
        printf("参数不足\n");
        return;
    }
    int fd = sys_opendir(path);
    if (fd < 0)
    {
        print_fs_error(fd);
        return;
    }
    struct vfs_dirent_t dirent;
    while (sys_readdir(fd, &dirent) > 0)
    {
        char file_path[VFS_MAX_FILE_PATH_LEN];
        strcpy(file_path, path);
        strcat(file_path, "/");
        strcat(file_path, dirent.d_name);
        struct vfs_stat_t stat;
        sys_stat(file_path, &stat);
        show_file_info(&stat, dirent.d_name);
    }
    sys_closedir(fd);
}

void dump_fs();
void shell_df(long argc, char *argv[])
{
    dump_fs();
}

void shell_mkdir(long argc, char *argv[])
{
    if (argc >= 2)
    {
        int result = sys_mkdir(to_absolute_path(argv[1]), VFS_MODE_IRUSR | VFS_MODE_IWUSR);
        if (result < 0)
        {
            print_fs_error(result);
            return;
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_rmdir(long argc, char *argv[])
{
    if (argc >= 2)
    {
        int result = sys_rmdir(to_absolute_path(argv[1]));
        if (result < 0)
        {
            print_fs_error(result);
            return;
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_touch(long argc, char *argv[])
{
    if (argc >= 2)
    {
        int fd = sys_open(to_absolute_path(argv[1]), VFS_FLAG_WRONLY | VFS_FLAG_CREAT | VFS_FLAG_EXCL, VFS_MODE_IRUSR | VFS_MODE_IWUSR);
        if (fd < 0)
        {
            print_fs_error(fd);
            return;
        }
        sys_close(fd);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_mv(long argc, char *argv[])
{
    if (argc >= 3)
    {
        int result = sys_rename(to_absolute_path(argv[1]), to_absolute_path(argv[2]));
        if (result < 0)
        {
            print_fs_error(result);
            return;
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_rm(long argc, char *argv[])
{
    if (argc >= 2)
    {
        int result = sys_unlink(to_absolute_path(argv[1]));
        if (result < 0)
        {
            print_fs_error(result);
            return;
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_cp(long argc, char *argv[])
{
    if (argc >= 3)
    {
        int old_file = sys_open(to_absolute_path(argv[1]), VFS_FLAG_RDONLY, 0);
        if (old_file < 0)
        {
            print_fs_error(old_file);
            return;
        }
        int new_file = sys_open(to_absolute_path(argv[2]), VFS_FLAG_WRONLY | VFS_FLAG_CREAT | VFS_FLAG_EXCL, VFS_MODE_IRUSR | VFS_MODE_IWUSR);
        if (new_file < 0)
        {
            sys_close(old_file);
            print_fs_error(new_file);
            return;
        }
        char buff[1024];
        for (int64_t len = sys_read(old_file, buff, 1024); len > 0; len = sys_read(old_file, buff, 1024))
        {
            len = sys_write(new_file, buff, len);
            if (len <= 0)
            {
                print_fs_error(new_file);
                break;
            }
        }
        sys_close(new_file);
        sys_close(old_file);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_echo(long argc, char *argv[])
{
    if (argc >= 3)
    {
        int fd = -1;
        if (sys_kmp(argv[2], ">>") == 0)
        {
            fd = sys_open(to_absolute_path(&argv[2][2]), VFS_FLAG_WRONLY|VFS_FLAG_APPEND, 0);
        }
        else if(sys_kmp(argv[2], ">") == 0)
        {
            fd = sys_open(to_absolute_path(&argv[2][1]), VFS_FLAG_WRONLY|VFS_FLAG_CREAT, 0);
        }
        else
        {
            printf("%s %s\n", argv[1], argv[2]);
            return;
        }
        if (fd < 0)
        {
            print_fs_error(fd);
            return;
        }
        sys_write(fd, argv[1], strlen(argv[1]));
        sys_close(fd);
    }
    else if (argc >= 2)
    {
        printf("%s\n", argv[1]);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shell_cat(long argc, char *argv[])
{
    if (argc >= 2)
    {
        int fd = sys_open(to_absolute_path(argv[1]), VFS_FLAG_RDONLY, 0);
        if (fd < 0)
        {
            print_fs_error(fd);
            return;
        }
        char buff[1024];
        int len = 0;
        while ((len = sys_read(fd, buff, 1024 - 1)) > 0)
        {
            buff[len] = '\0';
            printf("%s", buff);
        }
        printf("\n");
        sys_close(fd);
    }
    else
    {
        printf("参数不足\n");
    }
}