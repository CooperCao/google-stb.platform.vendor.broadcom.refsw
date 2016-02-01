/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
******************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_PICTURE_DECODER
#include "bstd.h"
#include "bkni.h"
#include "nexus_types.h"
#include "nexus_display.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_output.h"
#include "nexus_video_window.h"
#include "nexus_video_image_input.h"
#include "nexus_video_input.h"

#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_picture_decoder.h"
#endif
#include "nexus_graphics2d.h"
#include <unistd.h>

BDBG_MODULE(jpeg_image_input);

#if NEXUS_HAS_PICTURE_DECODER
void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int decodeJpeg( const char *filename, NEXUS_SurfaceHandle *picSurface, NEXUS_PictureDecoderStatus *jpegStatus,
                NEXUS_SurfaceCreateSettings *jpegSurfaceSettings, NEXUS_PictureDecoderHandle  *pictureDecoder ) {

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
    pictureSettings.format = NEXUS_PictureFormat_eJpeg;
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
    NEXUS_Surface_GetDefaultCreateSettings(jpegSurfaceSettings);
    jpegSurfaceSettings->pixelFormat = jpegStatus->header.format;
    jpegSurfaceSettings->width       = jpegStatus->header.surface.width;
    jpegSurfaceSettings->height      = jpegStatus->header.surface.height;
    *picSurface = NEXUS_Surface_Create(jpegSurfaceSettings);

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
#endif

int main(int argc, char **argv)
{
    NEXUS_DisplayHandle          display;
    NEXUS_VideoWindowHandle      window;
    NEXUS_VideoWindowSettings    windowCfg;
    NEXUS_PictureDecoderHandle   pictureDecoder;
    NEXUS_SurfaceHandle          jpegSurface;
    NEXUS_PictureDecoderStatus   pictureStatus;
    NEXUS_SurfaceCreateSettings  jpegSurfaceSettings;
    NEXUS_Graphics2DHandle       gfx;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    BKNI_EventHandle             checkpointEvent;
    NEXUS_SurfaceHandle          img;
    NEXUS_SurfaceCreateSettings  surfaceCfg;
    NEXUS_PlatformSettings       platformSettings;
    NEXUS_PlatformConfiguration  platformConfig;
    NEXUS_VideoImageInputHandle  imageInput;
    NEXUS_VideoImageInputStatus  imageInputStatus;
    NEXUS_VideoInput             videoInput;
    NEXUS_DisplaySettings        displaySettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus       hdmiStatus;
#endif
    int rc;
    unsigned i;
    const char *filename = "Sunset.jpg"; /* change this to a jpeg of your choice */

    if (argc > 1) {
        filename = argv[1];
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

    NEXUS_Platform_GetConfiguration(&platformConfig);
    BKNI_CreateEvent(&checkpointEvent);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
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
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
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

    window = NEXUS_VideoWindow_Open(display, 0);
    BDBG_ASSERT(window);
    NEXUS_VideoWindow_GetSettings(window, &windowCfg);
    BDBG_MSG(("windowCfg: width=%d hght=%d x=%d, y=%d", windowCfg.position.width, windowCfg.position.height,
                                                        windowCfg.position.x,     windowCfg.position.y ));

    imageInput = NEXUS_VideoImageInput_Open(0, NULL);
    if(!imageInput) {
        BDBG_ERR(( "Can't create imageInput\n"));
        return -1;
    }
    videoInput = NEXUS_VideoImageInput_GetConnector(imageInput);
    NEXUS_VideoImageInput_GetStatus(imageInput, &imageInputStatus);

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
    /* Scale graphics to reduce bandwith requirements */
    if( windowCfg.position.width == 1920 ) {
        surfaceCfg.width = 960;
    }
    else {
        surfaceCfg.width = 720;
    }
    surfaceCfg.height = windowCfg.position.height;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus s;
        if (!platformConfig.heap[i] || NEXUS_Heap_GetStatus(platformConfig.heap[i], &s)) continue;
        if (s.memcIndex == imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eApplication) && s.largestFreeBlock >= 960*1080*2) {
            surfaceCfg.heap = platformConfig.heap[i];
            break;
        }
    }
    if (!surfaceCfg.heap) {
        BDBG_ERR(("no heap found. RTS failure likely."));
    }
    surfaceCfg.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    img = NEXUS_Surface_Create(&surfaceCfg);
    BDBG_ASSERT(img);

    /* use graphics to fit image into the imageinput surface */
    rc = decodeJpeg(filename, &jpegSurface, &pictureStatus, &jpegSurfaceSettings, &pictureDecoder );
    if (rc) {
        return -1;
    }

    gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface     = jpegSurface;
    blitSettings.source.rect.x      = 0;
    blitSettings.source.rect.y      = 0;
    blitSettings.source.rect.width  = pictureStatus.header.width;
    blitSettings.source.rect.height = pictureStatus.header.height;
    blitSettings.output.surface     = img;
    NEXUS_Graphics2D_Blit(gfx, &blitSettings);  /* don't wait for blit to complete */
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }

    NEXUS_VideoImageInput_SetSurface(imageInput, img);

    /* show picture */
    NEXUS_VideoWindow_AddInput(window, videoInput);

    fprintf(stderr, "\n\tDisplaying Picture\n");
    fprintf(stderr, "\n\tpress enter to exit\n");
    getchar();

    NEXUS_PictureDecoder_Close( pictureDecoder );
    NEXUS_VideoImageInput_SetSurface(imageInput, NULL);
    NEXUS_Display_RemoveAllOutputs(display);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_VideoWindow_RemoveAllInputs(window);
    NEXUS_VideoInput_Shutdown(videoInput);
    NEXUS_VideoImageInput_Close(imageInput);

    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Surface_Destroy(jpegSurface);
    NEXUS_Surface_Destroy(img);
    BKNI_DestroyEvent(checkpointEvent);
    NEXUS_Platform_Uninit();
    return 0;
}
#else /* NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_PICTURE_DECODER */
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
