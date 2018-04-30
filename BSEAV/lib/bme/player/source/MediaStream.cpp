/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include "./MediaStream.h"
#include <string>
#include "string.h"

namespace Broadcom {
namespace Media {

bool MediaStream::support4Kp60()
{
#if((((BCHP_CHIP == 7445) || (BCHP_CHIP == 7252)) && (BCHP_VER == BCHP_VER_D0)) || \
        (BCHP_CHIP == 7366 && BCHP_VER == BCHP_VER_B0))
    char const* boxMode = getenv("B_REFSW_BOXMODE");
    if ( boxMode == NULL ) {
        return true;
    } else {
        if (strncmp(boxMode, "3", 1) == 0) {
            return true;
        } else {
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}

void MediaStream::reset()
{
    _virtualWidth = 1280;
    _virtualHeight= 720;
    _source= 0;
    _surfaceClientId = 0;
    _windowId = 0;
    _duration = 0;
    _contentLength = 0;
    _subtitle = NULL;
    _recorder = NULL;
    _secureAudio = false;
    _secureVideo = false;
#ifdef BRCM_SAGE
    _svp = true;
#endif
    _stcAuto = true;

    metadata.reset();

    metadata.videoParam.maxWidth = 0;
    metadata.videoParam.maxHeight = 0;
#if(BCHP_CHIP == 7364)
    metadata.videoParam.colorDepth = 8;
#endif
    _preferredAudioPid = 0;
    _progressiveOverride = true;
}

MediaStream::MediaStream()
{
    reset();
}

MediaStream::MediaStream(std::string Uri)
{
    reset();
    _uri = Uri;
}

void MediaStream::operator=(const MediaStream& ms)
{
    _id = ms.getId();
    _uri = ms.getUri();
    _virtualHeight = ms.getVirtualHeight();
    _virtualWidth = ms.getVirtualWidth();
    _source = ms.getSource();
    _surfaceClientId = ms.getSurfaceClientId();
    _windowId = ms.getWindowId();
    _duration = ms.getDuration();
    _contentLength = ms.getContentLength();
    _additionalHeaders = ms.getAdditionalHeaders();
    _preferredAudioPid = ms.getPreferredAudioPid();

    metadata = ms.metadata;
    _subtitle = ms.getSubtitle();
    _recorder = ms.getRecorder();
}

std::string MediaStream::getId() const
{
    return _id;
}

void MediaStream::setId(const std::string& id)
{
    _id = id;
}

std::string MediaStream::getUri() const
{
    return _uri;
}

void MediaStream::setUri(const std::string& uri)
{
    _uri = uri;
}

uint32_t MediaStream::getSource() const
{
    return _source;
}

uint16_t MediaStream::getVirtualWidth() const
{
    return _virtualWidth;
}

void MediaStream::setVirtualWidth(const uint16_t virtualWidth)
{
    _virtualWidth = virtualWidth;
}

uint16_t MediaStream::getVirtualHeight() const
{
    return _virtualHeight;
}

void MediaStream::setVirtualHeight(const uint16_t virtualHeight)
{
    _virtualHeight = virtualHeight;
}

uint32_t MediaStream::getSurfaceClientId() const
{
    return _surfaceClientId;
}

void MediaStream::setSurfaceClientId(const uint32_t surfaceClientId)
{
    _surfaceClientId = surfaceClientId;
}

uint32_t MediaStream::getWindowId() const
{
    return _windowId;
}

void MediaStream::setWindowId(const uint32_t windowId)
{
    _windowId = windowId;
}

uint32_t MediaStream::getDuration() const
{
    return _duration;
}

void MediaStream::setDuration(const uint32_t duration)
{
    _duration = duration;
}

uint64_t MediaStream::getContentLength() const
{
    return _contentLength;
}

void MediaStream::setContentLength(const uint64_t contentLength)
{
    _contentLength = contentLength;
}

const std::string & MediaStream::getAdditionalHeaders() const
{
    return _additionalHeaders;
}

void  MediaStream::setAdditionalHeaders(const std::string & additionalHeaders)
{
    _additionalHeaders = additionalHeaders;
}

ISubtitle* MediaStream::getSubtitle() const
{
    return _subtitle;
}

void MediaStream::setSubtitle(ISubtitle* subtitle)
{
    _subtitle = subtitle;
}

void MediaStream::setPreferredAudioPid(const uint16_t pid)
{
    _preferredAudioPid = pid;
}

uint16_t MediaStream::getPreferredAudioPid() const
{
    return _preferredAudioPid;
}

MediaRecorder* MediaStream::getRecorder() const
{
    return _recorder;
}

void MediaStream::setRecorder(MediaRecorder* recorder)
{
    _recorder = recorder;
}

void MediaStream::setSecureAudio(bool secure)
{
    _secureAudio = secure;
}
bool MediaStream::isAudioSecure() const
{
    return _secureAudio;
}
void MediaStream::setSecureVideo(bool secure)
{
    _secureVideo = secure;
}
bool MediaStream::isVideoSecure() const
{
    return _secureVideo;
}
void MediaStream::setSvp(bool enable)
{
    _svp = enable;
}
bool MediaStream::isSvp() const
{
    return _svp;
}
void MediaStream::setStcMode(bool automatic)
{
    _stcAuto = automatic;
}
bool MediaStream::getStcMode() const
{
    return _stcAuto;
}

void MediaStream::setProgressiveOverride(bool override)
{
    _progressiveOverride = override;
}

bool MediaStream::isProgressiveOverrideDisabled() const
{
    return (_progressiveOverride == false);
}
}  // namespace Media
}  // namespace Broadcom
