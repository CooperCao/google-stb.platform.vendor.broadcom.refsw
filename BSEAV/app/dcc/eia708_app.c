/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include "708_test_string.h"
#include "b_dcc_lib.h"
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_component_output.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_frontend.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_video_window.h"
#include "nexus_video_output.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_userdata.h"
#include "nexus_video_input_vbi.h"
#include "b_os_lib.h"

#define CC_DATA_BUF_SIZE 128
#define MAX_INPUT_CHARS 128
#define MAX_LENGTH_FILE 64  /* max num characters for a playback stream file name */

typedef struct CCTest_InputParameters
{
    B_Dcc_Type      ccMode;
    uint32_t        ccService;
    uint32_t        channelChange;
    uint32_t        changeDisp;
    bool            dispOn; /* display on or off (allows sharing framebuffer with application) */
    bool            previewOn;
    bool            exit;
} CCTest_InputParameters;

typedef enum CCTest_Input_Source
{
    CCTest_Input_Streamer,
    CCTest_Input_File,
    CCTest_Input_QAM,
    CCTest_Input_VSB,
    CCTest_Input_Synth
} CCTest_Input_Source;

typedef struct CCTest_g_sourceParams
{
    CCTest_Input_Source inputSrc;
    uint32_t testNum;
    uint32_t video_pid;
    uint32_t audio_pid;
    NEXUS_PidChannelHandle pidChannel;
    unsigned int frequency;
    char szFile[ MAX_LENGTH_FILE ]; /* the name of the stream  file */
} CCTest_g_sourceParams;

/*
** Display mode constants.
*/
typedef enum CCTest_Display_Modes
{
    CCTest_Display_480i,
    CCTest_Display_480p,
    CCTest_Display_720p,
    CCTest_Display_1080i,
    CCTest_Display_Max_Size

}CCTest_Display_Modes;

typedef struct CCTest_Caption_Triplets
{
    uint8_t ccType;
    uint8_t data[2];
} CCTest_Caption_Triplets;

int getCmdLine(
    int argc,
    char *argv[]
    );

int initialize(void);

int32_t mainLoop(void);

void shutdown(void);

void loadFonts(
    BDCC_WINLIB_Handle g_WindowLib,
    uint32_t *scaleFactor
    );

void printHelpInfo( void );

void previewCaption(bool enable, B_Dcc_Type cc_mode, uint32_t cc_service, int inputData1, int inputData2, int inputData3);

void user_Input(
    const char *name
    );

void viewerInputThread(
    void *data
    );

bool parseInputStr(
    const char *pszInputStr
    );

extern BDCC_Error B_Dcc_SendTestString(
    B_Dcc_Handle hEngine,
    const unsigned char *pTestSt,
    const unsigned int TestStLen
    );



/*
** Global variables.
*/

B_Dcc_OverRides g_overrideValues;
uint32_t g_overrideMask;
CCTest_g_sourceParams g_sourceParams;

uint32_t g_DisplayMode = CCTest_Display_480i;

/* save a long switch statement */
uint32_t gDisplayWidth[4]   = {720, 720, 1280, 1920};
uint32_t gDisplayHeight[4]  = {480, 480, 720, 1080};
uint32_t gCaptionColumns[4] = {32, 32, 42, 42};
#ifdef SCALE_720P_FONTS
uint32_t gScaleFactor[4]    = {66, 66, 100, 150};
#endif
NEXUS_VideoFormat gVideoFormat[4] = {NEXUS_VideoFormat_eNtsc, NEXUS_VideoFormat_e480p, NEXUS_VideoFormat_e720p, NEXUS_VideoFormat_e1080i};

/* handles, global for convenience */
B_Dcc_Handle g_ccEngine              = NULL;
BDCC_WINLIB_Handle g_WindowLib          = NULL;
NEXUS_VideoDecoderHandle g_videoDecoder = NULL;
NEXUS_AudioDecoderHandle g_audioDecoder = NULL;
NEXUS_DisplayHandle g_display           = NULL;
NEXUS_VideoInput g_videoInput           = NULL;
NEXUS_AudioInput g_audioInput              = NULL;
NEXUS_VideoDecoderStartSettings g_videoProgram;
NEXUS_AudioDecoderStartSettings g_audioProgram;
NEXUS_VideoWindowHandle g_window        = NULL;
NEXUS_StcChannelHandle g_stcChannel     = NULL;
NEXUS_PlaypumpHandle g_playpump         = NULL;
NEXUS_FrontendHandle g_frontend         = NULL;
NEXUS_PlaybackHandle g_playback         = NULL;
NEXUS_PlatformConfiguration g_platformConfig;
NEXUS_SurfaceHandle g_framebuffer1, g_framebuffer2;
unsigned g_cur_framebuffer; /* the one last returned by flip */

BKNI_EventHandle g_newCcDataEvent;

/*
** The following variables are used to pass data from the "viewer" input
** thread to the main application.  This is a lazy/non-robust approach.
*/
CCTest_InputParameters g_UserParams;
bool g_bUserInputValid;



/*
** Currently only need this to set the semaphore because 'feature' in KNI
** doesn't allow a timeout of less than 90ms.
** When this 'feature' is removed we no longer need to set up to parse raw userdata
** - 04/29/08 garetht
*/
void userdata(void *context, int param)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)context;
    unsigned size;
    uint8_t *buffer;

    BSTD_UNUSED(param);

    NEXUS_VideoDecoder_GetUserDataBuffer(videoDecoder, (void**)&buffer, &size);
    NEXUS_VideoDecoder_UserDataReadComplete(videoDecoder, size);

    /* notify the 708 data servicing thread */
    BKNI_SetEvent(g_newCcDataEvent);
}



int main( int argc, char *argv[] )
{

    /*
    ** Paremeters for the thread which handles user input.
    ** ( medium priority, default stack size, thread name )
    */
    B_ThreadHandle hInputThread;
    B_ThreadSettings threadSettings;


    BKNI_Memset( &g_sourceParams, 0, sizeof ( CCTest_g_sourceParams ));

    if(getCmdLine( argc, argv ))
    {
        user_Input( argv[0] );
        return(1);
    }

    if ( initialize() )
    {
        printf("Fatal Error during initialization\n");
        goto error;
    }

    if( CCTest_Input_Synth == g_sourceParams.inputSrc )
    {

        /* using compiled in test strings from test_string.h */
        int firstTest, lastTest;
        uint32_t periodic;

        firstTest = (g_sourceParams.testNum == 0) ? 1 : g_sourceParams.testNum;
        lastTest  = (g_sourceParams.testNum == 0) ? BDCC_MAX_TESTS : g_sourceParams.testNum;

        do
        {
            periodic = 5;
            printf("TEST %d\n", firstTest);

            B_Dcc_SendTestString(g_ccEngine, Dtvcc708Stream[firstTest - 1] , Dtvcc708StreamLength[firstTest - 1]);

            BKNI_Sleep(1000);

            while(periodic--)
            {

                B_Dcc_Periodic(g_ccEngine);
                BKNI_Sleep(1000);
            }
        }
        while(firstTest++ < lastTest);

        BKNI_Sleep(5000);

    }
    else
    {
        /* Spawn user input task */
        g_bUserInputValid = false;
        B_Os_Init();
        B_Thread_GetDefaultSettings(&threadSettings);
        hInputThread = B_Thread_Create("InputThread", viewerInputThread, NULL, &threadSettings );

        if( mainLoop() )
        {
            printf("Error during caption processing\n");
            goto error;
        }
    }

    shutdown();
    return 0;

error:

    shutdown();
    return 1;
}



int getCmdLine(int argc, char *argv[])
{

    /*
    ** Process the command line parameters.
    */

    int i;

    if ( argc < 2 )
        return -1;
    else
    {
        for ( i = 1; i < argc; i++  )
        {
            if ( !strcmp(argv[i], "-480i" ) )
            {
                g_DisplayMode = CCTest_Display_480i;
                printf("--- display mode of 480i\n");
            }
            else if ( !strcmp(argv[i], "-480p" ) )
            {
                g_DisplayMode = CCTest_Display_480p;
                printf("--- display mode of 480p\n");
            }
            else if ( !strcmp(argv[i], "-720p" ) )
            {
                g_DisplayMode = CCTest_Display_720p;
                printf("--- display mode of 720p\n");
            }
            else if ( !strcmp(argv[i], "-1080i" ) )
            {
                g_DisplayMode = CCTest_Display_1080i;
                printf("--- display mode of 1080i\n");
            }
            else if ( !strcmp(argv[i], "-test" ) )
            {
                g_sourceParams.inputSrc = CCTest_Input_Synth;
                printf("--- Test String Selected\n");

                if (argv[++i])
                {
                    sscanf(argv[i], "%u", &g_sourceParams.testNum);
                    if((g_sourceParams.testNum > BDCC_MAX_TESTS) || (!g_sourceParams.testNum))
                    {
                        printf("--- You can select tests 1 thru %d\n", BDCC_MAX_TESTS);
                        return 1;
                    }
                }
                else
                {
                    printf("--- Must specifiy a test number\n");
                    return 1;
                }
            }
            else if ( !strcmp(argv[i], "-testall" ) )
            {
                g_sourceParams.inputSrc = CCTest_Input_Synth;
                printf("--- Test String Selected\n");
                g_sourceParams.testNum = 0; /* test all flag */
            }
            else if ( !strcmp(argv[i], "-file" ) )
            {
                g_sourceParams.inputSrc = CCTest_Input_File;
                printf("--- playing back from disk\n");
                if (argv[++i])
                {
                    strcpy(g_sourceParams.szFile, argv[i]);
                    if (argv[++i])
                    {
                        if(( argv[i][1] == 'x') || ( argv[i][1] == 'X'))
                            sscanf(argv[i], "%x", &g_sourceParams.video_pid);
                        else
                            sscanf(argv[i], "%u", &g_sourceParams.video_pid);
                    }
                    else
                    {
                        printf("--- Must specifiy a video pid in hex format\n");
                        return 1;
                    }
                    if (argv[++i])
                    {
                        if(( argv[i][1] == 'x') || ( argv[i][1] == 'X'))
                            sscanf(argv[i], "%x", &g_sourceParams.audio_pid);
                        else
                            sscanf(argv[i], "%u", &g_sourceParams.audio_pid);
                    }
                    else
                    {
                        printf("--- Must specifiy an audio pid in hex format\n");
                        return 1;
                    }
                }
                else
                {
                    printf("--- Must specify a file name\n");
                    return 1;
                }
            }
            else if ( !strcmp(argv[i], "-qam") || !strcmp(argv[i], "-vsb" ))
            {
                if(!strcmp(argv[i], "-qam"))
                {
                    g_sourceParams.inputSrc = CCTest_Input_QAM;
                    printf("--- Tune qam channel\n");
                }
                else
                {
                    g_sourceParams.inputSrc= CCTest_Input_VSB;
                    printf("--- Tune vsb channel\n");
                }
                if (argv[++i])
                {
                    g_sourceParams.frequency = atoi(argv[i]);
                    if (argv[++i])
                    {
                        if(( argv[i][1] == 'x') || ( argv[i][1] == 'X'))
                            sscanf(argv[i], "%x", &g_sourceParams.video_pid);
                        else
                            sscanf(argv[i], "%u", &g_sourceParams.video_pid);
                    }
                    else
                    {
                        printf("--- Must specifiy a video pid in hex format\n");
                        return 1;
                    }
                    if (argv[++i])
                    {
                        if(( argv[i][1] == 'x') || ( argv[i][1] == 'X'))
                            sscanf(argv[i], "%x", &g_sourceParams.audio_pid);
                        else
                            sscanf(argv[i], "%u", &g_sourceParams.audio_pid);
                    }
                    else
                    {
                        printf("--- Must specifiy an audio pid in hex format\n");
                        return 1;
                    }
                }
                else
                {
                    printf("--- Must specify a frequency in MHz\n");
                    return 1;
                }
            }
            else
            {
                return 1;
            }
        }
    }
    return (0);
}

static NEXUS_SurfaceHandle client_flip(void *context)
{
    BSTD_UNUSED(context);
    /* we are managing the graphics framebuffers, flip offscreen and onscreen */
    NEXUS_Display_SetGraphicsFramebuffer(g_display, g_cur_framebuffer?g_framebuffer2:g_framebuffer1);
    g_cur_framebuffer = 1 - g_cur_framebuffer;
    /* TODO: wait for display event */
    return g_cur_framebuffer?g_framebuffer2:g_framebuffer1;
}

static void client_get_framebuffer_dimensions(void *context, NEXUS_VideoFormat *format, unsigned *width, unsigned *height)
{
    NEXUS_DisplaySettings displaySettings;
    BSTD_UNUSED(context);
    NEXUS_Display_GetSettings( g_display, &displaySettings );
    *format = displaySettings.format;
    switch( displaySettings.format )
    {
        case NEXUS_VideoFormat_eCustom0:    /* For DTV panels */
        case NEXUS_VideoFormat_e1080i:
            *height = 1080;
            *width = 1920;
            break;
        case NEXUS_VideoFormat_e720p:
            *height = 720;
            *width = 1280;
            break;
        default:
            *height = 480;
            *width = 720;
            break;
    }
}

int initialize(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoInputVbiSettings viVbiSettings;

    /* playback support */
    NEXUS_FilePlayHandle file;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings pidSettings;

    /* frontend support */
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendVsbSettings vsbSettings;
    NEXUS_FrontendQamSettings qamSettings;

    BDCC_WINLIB_Interface winlibInterface;
    BDCC_WINLIB_OpenSettings openSettings;
    B_Dcc_Settings ccEngineSettings ;
    uint32_t scaleFactor;

    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_GraphicsSettings graphicsSettings;

    NEXUS_SurfaceMemory surfmem1, surfmem2;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = (g_sourceParams.inputSrc == CCTest_Input_QAM) || (g_sourceParams.inputSrc == CCTest_Input_VSB);
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_Platform_GetConfiguration(&g_platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);

    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = gVideoFormat[g_DisplayMode];
    g_display = NEXUS_Display_Open(0, &displaySettings);
    /* this app supports component out only */
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(g_display, NEXUS_ComponentOutput_GetConnector(g_platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(g_display, NEXUS_HdmiOutput_GetVideoConnector(g_platformConfig.outputs.hdmi[0]));
#endif

    /* create the nexus surfaces to be used as frame buffers */
    NEXUS_Surface_GetDefaultCreateSettings( &surfaceCreateSettings );
    surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
#ifdef ARGB2222_CANVAS
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_ePalette8;
#else
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
#endif

    /*
    ** The following size accomodates all display formats - where the horiz
    ** disp size is larger (e.g.1080i) the horizontal filter is automatically invoked,
    ** when the vertical disp size is larger it will automatically be cropped
    */
    surfaceCreateSettings.width = 640;
    surfaceCreateSettings.height = 1080;

    /* allocate the double buffer */
    g_framebuffer1 =  NEXUS_Surface_Create( &surfaceCreateSettings );
    BDBG_ASSERT(g_framebuffer1);
    g_framebuffer2 =  NEXUS_Surface_Create( &surfaceCreateSettings );
    BDBG_ASSERT(g_framebuffer2);

    NEXUS_Display_GetGraphicsSettings(g_display, &graphicsSettings);
    graphicsSettings.enabled = true;
    /* graphicsSettings.position will default to the display size */
    graphicsSettings.clip.width = surfaceCreateSettings.width;
    graphicsSettings.clip.height = gDisplayHeight[g_DisplayMode];
    NEXUS_Display_SetGraphicsSettings(g_display, &graphicsSettings);

    NEXUS_Surface_GetMemory(g_framebuffer1, &surfmem1);
    NEXUS_Surface_GetMemory(g_framebuffer2, &surfmem2);

    /* fill with transparent */
    BKNI_Memset(surfmem1.buffer, 0x00, surfaceCreateSettings.height * surfmem1.pitch);
    BKNI_Memset(surfmem2.buffer, 0x00, surfaceCreateSettings.height * surfmem2.pitch);


#ifdef ARGB2222_CANVAS
    BDBG_ASSERT(surfmem1.numPaletteEntries == 256);

    /* if either the canvas or the source surfaces are ARGB2222 then fill the palette */
    for( i=0 ; i< 256; i++)
    {
        surfmem1.palette[i] = surfmem2.palette[i] = ARGB_2222_TO_ARGB_8888(i);
    }
#endif

    NEXUS_Surface_Flush(g_framebuffer1);
    NEXUS_Surface_Flush(g_framebuffer2);

    printf("framebuffer1 %p, %dx%d, pitch %d\n\n", surfmem1.buffer,
        surfaceCreateSettings.width, surfaceCreateSettings.height, surfmem1.pitch);
    printf("framebuffer2 %p, %dx%d, pitch %d\n\n", surfmem2.buffer,
        surfaceCreateSettings.width, surfaceCreateSettings.height, surfmem2.pitch);

    /*
    ** Initialize the Closed Caption window library.
    ** This library interfaces to the customer window environment
    */
    BDCC_WINLIB_GetDefaultOpenSettings(&openSettings);
    openSettings.flip = client_flip;
    openSettings.get_framebuffer_dimensions = client_get_framebuffer_dimensions;
    openSettings.framebufferWidth = surfaceCreateSettings.width;
    openSettings.framebufferHeight = surfaceCreateSettings.height;
    if( BDCC_WINLIB_Open(&g_WindowLib, &openSettings)) {
        printf("Winlib Returned with an Error");
        goto error ;
    }

    /* get the winlib function table */
    BDCC_WINLIB_GetInterface( &winlibInterface );
    loadFonts( g_WindowLib, &scaleFactor );

    if( B_Dcc_Open( &g_ccEngine, g_WindowLib, &winlibInterface ) )
    {
        printf("B_Dcc_Open  Returned with an Error\n");
        goto error ;
    }

    B_Dcc_GetDefaultSettings( &ccEngineSettings );
    ccEngineSettings.iSafeTitleX = gDisplayWidth[g_DisplayMode] / 10; /* 20% of width / 2 according to CEB-10 */;
    ccEngineSettings.iSafeTitleY = gDisplayHeight[g_DisplayMode] / 10; /* 20% of height / 2 according to CEB-10 */;
    ccEngineSettings.ScaleFactor = scaleFactor;
    ccEngineSettings.Columns     = gCaptionColumns[g_DisplayMode]; /* 32 for 4:3 42 for 16:9 */;
    B_Dcc_SetSettings( g_ccEngine, &ccEngineSettings );

    /* let's start with 708 caption service 1 */
    g_UserParams.ccMode =  B_Dcc_Type_e708;
    g_UserParams.ccService = 1;
    g_UserParams.dispOn = true;

    if( B_Dcc_Init( g_ccEngine, g_UserParams.ccService, g_UserParams.ccMode ) )
    {
        printf("B_Dcc_Init  Returned with an Error\n");
        goto error ;
    }

    BKNI_CreateEvent(&g_newCcDataEvent);

    /* Open STC channel */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto; /* PVR */
    stcSettings.modeSettings.Auto.transportType = NEXUS_TransportType_eTs;
    g_stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    BDBG_ASSERT(NULL != g_stcChannel);

    if(( CCTest_Input_File == g_sourceParams.inputSrc ) ||
        ( CCTest_Input_VSB == g_sourceParams.inputSrc ) ||
        ( CCTest_Input_QAM == g_sourceParams.inputSrc ))

    {
        if( CCTest_Input_File == g_sourceParams.inputSrc )
        {
            printf("Playing back from file\n");

            g_playpump = NEXUS_Playpump_Open(0, NULL);
            BDBG_ASSERT(g_playpump);
            g_playback = NEXUS_Playback_Create();
            BDBG_ASSERT(g_playback);

            file = NEXUS_FilePlay_OpenPosix(g_sourceParams.szFile, NULL);
            if (!file) {
                fprintf(stderr, "can't open file:%s index:%s\n", g_sourceParams.szFile, "\n");
                goto error;
            }
            NEXUS_Playback_GetSettings(g_playback, &playbackSettings);
            playbackSettings.playpump = g_playpump;
            playbackSettings.stcChannel = g_stcChannel;
            NEXUS_Playback_SetSettings(g_playback, &playbackSettings);
        }
        if( CCTest_Input_VSB == g_sourceParams.inputSrc )
        {
            uint32_t i;

            printf("Tuning VSB freq = %dMHz\n", g_sourceParams.frequency);

            /* work out if we can support VSB */
            for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
            {
                NEXUS_FrontendCapabilities capabilities;
                g_frontend = g_platformConfig.frontend[i];
                if (g_frontend) {
                    NEXUS_Frontend_GetCapabilities(g_frontend, &capabilities);
                    /* Does this frontend support vsb? */
                    if ( capabilities.vsb )
                    {
                        break;
                    }
                }
            }

            if (NULL == g_frontend )
            {
                fprintf(stderr, "Unable to find VSB-capable frontend\n");
                goto error;
            }

            NEXUS_Frontend_GetDefaultVsbSettings(&vsbSettings);
            vsbSettings.frequency = g_sourceParams.frequency * 1000000;
            vsbSettings.mode = NEXUS_FrontendVsbMode_e8;
            vsbSettings.lockCallback.callback = NULL;
            NEXUS_Frontend_TuneVsb(g_frontend, &vsbSettings);
        }

        else if( CCTest_Input_QAM == g_sourceParams.inputSrc )
        {
            uint32_t i;

            printf("Tuning QAM freq = %dMHz\n", g_sourceParams.frequency);

            /* work out if we can support QAM */
            for ( i = 0; i < NEXUS_MAX_FRONTENDS; i++ )
            {
                NEXUS_FrontendCapabilities capabilities;
                g_frontend = g_platformConfig.frontend[i];
                if (g_frontend) {
                    NEXUS_Frontend_GetCapabilities(g_frontend, &capabilities);
                    /* Does this frontend support qam? */
                    if ( capabilities.qam )
                    {
                        break;
                    }
                }
            }

            if (NULL == g_frontend )
            {
                fprintf(stderr, "Unable to find QAM-capable frontend\n");
                goto error;
            }

            NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
            qamSettings.frequency = g_sourceParams.frequency * 1000000;
            qamSettings.mode = NEXUS_FrontendQamMode_e64;
            qamSettings.lockCallback.callback = NULL;
            NEXUS_Frontend_TuneQam(g_frontend, &qamSettings);
        }

        g_window = NEXUS_VideoWindow_Open(g_display, 0);

        /* bring up audio/video decoders and connect to display */
        g_videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
        BDBG_ASSERT(NULL != g_videoDecoder);
        g_videoInput = NEXUS_VideoDecoder_GetConnector(g_videoDecoder);
        BDBG_ASSERT(NULL != g_videoInput);
        NEXUS_VideoWindow_AddInput(g_window, g_videoInput);

        g_audioDecoder = NEXUS_AudioDecoder_Open(0, NULL); /* take default capabilities */
        BDBG_ASSERT(NULL != g_audioDecoder);
        g_audioInput = NEXUS_AudioDecoder_GetConnector(g_audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
        BDBG_ASSERT(NULL != g_videoInput);
        NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]), g_audioInput);

        NEXUS_VideoDecoder_GetSettings(g_videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.userDataEnabled = true;
        videoDecoderSettings.appUserDataReady.callback = userdata;
        videoDecoderSettings.appUserDataReady.context = g_videoDecoder;
        NEXUS_VideoDecoder_SetSettings(g_videoDecoder, &videoDecoderSettings);

        /* set this for EIA-608 capture */
        NEXUS_VideoInput_GetVbiSettings(NEXUS_VideoDecoder_GetConnector(g_videoDecoder), &viVbiSettings);
        viVbiSettings.closedCaptionEnabled = true;
        viVbiSettings.closedCaptionBufferSize = 200;
        NEXUS_VideoInput_SetVbiSettings(NEXUS_VideoDecoder_GetConnector(g_videoDecoder), &viVbiSettings);

        /* Open the pid channels */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&g_videoProgram);
        g_videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
        NEXUS_AudioDecoder_GetDefaultStartSettings(&g_audioProgram);
        g_audioProgram.codec = NEXUS_AudioCodec_eAc3;

        NEXUS_StcChannel_GetSettings(g_stcChannel, &stcSettings);
        if (g_playpump)
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);

            pidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            pidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
            pidSettings.pidTypeSettings.video.decoder = g_videoDecoder;
            pidSettings.pidTypeSettings.video.index = false;
            g_videoProgram.pidChannel = g_sourceParams.pidChannel =
                NEXUS_Playback_OpenPidChannel(g_playback, g_sourceParams.video_pid, &pidSettings);
            BDBG_ASSERT(g_videoProgram.pidChannel);

            NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);

            pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            pidSettings.pidTypeSettings.audio.primary = g_audioDecoder;
            pidSettings.pidSettings.pidTypeSettings.audio.codec = NEXUS_AudioCodec_eAc3;
            g_audioProgram.pidChannel = g_sourceParams.pidChannel =
                NEXUS_Playback_OpenPidChannel(g_playback, g_sourceParams.audio_pid, &pidSettings);
            BDBG_ASSERT(g_audioProgram.pidChannel);
            stcSettings.mode = NEXUS_StcChannelMode_eAuto; /* PVR */
            stcSettings.modeSettings.Auto.transportType = NEXUS_TransportType_eTs;
        }
        else
        {
            NEXUS_Frontend_GetUserParameters(g_frontend, &userParams);
            g_videoProgram.pidChannel = g_sourceParams.pidChannel =
                NEXUS_PidChannel_Open(userParams.param1, g_sourceParams.video_pid, NULL);
            BDBG_ASSERT(g_videoProgram.pidChannel);
            g_audioProgram.pidChannel = g_sourceParams.pidChannel =
                NEXUS_PidChannel_Open(userParams.param1, g_sourceParams.audio_pid, NULL);
            BDBG_ASSERT(g_audioProgram.pidChannel);
            stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
            stcSettings.modeSettings.pcr.pidChannel = g_videoProgram.pidChannel; /* PCR on video pid [default] */
        }
        NEXUS_StcChannel_SetSettings(g_stcChannel, &stcSettings);
        g_videoProgram.stcChannel = g_stcChannel;
        g_audioProgram.stcChannel = g_stcChannel;

        NEXUS_VideoDecoder_Start(g_videoDecoder, &g_videoProgram);
        NEXUS_AudioDecoder_Start(g_audioDecoder, &g_audioProgram);

        if( CCTest_Input_File == g_sourceParams.inputSrc ) {
            NEXUS_Playback_Start(g_playback, file, NULL);
        }

    }

    return 0;
error:
    return 1;
}



int32_t mainLoop(void)
{
    B_Dcc_OverRides overrideValues;
    unsigned int overrideMask;
    uint32_t ccService = 1;
    B_Dcc_Type ccMode =  B_Dcc_Type_e708;
    bool dispOn = true;

    uint32_t numEntries, numValidEntries, i;
    NEXUS_ClosedCaptionData captionData[ CC_DATA_BUF_SIZE ];
    CCTest_Caption_Triplets cc708UserData[ CC_DATA_BUF_SIZE ];

#ifdef SCALE_720P_FONTS
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    B_Dcc_Settings ccEngineSettings ;
#endif

    BKNI_Memset( &overrideValues, 0, sizeof( B_Dcc_OverRides ));
    overrideMask = 0;
    BKNI_Memset( &g_overrideValues, 0, sizeof( B_Dcc_OverRides ));
    g_overrideMask = 0;

    /*
    ** Now loop reading PictureUser Data queued by the VBI ISR and feeding it to the CC engine.
    ** Also, process any input from the "viewer".
    */
    while (!g_UserParams.exit)
    {
        /*
        ** Wait for new CC data or "x" milliseconds.
        ** The timeout allows B_Dcc_Process() to be called on a periodic
        ** basis, which in turn drives the time dependent actions (scrolling, flashing).
        */
        BKNI_WaitForEvent( g_newCcDataEvent, 1 );

        if( NEXUS_VideoInput_ReadClosedCaption(NEXUS_VideoDecoder_GetConnector(g_videoDecoder), captionData, CC_DATA_BUF_SIZE, &numEntries) )
        {
            printf("NEXUS_VideoDecoder_GetEia708Buffer returned error\n");
            continue;
        }

        for ( i=0, numValidEntries = 0; i < numEntries; i++ )
        {
            /* bit 1 of 'field' should be set for 708 data, clear for 608 data */
            if(((( captionData[ i ].field & 0x2) == 2 ) ^ ( B_Dcc_Type_e608 == ccMode )) && !captionData[i].noData)
            {
                cc708UserData[ numValidEntries ].ccType  = (uint8_t)(captionData[ i ].field) ;
                cc708UserData[ numValidEntries ].data[0] = captionData[ i ].data[0] ;
                cc708UserData[ numValidEntries ].data[1] = captionData[ i ].data[1] ;
                numValidEntries++;
            }
        }

        /*
        ** Call  B_Dcc_Process() even if new data is not available.
        ** This drives the time depending events; scrolling, flashing.
        */
        if( B_Dcc_Process(g_ccEngine, (unsigned char *)cc708UserData, numValidEntries))
        {
            printf("B_Dcc_Process returned with an Error\n");
            goto error ;
        }

        /*
        ** Check for "viewer" input.  Use "g_bUserInputValid" as an "event" to signal that
        ** viewer input is available. This is not robust, but should be fine for this sample.
        */
        if ( true == g_bUserInputValid )
        {
            /*
            ** A new caption mode or service has been specified.
            */
            if (( ccService != g_UserParams.ccService ) || ( ccMode != g_UserParams.ccMode ))
            {
                ccService = g_UserParams.ccService ;
                ccMode = g_UserParams.ccMode ;

                B_Dcc_Reset( g_ccEngine, NULL, ccMode, ccService );
            }

            /*
            ** Display has been switched on or off (used for sharing framebuffer with application).
            */
            if ( dispOn!= g_UserParams.dispOn )
            {
                NEXUS_SurfaceMemory surfmem1, surfmem2;
                NEXUS_SurfaceCreateSettings surfaceCreateSettings;
                BDCC_WINLIB_Interface winlibInterface;


                NEXUS_Surface_GetCreateSettings(g_framebuffer1, &surfaceCreateSettings);
                NEXUS_Surface_GetMemory(g_framebuffer1, &surfmem1);
                NEXUS_Surface_GetMemory(g_framebuffer2, &surfmem2);
                BDCC_WINLIB_GetInterface( &winlibInterface );

                dispOn = g_UserParams.dispOn;

                if(!dispOn)
                {
                    winlibInterface.HideDisp(g_WindowLib, true);
                }

                /* fill with transparent */
                BKNI_Memset(surfmem1.buffer, 0x00, surfaceCreateSettings.height * surfmem1.pitch);
                BKNI_Memset(surfmem2.buffer, 0x00, surfaceCreateSettings.height * surfmem2.pitch);

                NEXUS_Surface_Flush(g_framebuffer1);
                NEXUS_Surface_Flush(g_framebuffer2);

                if(dispOn)
                {
                    winlibInterface.HideDisp(g_WindowLib, false);
                }

            }

            /*
            ** An override has been requested
            */
            if ((g_overrideMask != overrideMask) || memcmp(&g_overrideValues, &overrideValues, sizeof(g_overrideValues)))
            {
                overrideMask = g_overrideMask;
                memcpy(&overrideValues, &g_overrideValues, sizeof(overrideValues));
                B_Dcc_Override( g_ccEngine, overrideMask, &overrideValues );
            }

#ifdef SCALE_720P_FONTS
            /*
            ** Iterate throught display formats, currently restricted to font scaling modes to avoid unload/reload of fonts
            */
            if ( 0 != g_UserParams.changeDisp )
            {
                g_UserParams.changeDisp = 0;

                B_Dcc_GetDefaultSettings(&ccEngineSettings);
                NEXUS_Display_GetGraphicsSettings( g_display, &graphicsSettings);
                NEXUS_Display_GetSettings( g_display, &displaySettings );

                g_DisplayMode = (g_DisplayMode + 1) % CCTest_Display_Max_Size;

                graphicsSettings.clip.width         = 640;
                graphicsSettings.clip.height        = gDisplayHeight[g_DisplayMode];
                displaySettings.format              = gVideoFormat[g_DisplayMode] ;
                graphicsSettings.position.width     = gDisplayWidth[g_DisplayMode];
                graphicsSettings.position.height    = gDisplayHeight[g_DisplayMode];
                ccEngineSettings.ScaleFactor        = gScaleFactor[g_DisplayMode];

                /* 708 spec.. 32 for 4:3, 42 for 16:9 */
                ccEngineSettings.Columns            = gCaptionColumns[g_DisplayMode];

                /* 10% of width/height according to CEB-10 */
                ccEngineSettings.iSafeTitleX        = gDisplayWidth[g_DisplayMode] / 10;
                ccEngineSettings.iSafeTitleY        = gDisplayHeight[g_DisplayMode] / 10;

                B_Dcc_SetSettings( g_ccEngine, &ccEngineSettings );
                B_Dcc_Reset( g_ccEngine, NULL, g_UserParams.ccMode, g_UserParams.ccService );

                displaySettings.displayType = NEXUS_DisplayType_eAuto;

                if ( NEXUS_Display_SetSettings( g_display, &displaySettings ))
                {
                    printf("--- NEXUS_Display_SetSettings returned error\n" );
                    goto error ;
                }

                graphicsSettings.enabled = true;
                NEXUS_Display_SetGraphicsSettings( g_display, &graphicsSettings);

                B_Dcc_SetSettings( g_ccEngine, &ccEngineSettings );
                B_Dcc_Reset( g_ccEngine, NULL, g_UserParams.ccMode, g_UserParams.ccService );
            }
#endif
            g_bUserInputValid = false;
        }
    }

    return 0;
error:
    return 1;
}



void shutdown(void)
{
    if( g_audioDecoder )
    {
        NEXUS_AudioDecoder_Stop(g_audioDecoder);
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]));
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(g_audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        NEXUS_AudioDecoder_Close(g_audioDecoder);
        if (!g_playback)
          NEXUS_PidChannel_Close(g_audioProgram.pidChannel);
    }

    if( g_videoDecoder )
    {
        NEXUS_VideoDecoder_Stop(g_videoDecoder);
        if ( g_playback )
        {
            NEXUS_Playback_Stop(g_playback);
            NEXUS_Playback_ClosePidChannel(g_playback, g_videoProgram.pidChannel);
            NEXUS_Playback_Destroy(g_playback);
            NEXUS_Playpump_Close(g_playpump);
        }
        else
        {
            NEXUS_PidChannel_Close(g_videoProgram.pidChannel);
        }

        NEXUS_VideoWindow_RemoveAllInputs(g_window);
        NEXUS_VideoInput_Shutdown(g_videoInput);
        NEXUS_VideoWindow_Close(g_window);
        NEXUS_VideoDecoder_Close(g_videoDecoder);
    }

    BKNI_DestroyEvent(g_newCcDataEvent);
    if( g_ccEngine) B_Dcc_Close( g_ccEngine );
    if( g_WindowLib ) BDCC_WINLIB_UnloadFonts( g_WindowLib );
    if( g_WindowLib ) BDCC_WINLIB_Close( g_WindowLib );

    if( g_display ) NEXUS_Display_Close(g_display);
    NEXUS_Surface_Destroy( g_framebuffer1 );
    NEXUS_Surface_Destroy( g_framebuffer2 );
}



void viewerInputThread(void *data)
{
    char chInput[ MAX_INPUT_CHARS ];
    bool bRet;
    BSTD_UNUSED(data);

    do
    {
        fgets( chInput, MAX_INPUT_CHARS, stdin );

        bRet = parseInputStr( chInput );

        if ( true == bRet )
        {
            /*
            ** signal that the CC mode has changed
            */
            g_bUserInputValid = true;
        }

    }
    while( g_UserParams.exit == false );
}

/*
** Maps the input string to the appropriate command.
** For parsing user input from the command line and during runtime.
*/

bool parseInputStr( const char * pszInputStr )
{
    bool bRet = true;
    char szInputStr[ MAX_INPUT_CHARS ];
    int inputData=0;
    int iConverted;

    BKNI_Memset( szInputStr, 0, ( sizeof( szInputStr) / sizeof( char ) ));

    iConverted = sscanf( pszInputStr, "%127s %d", szInputStr, &inputData );

    if ( !iConverted )
    {
        bRet = false;
        goto done;
    }

    if ( !strcmp( szInputStr, "cc" ) )
    {
        g_UserParams.ccMode = B_Dcc_Type_e608;

        g_UserParams.ccService =  ( BDCC_Min_608_Serivce <= inputData && BDCC_Max_608_Service >= inputData ) ? inputData : 1 ;

        printf("CCTest_P_ParseInputStr: selecting CC%d \n", g_UserParams.ccService );
    }

    else if ( !strcmp( szInputStr, "cs" ) )
    {
        g_UserParams.ccMode = B_Dcc_Type_e708;

        g_UserParams.ccService =  ( BDCC_Min_708_Serivce <= inputData && BDCC_Max_708_Service >= inputData ) ? inputData : 1 ;

        printf("CCTest_P_ParseInputStr: selecting CS%d \n", g_UserParams.ccService );
    }

    else if ( !strcmp( szInputStr, "fontsize" ) || !strcmp( szInputStr, "fsi" ) )
    {
        if(((inputData >= BDCC_PenSize_Small) && (inputData < BDCC_PenSize_Max_Size)) || inputData == 256)
        {
            if(inputData != 256)
            {
                char *pPenSizes[]={ "Small", "Regular", "Large"};

                g_overrideValues.PenSize = inputData;
                g_overrideMask |= BDCC_ATTR_MASK_PENSIZE;
                printf("CCTest_P_ParseInputStr: setting fontsize %s\n", pPenSizes[inputData] );
            }
            else
            {
                g_overrideMask &= ~BDCC_ATTR_MASK_PENSIZE;
                printf("CCTest_P_ParseInputStr: removing fontsize override\n");
            }
        }
        else
        {
            printf("CCTest_P_ParseInputStr: invalid fontsize\n");
        }
    }

    else if ( !strcmp( szInputStr, "fontstyle" ) ||  !strcmp( szInputStr, "fst" ) )
    {
        if( (inputData >= BDCC_FontStyle_Default && inputData < BDCC_FontStyle_Max_Value ) || (inputData == 256))
        {
            if(inputData < 256)
            {
                char *pFontStyles[]={ "Default", "MonoSerifs", "PropSerifs", "Mono", "Prop", "Casual", "Cursive", "SmallCaps"};

                g_overrideValues.FontStyle = inputData ;
                g_overrideMask |= BDCC_ATTR_MASK_FONTSTYLE ;
                printf("CCTest_P_ParseInputStr: setting fontstyle %s\n", pFontStyles[inputData] );
            }
            else
            {
                g_overrideMask &= ~BDCC_ATTR_MASK_FONTSTYLE ;
                printf("CCTest_P_ParseInputStr: removing fontstyle override\n");
            }
        }
        else
        {
            printf("CCTest_P_ParseInputStr: invalid fontstyle\n");
        }
    }

    else if ( !strcmp( szInputStr, "edgestyle" ) ||  !strcmp( szInputStr, "es" ) )
    {
        if (( inputData >= BDCC_Edge_Style_None && inputData < BDCC_Edge_Style_Max_Value) || (inputData == 256))
        {
            if(inputData < 256)
            {
                char *pEdgeType[]={ "None", "Raised", "Depressed", "Uniform", "LeftDropShadow", "RightDropShadow"};

                g_overrideValues.EdgeType = inputData ;
                g_overrideMask |= BDCC_ATTR_MASK_EDGETYPE;
                printf("CCTest_P_ParseInputStr: setting edge style %s\n", pEdgeType[inputData] );
            }
            else
            {
                g_overrideMask &= ~BDCC_ATTR_MASK_EDGETYPE;
                printf("CCTest_P_ParseInputStr: removing edge style override\n");
            }
        }
        else
        {
            printf("CCTest_P_ParseInputStr: invalid edge style\n");
        }
    }

    else if ( !strcmp( szInputStr, "edgecolor" ) || !strcmp( szInputStr, "ec" )  )
    {

        /*
        ** 2 bits each for RGB.
        */
        if((inputData < 64) || (inputData == 256))
        {

            if(inputData < 256)
        {
                g_overrideValues.EdgeColor = inputData ;
                g_overrideMask |= BDCC_ATTR_MASK_EDGECOLOR;

                printf("CCTest_P_ParseInputStr: setting edge color red=%d, green=%d, blue=%d\n",
                    (inputData & 0x30) >> 4,
                    (inputData & 0x0C) >> 2,
                    (inputData & 0x03));
            }
            else
            {
                g_overrideMask &= ~BDCC_ATTR_MASK_EDGECOLOR;
                printf("CCTest_P_ParseInputStr: removing edge color override\n");
            }
        }
        else
        {
            printf("CCTest_P_ParseInputStr: invalid edge color\n");
        }
    }


    else if ( !strcmp( szInputStr, "fgcolor" ) || !strcmp( szInputStr, "fgc" )  )
    {
        /*
        ** 2 bits each for RGB.
        */

		if((inputData < 64) || (inputData == 64))
		{

			if(inputData < 64)
			{
				g_overrideValues.FgColor = inputData ;
				g_overrideMask |= BDCC_ATTR_MASK_PENFG;

				printf("CCTest_P_ParseInputStr: setting pen fg red=%d, green=%d, blue=%d\n",
					(inputData & 0x30) >> 4,
					(inputData & 0x0C) >> 2,
					(inputData & 0x03));
			}
			else
			{
				g_overrideMask &= ~BDCC_ATTR_MASK_PENFG;
				printf("CCTest_P_ParseInputStr: removing pen fg color override\n");
			}
		}
		else
		{
			printf("CCTest_P_ParseInputStr: invalid pen fg color\n");
		}

    }

	else if ( !strcmp( szInputStr, "fgopacity" ) || !strcmp( szInputStr, "fgo" )	)
	{
		/*
		** 2 bits .
		*/

		if((inputData < 4) || (inputData == 4))
		{

			if(inputData < 4)
			{
				char *pOpacityType[]={"Solid", "Flash", "Translucent", "Transparent"};
				g_overrideValues.FgOpacity = inputData ;
				g_overrideMask |= BDCC_ATTR_MASK_PENFGOP;

				printf("CCTest_P_ParseInputStr: setting pen fg opacity type %s\n",
					pOpacityType[inputData]);
				}
			else
			{
				g_overrideMask &= ~BDCC_ATTR_MASK_PENFGOP;
				printf("CCTest_P_ParseInputStr: removing pen fg opacity override\n");
			}
		}
		else
		{
			printf("CCTest_P_ParseInputStr: invalid pen fg opacity\n");
		}
	}

    else if ( !strcmp( szInputStr, "bgcolor" ) || !strcmp( szInputStr, "bgc" )  )
    {
        /*
        ** 2 bits each for ARGB.
        */
		if((inputData < 64) || (inputData == 64))
		{

			if(inputData < 64)
			{
				g_overrideValues.BgColor = inputData ;
				g_overrideMask |= BDCC_ATTR_MASK_PENBG;

				printf("CCTest_P_ParseInputStr: setting pen bg red=%d, green=%d, blue=%d\n",
				(inputData & 0x30) >> 4,
				(inputData & 0x0C) >> 2,
				(inputData & 0x03));
			}
			else
			{
				g_overrideMask &= ~BDCC_ATTR_MASK_PENBG;
				printf("CCTest_P_ParseInputStr: removing pen bg color override\n");
			}
		}
		else
		{
			printf("CCTest_P_ParseInputStr: invalid pen bg color\n");
		}

    }

	else if ( !strcmp( szInputStr, "bgopacity" ) || !strcmp( szInputStr, "bgo" )	)
	{
		/*
		** 2 bits .
		*/

		if((inputData < 4) || (inputData == 4))
		{

			if(inputData < 4)
			{
				char *pOpacityType[]={"Solid", "Flash", "Translucent", "Transparent"};
				g_overrideValues.BgOpacity = inputData ;
				g_overrideMask |= BDCC_ATTR_MASK_PENBGOP;

				printf("CCTest_P_ParseInputStr: setting pen bg opacity type %s\n",
					pOpacityType[inputData]);
				}
			else
			{
				g_overrideMask &= ~BDCC_ATTR_MASK_PENBGOP;
				printf("CCTest_P_ParseInputStr: removing pen bg opacity override\n");
			}
		}
		else
		{
			printf("CCTest_P_ParseInputStr: invalid pen bg opacity\n");
		}
	}


    else if ( !strcmp( szInputStr, "wincolor" ) || !strcmp( szInputStr, "winc" )  )
    {
        /*
        ** 2 bits each for ARGB.
        */
		if((inputData < 64) || (inputData == 64))
		{

			if(inputData < 64)
			{
				g_overrideValues.WinColor = inputData ;
				g_overrideMask |= BDCC_ATTR_MASK_FILL;

				printf("CCTest_P_ParseInputStr: setting win color red=%d, green=%d, blue=%d\n",
				(inputData & 0x30) >> 4,
				(inputData & 0x0C) >> 2,
				(inputData & 0x03));
			}
			else
			{
				g_overrideMask &= ~BDCC_ATTR_MASK_FILL;
				printf("CCTest_P_ParseInputStr: removing window color override\n");
			}
		}
		else
		{
			printf("CCTest_P_ParseInputStr: invalid window color\n");
		}

    }

	else if ( !strcmp( szInputStr, "winopacity" ) || !strcmp( szInputStr, "wino" )	)
	{
		/*
		** 2 bits .
		*/

		if((inputData < 4) || (inputData == 4))
		{

			if(inputData < 4)
			{
				char *pOpacityType[]={"Solid", "Flash", "Translucent", "Transparent"};
				g_overrideValues.WinOpacity = inputData ;
				g_overrideMask |= BDCC_ATTR_MASK_FILLOP;

				printf("CCTest_P_ParseInputStr: setting window opacity type %s\n",
					pOpacityType[inputData]);
				}
			else
			{
				g_overrideMask &= ~BDCC_ATTR_MASK_FILLOP;
				printf("CCTest_P_ParseInputStr: removing window opacity override\n");
			}
		}
		else
		{
			printf("CCTest_P_ParseInputStr: invalid window opacity\n");
		}
	}


    else if ( !strcmp( szInputStr, "cu" ) || !strcmp( szInputStr, "up" )  )
    {
        g_UserParams.channelChange = 1;
        printf("CCTest_P_ParseInputStr:  channel up\n");
    }

    else if ( !strcmp( szInputStr, "cd" ) || !strcmp( szInputStr, "down" )  )
    {
        g_UserParams.channelChange = -1;
        printf("CCTest_P_ParseInputStr:  channel down\n");
    }

#ifdef SCALE_720P_FONTS
    else if ( !strcmp( szInputStr, "d" ) || !strcmp( szInputStr, "disp" )  )
    {
        g_UserParams.changeDisp = 1;
    }
#endif

    else if ( !strcmp( szInputStr, "i" ) || !strcmp( szInputStr, "italic" )  )
    {
        if (inputData <= 256)
        {
            if (inputData < 256) {
                if (inputData == 0) {
                    printf("CCTest_P_ParseInputStr:  setting standard font override\n");
                }
                else if (inputData == 1) {
                    printf("CCTest_P_ParseInputStr:  setting italic font override\n");
                }
                else {
                    printf("Set 0 or 1 to override standard or italic font\n");
                }
            }
            else {
                printf("Removing italic/standard override\n");
            }
        }

        else {
            printf("Invalid Setting\n");
        }
    }
    else if ( !strcmp( szInputStr, "h" ) || !strcmp( szInputStr, "help" )  )
    {
        bRet = false;
        printHelpInfo();
    }

    else if ( !strcmp( szInputStr, "doff" ) || !strcmp( szInputStr, "dispoff" )  )
    {
        if(g_UserParams.dispOn)
        {
            g_UserParams.dispOn = false;
            printf("Disabling caption display\n");
        }
        else
        {
            printf("Caption display is already disabled\n");
        }
    }

    else if ( !strcmp( szInputStr, "don" ) || !strcmp( szInputStr, "dispon" )  )
    {
        if(!g_UserParams.dispOn)
        {
            g_UserParams.dispOn = true;
            printf("Enabling caption display\n");
        }
        else
        {
            printf("Caption display is already Enabled\n");
        }
    }

    else if ( !strcmp( szInputStr, "p")  ||  !strcmp( szInputStr, "preview" ))
    {
        char previewInputStr[ MAX_INPUT_CHARS ];
		int inputData1=256, inputData2=256, inputData3=256;

		if(!g_UserParams.previewOn)
		{
			printf("Enter values for penfg,pengbg, and wincolor.  Should be in range from 0 - 255 (2 bits each for opacity/r/g/b). Enter 256 to use default values.:\n");
			fgets( previewInputStr, MAX_INPUT_CHARS, stdin );
			sscanf( previewInputStr, "%d %d %d", &inputData1, &inputData2, &inputData3 );
			g_UserParams.previewOn = true;
			previewCaption(g_UserParams.previewOn, g_UserParams.ccMode, g_UserParams.ccService, inputData1, inputData2, inputData3);
			printf("Enabling caption preview\n");
		}
		else
		{
			g_UserParams.previewOn = false;
			previewCaption(g_UserParams.previewOn, g_UserParams.ccMode, g_UserParams.ccService, inputData1, inputData2, inputData3);
			printf("Disabling caption preview\n");
		}
    }

    else if ( !strcmp( szInputStr, "x")  ||  !strcmp( szInputStr, "exit" ))
    {
        g_UserParams.exit = true;
    }

    else
    {
        printf("CCTest_P_ParseInputStr:  unknown command %s\n", szInputStr );
        bRet = false;
    }



done:

    return bRet;

}


/*
** Illustrates how to load fonts and associate them with a specific pen "font style".
*/
void loadFonts( BDCC_WINLIB_Handle g_WindowLib, uint32_t *scaleFactor )
{
    int i, j, iErr;
    BDCC_FONT_DESCRIPTOR *pFont;

    BDCC_FONT_DESCRIPTOR standardFonts[ BDCC_FontStyle_Max_Value ][BDCC_PenSize_Max_Size];
    BDCC_FONT_DESCRIPTOR italicsFonts [ BDCC_FontStyle_Max_Value ][BDCC_PenSize_Max_Size];


#ifdef FREETYPE_SUPPORT

    char *pszFontFilenames[BDCC_FontStyle_Max_Value] =
    {
        "cc_fonts/cinecavB_type_03.ttf", /* BDCC_FontStyle_Default    */
        "cc_fonts/cinecavB_type_03.ttf",  /* BDCC_FontStyle_MonoSerifs */
        "cc_fonts/cinecavB_serif_02.ttf" ,/* BDCC_FontStyle_PropSerifs */
        "cc_fonts/cinecavB_mono_03.ttf",  /* BDCC_FontStyle_Mono       */
        "cc_fonts/cinecavB_sans_02.ttf",  /* BDCC_FontStyle_Prop       */
        "cc_fonts/cinecavB_casual_02.ttf",/* BDCC_FontStyle_Casual     */
        "cc_fonts/cinecavB_script_02.ttf",/* BDCC_FontStyle_Cursive    */
        "cc_fonts/cinecavB_sc_02.ttf"     /* BDCC_FontStyle_SmallCaps  */
    };

    int iFontSize[BDCC_FontStyle_Max_Value][CCTest_Display_Max_Size][BDCC_PenSize_Max_Size] =
    {
        /* 480i S M L      480p S M L      720p S M L     1080i S M L */

        {{ 14, 18, 22 }, { 14, 18, 22 }, { 21, 28, 33 }, { 32, 41, 49 }}, /* BDCC_FontStyle_Default */
        {{ 14, 18, 22 }, { 14, 18, 22 }, { 21, 28, 33 }, { 32, 41, 49 }}, /* BDCC_FontStyle_MonoSerifs */
        {{ 12, 15, 19 }, { 12, 15, 19 }, { 18, 23, 29 }, { 28, 35, 44 }}, /* BDCC_FontStyle_PropSerifs */
        {{ 14, 18, 22 }, { 14, 18, 22 }, { 21, 29, 34 }, { 34, 43, 52 }}, /* BDCC_FontStyle_Mono       */
        {{ 12, 15, 19 }, { 12, 15, 19 }, { 18, 23, 29 }, { 28, 35, 44 }}, /* BDCC_FontStyle_Prop       */
        {{ 13, 16, 20 }, { 13, 16, 20 }, { 19, 24, 30 }, { 30, 36, 45 }}, /* BDCC_FontStyle_Casual     */
        {{ 12, 14, 18 }, { 12, 14, 18 }, { 16, 18, 25 }, { 25, 28, 38 }}, /* BDCC_FontStyle_Cursive    */
        {{ 13, 15, 19 }, { 13, 15, 19 }, { 18, 23, 30 }, { 28, 35, 45 }}  /* BDCC_FontStyle_SmallCaps  */
    };

    /* we do not need to scale these fonts using the m2mc since the freetype engine can do it */
    *scaleFactor = 100 ;  /* scale factor in hundredths i.e. scale factor is 1.0 */

    printf("USING TRUE-TYPE FONTS\n");


#else

/* The following is a trade-off for pre-rendered fonts.
**
** We can use a base set of 14 pre-rendered fonts (7 normal and 7 italic)
** and generate the rest using the scalers but the scaler will reduce the font's quality
** or we can use a full set of 126 fonts to cover all of the font permutations with maximum quality
*/
#ifdef SCALE_480I_FONTS

 char *pszStandardFontFilenames[BDCC_FontStyle_Max_Value][ CCTest_Display_Max_Size ][ BDCC_PenSize_Max_Size ]=
 {
  /* non-italic */
  /* 480i S M L                                                                          480p S M L                                                           720p S M L                                   1080i S M L */

  {{ "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }}, /* BDCC_FontStyle_Default */
  {{ "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }}, /* BDCC_FontStyle_MonoSerifs */
  {{ "cc_fonts/cinecavB12_serif.bwin_font",  "cc_fonts/cinecavB15_serif.bwin_font",  "cc_fonts/cinecavB19_serif.bwin_font"  }, { "cc_fonts/cinecavB12_serif.bwin_font",  "cc_fonts/cinecavB15_serif.bwin_font",  "cc_fonts/cinecavB19_serif.bwin_font"  }, { "cc_fonts/cinecavB12_serif.bwin_font",  "cc_fonts/cinecavB15_serif.bwin_font",  "cc_fonts/cinecavB19_serif.bwin_font"  }, { "cc_fonts/cinecavB12_serif.bwin_font",  "cc_fonts/cinecavB15_serif.bwin_font",  "cc_fonts/cinecavB19_serif.bwin_font"  }}, /* BDCC_FontStyle_PropSerifs */
  {{ "cc_fonts/cinecavB14_mono.bwin_font",   "cc_fonts/cinecavB18_mono.bwin_font",   "cc_fonts/cinecavB22_mono.bwin_font"   }, { "cc_fonts/cinecavB14_mono.bwin_font",   "cc_fonts/cinecavB18_mono.bwin_font",   "cc_fonts/cinecavB22_mono.bwin_font"   }, { "cc_fonts/cinecavB14_mono.bwin_font",   "cc_fonts/cinecavB18_mono.bwin_font",   "cc_fonts/cinecavB22_mono.bwin_font"   }, { "cc_fonts/cinecavB14_mono.bwin_font",   "cc_fonts/cinecavB18_mono.bwin_font",   "cc_fonts/cinecavB22_mono.bwin_font"   }}, /* BDCC_FontStyle_Mono  */
  {{ "cc_fonts/cinecavB12_sans.bwin_font",   "cc_fonts/cinecavB15_sans.bwin_font",   "cc_fonts/cinecavB19_sans.bwin_font"   }, { "cc_fonts/cinecavB12_sans.bwin_font",   "cc_fonts/cinecavB15_sans.bwin_font",   "cc_fonts/cinecavB19_sans.bwin_font"   }, { "cc_fonts/cinecavB12_sans.bwin_font",   "cc_fonts/cinecavB15_sans.bwin_font",   "cc_fonts/cinecavB19_sans.bwin_font"   }, { "cc_fonts/cinecavB12_sans.bwin_font",   "cc_fonts/cinecavB15_sans.bwin_font",   "cc_fonts/cinecavB19_sans.bwin_font"   }}, /* BDCC_FontStyle_Prop  */
  {{ "cc_fonts/cinecavB13_casual.bwin_font", "cc_fonts/cinecavB16_casual.bwin_font", "cc_fonts/cinecavB20_casual.bwin_font" }, { "cc_fonts/cinecavB13_casual.bwin_font", "cc_fonts/cinecavB16_casual.bwin_font", "cc_fonts/cinecavB20_casual.bwin_font" }, { "cc_fonts/cinecavB13_casual.bwin_font", "cc_fonts/cinecavB16_casual.bwin_font", "cc_fonts/cinecavB20_casual.bwin_font" }, { "cc_fonts/cinecavB13_casual.bwin_font", "cc_fonts/cinecavB16_casual.bwin_font", "cc_fonts/cinecavB20_casual.bwin_font" }}, /* BDCC_FontStyle_Casual  */
  {{ "cc_fonts/cinecavB12_script.bwin_font", "cc_fonts/cinecavB14_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font" }, { "cc_fonts/cinecavB12_script.bwin_font", "cc_fonts/cinecavB14_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font" }, { "cc_fonts/cinecavB12_script.bwin_font", "cc_fonts/cinecavB14_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font" }, { "cc_fonts/cinecavB12_script.bwin_font", "cc_fonts/cinecavB14_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font" }}, /* BDCC_FontStyle_Cursive */
  {{ "cc_fonts/cinecavB13_sc.bwin_font",     "cc_fonts/cinecavB15_sc.bwin_font",     "cc_fonts/cinecavB19_sc.bwin_font"     }, { "cc_fonts/cinecavB13_sc.bwin_font",     "cc_fonts/cinecavB15_sc.bwin_font",     "cc_fonts/cinecavB19_sc.bwin_font"     }, { "cc_fonts/cinecavB13_sc.bwin_font",     "cc_fonts/cinecavB15_sc.bwin_font",     "cc_fonts/cinecavB19_sc.bwin_font"     }, { "cc_fonts/cinecavB13_sc.bwin_font",     "cc_fonts/cinecavB15_sc.bwin_font",     "cc_fonts/cinecavB19_sc.bwin_font"     }}  /* BDCC_FontStyle_SmallCaps */
  };


 char *pszItalicFontFilenames[BDCC_FontStyle_Max_Value][ CCTest_Display_Max_Size ][ BDCC_PenSize_Max_Size ]=
 {
  /* italic */

  {{ "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }}, /* BDCC_FontStyle_Default */
  {{ "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }}, /* BDCC_FontStyle_MonoSerifs */
  {{ "cc_fonts/cinecavB12i_serif.bwin_font",  "cc_fonts/cinecavB15i_serif.bwin_font",  "cc_fonts/cinecavB19i_serif.bwin_font"  }, { "cc_fonts/cinecavB12i_serif.bwin_font",  "cc_fonts/cinecavB15i_serif.bwin_font",  "cc_fonts/cinecavB19i_serif.bwin_font"  }, { "cc_fonts/cinecavB12i_serif.bwin_font",  "cc_fonts/cinecavB15i_serif.bwin_font",  "cc_fonts/cinecavB19i_serif.bwin_font"  }, { "cc_fonts/cinecavB12i_serif.bwin_font",  "cc_fonts/cinecavB15i_serif.bwin_font",  "cc_fonts/cinecavB19i_serif.bwin_font"  }}, /* BDCC_FontStyle_PropSerifs */
  {{ "cc_fonts/cinecavB14i_mono.bwin_font",   "cc_fonts/cinecavB18i_mono.bwin_font",   "cc_fonts/cinecavB22i_mono.bwin_font"   }, { "cc_fonts/cinecavB14i_mono.bwin_font",   "cc_fonts/cinecavB18i_mono.bwin_font",   "cc_fonts/cinecavB22i_mono.bwin_font"   }, { "cc_fonts/cinecavB14i_mono.bwin_font",   "cc_fonts/cinecavB18i_mono.bwin_font",   "cc_fonts/cinecavB22i_mono.bwin_font"   }, { "cc_fonts/cinecavB14i_mono.bwin_font",   "cc_fonts/cinecavB18i_mono.bwin_font",   "cc_fonts/cinecavB22i_mono.bwin_font"   }}, /* BDCC_FontStyle_Mono  */
  {{ "cc_fonts/cinecavB12i_sans.bwin_font",   "cc_fonts/cinecavB15i_sans.bwin_font",   "cc_fonts/cinecavB19i_sans.bwin_font"   }, { "cc_fonts/cinecavB12i_sans.bwin_font",   "cc_fonts/cinecavB15i_sans.bwin_font",   "cc_fonts/cinecavB19i_sans.bwin_font"   }, { "cc_fonts/cinecavB12i_sans.bwin_font",   "cc_fonts/cinecavB15i_sans.bwin_font",   "cc_fonts/cinecavB19i_sans.bwin_font"   }, { "cc_fonts/cinecavB12i_sans.bwin_font",   "cc_fonts/cinecavB15i_sans.bwin_font",   "cc_fonts/cinecavB19i_sans.bwin_font"   }}, /* BDCC_FontStyle_Prop  */
  {{ "cc_fonts/cinecavB13i_casual.bwin_font", "cc_fonts/cinecavB16i_casual.bwin_font", "cc_fonts/cinecavB20i_casual.bwin_font" }, { "cc_fonts/cinecavB13i_casual.bwin_font", "cc_fonts/cinecavB16i_casual.bwin_font", "cc_fonts/cinecavB20i_casual.bwin_font" }, { "cc_fonts/cinecavB13i_casual.bwin_font", "cc_fonts/cinecavB16i_casual.bwin_font", "cc_fonts/cinecavB20i_casual.bwin_font" }, { "cc_fonts/cinecavB13i_casual.bwin_font", "cc_fonts/cinecavB16i_casual.bwin_font", "cc_fonts/cinecavB20i_casual.bwin_font" }}, /* BDCC_FontStyle_Casual  */
  {{ "cc_fonts/cinecavB12i_script.bwin_font", "cc_fonts/cinecavB14i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font" }, { "cc_fonts/cinecavB12i_script.bwin_font", "cc_fonts/cinecavB14i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font" }, { "cc_fonts/cinecavB12i_script.bwin_font", "cc_fonts/cinecavB14i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font" }, { "cc_fonts/cinecavB12i_script.bwin_font", "cc_fonts/cinecavB14i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font" }}, /* BDCC_FontStyle_Cursive */
  {{ "cc_fonts/cinecavB13i_sc.bwin_font",     "cc_fonts/cinecavB15i_sc.bwin_font",     "cc_fonts/cinecavB19i_sc.bwin_font"     }, { "cc_fonts/cinecavB13i_sc.bwin_font",     "cc_fonts/cinecavB15i_sc.bwin_font",     "cc_fonts/cinecavB19i_sc.bwin_font"     }, { "cc_fonts/cinecavB13i_sc.bwin_font",     "cc_fonts/cinecavB15i_sc.bwin_font",     "cc_fonts/cinecavB19i_sc.bwin_font"     }, { "cc_fonts/cinecavB13i_sc.bwin_font",     "cc_fonts/cinecavB15i_sc.bwin_font",     "cc_fonts/cinecavB19i_sc.bwin_font"     }}  /* BDCC_FontStyle_SmallCaps */
  };

    /* we need to scale these fonts using the m2mc since we have specified the 480i/p set for all of the display formats */

    uint32_t display_ratios[4] = {100,100,150,225};
    *scaleFactor = display_ratios[g_DisplayMode] ;  /* scale factor in hundredths i.e. scale factor is 1.0 */

    printf("SCALING 480i FONTS\n");

#else

#ifdef SCALE_720P_FONTS
 char *pszStandardFontFilenames[BDCC_FontStyle_Max_Value][ CCTest_Display_Max_Size ][ BDCC_PenSize_Max_Size ]=
 {
  /* non-italic */
  /* 480i S M L                                                                          480p S M L                                                           720p S M L                                   1080i S M L */

  {{ "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }, { "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }, { "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }, { "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }}, /* BDCC_FontStyle_Default */
  {{ "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }, { "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }, { "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }, { "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }}, /* BDCC_FontStyle_MonoSerifs */
  {{ "cc_fonts/cinecavB18_serif.bwin_font",  "cc_fonts/cinecavB23_serif.bwin_font",  "cc_fonts/cinecavB29_serif.bwin_font"  }, { "cc_fonts/cinecavB18_serif.bwin_font",  "cc_fonts/cinecavB23_serif.bwin_font",  "cc_fonts/cinecavB29_serif.bwin_font"  }, { "cc_fonts/cinecavB18_serif.bwin_font",  "cc_fonts/cinecavB23_serif.bwin_font",  "cc_fonts/cinecavB29_serif.bwin_font"  }, { "cc_fonts/cinecavB18_serif.bwin_font",  "cc_fonts/cinecavB23_serif.bwin_font",  "cc_fonts/cinecavB29_serif.bwin_font"  }}, /* BDCC_FontStyle_PropSerifs */
  {{ "cc_fonts/cinecavB21_mono.bwin_font",   "cc_fonts/cinecavB29_mono.bwin_font",   "cc_fonts/cinecavB34_mono.bwin_font"   }, { "cc_fonts/cinecavB21_mono.bwin_font",   "cc_fonts/cinecavB29_mono.bwin_font",   "cc_fonts/cinecavB34_mono.bwin_font"   }, { "cc_fonts/cinecavB21_mono.bwin_font",   "cc_fonts/cinecavB29_mono.bwin_font",   "cc_fonts/cinecavB34_mono.bwin_font"   }, { "cc_fonts/cinecavB21_mono.bwin_font",   "cc_fonts/cinecavB29_mono.bwin_font",   "cc_fonts/cinecavB34_mono.bwin_font"   }}, /* BDCC_FontStyle_Mono  */
  {{ "cc_fonts/cinecavB18_sans.bwin_font",   "cc_fonts/cinecavB23_sans.bwin_font",   "cc_fonts/cinecavB29_sans.bwin_font"   }, { "cc_fonts/cinecavB18_sans.bwin_font",   "cc_fonts/cinecavB23_sans.bwin_font",   "cc_fonts/cinecavB29_sans.bwin_font"   }, { "cc_fonts/cinecavB18_sans.bwin_font",   "cc_fonts/cinecavB23_sans.bwin_font",   "cc_fonts/cinecavB29_sans.bwin_font"   }, { "cc_fonts/cinecavB18_sans.bwin_font",   "cc_fonts/cinecavB23_sans.bwin_font",   "cc_fonts/cinecavB29_sans.bwin_font"   }}, /* BDCC_FontStyle_Prop  */
  {{ "cc_fonts/cinecavB19_casual.bwin_font", "cc_fonts/cinecavB24_casual.bwin_font", "cc_fonts/cinecavB30_casual.bwin_font" }, { "cc_fonts/cinecavB19_casual.bwin_font", "cc_fonts/cinecavB24_casual.bwin_font", "cc_fonts/cinecavB30_casual.bwin_font" }, { "cc_fonts/cinecavB19_casual.bwin_font", "cc_fonts/cinecavB24_casual.bwin_font", "cc_fonts/cinecavB30_casual.bwin_font" }, { "cc_fonts/cinecavB19_casual.bwin_font", "cc_fonts/cinecavB24_casual.bwin_font", "cc_fonts/cinecavB30_casual.bwin_font" }}, /* BDCC_FontStyle_Casual  */
  {{ "cc_fonts/cinecavB16_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font", "cc_fonts/cinecavB25_script.bwin_font" }, { "cc_fonts/cinecavB16_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font", "cc_fonts/cinecavB25_script.bwin_font" }, { "cc_fonts/cinecavB16_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font", "cc_fonts/cinecavB25_script.bwin_font" }, { "cc_fonts/cinecavB16_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font", "cc_fonts/cinecavB25_script.bwin_font" }}, /* BDCC_FontStyle_Cursive */
  {{ "cc_fonts/cinecavB18_sc.bwin_font",     "cc_fonts/cinecavB23_sc.bwin_font",     "cc_fonts/cinecavB30_sc.bwin_font"     }, { "cc_fonts/cinecavB18_sc.bwin_font",     "cc_fonts/cinecavB23_sc.bwin_font",     "cc_fonts/cinecavB30_sc.bwin_font"     }, { "cc_fonts/cinecavB18_sc.bwin_font",     "cc_fonts/cinecavB23_sc.bwin_font",     "cc_fonts/cinecavB30_sc.bwin_font"     }, { "cc_fonts/cinecavB18_sc.bwin_font",     "cc_fonts/cinecavB23_sc.bwin_font",     "cc_fonts/cinecavB30_sc.bwin_font"     }}  /* BDCC_FontStyle_SmallCaps */
  };


 char *pszItalicFontFilenames[BDCC_FontStyle_Max_Value][ CCTest_Display_Max_Size ][ BDCC_PenSize_Max_Size ]=
 {
  /* italic */

  {{ "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }, { "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }, { "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }, { "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }}, /* BDCC_FontStyle_Default */
  {{ "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }, { "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }, { "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }, { "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }}, /* BDCC_FontStyle_MonoSerifs */
  {{ "cc_fonts/cinecavB18i_serif.bwin_font",  "cc_fonts/cinecavB23i_serif.bwin_font",  "cc_fonts/cinecavB29i_serif.bwin_font"  }, { "cc_fonts/cinecavB18i_serif.bwin_font",  "cc_fonts/cinecavB23i_serif.bwin_font",  "cc_fonts/cinecavB29i_serif.bwin_font"  }, { "cc_fonts/cinecavB18i_serif.bwin_font",  "cc_fonts/cinecavB23i_serif.bwin_font",  "cc_fonts/cinecavB29i_serif.bwin_font"  }, { "cc_fonts/cinecavB18i_serif.bwin_font",  "cc_fonts/cinecavB23i_serif.bwin_font",  "cc_fonts/cinecavB29i_serif.bwin_font"  }}, /* BDCC_FontStyle_PropSerifs */
  {{ "cc_fonts/cinecavB21i_mono.bwin_font",   "cc_fonts/cinecavB29i_mono.bwin_font",   "cc_fonts/cinecavB34i_mono.bwin_font"   }, { "cc_fonts/cinecavB21i_mono.bwin_font",   "cc_fonts/cinecavB29i_mono.bwin_font",   "cc_fonts/cinecavB34i_mono.bwin_font"   }, { "cc_fonts/cinecavB21i_mono.bwin_font",   "cc_fonts/cinecavB29i_mono.bwin_font",   "cc_fonts/cinecavB34i_mono.bwin_font"   }, { "cc_fonts/cinecavB21i_mono.bwin_font",   "cc_fonts/cinecavB29i_mono.bwin_font",   "cc_fonts/cinecavB34i_mono.bwin_font"   }}, /* BDCC_FontStyle_Mono  */
  {{ "cc_fonts/cinecavB18i_sans.bwin_font",   "cc_fonts/cinecavB23i_sans.bwin_font",   "cc_fonts/cinecavB29i_sans.bwin_font"   }, { "cc_fonts/cinecavB18i_sans.bwin_font",   "cc_fonts/cinecavB23i_sans.bwin_font",   "cc_fonts/cinecavB29i_sans.bwin_font"   }, { "cc_fonts/cinecavB18i_sans.bwin_font",   "cc_fonts/cinecavB23i_sans.bwin_font",   "cc_fonts/cinecavB29i_sans.bwin_font"   }, { "cc_fonts/cinecavB18i_sans.bwin_font",   "cc_fonts/cinecavB23i_sans.bwin_font",   "cc_fonts/cinecavB29i_sans.bwin_font"   }}, /* BDCC_FontStyle_Prop  */
  {{ "cc_fonts/cinecavB19i_casual.bwin_font", "cc_fonts/cinecavB24i_casual.bwin_font", "cc_fonts/cinecavB30i_casual.bwin_font" }, { "cc_fonts/cinecavB19i_casual.bwin_font", "cc_fonts/cinecavB24i_casual.bwin_font", "cc_fonts/cinecavB30i_casual.bwin_font" }, { "cc_fonts/cinecavB19i_casual.bwin_font", "cc_fonts/cinecavB24i_casual.bwin_font", "cc_fonts/cinecavB30i_casual.bwin_font" }, { "cc_fonts/cinecavB19i_casual.bwin_font", "cc_fonts/cinecavB24i_casual.bwin_font", "cc_fonts/cinecavB30i_casual.bwin_font" }}, /* BDCC_FontStyle_Casual  */
  {{ "cc_fonts/cinecavB16i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font", "cc_fonts/cinecavB25i_script.bwin_font" }, { "cc_fonts/cinecavB16i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font", "cc_fonts/cinecavB25i_script.bwin_font" }, { "cc_fonts/cinecavB16i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font", "cc_fonts/cinecavB25i_script.bwin_font" }, { "cc_fonts/cinecavB16i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font", "cc_fonts/cinecavB25i_script.bwin_font" }}, /* BDCC_FontStyle_Cursive */
  {{ "cc_fonts/cinecavB18i_sc.bwin_font",     "cc_fonts/cinecavB23i_sc.bwin_font",     "cc_fonts/cinecavB30i_sc.bwin_font"     }, { "cc_fonts/cinecavB18i_sc.bwin_font",     "cc_fonts/cinecavB23i_sc.bwin_font",     "cc_fonts/cinecavB30i_sc.bwin_font"     }, { "cc_fonts/cinecavB18i_sc.bwin_font",     "cc_fonts/cinecavB23i_sc.bwin_font",     "cc_fonts/cinecavB30i_sc.bwin_font"     }, { "cc_fonts/cinecavB18i_sc.bwin_font",     "cc_fonts/cinecavB23i_sc.bwin_font",     "cc_fonts/cinecavB30i_sc.bwin_font"     }}  /* BDCC_FontStyle_SmallCaps */
  };

    /* we need to scale these fonts using the m2mc since we have specified the 720p set for all of the display formats */

    uint32_t display_ratios[4] = {66,66,100,150};
    *scaleFactor = display_ratios[g_DisplayMode] ;  /* scale factor in hundredths i.e. scale factor is 1.0 */

    printf("SCALING 720P FONTS\n");

#else /*MAX_RENDERED_FONTS*/
 char *pszStandardFontFilenames[BDCC_FontStyle_Max_Value][ CCTest_Display_Max_Size ][ BDCC_PenSize_Max_Size ]=
 {
  /* non-italic */
  /* 480i S M L                                                                          480p S M L                                                           720p S M L                                   1080i S M L */

  {{ "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }, { "cc_fonts/cinecavB32_type.bwin_font",   "cc_fonts/cinecavB41_type.bwin_font",   "cc_fonts/cinecavB49_type.bwin_font"   }}, /* BDCC_FontStyle_Default */
  {{ "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB14_type.bwin_font",   "cc_fonts/cinecavB18_type.bwin_font",   "cc_fonts/cinecavB22_type.bwin_font"   }, { "cc_fonts/cinecavB21_type.bwin_font",   "cc_fonts/cinecavB28_type.bwin_font",   "cc_fonts/cinecavB33_type.bwin_font"   }, { "cc_fonts/cinecavB32_type.bwin_font",   "cc_fonts/cinecavB41_type.bwin_font",   "cc_fonts/cinecavB49_type.bwin_font"   }}, /* BDCC_FontStyle_MonoSerifs */
  {{ "cc_fonts/cinecavB12_serif.bwin_font",  "cc_fonts/cinecavB15_serif.bwin_font",  "cc_fonts/cinecavB19_serif.bwin_font"  }, { "cc_fonts/cinecavB12_serif.bwin_font",  "cc_fonts/cinecavB15_serif.bwin_font",  "cc_fonts/cinecavB19_serif.bwin_font"  }, { "cc_fonts/cinecavB18_serif.bwin_font",  "cc_fonts/cinecavB23_serif.bwin_font",  "cc_fonts/cinecavB29_serif.bwin_font"  }, { "cc_fonts/cinecavB28_serif.bwin_font",  "cc_fonts/cinecavB35_serif.bwin_font",  "cc_fonts/cinecavB44_serif.bwin_font"  }}, /* BDCC_FontStyle_PropSerifs */
  {{ "cc_fonts/cinecavB14_mono.bwin_font",   "cc_fonts/cinecavB18_mono.bwin_font",   "cc_fonts/cinecavB22_mono.bwin_font"   }, { "cc_fonts/cinecavB14_mono.bwin_font",   "cc_fonts/cinecavB18_mono.bwin_font",   "cc_fonts/cinecavB22_mono.bwin_font"   }, { "cc_fonts/cinecavB21_mono.bwin_font",   "cc_fonts/cinecavB29_mono.bwin_font",   "cc_fonts/cinecavB34_mono.bwin_font"   }, { "cc_fonts/cinecavB34_mono.bwin_font",   "cc_fonts/cinecavB43_mono.bwin_font",   "cc_fonts/cinecavB52_mono.bwin_font"   }}, /* BDCC_FontStyle_Mono  */
  {{ "cc_fonts/cinecavB12_sans.bwin_font",   "cc_fonts/cinecavB15_sans.bwin_font",   "cc_fonts/cinecavB19_sans.bwin_font"   }, { "cc_fonts/cinecavB12_sans.bwin_font",   "cc_fonts/cinecavB15_sans.bwin_font",   "cc_fonts/cinecavB19_sans.bwin_font"   }, { "cc_fonts/cinecavB18_sans.bwin_font",   "cc_fonts/cinecavB23_sans.bwin_font",   "cc_fonts/cinecavB29_sans.bwin_font"   }, { "cc_fonts/cinecavB28_sans.bwin_font",   "cc_fonts/cinecavB35_sans.bwin_font",   "cc_fonts/cinecavB44_sans.bwin_font"   }}, /* BDCC_FontStyle_Prop  */
  {{ "cc_fonts/cinecavB13_casual.bwin_font", "cc_fonts/cinecavB16_casual.bwin_font", "cc_fonts/cinecavB20_casual.bwin_font" }, { "cc_fonts/cinecavB13_casual.bwin_font", "cc_fonts/cinecavB16_casual.bwin_font", "cc_fonts/cinecavB20_casual.bwin_font" }, { "cc_fonts/cinecavB19_casual.bwin_font", "cc_fonts/cinecavB24_casual.bwin_font", "cc_fonts/cinecavB30_casual.bwin_font" }, { "cc_fonts/cinecavB30_casual.bwin_font", "cc_fonts/cinecavB36_casual.bwin_font", "cc_fonts/cinecavB45_casual.bwin_font" }}, /* BDCC_FontStyle_Casual  */
  {{ "cc_fonts/cinecavB12_script.bwin_font", "cc_fonts/cinecavB14_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font" }, { "cc_fonts/cinecavB12_script.bwin_font", "cc_fonts/cinecavB14_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font" }, { "cc_fonts/cinecavB16_script.bwin_font", "cc_fonts/cinecavB18_script.bwin_font", "cc_fonts/cinecavB25_script.bwin_font" }, { "cc_fonts/cinecavB25_script.bwin_font", "cc_fonts/cinecavB28_script.bwin_font", "cc_fonts/cinecavB38_script.bwin_font" }}, /* BDCC_FontStyle_Cursive */
  {{ "cc_fonts/cinecavB13_sc.bwin_font",     "cc_fonts/cinecavB15_sc.bwin_font",     "cc_fonts/cinecavB19_sc.bwin_font"     }, { "cc_fonts/cinecavB13_sc.bwin_font",     "cc_fonts/cinecavB15_sc.bwin_font",     "cc_fonts/cinecavB19_sc.bwin_font"     }, { "cc_fonts/cinecavB18_sc.bwin_font",     "cc_fonts/cinecavB23_sc.bwin_font",     "cc_fonts/cinecavB30_sc.bwin_font"     }, { "cc_fonts/cinecavB28_sc.bwin_font",     "cc_fonts/cinecavB35_sc.bwin_font",     "cc_fonts/cinecavB45_sc.bwin_font"     }}  /* BDCC_FontStyle_SmallCaps */
  };


 char *pszItalicFontFilenames[BDCC_FontStyle_Max_Value][ CCTest_Display_Max_Size ][ BDCC_PenSize_Max_Size ]=
 {
  /* italic */

  {{ "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }, { "cc_fonts/cinecavB32i_type.bwin_font",   "cc_fonts/cinecavB41i_type.bwin_font",   "cc_fonts/cinecavB49i_type.bwin_font"   }}, /* BDCC_FontStyle_Default */
  {{ "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB14i_type.bwin_font",   "cc_fonts/cinecavB18i_type.bwin_font",   "cc_fonts/cinecavB22i_type.bwin_font"   }, { "cc_fonts/cinecavB21i_type.bwin_font",   "cc_fonts/cinecavB28i_type.bwin_font",   "cc_fonts/cinecavB33i_type.bwin_font"   }, { "cc_fonts/cinecavB32i_type.bwin_font",   "cc_fonts/cinecavB41i_type.bwin_font",   "cc_fonts/cinecavB49i_type.bwin_font"   }}, /* BDCC_FontStyle_MonoSerifs */
  {{ "cc_fonts/cinecavB12i_serif.bwin_font",  "cc_fonts/cinecavB15i_serif.bwin_font",  "cc_fonts/cinecavB19i_serif.bwin_font"  }, { "cc_fonts/cinecavB12i_serif.bwin_font",  "cc_fonts/cinecavB15i_serif.bwin_font",  "cc_fonts/cinecavB19i_serif.bwin_font"  }, { "cc_fonts/cinecavB18i_serif.bwin_font",  "cc_fonts/cinecavB23i_serif.bwin_font",  "cc_fonts/cinecavB29i_serif.bwin_font"  }, { "cc_fonts/cinecavB28i_serif.bwin_font",  "cc_fonts/cinecavB35i_serif.bwin_font",  "cc_fonts/cinecavB44i_serif.bwin_font"  }}, /* BDCC_FontStyle_PropSerifs */
  {{ "cc_fonts/cinecavB14i_mono.bwin_font",   "cc_fonts/cinecavB18i_mono.bwin_font",   "cc_fonts/cinecavB22i_mono.bwin_font"   }, { "cc_fonts/cinecavB14i_mono.bwin_font",   "cc_fonts/cinecavB18i_mono.bwin_font",   "cc_fonts/cinecavB22i_mono.bwin_font"   }, { "cc_fonts/cinecavB21i_mono.bwin_font",   "cc_fonts/cinecavB29i_mono.bwin_font",   "cc_fonts/cinecavB34i_mono.bwin_font"   }, { "cc_fonts/cinecavB34i_mono.bwin_font",   "cc_fonts/cinecavB43i_mono.bwin_font",   "cc_fonts/cinecavB52i_mono.bwin_font"   }}, /* BDCC_FontStyle_Mono  */
  {{ "cc_fonts/cinecavB12i_sans.bwin_font",   "cc_fonts/cinecavB15i_sans.bwin_font",   "cc_fonts/cinecavB19i_sans.bwin_font"   }, { "cc_fonts/cinecavB12i_sans.bwin_font",   "cc_fonts/cinecavB15i_sans.bwin_font",   "cc_fonts/cinecavB19i_sans.bwin_font"   }, { "cc_fonts/cinecavB18i_sans.bwin_font",   "cc_fonts/cinecavB23i_sans.bwin_font",   "cc_fonts/cinecavB29i_sans.bwin_font"   }, { "cc_fonts/cinecavB28i_sans.bwin_font",   "cc_fonts/cinecavB35i_sans.bwin_font",   "cc_fonts/cinecavB44i_sans.bwin_font"   }}, /* BDCC_FontStyle_Prop  */
  {{ "cc_fonts/cinecavB13i_casual.bwin_font", "cc_fonts/cinecavB16i_casual.bwin_font", "cc_fonts/cinecavB20i_casual.bwin_font" }, { "cc_fonts/cinecavB13i_casual.bwin_font", "cc_fonts/cinecavB16i_casual.bwin_font", "cc_fonts/cinecavB20i_casual.bwin_font" }, { "cc_fonts/cinecavB19i_casual.bwin_font", "cc_fonts/cinecavB24i_casual.bwin_font", "cc_fonts/cinecavB30i_casual.bwin_font" }, { "cc_fonts/cinecavB30i_casual.bwin_font", "cc_fonts/cinecavB36i_casual.bwin_font", "cc_fonts/cinecavB45i_casual.bwin_font" }}, /* BDCC_FontStyle_Casual  */
  {{ "cc_fonts/cinecavB12i_script.bwin_font", "cc_fonts/cinecavB14i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font" }, { "cc_fonts/cinecavB12i_script.bwin_font", "cc_fonts/cinecavB14i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font" }, { "cc_fonts/cinecavB16i_script.bwin_font", "cc_fonts/cinecavB18i_script.bwin_font", "cc_fonts/cinecavB25i_script.bwin_font" }, { "cc_fonts/cinecavB25i_script.bwin_font", "cc_fonts/cinecavB28i_script.bwin_font", "cc_fonts/cinecavB38i_script.bwin_font" }}, /* BDCC_FontStyle_Cursive */
  {{ "cc_fonts/cinecavB13i_sc.bwin_font",     "cc_fonts/cinecavB15i_sc.bwin_font",     "cc_fonts/cinecavB19i_sc.bwin_font"     }, { "cc_fonts/cinecavB13i_sc.bwin_font",     "cc_fonts/cinecavB15i_sc.bwin_font",     "cc_fonts/cinecavB19i_sc.bwin_font"     }, { "cc_fonts/cinecavB18i_sc.bwin_font",     "cc_fonts/cinecavB23i_sc.bwin_font",     "cc_fonts/cinecavB30i_sc.bwin_font"     }, { "cc_fonts/cinecavB28i_sc.bwin_font",     "cc_fonts/cinecavB35i_sc.bwin_font",     "cc_fonts/cinecavB45i_sc.bwin_font"     }}  /* BDCC_FontStyle_SmallCaps */
  };

    /* we do not need to scale these fonts using the m2mc since we have pre-rendered a complete set of 126!! */
    *scaleFactor = 100 ;  /* scale factor in hundredths i.e. scale factor is 1.0 */

    printf("USING MAX FONTS\n");

#endif

#endif

#endif

    BKNI_Memset( standardFonts, 0, ( sizeof( BDCC_FONT_DESCRIPTOR ) * BDCC_FontStyle_Max_Value ));
    BKNI_Memset( italicsFonts , 0, ( sizeof( BDCC_FONT_DESCRIPTOR ) * BDCC_FontStyle_Max_Value ));

#ifdef FREETYPE_SUPPORT

    for ( i = 0; i < BDCC_FontStyle_Max_Value; i++){
        for ( j = 0; j < BDCC_PenSize_Max_Size; j++){

            standardFonts[ i ][ j ].pszFontFile = pszFontFilenames[ i ];
            standardFonts[ i ][ j ].iFontSize = iFontSize[ i ][ g_DisplayMode ][ j ];

            italicsFonts[ i ][ j ].pszFontFile = pszFontFilenames[ i ];
            italicsFonts[ i ][ j ].iFontSize = iFontSize[ i ][ g_DisplayMode ][ j ];
        }
    }

#else

    for ( i = 0; i < BDCC_FontStyle_Max_Value; i++){
        for ( j = 0; j < BDCC_PenSize_Max_Size; j++){

            standardFonts[ i ][ j ].pszFontFile = pszStandardFontFilenames[ i ][ g_DisplayMode ][ j ];
            standardFonts[ i ][ j ].iFontSize = -1;

            italicsFonts[ i ][ j ].pszFontFile = pszItalicFontFilenames[ i ][ g_DisplayMode ][ j ];
            italicsFonts[ i ][ j ].iFontSize = -1;
        }
    }

#endif

    printf("\nCCTest_P_LoadFonts():\n" );

    /*
    ** First load the fonts.
    */
    for ( i=0; i < BDCC_FontStyle_Max_Value; i++ )
    {
        for(j=0; j < BDCC_PenSize_Max_Size; j++)
        {
            pFont = &standardFonts[ i ][ j ];

            if(pFont->pszFontFile) /* need not specify all fonts, minimum is the DEFAULT font */
            {
                iErr = BDCC_WINLIB_LoadFont(
                    g_WindowLib,
                    pFont->pszFontFile,
                    pFont->iFontSize,
                    i,
                    j,
                    BDCC_PenStyle_Standard
                    );
                printf("%s %s\n", iErr ? "\tFAILED to load font!!!:" : "\tloaded font:", pFont->pszFontFile );
            }

            pFont = &italicsFonts[ i ][ j ];

            if(pFont->pszFontFile) /* need not specify all fonts, minimum is the DEFAULT font */
            {
                iErr = BDCC_WINLIB_LoadFont(
                    g_WindowLib,
                    pFont->pszFontFile,
                    pFont->iFontSize,
                    i,
                    j,
                    BDCC_PenStyle_Italics
                    );

                printf("%s %s\n", iErr ? "\tFAILED to load font!!!:" : "\tloaded font:", pFont->pszFontFile );
            }

        }
    }

    printf("\n");

}   /* end of CCTest_P_LoadFonts() */




void user_Input(const char *name)
{
    printf("%s: eia708 application\n", name);
    printf("Usage: %s <args>\nWhere args are: -file <filename> <video pid> <audio pid> -vsb <freq in Mhz> <video pid> <audio pid> -qam <freq in MHz> <video pid> <audio pid>-480i -480p -720p -1080i -test -testall\n", name);
    return;
}

void printHelpInfo( void )
{

    char szHelpText1[] = {
        "\nAt startup, the following parameters can be specified:\n"
        "\t-480i : display resolution of 480i\n"
        "\t-480p : display resolution of 480p\n"
        "\t-720p : display resolution of 720p\n"
        "\t1-080i : display resolution of 1080i\n"
        "\t-file <filename> <video pid> <audio pid>: selects playback from a file, with specified pids"
        "\t-vsb <freq> <video pid> <audio pid> : Tune to ATSC channel with specified frequency and A/V pids \n"
        "\t-qam <freq> <video pid> <audio pid> : Tune to Cable channel with specified frequency and A/V pids  \n"
        };

    char szHelpText2[] = {
        "\t-test n selects test n from 708_test_string.h\n"
        "\t-testall runs all tests from 708_test_string.h\n"
        };


    printf("%s", szHelpText1 );
    printf("%s", szHelpText2 );
    return;

}

void previewCaption(bool enable, B_Dcc_Type cc_mode, uint32_t cc_service, int penfg, int penbg, int wincolor)
{
static uint32_t old_service;

#define sh(v,s)    ((v) << (s))
#define DefineWindow(id,p,ap,rp,av,ah,rc,cc,rl,cl,v,ws,ps)	\
		(0x98 | sh(id,0)), \
		(sh(v,5) | sh(rl,4) | sh(cl,3) | sh(p,0)), \
		(sh(rp,7) | sh(av,0)), \
		(sh(ah,0)), \
		(sh(ap,4) | sh((rc)-1,0)), \
		(sh((cc)-1,0)), \
		(sh(ws,3) | sh(ps,0))
#define SetPenColor(fo,fr,fg,fb,bo,br,bg,bb,er,eg,eb) \
		0x91, \
		(sh(fo,6) | sh(fr,4) | sh(fg,2) | sh(fb,0)), \
		(sh(bo,6) | sh(br,4) | sh(bg,2) | sh(bb,0)), \
		(			sh(er,4) | sh(eg,2) | sh(eb,0))

#define DeleteWindows(wmask) \
		0x8C, (wmask)

#define SetWindowAttr(j,pd,sd,ww,de,ed,es,fr,fg,fb,fo,bt,br,bg,bb) \
		0x97, \
		(sh(fo,6) | sh(fr,4) | sh(fg,2) | sh(fb,0)), \
		(sh(bt,6) | sh(br,4) | sh(bg,2) | sh(bb,0)), \
		(sh(ww,6) | sh(pd,4) | sh(sd,2) | sh(j,0)), \
		(sh(es,4) | sh(ed,2) | sh(de,0))
#define SetPenAttr(psz,fs,i,u,ed) \
		0x90, \
		psz, \
		(sh(i,7) | sh(u,6) | sh(ed,3) | sh(fs,0))
#define CurrentWindow(wnd)		(0x80 | wnd)
#define SetPenLocation(r,c)		\
		0x92, (r), (c)

	int penfgo, penfgr, penfgg, penfgb, penbgo, penbgr, penbgg, penbgb, winco, wincr, wincg, wincb;
	penfgo = (penfg == 256) ? 0 : (penfg & 0xC0) >> 6;
	penfgr = (penfg == 256) ? 3 : (penfg & 0x30) >> 4;
	penfgg = (penfg == 256) ? 3 : (penfg & 0x0C) >> 2;
	penfgb = (penfg == 256) ? 3 : (penfg & 0x03);
	penbgo = (penbg == 256) ? 0 : (penbg & 0xC0) >> 6;
	penbgr = (penbg == 256) ? 0 : (penbg & 0x30) >> 4;
	penbgg = (penbg == 256) ? 0 : (penbg & 0x0C) >> 2;
	penbgb = (penbg == 256) ? 0 : (penbg & 0x03);
	winco  = (wincolor == 256) ? 0 : (wincolor & 0xC0) >> 6;
	wincr  = (wincolor == 256) ? 3 : (wincolor & 0x30) >> 4;
	wincg  = (wincolor == 256) ? 3 : (wincolor & 0x0C) >> 2;
	wincb  = (wincolor == 256) ? 3 : (wincolor & 0x03);

	unsigned char testStream1[] =
	{
	DeleteWindows(0xFF),
	DefineWindow(2,0,2,0,0,159,4,8,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(0,0,3,0,0,0,0,wincr,wincg,wincb,winco,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(penfgo,penfgr,penfgg,penfgb,penbgo,penbgr,penbgg,penbgb,0,0,0),
	SetPenLocation(0,0),
	'P', 'r', 'e', 'v', 'i', 'e', 'w', 0x03
	};

	unsigned char testStream2[] =
	{
		DeleteWindows(0xFF)
	};

    if (enable)
    {
		old_service = cc_service;
		if (cc_mode == B_Dcc_Type_e608)
			cc_service = BDCC_Max_608_Service;
		else
			cc_service = BDCC_Max_708_Service;

		B_Dcc_Reset(g_ccEngine, NULL, cc_mode, cc_service);
		B_Dcc_SendTestString(g_ccEngine, testStream1, sizeof(testStream1));
    }
	else
	{
		cc_service = old_service;
		B_Dcc_SendTestString(g_ccEngine, testStream2, sizeof(testStream2));
		B_Dcc_Reset(g_ccEngine, NULL, cc_mode, cc_service);
	}
}
/* end of file */
