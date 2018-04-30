/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef LIB_MEDIA_LIVESOURCE_H_
#define LIB_MEDIA_LIVESOURCE_H_

#include <string>
#include "BaseSource.h"
#include "MediaStream.h"

namespace Broadcom
{
namespace Media {

typedef struct tagLiveSourceContext LiveSourceContext;

class LiveSource
    : public BaseSource
{
    public:
        LiveSource();
        virtual ~LiveSource();

        /* BaseSource */
        virtual void start();
        virtual void stop(bool holdLastFrame = false);
        virtual void pause();
        virtual IMedia::ErrorType prepare();
        virtual void prepareAsync();
        virtual void setDataSource(MediaStream *mediaStream);
        virtual void reset();
        virtual void release();
        virtual Connector connect(const ConnectSettings& settings);
        virtual void disconnect(const Connector& connector);
        virtual bool checkUrlSupport(const std::string& url);
        virtual uint32_t setConnector(SourceConnector* connector);
        virtual IMedia::StreamMetadata getStreamMetadata();
        virtual std::string getType();
        virtual IMedia::ErrorType seekTo(const uint32_t& milliseconds,
                IMedia::PlaybackOperation playOperation = IMedia::OperationForward,
                IMedia::PlaybackSeekTime  playSeekTime = IMedia::SeekTimeAbsolute);
        virtual void setPlaybackRate(const std::string& rate);
        virtual int getPlaybackRate();
        virtual IMedia::PlaybackOperation getPlaybackOperation();
        virtual std::string getAvailablePlaybackRate();
        virtual uint64_t getDuration();
        virtual uint32_t getCurrentPosition();
        virtual uint64_t getBytesDecoded();
        virtual const IMedia::TimeInfo getTimeInfo();
        virtual void setLooping(bool looping);

        void setMediaStream(MediaStream* mediaStream);

    private:
        LiveSourceContext* _context;
        IMedia::State _state;
        std::string _playbackRates;
        void* _source;
        IMedia::StreamMetadata _streamMetadata;

        void init();
        void uninit();
        SourceConnector* getConnector();
        void setStreamMetadata(const IMedia::StreamMetadata& metaData);
};

}  // namespace Media
}  // namespace Broadcom
#endif  // LIB_MEDIA_LIVESOURCE_H_
