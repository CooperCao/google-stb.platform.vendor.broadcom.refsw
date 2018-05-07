/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "SimpleDecoderNxclient.h"
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <string>
#include <stack>
#include "nxclient.h"
#include "nexus_surface_client.h"
#include "nexus_platform_features.h"
#include "nexus_surface_compositor.h"

#ifndef SINGLE_PROCESS
#define MAX_VOLUME 10
#define CONVERT_TO_USER_VOL(vol, max_vol) (((vol) - NEXUS_AUDIO_VOLUME_LINEAR_MIN) * (max_vol) \
        / (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN))
#define CONVERT_TO_NEXUS_LINEAR_VOL(vol, max_vol) (((vol) * (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL \
                - NEXUS_AUDIO_VOLUME_LINEAR_MIN) / (max_vol)) + NEXUS_AUDIO_VOLUME_LINEAR_MIN)

#define MAX_MOSAICS 8
#define MAX_HD_MOSAICS 4

namespace Broadcom {
namespace Media {

TRLS_DBG_MODULE(SimpleDecoderNxclient);

SimpleDecoderNxclient::SimpleDecoderMap SimpleDecoderNxclient::_videoDecoderMap;
SimpleDecoderNxclient::SimpleDecoderMap SimpleDecoderNxclient::_audioPlaybackMap;
// start with a large number to avoid loop around bug
uint16_t SimpleDecoderNxclient::_index = 65533;
uint16_t SimpleDecoderNxclient::_mosaicMaster = 0;
std::mutex SimpleDecoderNxclient::_mutex;

typedef struct tagMosaicDecodersContext {
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SurfaceClientHandle videoSurface;
} MosaicDecoderContext;

typedef struct tagSimpleDecoderNxclientContext {
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle surfaceClient;  // This is the container surface
    NEXUS_SurfaceClientHandle videoSurface;  // This is the individual video window
    uint32_t connectId;
    int16_t mosaicCount;  // tracks how many mosaic decoders
    uint16_t maxWidth;
    uint16_t maxHeight;
    uint16_t colorDepth;
    std::stack<MosaicDecoderContext*> mosaicsStack;
    bool isMosaic;
    bool isPip;
} SimpleDecoderNxclientContext;

SimpleDecoderNxclient::SimpleDecoderNxclient()
    : _width(0),
      _height(0),
      _virtualWidth(1280),
      _virtualHeight(720),
      _self(0)
{
}

SimpleDecoderNxclient::~SimpleDecoderNxclient()
{
}

static NxClient_AudioStatus audioStatus;

void SimpleDecoderNxclient::init()
{
    BME_CHECK(NxClient_Join(NULL));
    ++_index;

    if (_index == 0)
        ++_index;

    _self = _index;

    NxClient_GetAudioStatus(&audioStatus);
}

void SimpleDecoderNxclient::uninit()
{
}

uint32_t SimpleDecoderNxclient::allocMosaicMaster(SimpleDecoderNxclientContext* master,
        uint32_t windowId, uint16_t numMosaics)
{
    NEXUS_Error rc;
    int surfaceID;
    NxClient_AllocResults* allocResults = &master->allocResults;
    NxClient_AllocSettings allocSettings;
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = numMosaics;
    if (!_surfaceClientId) {
        allocSettings.surfaceClient = 1;
    }

    rc = NxClient_Alloc(&allocSettings, allocResults);

    if (rc) {
        BME_DEBUG_ERROR(("NxClient_Alloc returned rc=%d\n", rc));
        delete master;
        return DECODER_NOT_AVAILABLE;
    }

    if (!_surfaceClientId) {
        surfaceID = allocResults->surfaceClient[0].id;
        master->surfaceClient = NEXUS_SurfaceClient_Acquire(surfaceID);
        assert(master->surfaceClient);
    } else {
        surfaceID = _surfaceClientId;
    }

    for (int i = numMosaics-1; i > 0; i--) {
        MosaicDecoderContext* mosaics = new MosaicDecoderContext;
        if (!_surfaceClientId) {
            mosaics->videoSurface = NEXUS_SurfaceClient_AcquireVideoWindow(
                    master->surfaceClient, i);
        }
        uint32_t videoId = master->allocResults.simpleVideoDecoder[i].id;
        mosaics->videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(videoId);
        master->mosaicsStack.push(mosaics);
    }

    // We need to allocate all mosaics initially because NxServer needs to reconfigure
    NxClient_ConnectSettings connectSettings;
    NxClient_GetDefaultConnectSettings(&connectSettings);

    for (int i = 0; i < numMosaics; i++) {
        connectSettings.simpleVideoDecoder[i].id = allocResults->simpleVideoDecoder[i].id;
        connectSettings.simpleVideoDecoder[i].surfaceClientId = surfaceID;
        connectSettings.simpleVideoDecoder[i].windowId = windowId + i;
        connectSettings.simpleVideoDecoder[i].decoderCapabilities.maxWidth = master->maxWidth;
        connectSettings.simpleVideoDecoder[i].decoderCapabilities.maxHeight = master->maxHeight;
    }
    rc = NxClient_Connect(&connectSettings, &master->connectId);

    if (rc) {
        BME_DEBUG_ERROR(("NxClient_Connect returned error code %d", rc));
        BME_DEBUG_ERROR(("Could be max width/height exceeded the amount of "
                    "video decoder memory"));
        delete master;
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        return DECODER_NOT_AVAILABLE;
    }

    master->mosaicCount = 0;
    return 0;
}

uint32_t SimpleDecoderNxclient::setupMosaicMode(uint16_t maxWidth, uint16_t maxHeight,
        uint32_t windowId, NEXUS_SimpleVideoDecoderHandle* videoDecoder,
        IMedia::DecoderType decoderType)
{
    BME_DEBUG_ENTER();
    uint32_t rc;
    SimpleDecoderNxclientContext* context = NULL;

    if (_self != _mosaicMaster) {
        context = new SimpleDecoderNxclientContext;
        context->isMosaic = false;
        context->isPip = false;
        context->mosaicCount = 0;
        _videoDecoderMap[_self] = context;
    }

    // The first one becomes the master
    if (_mosaicMaster == 0) {
        _mosaicMaster = _self;
        context->maxHeight = maxHeight;
        context->maxWidth = maxWidth;
        if (decoderType == IMedia::DecoderType::Mosaic_HD)
            rc = allocMosaicMaster(context, windowId, MAX_HD_MOSAICS);
        else
            rc = allocMosaicMaster(context, windowId, MAX_MOSAICS);
        if (rc != 0)
            return rc;

        // zeroth one is saved for itslef
        if (!_surfaceClientId) {
            context->videoSurface = NEXUS_SurfaceClient_AcquireVideoWindow(
                    context->surfaceClient, 0);
        }
        *videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(
                context->allocResults.simpleVideoDecoder[0].id);

        context->isMosaic = true;

    } else {
        SimpleDecoderNxclientContext* master = _videoDecoderMap[_mosaicMaster];
        ++master->mosaicCount;
        context->mosaicCount = master->mosaicCount;
        context->connectId = 0;

        // master already created so just grab one from the stack
        MosaicDecoderContext* mosaics = master->mosaicsStack.top();
        assert(mosaics != NULL);
        master->mosaicsStack.pop();
        context->videoSurface = mosaics->videoSurface;
        *videoDecoder = mosaics->videoDecoder;
        delete mosaics;
        assert(context->videoSurface);
        context->isMosaic = true;
    }
    BME_DEBUG_EXIT();
    return 0;
}

void SimpleDecoderNxclient::connect()
{
    BME_DEBUG_ENTER();
    NxClient_AllocResults* allocResults;
    NxClient_ConnectSettings connectSettings;
    SimpleDecoderNxclientContext* context = _videoDecoderMap[_self];
    if (context && context->isMosaic == false) {
        allocResults = &context->allocResults;
        NxClient_GetDefaultConnectSettings(&connectSettings);
        connectSettings.simpleVideoDecoder[0].id = allocResults->simpleVideoDecoder[0].id;
        if (_surfaceClientId) {
            connectSettings.simpleVideoDecoder[0].surfaceClientId = _surfaceClientId;
        } else {
            connectSettings.simpleVideoDecoder[0].surfaceClientId =
                allocResults->surfaceClient[0].id;
        }
        connectSettings.simpleVideoDecoder[0].windowId = _windowId;
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = context->maxHeight;
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = context->maxWidth;
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.colorDepth = context->colorDepth;
#ifdef BRCM_SAGE
        // For now if sage is defined, then all decoders are assumed to be secured
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.secureVideo = _secureVideo;
#endif
        connectSettings.simpleAudioDecoder.id = allocResults->simpleAudioDecoder.id;
        if (_persistent)
            connectSettings.simpleAudioDecoder.decoderCapabilities.type = NxClient_AudioDecoderType_ePersistent;
        if (context->isPip) {
            BME_DEBUG_TRACE(("Setting up PIP window"));
            connectSettings.simpleVideoDecoder[0].windowCapabilities.type =
                NxClient_VideoWindowType_ePip;
        }
        BME_CHECK(NxClient_Connect(&connectSettings, &context->connectId));
    }
    SimpleDecoderNxclientContext* audioPlaybackContext = _audioPlaybackMap[_self];

    if (audioPlaybackContext) {
        allocResults = &audioPlaybackContext->allocResults;
        NxClient_GetDefaultConnectSettings(&connectSettings);
        connectSettings.simpleAudioPlayback[0].id = allocResults->simpleAudioPlayback[0].id;
        BME_CHECK(NxClient_Connect(&connectSettings, &audioPlaybackContext->connectId));
    }

    assert(context || audioPlaybackContext);
    BME_DEBUG_EXIT();
}

void SimpleDecoderNxclient::disconnect()
{
    BME_DEBUG_ENTER();
    SimpleDecoderNxclientContext* context = _videoDecoderMap[_self];

    if (context && !_mosaicMaster) {
        NxClient_Disconnect(context->connectId);
    }

    SimpleDecoderNxclientContext* audioPlaybackContext = _audioPlaybackMap[_self];

    if (audioPlaybackContext) {
        NxClient_Disconnect(audioPlaybackContext->connectId);
    }

    BME_DEBUG_EXIT();
}

uint32_t SimpleDecoderNxclient::setupRegular(
    bool video, bool audio,
    uint16_t maxWidth, uint16_t maxHeight,
    uint16_t virtualWidth, uint16_t virtualHeight,
    uint16_t colorDepth,
    IMedia::DecoderType decoderType)
{
    NEXUS_Error rc;
    SimpleDecoderNxclientContext* context = new SimpleDecoderNxclientContext;
    context->isMosaic = false;
    context->mosaicCount = 0;
    context->isPip = (decoderType == IMedia::DecoderType::Pip);
    context->surfaceClient = NULL;
    NxClient_AllocResults* allocResults = &context->allocResults;
    NxClient_AllocSettings allocSettings;
    NxClient_GetDefaultAllocSettings(&allocSettings);
    // request both now as NxServer now is managing stc so audio and video needs
    // to be requested together
    if (audio) {
        allocSettings.simpleAudioDecoder = 1;
    }
    if (video) {
        allocSettings.simpleVideoDecoder = 1;
        if (!_surfaceClientId)
            allocSettings.surfaceClient = 1;
    }
    rc = NxClient_Alloc(&allocSettings, allocResults);

    if (rc) {
        delete context;
        rc = BERR_TRACE(rc);
        return DECODER_NOT_AVAILABLE;
    }

    if ((allocResults->simpleVideoDecoder[0].id == 0 && video)  ||
        (allocResults->simpleAudioDecoder.id == 0 && audio)) {
        delete context;
        return DECODER_NOT_AVAILABLE;
    }
    _videoDecoderMap[_self] = context;

    if (!video)
        return DECODER_ACQUIRED;

    if (!_surfaceClientId) {
        NEXUS_SurfaceClientHandle surfaceClient;
        surfaceClient = NEXUS_SurfaceClient_Acquire(context->allocResults.surfaceClient[0].id);

        if (surfaceClient) {
            context->videoSurface = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);
            context->surfaceClient = surfaceClient;
        }

        _windowId = 0;

        NEXUS_SurfaceComposition comp;
        NEXUS_SurfaceRegion virtualDisplay = {virtualWidth, virtualHeight};
        NxClient_GetSurfaceClientComposition(context->allocResults.surfaceClient[0].id, &comp);
        comp.virtualDisplay = virtualDisplay;
        NEXUS_Rect rect = {0, 0, virtualWidth, virtualHeight};
        comp.position = rect;
        if (context->isPip) {
            comp.zorder = 101;
        }
        comp.zorder = 0;
        NxClient_SetSurfaceClientComposition(context->allocResults.surfaceClient[0].id, &comp);
        if (!context->isPip) {
            // initalize to fullscreen
            setLocation(0, 0, virtualWidth, virtualHeight);
        } else {
            setLocation(0, 0, 640, 320);
        }
        _virtualWidth = virtualWidth;
        _virtualHeight = virtualHeight;
    }
    context->maxWidth = maxWidth;
    context->maxHeight = maxHeight;
    context->colorDepth = colorDepth;
    return DECODER_ACQUIRED;
}

void SimpleDecoderNxclient::acquireSimpleDecoders(const acquireParameters& param,
        NEXUS_SimpleVideoDecoderHandle* videoDecoder,
        NEXUS_SimpleAudioDecoderHandle* audioDecoder)
{
    BME_DEBUG_ENTER();
    std::lock_guard<std::mutex> lock(_mutex);
    bool requestVideoDecoder = param.requestVideoDecoder;
    bool requestAudioDecoder = param.requestAudioDecoder;
    uint16_t maxWidth = param.maxWidth;
    uint16_t maxHeight = param.maxHeight;
    uint16_t virtualWidth = param.virtualWidth;
    uint16_t virtualHeight = param.virtualHeight;
    _secureVideo = param.secureVideo;
    _persistent = param.persistent && audioStatus.dolbySupport.mixer;
    IMedia::DecoderType decoderType = (IMedia::DecoderType)param.decoderType;
    NxClient_AllocResults* allocResults;
    uint32_t videoIndex, audioIndex, retval;
    BME_DEBUG_TRACE(("decoderType = %d %d %d\n", decoderType,
                param.surfaceClientId, param.windowId));

    // save this for connect call
    _surfaceClientId = param.surfaceClientId;
    _windowId = param.windowId;

    if (decoderType == IMedia::DecoderType::Mosaic ||
        decoderType == IMedia::DecoderType::Mosaic_HD) {
        setupMosaicMode(maxWidth, maxHeight, _windowId, videoDecoder, decoderType);
        // initializing to full screen causes issues when playing 4 hevc mosaic videos.
        // Hence setting an initial width and height.
        if (!_surfaceClientId)  // only can do it if we have videoSurface
            setLocation(0, 0, 320, 240);
    } else  {
        retval = setupRegular(requestVideoDecoder, requestAudioDecoder, maxWidth,
                maxHeight, virtualWidth, virtualHeight, param.colorDepth, decoderType);
        if (retval == DECODER_ACQUIRED) {
            SimpleDecoderNxclientContext* context = _videoDecoderMap[_self];
            allocResults = &context->allocResults;
            videoIndex = allocResults->simpleVideoDecoder[0].id;
            audioIndex = allocResults->simpleAudioDecoder.id;
        } else  {
            videoDecoder = NULL;
            audioDecoder = NULL;
            BME_DEBUG_EXIT();
            return;
        }
        if (videoIndex)
            *videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(videoIndex);
        if (audioIndex)
            *audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(audioIndex);
    }
    BME_DEBUG_EXIT();
}

#ifdef NEXUS_HAS_VIDEO_ENCODER
SimpleXcodeHandle* SimpleDecoderNxclient::acquireXcodeResource(bool realTime, bool audioXcode,
                                                               int maxWidth, int maxHeight,
                                                               int colorDepth, bool hdmiInput)
{
    BME_DEBUG_ENTER();
    BME_DEBUG_TRACE(("acquire Xcode resource: width: %d, height: %d, colorDepth: %d\n",
                                               maxWidth, maxHeight, colorDepth));
    SimpleXcodeHandle* xcodeHandle;
    NxClient_AllocSettings allocSettings;
    NxClient_ConnectSettings connectSettings;
    NEXUS_Error rc;
    SimpleDecoderNxclientContext* context = new SimpleDecoderNxclientContext;
    context->isMosaic = false;
    context->isPip = false;
    NxClient_AllocResults* allocResults = &context->allocResults;
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = audioXcode;
    allocSettings.simpleEncoder = 1;
    rc = NxClient_Alloc(&allocSettings, allocResults);

    if (rc) {
        delete context;
        rc = BERR_TRACE(rc);
        return NULL;
    }

    if (allocResults->simpleVideoDecoder[0].id == 0 ||
            (audioXcode && allocResults->simpleAudioDecoder.id == 0) ||
            allocResults->simpleEncoder[0].id == 0) {
        delete context;
        return NULL;
    }

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults->simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.encoder = true;
    if (!realTime) {
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.fifoSize = 3 * 1024 * 1024;
    }
    if (hdmiInput) {
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.connectType =
                                            NxClient_VideoDecoderConnectType_eWindowOnly;
    }
    connectSettings.simpleAudioDecoder.id = allocResults->simpleAudioDecoder.id;
    connectSettings.simpleEncoder[0].id = allocResults->simpleEncoder[0].id;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = maxHeight;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = maxWidth;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.colorDepth = colorDepth;
    connectSettings.simpleEncoder[0].nonRealTime = !realTime;
    rc = NxClient_Connect(&connectSettings, &context->connectId);

    if (rc) {
        delete context;
        return NULL;
    }

    xcodeHandle = new SimpleXcodeHandle;
    xcodeHandle->videoDecoder =
        NEXUS_SimpleVideoDecoder_Acquire(allocResults->simpleVideoDecoder[0].id);
    if (audioXcode) {
        xcodeHandle->audioDecoder =
            NEXUS_SimpleAudioDecoder_Acquire(allocResults->simpleAudioDecoder.id);
    } else {
        xcodeHandle->audioDecoder = NULL;
    }
    xcodeHandle->encoder =
        NEXUS_SimpleEncoder_Acquire(allocResults->simpleEncoder[0].id);
    xcodeHandle->context = context;
    return xcodeHandle;
}

void SimpleDecoderNxclient::releaseXcodeResource(
    SimpleXcodeHandle* xcodeHandle)
{
    BME_DEBUG_ENTER();
    SimpleDecoderNxclientContext* context;

    if (xcodeHandle) {
        context = xcodeHandle->context;
        assert(context);

        if (xcodeHandle->videoDecoder) {
            NEXUS_SimpleVideoDecoder_Release(xcodeHandle->videoDecoder);
        }

        if (xcodeHandle->audioDecoder) {
            NEXUS_SimpleAudioDecoder_Release(xcodeHandle->audioDecoder);
        }

        if (xcodeHandle->encoder) {
            NEXUS_SimpleEncoder_Release(xcodeHandle->encoder);
        }

        NxClient_Disconnect(context->connectId);
        NxClient_Free(&context->allocResults);
        delete context;
        delete xcodeHandle;
    }
}
#endif

void SimpleDecoderNxclient::freeMosaicsMaster(SimpleDecoderNxclientContext* master)
{
    BME_DEBUG_TRACE(("All mosaic decoders freed"));
    while (!master->mosaicsStack.empty()) {
        MosaicDecoderContext* mosaics = master->mosaicsStack.top();
        if (mosaics == NULL)
            break;
        NEXUS_SimpleVideoDecoder_Release(mosaics->videoDecoder);
        delete mosaics;
        master->mosaicsStack.pop();
    }
    NxClient_Disconnect(master->connectId);
    NxClient_Free(&master->allocResults);
    _videoDecoderMap.erase(_mosaicMaster);

    delete master;
    return;
}

void SimpleDecoderNxclient::releaseSimpleDecoders(NEXUS_SimpleVideoDecoderHandle videoDecoder,
        NEXUS_SimpleAudioDecoderHandle audioDecoder)
{
    BME_DEBUG_ENTER();

    SimpleDecoderNxclientContext* context = _videoDecoderMap[_self];
    assert(context);

    if (context->isMosaic) {
        SimpleDecoderNxclientContext* master = _videoDecoderMap[_mosaicMaster];
        assert(master);

        if (master->mosaicCount-- == 0) {
            freeMosaicsMaster(master);

            if (_mosaicMaster != _self) {
                delete context;
                _videoDecoderMap.erase(_self);
            }
            _mosaicMaster = 0;
            return;
        }

        MosaicDecoderContext* mosaics = new MosaicDecoderContext;
        mosaics->videoSurface = context->videoSurface;
        mosaics->videoDecoder = videoDecoder;
        master->mosaicsStack.push(mosaics);

        if (_mosaicMaster == _self) {
            return;
        }
    } else {
        if (videoDecoder)
            NEXUS_SimpleVideoDecoder_Release(videoDecoder);
        if (audioDecoder)
            NEXUS_SimpleAudioDecoder_Release(audioDecoder);

        if (context->surfaceClient) {
            NEXUS_SurfaceClient_Release(context->surfaceClient);
        }
        NxClient_Free(&context->allocResults);
    }

    delete context;
    _videoDecoderMap.erase(_self);
    BME_DEBUG_EXIT();
}

NEXUS_SimpleAudioPlaybackHandle SimpleDecoderNxclient::acquireSimpleAudioPlayback()
{
    BME_DEBUG_ENTER();
    NEXUS_Error rc;
    NEXUS_SimpleAudioPlaybackHandle audioPlayback;
    // NxClient_AudioSettings audioSettings;
    // NxClient_GetAudioSettings(&audioSettings);
    // audioSettings.hdmi.pcm = true;
    // audioSettings.spdif.pcm = true;
    // NxClient_SetAudioSettings(&audioSettings);

    SimpleDecoderNxclientContext* audioPlaybackContext = new SimpleDecoderNxclientContext;
    audioPlaybackContext->isMosaic = false;
    audioPlaybackContext->isPip = false;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults* allocResults = &audioPlaybackContext->allocResults;
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleAudioPlayback = 1;
    rc = NxClient_Alloc(&allocSettings, allocResults);

    if (rc) {
        rc = BERR_TRACE(rc);
        delete audioPlaybackContext;
        return NULL;
    }

    _audioPlaybackMap[_self] = audioPlaybackContext;

    audioPlayback = NEXUS_SimpleAudioPlayback_Acquire(allocResults->simpleAudioPlayback[0].id);
    if (!audioPlayback) {
        BME_DEBUG_ERROR(("Unable to acquire audioplayback!"));
    }
    BME_DEBUG_EXIT();
    return audioPlayback;
}

void SimpleDecoderNxclient::releaseSimpleAudioPlayback(
        NEXUS_SimpleAudioPlaybackHandle audioPlayback)
{
    BME_DEBUG_ENTER();
    SimpleDecoderNxclientContext* audioPlaybackContext = _audioPlaybackMap[_self];
    assert(audioPlaybackContext);
    NEXUS_SimpleAudioPlayback_Release(audioPlayback);
    NxClient_Free(&audioPlaybackContext->allocResults);
    _audioPlaybackMap.erase(_self);
    delete audioPlaybackContext;
    BME_DEBUG_EXIT();
}

void SimpleDecoderNxclient::setLocation(int x, int y, uint32_t width, uint32_t height)
{
    NEXUS_SurfaceComposition comp;
    SimpleDecoderNxclientContext* context = _videoDecoderMap[_self];
    assert(context);

    if (!context->isPip) {
        NEXUS_SurfaceClientSettings settings;
        NEXUS_SurfaceClient_GetSettings(context->videoSurface, &settings);
        settings.composition.position.width = width;
        settings.composition.position.height = height;
        settings.composition.position.x = x;
        settings.composition.position.y = y;
        settings.composition.virtualDisplay.width = _virtualWidth;
        settings.composition.virtualDisplay.height = _virtualHeight;
        settings.composition.zorder = context->mosaicCount;
        NEXUS_SurfaceClient_SetSettings(context->videoSurface, &settings);
    } else {
        // for pip, we scale the surfaceClient instead of video surface to avoid
        // our alpha hole covering main graphics
        NxClient_GetSurfaceClientComposition(context->allocResults.surfaceClient[0].id, &comp);
        NEXUS_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.width = width;
        rect.height = height;
        comp.position = rect;
        comp.zorder = 11;
        NxClient_SetSurfaceClientComposition(context->allocResults.surfaceClient[0].id, &comp);
    }
}

uint32_t SimpleDecoderNxclient::getVideoDisplayWidth()
{
    SimpleDecoderNxclientContext* context = _videoDecoderMap[_self];
    assert(context);
    NEXUS_SurfaceClientSettings settings;
    NEXUS_SurfaceClient_GetSettings(context->videoSurface, &settings);
    return settings.composition.position.width;
}

uint32_t SimpleDecoderNxclient::getVideoDisplayHeight()
{
    SimpleDecoderNxclientContext* context = _videoDecoderMap[_self];
    assert(context);
    NEXUS_SurfaceClientSettings settings;
    NEXUS_SurfaceClient_GetSettings(context->videoSurface, &settings);
    return settings.composition.position.height;
}

void SimpleDecoderNxclient::setVisibility(bool visible)
{
    SimpleDecoderNxclientContext* context = _videoDecoderMap[_self];
    assert(context);
    NEXUS_SurfaceClientSettings settings;
    NEXUS_SurfaceClient_GetSettings(context->videoSurface, &settings);
    settings.composition.visible = visible;
    NEXUS_SurfaceClient_SetSettings(context->videoSurface, &settings);
}

void SimpleDecoderNxclient::setAudioVolume(int32_t volume)
{
    NEXUS_Error rc;
    NxClient_AudioSettings audioSettings;

    if (volume > MAX_VOLUME) volume = MAX_VOLUME;
    if (volume < 0) volume = MAX_VOLUME / 2;

    NxClient_GetAudioSettings(&audioSettings);
    audioSettings.volumeType = NEXUS_AudioVolumeType_eLinear;
    audioSettings.rightVolume = audioSettings.leftVolume =
        CONVERT_TO_NEXUS_LINEAR_VOL(volume, MAX_VOLUME);
    rc = NxClient_SetAudioSettings(&audioSettings);
    if (rc != NEXUS_SUCCESS) {
        BME_DEBUG_ERROR(("Unable to set audio volume %d", rc));
    }
}

int32_t SimpleDecoderNxclient::getAudioVolume()
{
    int volume;
    NxClient_AudioSettings audioSettings;

    NxClient_GetAudioSettings(&audioSettings);
    volume = CONVERT_TO_USER_VOL(audioSettings.leftVolume, MAX_VOLUME);

    BME_DEBUG_PRINT(("volume: %s %d/%d %s\n",
        audioSettings.muted ? "muted ": "",
        CONVERT_TO_USER_VOL(audioSettings.leftVolume, MAX_VOLUME),
        CONVERT_TO_USER_VOL(audioSettings.rightVolume, MAX_VOLUME),
        audioSettings.volumeType == NEXUS_AudioVolumeType_eLinear ? "linear" : "decibel"));

    return volume;
}

void SimpleDecoderNxclient::mute(bool muted)
{
    NEXUS_Error rc;
    NxClient_AudioSettings audioSettings;

    NxClient_GetAudioSettings(&audioSettings);
    audioSettings.muted = muted;
    rc = NxClient_SetAudioSettings(&audioSettings);
    if (rc != NEXUS_SUCCESS) {
        BME_DEBUG_ERROR(("Unable to mute audio d", rc));
    }
}

}  // namespace Media
}  // namespace Broadcom
#endif  // SINGLE_PROCESS
