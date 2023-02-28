#include "resourcemonitor.h"
#include "ostask.h"
#include "osmem.h"
#include <string.h>
static int sRunning = 0;
static os_tid_t sTid = 0;
static void *monitorTask(void *arg)
{
    while (sRunning)
    {
        printf("\033[2J\033[0;0H");
        printf("cpu used: %ld%%    mem used: %ld%%    task count: %ld\n", osTaskGetCPUUsage(), (osTotalMem() - osFreeMem()) * 100 / osTotalMem(), osTaskGetTaskCount());
        os_task_ptr ptr;
        OsTaskInfo taskInfo;
        int result = osTaskFindFirst(&ptr, &taskInfo);
        while (0 == result)
        {
            printf("tid: %ld  ptid: %ld  stack: %p  stack size: %ld  state: %d  type: %d  priority: %ld  name: %s\n",
                   taskInfo.tid, taskInfo.ptid, taskInfo.stack, taskInfo.stackSize, taskInfo.taskState, taskInfo.taskType, taskInfo.priority, taskInfo.name);
            result = osTaskFindNext(&ptr, &taskInfo);
        }
        osTaskSleep(1000);
    }
    return NULL;
}

void resourceMonitorDump()
{
    sRunning = 1;
    if (0 == sTid)
    {
        osTaskCreate(&sTid, monitorTask, NULL, "resource monitor", OS_DEFAULT_TASK_PRIORITY, 0);
    }
}

void resourceMonitorStop()
{
    if (sTid != 0)
    {
        sRunning = 0;
        void *ret;
        osTaskJoin(&ret, sTid);
        sTid = 0;
    }
}

void shellRS(long argc, char *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "stop") == 0)
    {
        resourceMonitorStop();
    }
    else
    {
        resourceMonitorDump();
    }
}