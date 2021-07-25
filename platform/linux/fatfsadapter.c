#include "fatfsadapter.h"
#include "ff.h"
#include "osmem.h"
#include "osstring.h"
static OsFileError parseResult(FRESULT result)
{
    OsFileError ret = OS_FILE_ERROR_OK;
    switch (result)
    {
    case FR_OK:
        ret = OS_FILE_ERROR_OK;
        break;
    case FR_DISK_ERR:
        ret = OS_FILE_ERROR_DISK_ERR;
        break;
    case FR_NOT_READY:
        ret = OS_FILE_ERROR_NOT_READY;
        break;
    case FR_WRITE_PROTECTED:
        ret = OS_FILE_ERROR_WRITE_PROTECTED;
        break;
    case FR_INVALID_DRIVE:
        ret = OS_FILE_ERROR_INVALID_DRIVE;
        break;
    case FR_NO_FILESYSTEM:
        ret = OS_FILE_ERROR_NO_FILESYSTEM;
        break;
    case FR_NO_FILE:
        ret = OS_FILE_ERROR_NO_FILE;
        break;
    case FR_NO_PATH:
        ret = OS_FILE_ERROR_NO_PATH;
        break;
    case FR_INVALID_NAME:
        ret = OS_FILE_ERROR_INVALID_NAME;
        break;
    case FR_EXIST:
        ret = OS_FILE_ERROR_EXIST;
        break;
    case FR_INVALID_OBJECT:
        ret = OS_FILE_ERROR_INVALID_OBJECT;
        break;
    case FR_DENIED:
        ret = OS_FILE_ERROR_DENIED;
        break;
    case FR_INVALID_PARAMETER:
        ret = OS_FILE_ERROR_INVALID_PARAMETER;
        break;
    default:
        ret = OS_FILE_ERROR_OTHER;
        break;
    }
    return ret;
}

static OsFileError fatfsOpen(OsFile *file, const char *path, uint32_t mode)
{
    OsFileError ret = OS_FILE_ERROR_NOMEM;
    file->obj = osMalloc(sizeof(FIL));
    if (file->obj != NULL)
    {
        BYTE fmode = 0;
        if ((mode & OS_FILE_MODE_READ) > 0)
        {
            fmode |= FA_READ;
        }
        if ((mode & OS_FILE_MODE_WRITE) > 0)
        {
            fmode |= FA_WRITE;
        }
        if ((mode & OS_FILE_MODE_OPEN_EXISTING) > 0)
        {
            fmode |= FA_OPEN_EXISTING;
        }
        if ((mode & OS_FILE_MODE_CREATE_NEW) > 0)
        {
            fmode |= FA_CREATE_NEW;
        }
        if ((mode & OS_FILE_MODE_CREATE_ALWAYS) > 0)
        {
            fmode |= FA_CREATE_ALWAYS;
        }
        if ((mode & OS_FILE_MODE_OPEN_ALWAYS) > 0)
        {
            fmode |= FA_OPEN_ALWAYS;
        }
        if ((mode & OS_FILE_MODE_OPEN_APPEND) > 0)
        {
            fmode |= FA_OPEN_APPEND;
        }
        FRESULT result = f_open((FIL *)file->obj, (const TCHAR *)path, fmode);
        ret = parseResult(result);
        if (ret != OS_FILE_ERROR_OK)
        {
            osFree(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static OsFileError fatfsClose(OsFile *file)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_close((FIL *)file->obj);
        ret = parseResult(result);
        if (OS_FILE_ERROR_OK == ret)
        {
            osFree(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static OsFileError fatfsRead(OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_read((FIL *)file->obj, buff, (UINT)size, (UINT *)length);
        ret = parseResult(result);
    }
    return ret;
}

static OsFileError fatfsWrite(OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_write((FIL *)file->obj, buff, (UINT)size, (UINT *)length);
        ret = parseResult(result);
    }
    return ret;
}

static OsFileError fatfsSeek(OsFile *file, int64_t offset, OsSeekType whence)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        switch (whence)
        {
        case OS_SEEK_TYPE_SET:
            break;
        case OS_SEEK_TYPE_CUR:
            offset += (int64_t)f_tell((FIL *)file->obj);
            break;
        case OS_SEEK_TYPE_END:
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

static OsFileError fatfsTell(OsFile *file, uint64_t *offset)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        *offset = f_tell((FIL *)file->obj);
        ret = OS_FILE_ERROR_OK;
    }
    return ret;
}

static OsFileError fatfsTruncate(OsFile *file, uint64_t size)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
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

static OsFileError fatfsSync(OsFile *file)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_sync((FIL *)file->obj);
        ret = parseResult(result);
    }
    return ret;
}

static OsFileError fatfsOpenDir(OsDir *dir, const char *path)
{
    OsFileError ret = OS_FILE_ERROR_NOMEM;
    dir->obj = osMalloc(sizeof(DIR));
    if (dir->obj != NULL)
    {
        FRESULT result = f_opendir((DIR *)dir->obj, (const TCHAR *)path);
        ret = parseResult(result);
        if (ret != OS_FILE_ERROR_OK)
        {
            osFree(dir->obj);
            dir->obj = NULL;
        }
    }
    return ret;
}

static OsFileError fatfsCloseDir(OsDir *dir)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        FRESULT result = f_closedir((DIR *)dir->obj);
        ret = parseResult(result);
        if (OS_FILE_ERROR_OK == ret)
        {
            osFree(dir->obj);
            dir->obj = NULL;
        }
    }
    return ret;
}

static void parseFileInfo(FILINFO* fno, OsFileInfo *fileInfo)
{
    if ((fno->fattrib & AM_DIR) > 0)
    {
        fileInfo->type = OS_FILE_TYPE_DIRECTORY;
    }
    else
    {
        fileInfo->type = OS_FILE_TYPE_NORMAL;
    }
    fileInfo->attribute = 0;
    if (0 == (fno->fattrib & AM_SYS))
    {
        fileInfo->attribute |= OS_FILE_ATTR_OWNER_READ | OS_FILE_ATTR_OWNER_EXE;
        if (0 == (fno->fattrib & AM_RDO))
        {
            fileInfo->attribute |= OS_FILE_ATTR_OWNER_WRITE;
        }
    }
    OsFileTime time;
    time.day = (uint8_t)(fno->fdate & 0x1f);
    time.month = (uint8_t)(fno->fdate >> 5 & 0x0f);
    time.year = 1980 + (uint16_t)(fno->fdate >> 9 & 0x7f);

    time.second = (uint8_t)(fno->ftime & 0x1f);
    time.minute = (uint8_t)(fno->ftime >> 5 & 0x3f);
    time.hour = (uint16_t)(fno->ftime >> 11 & 0x1f);

    fileInfo->createTime = time;
    fileInfo->changeTime = time;
    fileInfo->accessTime = time;

    osStrCpy(fileInfo->name, (const char *)fno->fname, OS_MAX_FILE_NAME_LENGTH);
    fileInfo->fileSize = (uint64_t)fno->fsize;
}

static OsFileError fatfsReadDir(OsDir *dir, OsFileInfo *fileInfo)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        FILINFO fno;
        FRESULT result = f_readdir((DIR *)dir->obj, &fno);
        ret = parseResult(result);
        if (OS_FILE_ERROR_OK == ret)
        {
            parseFileInfo(&fno, fileInfo);
        }
    }
    return ret;
}

static OsFileError fatfsFindFirst(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern)
{
    OsFileError ret = OS_FILE_ERROR_NOMEM;
    dir->obj = osMalloc(sizeof(DIR));
    if (dir->obj != NULL)
    {
        FILINFO fno;
        FRESULT result = f_findfirst((DIR *)dir->obj, &fno, (const TCHAR *)path, (const TCHAR *)pattern);
        ret = parseResult(result);
        if (ret != OS_FILE_ERROR_OK)
        {
            osFree(dir->obj);
            dir->obj = NULL;
        }
        else
        {
            parseFileInfo(&fno, fileInfo);
        }
    }
    return ret;
}

static OsFileError fatfsFindNext(OsDir *dir, OsFileInfo *fileInfo)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        FILINFO fno;
        FRESULT result = f_findnext((DIR *)dir->obj, &fno);
        ret = parseResult(result);
        if (OS_FILE_ERROR_OK == ret)
        {
            parseFileInfo(&fno, fileInfo);
        }
    }
    return ret;
}

static OsFileError fatfsMkDir(const char *path)
{
    FRESULT result = f_mkdir((const TCHAR *)path);
    return parseResult(result);
}

static OsFileError fatfsUnlink(const char *path)
{
    FRESULT result = f_unlink((const TCHAR *)path);
    return parseResult(result);
}

static OsFileError fatfsRename(const char *oldPath, const char *newPath)
{
    FRESULT result = f_rename((const TCHAR *)oldPath, (const TCHAR *)newPath);
    return parseResult(result);
}

static OsFileError fatfsStat(const char *path, OsFileInfo *fileInfo)
{
    FILINFO fno;
    FRESULT result = f_stat((const TCHAR *)path, &fno);
    OsFileError ret = parseResult(result);
    if (OS_FILE_ERROR_OK == ret)
    {
        parseFileInfo(&fno, fileInfo);
    }
    return ret;
}

static OsFileError fatfsChMod(const char *path, uint32_t attr, uint32_t mask)
{
    BYTE fattr = 0;
    BYTE fmask = 0;
    if ((attr & OS_FILE_ATTR_OWNER_WRITE) > 0)
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

static OsFileError fatfsChDrive(const char *path)
{
    FRESULT result = f_chdrive((const TCHAR *)path);
    return parseResult(result);
}

static OsFileError fatfsStatFS(const char *path, OsFS *fs)
{
    DWORD nclst;
    FATFS *fatfs;
    FRESULT result = f_getfree((const TCHAR *)path, &nclst, &fatfs);
    OsFileError ret = parseResult(result);
    if (OS_FILE_ERROR_OK == ret)
    {
        switch (fatfs->fs_type)
        {
        case FS_FAT12:
            osStrCpy(fs->type, "FAT12", OS_MAX_FILE_NAME_LENGTH);
            break;
        case FS_FAT16:
            osStrCpy(fs->type, "FAT16", OS_MAX_FILE_NAME_LENGTH);
            break;
        case FS_FAT32:
            osStrCpy(fs->type, "FAT32", OS_MAX_FILE_NAME_LENGTH);
            break;
        case FS_EXFAT:
            osStrCpy(fs->type, "EXFAT", OS_MAX_FILE_NAME_LENGTH);
            break;
        default:
            osStrCpy(fs->type, "", OS_MAX_FILE_NAME_LENGTH);
            break;
        }
        fs->pageSize = (uint32_t)fatfs->ssize;
        fs->freePages = (uint64_t)nclst;
        fs->totalPages = (uint64_t)fatfs->n_fatent - 2;
    }
    return ret;
}

OsFileError fatfsMount(OsMountInfo *mountInfo)
{
    OsFileError ret = OS_FILE_ERROR_NOMEM;
    FATFS *fatfs = (FATFS *)osMalloc(sizeof(FATFS));
    if (fatfs != NULL)
    {
        mountInfo->obj = fatfs;
        FRESULT result = f_mount(fatfs, (const TCHAR *)mountInfo->drive, 1);
        ret = parseResult(result);
        if (ret != OS_FILE_ERROR_OK)
        {
            osFree(fatfs);
        }
    }
    return ret;
}

OsFileError fatfsUnmount(OsMountInfo *mountInfo)
{
    FRESULT result = f_unmount((const TCHAR *)mountInfo->drive);
    OsFileError ret = parseResult(result);
    osFree(mountInfo->obj);
    return ret;
}

OsFileError registerFatfs()
{
    OsFSInterfaces fsInterfaces;
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
    fsInterfaces.chDrive = fatfsChDrive;
    fsInterfaces.statFS = fatfsStatFS;
    fsInterfaces.mount = fatfsMount;
    fsInterfaces.unmount = fatfsUnmount;
    return osFAddFS(&fsInterfaces);
}