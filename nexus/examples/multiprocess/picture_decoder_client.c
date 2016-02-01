/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
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
 **************************************************************************/

#if NEXUS_HAS_PICTURE_DECODER && NEXUS_HAS_GRAPHICS2D
#include "nexus_platform_client.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_memory.h"
#include "nexus_base_mmap.h"
#include "nexus_surface_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_picture_decoder.h"
#include "nexus_core_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(picture_decoder_client);

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent)
{
    int rc;
    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        BKNI_WaitForEvent(checkpointEvent, BKNI_INFINITE);
    }
}

static int decode_picture(NEXUS_PictureDecoderHandle pictureDecoder, unsigned bufferSize, NEXUS_SurfaceHandle destSurface, const char *pictureFilename, NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent);

static void print_usage(void)
{
    printf(
        "usage: nexus.client picture_decoder_client FILE [FILE ...]\n"
        "  file types are chosen by extension as follows:\n"
        "    .mpg,.es = MPEG I frame\n"
        "    .png     = PNG\n"
        "    .gif     = GIF\n"
        "    .bmp     = BMP\n"
        "    default  = JPEG\n"
        );
}

int main(int argc, const char * const argv[])
{
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceMemory mem;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    BKNI_EventHandle checkpointEvent, packetSpaceAvailableEvent, displayedEvent;
    int i;
    NEXUS_Error rc;
    NEXUS_SurfaceClientHandle blit_client;
    NEXUS_SurfaceClientSettings client_settings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    int gfx_client_id = 1;
    int curarg = 1;
    NEXUS_PictureDecoderOpenSettings decoderSettings;
    NEXUS_PictureDecoderHandle pictureDecoder;
    unsigned timeout = 3000;
    
    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-client") && argc>curarg+1) {
            gfx_client_id = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else {
            break;
        }
        curarg++;
    }
    
    rc = NEXUS_Platform_AuthenticatedJoin(NULL);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }
    
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 720;
    createSettings.height = 480;
    /* createSettings.heap is NULL. proxy will populate. */
    surface = NEXUS_Surface_Create(&createSettings);
    NEXUS_Surface_GetMemory(surface, &mem);

    BKNI_CreateEvent(&checkpointEvent);
    BKNI_CreateEvent(&packetSpaceAvailableEvent);
    BKNI_CreateEvent(&displayedEvent);

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    NEXUS_Graphics2D_GetSettings(gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = complete;
    gfxSettings.checkpointCallback.context = checkpointEvent;
    gfxSettings.packetSpaceAvailable.callback = complete;
    gfxSettings.packetSpaceAvailable.context = packetSpaceAvailableEvent;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &gfxSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.rect.width = createSettings.width;
    fillSettings.rect.height = createSettings.height;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    BDBG_ASSERT(!rc);
    checkpoint(gfx, checkpointEvent);

    blit_client = NEXUS_SurfaceClient_Acquire(gfx_client_id);
    if (!blit_client) {
        BDBG_ERR(("NEXUS_SurfaceClient_Acquire %d failed. run client with '-client X' to change ids.", gfx_client_id));
        return -1;
    }

    NEXUS_SurfaceClient_GetSettings(blit_client, &client_settings);
    client_settings.displayed.callback = complete;
    client_settings.displayed.context = displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(blit_client, &client_settings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_SurfaceClient_SetSurface(blit_client, surface);
    BDBG_ASSERT(!rc);
    rc = BKNI_WaitForEvent(displayedEvent, 5000);
    BDBG_ASSERT(!rc);

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);

    NEXUS_PictureDecoder_GetDefaultOpenSettings(&decoderSettings);
    decoderSettings.multiScanBufferSize = 4*1024*1024; /* intentionally small because blit_server only gives a small heap */
    decoderSettings.bufferSize = 128*1024;
    pictureDecoder = NEXUS_PictureDecoder_Open(0, &decoderSettings);
    BDBG_ASSERT(pictureDecoder);
    
    if (curarg == argc) {
        BDBG_WRN(("no pictures to decode"));
        print_usage();
        return -1;
    }
    
    BDBG_WRN(("start"));
    for(i=curarg;i<argc;) {
        /* use SID to decode into surface */
        if (decode_picture(pictureDecoder, decoderSettings.bufferSize, surface, argv[i], gfx, checkpointEvent)) {
            continue;
        }

        /* tell server to blit */
        rc = NEXUS_SurfaceClient_UpdateSurface(blit_client, NULL);
        BDBG_ASSERT(!rc);
        /* for fastest update time, wait on the recycled event, not the displayed event. 
        for this app, we want the client to be paced by the display rate so that each picture appears on the display. */
        rc = BKNI_WaitForEvent(displayedEvent, 5000);
        BDBG_ASSERT(!rc);

        BKNI_Sleep(timeout);

        /* loop forever */
        if (++i==argc) i=curarg;
    }

    NEXUS_PictureDecoder_Close(pictureDecoder);
    NEXUS_SurfaceClient_Release(blit_client);
    BKNI_DestroyEvent(displayedEvent);
    BKNI_DestroyEvent(checkpointEvent);
    BKNI_DestroyEvent(packetSpaceAvailableEvent);
    NEXUS_Surface_Destroy(surface);
    NEXUS_Graphics2D_Close(gfx);
    NEXUS_Platform_Uninit();

    return 0;
}

/* code adapted from nexus/examples/graphics/picture_decoder.c */
static int decode_picture(NEXUS_PictureDecoderHandle pictureDecoder, unsigned bufferSize, NEXUS_SurfaceHandle destSurface, const char *pictureFilename, NEXUS_Graphics2DHandle gfx, BKNI_EventHandle checkpointEvent)
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

    BDBG_WRN(("decoding %s...", pictureFilename));
    fin = fopen(pictureFilename,"rb");
    if(!fin) {
        perror(pictureFilename);
        goto error;
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

    while (1) {
        /* this while(1) loop is temporarily necessary for a simple virtualization. if this function fails with NEXUS_NOT_AVAILABLE,
        the underlying resource is being used. When the SID FW is rewritten for multicontext support, this loop can be removed. */
        rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
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
            fprintf(stderr, "Picture file size %u is larger then buffer size %u, not supported for MPEG still frames\n", (unsigned)file_size, (unsigned)size);
            goto error;
        }
    }
    rc = NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
    BDBG_ASSERT(!rc);
    bytesRemain = pictureSettings.imageSize - size ;

    rc = NEXUS_PictureDecoder_Start(pictureDecoder, &pictureSettings);
    BDBG_ASSERT(!rc);

    cnt = 0;
    do {
        rc = NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
        BDBG_ASSERT(!rc);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed\n");
            goto error;
        } else if ( pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
            rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
            if ( size > bytesRemain ) size = bytesRemain;

            if ( rc == NEXUS_SUCCESS ) {
                rc = fread(buffer, 1, size, fin);                             /* read file into the decoder's buffer */
                if(rc) {
                    NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
                    bytesRemain -= size;
                }
                else {
                    BDBG_WRN(( "COULDN'T READ IN any more data !!! rc=%d bytesRemain=%d\n" , rc, bytesRemain  ));
                    cnt = 999;
                }
            }
        }
        usleep(1000);
        if (++cnt == 1000) { /* 1 second */
            BDBG_ERR(("unable to read header (state %d). aborting.", pictureStatus.state));
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

    if ( file_size > (unsigned)bufferSize ) {
        fprintf(stderr, "File requires multi part read, SID fw requires read from start\n");
        fseek(fin, 0, SEEK_SET);
        NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
        rc = fread(buffer, 1, size, fin);                               /* read file into the decoder's buffer */
        if(rc) {
            NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
        }
        else {
            fprintf(stderr, "failed to read in data for %s \n" , pictureFilename );
            goto error;
        }
    }

    /* start decoding */
    NEXUS_PictureDecoder_DecodeSegment(pictureDecoder, picture, NULL);
    do {
        NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed\n");
            goto error;
        } else if ( pictureStatus.state==NEXUS_PictureDecoderState_eMoreData ) {
            rc = NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
            if ( rc == NEXUS_SUCCESS ) {
                rc = fread(buffer, 1, size, fin);                             /* read file into the decoder's buffer */
                if ( size > bytesRemain ) size = bytesRemain;

                if(rc) {
                    NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, rc); /* commit data to the decoder */
                    bytesRemain -= size;
                }
                else {
                    fprintf( stderr, "rc=%d size=%d ..COULDN'T READ IN any more data !!! \n", rc, size );
                    goto error;
                }
            }
            else {
                BDBG_MSG(("failed to get buffer to read data into\n" ));
            }
        }
        usleep(1000);
    } while(pictureStatus.state!=NEXUS_PictureDecoderState_eSegmentDone);   /* wait for picture to decode */

    NEXUS_PictureDecoder_Stop(pictureDecoder);

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
    NEXUS_Graphics2D_Blit(gfx, &blitSettings);

    /* must checkpoint before destroying 'picture' surface */
    checkpoint(gfx, checkpointEvent);
    rc = 0;

error:
    fclose(fin);
    /* always call stop, even if we only called GetBuffer. this releases the internal resource */
    NEXUS_PictureDecoder_Stop(pictureDecoder);
    if (picture) {
        NEXUS_Surface_Destroy(picture);
    }
    return rc;
}

#else
#include <stdio.h>
int main(void)
{
    printf("ERROR: picture_decoder not supported\n");
    return -1;
}
#endif
