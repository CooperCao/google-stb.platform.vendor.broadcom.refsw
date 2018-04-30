/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string.h>
#include <json/reader.h>
#include <fstream>
#include <mutex>
#include "nexus_types.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_pid_channel.h"
#include "nexus_core_utils.h"
#include "b_os_lib.h"
#include "TimeshiftSource.h"
#include "nexus_playback.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_file_fifo.h"
#include "nexus_file_fifo_chunk.h"
#include "nxclient.h"
#include "nexus_message.h"

namespace Broadcom {
namespace Media {

TRLS_DBG_MODULE(TimeshiftSource);

#define MAX_BIT_RATE        (20 * 1024 * 1024)
#define B_DATA_ALIGN        ((188/4)*4096)

enum class PlayMode {
    Live,
    Playback
};

typedef struct tagTimeshiftSourceContext {
    NEXUS_TransportType transportType;
    unsigned videoPid, pcrPid, audioPid, extraVideoPid;
    NEXUS_ParserBand parserBand;
    NEXUS_VideoCodec videoCodec;
    NEXUS_VideoCodec extraVideoCodec;
    NEXUS_AudioCodec audioCodec;
    NEXUS_AspectRatio aspectRatio;
    struct {
        unsigned x, y;
    } sampleAspectRatio;
    uint16_t videoWidth;
    uint16_t videoHeight;

    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_PidChannelHandle videoExtPidChannel;

    unsigned colorDepth;

    PlayMode playMode;
    NEXUS_FilePlayHandle fifoFile;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    bool playbackOnlyMode;
    MediaStream* mediaStream;
} TimeshiftSourceContext;

TimeshiftSource::TimeshiftSource(MediaRecorder* mediaRecorder, bool playbackOnlyMode)
    : _context(NULL),
    _state(IMedia::IdleState),
    _rate("1"),
    _useInitialSeekPosition(false),
    _initialSeekPosition(0)
{
    if (!mediaRecorder) {
        BME_DEBUG_THROW_EXCEPTION(ReturnInvalidParam);
    }
    _mediaRecorder = mediaRecorder;
    init();
    if (_context) {
        _context->playbackOnlyMode = playbackOnlyMode;
    }
}

TimeshiftSource::~TimeshiftSource()
{
    uninit();
}

void TimeshiftSource::init()
{
    if (!_context) {
        _context = new TimeshiftSourceContext;
        memset(_context, 0, sizeof(*_context));
    }
    _context->playMode = PlayMode::Live;
}

void TimeshiftSource::uninit()
{
    if (_context) {
        delete _context;
        _context = NULL;
    }
    _state = IMedia::IdleState;
}

static void endOfStreamCallback(void *context, int param)
{
    BME_DEBUG_ENTER();
    TimeshiftSource* timeshiftSource = static_cast<TimeshiftSource*>(context);
    timeshiftSource->onCompletion(*timeshiftSource);
    BME_DEBUG_EXIT();
}

static void beginningOfStreamCallback(void *context, int param)
{
    BME_DEBUG_ENTER();
    TimeshiftSource* timeshiftSource = static_cast<TimeshiftSource*>(context);
    timeshiftSource->onBeginning(*timeshiftSource);
    BME_DEBUG_EXIT();
}

SourceConnector* TimeshiftSource::getConnector()
{
    return _connector;
}

uint32_t TimeshiftSource::setConnector(SourceConnector* connector)
{
    _connector = connector;
    return 0;
}

IMedia::ErrorType TimeshiftSource::startPlayback()
{
    BME_DEBUG_ENTER();
    NEXUS_Error rc = NEXUS_SUCCESS;
    int picturesIndexedCounter = 0;

    if (!_context->playback) {
        NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
        NEXUS_PlaybackSettings playbackSettings;
        NEXUS_PlaybackPidChannelSettings playbackPidSettings;

        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        _context->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
        if (!_context->playpump) {
            BME_DEBUG_ERROR(("NEXUS_Playpump_Open failed"));
            stopPlayback();
            return IMedia::MEDIA_ERROR_SOURCE_INVALID_HANDLE;
        }
        _context->playback = NEXUS_Playback_Create();
        if (!_context->playback) {
            BME_DEBUG_ERROR(("NEXUS_Playback_Create failed"));
            stopPlayback();
            return IMedia::MEDIA_ERROR_SOURCE_INVALID_HANDLE;
        }

        NEXUS_Playback_GetSettings(_context->playback, &playbackSettings);
        playbackSettings.playpump = _context->playpump;
        playbackSettings.simpleStcChannel = getConnector()->stcChannel;
        playbackSettings.stcTrick = true;
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay; /* when play hits the end, wait for record */
        playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePlay; /* when play hits the end, wait for record */
        playbackSettings.timeshifting = true;
        playbackSettings.timeshiftingSettings.endOfStreamGap = 0;
        playbackSettings.timeshiftingSettings.beginningOfStreamGap = 0;
        playbackSettings.startPaused = true;
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playbackSettings.endOfStreamCallback.callback = endOfStreamCallback;
        playbackSettings.endOfStreamCallback.context = static_cast<void*>(this);
        playbackSettings.beginningOfStreamCallback.callback = beginningOfStreamCallback;
        playbackSettings.beginningOfStreamCallback.context = static_cast<void*>(this);
        rc = NEXUS_Playback_SetSettings(_context->playback, &playbackSettings);
        if (rc != NEXUS_SUCCESS) {
            BME_DEBUG_ERROR(("NEXUS_Playback_SetSettings failed"));
            stopPlayback();
            return IMedia::MEDIA_ERROR_SOURCE_INVALID_HANDLE;
        }

        _mediaRecorder->addPlayback(_context->playback);
        _context->fifoFile = NEXUS_ChunkedFifoPlay_Open(_mediaRecorder->getFileName().c_str(),
            _mediaRecorder->getIndexName().c_str(),
            static_cast<NEXUS_ChunkedFifoRecordHandle>(_mediaRecorder->getChunkedFifoRecord()));

        if (!_context->fifoFile) {
            BME_DEBUG_ERROR(("can't open files:%s %s", _mediaRecorder->getFileName().c_str(),
                _mediaRecorder->getIndexName().c_str()));
            stopPlayback();
            return IMedia::MEDIA_ERROR_FILE_NOT_FOUND;
        }

        if (_context->audioCodec != NEXUS_AudioCodec_eUnknown && _context->audioPid != 0) {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.simpleDecoder =
                                                    getConnector()->audioDecoder;
            _context->audioPidChannel = NEXUS_Playback_OpenPidChannel(_context->playback,
                _context->audioPid, &playbackPidSettings);
            getConnector()->audioPidChannel = _context->audioPidChannel;
        }

        if (_context->videoCodec != NEXUS_VideoCodec_eNone && _context->videoPid != 0) {
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            playbackPidSettings.pidTypeSettings.video.codec = _context->videoCodec;
            playbackPidSettings.pidTypeSettings.video.index = true;
            playbackPidSettings.pidTypeSettings.video.simpleDecoder = getConnector()->videoDecoder;
            _context->videoPidChannel = NEXUS_Playback_OpenPidChannel(_context->playback,
                _context->videoPid, &playbackPidSettings);
            getConnector()->videoPidChannel = _context->videoPidChannel;
        }

        if (getConnector()->stcChannel) {
            NEXUS_SimpleStcChannelSettings stcSettings;
    //        NEXUS_SimpleStcChannel_GetSettings(getConnector()->stcChannel, &stcSettings);
            NEXUS_SimpleStcChannel_GetDefaultSettings(&stcSettings);
            stcSettings.modeSettings.Auto.transportType = NEXUS_TransportType_eTs;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            BME_CHECK(NEXUS_SimpleStcChannel_SetSettings(getConnector()->stcChannel,
                       &stcSettings));
        }

        //dont start playback until recorded pictures are indexed
        while (!_mediaRecorder->isPicturesIndexed() && picturesIndexedCounter < 100) {
            // Sync this sleep time up with NEXUS_RecordSettings.pollingTimer
            BKNI_Sleep(10);
            picturesIndexedCounter++;
        }

        if (picturesIndexedCounter >= 100) {
            BME_DEBUG_ERROR(("Pictures indexed timed out!"));
            stopPlayback();
            return IMedia::MEDIA_ERROR_RECORDER_NO_DATA_TIMEOUT;
        }
        rc = NEXUS_Playback_Start(_context->playback, _context->fifoFile, NULL);
        if (rc != NEXUS_SUCCESS) {
            BME_DEBUG_ERROR(("NEXUS_Playback_Start failed"));
            stopPlayback();
            return IMedia::MEDIA_ERROR_SOURCE_INVALID_HANDLE;
        }

        if (_useInitialSeekPosition) {
            NEXUS_Playback_Seek(_context->playback, _initialSeekPosition);
        }
        NEXUS_Playback_Play(_context->playback);
    } else {
        rc = NEXUS_Playback_Start(_context->playback, _context->fifoFile, NULL);
        if (rc != NEXUS_SUCCESS) {
            BME_DEBUG_ERROR(("NEXUS_Playback_Start failed"));
            stopPlayback();
            return IMedia::MEDIA_ERROR_SOURCE_INVALID_HANDLE;
        }
    }
    _context->playMode = PlayMode::Playback;
    onInfo(IMedia::MEDIA_INFO_DVR_TSB_PLAYBACK_START, 0);
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}

IMedia::ErrorType TimeshiftSource::startLive()
{
    BME_DEBUG_ENTER();
    NEXUS_Error rc = NEXUS_SUCCESS;

    // Open the audio and video pid channels
    if (_context->videoCodec != NEXUS_VideoCodec_eNone && _context->videoPid != 0) {
        _context->videoPidChannel = NEXUS_PidChannel_Open(_context->parserBand,
                _context->videoPid, NULL);
    }

    if (_context->extraVideoCodec != NEXUS_VideoCodec_eNone && _context->extraVideoPid != 0) {
        _context->videoExtPidChannel = NEXUS_PidChannel_Open(_context->parserBand,
                _context->extraVideoPid, NULL);
    }

    if (_context->audioCodec != NEXUS_AudioCodec_eUnknown && _context->audioPid != 0) {
        _context->audioPidChannel = NEXUS_PidChannel_Open(_context->parserBand,
                _context->audioPid, NULL);
    }

    if (_context->pcrPid != NEXUS_VideoCodec_eNone && _context->pcrPid != 0) {
        _context->pcrPidChannel = NEXUS_PidChannel_Open(_context->parserBand,
                _context->pcrPid, NULL);
    }

    if (getConnector()->stcChannel) {
        NEXUS_SimpleStcChannelSettings stcSettings;
        NEXUS_SimpleStcChannel_GetSettings(getConnector()->stcChannel, &stcSettings);
        stcSettings.mode = NEXUS_StcChannelMode_ePcr;
        stcSettings.modeSettings.pcr.pidChannel = _context->pcrPidChannel;
//        stcSettings.modeSettings.pcr.offsetThreshold = 0xFF;
        rc = NEXUS_SimpleStcChannel_SetSettings(getConnector()->stcChannel, &stcSettings);
        if (rc != NEXUS_SUCCESS) {
            BME_DEBUG_ERROR(("NEXUS_SimpleStcChannel_SetSettings failed"));
            return IMedia::MEDIA_ERROR_SOURCE_INVALID_PARAM;
        }
    }

    getConnector()->videoPidChannel = _context->videoPidChannel;
    getConnector()->audioPidChannel = _context->audioPidChannel;
    getConnector()->pcrPidChannel = _context->pcrPidChannel;
    _context->playMode = PlayMode::Live;

    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}

void TimeshiftSource::start()
{
    if (_state != IMedia::StartedState) {
        if (_context->playbackOnlyMode) {
            startPlayback();
        } else {
            if (_context->playMode == PlayMode::Live) {
                notify(SourceEvents::BeginningOfStream);    // Is this needed?
                startLive();
            } else {
                if (_state == IMedia::PausedState) {
                    setPlaybackRate("1");
                } else {
                    startPlayback();
                }
            }
        }
    }
    _state = IMedia::StartedState;
}

IMedia::ErrorType TimeshiftSource::stopLive(bool holdLastFrame)
{
    BME_DEBUG_ENTER();
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (_context->videoPidChannel) {
        NEXUS_PidChannel_Close(_context->videoPidChannel);
        _context->videoPidChannel = NULL;
    }

    if (_context->audioPidChannel) {
        NEXUS_PidChannel_Close(_context->audioPidChannel);
        _context->audioPidChannel = NULL;
    }

    if (_context->pcrPidChannel) {
        if (getConnector()->stcChannel) {
            NEXUS_SimpleStcChannelSettings stcSettings;
            NEXUS_SimpleStcChannel_GetSettings(getConnector()->stcChannel, &stcSettings);
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            rc = NEXUS_SimpleStcChannel_SetSettings(getConnector()->stcChannel, &stcSettings);
            if (rc != NEXUS_SUCCESS) {
                BME_DEBUG_ERROR(("NEXUS_SimpleStcChannel_SetSettings failed"));
                return IMedia::MEDIA_ERROR_SOURCE_INVALID_PARAM;
            }
        }
        NEXUS_PidChannel_Close(_context->pcrPidChannel);
        _context->pcrPidChannel = NULL;
    }

    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}

void TimeshiftSource::stopPlayback()
{
    if (_context->playback) {
        NEXUS_Playback_Stop(_context->playback);
        _mediaRecorder->removePlayback(_context->playback);
        NEXUS_Playback_ClosePidChannel(_context->playback, _context->videoPidChannel);
        NEXUS_Playback_ClosePidChannel(_context->playback, _context->audioPidChannel);
        NEXUS_Playback_Destroy(_context->playback);
    }
    if (_context->fifoFile) {
        NEXUS_FilePlay_Close(_context->fifoFile);
    }
    if (_context->playpump) {
        NEXUS_Playpump_Close(_context->playpump);
    }

    _context->videoPidChannel = NULL;
    _context->audioPidChannel = NULL;
    _context->fifoFile = NULL;
    _context->playback = NULL;
    _context->playpump = NULL;
}

void TimeshiftSource::stop(bool holdLastFrame)
{
    if (_state == IMedia::StartedState ||
        _state == IMedia::PausedState) {
        _state = IMedia::StoppedState;
        if (_context->playMode == PlayMode::Live) {
            stopLive(holdLastFrame);
        } else {
            stopPlayback();
        }
    }
}

void TimeshiftSource::pausePlayback()
{
    NEXUS_Playback_Pause(_context->playback);
}

void TimeshiftSource::pause()
{
    BME_DEBUG_ENTER();
    setPlaybackRate("0");
    _state = IMedia::PausedState;
    BME_DEBUG_EXIT();
}

void TimeshiftSource::setMediaStream(MediaStream* mediaStream)
{
    // set up stream meta data
    setStreamMetadata(mediaStream->metadata);
    _context->mediaStream = mediaStream;
}

void TimeshiftSource::setDataSource(MediaStream *mediaStream)
{
    BME_DEBUG_ENTER();
    _preferredAudioPid = mediaStream->getPreferredAudioPid();
    setMediaStream(mediaStream);
    BME_DEBUG_EXIT();
}

IMedia::ErrorType TimeshiftSource::prepare()
{
    _state = IMedia::StoppedState;
    return IMedia::MEDIA_SUCCESS;
}


void TimeshiftSource::prepareAsync()
{
    unsigned long t = _mediaRecorder->getCurrentTimestamp();
    NEXUS_Playback_Seek(_context->playback, t);
    NEXUS_Playback_Play(_context->playback);

    pausePlayback();
}

void TimeshiftSource::reset()
{
    uninit();
}

void TimeshiftSource::release()
{
    uninit();
}

bool TimeshiftSource::checkUrlSupport(const std::string& url)
{
    return false;
}

std::string TimeshiftSource::getType()
{
    return "SOURCE_TIMESHIFT_TV";
}

void TimeshiftSource::setStreamMetadata(const IMedia::StreamMetadata& metaData)
{
    IMedia::VideoParameters videoParam = metaData.videoParam;
    IMedia::AudioParameters audioParam;
    int audioParamIndex = 0;

    _context->parserBand = (NEXUS_ParserBand)metaData.parserBand;
    _context->videoPid = videoParam.streamId;
    _context->videoCodec = (NEXUS_VideoCodec)videoParam.videoCodec;
    _context->extraVideoPid = 0;
    _context->extraVideoCodec = NEXUS_VideoCodec_eUnknown;
    _context->pcrPid = videoParam.substreamId;
    _context->videoHeight = videoParam.maxHeight;
    _context->videoWidth = videoParam.maxWidth;

    if (videoParam.colorDepth) {
        _context->colorDepth = videoParam.colorDepth;
    } else {
        _context->colorDepth = 8;
    }
    BME_DEBUG_PRINT(("color depth is: %d\n", _context->colorDepth));
    BME_DEBUG_PRINT(("width: %d, height: %d\n", _context->videoWidth, _context->videoHeight));

    if (metaData.audioParamList.size() > 0) {
        if (_preferredAudioPid) {
            audioParamIndex = metaData.getAudioParamIndex(_preferredAudioPid);
            if (audioParamIndex == -1) {
                audioParamIndex = 0; // pick first if preferred pid not available
                BME_DEBUG_ERROR(("Audio pid: %d, not found\n", _preferredAudioPid));
            }
        }
        audioParam = metaData.audioParamList[audioParamIndex];
        _context->audioPid = audioParam.streamId;
        BME_DEBUG_TRACE(("selected audiPid is %d\n", _context->audioPid));
        _context->audioCodec = (NEXUS_AudioCodec)audioParam.audioCodec;
    } else {
        _context->audioPid = 0;
        _context->audioCodec = NEXUS_AudioCodec_eUnknown;
    }
}

IMedia::StreamMetadata TimeshiftSource::getStreamMetadata()
{
    IMedia::StreamMetadata metaData;

    IMedia::VideoParameters videoParam = {
        (uint16_t)_context->videoPid,
        (uint16_t)_context->pcrPid,
        (IMedia::VideoCodec)_context->videoCodec,
        IMedia::UnknownVideoCodec,
        (uint16_t)_context->videoWidth,
        (uint16_t)_context->videoHeight,
        (uint16_t)_context->colorDepth,
        (IMedia::VideoAspectRatio)_context->aspectRatio
    };

    IMedia::AudioParameters audioParam = {
        (uint16_t)_context->audioPid, 0,
        (IMedia::AudioCodec)_context->audioCodec,
        0, 0, 0
    };

    metaData.streamType = (IMedia::StreamType)_context->transportType;
    metaData.encryptionType = IMedia::NoEncryptionType;
    metaData.videoParam = videoParam;
    metaData.audioParamList.push_back(audioParam);
    metaData.parserBand = (uint32_t)_context->parserBand;
    metaData.timeStampEnabled = true;
    return metaData;
}

IMedia::ErrorType TimeshiftSource::seekTo(const uint32_t& milliseconds,
                        IMedia::PlaybackOperation playOperation,
                        IMedia::PlaybackSeekTime  playSeekTime)

{
    uint32_t seekPosition = milliseconds;

    if (_state == IMedia::IdleState || _state == IMedia::StoppedState) {
        _initialSeekPosition = seekPosition;
        _useInitialSeekPosition = true;
        return IMedia::MEDIA_ERROR_INVALID_STATE;
    } else {
        _useInitialSeekPosition = false;
    }

    if ((playOperation == Broadcom::Media::IMedia::PlaybackOperation::OperationEnd) &&
        (playSeekTime == Broadcom::Media::IMedia::PlaybackSeekTime::SeekTimeEnd)) {
        // Switch to live by seeking to the end
        seekPosition = _mediaRecorder->getCurrentTimestamp();
    } else {
        Broadcom::Media::IMedia::TimeInfo timeInfo = getTimeInfo();
        if ((milliseconds < timeInfo.startTime) || (milliseconds > timeInfo.endTime)) {
            BME_DEBUG_ERROR(("Invalid Time: %ld \n", milliseconds));
            return IMedia::MEDIA_ERROR_ARG_OUT_OF_RANGE;
        }
    }

    if (_context->playMode == PlayMode::Live) {
        // Switch to playback mode, seek and rewind from current frame
        stopLive(true);
        startPlayback();
    }

    if (_context->playback) {
        NEXUS_Playback_Seek(_context->playback, seekPosition);
    }
    return IMedia::MEDIA_SUCCESS;
}

void TimeshiftSource::setPlaybackRate(const std::string& rate)
{
    BME_DEBUG_ENTER();
    // No trickmode without video
    if (!_context->videoPid)
        return;

    int playbackRate = atoi(rate.c_str());

    if (_context->playMode == PlayMode::Live) {
        if (playbackRate == 1) {
            return;
        }
        // We don't support FF if current mode is live.
        if (playbackRate > 1) {
            BME_DEBUG_ERROR(("Fast forward is not supported in live mode"));
            return;
        } else {
            // Switch to playback mode, seek and rewind from current frame
            seekTo(_mediaRecorder->getCurrentTimestamp());
        }
    }
    if (_context->playback) {
        NEXUS_PlaybackTrickModeSettings settings;
        NEXUS_Playback_GetDefaultTrickModeSettings(&settings);
        settings.rate = playbackRate * NEXUS_NORMAL_PLAY_SPEED;
        NEXUS_Playback_TrickMode(_context->playback, &settings);
    }
    _rate = rate;
    BME_DEBUG_EXIT();
}

int TimeshiftSource::getPlaybackRate()
{
    return atoi(_rate.c_str());
}

std::string TimeshiftSource::getAvailablePlaybackRate()
{
    return "";
}

IMedia::PlaybackOperation TimeshiftSource::getPlaybackOperation()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
    if (atoi(_rate.c_str()) > 0) {
        return IMedia::OperationForward;
    } else {
        return IMedia::OperationRewind;
    }
}

uint64_t TimeshiftSource::getDuration()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
    return _mediaRecorder->getCurrentTimestamp();
}

uint32_t TimeshiftSource::getCurrentPosition()
{
    BME_DEBUG_ENTER();
    NEXUS_PlaybackStatus status;

    status.position = 0;
    if (_context->playMode == PlayMode::Playback) {
        if (_context->playback) {
            BME_CHECK(NEXUS_Playback_GetStatus(_context->playback, &status));
        }
    }
    BME_DEBUG_EXIT();
    return status.position;
}

Connector TimeshiftSource::connect(const ConnectSettings& settings)
{
    NEXUS_PidChannelHandle pidChannel;
    pidChannel = NEXUS_Playback_OpenPidChannel(_context->playback,
                                            settings.streamId, NULL);
    return reinterpret_cast<Connector>(pidChannel);
}

void TimeshiftSource::disconnect(const Connector& connector)
{
    NEXUS_PidChannelHandle pidChannel;
    pidChannel = reinterpret_cast<NEXUS_PidChannelHandle>(connector);
    NEXUS_Playback_ClosePidChannel(_context->playback, pidChannel);
}

uint64_t TimeshiftSource::getBytesDecoded()
{
    BME_DEBUG_ENTER();
    NEXUS_PlaybackStatus status;

    status.position = 0;
    if (_context->playMode == PlayMode::Playback) {
        if (_context->playback) {
            BME_CHECK(NEXUS_Playback_GetStatus(_context->playback, &status));
        }
    }
    BME_DEBUG_EXIT();
    return status.bytesPlayed;
}

const IMedia::TimeInfo TimeshiftSource::getTimeInfo()
{
    NEXUS_Error rc;
    IMedia::TimeInfo timeInfo;
    uint32_t tsbLength = 0; // in msec.
    NEXUS_ChunkedFifoRecordSettings chunkFifoRecSettings;
    NEXUS_ChunkedFifoRecordHandle recordHandle =
        static_cast<NEXUS_ChunkedFifoRecordHandle>(_mediaRecorder->getChunkedFifoRecord());

    if (!recordHandle) {
        BME_DEBUG_ERROR(("Record handle is NULL"));
        return timeInfo;
    }

    NEXUS_ChunkedFifoRecord_GetSettings(recordHandle, &chunkFifoRecSettings);
    tsbLength = chunkFifoRecSettings.interval * 1000;
    timeInfo.endTime = getDuration();
    timeInfo.currentPosition = getCurrentPosition();
    if (timeInfo.endTime > tsbLength) {
        timeInfo.startTime = timeInfo.endTime - tsbLength;
    } else {
        timeInfo.startTime = 0;
    }

    return timeInfo;
}

void TimeshiftSource::setLooping(bool looping)
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
}

void TimeshiftSource::onCompletion(TimeshiftSource&)
{
    BME_DEBUG_PRINT(("End of playback reached"));
    if (_context && _context->playbackOnlyMode == false) {
        notify(SourceEvents::Completed);
        stopPlayback();
        startLive();
    }
}

void TimeshiftSource::onBeginning(TimeshiftSource&)
{
    BME_DEBUG_PRINT(("Beginning of playback reached. Resuming normal playback."));
    notify(SourceEvents::BeginningOfStream);
}

void TimeshiftSource::onInfo(const IMedia::InfoType& infoType, int32_t extra)
{
    notify(SourceEvents::Info, infoType, extra);
}
}  // namespace Media
}  // namespace Broadcom
