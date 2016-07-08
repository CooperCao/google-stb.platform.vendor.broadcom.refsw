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
#include "picdecoder.h"

#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_picture_decoder.h"
#endif
#include "nexus_graphics2d.h"
#include "nexus_core_utils.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

BDBG_MODULE(picdecoder);

struct picdecoder {
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_PictureDecoderHandle pictureDecoder;
    NEXUS_PictureDecoderOpenSettings decoderSettings;
    unsigned multiScanBufferSize;
    NEXUS_PictureDecoderStatus pictureStatus;
#endif
    NEXUS_Graphics2DHandle gfx;
    BKNI_EventHandle checkpointEvent;
};

static NEXUS_SurfaceHandle picdecoder_p_decode_bmp(const char *pictureFilename);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

#if NEXUS_HAS_PICTURE_DECODER
static int picdecoder_p_opensid(picdecoder_t handle)
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
        if (handle->decoderSettings.multiScanBufferSize) {
            handle->decoderSettings.multiScanBufferSize = 0;
            BDBG_WRN(("unable to open SID with %d multiscan buffer. going back to 0.", handle->multiScanBufferSize));
            handle->pictureDecoder = NEXUS_PictureDecoder_Open(0, &handle->decoderSettings);
        }
        if (!handle->pictureDecoder) {
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
    }
    return 0;
}
#endif

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

#if NEXUS_HAS_PICTURE_DECODER
    rc = picdecoder_p_opensid(handle);
    if (rc) {
        BERR_TRACE(rc);
        goto error;
    }
#endif

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
#if NEXUS_HAS_PICTURE_DECODER
    if (handle->pictureDecoder) {
        NEXUS_PictureDecoder_Close(handle->pictureDecoder);
    }
#endif
    BKNI_Free(handle);
}

NEXUS_SurfaceHandle picdecoder_decode(picdecoder_t handle, const char *pictureFilename)
{
    NEXUS_SurfaceHandle picture = NULL;
#if NEXUS_HAS_PICTURE_DECODER
    int rc = -1;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_PictureDecoderStartSettings pictureSettings;
    void *buffer;
    size_t size,file_size;
    FILE *fin = NULL;
    unsigned cnt;
    size_t bytesRemain = 0;
    bool started = false;
#endif

    BDBG_MSG(("decoding %s...", pictureFilename));

#if !NEXUS_HAS_PICTURE_DECODER

    BSTD_UNUSED(handle);

    if (strcasestr(pictureFilename, ".bmp")) {
        picture = picdecoder_p_decode_bmp(pictureFilename);
        if (!picture) {
            return NULL;
        }
        goto done;
    }
    else {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }
#else
    /* TODO: probe file instead of relying on extensions. see BSEAV/lib/bwin/src/bwin_image*.c */
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
        picture = picdecoder_p_decode_bmp(pictureFilename);
        goto done;
    }
    else {
        pictureSettings.format = NEXUS_PictureFormat_eJpeg;
    }

    fin = fopen(pictureFilename,"rb");
    if(!fin) {
        perror(pictureFilename);
        rc = -1;
        goto error;
    }

    /* Determine file size of image to display */
    fseek(fin, 0, SEEK_END);
    file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    pictureSettings.imageSize = file_size;

    started = true;
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

    rc = fread(buffer, 1, size, fin);                               /* read file into the decoder's buffer */
    if(rc<0) {
        perror(pictureFilename);
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
        rc = NEXUS_PictureDecoder_GetStatus(handle->pictureDecoder, &handle->pictureStatus);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
        if(handle->pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            BDBG_ERR(("decoding failed reading header"));
            rc = -1;
            goto error;
        }
        else if ( handle->pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
            rc = NEXUS_PictureDecoder_GetBuffer(handle->pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
            if ( size > bytesRemain ) size = bytesRemain;

            if ( rc == NEXUS_SUCCESS ) {
                rc = fread(buffer, 1, size, fin);                             /* read file into the decoder's buffer */
                if(rc) {
                    NEXUS_PictureDecoder_ReadComplete(handle->pictureDecoder, 0, rc); /* commit data to the decoder */
                    bytesRemain -= size;
                }
                else {
                    BDBG_ERR(("couldn't read in any more data: rc=%d bytesRemain=%d" , rc, (unsigned)bytesRemain  ));
                    cnt = 999;
                }
            }
        }
        usleep(1000);
        if (++cnt == 1000) { /* 1 second */
            BDBG_ERR(("unable to read header (state %d). aborting.", handle->pictureStatus.state));
            rc = -1;
            goto error;
        }
    } while(!handle->pictureStatus.headerValid); /* wait for picture dimensions */

    if (handle->pictureStatus.header.multiscan) {
        /* internally allocate the multiscan buffer and restart decode if needed */
        unsigned minSize = handle->pictureStatus.header.surface.width * handle->pictureStatus.header.surface.height * 6; /* TODO: how to calc? */
        if (handle->multiScanBufferSize < minSize) {
            handle->multiScanBufferSize = minSize;
            rc = picdecoder_p_opensid(handle);
            if (rc) {
                rc = BERR_TRACE(rc);
                goto error;
            }
            return picdecoder_decode(handle, pictureFilename);
        }
    }

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = handle->pictureStatus.header.format;
    createSettings.width = handle->pictureStatus.header.surface.width;
    createSettings.height = handle->pictureStatus.header.surface.height;
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
    if (!picture) {
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto error;
    }
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
        fseek(fin, 0, SEEK_SET);
        NEXUS_PictureDecoder_GetBuffer(handle->pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
        rc = fread(buffer, 1, size, fin);                               /* read file into the decoder's buffer */
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
    rc = NEXUS_PictureDecoder_DecodeSegment(handle->pictureDecoder, picture, NULL);
    if (rc) { BERR_TRACE(rc); goto error; }
    do {
        rc = NEXUS_PictureDecoder_GetStatus(handle->pictureDecoder, &handle->pictureStatus);
        if (rc) { BERR_TRACE(rc); goto error; }
        if(handle->pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            BDBG_ERR(("decoding failed"));
            rc = -1;
            goto error;
        } else if ( handle->pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
            rc = NEXUS_PictureDecoder_GetBuffer(handle->pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
            if (rc) { BERR_TRACE(rc); goto error; }
            rc = fread(buffer, 1, size, fin);                             /* read file into the decoder's buffer */
            if ( size > bytesRemain ) size = bytesRemain;

            if(rc) {
                rc = NEXUS_PictureDecoder_ReadComplete(handle->pictureDecoder, 0, rc); /* commit data to the decoder */
                if (rc) { BERR_TRACE(rc); goto error; }
                bytesRemain -= size;
            }
            else {
                BDBG_WRN(("couldn't read any more data: size=%d", (unsigned)size ));
                rc = -1;
                goto error;
            }
        }
        usleep(1000);
    } while(handle->pictureStatus.state!=NEXUS_PictureDecoderState_eSegmentDone);   /* wait for picture to decode */
#endif

done:
#if NEXUS_HAS_PICTURE_DECODER
    if (fin) {
        fclose(fin);
    }
    if (started) {
        /* always call stop, even if we only called GetBuffer. this releases the internal resource */
        NEXUS_PictureDecoder_Stop(handle->pictureDecoder);
    }
#endif
    return picture;

#if NEXUS_HAS_PICTURE_DECODER
error:
    if (picture) {
        NEXUS_Surface_Destroy(picture);
        picture = NULL;
    }
    goto done;
#endif
}

int picdecoder_ar_correct( picdecoder_t handle, NEXUS_SurfaceHandle source, NEXUS_SurfaceHandle dest, enum picdecoder_aspect_ratio ar )
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceCreateSettings destSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    int rc;

    NEXUS_Surface_GetCreateSettings(source, &createSettings);
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = source;
    blitSettings.source.rect.x = 0;
    blitSettings.source.rect.y = 0;
    blitSettings.source.rect.width = createSettings.width;
    if(NEXUS_PIXEL_FORMAT_IS_YCRCB(createSettings.pixelFormat)) {
        blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
    }
    blitSettings.source.rect.height = createSettings.height;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;
    blitSettings.output.surface = dest;
    if (ar == picdecoder_aspect_ratio_box) {
        unsigned source_ar = createSettings.width * 100 / createSettings.height;
        unsigned dest_ar;
        NEXUS_Surface_GetCreateSettings(dest, &destSettings);
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
            ar = picdecoder_aspect_ratio_max;
        }
    }
    NEXUS_Graphics2D_Blit(handle->gfx, &blitSettings);

    if (ar == picdecoder_aspect_ratio_box) {
        /* black bars */
        NEXUS_Graphics2DFillSettings fillSettings;
        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = dest;
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
    return rc;
}

/* see http://en.wikipedia.org/wiki/BMP_file_format */
/* 14 byte struct requires gcc attribute */
struct __attribute__ ((__packed__)) bmp_header {
    unsigned short int type;
    unsigned int size;
    unsigned short int reserved1, reserved2;
    unsigned int offset;
};

struct bmp_infoheader {
    unsigned int size;
    int width,height;
    unsigned short int planes;
    unsigned short int bits;
    unsigned int compression;
    unsigned int imagesize;
    int xresolution,yresolution;
    unsigned int ncolours;
    unsigned int importantcolours;
};

#define HEADER_SIZE (sizeof(struct bmp_header) + sizeof(struct bmp_infoheader))

int picdecoder_write_bmp(const char *pictureFilename, NEXUS_SurfaceHandle surface, unsigned bpp)
{
    struct bmp_header header;
    struct bmp_infoheader infoheader;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceMemory mem;
    FILE *fout;
    unsigned image_size;
    int rc = 0;
    unsigned rownum;

    NEXUS_Surface_GetCreateSettings(surface, &createSettings);
    NEXUS_Surface_GetMemory(surface, &mem);
    /* TODO: expand support */
    if (createSettings.pixelFormat != NEXUS_PixelFormat_eA8_R8_G8_B8) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (bpp != 24 && bpp != 32) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    fout = fopen(pictureFilename, "wb+");
    if (!fout) {
        BDBG_WRN(("unable to create %s", pictureFilename));
        rc = -1;
        goto done;
    }

    image_size = createSettings.width * createSettings.height * (bpp / 8);

    memset(&header, 0, sizeof(header));
    ((unsigned char *)&header.type)[0] = 'B';
    ((unsigned char *)&header.type)[1] = 'M';
    header.size = image_size + HEADER_SIZE;
    header.offset = HEADER_SIZE;
    memset(&infoheader, 0, sizeof(infoheader));
    infoheader.size = sizeof(infoheader);
    infoheader.width = createSettings.width;
    infoheader.height = createSettings.height;
    infoheader.planes = 1;
    infoheader.bits = bpp;
    infoheader.imagesize = image_size;

    fwrite(&header, 1, sizeof(header), fout);
    fwrite(&infoheader, 1, sizeof(infoheader), fout);

    for (rownum=0;rownum<createSettings.height;rownum++) {
        unsigned rowsize = createSettings.width * infoheader.bits/8;
        unsigned char *row = (unsigned char *)mem.buffer + ((createSettings.height-rownum-1) * mem.pitch);
        if (bpp == 24) {
            unsigned i;
            for (i=0;i<(unsigned)createSettings.width*4;i+=4) {
                /* write BGR. TODO: big endian */
                fwrite(&row[i+0], 1, 1, fout);
                fwrite(&row[i+1], 1, 1, fout);
                fwrite(&row[i+2], 1, 1, fout);
            }
        }
        else {
            fwrite(row, 1, rowsize, fout);
        }
        if (rowsize % 4) {
            char pad[3] = {0,0,0};
            fwrite(pad, 1, 4 - rowsize % 4, fout);
        }
    }

done:
    if (fout) {
        fclose(fout);
    }
    return rc;
}

/* Access the data by bytes so we don't do un-aligned accesses */
#define B_GET_U16(x) (x[1]<<8  | x[0]<<0)
#define B_GET_U32(x) (x[3]<<24 | x[2]<<16 | x[1]<<8 | x[0]<<0)

static NEXUS_SurfaceHandle picdecoder_p_decode_bmp(const char *pictureFilename)
{
    struct bmp_header header;
    struct bmp_infoheader infoheader;
    unsigned file_size;
    NEXUS_SurfaceHandle surface = NULL;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceMemory mem;
    unsigned char data[HEADER_SIZE], *ptr = data;
    int n;
    bool transform_24bpp = false;
    FILE *fin;
    int rc = 0;

    fin = fopen(pictureFilename, "rb");
    if (!fin) {
        BDBG_WRN(("unable to open %s", pictureFilename));
        rc = -1;
        goto done;
    }

    n = fread(data, 1, sizeof(data), fin);
    if (n == -1) {
        rc = BERR_TRACE(errno);
        goto done;
    }

    /* BMP header format is little endian, access the data appropriate for the platform. */
    header.type      = B_GET_U16(ptr); ptr += 2;
    header.size      = B_GET_U32(ptr); ptr += 4;
    header.reserved1 = B_GET_U16(ptr); ptr += 2;
    header.reserved2 = B_GET_U16(ptr); ptr += 2;
    header.offset    = B_GET_U32(ptr); ptr += 4;
    infoheader.size        = B_GET_U32(ptr); ptr += 4;
    infoheader.width       = B_GET_U32(ptr); ptr += 4;
    infoheader.height      = B_GET_U32(ptr); ptr += 4;
    infoheader.planes      = B_GET_U16(ptr); ptr += 2;
    infoheader.bits        = B_GET_U16(ptr); ptr += 2;
    infoheader.compression = B_GET_U32(ptr); ptr += 4;
    infoheader.imagesize   = B_GET_U32(ptr); ptr += 4;
    infoheader.xresolution = B_GET_U32(ptr); ptr += 4;
    infoheader.yresolution = B_GET_U32(ptr); ptr += 4;
    infoheader.ncolours    = B_GET_U32(ptr); ptr += 4;
    infoheader.importantcolours = B_GET_U32(ptr); ptr += 4;

    BDBG_MSG(("image %c%c, %d bytes, %dx%d pixels, %d bpp",
        ((unsigned char *)&header.type)[0], ((unsigned char *)&header.type)[1],
        header.size, infoheader.width, infoheader.height, infoheader.bits));

    if (header.offset != sizeof(header) + sizeof(infoheader) ||
        infoheader.size != sizeof(infoheader)) {
        BDBG_ERR(("file '%s': different header %d %d", pictureFilename, header.offset, infoheader.size));
        rc = -1;
        goto done;
    }
    if (infoheader.compression) {
        BDBG_ERR(("file '%s': bmp decompression %d not supported.", pictureFilename, infoheader.compression));
        rc = -1;
        goto done;
    }
    if (infoheader.ncolours) {
        BDBG_ERR(("file '%s': palettized bmp's not supported. (ncolors=%d)", pictureFilename, infoheader.ncolours));
        rc = -1;
        goto done;
    }

    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);

    switch (infoheader.bits) {
    case 24:
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        transform_24bpp = true;
        break;
    case 32:
        createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        break;
    default:
        BDBG_ERR(("file '%s': %d bpp not supported.", pictureFilename, infoheader.bits));
        rc = -1;
        goto done;
    }

    createSettings.width = infoheader.width;
    createSettings.height = infoheader.height;
    surface = NEXUS_Surface_Create(&createSettings);
    if (!surface) {
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto done;
    }

    NEXUS_Surface_GetMemory(surface, &mem);
    {
        unsigned char *row;
        unsigned rowsize = createSettings.width * infoheader.bits/8;
        unsigned rownum = 0;

        /* BMP requires 4 byte aligned rows */
        if (rowsize % 4) rowsize += 4-rowsize%4;

        row = BKNI_Malloc(rowsize);
        if (!row) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto done;
        }
        file_size = createSettings.height * rowsize;
        while (1) {
            int i;

            /* write in reverse order */
            ptr = (unsigned char *)mem.buffer + ((createSettings.height-rownum-1) * mem.pitch);
            n = fread(row, 1, rowsize, fin);
            if (n < 0) {
                BERR_TRACE(errno);
                break;
            }
            if (n == 0) {
                BDBG_WRN(("%d bytes missing", file_size));
                break;
            }
            if (transform_24bpp) {
                for (i=0;i+2<n;i+=3) {
                    /* TODO: big endian */
                    ptr[0] = row[i]; /* B */
                    ptr[1] = row[i+1]; /* G */
                    ptr[2] = row[i+2]; /* R */
                    ptr[3] = 0xFF; /* A */
                    ptr += 4;
                }
            }
            else {
                BKNI_Memcpy(ptr, row, rowsize);
            }
            file_size -= n;
            if (!file_size) break;
            rownum++;
        }
        BKNI_Free(row);
    }
    NEXUS_Surface_Flush(surface);

done:
    if (rc && surface) {
        NEXUS_Surface_Destroy(surface);
        surface = NULL;
    }
    if (fin) {
        fclose(fin);
    }
    return surface;
}

#if NEXUS_HAS_PICTURE_DECODER
void picdecoder_get_status(picdecoder_t handle, NEXUS_PictureDecoderStatus *pStatus)
{
    *pStatus = handle->pictureStatus;
}
#endif
