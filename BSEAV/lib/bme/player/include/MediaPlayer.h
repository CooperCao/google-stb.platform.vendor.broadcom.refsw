/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifndef __IMEDIAPLAYERIMPL_H__
#define __IMEDIAPLAYERIMPL_H__

#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Observable.h"
#include "MediaStream.h"
#include "Media.h"
#include "HdrMetadata.h"

namespace Broadcom
{
namespace Media {

class SimpleDecoder;
class BaseSource;
class SourceConnector;
class ISubtitle;

typedef struct tagMediaPlayerContext MediaPlayerContext;

#define TRLS_MEDIA_DEBUG_ENTER() bmeDebugPrint(DebugTrace, bme_module_name, \
        bme_module_name, __LINE__, "(%p) ENTER %s (state is %s)", \
        this, __FUNCTION__, printState().c_str())
#define TRLS_MEDIA_DEBUG_EXIT() bmeDebugPrint(DebugTrace, bme_module_name, \
        bme_module_name, __LINE__, "(%p) EXIT %s (state is %s)", \
        this, __FUNCTION__, printState().c_str())

enum class MediaPlayerEvents {
    Completed,
    Prepared,
    Error,
    Info,
    SeekCompleted,
    VideoSizeChanged,
    PlaybackRateChanged,
    Beginning
};

enum CaptureSurfaceFormat {
    RGB565 = 0,
    RGBX8888
};

struct DecoderStatus {
    /* buffer status */
    unsigned fifoDepth;       /* depth in bytes of the compressed buffer */
    unsigned fifoSize;        /* size in bytes of the compressed buffer */
    unsigned queueDepth;      /* the number of decoded pictures currently ready to be displayed */
    unsigned cabacBinDepth;   /* depth in bytes of the cabac bin buffer */
    unsigned enhancementFifoDepth; /* depth in bytes of the enhancement compressed buffer */
    unsigned enhancementFifoSize; /* size in bytes of the enhancement compressed buffer */
    /* unsigned bufferLatency was moved for performance reasons.  See NEXUS_VideoDecoderFifoStatus.bufferLatency. */

    /* cumulative status */
    unsigned numDecoded;      /* total number of decoded pictures since Start */
    unsigned numDisplayed;    /* total number of display pictures since Start */
    unsigned numIFramesDisplayed; /* total number of displayed I-Frames pictures since Start */
    unsigned numDecodeErrors; /* total number of decoder errors since Start */
    unsigned numDecodeOverflows; /* total number of overflows of the input to the decoder since Start */
    unsigned numDisplayErrors;/* total number of display manager errors since Start. This
                             includes parity mismatches which may result in glitching on the display. */
    unsigned numDecodeDrops;  /* total number of pictures dropped by the decoder since Start */
    unsigned numPicturesReceived; /* total number of pictures which the decoder has received since Start.
                            This includes pictures which are skipped due to TSM, NEXUS_VideoDecoderDecodeMode,
                            or other factors. */
    unsigned numDisplayDrops; /* total number of pictures dropped by the display manager due to TSM failures since Start */
    unsigned numDisplayUnderflows;  /* total number of times the display manager was unable to
                            deliver a new picture since Start */
    unsigned ptsErrorCount;   /* counter for number of PTS error interrupts since Start */
    unsigned avdStatusBlock;  /* snap shot of the AVD channel status. See NEXUS_CHANNELSTATUS_AVD_XXX
                            macros for possible values. */
    unsigned numWatchdogs;    /* total number of watchdog events for the device since NEXUS_VideoDecoderModule_Init.
                            the count is per AVD device, not per VideoDecoder channel. */
    uint64_t numBytesDecoded; /* total number of ES bytes decoded since last start or flush */
    unsigned fifoEmptyEvents; /* total number of fifoEmpty events since last start or flush */
    unsigned fifoNoLongerEmptyEvents; /* total number of fifoNoLongerEmpty events since last start or flush.
                            If fifoEmptyEvents > fifoNoLongerEmptyEvents, then fifo is still empty */
};

struct AudioVolumeBase;
class BME_SO_EXPORT MediaPlayer
    : public IMedia,
    public Observable<MediaPlayerEvents>
{
public:
    MediaPlayer();
    virtual ~MediaPlayer();

    void setOptions(const MediaPlayerOptions& options);
    void start();
    void stop(bool holdLastFrame = false);
    void pause();
    IMedia::ErrorType prepare();
    void prepareAsync();
    void setDataSource(MediaStream* mediaStream);

    void reset();
    void release();
    std::string getType();
    BaseSource* getSource();
    void seekTo(uint32_t msec,
                           IMedia::PlaybackOperation playOperation = IMedia::OperationForward,
                           IMedia::PlaybackSeekTime  playSeekTime = IMedia::SeekTimeAbsolute);
    void setPlaybackRate(const std::string& rate);
    int getPlaybackRate();
    IMedia::PlaybackOperation getPlaybackOperation();
    std::string getAvailablePlaybackRate();
    uint32_t getCurrentPosition();
    uint64_t getDuration();
    const IMedia::TimeInfo getTimeInfo();
    bool isPlaying();
    bool isLooping();
    void setLooping(bool looping);
    TIME45k getCurrentPts();
    uint32_t getVideoHeight();
    uint32_t getVideoWidth();
    uint32_t getVideoDisplayHeight();
    uint32_t getVideoDisplayWidth();
    IMedia::VideoFrameRate getVideoFrameRate();
    IMedia::VideoAspectRatio getVideoAspectRatio();
    IMedia::AudioParametersList getTrackInfo();
    int32_t getTrack();
    void selectTrack(int32_t uid);
    void selectAudioParameters(const IMedia::AudioParameters& param);
    int32_t getAudioVolume();
    void setAudioVolume(int32_t volume);
    AudioVolumeBase *getAudioControl() const
    {
        return _audioVolumeBase;
    }
    void mute();
    void unmute();
    uint8_t getAudioDescriptionVolume();
    void setAudioDescriptionVolume(const uint8_t &volume);
    void setSubtitleLanguage(const std::string& language);
    void setSubtitle(bool enable);
    void setSubtitleVisibility(bool visible);
    bool getSubtitleVisibility();

    void setVideoWindowPosition(int32_t x, int32_t y, uint32_t width, uint32_t height,
                                        int32_t videoStreamIndex = -1);
    void setVideoWindowVisibility(bool visible, int32_t videoStreamIndex = -1);
    void animateVideoWindowPosition(const IMedia::Rect& start, const IMedia::Rect& end, uint32_t duration);
    IMedia::State getState();
    IMedia::VideoCodec getVideoCodec();
    IMedia::AudioCodec getAudioCodec();
    uint16_t getVideoPid();
    uint16_t getVideoPcrPid();
    uint16_t getVideoColorDepth();
    uint16_t getAudioPid();
    virtual void startFrameCapture(CaptureSurfaceFormat pixelFormat,
                                    uint32_t decodeWidth,
                                    uint32_t decodeHeight,
                                    bool displayEnabled,
                                    void *surfaceAddress,
                                    bool secure);
    void stopFrameCapture();
    size_t getFrame(uint32_t count = 1, bool flip = false);
    DecoderStatus getDecoderStatus();
    void setHdrMetadata(const HDRMetadata hdrMetadata);

    void onChannelChange();
    void onSTCRequest(uint32_t* stc);
    void onCompletion();
    void onError(const IMedia::ErrorType& errorType);
    void onPrepared();
    void onInfo(const IMedia::InfoType& infoType, int32_t extra);
    void onSeekComplete();
    void onVideoSizeChanged(uint16_t width, uint16_t height);
    void onBeginning();
    void onHdrSettings(const HDRMetadata hdrMetadata);
    bool isPipOrMosaic();
    void resetVariables();
    void animatePosition(uint32_t duration,
            const IMedia::Rect& start, const IMedia::Rect& end);

    void releaseVideoDecoder();

    virtual void SourceonCompletion();
    virtual void SourceonError(const IMedia::ErrorType& errorType) { TRLS_UNUSED(errorType);}
    virtual void SourceonPrepared();
    virtual void SourceonInfo(const IMedia::InfoType& infoType, int32_t extra) {TRLS_UNUSED(infoType); TRLS_UNUSED(extra);}
    virtual void SourceonSeekComplete() {}
    virtual void SourceonVideoSizeChanged(uint16_t width, uint16_t height) {TRLS_UNUSED(width); TRLS_UNUSED(height);}

protected:
    std::string printState();
    void releaseSource();
    bool _usePrimer;

private:
    SourceConnector *_connector;
    void disableMvcMode();
    void acquireResources();
    void releaseResources();
    void stopDecoder();
    void startDecoder();
    void reloadPidChannels();
    void stopPrimer();
    void init();
    void uninit();
    void startInternal();
    void setDecoderStcChannel();
    void addSourceListener();
    void addMediaStreamListener();
    void removeMediaStreamListener();
    void startSubtitle();
    void stopSubtitle();

    IMedia::State _state;
    bool _looping;
    bool _controlsAudio;
    bool _resourceAcquired;
    bool _videoPositionPreset;
    bool _decoderReleased;
    bool _capturing;
    bool _keepCapturing;
    uint32_t _initialSeekTimeMSec;
    uint32_t _previousPositionMSec;
    int32_t _x;
    int32_t _y;
    uint32_t _width;
    uint32_t _height;
    uint16_t _virtualWidth;
    uint16_t _virtualHeight;
    bool _visible;
    SimpleDecoder* _simpleDecoder;
    AudioVolumeBase* _audioVolumeBase;
    StreamMetadata _streamMetadata;
    MediaPlayerContext* _context;
    BaseSource* _source;
    MediaStream* _mediaStream;
    VideoParameters _videoParam;
    AudioParameters _audioParam;
    uint32_t captureSurfaceAddress;
    bool _pictureHeld;
    bool _sourceOverrided;
    uint32_t _surfaceClientId;
    uint32_t _windowId;
    MediaPlayerOptions _options;
    ISubtitle* _subtitle;
    bool _subtitleVisible;
    bool _subtitleEnabled;
    std::string _uri;

    std::string _rate;
    // for animating video
    std::thread _animationThread;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic<int> _exit;

    std::shared_ptr<void> _channelChangeListener;
    std::shared_ptr<void> _stcRequestedListener;
    std::string _mediaStreamId;
    HDRMetadata _hdrMetadata;
    bool _isHdrVideo;
    bool _svp;
    bool _disabledProgressiveOverride;
};
}  // namespace Media
}  // namespace Broadcom
#endif  // __IMEDIAPLAYERIMPL_H__
