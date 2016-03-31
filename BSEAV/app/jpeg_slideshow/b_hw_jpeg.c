/***************************************************************************
 *     Copyright (c) 2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *    Reference JPEG Decode into YUYV or RGB Color spaces with slideshow
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "b_decode_jpeg.h"
#include "nexus_picture_decoder.h"
#include <stdio.h>
#include <errno.h>

BDBG_MODULE(b_hw_jpeg);

NEXUS_PictureDecoderHandle g_pictureDecoder = NULL;

NEXUS_SurfaceHandle b_decompress_jpeg( const char *filename, unsigned int maxWidth, unsigned int maxHeight )
{
    FILE *fin;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_PictureDecoderStartSettings pictureSettings;
    NEXUS_PictureDecoderStatus pictureStatus;
    void *buffer;
    size_t size;
    int rc;

    BSTD_UNUSED(maxWidth);
    BSTD_UNUSED(maxHeight);

    if (!g_pictureDecoder) {
        NEXUS_PictureDecoderOpenSettings decoderSettings;
        NEXUS_PictureDecoder_GetDefaultOpenSettings(&decoderSettings);
        decoderSettings.multiScanBufferSize = 16*1024*1024;
        decoderSettings.bufferSize = 9*1024*1024;
        g_pictureDecoder = NEXUS_PictureDecoder_Open(0, &decoderSettings);
    }

    NEXUS_PictureDecoder_GetBuffer(g_pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */

    fin = fopen(filename, "rb");
    if (!fin) {
        BDBG_ERR(("Unable to open %s", filename));
        return NULL;
    }
    rc = fread(buffer, 1, size, fin); /* read file into the decoder's buffer */
    if(rc<0) {
        BDBG_ERR(("fread error %d", errno));
        return NULL;
    } else if(rc==(int)size) {
        fseek(fin, 0, SEEK_END);
        if((unsigned)ftell(fin)>size) {
            /* to decode larger file, requires to allocate larger buffer, see NEXUS_PictureDecoderOpenSettings bufferSize */
            BDBG_ERR(("JPEG file size %u is larger then buffer size %u, not supported", (unsigned)ftell(fin), (unsigned)size));
            return NULL;
        }
    }
    NEXUS_PictureDecoder_ReadComplete(g_pictureDecoder, 0, rc); /* commit data to the decoder */

    NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
    pictureSettings.format = NEXUS_PictureFormat_eJpeg;
    NEXUS_PictureDecoder_Start(g_pictureDecoder, &pictureSettings);

    do {
        NEXUS_PictureDecoder_GetStatus(g_pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            BDBG_ERR(("decoding failed"));
            return NULL;
        }
        BKNI_Sleep(1);
    } while (!pictureStatus.headerValid); /* wait for picture dimensions */

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.pixelFormat = pictureStatus.header.format;
    surfaceCreateSettings.width = pictureStatus.header.surface.width;
    surfaceCreateSettings.height = pictureStatus.header.surface.height;
    surface = NEXUS_Surface_Create(&surfaceCreateSettings);

    /* start decoding */
    NEXUS_PictureDecoder_DecodeSegment(g_pictureDecoder, surface, NULL);
    do {
        NEXUS_PictureDecoder_GetStatus(g_pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            BDBG_ERR(("decoding failed"));
            return NULL;
        }
        BKNI_Sleep(1);
    } while(pictureStatus.state!=NEXUS_PictureDecoderState_eSegmentDone); /* wait for picture to decode */

    NEXUS_PictureDecoder_Stop(g_pictureDecoder);

    fclose(fin);

    /* don't close g_pictureDecoder. need to rework api for instance management. */

    return surface;
}
