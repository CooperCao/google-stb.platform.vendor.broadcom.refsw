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
#ifndef BVDC_PRIV_H__
#define BVDC_PRIV_H__

#include "bvdc.h"
#include "bkni.h"
#include "bvdc_dbg.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_resource_priv.h"
#include "bchp_misc.h"
#include "btmr.h"
#include "brdc_rul.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(BVDC_VDC);
BDBG_OBJECT_ID_DECLARE(BVDC_SRC);
BDBG_OBJECT_ID_DECLARE(BVDC_CMP);
BDBG_OBJECT_ID_DECLARE(BVDC_WIN);
BDBG_OBJECT_ID_DECLARE(BVDC_DSP);

/***************************************************************************
Summary:
    List of BVN errors.

Description:
    This is the enumerated list of BVN module errors.

See Also:
****************************************************************************/
typedef enum
{
    BVDC_BvnError_eRdc,

    BVDC_BvnError_eMfd_0,
    BVDC_BvnError_eMfd_1,
    BVDC_BvnError_eMfd_2,
    BVDC_BvnError_eMfd_3,
    BVDC_BvnError_eMfd_4,
    BVDC_BvnError_eMfd_5,

    BVDC_BvnError_eVfd_0,
    BVDC_BvnError_eVfd_1,
    BVDC_BvnError_eVfd_2,
    BVDC_BvnError_eVfd_3,
    BVDC_BvnError_eVfd_4,
    BVDC_BvnError_eVfd_5,
    BVDC_BvnError_eVfd_6,
    BVDC_BvnError_eVfd_7,

    BVDC_BvnError_eScl_0,
    BVDC_BvnError_eScl_1,
    BVDC_BvnError_eScl_2,
    BVDC_BvnError_eScl_3,
    BVDC_BvnError_eScl_4,
    BVDC_BvnError_eScl_5,
    BVDC_BvnError_eScl_6,
    BVDC_BvnError_eScl_7,

    BVDC_BvnError_eDnr_0,
    BVDC_BvnError_eDnr_1,
    BVDC_BvnError_eDnr_2,
    BVDC_BvnError_eDnr_3,
    BVDC_BvnError_eDnr_4,
    BVDC_BvnError_eDnr_5,

    BVDC_BvnError_eXsrc_0,
    BVDC_BvnError_eXsrc_1,
    BVDC_BvnError_eXsrc_2,

    BVDC_BvnError_eTntd_0,

    BVDC_BvnError_eMvp_0,
    BVDC_BvnError_eMvp_1,
    BVDC_BvnError_eMvp_2,
    BVDC_BvnError_eMvp_3,
    BVDC_BvnError_eMvp_4,
    BVDC_BvnError_eMvp_5,

    BVDC_BvnError_eMcdi_0,
    BVDC_BvnError_eMcdi_1,
    BVDC_BvnError_eMcdi_2,
    BVDC_BvnError_eMcdi_3,
    BVDC_BvnError_eMcdi_4,
    BVDC_BvnError_eMcdi_5,

    BVDC_BvnError_eMctf_0,

    BVDC_BvnError_eHscl_0,
    BVDC_BvnError_eHscl_1,
    BVDC_BvnError_eHscl_2,
    BVDC_BvnError_eHscl_3,
    BVDC_BvnError_eHscl_4,
    BVDC_BvnError_eHscl_5,

    BVDC_BvnError_eCap_0,
    BVDC_BvnError_eCap_1,
    BVDC_BvnError_eCap_2,
    BVDC_BvnError_eCap_3,
    BVDC_BvnError_eCap_4,
    BVDC_BvnError_eCap_5,
    BVDC_BvnError_eCap_6,
    BVDC_BvnError_eCap_7,

    BVDC_BvnError_eGfd_0,
    BVDC_BvnError_eGfd_1,
    BVDC_BvnError_eGfd_2,
    BVDC_BvnError_eGfd_3,
    BVDC_BvnError_eGfd_4,
    BVDC_BvnError_eGfd_5,
    BVDC_BvnError_eGfd_6,

    BVDC_BvnError_eCmp_0_V0,
    BVDC_BvnError_eCmp_1_V0,
    BVDC_BvnError_eCmp_2_V0,
    BVDC_BvnError_eCmp_3_V0,
    BVDC_BvnError_eCmp_4_V0,
    BVDC_BvnError_eCmp_5_V0,
    BVDC_BvnError_eCmp_6_V0,

    BVDC_BvnError_eCmp_0_V1,
    BVDC_BvnError_eCmp_1_V1,

    BVDC_BvnError_eCmp_0_G0,
    BVDC_BvnError_eCmp_1_G0,
    BVDC_BvnError_eCmp_2_G0,
    BVDC_BvnError_eCmp_3_G0,
    BVDC_BvnError_eCmp_4_G0,
    BVDC_BvnError_eCmp_5_G0,
    BVDC_BvnError_eCmp_6_G0,

    BVDC_BvnError_eCmp_0_G1,
    BVDC_BvnError_eCmp_0_G2,

    BVDC_BvnError_eMaxCount,

    BVDC_BvnError_eInvalid = BVDC_BvnError_eMaxCount      /* must be last */
} BVDC_BvnError;
/* B0 does not support Letterbox Detection and VBI Pass through together */
#define B0_NO_LETTERBOX_DETECTION_AND_VBI_PASS_THROUGH_COMBO (1)

/* */
#define BVDC_P_SUPPORT_TDAC_VER_0                            (0) /* 3563 */
#define BVDC_P_SUPPORT_TDAC_VER_1                            (1) /* 7401, 7403, 7118 */
#define BVDC_P_SUPPORT_TDAC_VER_2                            (2) /* 7400 */
#define BVDC_P_SUPPORT_TDAC_VER_3                            (3) /* 7405A0, 7325 */
#define BVDC_P_SUPPORT_TDAC_VER_4                            (4) /* 7405Bx, 7335 */
#define BVDC_P_SUPPORT_TDAC_VER_5                            (5) /* 3548, 3556 */
#define BVDC_P_SUPPORT_TDAC_VER_6                            (6) /* 7420 */
#define BVDC_P_SUPPORT_TDAC_VER_7                            (7) /* 7340, 7342 */
#define BVDC_P_SUPPORT_TDAC_VER_8                            (8) /* 7550: no DAC_BG_CTRL_1 */
#define BVDC_P_SUPPORT_TDAC_VER_9                            (9) /* 7422: add DAC detection */
#define BVDC_P_SUPPORT_TDAC_VER_10                          (10) /* 7360: add recalibrate bit */
/* There are two type of reference boards CATV and DBS.
 * DBS ones have correct 75ohm termination on all DACs.
 * CATV boards have two 75 ohm registers in parallel for DAC3
 * (total 37.5ohm) , while for DAC0, DAC1, DAC2 it has one 75 ohm
 * register each.  So cable detect doesn't work for DAC3 on CATV boards.
 * customer board also needs comply. */
#define BVDC_P_SUPPORT_TDAC_VER_11                          (11) /* 7425 B0: new DAC grouping */
#define BVDC_P_SUPPORT_TDAC_VER_12                          (12) /* 7429, 7435: MISC_DAC_INST_PRBS_CTRL_0 fields moved */
#define BVDC_P_SUPPORT_TDAC_VER_13                          (13) /* 7445, 7145 */

#define BVDC_P_SUPPORT_QDAC_VER_0                            (0)
#define BVDC_P_SUPPORT_QDAC_VER_1                            (1) /* 7400, 3563 */

/* Display cracking macros. */
#define BVDC_P_DISP_GET_REG_IDX(reg) \
    ((BCHP##_##reg - BCHP_MISC_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_VDC_GET_MISC_REG_DATA(reg) \
    (hVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(reg)])

#define BVDC_P_DISP_GET_REG_DATA(reg) \
    (hDisplay->hVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(reg)])
#define BVDC_P_DISP_SET_REG_DATA(reg, data) \
    (BVDC_P_DISP_GET_REG_DATA(reg) = (uint32_t)(data))

/* Get field */
#define BVDC_P_DISP_GET_FIELD_NAME(reg, field) \
    (BVDC_P_GET_FIELD(BVDC_P_DISP_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BVDC_P_DISP_COMPARE_FIELD_DATA(reg, field, data) \
    (BVDC_P_COMPARE_FIELD_DATA(BVDC_P_DISP_GET_REG_DATA(reg), reg, field, (data)))
#define BVDC_P_DISP_COMPARE_FIELD_NAME(reg, field, name) \
    (BVDC_P_COMPARE_FIELD_NAME(BVDC_P_DISP_GET_REG_DATA(reg), reg, field, name))

/* number of registers in one block. */
#define BVDC_P_DISP_REGS_COUNT \
    BVDC_P_REGS_ENTRIES(MISC_REG_START, MISC_REG_END)

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_DISP_WRITE_TO_RUL(reg, addr_ptr) \
do { \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg); \
    *addr_ptr++ = BVDC_P_DISP_GET_REG_DATA(reg); \
} while(0)

/* these macro checks for VDC critical-section re-entry
 * it is still not perfect, but 3 check points are much more likely to catch
 */
#define BVDC_P_CHECK_CS_ENTER_VDC(vdc) \
    BDBG_ASSERT(0 == (vdc)->ulInsideCs); \
    (vdc)->ulInsideCs++; \
    BDBG_ASSERT(1 == (vdc)->ulInsideCs); \
    (vdc)->ulInsideCs++;

#define BVDC_P_CHECK_CS_LEAVE_VDC(vdc) \
    BDBG_ASSERT(2 == (vdc)->ulInsideCs); \
    (vdc)->ulInsideCs -= 2;

/* Max length for BVN error msg */
#define BVDC_P_ERROR_MAX_MSG_LENGTH    (256)

#define BVDC_P_CLIPRECT_PERCENT        (10000)

#ifndef BVDC_UINT32_ONLY
#define uintAR_t                       uint64_t
#define BVDC_P_ASPR_FRAC_BITS_NUM      (40)
#else
#define uintAR_t                       uint32_t
    /* for well bounded value such as pixel and full aspect ratio value */
#define BVDC_P_ASPR_FRAC_BITS_NUM      (16)
#endif

#define BVDC_P_PERCENTAGE_FACTOR            (100)
/* Normalization factor for 1080p display on 4k datapath */
#define BVDC_P_4K_TO_1080P_NORM_FACTOR      (4)
/* Normalization factor for 7445 boxmode 9 */
#define BVDC_P_MODE9_NORM_FACTOR            (135)
#define BVDC_P_MIN_WH_RATIO_PERCENTAGE      (95)

#define BVDC_P_WIDTH_HEIGHT_RATIO_PERCENTAGE(w, h)    \
    (((w)*(BVDC_P_PERCENTAGE_FACTOR))/(h))

/***************************************************************************
 * VDC Internal data structures
 ***************************************************************************/
/* Table for interrupt name and callback. */
typedef struct
{
    BVDC_BvnError                      eBvnError;
    BINT_Id                            ErrIntId;
    uint32_t                           ulL2ClearReg;
    uint32_t                           ulL2ClearMask;
    uint32_t                           ulGroupBase;
    uint32_t                           ulBase;
    uint32_t                           ulBvbStatus;
    BINT_CallbackFunc                  pfCallback;
    const char                        *pchInterruptName;
} BVDC_P_IntCbTbl;

/* Canvas Coverage in percentage for mosaic
 *    ulCanvasCoverage[i]: n = i + 1
 *
 *  n is the mosaic count
 */
typedef struct
{
    uint32_t   ulCanvasCoverage[12];
} BVDC_P_MosaicCanvasCoverage;

typedef struct BVDC_P_Context
{
    BDBG_OBJECT(BVDC_VDC)

    /* public fields */
    BVDC_Settings                  stSettings;

    /* handed down from app. */
    BCHP_Handle                    hChip;
    BREG_Handle                    hRegister;
    BMMA_Heap_Handle               hMemory;
    BINT_Handle                    hInterrupt;
    BRDC_Handle                    hRdc;
    BTMR_Handle                    hTmr;

    /* Created during BVDC_Open via a call to BVDC_P_InitTimer */
    BTMR_TimerHandle               hTimer;

    /* These handle get defer allocation until used by app. */
    BVDC_Source_Handle             ahSource[BVDC_P_MAX_SOURCE_COUNT];
    BVDC_Display_Handle            ahDisplay[BVDC_P_MAX_DISPLAY_COUNT];
    BVDC_Compositor_Handle         ahCompositor[BVDC_P_MAX_COMPOSITOR_COUNT];
#if BVDC_P_SUPPORT_VIP
    BVDC_P_Vip_Handle              ahVip[BVDC_P_SUPPORT_VIP];
#endif
    BVDC_P_BufferHeap_Handle       hBufferHeap;

    /* Allocated hardware resources */
    const BVDC_P_Features         *pFeatures;
    BVDC_P_Resource_Handle         hResource;

    /* Swap compositor/vec!  cmp_0 -> prim vs. cmp_0 -> sec */
    bool                           bSwapVec;

    /* Store other var that is global to VDC here.  If it's has window,
     * compositor, display, or source scope store it in respectives context. */
    BVDC_Compositor_Handle         hCmpCheckSource;

    /* Misc register (VEC's top-level registers) */
    uint32_t                       aulMiscRegs[BVDC_P_DISP_REGS_COUNT];
    uint32_t                       ulInsideCs;

    BINT_CallbackHandle            ahBvnErrHandlerCb[BVDC_BvnError_eMaxCount];
    uint32_t                       aulBvnErrCnt[BVDC_BvnError_eMaxCount];
    bool                           abBvnErrMask[BVDC_BvnError_eMaxCount];

    /* HD_DVI register shared between BVDC_P_HdDviId_eHdDvi0 and
     * BVDC_P_HdDviId_eHdDvi1. Need to keep track of it here to
     * work around the RDC read/modify/write problem */
    uint32_t                       ulHdDviBvbReg;
    uint32_t                       ulHdDviChMapReg;

#if BVDC_P_SUPPORT_MOSAIC_MODE
    /* capture drain buffer for mosaic mode, shared by all captures; */
    BMMA_Block_Handle              hVdcMosaicMmaBlock;
    BMMA_DeviceOffset              ullVdcNullBufOffset;
#endif

    /* Store the generic BVN error msg */
    BVDC_CallbackFunc_isr          pfGenericCallback;
    char                           achBuf[BVDC_P_ERROR_MAX_MSG_LENGTH];
    uint32_t                       ulApplyCnt;
    bool                           bForcePrint;

#if BVDC_P_MAX_DACS
    BVDC_DacOutput                 aDacOutput[BVDC_P_MAX_DACS];
    uint32_t                       aDacDisplay[BVDC_P_MAX_DACS];
    uint32_t                       aulDacScaleSel[BVDC_P_MAX_DACS];
    uint32_t                       aulDacSyncSource[BVDC_P_MAX_DACS];
    uint32_t                       aulDacSyncEn[BVDC_P_MAX_DACS];
    BVDC_DacConnectionState        aeDacStatus[BVDC_P_MAX_DACS];
#endif
    bool                           bCalibrated;
    bool                           bDacDetectionEnable;
    const uint32_t                *aulDacGrouping;
    uint32_t                       ulDacDetect;
    uint32_t                       ulDacDetectSyncCtrl;
    uint32_t                       ulPlugOutWait;
    uint32_t                       ulPlugInWait;

    /* Standby state */
    bool                           bStandby;

    /* Xcode window capture counter */
    uint32_t                       ulXcodeWinCap;      /* validate xcode window with capture */

    /* Xcode GFD counter */
    uint32_t                       ulXcodeGfd;        /* tracks number of used xcode GFD*/

    /* Box mode */
    BBOX_Config                    stBoxConfig;

    BVDC_P_MosaicCanvasCoverage    stMosaicCoverageTbl[BVDC_P_MAX_DISPLAY_COUNT];

    /* memory info */
    BCHP_MemoryInfo                stMemInfo;

    /* Memconfig settings */
    bool                           abSyncSlipInMemconfig[BVDC_MAX_DISPLAYS][BVDC_MAX_VIDEO_WINDOWS];
} BVDC_P_Context;


/***************************************************************************
 * VDC private functions
 ***************************************************************************/
void BVDC_P_CompositorDisplay_isr
    ( void                            *pvParam1,
      int                              iParam2 );

void BVDC_P_BuildNoOpsRul_isr
    ( BRDC_List_Handle                 hList );

void BVDC_P_ReadListInfo_isr
    ( BVDC_P_ListInfo                 *pList,
      BRDC_List_Handle                 hList );

void BVDC_P_WriteListInfo_isr
    ( const BVDC_P_ListInfo           *pList,
      BRDC_List_Handle                 hList );

void BVDC_P_Dither_Setting_isr
    ( BVDC_P_DitherSetting            *pDither,
      bool                             bDitherEn,
      uint32_t                         ulLfsrInitVale,
      uint32_t                         ulScale );

BERR_Code BVDC_P_CreateErrCb
    ( BVDC_P_Context                  *pVdc );

BERR_Code BVDC_P_DestroyErrCb
    ( BVDC_P_Context                  *pVdc );

void BVDC_P_BvnErrorHandler_isr
    ( void                            *pvhVdc,
      int                              iIdx );

const BVDC_P_IntCbTbl *BVDC_P_GetBvnErrorCb
    ( BVDC_BvnError                    eBvnErrId );

const BVDC_P_IntCbTbl *BVDC_P_GetBvnErrorCb_isr
    ( BVDC_BvnError                    eBvnErrId );

void BVDC_P_CalculateRect_isr
    ( const BVDC_ClipRect             *pClipRect,
      uint32_t                         ulWidth,
      uint32_t                         ulHeight,
      bool                             bInterlaced,
      BVDC_P_Rect                     *pRect );

bool BVDC_P_CbIsDirty_isr
    (void                           *pDirty,
     uint32_t                        ulSize );

#define BVDC_P_CbIsDirty       BVDC_P_CbIsDirty_isr

void BVDC_P_CalcuPixelAspectRatio_isr(
    BFMT_AspectRatio                 eFullAspectRatio,     /* full asp ratio enum */
    uint32_t                         ulSampleAspectRatioX, /* width of one sampled src pixel */
    uint32_t                         ulSampleAspectRatioY, /* height of one sampled src pixel */
    uint32_t                         ulFullWidth,          /* full asp ratio width */
    uint32_t                         ulFullHeight,         /* full asp ratio height */
    const BVDC_P_ClipRect*           pAspRatCnvsClip,      /* asp rat cnvs clip */
    uintAR_t *                       pulPxlAspRatio,       /* PxlAspR_int.PxlAspR_frac */
    uint32_t *                       pulPxlAspRatio_x_y,   /* PxlAspR_x<<16 | PxlAspR_y */
    BFMT_Orientation                 eOrientation );       /* orientation of the input stream  */

bool  BVDC_P_IsPxlfmtSupported
    (BPXL_Format                       ePxlFmt);

bool  BVDC_P_IsVidfmtSupported
    ( BFMT_VideoFmt                    eVideoFmt);

BERR_Code BVDC_P_CheckHeapSettings
    ( const BVDC_Heap_Settings        *pHeapSettings );

uint32_t BVDC_P_GetNumCmp
    ( const BVDC_P_Features           *pFeatures );

uint32_t BVDC_P_GetBoxWindowId_isrsafe
    ( BVDC_P_WindowId                  eWindowId );

void BVDC_P_MosaicCoverage_Init
    ( BBOX_Config                     *pBoxConfig,
      BVDC_DisplayId                   eDisplayId,
      BVDC_P_MosaicCanvasCoverage     *pCoverageTbl );

void BVDC_P_PrintHeapInfo
    ( const BVDC_Heap_Settings        *pHeap );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_PRIV_H__ */
/* End of file. */
