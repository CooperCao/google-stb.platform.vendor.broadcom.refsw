/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "bip.h"
#include "NetworkSource.h"
#include <string>
#include "b_os_lib.h"
#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#endif

TRLS_DBG_MODULE(NetworkSource);

#define CLIENT_TRICK_MODES "-48, -24, -12, -6, 1, 6, 12, 24, 48, 1/2, -1/2"
namespace Broadcom
{
namespace Media {

char NetworkSource::_userAgent[] = "Broadcom Media Player";
char NetworkSource::_additionalHeaders[] = "transferMode.dlna.org: Streaming\r\n";

const char NetworkSource::_rateFlag[] = "rate=";
const char NetworkSource::_channelsFlag[] = "channels=";
const char NetworkSource::_certFile[] = "./host.cert";

#define BME_DEFAULT_DURATION 3600000
#define IP_NETWORK_MAX_JITTER 300 /* in msec */

typedef struct tagNetworkSourceContext {
    //BIP variables
    BIP_PlayerHandle bipPlayer;
    BIP_MediaInfoHandle bipMediaInfo;
    BIP_PlayerStreamInfo bipStreamInfo;
    BIP_HttpResponseHandle responseHeaders;
    bool liveChannel;
    bool sslInitial;
    bool dtcpIpInitial;
    bool timeSeekRangeHeaderEnabled;

    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle enhancementVideoPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;

    BKNI_EventHandle prepareEvent;
    B_ThreadHandle  asyncThread;

    B_MutexHandle trickModeMutex;
    B_ThreadHandle fakeTrickModeThread;
    bool fakeTrickModeActive;
    int fakeTrickRate;
    float playbackRate;
} NetworkSourceContext;

bool getTrackOfType(BIP_MediaInfoHandle hMediaInfo,
                    BIP_MediaInfoTrackType trackType,
                    BIP_MediaInfoTrack *pMediaInfoTrackOut)
{
    bool                    trackFound = false;
    BIP_MediaInfoStream     *pMediaInfoStream;
    BIP_MediaInfoTrackGroup *pMediaInfoTrackGroup = NULL;
    bool                    trackGroupPresent = false;
    BIP_MediaInfoTrack      *pMediaInfoTrack;

    pMediaInfoStream = BIP_MediaInfo_GetStream(hMediaInfo);

    if (!pMediaInfoStream) {
        BME_DEBUG_ERROR(("No Media Info found\n"));
        return false;
    }

    if (pMediaInfoStream->numberOfTrackGroups != 0)
    {
        pMediaInfoTrackGroup = pMediaInfoStream->pFirstTrackGroupInfo;
        pMediaInfoTrack = pMediaInfoTrackGroup->pFirstTrackForTrackGroup;
        trackGroupPresent = true;
    } else {
        /* None of the track belongs to any trackGroup, in this case stream out all tracks from mediaInfoStream.*/
        pMediaInfoTrack = pMediaInfoStream->pFirstTrackInfoForStream;
    }

    if (!pMediaInfoTrack) {
        BME_DEBUG_ERROR(("No Media track found\n"));
        return false;
    }
    while (pMediaInfoTrack)
    {
        if (pMediaInfoTrack->trackType == trackType)
        {
            BME_DEBUG_TRACE(("Found trackType=%s with trackId=%d",
                    BIP_ToStr_BIP_MediaInfoTrackType(trackType),
                    pMediaInfoTrack->trackId));
            *pMediaInfoTrackOut = *pMediaInfoTrack;
            trackFound = true;
            break;
        }

        if (true == trackGroupPresent)
        {
            pMediaInfoTrack = pMediaInfoTrack->pNextTrackForTrackGroup;
        } else {
            pMediaInfoTrack = pMediaInfoTrack->pNextTrackForStream;
        }
    }
    return (trackFound);
} /* getTrackOfType */

NetworkSource::NetworkSource()
    : _context(NULL),
    _contentLength(0),
    _duration(0),
    _supportsStalling(true),
    _initialSeekTimeMSec(0),
    _state(IMedia::IdleState),
    _looping(false),
    _resourceAcquired(false),
    _serverTrickMode(false),
    _scid(0),
    _maxHeight(0),
    _maxWidth(0)
{
}

NetworkSource::~NetworkSource()
{
    if (_context)
        release();
}

void NetworkSource::init()
{
    BME_DEBUG_ENTER();
    BIP_Status bipStatus;

    if (!_context) {
        _context = new NetworkSourceContext();
        memset(_context, 0, sizeof(NetworkSourceContext));

        bipStatus = BIP_Init(NULL);
        if (bipStatus != BIP_SUCCESS) {
            BME_DEBUG_ERROR(("Failed to initial BIP"));
        }

#ifdef B_HAS_DTCP_IP
        /* Initialize DtcpIpClientFactory */
        BIP_DtcpIpClientFactoryInitSettings dtcpSettings;
        BIP_DtcpIpClientFactory_GetDefaultInitSettings(&dtcpSettings);

        if (getenv("TRELLIS_DTCP_IP_TESTKEY") &&
                !strcmp(getenv("TRELLIS_DTCP_IP_TESTKEY"), "y")) {
            dtcpSettings.keyFormat = B_DTCP_KeyFormat_eTest;
            BME_DEBUG_TRACE(("######## Using test key #######"));
        } else {
            dtcpSettings.keyFormat = B_DTCP_KeyFormat_eCommonDRM;
            BME_DEBUG_TRACE(("######## Using production key #######"));
        }
        bipStatus = BIP_DtcpIpClientFactory_Init(&dtcpSettings);
        if (bipStatus != BIP_SUCCESS) {
            BME_DEBUG_ERROR(("Failed to initial DTCP_IP client factory"));
        } else {
            _context->dtcpIpInitial = true;
        }
#endif

        /* Initialize SslClientFactory */
        BIP_SslClientFactoryInitSettings sslSettings;

        BIP_SslClientFactory_GetDefaultInitSettings(&sslSettings);
        sslSettings.pRootCaCertPath = _certFile;
        bipStatus = BIP_SslClientFactory_Init(&sslSettings);
        if (bipStatus != BIP_SUCCESS) {
            BME_DEBUG_ERROR(("Failed to initial SSL client factory"));
        } else {
            _context->sslInitial = true;
        }

        /* Create BIP Player instance */
        _context->bipPlayer = BIP_Player_Create(NULL);
        if (!_context->bipPlayer) {
            BME_DEBUG_ERROR(("Failed to create player"));
        }
        _context->liveChannel = false;
        _context->timeSeekRangeHeaderEnabled = false;
        _context->trickModeMutex = B_Mutex_Create(NULL);
        _context->playbackRate = 1;
        _playbackRates.clear();
        BKNI_CreateEvent(&_context->prepareEvent);
    } else {
        BIP_PlayerStatus status;
        BIP_Status rc;

        if (_context->bipPlayer) {
            rc = BIP_Player_GetStatus(_context->bipPlayer, &status);
            if (BIP_SUCCESS == rc) {
                if (status.state != BIP_PlayerState_eDisconnected) {
                    BIP_Player_Disconnect(_context->bipPlayer);
                }
            }
        }
    }
    BME_DEBUG_EXIT();
}

void NetworkSource::uninit()
{
    BME_DEBUG_ENTER();

    if (_context == NULL)
        return;

    releaseResources();
    BIP_Uninit();
    if (_context->trickModeMutex) {
        B_Mutex_Destroy(_context->trickModeMutex);
        _context->trickModeMutex = NULL;
    }

    if (_context->prepareEvent) {
        BKNI_DestroyEvent(_context->prepareEvent);
        _context->prepareEvent = NULL;
    }
    delete _context;
    _context = NULL;
    _state = IMedia::IdleState;
}

void NetworkSource::releaseResources()
{
    BME_DEBUG_ENTER();

    NEXUS_Error rc = NEXUS_SUCCESS;
    int reTry = 5;

    if (_state ==  IMedia::IdleState)
        return;

    if (_context && _context->bipPlayer) {
        if (_state == IMedia::PreparingState) {
            do {
                rc = BKNI_WaitForEvent(_context->prepareEvent, 1000);
                if (rc == NEXUS_SUCCESS) {
                    break;
                }
                BME_DEBUG_TRACE(("Timeout waiting for prepareEvent %d\n", rc));
                reTry--;
            } while ((rc == NEXUS_TIMEOUT) && (reTry > 0));
            _state = IMedia::StoppedState;
        }

        if (_state == IMedia::StartedState || _state == IMedia::PausedState) {
            stop(false);
        }
        if (_state == IMedia::StoppedState) {
            BIP_Player_Disconnect(_context->bipPlayer);
        }
        BIP_Player_Destroy(_context->bipPlayer);
        _context->bipPlayer = NULL;

#ifdef B_HAS_DTCP_IP
        if (_context->dtcpIpInitial) {
            BIP_DtcpIpClientFactory_Uninit();
        }
        _context->dtcpIpInitial = false;
#endif

        if (_context->sslInitial) {
            BIP_SslClientFactory_Uninit();
        }
        _context->sslInitial = false;
    }
    BME_DEBUG_EXIT();
}

void NetworkSource::updateMetadata()
{
    /* Probe media to find out Video and Audio decoder related information.*/
    BIP_MediaInfoTrack mediaInfoTrack;
    BIP_PlayerStreamInfo *pStreamInfo = &(_context->bipStreamInfo);
    BIP_MediaInfoHandle pMediaInfo = _context->bipMediaInfo;
    BIP_Status bipStatus;
    uint16_t colorDepth = 0;

    BME_DEBUG_ENTER();

    if (_context->bipMediaInfo == NULL) {
        BME_DEBUG_ERROR(("Invalid media info handle"));
        return;
    }

    bipStatus = BIP_Player_GetProbedStreamInfo(_context->bipPlayer, &_context->bipStreamInfo);
    if (bipStatus != BIP_SUCCESS) {
        BME_DEBUG_ERROR(("Failed to get stream info!"));
        return;
    }

    setDefaultNetworkSettings();
    if (_duration == 0) {
        _duration = pStreamInfo->durationInMs;
    }

    if (_contentLength == 0)
        _contentLength = pStreamInfo->contentLength;

    _context->liveChannel = pStreamInfo->liveChannel;
    _context->timeSeekRangeHeaderEnabled = pStreamInfo->timeSeekRangeHeaderEnabled;

    _metadata.streamType = (IMedia::StreamType)pStreamInfo->transportType;

    BME_DEBUG_TRACE(("Stream Info ==> TransportType:%d ContainerType:%d",
        pStreamInfo->transportType, pStreamInfo->containerType));

    if (getTrackOfType(pMediaInfo, BIP_MediaInfoTrackType_eVideo, &mediaInfoTrack)) {
        BME_DEBUG_TRACE(("Video Track Id:0x%x Codec:%d (w, h)=(%d, %d)",
            mediaInfoTrack.trackId, mediaInfoTrack.info.video.codec,
            mediaInfoTrack.info.video.width, mediaInfoTrack.info.video.height));

        // psi should not override user settings
        if (_maxWidth == 0) {
            _maxWidth = mediaInfoTrack.info.video.width;
        }

        if (_maxHeight == 0) {
            _maxHeight = mediaInfoTrack.info.video.height;
        }
        colorDepth = mediaInfoTrack.info.video.colorDepth;

        // because we didin't probe, but client still needs to know the width and height
        // hardcoded to give it a default size and aspect ratio
        if (_context->liveChannel) {
            if ((mediaInfoTrack.info.video.width == 0) && support4Kp60()) {
                if (_maxWidth == 0) {
                    _maxWidth = 3840;
                }
                if (_maxHeight == 0) {
                    _maxHeight = 2160;
                }
                colorDepth = 10;
            } else {
                if (_maxWidth == 0) {
                    _maxWidth = 1920;
                }
                if (_maxWidth == 0) {
                    _maxHeight = 1080;
                }
            }
        }

        // for adaptive streaming set max values.
        if (pStreamInfo->containerType == BIP_PlayerContainerType_eMpegDash) {
            _maxWidth = 1920;
            _maxHeight = 1080;
        }

        IMedia::VideoParameters videoParam = {
            (uint16_t)mediaInfoTrack.trackId, 0,
            (IMedia::VideoCodec)mediaInfoTrack.info.video.codec,
            IMedia::UnknownVideoCodec,
            _maxWidth, _maxHeight, colorDepth,
            IMedia::VideoAspectRatioUnknown};
        _metadata.videoParam = videoParam;
    }

    if (getTrackOfType(pMediaInfo, BIP_MediaInfoTrackType_eAudio, &mediaInfoTrack) )
    {
        BME_DEBUG_TRACE(("Audio Track Id:0x%x Codec:%d Samplerate:%d Bitrate:%d",
            mediaInfoTrack.trackId, mediaInfoTrack.info.audio.codec,
            mediaInfoTrack.info.audio.sampleRate, mediaInfoTrack.info.audio.bitrate));

        IMedia::AudioParameters audioParam = {
            (uint16_t)mediaInfoTrack.trackId, 0,
            (IMedia::AudioCodec)mediaInfoTrack.info.audio.codec,
            mediaInfoTrack.info.audio.sampleRate,
            mediaInfoTrack.info.audio.bitrate,
            mediaInfoTrack.info.audio.channelCount};
        _metadata.audioParamList.push_back(audioParam);
    }

    if (pStreamInfo->transportType == NEXUS_TransportType_eTs) {
        BME_DEBUG_TRACE(("TS Stream: transportTimeStampEnabled:%d pcrPID:0x%x pmtPID:0x%x",
            pStreamInfo->mpeg2Ts.transportTimeStampEnabled,
            pStreamInfo->mpeg2Ts.pcrPid,
            pStreamInfo->mpeg2Ts.pmtPid));

        if (_context->bipStreamInfo.mpeg2Ts.pmtPid) {
            IMedia::PmtParameters pmtParam = {
                (uint16_t)_context->bipStreamInfo.mpeg2Ts.pmtPid};
            _metadata.pmtParamList.push_back(pmtParam);
        }
        _metadata.videoParam.substreamId = _context->bipStreamInfo.mpeg2Ts.pcrPid;
        _metadata.timeStampEnabled = pStreamInfo->mpeg2Ts.transportTimeStampEnabled;
    }
    _serverTrickMode = pStreamInfo->serverSideTrickmodesSupported;

    if (!_serverTrickMode) {
        _playbackRates.clear();
        _playbackRates += CLIENT_TRICK_MODES;
    }

    BME_DEBUG_EXIT();
}

void NetworkSource::start()
{
    BME_DEBUG_ENTER();
    if (_state == IMedia::PausedState) {
        if (BIP_Player_Play(_context->bipPlayer) != BIP_SUCCESS) {
            BME_DEBUG_ERROR(("Failed to restart player"));
            BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
        }
        _state = IMedia::StartedState;
        return;
    }

    if (_context->playbackRate != 1.0) {
        setPlaybackRate(std::string("1"));
        return;
    }

    BIP_PlayerStatus status;
    BIP_Player_GetStatus(_context->bipPlayer, &status);

    if (status.state < BIP_PlayerState_ePreparing) {
        BIP_PlayerPrepareSettings prepareSettings;
        BIP_PlayerSettings playerSettings;
        BIP_PlayerPrepareStatus prepareStatus;


        BIP_Player_GetDefaultPrepareSettings(&prepareSettings);
        BIP_Player_GetDefaultSettings(&playerSettings);

        if (_metadata.videoParam.streamId && getConnector()->videoDecoder) {
            playerSettings.videoTrackId = _metadata.videoParam.streamId;
            playerSettings.videoTrackSettings.pidTypeSettings.video.codec = (NEXUS_VideoCodec)(_metadata.videoParam.streamId);
            playerSettings.extraVideoTrackId = _metadata.videoParam.substreamId;
            playerSettings.videoTrackSettings.pidTypeSettings.video.simpleDecoder = getConnector()->videoDecoder;
        }
        if (_metadata.audioParamList.size()) {
            IMedia::AudioParameters audioParam = _metadata.audioParamList[0];
            if (audioParam.streamId && getConnector()->audioDecoder) {
                playerSettings.audioTrackId = audioParam.streamId;
                playerSettings.audioTrackSettings.pidSettings.pidTypeSettings.audio.codec =
                    (NEXUS_AudioCodec)(audioParam.audioCodec);
                playerSettings.audioTrackSettings.pidTypeSettings.audio.simpleDecoder = getConnector()->audioDecoder;

                if (audioParam.audioCodec == IMedia::PcmWavAudioCodec) {
                    playerSettings.lpcm.convertLpcmToWave = true;
                    playerSettings.lpcm.sampleRate = audioParam.samplesPerSecond;
                    playerSettings.lpcm.numChannels = audioParam.numChannels;
                    playerSettings.lpcm.bitsPerSample = audioParam.bitsPerSample;
                    playerSettings.playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eWav;
                    if (playerSettings.lpcm.sampleRate == 0)
                        playerSettings.lpcm.sampleRate = 48000;
                    if (playerSettings.lpcm.bitsPerSample == 0)
                        playerSettings.lpcm.bitsPerSample = 16;
                }
            }
        }
        if (_context->bipStreamInfo.dataAvailabilityModel == BIP_PlayerDataAvailabilityModel_eLimitedRandomAccess) {
            playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
            playerSettings.playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
            prepareSettings.pauseTimeoutAction = NEXUS_PlaybackLoopMode_ePlay;
        } else if (_looping) {
            playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
            playerSettings.playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
        } else {
            playerSettings.playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
            playerSettings.playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
            playerSettings.playbackSettings.endOfStreamCallback.callback = NetworkSource::endOfStreamCallback;
            playerSettings.playbackSettings.endOfStreamCallback.context = static_cast<void*>(this);
        }

        playerSettings.playbackSettings.beginningOfStreamCallback.callback = NetworkSource::beginningOfStreamCallback;
        playerSettings.playbackSettings.beginningOfStreamCallback.context = static_cast<void*>(this);
        playerSettings.playbackSettings.errorCallback.callback = NetworkSource::errorCallback;
        playerSettings.playbackSettings.errorCallback.context = static_cast<void*>(this);
        if (_initialSeekTimeMSec)
            playerSettings.playbackSettings.startPaused = true;
#if 0
        // default to TS for now, but later need to scan for type */
        playerSettings.playbackSettings.playpumpSettings.transportType = (NEXUS_TransportType)_metadata.streamType;
        playerSettings.playbackSettings.playpumpSettings.mode = NEXUS_PlaypumpMode_eFifo;
#endif

        if (getConnector()->stcChannel) {
            playerSettings.playbackSettings.simpleStcChannel = getConnector()->stcChannel;
            playerSettings.playbackSettings.stcTrick = true;

            // now configure the simple stc channel
            if (_context->liveChannel) {
                NEXUS_SimpleStcChannelSettings stcSettings;
                NEXUS_SimpleStcChannel_GetSettings(getConnector()->stcChannel, &stcSettings);
                if (_context->pcrPidChannel) {
                    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
                    stcSettings.modeSettings.pcr.pidChannel = _context->pcrPidChannel;
                }

                if (_metadata.timeStampEnabled) {
                    // when timestamps are present, dejittering is done before feeding to xpt and
                    // thus is treated as a live playback
                    stcSettings.modeSettings.highJitter.mode =
                        NEXUS_SimpleStcChannelHighJitterMode_eTtsPacing;
                } else {
                    // when timestamps are not present, we directly feed the jittered packets
                    // to the transport and set the max network jitter in highJitterThreshold.
                    // This enables the SimpleStc to internally program the various stc &
                    // timebase related thresholds to account for network jitter
                    stcSettings.modeSettings.highJitter.threshold = IP_NETWORK_MAX_JITTER;
                    stcSettings.modeSettings.highJitter.mode =
                        NEXUS_SimpleStcChannelHighJitterMode_eDirect;
                }
                BME_CHECK(NEXUS_SimpleStcChannel_SetSettings(getConnector()->stcChannel, &stcSettings));
            }
        }

/*
        if (_metadata.timeStampEnabled) {
            playerSettings.playbackSettings.playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eMod300;
            playerSettings.playbackSettings.playpumpSettings.timestamp.pacing = false;
        } else {
            playerSettings.playbackSettings.playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eNone;
        }
*/

        if (BIP_SUCCESS != BIP_Player_Prepare(_context->bipPlayer, &prepareSettings,
                                               &playerSettings, NULL, &(_context->bipStreamInfo),
                                               &prepareStatus)) {
            BME_DEBUG_ERROR(("Failed to prepare player"));
            return;
        }

        _context->videoPidChannel = prepareStatus.hVideoPidChannel;
        _context->audioPidChannel = prepareStatus.hAudioPidChannel;
        _context->enhancementVideoPidChannel = prepareStatus.hExtraVideoPidChannel;

        if (playerSettings.videoTrackId && playerSettings.audioTrackId) {
             _context->pcrPidChannel = prepareStatus.hAudioPidChannel;
        }
    }
    getConnector()->videoPidChannel = _context->videoPidChannel;
    getConnector()->audioPidChannel = _context->audioPidChannel;
    getConnector()->pcrPidChannel = _context->pcrPidChannel;

    /* Start BIP_Player. */
    BIP_PlayerStartSettings startSettings;

    BIP_Player_GetDefaultStartSettings(&startSettings);
    startSettings.timePositionInMs = _initialSeekTimeMSec;

    if (BIP_SUCCESS != BIP_Player_Start(_context->bipPlayer, &startSettings)) {
        BME_DEBUG_ERROR(("Failed to start player"));
        return;
    }

    _initialSeekTimeMSec = 0;
    _context->playbackRate = 1;
    _state = IMedia::StartedState;
    BME_DEBUG_EXIT();
}

void NetworkSource::stopInternalThread(void* data)
{
    NetworkSource* source = static_cast<NetworkSource*>(data);
    source->onCompletion();
}

void NetworkSource::beginInternalThread(void* data)
{
    NetworkSource* source = static_cast<NetworkSource*>(data);
    source->onBeginning();
}

void NetworkSource::errorInternalThread(void* data)
{
    NetworkSource* source = static_cast<NetworkSource*>(data);
    source->onError(IMedia::MEDIA_ERROR_SERVER_DIED);
}

void NetworkSource::endOfStreamCallback(void* context, int)
{
    BME_DEBUG_ENTER();
    B_Thread_Create("", stopInternalThread, context, NULL);
    BME_DEBUG_EXIT();
}

void NetworkSource::beginningOfStreamCallback(void* context, int)
{
    BME_DEBUG_ENTER();
    B_Thread_Create("", beginInternalThread, context, NULL);
    BME_DEBUG_EXIT();
}

void NetworkSource::errorCallback(void* context, int)
{
    BME_DEBUG_ENTER();
    B_Thread_Create("", errorInternalThread, context, NULL);
    BME_DEBUG_EXIT();
}

static void asyncApiCompletionCallbackFromBip(void *context, int param)
{
    BME_DEBUG_ENTER();
    NetworkSource* source = static_cast<NetworkSource*>(context);

    switch (param) {
        case BIP_PlayerState_eConnecting:
            source->parseResponseHeader();
            source->probe(true);
            break;
        case BIP_PlayerState_eProbing:
            source->updateMetadata();
            source->onPrepared();
            break;
    }
    BME_DEBUG_EXIT();
} /* asyncApiCompletionCallbackFromBip */

void NetworkSource::stop(bool holdLastFrame)
{
    BME_DEBUG_ENTER();

    if ((_state != IMedia::StartedState) && (_state != IMedia::PausedState)) {
        return;
    }
/*
    if (_state == IMedia::PreparingState) {
        NEXUS_Error rc = BKNI_WaitForEvent(_context->prepareEvent, 1000);
        if (rc == NEXUS_TIMEOUT) {
            BME_DEBUG_TRACE(("Timeout waiting for prepareEvent %d\n", rc));
            return;
        }
    }
*/
    B_Mutex_Lock(_context->trickModeMutex);
    if (_context->fakeTrickModeThread) {
        _context->fakeTrickModeActive = false;
        B_Thread_Destroy(_context->fakeTrickModeThread);
        _context->fakeTrickModeThread = NULL;
    }
    B_Mutex_Unlock(_context->trickModeMutex);

    if (_context->bipPlayer) {
        BIP_Player_Stop(_context->bipPlayer);
    }
    _state = IMedia::StoppedState;
    BME_DEBUG_EXIT();
}

void NetworkSource::pause()
{
    BME_DEBUG_ENTER();

    if (!_context->bipPlayer) {
        BME_DEBUG_ERROR(("Player is not ready"));
        return;
    }

    if (_context->liveChannel) {
        BME_DEBUG_ERROR(("Pause is currently not supported for live channel"));
        return;
    }

    if (_metadata.audioParamList.size()) {
        IMedia::AudioParameters audioParam = _metadata.audioParamList[0];
        if (audioParam.audioCodec == IMedia::PcmAudioCodec) {
            BME_DEBUG_ERROR(("Pause is currently not supported for LPCM"));
            return;
        }
    }

    BIP_PlayerPauseSettings pauseSettings;
    BIP_Player_GetDefaultPauseSettings(&pauseSettings);

    if (_supportsStalling) {
        pauseSettings.pauseMethod = BIP_PlayerPauseMethod_eUseConnectionStalling;
    } else {
        pauseSettings.pauseMethod = BIP_PlayerPauseMethod_eUseDisconnectAndSeek;
    }

    if (BIP_SUCCESS != BIP_Player_Pause(_context->bipPlayer, &pauseSettings)) {
        return;
    }
    _state = IMedia::PausedState;
    BME_DEBUG_EXIT();
}


void timeRangeNoPlaySpeedTrickMode(void* data)
{
    BME_DEBUG_ENTER();
    NetworkSource* source = static_cast<NetworkSource*>(data);
    NetworkSourceContext* context = NULL;
    int32_t pos = 0;
    bool playPause = false;
    if (source) {
        context = source->getContext();
    }
    if (context) {
        pos = source->getCurrentPosition();
        while (context->fakeTrickModeActive) {
            if (context->fakeTrickRate < 0 || context->fakeTrickRate >= 1000) {
                pos = pos + context->fakeTrickRate;
                if (pos > (int32_t)source->getDuration()) {
                    pos = 0;
                } else if (pos < 0) {
                    pos = source->getDuration() - 2000;
                }
                source->seekTo(pos);
                usleep(300000);
             } else {
                if (playPause) {
                    source->start();
                } else {
                    source->pause();
                }
                sleep(2);
                playPause = !playPause;
             }
        }
    }
    BME_DEBUG_EXIT();
}

bool NetworkSource::checkUrlSupport(const std::string& url)
{
    // FIXME
    return false;
}

IMedia::ErrorType NetworkSource::prepare()
{
    return doPrepare(false);
}

void NetworkSource::prepareAsync()
{
    doPrepare(true);
}

IMedia::ErrorType NetworkSource::doPrepare(bool async)
{
    BIP_Status bipStatus;
    BME_DEBUG_ENTER();

    if (!_context || !_context->bipPlayer) {
        return  IMedia::MEDIA_ERROR_PLAYBACK_ABORT;
    }

    if (_state == IMedia::StartedState || _state == IMedia::PausedState) {
        stop(false);
    }

    if (_state == IMedia::StoppedState) {
        BIP_Player_Disconnect(_context->bipPlayer);
    }

    /* Connect to Server & do HTTP Request/Response exchange. */
    BIP_PlayerConnectSettings settings;

    _metadata.reset();

    BIP_Player_GetDefaultConnectSettings(&settings);
    settings.phHttpResponse = (BIP_HttpResponseHandle *)&(_context->responseHeaders);
    settings.pUserAgent = _userAgent;
    settings.pAdditionalHeaders = _additionalHeaders;
    settings.timeoutInMs = 3000;

    if (async) {
        BIP_CallbackDesc asyncCallbackDesc;
        BIP_Status asyncApiCompletionStatus;

        asyncCallbackDesc.context = static_cast<void*>(this);
        asyncCallbackDesc.callback = asyncApiCompletionCallbackFromBip;
        asyncCallbackDesc.param = BIP_PlayerState_eConnecting;

        if (_context->bipPlayer) {
            bipStatus = BIP_Player_ConnectAsync(_context->bipPlayer,
                                      _dataSource.c_str(),
                                      &settings,
                                      &asyncCallbackDesc,
                                      &asyncApiCompletionStatus);

            if (bipStatus != BIP_INF_IN_PROGRESS) {
                BME_DEBUG_ERROR(("Failed to connect to URL=%s", _dataSource.c_str()));
                return IMedia::MEDIA_ERROR_UNSUPPORTED_STREAM;
            }
            BKNI_ResetEvent(_context->prepareEvent);
        }
        _state = IMedia::PreparingState;
    } else {
        bipStatus = BIP_Player_Connect(_context->bipPlayer,
                            _dataSource.c_str(),
                            &settings);

        if (bipStatus != BIP_SUCCESS) {
            BME_DEBUG_ERROR(("Failed to connect to URL=%s", _dataSource.c_str()));
            return IMedia::MEDIA_ERROR_UNSUPPORTED_STREAM;
        }
        parseResponseHeader();

        if (probe(async) != IMedia::MEDIA_SUCCESS) {
            BME_DEBUG_ERROR(("Probe failed\n"));
            return IMedia::MEDIA_ERROR_UNSUPPORTED_STREAM;
        }
        _state = IMedia::StoppedState;
    }
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}

IMedia::ErrorType NetworkSource::probe(bool async)
{
    BME_DEBUG_ENTER();

    /* Probe media to find out Video and Audio decoder related information.*/
    BIP_PlayerProbeMediaInfoSettings probeMediaInfoSettings;
    BIP_Status bipStatus;

    if (!_context || !_context->bipPlayer) {
        return IMedia::MEDIA_ERROR_UNKNOWN;
    }

    if ((_metadata.streamType != IMedia::UnknownStreamType) &&
        (_metadata.streamType != IMedia::AutoStreamType)) {
        return IMedia::MEDIA_SUCCESS;
    }

    BIP_Player_GetDefaultProbeMediaInfoSettings(&probeMediaInfoSettings);
    probeMediaInfoSettings.contentLength = _contentLength;
    probeMediaInfoSettings.transportType = (NEXUS_TransportType)_metadata.streamType;
    probeMediaInfoSettings.timeoutInMs = 30000;
    probeMediaInfoSettings.dontUseIndex = false;
    _context->bipMediaInfo = NULL;

    if (async) {
        BIP_CallbackDesc asyncCallbackDesc;
        BIP_Status asyncApiCompletionStatus;

        asyncCallbackDesc.context = static_cast<void*>(this);
        asyncCallbackDesc.callback = asyncApiCompletionCallbackFromBip;
        asyncCallbackDesc.param = BIP_PlayerState_eProbing;

        bipStatus = BIP_Player_ProbeMediaInfoAsync(_context->bipPlayer,
                            &probeMediaInfoSettings,
                            &_context->bipMediaInfo,
                            &asyncCallbackDesc,
                            &asyncApiCompletionStatus);

        if (bipStatus != BIP_INF_IN_PROGRESS) {
            BME_DEBUG_ERROR(("Async probe Failed!"));
            return IMedia::MEDIA_ERROR_UNKNOWN;
        }
    } else {
        bipStatus = BIP_Player_ProbeMediaInfo(_context->bipPlayer,
                            &probeMediaInfoSettings,
                            &_context->bipMediaInfo);

        if (bipStatus != BIP_SUCCESS || !_context->bipMediaInfo) {
            BME_DEBUG_ERROR(("Failed to probe stream!"));
            return IMedia::MEDIA_ERROR_UNKNOWN;
        }
        updateMetadata();
    }
    BME_DEBUG_EXIT();
    return IMedia::MEDIA_SUCCESS;
}

void NetworkSource::setDataSource(MediaStream *mediaStream)
{
    BME_DEBUG_ENTER();
    _dataSource = mediaStream->getUri();
    if (mediaStream->getDuration()) {
        _duration = mediaStream->getDuration();
    }
    _contentLength = mediaStream->getContentLength();
    _metadata = mediaStream->metadata;

    if (_metadata.videoParam.maxWidth != 0) {
        _maxWidth = _metadata.videoParam.maxWidth;
    }
    if (_metadata.videoParam.maxHeight != 0) {
        _maxHeight = _metadata.videoParam.maxHeight;
    }
    BME_DEBUG_TRACE(("max w/h = %d %d\n", _maxHeight, _maxWidth));
    init();
    BME_DEBUG_EXIT();
}

void NetworkSource::reset()
{
    release();
}

void NetworkSource::release()
{
    releaseResources();
    uninit();
}

std::string NetworkSource::getType()
{
    return SOURCE_NETWORK;
}

void NetworkSource::onPrepared()
{
    _state = IMedia::StoppedState;
    notify(SourceEvents::Prepared);
}

void NetworkSource::onError(const IMedia::ErrorType& errorType)
{
    notify(SourceEvents::Error, errorType);
}

void NetworkSource::onCompletion()
{
    BME_DEBUG_ENTER();
    notify(SourceEvents::Completed);
    BME_DEBUG_EXIT();
}

void NetworkSource::onBeginning()
{
    BME_DEBUG_ENTER();
    notify(SourceEvents::BeginningOfStream);
    BME_DEBUG_EXIT();
}


void NetworkSource::setDefaultNetworkSettings()
{
    _supportsStalling = true;
}

void NetworkSource::parseResponseHeader()
{
    BME_DEBUG_ENTER();

    if (_context->responseHeaders == NULL) {
        BME_DEBUG_TRACE(("No valid response header"));
        return;
    }

    const char* contentFeature = NULL;
    const char* contentType = NULL;
    const char* contentLength = NULL;
    const char* scid = NULL;
    BIP_HttpHeaderHandle hHeader = NULL;
    BIP_Status bipStatus;

    BIP_HttpResponse_Print(_context->responseHeaders, NULL, NULL);

    bipStatus = BIP_HttpResponse_GetNextHeader(_context->responseHeaders,
                        NULL, "content-type", &hHeader, &contentType);

    if (bipStatus == BIP_SUCCESS) {
        BME_DEBUG_TRACE(("Got content type in response headers ==>%s\n", contentType));
    }

    bipStatus = BIP_HttpResponse_GetNextHeader(_context->responseHeaders,
                        NULL, "ContentFeatures.dlna.org", &hHeader, &contentFeature);
    if (bipStatus == BIP_SUCCESS) {
        BME_DEBUG_TRACE(("Got content feature in response headers ==>%s\n", contentFeature));
        parseContentFeatures(contentFeature, contentType);
    }

    bipStatus = BIP_HttpResponse_GetNextHeader(_context->responseHeaders,
                        NULL, "CONTENT-LENGTH", &hHeader, &contentLength);
    if (bipStatus == BIP_SUCCESS) {
        _contentLength = atoi(contentLength);
    }

    bipStatus = BIP_HttpResponse_GetNextHeader(_context->responseHeaders,
                        NULL, "scid.dlna.org", &hHeader, &scid);
    if (bipStatus == BIP_SUCCESS) {
        BME_DEBUG_TRACE(("Got scid in response headers ==>\n", scid));
        parseScidFlag(scid);
    }
    BME_DEBUG_EXIT();
}

void NetworkSource::parseContentFeatures(const char* contentFeature, const char* contentType)
{
    const char* temp;
    uint64_t orgFlag;
    char dlnaFlag[16];

    BME_DEBUG_ENTER();
    if (contentFeature == NULL)
        return;

    // If we find contentFeatures, we should use what we get back
    setDefaultNetworkSettings();

    /* Check if the format is LPCM */
    temp = strcasestr(contentFeature, "DLNA.ORG_PN=");
    if (temp != NULL) {
        temp += strlen("DLNA.ORG_PN=");

        if (contentType && MATCH_PREFIX(temp, "LPCM")) {
            getLpcmInfo(contentType);
        } else {
            _metadata.streamType = IMedia::AutoStreamType;
        }

        /* If we find a DLNA flag that means the server is DLNA aware,
        so we don't send header unless we are told it is okay */
        _supportsStalling = false;
    }

    temp = strcasestr(contentFeature, "DLNA.ORG_FLAGS=");
    if (temp != NULL) {
        temp += strlen("DLNA.ORG_FLAGS=");
        strncpy(dlnaFlag, temp, 8);
        dlnaFlag[8] = '\0';
        orgFlag = strtoul(dlnaFlag, NULL, 16);

        // Check if we can use HTTP Stalling
        if (orgFlag & ORG_FLAG_HTTP_STALLING) {
            _supportsStalling = true;
        } else {
            _supportsStalling = false;
        }
    }

    temp = strcasestr(contentFeature, "DLNA.ORG_PS=");
    if (temp != NULL)
    {
        /* From contentFeature header value look for DLNA.ORG_PS parameter value.*/
        temp += strlen("DLNA.ORG_PS=");

        _playbackRates.clear();
        _playbackRates = std::string(temp);
        _playbackRates = _playbackRates.substr(0, _playbackRates.find(";"));
        _serverTrickMode = true;
    }
    BME_DEBUG_EXIT();
}

void NetworkSource::getLpcmInfo(const char* mime)
{
    const char *rate, *channel;
    uint32_t sampleRate, channels;
    rate = strstr(mime, _rateFlag);

    if (rate != NULL) {
        sampleRate = atoi(rate + strlen(_rateFlag));
    } else {
        sampleRate = 48000;
        BME_DEBUG_TRACE(("Unable to determine sample rate set to 48 KHz"));
    }

    channel = strstr(mime, _channelsFlag);

    if (channel != NULL) {
        channels = atoi(channel + strlen(_channelsFlag));
    } else {
        channels = 2;
        BME_DEBUG_TRACE(("Unable to determine #of channels, set to 2"));
    }

    _metadata.streamType = IMedia::WavStreamType;
    IMedia::AudioParameters audioParam = {
        1, 0, IMedia::PcmWavAudioCodec, sampleRate, 0, (uint8_t)channels};
    _metadata.audioParamList.push_back(audioParam);

    _context->bipStreamInfo.transportType = NEXUS_TransportType_eWav;
     _context->bipStreamInfo.containerType = BIP_PlayerContainerType_eNexusTransportType;
}

SourceConnector* NetworkSource::getConnector()
{
    return _connector;
}

uint32_t NetworkSource::setConnector(SourceConnector* connector)
{
    _connector = connector;
    return 0;
}

IMedia::StreamMetadata NetworkSource::getStreamMetadata()
{
    return _metadata;
}

IMedia::ErrorType NetworkSource::seekTo(const uint32_t& milliseconds,
                              IMedia::PlaybackOperation playOperation,
                              IMedia::PlaybackSeekTime  playSeekTime)
{
    BME_DEBUG_ENTER();
    BIP_Status status = BIP_SUCCESS;

    if (_state == IMedia::PreparingState ||
        _state == IMedia::StoppedState) {
        _initialSeekTimeMSec = milliseconds;
        return IMedia::MEDIA_ERROR_INVALID_STATE;
    }

    if (_context->liveChannel) {
        BME_DEBUG_ERROR(("Seek is not supported during live mode"));
        return IMedia::MEDIA_ERROR_INVALID_STATE;
    }

    if (milliseconds > getDuration()) {
        BME_DEBUG_ERROR(("Seeking beyond duration, stopping stream (duration:%d)", milliseconds > getDuration()));
        B_Thread_Create("", stopInternalThread, static_cast<void*>(this), NULL);
        return IMedia::MEDIA_ERROR_ARG_OUT_OF_RANGE;
    }

    status = BIP_Player_Seek(_context->bipPlayer, milliseconds);
    if (status == BIP_ERR_NOT_SUPPORTED) {
        BME_DEBUG_WARNING(("Seek is not spported."));
    } else if (status != BIP_SUCCESS) {
        BME_DEBUG_ERROR(("Player seek Failed,  (status:0x%x).", status));
        BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
    }
    return IMedia::MEDIA_SUCCESS;
    BME_DEBUG_EXIT();
}

void NetworkSource::setPlaybackRate(const std::string& rate)
{
    BME_DEBUG_ENTER();
    B_Mutex_Lock(_context->trickModeMutex);
    BIP_PlayerPlayAtRateSettings settings;
    bool startFakeTrickMode = false;
    float playbackRate = 0.0;

    if (rate == std::string("0.5")) {
       playbackRate = 0.5;
    } else if (rate == std::string("-0.5")) {
       playbackRate = -0.5;
    } else if (rate == std::string("1/2")) {
       playbackRate = 0.5;
    } else if (rate == std::string("-1/2")) {
       playbackRate = -0.5;
    } else {
        playbackRate = atoi(rate.c_str());
    }

    if (_context->playbackRate == playbackRate) {
        B_Mutex_Unlock(_context->trickModeMutex);
        return;
    }

    //not supporting trickmode on audio files
    if (_metadata.videoParam.streamId == 0) {
        BME_DEBUG_ERROR(("Trickmode not supported for audio only files"));
        B_Mutex_Unlock(_context->trickModeMutex);
        return;
    }

    if (_metadata.streamType == IMedia::WavStreamType) {
        BME_DEBUG_ERROR(("Trickmode not currently supported for Wave files"));
        B_Mutex_Unlock(_context->trickModeMutex);
        return;
    }

    if (1 == playbackRate) {
        if (_context->fakeTrickModeThread) {
            _context->fakeTrickModeActive = false;
            B_Thread_Destroy(_context->fakeTrickModeThread);
            _context->fakeTrickModeThread = NULL;
        }
        if (_state == IMedia::StartedState) {
            BIP_Player_GetDefaultPlayAtRateSettings(&settings);
            if (BIP_SUCCESS != BIP_Player_PlayAtRateAsString(_context->bipPlayer,
                                                        rate.c_str(), &settings)) {
                BME_DEBUG_ERROR(("Failed to set playback rate:%s", rate.c_str()));
            }
        } else {
            if (BIP_SUCCESS != BIP_Player_Play(_context->bipPlayer)) {
                BME_DEBUG_ERROR(("Failed to restart IP Playback"));
            }
        }
        _context->playbackRate = playbackRate;
        B_Mutex_Unlock(_context->trickModeMutex);
        return;
    }

    if (!_serverTrickMode) {
        BME_DEBUG_TRACE(("Using CLIENT side trickmodes"));
        if (playbackRate > -1.0 && playbackRate < 1.0) {
            startFakeTrickMode = true;
        }
    }

    if (startFakeTrickMode) {
        /* CLIENT side cases */
        if (_context->fakeTrickModeThread == NULL) {
            _context->fakeTrickModeActive = true;
            _context->fakeTrickRate = playbackRate * 1000;
            _context->fakeTrickModeThread =
                                B_Thread_Create("", timeRangeNoPlaySpeedTrickMode,
                                                static_cast<void*>(this), NULL);
        } else {
            /* faketrick mode already active, just change the rate */
            _context->fakeTrickRate = playbackRate * 1000;
        }
        _context->playbackRate = playbackRate;
        B_Mutex_Unlock(_context->trickModeMutex);
        return;
    }

    BIP_Player_GetDefaultPlayAtRateSettings(&settings);
    if (BIP_SUCCESS != BIP_Player_PlayAtRateAsString(_context->bipPlayer, rate.c_str(), &settings)) {
        BME_DEBUG_ERROR(("Failed to set playback rate:%s", rate.c_str()));
    }
    _context->playbackRate = playbackRate;
    B_Mutex_Unlock(_context->trickModeMutex);
    BME_DEBUG_EXIT();
}

int NetworkSource::getPlaybackRate()
{
    return 0;
}

IMedia::PlaybackOperation NetworkSource::getPlaybackOperation()
{
    return IMedia::OperationForward;
}

uint64_t NetworkSource::getDuration()
{
    uint64_t duration = 0;

    if (_duration) {
        duration = _duration;
    } else if (_context->bipPlayer) {
        BIP_PlayerStatus status;
        if (BIP_SUCCESS == BIP_Player_GetStatus(_context->bipPlayer, &status)) {
            duration = status.lastPositionInMs;
        }
    }

    // webkit really doesn't like it when we don't have duration
    if (_context->liveChannel || duration == 0)
        duration = BME_DEFAULT_DURATION;

    return duration;
}

uint64_t NetworkSource::getBytesDecoded()
{
    return 0;
}

uint32_t NetworkSource::getCurrentPosition()
{
    uint32_t position = 0;

    BME_DEBUG_ENTER();
    if (_state == IMedia::StoppedState) {
        // stream is technically ended
        position = getDuration();
    } else if (_context->bipPlayer) {
        BIP_PlayerStatus status;
        BIP_Status rc;

        rc = BIP_Player_GetStatus(_context->bipPlayer, &status);
        if (BIP_SUCCESS == rc) {
            position = status.currentPositionInMs;
            // The browser stops playback when it sees the full stream length, so adjust it for
            // autorewind to work
            if ((_context->playbackRate != 1.0) && (position == getDuration())) {
                position = getDuration() - 1;
            }
        }
    }
    BME_DEBUG_EXIT();
    return position;
}

const IMedia::TimeInfo NetworkSource::getTimeInfo()
{
    IMedia::TimeInfo timeinfo;
    return timeinfo;
}

void NetworkSource::setLooping(bool looping)
{
    BME_DEBUG_ENTER();
    _looping = looping;
    BME_DEBUG_EXIT();
}

Connector NetworkSource::connect(const ConnectSettings& settings)
{
    return NULL;
}

void NetworkSource::disconnect(const Connector& connector)
{
}


/* parse scid.dlna.org: to retreive the connection ID for VPOP */
void NetworkSource::parseScidFlag(const char* scid)
{
    BME_DEBUG_ENTER();
    if (scid != NULL) {
        _scid = atoi(scid);
        BME_DEBUG_TRACE(("NetworkSource::parseScidFlag: %d", _scid));
    }
    BME_DEBUG_EXIT();
}

int NetworkSource::getScid()
{
    BME_DEBUG_TRACE(("NetworkSource::getScid: %d", _scid));
    return _scid;
}

std::string NetworkSource::getAvailablePlaybackRate()
{
    BME_DEBUG_ENTER();
    return _playbackRates;
}
}  // namespace Media
}  // namespace Broadcom
