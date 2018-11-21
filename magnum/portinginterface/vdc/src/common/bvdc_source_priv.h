/***************************************************************************
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
 ***************************************************************************/
#ifndef BVDC_SOURCE_PRIV_H__
#define BVDC_SOURCE_PRIV_H__

#include "bstd.h"             /* standard types */
#include "bkni.h"             /* memcpy calls */
#include "bvdc.h"             /* Video display */
#include "bvdc_common_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_anr_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/

#define BVDC_P_SRC_IS_ITU656(source_id) \
    ((BAVC_SourceId_e656In0==(source_id)) || \
     (BAVC_SourceId_e656In1==(source_id)))

#define BVDC_P_SRC_IS_MPEG(source_id) \
     (BAVC_SourceId_eMpegMax>=(source_id))

#define BVDC_P_SRC_IS_GFX(source_id) \
    ((BAVC_SourceId_eGfx0<=(source_id)) && \
     (BAVC_SourceId_eGfxMax>=(source_id)))

#define BVDC_P_SRC_IS_HDDVI(source_id) \
    ((BAVC_SourceId_eHdDvi0==(source_id)) || \
     (BAVC_SourceId_eHdDvi1==(source_id)))

#define BVDC_P_SRC_IS_VIDEO(source_id) \
    ((BVDC_P_SRC_IS_MPEG(source_id)) || \
     (BVDC_P_SRC_IS_VFD(source_id)) || \
     (BVDC_P_SRC_IS_HDDVI(source_id)) || \
     (BVDC_P_SRC_IS_ITU656(source_id)))

#define BVDC_P_SRC_NEED_DRAIN(source_id) \
    ((BVDC_P_SRC_IS_HDDVI(source_id)) || \
     (BVDC_P_SRC_IS_ITU656(source_id)))

#define BVDC_P_SRC_IS_VFD(source_id) \
    ((BAVC_SourceId_eVfd0<=(source_id)) && \
     (BAVC_SourceId_eVfdMax>=(source_id)))

#define BVDC_P_SRC_GET_LIST_IDX(polarity_id, idx) \
    (BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT * (polarity_id) + (idx))

/* For RUL multi-buffering. */
#define BVDC_P_SRC_NEXT_RUL(pSource, polarity_id) \
    ((pSource)->aulRulIdx[(polarity_id)] = \
    BVDC_P_NEXT_RUL_IDX((pSource)->aulRulIdx[(polarity_id)]))

/* Get the current list pointed by aulRulIdx[field]. */
#define BVDC_P_SRC_GET_LIST(pSource, polarity_id) \
    ((pSource)->ahList[BVDC_P_SRC_GET_LIST_IDX(polarity_id, \
        (pSource)->aulRulIdx[polarity_id])])

/* MosaicMode: rotate all 4 slave RULs, to avoid premature overwrite of slave
 * RUL in case it's not executed yet, which is easy to happen since slave RULs
 * execution(multiple triggers) spreads across the whole frame and back-to-back
 * callbacks would easily run over the double-buffer slave RULs once for
 * progressive format;
 */
#define BVDC_P_SRC_GET_NEXT_SLAVE_RUL_IDX(hSource) \
{ \
    uint32_t tmp = ((hSource)->ulSlaveRulIdx + 1) & 3; \
    ((hSource)->ulSlaveRulIdx = ((tmp == 1)||(tmp == 2)) ? \
        tmp : 0); \
}

/* Src uses all T/B/F slots. */
#define BVDC_P_SRC_MAX_SLOT_COUNT \
    (BVDC_P_MAX_POLARITY)

#define BVDC_P_SRC_MAX_LIST_COUNT \
    (BVDC_P_SRC_MAX_SLOT_COUNT * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT)

/* Get the current slot pointed by field. */
#define BVDC_P_SRC_GET_SLOT(pSource, polarity_id) \
    ((pSource)->ahSlot[(polarity_id)])

#define BVDC_P_SRC_VALID_COMBO_TRIGGER(eCombTrigger) \
    (((eCombTrigger) >= BRDC_Trigger_eComboTrig0) &&  \
     ((eCombTrigger) <= BRDC_Trigger_eComboTrig3))

#define BVDC_P_SRC_FIELD_DATA_MASK(r, f, v) (BCHP_FIELD_DATA(r, f, v) & BCHP_MASK(r, f))
#define BVDC_P_SRC_VALIDATE_FIELD(r, f, v) (BVDC_P_SRC_FIELD_DATA_MASK(r, f, v) == (BCHP_FIELD_DATA(r, f, v)))

/* This is to ensure that we don't have a big scale up. */
#define BVDC_P_SRC_INPUT_H_MIN             (64)
#define BVDC_P_SRC_INPUT_V_MIN             (64)

#define BVDC_P_HdDvi_PostMuxValue(hHdDvi)  (BCHP_VNET_F_SCL_0_SRC_SOURCE_HD_DVI_0 + (hHdDvi)->eId)

#if (BVDC_P_SUPPORT_656_IN > 1)
#define BVDC_P_656In_PostMuxValue(h656In)    \
   ((0 == (h656In)->eId)? BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656_0 : BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656_1)
#else
#define BVDC_P_656In_PostMuxValue(h656In)    (BCHP_VNET_F_SCL_0_SRC_SOURCE_CCIR_656_0)
#endif

#define BVDC_P_Source_PostMuxValue(hSrc) \
   (BVDC_P_SRC_IS_MPEG((hSrc)->eId) ?    BVDC_P_Feeder_PostMuxValue((hSrc)->hMpegFeeder) : \
    BVDC_P_SRC_IS_VFD((hSrc)->eId) ?    BVDC_P_Feeder_PostMuxValue((hSrc)->hVfdFeeder) : \
    BVDC_P_SRC_IS_ITU656((hSrc)->eId) ?  BVDC_P_656In_PostMuxValue((hSrc)->h656In)       : \
    /*BVDC_P_SRC_IS_HDDVI((hSrc)->eId)?*/BVDC_P_HdDvi_PostMuxValue((hSrc)->hHdDvi))

/* Oversample */
#define BVDC_P_OVER_SAMPLE(factor, value)            (((factor)+1) * (value))

/* Who control the slot triggering. */
typedef enum BVDC_P_TriggerCtrl
{
    BVDC_P_TriggerCtrl_eSource = 0,
    BVDC_P_TriggerCtrl_eXfering,
    BVDC_P_TriggerCtrl_eDisplay

} BVDC_P_TriggerCtrl;

#define BVDC_P_NULL_SOURCE                 ((BAVC_SourceId)(-1))

/****************************************************************************/
/* Making an entry for src params                                           */
/****************************************************************************/
#define BVDC_P_MAKE_SRC_PARAMS(e_source_id, e_trig_0, e_trig_1)              \
{                                                                            \
    BAVC_SourceId_##e_source_id,                                             \
{                                                                        \
    BRDC_Trigger_##e_trig_0,                                             \
    BRDC_Trigger_##e_trig_1,                                             \
    BRDC_Trigger_UNKNOWN                                                 \
}                                                                        \
}

#define BVDC_P_MAKE_SRC_PARAMS_NULL(e_source_id)                             \
{                                                                            \
    (e_source_id),                                                           \
{                                                                        \
    BRDC_Trigger_UNKNOWN,                                                \
    BRDC_Trigger_UNKNOWN,                                                \
    BRDC_Trigger_UNKNOWN                                                 \
}                                                                        \
}

typedef struct
{
    BAVC_SourceId             eSourceId;
    BRDC_Trigger              aeTrigger[BVDC_P_SRC_MAX_SLOT_COUNT];
} BVDC_P_SourceParams;


/****************************************************************************
 * Source dirty bits for building RUL.  New and other flags should be moving
 * as needed.
 */
typedef union
{
    struct
    {
        uint32_t                         bWinChanges       : 1; /* Bit[0]: from window user setting */
        uint32_t                         bUserChanges      : 1; /* Bit[1]: from user, no detect */

        /* Common dirty bits (for all source) */
        uint32_t                         bPicCallback      : 1; /* Bit[2]: from user */
        uint32_t                         bGenCallback      : 1; /* Bit[3]: from user */
        uint32_t                         bWindowNum        : 1; /* Bit[4]: from window */
        uint32_t                         bAddWin           : 1; /* Bit[5]: from window */
        uint32_t                         bRecAdjust        : 1; /* Bit[6]: ScanOut Rect changed. */

        uint32_t                         bInputFormat      : 1; /* Bit[7]: from user for VDEC
                                                                * from user or XVD for HD_DVI */

        uint32_t                         bAspectRatio      : 1; /* Bit[8]: from user */
        uint32_t                         bAspectRatioClip  : 1; /* Bit[9]: from user */

        /* mosaic dirty bits */
        uint32_t                         bMosaicIntra      : 1; /* Bit[10]: from user */

        /* VDEC's dirty bits obselete */
        uint32_t                         bAutoDetectFmt    : 1; /* Bit[13]: from user */
        uint32_t                         bManualPos        : 1; /* Bit[16]: from user */
        uint32_t                         bVideoDetected    : 1; /* Bit[18]: from detection */
        uint32_t                         bFrameRateCode    : 1; /* Bit[21]: from detection */
        uint32_t                         bNoisy            : 1; /* Bit[22]: from detection */
        uint32_t                         bFvFhShift        : 1; /* Bit[23]: from detection */

        /* user CSC for VDEC or HD_DVI or xvd/mvd for MPEG */
        uint32_t                         bColorspace       : 1; /* Bit[25]: from xvd/mvd or user */

        /* HD_DVI dirty bits */
        uint32_t                         bMiscCtrl         : 1; /* Bit[26]: from user */

        /* DNR: MNR, BNR */
        uint32_t                         bDnrAdjust        : 1; /* Bit[27]: from user */

        /* MPEG's dirty bit */
        uint32_t                         bPsfMode          : 1; /* Bit[28]: from user */

        /* MPEG, HDDVI, VDEC: Indicate to resume */
        uint32_t                         bResume           : 1; /* Bit[29]: from user */

        /* MPEG, HDDVI */
        uint32_t                         bOrientation       : 1; /* Bit[31]: from user */
    } stBits;

    uint32_t aulInts [BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_Source_DirtyBits;


/***************************************************************************
 * Source Context
 ***************************************************************************/
typedef struct BVDC_P_Source_Info
{
    /*+-----------------------+
      | Common                |
      +-----------------------+ */
    /* Color used for fixed color feed for MPEG feeder */
    uint8_t                          ucRed;
    uint8_t                          ucGreen;
    uint8_t                          ucBlue;
    uint32_t                         ulMuteColorYCrCb;
    BVDC_MuteMode                    eMuteMode;

    /* Fix color for each field polarity */
    bool                             bFixColorEnable;
    uint32_t                         aulFixColorYCrCb[3];

    /* User CSC */
    bool                             bUserCsc;
    uint32_t                         ulUserShift;
    int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT];

    /* User Frontend CSC */
    bool                             bUserFrontendCsc;
    uint32_t                         ulUserFrontendShift;
    int32_t                          pl32_FrontendMatrix[BVDC_CSC_COEFF_COUNT];

    /* aspect-ratio canvas clipping (in pixel unit) */
    BVDC_P_ClipRect                  stAspRatRectClip;

    /* Input video format. Used by VDEC, HD_DVI, and DS source */
    const BFMT_VideoInfo            *pFmtInfo;
    const BVDC_P_FormatInfo         *pVdcFmt;
    bool                             bAutoFmtDetection;
    bool                             bAutoDetect;
    uint32_t                         ulNumFormats;
    BFMT_VideoFmt                    aeFormats[BFMT_VideoFmt_eMaxCount];
    BFMT_AspectRatio                 eAspectRatio;
    uint32_t                         ulWindows;      /* Number of window created with this source. */

    /* HVStart config for PC In and HD_DVI source */
    bool                             bHVStartOverride;
    uint32_t                         ulHstart;
    uint32_t                         ulVstart;

    /* Orientation */
    bool                             bOrientationOverride;
    BFMT_Orientation                 eOrientation;

    /* DNR filter user settings */
    bool                             bDnr;
    BVDC_Dnr_Settings                stDnrSettings;

    /* This is used for demo mode */
    BVDC_Source_SplitScreenSettings  stSplitScreenSetting;

    /* Set by source detection, and/or window parameter.  */
    bool                             bDeinterlace;

    /* Source generic callback */
    BVDC_CallbackFunc_isr            pfGenericCallback;
    void                            *pvGenericParm1;
    int                              iGenericParm2;
    BVDC_Source_CallbackSettings     stCallbackSettings;

    /* Source Pending flags */
    BVDC_ResumeMode                  eResumeMode;
    bool                             bForceSrcPending;

    /* derived in validate for ease of access, and save computation in _isr. */
    BVDC_P_CtInput                   eCtInputType;

    /* PR29659: C0: Support 1080p @ 60 through component input. */
    bool                             bUsePllClk;    /* Use PLL, or running at 216Mhz for non-pc */

    /*+-----------------------------+
      | HD_DVI, VFD, and GFX Source |
      +-----------------------------+ */
    /* Callback function for HD_DVI, VFD, and GFX source */
    BVDC_Source_PictureCallback_isr  pfPicCallbackFunc;
    void                            *pvParm1;
    int                              iParm2;
    uint32_t                         ulInputPort;
    BVDC_HdDvi_Settings              stHdDviSetting;

    /*+-----------------------+
      | Mpeg Feeder Source    |
      +-----------------------+ */
    bool                             bMosaicMode;
    /* This flag indicates the scanout mode for the windows connecting to the
     * MPEG source. The scanout mode for the source is stored in feeder */
    BVDC_P_ScanoutMode               eWinScanoutMode;
    bool                             bPsfEnable;
    bool                             bForceFrameCapture;
    /*  SW7425-686 XVD and BVN display rate mismatch */
    bool                             bFrameRateMismatch;

    BVDC_Source_CrcType              eCrcType;

    /* Dirty bits of source. */
    BVDC_P_Source_DirtyBits          stDirty;

    /* Flag to indicate last error from user setting */
    bool                             bErrorLastSetting;

} BVDC_P_Source_Info;

typedef struct
{
    /*+-----------------------+
      | Common                |
      +-----------------------+ */
    /* aspect-ratio canvas clipping (in pixel unit) */
    BVDC_P_ClipRect                  stAspRatRectClip;

    /* Input video format. Used by VDEC and HD_DVI source */
    BFMT_AspectRatio                 eAspectRatio;

    /* Dirty bits of new isr setting */
    BVDC_P_Source_DirtyBits          stDirty;

    /* for copying activated isr setting into new info in next ApplyChanges */
    BVDC_P_Source_DirtyBits          stActivated;

    /* Flag to indicate last error from user setting */
    bool                             bErrorLastSetting;

} BVDC_P_Source_IsrInfo;

typedef struct BVDC_P_SourceContext
{
    BDBG_OBJECT(BVDC_SRC)

    /* Input signal loss / bad causes unable to startFeed to downstream */
    bool                      bStartFeed;
    bool                      bPrevStartFeed;

    /* public fields that expose thru API. */
    BVDC_P_Source_Info        stNewInfo;
    BVDC_P_Source_Info        stCurInfo;
    BVDC_P_Source_IsrInfo     stIsrInfo;

    /* Set to true when new & old validated by apply changes   These
     * flags get updated at applychanges. */
    bool                      bUserAppliedChanges;

    BAVC_SourceId             eId;            /* Identify the source */
    BVDC_P_State              eState;         /* Context state. */
    bool                      bInitial;       /* No Rul executed yet */
    uint32_t                  ulConnectedWindow;
    uint32_t                  ulTransferLock;
    BVDC_Compositor_Handle    hCmpToBeLocked;
    bool                      bFieldSwap;
    bool                      bPrevFieldSwap;

    /* Event to nofify that changes has been applied to hardware. */
    BKNI_EventHandle          hAppliedDoneEvent;

    /* RUL use for this source */
    BAVC_Polarity             eNextFieldId;
    BAVC_Polarity             eNextFieldIntP;
    BAVC_Polarity             eNextFieldFake;
    uint32_t                  ulSlotUsed;
    uint32_t                  aulRulIdx[BVDC_P_SRC_MAX_SLOT_COUNT];
    BRDC_Trigger              aeTrigger[BVDC_P_SRC_MAX_SLOT_COUNT];
    BRDC_Trigger              eCombTrigger;
    BRDC_Slot_Handle          ahSlot[BVDC_P_SRC_MAX_SLOT_COUNT];
    BRDC_List_Handle          ahList[BVDC_P_SRC_MAX_LIST_COUNT];
    uint32_t                  ulSlaveRulIdx; /* current slave list index to ahListSlave */
    BRDC_Slot_Handle          hSlotSlave;    /* only need 1 slave slot each source */
    BRDC_List_Handle          ahListSlave[2 * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT];
    BINT_CallbackHandle       ahCallback[BVDC_P_SRC_MAX_SLOT_COUNT]; /* For HdDvi, 656, & Analog */

    /* Trigger lost, and artificial trigger work-around. */
    BINT_CallbackHandle       hTrigger0Cb;
    BINT_CallbackHandle       hTrigger1Cb;
    BVDC_P_TriggerCtrl        eTrigCtrl;
    uint32_t                  ulSrcTrigger;
    uint32_t                  ulVecTrigger;

    /* Slot's force trigger reg addr. */
    uint32_t                  aulImmTriggerAddr[BVDC_P_SRC_MAX_SLOT_COUNT];

    /* Saved dirty bits */
    BVDC_P_Source_DirtyBits   astOldDirty[BVDC_P_SRC_MAX_SLOT_COUNT];

    /* One of these source. */
    BVDC_P_HdDvi_Handle       hHdDvi;
    BVDC_P_GfxFeeder_Handle   hGfxFeeder;
    BVDC_P_Feeder_Handle      hMpegFeeder;
    BVDC_P_Feeder_Handle      hVfdFeeder;
    BVDC_P_656In_Handle       h656In;

    bool                      bSrcIs444;
    bool                      bSrcInterlaced;

    /* this affects XSRC allocation */
    bool                      bIs10BitCore;
    bool                      bIs2xClk;

    /* Hold the previous field from mvd. */
    BAVC_MVD_Field            stPrevMvdField;
    BAVC_MVD_Field            stNewPic[BAVC_MOSAIC_MAX];

    BAVC_VDC_HdDvi_Picture    stNewXvdField;
    bool                      bPictureChanged;
    bool                      bRasterChanged;     /* I <-> P */
    BAVC_MatrixCoefficients   eMatrixCoefficients; /* Source color space */

    /* video format info used with callback for ext (non-enumerated) mpeg video format */
    BFMT_VideoInfo            stExtVideoFmtInfo;

    /* the CRC capture flag of two pictures ago
     * Note: when a picture is required to capture CRC, its RUL is executed one
     *       vsync later; and its CRC computation is completed at end of that
     *       picture, i.e. another vsync later! */
    bool                      bCaptureCrc;

    /* The windows using/displaying this source. */
    BVDC_Window_Handle        ahWindow[BVDC_P_MAX_WINDOW_COUNT];

    /* Source information prepare to hand back to user when it's changed. */
    BVDC_Source_CallbackData  stSourceCbData;

    /* TODO: get rid off ANR's status */
    BVDC_P_AnrStatus          stAnrStatus;

    /* created from VDC */
    BVDC_Handle               hVdc;          /* Created from this Vdc */

    /* Internal VDC or App handed down. */
    BVDC_Heap_Handle          hHeap;

    /* Derived flags from context */
    BVDC_Compositor_Handle    hSyncLockCompositor;
    BVDC_P_Rect               stScanOut;

    /* Source refresh rate; */
    uint32_t                  ulVertFreq;            /* Vert refresh rate in Hz */
    uint32_t                  ulClkPerVsync;         /* Sysclk per vsync */
    BAVC_FrameRateCode        eFrameRateCode;        /* convert to frame rate code */

    /* Source content rate */
    uint32_t                  ulStreamVertFreq;

    /* for src aspect ratio override */
    bool                      bNewUserModeAspRatio;

    /* number of shutdown windows */
    uint32_t                  ulShutdownWins;

    /* vnet drain for the source */
    BVDC_P_DrainContext       stDrain;

    /* Oversample? */
    uint32_t                  ulSampleFactor; /* (1+ulSampleFactor) */

    /* PsF scanout state: if just being scaned-out, don't do it again; */
    bool                      bPsfScanout; /* store the scanout state of the previous 1080p */
    uint32_t                  ulDropFramesCnt;
    uint32_t                  ulDispVsyncFreq;/* MPG driving display vsync rate */

    /* Source is currently in pending, free memory allocations */
    bool                      bPending;

    /* defer source pending callback until all its windows are shutdown! */
    bool                      bDeferSrcPendingCb;
    /* source fresh rate mismatch with VEC */
    uint32_t                  ulRefreshRateMismatchCntr;

    /* Compression */
    bool                      bCompression;
    bool                      bWait4ReconfigVnet;

    BRDC_Trigger              aeCombTriggerList[BVDC_P_MAX_WINDOW_COUNT];
    uint32_t                  aulMosaicZOrderIndex[BAVC_MOSAIC_MAX];
    uint32_t                  aulChannelId[BAVC_MOSAIC_MAX];
    uint32_t                  ulMosaicCount;
    /* Used for cadence handling in mosaic mode */
    uint32_t                  ulMosaicFirstUnmuteRectIndex;
    bool                      bMosaicFirstUnmuteRectIndexSet;

    bool                      bEnablePsfBySize;

    /* Used for ignore picture handling in NRT mode */
    uint32_t                  ulScratchPolReg;
#if BVDC_P_STG_NRT_CADENCE_WORKAROUND /* to prompt sync-locked NRT display RUL a repeat pol occured */
    bool                      bToggleCadence; /* everytime ignore picture would toggle this flag */
#endif

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK0
    uint32_t                  ulHdmiPwrAcquire;
    uint32_t                  ulHdmiPwrRelease;
    uint32_t                  ulHdmiPwrId;
#endif

    bool                      b4kSupportPresent; /* this MFD has 4k hw via MFD_0_HW_CONFIGURATION.SUPPORTS_4K */
    bool                      bMtgIsPresent; /* this MFD has MTG hw via MFD_0_HW_CONFIGURATION.MFD_TRIGGER */
    bool                      bGfxSrc; /* this MFD src is used to feed gfx surface */
    bool                      bMtgSrc; /* this MFD src uses MTG */

    /* frameRateCode for MFD */
    uint32_t                  ulDefMfdVertRefRate;
    BAVC_FrameRateCode        eDefMfdVertRateCode;
    BAVC_FrameRateCode        eMfdVertRateCode;
    BVDC_Window_Handle        hMfdVertDrivingWin;
    bool                      bUsedMadAtWriter;

    /* total amount of pixels */
    uint32_t                  ulPixelCount;

    bool                      bPqNcl;
#if BVDC_P_SUPPORT_MTG
    BVDC_Display_Handle       hDspTimebaseLocked;
    BAVC_Timebase             eTimeBase;
#endif

#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
    uint32_t                  ulStcFlag; /* STC flag to trigger decoder STC snapshot */
    unsigned                  ulStcFlagTrigSel;
#endif
} BVDC_P_SourceContext;


/***************************************************************************
 * Private functions
 ***************************************************************************/
BERR_Code BVDC_P_Source_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Source_Handle              *phSource,
      BAVC_SourceId                    eSourceId,
      BVDC_P_Resource_Handle           hResource,
      bool                             b3dSrc );

void BVDC_P_Source_Destroy
    ( BVDC_Source_Handle               hSource );

BERR_Code BVDC_P_Source_Init
    ( BVDC_Source_Handle               hSource,
      const BVDC_Source_CreateSettings *pDefSettings );

BERR_Code BVDC_P_Source_ValidateChanges
    ( const BVDC_Source_Handle         ahSource[] );

void BVDC_P_Source_ApplyChanges_isr
    ( BVDC_Source_Handle               hSource );

void BVDC_P_Source_AbortChanges
    ( BVDC_Source_Handle               hSource );

void BVDC_P_Source_UpdateSrcState_isr
    ( BVDC_Source_Handle               hSource );

void BVDC_P_Source_UpdateStatus_isr
    ( BVDC_Source_Handle               hSource );

void BVDC_P_Source_ConnectWindow_isr
    ( BVDC_Source_Handle               hSource,
      BVDC_Window_Handle               hWindow );

void BVDC_P_Source_DisconnectWindow_isr
    ( BVDC_Source_Handle               hSource,
      BVDC_Window_Handle               hWindow );

void BVDC_P_Source_GetScanOutRect_isr
    ( BVDC_Source_Handle               hSource,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      BVDC_P_Rect                     *pScanOutRect );

void BVDC_P_Source_GetWindowVnetmodeInfo_isr
    ( BVDC_Source_Handle               hSource );

void BVDC_P_Source_MpegGetStatus_isr
    ( const BVDC_Source_Handle         hSource,
      bool                            *pbVideoDetected );

void BVDC_P_Source_BuildRul_isr
    ( const BVDC_Source_Handle         hSource,
      BVDC_P_ListInfo                 *pList );

void BVDC_P_Source_CleanupSlots_isr
    ( BVDC_Source_Handle               hSource );

void BVDC_P_Source_FindLockWindow_isr
    ( BVDC_Source_Handle               hSource,
      bool                             bUpdate );

void BVDC_P_Source_AnalogDataReady_isr
    ( void                            *pvSourceHandle,
      int                              iParam2 );

void BVDC_P_Source_VfdGfxDataReady_isr
    ( BVDC_Source_Handle               hSource,
      BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList,
      int                              iParam2 );

void BVDC_P_Source_MfdGfxCallback_isr
    ( void                            *pvSourceHandle,
      int                              iField);

void BVDC_P_Source_HdDviDataReady_isr
    ( void                            *pvSourceHandle,
      int                              iParam2 );

#if (BVDC_P_SUPPORT_HDDVI)
void BVDC_P_Source_UpdateFrameRate_isr
    ( const BFMT_VideoInfo            *pFmtInfo,
      uint32_t                         ulClkPerVsync,
      uint32_t                         ulDelta,
      BAVC_FrameRateCode              *peFrameRateCode );
#endif /* BVDC_P_SUPPORT_HDDVI */

void BVDC_P_Source_CheckAndIssueCallback_isr
    ( BVDC_P_SourceContext            *pSource,
      BVDC_Source_CallbackMask        *pCbMask );

BERR_Code BVDC_P_Source_GetOutputOrientation_isr
    ( BVDC_Source_Handle               hSource,
      const BAVC_MVD_Field            *pFieldData,
      const BFMT_VideoInfo            *pFmtInfo,
      BFMT_Orientation                *peOutOrientation );

uint32_t BVDC_P_Source_RefreshRate_FromFrameRateCode_isrsafe
    ( BAVC_FrameRateCode               eFrameRateCode );

BAVC_FrameRateCode BVDC_P_Source_RefreshRateCode_FromRefreshRate_isrsafe
    ( uint32_t                         ulVertRefreshRate);

#if (BVDC_P_SUPPORT_MTG)
BAVC_FrameRateCode BVDC_P_Source_MtgRefreshRate_FromFrameRateCode_isrsafe
    ( BVDC_Source_Handle               hSource,
      BAVC_FrameRateCode               eFrameRateCode );
#endif /* (BVDC_P_SUPPORT_MTG) */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_SOURCE_PRIV_H__ */
/* End of file. */
