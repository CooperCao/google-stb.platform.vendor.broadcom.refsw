/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef LIB_MEDIA_FILESOURCE_H_
#define LIB_MEDIA_FILESOURCE_H_

#include <string>
#include <thread>
#include "BaseSource.h"
#include "MediaRecorder.h"

namespace Broadcom
{
namespace Media {

typedef struct tagFileSourceContext FileSourceContext;

class BME_SO_EXPORT FileSource
    : public BaseSource
{
    public:
        FileSource();
        virtual ~FileSource();

        /* BaseSource */
        virtual void start();
        virtual void stop(bool holdLastFrame = false);
        virtual void pause();
        virtual IMedia::ErrorType prepare();
        virtual void prepareAsync();
        virtual void setDataSource(MediaStream *mediaStream);
        virtual void reset();
        virtual void release();
        virtual bool checkUrlSupport(const std::string& url);
        virtual uint32_t setConnector(SourceConnector* connector);
        virtual std::string getType();
        virtual IMedia::StreamMetadata getStreamMetadata();
        virtual IMedia::ErrorType seekTo(const uint32_t& milliseconds,
                IMedia::PlaybackOperation playOperation = IMedia::OperationForward,
                IMedia::PlaybackSeekTime  playSeekTime = IMedia::SeekTimeAbsolute);
        virtual void setPlaybackRate(const std::string& rate);
        virtual int getPlaybackRate();
        virtual IMedia::PlaybackOperation getPlaybackOperation();
        virtual uint64_t getDuration();
        virtual uint32_t getCurrentPosition();
        virtual uint64_t getBytesDecoded();
        virtual const IMedia::TimeInfo getTimeInfo();
        virtual void setLooping(bool looping);
        virtual Connector connect(const ConnectSettings& settings);
        virtual void disconnect(const Connector& connector);
        virtual std::string getAvailablePlaybackRate();

        static void endOfStreamCallback(void* context, int param);
        static void beginningOfStreamCallback(void* context, int param);
        static void prepareAsyncThread(void* data);
        void onPrepared(FileSource&);
        void onError(FileSource&, const IMedia::ErrorType& errorType);
        void onCompletion(FileSource&);
        void onBeginning(FileSource&);
        std::string getDataSource();
        void probe(const std::string& filename, const std::string& indexname, bool* fileCanIndex);
        FileSourceContext* getContext() {
            return _context;
        }
        static void stopInternalThread(void* data);

    private:
        static const int16_t _trickModes[13];
        static const uint32_t _trickModeCount;
        FileSourceContext* _context;
        IMedia::State _state;
        uint32_t _initialSeekTimeMSec;
        bool _looping;
        std::string _dataSource;
        uint64_t _duration;
        bool _resourceAcquired;
        IMedia::StreamMetadata _metadata;
        std::string _playbackRates;
        RecordMode _fileType;
        std::thread _prepareAsyncThread;
        std::thread _stopInternalThread;

        void init();
        void uninit();
        void acquireResources();
        void releaseResources();
        SourceConnector* getConnector();
        void setStreamMetadata(const IMedia::StreamMetadata& metaData);

        IMedia::ErrorType acquirePicDecoder();
        void releasePicDecoder();
        void startPicDecoder();
        void stopPicDecoder();
        bool isImage(const std::string& fileName);
        void flush();
        void updateChunkFileDuration();
};

}  // namespace Media
}  // namespace Broadcom
#endif  // LIB_MEDIA_FILESOURCE_H_
