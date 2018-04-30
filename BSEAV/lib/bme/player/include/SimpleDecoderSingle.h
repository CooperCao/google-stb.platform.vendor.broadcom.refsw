/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __BME_SIMPLEDECODERSINGLE_H_
#define __BME_SIMPLEDECODERSINGLE_H_

#include <map>
#include "SimpleDecoder.h"
#include "Singleton.h"

namespace Broadcom
{
namespace Media {

class MediaPlayer;
typedef struct tagSimpleDecoderSingleContext SimpleDecoderSingleContext;

/**
 * @class SimpleDecoderSingle SimpleDecoderSingle.h
 *
 * @brief Class to allow media player to work as a standalone module.
 *
 * This class is used when application was to run without any server process, but
 * still wishes to use media player library. In this case, the application
 * should open the display on it's own and create NEXUS_VideoDecoder as well as
 * NEXUS_SimpleVideoDecoder. This class will use NEXUS_SimpleVideoDecoder to get
 * the video window. This is done to avoid the need to pass NEXUS_Display handle
 * around, which generally needs to be shared between apps and media player
 */
class SimpleDecoderSingle
    : public SimpleDecoder
{
public:
    SimpleDecoderSingle();
    ~SimpleDecoderSingle();

    virtual void init();
    virtual void uninit();

    virtual void acquireSimpleDecoders(const acquireParameters& param,
            NEXUS_SimpleVideoDecoderHandle* videoDecoder,
            NEXUS_SimpleAudioDecoderHandle* audioDecoder);
    virtual void releaseSimpleDecoders(NEXUS_SimpleVideoDecoderHandle videoDecoder,
            NEXUS_SimpleAudioDecoderHandle audioDecoder);
    virtual NEXUS_SimpleAudioPlaybackHandle acquireSimpleAudioPlayback();
    virtual void releaseSimpleAudioPlayback(NEXUS_SimpleAudioPlaybackHandle audioPlayback);

    virtual void setLocation(int x, int y, uint32_t width, uint32_t height);
    virtual void setVisibility(bool visible);
    virtual uint32_t getVideoDisplayWidth();
    virtual uint32_t getVideoDisplayHeight();
    virtual void setAudioVolume(int32_t volume);
    virtual int32_t getAudioVolume();
    virtual void mute(bool muted);

    // TODO(dliu): add support for these
    virtual SimpleXcodeHandle* acquireXcodeResource(bool nrtMode, bool audioXcode,
                                                    int maxWidth, int maxHeight,
                                                    int colorDepth, bool hdmiInput) {return NULL;}
    virtual void releaseXcodeResource(SimpleXcodeHandle* xcodeHandle) {}

    virtual void connect() {}
    virtual void disconnect() {}

private:
    uint16_t _self;
    typedef std::map<uint32_t, SimpleDecoderSingleContext*> SimpleDecoderMap;
    static SimpleDecoderMap _decoderMap;
    static uint16_t _index;
    static uint32_t _displayHeight;
    static uint32_t _displayWidth;
};
}  // namespace Broadcom
}
#endif  // __BME_SIMPLEDECODERSINGLE_H_