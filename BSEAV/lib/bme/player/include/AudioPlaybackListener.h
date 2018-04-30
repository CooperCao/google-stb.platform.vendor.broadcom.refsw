/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __AUDIOPLAYBACKLISTENER_H__
#define __AUDIOPLAYBACKLISTENER_H__

#include "AudioPlayback.h"

namespace Broadcom
{
namespace Media
{

class AudioPlaybackListener
{
    public:
        AudioPlaybackListener() {}
        virtual ~AudioPlaybackListener()
        {
        }

        void observe(AudioPlayback* audioPlayback)
        {
            std::function<void()> onBufferReady = std::bind(&AudioPlaybackListener::onBufferReady, this);
            _listenerId = audioPlayback->addListener(AudioPlaybackEvents::BufferReady, onBufferReady);
        }
    private:
        virtual void onBufferReady()
        {
        }

        std::shared_ptr<void> _listenerId;
};

}  // namespace Media
}  // namespace Broadcom
#endif  // __AUDIOPLAYBACKLISTENER_H__
