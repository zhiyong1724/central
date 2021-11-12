#include "saiaudiooutput.h"
#include "sai.h"
#include <stdio.h>
#include <string.h>
static SaiAudioOutput *sSaiAudioOutput = NULL;
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    osSemaphorePost(&sSaiAudioOutput->semaphore);
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    printf("HAL_SAI_ErrorCallback\n");
    osSemaphorePost(&sSaiAudioOutput->semaphore);
}

static void *audioPlayTask(void *arg)
{
    sSaiAudioOutput = (SaiAudioOutput *)arg;
    sSaiAudioOutput->audioOutput.time = -PERIOD_TIME;
    sSaiAudioOutput->audioOutput.exchange = 0;
    memset(sSaiAudioOutput->audioOutput.bufferA, 0, sSaiAudioOutput->audioOutput.bufferSize);
    memset(sSaiAudioOutput->audioOutput.bufferB, 0, sSaiAudioOutput->audioOutput.bufferSize);
    sSaiAudioOutput->audioOutput.audioOutputCallback.onPlaying();
    while (sSaiAudioOutput->audioOutput.running > 0)
    {
        int result = 0;
        osSemaphoreWait(&sSaiAudioOutput->semaphore, OS_SEMAPHORE_MAX_WAIT_TIME);
        if (sSaiAudioOutput->audioOutput.exchange > 0)
        {
            MX_SAI1_Send(sSaiAudioOutput->audioOutput.bufferA, sSaiAudioOutput->audioOutput.bufferSize / sizeof(int16_t));
            result = sSaiAudioOutput->audioOutput.audioOutputCallback.onReadDataStream(sSaiAudioOutput->audioOutput.bufferB, sSaiAudioOutput->audioOutput.bufferSize);
            
        }
        else
        {
            MX_SAI1_Send(sSaiAudioOutput->audioOutput.bufferB, sSaiAudioOutput->audioOutput.bufferSize / sizeof(int16_t));
            result = sSaiAudioOutput->audioOutput.audioOutputCallback.onReadDataStream(sSaiAudioOutput->audioOutput.bufferA, sSaiAudioOutput->audioOutput.bufferSize);
        }
        if (0 == result)
        {
            osTaskDetach(sSaiAudioOutput->tid);
            sSaiAudioOutput->audioOutput.running = 0;
        }
        sSaiAudioOutput->audioOutput.time += PERIOD_TIME;
        sSaiAudioOutput->audioOutput.audioOutputCallback.onPositionChanged(sSaiAudioOutput->audioOutput.time);
        sSaiAudioOutput->audioOutput.exchange ^= 1;
    }
    sSaiAudioOutput->audioOutput.audioOutputCallback.onStopped();
    return sSaiAudioOutput;
}

static int play(void *audioOutput)
{
    int ret = -1;
    SaiAudioOutput *saiAudioOutput = (SaiAudioOutput *)audioOutput;
    if (0 == saiAudioOutput->audioOutput.running)
    {
        saiAudioOutput->audioOutput.running = 1;
        ret = osTaskCreateRT(&saiAudioOutput->tid, audioPlayTask, audioOutput, "audio play", 30, OS_DEFAULT_TASK_STACK_SIZE);
        if (ret != 0)
        {
            saiAudioOutput->audioOutput.running = 0;
        }
    }
    return ret;
}

static int stop(void *audioOutput)
{
    int ret = -1;
    SaiAudioOutput *saiAudioOutput = (SaiAudioOutput *)audioOutput;
    if (saiAudioOutput->audioOutput.running > 0)
    {
        ret = 0;
        saiAudioOutput->audioOutput.running = 0;
        void *retval;
        osTaskJoin(&retval, saiAudioOutput->tid);
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
    int ret = audioOutputInit((AudioOutput *)saiAudioOutput, &audioOutputInfo, &audioOutputInterface);
    if (0 == ret)
    {
        ret = osSemaphoreCreate(&saiAudioOutput->semaphore, 1, 1);
        if (ret != 0)
        {
            audioOutputUninit((AudioOutput *)saiAudioOutput);
        }
    }
    return ret;
}

int saiAudioOutputUninit(SaiAudioOutput *saiAudioOutput)
{
    int ret = audioOutputUninit((AudioOutput *)saiAudioOutput);
    if (0 == ret)
    {
        ret = osSemaphoreDestory(&saiAudioOutput->semaphore);
    }
    return ret;
}