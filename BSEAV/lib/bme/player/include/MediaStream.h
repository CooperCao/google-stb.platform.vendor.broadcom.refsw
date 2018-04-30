/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __MEDIASTREAM_H__
#define __MEDIASTREAM_H__

#include <vector>
#include <string>
#include "Observable.h"
#include "Media.h"
#include "ISubtitle.h"

namespace Broadcom {
namespace Media {

class MediaRecorder;

enum class MediaStreamEvents {
    ChannelChanged,
    STCRequested
};

class BME_SO_EXPORT MediaStream
    : public Observable<MediaStreamEvents>
{
    private:
        bool support4Kp60();
        void reset();

        std::string _id;
        std::string _uri;
        uint16_t _virtualWidth;
        uint16_t _virtualHeight;
        uint32_t _source;
        uint32_t _surfaceClientId;
        uint32_t _windowId;
        uint32_t _duration;
        uint64_t _contentLength;
        std::string _additionalHeaders;
        ISubtitle* _subtitle;
        bool _secureAudio;
        bool _secureVideo;
        bool _svp;
        uint16_t _preferredAudioPid;
        MediaRecorder* _recorder;
        bool _stcAuto;
        bool _progressiveOverride;

    public:
        MediaStream();
        explicit MediaStream(std::string uri);
        void operator=(const MediaStream& ms);

        std::string getId() const;
        void setId(const std::string& id);
        std::string getUri() const;
        void setUri(const std::string& uri);
        uint32_t getSource() const;
        uint16_t getVirtualWidth() const;
        void setVirtualWidth(const uint16_t virtualWidth);
        uint16_t getVirtualHeight() const;
        void setVirtualHeight(const uint16_t virtualHeight);
        uint32_t getSurfaceClientId() const;
        void setSurfaceClientId(const uint32_t surfaceClientId);
        uint32_t getWindowId() const;
        void setWindowId(const uint32_t windowId);
        uint32_t getDuration() const;
        void setDuration(const uint32_t duration);
        uint64_t getContentLength() const;
        void setContentLength(const uint64_t contentLength);
        const std::string & getAdditionalHeaders() const;
        void  setAdditionalHeaders(const std::string & additionalHeaders);
        ISubtitle* getSubtitle() const;
        void setSubtitle(ISubtitle* subtitle);

        // secure video and audio means playpump will be using secure memory
        void setSecureAudio(bool secure);
        bool isAudioSecure() const;
        void setSecureVideo(bool secure);
        bool isVideoSecure() const;

        // SVP means decoder will use secure memory. This only works if
        // nxserver is started with following flag
        // -memconfig videoDecoder,svp,0,su -memconfig display,svp,0,su
        void setSvp(bool enable);
        bool isSvp() const;
        IMedia::StreamMetadata metadata;
        void setPreferredAudioPid(const uint16_t pid);
        uint16_t getPreferredAudioPid() const;
        MediaRecorder* getRecorder() const;
        void setRecorder(MediaRecorder* recorder);
        void setStcMode(bool automatic);
        bool getStcMode() const;

        void setProgressiveOverride(bool override);
        bool isProgressiveOverrideDisabled() const;
};
}  // namespace Media
}  // namespace Broadcom

#endif  // __MEDIASTREAM_H__
