/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __AUDIOPLAYBACK_H__
#define __AUDIOPLAYBACK_H__

#include <stddef.h> // size_t
#include <string.h> // memset
#include "Debug.h"
#include "Observable.h"

namespace Broadcom
{
namespace Media {

enum class AudioPlaybackEvents {
    BufferReady     // Event triggers when available space in buffer drops below threshold
};

struct AudioPlaybackOpenSettings {
    AudioPlaybackOpenSettings()
    {
        memset(this, 0, sizeof(*this));
    }
    uint32_t threshold; // Not currently supported (consider changing default in bape_playback.c)
    bool persistent;    // Allows multiple instances to connect to the display (requires a decoder)
    bool ignoreMasterVolume; // Allows audio to stand-out when the master volume is lowered, e.g. text to speech
};

struct AudioPlaybackParameters {
    AudioPlaybackParameters()
    {
        memset(this, 0, sizeof(*this));
    }
    uint32_t sampleRate;
    uint32_t bitsPerSample;
    uint8_t numOfChannels;
};

struct AudioPlaybackStatus {
    bool started;
    uint32_t queuedBytes;
    uint32_t fifoSize;
    uint32_t playedBytes;
};

struct BME_SO_EXPORT AudioPlayback : public Observable<AudioPlaybackEvents>
{
    AudioPlayback() {}
    virtual ~AudioPlayback() {}

    static AudioPlayback *open(const AudioPlaybackOpenSettings& audioPlaybackOpenSettings);
    virtual void close() = 0;
    virtual int start(const AudioPlaybackParameters& audioPlaybackParameters) = 0;
    virtual void stop() = 0;
    virtual void setVolume(float volume) = 0;
    virtual float getVolume() = 0;
    virtual AudioPlaybackStatus getStatus() = 0;

    // Returns number of bytes copied to buffer
    virtual int pushAudioChunk(const uint8_t *data, size_t size) = 0;
};


}  // namespace Media
}  // namespace Broadcom
#endif  // __AUDIOPLAYBACK_H__
