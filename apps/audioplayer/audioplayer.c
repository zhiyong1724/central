#include "audioplayer.h"
#include "ffmpegaudiodecoder.h"
#include "osmem.h"
#include <string.h>
static void onPrepared(void *object, const AudioInfo *audioInfo)
{
    AudioPlayer *audioPlayer = (AudioPlayer *)object;
    if (object != NULL)
    {
        audioPlayer->callBack.onPrepared(audioPlayer->callBack.object, audioInfo);
    }
}

static void onDecoded(void *object, uint8_t *data, int dataSize)
{
    AudioPlayer *audioPlayer = (AudioPlayer *)object;
    if (object != NULL)
    {
        osMutexLock(&audioPlayer->mutex);
        int unitSize = osQueueUnitSize(&audioPlayer->buffer);
        int size = dataSize / unitSize;
        for (int i = 0; i < size; i++)
        {
            osQueuePush(&audioPlayer->buffer, &data[i * unitSize]);
        }
        osMutexUnlock(&audioPlayer->mutex);
        while (osQueueSize(&audioPlayer->buffer) > 48000)
        {
            osTaskSleep(50);
        }
    }
}

static void onDecodeStarting(void *object)
{
}

static void onDecodeStopped(void *object)
{
    AudioPlayer *audioPlayer = (AudioPlayer *)object;
    if (object != NULL)
    {
        audioPlayer->isDecoding = 0;
    }
}

static int onReadDataStream(void *object, void *buffer, int bufferSize)
{
    int ret = 0;
    AudioPlayer *audioPlayer = (AudioPlayer *)object;
    if (object != NULL)
    {
        int unitSize = osQueueUnitSize(&audioPlayer->buffer);
        while (audioPlayer->isDecoding > 0 && osQueueSize(&audioPlayer->buffer) * unitSize < bufferSize)
        {
            osTaskSleep(50);
        }
        osMutexLock(&audioPlayer->mutex);
        int size = bufferSize / unitSize;
        uint8_t *buff = (uint8_t *)buffer;
        int i = 0;
        for (; i < size; i++)
        {
            void *data = osQueueFront(&audioPlayer->buffer);
            if (data != NULL)
            {
                memcpy(&buff[i * unitSize], data, unitSize);
            }
            else
            {
                break;
            }
            osQueuePop(&audioPlayer->buffer);
        }
        osMutexUnlock(&audioPlayer->mutex);
        ret = i * unitSize;
    }
    return ret;
}

static void onPlaying(void *object)
{
    AudioPlayer *audioPlayer = (AudioPlayer *)object;
    if (object != NULL)
    {
        audioPlayer->callBack.onPlaying(audioPlayer->callBack.object);
    }
}

static void onStopped(void *object)
{
    AudioPlayer *audioPlayer = (AudioPlayer *)object;
    if (object != NULL)
    {
        audioPlayer->callBack.onStopped(audioPlayer->callBack.object);
    }
}

static void onPositionChanged(void *object, int ms)
{
    AudioPlayer *audioPlayer = (AudioPlayer *)object;
    if (object != NULL)
    {
        audioPlayer->callBack.onPositionChanged(audioPlayer->callBack.object, ms);
    }
}

int audioPlayerInit(AudioPlayer *audioPlayer, const AudioOutput *audioOutput)
{
    audioPlayer->isDecoding = 0;
    
    audioPlayer->audioOutput = (AudioOutput *)audioOutput;
    AudioOutputCallback audioOutputCallback;
    audioOutputCallback.object = audioPlayer;
    audioOutputCallback.onPlaying = onPlaying;
    audioOutputCallback.onStopped = onStopped;
    audioOutputCallback.onPositionChanged = onPositionChanged;
    audioOutputCallback.onReadDataStream = onReadDataStream;
    audioOutputSetCallback(audioPlayer->audioOutput, &audioOutputCallback);

    audioPlayer->audioDecoder = (AudioDecoder *)osMalloc(sizeof(FFMPEGAudioDecoder));
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.audioType = AUDIO_TYPE_PCM_S16;
    audioStreamInfo.bitsPerSample = 16;
    audioStreamInfo.numChannels = 2;
    audioStreamInfo.samplesPerSec = 48000;
    ffmpegAudioDecoderInit((FFMPEGAudioDecoder *)audioPlayer->audioDecoder, &audioStreamInfo);
    AudioDecoderCallback audioDecoderCallback;
    audioDecoderCallback.object = audioPlayer;
    audioDecoderCallback.onPrepared = onPrepared;
    audioDecoderCallback.onDecoded = onDecoded;
    audioDecoderCallback.onDecodeStarting = onDecodeStarting;
    audioDecoderCallback.onDecodeStopped = onDecodeStopped;
    audioDecoderSetCallback(audioPlayer->audioDecoder, &audioDecoderCallback);

    osQueueInit(&audioPlayer->buffer, audioStreamInfo.bitsPerSample / 8 * audioStreamInfo.numChannels);
    osMutexCreate(&audioPlayer->mutex);
    return 0;
}

int audioPlayerUninit(AudioPlayer *audioPlayer)
{
    if (audioPlayer->audioDecoder != NULL)
    {
        ffmpegAudioDecoderUninit((FFMPEGAudioDecoder *)audioPlayer->audioDecoder);
        osFree((FFMPEGAudioDecoder *)audioPlayer->audioDecoder);
        audioPlayer->audioDecoder = NULL;
    }
    osQueueFree(&audioPlayer->buffer);
    osMutexDestory(&audioPlayer->mutex);
    return 0;
}

int audioPlayerSetInput(AudioPlayer *audioPlayer, const char *src)
{
    return audioDecoderSetInput(audioPlayer->audioDecoder, src);
}

int audioPlayerSetCallback(AudioPlayer *audioPlayer, AudioPlayerCallback *callback)
{
    audioPlayer->callBack = *callback;
    return 0;
}

int audioPlayerPrepare(AudioPlayer *audioPlayer)
{
    return audioDecoderPrepare(audioPlayer->audioDecoder);
}

int audioPlayerPlay(AudioPlayer *audioPlayer)
{
    if (audioPlayer->audioDecoder != NULL && audioPlayer->audioOutput != NULL)
    {
        audioDecoderStart(audioPlayer->audioDecoder);
        audioPlayer->isDecoding = 1;
        audioOutputPlay(audioPlayer->audioOutput);
    }
    return 0;
}

int audioPlayerStop(AudioPlayer *audioPlayer)
{
    if (audioPlayer->audioDecoder != NULL && audioPlayer->audioOutput != NULL)
    {
        audioDecoderStop(audioPlayer->audioDecoder);
        audioOutputStop(audioPlayer->audioOutput);
    }
    return 0;
}