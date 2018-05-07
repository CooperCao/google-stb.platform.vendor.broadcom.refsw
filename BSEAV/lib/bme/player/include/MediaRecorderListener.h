/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __MEDIARECORDERLISTENER_H_
#define __MEDIARECORDERLISTENER_H_

#include "MediaRecorder.h"

namespace Broadcom {
namespace Media {

class MediaRecorderListener
{
    public:
        MediaRecorderListener() {}
        virtual ~MediaRecorderListener() {}

        void observe(MediaRecorder* mediaRecorder)
        {
            std::function<void()> onStarted = std::bind(&MediaRecorderListener::onStarted, this);
            std::function<void()> onStopped = std::bind(&MediaRecorderListener::onStopped, this);
            std::function<void(IMedia::ErrorType)> onError =
                std::bind(&MediaRecorderListener::onError, this, _1);
            mediaRecorder->addListener(MediaRecorderEvents::Started, onStarted);
            mediaRecorder->addListener(MediaRecorderEvents::Stopped, onStopped);
            mediaRecorder->addListener(MediaRecorderEvents::Error, onError);
        }

    private:
        virtual void onStarted()
        {
        }

        virtual void onStopped()
        {
        }

        virtual void onError(IMedia::ErrorType errorType)
        {
            TRLS_UNUSED(errorType);
        }
};

}  // namespace Media
}  // namespace Broadcom
#endif  // __MEDIARECORDERLISTENER_H_
