/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_SURFACE_TYPES_H__
#define NEXUS_SURFACE_TYPES_H__

#include "nexus_types.h"
#include "bm2mc_packet.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
Summary:
Information about the memory used by a surface.

Description:
Returned by NEXUS_Surface_GetMemory
*/
typedef struct NEXUS_SurfaceMemory
{
    void *buffer; /*  attr{memory=cached}  Pointer to cached memory. Directly accessible by the application.
                        This must be typecasted to the correct pixel format. For example
                        (unsigned short *) for a 16 bit pixel format or (unsigned long *) for a 32 bit pixel format. */
    unsigned int pitch; /* The memory width of the surface in bytes.
                            Pitch is based on the number of bytes used for all the pixels in one line along with an optional
                            padding on the right side. This padding maybe required for a variety of reasons (e.g. word alignment).
                            Pitch must be used to calculate the next line of the surface.
                            See the example in the NEXUS_Surface_GetMemory documentation. */

    NEXUS_Pixel *palette; /* attr{memory=cached}  This points to the palette memory for palette pixel formats. Otherwise NULL.
                             The palette pixel format is always ARGB8888.
                             The app can update the palette memory directly. After making a change, call NEXUS_Surface_Flush to
                             flush cached memory. If this surface is the framebuffer, you must also call NEXUS_Display_SetGraphicsFramebuffer
                             to read the updated palette into the graphics feeder. */
    unsigned numPaletteEntries; /* number of entries in palette */
    size_t  bufferSize;     /* size of the surface buffer in bytes */
} NEXUS_SurfaceMemory;

/*
Summary:
Status of a surface
*/
typedef struct NEXUS_SurfaceStatus {
    NEXUS_PixelFormat pixelFormat; /* pixel format of the surface. */
    NEXUS_PixelFormat palettePixelFormat; /* pixel format of NEXUS_SurfaceMemory.palette if pixelFormat is a palette format.
                                             Typically NEXUS_PixelFormat_eA8_R8_G8_B8 or NEXUS_PixelFormat_eA8_Y8_Cb8_Cr8 are supported. */
    uint16_t width;            /* visible width of the surface in pixels. */
    uint16_t height;           /* visible height of the surface in pixels. */
    unsigned int pitch; /* The memory width of the surface in bytes.
                            Pitch is based on the number of bytes used for all the pixels in one line along with an optional
                            padding on the right side. This padding maybe required for a variety of reasons (e.g. word alignment).
                            Pitch must be used to calculate the next line of the surface. See the example in the NEXUS_Surface_GetMemory documentation. */
    unsigned bufferSize;
    unsigned numPaletteEntries; /* number of entries in palette */
} NEXUS_SurfaceStatus;

/*
Summary:
Settings needed to create a surface.

Description:
These cannot be changed after the surface has been created.
For palletized surfaces, a default palette of all black will be created.
*/
typedef struct NEXUS_SurfaceCreateSettings
{
    NEXUS_PixelFormat pixelFormat; /* pixel format of the surface. */
    NEXUS_PixelFormat palettePixelFormat; /* pixel format of NEXUS_SurfaceMemory.palette if pixelFormat is a palette format.
                                             Typically NEXUS_PixelFormat_eA8_R8_G8_B8 or NEXUS_PixelFormat_eA8_Y8_Cb8_Cr8 are supported. */
    uint16_t width;            /* visible width of the surface in pixels. */
    uint16_t height;           /* visible height of the surface in pixels. */

    uint8_t alignment;        /* optional buffer alignment specified as a power of 2, measured in bytes.
                                  0 is no alignment (default), 1 is 2-byte aligned, 2 is 4-byte aligned, etc. */
    bool    managedAccess;     /* surface supports only managed access, and calls to legacy functions like NEXUS_Surface_GetMemory and NEXUS_Surface_InitPlane would fail */

    unsigned int pitch;        /* optional buffer pitch, measured in bytes. 0 is the default pitch (width * sizeof(pixel)). */

    void *pMemory;             /* attr{memory=cached} Device memory address to use for the surface. Must be allocated using NEXUS_Memory_Allocate().
                                  If NULL, Nexus will allocate.
                                  Size of user allocated buffer must be >= height * pitch, where pitch >= width * sizeof(pixel); otherwise there will be a memory overrun. */
    void *pPaletteMemory;      /* attr{memory=cached} Device memory address to use for the palette. Must be allocated using NEXUS_Memory_Allocate().
                                  If NULL, Nexus will allocate.
                                  Size of user allocated buffer must be >= sizeof(NEXUS_Pixel) * expected NEXUS_SurfaceMemory.numPaletteEntries; otherwise there will be a memory overrun.
                                  Palette memory must be 32 byte aligned. */

    NEXUS_MemoryBlockHandle pixelMemory; /* memory block that would be used for the surface buffer, pMemory and pixelMemory are mutually exclusive */
    unsigned pixelMemoryOffset;    /* offset from the start of the pixelMemory to the pixel buffer */
    NEXUS_MemoryBlockHandle paletteMemory; /* memory block that would be used for the palette buffer, pPaletteMemory and paletteMemory are  mutually exclusive */
    unsigned paletteMemoryOffset;    /* offset from the start of the paletteMemory to the palette buffer */
    unsigned mipLevel;         /*  mipmap level */
    NEXUS_HeapHandle heap;     /* Optional handle for memory heap. If NULL, the surface and palette will be allocated from the default main heap. */
} NEXUS_SurfaceCreateSettings;

/**
Summary:
Settings for a striped surface
**/
typedef struct NEXUS_StripedSurfaceCreateSettings {
    unsigned pitch;       /* in bytes */

    /* these come from BXVD_StillPictureBuffers */
    unsigned long   imageWidth; /* in pixels */
    unsigned long   imageHeight; /* in pixels */
    unsigned long   stripedWidth; /* in bytes, 128 or 256 */
    unsigned long   lumaStripedHeight; /* in pixels */
    unsigned long   chromaStripedHeight; /* in pixels */
    NEXUS_VideoBufferType bufferType; /* could be set to eFieldPair only if the picture is truly progressive */
    NEXUS_PixelFormat lumaPixelFormat; /* Y8 or Y10. */
    NEXUS_PixelFormat chromaPixelFormat; /* Cb8_Cr8, Cr8_Cb8, Cb10_Cr10, or Cr10_Cb10 */
    NEXUS_MemoryBlockHandle lumaBuffer; /* top field or frame */
    unsigned lumaBufferOffset;    /* offset from the start of the lumaBuffer to the luma data */
    NEXUS_MemoryBlockHandle chromaBuffer;  /* top field or frame */
    unsigned chromaBufferOffset;    /* offset from the start of the chromaBuffer to the chroma data */
    NEXUS_MemoryBlockHandle bottomFieldLumaBuffer; /* ignored if bufferType is NOT eFieldPair */
    unsigned bottomFieldLumaBufferOffset;    /* offset from the start of the bottomFieldLumaBuffer to the luma data */
    NEXUS_MemoryBlockHandle bottomFieldChromaBuffer; /* ignored if bufferType is NOT eFieldPair */
    unsigned bottomFieldChromaBufferOffset;    /* offset from the start of the bottomFieldChromaBuffer to the chroma data */

    /* The following are used in NEXUS_StripedSurface_Create along with imageWidth and imageHeight; all other fields should be default values. */
    NEXUS_HeapHandle lumaHeap; /* default zero; It's mutual exclusive with lumaBuffer! Only one can be set at create call! */
    NEXUS_HeapHandle chromaHeap; /* default zero; It's mutual exclusive with chromaBuffer! Only one can be set at create call! */

    NEXUS_MatrixCoefficients matrixCoefficients; /* video color space standard, defaults to eItu_R_BT_709 */
} NEXUS_StripedSurfaceCreateSettings;

#ifdef __cplusplus
}
#endif

#endif
