/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
// stdint must be included after bstd(_defs)
#include "bstd.h"
#include <stdint.h>

#include "PushESSource.h"
#include "Playpump.h"
#include "WavFormatHeader.h"
#include "nexus_timebase.h"
#include "bmedia_util.h"
#include "nexus_types.h"
#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_playback.h"
#include "bmedia_pcm.h"
#include "bmedia_probe.h"
#include "nexus_simple_video_decoder_server.h"
#include "nexus_security.h"
#include "nexus_video_window.h"
#include "nexus_dma.h"
#include "nexus_platform_client.h"
#if defined(BRCM_SAGE)
#include "nexus_security_client.h"
#include "sage_srai.h"
#endif
#include "nxclient_global.h" // NxClient_{Get|Set}AudioProcessingSettings()
#include "MediaDrmContext.h"

#include "bfile_stdio.h"

#include <errno.h>
#include <string>
#include <cstring>

#include <assert.h>

typedef uint UINT32_C;

static const size_t AudioFifoBytes = 1U << 20;
static const size_t VideoFifoBytes = 8U << 20;

namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(PushESSource);

const uint32_t TASK_STACK_SIZE = 32 << 10;
//#define STREAM_CAPTURE

// Remove the format specifier to write to a single file
#define VIDEOPES_CAPTURE_name "video%06u.pes"
#define AUDIOPES_CAPTURE_name "audio%06u.pes"

struct StreamCapture_t
{
    FILE        *file;
    unsigned int count;
    const char  *baseName;

    StreamCapture_t(const char *name)
    {
        file = 0;
        count = 0;
        baseName = name;
    }

    void createFile();

    void close()
    {
        if (file == 0)
            return;
        fclose(file);
        file = 0;
    }

    void write(const void *data, size_t bytes);
};

void StreamCapture_t::createFile()
{
#ifdef STREAM_CAPTURE
    if (file == 0) {
        char fname[64];
        snprintf(fname, sizeof(fname), baseName, count);
        ++count;
        file = fopen(fname, "ab");
    }
#endif
}

void StreamCapture_t::write(const void *data, size_t bytes)
{
#ifdef STREAM_CAPTURE
    createFile();
    if (file != 0) {
        fwrite(data, bytes, 1, file);
        close();
    }
#endif
}

struct Stream_t
{
    Playpump_t       pump;

    StreamCapture_t  capture;

    bool             secure;

    Stream_t(const char *CaptureName) : capture(CaptureName), secure(true)
    {
    }
    bool initialise(bool secure, size_t bytes);
    void deinitialise();
};

bool Stream_t::initialise(bool secure, size_t bytes)
{
    if (!pump.initialise(secure, bytes))
        return false;
    return true;
}

void Stream_t::deinitialise()
{
    pump.deinitialise();
}

struct PushESSourcePrivate_t
{
    unsigned int initialised;

    struct
    {
        uint32_t width;
        uint32_t height;
    } videoDimensions;

    WavFormatHeader *wavHeader;

    struct
    {
        uint16_t pcr;
        uint16_t audio;
        uint16_t video;

        struct
        {
            NEXUS_PidChannelHandle pcr;
            NEXUS_PidChannelHandle audio;
            NEXUS_PidChannelHandle video;
        } channel;
    } pid;

    // Parameters for streaming data
    Stream_t audio;
    Stream_t video;

    // Drm_Reader_Decrypt is not reentrant
    BKNI_MutexHandle drmLock;

    struct
    {
        BKNI_EventHandle video;
        BKNI_EventHandle stateChange;
        BKNI_EventHandle audioUnderflow;
    } event;

    struct
    {
        NEXUS_AudioCodec audio;
        NEXUS_VideoCodec video;
    } codec;

    bool stcMode; // Auto?

    struct
    {
        unsigned int channels;
        unsigned int sampleRate;
    } audioParameters;

    NEXUS_TransportType transport;
    uint16_t colorDepth;

    PushESSourcePrivate_t() : initialised(0), audio(AUDIOPES_CAPTURE_name), video(VIDEOPES_CAPTURE_name)
    {
    }
};

// ===================================================================

PushESSource::PushESSource() :
        _context(NULL),
        _playbackRate(1000),
        _state(IMedia::IdleState),
        _lastStc(0),
        _ptsHi(0),
        _videoPaused(false)
{
    initialise(true);
}

PushESSource::~PushESSource()
{
    deinitialise();

    if (_prepareAsyncThread.joinable()) {
        _prepareAsyncThread.join();
    }
    if (_context)
        delete _context;
}

static void SetCallback(NEXUS_CallbackDesc *instance, NEXUS_Callback function, void *context)
{
    instance->callback = function;
    instance->context  = context;
    instance->param    = 0;  // Unused
}

bool PushESSource::initialise(bool basic)
{
    PushESSourcePrivate_t *instance = _context;
    if (instance == 0) {
        instance = new PushESSourcePrivate_t();
        if (instance == 0)
            return false;
        _context = instance;
    }

    // Continue initialisation as far as we can with the current state
    // Some fields only get set when we start playback, so later initialisation only occurs then
    SourceConnector *connector = getConnector();
    BME_DEBUG_TRACE(("%s: Initialising from %d\n", __FUNCTION__, instance->initialised));
    switch (instance->initialised) // Intentional drop-throughs
    {
    case 0:
        BKNI_CreateEvent(&instance->event.stateChange);
        BKNI_CreateMutex(&instance->drmLock);
        BKNI_CreateEvent(&instance->event.audioUnderflow);
        instance->stcMode = true;
        ++instance->initialised;

    case 1:
        if (basic)
            break;

        BME_CHECK(!instance->audio.initialise(instance->audio.secure, AudioFifoBytes));
        ++instance->initialised;

    case 2:
        BME_CHECK(!instance->video.initialise(instance->video.secure, VideoFifoBytes));
        ++instance->initialised;

    case 3:
        if (connector == 0 || (connector->audioDecoder == 0 && connector->videoDecoder == 0)) {
            BME_DEBUG_TRACE(("%s: Basic init done to %u\n", __FUNCTION__, instance->initialised));
            break; // Codecs not yet provided by the player
        }

        BME_DEBUG_TRACE(("audio PID: %2x Decoder %p Codec %d",
            instance->pid.audio, connector->audioDecoder, (int) instance->codec.audio));
        if (instance->codec.audio != NEXUS_AudioCodec_eUnknown && connector->audioDecoder) {
            NEXUS_SimpleAudioDecoderSettings Settings;
            NEXUS_SimpleAudioDecoder_GetSettings(connector->audioDecoder, &Settings);
            SetCallback(&Settings.primary.fifoUnderflow, staticAudioDataExhaustedCallback, this);
            SetCallback(&Settings.primary.firstPts,      staticAudioFirstPts,              this);
            SetCallback(&Settings.primary.sourceChanged, staticAudioSourceChanged,         this);
            NEXUS_SimpleAudioDecoder_SetSettings(connector->audioDecoder, &Settings);

            {
                NxClient_AudioProcessingSettings settings;
                NxClient_GetAudioProcessingSettings(&settings);
                settings.advancedTsm.mode = NEXUS_AudioAdvancedTsmMode_eOff;
                NxClient_SetAudioProcessingSettings(&settings);
            }

            NEXUS_PlaypumpOpenPidChannelSettings PidSettings;
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&PidSettings);
            PidSettings.pidType                     = NEXUS_PidType_eAudio;
            PidSettings.pidTypeSettings.audio.codec = instance->codec.audio;
            instance->pid.channel.audio = NEXUS_Playpump_OpenPidChannel(
                instance->audio.pump.handle, instance->pid.audio, &PidSettings);
            if (instance->pid.channel.audio == 0) {
                BME_DEBUG_ERROR(("Unable to Open playpump audio pid channel"));
                BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
            }
#if defined(BRCM_SAGE)
            if (instance->audio.secure)
                NEXUS_SetPidChannelBypassKeyslot(instance->pid.channel.audio, NEXUS_BypassKeySlot_eGR2R);
#endif
        }
        ++instance->initialised;

    case 4:
        BME_DEBUG_TRACE(("video PID: %2x Decoder %p Codec %d",
            instance->pid.video, connector->videoDecoder, (int) instance->codec.video));
        if (instance->codec.video != NEXUS_VideoCodec_eNone && instance->pid.video != 0) {
            NEXUS_PlaypumpOpenPidChannelSettings PidSettings;
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&PidSettings);
            PidSettings.pidType = NEXUS_PidType_eVideo;
            instance->pid.channel.video = NEXUS_Playpump_OpenPidChannel(
                instance->video.pump.handle, instance->pid.video, &PidSettings);
            if (instance->pid.channel.video == 0) {
                BME_DEBUG_ERROR(("Unable to Open playpump video pid channel"));
                BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
            }
#if defined(BRCM_SAGE)
            // The keyslot settings doesn't reset when playpump is closed so
            // must explicity set it. Otherwise we can switch from non-svp to svp
            if (instance->video.secure)
                NEXUS_SetPidChannelBypassKeyslot(instance->pid.channel.video, NEXUS_BypassKeySlot_eGR2R);
            else
                NEXUS_SetPidChannelBypassKeyslot(instance->pid.channel.video, NEXUS_BypassKeySlot_eG2GR);
#endif

            NEXUS_VideoDecoderSettings Settings;
            NEXUS_SimpleVideoDecoder_GetSettings(connector->videoDecoder, &Settings);
            SetCallback(&Settings.sourceChanged,  staticVideoSourceCallback,         this);
            SetCallback(&Settings.fifoEmpty,      staticVideoDataExhaustedCallback,  this);
            SetCallback(&Settings.firstPts,       staticVideoFirstPts,               this);
            SetCallback(&Settings.firstPtsPassed, staticVideoFirstPtsPassedCallback, this);
            Settings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
            if (instance->codec.video == NEXUS_VideoCodec_eVp9) {
                Settings.maxWidth = 3840;
                Settings.maxHeight = 2160;
            }

            NEXUS_SimpleVideoDecoder_SetSettings(connector->videoDecoder, &Settings);

            NEXUS_VideoDecoderExtendedSettings ExtendedSettings;
            NEXUS_SimpleVideoDecoder_GetExtendedSettings(connector->videoDecoder, &ExtendedSettings);
            SetCallback(&ExtendedSettings.dataReadyCallback, staticVideoPtsCallback, this);
            NEXUS_SimpleVideoDecoder_SetExtendedSettings(connector->videoDecoder, &ExtendedSettings);
        }
        ++instance->initialised;

    case 5:
#if 0
        if (instance->pid.pcr != instance->pid.audio && instance->pid.pcr != instance->pid.video && audio) {
            NEXUS_PlaypumpOpenPidChannelSettings Settings;
            NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&Settings);
            Settings.pidType = NEXUS_PidType_eUnknown;
            instance->pid.channel.pcr = NEXUS_Playpump_OpenPidChannel(instance->audio.pump.handle, instance->pid.pcr, &Settings);
            if (instance->pid.channel.pcr == 0) {
                BME_DEBUG_ERROR(("Unable to Open playpump prc pid channel"));
                BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
            }
        }
#endif
        ++instance->initialised;

    case 6:
        if (connector->stcChannel != 0) {
            NEXUS_SimpleStcChannelSettings Settings;
            NEXUS_SimpleStcChannel_GetSettings(connector->stcChannel, &Settings);
            if (instance->stcMode) {
                Settings.mode                            = NEXUS_StcChannelMode_eAuto;
                Settings.modeSettings.Auto.transportType = NEXUS_TransportType_eMpeg2Pes;
                Settings.sync                            = NEXUS_SimpleStcChannelSyncMode_eOff;
            } else {
                Settings.mode                            = NEXUS_StcChannelMode_eHost;
                Settings.modeSettings.host.transportType = NEXUS_TransportType_eMpeg2Pes;
            }
            BME_CHECK(NEXUS_SimpleStcChannel_SetSettings(connector->stcChannel, &Settings));
        }
        ++instance->initialised;

        BME_DEBUG_TRACE(("%s: PID channels: v %p a %p\n",
            __FUNCTION__, instance->pid.channel.video, instance->pid.channel.audio));
        connector->videoPidChannel = instance->pid.channel.video;
        connector->audioPidChannel = instance->pid.channel.audio;
        connector->pcrPidChannel   = 0; //instance->pid.channel.pcr;

    case 7:
        break;

    default:
        BME_DEBUG_ERROR(("Invalid initialisation state %d", instance->initialised));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        return false;
    }
    BME_DEBUG_TRACE(("%s: initialised to %u\n", __FUNCTION__, instance->initialised));
    return true;
}

void PushESSource::swapAudioCodecs(IMedia::AudioCodec codec)
{
    swapAudioCodecs(convertAudioCodec(codec));
}

void PushESSource::swapAudioCodecs(NEXUS_AudioCodec codec)
{
    PushESSourcePrivate_t *instance = _context;
    assert(codec != NEXUS_AudioCodec_eUnknown);

    // Wait for the current audio codec stream to complete
    // processing (almost - we may drop some audio)
    SourceConnector *connector = getConnector();
    NEXUS_SimpleAudioDecoderHandle decoder = connector->audioDecoder;
    while (_state == IMedia::StartedState) {
        NEXUS_AudioDecoderStatus status;
        NEXUS_SimpleAudioDecoder_GetStatus(decoder, &status);
        if (status.queuedFrames == 0)
            break;
        BME_DEBUG_TRACE(("%s: frames %u depth %6u size %6u state %d\n",
            __FUNCTION__, status.queuedFrames, status.fifoDepth, status.fifoSize, (int) _state));
        BKNI_WaitForEvent(instance->event.audioUnderflow, BKNI_INFINITE);
    }

    // Stop and flush the audio decoder
    NEXUS_SimpleAudioDecoder_Stop(decoder);
    NEXUS_SimpleAudioDecoder_Flush(decoder);

    // Reconfigure for the next codec type and restart the audio decoder
    NEXUS_SimpleAudioDecoderStartSettings Settings;
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&Settings);
    Settings.primary.codec      = codec;
    Settings.primary.pidChannel = connector->audioPidChannel;
    Settings.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eStandalone;
    Settings.master             = true;
    BME_CHECK(NEXUS_SimpleAudioDecoder_Start(decoder, &Settings));
    instance->codec.audio = codec;
}


// ===================================================================

void PushESSource::deinitialise()
{
    PushESSourcePrivate_t *instance = _context;
    if (instance == 0)
        return;

    BME_DEBUG_TRACE(("%s(%p): initialised %u\n", __FUNCTION__, instance, instance->initialised));
    resetPlayState();
    if (_onCompletionThread.joinable())
        _onCompletionThread.join();

    // Signal that we're deinitialising
    _state = IMedia::IdleState;

    switch (instance->initialised)
    {
    case 7: // Fully initialised
        // Unblock any potential audio codec change
        BKNI_SetEvent(instance->event.audioUnderflow);

        // Wait for data pushes to complete
        // Order must be audio, video to prevent deadlock
        instance->audio.pump.acquireLock();
        instance->video.pump.acquireLock();

        // Nothing to clean-up (NEXUS_SimpleStcChannel_SetSettings)
    case 6:
        // Nothing to clean-up (PCR, awaiting removal of this step)
    case 5:
        // Always put keyslot baack to G2GR for the next app, in case they assume default
#if defined(BRCM_SAGE)
        NEXUS_SetPidChannelBypassKeyslot(instance->pid.channel.video, NEXUS_BypassKeySlot_eG2GR);
#endif
        if (instance->codec.video != NEXUS_VideoCodec_eNone && instance->pid.video != 0)
            NEXUS_Playpump_ClosePidChannel(instance->video.pump.handle, instance->pid.channel.video);
    case 4:
        if (instance->codec.audio != NEXUS_AudioCodec_eUnknown && instance->pid.audio != 0)
            NEXUS_Playpump_ClosePidChannel(instance->audio.pump.handle, instance->pid.channel.audio);
    case 3:
        instance->video.deinitialise();
    case 2:
        instance->audio.deinitialise();
        if (instance->wavHeader)
            delete instance->wavHeader;
    case 1:
        BKNI_DestroyEvent(instance->event.stateChange);
        BKNI_DestroyEvent(instance->event.audioUnderflow);
        BKNI_DestroyMutex(instance->drmLock);
    default:
    case 0:
        break;
    }
    instance->initialised = 0;
}

bool PushESSource::isRunning()
{
    return getState() == IMedia::StartedState || getState() == IMedia::PausedState;
}

void PushESSource::staticErrorCallback(void *context, int parameter)
{
    (void) parameter;
    reinterpret_cast<PushESSource *>(context)->onError(IMedia::MEDIA_ERROR_UNKNOWN);
}

void PushESSource::staticVideoDataExhaustedCallback(void *context, int parameter)
{
    (void) parameter;
    reinterpret_cast<PushESSource *>(context)->onInfo(IMedia::MEDIA_INFO_DATA_EXHAUSTED, IMedia::VideoTrackType);
}

void PushESSource::staticVideoPtsCallback(void *context, int parameter)
{
    (void) parameter;
    PushESSource *instance = reinterpret_cast<PushESSource *>(context);
    (void) instance->getCurrentStc();
    instance->onInfo(IMedia::MEDIA_INFO_VIDEO3D_PTS_CALLBACK);
}

void PushESSource::staticVideoSourceCallback(void *context, int parameter)
{
    PushESSource *instance = reinterpret_cast<PushESSource *>(context);
    if (instance->getConnector()->videoDecoder == 0)
        return;

    NEXUS_VideoDecoderStatus Status;
    BME_CHECK(NEXUS_SimpleVideoDecoder_GetStatus(instance->getConnector()->videoDecoder,
                         &Status));

    if (Status.started && Status.source.width > 0 && Status.source.height > 0) {
        unsigned int width  = Status.source.width;
        unsigned int height = Status.source.height;
        BME_DEBUG_TRACE(("videoSourceCallback: video %ux%u", width, height));
        instance->onVideoSizeChanged(width, height);
    }

    NEXUS_VideoDecoderExtendedStatus ExtendedStatus;
    BME_CHECK(NEXUS_SimpleVideoDecoder_GetExtendedStatus(
        instance->getConnector()->videoDecoder, &ExtendedStatus));
    if (ExtendedStatus.lastPictureFlag != 0) {
        BME_DEBUG_TRACE(("eopThreadProc: trigger onCompletion"));
        BME_DEBUG_TRACE(("\n"
                "  Decoded            %6u\n"
                "  Displayed          %6u\n"
                "  IFramesDisplayed   %6u\n"
                "  Decode Error       %6u\n"
                "  Decode Overflows   %6u\n"
                "  Display Errors     %6u\n"
                "  Decode Drops       %6u\n"
                "  Pictures rxed      %6u\n"
                "  Display Drops      %6u\n"
                "  Display Underflows %6u\n"
                "  PTS Errors         %6u\n"
                "  FIFO Empty         %6u\n",
                Status.numDecoded,
                Status.numDisplayed,
                Status.numIFramesDisplayed,
                Status.numDecodeErrors,
                Status.numDecodeOverflows,
                Status.numDisplayErrors,
                Status.numDecodeDrops,
                Status.numPicturesReceived,
                Status.numDisplayDrops,
                Status.numDisplayUnderflows,
                Status.ptsErrorCount,
                Status.fifoEmptyEvents));

        instance->_onCompletionThread = std::thread(triggerOnCompletion, context);
    }
}

namespace
{
unsigned int greatestCommonDivisor(unsigned int x, unsigned int y)
{
    return y == 0 ? x : greatestCommonDivisor(y, x % y);
}
}

void PushESSource::stcRate(unsigned int rate)
{
    unsigned int prescale  = 250; // We only have 8b for this register setting
    unsigned int divisor   = NEXUS_NORMAL_PLAY_SPEED / prescale;
    unsigned int increment = (rate + divisor/2) / divisor; // Fair rounding

    // Remove the greatest common divisor from increment / prescale
    unsigned int gcd = increment > prescale ?
        greatestCommonDivisor(increment, prescale) : greatestCommonDivisor(prescale, increment);
    increment /= gcd;
    prescale  /= gcd;
    BME_CHECK(NEXUS_SimpleStcChannel_SetRate(getConnector()->stcChannel, increment, prescale-1));
}

unsigned int PushESSource::setAudioRate(unsigned int rate, bool mute)
{
    NEXUS_SimpleAudioDecoderHandle audio = getConnector()->audioDecoder;
    assert(audio != 0);
    NEXUS_AudioDecoderTrickState audioState;
    NEXUS_SimpleAudioDecoder_GetTrickState(audio, &audioState);
    unsigned int oldRate = audioState.rate;
    audioState.rate = rate;
    audioState.muted = mute;
    BME_CHECK(NEXUS_SimpleAudioDecoder_SetTrickState(audio, &audioState));
    return oldRate;
}

unsigned int PushESSource::setVideoRate(unsigned int rate)
{
    NEXUS_SimpleVideoDecoderHandle video = getConnector()->videoDecoder;
    NEXUS_VideoDecoderTrickState videoState;
    NEXUS_SimpleVideoDecoder_GetTrickState(video, &videoState);
    unsigned int oldRate = videoState.rate;
    videoState.rate = rate;
    videoState.tsmEnabled = NEXUS_TsmMode_eEnabled;
    BME_CHECK(NEXUS_SimpleVideoDecoder_SetTrickState(video, &videoState));
    return oldRate;
}

void PushESSource::setRates(unsigned int audioRate, unsigned int videoRate)
{
    unsigned int rates[2];
    if (getConnector()->audioDecoder)
        rates[0] = setAudioRate(audioRate, audioRate == 0);
    if (getConnector()->videoDecoder)
        rates[1] = setVideoRate(videoRate);
    BME_DEBUG_TRACE(("%s: ar %4u (%4u) vr %4u (%4u)\n", __FUNCTION__,
                audioRate, rates[0], videoRate, rates[1]));
}

void PushESSource::audioWhen(const char *Function)
{
#ifdef BME_VERBOSE
    unsigned int pts = 0;
    NEXUS_AudioDecoderStatus status;
    if (isRunning()) {
        NEXUS_SimpleAudioDecoderHandle audioDecoder = getConnector()->audioDecoder;
        if (audioDecoder == 0)
            return;
        NEXUS_SimpleAudioDecoder_GetStatus(audioDecoder, &status);
        pts = status.pts;
    }

    unsigned int Stc = getCurrentPosition();
    BME_DEBUG_TRACE(("%s: stc %6u ms pts %6u ms\n", Function, Stc / 45, pts / 45));
#endif
}

void PushESSource::staticAudioDataExhaustedCallback(void *context, int parameter)
{
    PushESSource *instance = reinterpret_cast<PushESSource *>(context);
    (void) parameter;
    instance->audioWhen(__FUNCTION__);
    BKNI_SetEvent(instance->_context->event.audioUnderflow);
    instance->onInfo(IMedia::MEDIA_INFO_DATA_EXHAUSTED, IMedia::AudioTrackType);
}

void PushESSource::staticAudioFirstPts(void *context, int parameter)
{
    (void) parameter;
    PushESSource *instance = reinterpret_cast<PushESSource *>(context);
    instance->audioWhen(__FUNCTION__);
}

void PushESSource::staticAudioSourceChanged(void *context, int parameter)
{
    (void) parameter;
    PushESSource *instance = reinterpret_cast<PushESSource *>(context);
    instance->audioWhen(__FUNCTION__);
    instance->onInfo(IMedia::MEDIA_INFO_METADATA_UPDATE);
}

void PushESSource::videoWhen(const char *Function)
{
#ifdef BME_VERBOSE
    unsigned int Stc = getCurrentPosition();
    NEXUS_VideoDecoderStatus status;
    (void) getVideoStatus(status);
    BME_DEBUG_TRACE(("%s: stc %6u ms pts %6u ms\n", Function, Stc / 45, status.pts / 45));
#endif
}

// The order of FirstPts and FirstPtsPassed callbacks is not guaranteed
void PushESSource::staticVideoFirstPts(void *context, int parameter)
{
    PushESSource *instance = reinterpret_cast<PushESSource *>(context);
    instance->videoWhen(__FUNCTION__);
    instance->onInfo(IMedia::MEDIA_INFO_FIRST_PTS);
}

void PushESSource::staticVideoFirstPtsPassedCallback(void *context, int parameter)
{
    PushESSource *instance = reinterpret_cast<PushESSource *>(context);
    instance->videoWhen(__FUNCTION__);
}

void PushESSource::resetPlayState()
{
    _lastStc = 0;
}

void PushESSource::start()
{
    if (_state == IMedia::PausedState) {
        resumeFromPause();
        return;
    }

    initialise();
    _lastStc = 0;

    BME_DEBUG_TRACE(("%s(%p):%d: PIDs: a %p v %p\n", __FUNCTION__, this, __LINE__,
    getConnector()->audioPidChannel, getConnector()->videoPidChannel));
    stcRate(0); // We start() and then pause() to initialise. A bit of a hack ..
    setRates(NEXUS_NORMAL_PLAY_SPEED, NEXUS_NORMAL_PLAY_SPEED);
    _state = IMedia::StartedState;
}

void PushESSource::stop(bool holdPicture)
{
    if (!isRunning() || _state == IMedia::StoppedState)
        return;

    _state = IMedia::StoppedState;

    _videoPaused = holdPicture;
    resetPlayState();
}

void PushESSource::pause()
{
    if (_state != IMedia::StartedState)
        return;

    stcRate(0);
    _state = IMedia::PausedState;
}

void PushESSource::resumeFromPause()
{
    if (_state != IMedia::PausedState)
        return;

    stcRate(NEXUS_NORMAL_PLAY_SPEED);
    _state = IMedia::StartedState;
}

static void prepareAsyncThread(void *data)
{
    PushESSource *source = reinterpret_cast<PushESSource*>(data);
    // make sure state didn't change while we are putting in fixed delay
    NEXUS_Error rc = BKNI_WaitForEvent(source->getContext()->event.stateChange, 100);
    if (rc == BERR_TIMEOUT)
        source->onPrepared();
}

IMedia::ErrorType PushESSource::prepare()
{
    onPrepared();
    return IMedia::MEDIA_SUCCESS;
}

void PushESSource::prepareAsync()
{
    BKNI_ResetEvent(_context->event.stateChange);
    BME_DEBUG_TRACE(("create prepareAsyncThread, is this expected?"));
    _state = IMedia::PreparingState;
    _prepareAsyncThread = std::thread(&prepareAsyncThread,
            static_cast<void*>(this));
}

void PushESSource::setAudioPrivateHeader(const uint8_t *data, const size_t size)
{
    PushESSourcePrivate_t *instance = _context;

    if (data == NULL || size == 0) {
        BME_DEBUG_ERROR(("Invalid audio extra data"));
        return;
    }

    if (instance->codec.audio != NEXUS_AudioCodec_eVorbis &&
         instance->codec.audio != NEXUS_AudioCodec_eOpus) {
        BME_DEBUG_ERROR(("Invalid audio codec to set private header"));
        return;
    }
    if (!instance->wavHeader) {
        BME_DEBUG_ERROR(("Invalid wav format header"));
    }
    instance->wavHeader->AddExtendHeader(data, size);
}

void PushESSource::setDataSource(MediaStream *mediaStream)
{
    IMedia::StreamMetadata md = mediaStream->metadata;
    if (md.streamType == IMedia::UnknownStreamType) {
        BME_DEBUG_TRACE(("%s: mS %p Unknown stream type (%d)", __FUNCTION__,
                    mediaStream, md.streamType));
        return;
    }

    PushESSourcePrivate_t *instance = _context;
    instance->audio.secure = mediaStream->isAudioSecure();
    instance->video.secure = mediaStream->isVideoSecure();
    instance->stcMode      = mediaStream->getStcMode();
    IMedia::VideoParameters videoParam = md.videoParam;
    IMedia::AudioParameters audioParam;
    BME_DEBUG_TRACE(("%s: mS %p Codecs: v %d a %d",
        __FUNCTION__, mediaStream, (int) videoParam.videoCodec, (int) audioParam.audioCodec));
    if (md.audioParamList.size() > 0) {
        IMedia::AudioParameters audioParam = md.audioParamList[0];
        instance->wavHeader = new WavFormatHeader(audioParam);
        if (!instance->wavHeader) {
            BME_DEBUG_ERROR(("%s: failed to create wav format header", __FUNCTION__));
            return;
        }
        instance->codec.audio            = convertAudioCodec(audioParam.audioCodec);
        instance->pid.audio              = audioParam.streamId;
        instance->audio.pump.setPid(instance->pid.audio);

        instance->audioParameters.channels   = audioParam.numChannels;
        instance->audioParameters.sampleRate = audioParam.samplesPerSecond;
    }
    instance->codec.video            = convertVideoCodec(videoParam.videoCodec);
    instance->pid.video              = videoParam.streamId;
    instance->video.pump.setPid(instance->pid.video);
    instance->pid.pcr                = videoParam.substreamId;

    instance->transport              = convertStreamType(md.streamType);
    instance->colorDepth             = videoParam.colorDepth;
    instance->videoDimensions.width  = videoParam.maxWidth;
    instance->videoDimensions.height = videoParam.maxHeight;
}

void PushESSource::reset()
{
    deinitialise();
}

void PushESSource::release()
{
    deinitialise();
}

bool PushESSource::checkUrlSupport(const std::string& url)
{
    return url.find(IMedia::PUSH_ES_URI_PREFIX) != std::string::npos;
}

SourceConnector *PushESSource::getConnector() const
{
    return _connector;
}

uint32_t PushESSource::setConnector(SourceConnector* connector)
{
    this->_connector = connector;
    return 0;
}

std::string PushESSource::getType()
{
    return SOURCE_PUSHES;
}

IMedia::StreamMetadata PushESSource::getStreamMetadata()
{
    IMedia::StreamMetadata Metadata;

    IMedia::VideoParameters videoParameters =
    {
        _context->pid.video,
        _context->pid.pcr,
        (IMedia::VideoCodec) _context->codec.video,
        IMedia::UnknownVideoCodec,
        (uint16_t) _context->videoDimensions.width,
        (uint16_t) _context->videoDimensions.height,
        _context->colorDepth, IMedia::VideoAspectRatioUnknown
    };

    IMedia::AudioParameters audioParam =
    {
        _context->pid.audio,
        0,
        (IMedia::AudioCodec) _context->codec.audio,
        0, 0, 0
    };

    Metadata.streamType = (IMedia::StreamType) NEXUS_TransportType_eMpeg2Pes;
    Metadata.videoParam = videoParameters;
    Metadata.audioParamList.push_back(audioParam);
    return Metadata;
}

IMedia::ErrorType PushESSource::seekTo(const uint32_t &milliseconds,
                                                    IMedia::PlaybackOperation playOperation,
                                                    IMedia::PlaybackSeekTime  playSeekTime)
{
    TIME45k target;
    switch (playSeekTime)
    {
    case IMedia::SeekTimeAbsolute:
        target = 45ULL * (uint64_t) milliseconds;
        break;

    case IMedia::SeekTimeRelative: // Odd with an unsigned input
        {
            TIME45k Base = getCurrentStc();
            target = Base + 45ULL * (uint64_t) milliseconds;
            break;
        }

    default:
        target = ~0ULL;
        break;
    }
    if (isRunning())
        seekToPts(target);
    return IMedia::MEDIA_SUCCESS;
}

void PushESSource::seekToPts(TIME45k pts)
{
    // Pause for seek (rates, not clock)
    setRates(0, 0);

    // Flush decoder first before setting decoder's start pts.
    flush(false);

    uint32_t seek32 = (uint32_t) pts;

    BME_DEBUG_TRACE(("%s Seeking to %u ms (%d)\n", __FUNCTION__, seek32 / 45, seek32));
    NEXUS_SimpleVideoDecoderHandle video = getConnector()->videoDecoder;
    BME_CHECK(NEXUS_SimpleVideoDecoder_SetStartPts(video, seek32));
    // No equivalent for the audio decoder

    // Unpause the streams
    // Note: This is the rates, not the clock
    setRates(NEXUS_NORMAL_PLAY_SPEED, NEXUS_NORMAL_PLAY_SPEED);

    // Start the clock at the seek position in the beginning only or if we are
    // in eHost stc mode
    if (!_context->stcMode || pts == 0)
        NEXUS_SimpleStcChannel_SetStc(getConnector()->stcChannel, seek32);
}

void PushESSource::setPlaybackRate(const std::string& rate)
{
    setPlaybackRate(rate, false);
}

void PushESSource::setPlaybackRate(const std::string& rate, bool mute)
{
    int playbackRate = std::stof(rate, NULL) * NEXUS_NORMAL_PLAY_SPEED;
    BME_DEBUG_TRACE(("+++setSpeed(%d)\n", playbackRate));

    if (!isRunning())
        return;

    if (getConnector()->audioDecoder) {
        NEXUS_AudioDecoderTrickState audioState;
        NEXUS_SimpleAudioDecoder_GetTrickState(getConnector()->audioDecoder, &audioState);
        audioState.rate  = playbackRate;
        audioState.muted = mute;
        BME_CHECK(
            NEXUS_SimpleAudioDecoder_SetTrickState(getConnector()->audioDecoder, &audioState));
    }

    stcRate(playbackRate);

    if (getConnector()->videoDecoder) {
        NEXUS_VideoDecoderTrickState videoState;

        NEXUS_SimpleVideoDecoder_GetTrickState(getConnector()->videoDecoder, &videoState);
        videoState.rate = playbackRate;
        videoState.tsmEnabled = NEXUS_TsmMode_eEnabled;
        BME_CHECK(
            NEXUS_SimpleVideoDecoder_SetTrickState(getConnector()->videoDecoder, &videoState));
    }

    if (playbackRate == PLAYBACK_RATE_PAUSED) {
        _state = IMedia::PausedState;
    } else {
        if (_state == IMedia::PausedState)
            _state = IMedia::StartedState;
    }
    _playbackRate = playbackRate;
}

int PushESSource::getPlaybackRate()
{
    return _playbackRate;
}

IMedia::PlaybackOperation PushESSource::getPlaybackOperation()
{
    return IMedia::OperationForward;
}

uint32_t PushESSource::getCurrentPosition()
{
    return (uint32_t) getCurrentStc();
}

uint64_t PushESSource::getBytesDecoded()
{
    return 0;
}

const IMedia::TimeInfo PushESSource::getTimeInfo()
{
    IMedia::TimeInfo timeinfo;
    return timeinfo;
}

TIME45k PushESSource::getCurrentStc()
{
    if (!isRunning())
        return _lastStc;

    uint32_t stc;
    NEXUS_SimpleStcChannel_GetStc(getConnector()->stcChannel, &stc);
    return _lastStc = ((uint64_t) _ptsHi << 32) | stc;
}

bool PushESSource::getCurrentVideoPts(TIME45k* pts)
{
    NEXUS_Error rc;

    if (getConnector()->videoDecoder) {
        NEXUS_VideoDecoderStatus status;

        rc = NEXUS_SimpleVideoDecoder_GetStatus(getConnector()->videoDecoder, &status);
        if (rc == NEXUS_SUCCESS && status.ptsType != NEXUS_PtsType_eInterpolatedFromInvalidPTS) {
            *pts = status.pts;
            return true;
        }
    }
    return false;
}

bool PushESSource::getCurrentAudioPts(TIME45k* pts)
{
    NEXUS_Error rc;

    if (getConnector()->audioDecoder) {
        NEXUS_AudioDecoderStatus status;

        rc = NEXUS_SimpleAudioDecoder_GetStatus(getConnector()->audioDecoder, &status);
        if (rc == NEXUS_SUCCESS && status.ptsType != NEXUS_PtsType_eInterpolatedFromInvalidPTS) {
            *pts = status.pts;
            return true;
        }
    }
    return false;
}

void PushESSource::onPrepared()
{
    _state = IMedia::StoppedState;
    notify(SourceEvents::Prepared);
}

void PushESSource::onError(const IMedia::ErrorType& errorType)
{
    if (isRunning())
        notify(SourceEvents::Error, errorType);
}

void PushESSource::onSeekComplete()
{
    if (isRunning())
        notify(SourceEvents::SeekCompleted);
}

void PushESSource::onInfo(const IMedia::InfoType& infoType, int32_t extra)
{
    if (isRunning())
        notify(SourceEvents::Info, infoType, extra);
}

void PushESSource::onCompletion()
{
    if (isRunning())
        notify(SourceEvents::Completed);
}

void PushESSource::onVideoSizeChanged(uint16_t width, uint16_t height)
{
    if (isRunning())
        notify(SourceEvents::VideoSizeChanged, width, height);
}

bool PushESSource::getVideoStatus(NEXUS_VideoDecoderStatus &status)
{
    if (!isRunning())
        return false;

    NEXUS_SimpleVideoDecoderHandle videoDecoder = getConnector()->videoDecoder;
    if (videoDecoder == 0)
        return false;

    NEXUS_SimpleVideoDecoder_GetStatus(videoDecoder, &status);

    return true;
}

uint32_t PushESSource::getDisplayedFrameCount()
{
    NEXUS_VideoDecoderStatus status;
    if (!getVideoStatus(status))
        return -1;
    return (int) status.numDisplayed;
}

uint32_t PushESSource::getDropFrameCount()
{
    NEXUS_VideoDecoderStatus status;
    if (!getVideoStatus(status))
        return -1;

    BME_DEBUG_TRACE(("%s: Decode %u Display %u\n", __FUNCTION__,
                status.numDecodeDrops, status.numDisplayDrops));

    return (int) (status.numDecodeDrops + status.numDisplayDrops);
}

uint32_t PushESSource::getVideoDecodedFrameCount()
{
    NEXUS_VideoDecoderStatus status;
    if (!getVideoStatus(status))
        return -1;
    return (int) status.numDecoded;
}

uint32_t PushESSource::getAudioDecodedFrameCount()
{
    if (!isRunning())
        return -1;

    NEXUS_SimpleAudioDecoderHandle decoder = getConnector()->audioDecoder;
    NEXUS_AudioDecoderStatus status;

    if (!decoder)
        return -1;

    if (NEXUS_SimpleAudioDecoder_GetStatus(decoder, &status) != NEXUS_SUCCESS)
        return -1;

    return (int) status.framesDecoded;
}

void PushESSource::flush(bool holdPicture)
{
    PushESSourcePrivate_t *instance = _context;
    TRLS_UNUSED(holdPicture);
    BME_DEBUG_TRACE(("%s\n", __FUNCTION__));

    if (!isRunning())
        return;

    resetPlayState();

    // Order must be audio, video to prevent deadlock
    instance->audio.pump.acquireLock();
    instance->video.pump.acquireLock();

    if (instance->video.pump.handle)
        BME_CHECK(NEXUS_Playpump_Flush(instance->video.pump.handle));
    if (instance->audio.pump.handle)
        BME_CHECK(NEXUS_Playpump_Flush(instance->audio.pump.handle));

    if (getConnector()->videoDecoder != 0)
        NEXUS_SimpleVideoDecoder_Flush(getConnector()->videoDecoder);
    if (getConnector()->audioDecoder != 0)
        NEXUS_SimpleAudioDecoder_Flush(getConnector()->audioDecoder);

    BKNI_ReleaseMutex(instance->video.pump.lock);
    BKNI_ReleaseMutex(instance->audio.pump.lock);
}

void PushESSource::flushAudio()
{
    PushESSourcePrivate_t *instance = _context;
    BME_DEBUG_TRACE(("%s\n", __FUNCTION__));

    instance->audio.pump.acquireLock();

    if (instance->audio.pump.handle)
        BME_CHECK(NEXUS_Playpump_Flush(instance->audio.pump.handle));

    if (getConnector()->audioDecoder != 0)
        NEXUS_SimpleAudioDecoder_Flush(getConnector()->audioDecoder);

    BKNI_ReleaseMutex(instance->audio.pump.lock);
}

void PushESSource::flushVideo()
{
    PushESSourcePrivate_t *instance = _context;
    BME_DEBUG_TRACE(("%s\n", __FUNCTION__));

    instance->video.pump.acquireLock();

    if (instance->video.pump.handle)
        BME_CHECK(NEXUS_Playpump_Flush(instance->video.pump.handle));

    if (getConnector()->videoDecoder != 0)
        NEXUS_SimpleVideoDecoder_Flush(getConnector()->videoDecoder);

    BKNI_ReleaseMutex(instance->video.pump.lock);
}

void PushESSource::triggerOnCompletion(void *context)
{
    reinterpret_cast<PushESSource *>(context)->onCompletion();
}

void PushESSource::pushAudioEndOfStream()
{
    PushESSourcePrivate_t *instance = _context;

    if (instance->transport != NEXUS_TransportType_eEs)
        return;

    Playpump_t *pump = &instance->audio.pump;
    DataFragment_t last(pump->sample.last, sizeof(pump->sample.last));
    pump->sample.clear();
    pump->sample.fragmentList.push_back(last);
    pump->sample.bytes = sizeof(pump->sample.last);
    pushAudioChunk();
}

void PushESSource::pushVideoEndOfStream()
{
    PushESSourcePrivate_t *instance = _context;
    if (instance->transport != NEXUS_TransportType_eEs)
        return;

    Playpump_t *pump = &instance->video.pump;
    DataFragment_t flush(pump->sample.flush, sizeof(pump->sample.flush));
    DataFragment_t last(pump->sample.last, sizeof(pump->sample.last));
    pump->sample.clear();
    pump->sample.fragmentList.push_back(flush);
    pump->sample.fragmentList.push_back(last);
    pump->sample.fragmentList.push_back(flush);
    pump->sample.bytes = 2*sizeof(pump->sample.flush) + sizeof(pump->sample.last);
    pushVideoChunk();
}


// ===================================================================

void PushESSource::makeVideoChunk(TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    PushESSourcePrivate_t *instance = _context;
    switch (instance->codec.video)
    {
    case NEXUS_VideoCodec_eVp8:
    case NEXUS_VideoCodec_eVp9:
        instance->video.pump.makeVp9Chunk(instance->pid.video, pts, fragment, n);
        break;

    default:
        instance->video.pump.makePesChunk(instance->pid.video, pts, fragment, n, true);
        break;
    }
}

void PushESSource::makeAudioChunk(TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    PushESSourcePrivate_t *instance = _context;
    switch (instance->codec.audio)
    {
    case NEXUS_AudioCodec_eVorbis:
    case NEXUS_AudioCodec_eOpus:
        makeVorbisChunk(pts, fragment, n);
        break;

    case NEXUS_AudioCodec_ePcmWav:
        makePcmChunk(pts, fragment, n);
        break;

    default:
        makePesAudioChunk(pts, fragment, n);
        break;
    }
}


// ============================================================================
void PushESSource::makeVp9Chunk(TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    PushESSourcePrivate_t *instance = _context;
    assert(instance->codec.video == NEXUS_VideoCodec_eVp8 || instance->codec.video == NEXUS_VideoCodec_eVp9);
    instance->video.pump.makeVp9Chunk(instance->pid.video, pts, fragment, n);
}

void PushESSource::makeAdtsChunk(TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    PushESSourcePrivate_t *instance = _context;
    assert(instance->codec.audio == NEXUS_AudioCodec_eAac);
    instance->audio.pump.makeAdtsChunk(
        instance->audioParameters.sampleRate, instance->audioParameters.channels,
        instance->pid.audio, pts, fragment, n);
}

void PushESSource::makePcmChunk(TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    PushESSourcePrivate_t *instance = _context;
    assert(instance->codec.audio == NEXUS_AudioCodec_ePcmWav);
    instance->audio.pump.makePcmChunk(instance->wavHeader, instance->pid.audio, pts, fragment, n);
}

void PushESSource::makeVorbisChunk(TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    PushESSourcePrivate_t *instance = _context;
    assert(instance->codec.audio == NEXUS_AudioCodec_eVorbis || instance->codec.audio == NEXUS_AudioCodec_eOpus);
    instance->audio.pump.makeVorbisChunk(instance->wavHeader, instance->pid.audio, pts, fragment, n);
}

void PushESSource::makePesAudioChunk(TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    PushESSourcePrivate_t *instance = _context;
    instance->audio.pump.makePesChunk(instance->pid.audio, pts, fragment, n, false);
}
void PushESSource::makePesVideoChunk(TIME45k pts, const DataFragment_t *fragment, size_t n)
{
    PushESSourcePrivate_t *instance = _context;
    instance->video.pump.makePesChunk(instance->pid.video, pts, fragment, n, true);
}

bool PushESSource::pushAudioChunk(
    void        *drmContext,
    uint64_t     vector,
    bool         block)
{
    BKNI_AcquireMutex(_context->audio.pump.lock);
    bool sent = _context->audio.pump.pushChunk(drmContext, vector, block);
    BKNI_ReleaseMutex(_context->audio.pump.lock);
    return sent;
}

bool PushESSource::pushVideoChunk(
    void        *drmContext,
    uint64_t     vector,
    bool         block)
{
    BKNI_AcquireMutex(_context->video.pump.lock);
    bool sent = _context->video.pump.pushChunk(drmContext, vector, block);
    BKNI_ReleaseMutex(_context->video.pump.lock);
    return sent;
}


// ============================================================================

Connector PushESSource::connect(const ConnectSettings &settings)
{
    return NULL;
}

void PushESSource::disconnect(const Connector &connector)
{
}
}  // namespace Media
}  // namespace Broadcom
