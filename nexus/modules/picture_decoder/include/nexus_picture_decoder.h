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
#ifndef NEXUS_PICTURE_DECODER_H__
#define NEXUS_PICTURE_DECODER_H__

#include "nexus_types.h"
#include "nexus_surface.h"

/*=**************************
The Picture decoder provides a method for decoding a single picture
into memory for use as a graphics surface.

This module performs a different function from the StillDecoder. The Nexus StillDecoder is an extension
of the VideoDecoder. The StillDecoder receives MPEG/AVC/VC1 I-frames from transport and decodes them with the
VideoDecoder hardware. Therefore it is used for PVR still frames. The PictureDecoder is used for non-PVR images.
****************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for Picture Decoder
**/
typedef struct NEXUS_PictureDecoder *NEXUS_PictureDecoderHandle;

/******************************************************************************
Summary:
Image format supported by Picture Decoder
******************************************************************************/
typedef enum NEXUS_PictureFormat {
    NEXUS_PictureFormat_eRleDvd,
    NEXUS_PictureFormat_eRleBluRay,
    NEXUS_PictureFormat_eJpeg, /* - JPEG: baseline, 8 bits per pixel, sequential and progressive:
                                         o   Greyscale and YUV 4:4:4, 4:2:2, 4:2:0.
                                         o   Image width: up to 8K. */
    NEXUS_PictureFormat_ePng, /* - PNG: version 1.2
                                            o   All color types & bit depths (16-bit data is truncated to 8-bit).
                                            o   Greyscale (1,2,4,8,16 bpp), Pseudo-color (1,2,4,8 bpp), RGB,
                                                RGB + Alpha.
                                            o   Deflate version 1.3.
                                            o   Sequential and interlaced support (re-interlacing done by the ARC).
                                            o   Images up to 8K pixels wide (8 bpp or less), 2K wide (RGBA), and
                                                1K for 16-bit. */
    NEXUS_PictureFormat_eGif, /* - GIF: basic pixel + run encoding
                                            o   Support for 2 bits/pixel (DVD), 8 bits/pixel (DVD and Blu-Ray). */
    NEXUS_PictureFormat_eMpeg,
    NEXUS_PictureFormat_eMax
} NEXUS_PictureFormat;

/**
Summary:
Settings used to open a picture decoder.

Description:
Max capabilities must be specified at open time so that memory can be allocated then.
**/
typedef struct NEXUS_PictureDecoderOpenSettings
{
    struct {
        uint16_t maxWidth;
        uint8_t chromaVerticalShift; /* vertical upsampling shift in 1/4 pels */
        uint8_t chromaHorizontalShift; /* horizontal upsampling shift in 1/4 pels */
    } jpeg;
    struct {
        uint16_t maxWidthRgb;
        uint16_t maxWidthPalette;
        uint16_t maxWidth16Bpp;
    } png;
    struct {
        uint16_t maxWidth;
    } gif;
    struct {
        uint16_t maxWidthDvd;
        uint16_t maxWidthBluRay;
    } rle;
    unsigned bufferSize; /* size of the compressed buffer */
    unsigned multiScanBufferSize; /* size of the decoded picture buffer that is required for multi scan decoding */
    NEXUS_HeapHandle heap;     /* Optional handle for memory heap. If NULL, heap[0] will be used for decoder allocations. */
    void *bufferPtr; /* attr{memory=cached} Optional user-supplied memory buffer which is used for the compressed buffer */
    void *multiScanBufferPtr; /* attr{memory=cached} Optional used-supplied memory buffer which is used for multi scan decoding */
} NEXUS_PictureDecoderOpenSettings;

/**
Summary:
Get default settings for use in NEXUS_PictureDecoder_Open.
**/
void NEXUS_PictureDecoder_GetDefaultOpenSettings(
    NEXUS_PictureDecoderOpenSettings *pSettings /* [out] */
    );

/**
Summary:
Open an interface to decode pictures.
**/
NEXUS_PictureDecoderHandle NEXUS_PictureDecoder_Open(  /* attr{destructor=NEXUS_PictureDecoder_Close}  */
    unsigned index,
    const NEXUS_PictureDecoderOpenSettings *pSettings
    );

/**
Summary:
Close the picture decoder.
**/
void NEXUS_PictureDecoder_Close(
    NEXUS_PictureDecoderHandle handle
    );



/**
Summary:
Settings used to decode a picture.

Description:
The application is responsible for handling timeouts. If the image cannot be decoded, either because of errors or because
the image is incomplete, the application may receive no callback. The application must program its own timeout
when handling the Nexus callbacks.
**/
typedef struct NEXUS_PictureDecoderStartSettings
{
    NEXUS_PictureFormat format;
    NEXUS_CallbackDesc pictureParsed; /* Callback called when picture has been parsed and picture header information is available with NEXUS_PictureDecoder_GetStatus */
    NEXUS_CallbackDesc segmentDecoded; /* Callback called when picture segment has been decoded and data available in the surface */
    unsigned imageSize;                /* when using a buffer smaller than jpeg image file size, need to pass in image size */
} NEXUS_PictureDecoderStartSettings;

/**
Summary:
Get default settings for use in NEXUS_PictureDecoder_Start.
**/
void NEXUS_PictureDecoder_GetDefaultStartSettings(
    NEXUS_PictureDecoderStartSettings *pSettings /* [out] */
    );

/**
Summary:
Start decoding a picture

Description:
You can feed data to the decoder using GetBuffer/ReadComplete before or after Start.

After calling Start, you can wait for the pictureParsed callback or poll NEXUS_PictureDecoderStatus.headerValid.
After headerValid is true, you should create a surface (NEXUS_Surface_Create) with at least NEXUS_PictureDecoderStatus.header.surface.width and height suitable for segment decode.
NEXUS_PictureDecoderStatus.header.surface.height is height of surface suitable for full picture decode.

After you create the surface, call NEXUS_PictureDecoder_DecodeSegment to decode the picture into your destination surface.

Call NEXUS_PictureDecoder_Stop when you are done decoding the picture.
**/
NEXUS_Error NEXUS_PictureDecoder_Start(
    NEXUS_PictureDecoderHandle handle,
    const NEXUS_PictureDecoderStartSettings *pSettings
    );

/**
Summary:
Decode a single picture segment.

Description:
You must create a surface with the width and height returned in NEXUS_PictureDecoderStatus.header.

If you decode the picture in multiple segments, the segments must be done in order, from top to bottom,
with each segment width being the full width of the picture.
**/
NEXUS_Error NEXUS_PictureDecoder_DecodeSegment(
    NEXUS_PictureDecoderHandle handle,
    NEXUS_SurfaceHandle destination, /* surface that is used as destination */
    const NEXUS_Rect *pSegment  /* attr{null_allowed=y} NULL is used to decode entire picture. This width and height are applied to the source, not the destination. */
    );

/**
Summary:
Stop decoding a picture

Description:
You must call Stop before Starting another picture decode.
**/
void NEXUS_PictureDecoder_Stop(
    NEXUS_PictureDecoderHandle handle
    );

/*
Summary:
Get a pointer and size for the next location in the buffer which can accept data for the decoder.

Description:
NEXUS_PictureDecoder_GetBuffer is non-destructive. You can safely call it multiple times.

NEXUS_PictureDecoder_GetBuffer can be limited by two things: space in the ring buffer or a wrap around condition.
*/
NEXUS_Error NEXUS_PictureDecoder_GetBuffer(
    NEXUS_PictureDecoderHandle decoder,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for picture data */
    size_t *pSize /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    );

/**
Summary:
Notify PictureDecoder of how much data was added into the buffer.

Description:
You can only call NEXUS_PictureDecoder_ReadComplete once after a NEXUS_PictureDecoder_GetBuffer call.
After calling it, you must call NEXUS_PictureDecoder_GetBuffer before adding more data.

The skip parameter allows an application to copy data into the playpump buffer with
whatever alignment it desired. This is needed for "direct I/O" from disk drives which requires
all memory access to be page aligned (e.g. 4K).
**/
NEXUS_Error NEXUS_PictureDecoder_ReadComplete(
    NEXUS_PictureDecoderHandle decoder,
    size_t skip,            /* skip is the number of bytes at the beginning of the current buffer pointer
                               which PictureDecoder should skip over. */
    size_t amountUsed       /* amountUsed is the number of bytes, following any skip bytes,
                               which PictureDecoder should feed into decoder. */
    );


/**
Summary:
State of the picture decoder returned by NEXUS_PictureDecoderStatus
**/
typedef enum NEXUS_PictureDecoderState {
    NEXUS_PictureDecoderState_eIdle,
    NEXUS_PictureDecoderState_eDecoding,
    NEXUS_PictureDecoderState_eSegmentDone,
    NEXUS_PictureDecoderState_eError,
    NEXUS_PictureDecoderState_eMoreData,
    NEXUS_PictureDecoderState_eMax
} NEXUS_PictureDecoderState;

/**
Summary:
Palette of decoded picture returned by NEXUS_PictureDecoder_GetPalette
**/
typedef struct NEXUS_PictureDecoderPalette
{
    unsigned numPaletteEntries; /* number of valid entries in palette */
    NEXUS_Pixel palette[256];
} NEXUS_PictureDecoderPalette;

/**
Summary:
Status of picture decode
**/
typedef struct NEXUS_PictureDecoderStatus
{
    NEXUS_PictureDecoderState state;
    bool headerValid;
    struct {
        uint16_t width;  /* picture width  */
        uint16_t height; /* picture height */
        NEXUS_Rect surface; /* rectangle that would be sufficient to decode entire image.
                               for block based images (e.g. JPEG) width and height of rectangle could be higher than that of the picture. */
        NEXUS_PixelFormat format;  /* pixel format of the picture */
        bool multiscan;  /* true for interlaced pictures. interlaced pictures may require that all segments be
                            decoded before getting a complete picture. */
        bool grayscale;
        bool palette; /* true if image is palettized, NEXUS_PictureDecoder_GetPalette is used to obtain palette */
        bool rotated; /* image encoded as bottom-up and requires multiscan buffer for a post-processing */
        bool hasTransparentData;
        unsigned transparentColorIndex; /* if hasTransparentData==true, indicates the entry in the CLUT that that shall be made transparent. only applies to GIF. */
        uint32_t transparentColorRgb; /* if hasTransparentData==true, indicates the explicit color (RGB888) that shall be made transparent. */
    } header;
    uint32_t crc; /* CRC for completed segment. valid when state == NEXUS_PictureDecoderState_eSegmentDone. */
    struct {
        unsigned firmware;
    } version;
} NEXUS_PictureDecoderStatus;

/**
Summary:
Get status
**/
NEXUS_Error NEXUS_PictureDecoder_GetStatus(
    NEXUS_PictureDecoderHandle decoder,
    NEXUS_PictureDecoderStatus *pStatus /* [out] */
    );

/**
Summary:
Get palette
**/
NEXUS_Error NEXUS_PictureDecoder_GetPalette(
    NEXUS_PictureDecoderHandle decoder,
    NEXUS_PictureDecoderPalette *pPalette /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_PICTURE_DECODER_H__ */

