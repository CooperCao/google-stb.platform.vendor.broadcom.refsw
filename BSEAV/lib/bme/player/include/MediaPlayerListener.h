/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef LIB_MEDIA_MEDIAPLAYERLISTENER_H_
#define LIB_MEDIA_MEDIAPLAYERLISTENER_H_

#include "Media.h"
namespace Broadcom
{
namespace Media
{

class MediaPlayer;

class MediaPlayerListener
{
    public:
        MediaPlayerListener() {}
        virtual ~MediaPlayerListener() {}

        void observe(MediaPlayer* mp)
        {
            std::function<void()> onCompletion = std::bind(&MediaPlayerListener::onCompletion, this);
            std::function<void()> onPrepared = std::bind(&MediaPlayerListener::onPrepared, this);
            std::function<void()> onRateChanged = std::bind(&MediaPlayerListener::onRateChanged, this);
            std::function<void(const IMedia::ErrorType)> onError =
                std::bind(&MediaPlayerListener::onError, this, _1);
            std::function<void(const IMedia::InfoType, int32_t extra)> onInfo =
                std::bind(&MediaPlayerListener::onInfo, this, _1, _2);
            std::function<void()> onSeekComplete = std::bind(&MediaPlayerListener::onSeekComplete, this);
            std::function<void(uint16_t width, uint16_t height)> onVideoSizeChanged =
                std::bind(&MediaPlayerListener::onVideoSizeChanged, this, _1, _2);
            std::function<void()> onBeginning = std::bind(&MediaPlayerListener::onBeginning, this);
            mp->addListener(MediaPlayerEvents::Completed, onCompletion);
            mp->addListener(MediaPlayerEvents::Prepared, onPrepared);
            mp->addListener(MediaPlayerEvents::Error, onError);
            mp->addListener(MediaPlayerEvents::Info, onInfo);
            mp->addListener(MediaPlayerEvents::SeekCompleted, onSeekComplete);
            mp->addListener(MediaPlayerEvents::VideoSizeChanged, onVideoSizeChanged);
            mp->addListener(MediaPlayerEvents::PlaybackRateChanged, onRateChanged);
            mp->addListener(MediaPlayerEvents::Beginning, onBeginning);
        }
    private:
        virtual void onCompletion()
        {
        }

        virtual void onPrepared()
        {
        }

        virtual void onError(const IMedia::ErrorType& errorType)
        {
            TRLS_UNUSED(errorType);
        }

        virtual void onInfo(const IMedia::InfoType& infoType, int32_t extra)
        {
            TRLS_UNUSED(infoType);
            TRLS_UNUSED(extra);
        }

        virtual void onSeekComplete()
        {
        }

        virtual void onVideoSizeChanged(uint16_t width, uint16_t height)
        {
            TRLS_UNUSED(width);
            TRLS_UNUSED(height);
        }

        virtual void onRateChanged()
        {
        }

        virtual void onBeginning()
        {
        }
};

}  // namespace Media
}  // namespace Broadcom
#endif  // LIB_MEDIA_MEDIAPLAYERLISTENER_H_
