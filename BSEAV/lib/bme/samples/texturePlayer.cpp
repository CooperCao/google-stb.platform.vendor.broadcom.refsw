/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include "nexus_platform_client.h"
#include "MediaPlayer.h"
#include "MediaPlayerListener.h"
#include "nexus_surface_client.h"
#include "nexus_graphics2d.h"
#include "bstd.h"
#include "bkni.h"
#include "nxclient.h"

using namespace Broadcom;
using namespace Broadcom::Media;

void printUsage();

class MediaService
    : public MediaPlayerListener
{
 public:
    MediaService(int argc, char** argv)
        : _argc(argc),
        _argv(argv),
        _maxWidth(0),
        _maxHeight(0),
        _secure(false),
        _flip(false)
    {
    }

    static void gfx_checkpoint(void *data, int unused)
    {
        BSTD_UNUSED(unused);
        BKNI_SetEvent((BKNI_EventHandle)data);
    }

    void parseCmdLine()
    {
        // parse command line
        for (int n = 1; n < _argc; n++) {
            if (std::string(_argv[n]).compare(0, 5, "--url") == 0) {
                _url = std::string(_argv[n]).substr(6);
            }
            if (std::string(_argv[n]).compare(0, 5, "--max") == 0) {
                int res = sscanf(std::string(_argv[n]).substr(6).c_str(),
                        "%u,%u", &_maxWidth, &_maxHeight);
                if (res != 2) {
                    printUsage();
                }
            }
            if (std::string(_argv[n]).compare(0, 8, "--secure") == 0) {
                _secure = true;
            }
            if (std::string(_argv[n]).compare(0, 6, "--flip") == 0) {
                _flip = true;
            }
        }
    }

    void run(void)
    {
        NEXUS_Graphics2DOpenSettings graphics2dOpenSettings;
        BKNI_EventHandle displayedEvent, checkpointEvent;
        NEXUS_Error rc;
        NEXUS_Graphics2DHandle gfx;
        NxClient_JoinSettings joinSettings;
        NxClient_GetDefaultJoinSettings(&joinSettings);
        rc = NxClient_Join(&joinSettings);

        parseCmdLine();

        BKNI_CreateEvent(&displayedEvent);
        BKNI_CreateEvent(&checkpointEvent);

        NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dOpenSettings);
        graphics2dOpenSettings.secure = _secure;
        gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphics2dOpenSettings);
        NEXUS_Graphics2DSettings gfxSettings;
        NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
        gfxSettings.checkpointCallback.callback = gfx_checkpoint;
        gfxSettings.checkpointCallback.context = checkpointEvent;
        rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

        NxClient_AllocSettings allocSettings;
        NxClient_AllocResults allocResults;
        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.surfaceClient = 1;
        NEXUS_SurfaceClientHandle surfaceClient;
        rc = NxClient_Alloc(&allocSettings, &allocResults);
        surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
        NEXUS_SurfaceClientSettings clientSettings;
        NEXUS_SurfaceClient_GetSettings(surfaceClient, &clientSettings);
        clientSettings.displayed.callback = gfx_checkpoint;
        clientSettings.displayed.context = displayedEvent;
        rc = NEXUS_SurfaceClient_SetSettings(surfaceClient, &clientSettings);

        NEXUS_SurfaceComposition comp;
        NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        comp.zorder = 101;
        NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);

        // create main surface
        NEXUS_SurfaceCreateSettings createSettings;
        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        createSettings.width = 1280;
        createSettings.height = 720;
        NEXUS_ClientConfiguration clientConfig;
        NEXUS_Platform_GetClientConfiguration(&clientConfig);
        if (_secure)
            createSettings.heap = clientConfig.heap[NXCLIENT_SECURE_GRAPHICS_HEAP];
        NEXUS_SurfaceHandle mainSurface;
        mainSurface = NEXUS_Surface_Create(&createSettings);

        NEXUS_Graphics2DFillSettings fillSettings;
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = mainSurface;
        fillSettings.color = 0x0;
        rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, 5000);
        }

        rc = NEXUS_SurfaceClient_SetSurface(surfaceClient, mainSurface);
        rc = BKNI_WaitForEvent(displayedEvent, 5000);

        mediaPlayer = new MediaPlayer;
        this->observe(mediaPlayer);
        MediaStream ms(_url);

        IMedia::VideoParameters videoParam;
        videoParam.maxWidth = _maxWidth;
        videoParam.maxHeight = _maxHeight;

        ms.metadata.videoParam = videoParam;

        mediaPlayer->setDataSource(&ms);
        IMedia::ErrorType error = mediaPlayer->prepare();
        if (error == IMedia::MEDIA_SUCCESS) {
            mediaPlayer->start();
            mediaPlayer->startFrameCapture(RGB565, 1280, 720, 0,
                    mainSurface, _secure);
            mediaPlayer->setLooping(true);
        } else  {
            printf("Unable to play url %s!!\n", _url.c_str());
            exit(-1);
        }
        mediaPlayer->setVideoWindowPosition(0, 0, 320, 240);
        sleep(1);
        while (1) {
            mediaPlayer->getFrame(1, false);

            /* tell server to blit */
            rc = NEXUS_SurfaceClient_UpdateSurface(surfaceClient, NULL);
            BDBG_ASSERT(!rc);
            rc = BKNI_WaitForEvent(displayedEvent, 5000);
            BDBG_ASSERT(!rc);
        }
        mediaPlayer->stopFrameCapture();
        mediaPlayer->release();
    }

 public:
    int _argc;
    char** _argv;
    uint32_t _maxWidth;
    uint32_t _maxHeight;
    std::string _url;
    bool _secure;
    bool _flip;
    MediaPlayer* mediaPlayer;
};

void printUsage(void)
{
    printf("Usage: player --resolve=<type> --url=<URI> --max=w,h\n");
    printf(" -uri options are:\n");
    printf("\thttp://10.13.134.104/~dliu/media/cnnticker.mpg\n");
    printf("\tfile:///mnt/nfs/cnnticker.mpg\n");
    printf("\trtp://192.168.1.109:1234\n");
    printf("\tudp://192.168.1.109:1234\n");
    printf(" --max=WIDTH,HEIGHT\t max video decoder resolution\n");
    printf(" --secure\t to capture graphics from SVP\n");
    printf(" --flip\t flip output of capture\n");
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
