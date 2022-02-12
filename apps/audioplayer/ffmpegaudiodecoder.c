#include "ffmpegaudiodecoder.h"
#include <stdio.h>
#include "ostask.h"
static int prepare(void *audioDecoder)
{
    FFMPEGAudioDecoder *ffmpegAudioDecoder = (FFMPEGAudioDecoder *)audioDecoder;
    ffmpegAudioDecoder->audioDecoder.callback.onPrepared(ffmpegAudioDecoder->audioDecoder.callback.object, NULL);
    return 0;
}

static void *decodeTask(void *arg)
{
    FFMPEGAudioDecoder *ffmpegAudioDecoder = (FFMPEGAudioDecoder *)arg;
    ffmpegAudioDecoder->audioDecoder.callback.onDecodeStarting(ffmpegAudioDecoder->audioDecoder.callback.object);
    FILE *file = fopen(ffmpegAudioDecoder->audioDecoder.src, "rb");
    if (file != NULL)
    {
        while (ffmpegAudioDecoder->isDecodeTaskWorking > 0)
        {
            uint8_t buffer[1024];
            size_t len = fread(buffer, 1, 1024, file);
            if (len > 0)
            {
                ffmpegAudioDecoder->audioDecoder.callback.onDecoded(ffmpegAudioDecoder->audioDecoder.callback.object, buffer, len);
            }
            else
            {
                ffmpegAudioDecoder->isDecodeTaskWorking = 0;
                osTaskDetach(ffmpegAudioDecoder->tid);
                ffmpegAudioDecoder->tid = 0;
            }
        }
        fclose(file);
    }
    else
    {
        ffmpegAudioDecoder->isDecodeTaskWorking = 0;
        osTaskDetach(ffmpegAudioDecoder->tid);
        ffmpegAudioDecoder->tid = 0;
    }
    ffmpegAudioDecoder->audioDecoder.callback.onDecodeStopped(ffmpegAudioDecoder->audioDecoder.callback.object);
    return NULL;
}

static int start(void *audioDecoder)
{
    int ret = -1;
    FFMPEGAudioDecoder *ffmpegAudioDecoder = (FFMPEGAudioDecoder *)audioDecoder;
    if (0 == ffmpegAudioDecoder->tid && 0 == ffmpegAudioDecoder->isDecodeTaskWorking)
    {
        ffmpegAudioDecoder->isDecodeTaskWorking = 1;
        ret = osTaskCreate(&ffmpegAudioDecoder->tid, decodeTask, ffmpegAudioDecoder, "audio decode", 20, 1024 * 1024);
        if (ret != 0)
        {
            ffmpegAudioDecoder->isDecodeTaskWorking = 0;
        }
    }
    return ret;
}

static int stop(void *audioDecoder)
{
    int ret = -1;
    FFMPEGAudioDecoder *ffmpegAudioDecoder = (FFMPEGAudioDecoder *)audioDecoder;
    if (ffmpegAudioDecoder->tid > 0)
    {
        ret = 0;
        ffmpegAudioDecoder->isDecodeTaskWorking = 0;
        void *retval = 0;
        osTaskJoin(&retval, ffmpegAudioDecoder->tid);
        ffmpegAudioDecoder->tid = 0;
    }
    return ret;
}

int ffmpegAudioDecoderInit(FFMPEGAudioDecoder *ffmpegAudioDecoder, const AudioInfo *outputInfo)
{
    AudioDecoderInterface audioDecoderInterface;
    audioDecoderInterface.prepare = prepare;
    audioDecoderInterface.start = start;
    audioDecoderInterface.stop = stop;
    audioDecoderInit(&ffmpegAudioDecoder->audioDecoder, &audioDecoderInterface, outputInfo);

    ffmpegAudioDecoder->isDecodeTaskWorking = 0;
    ffmpegAudioDecoder->tid = 0;
    return 0;
}

int ffmpegAudioDecoderUninit(FFMPEGAudioDecoder *ffmpegAudioDecoder)
{
    if (ffmpegAudioDecoder->tid > 0)
    {
        ffmpegAudioDecoder->isDecodeTaskWorking = 0;
        void *retval = 0;
        osTaskJoin(&retval, ffmpegAudioDecoder->tid);
        ffmpegAudioDecoder->tid = 0;
    }
    audioDecoderUninit(&ffmpegAudioDecoder->audioDecoder);
    return 0;
}

