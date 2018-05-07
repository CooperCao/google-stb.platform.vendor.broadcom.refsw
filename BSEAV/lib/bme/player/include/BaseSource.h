/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef LIB_MEDIA_BASESOURCE_H_
#define LIB_MEDIA_BASESOURCE_H_

#include <stdint.h>
#include "Media.h"
#include "Debug.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "Observable.h"
#include "MediaStream.h"

namespace Broadcom {
namespace Media {

typedef void* Connector;

class ConnectSettings
{
 public:
    uint16_t streamId;
    IMedia::AudioCodec audioCodec;
};

#define SOURCE_NETWORK "SOURCE_NETWORK"
#define SOURCE_FILE "SOURCE_FILE"
#define SOURCE_TV "SOURCE_TV"
#define SOURCE_TIMESHIFT_TV "SOURCE_TIMESHIFT_TV"
#define SOURCE_QAM "SOURCE_QAM"
#define SOURCE_SATELLITE "SOURCE_QAM"
#define SOURCE_OFDM "SOURCE_OFDM"
#define SOURCE_HDMI "SOURCE_HDMI"
#define SOURCE_PUSHES "SOURCE_PUSHES"
#define SOURCE_PUSHTS "SOURCE_PUSHTS"
#define SOURCE_PUSHYTMS "SOURCE_PUSHYTMS"
#define SOURCE_BROADCAST "SOURCE_HDMI"
#define SOURCE_DVR "SOURCE_DVR"
#define PLAYBACK_RATE_PAUSED 0  // PAUSED_SPEED;

const TIME45k INVALID_TIMESTAMP = 0xFFFFFFFF;

/**
  * Attributes specific to a video encoding.
  */
enum Format3D {
    NOT_3D,
    MVC_COMBINED,
    MVC_SPLIT
};

enum SourceType {
    VIDEO_STREAM,
    AUDIO_STREAM,
    VIDEO3D_STREAM
};

enum E_PUMPBUF_STATE {
    E_IDLE,
    E_READY,
    E_FULL,
    E_ERROR
};


class SourceConnector
{
public:
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle enhancementVideoPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle patPidChannel;
    NEXUS_PidChannelHandle pmtPidChannel;
    NEXUS_PidChannelHandle streamExtractorPidChannel;
    uint16_t streamExtractorPid;
};

enum class SourceEvents {
    Completed,
    Prepared,
    Error,
    Info,
    SeekCompleted,
    VideoSizeChanged,
    DrmKeyAdded,
    DrmKeyError,
    DrmKeyMessage,
    DrmKeyNeeded,
    WVHdcpRequested,
    BeginningOfStream,
    HdrSettings
};

class BME_SO_EXPORT BaseSource
    : public Observable<SourceEvents>
{
    public:
        BaseSource();
        virtual ~BaseSource();

        virtual void start() = 0;
        virtual void stop(bool holdLastFrame = false) = 0;
        virtual void pause() = 0;
        virtual IMedia::ErrorType prepare() = 0;
        virtual void prepareAsync() = 0;
        virtual void setDataSource(MediaStream *mediaStream) = 0;
        virtual void reset() = 0;
        virtual void release() = 0;
        // TODO: Rename connect/disconnect
        virtual Connector connect(const ConnectSettings& settings) = 0;
        virtual void disconnect(const Connector& connector) = 0;
        virtual bool checkUrlSupport(const std::string& url) = 0;

        // TODO: Rename setConnector
        virtual uint32_t setConnector(SourceConnector* connector) = 0;
        virtual IMedia::StreamMetadata getStreamMetadata() = 0;

        virtual std::string getType() = 0;
        virtual IMedia::ErrorType seekTo(const uint32_t& milliseconds,
                IMedia::PlaybackOperation playOperation = IMedia::OperationForward,
                IMedia::PlaybackSeekTime  playSeekTime = IMedia::SeekTimeAbsolute) = 0;

        virtual void setPlaybackRate(const std::string& rate) = 0;
        virtual int getPlaybackRate() = 0;
        virtual IMedia::PlaybackOperation getPlaybackOperation() = 0;
        virtual std::string getAvailablePlaybackRate() = 0;
        virtual uint64_t getDuration() = 0;
        virtual uint32_t getCurrentPosition() = 0;
        virtual const IMedia::TimeInfo getTimeInfo() = 0;
        virtual uint64_t getBytesDecoded() = 0;
        virtual void setLooping(bool looping) = 0;

    public:
        static NEXUS_VideoCodec convertVideoCodec(const IMedia::VideoCodec& media_value);
        static NEXUS_AudioCodec convertAudioCodec(const IMedia::AudioCodec& media_value);
        static NEXUS_TransportType convertStreamType(const IMedia::StreamType& media_value);
        bool support4Kp60();
        void setDecoderType(const IMedia::DecoderType decoderType);

    protected:
        SourceConnector* _connector;
        IMedia::DecoderType _decoderType;
        uint16_t _preferredAudioPid;
};

}  // namespace Media
}  // namespace Broadcom

#endif  // LIB_MEDIA_BASESOURCE_H_
