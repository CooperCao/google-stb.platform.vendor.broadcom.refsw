/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
    unsigned int uiWidth, uiHeight, uiPitch;

    BDBG_ASSERT(format !=BPXL_eUIF_R8_G8_B8_A8);

    BPXL_GetAlignment_isrsafe(format, width, height, &uiWidth, &uiHeight, &uiPitch);

    BKNI_Memset(plane, 0, sizeof(*plane));
    plane->ulWidth = uiWidth;
    plane->ulHeight = uiHeight;
    plane->ulPitch = uiPitch;
    plane->ulAlignment = BPXL_IS_YCbCr422_FORMAT(format)?
        BPXL_P_PITCH_YCBCR422_ALIGNMENT : BPXL_P_PITCH_ALIGNMENT;
    plane->ulBufSize = uiPitch * uiHeight;
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
