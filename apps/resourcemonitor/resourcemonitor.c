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
        printf("cpu used: %ld%%    mem used: %ld%%    task count: %ld", osTaskGetCPUUsage(), (osTotalMem() - osFreeMem()) * 100 / osTotalMem(), osTaskGetTaskCount());
        fflush(stdout);
        osTaskSleep(1000);
        printf("\r\033[K");
    }
    return NULL;
}

void resourceMonitorDump()
{
    sRunning = 1;
    if (0 == sTid)
    {
        osTaskCreate(&sTid, monitorTask, NULL, "resource monitor", 20, 0);
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