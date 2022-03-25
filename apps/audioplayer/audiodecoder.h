#ifndef __AUDIODECODER_H__
#define __AUDIODECODER_H__
#include <stdint.h>
typedef enum AudioType
{
    AUDIO_TYPE_PCM_U8,
    AUDIO_TYPE_PCM_S16,
    AUDIO_TYPE_PCM_S32,
    AUDIO_TYPE_PCM_S64,
    AUDIO_TYPE_PCM_FLOAT,
    AUDIO_TYPE_PCM_DOUBLE,
} AudioType;

typedef struct AudioStreamInfo
{
    AudioType audioType;
    int numChannels;
    int samplesPerSec;
    int bitsPerSample;
} AudioStreamInfo;

typedef struct AudioInfo
{
    int duration;
    char codecName[256];
    int bitRate;
    AudioStreamInfo audioStreamInfo;
} AudioInfo;

typedef struct AudioDecoderCallback
{
    void (*onPrepared)(void *object, const AudioInfo *audioInfo);
    void (*onDecoded)(void *object, uint8_t *data, int dataSize);
    void (*onDecodeStarting)(void *object);
    void (*onDecodeStopped)(void *object);
    void *object;
} AudioDecoderCallback;

typedef struct AudioDecoderInterface
{
    int (*prepare)(void *audioDecoder);
    int (*start)(void *audioDecoder);
    int (*stop)(void *audioDecoder);
} AudioDecoderInterface;

typedef struct AudioDecoder
{
    char *src;
    AudioStreamInfo outputInfo;
    AudioDecoderInterface interface;
    AudioDecoderCallback callback;
} AudioDecoder;

int audioDecoderInit(AudioDecoder *audioDecoder, const AudioDecoderInterface *audioDecoderInterface, const AudioStreamInfo *outputInfo);
int audioDecoderUninit(AudioDecoder *audioDecoder);
int audioDecoderSetInput(AudioDecoder *audioDecoder, const char *src);
int audioDecoderSetCallback(AudioDecoder *audioDecoder, AudioDecoderCallback *callback);
int audioDecoderPrepare(AudioDecoder *audioDecoder);
int audioDecoderStart(AudioDecoder *audioDecoder);
int audioDecoderStop(AudioDecoder *audioDecoder);
#endif