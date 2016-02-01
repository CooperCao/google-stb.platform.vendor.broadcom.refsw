/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/
/* Nexus example app: show jpeg slide show image */
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_DISPLAY && NEXUS_HAS_PICTURE_DECODER
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
#include "bkni_multi.h"
#include <stdlib.h>
#include <unistd.h>

static void PictureDecoderStep(void *context, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent(context);
    fprintf(stderr, "PictureDecoderStep\n");
    return;
}

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_SurfaceHandle picture, framebuffer;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle checkpointEvent;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PictureDecoderHandle pictureDecoder;
    NEXUS_PictureDecoderStartSettings pictureSettings;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error errCode;
#endif
    unsigned i;
    BKNI_EventHandle event;
    FILE *fin;
    void *buffer;
    size_t size;
    int rc;

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

    /* use graphics to fit image to the display framebuffer */
    gfx = NEXUS_Graphics2D_Open(0, NULL);
    BKNI_CreateEvent(&checkpointEvent);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    
    BKNI_CreateEvent(&event);
    pictureDecoder = NEXUS_PictureDecoder_Open(0, NULL);

    for(i=0;;) {
        char jpeg[128];

        if (argc > 1) {
            /* run as: nexus slide_jpeg *.jpg */
            if (i+1 >= (unsigned)argc) {
                i = 0;
            }
            BKNI_Snprintf(jpeg, sizeof(jpeg), "%s", argv[i+1]);
        }
        else {
            BKNI_Snprintf(jpeg, sizeof(jpeg), "videos/picture%u.jpg", i);
        }
        fin = fopen(jpeg,"rb");
        if(!fin) {
            perror(jpeg);
            if(i==0) {
                break;
            }
            i=0;
            continue;
        }
        i++;

        fprintf(stderr, "decoding %s ...\n", jpeg);

        NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
        rc = fread(buffer, 1, size, fin); /* read file into the decoder's buffer */
        fclose(fin);
        if(rc<0) {
            perror(jpeg);
            return 1;
        } else if(rc==(int)size) {
            fseek(fin, 0, SEEK_END);
            if((unsigned)ftell(fin)>size) {
                /* to decode larger file, requires to allocate larger buffer, see NEXUS_PictureDecoderOpenSettings bufferSize */
                fprintf(stderr, "JPEG file size %u is larger then buffer size %u, not supported\n", (unsigned)ftell(fin), (unsigned)size);
                goto err_buf;
            }
        }
        NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */

        NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
        pictureSettings.format = NEXUS_PictureFormat_eJpeg;
        pictureSettings.pictureParsed.callback = PictureDecoderStep;
        pictureSettings.pictureParsed.context = event;
        pictureSettings.segmentDecoded.callback = PictureDecoderStep;
        pictureSettings.segmentDecoded.context = event;
        NEXUS_PictureDecoder_Start(pictureDecoder, &pictureSettings);

        do {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
            if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
                fprintf(stderr, "decoding failed\n");
                goto err_start;
            }
        } while(!pictureStatus.headerValid); /* wait for picture dimensions */

        /* create picture that could handle complete picture */
        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.pixelFormat = pictureStatus.header.format;
        createSettings.width = pictureStatus.header.surface.width;
        createSettings.height = pictureStatus.header.surface.height;
        picture = NEXUS_Surface_Create(&createSettings);

        /* start decoding */
        NEXUS_PictureDecoder_DecodeSegment(pictureDecoder, picture, NULL);
        do {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
            if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
                fprintf(stderr, "decoding failed\n");
                NEXUS_PictureDecoder_Stop(pictureDecoder);
                goto err_segment;
            }
        } while(pictureStatus.state!=NEXUS_PictureDecoderState_eSegmentDone); /* wait for picture to decode */

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = picture;
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = pictureStatus.header.width;
        blitSettings.source.rect.height = pictureStatus.header.height;
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

        NEXUS_Surface_GetCreateSettings(framebuffer, &createSettings);

        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = 0;
        blitSettings.output.rect.y = 0;
        blitSettings.output.rect.width = createSettings.width; /* fill to fit entire screen */
        blitSettings.output.rect.height = createSettings.height;

        NEXUS_Graphics2D_Blit(gfx, &blitSettings);  /* don't wait for blit to complete */
        rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
        }
        BDBG_ASSERT(!rc);

        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.enabled = true;
        NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
        BKNI_Sleep(500);
err_segment:
        NEXUS_Surface_Destroy(picture);
err_start:
err_buf:
        NEXUS_PictureDecoder_Stop(pictureDecoder);
    }
    NEXUS_PictureDecoder_Close(pictureDecoder);

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
