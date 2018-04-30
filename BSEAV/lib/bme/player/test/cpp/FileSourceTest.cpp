/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <gtest/gtest.h>
#include <string>
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"

const char gMediaTS[] = "file:///data/media/cnnticker.mpg";
const char gMediaMp4[] = "file:///data/media/Chrome_ImF.mp4";
const char gMediaMosaic[] = "file:///data/media/aliceinwonderland_AVC_Mosaic.ts";
const char gMediaMp3[] = "file:///data/media/O-MP3-8.mp3";

namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(FileSourceTest);

class FileSourceTest
    : public ::testing::Test,
    public MediaPlayerListener
{
 public:
    FileSourceTest() :
        _mediaPlayer(NULL) {
    }

    virtual void SetUp() {
        updateTestURL();
        _gotEndofStream = false;
        _gotPrepared = false;
        _gotError = false;
    }

    virtual void TearDown() {
    }

    void updateTestURL() {
        testURL1 = gMediaTS;
        testURL2 = gMediaMp4;
        testMosaicURL = gMediaMosaic;
        testMp3URL = gMediaMp3;
    }

    void startVideo(std::string url, bool async = false) {
        _mediaPlayer = new MediaPlayer();
        this->observe(_mediaPlayer);
        Broadcom::Media::MediaStream mediaStream(url);
        _mediaPlayer->setDataSource(&mediaStream);

        if (async) {
            _mediaPlayer->prepareAsync();
            sleep(3);  // give time to prepare
            ASSERT_TRUE(_gotPrepared);
        } else {
            _mediaPlayer->prepare();
        }
        ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StoppedState);

        _mediaPlayer->start();

        // make sure it started properly
        if (_gotError) {
            BME_DEBUG_TRACE(("FileSourceTest: Got error!!!\n"));
        }
    }

    void playVideo(std::string url, uint32_t playDuration = 5, bool looping = false) {
        startVideo(url);
        _mediaPlayer->setLooping(looping);
        sleep(playDuration);
        stopVideo();
    }

    void playVideoToEnd(std::string url, uint32_t timeout) {
        unsigned positionExpected = 10;
        startVideo(url);
        sleep(5);
        _mediaPlayer->seekTo(10000);
        ASSERT_GT(_mediaPlayer->getCurrentPosition(), positionExpected);
        _gotEndofStream = false;
        sleep(timeout);
        ASSERT_TRUE(_gotEndofStream);
        stopVideo();
    }

    void stopVideo() {
        _mediaPlayer->stop();
        ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StoppedState);
        _mediaPlayer->release();
        delete _mediaPlayer;
    }

    void pauseVideo() {
        _mediaPlayer->pause();
    }

    void resumeVideo() {
        _mediaPlayer->start();
    }

    uint32_t getPosition() {
        return _mediaPlayer->getCurrentPosition();
    }

    MediaPlayer::State getState() {
        return _mediaPlayer->getState();
    }

    void setTrickMode(std::string rate) {
        _mediaPlayer->setPlaybackRate(rate);
    }

    virtual void onCompletion() {
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

    virtual void onSeekComplete() {
    }

    virtual void onVideoSizeChanged(uint16_t width, uint16_t height) {
        TRLS_UNUSED(width);
        TRLS_UNUSED(height);
    }
    MediaPlayer* _mediaPlayer;
    bool _gotEndofStream;
    bool _gotPrepared;
    bool _gotError;

    std::string testURL1;
    std::string testURL2;
    std::string testMosaicURL;
    std::string testMp3URL;
    std::string resolveSource;
};

TEST_F(FileSourceTest, BasicTest)
{
    uint32_t width = 600;
    uint32_t height = 300;
    startVideo(testURL2);
    // Test set and get video window position
    _mediaPlayer->setVideoWindowPosition(100, 300, width, height);
    ASSERT_EQ(_mediaPlayer->getVideoDisplayWidth(), width);
    ASSERT_EQ(_mediaPlayer->getVideoDisplayHeight(), height);
    sleep(5);
    ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StartedState);
    pauseVideo();
    sleep(5);
    ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::PausedState);
    resumeVideo();
    ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StartedState);
    sleep(5);
    // Test Set visibility
    _mediaPlayer->setVideoWindowVisibility(false);
    sleep(1);
    _mediaPlayer->setVideoWindowVisibility(true);
    stopVideo();
}

TEST_F(FileSourceTest, AsyncTest)
{
    startVideo(testURL2, true);
    sleep(5);
    stopVideo();
}

// This test makes sure end of stream callback is triggered correctly
TEST_F(FileSourceTest, PlayVideoToEnd)
{
    playVideoToEnd(testURL2, 30);
}

TEST_F(FileSourceTest, LoopingTest)
{
    MediaPlayer* mediaPlayer = new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL1);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->setLooping(false);
    mediaPlayer->start();
    sleep((mediaPlayer->getDuration() / 1000) + 5);
    ASSERT_EQ(mediaPlayer->getState(), MediaPlayer::StartedState);
    mediaPlayer->stop();
    ASSERT_EQ(mediaPlayer->getState(), MediaPlayer::StoppedState);
    mediaPlayer->prepare();
    mediaPlayer->start();
    mediaPlayer->setLooping(true);
    sleep((mediaPlayer->getDuration() / 1000) + 5);
    ASSERT_EQ(mediaPlayer->getState(), MediaPlayer::StartedState);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
}

TEST_F(FileSourceTest, PipTest)
{
    // Start main video
    startVideo(testURL1);
    sleep(5);
    MediaPlayer* pipPlayer;
    pipPlayer = new MediaPlayer();
    this->observe(pipPlayer);
    IMedia::MediaPlayerOptions options;
    options.decoderType = IMedia::DecoderType::Pip;
    options.primingMode = false;
    pipPlayer->setOptions(options);
    Broadcom::Media::MediaStream mediaStream(testURL2);
    pipPlayer->setDataSource(&mediaStream);
    pipPlayer->prepare();
    pipPlayer->start();
    pipPlayer->setVideoWindowPosition(0, 0, 400, 300);
    sleep(15);
    pipPlayer->stop();
    pipPlayer->release();
    delete pipPlayer;
    // Stop main video
    stopVideo();
}

// This is the full mosaic test. It sets up a mosaic playback, then switches to regular
// decode then switches back to mosaic again
#define mosaicCount 4
TEST_F(FileSourceTest, MosaicTest)
{
    // Play mosaic, then single video then back to mosaics
    // can only do 8 due to nxserver limitations
    MediaPlayer* mediaPlayers[mosaicCount];

    IMedia::MediaPlayerOptions options;
    options.decoderType = IMedia::DecoderType::Mosaic;
    options.primingMode = false;

    Broadcom::Media::MediaStream mediaStream(testMosaicURL);
    // First pass, use capture mode, so nothing will show on the screen
    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i] = new MediaPlayer();
        this->observe(mediaPlayers[i]);

        mediaPlayers[i]->setOptions(options);
        mediaPlayers[i]->setDataSource(&mediaStream);
        mediaPlayers[i]->prepare();
        mediaPlayers[i]->start();
        mediaPlayers[i]->setVideoWindowPosition(i * 200, i * 200, 250, 250);
    }

    sleep(10);

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
        mediaPlayers[i]->setVideoWindowPosition(i * 200, i * 200, 250, 250);
    }

    sleep(3);

    for (int i = 0; i < mosaicCount; i++) {
        mediaPlayers[i]->stop();
        mediaPlayers[i]->release();
        delete mediaPlayers[i];
    }
}

TEST_F(FileSourceTest, SeekTest)
{
    uint32_t positionExpected = 20000;
    MediaPlayer* mediaPlayer =  new MediaPlayer();
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
    ASSERT_TRUE(_gotEndofStream);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
}

TEST_F(FileSourceTest, DurationTest)
{
    uint32_t mediaURLDuration = 34200;
    MediaPlayer* mediaPlayer =  new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL2);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->start();
    ASSERT_EQ(mediaPlayer->getDuration(), mediaURLDuration);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
}

TEST_F(FileSourceTest, Mp3TrickModeTest)
{
    startVideo(testMp3URL);
    sleep(3);
    // This is not supported so make sure it doesn't crash
    setTrickMode("4");
    sleep(1);
    stopVideo();
}

TEST_F(FileSourceTest, TsTrickModeTest)
{
    startVideo(testURL1);

    try {
        setTrickMode("2");
        sleep(3);
        setTrickMode("-2");
        stopVideo();
    } catch(...) {
        // expect to fail w/o index
    }
}

TEST_F(FileSourceTest, Mp4TrickModeTest)
{
    startVideo(testURL2);
    std::string list = _mediaPlayer->getAvailablePlaybackRate();
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list, "-64,-32,-16,-8,-4,-2,1,2,4,8,16,32,64");
    setTrickMode("2");
    sleep(5);
    setTrickMode("-2");
    sleep(5);
    setTrickMode("1");
    sleep(5);
    stopVideo();
}

TEST_F(FileSourceTest, IOErrorTest)
{
    MediaPlayer* mediaPlayer =  new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(std::string("file:///fake"));
    mediaPlayer->setDataSource(&mediaStream);
    IMedia::ErrorType ret = mediaPlayer->prepare();
    ASSERT_NE(ret, IMedia::MEDIA_SUCCESS);
    mediaPlayer->release();
    delete mediaPlayer;
}

TEST_F(FileSourceTest, SwitchAudioStreamTest)
{
    uint16_t listSize = 1;
    MediaPlayer* mediaPlayer =  new MediaPlayer();
    this->observe(mediaPlayer);
    Broadcom::Media::MediaStream mediaStream(testURL1);
    mediaPlayer->setDataSource(&mediaStream);
    mediaPlayer->prepare();
    mediaPlayer->start();
    IMedia::AudioParametersList list = mediaPlayer->getTrackInfo();
    ASSERT_GE(list.size(), listSize);
    sleep(2);
    mediaPlayer->selectTrack(0);
    sleep(3);
    mediaPlayer->selectAudioParameters(list[0]);
    sleep(3);
    mediaPlayer->stop();
    mediaPlayer->release();
    delete mediaPlayer;
}
}  // namespace Media
}  // namespace Broadcom
