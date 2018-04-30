/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifdef BME_OVER_GSTREAMER
#include "Gstreamer/PushTSSource.h"
#else
#ifndef __IPUSHTSSOURCEIMPL_H__
#define __IPUSHTSSOURCEIMPL_H__

#include "BaseSource.h"


namespace Broadcom
{
namespace Media {

typedef struct tagBMEPushTSEngineContext  BMEPushTSEngineContext;

class PushTSSource :
    public BaseSource
{
public:
    PushTSSource();
    virtual ~PushTSSource();

    // ISource
    virtual void start();
    virtual void stop(bool holdLastFrame = false);
    virtual void pause();
    virtual IMedia::ErrorType prepare();
    virtual void prepareAsync();
    virtual void setDataSource(MediaStream *mediaStream);
    virtual void reset();
    virtual void release();
    virtual void flush(bool  holdPicture);
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
    virtual void setLooping(bool looping) {TRLS_UNUSED(looping);}
    virtual Connector connect(const ConnectSettings& settings);
    virtual void disconnect(const Connector& connector);
    virtual std::string getAvailablePlaybackRate() {
        return std::string("");
    }

    //IPushTSSource
    virtual void resumeFromPause();
    virtual E_PUMPBUF_STATE pushMediaChunk(const IMedia::DataBuffer& buffer);
    virtual uint32_t getLastFifoLevel()  {
        return _lastPumpLevel;
    }
    virtual tagBMEPushTSEngineContext* getContext() {
        return _context;
    }
    virtual void onPrepared();

private:
    void init();
    void uninit();
    void acquireResources();
    void postBufferLevel();
    bool isRunning();
    void resetPlayState();
    void updatePlayTimeLocked();
    IMedia::State getState() {
        return _state;
    }
    void  stopPlayback(bool holdPicture);
    uint32_t getCurrentPts(bool &decoderTime);
    SourceConnector* getConnector();

    void onSeekComplete();
    void onError(const IMedia::ErrorType& errorType);
    void onCompletion();
    void onInfo(const IMedia::InfoType& infoType, int32_t extra);
    void onVideoSizeChanged(uint16_t width, uint16_t height);
    void dataCallback();
    static void staticDataCallback(void *context, int param);
    void errorCallback();
    static void staticErrorCallback(void *context, int param);

    BMEPushTSEngineContext* _context;
    std::thread             _prepareAsyncThread;
    bool                    _endOfStream;
    uint32_t                _targetPts; /* timestamp we are waiting for */
    uint32_t                _lastPumpLevel;
    E_PUMPBUF_STATE         _pumpBufState;
    bool                    _nexusFirstPtsPassed;
    bool                    _nexusDataExhausted;
    bool                    _inAccurateSeek;
    int                     _playbackRate;
    IMedia::State                   _state;
    /* set to false when host starts playback asd sets to true
       if playTime updated based on the decoder's PTS */
    bool                    _playTimeUpdated;
    /* set to false when host starts playback and sets to true
       if playTime updated based on the data PTS */
    bool                    _decodeTimeUpdated;
    /* last known good position */
    uint32_t                _lastPlayTime;
    /* last PTS send to the decoder */
    uint32_t                _lastDecodeVideoTime;
    bool                    _videoPaused;
};

}  // namespace Broadcom
}
#endif  // __IPUSHTSSOURCEIMPL_H__
#endif // BME_OVER_GSTREAMER
