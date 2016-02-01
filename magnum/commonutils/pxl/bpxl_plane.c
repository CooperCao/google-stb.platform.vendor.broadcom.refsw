/***************************************************************************
 *     Copyright (c) 2013, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bpxl_plane.h"
#include "bkni.h"
#include "bchp_m2mc.h"     /* for gfx compression mode */

BDBG_MODULE(BPXL_Plane);

/***************************************************************************/
void BPXL_Plane_Init(BPXL_Plane *plane, size_t width, size_t height, BPXL_Format format)
{
    uint32_t ulHeight = height, ulPitch = 0, ulAlignment = 4;

    /* check if format is YCbCr 422 10-bit */
    if( BPXL_IS_YCbCr422_10BIT_FORMAT(format) )
    {
        /* align size to 6 pixels (per 4 dwords) */
        ulPitch = ((width + 5) / 6) * 16;
    }
    /* check if format is YCbCr 422 10-bit packed format (eg. BPXL_eCr10_Y110_Cb10_Y010)*/
    else if( BPXL_IS_YCbCr422_10BIT_PACKED_FORMAT(format) )
    {
        /* align size to 8 pixels (per 5 dwords) */
        ulPitch = ((width + 7) / 8) * 20;
    }
    /* check if format is YCbCr 420 Luma 10-bit (non-packed) format (BPXL_eY10)*/
    else if (BPXL_IS_YCbCr420_LUMA_10BIT_FORMAT(format))
    {
        /* align size to 6 pixels (per 2 dwords) */
        ulPitch = ((width + 5) / 6) * 8;
        ulHeight = (height + 1) & (~0x1);
    }
    /* check if format is YCbCr 420 Chroma 10-bit (non-packed) format (BPXL_eCb10_Cr10)*/
    else if (BPXL_IS_YCbCr420_CHROMA_10BIT_FORMAT(format))
    {
        /* align size to 3 pixels (per 2 dwords), width passed in is already half of luma plane */
        ulPitch = ((width + 2) / 3) * 8;
    }
    /* check if format is BPXL_eCompressed_A8_R8_G8_B8 */
    else if (BPXL_IS_COMPRESSED_FORMAT(format))
    {
#if defined(BCHP_M2MC_DCEG_CFG)
        ulPitch = ((((((width + 3) / 4) * 66) + 7) / 8) + 3) & (~0x3); /* align to 32-bits dwords */
#elif defined(BCHP_M2MC_BSTC_COMPRESS_CONTROL)
        ulPitch = ((width + 3) / 4) * 8;
        ulHeight = (ulHeight + 3) & ~0x3;
        ulAlignment = 32;
#else
        ulPitch = width * 4; /* non-compressed ARGB8888 */
#endif
    }
    else if( format != BPXL_INVALID )
    {
        unsigned int uiBitsPerPixel = BPXL_BITS_PER_PIXEL(format);

        /* calculate bytes for sub-byte formats */
        if( uiBitsPerPixel < 8 )
        {
            width += 8 / uiBitsPerPixel - 1;
            ulPitch = width / (8 / uiBitsPerPixel);
        }
        /* calculate bytes for formats that are a byte or larger */
        else
        {
            /* align size of YCbCr 422 and YCbCr 420 formats to 2 pixels */
            if( BPXL_IS_YCbCr422_FORMAT(format) )
                width = (width + 1) & (~1);
            /* for YCbCr420 chroma plane the width/height passed in is already half of luma plane */
            else if( BPXL_IS_YCbCr420_LUMA_FORMAT(format) )
            {
                width = (width + 1) & ~0x1;
                ulHeight = (ulHeight + 1) & ~0x1;
            }
            ulPitch = (width * uiBitsPerPixel) / 8;
        }
    }

    BKNI_Memset(plane, 0, sizeof(*plane));
    plane->ulWidth = width;
    plane->ulHeight = height;
    plane->ulPitch = ulPitch;
    plane->ulAlignment = ulAlignment;
    plane->ulBufSize = (BPXL_IS_YCbCr420_CHROMA_FORMAT(format) || BPXL_IS_YCbCr420_CHROMA_10BIT_FORMAT(format))?
        ulPitch * ulHeight / 2 : ulPitch * ulHeight;
    plane->eFormat = format;

    if(BPXL_IS_PALETTE_FORMAT(format)) {
        plane->ulNumPaletteEntries = BPXL_NUM_PALETTE_ENTRIES(format);
        plane->ePalettePixelFormat = BPXL_eA8_R8_G8_B8;
    }
    return;
}

BERR_Code BPXL_Plane_AllocateBuffers(BPXL_Plane *pPlane, BMMA_Heap_Handle hHeap)
{
    if(pPlane->ulPitch==0) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    pPlane->ulPixelsOffset = 0;
    pPlane->ulPaletteOffset = 0;
    pPlane->hPalette = NULL;
    pPlane->hPixels = BMMA_Alloc(hHeap, pPlane->ulPitch * pPlane->ulHeight, pPlane->ulAlignment, NULL);
    if(!pPlane->hPixels) {
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    if(pPlane->ulNumPaletteEntries) {
        pPlane->hPalette = BMMA_Alloc(hHeap, 1024, 1<<5, NULL);
        if(!pPlane->hPalette) {
            BMMA_Free(pPlane->hPixels);
            pPlane->hPixels = NULL;
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }
    }
    return BERR_SUCCESS;
}
