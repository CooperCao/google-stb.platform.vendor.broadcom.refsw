/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "LiveSource.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "nexus_types.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_pid_channel.h"
#include "nexus_core_utils.h"
#include "b_os_lib.h"

namespace Broadcom
{
namespace Media {

TRLS_DBG_MODULE(LiveSource);

typedef struct tagLiveSourceContext {
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
} LiveSourceContext;

LiveSource::LiveSource()
    : _context(NULL),
    _state(IMedia::IdleState)
{
}

LiveSource::~LiveSource()
{
}

void LiveSource::init()
{
    if (!_context) {
        _context = new LiveSourceContext();
        memset(_context, 0, sizeof(*_context));
    }
}

void LiveSource::uninit()
{
    if (_context) {
        delete _context;
        _context = NULL;
    }
}

SourceConnector* LiveSource::getConnector()
{
    return _connector;
}

uint32_t LiveSource::setConnector(SourceConnector* connector)
{
    _connector = connector;
    return 0;
}

void LiveSource::start()
{
    BME_DEBUG_ENTER();

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
        stcSettings.modeSettings.pcr.offsetThreshold = 0xFF;
        NEXUS_SimpleStcChannel_SetSettings(getConnector()->stcChannel, &stcSettings);
    }

    getConnector()->videoPidChannel = _context->videoPidChannel;
    getConnector()->audioPidChannel = _context->audioPidChannel;
    getConnector()->pcrPidChannel = _context->pcrPidChannel;

    _state = IMedia::StartedState;
    BME_DEBUG_EXIT();
}

void LiveSource::stop(bool holdLastFrame)
{
    BME_DEBUG_ENTER();

    if (_state == IMedia::StartedState) {
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
                NEXUS_SimpleStcChannel_SetSettings(getConnector()->stcChannel, &stcSettings);
            }
            NEXUS_PidChannel_Close(_context->pcrPidChannel);
            _context->pcrPidChannel = NULL;
        }
        _state = IMedia::StoppedState;
    }

    BME_DEBUG_EXIT();
}

void LiveSource::pause()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
}

void LiveSource::setMediaStream(MediaStream* mediaStream)
{
    init();

    // set up stream meta data
    setStreamMetadata(mediaStream->metadata);
}

void LiveSource::setDataSource(MediaStream *mediaStream)
{
    BME_DEBUG_ENTER();
    setMediaStream(mediaStream);
    BME_DEBUG_EXIT();
}

IMedia::ErrorType LiveSource::prepare()
{
    _state = IMedia::StoppedState;
    return IMedia::MEDIA_SUCCESS;
}


void LiveSource::prepareAsync()
{
}

void LiveSource::reset()
{
    uninit();
}

void LiveSource::release()
{
    uninit();
}

bool LiveSource::checkUrlSupport(const std::string& url)
{
    return false;
}

std::string LiveSource::getType()
{
    return "SOURCE_TV";
}

void LiveSource::setStreamMetadata(const IMedia::StreamMetadata& metaData)
{
    _streamMetadata = metaData;
    IMedia::VideoParameters videoParam = metaData.videoParam;
    IMedia::AudioParameters audioParam;

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
    BME_DEBUG_TRACE(("color depth is: %d\n", _context->colorDepth));
    BME_DEBUG_TRACE(("width: %d, height: %d\n", _context->videoWidth,
                _context->videoHeight));

    if (metaData.audioParamList.size() > 0) {
        audioParam = metaData.audioParamList[0];
        _context->audioPid = audioParam.streamId;
        _context->audioCodec = (NEXUS_AudioCodec)audioParam.audioCodec;
    } else {
        _context->audioPid = 0;
        _context->audioCodec = NEXUS_AudioCodec_eUnknown;
    }
}

IMedia::StreamMetadata LiveSource::getStreamMetadata()
{
    if ((_context->videoWidth == 0) && support4Kp60()) {
         _context->videoWidth = 3840;
         _context->videoHeight = 2160;
         _context->colorDepth = 10;
    } else {
         _context->videoWidth = 1920;
         _context->videoHeight = 1080;
    }

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

    _streamMetadata.streamType = (IMedia::StreamType)_context->transportType;
    _streamMetadata.encryptionType = IMedia::NoEncryptionType;
    _streamMetadata.videoParam = videoParam;
    _streamMetadata.parserBand = (uint32_t)_context->parserBand;
    _streamMetadata.timeStampEnabled = true;
    return _streamMetadata;
}

IMedia::ErrorType LiveSource::seekTo(const uint32_t& milliseconds,
                        IMedia::PlaybackOperation playOperation,
                        IMedia::PlaybackSeekTime  playSeekTime)

{
    return IMedia::MEDIA_ERROR_NOT_SUPPORTED;
}

void LiveSource::setPlaybackRate(const std::string& rate)
{
}

int LiveSource::getPlaybackRate()
{
    return 0;
}

std::string LiveSource::getAvailablePlaybackRate()
{
    return "";
}

IMedia::PlaybackOperation LiveSource::getPlaybackOperation()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
    return IMedia::OperationForward;
}

uint64_t LiveSource::getDuration()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
    return 0;
}

uint32_t LiveSource::getCurrentPosition()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
    return 0;
}

uint64_t LiveSource::getBytesDecoded()
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
    return 0;
}

const IMedia::TimeInfo LiveSource::getTimeInfo()
{
    IMedia::TimeInfo timeinfo;
    return timeinfo;
}

Connector LiveSource::connect(const ConnectSettings& settings)
{
    NEXUS_PidChannelHandle pidChannel;
    pidChannel = NEXUS_PidChannel_Open(_context->parserBand, settings.streamId, NULL);
    return reinterpret_cast<Connector>(pidChannel);
}

void LiveSource::disconnect(const Connector& connector)
{
    NEXUS_PidChannelHandle pidChannel;
    pidChannel = reinterpret_cast<NEXUS_PidChannelHandle>(connector);
    NEXUS_PidChannel_Close(pidChannel);
}

void LiveSource::setLooping(bool looping)
{
    BME_DEBUG_ENTER();
    BME_DEBUG_EXIT();
}

}  // namespace Media
}  // namespace Broadcom
