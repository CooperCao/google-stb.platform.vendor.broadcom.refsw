/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <gtest/gtest.h>
#include "nxclient.h"
#include "nexus_surface_client.h"
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"
#define mosaicCount 4

namespace Broadcom
{
namespace Media
{
TRLS_DBG_MODULE(NetworkSourceTest);

const char gMediaTS[] = "/cnnticker.mpg";
const char gMediaMp4[] = "/Chrome_ImF.mp4";
const char gMediaMosaic[] = "/aliceinwonderland_AVC_Mosaic.ts";
const char gMediaMp3[] = "/O-MP3-8.mp3";
const char gMediaLpcm[] = "/B-LPCM-1.pcm";

class NetworkSourceTest
    : public ::testing::Test,
    public MediaPlayerListener
{
public:
    NetworkSourceTest() :
        _mediaPlayer(NULL),
        _gotEndofStream(false),
        _gotPrepared(false),
        _gotError(false) {
        BME_DEBUG_ENTER();
        BME_DEBUG_EXIT();
    }

    virtual void SetUp() {
        BME_DEBUG_ENTER();
        updateTestURL();
        _gotEndofStream = false;
        _gotPrepared = false;
        _gotError = false;
        BME_DEBUG_EXIT();
    }

    virtual void TearDown() {
        BME_DEBUG_ENTER();
        BME_DEBUG_EXIT();
    }

    void updateTestURL() {
        BME_DEBUG_ENTER();
        std::string ipAddress;
        // TODO(dliu): Switch to use own server
        // getIPAddress(ipAddress);
        ipAddress = "10.13.134.104/~dliu/media/";
        testURL1 = IMedia::NETWORK_HTTP_URI_PREFIX + ipAddress + gMediaTS;
        testURL2 = IMedia::NETWORK_HTTP_URI_PREFIX + ipAddress + gMediaMp4;
        testMosaicURL = IMedia::NETWORK_HTTP_URI_PREFIX + ipAddress + gMediaMosaic;
        testMp3URL = IMedia::NETWORK_HTTP_URI_PREFIX + ipAddress + gMediaMp3;
        testLpcmUrl = IMedia::NETWORK_HTTP_URI_PREFIX + ipAddress + gMediaLpcm;
        BME_DEBUG_EXIT();
    }

    void startVideo(std::string url, bool async = false) {
        BME_DEBUG_ENTER();
        _mediaPlayer = new MediaPlayer;
        this->observe(_mediaPlayer);

        Broadcom::Media::MediaStream mediaStream(url);
        IMedia::StreamMetadata metadata;
        metadata.videoParam.maxHeight = 0;
        metadata.videoParam.maxWidth = 0;
        mediaStream.metadata = metadata;
        _mediaPlayer->setDataSource(&mediaStream);

        if (async) {
            _mediaPlayer->prepareAsync();
            sleep(3);  // give time to prepare
            ASSERT_TRUE(_gotPrepared);
        } else {
            _mediaPlayer->prepare();
        }

        ASSERT_EQ(_mediaPlayer->getState(), IMedia::StoppedState);
        _gotEndofStream = false;
        _mediaPlayer->start();

        // make sure it started properly
        if (_gotError) {
            BME_DEBUG_TRACE(("NetworkSourceTest: Got error!!!\n"));
        }
        BME_DEBUG_EXIT();
    }

    void playVideo(std::string url, uint32_t playDuration = 5, bool looping = false) {
        BME_DEBUG_ENTER();
        startVideo(url);
        _mediaPlayer->setLooping(looping);
        sleep(playDuration);
        stopVideo();
        BME_DEBUG_EXIT();
    }

    void playVideoToEnd(std::string url, uint32_t timeout) {
        BME_DEBUG_ENTER();
        unsigned positionExpected = 10;
        unsigned timeElapsed = 0;
        startVideo(url);
        sleep(3);
        _mediaPlayer->seekTo(10000);
        ASSERT_GT(_mediaPlayer->getCurrentPosition(), positionExpected);

        while (_gotEndofStream != true) {
            sleep(1);
            timeElapsed++;
            if (timeElapsed > timeout)
                break;
        }
        ASSERT_TRUE(_gotEndofStream);
        _mediaPlayer->stop();
        ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StoppedState);
        stopVideo();
        BME_DEBUG_EXIT();
    }

    void stopVideo() {
        BME_DEBUG_ENTER();
        _mediaPlayer->stop();
        ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StoppedState);
        _mediaPlayer->release();
        delete _mediaPlayer;
        BME_DEBUG_EXIT();
    }

    void pauseVideo() {
        BME_DEBUG_ENTER();
        _mediaPlayer->pause();
        BME_DEBUG_EXIT();
    }

    void resumeVideo() {
        BME_DEBUG_ENTER();
        _mediaPlayer->start();
        BME_DEBUG_EXIT();
    }

    uint32_t getPosition() {
        BME_DEBUG_ENTER();
        BME_DEBUG_EXIT();
        return _mediaPlayer->getCurrentPosition();
    }

    MediaPlayer::State getState() {
        BME_DEBUG_ENTER();
        BME_DEBUG_EXIT();
        return _mediaPlayer->getState();
    }

    void setTrickMode(std::string rate) {
        BME_DEBUG_ENTER();
        std::string list = _mediaPlayer->getAvailablePlaybackRate();
        BME_DEBUG_TRACE(("list = %s", list.c_str()));
        _mediaPlayer->setPlaybackRate(rate);
        BME_DEBUG_EXIT();
    }

    virtual void onCompletion() {
        _mediaPlayer->stop();
        _gotEndofStream = true;
    }

    virtual void onError(const IMedia::ErrorType& errorType) {
        TRLS_UNUSED(errorType);
        _gotError = true;
    }

    virtual void onPrepared() {
        _gotPrepared = true;
    }

    virtual void onInfo(const IMedia::InfoType& infoType, int32_t extra) {
        TRLS_UNUSED(infoType);
        TRLS_UNUSED(extra);
    }
    virtual void onSeekComplete() { }
    virtual void onVideoSizeChanged(uint16_t width, uint16_t height) {
        _width = width;
        _height = height;
    }

    virtual void onRateChanged() {
    }

public:
    MediaPlayer* _mediaPlayer;
    bool _gotEndofStream;
    bool _gotPrepared;
    bool _gotError;
    uint16_t _width;
    uint16_t _height;

    std::string testURL1;
    std::string testURL2;
    std::string testMosaicURL;
    std::string testMp3URL;
    std::string testLpcmUrl;
    std::string resolveSource;
};

TEST_F(NetworkSourceTest, BasicTest)
{
    BME_DEBUG_ENTER();
    uint32_t width = 500;
    uint32_t height = 400;
    uint32_t positionExpected = 3;
    startVideo(testURL2);
    // Test set and get video window position
    _mediaPlayer->setVideoWindowPosition(0, 0, width, height);
    ASSERT_EQ(_mediaPlayer->getVideoDisplayWidth(), width);
    ASSERT_EQ(_mediaPlayer->getVideoDisplayHeight(), height);
    sleep(5);
    _mediaPlayer->getDuration();
    ASSERT_GT(_mediaPlayer->getCurrentPosition(), positionExpected);
    ASSERT_EQ(_mediaPlayer->getState(), IMedia::StartedState);
    pauseVideo();
    sleep(5);
    ASSERT_EQ(_mediaPlayer->getState(), IMedia::PausedState);
    resumeVideo();
    ASSERT_EQ(_mediaPlayer->getState(), IMedia::StartedState);
    sleep(5);
    // Test Set visibility
    _mediaPlayer->setVideoWindowVisibility(false);
    sleep(1);
    _mediaPlayer->setVideoWindowVisibility(true);
    stopVideo();
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, VideoSizeChangedCallbackTest)
{
    BME_DEBUG_ENTER();
    startVideo(testURL2, true);
    sleep(3);
    ASSERT_EQ(_width, 640);
    ASSERT_EQ(_height, 320);
    stopVideo();
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, ExternalVideoWindowTest)
{
    // Test setting visibility before calling start doesn't crash
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);

    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle surfaceClient;
    NxClient_AllocSettings allocSettings;
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    NxClient_Alloc(&allocSettings, &allocResults);
    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);

    Broadcom::Media::MediaStream mediaStream(testURL1);
    mediaStream.setSurfaceClientId(allocResults.surfaceClient[0].id);
    mediaStream.setWindowId(0);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->start();
    sleep(5);
    mediaPlayer->stop();
    ASSERT_EQ(mediaPlayer->getState(), MediaPlayer::StoppedState);
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, ExternalMosaicsVideoWindowTest)
{
    // Play mosaic, then single video then back to mosaics
    // can only do 8 due to nxserver limitations
    MediaPlayer* mediaPlayers[mosaicCount];
    NEXUS_SurfaceClientHandle videoSurface[mosaicCount];
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle surfaceClient;
    NxClient_AllocSettings allocSettings;
    BME_DEBUG_ENTER();
    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    NxClient_Alloc(&allocSettings, &allocResults);
    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    for (int i = 0; i < mosaicCount; i++) {
        videoSurface[i] = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, i);
    }

    IMedia::MediaPlayerOptions options;
    options.decoderType = IMedia::DecoderType::Mosaic;
    options.primingMode = false;

    Broadcom::Media::MediaStream mediaStream(testMosaicURL);
    IMedia::StreamMetadata metadata;
    metadata.videoParam.maxHeight = 0;
    metadata.videoParam.maxWidth = 0;
    mediaStream.metadata = metadata;

    // pass extern surfaceClient and window id
    // First pass, use capture mode, so nothing will show on the screen
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i] = new MediaPlayer();
        this->observe(mediaPlayers[i]);
        mediaStream.setSurfaceClientId(allocResults.surfaceClient[0].id);
        mediaStream.setWindowId(i);
        mediaPlayers[i]->setOptions(options);
        mediaPlayers[i]->setDataSource(&mediaStream);
        mediaPlayers[i]->prepare();
        mediaPlayers[i]->start();
        //mediaPlayers[i]->setVideoWindowPosition(i * 45, i * 45, 100, 100);
        NEXUS_SurfaceClientSettings settings;
        NEXUS_SurfaceClient_GetSettings(videoSurface[i], &settings);
        settings.composition.position.width = 100;
        settings.composition.position.height = 100;
        settings.composition.position.x = i * 45;
        settings.composition.position.y = i * 45;
        settings.composition.virtualDisplay.width = 1280;
        settings.composition.virtualDisplay.height = 720;
        NEXUS_SurfaceClient_SetSettings(videoSurface[i], &settings);
    }
    sleep(5);
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i]->stop();
        mediaPlayers[i]->release();
        delete mediaPlayers[i];
    }
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, VisibilityTest)
{
    // Test setting visibility before calling start doesn't crash
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream;
    // test default values
    ASSERT_EQ(mediaStream.getVirtualWidth(), 1280);
    ASSERT_EQ(mediaStream.getVirtualHeight(), 720);
    mediaStream.setUri(testURL1);
    IMedia::StreamMetadata metadata;
    metadata.videoParam.maxHeight = 0;
    metadata.videoParam.maxWidth = 0;
    mediaStream.metadata = metadata;
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->setVideoWindowVisibility(false);
    mediaPlayer->prepare();
    mediaPlayer->start();
    sleep(5);
    mediaPlayer->setVideoWindowVisibility(true);
    sleep(5);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, AsyncTest)
{
    BME_DEBUG_ENTER();
    startVideo(testURL2, true);
    sleep(5);
    stopVideo();
    BME_DEBUG_EXIT();
}

// This test makes sure end of stream callback is triggered correctly
TEST_F(NetworkSourceTest, PlayVideoToEnd)
{
    BME_DEBUG_ENTER();
    playVideoToEnd(testURL1, 30);
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, LoopingMp3Test)
{
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);

    Broadcom::Media::MediaStream mediaStream(testMp3URL);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->setLooping(true);
    mediaPlayer->start();
    sleep((mediaPlayer->getDuration() / 1000) + 5);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StartedState);
    mediaPlayer->stop();
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

// This test checks looping property works as advertised
TEST_F(NetworkSourceTest, LoopingTest)
{
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    _mediaPlayer = mediaPlayer;
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL1);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->setLooping(false);
    mediaPlayer->start();
    sleep((mediaPlayer->getDuration() / 1000) + 5);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    mediaPlayer->stop();
    // manually started
    mediaPlayer->start();
    sleep(5);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StartedState);
    mediaPlayer->stop();
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    mediaPlayer->prepare();
    mediaPlayer->start();
    mediaPlayer->setLooping(true);
    sleep((mediaPlayer->getDuration() / 1000) + 5);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StartedState);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, PlaybackCompletedtoPlaybackCompletedState)
{
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    _mediaPlayer = mediaPlayer;
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL1);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->setLooping(false);
    mediaPlayer->start();
    sleep((mediaPlayer->getDuration() / 1000) + 5);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    mediaPlayer->seekTo(5000);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, PausedToStopTest)
{
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    _mediaPlayer = mediaPlayer;
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL1);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->setLooping(false);
    mediaPlayer->start();
    sleep(5);
    mediaPlayer->pause();
    ASSERT_EQ(mediaPlayer->getState(), IMedia::PausedState);
    mediaPlayer->stop();
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

// This test checks if app tries to do looping. The difference is that app will receive
// end of stream callback and it will immediately invoke a start call.
// Really it is testing StoppedState to Started State
TEST_F(NetworkSourceTest, CompleteToStartTest)
{
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    _mediaPlayer = mediaPlayer;
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL1);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->setLooping(false);
    mediaPlayer->start();
    sleep((mediaPlayer->getDuration() / 1000) + 5);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    mediaPlayer->start();
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StartedState);
    sleep(3);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, PipTest)
{
    /*
    BME_DEBUG_ENTER();
    // Start main video
    startVideo(testURL1);
    sleep(5);
    MediaPlayer* pipPlayer = new MediaPlayer();
    _mediaPlayer = pipPlayer;
    this->observe(pipPlayer);
    IMedia::MediaPlayerOptions options;
    options.decoderType = IMedia::DecoderType::Pip;
    options.primingMode = false;
    pipPlayer->setOptions(options);
    Broadcom::Media::MediaStream mediaStream(testURL2);
    pipPlayer->setDataSource(&mediaStream);
    pipPlayer->prepare();
    pipPlayer->setVideoWindowPosition(0, 0, 400, 300);
    pipPlayer->start();
    sleep(15);
    pipPlayer->stop();
    pipPlayer->release();
    delete pipPlayer;
    // Stop main video
    stopVideo();
    BME_DEBUG_EXIT();
    */
}

TEST_F(NetworkSourceTest, MosaicAndPipTestCycle)
{
    BME_DEBUG_ENTER();
    // Play 3 mosaic and 1 PIP
    // Simulates multiPip setup
    MediaPlayer* mediaPlayers[mosaicCount];
    Broadcom::Media::MediaStream mediaStream(testMosaicURL);
    IMedia::StreamMetadata metadata;
    metadata.videoParam.maxHeight = 0;
    metadata.videoParam.maxWidth = 0;
    mediaStream.metadata = metadata;
    IMedia::MediaPlayerOptions options;
    options.primingMode = false;
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i] = new MediaPlayer();
        this->observe(mediaPlayers[i]);

        if (i == (mosaicCount - 1))
            options.decoderType = IMedia::DecoderType::Pip;
        else
            options.decoderType = IMedia::DecoderType::Mosaic;
        mediaPlayers[i]->setOptions(options);
        mediaPlayers[i]->setDataSource(&mediaStream);
        mediaPlayers[i]->prepare();
        mediaPlayers[i]->setVideoWindowPosition((i % 2) * 640, (i / 2) * 360, 640, 360);
        mediaPlayers[i]->start();
    }
    sleep(5);
    for (int j = 0; j < mosaicCount; j++) {
        mediaPlayers[j]->stop();
        mediaPlayers[j]->release();
        delete mediaPlayers[j];

        mediaPlayers[j] = new MediaPlayer();
        this->observe(mediaPlayers[j]);
        options.decoderType = IMedia::DecoderType::Mosaic;
        mediaPlayers[j]->setOptions(options);
        mediaPlayers[j]->setDataSource(&mediaStream);
        mediaPlayers[j]->prepare();
        mediaPlayers[j]->start();
        mediaPlayers[j]->setVideoWindowPosition((j % 2) * 640, (j / 2) * 360, 640, 360);
        sleep(5);
    }
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i]->stop();
        mediaPlayers[i]->release();
        delete mediaPlayers[i];
    }
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, MosaicTestCycle)
{
    BME_DEBUG_ENTER();
    // Play mosaic, then stop and start each one
    MediaPlayer* mediaPlayers[mosaicCount];
    Broadcom::Media::MediaStream mediaStream(testMosaicURL);
    IMedia::StreamMetadata metadata;
    metadata.videoParam.maxHeight = 0;
    metadata.videoParam.maxWidth = 0;
    mediaStream.metadata = metadata;

    IMedia::MediaPlayerOptions options;
    options.primingMode = false;
    options.decoderType = IMedia::DecoderType::Mosaic;

    // First pass, use capture mode, so nothing will show on the screen
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i] = new MediaPlayer();
        this->observe(mediaPlayers[i]);
        mediaPlayers[i]->setOptions(options);
        mediaPlayers[i]->setDataSource(&mediaStream);
        mediaPlayers[i]->prepare();
        mediaPlayers[i]->start();
        mediaPlayers[i]->setVideoWindowPosition(i * 45, i * 45, 100, 100);
    }
    sleep(5);
    for (int j = 0; j < mosaicCount; j++) {
        mediaPlayers[j]->stop();
        mediaPlayers[j]->release();
        delete mediaPlayers[j];
        mediaPlayers[j] = new MediaPlayer();
        this->observe(mediaPlayers[j]);
        mediaPlayers[j]->setOptions(options);
        mediaPlayers[j]->setDataSource(&mediaStream);
        mediaPlayers[j]->prepare();
        mediaPlayers[j]->start();
        mediaPlayers[j]->setVideoWindowPosition(j * 45, j * 45, 100, 100);
        sleep(5);
    }
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i]->stop();
        mediaPlayers[i]->release();
        delete mediaPlayers[i];
    }
    BME_DEBUG_EXIT();
}

// This is the full mosaic test. It sets up a mosaic playback, then switches to regular
// decode then switches back to mosaic again
TEST_F(NetworkSourceTest, MosaicTest)
{
    BME_DEBUG_ENTER();
    // Play mosaic, then single video then back to mosaics
    // can only do 10 due to miracast source taking 2 playpump
    MediaPlayer* mediaPlayers[mosaicCount];
    Broadcom::Media::MediaStream mediaStream(testMosaicURL);
    IMedia::StreamMetadata metadata;
    metadata.videoParam.maxHeight = 0;
    metadata.videoParam.maxWidth = 0;
    mediaStream.metadata = metadata;

    IMedia::MediaPlayerOptions options;
    options.primingMode = false;
    options.decoderType = IMedia::DecoderType::Mosaic;

    // First pass, use capture mode, so nothing will show on the screen
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i] = new MediaPlayer();
        this->observe(mediaPlayers[i]);

        mediaPlayers[i]->setOptions(options);
        mediaPlayers[i]->setDataSource(&mediaStream);
        mediaPlayers[i]->prepare();
        mediaPlayers[i]->start();
        mediaPlayers[i]->setVideoWindowPosition(i * 45, i * 45, 100, 100);
    }
    sleep(5);
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i]->stop();
        mediaPlayers[i]->release();
        delete mediaPlayers[i];
    }
    // switch to non-mosaics mode
    playVideo(testURL2);
    // Play mosaics again without capture mode
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i] = new MediaPlayer();
        this->observe(mediaPlayers[i]);

        mediaPlayers[i]->setOptions(options);
        mediaPlayers[i]->setDataSource(&mediaStream);
        mediaPlayers[i]->prepare();
        mediaPlayers[i]->start();
        mediaPlayers[i]->setVideoWindowPosition(i * 45, i * 45, 100, 100);
    }
    sleep(10);
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i]->stop();
        mediaPlayers[i]->release();
        delete mediaPlayers[i];
    }
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, MosaicTestReverseClose)
{
    BME_DEBUG_ENTER();
    // Play mosaic, then single video then back to mosaics
    // can only do 10 due to miracast source taking 2 playpump
    MediaPlayer* mediaPlayers[mosaicCount];
    Broadcom::Media::MediaStream mediaStream(testMosaicURL);
    IMedia::StreamMetadata metadata;
    metadata.videoParam.maxHeight = 0;
    metadata.videoParam.maxWidth = 0;
    mediaStream.metadata = metadata;

    IMedia::MediaPlayerOptions options;
    options.primingMode = false;
    options.decoderType = IMedia::DecoderType::Mosaic;

    // First pass, use capture mode, so nothing will show on the screen
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i] = new MediaPlayer();
        this->observe(mediaPlayers[i]);

        mediaPlayers[i]->setOptions(options);
        mediaPlayers[i]->setDataSource(&mediaStream);
        mediaPlayers[i]->prepare();
        mediaPlayers[i]->start();
        mediaPlayers[i]->setVideoWindowPosition(i * 45, i * 45, 100, 100);
    }
    sleep(5);
    for (int i = mosaicCount-1; i >= 0 ; i--) {
        mediaPlayers[i]->stop();
        mediaPlayers[i]->release();
        delete mediaPlayers[i];
    }
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, SeekTest)
{
    BME_DEBUG_ENTER();
    uint32_t positionExpected = 20000;
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL2);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    // test initial seek
    mediaPlayer->seekTo(positionExpected);
    mediaPlayer->start();
    sleep(3);
    ASSERT_GT(mediaPlayer->getCurrentPosition(), positionExpected);
    int duration = mediaPlayer->getDuration();
    // seek back to beginning
    mediaPlayer->seekTo(0);
    sleep(3);  // for visual feedback
    ASSERT_LT(mediaPlayer->getCurrentPosition(), positionExpected);
    // Seek to outside of duration. This will trigger end of stream
    mediaPlayer->seekTo(duration + 3000);
    sleep(3);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, DurationTest)
{
    BME_DEBUG_ENTER();
    uint32_t duration = 20000;
    uint32_t mediaURLDuration = 34200;
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL2);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->start();
    ASSERT_EQ(mediaPlayer->getDuration(), mediaURLDuration);
    mediaPlayer->stop();
    // Apps passes in duration to media player
    mediaStream.setDuration(duration);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->start();
    ASSERT_EQ(mediaPlayer->getDuration(), duration);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, TrickModeTest)
{
    BME_DEBUG_ENTER();
    startVideo(testMp3URL);
    sleep(1);
    // This is not supported so make sure it doesn't crash
    setTrickMode("4");
    sleep(1);
    stopVideo();
    startVideo(testURL1);
    setTrickMode("2");
    sleep(5);
    // Need to consider rewind operation in NetwordSource.cpp
    setTrickMode("-2");
    sleep(2);
    setTrickMode("1");
    sleep(5);
    ASSERT_EQ(_mediaPlayer->getState(), IMedia::StartedState);
    stopVideo();
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, PrimingToResetTest)
{
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL1);
    IMedia::MediaPlayerOptions options;
    options.primingMode = true;
    options.decoderType = IMedia::DecoderType::Main;

    mediaPlayer->setOptions(options);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepareAsync();
    sleep(5);
    mediaPlayer->reset();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, IOErrorTest)
{
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);

    Broadcom::Media::MediaStream mediaStream(std::string("http://10.13.134.104/fake_url"));
    mediaPlayer->setDataSource(&mediaStream);
    IMedia::ErrorType ret = mediaPlayer->prepare();
    ASSERT_NE(ret, IMedia::MEDIA_SUCCESS);
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, StopPictureTest)
{
    BME_DEBUG_ENTER();
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL1);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->start();
    sleep(2);
    mediaPlayer->stop(true);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    sleep(2);
    mediaStream.setUri(testURL2);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->start();
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StartedState);
    sleep(2);
    mediaPlayer->stop();
    mediaStream.setUri(testURL1);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->start();
    sleep(2);
    mediaPlayer->stop(true);
    ASSERT_EQ(mediaPlayer->getState(), IMedia::StoppedState);
    sleep(2);
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}

TEST_F(NetworkSourceTest, prepareAsyncToReset)
{
    BME_DEBUG_ENTER();
    // to run this test properly, need to insert 1 second delay in SessionSetup call
    // in playback_ip
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL2);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepareAsync();
    sleep(3);
    mediaPlayer->reset();
    mediaPlayer->release();
    delete mediaPlayer;
    BME_DEBUG_EXIT();
}
}  // namespace Media
}  // namespace Broadcom
