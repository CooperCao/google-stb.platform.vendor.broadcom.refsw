/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "MediaPlayer.h"
#include <string.h>
#include <assert.h>
#include <string>
#include <map>
#include "nexus_platform.h"
#include "nexus_surface_client.h"
#include "nexus_core_utils.h"
#include "nxclient.h"
#include "nexus_graphics2d.h"
#include "nexus_stc_channel.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_video_decoder_primer.h"

#include "AudioVolumeChange.h"
#include "SimpleDecoderNxclient.h"
#include "FileSource.h"
#include "PushTSSource.h"
#include "PushESSource.h"
#ifdef NEXUS_HAS_HDMI_INPUT
#include "HdmiSource.h"
#endif
#ifdef BME_ENABLE_TV
#include "TimeshiftSource.h"
#include "LiveSource.h"
#endif

typedef uint UINT32_C;

namespace Broadcom {
namespace Media {

TRLS_DBG_MODULE(MediaPlayer);

typedef AudioOutput<NEXUS_SimpleAudioDecoderHandle, NEXUS_SimpleAudioDecoderSettings> AudioOutputDecoder;

typedef struct tagMediaPlayerContext {
    NEXUS_SimpleVideoDecoderHandle videoDecoderHandle;
    NEXUS_SimpleAudioDecoderHandle audioDecoderHandle;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;

    NEXUS_SurfaceHandle captureSurface;
    NEXUS_Graphics2DHandle gfx;
} MediaPlayerContext;

MediaPlayer::MediaPlayer()
    :_usePrimer(false),
     _connector(NULL),
     _state(IdleState),
     _looping(false),
     _controlsAudio(false),
     _resourceAcquired(false),
     _videoPositionPreset(false),
     _capturing(false),
     _keepCapturing(false),
     _initialSeekTimeMSec(0),
     _x(0),
     _y(0),
     _width(0),
     _height(0),
     _virtualWidth(1280),
     _virtualHeight(720),
     _visible(true),
     _simpleDecoder(NULL),
     _context(NULL),
     _source(NULL),
     _mediaStream(NULL),
     _pictureHeld(false),
     _sourceOverrided(false),
     _subtitle(NULL),
     _subtitleVisible(false),
     _subtitleEnabled(false),
     _rate("1"),
     _isHdrVideo(false),
     _disabledProgressiveOverride(false)
{
    init();
    debugInitialize();
    _decoderReleased = false;
}

void MediaPlayer::init()
{
    TRLS_MEDIA_DEBUG_ENTER();
    if (NULL == _context) {
        _context = new MediaPlayerContext();
        memset(_context, 0, sizeof(MediaPlayerContext));
    }

    _options.decoderType = DecoderType::Main;
    _options.primingMode = false;

    // Need to check if _simpleDecoder is NULL since
    // init() can be called multiple times without
    // disposing _simpleDecoder to avoid mem leak
    if (_simpleDecoder == NULL) {
#ifdef SINGLE_PROCESS
        _simpleDecoder = new SimpleDecoderSingle();
#else
        _simpleDecoder = new SimpleDecoderNxclient();
#endif
    }

    _simpleDecoder->init();
    _connector = NULL;

    // Reset _videoParam / _audioParam
    memset(&_videoParam, 0, sizeof(VideoParameters));
    memset(&_audioParam, 0, sizeof(AudioParameters));
    _options.primingMode = false;
    _options.decoderType = DecoderType::Main;
    _mediaStreamId = "";

    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::uninit()
{
}

MediaPlayer::~MediaPlayer()
{
    // This will call stop if still playing
    release();

    if (_simpleDecoder) {
        delete _simpleDecoder;
        _simpleDecoder = NULL;
    }

    if (_context) {
        delete _context;
        _context = NULL;
    }

    if (_animationThread.joinable()) {
        _exit = 1;
        _cv.notify_all();
        _animationThread.join();
    }
}

void MediaPlayer::setOptions(const MediaPlayerOptions& options)
{
    _options.decoderType = options.decoderType;
    _options.primingMode = options.primingMode;
}

void MediaPlayer::startInternal()
{
    TRLS_MEDIA_DEBUG_ENTER();

#ifdef BME_ENABLE_TV
    if (_source->getType() == SOURCE_TV) {
        ((LiveSource *)_source)->setMediaStream(_mediaStream);
    } else if (_source->getType() == SOURCE_TIMESHIFT_TV) {
        ((TimeshiftSource *)_source)->setMediaStream(_mediaStream);
    }
#endif

    // Source needs to tell us what type of stream to play
    _streamMetadata = _source->getStreamMetadata();
    // Reset _videoParam / _audioParam
    memset(&_videoParam, 0, sizeof(VideoParameters));
    memset(&_audioParam, 0, sizeof(AudioParameters));

    _videoParam = _streamMetadata.videoParam;
    if (_streamMetadata.audioParamList.size() > 0) {
        _audioParam = _streamMetadata.audioParamList[0];
    }

    // Disable audio when in mosaics mode
    if (isPipOrMosaic()) {
        memset(&_audioParam, 0, sizeof(AudioParameters));
    }

    // Get the necessary resources
    acquireResources();

    // Create the connector between us and the source
    if (NULL == _connector) {
        _connector = new SourceConnector();
    }

    _connector->videoDecoder = _context->videoDecoderHandle;
    _connector->audioDecoder = _context->audioDecoderHandle;
    _connector->stcChannel = _context->stcChannel;
    _source->setConnector(_connector);

    BME_DEBUG_TRACE(("audio codec: %d\n", _audioParam.audioCodec));
    BME_DEBUG_TRACE(("video codec: %d\n", _videoParam.videoCodec));
    _source->start();

    // Start the Subtitle
    startSubtitle();

    if (_videoParam.streamId) {
        NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&_context->videoProgram);
        _context->videoProgram.settings.codec = BaseSource::convertVideoCodec(_videoParam.videoCodec);
        _context->videoProgram.settings.pidChannel = _connector->videoPidChannel;
        _context->videoProgram.smoothResolutionChange = true;
        BME_DEBUG_TRACE(("maxWidth = %d, maxHeight = %d, codec= %d, colorDepth = %d",
                          _videoParam.maxWidth, _videoParam.maxHeight,
                          _videoParam.videoCodec, _videoParam.colorDepth));

        if ((_videoParam.maxWidth != 0) && (_videoParam.maxHeight != 0)) {
            if (BaseSource::convertVideoCodec(_videoParam.videoCodec) != NEXUS_VideoCodec_eVp8) {
                _context->videoProgram.maxWidth = _videoParam.maxWidth;
                _context->videoProgram.maxHeight = _videoParam.maxHeight;
            }
        }

        if (_disabledProgressiveOverride)
            _context->videoProgram.settings.progressiveOverrideMode = NEXUS_VideoDecoderProgressiveOverrideMode_eDisable;

        if (_isHdrVideo) {
            MasteringMetadata *pSrcMm = &(_hdrMetadata._mastering_metadata);
            NEXUS_MasteringDisplayColorVolume *pDstMm =
                &(_context->videoProgram.settings.masteringDisplayColorVolume);

            if (_hdrMetadata._transfer_id == Broadcom::Media::kTransferIdSmpteSt2084)
                _context->videoProgram.settings.eotf = NEXUS_VideoEotf_eHdr10;
            else if (_hdrMetadata._transfer_id == Broadcom::Media::kTransferIdAribStdB67)
                _context->videoProgram.settings.eotf = NEXUS_VideoEotf_eHlg;

            _context->videoProgram.settings.contentLightLevel.max = _hdrMetadata._max_cll;
            _context->videoProgram.settings.contentLightLevel.maxFrameAverage = _hdrMetadata._max_fall;

            pDstMm->redPrimary.x = (pSrcMm->_red_chromaticity_x * 50000);
            pDstMm->redPrimary.y = (pSrcMm->_red_chromaticity_y * 50000);
            pDstMm->greenPrimary.x = (pSrcMm->_green_chromaticity_x * 50000);
            pDstMm->greenPrimary.y = (pSrcMm->_green_chromaticity_y * 50000);
            pDstMm->bluePrimary.x = (pSrcMm->_blue_chromaticity_x * 50000);
            pDstMm->bluePrimary.y = (pSrcMm->_blue_chromaticity_y * 50000);
            pDstMm->whitePoint.x = (pSrcMm->_white_chromaticity_x * 50000);
            pDstMm->whitePoint.y = (pSrcMm->_white_chromaticity_y * 50000);
            pDstMm->luminance.max = (pSrcMm->_luminance_max * 50000);
            pDstMm->luminance.min = (pSrcMm->_luminance_min * 50000);
        }

        BME_DEBUG_TRACE(("max (%d,%d), frameRate %d\n",
                          _context->videoProgram.maxWidth,
                          _context->videoProgram.maxHeight,
                          _context->videoProgram.settings.frameRate));
    }

    if (_audioParam.streamId) {
        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&_context->audioProgram);
        _context->audioProgram.primary.codec = BaseSource::convertAudioCodec(_audioParam.audioCodec);
        _context->audioProgram.primary.pidChannel = _connector->audioPidChannel;
        _context->audioProgram.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eStandalone;
        _context->audioProgram.master = true;
    }

    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::setDecoderStcChannel()
{
    if (isPipOrMosaic())
        return;

    if (_videoParam.streamId && _context->videoDecoderHandle) {
        BME_CHECK(NEXUS_SimpleVideoDecoder_SetStcChannel(_context->videoDecoderHandle,
                   _context->stcChannel));
    }

    if (_audioParam.streamId && _context->audioDecoderHandle) {
        BME_CHECK(NEXUS_SimpleAudioDecoder_SetStcChannel(_context->audioDecoderHandle,
                   _context->stcChannel));
    }
}

static void enableAtmos(NEXUS_SimpleAudioDecoderHandle handle)
{
    if (!AudioVolumeBase::isMs12())
        return;

    NEXUS_AudioDecoderCodecSettings codecSettings;
    NEXUS_SimpleAudioDecoder_GetCodecSettings(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary, NEXUS_AudioCodec_eAc3Plus, &codecSettings);
    codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = true;
    NEXUS_SimpleAudioDecoder_SetCodecSettings(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary, &codecSettings);
}

void MediaPlayer::start()
{
    TRLS_MEDIA_DEBUG_ENTER();

    if (_state == IMedia::PreparingState) {
        BME_DEBUG_ERROR(("Invalid State (%s)", printState().c_str()));
        BME_DEBUG_THROW_EXCEPTION(ReturnInvalidState);
    }

    if ((getState() == IMedia::PausedState) || (_rate.compare("1") != 0)) {
        _source->start();
        _rate = std::string("1");
        _state = IMedia::StartedState;
        return;
    }

    // FIXME(DLiu): Letting this through for now. TV app always wants to call
    // stop in the beginning
    if (_state == IMedia::IdleState || _state == IMedia::StartedState) {
        BME_DEBUG_ERROR(("Calling while in %s state", printState().c_str()));
        return;
    }

    if (_options.primingMode) {
        _simpleDecoder->connect();
        _simpleDecoder->setVisibility(true);
        BME_CHECK(NEXUS_SimpleVideoDecoder_StopPrimerAndStartDecode(_context->videoDecoderHandle));

        if (_audioParam.streamId && _context->audioDecoderHandle) {
            BME_CHECK(NEXUS_SimpleAudioDecoder_SetStcChannel(_context->audioDecoderHandle,
                       _context->stcChannel));
            BME_CHECK(NEXUS_SimpleAudioDecoder_Start(_context->audioDecoderHandle,
                       &_context->audioProgram));
        }
    } else {
        if (_pictureHeld) {
            _simpleDecoder->disconnect();
            _pictureHeld = false;
        }

        if (_decoderReleased) {
            SimpleDecoder::VideoWindow videoWindow = SimpleDecoder::VideoWindowMain;
            bool requestAudio = ((_audioParam.audioCodec != IMedia::UnknownAudioCodec)
                                 && (!isPipOrMosaic()));
            bool requestVideo = _videoParam.streamId;
            SimpleDecoder::acquireParameters acquireParam;
            acquireParam.mediaPlayer = this;
            acquireParam.requestVideoDecoder = requestVideo;
            acquireParam.requestAudioDecoder = requestAudio;
            acquireParam.videoWindow = videoWindow;
            acquireParam.maxWidth = _videoParam.maxWidth;
            acquireParam.maxHeight = _videoParam.maxHeight;
            acquireParam.virtualWidth = _virtualWidth;
            acquireParam.virtualHeight = _virtualHeight;
            acquireParam.decoderType = (uint16_t)_options.decoderType;
            acquireParam.surfaceClientId = _surfaceClientId;
            acquireParam.windowId = _windowId;
            acquireParam.colorDepth = _videoParam.colorDepth;
            acquireParam.persistent = true;
            _simpleDecoder->acquireSimpleDecoders(acquireParam,
                                                  &_context->videoDecoderHandle,
                                                  &_context->audioDecoderHandle);
            _audioVolumeBase = new AudioOutputDecoder(_context->audioDecoderHandle, false, "TMP");
            enableAtmos(_context->audioDecoderHandle);

            _source->seekTo(_previousPositionMSec);
            _decoderReleased = false;
        }

        startInternal();
        // Need to restore callbacks here because the connection is not made to real video decoder
        // till after connect
        NEXUS_VideoDecoderSettings settings;
        NEXUS_VideoDecoderSettings settingsAfter;
        NEXUS_SimpleAudioDecoderSettings audioSettings;
        NEXUS_SimpleAudioDecoderSettings audioSettingsAfter;

        if (_context->videoDecoderHandle)
            NEXUS_SimpleVideoDecoder_GetSettings(_context->videoDecoderHandle, &settings);

        if (_context->audioDecoderHandle)
            NEXUS_SimpleAudioDecoder_GetSettings(_context->audioDecoderHandle, &audioSettings);

        _simpleDecoder->connect();

        if (_context->videoDecoderHandle) {
            NEXUS_SimpleVideoDecoder_GetSettings(_context->videoDecoderHandle, &settingsAfter);
            settingsAfter.firstPts.callback = settings.firstPts.callback;
            settingsAfter.firstPts.context = settings.firstPts.context;
            settingsAfter.firstPtsPassed.callback = settings.firstPtsPassed.callback;
            settingsAfter.firstPtsPassed.context = settings.firstPtsPassed.context;
            settingsAfter.sourceChanged.callback = settings.sourceChanged.callback;
            settingsAfter.sourceChanged.context = settings.sourceChanged.context;
            settingsAfter.ptsError.callback = settings.ptsError.callback;
            settingsAfter.ptsError.context = settings.ptsError.context;
            settingsAfter.colorDepth = _videoParam.colorDepth;
            // Only set it if source didn't specify it and chip supports it
            if (_videoParam.colorDepth == 0) {
                settingsAfter.colorDepth = 8;
            }
            BME_DEBUG_TRACE(("color depth set to : %d\n", settingsAfter.colorDepth));
            NEXUS_SimpleVideoDecoder_SetSettings(_context->videoDecoderHandle, &settingsAfter);
        }

        if (_context->audioDecoderHandle) {
            NEXUS_SimpleAudioDecoder_GetSettings(_context->audioDecoderHandle, &audioSettingsAfter);
            audioSettingsAfter.primary.firstPts.callback = audioSettings.primary.firstPts.callback;
            audioSettingsAfter.primary.firstPts.context = audioSettings.primary.firstPts.context;
            audioSettingsAfter.primary.alwaysEnableDsola = true;
            NEXUS_SimpleAudioDecoder_SetSettings(_context->audioDecoderHandle, &audioSettingsAfter);
        }

        if (_videoParam.streamId &&
                (_context->videoDecoderHandle) &&
                (_source->getType() != SOURCE_HDMI)) {
            NEXUS_Error rc = NEXUS_SimpleVideoDecoder_Start(_context->videoDecoderHandle,
                             &_context->videoProgram);

            if (rc) {
                BME_DEBUG_ERROR(("Unable to start Simple Video Decoder!!"));
                onError(IMedia::MEDIA_ERROR_UNSUPPORTED_STREAM);
                return;
            }
        }

        if (_audioParam.streamId &&
                (_context->audioDecoderHandle) &&
                (_source->getType() != SOURCE_HDMI)) {
            NEXUS_Error rc = (NEXUS_SimpleAudioDecoder_Start(_context->audioDecoderHandle,
                              &_context->audioProgram));

            if (rc) {
                // If we can't start audio, we should still let video go on, instead
                // of flagging an error
                BME_DEBUG_ERROR(("Unable to start Simple Audio Decoder!!"));
            }
        }
    }

    _state = IMedia::StartedState;

    TRLS_MEDIA_DEBUG_EXIT();
}


void MediaPlayer::stop(bool holdLastFrame)
{
    TRLS_MEDIA_DEBUG_ENTER();

    if (_state == IMedia::StoppedState ||
        _state == IMedia::IdleState) {
        BME_DEBUG_ERROR(("Calling while in %s", printState().c_str()));
        TRLS_MEDIA_DEBUG_EXIT();
        return;
    }

    if (_options.primingMode) {
        if (_state == IMedia::StartedState) {
            stopPrimer();
            _simpleDecoder->setVisibility(false);
        }
    } else {
        if (_state == IMedia::PreparingState) {
            // end of stream already stopped most of playback so just clean up
            _state = IMedia::StoppedState;
            TRLS_MEDIA_DEBUG_EXIT();
            return;
        } else {
            _pictureHeld = holdLastFrame;
            stopDecoder();
            if (_source)
                _source->stop(holdLastFrame);
        }
        if (_connector) {
            delete _connector;
            _connector = NULL;
        }
    }
    stopSubtitle();

    if (!_pictureHeld) {
        _simpleDecoder->disconnect();
    }

    _state = IMedia::StoppedState;
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::stopPrimer()
{
    BME_CHECK(NEXUS_SimpleVideoDecoder_StopDecodeAndStartPrimer(
                   _context->videoDecoderHandle));

    if (_controlsAudio && _audioParam.streamId && _context->audioDecoderHandle) {
        NEXUS_SimpleAudioDecoder_Stop(_context->audioDecoderHandle);
        NEXUS_SimpleAudioDecoder_SetStcChannel(_context->audioDecoderHandle, NULL);
    }
}

void MediaPlayer::stopDecoder()
{
    TRLS_MEDIA_DEBUG_ENTER();

    if (!_resourceAcquired)
        return;

    if (_pictureHeld) {
        NEXUS_VideoDecoderSettings videoSettings;
        NEXUS_SimpleVideoDecoder_GetSettings(_context->videoDecoderHandle, &videoSettings);
        videoSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
        NEXUS_SimpleVideoDecoder_SetSettings(_context->videoDecoderHandle, &videoSettings);
        BME_DEBUG_TRACE(("Hold the last frame before timeshift playback starts"));
    }

    if (_videoParam.streamId && _context->videoDecoderHandle) {
        NEXUS_SimpleVideoDecoder_Stop(_context->videoDecoderHandle);
        NEXUS_SimpleVideoDecoder_SetStcChannel(_context->videoDecoderHandle, NULL);
    }

    if (_controlsAudio && _audioParam.streamId && _context->audioDecoderHandle) {
        NEXUS_SimpleAudioDecoder_Stop(_context->audioDecoderHandle);
        NEXUS_SimpleAudioDecoder_SetStcChannel(_context->audioDecoderHandle, NULL);
    }

    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::startDecoder()
{
    setDecoderStcChannel();
    if (_videoParam.streamId && _context->videoDecoderHandle) {
        NEXUS_SimpleVideoDecoder_Start(_context->videoDecoderHandle,
            &_context->videoProgram);
    }
    if (_controlsAudio && _audioParam.streamId && _context->audioDecoderHandle) {
        NEXUS_SimpleAudioDecoder_Start(_context->audioDecoderHandle,
            &_context->audioProgram);
    }
}
void MediaPlayer::reloadPidChannels()
{
    if (_videoParam.streamId) {
        _context->videoProgram.settings.pidChannel = _connector->videoPidChannel;
    }
    if (_audioParam.streamId) {
        _context->audioProgram.primary.pidChannel = _connector->audioPidChannel;
    }
}
void MediaPlayer::pause()
{
    TRLS_MEDIA_DEBUG_ENTER();
    _source->pause();
    _state = IMedia::PausedState;
    _previousPositionMSec = getCurrentPosition();
    TRLS_MEDIA_DEBUG_EXIT();
}

IMedia::ErrorType MediaPlayer::prepare()
{
    TRLS_MEDIA_DEBUG_ENTER();
    if ((_state != IMedia::IdleState) &&
        (_state != IMedia::StoppedState)) {
        BME_DEBUG_ERROR(("Invalid State (%s)", printState().c_str()));
        BME_DEBUG_THROW_EXCEPTION(ReturnInvalidState);
    }

    _source->setDecoderType(_options.decoderType);
    IMedia::ErrorType ret = _source->prepare();

    if (ret != IMedia::MEDIA_SUCCESS) {
        return ret;
    } else if (_options.primingMode) {
        startInternal();

        // Only set video decoder stcChannel here.
        // Setting audio decoder stcChannel here will
        // cause a slower channel change in fcc mode,
        // so set audio decoder stcChannel right before
        // NEXUS_SimpleAudioDecoder_Start when in fcc mode.
        // setDecoderStcChannel();
        if (_source->getType() != SOURCE_PUSHES) {
            if (_videoParam.streamId && _context->videoDecoderHandle) {
                BME_CHECK(NEXUS_SimpleVideoDecoder_SetStcChannel(_context->videoDecoderHandle,
                           _context->stcChannel));
            }
        }

        BME_CHECK(NEXUS_SimpleVideoDecoder_StartPrimer(_context->videoDecoderHandle,
                   &_context->videoProgram));
    }

    _state = IMedia::StoppedState;
    TRLS_MEDIA_DEBUG_EXIT();
    return ret;
}

void MediaPlayer::prepareAsync()
{
    TRLS_MEDIA_DEBUG_ENTER();

    if ((_state != IMedia::IdleState) &&
        (_state != IMedia::StoppedState)) {
        BME_DEBUG_ERROR(("Invalid State (%s)", printState().c_str()));
        BME_DEBUG_THROW_EXCEPTION(ReturnInvalidState);
    }

    _state = IMedia::PreparingState;
    _source->prepareAsync();
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::addMediaStreamListener()
{
    // Assumes only 1 listener per mediaStream per event
    std::function<void()> onChannelChange = std::bind(&MediaPlayer::onChannelChange, this);
    _channelChangeListener = _mediaStream->addListener(MediaStreamEvents::ChannelChanged,
        onChannelChange);
    std::function<void(uint32_t *stc)> onSTCRequest = std::bind(&MediaPlayer::onSTCRequest,
        this, std::placeholders::_1);
    _stcRequestedListener = _mediaStream->addListener(MediaStreamEvents::STCRequested,
        onSTCRequest);
}

void MediaPlayer::removeMediaStreamListener()
{
    TRLS_MEDIA_DEBUG_ENTER();
    if (_mediaStream) {
        _mediaStream->removeListener(MediaStreamEvents::ChannelChanged, _channelChangeListener);
        _mediaStream->removeListener(MediaStreamEvents::STCRequested, _stcRequestedListener);
    }
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::addSourceListener()
{
    std::function<void()> onCompletion = std::bind(&MediaPlayer::onCompletion, this);
    std::function<void()> onPrepared = std::bind(&MediaPlayer::onPrepared, this);
    std::function<void()> onBeginning = std::bind(&MediaPlayer::onBeginning, this);
    std::function<void(const IMedia::ErrorType)> onError =
        std::bind(&MediaPlayer::onError, this, _1);
    std::function<void(const IMedia::InfoType, int32_t extra)> onInfo =
        std::bind(&MediaPlayer::onInfo, this, _1, _2);
    std::function<void()> onSeekComplete = std::bind(&MediaPlayer::onSeekComplete, this);
    std::function<void(uint16_t width, uint16_t height)> onVideoSizeChanged =
        std::bind(&MediaPlayer::onVideoSizeChanged, this, _1, _2);
    std::function<void(const HDRMetadata)> onHdrSettings =
        std::bind(&MediaPlayer::onHdrSettings, this, _1);

    _source->addListener(SourceEvents::Completed, onCompletion);
    _source->addListener(SourceEvents::Prepared, onPrepared);
    _source->addListener(SourceEvents::Error, onError);
    _source->addListener(SourceEvents::Info, onInfo);
    _source->addListener(SourceEvents::SeekCompleted, onSeekComplete);
    _source->addListener(SourceEvents::VideoSizeChanged, onVideoSizeChanged);
    _source->addListener(SourceEvents::BeginningOfStream, onBeginning);
    _source->addListener(SourceEvents::HdrSettings, onHdrSettings);
}

void MediaPlayer::setDataSource(MediaStream* mediaStream)
{
    if ((_state != IMedia::IdleState) &&
        (_state != IMedia::StoppedState)) {
        BME_DEBUG_ERROR(("Invalid State (%s)", printState().c_str()));
        BME_DEBUG_THROW_EXCEPTION(ReturnInvalidState);
    }
    _uri = mediaStream->getUri();
    _svp = mediaStream->isSvp();
    _disabledProgressiveOverride = mediaStream->isProgressiveOverrideDisabled();
    BME_DEBUG_TRACE(("uri = %s", _uri.c_str()));
    // _mediaStream is only guarantee to be persistent in tv:// case
    // Be sure to remove any listeners that
    // were registered before changing mediaStreams
    if (_mediaStream) {
        removeMediaStreamListener();
    }
    _mediaStream = NULL;

    if (mediaStream->getSource()) {
        // Dispose of current source
        // If source has already been overrided, MediaPlayer
        // shouldn't release it since it's not the "owner".
        if (_source && !_sourceOverrided) {
            releaseSource();
        }

        _sourceOverrided = true;
        BME_DEBUG_TRACE(("Setting source (0x%x) from mediaStream", mediaStream->getSource()));
        _source = reinterpret_cast<BaseSource*>(mediaStream->getSource());
    } else {
        if ((_uri.find(IMedia::FILE_URI_PREFIX) != std::string::npos)) {
            if (_source != NULL && !_sourceOverrided) {
                if (_source->getType() != SOURCE_FILE) {
                    releaseSource();
                    _source = new FileSource();
                }
            } else {
                _source = new FileSource();
            }
#ifdef BME_ENABLE_TV
        } else if ((_uri.find(IMedia::TV_URI_PREFIX) != std::string::npos) ||
                    (_uri.find(IMedia::TIMESHIFT_URI_PREFIX) != std::string::npos)) {
            _mediaStream = mediaStream;
            if (_source != NULL && !_sourceOverrided) {
                if ((_source->getType() != SOURCE_TV) &&
                    (_source->getType() != SOURCE_TIMESHIFT_TV)) {
                    releaseSource();
                    if (_uri.find(IMedia::TV_URI_PREFIX) != std::string::npos) {
                        _source = new LiveSource();
                    } else if (_uri.find(IMedia::TIMESHIFT_URI_PREFIX) != std::string::npos) {
                        _source = new TimeshiftSource(_mediaStream->getRecorder());
                    }
                    _source->prepare();
                }
            } else {
                    if (_uri.find(IMedia::TIMESHIFT_URI_PREFIX) != std::string::npos) {
                        _source = new TimeshiftSource(_mediaStream->getRecorder());
                    } else if (_uri.find(IMedia::TV_URI_PREFIX) != std::string::npos) {
                        _source = new LiveSource();
                    }
                _source->prepare();
            }
            addMediaStreamListener();
#endif  // BME_ENABLE_TV
        } else if (_uri.find(IMedia::PUSH_ES_URI_PREFIX) != std::string::npos) {
            if (_source != NULL && !_sourceOverrided) {
                if (_source->getType() != SOURCE_PUSHES) {
                    releaseSource();
                    _source = new PushESSource();
                }
            } else {
                _source = new PushESSource();
            }
        } else if (_uri.find(IMedia::PUSH_TS_URI_PREFIX) != std::string::npos) {
            if (_source != NULL && !_sourceOverrided) {
                if (_source->getType() != SOURCE_PUSHTS) {
                    releaseSource();
                    _source = new PushTSSource();
                }
            } else {
                _source = new PushTSSource();
            }

#ifdef NEXUS_HAS_HDMI_INPUT
        } else if (_uri.find(IMedia::HDMI_URI_PREFIX) != std::string::npos) {
            if (_source != NULL && !_sourceOverrided) {
                if (_source->getType() != SOURCE_HDMI) {
                    releaseSource();
                    _source = new HdmiSource();
                }
            } else {
                _source = new HdmiSource();
            }

#endif  // NEXUS_HAS_HDMI_INPUT
        } else {
            BME_DEBUG_ERROR(("%s is not supported", _uri.c_str()));
            BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        }

        _sourceOverrided = false;
    }

    if (!_source) {
        BME_DEBUG_ERROR(("resolved source returned NULL"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
    }

    _source->setDataSource(mediaStream);
    addSourceListener();

    if (mediaStream->getVirtualWidth() != 0) {
        _virtualWidth = mediaStream->getVirtualWidth();
    }
    if (mediaStream->getVirtualHeight() != 0) {
        _virtualHeight = mediaStream->getVirtualHeight();
    }
    _surfaceClientId = mediaStream->getSurfaceClientId();
    _windowId =  mediaStream->getWindowId();
    if ((_source->getType() == SOURCE_TV) ||
        (_source->getType() == SOURCE_TIMESHIFT_TV)) {
        _state = IMedia::StoppedState;
    }
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::releaseSource()
{
    BME_DEBUG_ENTER();
    if (_source && !_sourceOverrided) {
        _source->release();
    }
    delete _source;
    _source = NULL;
    _sourceOverrided = false;
    BME_DEBUG_EXIT();
}

void MediaPlayer::resetVariables()
{
    _looping = false;
    _usePrimer = false;
    _controlsAudio = false;
    _resourceAcquired = false;
    _videoPositionPreset = false;
    _initialSeekTimeMSec = 0;
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;
    _options.primingMode = false;
    _options.decoderType = DecoderType::Main;
    _rate = std::string("1");
    _mediaStreamId = "";
}

void MediaPlayer::reset()
{
    TRLS_MEDIA_DEBUG_ENTER();

    if (_state == IdleState)
        return;

    if ((isPlaying()) ||
            (getState() == IMedia::PausedState))
        stop();

    if (_options.primingMode) {
        _source->stop();

        if (_connector) {
            delete _connector;
            _connector = NULL;
        }
    }

    releaseResources();
    resetVariables();

    _state = IdleState;
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::release()
{
    TRLS_MEDIA_DEBUG_ENTER();

    if (_state == IdleState)
        return;

    if ((_state == IMedia::StartedState) ||
        (_state == IMedia::PausedState)) {
        stop();
    }

    // in case it is not stopped
    if (_capturing)
        stopFrameCapture();

    if (_pictureHeld) {
        _simpleDecoder->disconnect();
        _pictureHeld = false;
    }

    if (_options.primingMode && _source) {
        _source->stop();

        if (_connector) {
            delete _connector;
            _connector = NULL;
        }
    }

    releaseResources();
    resetVariables();
    _state = IdleState;
    TRLS_MEDIA_DEBUG_EXIT();
}

std::string MediaPlayer::getType()
{
    return std::string("");
}

BaseSource* MediaPlayer::getSource()
{
    return _source;
}

void MediaPlayer::seekTo(uint32_t msec,
                         IMedia::PlaybackOperation playOperation,
                         IMedia::PlaybackSeekTime  playSeekTime)
{
    TRLS_MEDIA_DEBUG_ENTER();
    _source->seekTo(msec, playOperation, playSeekTime);
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::setPlaybackRate(const std::string& rate)
{
    if (!_source || rate.empty())
        return;

    _source->setPlaybackRate(rate);
    _rate = rate;
}

int MediaPlayer::getPlaybackRate()
{
    if (_source)
        return _source->getPlaybackRate();

    return 0;
}

IMedia::PlaybackOperation MediaPlayer::getPlaybackOperation()
{
    if (_source)
        return _source->getPlaybackOperation();

    return IMedia::OperationUnknown;
}

std::string MediaPlayer::getAvailablePlaybackRate()
{
    if (_source)
        return _source->getAvailablePlaybackRate();

    return "";
}

uint32_t MediaPlayer::getCurrentPosition()
{
    if (_source)
        return _source->getCurrentPosition();

    return 0;
}

uint64_t MediaPlayer::getDuration()
{
    if (_source)
        return _source->getDuration();

    return 0;
}

const IMedia::TimeInfo MediaPlayer::getTimeInfo()
{
    TimeInfo timeinfo;
    BME_DEBUG_ENTER();

    if (_source)
        return _source->getTimeInfo();

    BME_DEBUG_EXIT();
    return timeinfo;
}

bool MediaPlayer::isPlaying()
{
    return (_state == IMedia::StartedState);
}

bool MediaPlayer::isLooping()
{
    return _looping;
}

void MediaPlayer::setLooping(bool looping)
{
    _looping = looping;

    if (_source)
        _source->setLooping(looping);
}

TIME45k MediaPlayer::getCurrentPts()
{
    return 0;
}

uint32_t MediaPlayer::getVideoHeight()
{
    if (!_source)
        return 0;

    _streamMetadata = _source->getStreamMetadata();

    _videoParam = _streamMetadata.videoParam;

#ifdef BME_ENABLE_TV
    NEXUS_VideoDecoderStreamInformation streamInfo;
    if (_context->videoDecoderHandle) {
        NEXUS_SimpleVideoDecoder_GetStreamInformation(_context->videoDecoderHandle, &streamInfo);
        BME_DEBUG_TRACE(("sourceHorizontalSize=%d, sourceVerticalSize=%d\n",
            streamInfo.sourceHorizontalSize, streamInfo.sourceVerticalSize));
        _videoParam.maxHeight = streamInfo.sourceVerticalSize;
    }
#endif

    return _videoParam.maxHeight;
}

uint32_t MediaPlayer::getVideoWidth()
{
    if (!_source)
        return 0;

    _streamMetadata = _source->getStreamMetadata();

    _videoParam = _streamMetadata.videoParam;

#ifdef BME_ENABLE_TV
    NEXUS_VideoDecoderStreamInformation streamInfo;

    if (_context->videoDecoderHandle) {
        NEXUS_SimpleVideoDecoder_GetStreamInformation(_context->videoDecoderHandle, &streamInfo);
        BME_DEBUG_TRACE(("sourceHorizontalSize=%d, sourceVerticalSize=%d\n",
            streamInfo.sourceHorizontalSize, streamInfo.sourceVerticalSize));
        _videoParam.maxWidth = streamInfo.sourceHorizontalSize;
    }
#endif

    return _videoParam.maxWidth;
}

uint32_t MediaPlayer::getVideoDisplayHeight()
{
    if (!_videoParam.streamId) {
        BME_DEBUG_PRINT(("getVideoDisplayHeight on audio files"));
        TRLS_MEDIA_DEBUG_EXIT();
        return 0;
    }

    if (!_resourceAcquired) {
        return _height;
    } else {
        return _simpleDecoder->getVideoDisplayHeight();
    }
}

uint32_t MediaPlayer::getVideoDisplayWidth()
{
    if (!_videoParam.streamId) {
        BME_DEBUG_PRINT(("getVideoDisplayHeight on audio files"));
        TRLS_MEDIA_DEBUG_EXIT();
        return 0;
    }

    if (!_resourceAcquired) {
        return _width;
    } else {
        return _simpleDecoder->getVideoDisplayWidth();
    }
}

IMedia::VideoFrameRate MediaPlayer::getVideoFrameRate()
{
    NEXUS_VideoDecoderStatus status;

    if (_context->videoDecoderHandle) {
        BME_CHECK(NEXUS_SimpleVideoDecoder_GetStatus(_context->videoDecoderHandle, &status));
        return (IMedia::VideoFrameRate)status.frameRate;
    }

    return VideoFrameRateUnknown;
}

IMedia::VideoAspectRatio MediaPlayer::getVideoAspectRatio()
{
    NEXUS_VideoDecoderStatus status;
    NEXUS_VideoDecoderStreamInformation streamInfo;
    uint32_t sourceHorizontalSize, sourceVerticalSize;

    if (_context->videoDecoderHandle) {
        BME_CHECK(NEXUS_SimpleVideoDecoder_GetStatus(_context->videoDecoderHandle, &status));

        if (status.aspectRatio == NEXUS_AspectRatio_eSar) {
            NEXUS_SimpleVideoDecoder_GetStreamInformation(_context->videoDecoderHandle, &streamInfo);
            sourceHorizontalSize = streamInfo.sourceHorizontalSize;
            sourceVerticalSize = streamInfo.sourceVerticalSize;
            BME_DEBUG_TRACE(("eSar: sourceHorizontalSize=%d, sourceVerticalSize=%d\n",
                sourceHorizontalSize, sourceVerticalSize));
            BME_DEBUG_TRACE(("eSar: sampleAspectRatioX=%d, sampleAspectRatioY=%d\n",
                streamInfo.sampleAspectRatioX, streamInfo.sampleAspectRatioY));
            if ((streamInfo.sampleAspectRatioX == 1) && (streamInfo.sampleAspectRatioY == 1)) {
                if (4*sourceVerticalSize == 3*sourceHorizontalSize) {
                    return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e4x3;
                } else if (16*sourceVerticalSize == 9*sourceHorizontalSize) {
                    return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e16x9;
                } else if (15*sourceVerticalSize == 9*sourceHorizontalSize) {
                    return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e15x9;
                }
            } else if ((streamInfo.sampleAspectRatioX == 4) && (streamInfo.sampleAspectRatioY == 3)) {
                return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e4x3;
            } else if ((streamInfo.sampleAspectRatioX == 16) && (streamInfo.sampleAspectRatioY == 9)) {
                return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e16x9;
            } else if ((streamInfo.sampleAspectRatioX == 15) && (streamInfo.sampleAspectRatioY == 9)) {
                return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e15x9;
            }
            return (IMedia::VideoAspectRatio)status.aspectRatio;
        } else if (status.aspectRatio == NEXUS_AspectRatio_eUnknown) {
            NEXUS_SimpleVideoDecoder_GetStreamInformation(_context->videoDecoderHandle, &streamInfo);
            sourceHorizontalSize = streamInfo.sourceHorizontalSize;
            sourceVerticalSize = streamInfo.sourceVerticalSize;
            BME_DEBUG_TRACE(("sourceHorizontalSize=%d, sourceVerticalSize=%d\n",
                sourceHorizontalSize, sourceVerticalSize));
            if ((sourceHorizontalSize != 0) && (sourceVerticalSize != 0)) {
                if (4*sourceVerticalSize == 3*sourceHorizontalSize) {
                    return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e4x3;
                } else if (16*sourceVerticalSize == 9*sourceHorizontalSize) {
                    return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e16x9;
                } else if (15*sourceVerticalSize == 9*sourceHorizontalSize) {
                    return (IMedia::VideoAspectRatio)NEXUS_AspectRatio_e15x9;
                }
            }
            return VideoAspectRatioUnknown;
        } else {
            return (IMedia::VideoAspectRatio)status.aspectRatio;
        }
    }

    return VideoAspectRatioUnknown;
}

IMedia::AudioParametersList MediaPlayer::getTrackInfo()
{
    return _streamMetadata.audioParamList;
}

int32_t MediaPlayer::getTrack()
{
    BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
}

void MediaPlayer::selectTrack(int32_t uid)
{
    if (uid < 0) {
        uid = 0;  // default is track 0
    }

    if ((uint32_t)uid >= _streamMetadata.audioParamList.size()) {
        BME_DEBUG_ERROR(("Track selected is greater than number of audio tracks\n"));
        BME_DEBUG_THROW_EXCEPTION(ReturnInvalidParam);
        return;
    }

    IMedia::AudioParameters audioParam;
    audioParam = _streamMetadata.audioParamList[uid];
    selectAudioParameters(audioParam);
}

void MediaPlayer::selectAudioParameters(const IMedia::AudioParameters& param)
{
    // if (param.streamId == 0 || param.audioCodec == IMedia::UnknownAudioCodec) {
    //     BME_DEBUG_ERROR(("Invalid audio parameters"));
    //     BME_DEBUG_THROW_EXCEPTION(ReturnInvalidParam);
    //     return;
    // }

    // // stop audio decoder and close pid channel
    // if (_controlsAudio && _audioParam.streamId && _context->audioDecoderHandle) {
    //     NEXUS_SimpleAudioDecoder_Stop(_context->audioDecoderHandle);
    //     NEXUS_SimpleAudioDecoder_SetStcChannel(_context->audioDecoderHandle, NULL);
    // }

    // SourceConnector pin;
    // pin.audioPidChannel = _connector->audioPidChannel;
    // _source->disconnect(pin);
    // // Create new pid channel and restart
    // ConnectSettings settings;
    // settings.streamId = param.streamId;
    // settings.audioCodec = param.audioCodec;
    // SourceConnector outPin = _source->connect(settings);
    // NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&_context->audioProgram);
    // _context->audioProgram.primary.codec = BaseSource::convertAudioCodec(param.audioCodec);
    // _context->audioProgram.primary.pidChannel = outPin.audioPidChannel;
    // if (_context->audioProgram.primary.pidChannel) {
    //     NEXUS_SimpleAudioDecoder_SetStcChannel(_context->audioDecoderHandle, _context->stcChannel);
    //     NEXUS_Error rc = NEXUS_SimpleAudioDecoder_Start(_context->audioDecoderHandle,
    //                     &_context->audioProgram);
    //     if (rc) {
    //         // print the error, but don't throw exception in this case
    //         BME_DEBUG_ERROR(("Unable to start audio decoder!!"));
    //     }
    // }
}

int32_t MediaPlayer::getAudioVolume()
{
    if (!_resourceAcquired)
        return 0;

    return _simpleDecoder->getAudioVolume();
}

void MediaPlayer::setAudioVolume(int32_t volume)
{
    if (!_resourceAcquired)
        return;

    _simpleDecoder->setAudioVolume(volume);
}

void MediaPlayer::mute()
{
    if (!_resourceAcquired)
        return;

    _simpleDecoder->mute(true);
}

void MediaPlayer::unmute()
{
    if (!_resourceAcquired)
        return;

    _simpleDecoder->mute(false);
}

uint8_t MediaPlayer::getAudioDescriptionVolume()
{
    BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
}

void MediaPlayer::setAudioDescriptionVolume(const uint8_t &volume)
{
    BME_DEBUG_THROW_EXCEPTION(ReturnNotSupported);
}

void MediaPlayer::setSubtitleLanguage(const std::string& language)
{
    if (_subtitle)
        _subtitle->setLanguage(language);
}

void MediaPlayer::setSubtitle(bool enable)
{
    if (enable && !_subtitleEnabled) {
        _subtitleEnabled = true;

        if ((_state == IMedia::StartedState) ||
            (_state == IMedia::PausedState)) {
            startSubtitle();
        }
    } else if (!enable && _subtitleEnabled) {
        // turn it off
        if ((_state == IMedia::StartedState) ||
            (_state == IMedia::PausedState)) {
            stopSubtitle();
        }
        _subtitleEnabled = false;
        _subtitleVisible = false;
    }
}

void MediaPlayer::setSubtitleVisibility(bool visible)
{
    if (_subtitle) {
        _subtitleVisible = visible;
        _subtitle->setVisibility(visible);
    }
}

bool MediaPlayer::getSubtitleVisibility()
{
    return _subtitleVisible;
}

void MediaPlayer::setVideoWindowPosition(int32_t x, int32_t y,
        uint32_t width, uint32_t height, int32_t videoStreamIndex)
{
    BME_DEBUG_TRACE(("setVideoWindowPosition: stream id %d state = %d %d %d %d %d\n",
                      _videoParam.streamId, _state, x, y, width, height));

    // _videoParam won't be set till prepared
    if (isPlaying() && !_videoParam.streamId) {
        BME_DEBUG_PRINT(("setVideoWindowPosition on audio only files"));
        TRLS_MEDIA_DEBUG_EXIT();
        return;
    }

    if (!_resourceAcquired) {
        _x = x;
        _y = y;
        _width = width;
        _height = height;
        _videoPositionPreset = true;
        BME_DEBUG_TRACE((" window is not initialized"));
        TRLS_MEDIA_DEBUG_EXIT();
    } else {
        _simpleDecoder->setLocation(x, y, width, height);
    }
}

void MediaPlayer::setVideoWindowVisibility(bool visible, int32_t videoStreamIndex)
{
    if (!_resourceAcquired) {
        _visible = visible;
    }

    if (!_videoParam.streamId) {
        BME_DEBUG_ERROR(("setVideoWindowVisibility on audio only files"));
        TRLS_MEDIA_DEBUG_EXIT();
        return;
    }

    _simpleDecoder->setVisibility(visible);
}

IMedia::State MediaPlayer::getState()
{
    return _state;
}

void MediaPlayer::startFrameCapture(CaptureSurfaceFormat pixelFormat,
                                    uint32_t decodeWidth,
                                    uint32_t decodeHeight,
                                    bool displayEnabled,
                                    void *surfaceAddress,
                                    bool secure)
{
    NEXUS_SimpleVideoDecoderStartCaptureSettings captureSettings;
    NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&captureSettings);
    captureSettings.displayEnabled = displayEnabled;
    captureSettings.secure = secure;
    captureSettings.forceFrameDestripe = true;

    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = decodeWidth;
    createSettings.height = decodeHeight;
    if (secure) {
        createSettings.heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE);
    }

    for (uint i = 0; i < NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++) {
        captureSettings.surface[i] = (NEXUS_SurfaceHandle)surfaceAddress;
    }
    captureSettings.secure = secure;

    BME_CHECK(NEXUS_SimpleVideoDecoder_StartCapture(_context->videoDecoderHandle,
               &captureSettings));
    _capturing = true;
    _context->captureSurface = (NEXUS_SurfaceHandle)surfaceAddress;
    BME_DEBUG_EXIT();
}

void MediaPlayer::stopFrameCapture()
{
    TRLS_MEDIA_DEBUG_ENTER();
    NEXUS_SimpleVideoDecoder_StopCapture(_context->videoDecoderHandle);
    _capturing = false;
    TRLS_MEDIA_DEBUG_EXIT();
}

size_t MediaPlayer::getFrame(uint32_t count, bool flip)
{
    if (!_capturing) {
        BME_DEBUG_ERROR(("Must call start capture before startFrameCapture"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
    }
    uint numReturned = 0;
    NEXUS_SurfaceHandle surface = NULL;
    NEXUS_SimpleVideoDecoderCaptureStatus captureStatus;
    NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(_context->videoDecoderHandle,
            &surface, &captureStatus,
            count, &numReturned);

    if (numReturned == 0) {
        BKNI_Sleep(10);
        return (int32_t)NULL;
    }
    NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(_context->videoDecoderHandle,
            &surface, numReturned);
    return (size_t)_context->captureSurface;
}

void MediaPlayer::disableMvcMode()
{
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    if (!_videoParam.streamId)
        return;

    // Turn off SVC/MVC capability when not needed to save memory
    if (((NEXUS_VideoCodec)_videoParam.videoCodec != NEXUS_VideoCodec_eH264_Mvc) &&
            ((NEXUS_VideoCodec)_videoParam.videoCodec != NEXUS_VideoCodec_eH264_Svc)) {
        NEXUS_SimpleVideoDecoder_GetSettings(_context->videoDecoderHandle, &videoDecoderSettings);
        videoDecoderSettings.supportedCodecs[NEXUS_VideoCodec_eH264_Svc] = false;
        videoDecoderSettings.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = false;
        BME_CHECK(NEXUS_SimpleVideoDecoder_SetSettings(_context->videoDecoderHandle,
                   &videoDecoderSettings));
    }
}

#define VP9_MAXWIDTH 3840
#define VP9_MAXHIGHT 2160

void MediaPlayer::acquireResources()
{
    TRLS_MEDIA_DEBUG_ENTER();

    // stc will be destroyed when decoder is stopped, so create again
    if (_source->getType() != SOURCE_HDMI) {
        if (!_context->stcChannel) {
            _context->stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
        }

        if (!_context->stcChannel)  {
            BME_DEBUG_ERROR(("Unable to create NEXUS StcChannel channel"));
        }
    }

    if (_resourceAcquired) {
        // need to run this each time in case codec changed
        disableMvcMode();
        TRLS_MEDIA_DEBUG_EXIT();
        return;
    }

    SimpleDecoder::VideoWindow videoWindow = SimpleDecoder::VideoWindowMain;

    if (_options.decoderType == DecoderType::Pip) {
        videoWindow = SimpleDecoder::VideoWindowPip;
    }

    assert(_simpleDecoder);
    bool requestAudio = ((_audioParam.audioCodec != IMedia::UnknownAudioCodec)
                         && (!isPipOrMosaic())) || (_source->getType() == SOURCE_HDMI);
    bool requestVideo = _videoParam.streamId || (_source->getType() == SOURCE_HDMI);
    if (BaseSource::convertVideoCodec(_videoParam.videoCodec) == NEXUS_VideoCodec_eVp9) {
       _videoParam.maxWidth = VP9_MAXWIDTH;
       _videoParam.maxHeight = VP9_MAXHIGHT;
    }

    SimpleDecoder::acquireParameters acquireParam;
    acquireParam.mediaPlayer = this;
    acquireParam.requestVideoDecoder = requestVideo;
    acquireParam.requestAudioDecoder = requestAudio;
    acquireParam.videoWindow = videoWindow;
    acquireParam.maxWidth = _videoParam.maxWidth;
    acquireParam.maxHeight = _videoParam.maxHeight;
    acquireParam.virtualWidth = _virtualWidth;
    acquireParam.virtualHeight = _virtualHeight;
    acquireParam.decoderType = (uint16_t)_options.decoderType;
    acquireParam.surfaceClientId = _surfaceClientId;
    acquireParam.windowId = _windowId;
    acquireParam.secureVideo = _svp;
    acquireParam.persistent = true;
    if ((_videoParam.colorDepth == 0) ||
       (_videoParam.colorDepth > 12)) {
       _videoParam.colorDepth = 8;
    }
    acquireParam.colorDepth = _videoParam.colorDepth;
    _simpleDecoder->acquireSimpleDecoders(acquireParam,
                                          &_context->videoDecoderHandle,
                                          &_context->audioDecoderHandle);
    _audioVolumeBase = new AudioOutputDecoder(_context->audioDecoderHandle, false, "TMP");
    enableAtmos(_context->audioDecoderHandle);

    // set stcChannel to all decoders before starting any
    // set stcChannel should be done before _source->start()
    setDecoderStcChannel();

    if (requestVideo)
        assert(_context->videoDecoderHandle);

    if (requestAudio)
        assert(_context->audioDecoderHandle);

    _controlsAudio = requestAudio;

    if (!_visible) {
        setVideoWindowVisibility(_visible);
    }

    _resourceAcquired = true;

    if (true == _videoPositionPreset) {
        setVideoWindowPosition(_x, _y, _width, _height);
        _videoPositionPreset = false;
    }

    disableMvcMode();
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::releaseResources()
{
    TRLS_MEDIA_DEBUG_ENTER();

    releaseSource();
    if (_resourceAcquired) {
        _simpleDecoder->releaseSimpleDecoders(_context->videoDecoderHandle,
                                              _context->audioDecoderHandle);
        delete _audioVolumeBase;
        _audioVolumeBase = 0;
        _context->videoDecoderHandle = NULL;
        _context->audioDecoderHandle = NULL;

        if (_context->stcChannel) {
            NEXUS_SimpleStcChannel_Destroy(_context->stcChannel);
            _context->stcChannel = NULL;
        }

        _resourceAcquired = false;
        _videoPositionPreset = false;
    }
    // Remove mediaStream listeners
    removeMediaStreamListener();

    TRLS_MEDIA_DEBUG_EXIT();
}

// Source Listener
void MediaPlayer::onPrepared()
{
    // Only execute callback if we are in "Preparing".
    // (ie cancel the callback to the listeners if app called
    // reset() while we were in "Preparing" state)
    if (_state == IMedia::PreparingState) {
        _state = IMedia::StoppedState;

        if (_options.primingMode) {
            startInternal();

            // Only set video decoder stcChannel here.
            // Setting audio decoder stcChannel here will
            // cause a slower channel change in fcc mode,
            // so set audio decoder stcChannel right before
            // NEXUS_SimpleAudioDecoder_Start when in fcc mode.
            // setDecoderStcChannel();
            if (_source->getType() != SOURCE_PUSHES) {
                if (_videoParam.streamId && _context->videoDecoderHandle) {
                    BME_CHECK(NEXUS_SimpleVideoDecoder_SetStcChannel(_context->videoDecoderHandle,
                               _context->stcChannel));
                }
            }

            BME_CHECK(NEXUS_SimpleVideoDecoder_StartPrimer(_context->videoDecoderHandle,
                       &_context->videoProgram));
        }

        notify(MediaPlayerEvents::Prepared);
        // at this point we should have video size
        _streamMetadata = _source->getStreamMetadata();

        memset(&_videoParam, 0, sizeof(VideoParameters));
        _videoParam = _streamMetadata.videoParam;
        onVideoSizeChanged(_videoParam.maxWidth, _videoParam.maxHeight);
    }
}

void MediaPlayer::onChannelChange()
{
    BME_DEBUG_TRACE(("onChannelChange\n"));
    _subtitle = _mediaStream->getSubtitle();
    if (_mediaStream->getId() != _mediaStreamId) {
        if (_state == IMedia::StartedState)
            stop();
        start();
        _mediaStreamId = _mediaStream->getId();
    } else if (_state == IMedia::StoppedState) {
        // App could've explicitly stopped the mediaPlayer
        // (ie when they start channel scan)
        start();
    }
}

void MediaPlayer::onSTCRequest(uint32_t* stc)
{
    if (_context->stcChannel)
        NEXUS_SimpleStcChannel_GetStc(_context->stcChannel, stc);
    else
        *stc = 0;
}

void MediaPlayer::onCompletion()
{
    TRLS_MEDIA_DEBUG_ENTER();
    if (_uri.find(IMedia::TIMESHIFT_URI_PREFIX) != std::string::npos) {
        // Timeshift source is switching from Playback to Live
        _pictureHeld = true;
        stopDecoder();
        reloadPidChannels();
        startDecoder();
        _pictureHeld = false;
        notify(MediaPlayerEvents::Completed);
    } else {
        BME_DEBUG_TRACE(("looping = %d\n", _looping));

        if (!_looping) {
            notify(MediaPlayerEvents::Completed);
        }
    }
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::onError(const IMedia::ErrorType& errorType)
{
    if (isPlaying()) {
        _keepCapturing = true;
        stop();

        // If looping is set then we try to restart it
        if (_looping) {
            prepare();
            start();
        } else {
            notify(MediaPlayerEvents::Error, errorType);
        }
    } else {
        if (_state != IdleState) {
            notify(MediaPlayerEvents::Error, errorType);
        }
    }
}

void MediaPlayer::onInfo(const IMedia::InfoType& infoType, int32_t extra)
{
    if (_uri.find(IMedia::TIMESHIFT_URI_PREFIX) != std::string::npos) {
        // Timeshift source is switching from Live to Playback
        if (infoType == MEDIA_INFO_DVR_TSB_PLAYBACK_START) {
            _pictureHeld = true;
            stopDecoder();
            reloadPidChannels();
            startDecoder();
            _pictureHeld = false;
        }
    }
    notify(MediaPlayerEvents::Info, infoType, extra);
}

void MediaPlayer::onSeekComplete()
{
    notify(MediaPlayerEvents::SeekCompleted);
}

void MediaPlayer::onVideoSizeChanged(uint16_t width, uint16_t height)
{
    notify(MediaPlayerEvents::VideoSizeChanged, width, height);
}

void MediaPlayer::onBeginning()
{
    TRLS_MEDIA_DEBUG_ENTER();
    if (_uri.find(IMedia::TIMESHIFT_URI_PREFIX) != std::string::npos) {
        setPlaybackRate("1");
        notify(MediaPlayerEvents::Beginning);
    } else {
        notify(MediaPlayerEvents::Beginning);
        pause();
        seekTo(0, OperationForward, SeekTimeAbsolute);
        if (_looping) {
            start();
            notify(MediaPlayerEvents::PlaybackRateChanged);
        } else {
            notify(MediaPlayerEvents::Completed);
        }
    }

    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::onHdrSettings(const HDRMetadata hdrMetadata)
{
    TRLS_MEDIA_DEBUG_ENTER();
    setHdrMetadata(hdrMetadata);
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::setHdrMetadata(const HDRMetadata hdrMetadata)
{
    TRLS_MEDIA_DEBUG_ENTER();
     NEXUS_VideoDecoderStatus status;
    _hdrMetadata = hdrMetadata;
    _isHdrVideo = true;

    BME_DEBUG_TRACE(("HDR stream (%d)\n", hdrMetadata._transfer_id));
    if (_context->videoDecoderHandle) {
        NEXUS_SimpleVideoDecoder_GetStatus(_context->videoDecoderHandle, &status);

        if (status.started) {
            NEXUS_SimpleVideoDecoder_Stop(_context->videoDecoderHandle);

            MasteringMetadata *pSrcMm = &(_hdrMetadata._mastering_metadata);
            NEXUS_MasteringDisplayColorVolume *pDstMm =
                &(_context->videoProgram.settings.masteringDisplayColorVolume);

            if (_hdrMetadata._transfer_id == Broadcom::Media::kTransferIdSmpteSt2084)
                _context->videoProgram.settings.eotf = NEXUS_VideoEotf_eHdr10;
            else if (_hdrMetadata._transfer_id == Broadcom::Media::kTransferIdAribStdB67)
                _context->videoProgram.settings.eotf = NEXUS_VideoEotf_eHlg;

            _context->videoProgram.settings.contentLightLevel.max = _hdrMetadata._max_cll;
            _context->videoProgram.settings.contentLightLevel.maxFrameAverage = _hdrMetadata._max_fall;

            pDstMm->redPrimary.x = (pSrcMm->_red_chromaticity_x * 50000);
            pDstMm->redPrimary.y = (pSrcMm->_red_chromaticity_y * 50000);
            pDstMm->greenPrimary.x = (pSrcMm->_green_chromaticity_x * 50000);
            pDstMm->greenPrimary.y = (pSrcMm->_green_chromaticity_y * 50000);
            pDstMm->bluePrimary.x = (pSrcMm->_blue_chromaticity_x * 50000);
            pDstMm->bluePrimary.y = (pSrcMm->_blue_chromaticity_y * 50000);
            pDstMm->whitePoint.x = (pSrcMm->_white_chromaticity_x * 50000);
            pDstMm->whitePoint.y = (pSrcMm->_white_chromaticity_y * 50000);
            pDstMm->luminance.max = (pSrcMm->_luminance_max);
            pDstMm->luminance.min = (pSrcMm->_luminance_min * 10000);

            NEXUS_SimpleVideoDecoder_Start(_context->videoDecoderHandle,
                &_context->videoProgram);
        }
    }
    TRLS_MEDIA_DEBUG_EXIT();
}

void MediaPlayer::releaseVideoDecoder()
{
    TRLS_MEDIA_DEBUG_ENTER();
    onCompletion();
    _simpleDecoder->releaseSimpleDecoders(_context->videoDecoderHandle,
                                          _context->audioDecoderHandle);
    delete _audioVolumeBase;
    _audioVolumeBase = 0;
    _context->videoDecoderHandle = NULL;
    _context->audioDecoderHandle = NULL;
    _decoderReleased = true;
    TRLS_MEDIA_DEBUG_EXIT();
}

std::string MediaPlayer::printState()
{
    switch (_state) {
    case IMedia::IdleState:
        return "Idle";

    case IMedia::PreparingState:
        return "Preparing";

    case IMedia::StoppedState:
        return "Stopped";

    case IMedia::StartedState:
        return "Started";

    case IMedia::PausedState:
        return "Paused";

    default:
        return "Invalid";
    }
}

bool MediaPlayer::isPipOrMosaic()
{
    return (_options.decoderType == DecoderType::Mosaic) ||
           (_options.decoderType == DecoderType::Mosaic_HD) ||
           (_options.decoderType == DecoderType::Pip);
}

void MediaPlayer::SourceonCompletion()
{
    onCompletion();
}

void MediaPlayer::SourceonPrepared()
{
    onPrepared();
}

void MediaPlayer::startSubtitle()
{
    if (_subtitle && _subtitleEnabled) {
        _subtitle->start(_mediaStream);
        _subtitleVisible = true;
    }
}

void MediaPlayer::stopSubtitle()
{
    if (_subtitle && _subtitleEnabled) {
        _subtitle->stop();
    }
}

IMedia::VideoCodec MediaPlayer::getVideoCodec()
{
    return _videoParam.videoCodec;
}

IMedia::AudioCodec MediaPlayer::getAudioCodec()
{
    return _audioParam.audioCodec;
}

uint16_t MediaPlayer::getVideoPid()
{
    return _videoParam.streamId;
}

uint16_t MediaPlayer::getVideoPcrPid()
{
    return _videoParam.substreamId;
}

uint16_t MediaPlayer::getVideoColorDepth()
{
    return _videoParam.colorDepth;
}

uint16_t MediaPlayer::getAudioPid()
{
    return _audioParam.streamId;
}

float QuadraticEaseOut(float t, float b, float c, float d)
{
    t /= d;
    return -c * t * (t - 2) + b;
}

void MediaPlayer::animatePosition(uint32_t duration,
        const IMedia::Rect& start, const IMedia::Rect& end)
{
    uint32_t count = duration / 10;
    float width, height, x, y;

    for (unsigned i = 0; i < count; i++) {
        width = QuadraticEaseOut(i*10, start.width, (end.width - start.width), duration);
        height = QuadraticEaseOut(i*10, start.height, (end.height - start.height), duration);
        x = QuadraticEaseOut(i*10, start.x, (end.x - start.x), duration);
        y = QuadraticEaseOut(i*10, start.y, (end.y - start.y), duration);
        setVideoWindowPosition(x, y, width, height);

        std::unique_lock<std::mutex> lk(_mutex);
        if (_cv.wait_for(lk, std::chrono::milliseconds(10), [&]() {return (_exit == 1);})) {
            return;
        }
    }
}

void MediaPlayer::animateVideoWindowPosition(const IMedia::Rect& start,
        const IMedia::Rect& end, uint32_t duration)
{
    if (_animationThread.joinable()) {
        _exit = 1;
        _cv.notify_all();
        _animationThread.join();
    }
    _exit = 0;
    _animationThread = std::thread(&MediaPlayer::animatePosition, this, duration, start, end);
}

}  // namespace Media
}  // namespace Broadcom
