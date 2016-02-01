/******************************************************************************
 *    (c)2012 Broadcom Corporation
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
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_DISPLAY
#include "nexus_display.h"
#include "nexus_core_utils.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_decoder.h"
#include "nexus_still_decoder.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_playpump.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bmedia_util.h"

void still_picture_ready(void *context, int unused)
{
    BSTD_UNUSED(unused);
    printf("still_picture_ready\n");
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void blitter_done(void *context, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static NEXUS_Error feed_file(NEXUS_PlaypumpHandle playpump, FILE *file, size_t data_size)
{
    unsigned offset;
    for(offset=0;offset<data_size;) {
        size_t size;
        void *buffer;
        ssize_t bytes;
        NEXUS_Error rc = NEXUS_Playpump_GetBuffer(playpump, &buffer, &size);
        if(rc!=NEXUS_SUCCESS) {return rc;}

        if (size==0) {
            BKNI_Sleep(10);
            continue;
        }

        if(size > data_size - offset) {
            size = data_size + offset;
        }

        bytes = fread(buffer, 1, size, file);
        if(bytes!=(ssize_t)size) {
            return NEXUS_NOT_SUPPORTED;
        }
        rc = NEXUS_Playpump_WriteComplete(playpump, 0, size);
        if(rc!=NEXUS_SUCCESS) {return rc;}
        offset += size;
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error feed_data(NEXUS_PlaypumpHandle playpump, const void *data, size_t data_size)
{
    unsigned offset;
    for(offset=0;offset<data_size;) {
        size_t size;
        void *buffer;
        NEXUS_Error rc = NEXUS_Playpump_GetBuffer(playpump, &buffer, &size);
        if(rc!=NEXUS_SUCCESS) {return rc;}

        if (size==0) {
            BKNI_Sleep(10);
            continue;
        }
        if(size > data_size - offset) {
            size = data_size + offset;
        }
        BKNI_Memcpy(buffer, (uint8_t *)data + offset, size);
        rc = NEXUS_Playpump_WriteComplete(playpump, 0, size);
        if(rc!=NEXUS_SUCCESS) {return rc;}
        offset += size;
    }
    return NEXUS_SUCCESS;
}

int main(int argc, char **argv)
{
    NEXUS_SurfaceHandle framebufferSurface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_Graphics2DHandle blitter;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_StillDecoderHandle stillDecoder;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpSettings playpumpSettings;
    BKNI_EventHandle stillPictureReadyEvent,checkpointEvent;
    NEXUS_Graphics2DFillSettings fillSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    int i;
    NEXUS_Error rc;
    NEXUS_VideoCodec codec = NEXUS_VideoCodec_eMpeg2;
    NEXUS_StripedSurfaceStatus stripedSurfaceStatus;
    NEXUS_Graphics2DSettings graphics2DSettings;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.videoDecoderModuleSettings.supportedCodecs[NEXUS_VideoCodec_eVp8] = true;
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&stillPictureReadyEvent);
    BKNI_CreateEvent(&checkpointEvent);

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
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( 0 && !rc && hdmiStatus.connected )
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

    blitter = NEXUS_Graphics2D_Open(0, NULL);

    NEXUS_Graphics2D_GetSettings(blitter, &graphics2DSettings);
    graphics2DSettings.checkpointCallback.callback = blitter_done;
    graphics2DSettings.checkpointCallback.context = checkpointEvent;
    NEXUS_Graphics2D_SetSettings(blitter, &graphics2DSettings);

    /* configure transport and still decoder */
    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);

    /* This simple example is hardcoded for MPEG ES streams. It can be easily converted to other
    ES formats, but cannot be easily converted to extracting from any stream format other than PES and TS.
    See BSEAV/app/thumbnail and BSEAV/lib/thumbnail for a more sophisticated example which extracts from
    a variety of containers like ASF, AVI, MKV and MP4. Extracting from these containers
    requires bmedia_player, which is used by BSEAV/lib/thumbnail/bthumbnail_extractor.c. */
    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
    stillDecoder = NEXUS_StillDecoder_Open(NULL, 0, NULL);

    /* create graphics framebuffer */

    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = videoFormatInfo.height;
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebufferSurface = NEXUS_Surface_Create(&createSettings);
    assert(framebufferSurface);

    /* fill black before the first still */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = framebufferSurface;
    fillSettings.color = 0xFF000000; /* black */
    rc = NEXUS_Graphics2D_Fill(blitter, &fillSettings);
    assert(!rc);
    
    NEXUS_Graphics2D_Checkpoint(blitter, NULL);

    /* force full screen framebuffer. no horizontal upscale. */
    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.clip.width = createSettings.width;
    graphicsSettings.clip.height = createSettings.height;
    graphicsSettings.enabled = true;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
    NEXUS_Display_SetGraphicsFramebuffer(display, framebufferSurface);

    /* decode and display each still on the command line */
    for (i=1;i<argc;i++) 
    {
        NEXUS_Graphics2DBlitSettings blitSettings;
        NEXUS_StillDecoderStartSettings stillDecoderSettings;
        NEXUS_StripedSurfaceHandle stripedSurface;
        NEXUS_SurfaceHandle stillSurface = NULL;
        FILE *file = NULL;
        struct stat st;
        uint8_t buffer[256+32];
        ssize_t bytes;
        size_t vp8_bytes;
        bmedia_pes_info pes_info;
        size_t pes_header_len;
        size_t pes_payload_len;
        unsigned offset = 0;
        bmedia_bcmv_hdr bcmv_hdr;

        codec = NEXUS_VideoCodec_eVp8;

        rc = stat(argv[i], &st);
        if (!rc) {
            if (S_ISDIR(st.st_mode)) {
                fprintf(stderr, "skipping directory %s\n", argv[i]);
                continue;
            }
            file = fopen(argv[i], "r");
        }
        if (!file) {
            fprintf(stderr, "can't open file:%s\n", argv[i]);
            continue;
        }
        bytes = fread(buffer, 1, 20, file);
        vp8_bytes = 0;
        if(bytes==20 && 
           B_MEDIA_LOAD_UINT32_LE(buffer, 0) == BMEDIA_FOURCC('R','I','F','F') &&
           B_MEDIA_LOAD_UINT32_LE(buffer, 8) == BMEDIA_FOURCC('W','E','B','P') &&
           B_MEDIA_LOAD_UINT32_LE(buffer, 12) == BMEDIA_FOURCC('V','P','8',' ')
          ) {
           vp8_bytes = B_MEDIA_LOAD_UINT32_LE(buffer, 16);
        }
        if(vp8_bytes==0) {
            fprintf(stderr, "invalid file :%s\n", argv[i]);
            fclose(file);
            continue;
        }


        printf("decoding %s...\n", argv[i]);
        BKNI_ResetEvent(stillPictureReadyEvent);

        NEXUS_StillDecoder_GetDefaultStartSettings(&stillDecoderSettings);
        stillDecoderSettings.pidChannel = NEXUS_Playpump_OpenPidChannel(playpump, 0xE0, NULL);
        assert(stillDecoderSettings.pidChannel);
        stillDecoderSettings.stillPictureReady.callback = still_picture_ready;
        stillDecoderSettings.stillPictureReady.context = stillPictureReadyEvent;
        stillDecoderSettings.codec = codec;
        rc = NEXUS_StillDecoder_Start(stillDecoder, &stillDecoderSettings);
        assert(!rc);

        rc = NEXUS_Playpump_Start(playpump);
        assert(!rc);

        bmedia_pes_info_init(&pes_info, 0xE0);

        /* feed first packet with PTS */
        BMEDIA_PES_SET_PTS(&pes_info, 0);
        pes_payload_len = bmedia_frame_bcmv.len +  sizeof(bcmv_hdr.header);
        pes_header_len = bmedia_pes_header_init(buffer, pes_payload_len, &pes_info);
        bmedia_bcmv_hdr_init(&bcmv_hdr, vp8_bytes);
        BKNI_Memcpy(buffer+pes_header_len, bmedia_frame_bcmv.base, bmedia_frame_bcmv.len);
        BKNI_Memcpy(buffer+pes_header_len + bmedia_frame_bcmv.len, bcmv_hdr.header, sizeof(bcmv_hdr.header));
        
        feed_data(playpump, buffer, pes_header_len + pes_payload_len);

        /* clear PTS and send rest of the frame */
        BMEDIA_PES_UNSET_PTS(&pes_info);
        for(offset=0;offset<vp8_bytes;) {
            pes_payload_len = vp8_bytes - offset;
            if(pes_payload_len>48*1024) {
                pes_payload_len = 48*1024;
            }
            pes_header_len = bmedia_pes_header_init(buffer, pes_payload_len, &pes_info);
            feed_data(playpump, buffer, pes_header_len);
            feed_file(playpump, file, pes_payload_len);
            offset += pes_payload_len;
        }
        /* send EOS  twice */
        BMEDIA_PES_SET_PTS(&pes_info, 0xFFFFFFFF);
        pes_payload_len = bmedia_eos_bcmv.len;
        pes_header_len = bmedia_pes_header_init(buffer, pes_payload_len, &pes_info);
        BKNI_Memcpy(buffer+pes_header_len, bmedia_eos_bcmv.base, bmedia_eos_bcmv.len);
        feed_data(playpump, buffer, pes_header_len+pes_payload_len);
        feed_data(playpump, buffer, pes_header_len+pes_payload_len);

        /* feeding 256 bytes of zeros is required to flush the last packet of data through transport */
        BMEDIA_PES_UNSET_PTS(&pes_info);
        pes_payload_len = 256;
        pes_header_len = bmedia_pes_header_init(buffer, pes_payload_len, &pes_info);
        BKNI_Memset(buffer+pes_header_len, 0, pes_payload_len);
        feed_data(playpump, buffer, pes_header_len+pes_payload_len);

        /* now we can decode the still */
        rc = BKNI_WaitForEvent(stillPictureReadyEvent, 3000);
        if (rc) {
            printf("Unable to decode still\n");
            goto done;
        }

        rc = NEXUS_StillDecoder_GetStripedSurface(stillDecoder, &stripedSurface);
        assert(!rc);
        assert(stripedSurface);

        NEXUS_StripedSurface_GetStatus(stripedSurface, &stripedSurfaceStatus);
        printf("surface will be %dx%d\n", stripedSurfaceStatus.width, stripedSurfaceStatus.height);

        stillSurface = NEXUS_Graphics2D_Destripe(blitter, stripedSurface);
        assert(stillSurface);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = stillSurface;
        blitSettings.output.surface = framebufferSurface;
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopyConstant; /* YCrCb has no alpha, so we must set 0xFF */
        blitSettings.constantColor = 0xFF000000; /* alpha is opaque */
        rc = NEXUS_Graphics2D_Blit(blitter, &blitSettings);
        assert(!rc);
        rc = NEXUS_Graphics2D_Checkpoint(blitter, NULL);
        assert(rc==NEXUS_SUCCESS || rc==NEXUS_GRAPHICS2D_QUEUED);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
            assert(!rc);
        }
        
done:
        NEXUS_StillDecoder_Stop(stillDecoder);
        NEXUS_Playpump_Stop(playpump);

        printf("Press ENTER for next still\n");
        getchar();

        fclose(file);
        NEXUS_Playpump_ClosePidChannel(playpump, stillDecoderSettings.pidChannel);
        if (stillSurface) {
            NEXUS_Surface_Destroy(stillSurface);
        }
    }
    
    NEXUS_StillDecoder_Close(stillDecoder);
    NEXUS_Playpump_Close(playpump);
    NEXUS_Display_Close(display);
    NEXUS_Graphics2D_Close(blitter);
    BKNI_DestroyEvent(stillPictureReadyEvent);
    BKNI_DestroyEvent(checkpointEvent);
    NEXUS_Surface_Destroy(framebufferSurface);
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
