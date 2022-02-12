#ifndef __FFMPEGAUDIODECODER_H__
#define __FFMPEGAUDIODECODER_H__
#include "audiodecoder.h"
#include "ostask.h"
typedef struct FFMPEGAudioDecoder
{
    AudioDecoder audioDecoder;
    int isDecodeTaskWorking;
    os_tid_t tid;
} FFMPEGAudioDecoder;

int ffmpegAudioDecoderInit(FFMPEGAudioDecoder *ffmpegAudioDecoder, const AudioInfo *outputInfo);
int ffmpegAudioDecoderUninit(FFMPEGAudioDecoder *ffmpegAudioDecoder);
#endif