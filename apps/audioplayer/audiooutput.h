#ifndef __AUDIOOUTPUT_H__
#define __AUDIOOUTPUT_H__
#define PERIOD_TIME 20
typedef enum AudioOutputType
{
    AUDIO_OUTPUT_TYPE_PCM_S16,
    AUDIO_OUTPUT_TYPE_PCM_S32,
    AUDIO_OUTPUT_TYPE_PCM_S64,
    AUDIO_OUTPUT_TYPE_PCM_FLOAT,
    AUDIO_OUTPUT_TYPE_PCM_DOUBLE,
} AudioOutputType;

typedef struct AudioOutputInfo
{
    AudioOutputType audioOutputType;
    int numChannels;
    int samplesPerSec;
    int bitsPerSample;
} AudioOutputInfo;

typedef struct AudioOutputCallback
{
    int (*onReadDataStream)(void *object, void *buffer, int bufferSize);
    void (*onPlaying)(void *object);
    void (*onStopped)(void *object);
    void (*onPositionChanged)(void *object, int ms);
    void *object;
} AudioOutputCallback;

typedef struct AudioOutputInterface
{
    int (*play)(void *audioOutput);
    int (*stop)(void *audioOutput);
} AudioOutputInterface;

typedef struct AudioOutput
{
    AudioOutputInfo audioOutputInfo;
    AudioOutputInterface audioOutputInterface;
    AudioOutputCallback audioOutputCallback;
} AudioOutput;

int audioOutputInit(AudioOutput *audioOutput, const AudioOutputInfo *audioOutputInfo, const AudioOutputInterface *audioOutputInterface);
int audioOutputUninit(AudioOutput *audioOutput);
int audioOutputSetCallback(AudioOutput *audioOutput, const AudioOutputCallback *audioOutputCallback);
int audioOutputPlay(AudioOutput *audioOutput);
int audioOutputStop(AudioOutput *audioOutput);
#endif