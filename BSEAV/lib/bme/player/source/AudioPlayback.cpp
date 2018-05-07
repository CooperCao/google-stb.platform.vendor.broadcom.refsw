/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "AudioPlayback.h"
#include <algorithm> // std::min()
#include <cmath>
#include <string.h>
#include <assert.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <stdio.h>

#include "nexus_platform.h"
#include "nexus_simple_audio_playback.h"
#include "nexus_pid_channel.h"
#include "nexus_simple_audio_decoder.h"
#include "nxclient.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "Debug.h" // TRLS_*
#include "Media.h"
#include "SimpleDecoderNxclient.h"
#include "Playpump.h"
#include "WavFormatHeader.h"
#include "BaseSource.h"
#include "AudioVolumeChange.h"

static const size_t AudioFifoBytes = 128U << 10;

namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(AudioNexusPlayback);

// ===================================================================
// Two underlying implementations - NEXUS_SimplePlayback and NEXUS_SimpleDecode

struct BME_SO_EXPORT AudioNexusPlayback : public AudioPlayback
{
    AudioNexusPlayback();
    virtual ~AudioNexusPlayback();

    static AudioPlayback *open(const AudioPlaybackOpenSettings &settings);
    void close() override;
    int start(const AudioPlaybackParameters &parmaters) override;
    void stop() override;
    void setVolume(float volume) override;
    float getVolume() override;
    AudioPlaybackStatus getStatus() override;

    // Returns number of bytes copied to buffer
    int pushAudioChunk(const uint8_t* data, size_t size) override;

    unsigned int                    _initialized;

    SimpleDecoderNxclient          *_simpleDecoder;
    NEXUS_SimpleAudioPlaybackHandle _handle;

    typedef AudioOutput<NEXUS_SimpleAudioPlaybackHandle, NEXUS_SimpleAudioPlaybackSettings> AudioPlaybackVolumeControl;
    AudioPlaybackVolumeControl     *_volume;
};

struct PlaypumpWithCallbacks : Playpump_t
{
    PlaypumpWithCallbacks(AudioPlayback *playback);
    void onError() override;
    void onData() override;
    AudioPlayback *playback;
};

struct BME_SO_EXPORT AudioNexusDecode : public AudioPlayback
{
    AudioNexusDecode() : _pump(static_cast<AudioPlayback *>(this)), _initialized(0)
    {
    }
    virtual ~AudioNexusDecode();

    static AudioPlayback *open(const AudioPlaybackOpenSettings &settings);
    void close() override;
    int start(const AudioPlaybackParameters &parmaters) override;
    void stop() override;
    void setVolume(float volume) override;
    float getVolume() override;
    AudioPlaybackStatus getStatus() override;

    // Returns number of bytes copied to buffer
    int pushAudioChunk(const uint8_t* data, size_t size) override;

    unsigned int                     _initialized;

    SimpleDecoderNxclient           *_simpleDecoder;
    PlaypumpWithCallbacks            _pump;
    NEXUS_PidChannelHandle           _pidChannel;
    NEXUS_SimpleAudioDecoderHandle   _decoder;
    NEXUS_SimpleAudioDecoderSettings _settings;
    IMedia::AudioParameters          _parameters;
    bool                             _mix;
    WavFormatHeader                 *_header;

    typedef AudioOutput<NEXUS_SimpleAudioDecoderHandle, NEXUS_SimpleAudioDecoderSettings> AudioDecoderVolumeControl;
    AudioDecoderVolumeControl *_volume;

    bool initialize(const AudioPlaybackOpenSettings &settings);
    void deinitialize();
};


// ===================================================================

AudioPlayback *AudioPlayback::open(const AudioPlaybackOpenSettings &settings)
{
    // AudioNexusDecode is required for MS12 fade support
    return settings.persistent ? AudioNexusDecode::open(settings) : AudioNexusPlayback::open(settings);
}

static void bufferReady(void *context, int param)
{
    BME_DEBUG_TRACE(("buffer ready callback"));
    AudioPlayback *audioPlayback = static_cast<AudioPlayback *>(context);
    audioPlayback->notify(AudioPlaybackEvents::BufferReady);
}


// ===================================================================
// AudioNexusPlayback

AudioNexusPlayback::AudioNexusPlayback() : _initialized(0)
{
    _simpleDecoder = new SimpleDecoderNxclient();
    _handle = 0;
}

AudioNexusPlayback::~AudioNexusPlayback()
{
    close();
    if (_simpleDecoder) {
        delete _simpleDecoder;
        _simpleDecoder = 0;
    }
}

AudioPlayback *AudioNexusPlayback::open(const AudioPlaybackOpenSettings& audioPlaybackOpenSettings)
{
    AudioNexusPlayback *instance = new AudioNexusPlayback();
    if (instance == 0) {
        BME_DEBUG_ERROR(("Failed allocation in %s", __PRETTY_FUNCTION__));
        return 0;
    }

    // Get the necessary resources
    instance->_simpleDecoder->init();
    instance->_handle = instance->_simpleDecoder->acquireSimpleAudioPlayback();
    if (instance->_handle == 0) {
        BME_DEBUG_ERROR(("acquireSimpleAudioPlayback failed in ", __PRETTY_FUNCTION__));
        return 0;
    }
    instance->_simpleDecoder->connect();
    ++instance->_initialized;

    instance->_volume = new AudioPlaybackVolumeControl(
        instance->_handle, audioPlaybackOpenSettings.ignoreMasterVolume, "AudioPlayback");
    if (instance->_volume == 0) {
        BME_DEBUG_ERROR(("AudioPlaybackVolumeControl allocation failed in %s", __PRETTY_FUNCTION__));
        instance->close();
        return 0;
    }
    ++instance->_initialized;

    return instance;
}

void AudioNexusPlayback::close()
{
    switch (_initialized) {
    case 3:
        stop();

    case 2:
        delete _volume;

    case 1:
        _simpleDecoder->disconnect();
        _simpleDecoder->releaseSimpleAudioPlayback(_handle);
        _handle = 0;
        _simpleDecoder->uninit();

    default:
    case 0:
        break;
    }
    _initialized = 0;
}

int AudioNexusPlayback::start(const AudioPlaybackParameters &parameters)
{
    if (_initialized != 2)
        return NEXUS_OS_ERROR;

    NEXUS_Error rc;
    NEXUS_SimpleAudioPlaybackStartSettings startSettings;
    NEXUS_SimpleAudioPlayback_GetDefaultStartSettings(&startSettings);
    startSettings.dataCallback.callback = bufferReady;
    startSettings.dataCallback.context = this;
    startSettings.sampleRate = parameters.sampleRate;
    startSettings.bitsPerSample = parameters.bitsPerSample;
    startSettings.stereo = parameters.numOfChannels > 1;
    startSettings.loopAround = false;   // This is not used for looping the whole stream

    rc = NEXUS_SimpleAudioPlayback_Start(_handle, &startSettings);
    if (rc)
        return static_cast<int>(BERR_TRACE(rc));

    ++_initialized;
    return static_cast<int>(rc);
}

void AudioNexusPlayback::stop()
{
    if (_initialized != 3)
        return;

    NEXUS_SimpleAudioPlayback_Stop(_handle);
    _initialized = 2;
}

void AudioNexusPlayback::setVolume(float volume)
{
    _volume->setVolume(volume);
}

float AudioNexusPlayback::getVolume()
{
    return _volume->getVolume();
}

AudioPlaybackStatus AudioNexusPlayback::getStatus()
{
    AudioPlaybackStatus audioPlaybackStatus;
    NEXUS_SimpleAudioPlaybackStatus status;
    NEXUS_Error rc = NEXUS_SimpleAudioPlayback_GetStatus(_handle, &status);
    if (rc != NEXUS_SUCCESS) {
        BME_DEBUG_ERROR(("NEXUS_SimpleAudioPlayback_GetStatus,return code:%d", rc));
        return audioPlaybackStatus;
    }

    audioPlaybackStatus.started = status.started;
    audioPlaybackStatus.queuedBytes = status.queuedBytes;
    audioPlaybackStatus.fifoSize = status.fifoSize;
    audioPlaybackStatus.playedBytes = status.playedBytes;
    return audioPlaybackStatus;
}

int AudioNexusPlayback::pushAudioChunk(const uint8_t *data, size_t size)
{
    if (size == 0)
        return 0;

    void  *playbackBuffer;
    size_t playbackSize;
    NEXUS_Error rc = NEXUS_SimpleAudioPlayback_GetBuffer(
        _handle, &playbackBuffer, &playbackSize);
    if (rc != NEXUS_SUCCESS) {
        BME_DEBUG_ERROR(("NEXUS_SimpleAudioPlayback_GetBuffer, return code:%d", rc));
        return -1;
    }

    size_t sizeCopied = std::min(size, playbackSize);
    memcpy(playbackBuffer, data, sizeCopied);
    rc = NEXUS_SimpleAudioPlayback_WriteComplete(_handle, sizeCopied);
    if (rc != NEXUS_SUCCESS) {
        BME_DEBUG_ERROR(("Error in NEXUS_SimpleAudioPlayback_WriteComplete, return code:%d", rc));
        return -1;
    }

    return (int) sizeCopied;
}


// ===================================================================
// AudioNexusDecode

AudioNexusDecode::~AudioNexusDecode()
{
    deinitialize();
}

bool AudioNexusDecode::initialize(const AudioPlaybackOpenSettings &audioPlaybackOpenSettings)
{
    // Get the necessary resources
    if (_initialized != 0) {
        BME_DEBUG_ERROR(("Must close before re-opening"));
        return false;
    }

    // Initialisation that cannot fail
    _mix = audioPlaybackOpenSettings.persistent;

    // Initialisation that can fail
    _simpleDecoder = new SimpleDecoderNxclient();
    if (_simpleDecoder == 0) {
        BME_DEBUG_ERROR(("Out of memory in %s", __PRETTY_FUNCTION__));
        deinitialize();
        return false;
    }
    _simpleDecoder->init();
    ++_initialized;

    SimpleDecoder::acquireParameters acquireParam;
    memset(&acquireParam, 0, sizeof(acquireParam));
    acquireParam.requestAudioDecoder = true;
    acquireParam.persistent = _mix;
    _simpleDecoder->acquireSimpleDecoders(acquireParam, 0, &_decoder);
    if (_decoder == 0) {
        BME_DEBUG_ERROR(("acquireSimpleAudioPlayback failed in %s", __PRETTY_FUNCTION__));
        deinitialize();
        return false;
    }
    _simpleDecoder->connect();
    ++_initialized;

    _volume = new AudioDecoderVolumeControl(_decoder, audioPlaybackOpenSettings.ignoreMasterVolume, "AudioDecode");
    if (_volume == 0) {
        BME_DEBUG_ERROR(("Failed to intialise audio volume in %s", __PRETTY_FUNCTION__));
        deinitialize();
        return false;
    }
    ++_initialized;

    if (!_pump.initialise(false, AudioFifoBytes, 0 /* No DMA buffer required */)) {
        BME_DEBUG_ERROR(("Failed to intialise the playpump in %s", __PRETTY_FUNCTION__));
        deinitialize();
        return false;
    }
    ++_initialized;
    return true;
}

void AudioNexusDecode::deinitialize()
{
    switch (_initialized) {
    default:
    case 5:
        stop();

    case 4:
        _pump.deinitialise();

    case 3:
        delete _volume;

    case 2:
        _simpleDecoder->disconnect();
        _simpleDecoder->releaseSimpleDecoders(0, _decoder);

    case 1:
        _simpleDecoder->uninit();
        delete _simpleDecoder;

    case 0:
        break;
    }
    _initialized = 0;
}

int AudioNexusDecode::start(const AudioPlaybackParameters &parameters)
{
    BME_CHECK(_initialized < 4); // Inverse assert ..
    if (_initialized == 5)
        return -1; // Already started

    _parameters.streamId         = 0xc0;
    _parameters.substreamId      = 0;
    _parameters.audioCodec       = IMedia::PcmWavAudioCodec;
    _parameters.samplesPerSecond = parameters.sampleRate;
    _parameters.bitsPerSample    = parameters.bitsPerSample;
    _parameters.numChannels      = parameters.numOfChannels;
    _header = new WavFormatHeader(_parameters);
    if (_header == 0) {
        BME_DEBUG_ERROR(("Unable to Open playpump audio pid channel"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        return -1;
    }

    NEXUS_PlaypumpOpenPidChannelSettings pidSettings;
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidSettings);
    pidSettings.pidType                     = NEXUS_PidType_eAudio;
    pidSettings.pidTypeSettings.audio.codec = NEXUS_AudioCodec_ePcmWav;
    _pidChannel = NEXUS_Playpump_OpenPidChannel(_pump.handle, _parameters.streamId, &pidSettings);
    if (_pidChannel == 0) {
        BME_DEBUG_ERROR(("Unable to Open playpump audio pid channel"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        return -1;
    }

    NEXUS_SimpleAudioDecoderStartSettings start;
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&start);
    start.primary.codec      = BaseSource::convertAudioCodec(_parameters.audioCodec);
    start.primary.pidChannel = _pidChannel;
    if (_mix)
        start.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eApplicationAudio;
    NEXUS_SimpleAudioDecoder_Start(_decoder, &start);
    ++_initialized;
    return 0;
}

void AudioNexusDecode::stop()
{
    BME_CHECK(_initialized < 4); // Inverse assert ..
    if (_initialized == 4)
        return; // Already stopped

    NEXUS_SimpleAudioDecoder_Stop(_decoder);
    NEXUS_Playpump_ClosePidChannel(_pump.handle, _pidChannel);
    delete _header;
    --_initialized;
}

float AudioNexusDecode::getVolume()
{
    return _volume->getVolume();
}

void AudioNexusDecode::setVolume(float level)
{
    _volume->setVolume(level);
}

AudioPlaybackStatus AudioNexusDecode::getStatus()
{
    NEXUS_AudioDecoderStatus decoderStatus;
    BME_CHECK(NEXUS_SimpleAudioDecoder_GetStatus(_decoder, &decoderStatus));

    // Note that the byte counts will not match what the user has sent, as we add WAV and PES encapsulation
    AudioPlaybackStatus status;
    status.started     = decoderStatus.started == NEXUS_AudioRunningState_eStarted;
    status.queuedBytes = decoderStatus.fifoDepth;
    status.fifoSize    = decoderStatus.fifoSize;
    status.playedBytes = decoderStatus.numBytesDecoded;
    return status;
}

int AudioNexusDecode::pushAudioChunk(const uint8_t *data, size_t size)
{
    size = std::min<size_t>(0x4000U, size);
    if (!_pump.getPlaypumpSpace(size))
        return 0;

    DataFragment_t fragment(data, size);
    _pump.makePcmChunk(_header, _parameters.streamId, ~0ULL, &fragment, 1);
    _pump.pushClearChunk();
    return static_cast<int>(size);
}

AudioPlayback *AudioNexusDecode::open(const AudioPlaybackOpenSettings &settings)
{
    AudioNexusDecode *instance = new AudioNexusDecode();
    if (instance == 0)
        return 0;

    if (!instance->initialize(settings)) {
        delete instance;
        return 0;
    }
    return instance;
}

void AudioNexusDecode::close()
{
    delete this;
}

PlaypumpWithCallbacks::PlaypumpWithCallbacks(AudioPlayback *playback) : playback(playback)
{
}

void PlaypumpWithCallbacks::onError()
{
}

void PlaypumpWithCallbacks::onData()
{
    playback->notify(AudioPlaybackEvents::BufferReady);
}

}  // namespace Media
}  // namespace Broadcom
