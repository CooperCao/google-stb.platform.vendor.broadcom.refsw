/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"

using namespace Broadcom;
using namespace Broadcom::Media;

void printUsage();
char ibuffer[256];

#define FORWARD 1
#define BACKWARD 2

class MediaService
    : public MediaPlayerListener
{
 public:
    MediaService(int argc, char** argv)
        : _argc(argc),
        _argv(argv),
        _rate(1),
        _maxWidth(0),
        _maxHeight(0)
    {
    }

    void PrintMenu(void)
    {
        printf("***********************************************\n");
        printf("*** Player Console                          ***\n");
        printf("***********************************************\n");
        printf("State: Playing\n");
        printf("0) Exit\n");
        printf("1) Start\n");
        printf("2) Stop\n");
        printf("3) Pause\n");
        printf("4) Fast Forward\n");
        printf("5) Fast Rewind\n");
        printf("6) Seek\n");
        printf("7) Refresh status\n");
        printf("8) Change size\n");
    }

    unsigned input_number(void)
    {
        fgets(ibuffer, 100, stdin);
        return atoi(ibuffer);
    }

    void run(void)
    {
        int command;
        bool enablePip = false;
        bool loop = false;
        int speed = 1;
        int audioPid = 0;
        std::string url;

        // parse command line
        for (int n = 1; n < _argc; n++) {
            if (std::string(_argv[n]).compare(0, 5, "--url") == 0) {
                url = std::string(_argv[n]).substr(6);
            }
            if (std::string(_argv[n]).compare(0, 5, "--max") == 0) {
                int res = sscanf(std::string(_argv[n]).substr(6).c_str(),
                        "%u,%u", &_maxWidth, &_maxHeight);
                if (res != 2) {
                    printUsage();
                }
            }
            if (std::string(_argv[n]).compare(0, 10, "--audioPid") == 0) {
                sscanf(std::string(_argv[n]).substr(11).c_str(), "%u", &audioPid);
            }
        }

        mediaPlayer = new MediaPlayer;
        this->observe(mediaPlayer);
        MediaStream ms(url);

        IMedia::VideoParameters videoParam;
        videoParam.maxWidth = _maxWidth;
        videoParam.maxHeight = _maxHeight;

        ms.metadata.videoParam = videoParam;
        if (audioPid) {
            ms.setPreferredAudioPid(audioPid);
        }

        mediaPlayer->setDataSource(&ms);
        IMedia::ErrorType error = mediaPlayer->prepare();
        if (error == IMedia::MEDIA_SUCCESS) {
            mediaPlayer->start();
            mediaPlayer->setLooping(true);
        } else  {
            printf("Unable to play url %s!!\n", url.c_str());
            exit(-1);
        }

        PrintMenu();

        ///////////////////////////////////
        while ((command = input_number()) != EOF) {
            printf("command = %d\n", command);
            PrintMenu();

            if (command == 0)
                break;

            switch (command) {
                case 1:
                    mediaPlayer->start();
                    break;
                case 2:
                    mediaPlayer->stop();
                    break;
                case 3:
                    mediaPlayer->pause();
                    break;
                case 4:
                    setRate(FORWARD);
                    break;
                case 5:
                    setRate(BACKWARD);
                    break;
                case 6:
                    mediaPlayer->seekTo(5000);
                    break;
                case 7:
                    if (mediaPlayer->isPlaying()) {
                        printf("duration = %d\n", mediaPlayer->getDuration());
                        printf("position = %d\n", mediaPlayer->getCurrentPosition());
                        printf("video width = %d\n", mediaPlayer->getVideoWidth());
                        printf("video height = %d\n", mediaPlayer->getVideoHeight());
                        printf("framerate = %d\n", mediaPlayer->getVideoFrameRate());
                        printf("aspect ratio = %d\n", mediaPlayer->getVideoAspectRatio());
                    }
                    break;
                case 8:
                    mediaPlayer->setVideoWindowPosition(0, 0, 1280, 720);
                    break;
            }
        }
        mediaPlayer->release();
    }

    void setRate(int direction)
    {
        std::string list;
        bool rateSet = false;

        while (!rateSet) {
            if (direction == FORWARD) {
                if (_rate > 1)
                    _rate++;
                else
                    _rate = 2;
            } else {
                if (_rate < 1)
                    _rate--;
                else
                    _rate = -2;
            }

            if ((_rate > 256) || (_rate < -256)) {
                printf("exceeded the limit\n");
                break;
            }

            list = mediaPlayer->getAvailablePlaybackRate();
            printf("list = %s\n", list.c_str());

            char* p = strdup(list.c_str());
            char* tok = strtok(p, ",");
            while (tok) {
                if (atoi(tok) == (int)_rate) {
                    printf("Setting rate = %d\n", _rate);
                    mediaPlayer->setPlaybackRate(tok);
                    rateSet = true;
                    break;
                }
                tok = strtok(NULL, ",");
            }
        }
    }

 public:
    int _argc;
    char** _argv;
    int32_t _rate;
    uint32_t _maxWidth;
    uint32_t _maxHeight;
    MediaPlayer* mediaPlayer;
};

void printUsage(void)
{
    printf("Usage: player --resolve=<type> --url=<URI> --max=w,h --audioPid=<pidNumber>\n");
    printf(" -resolve options are:\n");
    printf("\tlocal \tUse this if you are not running bme services\n");
    printf(" -uri options are:\n");
    printf("\thttp://10.13.134.104/~dliu/media/cnnticker.mpg\n");
    printf("\tfile:///mnt/nfs/cnnticker.mpg\n");
    printf("\trtp://192.168.1.109:1234\n");
    printf("\tudp://192.168.1.109:1234\n");
    printf(" --max=WIDTH,HEIGHT\t max video decoder resolution\n");
}

int main(int argc, char **argv)
{
    if ((argc <= 1) || (std::string(argv[1]) == "--help")) {
        printUsage();
        return -1;
    }

    MediaService* app = new MediaService(argc, argv);
    app->run();

    delete app;
    return 0;
}
