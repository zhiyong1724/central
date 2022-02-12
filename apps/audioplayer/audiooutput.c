#include "audiooutput.h"
int audioOutputInit(AudioOutput *audioOutput, const AudioOutputInfo *audioOutputInfo, const AudioOutputInterface *audioOutputInterface)
{
    audioOutput->audioOutputInfo = *audioOutputInfo;
    audioOutput->audioOutputInterface = *audioOutputInterface;
    return 0;
}

int audioOutputUninit(AudioOutput *audioOutput)
{
    return 0;
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