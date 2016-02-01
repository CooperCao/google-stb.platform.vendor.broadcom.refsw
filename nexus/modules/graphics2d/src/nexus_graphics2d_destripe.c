/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#include "nexus_graphics2d_module.h"
#include "nexus_graphics2d_impl.h"
#include "priv/nexus_surface_priv.h"

BDBG_MODULE(nexus_graphics2d_destripe);

NEXUS_SurfaceHandle NEXUS_Graphics2D_Destripe( NEXUS_Graphics2DHandle gfx, NEXUS_StripedSurfaceHandle stripedSurface )
{
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    BERR_Code rc = 0;
    NEXUS_StripedSurfaceCreateSettings stripedSurfaceCreateSettings;

    NEXUS_StripedSurface_GetCreateSettings(stripedSurface, &stripedSurfaceCreateSettings);

    BDBG_MSG(("Retrieve still %lux%lu (striped %ldx%ld)", stripedSurfaceCreateSettings.imageWidth, stripedSurfaceCreateSettings.imageHeight, stripedSurfaceCreateSettings.stripedWidth, stripedSurfaceCreateSettings.lumaStripedHeight));
    if (stripedSurfaceCreateSettings.imageHeight == 0 || stripedSurfaceCreateSettings.imageWidth == 0) {
        BDBG_ERR(("invalid still image %lux%lu", stripedSurfaceCreateSettings.imageHeight, stripedSurfaceCreateSettings.imageWidth));
        return NULL;
    }

    /* create a bsurface_t which wraps the BSUR_Handle used by VDC */
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.height = stripedSurfaceCreateSettings.imageHeight;
    surfaceCreateSettings.width = stripedSurfaceCreateSettings.imageWidth;
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
    surface = NEXUS_Surface_Create(&surfaceCreateSettings);
    if (!surface) {rc=BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);return NULL;}

    rc = NEXUS_Graphics2D_DestripeToSurface(gfx, stripedSurface, surface, NULL);
    if (rc) {
        rc = BERR_TRACE(rc);
        NEXUS_Surface_Destroy(surface);
        surface = NULL;
    }
    else {
        NEXUS_OBJECT_REGISTER(NEXUS_Surface, surface, Create);
    }
    return surface;
}


NEXUS_Error NEXUS_Graphics2D_DestripeToSurface( NEXUS_Graphics2DHandle gfx,
                                                NEXUS_StripedSurfaceHandle sourceSurface,
                                                NEXUS_SurfaceHandle outputSurface,
                                                const NEXUS_Rect *pOutputRect )
{
    NEXUS_Graphics2DDestripeBlitSettings settings;
    NEXUS_Graphics2D_GetDefaultDestripeBlitSettings(&settings);

    settings.source.stripedSurface = sourceSurface;
    settings.output.surface = outputSurface;
    if ( pOutputRect ) {
        settings.output.rect = *pOutputRect;
    }

    return NEXUS_Graphics2D_DestripeBlit(gfx, &settings);
}

void NEXUS_Graphics2D_GetDefaultDestripeBlitSettings(
                                                    NEXUS_Graphics2DDestripeBlitSettings *pSettings /* [out] */
                                                    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->horizontalFilter = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
    pSettings->verticalFilter = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
    pSettings->chromaFilter = true;
}


NEXUS_Error NEXUS_Graphics2D_DestripeBlit(
                                         NEXUS_Graphics2DHandle gfx,
                                         const NEXUS_Graphics2DDestripeBlitSettings *pSettings
                                         )
{
    BERR_Code err = BERR_SUCCESS;
    BGRClib_DestripeBlitParams blitParams;
    BM2MC_PACKET_StripedPlane srcSurface;
    BM2MC_PACKET_Plane outSurface;
    BGRClib_DestripeBlitParams destripeBlitParams;
    NEXUS_Addr paletteOffset;
    NEXUS_SurfaceCreateSettings targetSettings;
    NEXUS_Addr lumaOffset, chromaOffset;
    NEXUS_Addr bottomLumaOffset = 0, bottomChromaOffset = 0;
    bool interlaced;
    NEXUS_StripedSurfaceCreateSettings stripedSurfaceCreateSettings;
    const int32_t *pMatrix=NULL;
    uint32_t matrixShift=10;

    if ( NULL == pSettings || NULL == pSettings->source.stripedSurface || NULL == pSettings->output.surface ) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BGRClib_GetDefaultDestripeBlitParams(&blitParams);

    NEXUS_StripedSurface_GetCreateSettings(pSettings->source.stripedSurface, &stripedSurfaceCreateSettings);
    if (stripedSurfaceCreateSettings.lumaBuffer==NULL || stripedSurfaceCreateSettings.chromaBuffer==NULL) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* for now do a short-cut and assume that NEXUS_MemoryBlock is already pinned */
    lumaOffset = NEXUS_MemoryBlock_GetOffset_priv(stripedSurfaceCreateSettings.lumaBuffer, stripedSurfaceCreateSettings.lumaBufferOffset);
    chromaOffset = NEXUS_MemoryBlock_GetOffset_priv(stripedSurfaceCreateSettings.chromaBuffer, stripedSurfaceCreateSettings.chromaBufferOffset);
    if (stripedSurfaceCreateSettings.bufferType == NEXUS_VideoBufferType_eFieldPair &&
        stripedSurfaceCreateSettings.bottomFieldLumaBuffer &&
        stripedSurfaceCreateSettings.bottomFieldChromaBuffer) {
        bottomLumaOffset = NEXUS_MemoryBlock_GetOffset_priv(stripedSurfaceCreateSettings.bottomFieldLumaBuffer, stripedSurfaceCreateSettings.bottomFieldLumaBufferOffset);
        bottomChromaOffset = NEXUS_MemoryBlock_GetOffset_priv(stripedSurfaceCreateSettings.bottomFieldChromaBuffer, stripedSurfaceCreateSettings.bottomFieldChromaBufferOffset);
    }

    interlaced = (stripedSurfaceCreateSettings.bufferType == NEXUS_VideoBufferType_eTopField) || (stripedSurfaceCreateSettings.bufferType == NEXUS_VideoBufferType_eBotField);
    BDBG_MSG(("destripe %lux%lu%c", stripedSurfaceCreateSettings.imageWidth, stripedSurfaceCreateSettings.imageHeight, interlaced?'i':'p'));

    /* for bottom field, we must apply an offset to the start address */
    if (stripedSurfaceCreateSettings.bufferType == NEXUS_VideoBufferType_eBotField) {
        lumaOffset += stripedSurfaceCreateSettings.stripedWidth;
        chromaOffset += stripedSurfaceCreateSettings.stripedWidth;
    }

    NEXUS_Surface_GetCreateSettings( pSettings->output.surface, &targetSettings );

    err = NEXUS_Graphics2D_P_LockPlaneAndPalette( gfx, pSettings->output.surface, &outSurface, &paletteOffset );
    if (err!=NEXUS_SUCCESS) {
        return BERR_TRACE(err);
    }

    srcSurface.luma_address = lumaOffset;
    srcSurface.chroma_address = chromaOffset;
    srcSurface.bottom_field_luma_address = bottomLumaOffset;
    srcSurface.bottom_field_chroma_address = bottomChromaOffset;
    srcSurface.width = stripedSurfaceCreateSettings.imageWidth;
    srcSurface.height = interlaced ? stripedSurfaceCreateSettings.imageHeight / 2 : stripedSurfaceCreateSettings.imageHeight;
    srcSurface.stripe_width = stripedSurfaceCreateSettings.stripedWidth;
    srcSurface.stripe_pitch = interlaced ? stripedSurfaceCreateSettings.stripedWidth * 2: stripedSurfaceCreateSettings.stripedWidth;
    srcSurface.luma_stripe_height = stripedSurfaceCreateSettings.lumaStripedHeight;
    srcSurface.chroma_stripe_height = stripedSurfaceCreateSettings.chromaStripedHeight;
    srcSurface.luma_format = stripedSurfaceCreateSettings.lumaPixelFormat;
    srcSurface.chroma_format = stripedSurfaceCreateSettings.chromaPixelFormat;

    destripeBlitParams.srcSurface = &srcSurface;
    if ( pSettings->source.rect.x != 0 || pSettings->source.rect.y != 0 ||
         pSettings->source.rect.width != 0 || pSettings->source.rect.height != 0 ) {
        destripeBlitParams.srcRect = (const BRect *)(void*)&pSettings->source.rect;
    } else {
        destripeBlitParams.srcRect = NULL;
    }
    destripeBlitParams.outSurface = &outSurface;
    if ( pSettings->output.rect.x != 0 || pSettings->output.rect.y != 0 ||
         pSettings->output.rect.width != 0 || pSettings->output.rect.height != 0 ) {
        destripeBlitParams.outRect = (const BRect *)(void*)&pSettings->output.rect;
    } else {
        destripeBlitParams.outRect = NULL;
    }

    /* There are CASSERTs to check this in nexus_graphics2d.c*/
    destripeBlitParams.horzFilter = pSettings->horizontalFilter;
    destripeBlitParams.vertFilter = pSettings->verticalFilter;

    destripeBlitParams.chromaFilter = pSettings->chromaFilter;

    if ( pSettings->conversionMatrixEnabled ) {
        pMatrix = (const int32_t *)pSettings->conversionMatrix.coeffMatrix;
        matrixShift = pSettings->conversionMatrix.shift;
    } else if ( !NEXUS_PIXEL_FORMAT_IS_YCRCB(targetSettings.pixelFormat) ) {
        pMatrix = g_NEXUS_ai32_Matrix_YCbCrtoRGB;
    }
    destripeBlitParams.matrixParams.conversionMatrix = pMatrix;
    destripeBlitParams.matrixParams.matrixShift = matrixShift;

    err = BGRClib_Destripe_Blit(gfx->grclib, &destripeBlitParams);

    if ( (err == BERR_OUT_OF_DEVICE_MEMORY) || (err == BERR_OUT_OF_SYSTEM_MEMORY) )
        return NEXUS_GRAPHICS2D_QUEUE_FULL; /* no BERR_TRACE */

    return BERR_TRACE(err);
}
