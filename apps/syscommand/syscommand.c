#include <stdio.h>
#include "ostask.h"
#include "osmem.h"
#include "oscentral.h"
void shellFree(long argc, char *argv[])
{
    printf("所有内存：%ld\n", osTotalMem());
    printf("可用内存：%ld\n", osFreeMem());
    printf("所有页：%ld\n", osTotalPage());
    printf("可用页：%ld\n", osFreePage());
}

void shellPS(long argc, char *argv[])
{
    os_task_ptr ptr;
    OsTaskInfo taskInfo;
    int result = osTaskFindFirst(&ptr, &taskInfo);
    while (0 == result)
    {
        printf("tid: %ld  ptid: %ld  stack: %p  stack size: %ld  state: %d  type: %d  priority: %ld  name: %s\n",
                   taskInfo.tid, taskInfo.ptid, taskInfo.stack, taskInfo.stackSize, taskInfo.taskState, taskInfo.taskType, taskInfo.priority, taskInfo.name);
        result = osTaskFindNext(&ptr, &taskInfo);
    }
}

void shellUName(long argc, char *argv[])
{
    printf("Central V%d %s %s\n", osVersion(), __DATE__, __TIME__);
} 
