#include "ffmpegaudiodecoder.h"
#include <stdio.h>
#include "ostask.h"
static int prepare(void *audioDecoder)
{
    int ret = -1;
    FFMPEGAudioDecoder *ffmpegAudioDecoder = (FFMPEGAudioDecoder *)audioDecoder;
    if (NULL == ffmpegAudioDecoder->formatContext)
    {
        do
        {
            ret = avformat_open_input(&ffmpegAudioDecoder->formatContext, ffmpegAudioDecoder->audioDecoder.src, NULL, NULL);
            if (ret != 0)
            {
                printf("Call avformat_open_input fail.\n");
                char errorBuff[1024];
                av_strerror(ret, errorBuff, 1024);
                printf("%s\n", errorBuff);
                break;
            }
            ret = avformat_find_stream_info(ffmpegAudioDecoder->formatContext, NULL);
            if (ret != 0)
            {
                printf("Call avformat_find_stream_info fail.\n");
                char errorBuff[1024];
                av_strerror(ret, errorBuff, 1024);
                printf("%s\n", errorBuff);
                break;
            }
            for (ffmpegAudioDecoder->audioStreamIndex = 0; ffmpegAudioDecoder->audioStreamIndex < ffmpegAudioDecoder->formatContext->nb_streams; ffmpegAudioDecoder->audioStreamIndex++)
            {
                ffmpegAudioDecoder->codec = avcodec_find_decoder(ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->codec_id);
                if (NULL == ffmpegAudioDecoder->codec)
                {
                    printf("Call avcodec_find_decoder fail.\n");
                    break;
                }
                if (AVMEDIA_TYPE_AUDIO == ffmpegAudioDecoder->codec->type)
                {
                    break;
                }
            }
            if (ffmpegAudioDecoder->audioStreamIndex < ffmpegAudioDecoder->formatContext->nb_streams)
            {
                StreamInfo streamInfo;
                streamInfo.duration = ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->duration * ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->time_base.num / ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->time_base.den;
                strcpy(streamInfo.codecName, ffmpegAudioDecoder->codec->name);
                streamInfo.bitRate = ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->bit_rate;
                switch ((enum AVSampleFormat)ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->format)
                {
                case AV_SAMPLE_FMT_U8:
                case AV_SAMPLE_FMT_U8P:
                {
                    streamInfo.audioInfo.audioType = AUDIO_TYPE_PCM_U8;
                    break;
                }
                case AV_SAMPLE_FMT_S16:
                case AV_SAMPLE_FMT_S16P:
                {
                    streamInfo.audioInfo.audioType = AUDIO_TYPE_PCM_S16;
                    break;
                }
                case AV_SAMPLE_FMT_S32:
                case AV_SAMPLE_FMT_S32P:
                {
                    streamInfo.audioInfo.audioType = AUDIO_TYPE_PCM_S32;
                    break;
                }
                case AV_SAMPLE_FMT_S64:
                case AV_SAMPLE_FMT_S64P:
                {
                    streamInfo.audioInfo.audioType = AUDIO_TYPE_PCM_S64;
                    break;
                }
                case AV_SAMPLE_FMT_FLT:
                case AV_SAMPLE_FMT_FLTP:
                {
                    streamInfo.audioInfo.audioType = AUDIO_TYPE_PCM_FLOAT;
                    break;
                }
                case AV_SAMPLE_FMT_DBL:
                case AV_SAMPLE_FMT_DBLP:
                {
                    streamInfo.audioInfo.audioType = AUDIO_TYPE_PCM_DOUBLE;
                    break;
                }
                default:
                break;
                }
                streamInfo.audioInfo.numChannels = ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->channels;
                streamInfo.audioInfo.samplesPerSec = ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->sample_rate;
                streamInfo.audioInfo.bitsPerSample = av_get_bytes_per_sample((enum AVSampleFormat)ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->format) * 8;
                ffmpegAudioDecoder->audioDecoder.callback.onPrepared(ffmpegAudioDecoder->audioDecoder.callback.object, &streamInfo);
            }
        } while (0);
    }
    return ret;
}

static void *decodeTask(void *arg)
{
    FFMPEGAudioDecoder *ffmpegAudioDecoder = (FFMPEGAudioDecoder *)arg;
    ffmpegAudioDecoder->audioDecoder.callback.onDecodeStarting(ffmpegAudioDecoder->audioDecoder.callback.object);
    do
    {
        AVCodecContext *codecContext = avcodec_alloc_context3(ffmpegAudioDecoder->codec);
        if (NULL == codecContext)
        {
            printf("Call avcodec_alloc_context3 fail.\n");
            break;
        }
        avcodec_parameters_to_context(codecContext, ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar);
        int ret = avcodec_open2(codecContext, ffmpegAudioDecoder->codec, NULL);
        if (ret != 0)
        {
            printf("Call avcodec_open2 fail.\n");
            char errorBuff[1024];
            av_strerror(ret, errorBuff, 1024);
            printf("%s\n", errorBuff);
            break;
        }
        SwrContext *swrContext = swr_alloc_set_opts(NULL, av_get_default_channel_layout(2), AV_SAMPLE_FMT_S16, 48000,
                                                    ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->channel_layout, (enum AVSampleFormat)ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->format,
                                                    ffmpegAudioDecoder->formatContext->streams[ffmpegAudioDecoder->audioStreamIndex]->codecpar->sample_rate, 0, NULL);
        AVPacket *packet = av_packet_alloc();
        AVFrame *srcFrame = av_frame_alloc();
        AVFrame *destFrame = av_frame_alloc();
        while (ffmpegAudioDecoder->isDecodeTaskWorking > 0)
        {
            ret = av_read_frame(ffmpegAudioDecoder->formatContext, packet);
            if (0 == ret)
            {
                if (ffmpegAudioDecoder->audioStreamIndex == packet->stream_index)
                {
                    ret = avcodec_send_packet(codecContext, packet);
                    if (AVERROR(EAGAIN) == ret)
                    {
                        for (;;)
                        {
                            ret = avcodec_receive_frame(codecContext, srcFrame);
                            if (AVERROR(EAGAIN) == ret)
                            {
                                break;
                            }
                            else if (ret != 0)
                            {
                                printf("Call avcodec_receive_frame fail.\n");
                                char errorBuff[1024];
                                av_strerror(ret, errorBuff, 1024);
                                printf("%s\n", errorBuff);
                                break;
                            }
                            destFrame->channel_layout = av_get_default_channel_layout(2);
                            destFrame->format = AV_SAMPLE_FMT_S16;
                            destFrame->sample_rate = 48000;
                            ret = swr_convert_frame(swrContext, destFrame, srcFrame);
                            if (0 == ret)
                            {
                                ffmpegAudioDecoder->audioDecoder.callback.onDecoded(ffmpegAudioDecoder->audioDecoder.callback.object, destFrame->data[0], av_get_bytes_per_sample((enum AVSampleFormat)destFrame->format) * destFrame->nb_samples * destFrame->channels);
                            }
                            av_frame_unref(destFrame);
                            av_frame_unref(srcFrame);
                        }
                        avcodec_send_packet(codecContext, packet);
                    }
                    else if (ret != 0)
                    {
                        printf("Call avcodec_send_packet fail.\n");
                        char errorBuff[1024];
                        av_strerror(ret, errorBuff, 1024);
                        printf("%s\n", errorBuff);
                        break;
                    }
                }
            }
            else
            {
                avcodec_send_packet(codecContext, NULL);
                avcodec_flush_buffers(codecContext);
                break;
            }
            av_packet_unref(packet);
        }
        av_frame_free(&destFrame);
        av_frame_free(&srcFrame);
        av_packet_free(&packet);
        swr_free(&swrContext);
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        avformat_close_input(&ffmpegAudioDecoder->formatContext);
    } while (0);
    osTaskDetach(ffmpegAudioDecoder->tid);
    ffmpegAudioDecoder->tid = 0;
    ffmpegAudioDecoder->isDecodeTaskWorking = 0;
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
    ffmpegAudioDecoder->formatContext = NULL;
    ffmpegAudioDecoder->codec = NULL;
    ffmpegAudioDecoder->audioStreamIndex = 0;
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

