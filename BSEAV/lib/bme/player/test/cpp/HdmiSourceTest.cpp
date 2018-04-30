/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#ifdef NEXUS_HAS_HDMI_INPUT
#include <gtest/gtest.h>
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"
#include "HdmiSource.h"
#include <iostream>

namespace Broadcom
{
namespace Media
{

class HdmiSourceTest :
      public ::testing::Test,
      public MediaPlayerListener
{
public:
    HdmiSourceTest() :
      _mediaPlayer(NULL),
      _gotPrepared(false),
      _gotError(false),
      _gotEndofStream(false)
    {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    void PlayVideo(const std::string url, bool async = false) {
        TRLS_UNUSED(async);
        _mediaPlayer = new MediaPlayer();

        this->observe(_mediaPlayer);
        Broadcom::Media::MediaStream mediaStream(url);
        _mediaPlayer->setDataSource(&mediaStream);

        _mediaPlayer->prepare();
        ASSERT_EQ(_mediaPlayer->getState(), IMedia::StoppedState);

        _mediaPlayer->start();

        // make sure it started properly
        if (_gotError) {
            printf("HdmiSourceTest: Got error!!!\n");
        }

        printf("Hdmi Source playback............\n");

        sleep(150);
        _mediaPlayer->stop();

        ASSERT_EQ(_mediaPlayer->getState(), IMedia::StoppedState);

        _mediaPlayer->release();

        delete _mediaPlayer;
    }

    IMedia::State getState() {
        return _mediaPlayer->getState();
    }
    virtual void onError(const IMedia::ErrorType& errorType) {
        TRLS_UNUSED(errorType);
        _gotError = true;
    }

    virtual void onPrepared() {
        _gotPrepared = true;
    }

    virtual void onCompletion() {
        _gotEndofStream = true;
    }

    MediaPlayer* _mediaPlayer;
    bool _gotPrepared;
    bool _gotError;
    bool _gotEndofStream;
};

TEST_F(HdmiSourceTest, BasicTest)
{
    PlayVideo(IMedia::HDMI_URI_PREFIX);
}

}
}
#endif  // NEXUS_HAS_HDMI_INPUT
