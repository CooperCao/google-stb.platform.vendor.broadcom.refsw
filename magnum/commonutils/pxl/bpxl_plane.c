/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#include "bstd.h"
#include "bpxl_plane.h"
#include "bpxl_uif.h"
#include "bkni.h"
#include "bchp_common.h"
#ifdef BCHP_M2MC_REG_START
#include "bchp_m2mc.h"     /* for gfx compression mode */
#endif
#include "bpxl_priv.h"

#include "bchp_memc_ddr_0.h"

BDBG_MODULE(BPXL_Plane);

/***************************************************************************/
void BPXL_Plane_Init(BPXL_Plane *plane, size_t width, size_t height, BPXL_Format format)
{
    uint32_t ulHeight = height, ulPitch = 0;
    /* SWSTB-8271 maximize the alignment to the largest requirement so far*/
    unsigned int ulAlignment = BPXL_IS_YCbCr422_FORMAT(format)?
        BPXL_P_PITCH_YCBCR422_ALIGNMENT:BPXL_P_PITCH_ALIGNMENT;

    BDBG_ASSERT(format !=BPXL_eUIF_R8_G8_B8_A8);


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
            unsigned int uiPixPerByte = (8 / uiBitsPerPixel);
            ulPitch = (width + (uiPixPerByte - 1)) / uiPixPerByte;
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
            ulPitch = (width * uiBitsPerPixel + 7) / 8;
        }
    }

    ulPitch = BPXL_P_ALIGN_UP(ulPitch, ulAlignment);
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

/***************************************************************************/
BERR_Code BPXL_Plane_Uif_Init(
        BPXL_Plane *plane,
        size_t width,
        size_t height,
        unsigned mipLevel,
        BPXL_Format format,
        BCHP_Handle hChip)
{
    BCHP_MemoryInfo stMemInfo;
    BPXL_Uif_Memory_Info stUifMemInfo;
    BPXL_Uif_Surface     stUifSurface;
    uint32_t ulBank=4;

    BDBG_ASSERT(format ==BPXL_eUIF_R8_G8_B8_A8);

    if((width < BPXL_UIF_MINSURFACE) ||(height < BPXL_UIF_MINSURFACE))
         return BERR_TRACE(BERR_INVALID_PARAMETER);

    /* Check the number of miplevels is valid for the base image size */
    if ((width >> mipLevel) == 0 || (height >> mipLevel) == 0)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BCHP_GetMemoryInfo(hChip, &stMemInfo);

#if BCHP_MEMC_DDR_0_CNTRLR_CONFIG_2_BANK_BITS_DEFAULT
    switch(BCHP_MEMC_DDR_0_CNTRLR_CONFIG_2_BANK_BITS_DEFAULT)
    {
        case 3:
            ulBank = 8;
            break;
        case 4:
            ulBank = 16;
            break;
        case 5:
            ulBank = 32;
            break;
        case 2:
        default:
            ulBank = 4;
            break;
    }
#endif

    stUifMemInfo.ulPageSize = stMemInfo.memc[0].ulPageSize;
    stUifMemInfo.ulPageinUBRows = stMemInfo.memc[0].ulPageSize/(BPXL_UIF_BLOCKSIZE * BPXL_UIF_COL_WIDTH_IN_BLOCK);
    stUifMemInfo.ul15PageinUBRows = stUifMemInfo.ulPageinUBRows*3/2;    /*1.5  UIFBlockRows */
    /* How many UIF-block rows the "page cache" covers */
    stUifMemInfo.ulPcInUBRows = ulBank*stUifMemInfo.ulPageinUBRows;
    stUifMemInfo.ulPc15InUBRows = stUifMemInfo.ulPcInUBRows - stUifMemInfo.ul15PageinUBRows;

    stUifSurface.ulWidth = width;
    stUifSurface.ulHeight = height;
    stUifSurface.ulMipLevel = mipLevel;
    BPXL_Uif_SurfaceCfg(&stUifMemInfo, &stUifSurface);

    BKNI_Memset(plane, 0, sizeof(*plane));
    plane->ulWidth = width;
    plane->ulHeight = height;
    plane->ulPitch = stUifSurface.ulL0Pitch;
    plane->ulAlignment = stUifSurface.ulAlign;
    plane->ulBufSize = stUifSurface.ulSize;
    plane->eFormat = format;
    plane->ulMipLevel = mipLevel;
    return BERR_SUCCESS;
}


BERR_Code BPXL_Plane_AllocateBuffers(BPXL_Plane *pPlane, BMMA_Heap_Handle hHeap)
{
    if(pPlane->ulPitch==0) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    pPlane->ulPixelsOffset = 0;
    pPlane->ulPaletteOffset = 0;
    pPlane->hPalette = NULL;
    pPlane->hPixels = BMMA_Alloc(hHeap,
        (pPlane->eFormat==BPXL_eUIF_R8_G8_B8_A8)?pPlane->ulBufSize:(pPlane->ulPitch * pPlane->ulHeight),
        pPlane->ulAlignment, NULL);
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
