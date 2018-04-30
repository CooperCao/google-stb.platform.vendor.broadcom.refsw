/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __BME_SIMPLEDECODERNXCLIENT_H_
#define __BME_SIMPLEDECODERNXCLIENT_H_

#include <map>
#include "SimpleDecoder.h"
#include "Debug.h"
#include "Media.h"
#include <mutex>

namespace Broadcom
{
namespace Media {

class MediaPlayer;
typedef struct tagSimpleDecoderNxclientContext SimpleDecoderNxclientContext;
typedef struct tagMosaicDecodersContext MosaicDecoderContext;

/**
 * @class SimpleDecoderNxclient SimpleDecoderNxclient.h
 *
 * @brief Class to allow BME to work with standard Nexus reference multiprocess server
 */

class SimpleDecoderNxclient;

#ifdef NEXUS_HAS_VIDEO_ENCODER
typedef struct SimpleXcodeHandle { NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
    NEXUS_SimpleEncoderHandle encoder;
    SimpleDecoderNxclientContext* context;
} SimpleXcodeHandle;
#endif


class BME_SO_EXPORT SimpleDecoderNxclient
    : public SimpleDecoder
{
 public:
    SimpleDecoderNxclient();
    ~SimpleDecoderNxclient();

    virtual void init();
    virtual void uninit();

    virtual void acquireSimpleDecoders(const acquireParameters& param,
            NEXUS_SimpleVideoDecoderHandle* videoDecoder,
            NEXUS_SimpleAudioDecoderHandle* audioDecoder);

    virtual void releaseSimpleDecoders(NEXUS_SimpleVideoDecoderHandle videoDecoder,
            NEXUS_SimpleAudioDecoderHandle audioDecoder);

    virtual NEXUS_SimpleAudioPlaybackHandle acquireSimpleAudioPlayback();
    virtual void releaseSimpleAudioPlayback(NEXUS_SimpleAudioPlaybackHandle);

    virtual void setLocation(int x, int y, uint32_t width, uint32_t height);
    virtual void setVisibility(bool visible);
    virtual uint32_t getVideoDisplayWidth();
    virtual uint32_t getVideoDisplayHeight();
    virtual void setAudioVolume(int32_t volume);
    virtual int32_t getAudioVolume();
    virtual void mute(bool muted);

    virtual void connect();
    virtual void disconnect();

#ifdef NEXUS_HAS_VIDEO_ENCODER
    virtual SimpleXcodeHandle* acquireXcodeResource(bool nrtMode, bool audioXcode,
                                                    int maxWidth, int maxHeight,
                                                    int colorDepth, bool hdmiInput);
    virtual void releaseXcodeResource(SimpleXcodeHandle* xcodeHandle);
#endif

 private:
    void freeMosaicsMaster(SimpleDecoderNxclientContext* master);
    uint32_t allocMosaicMaster(SimpleDecoderNxclientContext* master,
            uint32_t windowId, uint16_t maxMosaics);
    uint32_t setupMosaicMode(uint16_t maxWidth, uint16_t maxHeight, uint32_t windowId,
            NEXUS_SimpleVideoDecoderHandle*, IMedia::DecoderType decoderType);
    uint32_t setupRegular(bool requestVideoDecoder, bool requestAudioDecoder, uint16_t maxWidth,
            uint16_t maxHeight, uint16_t virtualWidth, uint16_t virtualHeight, uint16_t colorDepth,
            IMedia::DecoderType decoderType);
    uint32_t _width;
    uint32_t _height;
    uint32_t _virtualWidth;
    uint32_t _virtualHeight;
    uint16_t _self;
    bool _secureVideo;
    bool _persistent;

    typedef std::map<int16_t, SimpleDecoderNxclientContext*> SimpleDecoderMap;
    static SimpleDecoderMap _videoDecoderMap;
    static SimpleDecoderMap _audioDecoderMap;
    static SimpleDecoderMap _audioPlaybackMap;
    static uint16_t _index;
    static uint16_t _mosaicMaster;
    static std::mutex _mutex;
};
}  // namespace Media
}  // namespace Broadcom
#endif  // __BME_SIMPLEDECODERNXCLIENT_H_
