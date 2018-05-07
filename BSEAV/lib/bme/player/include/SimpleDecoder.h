/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __BME_SIMPLEDECODER_H_
#define __BME_SIMPLEDECODER_H_
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_audio_playback.h"
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "nexus_simple_encoder.h"
#endif
#include <string>

namespace Broadcom
{
namespace Media {

#define DECODER_NOT_AVAILABLE 0
#define DECODER_ACQUIRED 1

class MediaPlayer;
struct SimpleXcodeHandle;

class SimpleDecoder
{
    public:
        enum VideoWindow {
            VideoWindowMain,
            VideoWindowPip
        };

        class acquireParameters
        {
            public:
                MediaPlayer* mediaPlayer;
                bool requestVideoDecoder;
                bool requestAudioDecoder;
                VideoWindow videoWindow;
                uint16_t maxWidth;
                uint16_t maxHeight;
                uint16_t virtualWidth;
                uint16_t virtualHeight;
                uint16_t decoderType;
                uint32_t surfaceClientId;
                uint32_t windowId;
                uint16_t colorDepth;
                bool secureVideo;
                bool persistent;
        };

 public:
    SimpleDecoder() {}
    virtual ~SimpleDecoder() {}

    virtual void init() = 0;
    virtual void uninit() = 0;

    virtual void acquireSimpleDecoders(const acquireParameters& param,
            NEXUS_SimpleVideoDecoderHandle* videoDecoder,
            NEXUS_SimpleAudioDecoderHandle* audioDecoder) = 0;

    virtual void releaseSimpleDecoders(NEXUS_SimpleVideoDecoderHandle videoDecoder,
            NEXUS_SimpleAudioDecoderHandle audioDecoder) = 0;
#ifdef NEXUS_HAS_VIDEO_ENCODER
    virtual SimpleXcodeHandle* acquireXcodeResource(bool nrtMode, bool audioXcode,
                                                    int maxWidth, int maxHeight,
                                                    int colorDepth, bool hdmiInput) = 0;
    virtual void releaseXcodeResource(SimpleXcodeHandle* xcodeHandle) = 0;
#endif
    virtual NEXUS_SimpleAudioPlaybackHandle acquireSimpleAudioPlayback() = 0;
    virtual void releaseSimpleAudioPlayback(NEXUS_SimpleAudioPlaybackHandle audioPlayback) = 0;

    virtual void setLocation(int x, int y, uint32_t width, uint32_t height) = 0;
    virtual void setVisibility(bool visible) = 0;
    virtual uint32_t getVideoDisplayWidth() = 0;
    virtual uint32_t getVideoDisplayHeight() = 0;
    virtual void setAudioVolume(int32_t volume) = 0;
    virtual int32_t getAudioVolume() = 0;
    virtual void mute(bool muted) = 0;

    virtual void connect() = 0;
    virtual void disconnect() = 0;

 protected:
    uint32_t _surfaceClientId;
    uint32_t _windowId;
};
}  // namespace Media
}  // namespace Broadcom
#endif  // __BME_SIMPLEDECODER_H_