/***************************************************************************
 *     (c)2002-2013 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_VIDEO_DECODER
#include <stdio.h>
#include <stdlib.h>
#include "708_test_string.h"
#include "b_dcc_lib.h"
#include "nxclient.h"
#include "media_player.h"
#include "nexus_surface_client.h"
#include "parse_userdata.h"

enum CCTest_Display_Modes
{
    CCTest_Display_480i,
    CCTest_Display_480p,
    CCTest_Display_720p,
    CCTest_Display_1080i,
    CCTest_Display_Max_Size
};

uint32_t g_DisplayMode = CCTest_Display_480i;
static const uint32_t gDisplayWidth[4]   = {720, 720, 1280, 1920};
static const uint32_t gDisplayHeight[4]  = {480, 480, 720, 1080};
static const uint32_t gCaptionColumns[4] = {32, 32, 42, 42};

typedef struct CCTest_Caption_Triplets
{
    uint8_t ccType;
    uint8_t data[2];
} CCTest_Caption_Triplets;

static void loadFonts( BDCC_WINLIB_Handle winlib, uint32_t *scaleFactor );

static void print_usage(void)
{
    printf(
    "ccgfx OPTIONS FILENAME\n"
    "\n"
    "OPTIONS:\n"
    "  --help or -h for help\n"
    "  -608    parse EIA608 data. default is EIA708\n"
    );
    return;
}

/* state for callbacks */
struct context {
    NEXUS_SurfaceHandle framebuffer1;
    NEXUS_SurfaceClientHandle surfaceClient;
    BKNI_EventHandle displayedEvent;
};

static void set_event(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static NEXUS_SurfaceHandle client_flip(void *context)
{
    struct context *pContext = context;
    NEXUS_SurfaceClient_SetSurface(pContext->surfaceClient, pContext->framebuffer1);
    BKNI_WaitForEvent(pContext->displayedEvent, 1000);
    return pContext->framebuffer1; /* same framebuffer. SurfaceCompositor will copy. */
}

static void client_get_framebuffer_dimensions(void *context, NEXUS_VideoFormat *format, unsigned *width, unsigned *height)
{
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    struct context *pContext = context;
    NEXUS_Surface_GetCreateSettings(pContext->framebuffer1, &surfaceCreateSettings);
    *width = surfaceCreateSettings.width;
    *height = surfaceCreateSettings.height;
    *format = NEXUS_VideoFormat_eNtsc;
}

/* This app is a simplification of rockford/unittests/applibs/dcc/eia708_app.c. See that
code for more dcc lib test options. */
int main( int argc, char *argv[] )
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SurfaceMemory surfmem1;
    media_player_t media_player;
    media_player_create_settings create_settings;
    media_player_start_settings start_settings;
    BDCC_WINLIB_Interface winlibInterface;
    B_Dcc_Handle ccEngine;
    B_Dcc_Settings ccEngineSettings;
    uint32_t scaleFactor;
    BDCC_WINLIB_OpenSettings openSettings;
    BDCC_WINLIB_Handle winlib;
    int rc;
    NEXUS_SurfaceClientSettings surfaceClientSettings;
    B_Dcc_Type ccMode = B_Dcc_Type_e708;
    int curarg = 1;
    struct context context, *pContext = &context;
    buserdata_t userdata;

    media_player_get_default_start_settings(&start_settings);
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-608")) {
            ccMode = B_Dcc_Type_e608;
        }
        else if (!start_settings.stream_url) {
            start_settings.stream_url = argv[curarg];
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }
    if (!start_settings.stream_url) {
        print_usage();
        return -1;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    BKNI_CreateEvent(&pContext->displayedEvent);

    pContext->surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (!pContext->surfaceClient) {
        return -1;
    }
    NEXUS_SurfaceClient_AcquireVideoWindow(pContext->surfaceClient, 0);

    NEXUS_SurfaceClient_GetSettings(pContext->surfaceClient, &surfaceClientSettings);
    surfaceClientSettings.displayed.callback = set_event;
    surfaceClientSettings.displayed.context = pContext->displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(pContext->surfaceClient, &surfaceClientSettings);
    BDBG_ASSERT(!rc);

    /* create the nexus surfaces to be used as frame buffers */
    NEXUS_Surface_GetDefaultCreateSettings( &surfaceCreateSettings );
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    surfaceCreateSettings.width = 720;
    surfaceCreateSettings.height = 480;
    pContext->framebuffer1 =  NEXUS_Surface_Create( &surfaceCreateSettings );
    NEXUS_Surface_GetMemory(pContext->framebuffer1, &surfmem1);
    BKNI_Memset(surfmem1.buffer, 0x00, surfaceCreateSettings.height * surfmem1.pitch);
    NEXUS_Surface_Flush(pContext->framebuffer1);

    BDCC_WINLIB_GetDefaultOpenSettings(&openSettings);
    openSettings.flip = client_flip;
    openSettings.get_framebuffer_dimensions = client_get_framebuffer_dimensions;
    openSettings.context = pContext;
    openSettings.framebufferWidth = surfaceCreateSettings.width;
    openSettings.framebufferHeight = surfaceCreateSettings.height;
    rc = BDCC_WINLIB_Open(&winlib, &openSettings);
    BDBG_ASSERT(!rc);

    /* get the winlib function table */
    BDCC_WINLIB_GetInterface( &winlibInterface );
    loadFonts( winlib, &scaleFactor );

    rc = B_Dcc_Open( &ccEngine, winlib, &winlibInterface );
    BDBG_ASSERT(!rc);

    B_Dcc_GetDefaultSettings( &ccEngineSettings );
    ccEngineSettings.iSafeTitleX = gDisplayWidth[g_DisplayMode] / 10; /* 20% of width / 2 according to CEB-10 */;
    ccEngineSettings.iSafeTitleY = gDisplayHeight[g_DisplayMode] / 10; /* 20% of height / 2 according to CEB-10 */;
    ccEngineSettings.ScaleFactor = scaleFactor;
    ccEngineSettings.Columns     = gCaptionColumns[g_DisplayMode]; /* 32 for 4:3 42 for 16:9 */;
    B_Dcc_SetSettings( ccEngine, &ccEngineSettings );

    rc = B_Dcc_Init( ccEngine, 1, ccMode);
    BDBG_ASSERT(!rc);

    media_player_get_default_create_settings(&create_settings);
    create_settings.window.surfaceClientId = allocResults.surfaceClient[0].id;
    media_player = media_player_create(&create_settings);
    BDBG_ASSERT(media_player);

    rc = media_player_start(media_player, &start_settings);
    BDBG_ASSERT(!rc);

    userdata = buserdata_create(NULL);

    while (1)
    {
#define CC_DATA_BUF_SIZE 128
        NEXUS_ClosedCaptionData captionData[ CC_DATA_BUF_SIZE ];
        CCTest_Caption_Triplets cc708UserData[ CC_DATA_BUF_SIZE ];
        uint32_t numEntries, numValidEntries, i;

        rc = buserdata_parse(userdata, media_player_get_video_decoder(media_player), captionData, CC_DATA_BUF_SIZE, &numEntries);
        if (rc || !numEntries) {
            if (rc) BERR_TRACE(rc);
            BKNI_Sleep(30);
            continue;
        }

        for ( i=0, numValidEntries = 0; i < numEntries; i++ )
        {
            /* bit 1 of 'field' should be set for 708 data, clear for 608 data */
            if(((( captionData[ i ].field & 0x2) == 2 ) ^ ( B_Dcc_Type_e608 == ccMode )) && !captionData[i].noData)
            {
                cc708UserData[ numValidEntries ].ccType  = (uint8_t)(captionData[ i ].field);
                cc708UserData[ numValidEntries ].data[0] = captionData[ i ].data[0];
                cc708UserData[ numValidEntries ].data[1] = captionData[ i ].data[1];
                numValidEntries++;
            }
        }

        if( B_Dcc_Process(ccEngine, (unsigned char *)cc708UserData, numValidEntries)) {
            printf("B_Dcc_Process returned with an Error\n");
            break;
        }
    }

    buserdata_destroy(userdata);
    media_player_destroy(media_player);
    B_Dcc_Close( ccEngine );
    BDCC_WINLIB_UnloadFonts( winlib );
    BDCC_WINLIB_Close( winlib );
    NEXUS_Surface_Destroy( pContext->framebuffer1 );

    return rc;
}

/*
** Illustrates how to load fonts and associate them with a specific pen "font style".
*/
static void loadFonts( BDCC_WINLIB_Handle winlib, uint32_t *scaleFactor )
{
    int i, j, iErr;
    BDCC_FONT_DESCRIPTOR *pFont;

    BDCC_FONT_DESCRIPTOR standardFonts[ BDCC_FontStyle_Max_Value ][BDCC_PenSize_Max_Size];
    BDCC_FONT_DESCRIPTOR italicsFonts [ BDCC_FontStyle_Max_Value ][BDCC_PenSize_Max_Size];


/* The following is a trade-off for pre-rendered fonts.
**
** We can use a base set of 14 pre-rendered fonts (7 normal and 7 italic)
** and generate the rest using the scalers but the scaler will reduce the font's quality
** or we can use a full set of 126 fonts to cover all of the font permutations with maximum quality
*/
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
    *scaleFactor = 100;  /* scale factor in hundredths i.e. scale factor is 1.0 */

    BKNI_Memset( standardFonts, 0, ( sizeof( BDCC_FONT_DESCRIPTOR ) * BDCC_FontStyle_Max_Value ));
    BKNI_Memset( italicsFonts , 0, ( sizeof( BDCC_FONT_DESCRIPTOR ) * BDCC_FontStyle_Max_Value ));

    for ( i = 0; i < BDCC_FontStyle_Max_Value; i++){
        for ( j = 0; j < BDCC_PenSize_Max_Size; j++){

            standardFonts[ i ][ j ].pszFontFile = pszStandardFontFilenames[ i ][ g_DisplayMode ][ j ];
            standardFonts[ i ][ j ].iFontSize = -1;

            italicsFonts[ i ][ j ].pszFontFile = pszItalicFontFilenames[ i ][ g_DisplayMode ][ j ];
            italicsFonts[ i ][ j ].iFontSize = -1;
        }
    }

    for ( i=0; i < BDCC_FontStyle_Max_Value; i++ )
    {
        for(j=0; j < BDCC_PenSize_Max_Size; j++)
        {
            pFont = &standardFonts[ i ][ j ];

            if (pFont->pszFontFile) /* need not specify all fonts, minimum is the DEFAULT font */
            {
                iErr = BDCC_WINLIB_LoadFont( winlib, pFont->pszFontFile, pFont->iFontSize, i, j, BDCC_PenStyle_Standard );
                printf("%s %s\n", iErr ? "\tFAILED to load font!!!:" : "\tloaded font:", pFont->pszFontFile );
            }

            pFont = &italicsFonts[ i ][ j ];

            if(pFont->pszFontFile) /* need not specify all fonts, minimum is the DEFAULT font */
            {
                iErr = BDCC_WINLIB_LoadFont( winlib, pFont->pszFontFile, pFont->iFontSize, i, j, BDCC_PenStyle_Italics );
                printf("%s %s\n", iErr ? "\tFAILED to load font!!!:" : "\tloaded font:", pFont->pszFontFile );
            }
        }
    }
    printf("\n");
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs playback, simple_decoder and video_decoder)!\n");
    return 0;
}
#endif
