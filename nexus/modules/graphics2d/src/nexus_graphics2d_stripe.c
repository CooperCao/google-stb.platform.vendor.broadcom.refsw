/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/
#include "nexus_graphics2d_module.h"
#include "nexus_graphics2d_impl.h"
#include "priv/nexus_surface_priv.h"

BDBG_MODULE(nexus_graphics2d_stripe);
BDBG_FILE_MODULE(pxlval);

#define MB_HEIGHT     16
/* Combine 4 8-bit value to 32-bit value. c0, c1, c2, c3 are 8-bit value */
/* NOTE: MFD striped format is scanned out in big endian */
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
    #define BTST_MAKE_32BIT(c0, c1, c2, c3)          \
    ( (((c0) & 0xFF) << 24) | (((c1) & 0xFF) << 16)  | \
      (((c2) & 0xFF) <<  8) | (((c3) & 0xFF) << 0) )
#else
    #define BTST_MAKE_32BIT(c0, c1, c2, c3)          \
    ( (((c3) & 0xFF) << 24) | (((c2) & 0xFF) << 16)  | \
      (((c1) & 0xFF) <<  8) | (((c0) & 0xFF) << 0) )
#endif

void NEXUS_Graphics2D_GetDefaultStripeBlitSettings(
    NEXUS_Graphics2DStripeBlitSettings *pSettings /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_Graphics2D_StripeBlit(
     NEXUS_Graphics2DHandle gfx,
     const NEXUS_Graphics2DStripeBlitSettings *pSettings
     )
{
    bool interlaced;
    unsigned xi, yi, surfaceOffset;
    BSTD_DeviceOffset offsetY=0, offsetC=0;
    unsigned stripeNum;
    unsigned lumaBufSize, chromaBufSize, totalByteWidth;
    unsigned luma32bit, chroma32bit;
    void *ptr;
    uint8_t *pvLumaStartAddress, *pvChromaStartAddress;
    uint8_t *pvSurface;
    NEXUS_SurfaceMemory surfaceMem;
    NEXUS_SurfaceCreateSettings surfaceSettings;
    NEXUS_StripedSurfaceCreateSettings picCfg;

    BSTD_UNUSED(gfx);
    if ( NULL == pSettings || NULL == pSettings->source.surface || NULL == pSettings->output.stripedSurface ) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    NEXUS_StripedSurface_GetCreateSettings(pSettings->output.stripedSurface, &picCfg);
    if (picCfg.lumaBuffer==NULL || picCfg.chromaBuffer==NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    NEXUS_Surface_GetCreateSettings(pSettings->source.surface, &surfaceSettings);
    if(!NEXUS_PIXEL_FORMAT_IS_422(surfaceSettings.pixelFormat) ||
       (surfaceSettings.width != picCfg.imageWidth) ||
       (surfaceSettings.height != picCfg.imageHeight)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* NOTE: current solution requires cpu access to surface, so must protect cpu
       accessible address to avoid crash; */
    NEXUS_Surface_GetMemory(pSettings->source.surface, &surfaceMem);
    if(!NEXUS_P_CpuAccessibleAddress(surfaceMem.buffer)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* pin down striped surface memory */
    if( NEXUS_SUCCESS != NEXUS_MemoryBlock_Lock(picCfg.lumaBuffer, &ptr) ||
        !NEXUS_P_CpuAccessibleAddress(ptr) ) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    pvLumaStartAddress = ptr;
    pvLumaStartAddress += picCfg.lumaBufferOffset;

    if( NEXUS_SUCCESS != NEXUS_MemoryBlock_Lock(picCfg.chromaBuffer, &ptr) ||
        !NEXUS_P_CpuAccessibleAddress(ptr) ) {
        NEXUS_MemoryBlock_Unlock(picCfg.lumaBuffer);
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    pvChromaStartAddress = ptr;
    pvChromaStartAddress += picCfg.chromaBufferOffset;

    interlaced = (picCfg.bufferType == NEXUS_VideoBufferType_eTopField) || (picCfg.bufferType == NEXUS_VideoBufferType_eBotField);
    BDBG_MSG(("stripe %lux%lu%c to [%p, %p]", picCfg.imageWidth, picCfg.imageHeight, interlaced?'i':'p',
        pvLumaStartAddress, pvChromaStartAddress));

    /* for bottom field, we must apply an offset to the start address */
    if (picCfg.bufferType == NEXUS_VideoBufferType_eBotField) {
        pvLumaStartAddress   += picCfg.stripedWidth;
        pvChromaStartAddress += picCfg.stripedWidth;
    }

    /* get the source surface address */
    pvSurface = surfaceMem.buffer;

    /* flush the offscreen surface filled by GRC hw before host access */
    NEXUS_Surface_Flush(pSettings->source.surface);
    BDBG_MODULE_MSG(pxlval, ("%p(0, 0): pxl = %#x", (void*)((unsigned*)surfaceMem.buffer), *(unsigned *)surfaceMem.buffer));
    BDBG_MODULE_MSG(pxlval, ("%p(4, 0): pxl = %#x", (void*)((unsigned*)surfaceMem.buffer + 1), *((unsigned *)surfaceMem.buffer + 1)));

    /* get the stripe surface info */
    totalByteWidth = picCfg.stripedWidth * (picCfg.imageWidth + (picCfg.stripedWidth - 1)) / picCfg.stripedWidth;
    lumaBufSize   = totalByteWidth * picCfg.lumaStripedHeight;
    chromaBufSize = totalByteWidth * picCfg.chromaStripedHeight;

    /* Traverse through each line in the frame */
    BDBG_MSG(("to stripe the image: %ux%u...", (unsigned)picCfg.imageWidth, (unsigned)picCfg.imageHeight));
    for( yi = 0; yi < picCfg.imageHeight; yi++ )
    {
        /* Traverse through each 4 pixels in the line */
        for ( xi = 0; xi < picCfg.imageWidth; xi += 4)
        {
            surfaceOffset = surfaceMem.pitch * yi + xi*2;/* 422 source surface */

            /* determine which stripe contains this pixel */
            stripeNum = xi / picCfg.stripedWidth;

            /* calculate the pixel's luma byte offset */
            offsetY = (xi % picCfg.stripedWidth) +
                (yi * picCfg.stripedWidth) +
                (stripeNum * picCfg.stripedWidth * picCfg.lumaStripedHeight);


            /* store luma byte */
            BDBG_ASSERT( offsetY < lumaBufSize);

            /* Note: the stripe surface is always aligned in 1K pages, so the address shuffling
               can be applied only to the surface offset address. */
            offsetY = BCHP_ShuffleStripedPixelOffset(g_NEXUS_pCoreHandles->chp, NEXUS_Heap_GetMemcIndex_isrsafe(picCfg.lumaHeap), offsetY);

            /* Y0Cr0Y1Cb0Y2Cr2Y3Cb2 -> Y0Y1Y2Y3 and Cr0Cb0Cr2Cb2 */
            luma32bit = BTST_MAKE_32BIT(pvSurface[surfaceOffset], pvSurface[surfaceOffset+2],
                pvSurface[surfaceOffset+4], pvSurface[surfaceOffset+6]);
            chroma32bit = BTST_MAKE_32BIT(pvSurface[surfaceOffset+1], pvSurface[surfaceOffset+3],
                pvSurface[surfaceOffset+5], pvSurface[surfaceOffset+7]);

            BDBG_MODULE_MSG(pxlval, ("(%u, %u): Y = %#x, C = %#x", xi, yi, luma32bit, chroma32bit));
            BDBG_MODULE_MSG(pxlval, ("src[%x]: %x %x %x %x, %x %x %x %x", surfaceOffset, pvSurface[surfaceOffset], pvSurface[surfaceOffset+2],
                pvSurface[surfaceOffset+4], pvSurface[surfaceOffset+6], pvSurface[surfaceOffset+1], pvSurface[surfaceOffset+3],
                pvSurface[surfaceOffset+5], pvSurface[surfaceOffset+7]));
            BDBG_MODULE_MSG(pxlval, ("dst["BDBG_UINT64_FMT"]", BDBG_UINT64_ARG(offsetY)));

            /* Write luma */
            /* NOTE: mpeg feeder is in big-endian if it's mpeg format data!!! assume cpu is little endian */
            *(uint32_t*)(pvLumaStartAddress + offsetY) = luma32bit;

            /* store 4:2:0 chroma (point sample for now); TODO: add better 422->420 chroma filter */
            if((yi & 1) == 0)
            {
                /* calculate pixel's Cr byte offset */
                offsetC = (xi % picCfg.stripedWidth) +
                    (yi/2) * picCfg.stripedWidth +
                    (stripeNum * picCfg.stripedWidth * picCfg.chromaStripedHeight);

                /* store chroma bytes */
                BDBG_ASSERT( offsetC < chromaBufSize);

                /* NOTE: mpeg feeder is in big-endian if it's mpeg format data!!! */
                offsetC = BCHP_ShuffleStripedPixelOffset(g_NEXUS_pCoreHandles->chp, NEXUS_Heap_GetMemcIndex_isrsafe(picCfg.chromaHeap), offsetC);

                /* Write chroma */
                *(uint32_t*)(pvChromaStartAddress + offsetC) = chroma32bit;
            }
        }
    }
    BDBG_MSG(("Done striping the image..."));

    /* flush cache after filling the striped surface, before submitting to image input hw */
    NEXUS_Memory_FlushCache(pvLumaStartAddress, lumaBufSize);
    NEXUS_Memory_FlushCache(pvChromaStartAddress, chromaBufSize);

    NEXUS_MemoryBlock_Unlock(picCfg.lumaBuffer);
    NEXUS_MemoryBlock_Unlock(picCfg.chromaBuffer);

    return NEXUS_SUCCESS;
}
