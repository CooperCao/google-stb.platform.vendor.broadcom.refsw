/******************************************************************************
 *    (c)2010-2014 Broadcom Corporation
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
#include "bkni_multi.h"
#include "bsu_picdecoder.h"

#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_picture_decoder.h"
#include "nexus_graphics2d.h"
#include "nexus_core_utils.h"
#include <stdio.h>
#include <string.h>
#include "bsu.h"
#include "bsu_filesys.h"
#include "bsu-api.h"
#include "bsu-api2.h"

BDBG_MODULE(picdecoder);

struct picdecoder {
    NEXUS_PictureDecoderHandle pictureDecoder;
    NEXUS_PictureDecoderOpenSettings decoderSettings;
    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle checkpointEvent;
    unsigned multiScanBufferSize;
};

extern fileio_ctx_t *fsctx_usb0;

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int picdecoder_p_opensid(picdecoder_t handle)
{
    /* TODO: rework SID to do internal, on-demand alloc of multiscan without close/open */
    if (handle->pictureDecoder) {
        NEXUS_PictureDecoder_Close(handle->pictureDecoder);
        handle->pictureDecoder = NULL;
        BDBG_WRN(("reopening SID with %d multiscan buffer", handle->multiScanBufferSize));
    }
    NEXUS_PictureDecoder_GetDefaultOpenSettings(&handle->decoderSettings);
    handle->decoderSettings.bufferSize = 128*1024*10;
    handle->decoderSettings.multiScanBufferSize = handle->multiScanBufferSize;
    handle->pictureDecoder = NEXUS_PictureDecoder_Open(0, &handle->decoderSettings);
    if (!handle->pictureDecoder) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    return 0;
}

picdecoder_t picdecoder_open(void)
{
    picdecoder_t handle;
    NEXUS_Graphics2DOpenSettings openSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    int rc;

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    memset(handle, 0, sizeof(*handle));

    rc = picdecoder_p_opensid(handle);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }

    rc = BKNI_CreateEvent(&handle->checkpointEvent);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }

    NEXUS_Graphics2D_GetDefaultOpenSettings(&openSettings);
    openSettings.packetFifoSize = 1024; /* only single destripe blit queued */
    handle->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &openSettings);
    if (!handle->gfx) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
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

    return handle;

error:
    picdecoder_close(handle);
    return NULL;
}

void picdecoder_close(picdecoder_t handle)
{
    if (handle->gfx) {
        NEXUS_Graphics2D_Close(handle->gfx);
    }
    if (handle->checkpointEvent) {
        BKNI_DestroyEvent(handle->checkpointEvent);
    }
    if (handle->pictureDecoder) {
        NEXUS_PictureDecoder_Close(handle->pictureDecoder);
    }
    BKNI_Free(handle);
}

int picdecoder_decode(picdecoder_t handle, const char *pictureFilename, NEXUS_SurfaceHandle destSurface, enum picdecoder_aspect_ratio ar)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_PictureDecoderStartSettings pictureSettings;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;
    void *buffer;
    size_t size,file_size;
    int rc = -1;
    FILE *fin;
    NEXUS_SurfaceHandle picture = NULL;
    unsigned cnt;
    size_t bytesRemain = 0;
    NEXUS_SurfaceCreateSettings destSettings;

    BDBG_MSG(("decoding %s...", pictureFilename));
    fin = bsu_fs_open(fsctx_usb0, (char *)pictureFilename, FILE_MODE_READ);
    if (!fin) {
        printf("error from bsu_fs_open\n");
        rc = -1;
        goto error;
    }

    /* Determine file size of image to display */
    file_size = bsu_fs_seek(fsctx_usb0, fin, 1024*1024, FILE_SEEK_BEGINNING);
    rc = bsu_fs_seek(fsctx_usb0, fin, 0, FILE_SEEK_BEGINNING);
    if (rc < 0) {
        printf("error from bsu_fs_seek\n");
    }

    NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
#if 0
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
        rc = -1;
        goto error;
    }
    else {
        pictureSettings.format = NEXUS_PictureFormat_eJpeg;
    }
#else
    pictureSettings.format = NEXUS_PictureFormat_ePng;
#endif
    pictureSettings.imageSize = file_size;

    while (1) {
        /* this while(1) loop is temporarily necessary for a simple virtualization. if this function fails with NEXUS_NOT_AVAILABLE,
        the underlying resource is being used. When the SID FW is rewritten for multicontext support, this loop can be removed. */
        rc = NEXUS_PictureDecoder_GetBuffer(handle->pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
        if (!rc) break; /* we have it */
        if (rc == NEXUS_NOT_AVAILABLE) {
            BKNI_Sleep(1);
        }
        else goto error;
    }

    rc = bsu_fs_read(fsctx_usb0, buffer, 1, size, fin);            /* read file into the decoder's buffer */
    if(rc<0) {
        printf("error:  %s\n", pictureFilename);
        goto error;
    }
    else if(rc==(int)size) {
        if( (unsigned)file_size > size && pictureSettings.format == NEXUS_PictureFormat_eMpeg ) {
            BDBG_ERR(("Picture file size %u is larger then buffer size %u, not supported for MPEG still frames", (unsigned)file_size, (unsigned)size));
            rc = -1;
            goto error;
        }
    }
    rc = NEXUS_PictureDecoder_ReadComplete(handle->pictureDecoder, 0, rc); /* commit data to the decoder */
    if (rc) {rc = BERR_TRACE(rc); goto error;}
    bytesRemain = pictureSettings.imageSize - size ;

    rc = NEXUS_PictureDecoder_Start(handle->pictureDecoder, &pictureSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    cnt = 0;
    do {
        rc = NEXUS_PictureDecoder_GetStatus(handle->pictureDecoder, &pictureStatus);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            BDBG_ERR(("decoding failed reading header"));
            rc = -1;
            goto error;
        }
        else if ( pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
            rc = NEXUS_PictureDecoder_GetBuffer(handle->pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
            if ( size > bytesRemain ) size = bytesRemain;

            if ( rc == NEXUS_SUCCESS ) {
                rc = bsu_fs_read(fsctx_usb0, buffer, 1, size, fin);           /* read file into the decoder's buffer */
                if(rc) {
                    NEXUS_PictureDecoder_ReadComplete(handle->pictureDecoder, 0, rc); /* commit data to the decoder */
                    bytesRemain -= size;
                }
                else {
                    BDBG_ERR(("couldn't read in any more data: rc=%d bytesRemain=%d" , rc, bytesRemain  ));
                    cnt = 999;
                }
            }
        }
        BKNI_Sleep(1);
        if (++cnt == 1000) { /* 1 second */
            BDBG_ERR(("unable to read header (state %d). aborting.", pictureStatus.state));
            rc = -1;
            goto error;
        }
    } while(!pictureStatus.headerValid); /* wait for picture dimensions */

    if (pictureStatus.header.multiscan) {
        /* internally allocate the multiscan buffer and restart decode if needed */
        unsigned minSize = pictureStatus.header.surface.width * pictureStatus.header.surface.height * 6; /* TODO: how to calc? */
        if (handle->multiScanBufferSize < minSize) {
            handle->multiScanBufferSize = minSize;
            rc = picdecoder_p_opensid(handle);
            if (rc) {
                rc = BERR_TRACE(rc);
                goto error;
            }
            return picdecoder_decode(handle, pictureFilename, destSurface, ar);
        }
    }

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
    BDBG_MSG(("creating surface: format %d, %dx%d, pitch %d", createSettings.pixelFormat, createSettings.width, createSettings.height, createSettings.pitch));

    /* copy palette if needed */
    if (NEXUS_PIXEL_FORMAT_IS_PALETTE(createSettings.pixelFormat)) {
        NEXUS_PictureDecoderPalette sidPalette;
        NEXUS_SurfaceMemory mem;

        rc = NEXUS_PictureDecoder_GetPalette(handle->pictureDecoder, &sidPalette);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        NEXUS_Surface_GetMemory(picture, &mem),
        BKNI_Memcpy(mem.palette, sidPalette.palette, mem.numPaletteEntries*sizeof(NEXUS_PixelFormat));
        NEXUS_Surface_Flush(picture);
    }

    if ( file_size > (unsigned)handle->decoderSettings.bufferSize ) {
        BDBG_MSG(("File requires multi part read, SID fw requires read from start"));
        bsu_fs_seek(fsctx_usb0,fin,0,FILE_SEEK_BEGINNING);
        NEXUS_PictureDecoder_GetBuffer(handle->pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
        rc = bsu_fs_read(fsctx_usb0, buffer, 1, size, fin);             /* read file into the decoder's buffer */
        if(rc) {
            NEXUS_PictureDecoder_ReadComplete(handle->pictureDecoder, 0, rc); /* commit data to the decoder */
        }
        else {
            BDBG_ERR(("failed to read in data for %s " , pictureFilename ));
            rc = -1;
            goto error;
        }
    }

    /* start decoding */
    NEXUS_PictureDecoder_DecodeSegment(handle->pictureDecoder, picture, NULL);
    do {
        NEXUS_PictureDecoder_GetStatus(handle->pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            BDBG_ERR(("decoding failed"));
            rc = -1;
            goto error;
        } else if ( pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
            rc = NEXUS_PictureDecoder_GetBuffer(handle->pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
            if ( rc == NEXUS_SUCCESS ) {
                rc = bsu_fs_read(fsctx_usb0, buffer, 1, size, fin);           /* read file into the decoder's buffer */
                if ( size > bytesRemain ) size = bytesRemain;

                if(rc) {
                    NEXUS_PictureDecoder_ReadComplete(handle->pictureDecoder, 0, rc); /* commit data to the decoder */
                    bytesRemain -= size;
                }
                else {
                    BDBG_WRN(("couldn't read any more data: size=%d", size ));
                    rc = -1;
                    goto error;
                }
            }
            else {
                BDBG_WRN(("failed to get buffer to read data into"));
            }
        }
        BKNI_Sleep(1);
    } while(pictureStatus.state!=NEXUS_PictureDecoderState_eSegmentDone);   /* wait for picture to decode */

    NEXUS_PictureDecoder_Stop(handle->pictureDecoder);

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
    blitSettings.output.surface = destSurface;
    if (ar == picdecoder_aspect_ratio_box) {
        unsigned source_ar = pictureStatus.header.width * 100 / pictureStatus.header.height;
        unsigned dest_ar;
        NEXUS_Surface_GetCreateSettings(destSurface, &destSettings);
        dest_ar = destSettings.width * 100 / destSettings.height;
        if (source_ar > dest_ar) {
            /* letterbox: black bars on top and bottom */
            blitSettings.output.rect.x = 0;
            blitSettings.output.rect.width = destSettings.width;
            blitSettings.output.rect.height = destSettings.width / source_ar * 100;
            blitSettings.output.rect.y = (destSettings.height - blitSettings.output.rect.height)/2;
        }
        else if (source_ar < dest_ar) {
            /* windowbox: black bars on left and right */
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.height = destSettings.height;
            blitSettings.output.rect.width = destSettings.height * source_ar / 100;
            blitSettings.output.rect.x = (destSettings.width - blitSettings.output.rect.width)/2;
        }
        else {
            /* bypass drawing black bars */
            ar = picdecoder_aspect_ratio_full;
        }
    }
    NEXUS_Graphics2D_Blit(handle->gfx, &blitSettings);

    if (ar == picdecoder_aspect_ratio_box) {
        /* black bars */
        NEXUS_Graphics2DFillSettings fillSettings;
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = destSurface;
        fillSettings.color = 0xFF000000; /* opaque black */
        fillSettings.rect = blitSettings.output.rect;
        if (!blitSettings.output.rect.x) {
            /* top */
            fillSettings.rect.height = fillSettings.rect.y;
            fillSettings.rect.y = 0;
        }
        else {
            /* left */
            fillSettings.rect.width = fillSettings.rect.x;
            fillSettings.rect.x = 0;
        }
        NEXUS_Graphics2D_Fill(handle->gfx, &fillSettings);
        fillSettings.rect = blitSettings.output.rect;
        if (!blitSettings.output.rect.x) {
            /* bottom */
            fillSettings.rect.y += fillSettings.rect.height;
            fillSettings.rect.height = destSettings.height - fillSettings.rect.y;
        }
        else {
            /* right */
            fillSettings.rect.x += fillSettings.rect.width;
            fillSettings.rect.width = destSettings.width - fillSettings.rect.x;
        }
        NEXUS_Graphics2D_Fill(handle->gfx, &fillSettings);
    }

    rc = NEXUS_Graphics2D_Checkpoint(handle->gfx, NULL); /* require to execute queue */
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(handle->checkpointEvent, BKNI_INFINITE);
    }
    rc = 0;

error:
    if (fin) {
        bsu_fs_close(fsctx_usb0, fin);
    }
    /* always call stop, even if we only called GetBuffer. this releases the internal resource */
    NEXUS_PictureDecoder_Stop(handle->pictureDecoder);
    if (picture) {
        NEXUS_Surface_Destroy(picture);
    }
    return rc;
}
#else
struct picdecoder {
    unsigned unused;
};

picdecoder_t picdecoder_open(void) {
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}
void picdecoder_close(picdecoder_t handle)
{
    BSTD_UNUSED(handle);
}
int picdecoder_decode(picdecoder_t handle, const char *pictureFilename, NEXUS_SurfaceHandle destSurface, enum picdecoder_aspect_ratio ar)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pictureFilename);
    BSTD_UNUSED(destSurface);
    BSTD_UNUSED(ar);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
#endif
