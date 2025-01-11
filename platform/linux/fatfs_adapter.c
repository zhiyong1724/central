#include "fatfs_adapter.h"
#include "ff.h"
#include "sys_mem.h"
#include "sys_string.h"
static sys_file_error_t parseResult(FRESULT result)
{
    sys_file_error_t ret = SYS_FILE_ERROR_OK;
    switch (result)
    {
    case FR_OK:
        ret = SYS_FILE_ERROR_OK;
        break;
    case FR_DISK_ERR:
        ret = SYS_FILE_ERROR_DISK_ERR;
        break;
    case FR_NOT_READY:
        ret = SYS_FILE_ERROR_NOT_READY;
        break;
    case FR_WRITE_PROTECTED:
        ret = SYS_FILE_ERROR_WRITE_PROTECTED;
        break;
    case FR_INVALID_DRIVE:
        ret = SYS_FILE_ERROR_INVALID_DRIVER;
        break;
    case FR_NO_FILESYSTEM:
        ret = SYS_FILE_ERROR_NO_FILESYSTEM;
        break;
    case FR_NO_FILE:
        ret = SYS_FILE_ERROR_NO_FILE;
        break;
    case FR_NO_PATH:
        ret = SYS_FILE_ERROR_NO_PATH;
        break;
    case FR_INVALID_NAME:
        ret = SYS_FILE_ERROR_INVALID_NAME;
        break;
    case FR_EXIST:
        ret = SYS_FILE_ERROR_EXIST;
        break;
    case FR_INVALID_OBJECT:
        ret = SYS_FILE_ERROR_INVALID_OBJECT;
        break;
    case FR_DENIED:
        ret = SYS_FILE_ERROR_DENIED;
        break;
    case FR_INVALID_PARAMETER:
        ret = SYS_FILE_ERROR_INVALID_PARAMETER;
        break;
    default:
        ret = SYS_FILE_ERROR_OTHER;
        break;
    }
    return ret;
}

static sys_file_error_t fatfsOpen(sys_file_t *file, const char *path, uint32_t mode)
{
    sys_file_error_t ret = SYS_FILE_ERROR_NOMEM;
    file->obj = sys_malloc(sizeof(FIL));
    if (file->obj != NULL)
    {
        sys_memset(file->obj, 0, sizeof(FIL));
        BYTE fmode = 0;
        if ((mode & SYS_FILE_MODE_READ) > 0)
        {
            fmode |= FA_READ;
        }
        if ((mode & SYS_FILE_MODE_WRITE) > 0)
        {
            fmode |= FA_WRITE;
        }
        if ((mode & SYS_FILE_MODE_OPEN_EXISTING) > 0)
        {
            fmode |= FA_OPEN_EXISTING;
        }
        if ((mode & SYS_FILE_MODE_CREATE_NEW) > 0)
        {
            fmode |= FA_CREATE_NEW;
        }
        if ((mode & SYS_FILE_MODE_CREATE_ALWAYS) > 0)
        {
            fmode |= FA_CREATE_ALWAYS;
        }
        if ((mode & SYS_FILE_MODE_OPEN_ALWAYS) > 0)
        {
            fmode |= FA_OPEN_ALWAYS;
        }
        if ((mode & SYS_FILE_MODE_OPEN_APPEND) > 0)
        {
            fmode |= FA_OPEN_APPEND;
        }
        FRESULT result = f_open((FIL *)file->obj, (const TCHAR *)path, fmode);
        ret = parseResult(result);
        if (ret != SYS_FILE_ERROR_OK)
        {
            sys_free(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static sys_file_error_t fatfsClose(sys_file_t *file)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_close((FIL *)file->obj);
        ret = parseResult(result);
        if (SYS_FILE_ERROR_OK == ret)
        {
            sys_free(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static sys_file_error_t fatfsRead(sys_file_t *file, void *buff, uint64_t size, uint64_t *length)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        UINT len = 0;
        FRESULT result = f_read((FIL *)file->obj, buff, (UINT)size, &len);
        *length = len;
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t fatfsWrite(sys_file_t *file, const void *buff, uint64_t size, uint64_t *length)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        UINT len = 0;
        FRESULT result = f_write((FIL *)file->obj, buff, (UINT)size, &len);
        *length = len;
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t fatfsSeek(sys_file_t *file, int64_t offset, sys_seek_type_t whence)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        switch (whence)
        {
        case SYS_SEEK_TYPE_SET:
            break;
        case SYS_SEEK_TYPE_CUR:
            offset += (int64_t)f_tell((FIL *)file->obj);
            break;
        case SYS_SEEK_TYPE_END:
            offset += (int64_t)f_size((FIL *)file->obj);
            break;
        default:
            break;
        }

        FRESULT result = f_lseek((FIL *)file->obj, (FSIZE_t)offset);
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t fatfsTell(sys_file_t *file, uint64_t *offset)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        *offset = f_tell((FIL *)file->obj);
        ret = SYS_FILE_ERROR_OK;
    }
    return ret;
}

static sys_file_error_t fatfsTruncate(sys_file_t *file, uint64_t size)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FSIZE_t offset = f_tell((FIL *)file->obj);
        f_lseek((FIL *)file->obj, (FSIZE_t)size);
        FRESULT result = f_truncate((FIL *)file->obj);
        f_lseek((FIL *)file->obj, offset);
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t fatfsSync(sys_file_t *file)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_sync((FIL *)file->obj);
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t fatfsOpenDir(sys_dir_t *dir, const char *path)
{
    sys_file_error_t ret = SYS_FILE_ERROR_NOMEM;
    dir->obj = sys_malloc(sizeof(DIR));
    if (dir->obj != NULL)
    {
        sys_memset(dir->obj, 0, sizeof(DIR));
        FRESULT result = f_opendir((DIR *)dir->obj, (const TCHAR *)path);
        ret = parseResult(result);
        if (ret != SYS_FILE_ERROR_OK)
        {
            sys_free(dir->obj);
            dir->obj = NULL;
        }
    }
    return ret;
}

static sys_file_error_t fatfsCloseDir(sys_dir_t *dir)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        FRESULT result = f_closedir((DIR *)dir->obj);
        ret = parseResult(result);
        if (SYS_FILE_ERROR_OK == ret)
        {
            sys_free(dir->obj);
            dir->obj = NULL;
        }
    }
    return ret;
}

static void parseFileInfo(FILINFO* fno, sys_file_info_t *file_info)
{
    if ((fno->fattrib & AM_DIR) > 0)
    {
        file_info->type = SYS_FILE_TYPE_DIRECTORY;
    }
    else
    {
        file_info->type = SYS_FILE_TYPE_NORMAL;
    }
    file_info->attribute = 0;
    if (0 == (fno->fattrib & AM_SYS))
    {
        file_info->attribute |= SYS_FILE_ATTR_OWNER_READ | SYS_FILE_ATTR_OWNER_EXE;
        if (0 == (fno->fattrib & AM_RDO))
        {
            file_info->attribute |= SYS_FILE_ATTR_OWNER_WRITE;
        }
    }
    sys_file_time_t time;
    time.day = (uint8_t)(fno->fdate & 0x1f);
    time.month = (uint8_t)(fno->fdate >> 5 & 0x0f);
    time.year = 1980 + (uint16_t)(fno->fdate >> 9 & 0x7f);

    time.second = (uint8_t)(fno->ftime & 0x1f);
    time.minute = (uint8_t)(fno->ftime >> 5 & 0x3f);
    time.hour = (uint16_t)(fno->ftime >> 11 & 0x1f);

    file_info->create_time = time;
    file_info->change_time = time;
    file_info->access_time = time;

    sys_strcpy(file_info->name, (const char *)fno->fname, SYS_MAX_FILE_NAME_LENGTH);
    file_info->file_size = (uint64_t)fno->fsize;
}

static sys_file_error_t fatfsReadDir(sys_dir_t *dir, sys_file_info_t *file_info)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        FILINFO fno;
        FRESULT result = f_readdir((DIR *)dir->obj, &fno);
        ret = parseResult(result);
        if (SYS_FILE_ERROR_OK == ret)
        {
            parseFileInfo(&fno, file_info);
        }
    }
    return ret;
}

static sys_file_error_t fatfsFindFirst(sys_dir_t *dir, sys_file_info_t *file_info, const char *path, const char *pattern)
{
    sys_file_error_t ret = SYS_FILE_ERROR_NOMEM;
    dir->obj = sys_malloc(sizeof(DIR));
    if (dir->obj != NULL)
    {
        sys_memset(dir->obj, 0, sizeof(DIR));
        FILINFO fno;
        FRESULT result = f_findfirst((DIR *)dir->obj, &fno, (const TCHAR *)path, (const TCHAR *)pattern);
        ret = parseResult(result);
        if (ret != SYS_FILE_ERROR_OK)
        {
            sys_free(dir->obj);
            dir->obj = NULL;
        }
        else
        {
            parseFileInfo(&fno, file_info);
        }
    }
    return ret;
}

static sys_file_error_t fatfsFindNext(sys_dir_t *dir, sys_file_info_t *file_info)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        FILINFO fno;
        FRESULT result = f_findnext((DIR *)dir->obj, &fno);
        ret = parseResult(result);
        if (SYS_FILE_ERROR_OK == ret)
        {
            parseFileInfo(&fno, file_info);
        }
    }
    return ret;
}

static sys_file_error_t fatfsMkDir(const char *path)
{
    FRESULT result = f_mkdir((const TCHAR *)path);
    return parseResult(result);
}

static sys_file_error_t fatfsUnlink(const char *path)
{
    FRESULT result = f_unlink((const TCHAR *)path);
    return parseResult(result);
}

static sys_file_error_t fatfsRename(const char *oldPath, const char *newPath)
{
    FRESULT result = f_rename((const TCHAR *)oldPath, (const TCHAR *)newPath);
    return parseResult(result);
}

static sys_file_error_t fatfsStat(const char *path, sys_file_info_t *file_info)
{
    FILINFO fno;
    FRESULT result = f_stat((const TCHAR *)path, &fno);
    sys_file_error_t ret = parseResult(result);
    if (SYS_FILE_ERROR_OK == ret)
    {
        parseFileInfo(&fno, file_info);
    }
    return ret;
}

static sys_file_error_t fatfsChMod(const char *path, uint32_t attr, uint32_t mask)
{
    BYTE fattr = 0;
    BYTE fmask = 0;
    if ((attr & SYS_FILE_ATTR_OWNER_WRITE) > 0)
    {
        fattr &= ~AM_RDO;
    }
    else
    {
        fattr |= AM_RDO;
    }
    fmask |= AM_RDO;
    FRESULT result = f_chmod((const TCHAR *)path, fattr, fmask);
    return parseResult(result);
}

static sys_file_error_t fatfsChDriver(const char *path)
{
    FRESULT result = f_chdrive((const TCHAR *)path);
    return parseResult(result);
}

static sys_file_error_t fatfsStatFS(const char *path, sys_fs_t *fs)
{
    DWORD nclst;
    FATFS *fatfs;
    FRESULT result = f_getfree((const TCHAR *)path, &nclst, &fatfs);
    sys_file_error_t ret = parseResult(result);
    if (SYS_FILE_ERROR_OK == ret)
    {
        switch (fatfs->fs_type)
        {
        case FS_FAT12:
            sys_strcpy(fs->type, "FAT12", SYS_MAX_FILE_NAME_LENGTH);
            break;
        case FS_FAT16:
            sys_strcpy(fs->type, "FAT16", SYS_MAX_FILE_NAME_LENGTH);
            break;
        case FS_FAT32:
            sys_strcpy(fs->type, "FAT32", SYS_MAX_FILE_NAME_LENGTH);
            break;
        case FS_EXFAT:
            sys_strcpy(fs->type, "EXFAT", SYS_MAX_FILE_NAME_LENGTH);
            break;
        default:
            sys_strcpy(fs->type, "", SYS_MAX_FILE_NAME_LENGTH);
            break;
        }
        fs->page_size = (uint32_t)(fatfs->ssize * fatfs->csize);
        fs->free_pages = (uint64_t)nclst;
        fs->total_pages = (uint64_t)fatfs->n_fatent - 2;
    }
    return ret;
}

sys_file_error_t fatfsMount(sys_mount_info_t *mount_info)
{
    sys_file_error_t ret = SYS_FILE_ERROR_NOMEM;
    FATFS *fatfs = (FATFS *)sys_malloc(sizeof(FATFS));
    if (fatfs != NULL)
    {
        sys_memset(fatfs, 0, sizeof(FATFS));
        mount_info->obj = fatfs;
        FRESULT result = f_mount(fatfs, (const TCHAR *)mount_info->driver, 1);
        ret = parseResult(result);
        if (ret != SYS_FILE_ERROR_OK)
        {
            sys_free(fatfs);
        }
    }
    return ret;
}

sys_file_error_t fatfsUnmount(sys_mount_info_t *mount_info)
{
    FRESULT result = f_unmount((const TCHAR *)mount_info->driver);
    sys_file_error_t ret = parseResult(result);
    sys_free(mount_info->obj);
    return ret;
}

sys_file_error_t register_fatfs()
{
    sys_fs_interfaces_t fsInterfaces;
    fsInterfaces.open = fatfsOpen;
    fsInterfaces.close = fatfsClose;
    fsInterfaces.read = fatfsRead;
    fsInterfaces.write = fatfsWrite;
    fsInterfaces.seek = fatfsSeek;
    fsInterfaces.tell = fatfsTell;
    fsInterfaces.truncate = fatfsTruncate;
    fsInterfaces.sync = fatfsSync;
    fsInterfaces.openDir = fatfsOpenDir;
    fsInterfaces.closeDir = fatfsCloseDir;
    fsInterfaces.readDir = fatfsReadDir;
    fsInterfaces.findFirst = fatfsFindFirst;
    fsInterfaces.findNext = fatfsFindNext;
    fsInterfaces.mkDir = fatfsMkDir;
    fsInterfaces.unlink = fatfsUnlink;
    fsInterfaces.rename = fatfsRename;
    fsInterfaces.stat = fatfsStat;
    fsInterfaces.chMod = fatfsChMod;
    fsInterfaces.chDriver = fatfsChDriver;
    fsInterfaces.statFS = fatfsStatFS;
    fsInterfaces.mount = fatfsMount;
    fsInterfaces.unmount = fatfsUnmount;
    return osFAddFS(&fsInterfaces);
}