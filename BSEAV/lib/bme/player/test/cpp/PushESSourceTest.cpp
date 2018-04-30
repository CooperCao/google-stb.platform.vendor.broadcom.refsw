/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <gtest/gtest.h>
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"
#include "PushESSource.h"
#include <iostream>

const uint32_t TASK_STACK_SIZE =  (256 << 10);

namespace Broadcom
{
namespace Media
{

TRLS_DBG_MODULE(PushESSourceTest);

/* Flag for H264 convert to AnnexB.
 * #define CONVERT_TO_ISO_IEC_14496_10_FORMAT
 */

// Please run get_unittest_streams.sh on the board to get test stream
std::string vFile = "/data/media/cnnticker.video.es";
std::string aFile = "/data/media/cnnticker.audio.es";
IMedia::AudioCodec audioCodec = IMedia::MpegAudioCodec;
IMedia::VideoCodec videoCodec = IMedia::Mpeg2VideoCodec;
IMedia::StreamType streamType = IMedia::EsStreamType;
int playbackTime = 15;
std::thread videoDecThread, audioDecThread;

class PushESSourceTest :
      public ::testing::Test,
      public MediaPlayerListener
{
public:
    PushESSourceTest() :
        _mediaPlayer(NULL),
        _gotEndofPlayback(false),
        _gotPrepared(false),
        _gotError(false) {
    }

    virtual void SetUp() {
        _mediaPlayer = new MediaPlayer();
        this->observe(_mediaPlayer);
    }

    virtual void TearDown() {
    }

    static void RunVideoDecoder(void * param) {
        //*** Example on open file and read data, send to pump.
        unsigned cnt = 0;
        FILE *fd = NULL;
        size_t bufLen = 188 * (1 << 15);  // times of 188 bytes packets.
        PushESSource   *ESengine = (PushESSource*)param;
        char *buf = NULL;

        if (NULL == (buf = (char*)malloc(bufLen))) {
            BME_DEBUG_ERROR(("Failed to allocate memory\n"));
            return;
        }

        if (NULL  == (fd =  fopen(vFile.c_str(), "rb"))) {
            free(buf);
            BME_DEBUG_ERROR(("failed to open file !\n"));
            return;
        }

        for (int i = 0;; i++) {
            if (bufLen) {
                //printf("video LastFifoLevel: %d\n", ESengine->getLastAFifoLevel());
                if (bufLen != (cnt = fread(buf, 1, bufLen, fd))) {
                    if (cnt == 0) {
                        std::cout << "Finsihed to read video file to the end." << std::endl;
                        fseek(fd, 0, SEEK_SET);
                    } else {
                        //std::cout<<"failed to read video file !" <<std::endl;
                        break;
                    }
                }
            }

            //send to pump
#ifdef CONVERT_TO_ISO_IEC_14496_10_FORMAT
            IMedia::DataBuffer buffer;
            bool droppable;
            buffer.setData(reinterpret_cast<uint32_t>(buf));
            buffer.setSize(bufLen);
            ESengine->convertToAnnexBAndCheckProperties(buffer, (Format3D)0, droppable);
#endif
            uint32_t size = bufLen;

            // SEAN,the pts should from demux insted of engine. otherwise would not work normally !!!
            // Since it use current frame PTS for next fram.
            int32_t pts = ESengine->getCurrentPosition();

            PushESSource::DataFragment fragment;
            fragment.data      = buf;
            fragment.bytes     = size;
            fragment.encrypted = false;
            ESengine->makeVideoChunk(pts, &fragment, 1);
            ESengine->pushVideoChunk();
        }

        if (NULL != buf) {
            free(buf);
        }

        fclose(fd);
        return;
    }

    static void RunAudioDecoder(void *param) {
        //*** Example on open file and read data, send to pump.
        unsigned cnt = 0;
        FILE *fd = NULL;
        PushESSource   *ESengine = (PushESSource*)param;
        size_t bufLen = 188 * (1 << 8); // times of 188 bytes packets.
        char *buf = NULL;

        if (NULL == (buf = (char*)malloc(bufLen))) {
            BME_DEBUG_ERROR(("Failed to allocate memory\n"));
            return;
        }

        if (NULL  == (fd =  fopen(aFile.c_str(), "rb"))) {
            BME_DEBUG_ERROR(("failed to open file !\n"));
            return;
        }

        for (int i = 0;; i++) {
            if (bufLen) {
                //printf("Audio LastFifoLevel: %d\n", ESengine->getLastAFifoLevel());
                if (bufLen != (cnt = fread(buf, 1, bufLen, fd))) {
                    if (cnt == 0) {
                        std::cout << "Finsihed to read Audio file to the end." << std::endl;
                        fseek(fd, 0, SEEK_SET);
                    } else {
                        //std::cout<<"failed to read Audio file !" <<std::endl;
                        break;
                    }
                }
            }

            //send to pump
            uint32_t size = bufLen;

            // SEAN,the pts should from demux insted of engine. otherwise would not work normally !!!
            // Since it use current frame PTS for next fram.
            int32_t pts = ESengine->getCurrentPosition();

            PushESSource::DataFragment fragment;
            fragment.data      = buf;
            fragment.bytes     = size;
            fragment.encrypted = false;
            ESengine->makeAudioChunk(pts, &fragment, 1);
            ESengine->pushAudioChunk();
        }

        if (NULL != buf) {
            free(buf);
        }

        fclose(fd);
        return;
    }

    void StartVideo(const std::string url, bool async = false) {
        TRLS_UNUSED(async);

        IMedia::VideoParameters videoParam = {
            0xE0, 0x44, videoCodec, IMedia::UnknownVideoCodec,
            1080, 720, 0, IMedia::VideoAspectRatioUnknown};

        IMedia::AudioParameters audioParam = {
            0xC0, 0, audioCodec, 0, 0, 0};

        Broadcom::Media::MediaStream mediaStream;
        IMedia::StreamMetadata metadata;
        mediaStream.setUri(url);
        metadata.streamType = streamType;
        metadata.videoParam = videoParam;
        metadata.audioParamList.push_back(audioParam);
        mediaStream.metadata = metadata;
        _mediaPlayer->setDataSource(&mediaStream);
        _mediaPlayer->prepare();
        ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StoppedState);
        _mediaPlayer->start();
        /*Will creat threads to run decoder pipeline*/
        PushESSource* pushESSource  = reinterpret_cast<PushESSource*>(_mediaPlayer->getSource());
        videoDecThread = std::thread(RunVideoDecoder, (void*)pushESSource);
        videoDecThread = std::thread(RunAudioDecoder, (void*)pushESSource);

        // make sure it started properly
        if (_gotError) {
            BME_DEBUG_ERROR(("PushESSourceTest: Got error!!!\n"));
        }
    }

    void PlayVideo(std::string url, uint32_t playDuration = 5, bool looping = false) {
        TRLS_UNUSED(looping);
        uint32_t width = 1080;
        uint32_t height = 720;
        StartVideo(url);
        // Test set and get video window position
        _mediaPlayer->setVideoWindowPosition(0, 0, width, height);
        ASSERT_EQ((int)_mediaPlayer->getVideoDisplayWidth(), (int)width);
        ASSERT_EQ((int)_mediaPlayer->getVideoDisplayHeight(), (int)height);
        sleep(playDuration);
        ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StartedState);
        // Test Set visibility
        _mediaPlayer->setVideoWindowVisibility(false);
        sleep(1);
        _mediaPlayer->setVideoWindowVisibility(true);
        sleep(playbackTime - 10);
        StopVideo();
    }

    void PauseVideo(std::string url, uint32_t playDuration = 5, bool looping = false) {
        TRLS_UNUSED(looping);
        uint32_t width = 1920;
        uint32_t height = 1080;
        StartVideo(url);
        // Test set and get video window position
        _mediaPlayer->setVideoWindowPosition(0, 0, width, height);
        ASSERT_EQ((int)_mediaPlayer->getVideoDisplayWidth(), (int)width);
        ASSERT_EQ((int)_mediaPlayer->getVideoDisplayHeight(), (int)height);
        sleep(playDuration);
        ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StartedState);
        // paused decoder rate
        std::cout << "PAUSE 2 seconds............" << std::endl;
        _mediaPlayer->setPlaybackRate("0");
        sleep(2);
        // normal decoder rate,any positive value passed-in will work.
        _mediaPlayer->setPlaybackRate("1000");
        std::cout << "resume from PAUSE............" << std::endl;
        sleep(playbackTime);
        PushESSource* pushESSource  = reinterpret_cast<PushESSource*>(_mediaPlayer->getSource());
        uint32_t dropCnt = pushESSource->getDropFrameCount();
        std::cout << "drop frame cnt :" << dropCnt << std::endl;
        StopVideo();
    }

    void SeekVideo(std::string url, uint32_t timeout) {
        StartVideo(url);
        sleep(5);
        /**********************************************************
         * Does not work probably in this sample since it is unable
         * to provide precise PTS here. use it have its own test.

        pushESSource->seekToPts(10000,true, true);
        ASSERT_GT((int)_mediaPlayer->getCurrentPosition(), 10);

        **********************************************************/
        sleep(timeout);
        StopVideo();
    }

    void StopVideo() {
        PushESSource* pushESSource  = reinterpret_cast<PushESSource*>(_mediaPlayer->getSource());

        if (videoDecThread.joinable())
            videoDecThread.join();
        if (audioDecThread.joinable())
            audioDecThread.join();

        pushESSource->pushVideoEndOfStream();
        pushESSource->flush(false);
        while (!_gotEndofPlayback) {
           sleep(1);
        }
        _mediaPlayer->stop();
        ASSERT_EQ(_mediaPlayer->getState(), MediaPlayer::StoppedState);
        _mediaPlayer->release();
        delete _mediaPlayer;
    }

    void PauseVideo() {
        _mediaPlayer->pause();
    }

    void ResumeVideo() {
        _mediaPlayer->start();
    }

    uint32_t getPosition() {
        return _mediaPlayer->getCurrentPosition();
    }

    MediaPlayer::State getState() {
        return _mediaPlayer->getState();
    }

    // listeners
    virtual void onCompletion() {
        _gotEndofPlayback = true;
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
        TRLS_UNUSED(width);
        TRLS_UNUSED(height);
    }
    MediaPlayer* _mediaPlayer;
    bool _gotEndofPlayback;
    bool _gotPrepared;
    bool _gotError;
};

TEST_F(PushESSourceTest, BasicTest)
{
    PlayVideo(IMedia::PUSH_ES_URI_PREFIX);
}

TEST_F(PushESSourceTest, PauseTest)
{
    PauseVideo(IMedia::PUSH_ES_URI_PREFIX);
}

TEST_F(PushESSourceTest, SeekTest)
{
    SeekVideo(IMedia::PUSH_ES_URI_PREFIX, playbackTime + 5);
}

}  // namespace Media
}  // namespace Broadcom
