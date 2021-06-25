#include "osmemoryfilesystem.h"
#include "osmem.h"
#include "osstring.h"
#define ENABLE_MEMORY_FILE_SYSTEM_LOG 0
#if ENABLE_MEMORY_FILE_SYSTEM_LOG
#define memoryFileSystemLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define memoryFileSystemLog(format, ...) (void)0
#endif 
uint64_t portGetPosixTime();
static int fileNodeInit(OsFileNode *fileNode)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    fileNode->type = OS_FILE_TYPE_NORMAL;
    fileNode->attribute = OS_FILE_ATTR_OWNER_READ | OS_FILE_ATTR_OWNER_WRITE | OS_FILE_ATTR_GROUP_READ | OS_FILE_ATTR_GROUP_WRITE | OS_FILE_ATTR_OTHER_READ;
    fileNode->createTime = portGetPosixTime();
    fileNode->changeTime = fileNode->createTime;
    fileNode->accessTime = fileNode->createTime;
    fileNode->ownerNameSize = 0;
    fileNode->groupNameSize = 0;
    fileNode->fileNameSize = 0;
    fileNode->fileSize = 0;
    fileNode->data = NULL;
    return 0;
}

int osMemoryFileSystemInit(OsMemoryFileSystem *memoryFileSystem)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    fileNodeInit(&memoryFileSystem->root);
    memoryFileSystem->root.type = OS_FILE_TYPE_DIRECTORY;
    return 0;
}

static int findNextPathIndex(const char *path, int index)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    while (path[index] != '\0' && path[index] != '/')
    {
        index++;
    }
    if (path[index] != '\0' && '/' == path[index])
    {
        index++;
        while (path[index] != '\0' && '/' == path[index])
        {
            index++;
        }
        if (path[index] != '\0' && path[index] != '/')
        {
            return index;
        }
    }
    return -1;
}

static uint32_t nameLength(const char *path)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    uint32_t index = 0;
    while (path[index] != '\0' && path[index] != '/')
    {
        index++;
    }
    return index;
}

static int strCmp(const char *str1, const char *str2)
{
	memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
	os_size_t i;
	for (i = 0; (str1[i] != '\0' && str1[i] != '/') && (str2[i] != '\0' && str2[i] != '/'); i++)
	{
		if (str1[i] > str2[i])
		{
			return 1;
		}
		else if (str1[i] < str2[i])
		{
			return -1;
		}
	}
	if (('\0' == str1[i] || '/' == str1[i]) && ('\0' == str2[i] || '/' == str2[i]))
	{
		return 0;
	}
	else if ('\0' == str1[i] || '/' == str1[i])
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

static int onCompare(void *key1, void *key2, void *arg)
{
	memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileNode *fileNode1 = (OsFileNode *)key1;
    OsFileNode *fileNode2 = (OsFileNode *)key2;
    return strCmp((const char *)(fileNode1 + 1), (const char *)(fileNode2 + 1));
}

static int onFindCompare(void *key1, void *key2, void *arg)
{
	memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const char *key = (const char *)key1;
    OsFileNode *fileNode2 = (OsFileNode *)key2;
    return strCmp(key, (const char *)(fileNode2 + 1));
}

static OsFileNode *followLinkFile(OsFileNode *fileNode)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    while (OS_FILE_TYPE_LINK == fileNode->type)
    {
        fileNode = (OsFileNode *)fileNode->data;
    }
    return fileNode;
}

static OsFileNode *openParentFileNode(OsFileNode *parent, const char *path, int *index)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileNode *fileNode = NULL;
    *index = findNextPathIndex(path, *index);
    if (*index != -1)
    {
        fileNode = followLinkFile(parent);
        if (findNextPathIndex(path, *index) != -1)
        {
            if (OS_FILE_TYPE_DIRECTORY == parent->type)
            {
                fileNode = (OsFileNode *)osFindNode((OsTreeNode *)parent->data, (void *)&path[*index], onFindCompare, NULL);
                fileNode = openParentFileNode(fileNode, path, index);
            }
            else
            {
                fileNode = NULL;
            }
        }
    }
    return fileNode;
}

static OsFileNode *createFileNode(OsFileNode *root, const char *path)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int index = 0;
    OsFileNode *parentNode = openParentFileNode(root, path, &index);
    if (parentNode != NULL)
    {
        parentNode = followLinkFile(parentNode);
        if (OS_FILE_TYPE_DIRECTORY == parentNode->type)
        {
            uint32_t length = nameLength(&path[index]) + 1;
            OsFileNode *fileNode = osMalloc(sizeof(OsFileNode) + length);
            osAssert(fileNode != NULL);
            if (fileNode != NULL)
            {
                fileNodeInit(fileNode);
                fileNode->fileNameSize = length;
                osStrCpy((char *)(fileNode + 1), &path[index], length);
                if (osInsertNode((OsTreeNode **)&parentNode->data, &fileNode->node, onCompare, NULL) == 0)
                {
                    return fileNode;
                }
                else
                {
                    osFree(fileNode);
                }
            }
        }
    }
    return NULL;
}

static OsFileNode *openFileNode(OsFileNode *root, const char *path)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int index = 0;
    OsFileNode *parentNode = openParentFileNode(root, path, &index);
    if (parentNode != NULL)
    {
        parentNode = followLinkFile(parentNode);
        if (OS_FILE_TYPE_DIRECTORY == parentNode->type)
        {
           OsFileNode *fileNode = (OsFileNode *)osFindNode((OsTreeNode *)parentNode->data, (void *)&path[index], onFindCompare, NULL);
           return fileNode;
        }
    }
    return NULL;
}

int osMemoryFileSystemOpen(OsMemoryFileSystem *memoryFileSystem, OsFile *file, const char *path, uint32_t mode)
{
    memoryFileSystemLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    int ret = -1;
    OsFileNode *fileNode = NULL;
    if ((mode & OS_FILE_MODE_OPEN_EXISTING) > 0)
    {
        fileNode = openFileNode(&memoryFileSystem->root, path);
    }
    else if ((mode & OS_FILE_MODE_CREATE_NEW) > 0)
    {
        fileNode = openFileNode(&memoryFileSystem->root, path);
        if (NULL == fileNode)
        {
            fileNode = createFileNode(&memoryFileSystem->root, path);   
        }
        else
        {
            fileNode = NULL;
        }
    }
    else if ((mode & OS_FILE_MODE_CREATE_ALWAYS) > 0)
    {
        //暂时还没实现
    }
    else if ((mode & OS_FILE_MODE_OPEN_ALWAYS) > 0)
    {
        fileNode = openFileNode(&memoryFileSystem->root, path);
        if (NULL == fileNode)
        {
            fileNode = createFileNode(&memoryFileSystem->root, path);
        }
    }
    if (fileNode != NULL)
    {
        fileNode = followLinkFile(fileNode);
        file->buffer = osAllocPages(1);
        if (file->buffer != NULL)
        {
            file->mode = mode;
            file->fileNode = fileNode;
            ret = 0;
        }
    }
    return ret;
}