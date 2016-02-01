/***************************************************************************
 *     (c)2003-2012 Broadcom Corporation
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
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_DISPLAY && NEXUS_HAS_PICTURE_DECODER
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(rasterOps);

#include "nexus_picture_decoder.h"

#define LIGHT_GRAY   0xFF404040
#define HOT_PINK     0x80ff69b4
#define BLACK        0xFF000000
#define GREEN        0xFF00ff00
#define YELLOW       0x80ffff00
#define DIRTY_YELLOW 0xFF404010
#define FULL_ALPHA   0xFF000000

#ifdef DEFAULT_IS_PAL
    #define DEFAULT_HD_VIDEOFORMAT NEXUS_VideoFormat_e1080i50hz
#else
    #define DEFAULT_HD_VIDEOFORMAT NEXUS_VideoFormat_e720p /*NEXUS_VideoFormat_e1080i*/
#endif

/*
   Keep height and width a multiple of 8 as non multiples
   will cause pattern to be non regular
*/
#define BLOCK_WIDTH  128
#define BLOCK_HEIGHT 128

static void checkpoint(NEXUS_Graphics2DHandle gfx)
{
    NEXUS_Graphics2DSettings gfxSettings;
    int rc;

    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.pollingCheckpoint = true;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);
    do {
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_BUSY) {
            BKNI_Sleep(1);
            continue;
        }
        BDBG_ASSERT(!rc);
        break;
    } while (1);
}

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int decodeImage( const char *filename, NEXUS_SurfaceHandle *picSurface, NEXUS_PictureDecoderStatus *jpegStatus,
                NEXUS_SurfaceCreateSettings *imgSurfaceSettings, NEXUS_PictureDecoderHandle  *pictureDecoder ) {

    NEXUS_PictureDecoderStartSettings pictureSettings;
    NEXUS_PictureDecoderOpenSettings  decoderSettings;
    int rc;
    FILE *fin ;
    void *buffer;
    size_t size;

    fin = fopen(filename,"rb");
    if(!fin) {
        perror(filename);
        return 1;
    }

    NEXUS_PictureDecoder_GetDefaultOpenSettings(&decoderSettings);
    decoderSettings.multiScanBufferSize = 16*1024*1024;
    decoderSettings.bufferSize = 9*1024*1024;
    *pictureDecoder = NEXUS_PictureDecoder_Open(0, &decoderSettings);

    NEXUS_PictureDecoder_GetBuffer(*pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
    rc = fread(buffer, 1, size, fin); /* read file into the decoder's buffer */
    if(rc<0) {
        perror(filename);
        return 1;
    } else if(rc==(int)size) {
        fseek(fin, 0, SEEK_END);
        if((unsigned)ftell(fin)>size) {
            /* to decode larger file, requires to allocate larger buffer, see NEXUS_PictureDecoderOpenSettings * bufferSize */
            fprintf(stderr, "JPEG file size %u is larger then buffer size %u, not supported\n", (unsigned)ftell(fin), (unsigned)size);
            return -1;
        }
    }
    NEXUS_PictureDecoder_ReadComplete(*pictureDecoder, 0, rc); /* commit data to the decoder */

    NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
    if (strcasestr(filename, ".png")) {
        pictureSettings.format = NEXUS_PictureFormat_ePng;
    }
    else {
        pictureSettings.format = NEXUS_PictureFormat_eJpeg;
    }
    NEXUS_PictureDecoder_Start(*pictureDecoder, &pictureSettings);

    do {
        NEXUS_PictureDecoder_GetStatus(*pictureDecoder, jpegStatus);
        if( jpegStatus->state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed\n");
            return 1;
        }
        usleep(1000);
    } while(!jpegStatus->headerValid); /* wait for picture dimensions */

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(imgSurfaceSettings);
    imgSurfaceSettings->pixelFormat = jpegStatus->header.format;
    imgSurfaceSettings->width       = jpegStatus->header.surface.width;
    imgSurfaceSettings->height      = jpegStatus->header.surface.height;
    *picSurface = NEXUS_Surface_Create(imgSurfaceSettings);

    /* start decoding */
    NEXUS_PictureDecoder_DecodeSegment(*pictureDecoder, *picSurface, NULL);
    do {
        NEXUS_PictureDecoder_GetStatus(*pictureDecoder, jpegStatus);
        if( jpegStatus->state == NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed\n");
            return 1;
        }
        usleep(1000);
    } while( jpegStatus->state!=NEXUS_PictureDecoderState_eSegmentDone); /* wait for picture to decode */

    NEXUS_PictureDecoder_Stop(*pictureDecoder);

    return 0;
}

uint8_t ropPattern[8] = { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 }; /* cross pattern */
#if 0
uint8_t ropPattern[8] = { 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC }; /* stripe pattern */
uint8_t ropPattern[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; /* solid  pattern */
#endif

typedef struct {
    uint8_t ropVec;
    uint8_t ropName[13];
} ropVecName;

ropVecName ropVecTable[] = { 
                             {0x00 , "BLACKNESS"  },
                             {0x11 , "NOTSRCERASE"},
                             {0x33 , "NOTSRCCOPY" },
                             {0x44 , "SRCERASE"   },
                             {0x55 , "DSTINVERT"  },
                             {0x5A , "PATINVERT"  },
                             {0x66 , "SRCINVERT"  },
                             {0X88 , "SRCAND"     },
                             {0xBB , "MERGEPAINT" },
                             {0xC0 , "MERGECOPY"  },
                             {0xCC , "SRCCOPY"    },
                             {0xEE , "SRCPAINT"   },
                             {0xF0 , "PATCOPY"    },
                             {0xFB , "PATPAINT"   },
                             {0xFF , "WHITENESS"  }
                            };
        
int main(int argc, char **argv)
{
    NEXUS_SurfaceHandle framebuffer;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoFormatInfo info;
    NEXUS_Error rc;
    int i;  
    NEXUS_SurfaceHandle srcSurface,destSurface; 
    NEXUS_VideoFormatInfo formatInfo;
    NEXUS_DisplaySettings dispSettings;

    NEXUS_PictureDecoderHandle   pictureDecoder;
    NEXUS_SurfaceHandle          imgSurface,scaledJpegSurface;
    NEXUS_PictureDecoderStatus   pictureStatus;
    NEXUS_SurfaceCreateSettings  imgSurfaceSettings;

    const char *filename = "akebono.png"; /* Sunset.jpg; change this to a jpeg of your choice */

    if (argc > 1) {
        filename = argv[1];
    }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* HD display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = DEFAULT_HD_VIDEOFORMAT;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    /* Set Video plane background to given color , This color will be visible when gfx layer is transparent */
    NEXUS_Display_GetSettings( display, &dispSettings );
    dispSettings.background = DIRTY_YELLOW;
    NEXUS_Display_SetSettings( display, &dispSettings);        
    /* getchar(); - uncomment if you want to see Video plane background before proceeding */

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &info);
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width  = info.width;
    createSettings.height = info.height;
    createSettings.heap   = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSettings);

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context  = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &formatInfo);

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width  = formatInfo.width;
    createSettings.height = formatInfo.height;
    srcSurface  = NEXUS_Surface_Create(&createSettings);
    destSurface = NEXUS_Surface_Create(&createSettings);

    /* Create a surface with BLOCK_ height & width to blit ( scale up/down )  image into */
    createSettings.width  = BLOCK_WIDTH;
    createSettings.height = BLOCK_HEIGHT;
    scaledJpegSurface  = NEXUS_Surface_Create(&createSettings);

    /* enable display output */
    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);

    /* Fill entire screen with light gray */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebuffer;
    fillSettings.color   = LIGHT_GRAY;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    checkpoint(gfx); /* wait for blit to finish and display */

#ifdef SINGLE_STEP
    printf("blitted gray background, press enter to continue\n");
    getchar();
#endif

    /* Fill a destination surface with color */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = destSurface;
    fillSettings.color   = HOT_PINK;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    checkpoint(gfx); /* wait for blit to finish and display */

    rc = decodeImage(filename, &imgSurface, &pictureStatus, &imgSurfaceSettings, &pictureDecoder );
    if (rc) {
        return -1;
    }
    BDBG_WRN(("Decoded jpeg %s" , filename ));

    /* Create a scaled version of picture file that fits in "BLOCK" dimensions
       because destination has no scaling capability 
    */
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface     = imgSurface;
    blitSettings.output.surface     = scaledJpegSurface;
    blitSettings.output.rect.width  = BLOCK_WIDTH;
    blitSettings.output.rect.height = BLOCK_HEIGHT;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    BDBG_ASSERT(!rc);
    checkpoint(gfx); /* wait for blit to finish and display */

    for ( i=0 ; i < 2 ; i++ ) {
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface     = imgSurface;
        blitSettings.output.surface     = framebuffer;
        blitSettings.output.rect.x      = 160;
        blitSettings.output.rect.y      = 200 + i*200;
        blitSettings.output.rect.width  = BLOCK_WIDTH;
        blitSettings.output.rect.height = BLOCK_HEIGHT;
        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        BDBG_ASSERT(!rc);
        checkpoint(gfx); /* wait for blit to finish and display */
#ifdef SINGLE_STEP
        printf("Blitted %s at x=%d,y=%d\n" , filename, blitSettings.output.rect.x, blitSettings.output.rect.y );
        getchar();
#endif

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.patternSettingsEnabled = true;
        blitSettings.source.surface     = imgSurface;
        blitSettings.output.surface     = framebuffer;
        blitSettings.output.rect.x      = 2*160;
        blitSettings.output.rect.y      = 200 + i*200;
        blitSettings.output.rect.width  = BLOCK_WIDTH;
        blitSettings.output.rect.height = BLOCK_HEIGHT;

        /* OR pattern with negative of source to give a "color negative" appearance */
        BKNI_Memcpy( blitSettings.patternSettings.pattern , ropPattern , sizeof ( ropPattern ) );
        blitSettings.patternSettings.ropVector = ropVecTable[13].ropVec; 
        /* Making pattern fore and background color same, makes for a solid color */
        blitSettings.patternSettings.backColor = BLACK;
        blitSettings.patternSettings.foreColor = BLACK;
        /* for second row, allow pattern to be seen, diagonal lines in green */
        if ( i ) blitSettings.patternSettings.foreColor = GREEN;

        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        fprintf( stderr, "1 ROP Vec=0x%04x Name=%s \n" , ropVecTable[13].ropVec , ropVecTable[13].ropName );
        checkpoint(gfx); /* wait for blit to finish and display */
#ifdef SINGLE_STEP
        fprintf( stderr, "press enter to continue" );
        getchar();
#endif

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.patternSettingsEnabled = true;
        blitSettings.source.surface     = imgSurface;
        blitSettings.output.surface     = framebuffer;
        blitSettings.output.rect.x      = 3*160;
        blitSettings.output.rect.y      = 200 + i*200;
        blitSettings.output.rect.width  = BLOCK_WIDTH;
        blitSettings.output.rect.height = BLOCK_HEIGHT;

        /* OR pattern in hot pink with negative of source to give a "hot pink color negative" appearance */
        BKNI_Memcpy( blitSettings.patternSettings.pattern , ropPattern , sizeof ( ropPattern ) );
        blitSettings.patternSettings.ropVector = ropVecTable[13].ropVec;  
        blitSettings.patternSettings.backColor = HOT_PINK;
        blitSettings.patternSettings.foreColor = HOT_PINK;
        /* for second row, allow pattern to be seen, diagonal lines in yellow */
        if ( i ) blitSettings.patternSettings.foreColor = GREEN;

        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        fprintf( stderr, "2 ROP Vec=0x%04x Name=%s \n" , ropVecTable[13].ropVec, ropVecTable[13].ropName );
        checkpoint(gfx); /* wait for blit to finish and display */
#ifdef SINGLE_STEP
        fprintf( stderr, "press enter to continue" );
        getchar();
#endif

        /* pattern Invert */
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.patternSettingsEnabled = true;
        blitSettings.source.surface     = destSurface;
        blitSettings.dest.surface       = scaledJpegSurface;
        blitSettings.output.surface     = framebuffer;
        blitSettings.dest.rect.x        = 0 ; 
        blitSettings.dest.rect.y        = 0 ;
        blitSettings.dest.rect.width    = BLOCK_WIDTH;
        blitSettings.dest.rect.height   = BLOCK_HEIGHT;
        blitSettings.output.rect.x      = 4*160 ; 
        blitSettings.output.rect.y      = 200 + i*200;
        blitSettings.output.rect.width  = BLOCK_WIDTH;
        blitSettings.output.rect.height = BLOCK_HEIGHT;

        BKNI_Memcpy( blitSettings.patternSettings.pattern , ropPattern , sizeof ( ropPattern ) );
        blitSettings.patternSettings.ropVector = ropVecTable[5].ropVec;  
        blitSettings.patternSettings.backColor = BLACK & 0x00FFFFFF; /* remove alpha */
        blitSettings.patternSettings.foreColor = BLACK & 0x00FFFFFF;
        if ( i ) blitSettings.patternSettings.foreColor = GREEN;

        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        fprintf( stderr, "3 ROP Vec=0x%04x Name=%s \n" , ropVecTable[5].ropVec, ropVecTable[5].ropName );
        checkpoint(gfx); /* wait for blit to finish and display */
#ifdef SINGLE_STEP
        fprintf( stderr, "press enter to continue" );
        getchar();
#endif

        /* Merge copy */
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.patternSettingsEnabled = true;
        blitSettings.source.surface     = imgSurface;
        blitSettings.dest.surface       = NULL;
        blitSettings.output.surface     = framebuffer;
        blitSettings.output.rect.x      = 5*160; 
        blitSettings.output.rect.y      = 200 + i*200;
        blitSettings.output.rect.width  = BLOCK_WIDTH;
        blitSettings.output.rect.height = BLOCK_HEIGHT;
        BKNI_Memcpy( blitSettings.patternSettings.pattern , ropPattern , sizeof ( ropPattern ) );
        blitSettings.patternSettings.ropVector = ropVecTable[9].ropVec;  
        blitSettings.patternSettings.backColor = HOT_PINK | FULL_ALPHA; /* set alpha to max */
        blitSettings.patternSettings.foreColor = HOT_PINK | FULL_ALPHA;
        if ( i ) blitSettings.patternSettings.foreColor = GREEN;

        rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
        fprintf( stderr, "4 ROP Vec=0x%04x Name=%s \n" , ropVecTable[9].ropVec, ropVecTable[9].ropName );
        checkpoint(gfx); /* wait for blit to finish and display */
        printf("\n");
    }
    printf("Press any key to exit\n");
    getchar();

    NEXUS_Surface_Destroy(imgSurface);
    NEXUS_PictureDecoder_Close(pictureDecoder);        
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(srcSurface);
    NEXUS_Surface_Destroy(destSurface);
    NEXUS_Surface_Destroy(framebuffer);
    BKNI_DestroyEvent(checkpointEvent);
    NEXUS_Platform_Uninit();
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
