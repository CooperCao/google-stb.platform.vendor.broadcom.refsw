/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <thread>
#include <string>
#include <cstring>
#include "PushTSSource.h"
#include "nexus_timebase.h"
#include "bmedia_util.h"
#include "nexus_types.h"
#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_playback.h"
#include "bmedia_pcm.h"
#include "bmedia_probe.h"
#include "nexus_simple_video_decoder_server.h"
#include "nexus_video_window.h"

#include "bfile_stdio.h"

namespace Broadcom
{
namespace Media {

TRLS_DBG_MODULE(PushTSSource);

typedef struct tagBMEPushTSEngineContext {
    uint16_t pcrPid;
    uint16_t audioPid;
    uint16_t videoPid;
    uint16_t extVideoPid;

    uint32_t videoWidth;
    uint32_t videoHeight;

    BKNI_EventHandle stateChanged;
    BKNI_EventHandle event;

    NEXUS_TransportType transportType;
    NEXUS_VideoCodec videoCodec;
    NEXUS_VideoCodec extVideoCodec;
    NEXUS_AudioCodec audioCodec;

    NEXUS_PlaypumpHandle playpump;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;

    NEXUS_PidChannelHandle videoExtPidChannel;
} BMEPushTSEngineContext;

PushTSSource::PushTSSource() :
    _context(NULL),
    _endOfStream(false),
    _targetPts(0),
    _lastPumpLevel(0),
    _pumpBufState(E_IDLE),
    _nexusFirstPtsPassed(false),
    _nexusDataExhausted(false),
    _inAccurateSeek(false),
    _playbackRate(1000),
    _state(IMedia::IdleState),
    _playTimeUpdated(false),
    _decodeTimeUpdated(false),
    _lastPlayTime(0),
    _lastDecodeVideoTime(0),
    _videoPaused(false)
{
    init();
}

PushTSSource::~PushTSSource()
{
    uninit();
    _pumpBufState = E_IDLE;

    if (_prepareAsyncThread.joinable()) {
        _prepareAsyncThread.join();
    }
    if (_context) {
        delete _context;
        _context = NULL;
    }
}

void PushTSSource::init()
{
    BME_DEBUG_ENTER();

    if (NULL == _context) {
        _context = new BMEPushTSEngineContext();
        std::memset(_context, 0, sizeof(BMEPushTSEngineContext));
    }

    if (NULL == _context->stateChanged) {
        BKNI_CreateEvent(&_context->stateChanged);
    }

    if (NULL == _context->event) {
        BKNI_CreateEvent(&_context->event);
    }

    BME_DEBUG_EXIT();
}

void PushTSSource::uninit()
{
    BME_DEBUG_ENTER();
    resetPlayState();

    if (_context) {
        if (_context->stateChanged) {
            BKNI_DestroyEvent(_context->stateChanged);
            _context->stateChanged = NULL;
        }

        if (_context->event) {
            BKNI_DestroyEvent(_context->event);
            _context->event = NULL;
        }

        stopPlayback(false);
    }
    _state = IMedia::IdleState;
    BME_DEBUG_EXIT();
}

bool PushTSSource::isRunning()
{
    if ((getState() == IMedia::StartedState) ||
        (getState() ==  IMedia::PausedState)) {
        return true;
    } else {
        return false;
    }
}

void PushTSSource::staticDataCallback(void *context, int param)
{
    BME_DEBUG_ENTER();
    PushTSSource *instance = reinterpret_cast<PushTSSource *>(context);
    BKNI_SetEvent((BKNI_EventHandle)instance->_context->event);
    reinterpret_cast<PushTSSource*>(context)->dataCallback();
    BME_DEBUG_EXIT();
}

void PushTSSource::dataCallback()
{
    BME_DEBUG_ENTER();
    postBufferLevel();
    onInfo(IMedia::MEDIA_INFO_PTS_UPDATE, getCurrentPosition());
    BME_DEBUG_EXIT();
}

void PushTSSource::staticErrorCallback(void *context, int param)
{
    TRLS_UNUSED(param);
    reinterpret_cast<PushTSSource*>(context)->errorCallback();
}

void PushTSSource::errorCallback()
{
    BME_DEBUG_ENTER();
    onError(IMedia::MEDIA_ERROR_UNKNOWN);
    BME_DEBUG_EXIT();
}

void PushTSSource::postBufferLevel()
{
    BME_DEBUG_ENTER();
    bool sendUpdate = false;
    NEXUS_PlaypumpStatus playpumpStatus;
    BME_CHECK(NEXUS_Playpump_GetStatus(_context->playpump, &playpumpStatus));

    if (_lastPumpLevel != playpumpStatus.fifoDepth) {
        _lastPumpLevel = playpumpStatus.fifoDepth * 100 / playpumpStatus.fifoSize;
        sendUpdate = true;
    }

    if (sendUpdate) {
        BME_DEBUG_TRACE(("buffer level is %u percent\n", _lastPumpLevel));
        onInfo(IMedia::MEDIA_INFO_LASTPUMP_UPDATE, _lastPumpLevel);
    }

    BME_DEBUG_EXIT();
}

void PushTSSource::resetPlayState()
{
    BME_DEBUG_ENTER();
    _inAccurateSeek = false;
    BME_DEBUG_EXIT();
}

void PushTSSource::acquireResources()
{
    BME_DEBUG_ENTER();
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.fifoSize *= 2;
    playpumpOpenSettings.numDescriptors = 200;
    // playpump requires heap with eFull mapping
    _context->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    int retry  = 0;

    while (_context->playpump  == NULL && retry < 10) {
        retry++;
        BME_DEBUG_ERROR(("Warning: NEXUS_Playpump_Open failed: reduce memory and try again: %d", retry));
        // GraphicsNexus::ReduceSurfaceMemory (1024*1024);  //SEAN how to reduce mem?
        _context->playpump  = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    }

    if (!_context->playpump) {
        BME_DEBUG_ERROR(("Unable to create NEXUS Playpump handle"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
    }

    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_Playpump_GetSettings(_context->playpump, &playpumpSettings);
    _context->transportType = NEXUS_TransportType_eTs;
    playpumpSettings.transportType = _context->transportType;
    playpumpSettings.dataCallback.callback = staticDataCallback;
    playpumpSettings.dataCallback.context = this;
    playpumpSettings.dataCallback.param = reinterpret_cast<size_t>(_context->event);
    playpumpSettings.errorCallback.callback = staticErrorCallback;
    playpumpSettings.errorCallback.context = this;
    NEXUS_Playpump_SetSettings(_context->playpump, &playpumpSettings);
    BME_CHECK(NEXUS_Playpump_Start(_context->playpump));
    BME_DEBUG_EXIT();
}

void PushTSSource::start()
{
    BME_DEBUG_ENTER();
    NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;
    acquireResources();

    if (_context->audioPid && getConnector()->audioDecoder) {
        // Open the audio pid channel
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eAudio;
        pidChannelSettings.pidTypeSettings.audio.codec = _context->audioCodec;
        _context->audioPidChannel = NEXUS_Playpump_OpenPidChannel(_context->playpump,
                                    _context->audioPid, &pidChannelSettings);

        if (!_context->audioPidChannel)  {
            BME_DEBUG_ERROR(("Unable to Open playpump audio pid channel"));
            BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        }
    }

    if (_context->videoCodec != NEXUS_VideoCodec_eNone && _context->videoPid != 0) {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eVideo;
        _context->videoPidChannel = NEXUS_Playpump_OpenPidChannel(_context->playpump,
                                    _context->videoPid, &pidChannelSettings);

        if (!_context->videoPidChannel) {
            BME_DEBUG_ERROR(("Unable to Open playpump video pid channel"));
            BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        }
    }

    if (_context->pcrPid != _context->audioPid && _context->pcrPid != _context->videoPid) {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eUnknown;
        _context->pcrPidChannel = NEXUS_Playpump_OpenPidChannel(_context->playpump,
                                  _context->pcrPid, &pidChannelSettings);

        if (!_context->pcrPidChannel) {
            BME_DEBUG_ERROR(("Unable to Open playpump prc pid channel"));
            BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        }
    } else {
        _context->pcrPidChannel = _context->videoPidChannel;
    }

    // Open the extra video pid channel if present in the stream
    if ((_context->extVideoCodec != NEXUS_VideoCodec_eNone) && (_context->extVideoPid != 0)) {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eVideo;
        _context->videoExtPidChannel = NEXUS_Playpump_OpenPidChannel(_context->playpump,
                                       _context->extVideoPid, &pidChannelSettings);

        if (!_context->videoExtPidChannel) {
            BME_DEBUG_ERROR(("Unable to Open playpump enhancement pid channel"));
            BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        }
    }

    NEXUS_PidChannelStatus                pidVideoStatus;
    NEXUS_PidChannelStatus                pidAudioStatus;
    BME_CHECK(NEXUS_PidChannel_GetStatus(_context->videoPidChannel,
                                          &pidVideoStatus));
    BME_CHECK(NEXUS_PidChannel_GetStatus(_context->audioPidChannel,
                                          &pidAudioStatus));
    BME_DEBUG_TRACE(("video PID: %u, audio PID: %u", pidVideoStatus.pidChannelIndex,
                      pidAudioStatus.pidChannelIndex));

    if (getConnector()->stcChannel) {
        NEXUS_SimpleStcChannelSettings stcSettings;
        NEXUS_SimpleStcChannel_GetSettings(getConnector()->stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_eAuto; /* playback */
        stcSettings.modeSettings.Auto.transportType = _context->transportType;
        BME_CHECK(NEXUS_SimpleStcChannel_SetSettings(getConnector()->stcChannel,
                   &stcSettings));
    }

    getConnector()->videoPidChannel = _context->videoPidChannel;
    getConnector()->audioPidChannel = _context->audioPidChannel;
    getConnector()->pcrPidChannel    = _context->pcrPidChannel;
    getConnector()->enhancementVideoPidChannel = _context->videoExtPidChannel;
    _state = IMedia::StartedState;
    _lastPlayTime = 0;
    _lastDecodeVideoTime = 0;
    BME_DEBUG_EXIT();
}

void PushTSSource::stop(bool holdLastFrame)
{
    stopPlayback(holdLastFrame);
}
void PushTSSource::stopPlayback(bool holdPicture)
{
    BME_DEBUG_ENTER();

    if (!isRunning()) {
        return;
    }

    // resetPlayState();

    if (_videoPaused != holdPicture) {
        if (getConnector()->videoDecoder) {
            NEXUS_VideoDecoderSettings videoDecoderSettings;
            NEXUS_SimpleVideoDecoder_GetSettings(getConnector()->videoDecoder, &videoDecoderSettings);
            videoDecoderSettings.channelChangeMode = \
                    holdPicture ? NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock : \
                    NEXUS_VideoDecoder_ChannelChangeMode_eMute;
            BME_CHECK(NEXUS_SimpleVideoDecoder_SetSettings(getConnector()->videoDecoder,
                       &videoDecoderSettings))
        }
    }

    if (_videoPaused) {
        updatePlayTimeLocked();
    } else {
        _lastPlayTime = 0;
    }

    if (_context) {
        if (_context->videoPidChannel) {
            NEXUS_PidChannel_Close(_context->videoPidChannel);
            _context->videoPidChannel = NULL;
        }

        if (_context->audioPidChannel) {
            NEXUS_PidChannel_Close(_context->audioPidChannel);
            _context->audioPidChannel = NULL;
        }

        if (_context->pcrPidChannel) {
            NEXUS_PidChannel_Close(_context->pcrPidChannel);
            _context->pcrPidChannel = NULL;
        }

        if (_context->videoExtPidChannel) {
            NEXUS_PidChannel_Close(_context->videoExtPidChannel);
            _context->videoExtPidChannel = NULL;
        }

        if (_context->playpump) {
            NEXUS_Playpump_Stop(_context->playpump);
            NEXUS_Playpump_Close(_context->playpump);
            _context->playpump = NULL;
        }

#ifdef USE_COMMON_DRM
        DRM_Vudu_Finalize();
#else
        // NEXUS_Security_FreeKeySlot(m_KeySlotHandle);
        // m_KeySlotHandle = NULL;
#endif  // USE_COMMON_DRM
    }

    _lastPlayTime = 0;
    _videoPaused = holdPicture;
    resetPlayState();
    _state = IMedia::StoppedState;
    BME_DEBUG_EXIT();
}

void PushTSSource::pause()
{
    BME_DEBUG_ENTER();
    BME_CHECK(NEXUS_SimpleStcChannel_SetRate(getConnector()->stcChannel, 0, 0));
    _state = IMedia::PausedState;
    BME_DEBUG_EXIT();
    return;
}

void PushTSSource::resumeFromPause()
{
    BME_DEBUG_ENTER();
    NEXUS_AudioDecoderTrickState audioState;
    NEXUS_VideoDecoderTrickState videoState;

    if (getConnector()->videoDecoder) {
        NEXUS_SimpleVideoDecoder_GetTrickState(getConnector()->videoDecoder, &videoState);
        videoState.rate = NEXUS_NORMAL_DECODE_RATE;
        videoState.tsmEnabled = NEXUS_TsmMode_eEnabled;
        BME_CHECK(NEXUS_SimpleVideoDecoder_SetTrickState(getConnector()->videoDecoder, &videoState));
    }

    if (getConnector()->audioDecoder) {
        NEXUS_SimpleAudioDecoder_GetTrickState(getConnector()->audioDecoder, &audioState);
        audioState.rate = NEXUS_NORMAL_DECODE_RATE;
        audioState.muted = false;
        BME_CHECK(NEXUS_SimpleAudioDecoder_SetTrickState(getConnector()->audioDecoder, &audioState));
        BME_CHECK(NEXUS_SimpleStcChannel_SetRate(getConnector()->stcChannel, 1, 0));
    }

    BME_DEBUG_EXIT();
}

static void prepareAsyncThread(void* data)
{
    PushTSSource* source = reinterpret_cast<PushTSSource*>(data);
    // BMEPushTSengineContext* _context = source->getContext();
    // make sure state didn't change while we are putting in fixed delay
    NEXUS_Error rc = BKNI_WaitForEvent(source->getContext()->stateChanged, 100);

    if (BERR_TIMEOUT == rc) {
        source->onPrepared();
    }
}

IMedia::ErrorType PushTSSource::prepare()
{
    BME_DEBUG_ENTER();
    _state = IMedia::StoppedState;
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}

void PushTSSource::prepareAsync()
{
    BME_DEBUG_ENTER();
    BKNI_ResetEvent(_context->stateChanged);
    BME_DEBUG_TRACE(("create prepareAsyncThread, is this expected?"));
    _state = IMedia::PreparingState;
    _prepareAsyncThread = std::thread(&prepareAsyncThread,
            static_cast<void*>(this));
    BME_DEBUG_EXIT();
}

void PushTSSource::setDataSource(MediaStream *mediaStream)
{
    IMedia::StreamMetadata metadata = mediaStream->metadata;
    IMedia::VideoParameters videoParam = metadata.videoParam;
    IMedia::AudioParameters audioParam = metadata.audioParamList[0];
    _context->transportType = convertStreamType(metadata.streamType);
    _context->videoCodec = convertVideoCodec(videoParam.videoCodec);
    _context->audioCodec = convertAudioCodec(audioParam.audioCodec);
    _context->videoPid =  videoParam.streamId;
    _context->audioPid = audioParam.streamId;
    _context->pcrPid  =  videoParam.substreamId;
    _context->videoWidth = videoParam.maxWidth;
    _context->videoHeight = videoParam.maxHeight;
}

E_PUMPBUF_STATE PushTSSource::pushMediaChunk(const IMedia::DataBuffer& buffer)
{
    BME_DEBUG_ENTER();
    void *                      playpump_buffer;
    size_t                    playpump_buffer_size;
    uint32_t size = buffer.getSize();
    const uint8_t *chunk = reinterpret_cast<const uint8_t *>(buffer.getData());
    BKNI_ResetEvent(_context->event);

    if (NULL == _context->playpump) {
        BME_DEBUG_EXIT();
        return E_ERROR;
    }

    BME_CHECK(NEXUS_Playpump_GetBuffer(_context->playpump, &playpump_buffer, &playpump_buffer_size));

    switch (_pumpBufState) {
    case E_IDLE:
        if (playpump_buffer_size != 0) {
            _pumpBufState = E_READY;
        }

        break;

    case E_READY:
        if (playpump_buffer_size == 0) {
            BKNI_WaitForEvent(_context->event, BKNI_INFINITE);
            BME_CHECK(NEXUS_Playpump_GetBuffer(_context->playpump, &playpump_buffer, &playpump_buffer_size));

            if (playpump_buffer_size == 0) {
                BME_DEBUG_ERROR(("Error in NEXUS_Playpump_GetBuffer,playpump_buffer_size:%d",
                                  playpump_buffer_size));
                return E_FULL;
            }
        }

        break;

    case E_FULL:
        BKNI_WaitForEvent(_context->event, BKNI_INFINITE);
        BME_CHECK(NEXUS_Playpump_GetBuffer(_context->playpump, &playpump_buffer, &playpump_buffer_size));

        if (playpump_buffer_size == 0) {
            BME_DEBUG_ERROR(("Error in NEXUS_Playpump_GetBuffer,playpump_buffer_size:%d",
                              playpump_buffer_size));
            return E_FULL;
        }

        break;

    case E_ERROR:
    default:
        BME_DEBUG_EXIT();
        return E_ERROR;
    }

    /* we still need a while loop here. There are two reasons.
       1. For big chunk data. most time when playpump has space abailable, at that time,
          the space size < request size, so we still have to wait a 2nd or 3rd time until
          the space is enough to hold the pass-in data.
       2. When (playpumpStatus.fifoSize - playpumpStatus.fifoDepth) >= size, if we
          feed data to buffer, it casues video flash.
     */
    while (playpump_buffer_size < size) {
        std::memcpy(playpump_buffer, chunk, playpump_buffer_size);
        BME_CHECK(NEXUS_Playpump_WriteComplete(_context->playpump, 0, playpump_buffer_size));
        size -= playpump_buffer_size;
        BKNI_WaitForEvent(_context->event, BKNI_INFINITE);
        BME_CHECK(NEXUS_Playpump_GetBuffer(_context->playpump, &playpump_buffer, &playpump_buffer_size));
    }

    std::memcpy(playpump_buffer, chunk, size);
    BME_CHECK(NEXUS_Playpump_WriteComplete(_context->playpump, 0, size));
    BME_DEBUG_EXIT();
    return _pumpBufState;
}

void PushTSSource::reset()
{
    uninit();
}

void PushTSSource::release()
{
    uninit();
}

bool PushTSSource::checkUrlSupport(const std::string& url)
{
    BME_DEBUG_ENTER();

    if (url.find(IMedia::PUSH_TS_URI_PREFIX) != std::string::npos) {
        return true;
    }

    BME_DEBUG_EXIT();
    return false;
}


SourceConnector* PushTSSource::getConnector()
{
    return _connector;
}

uint32_t PushTSSource::setConnector(SourceConnector* connector)
{
    _connector = connector;
    return 0;
}


std::string PushTSSource::getType()
{
    return SOURCE_PUSHTS;
}

IMedia::StreamMetadata PushTSSource::getStreamMetadata()
{
    IMedia::VideoParameters videoParam = {
        _context->videoPid,
        _context->pcrPid,
        (IMedia::VideoCodec)_context->videoCodec,
        IMedia::UnknownVideoCodec,
        (uint16_t)_context->videoWidth,
        (uint16_t)_context->videoHeight,
        0, IMedia::VideoAspectRatioUnknown};
    IMedia::AudioParameters audioParam = {
        _context->audioPid,
        0,
        (IMedia::AudioCodec)_context->audioCodec,
        0, 0, 0};

    IMedia::StreamMetadata metaData;
    metaData.streamType = (IMedia::StreamType)_context->transportType;
    metaData.videoParam = videoParam;
    metaData.audioParamList.push_back(audioParam);

    return metaData;
}

IMedia::ErrorType PushTSSource::seekTo(const uint32_t& milliseconds,
                          IMedia::PlaybackOperation playOperation,
                          IMedia::PlaybackSeekTime  playSeekTime)
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_ERROR_NOT_SUPPORTED;
}

void PushTSSource::setPlaybackRate(const std::string& rate)
{
    BME_DEBUG_ENTER();
    int playbackRate = atoi(rate.c_str());
    BME_DEBUG_TRACE(("+++setSpeed(%d)\n", playbackRate));

    if (playbackRate < 0) {
        BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
    }

    NEXUS_AudioDecoderTrickState audioState;
    NEXUS_SimpleAudioDecoder_GetTrickState(getConnector()->audioDecoder, &audioState);
    audioState.rate  = ((uint32_t)playbackRate == PLAYBACK_RATE_PAUSED) ? 0 : NEXUS_NORMAL_PLAY_SPEED;
    audioState.muted = ((uint32_t)playbackRate == PLAYBACK_RATE_PAUSED);
    BME_CHECK(NEXUS_SimpleAudioDecoder_SetTrickState(getConnector()->audioDecoder, &audioState));
    BME_CHECK(NEXUS_SimpleStcChannel_SetRate(getConnector()->stcChannel,
               ((uint32_t)playbackRate != PLAYBACK_RATE_PAUSED), 0));

    if ((uint32_t)playbackRate != PLAYBACK_RATE_PAUSED) {
        _nexusDataExhausted = false;
        NEXUS_VideoDecoderTrickState videoState;

        if (getConnector()->videoDecoder) {
            NEXUS_SimpleVideoDecoder_GetTrickState(getConnector()->videoDecoder, &videoState);
            videoState.rate = NEXUS_NORMAL_DECODE_RATE;
            videoState.tsmEnabled = NEXUS_TsmMode_eEnabled;
            BME_CHECK(NEXUS_SimpleVideoDecoder_SetTrickState(getConnector()->videoDecoder, &videoState));

            if (_state == IMedia::PausedState)
                _state = IMedia::StartedState;

            _playbackRate = NEXUS_NORMAL_DECODE_RATE;
        }
    } else {
        NEXUS_VideoDecoderTrickState videoState;

        if (getConnector()->videoDecoder) {
            NEXUS_SimpleVideoDecoder_GetTrickState(getConnector()->videoDecoder, &videoState);
            videoState.rate = 0;
            BME_CHECK(NEXUS_SimpleVideoDecoder_SetTrickState(getConnector()->videoDecoder, &videoState));
            _state = IMedia::PausedState;
            _playbackRate = 0;
        }
    }
    BME_DEBUG_EXIT();
}

int PushTSSource::getPlaybackRate()
{
    return _playbackRate;
}

IMedia::PlaybackOperation PushTSSource::getPlaybackOperation()
{
    return IMedia::OperationForward;
}

uint64_t PushTSSource::getDuration()
{
    return 0;
}

void
PushTSSource::updatePlayTimeLocked()
{
    BME_DEBUG_ENTER();
    bool decoderTime;
    _lastPlayTime = getCurrentPts(decoderTime);

    if (decoderTime) {
        _playTimeUpdated = true;
    }

    BME_DEBUG_EXIT();
}

uint32_t PushTSSource::getCurrentPosition()
{
    BME_DEBUG_ENTER();
    bool decoderTime;
    _lastPlayTime = getCurrentPts(decoderTime);

    if (decoderTime) {
        _playTimeUpdated = true;
        return _lastPlayTime;
    }

    BME_DEBUG_EXIT();
    return 0;
}

uint64_t PushTSSource::getBytesDecoded()
{
    return 0;
}

const IMedia::TimeInfo PushTSSource::getTimeInfo()
{
    IMedia::TimeInfo timeinfo;
    return timeinfo;
}

uint32_t PushTSSource::getCurrentPts(bool &decoderTime)
{
    BME_DEBUG_ENTER();
    NEXUS_AudioDecoderStatus audioStatus;
    NEXUS_VideoDecoderStatus videoStatus;
    NEXUS_Error         rc = 0;
    uint32_t            pts = 0;
    uint32_t            decoderpts = 0;
    NEXUS_PtsType       ptsType = NEXUS_PtsType_eInterpolatedFromInvalidPTS;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    decoderTime = false;

    if (!isRunning()) {
        return 0;
    }

    videoDecoder = getConnector()->videoDecoder;
    audioDecoder = getConnector()->audioDecoder;

    if (NULL  == videoDecoder) {
        // no video use audio for pts
        if (NULL != audioDecoder) {
            rc = NEXUS_SimpleAudioDecoder_GetStatus(audioDecoder, &audioStatus);

            if (rc == NEXUS_SUCCESS) {
                decoderpts = audioStatus.pts;
                ptsType = audioStatus.ptsType;
            } else {
                BME_DEBUG_ERROR(("NEXUS_SimpleAudioDecoder_GetStatus failed with %d", rc));
            }
        }
    } else {
        rc = NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &videoStatus);

        if (rc == NEXUS_SUCCESS) {
            decoderpts = videoStatus.pts;
            ptsType = videoStatus.ptsType;
        } else {
            BME_DEBUG_ERROR(("NEXUS_SimpleVideoDecoder_GetStatus failed with %d", rc));
        }
    }

    if (ptsType == NEXUS_PtsType_eInterpolatedFromInvalidPTS) {
        return _lastPlayTime;
    }

    pts = decoderpts;
    decoderTime = true;
    BME_DEBUG_EXIT();
    return pts;
}

void PushTSSource::onPrepared()
{
    _state = IMedia::StoppedState;
    notify(SourceEvents::Prepared);
}

void PushTSSource::onError(const IMedia::ErrorType& errorType)
{
    notify(SourceEvents::Error, errorType);
}

void PushTSSource::onSeekComplete()
{
    BME_DEBUG_ENTER();
    notify(SourceEvents::SeekCompleted);
    BME_DEBUG_EXIT();
}

void PushTSSource::onInfo(const IMedia::InfoType& infoType, int32_t extra)
{
    notify(SourceEvents::Info, infoType, extra);
}

void PushTSSource::onCompletion()
{
    BME_DEBUG_ENTER();
    notify(SourceEvents::Completed);
    BME_DEBUG_EXIT();
}

void PushTSSource::onVideoSizeChanged(uint16_t width, uint16_t height)
{
    notify(SourceEvents::VideoSizeChanged, width, height);
}

void PushTSSource::flush(bool holdPicture)
{
    BME_DEBUG_ENTER();
    TRLS_UNUSED(holdPicture);

    if (!isRunning()) {
        return;
    }

    updatePlayTimeLocked();
    resetPlayState();

    if (_context->playpump) {
        BME_CHECK(NEXUS_Playpump_Flush(_context->playpump));
    }

    if (getConnector()->videoDecoder) {
        NEXUS_SimpleVideoDecoder_Flush(getConnector()->videoDecoder);
    }

    if (getConnector()->audioDecoder) {
        NEXUS_SimpleAudioDecoder_Flush(getConnector()->audioDecoder);
    }

    _endOfStream = false;
    _nexusFirstPtsPassed = false;
    BME_DEBUG_EXIT();
}

Connector PushTSSource::connect(const ConnectSettings& settings)
{
    return NULL;
}

void PushTSSource::disconnect(const Connector& connector)
{
}

}  // namespace Broadcom
}
