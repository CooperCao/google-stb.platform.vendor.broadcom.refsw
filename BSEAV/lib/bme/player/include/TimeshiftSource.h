/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef MEDIA_PLAYER_INCLUDE_TIMESHIFTSOURCE_H_
#define MEDIA_PLAYER_INCLUDE_TIMESHIFTSOURCE_H_

#include <string>
#include "BaseSource.h"
#include "MediaStream.h"
#include "ThumbnailImgGen.h"
#include "MediaRecorder.h"

namespace Broadcom
{
namespace Media {

typedef struct tagTimeshiftSourceContext TimeshiftSourceContext;

class BME_SO_EXPORT TimeshiftSource
    : public BaseSource
{
    public:
        TimeshiftSource(MediaRecorder* mediaRecorder, bool playbackOnlyMode = false);
        virtual ~TimeshiftSource();

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
        void onCompletion(TimeshiftSource&);
        void onBeginning(TimeshiftSource&);
        void onInfo(const IMedia::InfoType& infoType, int32_t extra);

    private:
        TimeshiftSourceContext* _context;
        IMedia::State _state;
        std::string _playbackRates;
        std::string _rate;
        void* _source;
        void init();
        void uninit();
        SourceConnector* getConnector();
        void setStreamMetadata(const IMedia::StreamMetadata& metaData);
        IMedia::ErrorType startLive();
        IMedia::ErrorType stopLive(bool holdLastFrame);
        IMedia::ErrorType startPlayback();
        void stopPlayback();
        void pausePlayback();
        MediaRecorder* _mediaRecorder;
        bool _useInitialSeekPosition;
        uint32_t _initialSeekPosition;
};

}  // namespace Media
}  // namespace Broadcom
#endif  // MEDIA_PLAYER_INCLUDE_TIMESHIFTSOURCE_H_
