/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef BVDC_GFXFEEDER_PRIV_H__
#define BVDC_GFXFEEDER_PRIV_H__

#include "bstd.h"             /* standard types */
#include "bvdc.h"
#include "bvdc_source_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_gfxsurface_priv.h"
#include "bvdc_cfc_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BVDC_GFX);

/* Core revs to track increatmental changes! */
/* CSC, Gamma-correction, and HSCL are removed, only P4 and AYCbCr8888 supported */
#define BVDC_P_SUPPORT_GFD_VER_0           (0) /* 7550 GFD1 */

#define BVDC_P_SUPPORT_GFD_VER_1           (1) /* 7400, 7405, 7325, 7335 */

/* 6 taps + 2 coeff phases HSCL */
#define BVDC_P_SUPPORT_GFD_VER_2           (2) /* 3548, 3556, 7125 */

/* VSCL added */
#define BVDC_P_SUPPORT_GFD_VER_3           (3) /* 7420, 7550 */

/* 3D Graphics: HW7231-80 issue:
 * GFD REV 0.1.0.B: 7231-A0, 7344-A0, 7346-A0, 7422-A0, 7425-A0
 * GFD REV 0.1.0.C: 7358-A0, 7360-A0, 7362-A0, 7552-A0, 7552-B0, 73625-A0 */
#define BVDC_P_SUPPORT_GFD_VER_4           (4)

/* 3D Graphics: HW7231-80 issue:
 * GFD REV 0.1.0.D : 7231-B0, 7344-B0, 7346-B0, 7425-B0
 * GFD REV 0.1.0.E : 7428-A0
 * GFD REV 0.1.1.0 : 7435-A0
 * GFD REV 0.1.1.1 : 7429-B0, 7435-B0, 7543-A0, 7563-A0, 75635-A0, 7584-A0, 75845-A0 */
#define BVDC_P_SUPPORT_GFD_VER_5           (5)

/* 3D Graphics with VSCL_LSBF_SIZE indicaton etc*/
#define BVDC_P_SUPPORT_GFD_VER_6           (6) /* 7428/29 7435, a0 */

/* HW_CONFIG bad read back. */
#define BVDC_P_SUPPORT_GFD_VER_7           (7) /* 7445D0/1, 7366B0, 7364A0, 7250A0 */

/* fixed "HW_CONFIG bad read back". */
#define BVDC_P_SUPPORT_GFD_VER_8           (8) /* 7145 B0 */

/* fixed "color space matrix a BCHP_GFD_0_CSC_R0_MA_COEFFxx */
#define BVDC_P_SUPPORT_GFD_VER_9           (9) /* 7439 b0 */

/* BSTC with  HW7278-594 to be fixed since 7278b0*/
#define BVDC_P_SUPPORT_GFD_VER_10           (10) /* 7268 a0/b0 7271 a0/b0 7278 a0 7586 */

/* register re-org to optimize block writes */
#define BVDC_P_SUPPORT_GFD_VER_11           (11) /* 7278 b0, 7260 B0*/

#if ((BVDC_P_SUPPORT_GFD_VER == BVDC_P_SUPPORT_GFD_VER_4)|| \
     (BVDC_P_SUPPORT_GFD_VER == BVDC_P_SUPPORT_GFD_VER_5))
/* HW7231-187 transition failure between 2D->3D 3D->2D*/
#define BVDC_P_GFX_INIT_WORKAROUND         (1)
#else
#define BVDC_P_GFX_INIT_WORKAROUND         (0)
#endif

#define  BVDC_P_GFX_INIT_CNTR               1

#define  GFD_NUM_REGS_HSCL_COEFF            4
#define  GFD_NUM_REGS_VSCL_COEFF            2
/*-------------------------------------------------------------------------
 *
 */
typedef union BVDC_P_GfxDirtyBits
{
    struct
    {
        /* configure dirty bits */
        uint32_t                 bChromaExpan             : 1; /* 0 */
        uint32_t                 bKey                     : 1;
        uint32_t                 bScaleCoeffs             : 1;
        uint32_t                 bGammaTable              : 1;
        uint32_t                 bConstantColor           : 1;
        uint32_t                 bClutTable               : 1; /* unused */
        uint32_t                 bScanType                : 1; /* for bInterlaced */
        uint32_t                 bSrcClip                 : 1;
        uint32_t                 bOutRect                 : 1; /* 8 */
        uint32_t                 bColorMatrix             : 1; /* unused */
        uint32_t                 bFlags                   : 1; /* some change in BVDC_P_GfxCfgFlags */
        uint32_t                 bDemoMode                : 1;
        uint32_t                 bOrientation             : 1;
        uint32_t                 bSdrGfx2HdrAdj           : 1;
        uint32_t                 bLuminance               : 1;

        /* confugure and surface combined dirty bits */
        uint32_t                 bClipOrOut               : 1;
        uint32_t                 bCsc                     : 1; /* 16 */
        uint32_t                 bPxlFmt                  : 1;
        uint32_t                 bSurOffset               : 1;
        uint32_t                 bSurface                 : 1;
        uint32_t                 bPaletteTable            : 1;
        uint32_t                 bCompress                : 1;

    } stBits;

    uint32_t aulInts [BVDC_P_DIRTY_INT_ARRAY_SIZE];

} BVDC_P_GfxDirtyBits;

/*-------------------------------------------------------------------------
 *
 */
typedef struct BVDC_P_GfxCfgFlags
{
    /* enable bits */
    uint32_t                 bEnableKey                     : 1;
    uint32_t                 bEnGfdHwAlphaPreMultiply       : 1;
    uint32_t                 bEnableGammaCorrection         : 1;
    uint32_t                 bNeedHorizScale                : 1;
    uint32_t                 bNeedVertScale                 : 1;
    uint32_t                 bNeedColorSpaceConv            : 1; /* unused */
    uint32_t                 bConstantBlending              : 1;

    /* output video scantype */
    uint32_t                 bInterlaced                    : 1;

    /* enable control: continuous or stop_on_field_end */
    uint32_t                 bContinueOnFieldEnd            : 1;

    uint32_t                 bDeringDemoMode                : 1;
    uint32_t                 bDejagDemoMode                 : 1;
    uint32_t                 bEnDering                      : 1;
    uint32_t                 bEnDejag                       : 1;

    uint32_t                 bEnDecompression               : 1;
} BVDC_P_GfxCfgFlags;

/*-------------------------------------------------------------------------
 * graphics feeder proccess configure info
 */
typedef struct BVDC_P_GfxFeederCfgInfo
{
    /* dirty bits: when ApplyChanges is called, current Dirty should be
     * NewDirty | CurDirty, in order to handle that more than one
     * applyChange is called before the RUL is built */
    BVDC_P_GfxDirtyBits      stDirty;

    /* misc one bit configs */
    BVDC_P_GfxCfgFlags       stFlags;

    /* chroma expansion method */
    BVDC_ChromaExpansion     eChromaExpansion;

    /* color key */
    uint8_t                  ucKeyedAlpha;
    uint32_t                 ulKeyMinAMNO;
    uint32_t                 ulKeyMaxAMNO;
    uint32_t                 ulKeyMaskAMNO;

    /* constant color for alpha only pixel */
    uint32_t                 ulConstantColor;

    /* window alpha */
    uint8_t                  ucWindowAlpha;  /* set to HW as default key alpha */

    /* horizontal/vertical up scale */
    BVDC_FilterCoeffs        eHorzScaleCoeffs;  /* coeff mode */
    BVDC_FilterCoeffs        eVertScaleCoeffs;  /* coeff mode */

    /* gamma correction */
    uint32_t                 ulNumGammaClutEntries;
    uint32_t                 ulGammaClutAddress;

    /* src clip rect, dest scale rectangle dimension, and up-scale */
    uint32_t                 ulCntLeft;  /* left with frac */
    uint32_t                 ulCntTopInt;  /* int, top int part only */
    uint32_t                 ulCntWidth;  /* int, round to ceiling */
    uint32_t                 ulCntHeight; /* int, round to ceiling */
    uint32_t                 ulOutWidth;  /* int */
    uint32_t                 ulOutHeight;  /* int */
    uint32_t                 ulHsclSrcStep;  /* src / out, with frac  */
    uint32_t                 ulVsclSrcStep;  /* src / out, with frac  */
    uint32_t                 ulVsclBlkAvgSize;  /* vert block average size */
    uint32_t                 ulVsclInitPhase;  /* for frame or top-field display */
    uint32_t                 ulVsclInitPhaseBot;  /* for bot-field display */

    /* display orientation */
    BFMT_Orientation         eOutOrientation;    /* output Orientation, might not be useful*/
    bool                     bOrientationOverride;
    BFMT_Orientation         eInOrientation;

    /* adjust to linear approximation of sdr gfx to hdr conversion */
    BVDC_Source_SdrGfxToHdrApproximationAdjust stSdrGfx2HdrAdj;
    struct
    {
        unsigned ulMin;
        unsigned ulMax;
    } stLuminance;
} BVDC_P_GfxFeederCfgInfo;

/*-------------------------------------------------------------------------
 * graphics feeder main context
 */
typedef struct BVDC_P_GfxFeederContext
{
    BDBG_OBJECT(BVDC_GFX)

    /* Gfx Feeder Id */
    BAVC_SourceId                    eId;
    uint32_t                         ulRegOffset;

    /* only 3d src needs ping-pong buffer mechanism */
    bool                             b3dSrc;
    BREG_Handle                      hRegister;
    BRDC_Handle                      hRdc;
    BVDC_Window_Handle               hWindow;
    BVDC_Source_Handle               hSource;

    /* gfx surface manager */
    BVDC_P_GfxSurfaceContext         stGfxSurface;

#if (BVDC_P_SUPPORT_OLD_SET_ALPHA_SUR)
    /* temporarily used for supporting BVDC_Source_SetAlphaSurface */
    BAVC_Gfx_Picture                 stTmpNewAvcPic;
    BAVC_Gfx_Picture                 stTmpIsrAvcPic;
#endif

    /* could be user set new sur, isr set new, or cur sur,
     * decided by Validate, used by ApplyChange */
    BVDC_P_SurfaceInfo              *pNewSur;

    /* gfx feeder private processing cfg activated by user with ApplyChange */
    BVDC_P_GfxFeederCfgInfo          stNewCfgInfo;
    BVDC_P_GfxFeederCfgInfo          stCurCfgInfo;

    /* record previous dirty in case RUL was not executed */
    BVDC_P_GfxDirtyBits              stPrevDirty;

    /* current matrix to convert SDR gfx to HDR in pre-7271 chips */
    BCFC_Csc3x4                      stCscSdr2Hdr;

    /* color space conversion */
#if BVDC_P_CMP_CFC_VER >= 3
    /* CFC LUT heap */
    BMMA_Heap_Handle                 hCfcHeap; /* must be cpu accessible for LUT fill */
    BCFC_LutLoadListInfo             stCfcLutList; /* for CFC ram table loading */
#if BVDC_P_DBV_SUPPORT
    bool                             bDbvEnabled;
#endif
#endif
    BCFC_Context                     stCfc;

    bool                             bSupportVertScl;

    uint32_t                         ulInitVsyncCntr;

    uint32_t                         ulOffsetPixInByte;   /* Num of offset pix for sub byte pixel format */
    uint32_t                         ulAlphaOffsetPixInByte;   /* Alpha Sur: Num of offset pix in a byte */
    uint32_t                         ulFirInitPhase;    /* scale: init phase */

    uint32_t                         ulVertLineBuf;    /* line buffer length of Vert scaler */
    uint32_t                         ulResetRegAddr;
    uint32_t                         ulResetMask;

} BVDC_P_GfxFeederContext;


/***************************************************************************
 * Gfx feeder private functions
 ***************************************************************************/

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
      BVDC_Source_Handle               hSource);

/*************************************************************************
 * {private}
 *
 * Called by BVDC_P_Source_Destroy to destroy gfx feeder specific context
 * when BVDC_Handle is closed with BVDC_Close
 */
BERR_Code BVDC_P_GfxFeeder_Destroy
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder );

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_Init
 *
 * initialize stuff that will change after destory and re-create. It also
 * allocate surface address shadow registers. We don't want to allocate them
 * until the GFD is really going to be used.
 */
void BVDC_P_GfxFeeder_Init(
    BVDC_P_GfxFeeder_Handle          hGfxFeeder,
    const BVDC_Source_Settings      *pSettings );

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_GetAdjSclOutRect_isr
 *
 * Called by BVDC_P_Window_ApplyChanges to get the adjusted scal-out rect
 * as it SetSurfaceSize in compositor,
 * It should match the design of BVDC_P_GfxFeeder_ValidateSurAndRects
 */
BERR_Code BVDC_P_GfxFeeder_GetAdjSclOutRect_isr
    ( const BVDC_P_ClipRect           *pClipRect,            /* in */
      const BVDC_P_Rect               *pSclOutRect,          /* in */
      const BVDC_P_Rect               *pDstRect,             /* in */
      BVDC_P_Rect                     *pAdjSclOutRect );      /* out */

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
      BVDC_Source_PictureCallback_isr  pfPicCallbackFunc );

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_ApplyChanges_isr
 *
 * To be called by BVDC_ApplyChanges, to copy "new user state" to "current
 * state", after validation of all VDC modules passed.
 *
 */
BERR_Code BVDC_P_GfxFeeder_ApplyChanges_isr
    ( BVDC_P_GfxFeeder_Handle     hGfxFeeder );

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_AbortChanges
 *
 * Cancel the user settings that either fail to validate or simply
 * because user wish to abort the changes in mid-way.
 */
void BVDC_P_GfxFeeder_AbortChanges
    ( BVDC_P_GfxFeeder_Handle     hGfxFeeder );

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_BuildRul_isr
 *
 * Append GfxFeeder specific RULs into hList.
 *
 */
void BVDC_P_GfxFeeder_BuildRul_isr
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId,
      BVDC_P_State                     eVnetState );

/*************************************************************************
 * {private}
 * BVDC_P_GfxFeeder_UpdateState_isr
 *
 * Update GfxFeeder state.
 *
 */
void BVDC_P_GfxFeeder_UpdateState_isr
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder,
      BVDC_P_Source_Info *             pCurSrcInfo,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId );

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
      uint8_t                      ucConstantAlpha );

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
    ( BVDC_P_GfxFeeder_Handle      hGfxFeeder,
      BVDC_BlendFactor            *peSrcBlendFactor,
      BVDC_BlendFactor            *peDstBlendFactor,
      uint8_t                     *pucConstantAlpha );

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecideFilterCoeff_isr
 *
 * output: Hscl filter coeff
 */
BERR_Code BVDC_P_GfxFeeder_DecideFilterCoeff_isr
    ( BVDC_FilterCoeffs     eCoeffs,
      uint32_t              ulCtIndex,
      uint32_t              ulSrcSize,
      uint32_t              ulOutSize,
      uint32_t **           paulCoeff );

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecideVsclFirCoeff_isr
 *
 * output: filter coeff
 *
 * Note: This implementation is originally copied from bgrc, we should watch
 * bgrc's update and update this code accordingly.
 */
BERR_Code BVDC_P_GfxFeeder_DecideVsclFirCoeff_isr
    ( BVDC_FilterCoeffs     eCoeffs,
      uint32_t              ulCtIndex,
      uint32_t              ulSrcSize,
      uint32_t              ulOutSize,
      uint32_t **           paulCoeff );

#if (BVDC_P_CMP_CFC_VER <= BVDC_P_CFC_VER_1)
/* Adjust Mc for SDR to HDR10/HLG CSC
 */
void BVDC_P_GfxFeeder_AdjustMcForSdr2Hdr_isr
    ( BVDC_P_GfxFeeder_Handle      hGfxFeeder );
#endif

/* init GFD CFC
 */
void BVDC_P_GfxFeeder_InitCfc
    ( BVDC_P_GfxFeeder_Handle      hGfxFeeder);

/* copy input color space info from gfx surface
 */
void BVDC_P_GfxFeeder_UpdateGfxInputColorSpace_isr(
    const BVDC_P_SurfaceInfo   *pGfxSurface,
    BCFC_ColorSpaceExt         *pColorSpaceExt );

/* Build RUL for cfc inside a GFD
 */
void BVDC_P_GfxFeeder_BuildCfcRul_isr
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder,
      BVDC_P_ListInfo                 *pList);

void BVDC_P_GfxFeeder_GetCfcCapabilities
    ( BREG_Handle                      hRegister,
      BAVC_SourceId                    eGfdId,
      BCFC_Capability                 *pCapability );
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_GFXFEEDER_PRIV_H__ */
/* End of file. */
