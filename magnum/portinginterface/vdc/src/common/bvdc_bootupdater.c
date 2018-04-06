/***************************************************************************
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
 *
 * Module Description:
 *
 ***************************************************************************/
#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */

#include "bvdc.h"                /* Video display */
#include "bvdc_priv.h"           /* VDC internal data structures */
#include "bvdc_common_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"  /* only for err msg as time out */


BDBG_MODULE(BVDC_FOR_BOOTUPDATER);
BDBG_OBJECT_ID(BVDC_FOR_BOOTUPDATER);

extern const uint32_t s_aulDacGrouping[BVDC_MAX_DACS];
extern const BVDC_OpenSettings s_stDefaultSettings;
extern const BVDC_P_Features s_VdcFeatures;
/***************************************************************************
 * BVDC_Open()
 *
 */
BERR_Code BVDC_Open
    ( BVDC_Handle                     *phVdc,
      BCHP_Handle                      hChip,
      BREG_Handle                      hRegister,
      BMMA_Heap_Handle                 hMemory,
      BINT_Handle                      hInterrupt,
      BRDC_Handle                      hRdc,
      BTMR_Handle                      hTmr,
      const BVDC_OpenSettings             *pDefSettings )
{
    BVDC_P_Context *pVdc = NULL;
    BERR_Code eStatus = BERR_SUCCESS;
    uint32_t i;

    /* The handle will be NULL if create fails. */
    *phVdc = NULL;


    /* (1) Alloc the main VDC context. */
    pVdc = (BVDC_P_Context*)(BKNI_Malloc(sizeof(BVDC_P_Context)));
    if(NULL == pVdc)
    {
        eStatus = BERR_OUT_OF_SYSTEM_MEMORY;
        goto BVDC_Open_Done;
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pVdc, 0x0, sizeof(BVDC_P_Context));
    BDBG_OBJECT_SET(pVdc, BVDC_VDC);

    /* Store the hChip, hRegister, hMemory, and hRdc for later use. */
    pVdc->hChip      = hChip;
    pVdc->hRegister  = hRegister;
    pVdc->hMemory    = hMemory;
    pVdc->hInterrupt = hInterrupt;
    pVdc->hRdc       = hRdc;
    pVdc->hTmr       = hTmr;

    /* Take in feature, this should be the centralize place to discover about
     * chip information and features. */
    pVdc->pFeatures = &s_VdcFeatures;

    /* Take in default settings. */
    if (pDefSettings)
    {
        pVdc->stSettings = *pDefSettings;
    }
    else
    {
        pVdc->stSettings = s_stDefaultSettings;
        pVdc->stSettings.eVideoFormat = BFMT_VideoFmt_e480p;
        BKNI_Memset((void*)&pVdc->stSettings.stHeapSettings, 0x0, sizeof(BVDC_Heap_Settings));
    }

    /* Do we need to swap the CMP/VEC. */
    pVdc->bSwapVec = (
        (pVdc->stSettings.bVecSwap) &&
        (pVdc->pFeatures->abAvailCmp[BVDC_CompositorId_eCompositor1]));

    /* (4) Create resource */
    BVDC_P_Resource_Create(&pVdc->hResource, pVdc);

    /* (8) Initialize all DACs to unused */
    for (i = 0; i < BVDC_P_MAX_DACS; i++ )
    {
        pVdc->aDacOutput[i] = BVDC_DacOutput_eUnused;
        pVdc->aulDacSyncSource[i] = i;
    }
    pVdc->aulDacGrouping = s_aulDacGrouping;
    /* Default Auto = Off */
    pVdc->bDacDetectionEnable = (pVdc->stSettings.eDacDetection == BVDC_Mode_eOn) ? true : false;

    /* Reset used Xcode GFD counter */
    pVdc->ulXcodeGfd = 0;

    /* All done. now return the new fresh context to user. */
    *phVdc = (BVDC_Handle)pVdc;

BVDC_Open_Done:

    return BERR_TRACE(eStatus);
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
    BVDC_P_SourceContext *pSource;
    BERR_Code eStatus = BERR_SUCCESS;

    BSTD_UNUSED(hResource);
    BDBG_ENTER(BVDC_P_Source_Create);
    BDBG_ASSERT(phSource);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* BDBG_SetModuleLevel("BVDC_SRC", BDBG_eMsg); */

    /* (1) Alloc the window context. */
    pSource = (BVDC_P_SourceContext*)
        (BKNI_Malloc(sizeof(BVDC_P_SourceContext)));

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pSource, 0x0, sizeof(BVDC_P_SourceContext));
    BDBG_OBJECT_SET(pSource, BVDC_SRC);

    /* Initialize non-changing states.  These are not to be changed by runtime. */
    pSource->eId           = eSourceId;
    pSource->hVdc          = hVdc;

    /* (5) Created a specific source handle. */
    if (BAVC_SourceId_eGfx0 <= eSourceId &&
        BAVC_SourceId_eGfx6 >= eSourceId)
        BVDC_P_GfxFeeder_Create(
            &pSource->hGfxFeeder, hVdc->hRegister, hVdc->hRdc, eSourceId, b3dSrc, pSource);

    /* (6) create a AppliedDone event. */
    BKNI_CreateEvent(&pSource->hAppliedDoneEvent);

    /* (7) Added this compositor to hVdc */
    hVdc->ahSource[eSourceId] = (BVDC_Source_Handle)pSource;

    /* All done. now return the new fresh context to user. */
    *phSource = (BVDC_Source_Handle)pSource;

    BDBG_LEAVE(BVDC_P_Source_Create);

    return BERR_TRACE(eStatus);
}

/***************************************************************************
* {private}
*
*/
void BVDC_P_Source_Init
    ( BVDC_Source_Handle               hSource,
      const BVDC_Source_CreateSettings *pDefSettings )
{
    BVDC_P_Source_Info *pNewInfo;
    BVDC_P_Source_Info *pCurInfo;
    BVDC_P_Source_IsrInfo *pIsrInfo;

    BDBG_ENTER(BVDC_P_Source_Init);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* Which heap to use? */
    hSource->hHeap = ((pDefSettings) && (pDefSettings->hHeap))
        ? pDefSettings->hHeap : hSource->hVdc->hBufferHeap;

    /* New/Cur/Isr Info */
    pNewInfo = &hSource->stNewInfo;
    pCurInfo = &hSource->stCurInfo;
    pIsrInfo = &hSource->stIsrInfo;

    /* Initialize states can be changed by runtime. */
    hSource->bInitial             = true;
    hSource->bRasterChanged       = true;
    hSource->eState               = BVDC_P_State_eInactive;

    /* Reset done events */
    BKNI_ResetEvent(hSource->hAppliedDoneEvent);

    /* Clear out user's states. */
    BKNI_Memset((void*)pNewInfo, 0x0, sizeof(BVDC_P_Source_Info));
    BKNI_Memset((void*)pCurInfo, 0x0, sizeof(BVDC_P_Source_Info));
    BKNI_Memset((void*)pIsrInfo, 0x0, sizeof(BVDC_P_Source_IsrInfo));

    if(hSource->hGfxFeeder)
    {
        BVDC_P_GfxFeeder_Init(hSource->hGfxFeeder, NULL);
        pNewInfo->eCtInputType = BVDC_P_CtInput_eUnknown;
    }

    BDBG_LEAVE(BVDC_P_Source_Init);
    return;
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
        if((BVDC_P_STATE_IS_ACTIVE(ahSource[i]) ||
            BVDC_P_STATE_IS_CREATE(ahSource[i])) &&
           (BVDC_P_SRC_IS_GFX(ahSource[i]->eId)))
        {
            /* validate Gfx feeder, surface and window combination,
             * note: Gfx feeder can not be shared by more than one window */
            eStatus = BERR_TRACE(BVDC_P_GfxFeeder_ValidateChanges(
                ahSource[i]->hGfxFeeder, ahSource[i]->stNewInfo.pfPicCallbackFunc));
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
        BVDC_P_Source_DirtyBits *pIsrDirty;

        pIsrDirty = &hSource->stIsrInfo.stDirty;

        /* inform next ApplyChanges to copy activated isr setting into new info */
        BVDC_P_OR_ALL_DIRTY(&hSource->stIsrInfo.stActivated, pIsrDirty);

        /* Clear dirty bits since it's already OR'ed into current */
        BVDC_P_CLEAN_ALL_DIRTY(pIsrDirty);
    }

    BDBG_LEAVE(BVDC_P_Source_UpdateSrcState_isr);
    return;
}

/***************************************************************************
* {private}
*
*/
void BVDC_P_Source_ApplyChanges_isr
( BVDC_Source_Handle               hSource )
{
    BVDC_P_Source_DirtyBits *pNewDirty;

    BDBG_ENTER(BVDC_P_Source_ApplyChanges_isr);

    /* Did any of the dirty got set. */
    pNewDirty = &hSource->stNewInfo.stDirty;
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

    /* First time bring up the source, kick start the initial trigger. */
    if(hSource->bInitial)
    {
        BVDC_P_Source_UpdateSrcState_isr(hSource);
        hSource->bInitial = false;
    }

    BDBG_LEAVE(BVDC_P_Source_ApplyChanges_isr);
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
    hSource->ahWindow[hWindow->eId] = hWindow;
    hWindow->hCompositor->ulActiveGfxWindow++;
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
    BDBG_ENTER(BVDC_P_Source_DisconnectWindow_isr);
    hSource->ahWindow[hWindow->eId] = NULL;
    hWindow->hCompositor->ulActiveGfxWindow--;
    BDBG_LEAVE(BVDC_P_Source_DisconnectWindow_isr);
    return;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Source_SetSurface
    ( BVDC_Source_Handle      hSource,
     const BAVC_Gfx_Picture  *pAvcGfxPic)
{
    BERR_Code  eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Source_SetSurface);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_ASSERT(NULL != pAvcGfxPic);

    BKNI_EnterCriticalSection();
    BVDC_P_CHECK_CS_ENTER_VDC(hSource->hVdc);

    if (NULL != hSource->hGfxFeeder && NULL != pAvcGfxPic)
    {
        eStatus = BVDC_P_GfxSurface_SetSurface_isr(
            &hSource->hGfxFeeder->stGfxSurface,
            &hSource->hGfxFeeder->stGfxSurface.stNewSurInfo, pAvcGfxPic, hSource);
    }
    else
    {
        BDBG_ERR(("Invalid source handle or gfx pic to set gfx surface."));
        eStatus = BERR_INVALID_PARAMETER;
    }

    BVDC_P_CHECK_CS_LEAVE_VDC(hSource->hVdc);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVDC_Source_SetSurface);
    return BERR_TRACE(eStatus);
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Window_Create
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_Window_Handle              *phWindow,
      BVDC_WindowId                    eId,
      BVDC_Source_Handle               hSource,
      const BVDC_Window_Settings      *pDefSettings )
{
    BVDC_Window_Handle hWindow;
    BERR_Code eRet = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Window_Create);

    BDBG_ASSERT(hCompositor);
    BDBG_ASSERT(phWindow);
    BDBG_ASSERT(hSource);

    /* Check internally if we created. */
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

   /* Make sure the source/compositor is active or created. */
    if(BVDC_P_STATE_IS_INACTIVE(hSource) ||
       BVDC_P_STATE_IS_DESTROY(hSource) ||
       BVDC_P_STATE_IS_INACTIVE(hCompositor) ||
       BVDC_P_STATE_IS_DESTROY(hCompositor))
    {
        BDBG_ERR(("Compositor and/or Source are not active."));
        return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
    }

    eRet = BVDC_P_Window_Create(hCompositor, phWindow,hSource->eId, eId);
    if (BERR_SUCCESS != eRet)
    {
        return BERR_TRACE(eRet);
    }

    /* select the corresponding private handle created at BVDC_Open */
    eRet = BVDC_P_Window_GetPrivHandle(hCompositor, eId, hSource->eId, &hWindow, NULL);
    if (BERR_SUCCESS != eRet)
    {
        return BERR_TRACE(eRet);
    }

    /* Check if the window is already created or not. */
    if(!BVDC_P_STATE_IS_INACTIVE(hWindow))
    {
        BDBG_ERR(("Window[%d] state = %d", hWindow->eId, hWindow->eState));
        return BERR_TRACE(BVDC_ERR_WINDOW_ALREADY_CREATED);
    }

    BDBG_MSG(("Creating window%d", hWindow->eId));
    BDBG_ASSERT(BVDC_P_STATE_IS_INACTIVE(hWindow));

    if (BVDC_P_SRC_IS_GFX(hSource->eId))
    {
        /* this makes gfx surface be validated in next ApplyChanges */
        hSource->hGfxFeeder->hWindow = hWindow;
        hSource->hGfxFeeder->stCfc.pColorSpaceExtOut = &(hWindow->hCompositor->stOutColorSpaceExt);
    }

    /* Reinitialize context.  But not make it active until apply. */
    *phWindow = hWindow;

    /* Which heap to use? */
    if(pDefSettings)
    {
        hWindow->hCapHeap = (pDefSettings->hHeap)
            ? pDefSettings->hHeap  : hCompositor->hVdc->hBufferHeap;
        hWindow->hDeinterlacerHeap = (pDefSettings->hDeinterlacerHeap)
            ? pDefSettings->hDeinterlacerHeap : hWindow->hCapHeap;
    }
    else
    {
        hWindow->hCapHeap = hCompositor->hVdc->hBufferHeap;
        hWindow->hDeinterlacerHeap = hWindow->hCapHeap;
    }

    /* Default settings */
    if(pDefSettings)
    {
        bool bBypassDirty;

        hWindow->stSettings = *pDefSettings;
        /* Do not allow override of number of rect this is read-only
         * information. */
        hWindow->hCompositor->hDisplay->stNewInfo.bBypassVideoProcess =
            pDefSettings->bBypassVideoProcessings;

        bBypassDirty =
            hWindow->hCompositor->hDisplay->stNewInfo.bBypassVideoProcess ^
            hWindow->hCompositor->hDisplay->stCurInfo.bBypassVideoProcess;
        hWindow->hCompositor->hDisplay->stNewInfo.stDirty.stBits.bHdmiCsc |= bBypassDirty;
    }
    else
    {
        BVDC_Window_GetDefaultSettings(hWindow->eId, &hWindow->stSettings);
    }


    /* Inititial window fields */
    BVDC_P_Window_Init(*phWindow, hSource);

    /* Keep count of new number of windows that connected to source! */
    hSource->stNewInfo.ulWindows++;
    hSource->stNewInfo.stDirty.stBits.bWindowNum = BVDC_P_DIRTY;

    /* Mark as create, awating for apply. */
    hWindow->eState = BVDC_P_State_eCreate;

    BDBG_MSG(("Window[%d] is BVDC_P_State_eCreate", hWindow->eId));

    BDBG_LEAVE(BVDC_Window_Create);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Window_GetPrivHandle
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_WindowId                    eWinId,
      BAVC_SourceId                    eSrcId,
      BVDC_Window_Handle              *phWindow,
      BVDC_P_WindowId                 *peWindowId )
{
    BVDC_Window_Handle  hWindow;
    BVDC_P_WindowId  eWindowId;

    BSTD_UNUSED (eWinId);
    BSTD_UNUSED (eSrcId);
    eWindowId = (BVDC_CompositorId_eCompositor1 == hCompositor->eId)?
        BVDC_P_WindowId_eComp1_G0 : BVDC_P_WindowId_eComp0_G0;
    hWindow = hCompositor->ahWindow[eWindowId];
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(phWindow)
    {
        *phWindow = hWindow;
    }

    if(peWindowId)
    {
        *peWindowId = eWindowId;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Window_Create
    ( BVDC_Compositor_Handle            hCompositor,
      BVDC_Window_Handle               *phWindow,
      BAVC_SourceId                     eSrcId,
      BVDC_WindowId                     eWinId )
{
    BVDC_P_WindowContext *pWindow;
    BVDC_P_WindowId      eWindowId;

    BSTD_UNUSED (eSrcId);
    BSTD_UNUSED (eWinId);
    BDBG_ENTER(BVDC_P_Window_Create);
    BDBG_ASSERT(phWindow);

    /* Get relate context. */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

    /* Determine what window id to use. */
    eWindowId = (BVDC_CompositorId_eCompositor1 == hCompositor->eId)?
        BVDC_P_WindowId_eComp1_G0 : BVDC_P_WindowId_eComp0_G0;

    if(hCompositor->ahWindow[eWindowId]!=NULL)
    {
        BDBG_MSG(("cmp[%d] win[%d] has been created state %d", hCompositor->eId, eWinId, hCompositor->ahWindow[eWindowId]->eState));
        *phWindow = hCompositor->ahWindow[eWindowId];
        BDBG_LEAVE(BVDC_P_Window_Create);
        return BERR_SUCCESS;
    }

    /* (1) Alloc the context. */
    pWindow = (BVDC_P_WindowContext*)
        (BKNI_Malloc(sizeof(BVDC_P_WindowContext)));
    if(!pWindow)
    {
        BDBG_LEAVE(BVDC_P_Window_Create);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pWindow, 0x0, sizeof(BVDC_P_WindowContext));
    BDBG_OBJECT_SET(pWindow, BVDC_WIN);

    /* Initialize window context */
    pWindow->eId          = eWindowId;
    pWindow->ulRegOffset  = BVDC_P_WIN_GET_REG_OFFSET(eWindowId);
    pWindow->hCompositor  = hCompositor;

    /* (6) create a DestroyDone event. */
    BKNI_CreateEvent(&pWindow->hDestroyDoneEvent);

    /* (7) create a AppliedDone event. */
    BKNI_CreateEvent(&pWindow->hAppliedDoneEvent);

    /* (9) Sync up stNewResource and stCurResource. For historical reasons, these resources are
    * not allocated via the resource management library. Weird, but that's how it has been.
    */
    pWindow->stCurResource = pWindow->stNewResource;

    /* (10) Added this compositor to hVdc */
    hCompositor->ahWindow[pWindow->eId] = (BVDC_Window_Handle)pWindow;

    /* All done. now return the new fresh context to user. */
    *phWindow = (BVDC_Window_Handle)pWindow;

    BDBG_LEAVE(BVDC_P_Window_Create);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Window_Init
    ( BVDC_Window_Handle               hWindow,
      BVDC_Source_Handle               hSource )
{
    BVDC_P_Window_Info *pNewInfo;
    BVDC_P_Window_Info *pCurInfo;
    /* coverity[result_independent_of_operands: FALSE] */

    BDBG_ENTER(BVDC_P_Window_Init);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    /* Init to default state. */
    hWindow->eState               = BVDC_P_State_eInactive;

    /* Reset done events */
    BKNI_ResetEvent(hWindow->hDestroyDoneEvent);
    BKNI_ResetEvent(hWindow->hAppliedDoneEvent);
    hWindow->bSetDestroyEventPending = false;
    hWindow->bSetAppliedEventPending = false;

    /* Initial new/current public states */
    pNewInfo = &hWindow->stNewInfo;
    pCurInfo = &hWindow->stCurInfo;

    /* Clear out user's states. */
    BKNI_Memset((void*)pNewInfo, 0x0, sizeof(BVDC_P_Window_Info));
    BKNI_Memset((void*)pCurInfo, 0x0, sizeof(BVDC_P_Window_Info));

    /* Disconnecting/Connecting Source */
    hWindow->stNewInfo.hSource     = hSource;

    /* Misc flags. */
    pNewInfo->ucAlpha              = BVDC_ALPHA_MAX;
    pNewInfo->ucConstantAlpha      = BVDC_ALPHA_MAX;
    pNewInfo->eFrontBlendFactor    = BVDC_BlendFactor_eSrcAlpha;
    pNewInfo->eBackBlendFactor     = BVDC_BlendFactor_eOneMinusSrcAlpha;

    /* Where on the canvas */
    pNewInfo->stDstRect.ulWidth  = hWindow->hCompositor->stCurInfo.pFmtInfo->ulWidth;
    pNewInfo->stDstRect.ulHeight = hWindow->hCompositor->stCurInfo.pFmtInfo->ulHeight;

    /* Where on the scaler's output. */
    pNewInfo->stScalerOutput     = pNewInfo->stDstRect;

    /* Clear out user's states. */
    BKNI_Memcpy(pCurInfo, pNewInfo, sizeof(BVDC_P_Window_Info));

    return;
}

/***************************************************************************
 * {private}
 *
 * This should contains all the information to detect user error settings.
 * User settings that required checking happen here.
 */
BERR_Code BVDC_P_Window_ValidateChanges
    ( const BVDC_Window_Handle         hWindow,
      const BFMT_VideoInfo            *pDstFormatInfo )
{
    BVDC_P_Window_Info *pNewInfo;
    uint32_t ulHsize, ulVsize;
    bool  bDtg;

    BDBG_ENTER(BVDC_P_Window_ValidateChanges);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stNewInfo.hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor->hDisplay, BVDC_DSP);

    bDtg = BVDC_P_DISPLAY_USED_DIGTRIG(hWindow->hCompositor->hDisplay->eMasterTg);
    ulHsize = bDtg? pDstFormatInfo->ulDigitalWidth : pDstFormatInfo->ulWidth;
    ulVsize = bDtg? pDstFormatInfo->ulDigitalHeight: pDstFormatInfo->ulHeight;

    /* To reduce the amount of typing */
    pNewInfo = &hWindow->stNewInfo;

    /* (1) Destination rect is bigger than canvas. */
    if((pNewInfo->stDstRect.ulWidth  > ulHsize) ||
       (pNewInfo->stDstRect.ulHeight > ulVsize))
    {
        BDBG_ERR(("DstRect[%dx%d], Canvas[%dx%d], Orientation[%d].",
            pNewInfo->stDstRect.ulWidth, pNewInfo->stDstRect.ulHeight,
            ulHsize, ulVsize, hWindow->hCompositor->stNewInfo.eOrientation));
        BDBG_ASSERT(0);
        return BERR_TRACE(BVDC_ERR_DST_SIZE_LARGER_THAN_CANVAS);
    }

    /* (2) DstRect can not be larger than scaler output. */
    if((pNewInfo->stDstRect.ulWidth  + pNewInfo->stScalerOutput.lLeft > pNewInfo->stScalerOutput.ulWidth) ||
       (pNewInfo->stDstRect.ulHeight + pNewInfo->stScalerOutput.lTop  > pNewInfo->stScalerOutput.ulHeight))
    {
        return BERR_TRACE(BVDC_ERR_DST_SIZE_LARGER_THAN_SCL_OUTPUT);
    }

    /* (2.5) DstRect can not be out of bound of canvas. */
    if((pNewInfo->stDstRect.ulWidth  + pNewInfo->stDstRect.lLeft > ulHsize)  ||
       (pNewInfo->stDstRect.ulHeight + pNewInfo->stDstRect.lTop  > ulVsize) ||
       (pNewInfo->stDstRect.lLeft < 0) || (pNewInfo->stDstRect.lTop < 0))
    {
        BDBG_ERR(("DstRect has to be inside display boundary for now!"));
        return BERR_TRACE(BVDC_ERR_DST_RECT_OUT_OF_BOUND);
    }

    BDBG_LEAVE(BVDC_P_Window_ValidateChanges);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Window_SetMiscellaneous_isr
    ( BVDC_Window_Handle               hWindow,
      const BVDC_P_Window_Info        *pWinInfo )
{
    BDBG_ENTER(BVDC_P_Window_SetMiscellaneous_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        /* Enable or disable visibility
         * note: non-zero value being written to the reserved bits of
         * CMP_x_G0_SURFACE_CTRL register might kill VEC */
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE));
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
            BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, pWinInfo->bVisible ? 1 : 0));
    }

    BDBG_LEAVE(BVDC_P_Window_SetMiscellaneous_isr);
    return;
}

/* blender address tables */
static const uint32_t s_aulBlendAddr[] =
{
    BCHP_CMP_0_BLEND_0_CTRL,
    BCHP_CMP_0_BLEND_1_CTRL,
};

#define BVDC_P_CMP_GET_REG_ADDR_IDX(reg) \
    ((reg- BCHP_CMP_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_CMP_GET_REG_ADDR_DATA(reg) \
    (hCompositor->aulRegs[BVDC_P_CMP_GET_REG_ADDR_IDX(reg)])

/***************************************************************************
 * {private}
 *
 * Configure a blender.
 */
void BVDC_P_Window_SetBlender_isr
    ( BVDC_Window_Handle               hWindow,
      uint8_t                          ucZOrder,
      uint8_t                          ucConstantAlpha,
      BVDC_BlendFactor                 eFrontBlendFactor,
      BVDC_BlendFactor                 eBackBlendFactor )
{
    BVDC_Compositor_Handle hCompositor;
    uint32_t ulBlendAddr     = 0;
    uint32_t ulBlendSrcSel   = 0;

    BSTD_UNUSED(ucZOrder);
    BDBG_ENTER(BVDC_P_Window_SetBlender_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Get the compositor that this window belongs to.   The blender
     * registers are spread out in the compositor. */
    hCompositor = hWindow->hCompositor;
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* Note: These are compositor's MACRO.  The Z-Order of each window
     * acctually selected the blender. */
    ulBlendAddr       = s_aulBlendAddr[hWindow->ulBlenderId];

    /* apply blender source to the correct blender */
    ulBlendSrcSel = BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, SURFACE_G0);
    BVDC_P_CMP_GET_REG_ADDR_DATA(ulBlendAddr) &= ~(
        BCHP_MASK(CMP_0_BLEND_0_CTRL, BLEND_SOURCE));
    BVDC_P_CMP_GET_REG_ADDR_DATA(ulBlendAddr) |= ulBlendSrcSel;

    BVDC_P_GfxFeeder_AdjustBlend_isr(hWindow->stNewInfo.hSource->hGfxFeeder,
        &eFrontBlendFactor, &eBackBlendFactor, &ucConstantAlpha);

    /* Blending factors */
    BVDC_P_CMP_GET_REG_ADDR_DATA(ulBlendAddr) &= ~(
        BCHP_MASK(CMP_0_BLEND_0_CTRL, CONSTANT_ALPHA          ) |
        BCHP_MASK(CMP_0_BLEND_0_CTRL, FRONT_COLOR_BLEND_FACTOR) |
        BCHP_MASK(CMP_0_BLEND_0_CTRL, BACK_COLOR_BLEND_FACTOR ));
    BVDC_P_CMP_GET_REG_ADDR_DATA(ulBlendAddr) |=  (
        BCHP_FIELD_DATA(CMP_0_BLEND_0_CTRL, CONSTANT_ALPHA, ucConstantAlpha) |
        BCHP_FIELD_DATA(CMP_0_BLEND_0_CTRL, FRONT_COLOR_BLEND_FACTOR, eFrontBlendFactor) |
        BCHP_FIELD_DATA(CMP_0_BLEND_0_CTRL, BACK_COLOR_BLEND_FACTOR, eBackBlendFactor));

    BDBG_LEAVE(BVDC_P_Window_SetBlender_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Window_UpdateUserState_isr
    ( BVDC_Window_Handle               hWindow )
{
    BDBG_ENTER(BVDC_P_Window_UpdateUserState_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Use the new applied states, and make it as current. */
    if(hWindow->bUserAppliedChanges)
    {
        BVDC_P_Window_DirtyBits *pNewDirty, *pCurDirty;

        pCurDirty = &hWindow->stCurInfo.stDirty;
        pNewDirty = &hWindow->stNewInfo.stDirty;

        /* Copying the new info to the current info.  Must be careful here
         * of not globble current dirty bits set by source, but rather ORed
         * them together. */
        BVDC_P_OR_ALL_DIRTY(pNewDirty, pCurDirty);

        /* copy stNewInfo to stCurInfo here !!! */
        hWindow->stCurInfo = hWindow->stNewInfo;

        /* Clear dirty bits since it's already OR'ed into current.  Notes
         * the it might not apply until next vysnc, so we're defering
         * setting the event until next vsync. */
        BVDC_P_CLEAN_ALL_DIRTY(pNewDirty);
        hWindow->bUserAppliedChanges = false;

        /* can not set them into Active here in case ApplyChanges are called
         * twice before Writer_isr and Reader_isr are called ? */
        hWindow->stNewInfo.eWriterState = BVDC_P_State_eInactive;
        hWindow->stNewInfo.eReaderState = BVDC_P_State_eInactive;
    }

    BDBG_LEAVE(BVDC_P_Window_UpdateUserState_isr);
    return;
}

/***************************************************************************
 * {private}
 */
BERR_Code BVDC_P_Window_ApplyChanges_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_Window_Info *pNewInfo;

    BDBG_ENTER(BVDC_P_Window_ApplyChanges_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /* Get the compositor that this window belongs to. */
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hWindow->stNewInfo.hSource, BVDC_SRC);

    /* To reduce the amount of typing */
    pNewInfo  = &hWindow->stNewInfo;

    /* Update to take in new changes. */
    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        BVDC_P_Window_SetBlender_isr(hWindow, pNewInfo->ucZOrder,
            pNewInfo->ucConstantAlpha, pNewInfo->eFrontBlendFactor,
            pNewInfo->eBackBlendFactor);
    }

    BVDC_P_Window_SetMiscellaneous_isr(hWindow, pNewInfo);

    /* State transitions. */
    if(BVDC_P_STATE_IS_CREATE(hWindow))
    {
        /* (1) Connect this window with new source. */
        hWindow->eState = BVDC_P_State_eActive;

        /* this flags a window is being created; */

        BVDC_P_Source_ConnectWindow_isr(hWindow->stNewInfo.hSource, hWindow);
    }
    hWindow->bUserAppliedChanges = true;

    /* reset the count for gfx window to be programmed later in reader isr to
     * accomandate vbi pass through info. */
    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        /* hWindow->ulGwinSetCount = 0; */
        /* BACK OUT the above changes for now to make analog working */
        BAVC_Polarity eScanType = (hWindow->hCompositor->stNewInfo.pFmtInfo->bInterlaced)
            ? BAVC_Polarity_eTopField : BAVC_Polarity_eFrame;
        BVDC_P_GfxFeeder_GetAdjSclOutRect_isr(&pNewInfo->stSrcClip, &pNewInfo->stScalerOutput,
            &pNewInfo->stDstRect, &hWindow->stAdjSclOut);
        BVDC_P_Window_SetSurfaceSize_isr(hWindow, &hWindow->stAdjSclOut, eScanType);
        BVDC_P_Window_SetDisplaySize_isr(hWindow, &pNewInfo->stDstRect, eScanType,
            (uint32_t)(pNewInfo->stDstRect.lLeft + pNewInfo->lRWinXOffsetDelta));
    }

    /* Isr will set event to notify apply done. */
    if(hWindow->bUserAppliedChanges)
    {
        /* copy stNewInfo to stCurInfo here !!! */
        BVDC_P_Window_UpdateUserState_isr(hWindow);
    }

    BDBG_LEAVE(BVDC_P_Window_ApplyChanges_isr);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 * note: a sub-module might be shared by more than one window, but the
 * sharing is handled by each sub-module. And each sub-module will
 * acquire and release the pre-patch of free-channel or loop-back
 * internally basing on eVnetPatchMode.
 *
 * return: if reader is on
 */
static bool BVDC_P_Window_BuildReaderRul_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eNextFieldId,
      bool                             bBuildCanvasCtrl )
{
    BVDC_Compositor_Handle hCompositor;
    BVDC_P_State  eReaderState;

    BSTD_UNUSED (bBuildCanvasCtrl);
    BDBG_ENTER(BVDC_P_Window_BuildReaderRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    hCompositor = hWindow->hCompositor;
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hDisplay, BVDC_DSP);

    /* Note: When building RUL, hardware highly recommend that we build from
     * backend to the frontend, that is starting with:
     *   VEC, CMP, SCL, VFD, CAP, MFD.  This to prevent the false start of
     * downstream modules.
     * Note: In the case readerState is not eActive, we still needs to build
     * RUL in order to shut down. VnetMode is the only one that indicates if
     * we need to build RUL */
    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        if((BVDC_P_State_eInactive == hWindow->stCurInfo.eReaderState) &&
           (hWindow->stCurInfo.bVisible))
        {
            hWindow->stCurInfo.eReaderState = BVDC_P_State_eActive;
            hWindow->stCurInfo.eWriterState = BVDC_P_State_eActive;
        }

        eReaderState = hWindow->stCurInfo.eReaderState;

        /* VEC alignment may be too fast to sustain gfx RTS; mute it! */
        if((BVDC_P_State_eActive == eReaderState) &&
           ((hCompositor->hDisplay->bAlignAdjusting && !hCompositor->hDisplay->stCurInfo.stAlignCfg.bKeepBvnConnected)||

            (0==hWindow->stCurInfo.hSource->hGfxFeeder->stGfxSurface.stCurSurInfo.ullAddress) || /* no valid sur */

            (!hWindow->stCurInfo.bVisible)))  /* muted by user */
        {
            eReaderState = BVDC_P_State_eShutDownRul;
        }

        /* according to readerState, enable or disable gfx surface in cmp,
         * and build rul for GFD */
        if (BVDC_P_State_eActive != eReaderState)
        {
            BVDC_P_WIN_WRITE_IMM_TO_RUL(CMP_0_V0_SURFACE_CTRL, 0, pList->pulCurrent);
        }
        else
        {
            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
                BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, 1));
            BVDC_P_WIN_WRITE_TO_RUL(CMP_0_V0_SURFACE_CTRL, pList->pulCurrent);
        }

        if (hWindow->stCurInfo.hSource->hGfxFeeder->stGfxSurface.stCurSurInfo.ullAddress)
        {
            BVDC_P_GfxFeeder_UpdateState_isr(hWindow->stCurInfo.hSource->hGfxFeeder,
                &(hWindow->stCurInfo.hSource->stCurInfo), pList, eNextFieldId);
            BVDC_P_GfxFeeder_BuildRul_isr(hWindow->stCurInfo.hSource->hGfxFeeder,
                pList, eNextFieldId, eReaderState);
        }
    }

    BDBG_LEAVE(BVDC_P_Window_BuildReaderRul_isr);
    return (BVDC_P_State_eActive == eReaderState);
}


/***************************************************************************
 * {private}
 * note: a sub-module might be shared by more than one window, but the
 * sharing is handled by each sub-module. And each sub-module will
 * acquire and release the pre-patch of free-channel or loop-back
 * internally basing on eVnetPatchMode.
 */
void BVDC_P_Window_BuildRul_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eNextFieldId,
      bool                             bBuildWriter,
      bool                             bBuildReader,
      bool                             bBuildCanvasCtrl )
{
    BSTD_UNUSED (bBuildWriter);
    BDBG_ENTER(BVDC_P_Window_BuildRul_isr);

    /* Note: When building RUL, hardware highly recommend that we build from
     * backend to the frontend, that is starting with:
     *   VEC, CMP, SCL, VFD, CAP, MFD.  This to prevent the false start of
     * downstream modules. */
    /* TODO: in the case of sync mpeg display, we might want to call
     * BVDC_P_Window_BuildReaderRul_isr and BVDC_P_Window_BuildWriterRul_isr
     * twice in the same vsync/RUL, shut-down old vnet the 1st time and build
     * new vnet the the 2nd time. If we do so, remember to modify subrul to
     * store the last released patch mux addr/mode for RUL loss handling */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /*BDBG_MSG(("Build Rul for Window[%d], Reader %d, Writer %d, nextFld %d, List 0x%x",
       hWindow->eId, bBuildReader, bBuildWriter, eNextFieldId, pList));*/
    if((hWindow->stCurInfo.stDirty.stBits.bShutdown) ||
       (BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId)))
    {
        if(bBuildReader)
        {
            BVDC_P_Window_BuildReaderRul_isr(hWindow, pList, eNextFieldId, bBuildCanvasCtrl);
        }
    }

    BDBG_LEAVE(BVDC_P_Window_BuildRul_isr);
    return;
}

/**************************************************************************
 * This function will call the READER of all the windows hold by this compositor,
 * and determine the input color space to VEC, also select the video window
 * color space conversion matrix if necessary.
 */
void BVDC_P_Compositor_WindowsReader_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BAVC_Polarity                    eNextFieldId,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t                  ulVSize, ulHSize;
    BVDC_DisplayTg            eMasterTg;
    bool                      bDTg;

    const BFMT_VideoInfo     *pFmtInfo;

    BSTD_UNUSED (pList);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* set compositor size -- number of lines. */
    eMasterTg = hCompositor->hDisplay->eMasterTg;
    bDTg      =  BVDC_P_DISPLAY_USED_DIGTRIG(eMasterTg);
    pFmtInfo = hCompositor->stCurInfo.pFmtInfo;

    ulHSize   = bDTg ? pFmtInfo->ulDigitalWidth :pFmtInfo->ulWidth;
    ulVSize   = bDTg ? pFmtInfo->ulDigitalHeight:pFmtInfo->ulHeight;
    ulVSize >>= (eNextFieldId != BAVC_Polarity_eFrame);
    /*BDBG_MODULE_MSG(BVDC_CMP_SIZE,("Canvas[%d] %4d x %4d", hCompositor->eId,  ulHSize, ulVSize));*/
    BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_SIZE) = (
        BCHP_FIELD_DATA(CMP_0_CANVAS_SIZE, HSIZE, ulHSize) |
        BCHP_FIELD_DATA(CMP_0_CANVAS_SIZE, VSIZE, ulVSize));

    BDBG_LEAVE(BVDC_P_Compositor_WindowsReader_isr);
    return;
}

void BVDC_P_Csc_ApplyYCbCrColor
    ( BVDC_P_CscCoeffs                *pCscCoeffs,
      uint32_t                         ulColor0,
      uint32_t                         ulColor1,
      uint32_t                         ulColor2 )
{
    BSTD_UNUSED(pCscCoeffs);
    BSTD_UNUSED(ulColor0);
    BSTD_UNUSED(ulColor1);
    BSTD_UNUSED(ulColor2);
}

/***************************************************************************
*
*/
void BVDC_P_Source_CleanupSlots_isr
( BVDC_Source_Handle               hSource )
{
    BSTD_UNUSED (hSource);
}

void BVDC_P_Csc_Print_isr
    ( const BVDC_P_CscCoeffs          *pCscCoeffs )
{
    BSTD_UNUSED (pCscCoeffs);
}

/* End of File */
