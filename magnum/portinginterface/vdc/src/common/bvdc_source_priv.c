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
 *
* Module Description:
*
***************************************************************************/

#include "bstd.h"
#include "bvdc_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_dnr_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_hddvi_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_656in_priv.h"
#include "bfmt.h"
#include "bmma.h"

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
#include "bvdc_mcvp_priv.h"
#include "bvdc_mcdi_priv.h"
#endif

BDBG_MODULE(BVDC_SRC);
BDBG_FILE_MODULE(BVDC_REPEATPOLARITY);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_OBJECT_ID(BVDC_SRC);

#ifndef BVDC_FOR_BOOTUPDATER

/****************************************************************************/
/* Inline helpers                                                           */
/****************************************************************************/
#define BVDC_P_SRC_RETURN_IF_ERR(result) \
    if ( BERR_SUCCESS != (result)) \
{\
    return BERR_TRACE(result);  \
}

#if (BVDC_P_SUPPORT_HDDVI)
/****************************************************************************/
/* Number of entries use this to loop, and/or allocate memory.              */
/****************************************************************************/
#define BVDC_P_FRT_COUNT_INTERLACED \
    (sizeof(s_aIntFramerateTbl)/sizeof(BVDC_P_FrameRateEntry))

#define BVDC_P_FRT_COUNT_PROGRESSIVE \
    (sizeof(s_aProFramerateTbl)/sizeof(BVDC_P_FrameRateEntry))

/****************************************************************************/
/* Making an entry for Frame Rate                                           */
/****************************************************************************/
#define BVDC_P_MAKE_FRT(e_frame_rate, vert_freq, ppt)                        \
{                                                                            \
    BAVC_FrameRateCode_##e_frame_rate,                                       \
    ((BVDC_P_BVB_BUS_CLOCK / (vert_freq * 1000)) * (ppt)),                   \
    (#e_frame_rate)                                                          \
}

/* Frame rate detection base on clk per vsync */
typedef struct
{
    BAVC_FrameRateCode             eFramerate;
    uint32_t                       ulClkPerVsync;
    const char                    *pchName;

} BVDC_P_FrameRateEntry;

/* Static frame rate lookup table. */
static const BVDC_P_FrameRateEntry s_aIntFramerateTbl[] =
{
    BVDC_P_MAKE_FRT(e29_97  , 60, 1001),
    BVDC_P_MAKE_FRT(e30     , 60, 1000),
    BVDC_P_MAKE_FRT(e25     , 50, 1000),
    BVDC_P_MAKE_FRT(e23_976 , 48, 1001),
    BVDC_P_MAKE_FRT(e24     , 48, 1000)
};

static const BVDC_P_FrameRateEntry s_aProFramerateTbl[] =
{
    BVDC_P_MAKE_FRT(e59_94  , 60, 1001),
    BVDC_P_MAKE_FRT(e60     , 60, 1000),
    BVDC_P_MAKE_FRT(e29_97  , 30, 1001),
    BVDC_P_MAKE_FRT(e30     , 30, 1000),
    BVDC_P_MAKE_FRT(e25     , 25, 1000),
    BVDC_P_MAKE_FRT(e50     , 50, 1000),
    BVDC_P_MAKE_FRT(e23_976 , 24, 1001),
    BVDC_P_MAKE_FRT(e24     , 24, 1000)
};
#endif /* BVDC_P_SUPPORT_HDDVI */

/* INDEX: By source id, do not put ifdefs and nested ifdefs around these that
* become impossible to decipher.  The eSourceId != BVDC_P_NULL_SOURCE will
* indicate that this source's trigger (lost) is to be monitor or poll by
* display interrupt. */
static const BVDC_P_SourceParams s_aSrcParams[] =
{
    /* Mpeg feeder 0/1/2/3/4/5 */
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    /* Analog Vdec 0/1 (swapped trigger) */
    BVDC_P_MAKE_SRC_PARAMS(eVdec0, eVdec0Trig1, eVdec0Trig0),
    BVDC_P_MAKE_SRC_PARAMS(eVdec1, eVdec1Trig1, eVdec1Trig0),
    /* Digital ITU-R-656 0/1 (swapped trigger)*/
    BVDC_P_MAKE_SRC_PARAMS(e656In0, e6560Trig1, e6560Trig0),
    BVDC_P_MAKE_SRC_PARAMS(e656In1, e6561Trig1, e6561Trig0),
    /* Gfx feeder 0/1/2/3/4/5/6 */
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    /* HD_DVI input 0/1 */
    BVDC_P_MAKE_SRC_PARAMS(eHdDvi0, eHdDvi0Trig0, eHdDvi0Trig1),
    BVDC_P_MAKE_SRC_PARAMS(eHdDvi1, eHdDvi1Trig0, eHdDvi1Trig1),
    /* Ds0 */
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    /* VFD feeder 0/1/2/3/4/5/6/7 */
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE),
    BVDC_P_MAKE_SRC_PARAMS_NULL(BVDC_P_NULL_SOURCE)
};

static const char s_chExtVideoFmtName[] =
    BDBG_STRING_INLINE("BFMT_VideoFmt: Ext");

/***************************************************************************
* {private}
*
*/
static void BVDC_P_SourceTrigger_isr
    ( void                            *pvhSource,
      int                              iParam1 )
{
    uint32_t i;
    BVDC_Source_Handle hSource = (BVDC_Source_Handle)pvhSource;
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    BSTD_UNUSED(iParam1);
    /* Test to see if we have reliable trigger source.  Once it's reliable
    * slot done RUL will disable this callback, and vec will enable when
    * it detects source trigger is lost. */
    if((BVDC_P_TriggerCtrl_eDisplay == hSource->eTrigCtrl) &&
        (hSource->ulSrcTrigger))
    {
        hSource->ulSrcTrigger--;
        if(!hSource->ulSrcTrigger)
        {
            hSource->eTrigCtrl    = BVDC_P_TriggerCtrl_eXfering;
            hSource->ulSrcTrigger = BVDC_P_TRIGGER_LOST_THRESHOLD;
            BDBG_WRN((" (S) Source[%d] enters Xfer state", hSource->eId));
        }
    }
    else if(BVDC_P_TriggerCtrl_eSource == hSource->eTrigCtrl)
    {
        /* Disable trigger interrupt (this func). */
        BINT_DisableCallback_isr(hSource->hTrigger0Cb);
        BINT_DisableCallback_isr(hSource->hTrigger1Cb);
        BINT_ClearCallback_isr(hSource->hTrigger0Cb);
        BINT_ClearCallback_isr(hSource->hTrigger1Cb);

        /* Setup slot to receive triggers again. */
        for(i = 0; i < hSource->ulSlotUsed; i++)
        {
            BRDC_Slot_ExecuteOnTrigger_isr(hSource->ahSlot[i],
                hSource->aeTrigger[i], true);
        }
        BDBG_WRN((" (S) Source[%d] re-acquires control of its slots", hSource->eId));
    }

    return;
}


/***************************************************************************
* {private}
*
* (1) combine connected windows' vbi pass thru settings
* (2) combine newly connected windows' vbi pass thru settings
* (3) combine newly dis-connected windows' vbi pass thru settings
*/
static void BVDC_P_Source_RefreshWindow_isr
    ( BVDC_Source_Handle               hSource,
      bool                             bUpdateApplyChanges )
{
    uint32_t i;
    bool bMosaicMode;
    bool bDeinterlace;
    BVDC_Window_Handle hWindow;
    BVDC_P_Source_Info *pNewInfo;
    BVDC_P_Source_Info *pCurInfo;
    BVDC_P_Source_DirtyBits *pNewDirty;
    const BVDC_P_Window_Info     *pWinInfo;
    const BVDC_P_Compositor_Info *pCmpInfo;
    BVDC_P_ScanoutMode            eWinScanoutMode;

    BDBG_ENTER(BVDC_P_Source_RefreshWindow_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    pCurInfo  = &hSource->stCurInfo;
    pNewInfo  = &hSource->stNewInfo;
    pNewDirty = &pNewInfo->stDirty;

    /* Initialize as if no vbi pass-thru, etc. */
    bMosaicMode   = false;
    bDeinterlace  = false;
    eWinScanoutMode  = pCurInfo->eWinScanoutMode;

    /* Search all window connected to this source, and see if it has vbi
    * pass-thru, mad, phase, etc. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if((!hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_DESTROY(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_SHUTDOWNRUL(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_SHUTDOWNPENDING(hSource->ahWindow[i]))
        {
            continue;
        }
        hWindow = hSource->ahWindow[i];
        BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

        pWinInfo = &hWindow->stNewInfo;
        pCmpInfo = &hWindow->hCompositor->stNewInfo;

        /* If this source has a window with deinterlacer */
        if(pWinInfo->bMosaicMode)
        {
            bMosaicMode = true;
        }

        /* If this source has a window with deinterlacer */
        if(pWinInfo->bDeinterlace)
        {
            bDeinterlace = true;
        }

        if(BFMT_IS_4kx2k(pCmpInfo->pFmtInfo->eVideoFmt) &&
           (BBOX_Vdc_SclCapBias_eAutoDisable1080p == hWindow->eBoxSclCapBias))
        {
            eWinScanoutMode = BVDC_P_GetScanoutMode4AutoDisable1080p_isr(&pWinInfo->stScalerOutput);
        }
    }

    /* source VBI changes will be reflected into the next RUL */
    if((pCurInfo->bDeinterlace  != bDeinterlace ) ||
        (pCurInfo->bMosaicMode  != bMosaicMode  ) ||
        (pCurInfo->eWinScanoutMode != eWinScanoutMode ))
    {
        /* set new info and dirty bits. */
        pNewInfo->bMosaicMode   = bMosaicMode;
        pNewInfo->bDeinterlace  = bDeinterlace;
        pNewInfo->eWinScanoutMode  = eWinScanoutMode;
        pNewDirty->stBits.bWinChanges  = BVDC_P_DIRTY; /* bWinChanges */

        /* On a connect/disconnect source is already gone thru applychanges
        * that's mean that it didn't detected these change.  We're now
        * mark the apply changes, so it can be pickup in next _isr */
        if(bUpdateApplyChanges)
        {
            hSource->bUserAppliedChanges = true;
        }
    }

    BDBG_LEAVE(BVDC_P_Source_RefreshWindow_isr);
    return;
}


/***************************************************************************
*
*/
static void BVDC_P_Source_Bringup_isr
    ( BVDC_Source_Handle               hSource )
{
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(BVDC_P_SRC_IS_ITU656(hSource->eId))
    {
#if (BVDC_P_SUPPORT_NEW_656_IN_VER)
        BVDC_P_656In_Bringup_isr(hSource->h656In);
#endif
    }
    else if(BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BVDC_P_HdDvi_Bringup_isr(hSource->hHdDvi);
    }
#if BVDC_P_SUPPORT_MTG
    else if (hSource->bMtgSrc)
    {
        BVDC_P_Feeder_Mtg_Bringup_isr(hSource->hMpegFeeder);
    }
#endif

    return;
}


/***************************************************************************
*
*/
static void BVDC_P_Source_DisableTriggers_isr
    ( BVDC_Source_Handle               hSource )
{
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        BVDC_P_HdDvi_DisableTriggers_isr(hSource->hHdDvi);
    }
#if BVDC_P_SUPPORT_MTG
    else if (hSource->bMtgSrc)
    {
        BVDC_P_Feeder_Mtg_DisableTriggers_isr(hSource->hMpegFeeder);
    }
#endif

    return;
}

/***************************************************************************
* {private}
*
*/
BERR_Code BVDC_P_Source_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Source_Handle              *phSource,
      BAVC_SourceId                    eSourceId,
      BVDC_P_Resource_Handle           hResource,
      bool                             b3dSrc )
{
    uint32_t i;
    BRDC_SlotId eSlotId;
    BVDC_P_SourceContext *pSource;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Source_Create);
    BDBG_ASSERT(phSource);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* BDBG_SetModuleLevel("BVDC_SRC", BDBG_eMsg); */

    /* (1) Alloc the window context. */
    pSource = (BVDC_P_SourceContext*)
        (BKNI_Malloc(sizeof(BVDC_P_SourceContext)));
    if(!pSource)
    {
        eStatus = BERR_OUT_OF_SYSTEM_MEMORY;
        goto BVDC_P_Source_Create_Done;
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pSource, 0x0, sizeof(BVDC_P_SourceContext));
    BDBG_OBJECT_SET(pSource, BVDC_SRC);

    /* Initialize non-changing states.  These are not to be changed by runtime. */
    pSource->eId           = eSourceId;
    pSource->hVdc          = hVdc;

    /* Note non-mpeg source only uses 2 slots */
    pSource->ulSlotUsed    = BVDC_P_SRC_IS_MPEG(pSource->eId)
        ? (BVDC_P_SRC_MAX_SLOT_COUNT) : (BVDC_P_SRC_MAX_SLOT_COUNT - 1);

    /* Triggers for each source . */
    for(i = 0; i < pSource->ulSlotUsed; i++)
    {
        pSource->aeTrigger[i] = s_aSrcParams[eSourceId].aeTrigger[i];
    }

    /* (2) Create RDC List & Slot */
    if(BVDC_P_SRC_IS_VIDEO(pSource->eId) && !BVDC_P_SRC_IS_VFD(pSource->eId))
    {
        for(i = 0; i < (pSource->ulSlotUsed * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT); i++)
        {
            eStatus = BRDC_List_Create(hVdc->hRdc, BVDC_P_MAX_ENTRY_PER_RUL, &pSource->ahList[i]);
            if(BERR_SUCCESS != eStatus)
            {
                goto BVDC_P_Source_Create_Done;
            }
        }

        eStatus = BRDC_Slots_Create(hVdc->hRdc, pSource->ahSlot, pSource->ulSlotUsed);
        if(BERR_SUCCESS != eStatus)
        {
            goto BVDC_P_Source_Create_Done;
        }
        for(i = 0; i < pSource->ulSlotUsed; i++)
        {
            BRDC_Slot_UpdateLastRulStatus_isr(pSource->ahSlot[i], pSource->ahList[i], true);
            /* Slot's force trigger reg addr. */
            eStatus = BRDC_Slot_GetId(pSource->ahSlot[i], &eSlotId);
            if(BERR_SUCCESS != eStatus)
            {
                goto BVDC_P_Source_Create_Done;
            }
            pSource->aulImmTriggerAddr[i] = BCHP_RDC_desc_0_immediate +
                (eSlotId * (BCHP_RDC_desc_1_immediate - BCHP_RDC_desc_0_immediate));

            /* Enable RDC priority for HDDVI source */
            if(BVDC_P_SRC_IS_HDDVI(pSource->eId))
            {
                BRDC_Slot_Settings  stSlotSettings;

                BRDC_Slot_GetConfiguration(pSource->ahSlot[i], &stSlotSettings);
                stSlotSettings.bHighPriority = true;
                eStatus = BRDC_Slot_SetConfiguration(pSource->ahSlot[i], &stSlotSettings);
                if(BERR_SUCCESS != eStatus)
                {
                    goto BVDC_P_Source_Create_Done;
                }
            }
            BDBG_MSG(("Source[%d] uses slot[%d]", pSource->eId, eSlotId));
        }

        if(!pSource->hVdc->stSettings.bDisableMosaic)
        {
            /* MosaicMode: create slave slot and list for mpeg source; */
            if(BVDC_P_SRC_IS_MPEG(pSource->eId))
            {
                /* frame src shares the same combined trigger as top field */
                /* mpeg0: combo0; mpeg1: combo1; mpeg2: combo2 */
                if(pSource->eId < BRDC_MAX_COMBO_TRIGGER_COUNT)
                {
                    pSource->eCombTrigger = BRDC_Trigger_eComboTrig0 + pSource->eId;

                    /* Only need 3 slave lists and one slave slot*/
                    for(i = 0; i < BVDC_P_MAX_MULTI_SLAVE_RUL_BUFFER_COUNT; i++)
                    {
                        eStatus = BRDC_List_Create(hVdc->hRdc, BVDC_P_MAX_ENTRY_PER_MPEG_RUL,
                            &pSource->ahListSlave[i]);
                        if(BERR_SUCCESS != eStatus)
                        {
                            goto BVDC_P_Source_Create_Done;
                        }
                    }

                    BRDC_Slot_Create(hVdc->hRdc, &pSource->hSlotSlave);
                    BRDC_Slot_GetId(pSource->hSlotSlave, &eSlotId);
                    BDBG_MSG(("Source[%d] uses slave slot[%d]", pSource->eId, eSlotId));
                }
                else
                {
                    pSource->eCombTrigger = BRDC_Trigger_UNKNOWN;
                }
                BDBG_MSG(("Src[%d] use combo trigger %d", pSource->eId, pSource->eCombTrigger));
            }
        }
    }

    /* (3) For analog/656/HdDvi sources VDC will handle the callback. */
    if(BVDC_P_SRC_IS_ITU656(pSource->eId) ||
       BVDC_P_SRC_IS_HDDVI(pSource->eId))
    {
        /* RUL done execution callbacks. */
        for(i = 0; i < pSource->ulSlotUsed; i++)
        {
            eStatus = BINT_CreateCallback(&pSource->ahCallback[i], hVdc->hInterrupt,
                BRDC_Slot_GetIntId(pSource->ahSlot[i]),
                BVDC_P_SRC_IS_HDDVI(pSource->eId) ? BVDC_P_Source_HdDviDataReady_isr
                : BVDC_P_Source_AnalogDataReady_isr,
                (void*)pSource, i);
            if(BERR_SUCCESS != eStatus)
            {
                goto BVDC_P_Source_Create_Done;
            }
        }
    }

    /* (4) Triggers detection callback. */
    if(BVDC_P_NULL_SOURCE != s_aSrcParams[eSourceId].eSourceId)
    {
        const BRDC_TrigInfo *pTrig0Info, *pTrig1Info;

        /* Trigger information. */
        pTrig0Info = BRDC_Trigger_GetInfo(hVdc->hRdc, s_aSrcParams[eSourceId].aeTrigger[0]);
        pTrig1Info = BRDC_Trigger_GetInfo(hVdc->hRdc, s_aSrcParams[eSourceId].aeTrigger[1]);

        BDBG_ASSERT(pTrig0Info && pTrig1Info);
        BDBG_MSG(("    Trigger lost detection for source[%d]: %s, %s",
            pSource->eId, pTrig0Info->pchTrigName, pTrig0Info->pchTrigName));

        /* Make need to update table due to source enum update. */
        BDBG_ASSERT(s_aSrcParams[eSourceId].eSourceId == eSourceId);
        eStatus = BINT_CreateCallback(&pSource->hTrigger0Cb, hVdc->hInterrupt,
            pTrig0Info->TrigIntId, BVDC_P_SourceTrigger_isr, (void*)pSource,
            pTrig0Info->TrigIntId);
        if(BERR_SUCCESS != eStatus)
        {
            goto BVDC_P_Source_Create_Done;
        }
        eStatus = BINT_CreateCallback(&pSource->hTrigger1Cb, hVdc->hInterrupt,
            pTrig1Info->TrigIntId, BVDC_P_SourceTrigger_isr, (void*)pSource,
            pTrig1Info->TrigIntId);
        if(BERR_SUCCESS != eStatus)
        {
            goto BVDC_P_Source_Create_Done;
        }
    }

    /* (5) Created a specific source handle. */
    switch(eSourceId)
    {
    case BAVC_SourceId_eMpeg0:
    case BAVC_SourceId_eMpeg1:
    case BAVC_SourceId_eMpeg2:
    case BAVC_SourceId_eMpeg3:
    case BAVC_SourceId_eMpeg4:
    case BAVC_SourceId_eMpeg5:
#if BVDC_P_SUPPORT_STG
        pSource->ulScratchPolReg = BRDC_AllocScratchReg(pSource->hVdc->hRdc);
        if(0 == pSource->ulScratchPolReg) {
            BDBG_ERR(("out of scratch registers for source [%d]!", eSourceId));
        }
        BDBG_MODULE_MSG(BVDC_REPEATPOLARITY, ("Allocated src[%d] scratch register %#x to hold REPEAT_POLARITY swap flag",
            eSourceId, pSource->ulScratchPolReg));
#endif
        /* STC flag for MFD source */
#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
        BKNI_EnterCriticalSection();
        pSource->ulStcFlag = BRDC_AcquireStcFlag_isr(hVdc->hRdc, BRDC_MAX_STC_FLAG_COUNT);
        pSource->ulStcFlagTrigSel = -1;
        BKNI_LeaveCriticalSection();
        BDBG_MSG(("MFD%d stc flag = %d", eSourceId, pSource->ulStcFlag));
        if(pSource->ulStcFlag == BRDC_MAX_STC_FLAG_COUNT)
        {
            BDBG_ERR(("No STC flag available for MFD source %d!", eSourceId));
        }
#endif

        BVDC_P_Feeder_Create(&pSource->hMpegFeeder, hVdc->hRdc, hVdc->hRegister,
            BVDC_P_FeederId_eMfd0 + eSourceId - BAVC_SourceId_eMpeg0,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
            hVdc->hTimer,
#endif
            pSource, hResource, true); /* MFD always allocates double shadow scratch registers for separate y/c buffers */
        break;

    case BAVC_SourceId_eGfx0:
    case BAVC_SourceId_eGfx1:
    case BAVC_SourceId_eGfx2:
    case BAVC_SourceId_eGfx3:
    case BAVC_SourceId_eGfx4:
    case BAVC_SourceId_eGfx5:
    case BAVC_SourceId_eGfx6:
        BVDC_P_GfxFeeder_Create(
            &pSource->hGfxFeeder, hVdc->hRegister, hVdc->hRdc, eSourceId, b3dSrc, pSource);
        break;

#if (BVDC_P_SUPPORT_NEW_656_IN_VER)

    case BAVC_SourceId_e656In0:
    case BAVC_SourceId_e656In1:
        BVDC_P_656In_Create(&pSource->h656In,
            BVDC_P_656Id_e656In0 + eSourceId - BAVC_SourceId_e656In0, pSource);
        break;
#endif

    case BAVC_SourceId_eHdDvi0:
    case BAVC_SourceId_eHdDvi1:
        BVDC_P_HdDvi_Create(&pSource->hHdDvi,
            BVDC_P_HdDviId_eHdDvi0 + eSourceId - BAVC_SourceId_eHdDvi0, hVdc->hRegister, pSource);
        break;

    case BAVC_SourceId_eVfd0:
    case BAVC_SourceId_eVfd1:
    case BAVC_SourceId_eVfd2:
    case BAVC_SourceId_eVfd3:
    case BAVC_SourceId_eVfd4:
    case BAVC_SourceId_eVfd5:
    case BAVC_SourceId_eVfd6:
    case BAVC_SourceId_eVfd7:
        BVDC_P_Feeder_Create(&pSource->hVfdFeeder, hVdc->hRdc, hVdc->hRegister,
            BVDC_P_FeederId_eVfd0 + eSourceId - BAVC_SourceId_eVfd0,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
            hVdc->hTimer,
#endif
            pSource, hVdc->hResource, b3dSrc);
        break;

    default:
        BDBG_ERR(("Unknown source to create."));
        BDBG_ASSERT(false);
    }

    /* (6) create a AppliedDone event. */
    BKNI_CreateEvent(&pSource->hAppliedDoneEvent);

    /* (7) Added this compositor to hVdc */
    hVdc->ahSource[eSourceId] = (BVDC_Source_Handle)pSource;

    /* All done. now return the new fresh context to user. */
    *phSource = (BVDC_Source_Handle)pSource;

BVDC_P_Source_Create_Done:
    BDBG_LEAVE(BVDC_P_Source_Create);

    if((BERR_SUCCESS != eStatus) && (NULL != pSource))
    {
        for(i = 0; i < pSource->ulSlotUsed; i++)
        {
            if(NULL != pSource->ahCallback[i])
                BINT_DestroyCallback(pSource->ahCallback[i]);
        }
        for(i = 0; i < pSource->ulSlotUsed; i++)
        {
            if(NULL != pSource->ahSlot[i])
                BRDC_Slot_Destroy(pSource->ahSlot[i]);
        }
        for(i = 0; i < BVDC_P_MAX_MULTI_SLAVE_RUL_BUFFER_COUNT; i++)
        {
            if(NULL != pSource->ahListSlave[i])
                BRDC_List_Destroy(pSource->ahListSlave[i]);
        }
        for(i = 0; i < (pSource->ulSlotUsed * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT); i++)
        {
            if(NULL != pSource->ahList[i])
                BRDC_List_Destroy(pSource->ahList[i]);
        }

        BDBG_OBJECT_DESTROY(pSource, BVDC_SRC);
        BKNI_Free((void*)pSource);
    }

    return BERR_TRACE(eStatus);
}

/***************************************************************************
* {private}
*
*/
void BVDC_P_Source_Destroy
    ( BVDC_Source_Handle               hSource )
{
    uint32_t i;

    BDBG_ENTER(BVDC_P_Source_Destroy);

    if(!hSource)
    {
        BDBG_LEAVE(BVDC_P_Source_Destroy);
        return;
    }

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* At this point application should have disable all the
    * callbacks &slots */

    /* [7] Removed this source from hVdc */
    hSource->hVdc->ahSource[hSource->eId] = NULL;

    /* [6] Destroy event */
    BKNI_DestroyEvent(hSource->hAppliedDoneEvent);

    /* [5] Destroy a specific source handle. */
#if (BVDC_P_SUPPORT_NEW_656_IN_VER)
    if(hSource->h656In)
    {
        BVDC_P_656In_Destroy(hSource->h656In);
    }
#endif
    if(hSource->hHdDvi)
    {
        BVDC_P_HdDvi_Destroy(hSource->hHdDvi);
    }
    if(hSource->hMpegFeeder)
    {
#if BVDC_P_SUPPORT_STG
        BDBG_MODULE_MSG(BVDC_REPEATPOLARITY, ("free src[%d] REPEAT_POLARITY scratch register %#x ",
                    hSource->eId, hSource->ulScratchPolReg));
        BRDC_FreeScratchReg(hSource->hVdc->hRdc, hSource->ulScratchPolReg);
#endif
#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
        if(hSource->ulStcFlag != BRDC_MAX_STC_FLAG_COUNT)
        {
            BKNI_EnterCriticalSection();
            BRDC_ReleaseStcFlag_isr(hSource->hVdc->hRdc, hSource->ulStcFlag);
            hSource->ulStcFlag = BRDC_MAX_STC_FLAG_COUNT;
            BKNI_LeaveCriticalSection();
        }
#endif
        BVDC_P_Feeder_Destroy(hSource->hMpegFeeder);
    }
    if(hSource->hGfxFeeder)
    {
        BVDC_P_GfxFeeder_Destroy(hSource->hGfxFeeder);
    }

    if(hSource->hVfdFeeder)
    {
        BVDC_P_Feeder_Destroy(hSource->hVfdFeeder);
    }

    /* [4] Trigger detection callback. */
    if(hSource->hTrigger0Cb)
    {
        BINT_DestroyCallback(hSource->hTrigger0Cb);
    }
    if(hSource->hTrigger1Cb)
    {
        BINT_DestroyCallback(hSource->hTrigger1Cb);
    }

    /* [3] Its callbacks. */
    if(BVDC_P_SRC_IS_ITU656(hSource->eId) ||
       BVDC_P_SRC_IS_HDDVI(hSource->eId) ||
       BVDC_P_SRC_IS_MPEG(hSource->eId))
    {
        for(i = 0; i < hSource->ulSlotUsed; i++)
        {
            if (NULL!=hSource->ahCallback[i])
                BINT_DestroyCallback(hSource->ahCallback[i]);
        }
    }

    /* [2] Destroy RDC List & Slot */
    if(BVDC_P_SRC_IS_VIDEO(hSource->eId) && !BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        if(!hSource->hVdc->stSettings.bDisableMosaic)
        {
            /* MosaicMode: create slave slot and list for mpeg source; */
            if(BVDC_P_SRC_IS_MPEG(hSource->eId))
            {
                if(NULL != hSource->hSlotSlave)
                    BRDC_Slot_Destroy(hSource->hSlotSlave);
                for(i = 0; i < BVDC_P_MAX_MULTI_SLAVE_RUL_BUFFER_COUNT; i++)
                {
                    if(NULL != hSource->ahListSlave[i])
                        BRDC_List_Destroy(hSource->ahListSlave[i]);
                }
            }
        }

        for(i = 0; i < hSource->ulSlotUsed; i++)
        {
            BRDC_Slot_Destroy(hSource->ahSlot[i]);
        }

        for(i = 0; i < (hSource->ulSlotUsed * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT); i++)
        {
            BRDC_List_Destroy(hSource->ahList[i]);
        }
    }

    BDBG_OBJECT_DESTROY(hSource, BVDC_SRC);
    /* [1] Free the context. */
    BKNI_Free((void*)hSource);

    BDBG_LEAVE(BVDC_P_Source_Destroy);
    return;
}

/***************************************************************************
* {private}
*
*/
BERR_Code BVDC_P_Source_Init
    ( BVDC_Source_Handle               hSource,
      const BVDC_Source_CreateSettings *pDefSettings )
{
    uint32_t i;
    const BFMT_VideoInfo *pDefFmt;
    BVDC_P_Source_Info *pNewInfo;
    BVDC_P_Source_Info *pCurInfo;
    BVDC_P_Source_IsrInfo *pIsrInfo;
    bool bGfxSrc = false;
    bool bMtgSrc = false;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Source_Init);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* Which heap to use? */
    hSource->hHeap = ((pDefSettings) && (pDefSettings->hHeap))
        ? pDefSettings->hHeap : hSource->hVdc->hBufferHeap;
    bGfxSrc = (pDefSettings)? pDefSettings->bGfxSrc : false;

    /* New/Cur/Isr Info */
    pNewInfo = &hSource->stNewInfo;
    pCurInfo = &hSource->stCurInfo;
    pIsrInfo = &hSource->stIsrInfo;

    /* Zero out the previous field. */
    BKNI_Memset((void*)&(hSource->stPrevMvdField), 0x0, sizeof(BAVC_MVD_Field));
    BKNI_Memset((void*)&(hSource->stNewXvdField), 0x0, sizeof(BAVC_VDC_HdDvi_Picture));
    BKNI_Memset((void*)&(hSource->stScanOut), 0x0, sizeof(BVDC_P_Rect));
#if BVDC_P_SUPPORT_MOSAIC_MODE
    for(i = 0; i < BAVC_MOSAIC_MAX; i++)
    {
        BKNI_Memset((void*)&(hSource->stNewPic[i]), 0x0, sizeof(BAVC_MVD_Field));
        hSource->stNewPic[i].bMute = true;
        hSource->stNewPic[i].ulChannelId = i;
        hSource->aulMosaicZOrderIndex[i] = i;
        hSource->aulChannelId[i] = i;
        hSource->stNewPic[i].ulSourceHorizontalSize = BVDC_P_SRC_INPUT_H_MIN;
        hSource->stNewPic[i].ulSourceVerticalSize = BVDC_P_SRC_INPUT_V_MIN;
        hSource->stNewPic[i].ulDisplayHorizontalSize = BVDC_P_SRC_INPUT_H_MIN;
        hSource->stNewPic[i].ulDisplayVerticalSize = BVDC_P_SRC_INPUT_V_MIN;
    }
#else
    BKNI_Memset((void*)&(hSource->stNewPic), 0x0, sizeof(BAVC_MVD_Field));
#endif

    BDBG_ASSERT(sizeof(pNewInfo->stDirty.stBits) <= sizeof(pNewInfo->stDirty.aulInts));

    /* Default format */
    if(BBOX_VDC_DISREGARD != hSource->hVdc->stBoxConfig.stVdc.astDisplay[0].eMaxVideoFmt)
    {
        pDefFmt = BFMT_GetVideoFormatInfoPtr(hSource->hVdc->stBoxConfig.stVdc.astDisplay[0].eMaxVideoFmt);
    }
    else
    {
        pDefFmt = BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_eNTSC);
    }

    /* Initialize states can be changed by runtime. */
    hSource->bInitial             = true;
    hSource->bCaptureCrc          = false;
    hSource->bPictureChanged      = true;
    hSource->bRasterChanged       = true;
    hSource->bPrevFieldSwap       = false;
    hSource->bDeferSrcPendingCb   = false;
    hSource->ulTransferLock       = 0;
    hSource->ulConnectedWindow    = 0;
    hSource->hCmpToBeLocked       = NULL;
    hSource->hSyncLockCompositor  = NULL;
    hSource->ulSrcTrigger         = BVDC_P_TRIGGER_LOST_THRESHOLD;
    hSource->ulVecTrigger         = BVDC_P_TRIGGER_LOST_THRESHOLD;
    hSource->eNextFieldId         = BAVC_Polarity_eTopField;
    hSource->eNextFieldIntP       = BAVC_Polarity_eTopField;
    hSource->eState               = BVDC_P_State_eInactive;
    hSource->eTrigCtrl            = BVDC_P_TriggerCtrl_eSource;
    hSource->eMatrixCoefficients  = BAVC_MatrixCoefficients_eItu_R_BT_709;
    hSource->bNewUserModeAspRatio = false;
    hSource->ulVertFreq           = pDefFmt->ulVertFreq;
    hSource->ulStreamVertFreq     = pDefFmt->ulVertFreq;
    hSource->bStartFeed           = true; /* always true for MPEG, reset every vsync for other src */
    hSource->ulSampleFactor       = 0;    /* (1+0)x time */
    hSource->bPsfScanout          = false;/* no PsF scanout */
    hSource->ulDropFramesCnt      = 0;    /* no PsF drop */
    hSource->ulDispVsyncFreq      = pDefFmt->ulVertFreq;
    hSource->bSrcIs444            = false;
    hSource->bSrcInterlaced       = true;
    hSource->bCompression         = false;
    hSource->bWait4ReconfigVnet   = false;
    hSource->bEnablePsfBySize     = false;
    hSource->stExtVideoFmtInfo.eVideoFmt = BFMT_VideoFmt_eNTSC;
    hSource->stExtVideoFmtInfo.pchFormatStr = &s_chExtVideoFmtName[0];
    hSource->ulMosaicFirstUnmuteRectIndex = 0;
    hSource->bMosaicFirstUnmuteRectIndexSet = true;
#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
    hSource->ulStcFlagTrigSel     = -1;
#endif

    /* Default MfdRate to BoxMode's display rate if it's not connected to any window */
    hSource->ulDefMfdVertRefRate  = pDefFmt->ulVertFreq;
    hSource->eDefMfdVertRateCode  = BVDC_P_Source_RefreshRateCode_FromRefreshRate_isrsafe(hSource->ulDefMfdVertRefRate);

    /* Reset done events */
    BKNI_ResetEvent(hSource->hAppliedDoneEvent);

    /* Clear out user's states. */
    BKNI_Memset((void*)pNewInfo, 0x0, sizeof(BVDC_P_Source_Info));
    BKNI_Memset((void*)pCurInfo, 0x0, sizeof(BVDC_P_Source_Info));
    BKNI_Memset((void*)pIsrInfo, 0x0, sizeof(BVDC_P_Source_IsrInfo));

    /* Initialize all common fields */
    pNewInfo->pFmtInfo          = pDefFmt;
    pNewInfo->pVdcFmt           = BVDC_P_GetFormatInfo_isrsafe(pDefFmt->eVideoFmt);
    pNewInfo->eAspectRatio      = pNewInfo->pFmtInfo->eAspectRatio;
    pNewInfo->ulMuteColorYCrCb  = BVDC_P_YCRCB_BLACK;
    pNewInfo->eMuteMode         = BVDC_MuteMode_eDisable;
    pNewInfo->pfPicCallbackFunc = NULL;
    pNewInfo->bAutoFmtDetection = false;
    pNewInfo->bAutoDetect       = false;
    pNewInfo->ulNumFormats      = 0;
    pNewInfo->bErrorLastSetting = false;
    pNewInfo->ulInputPort       = BVDC_HdDviInput_Hdr;

    /* hddvi */
    pNewInfo->stHdDviSetting.stFmtTolerence.ulWidth  = BVDC_P_HDDVI_FORMAT_TOLER_WIDTH;
    pNewInfo->stHdDviSetting.stFmtTolerence.ulHeight = BVDC_P_HDDVI_FORMAT_TOLER_HEIGHT;

    /* dnr */
    pNewInfo->bDnr                     = false;
    pNewInfo->stDnrSettings.eBnrMode   = BVDC_FilterMode_eDisable;
    pNewInfo->stDnrSettings.eMnrMode   = BVDC_FilterMode_eDisable;
    pNewInfo->stDnrSettings.eDcrMode   = BVDC_FilterMode_eDisable;
    pNewInfo->stDnrSettings.iBnrLevel  = 0;
    pNewInfo->stDnrSettings.iMnrLevel  = 0;
    pNewInfo->stDnrSettings.iDcrLevel  = 0;
    pNewInfo->stDnrSettings.ulQp       = 0;
    pNewInfo->stDnrSettings.pvUserInfo = NULL;

    /* Demo mode */
    pNewInfo->stSplitScreenSetting.eDnr = BVDC_SplitScreenMode_eDisable;

    /* Intialize format list */
    for(i = 0; i < BFMT_VideoFmt_eMaxCount; i++)
    {
        pNewInfo->aeFormats[i] = BFMT_VideoFmt_eMaxCount;
        pCurInfo->aeFormats[i] = BFMT_VideoFmt_eMaxCount;
    }

    /* Callback setting (from user) */
    pNewInfo->stCallbackSettings.stMask.bFmtInfo = BVDC_P_DIRTY;
    pNewInfo->stCallbackSettings.stMask.bActive = BVDC_P_DIRTY;
    pNewInfo->stCallbackSettings.stMask.bCrcValue = BVDC_P_CLEAN;
    pNewInfo->stCallbackSettings.stMask.bFrameRate = BVDC_P_DIRTY;
    pNewInfo->stCallbackSettings.stMask.bSrcPending = BVDC_P_DIRTY;

    /* When there no interrupt to update we need to at least has
    * the fixed color set to a valid color. */
    pCurInfo->ulMuteColorYCrCb = BVDC_P_YCRCB_BLACK;
    pCurInfo->pFmtInfo         = BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_eNTSC);
    pCurInfo->pVdcFmt          = BVDC_P_GetFormatInfo_isrsafe(BFMT_VideoFmt_eNTSC);
    pCurInfo->eAspectRatio     = pCurInfo->pFmtInfo->eAspectRatio;
    pCurInfo->ulInputPort      = BVDC_HdDviInput_Hdr;

    /* Initialize user CSC settings */
    pNewInfo->bUserCsc = false;
    pNewInfo->ulUserShift = 16;
    for(i = 0; i < BVDC_CSC_COEFF_COUNT; i++)
    {
        pNewInfo->pl32_Matrix[i] = 0;
    }

    /* Initial fix color */
    pNewInfo->bFixColorEnable = false;
    for(i = 0; i < 3; i++)
    {
        pNewInfo->aulFixColorYCrCb[i] = BVDC_P_YCRCB_BLACK;
    }

    /* Initialized window handle*/
    for(i=0; i<BVDC_P_MAX_WINDOW_COUNT;i++)
        hSource->ahWindow[i] = 0;

    if(BVDC_P_SRC_IS_VIDEO(hSource->eId) && !BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        /* reinitialized ruls. */
        for(i = 0; i < (hSource->ulSlotUsed * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT); i++)
        {
            BRDC_List_SetNumEntries_isr(hSource->ahList[i], 0);
            BVDC_P_BuildNoOpsRul_isr(hSource->ahList[i]);
        }

        /* Assign fresh new no-op list. */
        for(i = 0; i < hSource->ulSlotUsed; i++)
        {
            /* Initialized rul indexing. */
            hSource->aulRulIdx[i] = 0;
        }
        BRDC_Slots_SetList_isr(hSource->ahSlot, hSource->ahList[0], hSource->ulSlotUsed);
    }

    /* Initialize specific source. */
#if (BVDC_P_SUPPORT_NEW_656_IN_VER)
    if(hSource->h656In)
    {
        BVDC_P_656In_Init(hSource->h656In);
        pNewInfo->eCtInputType = BVDC_P_CtInput_eItu656;
    }
#endif
    if(hSource->hHdDvi)
    {
        BVDC_P_HdDvi_Init(hSource->hHdDvi);
        pNewInfo->eCtInputType = BVDC_P_CtInput_eHdDvi;
    }
    if(hSource->hMpegFeeder)
    {
        /* mpeg src might be MtgSrc or GfxSrc, we will not know before BVDC_P_Source_Init
         * now we create src RUL done execution callback */
        BINT_CallbackFunc MpegSrcCallBack = BVDC_P_Source_MfdGfxCallback_isr;
        uint32_t ulDebugReg = BVDC_P_MPEG_DEBUG_SCRATCH(hSource->eId);

        /* Initialize debug msg on demand */
        BREG_Write32_isr(hSource->hVdc->hRegister, ulDebugReg, 0);

#if BVDC_P_SUPPORT_MTG
        if ((pDefSettings == NULL) || (pDefSettings->eMtgMode == BVDC_Mode_eAuto))
        {
            BDBG_ASSERT(hSource->eId < BAVC_SourceId_eMax);
            bMtgSrc = hSource->hVdc->stBoxConfig.stVdc.astSource[hSource->eId].bMtgCapable && !bGfxSrc;
        }
        else
        {
            bMtgSrc = (pDefSettings->eMtgMode == BVDC_Mode_eOn)? true : false;
        }
        bMtgSrc &= hSource->bMtgIsPresent;
        MpegSrcCallBack = (bMtgSrc) ? BVDC_P_Source_MtgCallback_isr : BVDC_P_Source_MfdGfxCallback_isr;
        BDBG_ASSERT(!bMtgSrc || !bGfxSrc);

        hSource->eTimeBase = BAVC_Timebase_e0;
        hSource->hDspTimebaseLocked = NULL;
#endif
        hSource->bMtgSrc = bMtgSrc;
        hSource->bGfxSrc = bGfxSrc;
        if (bMtgSrc || bGfxSrc)
        {
            /* RUL done execution callbacks. */
            for(i = 0; i < hSource->ulSlotUsed; i++)
            {
                if (NULL!=hSource->ahCallback[i])
                    BINT_DestroyCallback(hSource->ahCallback[i]);
                eStatus = BINT_CreateCallback(&hSource->ahCallback[i], hSource->hVdc->hInterrupt,
                    BRDC_Slot_GetIntId(hSource->ahSlot[i]),
                    MpegSrcCallBack,
                    (void*)hSource, i);
                if(BERR_SUCCESS != eStatus)
                {
                    BDBG_ERR(("Failed to create src RUL done callback for GfxSrc or MtgSrc %d", hSource->eId));
                }
            }
            /* MTG source triggers STC flag for MFD */
#if BVDC_P_SUPPORT_RDC_STC_FLAG
            if(bMtgSrc && hSource->ulStcFlag != BRDC_MAX_STC_FLAG_COUNT)
            {
                BKNI_EnterCriticalSection();
                hSource->ulStcFlagTrigSel = BRDC_Trigger_eMfd0Mtg0 + hSource->eId*2;
                BRDC_ConfigureStcFlag_isr(hSource->hVdc->hRdc, hSource->ulStcFlag, BRDC_Trigger_eMfd0Mtg0 + hSource->eId*2);
                BKNI_LeaveCriticalSection();
                BDBG_MSG(("MFD%u STC flag%u selects trigger from MTG%u", hSource->eId, hSource->ulStcFlag, hSource->eId));
            }
#endif
        }
        BVDC_P_Feeder_Init(hSource->hMpegFeeder, NULL, bGfxSrc, bMtgSrc);
        pNewInfo->eCtInputType = BVDC_P_CtInput_eMpeg;
    }
    if(hSource->hVfdFeeder)
    {
        BVDC_P_Feeder_Init(hSource->hVfdFeeder, NULL, bGfxSrc, false);
        pNewInfo->eCtInputType = BVDC_P_CtInput_eMpeg;
    }
    if(hSource->hGfxFeeder)
    {
        eStatus = BVDC_P_GfxFeeder_Init(hSource->hGfxFeeder, pDefSettings);
        if(eStatus != BERR_SUCCESS)
            return BERR_TRACE(eStatus);
        pNewInfo->eCtInputType = BVDC_P_CtInput_eUnknown;
    }

    BDBG_LEAVE(BVDC_P_Source_Init);
    return eStatus;
}


/***************************************************************************
* {private}
*
*/
static BERR_Code BVDC_P_Source_Validate
    ( BVDC_Source_Handle         hSource )
{
    BVDC_P_Source_Info   *pNewInfo;
    BVDC_P_Source_DirtyBits *pNewDirty;
    const BFMT_VideoInfo *pSrcFmtInfo;
    BERR_Code  eStatus = BERR_SUCCESS;

    /* Get new source info and make sure everything is good. */
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    pNewInfo = &hSource->stNewInfo;
    pNewDirty = &pNewInfo->stDirty;

    /* If error happens, this will flagged, and will accept new user settings. */
    pNewInfo->bErrorLastSetting = true;

    if(BVDC_P_SRC_IS_GFX(hSource->eId))
    {
        /* validate Gfx feeder, surface and window combination,
         * note: Gfx feeder can not be shared by more than one window */
        return BERR_TRACE(BVDC_P_GfxFeeder_ValidateChanges(
            hSource->hGfxFeeder, pNewInfo->pfPicCallbackFunc));
    }
    else if(BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        /* DVI specific validation. */
        BERR_Code eStatus;
        eStatus = BERR_TRACE(BVDC_P_HdDvi_ValidateChanges(hSource->hHdDvi));
        if(BERR_SUCCESS != eStatus)
        {
            return eStatus;
        }
    }
    else if(BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        /* VFD Gfx Src specific validation. */
        eStatus = BERR_TRACE(BVDC_P_Feeder_ValidateChanges(
            hSource->hVfdFeeder, pNewInfo->pfPicCallbackFunc));
        if(BERR_SUCCESS != eStatus)
        {
            return eStatus;
        }
    }
    else if(hSource->bGfxSrc)
    {
        /* MPEG Gfx Src specific validation. */
        eStatus = BERR_TRACE(BVDC_P_Feeder_ValidateChanges(
            hSource->hMpegFeeder, pNewInfo->pfPicCallbackFunc));
        if(BERR_SUCCESS != eStatus)
        {
            return eStatus;
        }
    }
    /*
    else if(BVDC_P_SRC_IS_ITU656(hSource->eId))
    {
    ITU656 specific validation.
    }
    */

    /* Refresh to see if any of the window toggle vbi/mad. */
    if(BVDC_P_SRC_IS_VIDEO(hSource->eId))
    {
        /* aspect ratio canvas clip validation */
        pSrcFmtInfo =
            ((hSource->stNewInfo.bAutoFmtDetection) &&
            (hSource->stCurInfo.bAutoFmtDetection))
            ? hSource->stCurInfo.pFmtInfo : hSource->stNewInfo.pFmtInfo;

        if((2 * (pNewInfo->stAspRatRectClip.ulLeft + pNewInfo->stAspRatRectClip.ulRight) > pSrcFmtInfo->ulWidth) ||
            (2 * (pNewInfo->stAspRatRectClip.ulTop + pNewInfo->stAspRatRectClip.ulBottom) > pSrcFmtInfo->ulHeight))
        {
            return BERR_TRACE(BVDC_ERR_INVALID_SRC_ASPECT_RATIO_RECT);
        }

        /* (1) Update connected windows that may have vbi-passthru */
        BKNI_EnterCriticalSection();
        BVDC_P_Source_RefreshWindow_isr(hSource, false);
        BKNI_LeaveCriticalSection();
    }

    /* If source pending is enabled, and user generated an artifictial
    * source change.  Put source into pending. */
    if(BVDC_ResumeMode_eManual == pNewInfo->eResumeMode)
    {
        pNewDirty->stBits.bInputFormat   |= pNewInfo->bForceSrcPending;
        pNewInfo->bForceSrcPending = false;
    }

    pNewInfo->bErrorLastSetting = false;
    return BERR_SUCCESS;
}


/***************************************************************************
* {private}
*
*/
BERR_Code BVDC_P_Source_ValidateChanges
    ( const BVDC_Source_Handle         ahSource[] )
{
    int i;
    BERR_Code eStatus;

    BDBG_ENTER(BVDC_P_Source_ValidateChanges);

    for(i = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(ahSource[i]) ||
            BVDC_P_STATE_IS_CREATE(ahSource[i]))
        {
            eStatus = BVDC_P_Source_Validate(ahSource[i]);
            if(BERR_SUCCESS != eStatus)
            {
                return BERR_TRACE(eStatus);
            }
        }
    }

    BDBG_LEAVE(BVDC_P_Source_ValidateChanges);
    return BERR_SUCCESS;
}


/***************************************************************************
* {private}
*
*/
void BVDC_P_Source_ApplyChanges_isr
    ( BVDC_Source_Handle               hSource )
{
    BVDC_P_Source_Info *pNewInfo;
    const BVDC_P_Source_Info *pCurInfo;
    BVDC_P_Source_DirtyBits *pNewDirty;
    BVDC_P_Source_DirtyBits *pActIsrSet;
    uint32_t i;

    BDBG_ENTER(BVDC_P_Source_ApplyChanges_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    pNewInfo = &hSource->stNewInfo;
    pCurInfo = &hSource->stCurInfo;
    pNewDirty = &pNewInfo->stDirty;

    /* Did any of the dirty got set. */
    if(BVDC_P_IS_DIRTY(pNewDirty))
    {
        hSource->bUserAppliedChanges = true;
    }

    /* State transition for source. */
    if(BVDC_P_STATE_IS_CREATE(hSource))
    {
        BDBG_MSG(("Source[%d] activated.", hSource->eId));
        hSource->eState = BVDC_P_State_eActive;
        hSource->bUserAppliedChanges = true;
        BVDC_P_SET_ALL_DIRTY(pNewDirty);

        /* this dirty bit should set by user to resume the video */
        pNewDirty->stBits.bResume = BVDC_P_CLEAN;

        /* Re-enable source with callbacks. */
        if(BVDC_P_SRC_IS_ITU656(hSource->eId) ||
           BVDC_P_SRC_IS_HDDVI (hSource->eId) ||
           hSource->bGfxSrc ||
           hSource->bMtgSrc)
        {
            for(i = 0; i < hSource->ulSlotUsed; i++)
            {
                BINT_ClearCallback_isr(hSource->ahCallback[i]);
                BINT_EnableCallback_isr(hSource->ahCallback[i]);
            }
        }
        if(BVDC_P_SRC_IS_MPEG(hSource->eId) && !hSource->bMtgSrc)
        {
            for(i = 0; i < hSource->ulSlotUsed; i++)
            {
                BRDC_Slot_ExecuteOnTrigger_isr(hSource->ahSlot[i],
                    BRDC_Trigger_UNKNOWN, true);
            }
        }
    }
    else if(BVDC_P_STATE_IS_DESTROY(hSource))
    {
        BDBG_MSG(("Source[%d] de-activated.", hSource->eId));
        hSource->eState = BVDC_P_State_eInactive;

        if(BVDC_P_SRC_IS_VIDEO(hSource->eId) && !BVDC_P_SRC_IS_VFD(hSource->eId))
        {
            BVDC_P_Source_CleanupSlots_isr(hSource);
            for(i = 0; i < hSource->ulSlotUsed; i++)
            {
                BRDC_Slot_ExecuteOnTrigger_isr(hSource->ahSlot[i],
                    BRDC_Trigger_UNKNOWN, true);
                BRDC_Slot_Disable_isr(hSource->ahSlot[i]);
            }
        }
        if(BVDC_P_SRC_IS_ITU656(hSource->eId) ||
           BVDC_P_SRC_IS_HDDVI(hSource->eId)  ||
           hSource->bGfxSrc ||
           hSource->bMtgSrc)
        {
            for(i = 0; i < hSource->ulSlotUsed; i++)
            {
                BINT_DisableCallback_isr(hSource->ahCallback[i]);
                BINT_ClearCallback_isr(hSource->ahCallback[i]);
            }

            if(hSource->hTrigger0Cb)
            {
                BINT_DisableCallback_isr(hSource->hTrigger0Cb);
                BINT_ClearCallback_isr(hSource->hTrigger0Cb);
            }
            if(hSource->hTrigger1Cb)
            {
                BINT_DisableCallback_isr(hSource->hTrigger1Cb);
                BINT_ClearCallback_isr(hSource->hTrigger1Cb);
            }

            BVDC_P_Source_DisableTriggers_isr(hSource);
        }

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK0
        if(BVDC_P_SRC_IS_HDDVI(hSource->eId))
        {
            if(hSource->ulHdmiPwrAcquire)
            {
                BDBG_MSG(("SRC[%d] Release pending BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK = 0x%08x",
                    hSource->eId, hSource->ulHdmiPwrId));
                hSource->ulHdmiPwrAcquire--;
                hSource->ulHdmiPwrRelease = 1;
            }
        }
#endif
    }

    if(BVDC_P_SRC_IS_GFX(hSource->eId))
    {
        if(hSource->bUserAppliedChanges)
        {
            BKNI_ResetEvent(hSource->hAppliedDoneEvent);
            BVDC_P_Source_UpdateSrcState_isr(hSource);
        }
        BVDC_P_GfxFeeder_ApplyChanges_isr(hSource->hGfxFeeder);
    }
    else
    {
        /* Copy activated _isr setting into newinfo to avoid overriding in
         * UpdateSrcState_isr, iff the setting does not change after ApplyChanges */
        pActIsrSet = &hSource->stIsrInfo.stActivated;
        if(pActIsrSet->stBits.bInputFormat && !pNewDirty->stBits.bInputFormat)
        {
            pNewInfo->pFmtInfo = pCurInfo->pFmtInfo;
        }
        if(pActIsrSet->stBits.bAspectRatio && !pNewDirty->stBits.bAspectRatio)
        {
            pNewInfo->eAspectRatio = pCurInfo->eAspectRatio;
        }
        if(pActIsrSet->stBits.bAspectRatioClip && !pNewDirty->stBits.bAspectRatioClip)
        {
            pNewInfo->stAspRatRectClip = pCurInfo->stAspRatRectClip;
        }
        BVDC_P_CLEAN_ALL_DIRTY(pActIsrSet);

        /* Auto-update aspect-ratio according to source video format change if
        * user did not override at the same time */
        if(pNewDirty->stBits.bInputFormat && !hSource->bNewUserModeAspRatio)
        {
            pNewInfo->eAspectRatio = pNewInfo->pFmtInfo->eAspectRatio;
            pNewDirty->stBits.bAspectRatio = BVDC_P_DIRTY;
            /* Clean out-dated _isr setting which has never be used */
            hSource->stIsrInfo.stDirty.stBits.bAspectRatio = BVDC_P_CLEAN;
        }

        /* Allow auto-updating src aspect ratio in next ApplyChanges,
        * note: bNewUserModeAspRatio is only used here, it is set by
        * Source_OverrideAspectRatio */
        hSource->bNewUserModeAspRatio = false;

        /* Check raster change from user */
        if(pCurInfo->pFmtInfo->bInterlaced != pNewInfo->pFmtInfo->bInterlaced)
        {
            hSource->bRasterChanged = true;
        }

        /* Isr will set event to notify apply done. */
        if(hSource->bUserAppliedChanges)
        {
            BKNI_ResetEvent(hSource->hAppliedDoneEvent);
        }

        if (BVDC_P_SRC_IS_VFD(hSource->eId))
        {
            BVDC_P_Feeder_ApplyChanges_isr(hSource->hVfdFeeder);
        }
        else if (hSource->bGfxSrc)
        {
            BVDC_P_Feeder_ApplyChanges_isr(hSource->hMpegFeeder);
        }

        /* If no window is connected just update the source cur/new. */
        if(!hSource->ulConnectedWindow)
        {
            BVDC_P_Source_UpdateSrcState_isr(hSource);
        }
    }

    /* First time bring up the source, kick start the initial trigger. */
    if(hSource->bInitial)
    {
        BVDC_P_Source_UpdateSrcState_isr(hSource);
        BVDC_P_Source_Bringup_isr(hSource);
        hSource->bInitial = false;
    }

    BDBG_LEAVE(BVDC_P_Source_ApplyChanges_isr);
    return;
}


/***************************************************************************
* {private}
*
*/
void BVDC_P_Source_AbortChanges
    ( BVDC_Source_Handle               hSource )
{
    BDBG_ENTER(BVDC_P_Source_AbortChanges);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* Cancel the setting user set to the new state. */
    hSource->stNewInfo = hSource->stCurInfo;

    /* Gfx source has its own abort changes function. */
    if(BVDC_P_SRC_IS_GFX(hSource->eId))
    {
        BVDC_P_GfxFeeder_AbortChanges(hSource->hGfxFeeder);
    }

    BDBG_LEAVE(BVDC_P_Source_AbortChanges);
    return;
}


/***************************************************************************
* {private}
*
*/
void BVDC_P_Source_UpdateSrcState_isr
( BVDC_Source_Handle               hSource )
{
    BVDC_P_Source_DirtyBits *pNewDirty, *pCurDirty;

    BDBG_ENTER(BVDC_P_Source_UpdateSrcState_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* Get source applied informations */
    if(hSource->bUserAppliedChanges)
    {
        pCurDirty = &hSource->stCurInfo.stDirty;
        pNewDirty = &hSource->stNewInfo.stDirty;

        /* Copying the new info to the current info.  Must be careful here
        * of not globble current dirty bits set by source, but rather ORed
        * them together. */
        BVDC_P_OR_ALL_DIRTY(pNewDirty, pCurDirty);
        hSource->stCurInfo = hSource->stNewInfo;

        /* Clear dirty bits since it's already OR'ed into current.  Notes
        * the it might not apply until next vysnc, so we're defering
        * setting the event until next vsync. */
        BVDC_P_CLEAN_ALL_DIRTY(pNewDirty);
        hSource->bUserAppliedChanges = false;

        /* Notify applychanges that source has been updated. */
        BKNI_SetEvent_isr(hSource->hAppliedDoneEvent);
    }

    /* get isr set info */
    if(BVDC_P_IS_DIRTY(&hSource->stIsrInfo.stDirty))
    {
        BVDC_P_Source_DirtyBits *pIsrDirty, *pCurDirty;

        pCurDirty = &hSource->stCurInfo.stDirty;
        pIsrDirty = &hSource->stIsrInfo.stDirty;

        /* Copying the isr info to the current info */
        if(pIsrDirty->stBits.bAspectRatioClip)
        {
            BVDC_P_ClipRect *pClip = &hSource->stIsrInfo.stAspRatRectClip;
            const BFMT_VideoInfo *pSrcFmtInfo = hSource->stCurInfo.pFmtInfo;

            if((2 * (pClip->ulLeft + pClip->ulRight) <= pSrcFmtInfo->ulWidth) &&
                (2 * (pClip->ulTop + pClip->ulBottom) <= pSrcFmtInfo->ulHeight))
            {
                hSource->stCurInfo.stAspRatRectClip = hSource->stIsrInfo.stAspRatRectClip;
                pCurDirty->stBits.bAspectRatioClip = BVDC_P_DIRTY;
            }
        }

        /* note: if aspectRatio is set in user mode and activated by AppyChanges after
        * an _isr setting, we will not see isr dirty here */
        if(pIsrDirty->stBits.bAspectRatio)
        {
            hSource->stCurInfo.eAspectRatio = hSource->stIsrInfo.eAspectRatio;
            pCurDirty->stBits.bAspectRatio = BVDC_P_DIRTY;
        }

        /* inform next ApplyChanges to copy activated isr setting into new info */
        BVDC_P_OR_ALL_DIRTY(&hSource->stIsrInfo.stActivated, pIsrDirty);

        /* Clear dirty bits since it's already OR'ed into current */
        BVDC_P_CLEAN_ALL_DIRTY(pIsrDirty);
    }

    /* Inform windows to re-adjust rectangles */
    hSource->bPictureChanged = (
        hSource->bPictureChanged ||
        hSource->stCurInfo.stDirty.stBits.bAspectRatio ||
        hSource->stCurInfo.stDirty.stBits.bAspectRatioClip);

    /* Reset trigger by vec, telling that we receiving trigger. */
    hSource->ulVecTrigger = BVDC_P_TRIGGER_LOST_THRESHOLD;

    BDBG_LEAVE(BVDC_P_Source_UpdateSrcState_isr);
    return;
}


/***************************************************************************
*
*/
void BVDC_P_Source_UpdateStatus_isr
    ( BVDC_Source_Handle               hSource )
{
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

#if (BVDC_P_SUPPORT_NEW_656_IN_VER)
    BVDC_P_656In_UpdateStatus_isr(hSource->h656In);
#endif

    return;
}

/***************************************************************************
*
*/
void BVDC_P_Source_CleanupSlots_isr
( BVDC_Source_Handle               hSource )
{
    uint32_t i;
    BRDC_List_Handle hList;

    BDBG_ENTER(BVDC_P_Source_CleanupSlots_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BVDC_P_SRC_NEXT_RUL(hSource, BAVC_Polarity_eBotField);
    hList = BVDC_P_SRC_GET_LIST(hSource, BAVC_Polarity_eBotField);

    /* turn off slots exec tracking to not to lose the critical RUL
    in the source slots before the cleanup; */
    for(i = 0; i < hSource->ulSlotUsed; i++) {
        BRDC_Slot_UpdateLastRulStatus_isr(hSource->ahSlot[i], hList, false);
    }

    BRDC_List_SetNumEntries_isr(hList, 0);
    BVDC_P_BuildNoOpsRul_isr(hList);
    BRDC_Slots_SetList_isr(hSource->ahSlot, hList, hSource->ulSlotUsed);

    /* reenable tracking */
    for(i = 0; i < hSource->ulSlotUsed; i++) {
        BRDC_Slot_UpdateLastRulStatus_isr(hSource->ahSlot[i], hList, true);
    }

    BDBG_LEAVE(BVDC_P_Source_CleanupSlots_isr);
    return;
}


/***************************************************************************
* {private}
*/
void BVDC_P_Source_FindLockWindow_isr
    ( BVDC_Source_Handle               hSource,
      bool                             bUpdate )
{
    uint32_t i;
    BVDC_Window_Handle hTmpWindow;
    BVDC_Compositor_Handle hTmpCompositor;
#if BVDC_P_SUPPORT_STG /* first NRT sync slaved display grabs the sync-lock right away */
    bool bSyncSlave = false;
#endif

    BDBG_ENTER(BVDC_P_Source_FindLockWindow_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_ASSERT(!hSource->hSyncLockCompositor);

    /* Trying to find a window/compositor that isn't lock */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if((!hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
        {
            continue;
        }

        hTmpWindow = hSource->ahWindow[i];
        BDBG_OBJECT_ASSERT(hTmpWindow, BVDC_WIN);
        hTmpCompositor = hTmpWindow->hCompositor;
        BDBG_OBJECT_ASSERT(hTmpCompositor, BVDC_CMP);
        /* if mpeg src transfers the trigger base, delay the triggering for clean transition;
        * Note, it makes sense for both sync-locked window and slipped PIP window; */
        if(BVDC_P_SRC_IS_MPEG(hSource->eId))
        {
            /* don't need to be active to acquire lock since new window might just be
            * destroyed and pending; */
            if(!hTmpCompositor->hSyncLockSrc)
            {
#if BVDC_P_SUPPORT_STG /* first NRT sync slaved display grabs the sync-lock right away */
                bSyncSlave = hTmpCompositor->hDisplay->stCurInfo.bStgNonRealTime && hTmpWindow->bSyncSlave;
#endif
                if(bUpdate
#if BVDC_P_SUPPORT_STG /* first NRT sync slaved display grabs the sync-lock right away */
                   || bSyncSlave
#endif
                )
                {
                    /* only lock on the to-be-locked */
                    if((hSource->hCmpToBeLocked == hTmpCompositor)
#if BVDC_P_SUPPORT_STG /* first NRT sync slaved display grabs the sync-lock */
                       || bSyncSlave
#endif
                    )
                    {
                        BDBG_MSG(("Making cmp[%u] window[%d] synclock(3)", hTmpCompositor->eId, hTmpWindow->eId));
                        hSource->hSyncLockCompositor   = hTmpWindow->hCompositor;
                        hTmpWindow->bSyncLockSrc       = true;
                        hTmpWindow->hBuffer->bSyncLock = true;
#if BVDC_P_SUPPORT_STG /* when NRT sync slaved display grabs the sync-lock, no need to transition from slip and realloc buffer */
                        if(!bSyncSlave)
#endif
                        {
                            hTmpWindow->stCurInfo.stDirty.stBits.bReallocBuffers = BVDC_P_DIRTY;
                            hTmpCompositor->ulSlip2Lock    = 1;/* count */
                        }
                        hTmpCompositor->hSyncLockWin   = hTmpWindow;
                        hTmpCompositor->hSyncLockSrc   = hSource;
                        hTmpCompositor->hSrcToBeLocked = NULL;
                        hSource->hCmpToBeLocked        = NULL;
#if BVDC_P_SUPPORT_STG
                        /* cw(close sync-locked master window) and transfer lock to this window -> deassert slave flag. */
                        hTmpWindow->bSyncSlave         = false;
                        hTmpCompositor->bSyncSlave     = false;
                        hSource->ulTransferLock        = 0;
#endif

                        /* Notify callback event that synclock window has changed */
                        if(hTmpWindow->stCurInfo.stCbSettings.stMask.bSyncLock)
                        {
                            hTmpWindow->stCbData.stMask.bSyncLock = BVDC_P_DIRTY;
                        }

                        /* recalculate cap buf count for new sync lock window */
                        BVDC_P_Window_DecideCapture_isr(
                            hTmpWindow, hSource, hTmpWindow->hCompositor);

                        /* Enable RDC priority for sync locked source on c0 */
                        if(hTmpCompositor->eId == BVDC_CompositorId_eCompositor0)
                        {
                            uint32_t   j;
                            BRDC_Slot_Settings   stSlotSettings;

                            stSlotSettings.bHighPriority = true;
                            for(j = 0; j < hSource->ulSlotUsed; j++)
                            {
                                if(BRDC_Slot_SetConfiguration_isr(hSource->ahSlot[j], &stSlotSettings)
                                   != BERR_SUCCESS)
                                {
                                    return;
                                }
                            }
                            if(hSource->hSlotSlave)
                            {
                                if(BRDC_Slot_SetConfiguration_isr(hSource->hSlotSlave, &stSlotSettings)
                                   != BERR_SUCCESS)
                                {
                                    return;
                                }
                            }
                        }
                    }
                    else
                    {
                        BDBG_MSG(("hTmpCompositor %d != the To-Be-Locked Cmp %d",
                            hTmpCompositor->eId, hSource->hCmpToBeLocked->eId));
                        /* continue the search */
                        continue;
                    }
                }
                else
                {
                    hSource->ulTransferLock        = 3;
                    hTmpCompositor->hSrcToBeLocked = hSource;
                    hSource->hCmpToBeLocked        = hTmpWindow->hCompositor;
                    BDBG_MSG(("SRC%d to transfer lock to CMP%d=%p", hSource->eId, hTmpCompositor->eId, (void *)hTmpCompositor));
                }
                /* find one to-be-locked window then break in case of triple displays */
                break;
            }
            else if(hTmpCompositor->hSyncLockSrc != hSource)
            {
                BDBG_MSG(("Transfer trigger source to cmp%d for PIP window[%d]",
                    hTmpCompositor->eId, hTmpWindow->eId));
                hSource->ulTransferLock            = 3;
                break;
            }
        }
    }

    BDBG_ENTER(BVDC_P_Source_FindLockWindow_isr);
    return;
}


/***************************************************************************
* {private}
*
* For Mpeg source we need to determine if it's neccessary for the
* source slot isr to build the RUL (Synclock).
*/
void BVDC_P_Source_ConnectWindow_isr
    ( BVDC_Source_Handle               hSource,
      BVDC_Window_Handle               hWindow )
{
    BDBG_ENTER(BVDC_P_Source_ConnectWindow_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    BDBG_ASSERT(!hSource->ahWindow[hWindow->eId]);

    hSource->ahWindow[hWindow->eId] = hWindow;

    /* Update the window count in compositor. */
    if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
    {
        /* (2) Update newly connected windows that may have vbi-passthru */
        hWindow->hCompositor->ulActiveVideoWindow++;
        if(hSource->ulConnectedWindow++ == 0)
        {
            hSource->bFieldSwap = false;
        }

        BVDC_P_Source_RefreshWindow_isr(hSource, true);
    }
    else
    {
        hWindow->hCompositor->ulActiveGfxWindow++;
    }

    /* Set dirty bits */
    hSource->stNewInfo.stDirty.stBits.bAddWin = BVDC_P_DIRTY;
    hSource->bUserAppliedChanges = true;

    /* If the window is mpeg and it's first connected to this source then
    * it will be a synclock window.  Provided that its compositor is not
    * already has a synclock window or in middle of transition to lock. */
    if((BVDC_P_SRC_IS_MPEG(hSource->eId)) &&
#if BVDC_P_SUPPORT_MTG
       (!hSource->bMtgSrc) &&
#endif
       (hSource->ulConnectedWindow == 1) &&
       (!hSource->hSyncLockCompositor) &&
       (!hWindow->hCompositor->hSyncLockSrc) &&
       (!hWindow->hCompositor->hSrcToBeLocked))
    {
        BDBG_MSG(("Making initial window[%d] synclock(1) cmp%u", hWindow->eId, hWindow->hCompositor->eId));
        hSource->hSyncLockCompositor       = hWindow->hCompositor;
        hWindow->bSyncLockSrc              = true;
        hWindow->hBuffer->bSyncLock        = true;
        hWindow->hCompositor->ulSlip2Lock  = 1;
        hWindow->hCompositor->hSyncLockWin = hWindow;
        hWindow->hCompositor->hSyncLockSrc = hSource;

        /* Notify callback event that synclock window has changed */
        if(hWindow->stCurInfo.stCbSettings.stMask.bSyncLock)
        {
            hWindow->stCbData.stMask.bSyncLock = BVDC_P_DIRTY;
        }

        /* Enable RDC priority for sync locked source on c0 */
        if(hWindow->hCompositor->eId == BVDC_CompositorId_eCompositor0)
        {
            uint32_t   j;
            BRDC_Slot_Settings   stSlotSettings;

            stSlotSettings.bHighPriority = true;
            for(j = 0; j < hSource->ulSlotUsed; j++)
            {
                if(BRDC_Slot_SetConfiguration_isr(hSource->ahSlot[j], &stSlotSettings)
                   != BERR_SUCCESS)
                {
                    return;
                }
            }
            if(hSource->hSlotSlave)
            {
                if(BRDC_Slot_SetConfiguration_isr(hSource->hSlotSlave, &stSlotSettings)
                   != BERR_SUCCESS)
                {
                    return;
                }
            }
        }
    }
#if BVDC_P_SUPPORT_STG
    else if((BVDC_P_SRC_IS_MPEG(hSource->eId)) &&
        BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg) &&
        hWindow->hCompositor->hDisplay->stCurInfo.bStgNonRealTime) {
        hWindow->bSyncSlave = true;
        BDBG_MSG(("WIN[%d] becomes sync slaved.", hWindow->eId));
    }
#endif

#if BVDC_P_SUPPORT_MTG
    /* If the window is mpeg and it's first connected to mtg source then
    * it will lock timebase of the display. */
    if((BVDC_P_SRC_IS_MPEG(hSource->eId)) &&
       (hSource->bMtgSrc) &&
       (hSource->ulConnectedWindow == 1))
    {
        BDBG_MSG(("Making initial window[%d] source %d lock(1) disp%u timebase %d ",
            hWindow->eId, hSource->eId, hWindow->hCompositor->hDisplay->eId,
            hWindow->hCompositor->hDisplay->stCurInfo.eTimeBase));
        hSource->hDspTimebaseLocked = hWindow->hCompositor->hDisplay;
        hSource->eTimeBase = hWindow->hCompositor->hDisplay->stCurInfo.eTimeBase;
    }
#endif

    BDBG_LEAVE(BVDC_P_Source_ConnectWindow_isr);
    return;
}

/***************************************************************************
* {private}
*
*/
void BVDC_P_Source_DisconnectWindow_isr
    ( BVDC_Source_Handle               hSource,
      BVDC_Window_Handle               hWindow )
{
    int i;
    bool bCmpWasLocked;

    BDBG_ENTER(BVDC_P_Source_DisconnectWindow_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    BDBG_ASSERT(hSource->ahWindow[hWindow->eId]);
    hSource->ahWindow[hWindow->eId] = NULL;
    bCmpWasLocked = (hWindow->hCompositor->hSyncLockSrc != NULL);

    if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
    {
        uint32_t ulCount;

        hWindow->hCompositor->ulActiveVideoWindow--;
        hSource->ulConnectedWindow--;
#if BVDC_P_SUPPORT_STG
        /* cw(close sync-slave window) -> deassert slave flag. */
        hWindow->bSyncSlave = false;
        hWindow->hCompositor->bSyncSlave = false;
#endif
        /* retest the source's vbi pass thru property since the disconnected
        * window could have control the vdec source's vbi pass through
        * settings. */
        /* (3) Update newly dis-connected windows that may have vbi-passthru */
        BVDC_P_Source_RefreshWindow_isr(hSource, true);

        /* Release memory for capture */
        ulCount = hWindow->hBuffer->ulActiveBufCnt;
        if(ulCount)
        {
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] free  %d buffers (%s)", hWindow->eId, ulCount,
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest)));

            if(hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate)
            {
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] free  %d buffers (%s) for right", hWindow->eId, ulCount,
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest)));

                BVDC_P_Buffer_ReleasePictureNodes_isr(hWindow->hBuffer,
                    hWindow->apHeapNode, hWindow->apHeapNode_R, ulCount, 0);
                BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap, hWindow->apHeapNode_R,
                    ulCount, false);
            }
            else
            {
                BVDC_P_Buffer_ReleasePictureNodes_isr(hWindow->hBuffer,
                    hWindow->apHeapNode, NULL, ulCount, 0);
            }

            BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap, hWindow->apHeapNode,
                ulCount, false);
            hWindow->ulBufCntAllocated -= ulCount;
        }
    }
    else
    {
        hWindow->hCompositor->ulActiveGfxWindow--;
    }

    /* If there is a Mpeg PIP that was depended on the Sync-lock to drive
    * the Mpeg feeder, we're now make that Mpeg feeder source a sync-lock
    * source which will taken over the responsibility of build the
    * vec/compositor as well. */
    if(hWindow->bSyncLockSrc)
    {
        /* Disable RDC priority for old sync locked source on c0 */
        if(hWindow->hCompositor->eId == BVDC_CompositorId_eCompositor0)
        {
            uint32_t   j;
            BRDC_Slot_Settings   stSlotSettings;

            stSlotSettings.bHighPriority = false;
            if(hSource->hSlotSlave)
            {
                if(BRDC_Slot_SetConfiguration_isr(hSource->hSlotSlave, &stSlotSettings)
                   != BERR_SUCCESS)
                {
                    return;
                }
            }
            for(j = 0; j < hSource->ulSlotUsed; j++)
            {
                if(BRDC_Slot_SetConfiguration_isr(
                   hWindow->hCompositor->hSyncLockSrc->ahSlot[j], &stSlotSettings)
                   != BERR_SUCCESS)
                {
                    return;
                }
            }
        }

        hSource->hSyncLockCompositor       = NULL;
        hWindow->bSyncLockSrc              = false;
        hWindow->hCompositor->hSyncLockWin = NULL;
        hWindow->hCompositor->hSyncLockSrc = NULL;

        /* Notify callback event that synclock window has changed */
        if(hWindow->stCurInfo.stCbSettings.stMask.bSyncLock)
        {
            hWindow->stCbData.stMask.bSyncLock = BVDC_P_DIRTY;
        }

        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            BVDC_Window_Handle hTmpWindow;
            BVDC_Source_Handle hTmpSource;

            /* SKIP: If it's just created or inactive no need to build ruls. */
            if((!hWindow->hCompositor->ahWindow[i]) ||
                BVDC_P_STATE_IS_CREATE(hWindow->hCompositor->ahWindow[i]) ||
                BVDC_P_STATE_IS_INACTIVE(hWindow->hCompositor->ahWindow[i]))
            {
                continue;
            }

            hTmpWindow = hWindow->hCompositor->ahWindow[i];
            BDBG_OBJECT_ASSERT(hTmpWindow, BVDC_WIN);
            hTmpSource = hTmpWindow->stCurInfo.hSource;
            BDBG_OBJECT_ASSERT(hTmpSource, BVDC_SRC);
            if((BVDC_P_SRC_IS_MPEG(hTmpSource->eId)) &&
#if BVDC_P_SUPPORT_MTG
               (!hTmpSource->bMtgSrc) &&
#endif
               (!hTmpSource->hSyncLockCompositor) &&
               (!hWindow->hCompositor->hSyncLockSrc) &&
               (hTmpWindow != hWindow))
            {
                BDBG_MSG(("Making cmp%u PIP window[%d] synclock(2)", hTmpWindow->hCompositor->eId, hTmpWindow->eId));
                hTmpSource->hSyncLockCompositor    = hTmpWindow->hCompositor;
                hTmpWindow->bSyncLockSrc           = true;
                hTmpWindow->hBuffer->bSyncLock     = true;
                hTmpWindow->stCurInfo.stDirty.stBits.bReallocBuffers = BVDC_P_DIRTY;
                hWindow->hCompositor->hSyncLockWin = hTmpWindow;
                hWindow->hCompositor->hSyncLockSrc = hTmpSource;

                /* Notify callback event that synclock window has changed */
                if(hTmpWindow->stCurInfo.stCbSettings.stMask.bSyncLock)
                {
                    hTmpWindow->stCbData.stMask.bSyncLock = BVDC_P_DIRTY;
                }

                BDBG_MSG(("Source[%d] Reconfigs vnet!", hSource->eId));
                BVDC_P_Window_SetReconfiguring_isr(hTmpWindow, false, true, false);

                break;
            }
        }
    }

    /* There are windows on the other compositor that uses this source, but
    * was a sync slip.  Which could run at a different clock.  In order to
    * has it trigger this source we'd need properly transfering the
    * responsibility.  Otherwise FEEDER_ERROR_INTERRUPT_STATUS would ocurr.
    *
    * This is destroying synclock window with existing syncslip windows.
    * Delay one vsync to let the feeder drain by the old compositor. */
    if((hSource->ulConnectedWindow) &&
#if BVDC_P_SUPPORT_MTG
       (!hSource->bMtgSrc) &&
#endif
       (!hSource->hSyncLockCompositor) && bCmpWasLocked)
    {
        BVDC_P_Source_FindLockWindow_isr(hSource, false);
#if BVDC_P_SUPPORT_STG /* first simul sync slaved display grabs the sync-lock */
        hWindow->hCompositor->hCmpToLock = hSource->hSyncLockCompositor;
#endif
    }

#if BVDC_P_SUPPORT_MTG
    if(BVDC_P_SRC_IS_MPEG(hSource->eId) && hSource->bMtgSrc)
    {
        BVDC_Window_Handle hTmpWindow;
        BVDC_Source_Handle hTmpSource;
        BVDC_Display_Handle hNextDspTimebaseLocked = NULL;

        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            /* SKIP: If it's just created or inactive no need to build ruls. */
            if((!hSource->ahWindow[i]) ||
                BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
                BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
            {
                continue;
            }

            hTmpWindow = hSource->ahWindow[i];
            BDBG_OBJECT_ASSERT(hTmpWindow, BVDC_WIN);
            hTmpSource = hTmpWindow->stCurInfo.hSource;
            BDBG_OBJECT_ASSERT(hTmpSource, BVDC_SRC);
            if((BVDC_P_SRC_IS_MPEG(hTmpSource->eId)) &&
               (hTmpWindow != hWindow))
            {
                if (hTmpWindow->hCompositor->hDisplay == hSource->hDspTimebaseLocked)
                {
                    BDBG_MSG(("Found window[%d] on same disp%u timebase %d for source %d",
                        hTmpWindow->eId, hTmpWindow->hCompositor->hDisplay->eId,
                        hTmpSource->eTimeBase, hTmpSource->eId));
                    break;
                }
                else
                {
                    hNextDspTimebaseLocked = hTmpWindow->hCompositor->hDisplay;
                }
            }
        }

        if(hNextDspTimebaseLocked)
        {
            BDBG_MSG(("Making source %d lock (2) disp%u timebase %d", hSource->eId, hNextDspTimebaseLocked->eId, hNextDspTimebaseLocked->stCurInfo.eTimeBase));
            hSource->hDspTimebaseLocked = hNextDspTimebaseLocked;
            hSource->eTimeBase = hNextDspTimebaseLocked->stCurInfo.eTimeBase;
        }
    }
#endif

    /* if mpeg source is disconnected with all windows, clean up source slots here,
    not in source callback in case immediate next applychange increment
    connected window count; */
    if(!hSource->ulConnectedWindow && BVDC_P_SRC_IS_MPEG(hSource->eId))
    {
        BVDC_P_Source_CleanupSlots_isr(hSource);
        BDBG_MSG(("Clean up mfd[%d] slots 'cuz last window disconnected",
            hSource->eId));
        hSource->ulTransferLock = 0;
    }
    BDBG_LEAVE(BVDC_P_Source_DisconnectWindow_isr);
    return;
}


/***************************************************************************
 * This function checks if we can save both bandwidth and memory by optimize
 * 3d and 2d conversion.
 * MPEG source:
 *     Converting at MFD when src is 3d and display is 2d. For all the other
 *     cases, we still convert at VFD.
 * HD_DVI source:
 *     Capture 2d when src is 3d and display is 2d. For all the other
 *     cases, we still convert at VFD.
 */
BERR_Code BVDC_P_Source_GetOutputOrientation_isr
    ( BVDC_Source_Handle               hSource,
      const BAVC_MVD_Field            *pFieldData,
      const BFMT_VideoInfo            *pFmtInfo,
      BFMT_Orientation                *peOutOrientation )
{
    uint32_t   i;
    bool       bMatch = false, bFirstWindow = true;
    BFMT_Orientation  eCmpOrientation = BFMT_Orientation_e2D;
    BFMT_Orientation  eMfdOrientation = BFMT_Orientation_e2D;
    BVDC_Window_Handle hWindow;

    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if(!hSource->ahWindow[i] ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
        {
            continue;
        }

        hWindow = hSource->ahWindow[i];
        BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

        /* Get display orientation */
        if(BFMT_IS_3D_MODE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
            eCmpOrientation = hWindow->hCompositor->stCurInfo.pFmtInfo->eOrientation;
        else
            eCmpOrientation = hWindow->hCompositor->stCurInfo.eOrientation;

        if(bFirstWindow)
        {
            bMatch = true;
            bFirstWindow = false;
            eMfdOrientation = eCmpOrientation;
        }
        else
        {
            if(eMfdOrientation != eCmpOrientation)
            {
                bMatch = false;
            }
        }

    }

    if(bMatch && (eCmpOrientation == BFMT_Orientation_e2D))
    {
        *peOutOrientation = eCmpOrientation;
    }
    else if(hSource->stCurInfo.bOrientationOverride)
    {
        *peOutOrientation = hSource->stCurInfo.eOrientation;
    }
    else
    {
        if(BVDC_P_SRC_IS_MPEG(hSource->eId) && pFieldData)
        {
            if((pFieldData->eOrientation == BFMT_Orientation_e3D_Left) ||
               (pFieldData->eOrientation == BFMT_Orientation_e3D_Right) ||
               (pFieldData->eOrientation == BFMT_Orientation_eLeftRight_Enhanced))
            {
                /* 2 buffer case: 2 separate buffers provided from XVD.
                 * MFD always feeds out as over/under. */
                *peOutOrientation = BFMT_Orientation_e3D_OverUnder;
            }
            else
            {
                /* All other cases: Keep orientation unchanged in MFD */
                *peOutOrientation = pFieldData->eOrientation;
            }
        }
        else
        {
            *peOutOrientation = pFmtInfo->eOrientation;
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
* {private}
*
* Return scan out rectangle for feeder, and update source content rect for all the
* connected scalers.
*
* pScanOutRect should always within source rectangle of the source, and in the
* format described below. Pan scan is calculated for macroblock, pixel and sub-pixel
* because of the frame buffer organization in MVD.
*
*   pScanOutRect->lLeft:
*      This is the horizontal pan scan for feeder, a positive number in the format of U32,
*      where bits 0-3 are pixel pan scan in pixel grid, bits 4-31 are macroblock pan
*      scan in macroblock grid.
*  pScanOutRect->lTop:
*      This is the vertical pan scan for feeder, a positive number in the format of U32,
*      where bits 0-3 are pixel pan scan in pixel grid, bits 4-31 are macroblock pan scan
*      in macroblock grid.
*  pScanOutRect->ulWidth:
*      This is the horizontal scan out size, a positive number in units of pixel.
*  pScanOutRect->ulHeight:
*      This is the vertical scan out size, a positive number in units of pixel.
*
* ----------------------------------------------------------------------
*  Window source clipping or pan/scan diagram:
*
*       +---------------------------------------+
*       |                SourceRect             |
*       |    +-----------------------------+    |
*       |    | +---------+    ScanOutRect  |    |
*       |    | |  win1   |                 |    |
*       |    | | Source  |                 |    |
*       |    | | Content |                 |    |
*       |    | |     +---|---------------+ |    |
*       |    | |     |   |   win2        | |    |
*       |    | +-----|---+  source       | |    |
*       |    |       |      content      | |    |
*       |    |       +-------------------+ |    |
*       |    +-----------------------------+    |
*       +---------------------------------------+
* Note:
*   - the feeder will handle the source clipping from SourceRect to ScanOutRect;
*   - the window scaler will handle source clipping from ScanOutRect to the
*     individual windows source content;
*   - since the Mpeg Feeder has 420 format chroma source data, it can do
*     clipping or pan/scan down to even line and even colomn only; the rest of
*     source clipping or pan/scan will be handled by scaler;
*   - we always pass non-negative pan/scan vector to scaler;
*/
void BVDC_P_Source_GetScanOutRect_isr
    ( BVDC_Source_Handle               hSource,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      BVDC_P_Rect                     *pScanOutRect )
{
    int i;
#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
    uint32_t ulBitsPerGroup = 0, ulPixelPerGroup = 0, ulMadrCnt = 0;
    BVDC_P_Mcvp_Handle hMcvp = NULL;
#endif

    BDBG_ENTER(BVDC_P_Source_GetScanOutRect_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    BDBG_ASSERT(pScanOutRect);
    BSTD_UNUSED(pXvdFieldData);
    /* itu656/analog scanout everything. */
    if(BVDC_P_SRC_IS_ITU656(hSource->eId))
    {
        const BFMT_VideoInfo *pFmtInfo = hSource->stCurInfo.pFmtInfo;
        pScanOutRect->lLeft    = 0;
        pScanOutRect->lLeft_R  = 0;
        pScanOutRect->lTop     = 0;
        pScanOutRect->ulWidth  = pFmtInfo->ulWidth;
        pScanOutRect->ulHeight = pFmtInfo->ulHeight;

        /* Adjust for oversample */
        pScanOutRect->ulWidth =
            BVDC_P_OVER_SAMPLE(hSource->ulSampleFactor, pScanOutRect->ulWidth);
    }
    else if(BVDC_P_SRC_IS_HDDVI(hSource->eId))
    {
        const BFMT_VideoInfo *pFmtInfo = hSource->stCurInfo.pFmtInfo;
        pScanOutRect->lLeft    = 0;
        pScanOutRect->lLeft_R  = 0;
        pScanOutRect->lTop     = 0;
        pScanOutRect->ulWidth  = pFmtInfo->ulDigitalWidth;

#if BFMT_LEGACY_3DTV_SUPPORT
        if(BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt)) /* 3D format */
        {
            if(hSource->stCurInfo.stHdDviSetting.bEnableDe &&
                hSource->stCurInfo.bHVStartOverride)
            {
                pScanOutRect->ulHeight = pFmtInfo->ulDigitalHeight - hSource->stCurInfo.ulVstart + pFmtInfo->ulTopActive - 1;
            }
            else
            {
                pScanOutRect->ulHeight = pFmtInfo->ulDigitalHeight;
            }
        }
        else
#endif
        {
            BFMT_Orientation  eSrcOututOrientation;
            pScanOutRect->ulHeight = pFmtInfo->ulDigitalHeight;

            BVDC_P_Source_GetOutputOrientation_isr(hSource, NULL,
                pFmtInfo, &eSrcOututOrientation);

            if (!BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt))
            {
                if(eSrcOututOrientation == BFMT_Orientation_e3D_LeftRight)
                {
                    pScanOutRect->ulWidth = pScanOutRect->ulWidth / 2;
                }
                else if(eSrcOututOrientation  == BFMT_Orientation_e3D_OverUnder)
                {
                    pScanOutRect->ulHeight = pScanOutRect->ulHeight / 2;
                }
            }
        }
    }
    else if (BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        BVDC_P_SurfaceInfo  *pCurSur;
        int32_t  lWinXMin, lWinYMin, lWinXMax, lWinYMax;
        int32_t  lWinXMin_R;
        int32_t  lXSize, lYSize;

        BVDC_P_Window_GetSourceContentRect_isr(
            hSource->hVfdFeeder->hWindow, NULL, NULL,
            &lWinXMin, &lWinYMin, &lWinXMin_R, &lWinXMax, &lWinYMax);

        /* Note: BVDC_P_16TH_PIXEL_SHIFT = 4 */
        lXSize = (lWinXMax - lWinXMin + 0xF) >> 4;
        lYSize = (lWinYMax - lWinYMin + 0xF) >> 4;

        pCurSur = &(hSource->hVfdFeeder->stGfxSurface.stCurSurInfo);
        BVDC_P_Window_AlignPreMadRect_isr(lWinXMin, lWinXMin_R, lXSize, pCurSur->ulWidth,
            4, 2, &(pScanOutRect->lLeft), &(pScanOutRect->lLeft_R),
            &(pScanOutRect->ulWidth));
        BVDC_P_Window_AlignPreMadRect_isr(lWinYMin, lWinYMin, lYSize, pCurSur->ulHeight,
            4, 2, &(pScanOutRect->lTop), NULL,
            &(pScanOutRect->ulHeight));

        pScanOutRect->ulWidth = (pScanOutRect->ulWidth + 1) & ~1;
        pScanOutRect->ulHeight = (pScanOutRect->ulHeight + 1) & ~1;
    }
    else /* MFD */
    {
        int32_t  lXMin, lYMin, lXMax, lYMax;
        int32_t  lWinXMin, lWinYMin, lWinXMax, lWinYMax;
        int32_t  lFullSrcWidth, lFullSrcHeight;
        int32_t  lXSize, lYSize;
        int32_t  ulAlignUnit;
        int32_t  lWinXMin_R, lXMin_R;

        /* lXMin is the left, lYMin is the top, lXMax is the right, and lYMax is the bettom
        * of the scan out rectangle. All are signed number in the format of S27.4, where
        * bits 0-3 are sub-pixel values in 1/16 pixel grid, bits 4-30 are pixel values in
        * pixel grid, bit 31 is the sign bit.*/
        /* Initialize the scan out rectangle to the full source size */
        BDBG_ASSERT(pMvdFieldData);
        lFullSrcWidth  = pMvdFieldData->ulSourceHorizontalSize;
        lFullSrcHeight = pMvdFieldData->ulSourceVerticalSize;
        lXMin = lFullSrcWidth  << BVDC_P_16TH_PIXEL_SHIFT;
        lYMin = lFullSrcHeight << BVDC_P_16TH_PIXEL_SHIFT;
        lXMin_R = lXMin;
        lXMax = 0;
        lYMax = 0;

        /* Find the union of user pan scan of all the windows this source connected to */
        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            /* SKIP: If it's just created or inactive no need to build ruls. */
            if(!hSource->ahWindow[i] ||
                BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
                BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
            {
                continue;
            }

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
            hMcvp = hSource->ahWindow[i]->stCurResource.hMcvp;

            if(hMcvp && hMcvp->hMcdi)
            {
                if(hMcvp->hMcdi->bMadr)
                {
                    ulMadrCnt++;
                    ulBitsPerGroup = hSource->ahWindow[i]->stMadCompression.ulBitsPerGroup;
                    ulPixelPerGroup = hSource->ahWindow[i]->stMadCompression.ulPixelPerGroup;
                    /* Assert for more than 1 MADR */
                    BDBG_ASSERT(ulMadrCnt <= 1);
                }
            }
#endif

            /* Get the source content rectangle for the window.
            * lWinXMin, lWinYMin, lWinXMax and lWinYMax in S27.4 format;
            * Note: the returned source content rect will be bound by source rect.  */
            BVDC_P_Window_GetSourceContentRect_isr(hSource->ahWindow[i],
                pMvdFieldData, NULL, &lWinXMin, &lWinYMin, &lWinXMin_R,
                &lWinXMax, &lWinYMax);

            /* Combine it with previous value */
            lXMin = BVDC_P_MIN(lXMin, lWinXMin);
            lYMin = BVDC_P_MIN(lYMin, lWinYMin);
            lXMin_R = BVDC_P_MIN(lXMin_R, lWinXMin_R);
            lXMax = BVDC_P_MAX(lXMax, lWinXMax);
            lYMax = BVDC_P_MAX(lYMax, lWinYMax);
        }

        /* Handle the case when no window connect to source. Make sure
        * lXMax is greater or equal to lXMin, and lYMax is greater or
        * equal to lYMin. */
        if( lXMin > lXMax )
        {
            lXMax = lXMin;
            lXMin = 0;
        }
        if( lYMin > lYMax )
        {
            lYMax = lYMin;
            lYMin = 0;
        }

        /* Make sure lXMin_R is not too big */
        if( lXMin_R > lXMax )
        {
            lXMin_R = 0;
        }

        /* Currently MVD/XVD always pass 4:2:0 format to VDC. Therefore scanOut (left,
        * right) are expanded to multiple of 2 pixel boundary, (top, bot) are expanded
        * to multiple of 2 line in the progressive src case, and multiple of 4 line in
        * the interlaced src case.
        *
        * We expand here, rather than shrink, to avoid losing of video info. Down stream
        * CAP and/or SCL will clip up to sub-pixel position.
        *
        * Note: BVDC_P_16TH_PIXEL_SHIFT = 4 */
        lXSize = (lXMax - lXMin + 0xF) >> 4;
        lYSize = (lYMax - lYMin + 0xF) >> 4;
        /* if only one win is using this src, zoom and pan will get the same
        * scan size, so MAD don't need hard-start */
#if (BVDC_P_MADR_HSIZE_WORKAROUND)
        BVDC_P_Window_AlignPreMadRect_isr(lXMin, lXMin_R, lXSize, lFullSrcWidth,
            4, 4, &(pScanOutRect->lLeft), &(pScanOutRect->lLeft_R),
            &(pScanOutRect->ulWidth));

#else
        BVDC_P_Window_AlignPreMadRect_isr(lXMin, lXMin_R, lXSize, lFullSrcWidth,
            4, 2, &(pScanOutRect->lLeft), &(pScanOutRect->lLeft_R),
            &(pScanOutRect->ulWidth));
#endif
        ulAlignUnit = (((BAVC_Polarity_eFrame != pMvdFieldData->eSourcePolarity) &&
            (BAVC_YCbCrType_e4_2_0 == pMvdFieldData->eYCbCrType))? 4 :
            ((BAVC_Polarity_eFrame != pMvdFieldData->eSourcePolarity) ||
            (BAVC_YCbCrType_e4_2_0 == pMvdFieldData->eYCbCrType))? 2 : 1);
        BVDC_P_Window_AlignPreMadRect_isr(lYMin, lYMin, lYSize, lFullSrcHeight,
            4, ulAlignUnit, &(pScanOutRect->lTop), NULL,
            &(pScanOutRect->ulHeight));

        /* AlignPreMadRect_isr will not expands to more than original width from XMD,
        * and the original width might be odd number for aspect ratio rounding purpose */
        pScanOutRect->ulWidth = (pScanOutRect->ulWidth + 1) & ~1;
        pScanOutRect->ulHeight = (pScanOutRect->ulHeight + 1) & ~1;

#if (BVDC_P_MVFD_ALIGNMENT_WORKAROUND)
        if((hSource->bGfxSrc && (pMvdFieldData->ePxlFmt == BPXL_INVALID)) ||
           BPXL_IS_YCbCr422_FORMAT(pMvdFieldData->ePxlFmt))
        {
            if(pScanOutRect->ulWidth % BVDC_P_SCB_BURST_SIZE == 2)
            {
                /* Crop */
                pScanOutRect->ulWidth -= 2;
            }
        }
#endif

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
        if(hMcvp && hMcvp->hMcdi)
        {
            if( hMcvp->hMcdi->bMadr)
            {
                BVDC_P_Window_PreMadAdjustWidth_isr(pScanOutRect->ulWidth,
                    (pMvdFieldData->eSourcePolarity == BAVC_Polarity_eFrame) ?
                    pScanOutRect->ulHeight : pScanOutRect->ulHeight /2,
                    BVDC_P_MADR_DCXS_COMPRESSION(ulBitsPerGroup), ulPixelPerGroup,
                    &pScanOutRect->ulWidth);
            }
        }
#endif

    }

    BDBG_LEAVE(BVDC_P_Source_GetScanOutRect_isr);
    return;
}


/***************************************************************************
* {private}
*
* Collect the window related information based on vnetmode for all the
* windows connected to the source. Need to make sure vnetmode.is
* determined.
*/
void BVDC_P_Source_GetWindowVnetmodeInfo_isr
    ( BVDC_Source_Handle               hSource )
{
    int   i;
    bool  bCompression = false;
    BVDC_Window_Handle hWindow;

    BDBG_ENTER(BVDC_P_Source_GetWindowVnetmodeInfo_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    hSource->bWait4ReconfigVnet = false;
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if((!hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_DESTROY(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_SHUTDOWNRUL(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_SHUTDOWNPENDING(hSource->ahWindow[i]))
        {
            continue;
        }

        hWindow = hSource->ahWindow[i];
        BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

        if( hWindow->stCurInfo.stDirty.stBits.bReConfigVnet ||
            hWindow->stCurInfo.stDirty.stBits.bReDetVnet)
        {
            /* Wait until reconfig vnet is done */
            hSource->bWait4ReconfigVnet = true;
            BDBG_MSG(("Src[%d] Wait for win%d recofig vnet", hSource->eId, hWindow->eId));
        }
        else
        {
            /* If Test Feature is enabled for AND or MAD or CAP */
            if((BVDC_P_VNET_USED_CAPTURE(hWindow->stVnetMode) &&
                hWindow->stCapCompression.bEnable) ||
                (BVDC_P_MVP_USED_ANR(hWindow->stMvpMode) &&
                hWindow->stMadCompression.bEnable) ||
                (BVDC_P_MVP_USED_MAD(hWindow->stMvpMode) &&
                hWindow->stMadCompression.bEnable))
            {
                bCompression = true;
            }
        }
    }

    if(!hSource->bWait4ReconfigVnet &&
        (hSource->bCompression != bCompression))
    {
        BDBG_MSG(("Src[%d] Switch bCompression %d->%d", hSource->eId,
            hSource->bCompression, bCompression));

        hSource->bCompression = bCompression;
        hSource->stCurInfo.stDirty.stBits.bWinChanges = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_P_Source_GetWindowVnetmodeInfo_isr);
    return;
}

/***************************************************************************
*
*/
void BVDC_P_Source_MpegGetStatus_isr
    ( const BVDC_Source_Handle        hSource,
 bool                            *pbVideoDetected )
{
    /* Sanity check and get context */
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(pbVideoDetected)
    {
        *pbVideoDetected = hSource->stPrevMvdField.bMute ? false : true;
    }

    return;
}


/***************************************************************************
*
*/
void BVDC_P_Source_BuildRul_isr
    ( const BVDC_Source_Handle         hSource,
      BVDC_P_ListInfo                 *pList )
{
    /* hush compiler */
    BSTD_UNUSED(pList);

    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    BDBG_ASSERT(BVDC_P_SRC_IS_ITU656(hSource->eId));
#if (BVDC_P_SUPPORT_NEW_656_IN_VER)
    BVDC_P_656In_BuildRul_isr(hSource->h656In, pList, hSource->eNextFieldIntP);
#endif

    return;
}

#if (BVDC_P_SUPPORT_HDDVI)
/***************************************************************************
 *
 */
void BVDC_P_Source_UpdateFrameRate_isr
    ( const BFMT_VideoInfo            *pFmtInfo,
      uint32_t                         ulClkPerVsync,
      uint32_t                         ulDelta,
      BAVC_FrameRateCode              *peFrameRateCode )
{
    uint32_t i;
    uint32_t ulCount;
    const BVDC_P_FrameRateEntry *pFrTbl;

    if(pFmtInfo->bInterlaced)
    {
        ulCount = BVDC_P_FRT_COUNT_INTERLACED;
        pFrTbl  = s_aIntFramerateTbl;
    }
    else
    {
        ulCount = BVDC_P_FRT_COUNT_PROGRESSIVE;
        pFrTbl  = s_aProFramerateTbl;
    }

    for(i = 0; i < ulCount; i++)
    {
        if(BVDC_P_EQ_DELTA(ulClkPerVsync, pFrTbl[i].ulClkPerVsync, ulDelta))
        {
            *peFrameRateCode = pFrTbl[i].eFramerate;
            break;
        }
    }

    return;
}
#endif

/***************************************************************************
*
*/
void BVDC_P_Source_CheckAndIssueCallback_isr
    ( BVDC_Source_Handle               hSource,
      BVDC_Source_CallbackMask        *pCbMask )
{
    uint32_t i;

    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if(!hSource->ahWindow[i] ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
        {
            continue;
        }

        BDBG_OBJECT_ASSERT(hSource->ahWindow[i], BVDC_WIN);

        if(hSource->ahWindow[i]->stCurInfo.stDirty.stBits.bShutdown)
        {
            break;
        }
    }

    /* any window incompletes shutdown, don't callback source pending; */
    if(i == BVDC_P_MAX_WINDOW_COUNT)
    {
        BDBG_MSG(("hSource[%d] issues source pending callback!", hSource->eId));
        pCbMask->bSrcPending       = true;

        /* clear the defer flag */
        hSource->bDeferSrcPendingCb = false;
    }

    return;
}

/***************************************************************************
*
*/
typedef struct {
    /* for XVD src, it is from stream; for HDMI/656, it is from HW register */
    BAVC_FrameRateCode eFrameRateCode;
    uint32_t ulRefreshRate;             /* unit in 1/BFMT_FREQ_FACTOR HZ */
    BAVC_FrameRateCode eMtgRefreshCode;    /* MTG rate code */
} BVDC_P_RefreshRateEntry;

#define BVDC_P_MAKE_VRR(e_frame_rate, vert_freq, e_mtg_rate_code)    \
{                                                                    \
    BAVC_FrameRateCode_##e_frame_rate,                               \
    (vert_freq * BFMT_FREQ_FACTOR),                                  \
    BAVC_FrameRateCode_##e_mtg_rate_code,                            \
}

/* this table must match the order of BAVC_FrameRateCode enum */
static const BVDC_P_RefreshRateEntry s_aSrcRefreshRate[] =
{
    BVDC_P_MAKE_VRR(eUnknown, 60.000, e60 ),
    BVDC_P_MAKE_VRR(e23_976, 23.976, e59_94 ),
    BVDC_P_MAKE_VRR(e24, 24.000, e60 ),
    BVDC_P_MAKE_VRR(e25, 25.000, e50 ),
    BVDC_P_MAKE_VRR(e29_97, 29.970, e59_94 ),
    BVDC_P_MAKE_VRR(e30, 30.000, e60 ),
    BVDC_P_MAKE_VRR(e50, 50.000, e50 ),
    BVDC_P_MAKE_VRR(e59_94, 59.940, e59_94 ),
    BVDC_P_MAKE_VRR(e60, 60.000, e60 ),
    BVDC_P_MAKE_VRR(e14_985, 14.985, e59_94 ),
    BVDC_P_MAKE_VRR(e7_493, 7.493, e59_94 ),
    BVDC_P_MAKE_VRR(e10, 10.000, e60 ),
    BVDC_P_MAKE_VRR(e15, 15.000, e60 ),
    BVDC_P_MAKE_VRR(e20, 20.000, e60 ),
    BVDC_P_MAKE_VRR(e12_5, 12.500, e50 ),
    BVDC_P_MAKE_VRR(e100, 100.000, e100 ),
    BVDC_P_MAKE_VRR(e119_88, 119.880, e119_88),
    BVDC_P_MAKE_VRR(e120, 120.000, e120 ),
    BVDC_P_MAKE_VRR(e19_98, 19.980, e59_94 ),
    BVDC_P_MAKE_VRR(e7_5, 7.500, e60 ),
    BVDC_P_MAKE_VRR(e12, 12.000, e60 ),
    BVDC_P_MAKE_VRR(e11_988, 11.988, e59_94 ),
    BVDC_P_MAKE_VRR(e9_99, 9.990, e59_94 ),
};

/***************************************************************************
* return refresh rate in 1/BFMT_FREQ_FACTOR Hz
*/
uint32_t BVDC_P_Source_RefreshRate_FromFrameRateCode_isrsafe
    ( BAVC_FrameRateCode               eFrameRateCode )
{
    /* this assert could detect adding to BAVC_FrameRateCode enum */
    BDBG_ASSERT(sizeof(s_aSrcRefreshRate)/sizeof(BVDC_P_RefreshRateEntry) == BAVC_FrameRateCode_eMax);
    if ((eFrameRateCode == BAVC_FrameRateCode_eUnknown || eFrameRateCode >= BAVC_FrameRateCode_eMax))
    {
        BDBG_MSG(("Unknown eFrameRateCode %d. Setting refresh rate to 60 Hz.", eFrameRateCode));
        return s_aSrcRefreshRate[BAVC_FrameRateCode_e60].ulRefreshRate;
    }

    return s_aSrcRefreshRate[eFrameRateCode].ulRefreshRate;
}

/***************************************************************************
* return the BAVC_FrameRateCode from the vertical refresh rate in units of
* 1/BFMT_FREQ_FACTOR Hz
*/
BAVC_FrameRateCode BVDC_P_Source_RefreshRateCode_FromRefreshRate_isrsafe
    ( uint32_t                         ulVertRefreshRate)
{
    uint32_t i;
    for(i=0; i<sizeof(s_aSrcRefreshRate)/sizeof(BVDC_P_RefreshRateEntry); i++)
    {
        if (ulVertRefreshRate == s_aSrcRefreshRate[i].ulRefreshRate)
        {
            return s_aSrcRefreshRate[i].eFrameRateCode;
        }
    }
    BDBG_WRN(("Unknown vertical refresh rate %d. Returning unknown frame rate code.", ulVertRefreshRate));

    return BAVC_FrameRateCode_eUnknown;
}

#if (BVDC_P_SUPPORT_MTG)
/***************************************************************************
* return a BAVC_FrameRateCode that MTG should run at, it will be
* BAVC_FrameRateCode_e50, BAVC_FrameRateCode_e60, or BAVC_FrameRateCode_e59_94
*/
BAVC_FrameRateCode BVDC_P_Source_MtgRefreshRate_FromFrameRateCode_isrsafe
    ( BVDC_Source_Handle               hSource,
      BAVC_FrameRateCode               eFrameRateCode )
{
    if ((eFrameRateCode == BAVC_FrameRateCode_eUnknown || eFrameRateCode >= BAVC_FrameRateCode_eMax))
    {
        return hSource->eDefMfdVertRateCode;
    }
    else
    {
        return s_aSrcRefreshRate[eFrameRateCode].eMtgRefreshCode;
    }
}
#endif /* BVDC_P_SUPPORT_MTG */


/***************************************************************************
*
*/
void BVDC_P_Source_AnalogDataReady_isr
    ( void                            *pvSourceHandle,
      int                              iParam2 )
{
    int i;
    BVDC_P_ListInfo stList;
    BVDC_Source_Handle hSource = (BVDC_Source_Handle)pvSourceHandle;
    BVDC_P_Source_Info *pCurInfo;
    BRDC_Slot_Handle hSlot, hOtherSlot;
    BRDC_List_Handle hList;

    BDBG_ENTER(BVDC_P_Source_AnalogDataReady_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* We should not even get the Gfx or mpeg source here. */
    BDBG_ASSERT(BVDC_P_SRC_IS_ITU656(hSource->eId));

    /* Analog/656 source is always sync-slip with a compositor/vec. */
    BDBG_ASSERT(!hSource->hSyncLockCompositor);

    /* Make sure the BKNI enter/leave critical section works. */
    BVDC_P_CHECK_CS_ENTER_VDC(hSource->hVdc);

    /* Update source user info */
    BVDC_P_Source_UpdateSrcState_isr(hSource);

    /* Update source info */
    BVDC_P_Source_UpdateStatus_isr(hSource);

    /* Get current settings */
    pCurInfo = &hSource->stCurInfo;

    /* for progressive src format, we always expect Top slot interrupt */
    hSource->eNextFieldIntP = iParam2;

    /* this is actually the current slot */
    hSource->eNextFieldId   = (pCurInfo->pFmtInfo->bInterlaced)
        ? BVDC_P_NEXT_POLARITY(iParam2) : BAVC_Polarity_eFrame;

    /* Get the approriate slot/list for building RUL. */
    BVDC_P_SRC_NEXT_RUL(hSource, hSource->eNextFieldIntP);
    hSlot = BVDC_P_SRC_GET_SLOT(hSource, hSource->eNextFieldIntP);
    hList = BVDC_P_SRC_GET_LIST(hSource, hSource->eNextFieldIntP);

    /* Update the status of last executed RUL. */
    BRDC_Slot_UpdateLastRulStatus_isr(hSlot, hList, true);

    /* Assert: list is not connected to any slot */
    BRDC_List_SetNumEntries_isr(hList, 0);

    /* Get current pointer to RUL and info. */
    BVDC_P_ReadListInfo_isr(&stList, hList);
    stList.bMasterList = true;

    /* update windows that connected to this source, including user info,
    * destroy related state and disconnecting from source */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if(!hSource->ahWindow[i] ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
        {
            continue;
        }
        BVDC_P_Window_AdjustRectangles_isr(hSource->ahWindow[i], NULL, NULL, 0);
    }

    /* Defer update to make sure all windows get a chance of reconfiguring */
#if (BVDC_P_CLEANUP_VNET)
    if(BVDC_MuteMode_eRepeat != pCurInfo->eMuteMode)
    {
        hSource->bPrevStartFeed = hSource->bStartFeed;
    }
#endif

    /* Get the source scan out rectangle. Combine the user pan-scan info
    * from all the window that uses this source;
    * Note: pMvdField, pXvdField = NULL for analog video source. */
    if(pCurInfo->stDirty.stBits.bRecAdjust)
    {
        BVDC_P_Source_GetScanOutRect_isr(hSource, NULL, NULL, &hSource->stScanOut);
    }

    /* For each window using this source do the following. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        /* SKIP: If it's just created or inactive no need to build ruls. */
        if(!hSource->ahWindow[i] ||
            BVDC_P_STATE_IS_CREATE(hSource->ahWindow[i]) ||
            BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[i]))
        {
            continue;
        }

        BVDC_P_Window_Writer_isr(hSource->ahWindow[i], NULL, NULL,
            hSource->eNextFieldId, &stList, 0);

        /* skip window RUL if shutdown state */
        if(BVDC_P_State_eShutDown != hSource->ahWindow[i]->stCurInfo.eWriterState)
        {
            BVDC_P_Window_BuildRul_isr(hSource->ahWindow[i], &stList, hSource->eNextFieldId,
                true,  /* writer*/
                false, /* reader */
                false  /* canvasCtrl */ );
        }

        /* Window needs to adjust CSC because pedestal may have changed. */
        hSource->ahWindow[i]->stCurInfo.stDirty.stBits.bCscAdjust |=
            pCurInfo->stDirty.stBits.bMiscCtrl;
    }

    if(pCurInfo->stDirty.stBits.bRecAdjust || hSource->bWait4ReconfigVnet)
    {
        /* Gather window information after vnetmode is determined */
        BVDC_P_Source_GetWindowVnetmodeInfo_isr(hSource);
    }

    /* Now build the Vdec/656 block. */
    BVDC_P_Source_BuildRul_isr(hSource, &stList);

    /* UPdate current pictures, the eSourcePolarity must update every field. */
    hSource->bPictureChanged = false;
    hSource->bRasterChanged = false;

    /* Update entries count */
    BVDC_P_WriteListInfo_isr(&stList, hList);

    /* This programs the slot after next. This is needed in conjunction with
    * the programming of the next slot above and for accommodating same field
    * polarity sources. Moreover, this is so to make the 4 buffer algorithm
    * work correctly. */
    hOtherSlot = BVDC_P_SRC_GET_SLOT(hSource,
        BVDC_P_NEXT_POLARITY(hSource->eNextFieldIntP));

    /* Always assign single RUL to two slots to avoid uncovered transition
    * from dual to one; */
    /* Note: to flush the cached RUL only once, call the Dual function
    instead of two individual slot functions; */
    BRDC_Slot_SetListDual_isr(hSlot, hOtherSlot, hList);
    BVDC_P_CHECK_CS_LEAVE_VDC(hSource->hVdc);
    BDBG_LEAVE(BVDC_P_Source_AnalogDataReady_isr);
    return;
}

/***************************************************************************
*
*/
void BVDC_P_Source_VfdGfxDataReady_isr
    ( BVDC_Source_Handle               hSource,
      BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList,
      int                              iParam2 )
{
    BVDC_P_Source_Info  *pCurInfo;
    BVDC_P_Feeder_Handle  hVfdFeeder;

    BDBG_ENTER(BVDC_P_Source_VfdGfxDataReady_isr);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* We should not even get the Gfx or mpeg source here. */
    BDBG_ASSERT(BVDC_P_SRC_IS_VFD(hSource->eId));

    /* Update source user info */
    BVDC_P_Source_UpdateSrcState_isr(hSource);

    /* Get current settings */
    pCurInfo = &hSource->stCurInfo;

    /* for progressive src format, we always expect Top slot interrupt */
    hSource->eNextFieldIntP = iParam2;

    /* this is actually the current slot */
    hSource->eNextFieldId = BVDC_P_NEXT_POLARITY(iParam2);

    /* If surface has been set by *SetSurface_isr, or src pic call back func is
     * installed, we activate it now */
    hVfdFeeder = hSource->hVfdFeeder;
    BVDC_P_Feeder_HandleIsrGfxSurface_isr(hVfdFeeder, pCurInfo, hSource->eNextFieldId);
    hSource->bPictureChanged = hVfdFeeder->stGfxSurface.stCurSurInfo.bChangeSurface;

    if (hVfdFeeder->stGfxSurface.stCurSurInfo.ullAddress)
    {
        /* ajust rectangles */
        BVDC_P_Window_AdjustRectangles_isr(hWindow, NULL, NULL, 0);

        if (hVfdFeeder->hWindow)
        {
            BVDC_P_Source_GetScanOutRect_isr(hSource, NULL, NULL, &hSource->stScanOut);
        }

        /* we only call BVDC_P_Window_Writer_isr to setup vnet and picture node
         * RUL will be built with reader in display interrupt _isr */
        if( !BVDC_P_STATE_IS_CREATE(hWindow) &&
            !BVDC_P_STATE_IS_INACTIVE(hWindow) )
        {
            BVDC_P_Window_Writer_isr(hWindow, NULL, NULL, hSource->eNextFieldId, pList, 0);
        }

        /* Update current pictures, the eSourcePolarity must update every field. */
        hSource->bPictureChanged = false;
        hSource->bRasterChanged = false;
        hVfdFeeder->stGfxSurface.stCurSurInfo.bChangeSurface = false;
    }
    else
    {
        BVDC_P_CLEAN_ALL_DIRTY(&hWindow->stCurInfo.stDirty);
    }

    BDBG_LEAVE(BVDC_P_Source_VfdGfxDataReady_isr);
    return;
}

/***************************************************************************
*
*/
void BVDC_P_Source_MfdGfxCallback_isr
    ( void                            *pvSourceHandle,
      int                              iField )
{
    BVDC_Source_Handle  hSource;
    BVDC_P_Source_Info  *pCurInfo;
    BVDC_P_Feeder_Handle  hMpegFeeder;
    BAVC_Polarity  eNextFieldId;

    hSource = (BVDC_Source_Handle) pvSourceHandle;
    pCurInfo = &hSource->stCurInfo;

    /* this is actually the current slot */
    eNextFieldId = BVDC_P_NEXT_POLARITY(iField);

    /* If surface has been set by *SetSurface_isr, or src pic call back func is
     * installed, we activate it now */
    hMpegFeeder = hSource->hMpegFeeder;
    BVDC_P_Feeder_HandleIsrGfxSurface_isr(hMpegFeeder, pCurInfo, eNextFieldId);
    hSource->bPictureChanged = hMpegFeeder->stGfxSurface.stCurSurInfo.bChangeSurface;

    BKNI_Memset((void*)&hSource->stNewPic[BAVC_MOSAIC_MAX-1], 0x0, sizeof(BAVC_MVD_Field));

    hSource->stNewPic[BAVC_MOSAIC_MAX-1].eInterruptPolarity =
        (NULL!=hSource->hSyncLockCompositor)? eNextFieldId : (BAVC_Polarity) iField;

    if (hMpegFeeder->stGfxSurface.stCurSurInfo.ullAddress)
    {
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].eSourcePolarity = BAVC_Polarity_eFrame;/* frame only */
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].bStreamProgressive = true;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].bFrameProgressive = true;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].eMpegType = BAVC_MpegType_eMpeg2;
        if(hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pSurface) {
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].eYCbCrType = BAVC_YCbCrType_e4_2_2;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulRowStride = hMpegFeeder->stGfxSurface.stCurSurInfo.ulPitch;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulLuminanceFrameBufferBlockOffset = hMpegFeeder->stGfxSurface.stCurSurInfo.ullAddress;

            /* Check if fields that are to be written to registers fit within their field sizes. */
            if(!BVDC_P_SRC_VALIDATE_FIELD(MFD_0_STRIDE, PACKED_LINE_STRIDE, hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulRowStride))
            {
                BDBG_ERR(("ulRowStride is invalid"));
                hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulRowStride = BVDC_P_SRC_FIELD_DATA_MASK(MFD_0_STRIDE,
                    PACKED_LINE_STRIDE, hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulRowStride);
            }
        } else {/* striped surface */
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].eBitDepth = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pstMfdPic->eBitDepth;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].eStripeWidth = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pstMfdPic->eStripeWidth;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulLumaRangeRemapping = 8;/* must set range remapping to 8! */
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulChromaRangeRemapping = 8;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].eYCbCrType = BAVC_YCbCrType_e4_2_0;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulLuminanceNMBY = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pstMfdPic->ulLuminanceNMBY;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulChrominanceNMBY = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pstMfdPic->ulChrominanceNMBY;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].hLuminanceFrameBufferBlock = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pstMfdPic->hLuminanceFrameBufferBlock;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].hChrominanceFrameBufferBlock = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pstMfdPic->hChrominanceFrameBufferBlock;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulLuminanceFrameBufferBlockOffset = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pstMfdPic->ulLuminanceFrameBufferBlockOffset;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulChrominanceFrameBufferBlockOffset = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.pstMfdPic->ulChrominanceFrameBufferBlockOffset;
        }
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].eChrominanceInterpolationMode = BAVC_InterpolationMode_eFrame;

        hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulSourceHorizontalSize  = hMpegFeeder->stGfxSurface.stCurSurInfo.ulWidth;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulSourceVerticalSize    = hMpegFeeder->stGfxSurface.stCurSurInfo.ulHeight;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulDisplayHorizontalSize = hMpegFeeder->stGfxSurface.stCurSurInfo.ulWidth;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulDisplayVerticalSize   = hMpegFeeder->stGfxSurface.stCurSurInfo.ulHeight;

        /* Jira SW7405-4239: in case surface has odd width/height */
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulSourceHorizontalSize = hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulSourceHorizontalSize & ~1;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulSourceVerticalSize   = hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulSourceVerticalSize & ~1;

        /* mark this MpegDataReady call as from BVDC_P_Source_MfdGfxCallback_isr */

        hSource->stNewPic[BAVC_MOSAIC_MAX-1].ePxlFmt = hMpegFeeder->stGfxSurface.stCurSurInfo.eInputPxlFmt;
        /* SWSTB-3417/SWSTB-3427: init default bitDepth due to feed hw capability and pxel fmt */
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].eBitDepth = hSource->stNewPic[BAVC_MOSAIC_MAX-1].eChromaBitDepth
            = (hSource->bIs10BitCore && (hSource->stNewPic[BAVC_MOSAIC_MAX-1].ePxlFmt == BPXL_eY10))?
            BAVC_VideoBitDepth_e10Bit:BAVC_VideoBitDepth_e8Bit;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].eOrientation = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.eInOrientation;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].ulOrigPTS = hMpegFeeder->stGfxSurface.stCurSurInfo.stAvcPic.ulOrigPTS;

        /* info for Psf and forceFrameCapture */
        if (pCurInfo->bForceFrameCapture || hSource->stNewInfo.bForceFrameCapture)
        {
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].bPictureRepeatFlag = !hMpegFeeder->stGfxSurface.stCurSurInfo.bSetSurface;
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].eFrameRateCode = BAVC_FrameRateCode_e30;
        }
        else
        {
            hSource->stNewPic[BAVC_MOSAIC_MAX-1].bPictureRepeatFlag = false;
        }
        hMpegFeeder->stGfxSurface.stCurSurInfo.bChangeSurface = false;
        hMpegFeeder->stGfxSurface.stCurSurInfo.bSetSurface = false;
    }

    else /* no valid surface yet */
    {
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].bMute = true;
        hSource->stNewPic[BAVC_MOSAIC_MAX-1].bIgnorePicture = true;
    }

    /* call BVDC_Source_MpegDataReady_isr: get the surface built into the RUL which will
     * be executed after current field/frame display is done
     * note: must keep calling even if hMpegFeeder->hWindow becomes NULL, otherwise window
     * shutdown will time out */
    BVDC_Source_MpegDataReady_isr( pvSourceHandle, iField, &hSource->stNewPic[BAVC_MOSAIC_MAX-1] );
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/* End of file */
