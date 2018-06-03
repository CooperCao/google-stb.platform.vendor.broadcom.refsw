/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef BVDC_COMPOSITOR_PRIV_H__
#define BVDC_COMPOSITOR_PRIV_H__

#include "bvdc.h"
#include "bchp_common.h"
#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_cfc_priv.h"
#include "bchp_cmp_0.h"
#ifdef BCHP_HDR_CMP_0_REG_START
#include "bchp_hdr_cmp_0.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#define BVDC_P_CMP_OFFSET_GET_REG_IDX(reg, offset) \
    ((BCHP##_##reg + offset - BCHP_CMP_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset) \
    (hCompositor->aulRegs[BVDC_P_CMP_OFFSET_GET_REG_IDX(reg, offset)])
#define BVDC_P_CMP_OFFSET_SET_REG_DATA(reg, offset, data) \
    (BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset) = (uint32_t)(data))

#define BVDC_P_CMP_OFFSET_GET_REG_DATA_I(idx, reg, offset) \
    (hCompositor->aulRegs[BVDC_P_CMP_OFFSET_GET_REG_IDX(reg,offset) + (idx)])

/* Get field */
#define BVDC_P_CMP_OFFSET_GET_FIELD_NAME(reg, offset, field) \
    (BVDC_P_GET_FIELD(BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset), reg, field))

/* Compare field */
#define BVDC_P_CMP_OFFSET_COMPARE_FIELD_DATA(reg, offset, field, data) \
    (BVDC_P_COMPARE_FIELD_DATA(BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset), reg, field, (data)))
#define BVDC_P_CMP_OFFSET_COMPARE_FIELD_NAME(reg, offset, field, name) \
    (BVDC_P_COMPARE_FIELD_NAME(BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset), reg, field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_CMP_OFFSET_WRITE_TO_RUL(reg, offset, addr_ptr) \
{ \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hCompositor->ulRegOffset + offset); \
    *addr_ptr++ = BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, offset); \
}


/* This macro does a block write into RUL */
#define BVDC_P_CMP_OFFSET_BLOCK_WRITE_TO_RUL(from, to, offset, pulCurrent) \
do { \
    uint32_t ulBlockSize = \
        BVDC_P_REGS_ENTRIES(from, to);\
    *pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulBlockSize ); \
    *pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hCompositor->ulRegOffset+ offset); \
    BKNI_Memcpy((void*)pulCurrent, \
        (void*)&(hCompositor->aulRegs[BVDC_P_CMP_OFFSET_GET_REG_IDX(from, offset)]), \
        ulBlockSize * sizeof(uint32_t)); \
    pulCurrent += ulBlockSize; \
} while(0)

/* This macro does a block write into RUL */
#define BVDC_P_CMP_OFFSET_RECT_BLOCK_WRITE_TO_RUL(from, cnt, offset, pulCurrent) \
do { \
    *pulCurrent++ = BRDC_OP_IMMS_TO_REGS( cnt ); \
    *pulCurrent++ = BRDC_REGISTER(BCHP##_##from + hCompositor->ulRegOffset + offset); \
    BKNI_Memcpy((void*)pulCurrent, \
        (void*)&(hCompositor->aulRegs[BVDC_P_CMP_OFFSET_GET_REG_IDX(from, offset)]), \
        (cnt) * sizeof(uint32_t)); \
    pulCurrent += cnt; \
} while(0)

#define BVDC_P_CMP_GET_REG_IDX(reg) BVDC_P_CMP_OFFSET_GET_REG_IDX(reg, 0)

    /* Get/Set reg data */
#define BVDC_P_CMP_GET_REG_DATA(reg) \
    BVDC_P_CMP_OFFSET_GET_REG_DATA(reg, 0)

#define BVDC_P_CMP_SET_REG_DATA(reg, data) \
    BVDC_P_CMP_OFFSET_SET_REG_DATA(reg, 0, data)

#define BVDC_P_CMP_GET_REG_DATA_I(idx, reg) \
    BVDC_P_CMP_OFFSET_GET_REG_DATA_I(idx, reg, 0)

    /* Get field */
#define BVDC_P_CMP_GET_FIELD_NAME(reg, field) \
    BVDC_P_CMP_OFFSET_GET_FIELD_NAME(reg, 0, field)

    /* Compare field */
#define BVDC_P_CMP_COMPARE_FIELD_DATA(reg, field, data) \
    BVDC_P_CMP_OFFSET_COMPARE_FIELD_DATA(reg, 0, field, data)
#define BVDC_P_CMP_COMPARE_FIELD_NAME(reg, field, name) \
    BVDC_P_CMP_OFFSET_COMPARE_FIELD_NAME(reg, 0, field, name)

    /* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_CMP_WRITE_TO_RUL(reg, addr_ptr) \
    BVDC_P_CMP_OFFSET_WRITE_TO_RUL(reg, 0, addr_ptr)

    /* This macro does a block write into RUL */
#define BVDC_P_CMP_RECT_BLOCK_WRITE_TO_RUL(from, cnt, pulCurrent) \
    BVDC_P_CMP_OFFSET_BLOCK_WRITE_TO_RUL(from, cnt, 0, pulCurrent)
#define BVDC_P_CMP_REGS_COUNT \
    BVDC_P_REGS_ENTRIES(CMP_0_REG_START, CMP_0_REG_END)


#define BVDC_P_CMP_GET_LIST_IDX(polarity_id, idx) \
    (BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT * (polarity_id) + (idx))

/* For RUL multi-buffering. */
#define BVDC_P_CMP_NEXT_RUL(hCompositor, polarity_id) \
    ((hCompositor)->aulRulIdx[(polarity_id)] = \
    BVDC_P_NEXT_RUL_IDX((hCompositor)->aulRulIdx[(polarity_id)]))

/* Get the current list pointed by aulRulIdx[field]. */
#define BVDC_P_CMP_GET_LIST(hCompositor, polarity_id) \
    ((hCompositor)->ahList[BVDC_P_CMP_GET_LIST_IDX((polarity_id), \
        (hCompositor)->aulRulIdx[(polarity_id)])])

/* Compositor only uses T/B slot. */
#define BVDC_P_CMP_MAX_SLOT_COUNT \
    (2)

#define BVDC_P_CMP_MAX_LIST_COUNT \
    (BVDC_P_CMP_MAX_SLOT_COUNT * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT)

/* Get the current slot pointed by field. */
#define BVDC_P_CMP_GET_SLOT(hCompositor, polarity_id) \
    ((hCompositor)->ahSlot[(polarity_id)])

#define BVDC_P_CMP_GET_DISP_TOP_TRIGGER(hCompositor) \
    (BVDC_P_DISP_GET_TOP_TRIGGER((hCompositor)->hDisplay))

#define BVDC_P_CMP_GET_DISP_BOT_TRIGGER(hCompositor) \
    (BVDC_P_DISP_GET_BOT_TRIGGER((hCompositor)->hDisplay))

/* Number of blender availables. */
#define BVDC_P_CMP_MAX_BLENDER        (BVDC_Z_ORDER_MAX + 1)

#define BVDC_P_CMP_GET_V0ID(hCompositor) \
    (((hCompositor)->eId >= BVDC_CompositorId_eCompositor2)? \
     (BVDC_P_WindowId_eComp2_V0 + ((hCompositor)->eId - BVDC_CompositorId_eCompositor2)) :\
     (BVDC_P_WindowId_eComp0_V0 + ((hCompositor)->eId - BVDC_CompositorId_eCompositor0) *2))

/* Compositor feature entry */
typedef struct
{
    uint32_t                          ulMaxVideoWindow;
    uint32_t                          ulMaxGfxWindow;
    uint32_t                          ulMaxWindow;

} BVDC_P_Compositor_Features;

/* Compositor dirty bits */
typedef union
{
    struct
    {
        uint32_t                          bColorClip           : 1; /* new colorclip settings */
        uint32_t                          bOutColorSpace       : 1; /* color space changed */
        /* BVDC_P_Window_ValidateChanges checks diff of curDisp and newDisp, should change ??? */
    } stBits;

    uint32_t aulInts[BVDC_P_DIRTY_INT_ARRAY_SIZE];

} BVDC_P_Compositor_DirtyBits;

/* Compositor infos */
typedef struct
{
    uint32_t                          ulBgColorYCrCb;
    uint8_t                           ucRed;
    uint8_t                           ucGreen;
    uint8_t                           ucBlue;
    const BFMT_VideoInfo             *pFmtInfo;
    BVDC_ColorClipSettings            stColorClipSettings;
    BFMT_Orientation                  eOrientation;

    /* dirty bits */
    BVDC_P_Compositor_DirtyBits       stDirty;
} BVDC_P_Compositor_Info;


/***************************************************************************
 * Compositor Context
 ***************************************************************************/
typedef struct BVDC_P_CompositorContext
{
    BDBG_OBJECT(BVDC_CMP)

    /* flag initial state, requires reset; */
    bool                              bIsBypass;
    bool                              bInitial;
    uint32_t                          ulCoreResetAddr;
    uint32_t                          ulCoreResetMask;

    /* public fields that expose thru API. */
    BVDC_P_Compositor_Info            stNewInfo;
    BVDC_P_Compositor_Info            stCurInfo;

    /* for display to infom window ApplyChange */
    bool                              bDspAspRatDirty;

    /* Set to true when new & old validated by apply changes  These
     * flags get updated at applychanges. */
    bool                              bUserAppliedChanges;
    BVDC_Window_Handle                hSyncLockWin; /* window locked to this compositor */
    BVDC_Source_Handle                hSyncLockSrc; /* source locked to this compositor */
    BVDC_Source_Handle                hForceTrigPipSrc;
    BVDC_Source_Handle                hSrcToBeLocked;
    uint32_t                          ulSlip2Lock;
#if BVDC_P_SUPPORT_STG
    BVDC_Compositor_Handle            hCmpToLock; /* the sync-slaved compositor to grab the sync lock */
    bool                              bSyncSlave;
#endif

    /* RUL use for this compositor & display, (not created by compositor) */
    uint32_t                          aulRulIdx[BVDC_P_CMP_MAX_SLOT_COUNT];
    BRDC_Slot_Handle                  ahSlot[BVDC_P_CMP_MAX_SLOT_COUNT];
    BRDC_List_Handle                  ahList[BVDC_P_CMP_MAX_LIST_COUNT];
    BINT_CallbackHandle               ahCallback[BVDC_P_CMP_MAX_SLOT_COUNT];

    /* shadowed registers */
    BVDC_CompositorId                 eId;
    BVDC_P_State                      eState;
    uint32_t                          ulRegOffset; /* CMP_0, CMP_1, and etc. */
    uint32_t                          aulRegs[BVDC_P_CMP_REGS_COUNT];

    /* Compositor features. */
    const BVDC_P_Compositor_Features *pFeatures;

    /* Computed value */
    uint32_t                          ulActiveVideoWindow;
    uint32_t                          ulActiveGfxWindow;
    bool                              abBlenderUsed[BVDC_P_CMP_MAX_BLENDER];
    BVDC_P_WindowId                   aeBlenderWinId[BVDC_P_CMP_MAX_BLENDER];


    /* Compositor output to Display STG*/

    /* DW-1 MBOX Original PTS */
    uint32_t                          ulOrigPTS;
    int32_t                           uiHorizontalPanScan;   /* MPEG-2 Data format*/
    int32_t                           uiVerticalPanScan;     /* Same as above*/
    uint32_t                          ulDisplayHorizontalSize;
    uint32_t                          ulDisplayVerticalSize;
    uint32_t                          ulPicId;
    BAVC_USERDATA_PictureCoding       ePictureType;
    uint32_t                          ulChannelId;
    BAVC_Polarity                     eSourcePolarity;
    bool                              bPictureRepeatFlag;      /*picture repeat due to cadence detection or frame rate conversion*/
    bool                              bStgIgnorePicture;       /* actual ignore flag used for encoder */
    bool                              bCrcToIgnore, bCrcIgnored;/* delayed ignore flags(due to RUL delay and EOP property of CRC) used for CRC capture */
    bool                              bIgnorePicture;          /* from DM*/
    bool                              bStallStc;               /* from DM*/
    bool                              bLast;                   /* from DM*/
    bool                              bChannelChange;          /* from DM*/
    bool                              bGfxChannelChange;       /* GFX window indicator, maintained by vdc*/
    bool                              bMute;                   /* mute flag from DM */
    uint32_t                          ulStgPxlAspRatio_x_y;    /* PxlAspRatio_x<<16 | PxlAspRatio_y */
    uint32_t                          ulDecodePictureId;
    bool                              bValidAfd;
    uint32_t                          ulAfd;
    BAVC_BarDataType                  eBarDataType;
    uint32_t                          ulTopLeftBarValue;
    uint32_t                          ulBotRightBarValue;
    BFMT_Orientation                  eDspOrientation;

    /*  Fast non real time (FNRT) meta data support */
    bool                              bPreChargePicture;
    bool                              bEndofChunk;
    uint32_t                          ulChunkId;

    /* Ouptut to VEC. */
    BAVC_FrameRateCode                eSrcFRateCode;
    bool                              bFullRate;
    BCFC_ColorSpaceExt                stOutColorSpaceExt;


    /* active windows (declare max).  Could also be dynamically allocated
     * to BVDC_P_CMP_X_MAX_WINDOW_COUNT.  But this is pretty much fix. */
    BVDC_Window_Handle                ahWindow[BVDC_P_MAX_WINDOW_COUNT];
    uint32_t                          ulCscAdjust[BVDC_P_MAX_WINDOW_COUNT];
    bool                              bCscCompute[BVDC_P_MAX_WINDOW_COUNT];
    bool                              bCscDemoCompute[BVDC_P_MAX_WINDOW_COUNT];
    uint32_t                          ulColorKeyAdjust[BVDC_P_MAX_WINDOW_COUNT];
    uint32_t                          ulMosaicAdjust[BVDC_P_MAX_WINDOW_COUNT];
    uint32_t                          ulNLCscCtrl[BVDC_P_MAX_VIDEO_WINS_PER_CMP];  /* V0 and V1, for BlackBoxNLConv */
    bool                              bBypassDviCsc;
    bool                              bUnknownHdrMetadata;
    BAVC_StaticHdrMetadata            stStaticHdrMetadata;
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)
    bool                              bBlendMatrixOn;
    uint8_t                           ucBlendMatrixOnRulBuildCntr;
#endif
#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
    uint32_t                          aulNLCfg[BVDC_P_MAX_VIDEO_WINS_PER_CMP][BVDC_P_CMP_VER2_NL_CFG_REGS]; /* V0 and V1, 8 regs */
#endif
    BCFC_Capability                   stCfcCapability[BVDC_P_MAX_VIDEO_WINS_PER_CMP];  /* V0 and V1 */
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
    /* CFC LUT heap */
    BMMA_Heap_Handle                  hCfcHeap; /* must be cpu accessible for LUT fill */
    BCFC_LutLoadListInfo              stCfcLutList; /* for CFC ram table loading */
    /* DBV support */
#if BVDC_P_DBV_SUPPORT
    BVDC_P_DBV_Info                  * pstDbv;
#endif /* BVDC_P_DBV_SUPPORT */
#if BVDC_P_TCH_SUPPORT
    BVDC_P_TCH_Info                  * pstTch;
#endif /* BVDC_P_DVS_SUPPORT */
#endif /* BVDC_P_CMP_CFC_VER >= 3 */

    /* this affects dvi dither setting */
    bool                              bIs10BitCore;
    bool                              bIs2xClk;
    bool                              bInDither;
    bool                              bAlign12Bit;
    bool                              bHdr;

    /* Dither */
    uint32_t                          ulDitherChange[BVDC_P_MAX_WINDOW_COUNT];
    bool                              bInDitherEnable[BVDC_P_MAX_WINDOW_COUNT];
    bool                              bCscDitherEnable[BVDC_P_MAX_WINDOW_COUNT];
    BVDC_P_DitherSetting              stInDither;
    BVDC_P_DitherSetting              stCscDither;

    /* Associated w/ this display handle. */
    BVDC_Display_Handle               hDisplay;

     /* Created from this vdc */
    BVDC_Handle                       hVdc;
} BVDC_P_CompositorContext;


/***************************************************************************
 * Compositor private functions
 ***************************************************************************/
BERR_Code BVDC_P_Compositor_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Compositor_Handle          *phCompositor,
      BVDC_CompositorId                eCompositorId );

void BVDC_P_Compositor_Destroy
    ( BVDC_Compositor_Handle           hCompositor );

void BVDC_P_Compositor_Init
    ( BVDC_Compositor_Handle           hCompositor );

BERR_Code BVDC_P_Compositor_ValidateChanges
    ( const BVDC_Compositor_Handle     ahCompositor[] );

void BVDC_P_Compositor_ApplyChanges_isr
    ( BVDC_Compositor_Handle           hCompositor );

void BVDC_P_Compositor_AbortChanges
    ( BVDC_Compositor_Handle           hCompositor );

void BVDC_P_Compositor_BuildSyncLockRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId );

bool BVDC_P_Compositor_BuildSyncSlipRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId,
      bool                             bBuildCanvasCtrl );

void BVDC_P_Compositor_BuildConvasCtrlRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList );

/* Miscellaneous access functions */

BERR_Code BVDC_P_Compositor_AssignTrigger_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BRDC_Trigger                     eTopTrigger,
      BRDC_Trigger                     eBotTrigger );

void BVDC_P_Compositor_WindowsReader_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BAVC_Polarity                    eNextFieldId,
      BVDC_P_ListInfo                 *pList );

uint32_t BVDC_P_Compositor_GetCmpRegAddr_isr
    ( BVDC_CompositorId                eId,
      uint32_t                         ulRegAddr);

void BVDC_P_Compositor_SetMBoxMetaData_isr
    (
    const BVDC_P_PictureNode              *pPicture,
    BVDC_Compositor_Handle                hCompositor);

/* configure hCompositor->stOutColorSpaceExt for all cases,
 * plus display->stOutColorSpaceExt and
 * hDisplay->stCfc.stColorSpaceExtIn.stColorSpace for hdmi out
 *
 * note: CMP always output limited range YCbCr
 *
 */
void BVDC_P_Compositor_UpdateOutColorSpace_isr
    ( BVDC_Compositor_Handle           hCompositor,
      bool                             bApplyChanges );

/* Build RUL for blend out matrix
 */
void BVDC_P_Compositor_BuildBlendOutMatrixRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList);

/* init hdr_cmp ram table
 */
void BVDC_P_Compositor_InitHdrRam
    ( BVDC_Compositor_Handle           hCompositor );

void BVDC_P_Compositor_GetCfcCapabilities
    ( BREG_Handle                      hRegister,
      BVDC_CompositorId                eCmpId,
      BVDC_WindowId                    eWinId,
      BCFC_Capability                 *pCapability );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_COMPOSITOR_PRIV_H__ */
/* End of file. */
