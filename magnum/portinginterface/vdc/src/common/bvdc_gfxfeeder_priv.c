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
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_priv.h"
#include "bchp.h"
#include "bchp_gfd_0.h"
#include "bchp_cmp_0.h"
#include "bchp_bmisc.h"

#if (BVDC_P_MAX_GFX_WINDOWS >= 2)
#include "bchp_gfd_1.h"
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 3)
#include "bchp_gfd_2.h"
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 4)
#include "bchp_gfd_3.h"
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 5)
#include "bchp_gfd_4.h"
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 6)
#include "bchp_gfd_5.h"
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 7)
#include "bchp_gfd_6.h"
#endif


BDBG_MODULE(BVDC_GFX);
BDBG_OBJECT_ID(BVDC_GFX);


#define BVDC_P_GFD_MSG_ON              0
#if (BVDC_P_GFD_MSG_ON==1)
#define BDBG_P_GFD_MSG    BDBG_ERR
#else
#define BDBG_P_GFD_MSG    BDBG_MSG
#endif

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
#define BVDC_P_GFD_VSCL_LINE_BUFFER         (1280)
#define BVDC_P_GFD_4K_VSCL_LINE_BUFFER      (2048)
#endif

#define GFX_ALPHA_FULL   255
#define BPXL_eInvalid    ((BPXL_Format) 0)

#define BVDC_P_GFXFD_RETURN_IF_ERR(result) \
    if ( BERR_SUCCESS != (result)) \
    {\
        return BERR_TRACE(result);  \
    }

#define BVDC_P_GFXFD_END_IF_ERR(success, result, label) \
    if ( false == (success) ) \
    {\
        eResult = BERR_TRACE(result);  \
        goto (label);  \
    }

#define BVDC_P_GFX_RDC_OP_FIXED             0

#define BVDC_P_GFX_WIN_MAX_EDGE             (2047)

#define  GFD_NUM_BITS_PER_BYTE              8
#define  GFD_NUM_BITS_FIR_STEP_FRAC         17
#define  GFD_MASK_FIR_STEP_LOW              ((1<<(GFD_NUM_BITS_FIR_STEP_FRAC+1))-1)
#define  GFD_SHIFT_FIR_STEP_INT             (GFD_NUM_BITS_FIR_STEP_FRAC+1)
#define  GFD_MASK_FIR_STEP_INT              ((7)<<GFD_SHIFT_FIR_STEP_INT)
#define  GFD_MAX_FIR_STEP                   (GFD_MASK_FIR_STEP_INT | GFD_MASK_FIR_STEP_LOW)
#define  GFD_NUM_BITS_FIR_INIT_PHASE_FRAC   7
#define  GFD_MASK_FIR_INIT_PHASE_FRAC       ((1<<GFD_NUM_BITS_FIR_INIT_PHASE_FRAC)-1)
#define  GFD_HSCL_FIR_STEP_1                (1<<GFD_NUM_BITS_FIR_STEP_FRAC)
#define  GFD_HSCL_FIR_PHASE_1               (1<<GFD_NUM_BITS_FIR_INIT_PHASE_FRAC)

#define  GFD_VSCL_NUM_BITS_FIR_STEP_FRAC    17
#define  GFD_VSCL_MASK_FIR_STEP_FRAC        ((1<<GFD_VSCL_NUM_BITS_FIR_STEP_FRAC)-1)
#define  GFD_VSCL_NUM_BITS_FIR_INIT_PHASE_FRAC   6
#define  GFD_VSCL_MASK_FIR_INIT_PHASE_FRAC  ((1<<GFD_NUM_BITS_FIR_INIT_PHASE_FRAC)-1)
#define  GFD_VSCL_FIR_STEP_1                (1<<GFD_VSCL_NUM_BITS_FIR_STEP_FRAC)
#define  GFD_VSCL_FIR_PHASE_1               (1<<GFD_VSCL_NUM_BITS_FIR_INIT_PHASE_FRAC)

#define  GFD_VSCL_MAX_BLK_AVG_SIZE          2
#define  GFD_VSCL_BLK_AVG_THD               (4<<GFD_VSCL_NUM_BITS_FIR_STEP_FRAC)

#define  GFD_VSCL_FIR_STEP_TO_PHASE(s)      \
   ((s)>>(GFD_VSCL_NUM_BITS_FIR_STEP_FRAC - GFD_VSCL_NUM_BITS_FIR_INIT_PHASE_FRAC))

#define  GFD_NUM_REGS_ENABLE                1
#define  GFD_NUM_REGS_LOAD_PALETTE          1
#define  GFD_NUM_REGS_CTRL                  1
#define  GFD_NUM_REGS_PIXEL_FORMAT          2
#define  GFD_NUM_REGS_W_ALPHA               1
#define  GFD_NUM_REGS_KEY                   4
#define  GFD_NUM_REGS_ALPHA                 2
#define  GFD_NUM_REGS_PALETTE               2
#define  GFD_NUM_REGS_DISP                  1
#define  GFD_NUM_REGS_COLOR_MATRIX          9
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4)
#define  GFD_NUM_REGS_SRC                   5
#else
#define  GFD_NUM_REGS_SRC                   4
#endif


/***************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_Create
 *
 * Called by BVDC_P_Source_Create to create gfx feeder specific context
 * when BVDC_Handle is openned with BVDC_Open
 *
 * Note: assume parameter eSourceId are valid for gfx feeder
 */
BERR_Code BVDC_P_GfxFeeder_Create
    ( BVDC_P_GfxFeeder_Handle         *phGfxFeeder,
      BREG_Handle                      hRegister,
      BRDC_Handle                      hRdc,
      BAVC_SourceId                    eGfdId,
      bool                             b3dSrc,
      BVDC_Source_Handle               hSource)
{
    BVDC_P_GfxFeederContext *pGfxFeeder = NULL;
#ifdef BCHP_GFD_0_HW_CONFIGURATION
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
    uint32_t  ulHwCfg;
#endif
#endif

    /* Use: to see messages */
    /*BDBG_SetModuleLevel("BVDC_GFX", BDBG_eMsg);*/

    BDBG_ENTER(BVDC_P_GfxFeeder_Create);

    BDBG_ASSERT( NULL != phGfxFeeder );
    BDBG_ASSERT( NULL != hRegister );
    BDBG_ASSERT( NULL != hSource);

    /* The handle will be NULL if create fails */
    if ( NULL == phGfxFeeder )
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    *phGfxFeeder = NULL;

    /* Alloc and clear the gfxFeeder main context */
    pGfxFeeder = (BVDC_P_GfxFeederContext*)(
        BKNI_Malloc(sizeof(BVDC_P_GfxFeederContext)));
    if ( NULL == pGfxFeeder )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset((void*)pGfxFeeder, 0x0, sizeof(BVDC_P_GfxFeederContext));
    BDBG_OBJECT_SET(pGfxFeeder, BVDC_GFX);

    /* initialize stuff that will never change */
    pGfxFeeder->hRegister = hRegister;
    pGfxFeeder->eId = eGfdId;
    pGfxFeeder->hRdc = hRdc;
    pGfxFeeder->b3dSrc = b3dSrc;
    pGfxFeeder->hSource = hSource;

#if BVDC_P_SUPPORT_NEW_SW_INIT
    pGfxFeeder->ulResetRegAddr = BCHP_BMISC_SW_INIT;
    pGfxFeeder->ulResetMask    =
        BCHP_BMISC_SW_INIT_GFD_0_MASK << (eGfdId - BAVC_SourceId_eGfx0);
#else
    pGfxFeeder->ulResetRegAddr = BCHP_BMISC_SOFT_RESET;
    pGfxFeeder->ulResetMask    =
        BCHP_BMISC_SOFT_RESET_GFD_0_MASK << (eGfdId - BAVC_SourceId_eGfx0);
#endif

#if (BVDC_P_MAX_GFX_WINDOWS >= 2)
    /* register offset into which gfxfeeder */
    pGfxFeeder->ulRegOffset =
#if (BVDC_P_MAX_GFX_WINDOWS >= 3)
        (BAVC_SourceId_eGfx2 == pGfxFeeder->eId)?
        (BCHP_GFD_2_REVISION - BCHP_GFD_0_REVISION) :
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 4)
        (BAVC_SourceId_eGfx3 == pGfxFeeder->eId)?
        (BCHP_GFD_3_REVISION - BCHP_GFD_0_REVISION) :
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 5)
        (BAVC_SourceId_eGfx4 == pGfxFeeder->eId)?
        (BCHP_GFD_4_REVISION - BCHP_GFD_0_REVISION) :
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 6)
        (BAVC_SourceId_eGfx5 == pGfxFeeder->eId)?
        (BCHP_GFD_5_REVISION - BCHP_GFD_0_REVISION) :
#endif
#if (BVDC_P_MAX_GFX_WINDOWS >= 7)
        (BAVC_SourceId_eGfx6 == pGfxFeeder->eId)?
        (BCHP_GFD_6_REVISION - BCHP_GFD_0_REVISION) :
#endif
        ((BAVC_SourceId_eGfx1 == pGfxFeeder->eId)?
         (BCHP_GFD_1_REVISION - BCHP_GFD_0_REVISION) : 0);
#else
    pGfxFeeder->ulRegOffset = 0;
#endif

    /* HW feature related to vertical scaling */
    pGfxFeeder->ulVertLineBuf = 0;
    pGfxFeeder->bSupportVertScl = false;

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)

#ifdef BCHP_GFD_0_HW_CONFIGURATION
    /* See also SW7366-379 and SW7445-810 */
#if ((BCHP_CHIP==7445) && (BCHP_VER >= BCHP_VER_D0) && \
     (BCHP_CHIP==7445) && (BCHP_VER <= BCHP_VER_E0))
    if(BAVC_SourceId_eGfx0 == pGfxFeeder->eId)
    {
        ulHwCfg =
            BCHP_FIELD_ENUM(GFD_0_HW_CONFIGURATION, VSCL,           SUPPORTED) |
            BCHP_FIELD_ENUM(GFD_0_HW_CONFIGURATION, DCXG,           SUPPORTED) |
            BCHP_FIELD_DATA(GFD_0_HW_CONFIGURATION, VSCL_LSBF_SIZE, BVDC_P_GFD_4K_VSCL_LINE_BUFFER);
    }
    else if(pGfxFeeder->eId >= BAVC_SourceId_eGfx3)
    {
        ulHwCfg =
            BCHP_FIELD_ENUM(GFD_0_HW_CONFIGURATION, VSCL,           SUPPORTED) |
            BCHP_FIELD_ENUM(GFD_0_HW_CONFIGURATION, DCXG,           NOT_SUPPORTED) |
            BCHP_FIELD_DATA(GFD_0_HW_CONFIGURATION, VSCL_LSBF_SIZE, BVDC_P_GFD_VSCL_LINE_BUFFER);
    }
    else
    {
        ulHwCfg =
            BCHP_FIELD_ENUM(GFD_0_HW_CONFIGURATION, DCXG,           NOT_SUPPORTED) |
            BCHP_FIELD_ENUM(GFD_0_HW_CONFIGURATION, VSCL,           NOT_SUPPORTED) |
            BCHP_FIELD_DATA(GFD_0_HW_CONFIGURATION, VSCL_LSBF_SIZE, 0);
    }
#else
    ulHwCfg = BREG_Read32(pGfxFeeder->hRegister,
        BCHP_GFD_0_HW_CONFIGURATION + pGfxFeeder->ulRegOffset);
#endif

    pGfxFeeder->bSupportVertScl = BVDC_P_GET_FIELD(
        ulHwCfg, GFD_0_HW_CONFIGURATION, VSCL);

#ifdef BCHP_GFD_0_HW_CONFIGURATION_NL_MA_CSC_PRESENT_SHIFT
    pGfxFeeder->bSupportMACsc = BVDC_P_GET_FIELD(
        ulHwCfg, GFD_0_HW_CONFIGURATION, NL_MA_CSC_PRESENT);
#endif
#ifdef BCHP_GFD_0_HW_CONFIGURATION_NL_LUT_PRESENT_SHIFT
    pGfxFeeder->bSupportNLCsc = BVDC_P_GET_FIELD(
        ulHwCfg, GFD_0_HW_CONFIGURATION, NL_LUT_PRESENT);
#endif
#ifdef BCHP_GFD_0_HW_CONFIGURATION_HDR_VERSION_SHIFT
    if (BAVC_SourceId_eGfx0 == pGfxFeeder->eId)
    {
        pGfxFeeder->bSupportEotfConv = true;
    }
#endif
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_6)
    pGfxFeeder->ulVertLineBuf =
        BVDC_P_GET_FIELD(ulHwCfg, GFD_0_HW_CONFIGURATION, VSCL_LSBF_SIZE);
#else
    pGfxFeeder->ulVertLineBuf =
        pGfxFeeder->bSupportVertScl? BVDC_P_GFD_VSCL_LINE_BUFFER : 0;
#endif
#else
    pGfxFeeder->bSupportVertScl = (pGfxFeeder->eId == BAVC_SourceId_eGfx0);
    pGfxFeeder->ulVertLineBuf   =
        pGfxFeeder->bSupportVertScl? BVDC_P_GFD_VSCL_LINE_BUFFER : 0;
#endif
#endif /* (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3) */

    /* init stuff in surface manager */
    pGfxFeeder->stGfxSurface.hRegister = hRegister;
    pGfxFeeder->stGfxSurface.eSrcId = eGfdId;
    pGfxFeeder->stGfxSurface.ulHwMainSurAddrReg =
        BCHP_GFD_0_SRC_START + pGfxFeeder->ulRegOffset;

    /* allocate shadow registers for surface manager */
    BVDC_P_GfxSurface_AllocShadowRegs(
        &pGfxFeeder->stGfxSurface, pGfxFeeder->hRdc, b3dSrc);

    *phGfxFeeder = (BVDC_P_GfxFeeder_Handle) pGfxFeeder;

    /* Some debug print */
    BDBG_MSG(("GFD[%d] ulVertLineBuf   = %d", pGfxFeeder->eId, pGfxFeeder->ulVertLineBuf));
    BDBG_MSG(("GFD[%d] bSupportVertScl = %d", pGfxFeeder->eId, pGfxFeeder->bSupportVertScl));

    BDBG_LEAVE(BVDC_P_GfxFeeder_Create);
    return BERR_SUCCESS;
}

/*************************************************************************
 * {private}
 *
 * Called by BVDC_P_Source_Destroy to destroy gfx feeder specific context
 * when BVDC_Handle is closed with BVDC_Close
 */
BERR_Code BVDC_P_GfxFeeder_Destroy
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder )
{
    BDBG_ENTER(BVDC_P_GfxFeeder_Destroy);
    BDBG_OBJECT_ASSERT(hGfxFeeder, BVDC_GFX);

    /* free surface address shadow registers back to scratch pool */
    BVDC_P_GfxSurface_FreeShadowRegs(
        &hGfxFeeder->stGfxSurface, hGfxFeeder->hRdc);

    BDBG_OBJECT_DESTROY(hGfxFeeder, BVDC_GFX);

    BKNI_Free((void*)hGfxFeeder);

    BDBG_LEAVE(BVDC_P_GfxFeeder_Destroy);

    return BERR_SUCCESS;
}


/*--------------------------------------------------------------------------
 * {private}
 *
 */
static void BVDC_P_GfxFeeder_SetAllDirty(
    BVDC_P_GfxDirtyBits             *pDirty )
{
    int  iGammaTable, iPaletteTable;

    /* every thing need reset at init except tables */
    iGammaTable = pDirty->stBits.bGammaTable;
    iPaletteTable = pDirty->stBits.bPaletteTable;
    BVDC_P_SET_ALL_DIRTY(pDirty);
    pDirty->stBits.bGammaTable = iGammaTable;
    pDirty->stBits.bPaletteTable = iPaletteTable;
}

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_Init
 *
 * initialize stuff that will change after destory and re-create. It also
 * allocate surface address shadow registers. We don't want to allocate them
 * until the GFD is really going to be used.
 */
void BVDC_P_GfxFeeder_Init(
    BVDC_P_GfxFeeder_Handle          hGfxFeeder )
{
    BDBG_ENTER(BVDC_P_GfxFeeder_Init);
    BDBG_OBJECT_ASSERT(hGfxFeeder, BVDC_GFX);

    /* set defaults. */
    hGfxFeeder->pNewSur = NULL;
    BKNI_Memset((void*)&hGfxFeeder->stNewCfgInfo, 0x0, sizeof(BVDC_P_GfxFeederCfgInfo));
    BKNI_Memset((void*)&hGfxFeeder->stCurCfgInfo, 0x0, sizeof(BVDC_P_GfxFeederCfgInfo));
    hGfxFeeder->stNewCfgInfo.stFlags.bEnGfdHwAlphaPreMultiply = 1;
    hGfxFeeder->stNewCfgInfo.eHorzScaleCoeffs = BVDC_FilterCoeffs_eAuto;
    hGfxFeeder->stNewCfgInfo.eVertScaleCoeffs = BVDC_FilterCoeffs_eAuto;

#if (BVDC_P_SUPPORT_OLD_SET_ALPHA_SUR)
    BKNI_Memset((void*)&hGfxFeeder->stTmpNewAvcPic, 0x0, sizeof(BAVC_Gfx_Picture));
    BKNI_Memset((void*)&hGfxFeeder->stTmpIsrAvcPic, 0x0, sizeof(BAVC_Gfx_Picture));
#endif

    BVDC_P_GfxFeeder_SetAllDirty(&hGfxFeeder->stNewCfgInfo.stDirty);

    /* init GFD HW block when gfd source is created by user */
    hGfxFeeder->ulInitVsyncCntr = BVDC_P_GFX_INIT_CNTR;

    /* init surface manager */
    BVDC_P_GfxSurface_Init(&hGfxFeeder->stGfxSurface);

    BVDC_P_GfxFeeder_InitColorMatrix(hGfxFeeder);

    BDBG_LEAVE(BVDC_P_GfxFeeder_Init);
    return;
}

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_GetAdjSclOutRect_isr
 *
 * Called by BVDC_P_Window_ApplyChanges to get the adjusted scal-out rect
 * as it SetSurfaceSize in compositor,
 * It should match the design of BVDC_P_GfxFeeder_ValidateSurAndRects_isrsafe
 */
BERR_Code BVDC_P_GfxFeeder_GetAdjSclOutRect_isr
    ( const BVDC_P_ClipRect           *pClipRect,            /* in */
      const BVDC_P_Rect               *pSclOutRect,          /* in */
      const BVDC_P_Rect               *pDstRect,             /* in */
      BVDC_P_Rect                     *pAdjSclOutRect )      /* out */
{
    BSTD_UNUSED(pClipRect);
    BSTD_UNUSED(pSclOutRect);

    pAdjSclOutRect->lTop = 0;
    pAdjSclOutRect->lLeft = 0;
    pAdjSclOutRect->ulWidth = pDstRect->ulWidth;
    pAdjSclOutRect->ulHeight = pDstRect->ulHeight;

    return BERR_SUCCESS;
}

/* old chip GFD have U14.0 src pitch, some newer ones have S14.0 pitch */
#define BVDC_P_GFD_SRC_PITCH_MAX  BVDC_P_MAX(BCHP_GFD_0_SRC_PITCH_PITCH_MASK>>1, 0x3FFF)

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_ValidateSurAndRects_isrsafe
 *
 * It validates the combination of clip, scaler-out and dest rectangles
 * and surface size, and records the adjusted clip and output info and
 * scale related intermediate values
 *
 * It could be called by BVDC_P_GfxFeeder_ValidateChanges in user mode,
 * or during RUL build for isr surface.
 *
 * If it is called from ValidateChanges, pCfg is &hGfxFeeder->stNewCfgInfo,
 * therefore change to *pCfg is stored in hGfxFeeder->stNewCfgInfo, it will
 * not affect RUL build before GfxFeeder_ApplyChanges is called, and is ok
 * even if ValidateChanges failed in some other modules.
 *
 * If it is called from BuildRul, we must have a new surface set in _isr
 * mode, as long as we passed the error check at the first part of this
 * func, the new surface will be used. So it is right to change *pCfg
 * (i.e. hGfxFeeder->stCurCfgInfo), so that RUL build is affected right
 * now.
 */
static BERR_Code BVDC_P_GfxFeeder_ValidateSurAndRects_isrsafe
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder,
      BVDC_P_SurfaceInfo              *pSur, /* new or isr */
      BVDC_P_GfxFeederCfgInfo         *pCfg, /* new or cur */
      const BVDC_P_ClipRect           *pClipRect,
      const BVDC_P_Rect               *pSclOutRect,
      const BVDC_P_Rect               *pDstRect)
{
    BVDC_P_GfxFeederCfgInfo  *pCurCfg;
    BVDC_P_GfxDirtyBits  stDirty;
    BVDC_P_SurfaceInfo  *pCurSur;
    uint32_t  ulCntWidth, ulOutWidth, ulCntHeight, ulOutHeight;
    uint32_t  ulAdjCntWidth, ulAdjCntLeft, ulAdjCntHeight, ulAdjCntTopInt;
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
    uint32_t  ulOutOrientLR;
    uint32_t  ulVsclSrcStep, ulBlkAvgSize;
    uint32_t  ulCntTop, ulVsclInitPhase, ulVsclInitPhaseBot;
#endif
    uint32_t  ulGfdPitch;
    bool      bSource3d;
    bool      bAvoidPitchOverflow = false;

    /* Note: since this func could be called during BuildRul, which
     * would pass CurCfg in, hence pCfg->ulCnt* should not be updated until
     * all error cheking is completed.
     * Note: the combination of scale-out with dst rect and canvas is checked
     * by BVDC_P_Window_ValidateChanges */

    /* check if clut is used by both palette and gamma correction */
    if ( (BPXL_IS_PALETTE_FORMAT(pSur->eInputPxlFmt)) &&
         (pCfg->stFlags.bEnableGammaCorrection) )
    {
        return BERR_TRACE(BVDC_ERR_GFX_CLUT_REUSE);
    }

    /* check the combination of main surface with clip rectange */
    if ( ((pClipRect->ulLeft + pClipRect->ulRight)  > pSur->ulWidth) ||
         ((pClipRect->ulTop  + pClipRect->ulBottom) > pSur->ulHeight) )
    {
        return BERR_TRACE(BVDC_ERR_GFX_SRC_OVER_CLIP);
    }

    /* check the combination of clip and scale-out */
    ulCntWidth  = (pSur->ulWidth  - (pClipRect->ulLeft + pClipRect->ulRight));
    ulCntHeight = (pSur->ulHeight - (pClipRect->ulTop  + pClipRect->ulBottom));
    ulOutWidth  = pSclOutRect->ulWidth;
    ulOutHeight = pSclOutRect->ulHeight;

    BDBG_ASSERT(ulOutWidth);
    BDBG_ASSERT(ulOutHeight);

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
    if(hGfxFeeder->bSupportVertScl)
    {
        if ((0 == ulOutWidth) || (0 == ulCntWidth) ||
            (8 * ulCntWidth < ulOutWidth) || (ulCntWidth > 8 * ulOutWidth))
        {
            return BERR_TRACE(BVDC_ERR_GFX_HSCL_OUT_OF_RANGE);
        }
        if ((0 == ulOutHeight) || (0 == ulCntHeight) ||
            (8 * ulCntHeight < ulOutHeight) || (ulCntHeight > 12 * ulOutHeight))
        {
            return BERR_TRACE(BVDC_ERR_GFX_VSCL_OUT_OF_RANGE);
        }
    }
    else
    {
        if ( ulCntHeight < ulOutHeight )
        {
            return BERR_TRACE(BVDC_ERR_GFX_VERTICAL_SCALE);
        }
        if ( (0 == ulCntWidth) && (0 != ulOutWidth) )
        {
            return BERR_TRACE(BVDC_ERR_GFX_INFINITE_SCALE);
        }
    }
#else
    if ( ulCntHeight < ulOutHeight )
    {
        return BERR_TRACE(BVDC_ERR_GFX_VERTICAL_SCALE);
    }
    if ( (0 == ulCntWidth) && (0 != ulOutWidth) )
    {
        return BERR_TRACE(BVDC_ERR_GFX_INFINITE_SCALE);
    }
#endif /* #if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3) */

#if (BVDC_P_SUPPORT_GFD_VER_0 == BVDC_P_SUPPORT_GFD1_VER)
    if ((BAVC_SourceId_eGfx1 == hGfxFeeder->eId) &&
        ((ulOutWidth != ulCntWidth) || (ulOutHeight != ulCntHeight)))
    {
        return BERR_TRACE(BVDC_ERR_GFX_SUR_SIZE_MISMATCH);
    }
#endif

    ulAdjCntWidth = (pDstRect->ulWidth * ulCntWidth + (ulOutWidth - 1)) / ulOutWidth;
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
    if(pCfg->stFlags.bNeedVertScale)
    {
        /* we will do vertical scale */
        ulOutOrientLR = (pCfg->eOutOrientation==BFMT_Orientation_e3D_LeftRight)? 1 : 0;
        if (((ulAdjCntWidth     << ulOutOrientLR) > hGfxFeeder->ulVertLineBuf)
            &&
            ((pDstRect->ulWidth << ulOutOrientLR) > hGfxFeeder->ulVertLineBuf))
        {
            /* See also HW7425-385 for additonal information. */
            BDBG_ERR(("GFD[%d] line buffer length cannot be larger than internal HW line buffer (%d) when VSCL enabled",
                hGfxFeeder->eId - BAVC_SourceId_eGfx0, hGfxFeeder->ulVertLineBuf));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
#endif

    bSource3d = BVDC_P_ORIENTATION_IS_3D(pCfg->eInOrientation);
    if( bSource3d && (!hGfxFeeder->b3dSrc))
    {
        BDBG_ERR(("GFD[%d] usage does not support 3D", hGfxFeeder->eId-BAVC_SourceId_eGfx0));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    ulGfdPitch = (pCfg->stFlags.bInterlaced)? 2 * pSur->ulPitch : pSur->ulPitch;
    if (ulGfdPitch > BVDC_P_GFD_SRC_PITCH_MAX)
    {
        if (hGfxFeeder->bSupportVertScl && (pSur->ulPitch <= BVDC_P_GFD_SRC_PITCH_MAX))
            bAvoidPitchOverflow = true;
        else
        {
            BDBG_ERR(("GFD[%d] src pitch 0x%x overflow",
                      hGfxFeeder->eId-BAVC_SourceId_eGfx0, ulGfdPitch));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    /* after this point, no more error is possible */

    /* note: dest cut is xfered into src clip to save bandwidth; CntLeft and SrcStep
     * are fixed point number with precision provided by HW; cntWidth is rounded to
     * ceiling */
    pCfg->ulHsclSrcStep =
        BVDC_P_MIN(GFD_MAX_FIR_STEP,
            (ulCntWidth << GFD_NUM_BITS_FIR_STEP_FRAC) / ulOutWidth);
    ulAdjCntLeft =
        (pClipRect->ulLeft << GFD_NUM_BITS_FIR_INIT_PHASE_FRAC) +
        (pSclOutRect->lLeft * ulCntWidth << GFD_NUM_BITS_FIR_INIT_PHASE_FRAC) / ulOutWidth;
    /* ulAdjCntWidth = (pDstRect->ulWidth * ulCntWidth + (ulOutWidth - 1)) / ulOutWidth;
     * would not be outside of original src size */

    /* OutWidth and ulOutHeight are used to set GFD_0_DISP_PIC_SIZE */
    pCfg->ulOutWidth  = pDstRect->ulWidth;

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
    /* note: dest cut is xfered into src clip to save bandwidth; CntTop and SrcStep
     * are fixed point number with precision provided by HW; cntHeight is rounded to
     * ceiling */
    if(hGfxFeeder->bSupportVertScl)
    {
        if (ulCntHeight != ulOutHeight)
        {
            ulVsclSrcStep = (ulCntHeight << GFD_VSCL_NUM_BITS_FIR_STEP_FRAC) / ulOutHeight;
            ulCntTop =
                (pClipRect->ulTop << GFD_VSCL_NUM_BITS_FIR_INIT_PHASE_FRAC) +
                (pSclOutRect->lTop * ulCntHeight << GFD_VSCL_NUM_BITS_FIR_INIT_PHASE_FRAC) / ulOutHeight;
            ulAdjCntHeight = (pDstRect->ulHeight * ulCntHeight + ulOutHeight - 1) / ulOutHeight;
            BDBG_P_GFD_MSG(("vld ajust ulCntTop = 0x%x and %d/64 height = %d",
                            ulCntTop>>6, ulCntTop & 63, ulAdjCntHeight));
            /* would not be outside of orig src size */
        }
        else /* VSCL is turned off */
        {
            ulVsclSrcStep = GFD_VSCL_FIR_STEP_1;
            ulCntTop = (pClipRect->ulTop + pSclOutRect->lTop) <<
                GFD_VSCL_NUM_BITS_FIR_INIT_PHASE_FRAC;
            ulAdjCntHeight = ulCntHeight;
        }

        /* vertical init phase for frame/top-filed and bottom-field
         * note: vertical phase can be decided according to surface width, height,
         * clip-rect, out size, but horizontal init phase also depends on the
         * surface pixel format */
        ulAdjCntTopInt = ulCntTop >> GFD_VSCL_NUM_BITS_FIR_INIT_PHASE_FRAC;
        ulVsclInitPhase = ulCntTop & (GFD_VSCL_FIR_PHASE_1 - 1);
        BDBG_P_GFD_MSG(("orig InitPhase 0x%x and %d/64", ulVsclInitPhase >> 6, ulVsclInitPhase & 63));
        ulVsclInitPhase += GFD_VSCL_FIR_STEP_TO_PHASE(ulVsclSrcStep >> 1);
        BDBG_P_GFD_MSG(("after +srcStep/2, InitPhase 0x%x and %d/64", ulVsclInitPhase >> 6, ulVsclInitPhase & 63));
        ulVsclInitPhase = (ulVsclInitPhase > (GFD_VSCL_FIR_PHASE_1 >> 1))?
            (ulVsclInitPhase - (GFD_VSCL_FIR_PHASE_1 >> 1)) : 0;
        BDBG_P_GFD_MSG(("after -1/2, InitPhase 0x%x and %d/64", ulVsclInitPhase >> 6, ulVsclInitPhase & 63));
        ulVsclInitPhaseBot = ulVsclInitPhase + GFD_VSCL_FIR_STEP_TO_PHASE(ulVsclSrcStep);

        /* block average for vscl is needed if srcStep is bigger than vscl tap number */
        ulBlkAvgSize = 0;
        if (ulVsclSrcStep > GFD_VSCL_BLK_AVG_THD)
        {
            while((ulBlkAvgSize < GFD_VSCL_MAX_BLK_AVG_SIZE) &&
                  (ulVsclSrcStep > GFD_VSCL_BLK_AVG_THD * (ulBlkAvgSize + 1)))
            {
                ++ulBlkAvgSize;
            }
            ulVsclSrcStep /= (ulBlkAvgSize + 1);
            ulVsclInitPhase /= (ulBlkAvgSize + 1);
            ulVsclInitPhaseBot /= (ulBlkAvgSize + 1);
        }
        pCfg->ulVsclBlkAvgSize = ulBlkAvgSize;
        pCfg->ulVsclSrcStep = ulVsclSrcStep;
        pCfg->ulVsclInitPhase = ulVsclInitPhase;
        pCfg->ulVsclInitPhaseBot = ulVsclInitPhaseBot;
    }
    else
    {
        pCfg->ulVsclBlkAvgSize = 0;
        pCfg->ulVsclSrcStep = GFD_VSCL_FIR_STEP_1;
        ulAdjCntTopInt = pClipRect->ulTop + pSclOutRect->lTop;
        ulAdjCntHeight = pDstRect->ulHeight; /* not used */
    }
#else
    pCfg->ulVsclBlkAvgSize = 0;
    pCfg->ulVsclSrcStep = GFD_VSCL_FIR_STEP_1;
    ulAdjCntTopInt = pClipRect->ulTop + pSclOutRect->lTop;
    ulAdjCntHeight = pDstRect->ulHeight; /* not used */
#endif /* #if (BVDC_P_SUPPORT_GFD_VER == BVDC_P_SUPPORT_GFD_VER_3)   */

    /* OutWidth and ulOutHeight are used to set GFD_0_DISP_PIC_SIZE */
    pCfg->ulOutHeight = pDstRect->ulHeight;

    pCfg->stFlags.bNeedHorizScale =
        (GFD_HSCL_FIR_STEP_1 != pCfg->ulHsclSrcStep) ||
        (BVDC_FilterCoeffs_eSharp == pCfg->eHorzScaleCoeffs);
    pCfg->stFlags.bNeedVertScale =
        ((hGfxFeeder->bSupportVertScl) &&
         ((GFD_VSCL_FIR_STEP_1 != pCfg->ulVsclSrcStep) ||
          (0 != pCfg->ulVsclBlkAvgSize) ||
          (bAvoidPitchOverflow)));
#if BVDC_P_GFX_INIT_WORKAROUND
    pCfg->stFlags.bNeedHorizScale |= ((pCfg->eOutOrientation==BFMT_Orientation_e3D_LeftRight)? 1 : 0);
    pCfg->stFlags.bNeedVertScale  |= ((pCfg->eOutOrientation==BFMT_Orientation_e3D_OverUnder)? 1 : 0);
#endif
    pCfg->stFlags.bNeedColorSpaceConv = (!hGfxFeeder->hWindow->stSettings.bBypassVideoProcessings)? 1 : 0;
    pCfg->stFlags.bEnDecompression = BPXL_IS_COMPRESSED_FORMAT(pSur->eInputPxlFmt)? 1 : 0;
    pCfg->stFlags.bNeedVertScale  |= (pCfg->stFlags.bEnDecompression && pCfg->stFlags.bInterlaced)? 1 : 0;

    /* set the dirty bits due to combined surface or configure change
     * note: pCfg could be NewCfg or CurCfg */
    stDirty = pCfg->stDirty;
    pCurCfg = &(hGfxFeeder->stCurCfgInfo);
    pCurSur = &(hGfxFeeder->stGfxSurface.stCurSurInfo);

    if (pSur->eInputPxlFmt != pCurSur->eInputPxlFmt)
    {
        stDirty.stBits.bPxlFmt = true;
        if (BPXL_IS_COMPRESSED_FORMAT(pSur->eInputPxlFmt) != BPXL_IS_COMPRESSED_FORMAT(pCurSur->eInputPxlFmt))
        {
            stDirty.stBits.bCompress = true;
        }
    }

    if (pSur->stAvcPic.eInOrientation != pCurSur->stAvcPic.eInOrientation)
    {
        stDirty.stBits.bOrientation = BVDC_P_DIRTY;
    }

    if ( (ulAdjCntLeft   != pCurCfg->ulCntLeft)  ||
         (ulAdjCntTopInt != pCurCfg->ulCntTopInt) ||
         (ulAdjCntWidth  != pCurCfg->ulCntWidth) ||
         (ulAdjCntHeight != pCurCfg->ulCntHeight) )
    {
        /* note: dest clip is also transfered into src clip */
        stDirty.stBits.bSrcClip   = BVDC_P_DIRTY;
        stDirty.stBits.bClipOrOut = BVDC_P_DIRTY;
    }

    if ( (stDirty.stBits.bPxlFmt) ||
         (stDirty.stBits.bSrcClip) ||
         (pSur->ulPitch != pCurSur->ulPitch))
    {
        stDirty.stBits.bSurOffset = BVDC_P_DIRTY;
    }

    /* since pCfg could be pCurCfg, we can not store new cnt rect values
     * to pCfg until the comparing of those values in pCfg and pCurCfg are
     * done */
    pCfg->ulCntLeft = ulAdjCntLeft;
    pCfg->ulCntWidth = ulAdjCntWidth;
    pCfg->ulCntTopInt = ulAdjCntTopInt;
    pCfg->ulCntHeight = ulAdjCntHeight;

    pCfg->stDirty = stDirty;

    return BERR_SUCCESS;
}

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_ValidateChanges
 *
 * To be called by BVDC_ApplyChanges, to check whether there is conflict
 * between settings related to gfx feeder.
 *
 */
BERR_Code BVDC_P_GfxFeeder_ValidateChanges
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder,
      BVDC_Source_PictureCallback_isr  pfPicCallbackFunc )
{
    BVDC_P_SurfaceInfo  *pNewSur;
    BVDC_P_GfxFeederCfgInfo  *pNewCfg, *pCurCfg;
    BVDC_P_GfxCfgFlags stCurFlags;
    BVDC_P_GfxDirtyBits *pNewDirty;
    const BVDC_P_ClipRect  *pNewClip;
    const BVDC_P_Rect  *pNewDst, *pNewSclOut;
    BVDC_BlendFactor  eSrcFactor, eDstFactor;
    uint8_t  ucConstant;
    bool  bInterlaced;
    BVDC_P_Window_Info *pWinNewInfo, *pWinCurInfo;
    BVDC_P_DisplayInfo *pDispNewInfo, *pDispCurInfo;
    BFMT_Orientation  eOrientation, eSrcOrientation;
    BVDC_Window_Handle  hWindow;
    BERR_Code eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_GfxFeeder_ValidateChanges);
    BDBG_OBJECT_ASSERT(hGfxFeeder, BVDC_GFX);

    if (NULL == hGfxFeeder->hWindow)
    {
        /* this gfd src is not used by gfx window yet */
        return BERR_SUCCESS;
    }

    hWindow = hGfxFeeder->hWindow;
    pNewCfg = &(hGfxFeeder->stNewCfgInfo);
    pCurCfg = &(hGfxFeeder->stCurCfgInfo);
    stCurFlags = pCurCfg->stFlags;
    pNewDirty = &pNewCfg->stDirty;
    pWinNewInfo = &hWindow->stNewInfo;
    pWinCurInfo = &hWindow->stCurInfo;
    pDispNewInfo = &hWindow->hCompositor->hDisplay->stNewInfo;
    pDispCurInfo = &hWindow->hCompositor->hDisplay->stCurInfo;

    /* whether user set new surface, isr set surface, or current surface
     * will be used to be validated against configure ? */
    if ( true == hGfxFeeder->stGfxSurface.stNewSurInfo.bChangeSurface )
    {
        pNewSur = &(hGfxFeeder->stGfxSurface.stNewSurInfo);
    }
    else if ( true == hGfxFeeder->stGfxSurface.stIsrSurInfo.bChangeSurface )
    {
        pNewSur = &(hGfxFeeder->stGfxSurface.stIsrSurInfo);
    }
    else /* no new user or isr set to surface, contunue to use cur surface */
    {
        BAVC_Gfx_Picture     stAvcPic;
        const BPXL_Plane    *pSurface;
        uint32_t             ulPitch, ulSurOffset, ulWidth, ulHeight;
        BPXL_Format          ePxlFmt;
        bool                 bOrientationOverride;
        BVDC_P_SurfaceInfo  *pCurSur = &(hGfxFeeder->stGfxSurface.stCurSurInfo);

        stAvcPic = hGfxFeeder->stGfxSurface.stCurSurInfo.stAvcPic;
        /* get info from main surface */
        pSurface = stAvcPic.pSurface;
        ulPitch = pSurface->ulPitch;
        ulWidth = pSurface->ulWidth;
        ulHeight = pSurface->ulHeight;
        ePxlFmt = pSurface->eFormat;
        ulSurOffset = pCurSur->ulAddress;
        ulSurOffset += pSurface->ulPixelsOffset;

        hGfxFeeder->stGfxSurface.stTempSurInfo = hGfxFeeder->stGfxSurface.stCurSurInfo;
        pNewSur = &hGfxFeeder->stGfxSurface.stTempSurInfo;

        bOrientationOverride =
            hGfxFeeder->hSource->stNewInfo.bOrientationOverride ^
            hGfxFeeder->hSource->stCurInfo.bOrientationOverride;

        eSrcOrientation = hGfxFeeder->hSource->stNewInfo.bOrientationOverride?
            hGfxFeeder->hSource->stNewInfo.eOrientation :
            pNewSur->stAvcPic.eInOrientation;

        if((bOrientationOverride)&&
            (false == hGfxFeeder->hSource->stNewInfo.bOrientationOverride))
        {
            pNewSur->ulHeight = ulHeight;
            pNewSur->ulWidth  = ulWidth;

            pNewDirty->stBits.bSurOffset = BVDC_P_DIRTY;
            pNewSur->bChangeSurface = true;
        }

        if((hGfxFeeder->hSource->stNewInfo.bOrientationOverride) &&
            BVDC_P_ORIENTATION_IS_3D(eSrcOrientation))
        {
            /* only 2D surface allows to be override orientation */
            BDBG_ASSERT(stAvcPic.eInOrientation==BFMT_Orientation_e2D);

            pNewSur->ulHeight = ulHeight >> (BFMT_Orientation_e3D_OverUnder == eSrcOrientation);
            pNewSur->ulWidth  = ulWidth  >> (BFMT_Orientation_e3D_LeftRight == eSrcOrientation);
            pNewSur->ulRAddress = ulSurOffset +
                (BFMT_Orientation_e3D_OverUnder == eSrcOrientation) * pNewSur->ulHeight* ulPitch +
                (BFMT_Orientation_e3D_LeftRight == eSrcOrientation) * pNewSur->ulWidth * BPXL_BITS_PER_PIXEL(ePxlFmt)/8;

            pNewDirty->stBits.bSurOffset = BVDC_P_DIRTY;
            pNewSur->bChangeSurface = true;
        }
    }

    hGfxFeeder->pNewSur = pNewSur;
    pNewCfg->bOrientationOverride = hGfxFeeder->hSource->stNewInfo.bOrientationOverride;
    pNewCfg->eInOrientation       = pNewCfg->bOrientationOverride ?
        hGfxFeeder->hSource->stNewInfo.eOrientation: pNewSur->stAvcPic.eInOrientation;

    /* prepare to check the combination of the three rect and surface size,
     * note: the combination of scale-out with dst rect and canvas is checked
     * by BVDC_P_Window_ValidateChanges */
    BVDC_P_Window_GetNewScanType( hWindow, &bInterlaced );
    BVDC_P_Window_GetNewRectangles( hWindow, &pNewClip, &pNewSclOut, &pNewDst );
    BVDC_P_Window_GetNewWindowAlpha(hWindow, &(pNewCfg->ucWindowAlpha) );
    BVDC_P_Window_GetNewDispOrientation(hWindow, &eOrientation);
    pNewCfg->eOutOrientation = eOrientation;

    /* eOrientation conflicts with input pixel format? */
#if BVDC_P_GFX_INIT_WORKAROUND
    if ((BPXL_eA0 == pNewSur->eInputPxlFmt ||
         BPXL_eP0 == pNewSur->eInputPxlFmt) &&
        (BFMT_Orientation_e3D_OverUnder == eOrientation) &&
        (NULL == pfPicCallbackFunc))
    {
        BDBG_ERR(("A_0 and P_0 support 2D + 3D LR only!"));
        return BERR_TRACE((BERR_INVALID_PARAMETER));
    }
#endif

    /* validate surface size and rectangles, adjust rectangles and do some
     * scale related computation. The intermediate values are stored in
     * pNewCfg, and will not affect RUL build until ApplyChanges is called
     */
    pNewCfg->stFlags.bInterlaced = (bInterlaced)? 1 : 0;
    eResult = BVDC_P_GfxFeeder_ValidateSurAndRects_isrsafe(
        hGfxFeeder, pNewSur, pNewCfg, pNewClip, pNewSclOut, pNewDst);
    if ((BERR_SUCCESS != eResult) && (NULL == pfPicCallbackFunc))
    {
        return BERR_TRACE(eResult);
    }

    /* check the combination of window blending factor, gfd color key and
     * window alpha
     * Note: equation balance is already checked as setting blending factor,
     * refer to BVDC_P_GfxFeeder_ValidateBlend for detail
     * Note: bit filed access is slow if it is not in cpu register */
    BVDC_P_Window_GetNewBlendFactor(
        hWindow, &eSrcFactor, &eDstFactor, &ucConstant );
    if ( (BVDC_BlendFactor_eSrcAlpha         == eSrcFactor) &&
         (BVDC_BlendFactor_eOneMinusSrcAlpha == eDstFactor) )
    {
        /* case 1): user means that alpha pre-multiply has NOT been
         * performed in graphics source surface, but src alpha should be
         * used to blend. We should internally turn on alpha pre-multiply
         * in gfd HW, and change SrcFactor to use constant 1.0. This is
         * because the alpha output from color key stage is always
         * pre-multiplied to pixel color by HW. The blending setting adjust
         * is done in BVDC_P_GfxFeeder_AdjustBlend */
        pNewCfg->stFlags.bEnGfdHwAlphaPreMultiply = 1;
        pNewCfg->stFlags.bConstantBlending = 0;
    }
    else if ( (BVDC_BlendFactor_eConstantAlpha    == eSrcFactor) &&
              (BVDC_BlendFactor_eOneMinusSrcAlpha == eDstFactor) &&
              (GFX_ALPHA_FULL                     == ucConstant) )
    {
        /* Case 2): user means that alpha pre-multiply has been performed
         * in graphics source surface. */
        pNewCfg->stFlags.bEnGfdHwAlphaPreMultiply = 0;
        pNewCfg->stFlags.bConstantBlending = 0;
    }
    else
    {
        /* Case 3): User means to ignore src pixel alpha and to use
         * constant for blending if it is not case 1) or 2). Since
         * in HW the two-way-choice alpha output from key stage is
         * always  multiplied to pixel color after the pixel-alpha-pre-
         * multiply stage, case 3) must have 0xff output from color key.
         * Therefore we should turn off colorkey INTERNALLY and set key
         * default aplha to be 0xff. That is OK, because from API level
         * the pixel alpha meant to be ignored anyway */
        pNewCfg->stFlags.bEnGfdHwAlphaPreMultiply = 0;
        pNewCfg->stFlags.bConstantBlending = 1;
    }

    if ( pNewCfg->stFlags.bInterlaced != stCurFlags.bInterlaced )
    {
        pNewDirty->stBits.bScanType  = BVDC_P_DIRTY;
        pNewDirty->stBits.bOutRect   = BVDC_P_DIRTY;
        pNewDirty->stBits.bClipOrOut = BVDC_P_DIRTY;
    }

    if ( (pNewCfg->ulOutWidth  != pCurCfg->ulOutWidth) ||
         (pNewCfg->ulOutHeight != pCurCfg->ulOutHeight)||
         (pNewCfg->ulCntWidth  != pCurCfg->ulCntWidth)||
         (pNewCfg->ulCntHeight != pCurCfg->ulCntHeight))
    {
        pNewDirty->stBits.bOutRect   = BVDC_P_DIRTY;
        pNewDirty->stBits.bClipOrOut = BVDC_P_DIRTY;
    }

    if(( pNewCfg->eOutOrientation != pCurCfg->eOutOrientation ) ||
        (pNewCfg->eInOrientation  != pCurCfg->eInOrientation))
    {
        pNewDirty->stBits.bOrientation = BVDC_P_DIRTY;
    }

    if ( (pNewCfg->ucWindowAlpha             != pCurCfg->ucWindowAlpha) ||
         (pNewCfg->stFlags.bConstantBlending != stCurFlags.bConstantBlending) )
    {
        pNewDirty->stBits.bKey = BVDC_P_DIRTY;
    }

    /* set picture adjust dirty bit if picture adjustment values
     * have changed */
    if ((hWindow->hCompositor->hDisplay->stCurInfo.stHdmiSettings.stSettings.eEotf != hWindow->hCompositor->hDisplay->stNewInfo.stHdmiSettings.stSettings.eEotf) ||
        (pNewCfg->stFlags.bConstantBlending != pCurCfg->stFlags.bConstantBlending))
    {
        pNewDirty->stBits.bSdrGfx2HdrAdj = BVDC_P_DIRTY;
    }
    if((pNewDirty->stBits.bSdrGfx2HdrAdj) ||
       (pNewCfg->stFlags.bConstantBlending != pCurCfg->stFlags.bConstantBlending) ||
       (pWinNewInfo->stDirty.stBits.bCscAdjust) ||
       (pWinNewInfo->sHue                  != pWinCurInfo->sHue           ) ||
       (pWinNewInfo->sContrast             != pWinCurInfo->sContrast      ) ||
       (pWinNewInfo->sBrightness           != pWinCurInfo->sBrightness    ) ||
       (pWinNewInfo->sSaturation           != pWinCurInfo->sSaturation    ) ||
       (pWinNewInfo->sColorTemp            != pWinCurInfo->sColorTemp     ) ||
       (pWinNewInfo->lAttenuationR         != pWinCurInfo->lAttenuationR  ) ||
       (pWinNewInfo->lAttenuationG         != pWinCurInfo->lAttenuationG  ) ||
       (pWinNewInfo->lAttenuationB         != pWinCurInfo->lAttenuationB  ) ||
       (pWinNewInfo->lOffsetR              != pWinCurInfo->lOffsetR       ) ||
       (pWinNewInfo->lOffsetG              != pWinCurInfo->lOffsetG       ) ||
       (pWinNewInfo->lOffsetB              != pWinCurInfo->lOffsetB       ) ||
       (pWinNewInfo->bCscRgbMatching       != pWinCurInfo->bCscRgbMatching) ||
       (pDispNewInfo->bXvYcc               != pDispCurInfo->bXvYcc)         ||
       (pDispNewInfo->eHdmiOutput          != pDispCurInfo->eHdmiOutput)    ||
       (pDispNewInfo->stHdmiSettings.stSettings.eEotf != pDispCurInfo->stHdmiSettings.stSettings.eEotf)||
       (pDispNewInfo->stDirty.stBits.bTiming))
    {
        pNewDirty->stBits.bCsc = BVDC_P_DIRTY;
    }

    /* set dirty bit if coefficient index changed in window */
    if(pWinNewInfo->stDirty.stBits.bCtIndex)
    {
        pNewDirty->stBits.bScaleCoeffs = BVDC_P_DIRTY;
    }

#if BVDC_P_GFX_INIT_WORKAROUND
    if((eOrientation != pCurCfg->eOutOrientation)||
        (pNewCfg->eInOrientation != pCurCfg->eInOrientation))
    {
        hGfxFeeder->ulInitVsyncCntr = BVDC_P_GFX_INIT_CNTR;
        BVDC_P_GfxFeeder_SetAllDirty (pNewDirty);
    }
#endif

    BDBG_LEAVE(BVDC_P_GfxFeeder_ValidateChanges);
    return BERR_SUCCESS;
}

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_ApplyChanges_isr
 *
 * To be called by BVDC_ApplyChanges, to copy "new user state" to "current
 * state", after validation of all VDC modules passed.
 *
 */
BERR_Code BVDC_P_GfxFeeder_ApplyChanges_isr
    ( BVDC_P_GfxFeeder_Handle     hGfxFeeder )
{
    BVDC_P_GfxFeederCfgInfo  *pNewCfg, *pCurCfg;
    BVDC_P_SurfaceInfo  *pNewSur, *pCurSur;

    BDBG_ENTER(BVDC_P_GfxFeeder_ApplyChanges_isr);
    BDBG_OBJECT_ASSERT(hGfxFeeder, BVDC_GFX);

    if (NULL == hGfxFeeder->hWindow)
    {
        /* this gfd src is not used by gfx window yet */
        return BERR_SUCCESS;
    }

    pNewCfg = &(hGfxFeeder->stNewCfgInfo);
    pCurCfg = &(hGfxFeeder->stCurCfgInfo);
    pCurSur = &(hGfxFeeder->stGfxSurface.stCurSurInfo);
    pNewSur = hGfxFeeder->pNewSur; /* decided by ValidateChanges */

    /* Copy NewCfg to CurCfg if there is diff between cur and new Proc */
    if ( pNewCfg->stDirty.aulInts[0] )
    {
        /* Note: Current and new Dirty must be ORed in case that more
         *       than one ApplyChange called before RUL is built
         * Note: After NewCfg is copied to CurCfg, new Dirty should be set to
         *       0 to indicate that there is no diff between cur and new cfg
         */
        BVDC_P_OR_ALL_DIRTY(&pNewCfg->stDirty, &pCurCfg->stDirty);
        *pCurCfg = *pNewCfg;

        BVDC_P_CLEAN_ALL_DIRTY(&pNewCfg->stDirty);
    }
    else
    {
        /* might still need to copy Flags, since some of its bits changing does
         * not set dirty bits */
        pCurCfg->stFlags = pNewCfg->stFlags;
    }

    /* Copy NewSur to curSur if bChangeSurface.
     * Note: pNewSur could be _isr set Surface,
     * Note: pNewSur could be NULL before a window is connected to the src */
    if ( NULL != pNewSur && pNewSur->bChangeSurface && pNewSur != pCurSur)
    {
        /* pCurSur->bChangeSurface would stay true so that the surface change
         * is built into RUL later */
        *pCurSur = *pNewSur;

        /* to avoid future validation if surface no longer changes */
        pNewSur->bChangeSurface  = false;

        /* any previous set IsrSur should no longer be used */
        hGfxFeeder->stGfxSurface.stIsrSurInfo.bChangeSurface = false;
    }

    BDBG_LEAVE(BVDC_P_GfxFeeder_ApplyChanges_isr);
    return  BERR_SUCCESS;
}

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_AbortChanges
 *
 * Cancel the user settings that either fail to validate or simply
 * because user wish to abort the changes in mid-way.
 */
void BVDC_P_GfxFeeder_AbortChanges
    ( BVDC_P_GfxFeeder_Handle     hGfxFeeder )
{
    BVDC_P_GfxFeederCfgInfo  *pNewCfg;

    BDBG_ENTER(BVDC_P_GfxFeeder_AbortChanges);

    /* validate paramters */
    BDBG_OBJECT_ASSERT(hGfxFeeder, BVDC_GFX);

    /* */
    pNewCfg = &(hGfxFeeder->stNewCfgInfo);

    /* copy CurCfg back to NewCfg, including Flags */
    *pNewCfg = hGfxFeeder->stCurCfgInfo;
    BVDC_P_CLEAN_ALL_DIRTY(&(pNewCfg->stDirty));

    /* any user set surface should not be used again */
    hGfxFeeder->stGfxSurface.stNewSurInfo.bChangeSurface = false;

    BDBG_LEAVE(BVDC_P_GfxFeeder_AbortChanges);
    return;
}

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_BuildRulForUserChangeOnly_isr
 *
 * Builds RUL for user configure change
 *
 * designed to be called by BVDC_P_GfxFeeder_BuildRul_isr only, no paramter
 * validation performed here
 */
static BERR_Code BVDC_P_GfxFeeder_BuildRulForUserChangeOnly_isr
    ( BVDC_P_GfxFeederCfgInfo       *pCurCfg,
      uint32_t                     **ppulRulCur,
      uint32_t                       ulRulOffset )
{
    BERR_Code  eResult = BERR_SUCCESS;
    uint32_t  *pulRulCur;
    uint32_t  ulOutWidth, ulOutHeight;
    uint32_t  ulClutAddr, ulClutSize;
    uint8_t  ucWinAlpha;
    BVDC_P_GfxDirtyBits  stDirty = pCurCfg->stDirty;

    /* init RUL buffer pointers */
    pulRulCur = *ppulRulCur;

    /* set RUL for output size */
    if ( stDirty.stBits.bOutRect )
    {
        ulOutWidth  = pCurCfg->ulOutWidth;
        ulOutHeight = (pCurCfg->stFlags.bInterlaced)?
            pCurCfg->ulOutHeight / 2 :  pCurCfg->ulOutHeight;
        *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_DISP_PIC_SIZE ) + ulRulOffset;
        *pulRulCur++ =
            BCHP_FIELD_DATA( GFD_0_DISP_PIC_SIZE, HSIZE, ulOutWidth ) |
            BCHP_FIELD_DATA( GFD_0_DISP_PIC_SIZE, VSIZE, ulOutHeight);
    }

    /* set RUL for loading gamma correction table */
    if ( stDirty.stBits.bGammaTable )
    {
        ulClutAddr = pCurCfg->ulGammaClutAddress;
        ulClutSize = pCurCfg->ulNumGammaClutEntries;

        /* set the addr and size for table loading */
        *pulRulCur++ = BRDC_OP_IMMS_TO_REGS( GFD_NUM_REGS_PALETTE );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_PALETTE_START ) + ulRulOffset;
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_PALETTE_START, ADDR, ulClutAddr );
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_PALETTE_SIZE,  SIZE, ulClutSize );

        /* triger the table loading */
        *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_LOAD_PALETTE ) + ulRulOffset;
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_LOAD_PALETTE, LOAD_PALETTE, 1);
    }

    /* set RUL for color key */
    if ( stDirty.stBits.bKey )
    {
        ucWinAlpha = (pCurCfg->stFlags.bConstantBlending) ?
            GFX_ALPHA_FULL: pCurCfg->ucWindowAlpha;

        *pulRulCur++ = BRDC_OP_IMMS_TO_REGS( GFD_NUM_REGS_KEY );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_KEY_MAX ) + ulRulOffset;
        *pulRulCur++ = pCurCfg->ulKeyMaxAMNO;
        *pulRulCur++ = pCurCfg->ulKeyMinAMNO;
        *pulRulCur++ = pCurCfg->ulKeyMaskAMNO;
        *pulRulCur++ =
            BCHP_FIELD_DATA( GFD_0_KEY_ALPHA, DEFAULT_ALPHA, ucWinAlpha ) |
            BCHP_FIELD_DATA( GFD_0_KEY_ALPHA, KEY_ALPHA,     pCurCfg->ucKeyedAlpha );
    }

    /* set constant color for alpha-only pixels */
    if ( stDirty.stBits.bConstantColor )
    {
        *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_WIN_COLOR ) + ulRulOffset;
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_WIN_COLOR, WIN_COLOR, pCurCfg->ulConstantColor );
    }

    /* reset RUL buffer pointer */
    *ppulRulCur = pulRulCur;

    return BERR_TRACE(eResult);
}

/*------------------------------------------------------------------------
 *  {secret}
 *  BVDC_P_GfxFeeder_CalcSurfaceOffset_isr
 *
 *  Called during BuildRul, after validating the combination of surafce
 *  and graphics feeder configure
 */
static BERR_Code BVDC_P_GfxFeeder_CalcSurfaceOffset_isr
    ( BVDC_P_GfxFeeder_Handle             hGfxFeeder )
{
    BVDC_P_SurfaceInfo   *pCurSur;
    BVDC_P_GfxFeederCfgInfo  *pCurCfg;
    uint32_t  ulCntLeft, ulFirInitPhase;
    uint32_t  ulBitsPerPixel, ulOffsetBitsInLine, ulOffsetByteInLine;

    BDBG_OBJECT_ASSERT(hGfxFeeder, BVDC_GFX);

    pCurCfg = &(hGfxFeeder->stCurCfgInfo);
    pCurSur = &(hGfxFeeder->stGfxSurface.stCurSurInfo);

    ulFirInitPhase = pCurCfg->ulCntLeft & GFD_MASK_FIR_INIT_PHASE_FRAC;
    ulCntLeft = pCurCfg->ulCntLeft >> GFD_NUM_BITS_FIR_INIT_PHASE_FRAC;
    ulBitsPerPixel = BPXL_BITS_PER_PIXEL(pCurSur->eInputPxlFmt);
    ulOffsetBitsInLine = ulBitsPerPixel * ulCntLeft;
    ulOffsetByteInLine = ulOffsetBitsInLine / GFD_NUM_BITS_PER_BYTE;
    if ( (0 < ulBitsPerPixel) && (ulBitsPerPixel < GFD_NUM_BITS_PER_BYTE) )
    {
        /* ulOffsetPixInByte is the extra number of pixels to skip in after GFD
         * input for sub-byte format like P_1, P_2, P_4.
         * note:  use 'if' to avoid the expensive '/', it is typically not needed */
        hGfxFeeder->ulOffsetPixInByte =
            (ulOffsetBitsInLine - ulOffsetByteInLine * GFD_NUM_BITS_PER_BYTE) / ulBitsPerPixel;
    }
    else
    {
        hGfxFeeder->ulOffsetPixInByte = 0;
    }

    if ( BPXL_IS_YCbCr422_FORMAT(pCurSur->eInputPxlFmt) && (ulCntLeft & 0x1) )
    {
        /* need even pixel allignment for Y0PrY1Pb format */
        ulOffsetByteInLine -= 2; /* two byte per YCrCb422 pixel */
        ulFirInitPhase += (1 << GFD_NUM_BITS_FIR_INIT_PHASE_FRAC);
    }

    hGfxFeeder->ulFirInitPhase = ulFirInitPhase;
    hGfxFeeder->stGfxSurface.ulMainByteOffset =
        (BPXL_IS_COMPRESSED_FORMAT(pCurSur->eInputPxlFmt))? 0 :
        pCurCfg->ulCntTopInt * pCurSur->ulPitch + ulOffsetByteInLine;

    return BERR_SUCCESS;
}

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_BuildRulForSurCtrl_isr
 *
 * Builds RUL for surface related state changes.
 *
 * designed to be called by BVDC_P_GfxFeeder_BuildRul_isr only, no paramter
 * validation performed here
 */
static BERR_Code BVDC_P_GfxFeeder_BuildRulForSurCtrl_isr
    ( BVDC_P_GfxFeeder_Handle            hGfxFeeder,
      BAVC_Polarity                      eFieldId,
      uint32_t                         **ppulRulCur,
      uint32_t                           ulRulOffset )
{
    BERR_Code  eResult = BERR_SUCCESS;
    BVDC_P_GfxSurfaceContext  *pGfxSurface;
    BVDC_P_SurfaceInfo   *pCurSur;
    BVDC_P_GfxFeederCfgInfo  *pCurCfg;
    BVDC_P_GfxDirtyBits  stDirty;
    BVDC_P_GfxCfgFlags  stFlags;
    uint32_t  *pulRulCur;
    uint32_t  ulMainSurPitch;
    uint32_t  ulOffsetPixInByte;
    bool  bChangeClipOrField;
    uint32_t  ulFirStepLow, ulFirStepInt;
    uint32_t  *pulCoeffs;
    uint32_t  ulAddrShadowReg0, ulAddrShadowReg1;
    uint32_t  ulRAddrShadowReg0, ulRAddrShadowReg1;

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
    uint32_t  ulEnDejag, ulEnDering, ulEnDemoMode;
    uint32_t  ulEnVscl, ulFilterOrder;
    uint32_t  ulVsclFirStep;
    uint32_t  *pulVsclCoeffs;
    uint32_t  ulCntHeight;
#endif
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4)
    uint32_t  ulAddrReg;
#endif

    BDBG_ENTER(BVDC_P_GfxFeeder_BuildRulForSurCtrl_isr);
    BDBG_OBJECT_ASSERT(hGfxFeeder, BVDC_GFX);

    /* init RUL buffer pointers */
    pulRulCur = *ppulRulCur;

    pGfxSurface = &hGfxFeeder->stGfxSurface;
    pCurCfg = &(hGfxFeeder->stCurCfgInfo);
    pCurSur = &(pGfxSurface->stCurSurInfo);
    stDirty = pCurCfg->stDirty;
    stFlags = pCurCfg->stFlags;
    bChangeClipOrField =
        stDirty.stBits.bSrcClip || stDirty.stBits.bScanType || stFlags.bInterlaced;

    /* (1) set RUL for scale coeffs
     * note: stDirty.stBits.bClipOrOut will be set if clipRect, out size, or surface size change */
    if ( (stDirty.stBits.bScaleCoeffs | stDirty.stBits.bClipOrOut) && hGfxFeeder->hWindow )
    {
        const BVDC_CoefficientIndex *pCtIndex = &hGfxFeeder->hWindow->stCurInfo.stCtIndex;

        /* set horizontal scale coeffs */
        BVDC_P_GfxFeeder_DecideFilterCoeff_isr(pCurCfg->eHorzScaleCoeffs,
            pCtIndex->ulSclHorzLuma, pCurCfg->ulCntWidth, pCurCfg->ulOutWidth, &pulCoeffs );
        *pulRulCur++ = BRDC_OP_IMMS_TO_REGS( GFD_NUM_REGS_HSCL_COEFF );
        *pulRulCur++ = BRDC_REGISTER(BCHP_GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01) + ulRulOffset;
        BKNI_Memcpy( (void*) pulRulCur, (void*) pulCoeffs, 4 * GFD_NUM_REGS_HSCL_COEFF );
        pulRulCur += GFD_NUM_REGS_HSCL_COEFF;

        /* set vertical scale coeffs */
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
        if(hGfxFeeder->bSupportVertScl)
        {
            BVDC_P_GfxFeeder_DecideVsclFirCoeff_isr(pCurCfg->eVertScaleCoeffs,
                pCtIndex->ulSclVertLuma, pCurCfg->ulCntHeight, pCurCfg->ulOutHeight, &pulVsclCoeffs );
            *pulRulCur++ = BRDC_OP_IMMS_TO_REGS( GFD_NUM_REGS_VSCL_COEFF );
            *pulRulCur++ = BRDC_REGISTER(BCHP_GFD_0_VERT_FIR_COEFF_PHASE0_00_01) + ulRulOffset;
            BKNI_Memcpy( (void*) pulRulCur, (void*) pulVsclCoeffs, 4 * GFD_NUM_REGS_VSCL_COEFF );
            pulRulCur += GFD_NUM_REGS_VSCL_COEFF;
        }
#endif
    }

    /* (2) set RUL for other scale configures related to scale factors,
     * note: stDirty.stBits.bClipOrOut will be set if clipRect, out size, or surface size change
     */
    if ( stDirty.stBits.bClipOrOut || stDirty.stBits.bDemoMode )
    {
        /* set horizontal scale factor */
        ulFirStepLow = pCurCfg->ulHsclSrcStep & GFD_MASK_FIR_STEP_LOW;
        ulFirStepInt = ((pCurCfg->ulHsclSrcStep & GFD_MASK_FIR_STEP_INT) >>
                        GFD_SHIFT_FIR_STEP_INT);
        *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_HORIZ_FIR_SRC_STEP ) + ulRulOffset;
        *pulRulCur++ =
            BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_SRC_STEP,   STEP,     ulFirStepLow ) |
            BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_SRC_STEP,   STEP_INT, ulFirStepInt );

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
        /* dering/dejag features come with vertical scaling capacity */
        if(hGfxFeeder->bSupportVertScl)
        {
            /* turn on dering if horizontal scale factor is bigger than 2/3, alpha clip */
            ulEnDering =
                ( stFlags.bNeedHorizScale &&
                ((2 * pCurCfg->ulHsclSrcStep) <= (3 * GFD_HSCL_FIR_STEP_1)))? 1 : 0;
            ulEnDemoMode = (pCurCfg->stFlags.bDeringDemoMode)? 1 : 0;
            *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_DERINGING ) + ulRulOffset;
            *pulRulCur++ =
                BCHP_FIELD_DATA( GFD_0_DERINGING, HORIZ_ALPHA_DERINGING,  ulEnDering ) |
                BCHP_FIELD_DATA( GFD_0_DERINGING, HORIZ_CHROMA_DERINGING, ulEnDering ) |
                BCHP_FIELD_DATA( GFD_0_DERINGING, HORIZ_LUMA_DERINGING,   ulEnDering ) |
                BCHP_FIELD_DATA( GFD_0_DERINGING, DEMO_MODE,              ulEnDemoMode );

            /* set vertical scale factor and block average, and filter order
             */

            /* pCurCfg->ulVsclSrcStep has been adjusted according to block average */
            ulVsclFirStep = pCurCfg->ulVsclSrcStep;
            *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_VERT_FIR_SRC_STEP ) + ulRulOffset;
            *pulRulCur++ = (stFlags.bInterlaced)?
                BCHP_FIELD_DATA( GFD_0_VERT_FIR_SRC_STEP, SIZE, 2 * ulVsclFirStep ) :
                BCHP_FIELD_DATA( GFD_0_VERT_FIR_SRC_STEP, SIZE, ulVsclFirStep );

            *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_SRC_VSIZE ) + ulRulOffset;

            ulCntHeight = (stFlags.bNeedVertScale || !pCurCfg->stFlags.bInterlaced) ?
                pCurCfg->ulCntHeight :  pCurCfg->ulCntHeight / 2;
            *pulRulCur++ =
                BCHP_FIELD_DATA( GFD_0_SRC_VSIZE, VSIZE, ulCntHeight );

            /* turn on dejag if vertical scale factor is bigger than 1*/
            ulEnVscl = (stFlags.bNeedVertScale)? 1: 0;
            ulEnDejag =
                 (ulEnVscl &&
                 (pCurCfg->ulVsclSrcStep <= GFD_VSCL_FIR_STEP_1) &&
                 (0 == pCurCfg->ulVsclBlkAvgSize))? 1 : 0;
            ulEnDemoMode = (pCurCfg->stFlags.bDejagDemoMode)? 1 : 0;
            *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_DEJAGGING ) + ulRulOffset;
            *pulRulCur++ =
                BCHP_FIELD_DATA( GFD_0_DEJAGGING, HORIZ,           0)          |
                BCHP_FIELD_DATA( GFD_0_DEJAGGING, GAIN,            0)          |
                BCHP_FIELD_DATA( GFD_0_DEJAGGING, CORE,            0)          |
                BCHP_FIELD_DATA( GFD_0_DEJAGGING, VERT_DEJAGGING,  ulEnDejag ) |
                BCHP_FIELD_DATA( GFD_0_DEJAGGING, DEMO_MODE,       ulEnDemoMode );

            /* set gfd scale top configure */
            ulFilterOrder = (pCurCfg->ulHsclSrcStep < GFD_HSCL_FIR_STEP_1)?
                BCHP_FIELD_ENUM( GFD_0_GSCL_CFG, FILTER_ORDER, VERT_FIRST ) :
                BCHP_FIELD_ENUM( GFD_0_GSCL_CFG, FILTER_ORDER, HORIZ_FIRST );
            *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_GSCL_CFG ) + ulRulOffset;
            *pulRulCur++ =
                BCHP_FIELD_DATA( GFD_0_GSCL_CFG, BAVG_BLK_SIZE, pCurCfg-> ulVsclBlkAvgSize ) |
                BCHP_FIELD_DATA( GFD_0_GSCL_CFG, HCLIP_ENABLE,  1 )                          |
                BCHP_FIELD_DATA( GFD_0_GSCL_CFG, VCLIP_ENABLE,  1 )                          |
                ulFilterOrder                                                                |
                BCHP_FIELD_DATA( GFD_0_GSCL_CFG, IOBUF_ENABLE,  ulEnVscl )                   |
                BCHP_FIELD_DATA( GFD_0_GSCL_CFG, VSCL_ENABLE,   ulEnVscl )                   |
                BCHP_FIELD_DATA( GFD_0_GSCL_CFG, GSCL_ENABLE,   1 );
            /* TODO set demo_setting */
        }
#endif
    }

    /* (3) set BRDC_Variable_3 = (bot)? 0xFFFFFFFF: 0x0 */
    if ((hGfxFeeder->hWindow) && (eFieldId != BAVC_Polarity_eFrame))
    {
        /* BRDC_Variable_0 holds polarity */
        eResult = BVDC_P_Display_GetFieldPolarity_isr(hGfxFeeder->hWindow->hCompositor->hDisplay, &pulRulCur, eFieldId);
        if (eResult != BERR_SUCCESS)
            return BERR_TRACE(eResult);

        /* BRDC_Variable_3 holds factor to determine if pitch is to be applied for the next field */
        *pulRulCur++ = BRDC_OP_VAR_SUM_IMM_TO_VAR(BRDC_Variable_0, BRDC_Variable_3);
        *pulRulCur++ = 0xFFFFFFFF; /* 0x0->0xFFFFFFFF, 0x1->0x0 */
        *pulRulCur++ = BRDC_OP_NOT_VAR_TO_VAR(BRDC_Variable_3, BRDC_Variable_3);
    }
    else
    {
        *pulRulCur++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_3);
        *pulRulCur++ = 0;
    }

    /* (4) set RUL for main size, pitch, offset and scl initial phase
     * note: stDirty.stBits.bClipOrOut will be set if clipRect, out size, or surface size
     * change */
    if ( bChangeClipOrField || stDirty.stBits.bSurOffset )
    {
        if ( stDirty.stBits.bSurOffset )
        {
            BVDC_P_GfxFeeder_CalcSurfaceOffset_isr(hGfxFeeder);

#if (BVDC_P_SUPPORT_GFD_VER <  BVDC_P_SUPPORT_GFD_VER_4)
            *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_HORIZ_FIR_INIT_PHASE ) + ulRulOffset;
            *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_INIT_PHASE, PHASE, hGfxFeeder->ulFirInitPhase );
#else
            /* 3D support */
            *pulRulCur++ = BRDC_OP_IMMS_TO_REGS(2);
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_HORIZ_FIR_INIT_PHASE ) + ulRulOffset;
            *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_INIT_PHASE,   PHASE, hGfxFeeder->ulFirInitPhase );
            *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_INIT_PHASE_R, PHASE, hGfxFeeder->ulFirInitPhase );
#endif
        }

        /* if ClipRect or Surface changes, more likely some thing in the following
         * will change, therefore no need to comppare */
        ulOffsetPixInByte = hGfxFeeder->ulOffsetPixInByte;
        ulMainSurPitch =
#if defined(BCHP_GFD_0_CTRL_BSTC_ENABLE_ON)
            (stFlags.bEnDecompression)? 4 * pCurSur->ulPitch :
#endif
            (stFlags.bNeedVertScale || !stFlags.bInterlaced)? pCurSur->ulPitch : 2 * pCurSur->ulPitch;
        *pulRulCur++ = BRDC_OP_IMMS_TO_REGS( GFD_NUM_REGS_SRC );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_SRC_OFFSET ) + ulRulOffset;
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_SRC_OFFSET, BLANK_PIXEL_COUNT, ulOffsetPixInByte );
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_SRC_HSIZE,  HSIZE, pCurCfg->ulCntWidth );
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_SRC_START,  ADDR,  0 ); /* set later */

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4)
        /* 3D support */
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_SRC_START_R, ADDR, 0);
#endif
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_SRC_PITCH,  PITCH, ulMainSurPitch );

        /* vertical init phase will be diff for top and bot field, even if surface
         * and clip-rect and output rect has no change */
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
        if(hGfxFeeder->bSupportVertScl)
        {
            /* set vertical scl init phase  */
            if (BAVC_Polarity_eFrame == eFieldId)
            {
                *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
                *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_VERT_FIR_INIT_PHASE ) + ulRulOffset;
                *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_VERT_FIR_INIT_PHASE, PHASE, pCurCfg->ulVsclInitPhase );
            }
            else /* setup for RUL to automatically adjust vertical init phase according to polarity */
            {
                /* (BRDC_Variable_3 & ulVsclInitPhaseBot) | (~BRDC_Variable_3 & ulVsclInitPhaseBot) */
                *pulRulCur++ = BRDC_OP_VAR_AND_IMM_TO_VAR(BRDC_Variable_3, BRDC_Variable_0);
                *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_VERT_FIR_INIT_PHASE, PHASE, pCurCfg->ulVsclInitPhaseBot );
                *pulRulCur++ = BRDC_OP_NOT_VAR_TO_VAR(BRDC_Variable_3, BRDC_Variable_2);
                *pulRulCur++ = BRDC_OP_VAR_AND_IMM_TO_VAR(BRDC_Variable_2, BRDC_Variable_1);
                *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_VERT_FIR_INIT_PHASE, PHASE, pCurCfg->ulVsclInitPhase );
                *pulRulCur++ = BRDC_OP_VAR_OR_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_0);

                *pulRulCur++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_0);
                *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_VERT_FIR_INIT_PHASE ) + ulRulOffset;
            }
        }
#endif
    }

    /* ---------------------------------------------------------------------
     * (6) send new surface to address shadow registers.
     * note: we only want to call SetShadowRegs_isr once for this surface setting
     * even if this RUL get lost, . So we test pCurSur->bChangeSurface, rather than
     * stDirty.stBits.bSurface*/
    if (pCurSur->bChangeSurface || stDirty.stBits.bSurOffset)
    {
        BVDC_P_GfxSurface_SetShadowRegs_isr(&hGfxFeeder->stGfxSurface, pCurSur, hGfxFeeder->hSource);
    }

    /* ---------------------------------------------------------------------
     * (7) setup RUL to pick one set of surface address shadow registers.
     *
     * After this RUL is built, gfx surface might change with SetSurface, we
     * want the change showing up as soon as possible. SetSurface will put the
     * surface address to a shadow register, and here we pre-build the RUL to
     * copy surface address value from shadow register to GFD.
     *
     * In order to activate left and right surface atomically, we use an index
     * register, and ping-pong buffered left addr and right addr register
     * pairs.  The index indicates the left/right surface addr in which pair
     * should be used. The following is the algorithm:
     *
     * v_3 <- ulBottomOffset
     *
     * v_0  <-  index
     * v_1 = v_0 & left_start_1
     *
     * v_0  <-  ~index
     * v_2 = v_0 & left_start_0
     * v_2 = v_1 | v_2
     * BCHP_GFD_0_SRC_START_ADDR = v_2 + v3
     *
     * similarly for right surface
     */
    /* BRDC_Variable_3 now (bot)? 0xFFFFFFFF : 0,
     * and then holds ulBottomOffset for main  surfce */
    if ((eFieldId == BAVC_Polarity_eFrame) || (stFlags.bNeedVertScale) ||
        BPXL_IS_COMPRESSED_FORMAT(pCurSur->eInputPxlFmt))
    {
        /* in these cases, GFD always fetch frame */
        *pulRulCur++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_3);
        *pulRulCur++ = 0;
    }
    else
    {
        /* set RUL to auto adjust ulBottomOffset according to polarity */
        *pulRulCur++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_0);
        *pulRulCur++ = pCurSur->ulPitch;
        *pulRulCur++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_3, BRDC_Variable_3);
    }

    if (pGfxSurface->b3dSrc)
    {
        ulAddrShadowReg0  = BRDC_REGISTER(pGfxSurface->ulSurAddrReg[0]);
        ulAddrShadowReg1  = BRDC_REGISTER(pGfxSurface->ulSurAddrReg[1]);
        ulRAddrShadowReg0 = BRDC_REGISTER(pGfxSurface->ulRSurAddrReg[0]);
        ulRAddrShadowReg1 = BRDC_REGISTER(pGfxSurface->ulRSurAddrReg[1]);

        /* BRDC_Variable_0 = pGfxSurface->ulRegIdxReg  (0xffffffff or 0x0) */
        *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
        *pulRulCur++ = BRDC_REGISTER(pGfxSurface->ulRegIdxReg);
        *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_1);
        *pulRulCur++ = ulAddrShadowReg1;
        *pulRulCur++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_1);

        *pulRulCur++ = BRDC_OP_NOT_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_0);
        *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_2);
        *pulRulCur++ = ulAddrShadowReg0;
        *pulRulCur++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_2, BRDC_Variable_2);

        *pulRulCur++ = BRDC_OP_VAR_OR_VAR_TO_VAR (BRDC_Variable_1, BRDC_Variable_2, BRDC_Variable_2);

        /* BRDC_Variable_3 = ulBottomOffset */
        *pulRulCur++ = BRDC_OP_VAR_SUM_VAR_TO_VAR(BRDC_Variable_2, BRDC_Variable_3, BRDC_Variable_2);

        *pulRulCur++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_SRC_START ) + ulRulOffset;

        /* right surface and alpha surface are never used at the same time
         * and they share shadow registers */

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4)
        ulAddrReg = BRDC_REGISTER( BCHP_GFD_0_SRC_START_R);
#endif

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4)
        if(BVDC_P_ORIENTATION_IS_3D(pCurCfg->eInOrientation))
        {
            *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
            *pulRulCur++ = BRDC_REGISTER(pGfxSurface->ulRegIdxReg);
            *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_1);
            *pulRulCur++ = ulRAddrShadowReg1;
            *pulRulCur++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_1);

            *pulRulCur++ = BRDC_OP_NOT_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_0);
            *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_2);
            *pulRulCur++ = ulRAddrShadowReg0;
            *pulRulCur++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_2, BRDC_Variable_2);

            *pulRulCur++ = BRDC_OP_VAR_OR_VAR_TO_VAR (BRDC_Variable_1, BRDC_Variable_2, BRDC_Variable_2);

            /* BRDC_Variable_3 = ulBottomOffset */
            *pulRulCur++ = BRDC_OP_VAR_SUM_VAR_TO_VAR(BRDC_Variable_2, BRDC_Variable_3, BRDC_Variable_2);

            *pulRulCur++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
            *pulRulCur++ = ulAddrReg + ulRulOffset;
        }
        else
        {
            *pulRulCur++ = BRDC_OP_IMM_TO_REG();
            *pulRulCur++ = ulAddrReg + ulRulOffset;
            *pulRulCur++ = 0;
        }
#endif
    }

    /* !(pGfxSurface->b3dSrc) */
    else
    {
        *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
        *pulRulCur++ = BRDC_REGISTER(pGfxSurface->ulSurAddrReg[0]);

        /* BRDC_Variable_3 = ulBottomOffset */
        *pulRulCur++ = BRDC_OP_VAR_SUM_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_3, BRDC_Variable_0);
        *pulRulCur++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_0);
        *pulRulCur++ = BRDC_REGISTER(BCHP_GFD_0_SRC_START) + ulRulOffset;
    }

    /* (10) set RUL to increase VsyncCntr */
    *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_2);
    *pulRulCur++ = BRDC_REGISTER(pGfxSurface->ulVsyncCntrReg);
    *pulRulCur++ = BRDC_OP_VAR_SUM_IMM_TO_VAR(BRDC_Variable_2, BRDC_Variable_2);
    *pulRulCur++ = 1;
    *pulRulCur++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
    *pulRulCur++ = BRDC_REGISTER(pGfxSurface->ulVsyncCntrReg);

    /* reset RUL buffer pointer */
    *ppulRulCur = pulRulCur;

    BDBG_LEAVE(BVDC_P_GfxFeeder_BuildRulForSurCtrl_isr);
    return BERR_TRACE(eResult);
}

/* the following macro must match the HW / RDB spec */
#define GFD_PIXEL_FORMAT_TYPE_ALPHA     5
#define GFD_PIXEL_FORMAT_TYPE_YCRCB422  4
#define GFD_PIXEL_FORMAT_TYPE_PALETTE   3
#define GFD_PIXEL_FORMAT_TYPE_WRGB1565  2
#define GFD_PIXEL_FORMAT_TYPE_WRGB1555  1
#define GFD_PIXEL_FORMAT_TYPE_OTHER     0
/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecidePxlFmtType_isr
 *
 * output: the HW pixel format type of the specified pixel format
 */
static BERR_Code BVDC_P_GfxFeeder_DecidePxlFmtType_isr
    ( BPXL_Format   eMainSurPxlFmt,
      uint32_t     *pulPxlFmtType )
{
    if ( (true  == BPXL_IS_YCbCr_FORMAT(eMainSurPxlFmt)) &&
         (false == BPXL_HAS_ALPHA(eMainSurPxlFmt)) )
    {
        *pulPxlFmtType = GFD_PIXEL_FORMAT_TYPE_YCRCB422;
    }
    else if ( true == BPXL_IS_PALETTE_FORMAT(eMainSurPxlFmt) )
    {
        *pulPxlFmtType = GFD_PIXEL_FORMAT_TYPE_PALETTE;
    }
    else if ((BPXL_eR5_G6_B5 == eMainSurPxlFmt) ||
               (BPXL_eB5_G6_R5 == eMainSurPxlFmt))
    {
        *pulPxlFmtType = GFD_PIXEL_FORMAT_TYPE_WRGB1565;
    }
    else if ( (BPXL_eW1_R5_G5_B5 == eMainSurPxlFmt) ||
              (BPXL_eW1_B5_G5_R5 == eMainSurPxlFmt) ||
              (BPXL_eR5_G5_B5_W1 == eMainSurPxlFmt) ||
              (BPXL_eB5_G5_R5_W1 == eMainSurPxlFmt) )
    {
        *pulPxlFmtType = GFD_PIXEL_FORMAT_TYPE_WRGB1555;
    }
    else if ( true == BPXL_IS_ALPHA_ONLY_FORMAT(eMainSurPxlFmt) )
    {
        *pulPxlFmtType = GFD_PIXEL_FORMAT_TYPE_ALPHA;
    }

    else
    {
        *pulPxlFmtType = GFD_PIXEL_FORMAT_TYPE_OTHER;
    }

    return BERR_SUCCESS;
}

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_BuildRulForColorCtrl_isr
 *
 * Builds RUL for those color related state changes that are not covered
 * by BVDC_P_GfxFeeder_BuildRulForUserChangeOnly_isr
 *
 * designed to be called by BVDC_P_GfxFeeder_BuildRul_isr only, no paramter
 * validation performed here
 */
static BERR_Code BVDC_P_GfxFeeder_BuildRulForColorCtrl_isr
    ( BVDC_P_GfxFeeder_Handle            hGfxFeeder,
      BAVC_Polarity                      eFieldId,
      uint32_t                         **ppulRulCur,
      uint32_t                           ulRulOffset )
{
    BVDC_P_GfxFeederCfgInfo  *pCurCfg = &(hGfxFeeder->stCurCfgInfo);
    BVDC_P_CscCfg  *pGfxCsc = &hGfxFeeder->stGfxCsc;
    BVDC_P_SurfaceInfo  *pCurSur = &(hGfxFeeder->stGfxSurface.stCurSurInfo);
    BERR_Code   eResult = BERR_SUCCESS;
    uint32_t  *pulRulCur;
    uint32_t   ulClutAddr, ulClutSize;
    uint32_t   ulSurFormatType;
    BPXL_Format  eMainPxlFmt;
    BVDC_P_GfxDirtyBits  stDirty = pCurCfg->stDirty;

    /* init RUL buffer pointers */
    pulRulCur = *ppulRulCur;

    /* set RUL for palatte color look up table loading */
    if (stDirty.stBits.bPaletteTable)
    {
        ulClutAddr = pCurSur->ulPaletteAddress;
        ulClutSize = pCurSur->ulPaletteNumEntries;

        /* set the addr and size for table loading */
        *pulRulCur++ = BRDC_OP_IMMS_TO_REGS( GFD_NUM_REGS_PALETTE );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_PALETTE_START ) + ulRulOffset;
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_PALETTE_START, ADDR, ulClutAddr );
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_PALETTE_SIZE,  SIZE, ulClutSize );

        /* triger the table loading */
        *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_LOAD_PALETTE ) + ulRulOffset;
        *pulRulCur++ = BCHP_FIELD_DATA( GFD_0_LOAD_PALETTE, LOAD_PALETTE, 1);
    }

    /* set RUL for pixel format: if alpha is disabled, then its fmt had been
     * set to BPXL_eInvalid */
    if (stDirty.stBits.bPxlFmt)
    {
        eMainPxlFmt = pCurSur->eInputPxlFmt;
        BVDC_P_GfxFeeder_DecidePxlFmtType_isr( eMainPxlFmt, &ulSurFormatType );
        *pulRulCur++ = BRDC_OP_IMMS_TO_REGS( GFD_NUM_REGS_PIXEL_FORMAT );
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_FORMAT_DEF_1 ) + ulRulOffset;
        *pulRulCur++ =
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_1, FORMAT_TYPE,  ulSurFormatType ) |
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_1, CH3_NUM_BITS, BPXL_COMPONENT_SIZE(eMainPxlFmt, 3) ) |
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_1, CH2_NUM_BITS, BPXL_COMPONENT_SIZE(eMainPxlFmt, 2) ) |
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_1, CH1_NUM_BITS, BPXL_COMPONENT_SIZE(eMainPxlFmt, 1) ) |
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_1, CH0_NUM_BITS, BPXL_COMPONENT_SIZE(eMainPxlFmt, 0) );
        *pulRulCur++ =
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_2, CH3_LSB_POS, BPXL_COMPONENT_POS(eMainPxlFmt, 3) ) |
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_2, CH2_LSB_POS, BPXL_COMPONENT_POS(eMainPxlFmt, 2) ) |
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_2, CH1_LSB_POS, BPXL_COMPONENT_POS(eMainPxlFmt, 1) ) |
            BCHP_FIELD_DATA( GFD_0_FORMAT_DEF_2, CH0_LSB_POS, BPXL_COMPONENT_POS(eMainPxlFmt, 0) );
    }

#ifdef BCHP_GFD_0_DCXG_CFG
    /* set RUL for de-compress */
    if (stDirty.stBits.bCompress)
    {
        if (!BPXL_IS_COMPRESSED_FORMAT(pCurSur->eInputPxlFmt))
        {
            *pulRulCur++ = BRDC_OP_IMMS_TO_REGS(
                BVDC_P_REGS_ENTRIES(GFD_0_DCXG_CFG, GFD_0_CROP_SRC_HSIZE));
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_DCXG_CFG + ulRulOffset );
            *pulRulCur++ =
                BCHP_FIELD_ENUM(GFD_0_DCXG_CFG, ENABLE, Disable) |
                BCHP_FIELD_ENUM(GFD_0_DCXG_CFG, APPLY_QERR, Apply_Qerr ) |
                BCHP_FIELD_ENUM(GFD_0_DCXG_CFG, FIXED_RATE, Variable ) |
                BCHP_FIELD_ENUM(GFD_0_DCXG_CFG, COMPRESSION, BPP_16p5 );
            *pulRulCur++ =
                BCHP_FIELD_ENUM(GFD_0_CROP_CFG,  FIELD_SEL, FRAME_OUT) |
                BCHP_FIELD_DATA(GFD_0_CROP_CFG,  SRC_OFFSET_R, 0) |
                BCHP_FIELD_DATA(GFD_0_CROP_CFG,  SRC_OFFSET, 0);
            *pulRulCur++ =
                BCHP_FIELD_DATA(GFD_0_CROP_SRC_HSIZE, HSIZE, 0);
        }
        else /* compressed */
        {
            uint32_t ulFieldSelect = (eFieldId == BAVC_Polarity_eFrame || pCurCfg->stFlags.bNeedVertScale)?
                BCHP_FIELD_ENUM(GFD_0_CROP_CFG,  FIELD_SEL, FRAME_OUT) : (eFieldId == BAVC_Polarity_eTopField)?
                BCHP_FIELD_ENUM(GFD_0_CROP_CFG,  FIELD_SEL, TOP_FIELD_OUT) :
                BCHP_FIELD_ENUM(GFD_0_CROP_CFG,  FIELD_SEL, BOT_FIELD_OUT);
            *pulRulCur++ = BRDC_OP_IMMS_TO_REGS(
                BVDC_P_REGS_ENTRIES(GFD_0_DCXG_CFG, GFD_0_CROP_SRC_HSIZE));
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_DCXG_CFG + ulRulOffset );
            *pulRulCur++ =
                BCHP_FIELD_ENUM(GFD_0_DCXG_CFG, ENABLE, Enable) |
                BCHP_FIELD_ENUM(GFD_0_DCXG_CFG, APPLY_QERR, Apply_Qerr ) |
                BCHP_FIELD_ENUM(GFD_0_DCXG_CFG, FIXED_RATE, Fixed ) |
                BCHP_FIELD_ENUM(GFD_0_DCXG_CFG, COMPRESSION, BPP_16p5 );
            *pulRulCur++ =
                BCHP_FIELD_DATA(GFD_0_CROP_CFG,  FIELD_SEL, ulFieldSelect) |
                BCHP_FIELD_DATA(GFD_0_CROP_CFG,  SRC_OFFSET_R, pCurCfg->ulCntLeft) |
                BCHP_FIELD_DATA(GFD_0_CROP_CFG,  SRC_OFFSET, pCurCfg->ulCntLeft);
            *pulRulCur++ =
                BCHP_FIELD_DATA(GFD_0_CROP_SRC_HSIZE, HSIZE, pCurCfg->ulCntWidth);
        }
    }
#else
    BSTD_UNUSED(eFieldId);
#endif

    /* set RUL for color matrix */
    if (stDirty.stBits.bCsc)
    {
#if (BVDC_P_SUPPORT_GFD_VER_7 <= BVDC_P_SUPPORT_GFD_VER)
            /* SW7445-767 disable gfd dither */
            /* http://twiki-01.broadcom.com/bin/view/Arch/BVNChanges7445d0#BVN_dithering */
            *pulRulCur++ = BRDC_OP_IMM_TO_REG();
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_CSC_DITHER_CTRL) + ulRulOffset;
            *pulRulCur++ =
                BCHP_FIELD_ENUM(GFD_0_CSC_DITHER_CTRL, MODE,       ROUNDING) |
                BCHP_FIELD_ENUM(GFD_0_CSC_DITHER_CTRL, OFFSET_CH2,  DEFAULT) |
                BCHP_FIELD_ENUM(GFD_0_CSC_DITHER_CTRL, SCALE_CH2,   DEFAULT) |
                BCHP_FIELD_ENUM(GFD_0_CSC_DITHER_CTRL, OFFSET_CH1,  DEFAULT) |
                BCHP_FIELD_ENUM(GFD_0_CSC_DITHER_CTRL, SCALE_CH1,   DEFAULT) |
                BCHP_FIELD_ENUM(GFD_0_CSC_DITHER_CTRL, OFFSET_CH0,  DEFAULT) |
                BCHP_FIELD_ENUM(GFD_0_CSC_DITHER_CTRL, SCALE_CH0,   DEFAULT);
#endif

        /* write color elements from the table to rul */
#if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0)

#if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2)
        if (hGfxFeeder->bSupportEotfConv)
        {
            BVDC_P_EotfConvCfg *pEotfConv = &hGfxFeeder->stEotfConv;
            const BVDC_P_CscLRangeAdj *pLRangeAdj;
            const BVDC_P_CscAbCoeffs *pCscAb;

            *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_NL_CSC_CTRL ) + ulRulOffset;
            *pulRulCur++ =
                BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN,  ENABLE)             |
                BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_L2NL,       pEotfConv->ucL2NL)  |
                BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_NL2L,       pEotfConv->ucNL2L)  |
                BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_CL_IN,      pGfxCsc->ucInputCL) |
                BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_XVYCC,      pGfxCsc->ucXvYcc)   |
                BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, NL_CSC,         ENABLE);

            pLRangeAdj = pEotfConv->pLRangeAdj;
            *pulRulCur++ = BRDC_OP_IMMS_TO_REGS( 16 );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_NL_LR_SLOPEi_ARRAY_BASE ) + ulRulOffset;

            *pulRulCur++ = pLRangeAdj->aulLRangeAdjSlope[0];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjSlope[1];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjSlope[2];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjSlope[3];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjSlope[4];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjSlope[5];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjSlope[6];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjSlope[7];

            *pulRulCur++ = pLRangeAdj->aulLRangeAdjXY[0];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjXY[1];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjXY[2];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjXY[3];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjXY[4];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjXY[5];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjXY[6];
            *pulRulCur++ = pLRangeAdj->aulLRangeAdjXY[7];

            pCscAb = pGfxCsc->pCscAbMA;
            *pulRulCur++ = BRDC_OP_IMMS_TO_REGS(
                BVDC_P_REGS_ENTRIES(GFD_0_CSC_R0_MA_COEFF_C00, GFD_0_CSC_R0_MA_COEFF_C24));
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_CSC_R0_MA_COEFF_C00 ) + ulRulOffset;

            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C00, CSC_COEF_MULT, pCscAb->ulY0);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C01, CSC_COEF_MULT, pCscAb->ulY1);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C02, CSC_COEF_MULT, pCscAb->ulY2);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C03, CSC_COEF_MULT, pCscAb->ulYAlpha);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C04, CSC_COEF_ADD,  pCscAb->ulYOffset);

            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C10, CSC_COEF_MULT, pCscAb->ulCb0);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C11, CSC_COEF_MULT, pCscAb->ulCb1);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C12, CSC_COEF_MULT, pCscAb->ulCb2);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C13, CSC_COEF_MULT, pCscAb->ulCbAlpha);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C14, CSC_COEF_ADD,  pCscAb->ulCbOffset);

            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C20, CSC_COEF_MULT, pCscAb->ulCr0);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C21, CSC_COEF_MULT, pCscAb->ulCr1);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C22, CSC_COEF_MULT, pCscAb->ulCr2);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C23, CSC_COEF_MULT, pCscAb->ulCrAlpha);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C24, CSC_COEF_ADD,  pCscAb->ulCrOffset);

            pCscAb = pGfxCsc->pCscAbMB;
            *pulRulCur++ = BRDC_OP_IMMS_TO_REGS(
                BVDC_P_REGS_ENTRIES(GFD_0_CSC_R0_MB_COEFF_C00, GFD_0_CSC_R0_MB_COEFF_C23));
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_CSC_R0_MB_COEFF_C00 ) + ulRulOffset;

            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C00, CSC_COEF_MULT, pCscAb->ulY0);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C01, CSC_COEF_MULT, pCscAb->ulY1);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C02, CSC_COEF_MULT, pCscAb->ulY2);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C03, CSC_COEF_ADD,  pCscAb->ulYOffset);

            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C10, CSC_COEF_MULT, pCscAb->ulCb0);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C11, CSC_COEF_MULT, pCscAb->ulCb1);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C12, CSC_COEF_MULT, pCscAb->ulCb2);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C13, CSC_COEF_ADD,  pCscAb->ulCbOffset);

            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C20, CSC_COEF_MULT, pCscAb->ulCr0);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C21, CSC_COEF_MULT, pCscAb->ulCr1);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C22, CSC_COEF_MULT, pCscAb->ulCr2);
            *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MB_COEFF_C23, CSC_COEF_ADD,  pCscAb->ulCrOffset);
        }
        else

#endif /* #if (BVDC_P_CMP_NON_LINEAR_CSC_VER >= BVDC_P_NL_CSC_VER_2) */

        if (hGfxFeeder->bSupportNLCsc)
        {
#if defined(BCHP_GFD_0_NL_CSC_CTRL_NL_CSC_R0_ENABLE)
            *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
            *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_NL_CSC_CTRL ) + ulRulOffset;
            *pulRulCur++ = (pGfxCsc->ulNLCnv == BVDC_P_NL_CSC_CTRL_SEL_BYPASS) ? 0 :
                (BCHP_FIELD_ENUM( GFD_0_NL_CSC_CTRL, NL_CSC_R0, ENABLE) |
                 BCHP_FIELD_DATA( GFD_0_NL_CSC_CTRL, SEL_CONV_R0, pGfxCsc->ulNLCnv));

            if (hGfxFeeder->bSupportMACsc && (pGfxCsc->ulNLCnv != BVDC_P_NL_CSC_CTRL_SEL_BYPASS))
            {
                const BVDC_P_CscCoeffs *pCscMA = pGfxCsc->pCscMA;
                *pulRulCur++ = BRDC_OP_IMMS_TO_REGS(
                    BVDC_P_REGS_ENTRIES(GFD_0_CSC_R0_MA_COEFF_C00, GFD_0_CSC_R0_MA_COEFF_C24));
                *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_CSC_R0_MA_COEFF_C00 ) + ulRulOffset;
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C00, CSC_COEFF_C0, pCscMA->usY0);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C01, CSC_COEFF_C1, pCscMA->usY1);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C02, CSC_COEFF_C2, pCscMA->usY2);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C03, CSC_COEFF_C3, pCscMA->usYAlpha);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C04, CSC_COEFF_C4, pCscMA->usYOffset);

                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C10, CSC_COEFF_C0, pCscMA->usCb0);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C11, CSC_COEFF_C1, pCscMA->usCb1);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C12, CSC_COEFF_C2, pCscMA->usCb2);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C13, CSC_COEFF_C3, pCscMA->usCbAlpha);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C14, CSC_COEFF_C4, pCscMA->usCbOffset);

                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C20, CSC_COEFF_C0, pCscMA->usCr0);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C21, CSC_COEFF_C1, pCscMA->usCr1);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C22, CSC_COEFF_C2, pCscMA->usCr2);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C23, CSC_COEFF_C3, pCscMA->usCrAlpha);
                *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_R0_MA_COEFF_C24, CSC_COEFF_C4, pCscMA->usCrOffset);
            }
#endif /* #if defined(BCHP_GFD_0_NL_CSC_CTRL_NL_CSC_R0_ENABLE) */
        }
#endif /* #if (BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) */

        *pulRulCur++ = BRDC_OP_IMMS_TO_REGS(
            BVDC_P_REGS_ENTRIES(GFD_0_CSC_COEFF_C00, GFD_0_CSC_COEFF_C24));
        *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_CSC_COEFF_C00 ) + ulRulOffset;

        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C00, CSC_COEFF_C0, pGfxCsc->stCscMC.usY0);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C01, CSC_COEFF_C1, pGfxCsc->stCscMC.usY1);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C02, CSC_COEFF_C2, pGfxCsc->stCscMC.usY2);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C03, CSC_COEFF_C3, pGfxCsc->stCscMC.usYAlpha);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C04, CSC_COEFF_C4, pGfxCsc->stCscMC.usYOffset);

        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C10, CSC_COEFF_C0, pGfxCsc->stCscMC.usCb0);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C11, CSC_COEFF_C1, pGfxCsc->stCscMC.usCb1);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C12, CSC_COEFF_C2, pGfxCsc->stCscMC.usCb2);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C13, CSC_COEFF_C3, pGfxCsc->stCscMC.usCbAlpha);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C14, CSC_COEFF_C4, pGfxCsc->stCscMC.usCbOffset);

        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C20, CSC_COEFF_C0, pGfxCsc->stCscMC.usCr0);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C21, CSC_COEFF_C1, pGfxCsc->stCscMC.usCr1);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C22, CSC_COEFF_C2, pGfxCsc->stCscMC.usCr2);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C23, CSC_COEFF_C3, pGfxCsc->stCscMC.usCrAlpha);
        *pulRulCur++ = BCHP_FIELD_DATA(GFD_0_CSC_COEFF_C24, CSC_COEFF_C4, pGfxCsc->stCscMC.usCrOffset);
    }

    /* reset RUL buffer pointer */
    *ppulRulCur = pulRulCur;

    return BERR_TRACE(eResult);
}

#define GFD_ENABLE_GFD     1
/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_BuildRulForEnableCtrl_isr
 *
 * designed to be called by BVDC_P_GfxFeeder_BuildRul_isr only, no paramter
 * validation performed here
 */
static BERR_Code BVDC_P_GfxFeeder_BuildRulForEnableCtrl_isr
    ( BVDC_P_GfxFeeder_Handle        hGfxFeeder,
      BVDC_P_State                   eVnetState,
      uint32_t                     **ppulRulCur,
      uint32_t                       ulRulOffset )
{
    BVDC_P_GfxFeederCfgInfo  *pCurCfg = &(hGfxFeeder->stCurCfgInfo);
    BERR_Code  eResult = BERR_SUCCESS;
    uint32_t  *pulRulCur;
    uint32_t  ulEnClrMtrx, ulEnGamma, ulDoneAlphaPreMul, ulEnKey, ulEnHscl, ulEnBstc = 0;
    uint32_t  ulPointSample, ulContinue, ulEnGfd;
    BVDC_P_GfxCfgFlags  stFlags;
#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4)
    uint32_t ulInOrientation, ulOutOrientation;
    BFMT_Orientation  eInOrientation = pCurCfg->eInOrientation;
    uint32_t ulEnMA = 0;
#if ((BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC!=0) && (BVDC_P_CMP_NON_LINEAR_CSC_VER < BVDC_P_NL_CSC_VER_2))
    BVDC_P_CscCfg  *pGfxCsc = &hGfxFeeder->stGfxCsc;
    if (hGfxFeeder->bSupportMACsc && (pGfxCsc->ulNLCnv != BVDC_P_NL_CSC_CTRL_SEL_BYPASS))
    {
        ulEnMA = BCHP_FIELD_DATA( GFD_0_CTRL, CSC_R0_MA_ENABLE, 1 );
    }
#endif
#endif

    /* init RUL buffer pointers */
    pulRulCur = *ppulRulCur;

    /* set RUL for GFD_0_CTRL,
     * must be done after scale and color matrix are decided,
     * always set without compraring, because it is short and ulEnScale
     * depends on surface that could be set in isr
     */
    /* note: for alpha pre-multiply, our concept is the opposite of HW! */
    stFlags = pCurCfg->stFlags;
    ulEnClrMtrx       = stFlags.bNeedColorSpaceConv;
    ulEnHscl          = stFlags.bNeedHorizScale;
    ulEnGamma         = stFlags.bEnableGammaCorrection;
    ulDoneAlphaPreMul = (stFlags.bEnGfdHwAlphaPreMultiply)? 0: 1; /* NOT 1: 0*/
    ulEnKey           = (stFlags.bEnableKey) && (false == stFlags.bConstantBlending);
    ulPointSample     = (BVDC_FilterCoeffs_ePointSample == pCurCfg->eHorzScaleCoeffs)? 1: 0;
    BDBG_MSG(("ulEnClrMtrx %d ", stFlags.bNeedColorSpaceConv));

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_4)
    ulOutOrientation =
        BVDC_P_ORIENTATION_IS_3D(pCurCfg->eOutOrientation) ?
        pCurCfg->eOutOrientation : BFMT_Orientation_e2D;
    ulInOrientation  = BVDC_P_ORIENTATION_IS_3D(eInOrientation)?
        BCHP_FIELD_ENUM( GFD_0_CTRL, MEM_VIDEO,        MODE_3D_DUAL_POINTER) :
        BCHP_FIELD_ENUM( GFD_0_CTRL, MEM_VIDEO,        MODE_2D);
#if defined(BCHP_GFD_0_CTRL_BSTC_ENABLE_ON)
    ulEnBstc = (stFlags.bEnDecompression)?
        BCHP_FIELD_DATA( GFD_0_CTRL, BSTC_ENABLE,      1) : 0;
#endif

    *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
    *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_CTRL + ulRulOffset );
    *pulRulCur++ = ulEnMA | ulEnBstc |
        BCHP_FIELD_DATA( GFD_0_CTRL, CSC_ENABLE,       ulEnClrMtrx )       |
        BCHP_FIELD_DATA( GFD_0_CTRL, GC_ENABLE,        ulEnGamma )         |
        BCHP_FIELD_DATA( GFD_0_CTRL, HFIR_ENABLE,      ulEnHscl )          |
        BCHP_FIELD_DATA( GFD_0_CTRL, CLUT_SCALE_MODE,  ulPointSample )     |
        BCHP_FIELD_DATA( GFD_0_CTRL, ALPHA_PRE_MULT,   ulDoneAlphaPreMul ) |
        BCHP_FIELD_DATA( GFD_0_CTRL, COLOR_KEY_ENABLE, ulEnKey )           |
        ulInOrientation                                                    |
        BCHP_FIELD_DATA( GFD_0_CTRL, BVB_VIDEO,        ulOutOrientation);
#else
    BSTD_UNUSED(ulEnBstc);
    *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
    *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_CTRL + ulRulOffset );
    *pulRulCur++ =
        BCHP_FIELD_DATA( GFD_0_CTRL, CSC_ENABLE,       ulEnClrMtrx )       |
        BCHP_FIELD_DATA( GFD_0_CTRL, GC_ENABLE,        ulEnGamma )         |
        BCHP_FIELD_DATA( GFD_0_CTRL, HFIR_ENABLE,      ulEnHscl )          |
        BCHP_FIELD_DATA( GFD_0_CTRL, CLUT_SCALE_MODE,  ulPointSample )     |
        BCHP_FIELD_DATA( GFD_0_CTRL, ALPHA_PRE_MULT,   ulDoneAlphaPreMul ) |
        BCHP_FIELD_DATA( GFD_0_CTRL, COLOR_KEY_ENABLE, ulEnKey );
#endif

    /*vertical scaling is enabled in GSCL_CFG */

    /* Set RUL for gfx feeder enable:
     * Note: Enabling should be put at the last of gfx feeder config RUL,
     * We use both eVnetState to control gfd enabling and gfx surface enabling
     * in compositor */
    ulEnGfd = (BVDC_P_State_eActive == eVnetState)? GFD_ENABLE_GFD : 0;
    ulContinue = stFlags.bContinueOnFieldEnd;
    *pulRulCur++ = BRDC_OP_IMM_TO_REG( );
    *pulRulCur++ = BRDC_REGISTER( BCHP_GFD_0_ENABLE + ulRulOffset );
    *pulRulCur++ =
        BCHP_FIELD_DATA( GFD_0_ENABLE, ENABLE,      ulEnGfd ) |
        BCHP_FIELD_DATA( GFD_0_ENABLE, ENABLE_CTRL, ulContinue );

    /* reset RUL buffer pointer */
    *ppulRulCur = pulRulCur;

    return BERR_TRACE(eResult);
}

/*------------------------------------------------------------------------
 *  {secret}
 *  BVDC_P_GfxFeeder_HandleIsrSurface_isr
 *
 *  No paramter error check is performed. Designed to be called by
 *  BVDC_P_GfxFeeder_BuildRul_isr only
 *
 *  It pulls a new isr surface if callback func is installed; then activates
 *  any new set surace (by Source_SetSurface_isr or this callback func). It
 *  at first validates the combination of this surface and current confugure,
 *  and set up to build RUL for this combination.
 */
static void BVDC_P_GfxFeeder_HandleIsrSurface_isr
    ( BVDC_P_GfxFeeder_Handle      hGfxFeeder,
      BVDC_P_Source_Info *         pCurSrcInfo,
      BAVC_Polarity                eFieldId )
{
    BVDC_P_GfxFeederCfgInfo  *pCurCfg;
    void  *pvGfxPic;
    const BVDC_P_ClipRect  *pCurClip;
    const BVDC_P_Rect  *pCurDst, *pCurSclOut;
    BVDC_P_SurfaceInfo  *pIsrSur = &(hGfxFeeder->stGfxSurface.stIsrSurInfo);
    BERR_Code  eResult;

    if( NULL != pCurSrcInfo->pfPicCallbackFunc )
    {
        pCurSrcInfo->pfPicCallbackFunc(
            pCurSrcInfo->pvParm1, pCurSrcInfo->iParm2,
            eFieldId, BAVC_SourceState_eActive, &pvGfxPic);
        if ( NULL != pvGfxPic )
        {
            if (BVDC_P_GfxSurface_SetSurface_isr(&hGfxFeeder->stGfxSurface,
                pIsrSur, (BAVC_Gfx_Picture *)pvGfxPic, hGfxFeeder->hSource) != BERR_SUCCESS)
            {
                BDBG_ERR(("Previous surface is displayed."));
                return;
            }
        }
    }

    /* after previous vsync, pIsrSur->bChangeSurface might be set by
     * Source_SetSurface_isr, by the above callback function, or not re-set
     * at all. If it was re-set, and there was no change in format, size, or
     * pitch, then it has already been sent to HW in GfxSurface_SetSurface_isr,
     * and pIsrSur->bChangeSurface has been marked as false */
    if (pIsrSur->bChangeSurface && hGfxFeeder->hWindow)
    {
        BVDC_P_Window_GetCurrentRectangles_isr(
            hGfxFeeder->hWindow, &pCurClip, &pCurSclOut, &pCurDst);

        /* validate surface size and rectangles, adjust rectangles and do some
         * scale related computation. The intermediate values are stored in
         * CurCfg, and will affect RUL build immediately. */
        pCurCfg = &(hGfxFeeder->stCurCfgInfo);
        eResult = BVDC_P_GfxFeeder_ValidateSurAndRects_isrsafe(
            hGfxFeeder, pIsrSur, pCurCfg, pCurClip, pCurSclOut, pCurDst);
        if (BERR_SUCCESS == eResult)
        {
            /* Copy IsrSur to curSur
             * note: pCurSur->bChangeSurface would stay true so that the surface
             * change is built into RUL later */
            hGfxFeeder->stGfxSurface.stCurSurInfo = *pIsrSur;

            /* to avoid future validation if isr surface no longer changes */
            pIsrSur->bChangeSurface  = false;
        }
        /* otherwise we stay with original current surface and ignore this
         * IsrSur until either Cfg or surface changes
         * note: ValidateSurAndRects will NOT change CurCfg until all error
         * check are completed */
    }
}

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_BuildRul_isr
 *
 * Append GfxFeeder specific RULs into hList.
 *
 */
void BVDC_P_GfxFeeder_BuildRul_isr
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder,
      BVDC_P_Source_Info *             pCurSrcInfo,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId,
      BVDC_P_State                     eVnetState )
{
    BVDC_P_SurfaceInfo  *pCurSur;
    BVDC_P_GfxFeederCfgInfo  *pCurCfg;
    BVDC_P_GfxDirtyBits  stCurDirty;
    BVDC_P_CscCoeffs const *pRGBToYCbCr, *pYCbCrToRGB;
    uint32_t  *pulRulCur;
    uint32_t  ulRulOffset;

    BDBG_ENTER(BVDC_P_GfxFeeder_BuildRul_isr);

    /* Note: we will not be called if this src is not connected to a window */
    /* validate paramters */
    BDBG_OBJECT_ASSERT(hGfxFeeder, BVDC_GFX);
    BDBG_ASSERT( NULL != pCurSrcInfo );

    /* init current state ptrs */
    pCurSur = &(hGfxFeeder->stGfxSurface.stCurSurInfo);
    pCurCfg = &(hGfxFeeder->stCurCfgInfo);

    /* If surface has been set by *SetSurface_isr, or src pic call back func is
     * installed, we activate it now */
    BVDC_P_GfxFeeder_HandleIsrSurface_isr( hGfxFeeder, pCurSrcInfo, eFieldId );

    /* init RUL buffer pointers */
    pulRulCur = pList->pulCurrent;
    ulRulOffset = hGfxFeeder->ulRegOffset;

    /* this copy should be after HandleIsrSurface_isr since it might change Dirty */
    stCurDirty = pCurCfg->stDirty;
    if(!pList->bLastExecuted)
    {
        /* rebuild RUL for the changes done in last vsync, because last vsync's
         * RUL is lost */
        BVDC_P_OR_ALL_DIRTY(&stCurDirty, &hGfxFeeder->stPrevDirty);
    }

    /* at this point CurSur could be the same as last vsync, be changed by SetSurface
     * /SetSurface_isr, or by the callback func */
    if(pCurSur->bChangeSurface)
    {
        /* pass ChangeSurface bit to stDirty, This will ensure the change is
         * set to HW. */
        stCurDirty.stBits.bSurface = BVDC_P_DIRTY;
        stCurDirty.stBits.bPaletteTable =
            (pCurSur->bChangePaletteTable)? BVDC_P_DIRTY : BVDC_P_CLEAN;
        pCurSur->bChangePaletteTable = false;
    }

#if (BVDC_P_GFX_INIT_CNTR > 1)
    /* after bInitial, we need to SetAllDirty one more time? */
    if(hGfxFeeder->ulInitVsyncCntr && BVDC_P_GFX_INIT_CNTR > hGfxFeeder->ulInitVsyncCntr)
    {
        BVDC_P_GfxFeeder_SetAllDirty(&stCurDirty);
        hGfxFeeder->ulInitVsyncCntr --;
    }
#endif

    /* BuildRulFor* will use pCurCfg->stDirty, so copy back after modification */
    pCurCfg->stDirty = stCurDirty;

    /* SW3548-2976 workaround: reset GFD if DCX is siwtched ON->OFF */
    if(BVDC_P_GFX_INIT_CNTR == hGfxFeeder->ulInitVsyncCntr)
    {
        /*remove the software init after 7435A*/
#if (BVDC_P_SUPPORT_GFD_VER < BVDC_P_SUPPORT_GFD_VER_6)
        BVDC_P_BUILD_RESET(pulRulCur,
            hGfxFeeder->ulResetRegAddr, hGfxFeeder->ulResetMask);
#endif
        hGfxFeeder->ulInitVsyncCntr --;
    }

    /* resolve color conversion state */
    if (hGfxFeeder->hWindow)
    {
        if ( stCurDirty.stBits.bCsc )
        {
            BVDC_P_GfxFeeder_DecideColorMatrix_isr( pCurSur->eActivePxlFmt, hGfxFeeder, &pRGBToYCbCr, &pYCbCrToRGB);

#ifndef BVDC_FOR_BOOTUPDATER
        /* color adjustment */
            BVDC_P_Csc_ApplyContrast_isr(hGfxFeeder->hWindow->stCurInfo.sContrast, &hGfxFeeder->stGfxCsc.stCscMC);
            BVDC_P_Csc_ApplyBrightness_isr(hGfxFeeder->hWindow->stCurInfo.sBrightness, &hGfxFeeder->stGfxCsc.stCscMC);
            BVDC_P_Csc_ApplySaturationAndHue_isr(
                hGfxFeeder->hWindow->stCurInfo.sSaturation,
                hGfxFeeder->hWindow->stCurInfo.sHue, &hGfxFeeder->stGfxCsc.stCscMC);
            BVDC_P_Csc_ApplyAttenuationRGB_isr(
                hGfxFeeder->hWindow->stCurInfo.lAttenuationR,
                hGfxFeeder->hWindow->stCurInfo.lAttenuationG,
                hGfxFeeder->hWindow->stCurInfo.lAttenuationB,
                hGfxFeeder->hWindow->stCurInfo.lOffsetR,
                hGfxFeeder->hWindow->stCurInfo.lOffsetG,
                hGfxFeeder->hWindow->stCurInfo.lOffsetB,
                &hGfxFeeder->stGfxCsc.stCscMC,
                pYCbCrToRGB, /* YCbCr->RGB */
                pRGBToYCbCr, /* RGB->YCbCr */
                hGfxFeeder->hWindow->stCurInfo.bUserCsc);
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
        }
    }

    /* set RULs, gfx should be enabled at the last of config */
    BVDC_P_GfxFeeder_BuildRulForUserChangeOnly_isr( pCurCfg, &pulRulCur, ulRulOffset );

    BVDC_P_GfxFeeder_BuildRulForSurCtrl_isr( hGfxFeeder, eFieldId, &pulRulCur, ulRulOffset );

    BVDC_P_GfxFeeder_BuildRulForColorCtrl_isr( hGfxFeeder, eFieldId, &pulRulCur, ulRulOffset );

    BVDC_P_GfxFeeder_BuildRulForEnableCtrl_isr( hGfxFeeder, eVnetState, &pulRulCur, ulRulOffset );

    /* copy dirty bits to PrevDirty in case this RUL never goes to HW */
    hGfxFeeder->stPrevDirty = stCurDirty;

    BVDC_P_CLEAN_ALL_DIRTY(&(pCurCfg->stDirty));
    pCurSur->bChangeSurface = false; /* must reset after BuildRulForSurCtrl_isr */

    pList->pulCurrent = pulRulCur;
    BDBG_LEAVE(BVDC_P_GfxFeeder_BuildRul_isr);
    return;
}

/***************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_ValidateBlend
 *
 * Called by BVDC_Window_SetBlendFactor to validate the graphics window
 * blending factor setting
 */
BERR_Code BVDC_P_GfxFeeder_ValidateBlend
    ( BVDC_BlendFactor             eSrcBlendFactor,
      BVDC_BlendFactor             eDstBlendFactor,
      uint8_t                      ucConstantAlpha )
{
    BERR_Code eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_GfxFeeder_ValidateBlend);
    if ( false ==
         (((BVDC_BlendFactor_eSrcAlpha         == eSrcBlendFactor) &&
           (BVDC_BlendFactor_eOneMinusSrcAlpha == eDstBlendFactor))       ||
          ((BVDC_BlendFactor_eConstantAlpha    == eSrcBlendFactor) &&
           (BVDC_BlendFactor_eOneMinusSrcAlpha == eDstBlendFactor) &&
           (BVDC_ALPHA_MAX                     == ucConstantAlpha))       ||
          ((BVDC_BlendFactor_eConstantAlpha         == eSrcBlendFactor) &&
           (BVDC_BlendFactor_eOneMinusConstantAlpha == eDstBlendFactor))  ||
          ((BVDC_BlendFactor_eOneMinusConstantAlpha == eSrcBlendFactor) &&
           (BVDC_BlendFactor_eConstantAlpha         == eDstBlendFactor))  ||
          ((BVDC_BlendFactor_eOne  == eSrcBlendFactor) &&
           (BVDC_BlendFactor_eZero == eDstBlendFactor))                   ||
          ((BVDC_BlendFactor_eZero == eSrcBlendFactor) &&
           (BVDC_BlendFactor_eOne  == eDstBlendFactor))) )
    {
        /* we support 3 cases for blend factor, refer to
         * BVDC_P_GfxFeeder_ValidateChanges for detail */
        eResult = BERR_TRACE(BVDC_ERR_ILLEGAL_GFX_WIN_BLEND);
    }

    BDBG_LEAVE(BVDC_P_GfxFeeder_ValidateBlend);
    return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_AdjustBlend_isr
 *
 * Called by BVDC_P_Window_SetBlender to adjust the blending factor
 * of a graphics window for HW register setting
 *
 * Note: peSrcBlendFactor, peDstBlendFactor, and pucConstantAlpha are both
 * input and output of this function, they must be filled with current
 * values before calling this function
 */
BERR_Code BVDC_P_GfxFeeder_AdjustBlend_isr
    ( BVDC_BlendFactor            *peSrcBlendFactor,
      BVDC_BlendFactor            *peDstBlendFactor,
      uint8_t                     *pucConstantAlpha )
{
    BERR_Code eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_GfxFeeder_AdjustBlend_isr);

    if ( (NULL != peSrcBlendFactor) && (NULL != peDstBlendFactor) &&
         (NULL != pucConstantAlpha) )
    {
        /* refer to BVDC_P_GfxFeeder_ValidateChanges, there is only one
         * case, when users means to use src alpha for blending factors,
         * we need to adjust blending setting. In this case, we should
         * enable the alpha pre-multiply in GFD HW and accordingly
         * adjust SrcBlendDactor to BVDC_BlendFactor_eConstantAlpha.
         * This is because the alpha output from color key stage is always
         * pre-multiplied to pixel color by HW */
        if (BVDC_BlendFactor_eSrcAlpha == *peSrcBlendFactor )
        {
            *peSrcBlendFactor = BVDC_BlendFactor_eConstantAlpha;
            *pucConstantAlpha = GFX_ALPHA_FULL;
        }
    }
    else
    {
        eResult = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_LEAVE(BVDC_P_GfxFeeder_AdjustBlend_isr);
    return BERR_TRACE(eResult);
}

/* End of File */
