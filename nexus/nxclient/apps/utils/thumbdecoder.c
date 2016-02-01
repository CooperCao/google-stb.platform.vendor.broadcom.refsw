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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "thumbdecoder.h"
#include "nexus_still_decoder.h"
#include "nexus_graphics2d.h"
#include "bioatom.h"
#include "bfile_stdio.h"
#include "bfile_buffer.h"
#include "bthumbnail_extractor.h"
#include "media_probe.h"
#include "botf_bcmindexer.h"
#include "bcmindexer.h"
#include "bcmindexer_nav.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

BDBG_MODULE(thumbdecoder);

struct bfile_io_partial_read_stdio {
    struct bfile_io_read ops; /* shall be the first member */
    FILE *fin;
    unsigned len; /* only allow reading this number of bytes */
};

static ssize_t
b_partial_stdio_read(bfile_io_read_t fd, void *buf, size_t length)
{
    struct bfile_io_partial_read_stdio *f=(void *)fd;
    if (length > f->len) {
        length = f->len;
    }
    f->len -= length;
    return fread(buf, 1, length, f->fin);
}

static const struct bfile_io_read b_stdio_read_ops = {
    b_partial_stdio_read,
    NULL,
    NULL,
    BIO_DEFAULT_PRIORITY
};

struct thumbdecoder
{
    FILE *stdio_indexfile, *stdio_datafile;
    bfile_io_read_t indexfile, datafile;
    struct bfile_io_partial_read_stdio partial_read;
    bthumbnail_extractor_t thumbnail_extractor;

    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle checkpointEvent;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_StillDecoderHandle stillDecoder;
    BKNI_EventHandle stillDecoderEvent;
    struct probe_results results;
    bool cancel;
};

static void complete(void *context, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void checkpoint(thumbdecoder_t handle)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(handle->gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(handle->checkpointEvent, 2000);
    }
    BERR_TRACE(rc);
}

thumbdecoder_t thumbdecoder_open(void)
{
    thumbdecoder_t handle;
    int rc;
    NEXUS_Graphics2DOpenSettings openSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        return NULL;
    }
    memset(handle, 0, sizeof(*handle));
    
    rc = BKNI_CreateEvent(&handle->checkpointEvent);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }
    rc = BKNI_CreateEvent(&handle->stillDecoderEvent);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }

    NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
    openSettings.packetFifoSize = 1024; /* only single destripe blit queued */
    handle->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &openSettings);
    if (!handle->gfx) {
        BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
    
    NEXUS_Graphics2D_GetSettings(handle->gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = handle->checkpointEvent;
    rc = NEXUS_Graphics2D_SetSettings(handle->gfx, &gfxSettings);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }
    
    handle->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    if (!handle->playpump) {
        BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
    
    /* TODO: use simple decoder and video-as-graphics. */
    handle->stillDecoder = NEXUS_StillDecoder_Open(NULL, 0, NULL);
    if (!handle->stillDecoder) {
        BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
    
    return handle;
    
error:
    thumbdecoder_close(handle);
    return NULL;
}

void thumbdecoder_close(thumbdecoder_t handle)
{
    if (handle->stillDecoder) {
        NEXUS_StillDecoder_Close(handle->stillDecoder);
    }
    if (handle->playpump) {
        NEXUS_Playpump_Close(handle->playpump);
    }
    if (handle->gfx) {
        NEXUS_Graphics2D_Close(handle->gfx);
    }
    if (handle->checkpointEvent) {
        BKNI_DestroyEvent(handle->checkpointEvent);
    }
    if (handle->stillDecoderEvent) {
        BKNI_DestroyEvent(handle->stillDecoderEvent);
    }
    BKNI_Free(handle);
}

int thumbdecoder_open_file( thumbdecoder_t handle, const char *streamname, const char *indexname )
{
    bthumbnail_extractor_create_settings create_settings;
    bthumbnail_extractor_settings settings;
    int rc;
    
    BDBG_MSG(("thumbdecoder_open_file %s %s", streamname, indexname));
    rc = probe_media(streamname, &handle->results);
    if (rc) {
        goto error;
    }

    handle->stdio_datafile = fopen(streamname, "rb");
    if (!handle->stdio_datafile) {
        BDBG_ERR(("can't open file:%s", streamname));
        return -1;
    }
    handle->datafile = bfile_stdio_read_attach(handle->stdio_datafile);

    if (indexname) {
        handle->stdio_indexfile = fopen(indexname, "rb");
        if (!handle->stdio_indexfile) {
            BDBG_ERR(("can't open index:%s", indexname));
            goto error;
        }
        handle->indexfile = bfile_stdio_read_attach(handle->stdio_indexfile);
    }
    else if (handle->results.transportType == NEXUS_TransportType_eTs) {
        bfile_io_read_t read;
        BNAV_AVC_Entry nav;
        int n;
        unsigned packetSize = handle->results.timestampType == NEXUS_TransportTimestampType_eNone ? 188 : 192;
        /* for unindexed TS files, attempt an on-the-fly index to get the first I frame. */
        read = botf_bcmindexer_open(streamname, handle->results.video[0].pid, handle->results.video[0].codec, packetSize);
        if (read) {
            /* first NAV entry should be I frame with metadata */
            n = read->read(read, &nav, sizeof(nav));
            botf_bcmindexer_close(read);
            if (n > 0) {
                unsigned offset = BNAV_get_frameOffsetLo(&nav); /* ignore hi */
                unsigned size = BNAV_get_frameSize(&nav);
                unsigned backup;
                if (handle->results.video[0].codec == NEXUS_VideoCodec_eMpeg2) {
                    backup = BNAV_get_seqHdrStartOffset(&nav);
                }
                else {
                    backup = BNAV_get_SPS_Offset(&nav);
                }
                BDBG_MSG(("found I frame: %d %d with backup %d", offset, size, backup));
                offset -= backup;
                size += backup;
                /* packet align */
                if (offset % packetSize) {
                    size += offset % packetSize;
                    offset -= offset % packetSize;
                }
                if (size % packetSize) {
                    size += packetSize - (size % packetSize);
                }
                /* cancel full read and setup partial read so still decoder sees exactly one picture */
                bfile_stdio_read_detach(handle->datafile);
                handle->datafile = NULL;
                fseek(handle->stdio_datafile, offset, SEEK_SET);
                handle->partial_read.ops = b_stdio_read_ops;
                handle->partial_read.len = size;
                handle->partial_read.fin = handle->stdio_datafile;
            }
        }
    }

    bthumbnail_extractor_get_default_create_settings(&create_settings);
    handle->thumbnail_extractor = bthumbnail_extractor_create(&create_settings);
    if (!handle->thumbnail_extractor) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
    
    bthumbnail_extractor_get_settings(handle->thumbnail_extractor, &settings);
    settings.videoCodec = handle->results.video[0].codec;
    settings.transportType = handle->results.transportType;
    settings.timestampType = handle->results.timestampType;
    settings.videoPid = handle->results.video[0].pid;
    settings.playpump = handle->playpump;
    settings.datafile = handle->datafile ? handle->datafile : &handle->partial_read.ops;
    settings.indexfile = handle->indexfile;
    rc = bthumbnail_extractor_set_settings(handle->thumbnail_extractor, &settings);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto error;
    }

    return 0;
    
error:
    thumbdecoder_close_file(handle);
    return rc;
}

void thumbdecoder_close_file( thumbdecoder_t handle )
{
    if (handle->thumbnail_extractor) {
        bthumbnail_extractor_destroy(handle->thumbnail_extractor);
        handle->thumbnail_extractor = NULL;
    }
    if (handle->datafile) {
        bfile_stdio_read_detach(handle->datafile);
        handle->datafile = NULL;
    }
    if (handle->stdio_datafile) {
        fclose(handle->stdio_datafile);
        handle->stdio_datafile = NULL;
    }
    if (handle->indexfile) {
        bfile_stdio_read_detach(handle->indexfile);
        handle->indexfile = NULL;
    }
    if (handle->stdio_indexfile) {
        fclose(handle->stdio_indexfile);
        handle->stdio_indexfile = NULL;
    }
}

void thumbdecoder_decode_cancel(thumbdecoder_t handle)
{
    handle->cancel = true;
    BKNI_SetEvent(handle->stillDecoderEvent);
}

int thumbdecoder_decode_still( thumbdecoder_t handle, unsigned time, NEXUS_SurfaceHandle destSurface )
{
    NEXUS_Error rc;
    NEXUS_StillDecoderStartSettings stillDecoderSettings;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_PlaypumpOpenPidChannelSettings openPidChannelSettings;
    unsigned pid;

    if (!handle->results.num_video || handle->results.video[0].codec == NEXUS_VideoCodec_eUnknown) {
        BDBG_WRN(("no video"));
        return NEXUS_NOT_AVAILABLE;
    }
    
    handle->cancel = false;
    switch (handle->results.transportType) {
    case NEXUS_TransportType_eMkv:
    case NEXUS_TransportType_eMp4:
        pid = 0xE0;
        break;
    default:
        pid = handle->results.video[0].pid;
        break;
    }

    rc = bthumbnail_extractor_start_playpump(handle->thumbnail_extractor);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&openPidChannelSettings);
    openPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    pidChannel = NEXUS_Playpump_OpenPidChannel(handle->playpump, pid, &openPidChannelSettings);
    if (!pidChannel) {
        rc = BERR_TRACE(-1);
        goto done;
    }

    NEXUS_StillDecoder_GetDefaultStartSettings(&stillDecoderSettings);
    stillDecoderSettings.pidChannel = pidChannel;
    stillDecoderSettings.stillPictureReady.callback = complete;
    stillDecoderSettings.stillPictureReady.context = handle->stillDecoderEvent;
    stillDecoderSettings.codec = handle->results.video[0].codec;
    rc = NEXUS_StillDecoder_Start(handle->stillDecoder, &stillDecoderSettings);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto done;
    }

    rc = bthumbnail_extractor_feed_picture(handle->thumbnail_extractor, time);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto done;
    }

    rc = BKNI_WaitForEvent(handle->stillDecoderEvent, 2000);
    if (handle->cancel) {
        /* no op */
    }
    else if (rc) {
        NEXUS_StillDecoderStatus status;
        int rc; /* local rc. the WaitForEvent rc must be propagated to caller. */

        rc = NEXUS_StillDecoder_GetStatus(handle->stillDecoder, &status);
        if (!rc && !status.endCodeFound) {
            BDBG_ERR(("Still decode timed out because no end code was found in the ITB."));
        }
        else if (!rc && !status.stillPictureReady) {
            BDBG_ERR(("Still decode timed out because the decoder did not respond."));
        }
        else {
            BDBG_ERR(("Still decode timed out."));
        }
    }
    else {
        NEXUS_StripedSurfaceHandle stripedSurface;

        rc = NEXUS_StillDecoder_GetStripedSurface(handle->stillDecoder, &stripedSurface);
        if (rc) return BERR_TRACE(rc);
    
        rc = NEXUS_Graphics2D_DestripeToSurface(handle->gfx, stripedSurface, destSurface, NULL);
        if (rc) return BERR_TRACE(rc);

        checkpoint(handle);
    }

done:
    bthumbnail_extractor_stop_playpump(handle->thumbnail_extractor);
    NEXUS_StillDecoder_Stop(handle->stillDecoder);
    if (pidChannel) {
        NEXUS_Playpump_ClosePidChannel(handle->playpump, pidChannel);
    }

    return rc;
}
