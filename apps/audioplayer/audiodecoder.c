#include "audiodecoder.h"
#include <string.h>
#include "osmem.h"
int audioDecoderInit(AudioDecoder *audioDecoder, const AudioDecoderInterface *audioDecoderInterface, const AudioStreamInfo *outputInfo)
{
    audioDecoder->src = NULL;
    audioDecoder->interface = *audioDecoderInterface;
    audioDecoder->outputInfo = *outputInfo;
    return 0;
}

int audioDecoderUninit(AudioDecoder *audioDecoder)
{
    if (audioDecoder->src != NULL)
    {
        osFree(audioDecoder->src);
        audioDecoder->src = NULL;
    }
    return 0;
}

int audioDecoderSetInput(AudioDecoder *audioDecoder, const char *src)
{
    if (audioDecoder->src != NULL)
    {
        osFree(audioDecoder->src);
        audioDecoder->src = NULL;
    }
    audioDecoder->src = (char *)osMalloc(strlen(src) + 1);
    strcpy(audioDecoder->src, src);
    return 0;
}

int audioDecoderSetCallback(AudioDecoder *audioDecoder, AudioDecoderCallback *callback)
{
    audioDecoder->callback = *callback;
    return 0;
}

int audioDecoderPrepare(AudioDecoder *audioDecoder)
{
    return audioDecoder->interface.prepare(audioDecoder);
}

int audioDecoderStart(AudioDecoder *audioDecoder)
{
    return audioDecoder->interface.start(audioDecoder);
}

int audioDecoderStop(AudioDecoder *audioDecoder)
{
    return audioDecoder->interface.stop(audioDecoder);
}