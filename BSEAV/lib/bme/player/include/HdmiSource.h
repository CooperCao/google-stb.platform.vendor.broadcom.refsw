/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __HDMISOURCEIMPL_H__
#define __HDMISOURCEIMPL_H__

#include "BaseSource.h"

namespace Broadcom
{
namespace Media {

typedef struct tagBMEHDMIEngineContext  BMEHDMIEngineContext;

class BME_SO_EXPORT HdmiSource :
    public BaseSource
{
public:
    HdmiSource();
    virtual ~HdmiSource();

    // ISource
    virtual void start();
    virtual void stop(bool holdLastFrame = false);
    virtual void pause() { }
    virtual IMedia::ErrorType prepare();
    virtual void prepareAsync() { }
    virtual void setDataSource(MediaStream *mediaStream);
    virtual void reset();
    virtual void release();
    virtual void flush(bool holdPicture) {TRLS_UNUSED(holdPicture);}
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
    virtual uint32_t getCurrentPosition() {return 0;}
    virtual uint64_t getBytesDecoded() {return 0;}
    virtual const IMedia::TimeInfo getTimeInfo();
    virtual void setLooping(bool looping) {TRLS_UNUSED(looping);}
    virtual Connector connect(const ConnectSettings& settings);
    virtual void disconnect(const Connector& connector);
    virtual std::string getAvailablePlaybackRate() {
        return std::string("");
    }

    //IHdmiSource
    virtual tagBMEHDMIEngineContext* getContext() {
        return _context;
    }
    virtual void onPrepared();

private:
    void init();
    void uninit();
    bool isRunning();
    IMedia::State getState() {
        return _state;
    }
    SourceConnector* getConnector();

    void onError(const IMedia::ErrorType& errorType);
    void onCompletion();

    IMedia::State           _state;
    BMEHDMIEngineContext* _context;
};

}  // namespace Broadcom
}
#endif  // __HDMISOURCEIMPL_H__
