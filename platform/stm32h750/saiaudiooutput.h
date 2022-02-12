#ifndef __SAIAUDIOOUTPUT_H__
#define __SAIAUDIOOUTPUT_H__
#include "audiooutput.h"
#include "ostask.h"
#include "ossemaphore.h"
typedef struct SaiAudioOutput
{
    AudioOutput audioOutput;
    os_tid_t tid;
    OsSemaphore semaphore;
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