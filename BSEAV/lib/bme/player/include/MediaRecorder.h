/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef MEDIA_PLAYER_INCLUDE_MEDIARECORDER_H_
#define MEDIA_PLAYER_INCLUDE_MEDIARECORDER_H_

#include "Observable.h"
#include "MediaStream.h"
#include "ThumbnailImgGen.h"

namespace Broadcom {
namespace Media {

enum class RecordState {
    Started,
    Stopped
};

enum class RecordMode {
    Linear,
    ChunkedFile,
    Fifo,
    ChunkedFifo
};

enum class MediaRecorderEvents {
    Started,
    Stopped,
    Error
};

typedef std::function<void(void*, uint16_t, void*, uint16_t, IMedia::TransportSegmentFormat)>
                                                                    OnPidDataCBRecord;

class BaseSource;
class SourceConnector;

struct MediaRecorderOptions
{
    RecordMode mode = RecordMode::ChunkedFifo;
    ThumbnailOptions thumbnailOptions;
    std::string destFile;
    MediaStream* sourceMediaStream;

    struct
    {
        unsigned int interval = 1800;
        unsigned int snapshotInterval = 2;
        unsigned int chunkSize = 10485760;
    } chunkedFifoRecordSettings;

    struct
    {
        unsigned int chunkSize = 10485760;
        std::string chunkTemplate;
    } chunkedFileRecordSettings;
};

typedef struct tagRecordContext RecordContext;

class BME_SO_EXPORT MediaRecorder :
    public Observable<MediaRecorderEvents>
{
    public:
        explicit MediaRecorder();
        virtual ~MediaRecorder();
        static uint32_t getTotalRecords();
        void stop();
        IMedia::ErrorType onStartRecord();
        IMedia::ErrorType start();
        RecordState getRecordState();
        void setDataSource(MediaStream *mediaStream);
        void addPlayback(void* playback);
        void removePlayback(void* playback);
        void* getFifoRecord();
        void* getChunkedFifoRecord();
        std::string getFileName();
        std::string getIndexName();
        uint32_t getBeginTimestamp();
        uint32_t getCurrentTimestamp();
        bool isPicturesIndexed();
        void setOptions(const MediaRecorderOptions& options);
        void onError();
        void startPidMonitoring(uint16_t pid, IMedia::TransportSegmentFormat format);
        void stopPidMonitoring(uint16_t pid, IMedia::TransportSegmentFormat format);
        void stopAllPidMonitoring();
        void onMessageCallback(void* context);
        void addPid(uint16_t pid);
        void removePid(uint16_t pid);
        size_t getSource();

    private:
        void saveMetadata();
        void onStarted();
        void onStopped();
        static const int _tsbLiveBufferLength = 30 * 60;  // 30 minutes
        static const int _tsbMaxBitRate = 20 * 1024 * 1024;
        static const int _tsbDataAlign = (188/4)*4096;

        std::string _filenameNoExt;
        RecordState _state;
        RecordContext* _recContext;
        MediaStream* _mediaStream;
        std::shared_ptr<void> _mediaStreamListener;
        std::string _recordDir;
        MediaRecorderOptions _options;
        BaseSource* _source;
        std::string _uri;
        SourceConnector* _connector;
};

}  // namespace Media
}  // namespace Broadcom
#endif  // MEDIA_PLAYER_INCLUDE_MEDIARECORDER_H_
