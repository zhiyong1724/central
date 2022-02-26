#ifndef __FFMPEGAUDIODECODER_H__
#define __FFMPEGAUDIODECODER_H__
#include "audiodecoder.h"
#include "ostask.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
typedef struct FFMPEGAudioDecoder
{
    AudioDecoder audioDecoder;
    int isDecodeTaskWorking;
    os_tid_t tid;
    AVFormatContext *formatContext;
    AVCodec *codec;
    unsigned int audioStreamIndex;
} FFMPEGAudioDecoder;

int ffmpegAudioDecoderInit(FFMPEGAudioDecoder *ffmpegAudioDecoder, const AudioInfo *outputInfo);
int ffmpegAudioDecoderUninit(FFMPEGAudioDecoder *ffmpegAudioDecoder);
#endif