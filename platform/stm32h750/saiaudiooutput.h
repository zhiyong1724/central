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
} SaiAudioOutput;

int saiAudioOutputInit(SaiAudioOutput *saiAudioOutput);
int saiAudioOutputUninit(SaiAudioOutput *saiAudioOutput);
#endif