/******************************************************************************
 *    (c)2008 Broadcom Corporation
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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_component_output.h"
#include "nexus_video_decoder.h"

#include "bioatom.h"
#include "bfile_stdio.h"
#include "bfile_buffer.h"

#include "thumbnail.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

BDBG_MODULE(thumbnail_stills);

static NEXUS_SurfaceHandle thumbnail_reduce(const NEXUS_Rect *rect, NEXUS_SurfaceHandle stillSurface);
static void still_picture_ready(void *context, int unused);

int thumbnail_decode_stills_init()
{
    bthumbnail_extractor_create_settings create_settings;
    bthumbnail_extractor_settings settings;
    int rc;

    g_data.stdio_datafile = fopen(g_data.datafilename, "rb");
    if (!g_data.stdio_datafile) {
        fprintf(stderr, "can't open file:%s\n", g_data.datafilename);
        return -1;
    }
    g_data.datafile = bfile_stdio_read_attach(g_data.stdio_datafile);

    if (g_data.indexfilename) {
        g_data.stdio_indexfile = fopen(g_data.indexfilename, "rb");
        if (!g_data.stdio_indexfile) {
            fprintf(stderr, "can't open index:%s\n", g_data.indexfilename);
            return -1;
        }
        g_data.indexfile = bfile_stdio_read_attach(g_data.stdio_indexfile);
    }
    else {
        g_data.indexfile = NULL;
    }

    bthumbnail_extractor_get_default_create_settings(&create_settings);
    g_data.thumbnail_extractor = bthumbnail_extractor_create(&create_settings);

    bthumbnail_extractor_get_settings(g_data.thumbnail_extractor, &settings);
    settings.videoCodec = g_data.videoCodec;
    settings.transportType = g_data.transportType;
    settings.timestampType = g_data.timestampType;
    settings.videoPid = g_data.pid;
    settings.playpump = g_data.playpump;
    settings.datafile = g_data.datafile;
    settings.indexfile = g_data.indexfile;
    rc = bthumbnail_extractor_set_settings(g_data.thumbnail_extractor, &settings);

    BKNI_CreateEvent(&g_data.still_done);
    return 0;
}

void thumbnail_decode_stills_uninit(void)
{
    BKNI_DestroyEvent(g_data.still_done);
    bthumbnail_extractor_destroy(g_data.thumbnail_extractor);

    bfile_stdio_read_detach(g_data.datafile);
    fclose(g_data.stdio_datafile);

    if (g_data.indexfile) bfile_stdio_read_detach(g_data.indexfile);
    if (g_data.stdio_indexfile) fclose(g_data.stdio_indexfile);
}

NEXUS_SurfaceHandle thumbnail_decode_still(unsigned time)
{
    NEXUS_Error rc;
    NEXUS_StillDecoderStartSettings stillDecoderSettings;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_PlaypumpOpenPidChannelSettings openPidChannelSettings;
    unsigned pid;
    NEXUS_SurfaceHandle stillSurface = NULL;

    switch (g_data.transportType) {
    case NEXUS_TransportType_eMkv:
    case NEXUS_TransportType_eMp4:
        pid = 0xE0;
        break;
    default:
        pid = g_data.pid;
        break;
    }

    rc = bthumbnail_extractor_start_playpump(g_data.thumbnail_extractor);
    if (rc) {
        BERR_TRACE(rc);
        return NULL;
    }

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&openPidChannelSettings);
    openPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    pidChannel = NEXUS_Playpump_OpenPidChannel(g_data.playpump, pid, &openPidChannelSettings);
    if (!pidChannel) return NULL;

    NEXUS_StillDecoder_GetDefaultStartSettings(&stillDecoderSettings);
    stillDecoderSettings.pidChannel = pidChannel;
    stillDecoderSettings.stillPictureReady.callback = still_picture_ready;
    stillDecoderSettings.codec = g_data.videoCodec;
    rc = NEXUS_StillDecoder_Start(g_data.stillDecoder, &stillDecoderSettings);
    if (rc) {
        BERR_TRACE(rc);
        return NULL;
    }

    rc = bthumbnail_extractor_feed_picture(g_data.thumbnail_extractor, time * 1000);
    if (rc) {
        BERR_TRACE(rc);
        return NULL;
    }

#if 1
    rc = BKNI_WaitForEvent(g_data.still_done, 2000);
#else
    for (i=0; i<20; i++) {
        NEXUS_StillDecoderStatus status;
        BKNI_Sleep(10);
        NEXUS_StillDecoder_GetStatus(g_data.stillDecoder, &status);
        if (status.stillPictureReady) {
            rc = 0;
            break;
        }
        if (status.avdStatusBlock) { /* TODO: the interpretation of this value is stream-dependent, and not trivial.
                                        See http://consumer-elec-twiki.broadcom.com/bin/view/Avd/AvdStatusBlock */
            rc = 1;
            break;
        }
    }
    if (i==20) {
        rc = 1;
    }
#endif

    if (rc) {
        NEXUS_StillDecoderStatus status;

        rc = NEXUS_StillDecoder_GetStatus(g_data.stillDecoder, &status);
        if (!rc && !status.endCodeFound) {
            BDBG_ERR(("Still decode timed out because no end code was found in the ITB. Is this a valid still?"));
        }
        else if (!rc && !status.stillPictureReady) {
            BDBG_ERR(("Still decode timed out because the decoder did not respond. Is this a valid still?"));
        }
        else {
            BDBG_ERR(("Still decode timed out for unknown reasons."));
        }
    }
    else {
        NEXUS_StripedSurfaceHandle stripedSurface;
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_StripedSurfaceCreateSettings stripedSurfaceCreateSettings;
        NEXUS_Rect rect ={0,0,THUMB_WIDTH,THUMB_HEIGHT};
        bool reduce_more = false;

        rc = NEXUS_StillDecoder_GetStripedSurface(g_data.stillDecoder, &stripedSurface);
        BDBG_ASSERT(!rc);
    
        NEXUS_StripedSurface_GetCreateSettings(stripedSurface, &stripedSurfaceCreateSettings);
        
        /* create a bsurface_t which wraps the BSUR_Handle used by VDC */
        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
        /* scale on the destripe */
        if (stripedSurfaceCreateSettings.imageWidth / rect.width >= 15) {
            surfaceCreateSettings.width = stripedSurfaceCreateSettings.imageWidth / 4;
            reduce_more = true;
        }
        else {
            surfaceCreateSettings.width = rect.width;
        }
        if (stripedSurfaceCreateSettings.imageHeight / rect.height >= 7) {
            surfaceCreateSettings.height = stripedSurfaceCreateSettings.imageHeight / 4;
            reduce_more = true;
        }
        else {
            surfaceCreateSettings.height = rect.height;
        }
        surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
        stillSurface = NEXUS_Surface_Create(&surfaceCreateSettings);
        BDBG_ASSERT(stillSurface);
        
#if 0
/* not needed, but useful for debugging a still decode problem upstream or downstream from this point. */
        {
        NEXUS_Graphics2DFillSettings fillSettings;
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = stillSurface;
        fillSettings.color = 0x00000000; /* transparent */
        NEXUS_Graphics2D_Fill(g_data.blitter, &fillSettings);
        }
#endif
    
        rc = NEXUS_Graphics2D_DestripeToSurface(g_data.blitter, stripedSurface, stillSurface, NULL);
        BDBG_ASSERT(!rc);

        if (reduce_more) {
            stillSurface = thumbnail_reduce(&rect, stillSurface);
            BDBG_ASSERT(stillSurface);
        }

        BDBG_WRN(("got still %p", (void*)stillSurface));
    }

    bthumbnail_extractor_stop_playpump(g_data.thumbnail_extractor);
    NEXUS_StillDecoder_Stop(g_data.stillDecoder);
    NEXUS_Playpump_ClosePidChannel(g_data.playpump, pidChannel);

    return stillSurface;
}

static void still_picture_ready(void *context, int unused)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(unused);

    BKNI_SetEvent(g_data.still_done);
    /* NEXUS_StillDecoderStatus.avdStatusBlock can be used to poll for status instead */
}

static NEXUS_SurfaceHandle thumbnail_reduce(const NEXUS_Rect *rect, NEXUS_SurfaceHandle stillSurface)
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle newSurface;
    NEXUS_SurfaceCreateSettings newCreateSettings;
    bool do_more = false;

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopyConstant; /* YCrCb has no alpha, so we must set 0xFF */
    blitSettings.constantColor = 0xFF000000; /* alpha is opaque */

    /* blitter is limited to 15x downscale. if we need more, we must double blit. delete the
    surface we were given and return a new surface */
    NEXUS_Surface_GetCreateSettings(stillSurface, &createSettings);
    newCreateSettings = createSettings;
    if (createSettings.width / 15 > rect->width || createSettings.height / 15 > rect->height) {
        newCreateSettings.width /= 4;
        newCreateSettings.height /= 4;
        do_more = true;
    }
    else {
        newCreateSettings.width = rect->width;
        newCreateSettings.height = rect->height;
    }

    newCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    newSurface = NEXUS_Surface_Create(&newCreateSettings);

    blitSettings.source.surface = stillSurface;
    blitSettings.output.surface = newSurface;
    blitSettings.output.rect = *rect;
    NEXUS_Graphics2D_Blit(g_data.blitter, &blitSettings);
    
    checkpoint();

    NEXUS_Surface_Destroy(stillSurface);

    if (do_more)
        return thumbnail_reduce(rect, newSurface);
    else
        return newSurface;
}


