#include "saiaudiooutput.h"
#include "sai.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static SaiAudioOutput *sSaiAudioOutput = NULL;
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    osSemaphorePost(&sSaiAudioOutput->semaphore);
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    osSemaphorePost(&sSaiAudioOutput->semaphore);
}

static void *audioPlayTask(void *arg)
{
    sSaiAudioOutput = (SaiAudioOutput *)arg;
    sSaiAudioOutput->time = -PERIOD_TIME;
    sSaiAudioOutput->exchange = 0;
    memset(sSaiAudioOutput->bufferA, 0, sSaiAudioOutput->bufferSize);
    memset(sSaiAudioOutput->bufferB, 0, sSaiAudioOutput->bufferSize);
    sSaiAudioOutput->audioOutput.audioOutputCallback.onPlaying(sSaiAudioOutput->audioOutput.audioOutputCallback.object);
    while (sSaiAudioOutput->running > 0)
    {
        int result = 0;
        osSemaphoreWait(&sSaiAudioOutput->semaphore, OS_SEMAPHORE_MAX_WAIT_TIME);
        if (sSaiAudioOutput->exchange > 0)
        {
            MX_SAI1_Send(sSaiAudioOutput->bufferA, sSaiAudioOutput->bufferSize / sizeof(int16_t));
            result = sSaiAudioOutput->audioOutput.audioOutputCallback.onReadDataStream(sSaiAudioOutput->audioOutput.audioOutputCallback.object, sSaiAudioOutput->bufferB, sSaiAudioOutput->bufferSize);
            
        }
        else
        {
            MX_SAI1_Send(sSaiAudioOutput->bufferB, sSaiAudioOutput->bufferSize / sizeof(int16_t));
            result = sSaiAudioOutput->audioOutput.audioOutputCallback.onReadDataStream(sSaiAudioOutput->audioOutput.audioOutputCallback.object, sSaiAudioOutput->bufferA, sSaiAudioOutput->bufferSize);
        }
        if (0 == result)
        {
            osTaskDetach(sSaiAudioOutput->tid);
            sSaiAudioOutput->tid = 0;
            sSaiAudioOutput->running = 0;
        }
        sSaiAudioOutput->time += PERIOD_TIME;
        sSaiAudioOutput->audioOutput.audioOutputCallback.onPositionChanged(sSaiAudioOutput->audioOutput.audioOutputCallback.object, sSaiAudioOutput->time);
        sSaiAudioOutput->exchange ^= 1;
    }
    sSaiAudioOutput->audioOutput.audioOutputCallback.onStopped(sSaiAudioOutput->audioOutput.audioOutputCallback.object);
    return sSaiAudioOutput;
}

static int play(void *audioOutput)
{
    int ret = -1;
    SaiAudioOutput *saiAudioOutput = (SaiAudioOutput *)audioOutput;
    if (0 == saiAudioOutput->tid && 0 == saiAudioOutput->running) 
    {
        saiAudioOutput->running = 1;
        ret = osTaskCreateRT(&saiAudioOutput->tid, audioPlayTask, audioOutput, "audio play", 30, OS_DEFAULT_TASK_STACK_SIZE);
        if (ret != 0)
        {
            saiAudioOutput->running = 0;
        }
    }
    return ret;
}

static int stop(void *audioOutput)
{
    int ret = -1;
    SaiAudioOutput *saiAudioOutput = (SaiAudioOutput *)audioOutput;
    if (saiAudioOutput->tid > 0)
    {
        ret = 0;
        saiAudioOutput->running = 0;
        void *retval = NULL;
        osTaskJoin(&retval, saiAudioOutput->tid);
        sSaiAudioOutput->tid = 0;
    }
    return ret;
}

int saiAudioOutputInit(SaiAudioOutput *saiAudioOutput)
{
    AudioOutputInterface audioOutputInterface;
    audioOutputInterface.play = play;
    audioOutputInterface.stop = stop;
    AudioOutputInfo audioOutputInfo;
    audioOutputInfo.audioOutputType = AUDIO_OUTPUT_TYPE_PCM_S16;
    audioOutputInfo.bitsPerSample = 16;
    audioOutputInfo.samplesPerSec = 48000;
    audioOutputInfo.numChannels = 2;
    audioOutputInit((AudioOutput *)saiAudioOutput, &audioOutputInfo, &audioOutputInterface);
    osSemaphoreCreate(&saiAudioOutput->semaphore, 1, 1);

    saiAudioOutput->tid = 0;
    saiAudioOutput->running = 0;
    saiAudioOutput->time = 0;
    saiAudioOutput->exchange = 0;
    saiAudioOutput->bufferSize = audioOutputInfo.samplesPerSec * PERIOD_TIME / 1000 * audioOutputInfo.bitsPerSample / 8 * audioOutputInfo.numChannels;
    saiAudioOutput->bufferA = malloc(saiAudioOutput->bufferSize);
    saiAudioOutput->bufferB = malloc(saiAudioOutput->bufferSize);
    return 0;
}

int saiAudioOutputUninit(SaiAudioOutput *saiAudioOutput)
{
    if (saiAudioOutput->tid > 0)
    {
        saiAudioOutput->running = 0;
        void *retval = NULL;
        osTaskJoin(&retval, saiAudioOutput->tid);
        sSaiAudioOutput->tid = 0;
    }
    if (saiAudioOutput->bufferA != NULL)
    {
        free(saiAudioOutput->bufferA);
        saiAudioOutput->bufferA = NULL;
    }
    if (saiAudioOutput->bufferB != NULL)
    {
        free(saiAudioOutput->bufferB);
        saiAudioOutput->bufferB = NULL;
    }
    audioOutputUninit((AudioOutput *)saiAudioOutput);
    osSemaphoreDestory(&saiAudioOutput->semaphore);
    return 0;
}