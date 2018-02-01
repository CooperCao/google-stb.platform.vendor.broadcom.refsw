/******************************************************************************
 * Copyright (C) 2016-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
/* Nexus example app: show jpeg image */
#if NEXUS_HAS_PICTURE_DECODER && NEXUS_HAS_DISPLAY
#include "nexus_platform.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "nexus_picture_decoder.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

BDBG_MODULE(picture_decoder);

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, const char *argv[])
{
    NEXUS_SurfaceHandle framebuffer;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PictureDecoderStartSettings pictureSettings;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_PictureDecoderOpenSettings decoderSettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error errCode;
#endif
    NEXUS_PictureDecoderHandle pictureDecoder = NULL;
    int i;
    void *buffer;
    size_t size,file_size;
    int rc;

    if(argc < 2) {
        argv[1] = "videos/picture.jpg";
        argc = 2;
    }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    errCode = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !errCode && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    /* allocate framebuffer */
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = videoFormatInfo.width;
    createSettings.height = videoFormatInfo.height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer = NEXUS_Surface_Create(&createSettings);

    /* use graphics to fit image into the display framebuffer */
    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    NEXUS_PictureDecoder_GetDefaultOpenSettings(&decoderSettings);
    decoderSettings.multiScanBufferSize = 16*1024*1024;
    decoderSettings.bufferSize = 128*1024;
    pictureDecoder = NEXUS_PictureDecoder_Open(0, &decoderSettings);

    for (i=1;i<argc;i++) {
        FILE *fin;
        const char *pictureFilename = argv[i];
        bool started = false;
        
        NEXUS_SurfaceHandle picture = NULL;
        unsigned cnt,j;
        size_t bytesRemain =0;

        BDBG_WRN(("decoding %s...", pictureFilename));
        fin = fopen(pictureFilename,"rb");
        if(!fin) {
            perror(pictureFilename);
            continue;
        }

        /* Determine file size of image to display */
        fseek(fin, 0, SEEK_END);
        file_size = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
        if (strcasestr(pictureFilename, ".mpg") || strcasestr(pictureFilename, ".es")) {
            pictureSettings.format = NEXUS_PictureFormat_eMpeg;
        }
        else if (strcasestr(pictureFilename, ".png")) {
            pictureSettings.format = NEXUS_PictureFormat_ePng;
        }
        else if (strcasestr(pictureFilename, ".gif")) {
            pictureSettings.format = NEXUS_PictureFormat_eGif;
        }
        else if (strcasestr(pictureFilename, ".bmp")) {
            BDBG_ERR(("BMP is not supported"));
            goto error;
        }
        else {
            pictureSettings.format = NEXUS_PictureFormat_eJpeg;
        }
        pictureSettings.imageSize = file_size;

        rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
        BDBG_ASSERT(!rc);
        rc = fread(buffer, 1, size, fin);                               /* read file into the decoder's buffer */
        if (rc<0) {
            perror(pictureFilename);
            goto error;
        }
        if(rc==(int)size) {
            if( (unsigned)file_size > size && pictureSettings.format == NEXUS_PictureFormat_eMpeg ) {
                BDBG_ERR(("picture file size %u is larger then buffer size %u, not supported for MPEG still frames", (unsigned)file_size, (unsigned)size));
                goto error;
            }
        }
        rc = NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
        BDBG_ASSERT(!rc);
        bytesRemain = pictureSettings.imageSize - size ;

        rc = NEXUS_PictureDecoder_Start(pictureDecoder, &pictureSettings);
        BDBG_ASSERT(!rc);
        
        started = true;

        cnt = 0;
        do {
            rc = NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
            BDBG_ASSERT(!rc);
            if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
                BDBG_ERR(("decoding failed"));
                goto error;
            } else if ( pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
                rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
                BDBG_ASSERT(!rc);
                if ( size > bytesRemain ) size = bytesRemain;

                if ( rc == NEXUS_SUCCESS ) {
                    rc = fread(buffer, 1, size, fin);                             /* read file into the decoder's buffer */
                    if (rc <= 0) {
                        BDBG_ERR(("unable to read more data rc=%d bytesRemain=%u", rc, (unsigned)bytesRemain));
                        cnt = 999;
                    }
                    rc = NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
                    BDBG_ASSERT(!rc);
                    bytesRemain -= size;
                }
            }
            usleep(1000);
            if (++cnt == 1000) { /* 1 second */
                BDBG_ERR(("unable to read header. aborting."));
                goto error;
            }
        } while(!pictureStatus.headerValid); /* wait for picture dimensions */

        /* create picture that could handle complete picture */
        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = pictureStatus.header.format;
        createSettings.width = pictureStatus.header.surface.width;
        createSettings.height = pictureStatus.header.surface.height;
        createSettings.alignment = 2;
        {
            NEXUS_PixelFormatInfo pixelFormatInfo;
            NEXUS_PixelFormat_GetInfo(createSettings.pixelFormat, &pixelFormatInfo);
            /* can't rely on default pitch 0 because we must round up to 4 byte alignment for SID */
            createSettings.pitch = (createSettings.width*pixelFormatInfo.bpp + 7) / 8;
            createSettings.pitch = ((createSettings.pitch + 3) & ~3);
            BDBG_ASSERT(createSettings.pitch % 4 == 0);
        }
        picture = NEXUS_Surface_Create(&createSettings);
        BDBG_WRN(("creating surface: format %d, %dx%d, pitch %d", createSettings.pixelFormat, createSettings.width, createSettings.height, createSettings.pitch));

        /* copy palette if needed */
        if (NEXUS_PIXEL_FORMAT_IS_PALETTE(createSettings.pixelFormat)) {
            NEXUS_PictureDecoderPalette sidPalette;
            NEXUS_SurfaceMemory mem;

            rc = NEXUS_PictureDecoder_GetPalette(pictureDecoder, &sidPalette);
            BDBG_ASSERT(!rc);
            NEXUS_Surface_GetMemory(picture, &mem),
            BKNI_Memcpy(mem.palette, sidPalette.palette, mem.numPaletteEntries*sizeof(NEXUS_PixelFormat));
            NEXUS_Surface_Flush(picture);
        }

        if (file_size > decoderSettings.bufferSize) {
            BDBG_WRN(("file requires multi-part read. read again."));
            fseek(fin, 0, SEEK_SET);
            rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
            BDBG_ASSERT(!rc);
            rc = fread(buffer, 1, size, fin);                               /* read file into the decoder's buffer */
            if (rc < 0) {
                BDBG_ERR(("failed to read in data: %d", rc));
                goto error;
            }
            rc = NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
            BDBG_ASSERT(!rc);
        }

        /* start decoding */
        rc = NEXUS_PictureDecoder_DecodeSegment(pictureDecoder, picture, NULL);
        BDBG_ASSERT(!rc);
        j = cnt = 0;
        do {
            rc = NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
            BDBG_ASSERT(!rc);
            if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
                BDBG_ERR(("decoding failed"));
                goto error;
            } else if ( pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
                rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
                BDBG_ASSERT(!rc);
                if ( size > bytesRemain ) size = bytesRemain;

                rc = fread(buffer, 1, size, fin);                             /* read file into the decoder's buffer */
                if (rc < 0) {
                    BDBG_ERR(("unable to read more data rc=%d bytesRemain=%u", rc, (unsigned)bytesRemain));
                    goto error;
                }
                rc = NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
                BDBG_ASSERT(!rc);
                bytesRemain -= size;
            }
            usleep(1000);
            cnt++;
            if (cnt%1000 == 0) { /* 1 second */
                printf("waiting for DecodeSegment %d\n" , pictureStatus.state);
                if( ++j>=10 ) { /* increase timeout for (10) some large png files */
                    BDBG_ERR(("Segment decode taking too long. aborting."));
                    goto error;
                }
            }
        } while(pictureStatus.state!=NEXUS_PictureDecoderState_eSegmentDone);   /* wait for picture to decode */
        
        BDBG_WRN(("picture crc: %08x", pictureStatus.crc));

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = picture;
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = pictureStatus.header.width;
        if(NEXUS_PIXEL_FORMAT_IS_YCRCB(pictureStatus.header.format)) {
            blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
        }

        blitSettings.source.rect.height = pictureStatus.header.height;
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = 0;
        blitSettings.output.rect.y = 0;
        blitSettings.output.rect.width = videoFormatInfo.width; /* fill to fit entire screen */
        blitSettings.output.rect.height = videoFormatInfo.height;

        NEXUS_Graphics2D_Blit(gfx, &blitSettings);              /* don't wait for blit to complete */
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);

        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.enabled = true;
        NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);

error:
        if (started) {
            NEXUS_PictureDecoder_Stop(pictureDecoder);
        }
        
        BDBG_WRN(("Press ENTER for next picture"));
        getchar();

        /* Now clean up in reverse order */
        if (picture) {
            NEXUS_Surface_Destroy(picture);
        }
    }

    if (pictureDecoder) {
        NEXUS_PictureDecoder_Close(pictureDecoder);
    }

    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(framebuffer);
    NEXUS_Graphics2D_Close(gfx);
    BKNI_DestroyEvent(checkpointEvent);
    NEXUS_Platform_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void) {printf("picture decoder not supported\n");return -1;}
#endif
