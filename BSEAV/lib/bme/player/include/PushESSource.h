/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __IPUSHESSOURCEIMPL_H__
#define __IPUSHESSOURCEIMPL_H__

#include <thread>
#include "BaseSource.h"

#include "DataFragment.h"

namespace Broadcom
{
namespace Media
{

struct PushESSourcePrivate_t;
struct Stream_t;
struct StreamCapture_t;

class BME_SO_EXPORT PushESSource : public BaseSource
{
public:
    PushESSource();
    virtual ~PushESSource();

    typedef DataFragment_t DataFragment;

    // ISource
    void start();
    void stop(bool holdLastFrame = false);
    void pause();
    IMedia::ErrorType prepare();
    void prepareAsync();
    void setDataSource(MediaStream *mediaStream);
    void reset();
    void release();
    void flush(bool  holdPicture);
    void flushAudio();
    void flushVideo();
    bool checkUrlSupport(const std::string& url);
    uint32_t setConnector(SourceConnector* connector);
    std::string getType();
    IMedia::StreamMetadata getStreamMetadata();
    IMedia::ErrorType seekTo(
    const uint32_t &milliseconds,
    IMedia::PlaybackOperation playOperation = IMedia::OperationForward,
    IMedia::PlaybackSeekTime  playSeekTime  =  IMedia::SeekTimeAbsolute);
    void setPlaybackRate(const std::string& rate);
    void setPlaybackRate(const std::string& rate, bool mute);
    int getPlaybackRate();
    IMedia::PlaybackOperation getPlaybackOperation();
    uint64_t getDuration()
    {
        return 0;
    }
    uint32_t getCurrentPosition();
    uint64_t getBytesDecoded();
    const IMedia::TimeInfo getTimeInfo();
    void setLooping(bool looping) {TRLS_UNUSED(looping);}
    Connector connect(const ConnectSettings& settings);
    void disconnect(const Connector& connector);
    std::string getAvailablePlaybackRate()
    {
        return std::string("");
    }
    IMedia::State getState() const
    {
        return _state;
    }

    void swapAudioCodecs(IMedia::AudioCodec codec);
    void swapAudioCodecs(NEXUS_AudioCodec codec);

    // IPushESSource
    void resumeFromPause();
    void seekToPts(TIME45k seekTo);

    // Make the sample
    void makeVideoChunk(TIME45k pts, const DataFragment_t *fragment, size_t n);
    void makeVp9Chunk(TIME45k pts, const DataFragment_t *fragment, size_t n);
    void makePesVideoChunk(TIME45k pts, const DataFragment_t *fragment, size_t n);

    void makeAudioChunk(TIME45k pts, const DataFragment_t *fragment, size_t n);
    void makeAdtsChunk(TIME45k pts, const DataFragment_t *fragment, size_t n);
    void makePcmChunk(TIME45k pts, const DataFragment_t *fragment, size_t n);
    void makeVorbisChunk(TIME45k pts, const DataFragment_t *fragment, size_t n);
    void makePesAudioChunk(TIME45k pts, const DataFragment_t *fragment, size_t n);
    void makeAudioEndOfStreamChunk(void);
    void makeVideoEndOfStreamChunk(void);

    // Send the sample
    typedef bool (PushESSource::* PushFunction_t)(void *drmContext, uint64_t vector, bool block);
    bool pushAudioChunk(void *drmContext = 0, uint64_t vector = 0, bool block = true);
    bool pushVideoChunk(void *drmContext = 0, uint64_t vector = 0, bool block = true);

    uint32_t getDropFrameCount();
    uint32_t getDisplayedFrameCount();
    uint32_t getVideoDecodedFrameCount();
    uint32_t getAudioDecodedFrameCount();
    TIME45k getCurrentStc();
    bool getCurrentVideoPts(TIME45k* pts);
    bool getCurrentAudioPts(TIME45k* pts);
    PushESSourcePrivate_t *getContext() const
    {
        return _context;
    }
    bool isRunning();
    void onPrepared();

    void setAudioPrivateHeader(const uint8_t *data, const size_t size);

private:
    bool initialise(bool Basic = false);
    void deinitialise();

    SourceConnector *getConnector() const;
    void        resetPlayState();

    unsigned int setAudioRate(unsigned int rate, bool mute);
    unsigned int setVideoRate(unsigned int rate);
    void setRates(unsigned int audio, unsigned int video);
    // rate: the actual rate times NEXUS_NORMAL_PLAY_SPEED (currently 1000), e.g. 1500 = 1.5x
    void stcRate(unsigned int rate);

    void onSeekComplete();
    void onError(const IMedia::ErrorType& errorType);
    void onCompletion();
    void onInfo(const IMedia::InfoType& infoType, int32_t extra = 0);
    void onVideoSizeChanged(uint16_t width, uint16_t height);
    void dataCallback(void *context, int param);
    static void staticErrorCallback(void *context, int param);

    static void staticVideoDataExhaustedCallback(void *context, int param);
    static void staticVideoPtsCallback(void *context, int param);
    static void staticVideoFirstPts(void *context, int param);
    static void staticVideoFirstPtsPassedCallback(void *context, int param);
    static void staticVideoSourceCallback(void *context, int param);

    static void staticAudioDataExhaustedCallback(void *context, int param);
    static void staticAudioFirstPts(void *context, int param);
    static void staticAudioSourceChanged(void *context, int param);

    bool            getVideoStatus(NEXUS_VideoDecoderStatus &status);
    void            audioWhen(const char *function);
    void            videoWhen(const char *function);
    static void triggerOnCompletion(void *context);

    PushESSourcePrivate_t *_context;
    std::thread            _onCompletionThread;
    std::thread            _prepareAsyncThread;
    int                    _playbackRate;
    IMedia::State          _state;
    /* last known good position */
    TIME45k                _lastStc;
    /* last PTS send to the decoder */
    uint32_t               _ptsHi;
    bool                   _videoPaused;
};

}  // namespace Media
}  // namespace Broadcom
#endif  // __IPUSHESSOURCEIMPL_H__
