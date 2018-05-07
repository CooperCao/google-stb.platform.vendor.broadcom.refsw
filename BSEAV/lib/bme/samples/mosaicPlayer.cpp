/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <memory>
#include <functional>
#include <future>
#include "Debug.h"
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"
#include <unistd.h>

using namespace Broadcom;
using namespace Broadcom::Media;

class MosaicPlayer
    : public MediaPlayerListener
{
    public:
        MosaicPlayer(int argc, char** argv)
            : _argc(argc),
            _argv(argv)
    {
        int curarg = 1;
        while (curarg < argc) {
            urls[curarg - 1] = argv[curarg];
            _mosaicCount++;
            curarg++;
        }

        if (_mosaicCount == 0) {
            // set in code
            // "http://10.13.134.104/~dliu/cnnticker.mpg",
            urls[0] = "file:///mnt/nfs/cnnticker.mpg";
            urls[1] = "file:///mnt/nfs/cnnticker.mpg";
            urls[2] = "file:///mnt/nfs/cnnticker.mpg";
            urls[3] = "file:///mnt/nfs/cnnticker.mpg";
            _mosaicCount = 4;
        };
    }

    virtual void onCompletion() {
    }

    virtual void onPrepared() {
        _preparedCount++;
    }

    void run()
    {
        bool _cycle;
        _preparedCount = 0;
        for (int i = 0; i < _mosaicCount; i++) {
            IMedia::MediaPlayerOptions option;

            MediaStream ms(urls[i]);
            option.decoderType = IMedia::DecoderType::Mosaic_HD;
            if (i == 3)
                option.decoderType = IMedia::DecoderType::Pip;
            observe(&_mediaPlayers[i]);
            _mediaPlayers[i].setOptions(option);
            _mediaPlayers[i].setDataSource(&ms);
            _mediaPlayers[i].prepareAsync();
        }

        while (_preparedCount != _mosaicCount) {
            usleep(250000);
        }

        for (int i = 0; i < _mosaicCount; i++) {
            _mediaPlayers[i].setVideoWindowPosition((i % 2) * 640, (i / 2) * 360, 640, 360);
            _mediaPlayers[i].start();
            usleep(50000);
        }

        getchar();

        for (int i = 0; i < _mosaicCount; i++) {
            _mediaPlayers[i].stop();
            _mediaPlayers[i].release();
        }
    }

public:
    int _argc;
    char** _argv;
    uint8_t _preparedCount;
    MediaPlayer _mediaPlayers[8];
    int _mosaicCount;
    std::string urls[4];
};

int main(int argc, char **argv)
{
    debugInitialize();
    std::unique_ptr<MosaicPlayer> app(new MosaicPlayer(argc, argv));
    app->run();

    return 0;
}
