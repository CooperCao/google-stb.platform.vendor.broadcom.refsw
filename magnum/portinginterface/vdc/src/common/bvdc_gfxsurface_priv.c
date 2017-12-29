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

#include "bvdc_gfxsurface_priv.h"
#include "bvdc_feeder_priv.h"
#include "bchp_gfd_0.h"

BDBG_MODULE(BVDC_GFXSUR);
BDBG_OBJECT_ID(BVDC_GFXSUR);


#define BVDC_P_SUR_RETURN_IF_ERR(result) \
    if ( BERR_SUCCESS != (result)) \
{\
    return BERR_TRACE(result);  \
}

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxSurface_AllocShadowRegs
 *
 * Called by GfxFeeder or VfdFeeder to allocate surface address shadow
 * registers from scratch pool,
 */
BERR_Code BVDC_P_GfxSurface_AllocShadowRegs
    ( BVDC_P_GfxSurfaceContext        *pGfxSurface,
      BRDC_Handle                      hRdc,
      bool                             b3dSrc )
{
    BDBG_ASSERT(NULL != pGfxSurface);
    BDBG_ASSERT(NULL != hRdc);

    pGfxSurface->b3dSrc = b3dSrc;

    if (0 == pGfxSurface->ulSurAddrReg[0])
    {
        BDBG_ASSERT(0 == pGfxSurface->ulSurAddrReg[1] &&
                    0 == pGfxSurface->ulRSurAddrReg[0] &&
                    0 == pGfxSurface->ulRSurAddrReg[1] &&
                    0 == pGfxSurface->ulRegIdxReg &&
                    0 == pGfxSurface->ulVsyncCntrReg);

        pGfxSurface->ulVsyncCntrReg = BRDC_AllocScratchReg(hRdc);
        pGfxSurface->ulSurAddrReg[0] = BRDC_AllocScratchReg(hRdc);
        if (b3dSrc)
        {
            pGfxSurface->ulSurAddrReg[1] = BRDC_AllocScratchReg(hRdc);
            pGfxSurface->ulRSurAddrReg[0] = BRDC_AllocScratchReg(hRdc);
            pGfxSurface->ulRSurAddrReg[1] = BRDC_AllocScratchReg(hRdc);
            pGfxSurface->ulRegIdxReg = BRDC_AllocScratchReg(hRdc);
        }
        BDBG_MSG((" %s [%d]",
            (pGfxSurface->eSrcId >= BAVC_SourceId_eVfd0) ? "vfd":
            ((pGfxSurface->eSrcId >= BAVC_SourceId_eGfx0)?"gfd":"mfd"),
            (pGfxSurface->eSrcId >= BAVC_SourceId_eVfd0) ? (pGfxSurface->eSrcId - BAVC_SourceId_eVfd0):
            ((pGfxSurface->eSrcId >= BAVC_SourceId_eGfx0)?
                (pGfxSurface->eSrcId - BAVC_SourceId_eGfx0):(pGfxSurface->eSrcId - BAVC_SourceId_eMpeg0))));
        BDBG_MSG((" ulVsyncCntrReg   %x ulRegIdxReg %x",
            pGfxSurface->ulVsyncCntrReg, pGfxSurface->ulRegIdxReg));
        BDBG_MSG((" ulSurAddrReg %x   %x",
            pGfxSurface->ulSurAddrReg[0],  pGfxSurface->ulSurAddrReg[1]));
        BDBG_MSG((" ulRSurAddrReg %x   %x",
            pGfxSurface->ulRSurAddrReg[0], pGfxSurface->ulRSurAddrReg[1]));
    }

    if ((!b3dSrc && (!pGfxSurface->ulSurAddrReg[0] || !pGfxSurface->ulVsyncCntrReg)) ||
        ( b3dSrc && (!pGfxSurface->ulSurAddrReg[0] || !pGfxSurface->ulSurAddrReg[1] ||
                    !pGfxSurface->ulRSurAddrReg[0] || !pGfxSurface->ulRSurAddrReg[1] ||
                    !pGfxSurface->ulRegIdxReg || !pGfxSurface->ulVsyncCntrReg)))
    {
        BDBG_ERR(("Not enough scratch registers for gfx surface feeder[%d]",
                  pGfxSurface->eSrcId));
        return BERR_TRACE(BVDC_ERR_RESOURCE_NOT_AVAILABLE);
    }

    return BERR_SUCCESS;
}

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxSurface_FreeShadowRegs
 *
 * Called to fee surface address shadow registers back to scratch pool,
 * when the shadow registers are no longer needed.
 */
BERR_Code BVDC_P_GfxSurface_FreeShadowRegs
    ( BVDC_P_GfxSurfaceContext        *pGfxSurface,
      BRDC_Handle                      hRdc )
{
    if (pGfxSurface->ulSurAddrReg[0])
    {
        BDBG_ASSERT(0 != pGfxSurface->ulVsyncCntrReg);
        BRDC_FreeScratchReg(hRdc, pGfxSurface->ulSurAddrReg[0]);
        BRDC_FreeScratchReg(hRdc, pGfxSurface->ulVsyncCntrReg);

        if (pGfxSurface->b3dSrc)
        {
            BDBG_ASSERT(0 != pGfxSurface->ulSurAddrReg[1] &&
                        0 != pGfxSurface->ulRSurAddrReg[0] &&
                        0 != pGfxSurface->ulRSurAddrReg[1] &&
                        0 != pGfxSurface->ulRegIdxReg );
            BRDC_FreeScratchReg(hRdc, pGfxSurface->ulSurAddrReg[1]);
            BRDC_FreeScratchReg(hRdc, pGfxSurface->ulRSurAddrReg[0]);
            BRDC_FreeScratchReg(hRdc, pGfxSurface->ulRSurAddrReg[1]);
            BRDC_FreeScratchReg(hRdc, pGfxSurface->ulRegIdxReg);
        }

        pGfxSurface->ulSurAddrReg[0] = 0;
        pGfxSurface->ulSurAddrReg[1] = 0;
        pGfxSurface->ulRSurAddrReg[0] = 0;
        pGfxSurface->ulRSurAddrReg[1] = 0;
        pGfxSurface->ulRegIdxReg = 0;
        pGfxSurface->ulVsyncCntrReg = 0;
    }
    return BERR_SUCCESS;
}

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxSurface_Init
 *
 * Called by BVDC_P_GfxFeeder_Init or BVDC_P_Feeder_Init when a source
 * handle is created with BVDC_Source_Created
 */
BERR_Code BVDC_P_GfxSurface_Init
    ( BVDC_P_GfxSurfaceContext        *pGfxSurface )
{
    BDBG_ASSERT(NULL != pGfxSurface);

    BDBG_ASSERT(0 != pGfxSurface->ulSurAddrReg[0] &&
                0 != pGfxSurface->ulVsyncCntrReg);
    BREG_WriteAddr(pGfxSurface->hRegister, pGfxSurface->ulSurAddrReg[0],  0);
    BRDC_WriteScratch(pGfxSurface->hRegister, pGfxSurface->ulVsyncCntrReg,   1);

    if (pGfxSurface->b3dSrc)
    {
        BDBG_ASSERT(0 != pGfxSurface->ulSurAddrReg[1] &&
                    0 != pGfxSurface->ulRSurAddrReg[0] &&
                    0 != pGfxSurface->ulRSurAddrReg[1] &&
                    0 != pGfxSurface->ulRegIdxReg );
        BREG_WriteAddr(pGfxSurface->hRegister, pGfxSurface->ulSurAddrReg[1],  0);
        BREG_WriteAddr(pGfxSurface->hRegister, pGfxSurface->ulRSurAddrReg[0], 0);
        BREG_WriteAddr(pGfxSurface->hRegister, pGfxSurface->ulRSurAddrReg[1], 0);
        BRDC_WriteScratch(pGfxSurface->hRegister, pGfxSurface->ulRegIdxReg,      0);
    }

    BKNI_Memset((void*)&pGfxSurface->stIsrSurInfo, 0x0, sizeof(BVDC_P_SurfaceInfo));
    BKNI_Memset((void*)&pGfxSurface->stNewSurInfo, 0x0, sizeof(BVDC_P_SurfaceInfo));
    BKNI_Memset((void*)&pGfxSurface->stCurSurInfo, 0x0, sizeof(BVDC_P_SurfaceInfo));

    pGfxSurface->ucNodeIdx = 0;
    BKNI_Memset((void*)&pGfxSurface->stSurNode[0], 0x0, 4 * sizeof(BVDC_P_GfxSurNode));

    return BERR_SUCCESS;
}

/*------------------------------------------------------------------------
 *  {secret}
 *  BVDC_P_GfxSurface_SetSurface_isr
 *
 *  It will check the BAVC_Gfx_Picture struct pointed by pAvcGfxPic to ensure
 *  that there is no conflict inside itself. It will then program the surface
 *  address shadow registers if size / pitch and format match the current.
 *  Otherwise it will mark the surface as "changed" so that ApplyChange will
 *  validate it, and then the next rul build will program it into RUL
 */
BERR_Code BVDC_P_GfxSurface_SetSurface_isr
    ( BVDC_P_GfxSurfaceContext        *pGfxSurface,
      BVDC_P_SurfaceInfo              *pSurInfo,
      const BAVC_Gfx_Picture          *pAvcGfxPic,
      BVDC_Source_Handle               hSource)
{
    BERR_Code              eResult = BERR_SUCCESS;
    const BPXL_Plane      *pSurface;
    BMMA_DeviceOffset      ullSurOffset;
    uint32_t               ulPitch;
    uint32_t               ulWidth;
    uint32_t               ulHeight;
    BPXL_Format            ePxlFmt;
    BMMA_DeviceOffset      ullRSurOffset;
    uint32_t               ulPltOffset;
    uint32_t               ulPaletteNumEntries;
    BPXL_Format            ePaletteEntryFormat;
#ifndef BVDC_FOR_BOOTUPDATER
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4)
    const BPXL_Plane      *pRSurface;
    uint32_t               ulRPitch;
    uint32_t               ulRWidth;
    uint32_t               ulRHeight;
    BPXL_Format            eRPxlFmt;
#endif
    BBOX_Vdc_Capabilities *pBoxVdc = &hSource->hVdc->stBoxConfig.stVdc;
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
    BVDC_P_SurfaceInfo    *pCurSurInfo = &pGfxSurface->stCurSurInfo;

    /* get info from main surface */
    if(pAvcGfxPic->pSurface) {
        pSurface = pAvcGfxPic->pSurface;

        ulPitch  = pSurface->ulPitch;
        ulWidth  = pSurface->ulWidth;
        ulHeight = pSurface->ulHeight;

#ifndef BVDC_FOR_BOOTUPDATER
        /* Check for BOX mode limits */
        if (pBoxVdc->astSource[hSource->eId].stSizeLimits.ulHeight != BBOX_VDC_DISREGARD &&
            pBoxVdc->astSource[hSource->eId].stSizeLimits.ulWidth != BBOX_VDC_DISREGARD)
        {
            if (ulHeight > pBoxVdc->astSource[hSource->eId].stSizeLimits.ulHeight ||
                ulWidth > pBoxVdc->astSource[hSource->eId].stSizeLimits.ulWidth)
            {
                BDBG_ERR(("Source ID %d (GFX) surface frame buffer size [%dx%d] exceeds BOX limits [%dx%d].",
                    hSource->eId,
                    ulWidth, ulHeight,
                    pBoxVdc->astSource[hSource->eId].stSizeLimits.ulWidth,
                    pBoxVdc->astSource[hSource->eId].stSizeLimits.ulHeight));
                return BERR_TRACE(BBOX_FRAME_BUFFER_SIZE_EXCEEDS_LIMIT);
            }
        }
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

        ullSurOffset = BMMA_GetOffset_isr(pSurface->hPixels);
        ullSurOffset += pSurface->ulPixelsOffset;
        ePxlFmt = pSurface->eFormat;

        if (BPXL_eP0==ePxlFmt || BPXL_eA0==ePxlFmt)
        {
            /* to make later logic simple */
            ulPitch = (ulPitch)? ulPitch : 1;
            ullSurOffset = (ullSurOffset)? ullSurOffset : 0x1;
        }

        if (0==ulWidth || 0==ulHeight || 0==ulPitch || 0 == ullSurOffset)
        {
            BDBG_ERR(("ulWidth %d, ulHeight %d, ulPitch %d",
                      ulWidth, ulHeight, ulPitch));
            return BERR_TRACE(BVDC_ERR_GFX_SUR_SIZE_MISMATCH);
        }

#if (BVDC_P_SUPPORT_GFD_VER_0 == BVDC_P_SUPPORT_GFD1_VER)
        if ((BAVC_SourceId_eGfx1 == pGfxSurface->eSrcId) &&
            (!BPXL_IS_YCbCr_FORMAT(ePxlFmt)) &&
            (BPXL_eP1 != ePxlFmt) && (BPXL_eP2 != ePxlFmt) && (BPXL_eP4 != ePxlFmt))
        {
            return BERR_TRACE(BVDC_ERR_GFX_SUR_FMT_MISMATCH);
        }
#endif

        ullRSurOffset = 0; /* mark for 2D case */

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4) && !defined(BVDC_FOR_BOOTUPDATER)
        /* check right surface */
        if (BVDC_P_ORIENTATION_IS_3D(pAvcGfxPic->eInOrientation))
        {
            /* since orientation is 3D, hRSurface must be valid */
            pRSurface = pAvcGfxPic->pRSurface;
            ulRPitch = pRSurface->ulPitch;
            ulRWidth = pRSurface->ulWidth;
            ulRHeight = pRSurface->ulHeight;

            /* Check for BOX mode limits */
            if (pBoxVdc->astSource[hSource->eId].stSizeLimits.ulHeight != BBOX_VDC_DISREGARD &&
                pBoxVdc->astSource[hSource->eId].stSizeLimits.ulWidth != BBOX_VDC_DISREGARD)
            {
                if (ulRHeight > pBoxVdc->astSource[hSource->eId].stSizeLimits.ulHeight ||
                    ulRWidth > pBoxVdc->astSource[hSource->eId].stSizeLimits.ulWidth)
                {
                    BDBG_ERR(("3D gfx surface frame buffer size [%dx%d] exceeds BOX limits [%dx%d].",
                        ulRWidth, ulRHeight,
                        pBoxVdc->astSource[hSource->eId].stSizeLimits.ulWidth,
                        pBoxVdc->astSource[hSource->eId].stSizeLimits.ulHeight));
                    return BERR_TRACE(BBOX_FRAME_BUFFER_SIZE_EXCEEDS_LIMIT);
                }
            }

            ullRSurOffset = BMMA_GetOffset_isr(pRSurface->hPixels);
            ullRSurOffset += pRSurface->ulPixelsOffset;
            eRPxlFmt = pRSurface->eFormat;

            if (ulPitch != ulRPitch ||
                ulWidth != ulRWidth ||
                ulHeight != ulRHeight ||
                ePxlFmt != eRPxlFmt ||
                0 == ullRSurOffset)
            {
                return BERR_INVALID_PARAMETER;
            }
        }
        /* override 2D to 3D */
        else if( (hSource->stCurInfo.bOrientationOverride) &&
            (BVDC_P_ORIENTATION_IS_3D(hSource->stCurInfo.eOrientation)))
        {
            /* only 2D surface allows to be override orientation */
            BDBG_ASSERT(pAvcGfxPic->eInOrientation==BFMT_Orientation_e2D);
            ulRPitch = ulPitch;
            eRPxlFmt = ePxlFmt;
            ulRPitch = ulPitch;

            ulHeight >>= (BFMT_Orientation_e3D_OverUnder == hSource->stCurInfo.eOrientation);
            ulWidth  >>= (BFMT_Orientation_e3D_LeftRight == hSource->stCurInfo.eOrientation);

            ulRHeight = ulHeight;
            ulRWidth  = ulWidth;

            ullRSurOffset = ullSurOffset +
                (BFMT_Orientation_e3D_OverUnder == hSource->stCurInfo.eOrientation) * ulHeight* ulPitch +
                (BFMT_Orientation_e3D_LeftRight == hSource->stCurInfo.eOrientation) * ulWidth * BPXL_BITS_PER_PIXEL(ePxlFmt)/8;
        }
#endif /* #if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4) && !defined(BVDC_FOR_BOOTUPDATER) */

        /* handle palette format */
        if ( BPXL_IS_PALETTE_FORMAT(ePxlFmt) )
        {
            /* if it is palette input format, but hPalette is NULL, we assume
             * the palette does not change. This allow double buffering palette
             * surface without loading palette for every buffer swiching.
             */
            if (pSurface->hPalette)
            {
                ulPltOffset = BMMA_GetOffset_isr(pSurface->hPalette);
                ulPltOffset += pSurface->ulPaletteOffset;
                ulPaletteNumEntries = pSurface->ulNumPaletteEntries;
                ePaletteEntryFormat = pSurface->ePalettePixelFormat;

                /* no more error check */
                pSurInfo->ulPaletteAddress = ulPltOffset;
                pSurInfo->ulPaletteNumEntries = ulPaletteNumEntries;
                pSurInfo->eActivePxlFmt = ePaletteEntryFormat;
                pSurInfo->bChangePaletteTable = true;
            }
            else
            {
                /* no palette table change */
                pSurInfo->eActivePxlFmt = pCurSurInfo->eActivePxlFmt;
                pSurInfo->bChangePaletteTable = false;
            }
        }
        else if (false == (BPXL_IS_ALPHA_ONLY_FORMAT(ePxlFmt)))
        {
            pSurInfo->eActivePxlFmt = ePxlFmt;
            pSurInfo->bChangePaletteTable = false;
        }
        else /* alpha only format */
        {
            pSurInfo->eActivePxlFmt = BPXL_eA8_R8_G8_B8;
            pSurInfo->bChangePaletteTable = false;
        }

        /* store surface info
         */
        pSurInfo->ullAddress = ullSurOffset;
        pSurInfo->ulPitch   = ulPitch;
        pSurInfo->ullRAddress = ullRSurOffset;
    } else { /* striped surface */
#ifdef BVDC_FOR_BOOTUPDATER
        BDBG_ERR(("Boot updater code does not support striped surface"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
#else
        BVDC_P_Feeder_Handle hFeeder = hSource->hMpegFeeder;
        BDBG_ASSERT(pAvcGfxPic->pstMfdPic);
        ulWidth  = pAvcGfxPic->pstMfdPic->ulSourceHorizontalSize;
        ulHeight = pAvcGfxPic->pstMfdPic->ulSourceVerticalSize;
        ulPitch  = 0;
        ePxlFmt  = BPXL_INVALID; /* striped surface */
        pSurInfo->bChangePaletteTable = false;
        BVDC_P_Feeder_SetMpegAddr_isr(hFeeder, pAvcGfxPic->pstMfdPic, &hSource->stScanOut);
        /* only support y/c 2D and frame buffer format */
        pSurInfo->ullAddress = hFeeder->stRegs.ullPic0Addr0;
        pSurInfo->ullRAddress = hFeeder->stRegs.ullPic0Addr1;
        BDBG_MSG(("yAddr="BDBG_UINT64_FMT", cAddr="BDBG_UINT64_FMT", %ux%u",
            BDBG_UINT64_ARG(pSurInfo->ullAddress), BDBG_UINT64_ARG(pSurInfo->ullRAddress),
            ulWidth, ulHeight));
#endif /* #ifdef BVDC_FOR_BOOTUPDATER */
    }
    pSurInfo->ulWidth   = ulWidth;
    pSurInfo->ulHeight  = ulHeight;
    pSurInfo->eInputPxlFmt = ePxlFmt;

    pSurInfo->stAvcPic = *pAvcGfxPic;
    pSurInfo->bChangeSurface = true; /* inform ApplyChange */
    pSurInfo->bSetSurface = true; /* for Psf/ForceFrameCapture repeat bit */

    /* set this surface setting to shadow register if we can
     */
    if ((ulWidth == pCurSurInfo->ulWidth) &&
        (ulHeight == pCurSurInfo->ulHeight) &&
        (ulPitch == pCurSurInfo->ulPitch) &&
        (ePxlFmt == pCurSurInfo->eInputPxlFmt) &&
        (!pSurInfo->bChangePaletteTable) &&
        !pAvcGfxPic->pstMfdPic)
    {
        BVDC_P_GfxSurface_SetShadowRegs_isr(pGfxSurface, pSurInfo, hSource);

        /* surface is already in use */
        pSurInfo->bChangeSurface = false;
        *pCurSurInfo = *pSurInfo;
    }

    return eResult;
}

/*-------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxSurface_SetShadowRegs_isr
 *
 * set the surface addr to the shadow registers.
 * Called from BVDC_Source_SetSurface or BuildRul after ApplyChange
 */
void BVDC_P_GfxSurface_SetShadowRegs_isr
    ( BVDC_P_GfxSurfaceContext        *pGfxSurface,
      BVDC_P_SurfaceInfo              *pSurInfo,
      BVDC_Source_Handle              hSource)
{
    BREG_Handle  hRegister;
    BVDC_P_GfxSurNode  *pNode;
    uint32_t  ulVsyncCntr1, ulVsyncCntr2;
    int iNodeIdx;
    BMMA_DeviceOffset  ullSurAddr, ullRSurAddr, ullRegIdx;

    hRegister = pGfxSurface->hRegister;

    /* pre-read of ulVsyncCntrReg before setting shadow registers.
     * note: when RUL is executed it will increase the value in ulVsyncCntrReg by 1
     */
    ulVsyncCntr1 = BRDC_ReadScratch_isrsafe(hRegister, pGfxSurface->ulVsyncCntrReg);
    if (0 == ulVsyncCntr1)
    {
        /* we could see this once after 828.5 days of running */
        BDBG_MSG(("We see Vsync cntr wrapped back to 0"));
    }

    /* set surface addr to shadow registers
     * note: RUL will add pitch for bottom field
     */
    ullSurAddr = pSurInfo->ullAddress + pGfxSurface->ulMainByteOffset;
    BDBG_ASSERT(ullSurAddr);
    BSTD_UNUSED(hSource);

    if (pGfxSurface->b3dSrc || !pSurInfo->stAvcPic.pSurface)
    {
        ullRSurAddr = pSurInfo->ullRAddress + pGfxSurface->ulMainByteOffset;
        #if BRDC_64BIT_SUPPORT
        ullRegIdx = ~ pGfxSurface->ullRegIdx;/* the ping-pong bitmask */
        #else
        /* 32-bit RDC scratch register write would validate the bit range of the value, so tighten the bitmask here; */
        ullRegIdx = (uint32_t)(~ pGfxSurface->ullRegIdx);
        #endif
        BREG_WriteAddr_isrsafe(hRegister, pGfxSurface->ulSurAddrReg[ullRegIdx & 1], ullSurAddr);
        BREG_WriteAddr_isrsafe(hRegister, pGfxSurface->ulRSurAddrReg[ullRegIdx & 1], ullRSurAddr);

        /* adapt the pin-pong bitmask according to hw address RDB */
        BREG_WriteAddr_isrsafe(hRegister, pGfxSurface->ulRegIdxReg,
            BCHP_GET_FIELD_DATA(ullRegIdx, GFD_0_SRC_START,  ADDR));

        pGfxSurface->ullRegIdx = ullRegIdx;
        BDBG_MSG(("%s [%d] "BDBG_UINT64_FMT" surface "BDBG_UINT64_FMT" "BDBG_UINT64_FMT,
            (pGfxSurface->eSrcId >= BAVC_SourceId_eVfd0) ? "vfd":
            ((pGfxSurface->eSrcId >= BAVC_SourceId_eGfx0)?"gfd":"mfd"),
            (pGfxSurface->eSrcId >= BAVC_SourceId_eVfd0) ? (pGfxSurface->eSrcId - BAVC_SourceId_eVfd0):
            ((pGfxSurface->eSrcId >= BAVC_SourceId_eGfx0)?
            (pGfxSurface->eSrcId - BAVC_SourceId_eGfx0):(pGfxSurface->eSrcId - BAVC_SourceId_eMpeg0)),
            BDBG_UINT64_ARG(ullRegIdx),
            BDBG_UINT64_ARG(ullSurAddr), BDBG_UINT64_ARG(ullRSurAddr)));
    }
    else
    {
        BREG_WriteAddr_isrsafe(hRegister, pGfxSurface->ulSurAddrReg[0], ullSurAddr);
        ullRSurAddr = 0;
        BDBG_MSG(("%s [%d] %d surface "BDBG_UINT64_FMT", %x",
            (pGfxSurface->eSrcId >= BAVC_SourceId_eVfd0) ? "vfd":
            ((pGfxSurface->eSrcId >= BAVC_SourceId_eGfx0)?"gfd":"mfd"),
            (pGfxSurface->eSrcId >= BAVC_SourceId_eVfd0) ? (pGfxSurface->eSrcId - BAVC_SourceId_eVfd0):
            ((pGfxSurface->eSrcId >= BAVC_SourceId_eGfx0)?
            (pGfxSurface->eSrcId - BAVC_SourceId_eGfx0):(pGfxSurface->eSrcId - BAVC_SourceId_eMpeg0)),
            0, BDBG_UINT64_ARG(ullSurAddr), 0));
    }

    /* post-read of ulVsyncCntrReg after setting shadow registers.
     */
    ulVsyncCntr2 = BRDC_ReadScratch_isrsafe(hRegister, pGfxSurface->ulVsyncCntrReg);

    /* choose a node to record this sur setting */
    iNodeIdx = pGfxSurface->ucNodeIdx;
    if ((pGfxSurface->stSurNode[iNodeIdx].ulVsyncCntr != ulVsyncCntr1) ||
        (pGfxSurface->stSurNode[iNodeIdx].bExeDuringSet))
    {
        /* current node might have be executed, so we use next node */
        iNodeIdx = (iNodeIdx + 1) & 0x3;
        pGfxSurface->ucNodeIdx = iNodeIdx;
    }
    pNode = &pGfxSurface->stSurNode[iNodeIdx];

    /* record this setting to node */
    pNode->stAvcPic = pSurInfo->stAvcPic;
    pNode->ullAddr = ullSurAddr;
    pNode->ulPitch = pSurInfo->ulPitch;
    pNode->ullRAddr = ullRSurAddr;
    pNode->ulVsyncCntr = ulVsyncCntr2;  /* post read */
    pNode->bExeDuringSet = (ulVsyncCntr2 != ulVsyncCntr1);

    return;
}

/*------------------------------------------------------------------------
 *  {private}
 *  BVDC_P_GfxSurface_GetSurfaceInHw_isr
 *
 *  Read HW registers to decide which picture node the HW is using.
 */
BAVC_Gfx_Picture *BVDC_P_GfxSurface_GetSurfaceInHw_isr
    ( BVDC_P_GfxSurfaceContext        *pGfxSurface )
{
    BVDC_P_GfxSurNode  *pNode;
    uint32_t  ulVsyncCntr;
    BMMA_DeviceOffset ullAddr;
    int ii, iNodeIdx;

    /* when RUL is executed it will increase the value in ulVsyncCntrReg by 1 */
    ulVsyncCntr = BRDC_ReadScratch_isrsafe(pGfxSurface->hRegister, pGfxSurface->ulVsyncCntrReg);

    iNodeIdx = pGfxSurface->ucNodeIdx;
    for (ii=0; ii<4; ii++)
    {
        /* "&& pNode->ullAddr" here is for the first 4 surface setting after start */
        pNode = &pGfxSurface->stSurNode[iNodeIdx];
        if (ulVsyncCntr != pNode->ulVsyncCntr && pNode->ullAddr)
        {
            /* at least one vsync passed after the setting of this node */
            return &pNode->stAvcPic;
        }
        else
        {
            if (pNode->bExeDuringSet)
            {
                ullAddr = BREG_ReadAddr_isrsafe(pGfxSurface->hRegister, pGfxSurface->ulHwMainSurAddrReg);
                if ((0 != ullAddr) &&
                    (ullAddr == pNode->ullAddr || /* top field or frame */
                     ullAddr == pNode->ullAddr + pNode->ulPitch || /* bottom field */
                     ullAddr == pNode->ullRAddr || /* left/right swapped top field or frame */
                     ullAddr == pNode->ullRAddr + pNode->ulPitch)) /* left/right swapped bottom */
                {
                    /* left sur and right sur always update together, so we only
                     * need to check main sur addr */
                    return &pNode->stAvcPic;
                }
            }
            /* else this node is definately not used by HW yet */
        }

        iNodeIdx = (iNodeIdx + 4 - 1) & 0x3;
    }

    /* we might be here at beginning only. return the current sur setting
     * if no surface is set yet, the AvcPic returned could have all NULL surface handles */
    BDBG_MSG(("no node found, ulVsyncCntr=%d, srcId = %d", ulVsyncCntr, pGfxSurface->eSrcId));
    return &(pGfxSurface->stSurNode[pGfxSurface->ucNodeIdx].stAvcPic);
}

/* End of File */
