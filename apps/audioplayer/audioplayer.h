#ifndef __AUDIOPLAYER_H__
#define __AUDIOPLAYER_H__
#include "audiooutput.h"
#include "audiodecoder.h"
#include "osqueue.h"
#include "osmutex.h"
typedef struct AudioPlayerCallback
{
    void (*onPrepared)(void *object, const AudioInfo *audioInfo);
    void (*onPlaying)(void *object);
    void (*onStopped)(void *object);
    void (*onPositionChanged)(void *object, int ms);
    void *object;
} AudioPlayerCallback;

typedef struct AudioPlayer
{
    AudioDecoder *audioDecoder;
    AudioOutput *audioOutput;
    AudioPlayerCallback callBack;
    int isDecoding;
    OsQueue buffer;
    OsMutex mutex;
} AudioPlayer;

int audioPlayerInit(AudioPlayer *audioPlayer, const AudioOutput *audioOutput);
int audioPlayerUninit(AudioPlayer *audioPlayer);
int audioPlayerSetInput(AudioPlayer *audioPlayer, const char *src);
int audioPlayerSetCallback(AudioPlayer *audioPlayer, AudioPlayerCallback *callback);
int audioPlayerPrepare(AudioPlayer *audioPlayer);
int audioPlayerPlay(AudioPlayer *audioPlayer);
int audioPlayerStop(AudioPlayer *audioPlayer);
#endif