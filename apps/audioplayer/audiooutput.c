#include "audiooutput.h"
#include <stdlib.h>
int audioOutputInit(AudioOutput *audioOutput, const AudioOutputInfo *audioOutputInfo, const AudioOutputInterface *audioOutputInterface)
{
    int ret = -1;
    audioOutput->audioOutputInfo = *audioOutputInfo;
    audioOutput->audioOutputInterface = *audioOutputInterface;
    audioOutput->running = 0;
    audioOutput->time = 0;
    audioOutput->exchange = 0;
    audioOutput->bufferSize = audioOutputInfo->samplesPerSec * PERIOD_TIME / 1000 * audioOutputInfo->bitsPerSample / 8 * audioOutputInfo->numChannels;
    audioOutput->bufferA = malloc(audioOutput->bufferSize);
    if (audioOutput->bufferA != NULL)
    {
        audioOutput->bufferB = malloc(audioOutput->bufferSize);
        if (audioOutput->bufferB != NULL)
        {
            ret = 0;
        }
        else
        {
            free(audioOutput->bufferA);
            audioOutput->bufferA = NULL;
        }
    }
    return ret;
}

int audioOutputUninit(AudioOutput *audioOutput)
{
    int ret = -1;
    if (audioOutput->bufferA != NULL)
    {
        free(audioOutput->bufferA);
        audioOutput->bufferA = NULL;
        if (audioOutput->bufferB != NULL)
        {
            free(audioOutput->bufferB);
            audioOutput->bufferB = NULL;
            ret = 0;
        }
    }
    return ret;
}

int audioOutputSetCallback(AudioOutput *audioOutput, const AudioOutputCallback *audioOutputCallback)
{
    audioOutput->audioOutputCallback = *audioOutputCallback;
    return 0;
}

int audioOutputPlay(AudioOutput *audioOutput)
{
    return audioOutput->audioOutputInterface.play(audioOutput);
}

int audioOutputStop(AudioOutput *audioOutput)
{
    return audioOutput->audioOutputInterface.stop(audioOutput);
}