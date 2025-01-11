#ifndef __SAIAUDIOOUTPUT_H__
#define __SAIAUDIOOUTPUT_H__
#include "audiooutput.h"
#include "sys_task.h"
#include "sys_semaphore.h"
typedef struct SaiAudioOutput
{
    AudioOutput audioOutput;
    sys_tid_t tid;
    sys_semaphore_t semaphore;
    int running;
    void *bufferA;
    void *bufferB;
    int bufferSize;
    unsigned int time;
    int exchange;
} SaiAudioOutput;

int saiAudioOutputInit(SaiAudioOutput *saiAudioOutput);
int saiAudioOutputUninit(SaiAudioOutput *saiAudioOutput);
#endif