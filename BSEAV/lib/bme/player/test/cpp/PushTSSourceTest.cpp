/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <gtest/gtest.h>
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"
#include "PushTSSource.h"
#include <iostream>

namespace Broadcom
{
namespace Media
{
/* Please go \\brcm-irv.broadcom.com\dfs\projects\stbdevstream_scratch\streams\playback\
 * To get test stream
 */
std::string streamFile = "/data/media/cnnticker.mpeg";

class PushTSSourceTest
    : public ::testing::Test,
    public MediaPlayerListener
{
public:
    PushTSSourceTest() :
        _mediaPlayer(NULL),
        _gotEndofStream(false),
        _gotPrepared(false),
        _gotError(false) {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    void PlayVideo(const std::string url, bool async = false) {
        TRLS_UNUSED(async);
        IMedia::VideoParameters videoParam = {
            0x21, 0x0, IMedia::Mpeg2VideoCodec, IMedia::UnknownVideoCodec,
            704, 480, 0, IMedia::VideoAspectRatioUnknown};

        IMedia::AudioParameters audioParam = {
            0x22, 0, IMedia::MpegAudioCodec, 0, 0, 0};

        Broadcom::Media::MediaStream mediaStream;
        IMedia::StreamMetadata metadata;
        mediaStream.setUri(url);
        metadata.streamType = IMedia::EsStreamType;
        metadata.videoParam = videoParam;
        metadata.audioParamList.push_back(audioParam);
        mediaStream.metadata = metadata;
        _mediaPlayer = new MediaPlayer();
        this->observe(_mediaPlayer);
        _mediaPlayer->setDataSource(&mediaStream);

        _mediaPlayer->prepare();
        ASSERT_EQ(_mediaPlayer->getState(), IMedia::StoppedState);
        _mediaPlayer->start();
        _mediaPlayer->setVideoWindowVisibility(true);
        _mediaPlayer->setVideoWindowVisibility(false);
        _mediaPlayer->setVideoWindowVisibility(true);
        // Will creat threads to run decoder pipeline
        PushTSSource* pushTSSource  = reinterpret_cast<PushTSSource*>(_mediaPlayer->getSource());
        //*** Example on open file and read data, send to pump.
        uint32_t cnt = 0;
        FILE *fd = NULL;
        uint32_t bufLen = 188 * (1 << 10);
        char buf[188 * (1 << 10)] = {0};
        IMedia::DataBuffer buffer;

        if (NULL  == (fd =  fopen(streamFile.c_str(), "rb"))) {
            printf("failed to open file --- streamFile:%s\n", streamFile.c_str());
            return;
        }

        for (int i = 0;; i++) {
            if (bufLen) {
                if (bufLen != (cnt = fread(buf, 1, bufLen, fd))) {
                    if (cnt == 0) {
                        std::cout << "Finsihed to read stream file to the end." << std::endl;
                        break;
                    } else {
                        //std::cout<<"failed to read Audio file !" <<std::endl;
                        break;
                    }
                }
            } else {
                if (188 != fread(buf, 1, 188, fd))
                    break;

                if (buf[0] == 0x47 && buf[4] != 0x47) {
                    bufLen = 188 << 6;
                } else if (buf[0] != 0x47 && buf[4] == 0x47) {
                    if (4 != fread(buf + 188, 1, 4, fd))
                        break;

                    bufLen = 192 << 6;
                } else {
                    printf("unknown file type !\n");
                    return;
                }
            }

            //send to pump
            E_PUMPBUF_STATE  bufState = E_IDLE;
            buffer.setData(reinterpret_cast<size_t>(buf));
            buffer.setSize(bufLen);
            bufState = pushTSSource->pushMediaChunk(buffer);

            if (bufState == E_FULL) {
                printf("buffer is FULL............\n");
            }

            if (bufState == E_ERROR) {
                printf("buffer is in ERROR............\n");
            }
        }

        // make sure it started properly
        if (_gotError) {
            printf("PushTSSourceTest: Got error!!!\n");
        }

        //printf("PAUSE 2 seconds............\n");
        //paused decoder rate.
        //_mediaPlayer->setPlaybackRate("0");
        //sleep(2);
        // normal decoder rate,any positive value passed-in will work.
        //_mediaPlayer->setPlaybackRate("1000");
        //printf("after PAUSE............\n");
        sleep(20);
        fclose(fd);
        sleep(5);
        _mediaPlayer->stop();
        ASSERT_EQ(_mediaPlayer->getState(), IMedia::StoppedState);
        _mediaPlayer->release();
        delete _mediaPlayer;
    }

    uint32_t getPosition() {
        return _mediaPlayer->getCurrentPosition();
    }

    IMedia::State getState() {
        return _mediaPlayer->getState();
    }

    // listeners
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
    virtual void onSeekComplete() { }
    virtual void onVideoSizeChanged(uint16_t width, uint16_t height) {
        TRLS_UNUSED(width);
        TRLS_UNUSED(height);
    }
    MediaPlayer* _mediaPlayer;
    bool _gotEndofStream;
    bool _gotPrepared;
    bool _gotError;
};

TEST_F(PushTSSourceTest, BasicTest)
{
    PlayVideo(IMedia::PUSH_TS_URI_PREFIX + streamFile);
}

}
}
