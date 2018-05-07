/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "SimpleDecoderSingle.h"
#include <assert.h>
#include <algorithm>
#include <string>
#include "nxclient.h"
#include "nexus_surface_client.h"
#include "nexus_platform_features.h"
#include "nexus_surface_compositor.h"
#include "nexus_simple_video_decoder_server.h"
#include "MediaPlayer.h"

#ifdef SINGLE_PROCESS

namespace Broadcom
{
namespace Media {

TRLS_DBG_MODULE(SimpleDecoderSingle);

SimpleDecoderSingle::SimpleDecoderMap SimpleDecoderSingle::_decoderMap;
uint16_t SimpleDecoderSingle::_index = 0;
uint32_t SimpleDecoderSingle::_displayHeight =  720;
uint32_t SimpleDecoderSingle::_displayWidth = 1280;

typedef struct tagSimpleDecoderSingleContext {
    MediaPlayer* mediaPlayer;
    IMedia::DecoderType decoderType;
    NEXUS_VideoWindowHandle window;
    uint32_t index;
} SimpleDecoderSingleContext;

SimpleDecoderSingle::SimpleDecoderSingle()
{
}

SimpleDecoderSingle::~SimpleDecoderSingle()
{
}

void SimpleDecoderSingle::init()
{
    _index++;
    _self = _index;
}

void SimpleDecoderSingle::uninit()
{
}

void SimpleDecoderSingle::acquireSimpleDecoders(const acquireParameters& param,
        NEXUS_SimpleVideoDecoderHandle* videoDecoder,
        NEXUS_SimpleAudioDecoderHandle* audioDecoder)
{
    BME_DEBUG_ENTER();
    IMedia::DecoderType decoderType = param.decoderType;
    NxClient_AllocResults* allocResults;
    uint32_t videoIndex, audioIndex, retval;

    SimpleDecoderSingle::_displayWidth = param.virtualWidth;
    SimpleDecoderSingle::_displayHeight = param.virtualHeight;

    SimpleDecoderMap::iterator iter = _decoderMap.begin();

    while (iter != _decoderMap.end()) {
        SimpleDecoderSingleContext* context = iter->second;

        // don't need to notify ourself
        if (iter->first == _self) {
            iter++;
            continue;
        }

        if (decoderType != IMedia::DecoderType::Mosaic) {
            iter++;
            context->mediaPlayer->releaseVideoDecoder();
            //_decoderMap.erase(iter++);
            //delete context;
        } else {
            ++iter;
        }
    }

    SimpleDecoderSingleContext* context = new SimpleDecoderSingleContext;
    if (videoDecoder) {
        *videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(0);
        assert(*videoDecoder);
        // Get the window for setting position
        // We can get the the window here because we are in-proc of the app
        NEXUS_SimpleVideoDecoderServerSettings videoSettings;
        NEXUS_SimpleVideoDecoder_GetServerSettings(*videoDecoder, &videoSettings);
        context->window = videoSettings.window[0];
        context->index = _self;
        context->mediaPlayer = param.mediaPlayer;
        _decoderMap[_self] = context;
    }
    if (audioDecoder) {
        *audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(0);
    }
}

void SimpleDecoderSingle::releaseSimpleDecoders(NEXUS_SimpleVideoDecoderHandle videoDecoder,
            NEXUS_SimpleAudioDecoderHandle audioDecoder)
{
    SimpleDecoderSingleContext* context = _decoderMap[_self];
    _decoderMap.erase(_self);
    if (videoDecoder)
        NEXUS_SimpleVideoDecoder_Release(videoDecoder);
    if (audioDecoder)
        NEXUS_SimpleAudioDecoder_Release(audioDecoder);
    if (context)
        delete context;
}

void SimpleDecoderSingle::setLocation(int x, int y, uint32_t width, uint32_t height)
{
    SimpleDecoderSingleContext* context = _decoderMap[_self];

    if (!context) {
        BME_DEBUG_ERROR(("Unable to find context associated with uid"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        return;
    }

    NEXUS_VideoWindowHandle window = context->window;
    // Set the rect directly
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);

    if ((x > (int)_displayWidth) || (y > (int)_displayHeight)) {
        windowSettings.visible = false;
    } else if ((width == _displayWidth) && (height == _displayHeight)) {
        // fullscreen
        windowSettings.position.x = x;
        windowSettings.position.y = y;
        windowSettings.position.width = width;
        windowSettings.position.height = height;
        windowSettings.visible = true;
        windowSettings.clipRect = windowSettings.clipBase;
        windowSettings.contentMode = NEXUS_VideoWindowContentMode_eBox;
    } else {
        NEXUS_CalculateVideoWindowPositionSettings positionSettings;
        NEXUS_GetDefaultCalculateVideoWindowPositionSettings(&positionSettings);
        positionSettings.displayWidth = _displayWidth;
        positionSettings.displayHeight = _displayHeight;
        positionSettings.viewport.x = x;
        positionSettings.viewport.y = y;
        positionSettings.viewport.width = width;
        positionSettings.viewport.height = height;
        NEXUS_CalculateVideoWindowPosition(&positionSettings, &windowSettings, &windowSettings);
        windowSettings.contentMode = NEXUS_VideoWindowContentMode_eFull;
    }

    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
}

uint32_t SimpleDecoderSingle::getVideoDisplayWidth()
{
    SimpleDecoderSingleContext* context = _decoderMap[_self];

    if (!context) {
        BME_DEBUG_ERROR(("Unable to find context associated with uid"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        return 0;
    }

    NEXUS_VideoWindowHandle window = context->window;
    // Set the rect directly
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    return windowSettings.position.width;
}

uint32_t SimpleDecoderSingle::getVideoDisplayHeight()
{
    SimpleDecoderSingleContext* context = _decoderMap[_self];

    if (!context) {
        BME_DEBUG_ERROR(("Unable to find context associated with uid"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        return 0;
    }

    NEXUS_VideoWindowHandle window = context->window;
    // Set the rect directly
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    return windowSettings.position.height;
}

void SimpleDecoderSingle::setVisibility(bool visible)
{
    SimpleDecoderSingleContext* context = _decoderMap[_self];

    if (!context) {
        BME_DEBUG_ERROR(("Unable to find context associated with uid"));
        BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
        return;
    }

    NEXUS_VideoWindowHandle window = context->window;
    // Set the rect directly
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.visible = visible;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
}

void SimpleDecoderSingle::setAudioVolume(int32_t volume)
{
}

int32_t SimpleDecoderSingle::getAudioVolume()
{
}

void SimpleDecoderSingle::mute(bool muted)
{
}

NEXUS_SimpleAudioPlaybackHandle SimpleDecoderSingle::acquireSimpleAudioPlayback()
{
    // TBD
    BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
    return 0;
}

void SimpleDecoderSingle::releaseSimpleAudioPlayback(NEXUS_SimpleAudioPlaybackHandle)
{
    // TBD
    BME_DEBUG_THROW_EXCEPTION(ReturnFailure);
}
}  // namespace Media
}  // namespace Broadcom
#endif
