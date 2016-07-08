/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BVDC_ANR_PRIV_H__
#define BVDC_ANR_PRIV_H__

#include "bavc.h"
#include "breg_mem.h"      /* Chip register access (memory mapped). */
#include "bvdc_common_priv.h"
#include "bvdc_bufferheap_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_scaler_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * {private}
 *
 * Anr Sub-module Overview:
 *
 */

/*-------------------------------------------------------------------------
 * macro used by anr sub-module
 */
#define BVDC_P_ANR_BUFFER_COUNT                (4)
#define BVDC_P_SRC_INVALID                     (0xffffffff)

/* 7420: 8-bit MTCF/AND */
#define BVDC_P_MANR_VER_1                      (1) /* 7420 */
#define BVDC_P_MANR_VER_2                      (2) /* 7422, 7425 */
#define BVDC_P_MANR_VER_3                      (3) /* 7435 */
#define BVDC_P_MANR_VER_4                      (4) /* 7366a0, 7439a0, 74371a0 */
#define BVDC_P_MANR_VER_5                      (5) /* 7445D mosaic mode support*/

/***************************************************************************
 * K values look-up
 ***************************************************************************/
/*-------------------------------------------------------------------------
 * K value struct
 */
typedef struct
{
    uint16_t                           ulAlphLowThdMC;  /*MC_ALPHA_LOW_THRESHOLD*/
    uint16_t                           ulAlphLowThdNMC; /* NMC_ALPHA_LOW_THRESHOLD */
    uint16_t                           ulMcK0;       /* K0 for MC filter alpha */
    uint16_t                           ulMcK1;       /* K1 for MC filter alpha */
    uint16_t                           ulNonMcK0;    /* K0 for non-MC filter alpha */
    uint16_t                           ulNonMcK1;    /* K1 for non-MC filter alpha */
    uint16_t                           ulMcK0_CH;    /* K0 for chroma MC filter alpha */
    uint16_t                           ulMcK1_CH;    /* K1 for chroma MC filter alpha */
    uint16_t                           ulNonMcK0_CH; /* K0 for chroma non-MC filter alpha */
    uint16_t                           ulNonMcK1_CH; /* K1 for chroma non-MC filter alpha */
    uint16_t                           ulFinalK0;    /* K0 for blending results from MC and non-MC filter */
    uint16_t                           ulFinalK1;    /* K1 for blending results from MC and non-MC filter */
    uint16_t                           ulMcAdj;
    uint16_t                           ulNonMcAdj;

    /* an explicit boolean to switch MCTF filter ON/OFF; */
    bool                               bBypassFilter;
    uint16_t                           ulMctfSetting;
} BVDC_P_AnrKValue;


/*-------------------------------------------------------------------------
 * ANR status
 */
typedef struct
{
    bool                               bEnaError;  /* set when get re-enable error */

} BVDC_P_AnrStatus;


/****************************************************************************
 * Anr dirty bits to makr RUL building and executing dirty.
 */
typedef union
{
    struct
    {
        uint32_t                           bSize           : 1;
        uint32_t                           bPxlFmt         : 1;
        uint32_t                           bAnrAdjust      : 1;
        uint32_t                           bBuffer         : 1;
        uint32_t                           bChannel        : 1;
    } stBits;

    uint32_t aulInts [BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_AnrDirtyBits;

/* New alg */
#define BVDC_P_NOISE_LEVELS     5
#define BVDC_P_NOISE_HIST_NUM   8  /* (As defined in the existing code ) */
#define BVDC_P_LONG_ARRAY_LEN  64  /* Length of extended noise array to maintain a longer history, (this is for a new array) */
#define BVDC_P_PEAKS_NUM        4

/*-------------------------------------------------------------------------
 * anr main context
 */
typedef struct BVDC_P_AnrContext
{
    BDBG_OBJECT(BVDC_ANR)

    /* anr Id */
    BVDC_P_AnrId                       eId;
    uint32_t                           ulRegOffset;

    /* static info from creating */
    BREG_Handle                        hRegister;
    uint32_t                           ulMosaicMaxChannels;

    /* from acquireConnect */
    BVDC_Heap_Handle                   hHeap;
    BVDC_P_WindowId                    eWinId;

    /* buffers */
    BVDC_P_HeapNodePtr                 apHeapNode[BAVC_MOSAIC_MAX][BVDC_P_ANR_BUFFER_COUNT];
    uint32_t                           ulPxlBufCnt[BAVC_MOSAIC_MAX];
    uint32_t                           ulPxlBufSize[BAVC_MOSAIC_MAX];
    uint32_t                           ulAndThdScl;  /* due to diff size for diff video fmt */
    bool                               bEnableMaaf;  /* enable for SECAM input only */
    bool                               bBypassFilter;
    BPXL_Format                        ePxlFmt;
    BVDC_P_Compression_Settings        *pstCompression;
    uint32_t                           ulMosaicInit;

    /* for user config and src video info */
    BVDC_P_Source_InfoPtr              pCurSrcInfo;
    const BVDC_Anr_Settings           *pAnrSetting;
    BVDC_SplitScreenMode               eDemoMode;

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext               SubRul;

    /* vsync cntr for cleaning up history effect after big anr change */
    uint32_t                           ulCleanCntr;

    /* set as change is marked, clear after built into RUL */
    BVDC_P_AnrDirtyBits                stSwDirty;

    /* set after built into RUL, clear after executed */
    BVDC_P_AnrDirtyBits                stHwDirty;

    /* 422 to 444 up sampler setting */
    BVDC_422To444UpSampler             stUpSampler;
    /* 444 to 422 down sampler setting */
    BVDC_444To422DnSampler             stDnSampler;

    /* PR46735: B0: Need SECAM Dr/Db adjustment implemented */
    uint32_t                           ulAutoCtrlReg;

    /* for convenience of setting trick mode */
    uint32_t                           ulTopCtrlReg;
    uint32_t                           ulEsbConfig;
    bool                               bEsbEnable;

    /* MCTF BvbStatus */
    uint32_t                           ulBvbStatus;

    BVDC_P_BufferHeapId                eBufHeapId;
    bool                               bSplitBuf;



    /* new alg */
    bool                               bInitializeArray;
    uint32_t                           ulIframeCnt;
    uint32_t                           aulNoiseHist[BVDC_P_NOISE_HIST_NUM];
    uint32_t                           aulNoiseRatio[BVDC_P_NOISE_HIST_NUM];
    int32_t                            alDeltaArray[BVDC_P_LONG_ARRAY_LEN];
    int32_t                            alLongArray[BVDC_P_LONG_ARRAY_LEN];
    uint32_t                           ulNumNoisySampleThd;
    uint32_t                           ulNumNoisySampleThdBig;
    uint32_t                           ulNumDiffThd;
    uint32_t                           ulDropHist;
    int32_t                            lSnrMinP;

#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
    uint32_t                           ulHoldSync;
    uint32_t                           ulSyncReached;
    uint32_t                           aulPeakLocation[BVDC_P_PEAKS_NUM];
    uint32_t                           aulPeakAmplitude[BVDC_P_PEAKS_NUM];
    uint32_t                           aulFrameCount[BVDC_P_PEAKS_NUM];
    uint32_t                           ulPeakCounter;
    uint32_t                           ulWaitingCyclesToResync;
    uint32_t                           ulUnexpectedPeak;
#endif

    /* control knobs */
    bool                               bMcndEnable;   /* Enable the MCND module, default = 1 */
    bool                               bTstRepeat;    /* Enable the testing for repeat frames feature, default = 1 */
    bool                               bIPulsingReduceEnable;   /* Enable the I pulsing compensation/reduction feature, default = 1 */
    bool                               bFixMcCurve;   /* 1=> Assign fixed blend profile for MC */
    uint32_t                           ulMinMethod;   /* 0=> From instantaneous snr , 1=> from avg. snr , curr = 0 */
    uint32_t                           ulFilterUpdate;  /* 0=> Using snr_avg ,  1=> Using the min of snr_inst or snr_avg (depending upon the MIN_METHOD) , */
                                                        /* 2=> Using (snr_avg-snr_delta_avg), curr = 1 */
    uint32_t                           ulRndCeil;   /* 2=> round , 3=> ceil , default = 3 */


} BVDC_P_AnrContext;


/***************************************************************************
 * private functions
 ***************************************************************************/
#define BVDC_P_Anr_MuxAddr(hAnr)           (0)
#define BVDC_P_Anr_PostMuxValue(hAnr)      (0)

#define BVDC_P_Anr_SetUsrSettingPtr(hAnr, pAnrSetting) \
    (hAnr)->pAnrSetting = (pAnrSetting)
#define BVDC_P_Anr_SetSrcInfo(hAnr, pSrcInfo) \
    (hAnr)->pCurSrcInfo = (pSrcInfo)

#define BVDC_P_Anr_SetVnet_isr(hAnr, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((hAnr)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_Anr_UnsetVnet_isr(hAnr) \
   BVDC_P_SubRul_UnsetVnet_isr(&((hAnr)->SubRul))

#define BVDC_P_Anr_SetBufPxlFmt_isr(hAnr, eNewPxlFmt) \
   if ((hAnr)->ePxlFmt != (eNewPxlFmt)) { \
       (hAnr)->ePxlFmt = (eNewPxlFmt); \
       (hAnr)->stSwDirty.stBits.bPxlFmt = BVDC_P_DIRTY; \
   }


#define BVDC_P_Anr_SetBufNodes_isr(hAnr, ppHeapNode, ulChannelId) \
  do { \
      int ii; \
      for (ii=0; ii<(int)( BVDC_P_ANR_BUFFER_COUNT); ii++) \
          (hAnr)->apHeapNode[ulChannelId][ii] = (ppHeapNode)[ii]; \
  } while (0)

/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_Create
 *
 * called by BVDC_Open only
 */
BERR_Code BVDC_P_Anr_Create
    ( BVDC_P_Anr_Handle *              phAnr,
      BVDC_P_AnrId                     eAnrId,
      BREG_Handle                      hRegister,
      BVDC_P_Resource_Handle           hResource );

/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_Destroy
 *
 * called by BVDC_Close only
 */
BERR_Code BVDC_P_Anr_Destroy
    ( BVDC_P_Anr_Handle                hAnr );

void BVDC_P_Anr_Init_isr
    ( BVDC_P_Anr_Handle              hAnr,
      BVDC_Window_Handle             hWindow);

/***************************************************************************
 * {private}
 *
 * BVDC_P_Anr_SetDemoMode
 *
 * called by BVDC_Window_ApplyChanges to set anr demo mode
 */
BERR_Code BVDC_P_Anr_SetDemoMode_isr
    ( BVDC_P_Anr_Handle                hAnr,
      BVDC_SplitScreenMode             eDemoMode );

/*-------------------------------------------------------------------------
 * look up K value struct
 */
const BVDC_P_AnrKValue * BVDC_P_Anr_GetKValue_isr(
    uint32_t                           ulSnrDb,
    BVDC_P_CtInput                     eCtInputType,
    const BFMT_VideoInfo              *pFmtInfo,
    void                              *pvUserInfo );

/*-------------------------------------------------------------------------
 *
 */
void BVDC_P_Anr_BuildRul_SrcInit_isr(
    BVDC_P_AnrContext             *pAnr,
    BVDC_P_ListInfo               *pList,
    BVDC_P_PictureNode            *pPicture );

/*-------------------------------------------------------------------------
 *
 */
void BVDC_P_Anr_BuildRul_SetEnable_isr(
    BVDC_P_AnrContext             *pAnr,
    BVDC_P_PictureNode            *pPicture,
    BVDC_P_ListInfo               *pList,
    bool                           bEnable);

/*-------------------------------------------------------------------------
 *
 */
void BVDC_P_Anr_BuildRul_StatisRead_isr
    ( BVDC_P_AnrContext             *pAnr,
      BVDC_P_ListInfo               *pList);

/*-------------------------------------------------------------------------
 * New alg
 */
void  BVDC_P_Anr_McndReview_isr
    ( BVDC_P_Anr_Handle           hAnr,
      uint32_t                   *pulNoiseSampleNum,
      uint32_t                   *pulNoiseMsb,
      uint32_t                   *pulNoiseLsb,
      BVDC_P_AnrKValue           *pKValue );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_ANR_PRIV_H__ */
/* End of file. */
