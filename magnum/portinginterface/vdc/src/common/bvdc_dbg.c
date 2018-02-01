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

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bvdc.h"
#include "brdc_dbg.h"
#include "bvdc_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_dnr_priv.h"
#include "bvdc_hscaler_priv.h"
#include "bvdc_mcvp_priv.h"
#include "bvdc_mcdi_priv.h"
#include "bvdc_anr_priv.h"
#include "bvdc_capture_priv.h"
#include "bvdc_feeder_priv.h"

BDBG_MODULE(BVDC_DBG);


/***************************************************************************
 * Generic error recovery handler for BVN blocks.
 */
void BVDC_P_BvnErrorHandler_isr
    ( void                            *pvhVdc,
      int                              iIdx )
{
    BVDC_Handle hVdc = (BVDC_Handle)pvhVdc;
    const BVDC_P_IntCbTbl *pIntCb;
    uint32_t ulReg;

    /* log the interrupt count */
    hVdc->aulBvnErrCnt[iIdx]++;

    pIntCb = BVDC_P_GetBvnErrorCb_isr(iIdx);
    if(pIntCb != NULL)
    {
        if(pIntCb->ulBvbStatus != 0)
        {
            ulReg = BREG_Read32_isr(hVdc->hRegister, pIntCb->ulBase + pIntCb->ulBvbStatus);
            BKNI_Snprintf(hVdc->achBuf, BVDC_P_ERROR_MAX_MSG_LENGTH, "BVN Error: %s_STATUS %#x Addr 0x%x [ts=%u]", pIntCb->pchInterruptName, ulReg, pIntCb->ulBase + pIntCb->ulBvbStatus, BRDC_GetCurrentTimer_isr(hVdc->hRdc));
        }
        else
        {
            BKNI_Snprintf(hVdc->achBuf, BVDC_P_ERROR_MAX_MSG_LENGTH, "BVN error in %s [ts=%u]", pIntCb->pchInterruptName, BRDC_GetCurrentTimer_isr(hVdc->hRdc));
        }

        /* Log to RDC's circular buffer so we see what may have caused it */
        BRDC_DBG_LogErrorCode_isr(hVdc->hRdc, BRDC_DBG_BVN_ERROR, hVdc->achBuf);

        if(!hVdc->abBvnErrMask[iIdx])
        {
            BDBG_WRN(("%s", hVdc->achBuf));

            if(hVdc->pfGenericCallback != NULL)
            {
                hVdc->pfGenericCallback(hVdc,0, NULL);
            }
        }
    }

    return;
}

#if !defined(B_REFSW_MINIMAL)
/***************************************************************************
 *
 */
BERR_Code BVDC_Dbg_InstallCallback
    ( BVDC_Handle                      hVdc,
      const BVDC_CallbackFunc_isr      pfCallback,
      void                            *pvParm1,
      int                              iParm2 )
{
    hVdc->pfGenericCallback = pfCallback;
    BSTD_UNUSED(pvParm1);
    BSTD_UNUSED(iParm2);
    return BERR_SUCCESS;
}
#endif

#if !defined(B_REFSW_MINIMAL) || defined(BVDC_SUPPORT_BVN_DEBUG)
/***************************************************************************
 *
 */
BERR_Code BVDC_P_CreateErrCb
    ( BVDC_Handle                  hVdc )
{
    uint32_t i;
    BERR_Code eStatus = BERR_SUCCESS;
    const BVDC_P_IntCbTbl *pIntCb;

    for(i = 0; i < BVDC_BvnError_eMaxCount; i++)
    {
        BKNI_EnterCriticalSection();
        pIntCb = BVDC_P_GetBvnErrorCb_isr(i);
        BKNI_LeaveCriticalSection();
        if(pIntCb == NULL)
            continue;

#if (BVDC_P_DCXM_CAP_PADDING_WORKAROUND)
        if(pIntCb->ulGroupBase == BCHP_CAP_0_REG_START)
            continue;
#endif

#if (BVDC_P_MASK_GFD_BSTC_MIN_GT_MAX)
         if((hVdc->stBoxConfig.stVdc.astSource[BAVC_SourceId_eGfx0].bCompressed)
            &&(pIntCb->ulBase == BCHP_GFD_0_REG_START))
            continue;
#endif
        BDBG_MSG(("Install IntId %s(%d) L2Reg=0x%x Mask=0x%x",
            pIntCb->pchInterruptName, i, pIntCb->ulL2ClearReg, pIntCb->ulL2ClearMask));
        eStatus = BINT_CreateCallback(&hVdc->ahBvnErrHandlerCb[i], hVdc->hInterrupt,
            pIntCb->ErrIntId,
            pIntCb->pfCallback,
            (void*)hVdc, i);
        if(BERR_SUCCESS != eStatus)
        {
            return BERR_TRACE(eStatus);
        }

        eStatus = BINT_ClearCallback(hVdc->ahBvnErrHandlerCb[i]);
        if(BERR_SUCCESS != eStatus)
        {
            return BERR_TRACE(eStatus);
        }

        eStatus = BINT_EnableCallback(hVdc->ahBvnErrHandlerCb[i]);
        if(BERR_SUCCESS != eStatus)
        {
            return BERR_TRACE(eStatus);
        }
    }

    return eStatus;
}
#endif


#if !defined(B_REFSW_MINIMAL) || defined(BVDC_SUPPORT_BVN_DEBUG)
/***************************************************************************
 *
 */
BERR_Code BVDC_P_DestroyErrCb
    ( BVDC_Handle                  hVdc )
{
    uint32_t i;
    BERR_Code eStatus = BERR_SUCCESS;

    for(i = 0; i < BVDC_BvnError_eMaxCount; i++)
    {
        if(hVdc->ahBvnErrHandlerCb[i] == NULL)
            continue;

        eStatus = BINT_DisableCallback(hVdc->ahBvnErrHandlerCb[i]);
        if(BERR_SUCCESS != eStatus)
        {
            return BERR_TRACE(eStatus);
        }

        eStatus = BINT_DestroyCallback(hVdc->ahBvnErrHandlerCb[i]);
        if(BERR_SUCCESS != eStatus)
        {
            return BERR_TRACE(eStatus);
        }
    }

    return eStatus;
}
#endif


#if !defined(B_REFSW_MINIMAL)
/***************************************************************************
 *
 */
void BVDC_Dbg_ClearBvnError
    ( BVDC_Handle                  hVdc )
{
    uint32_t i;
    const BVDC_P_IntCbTbl *pIntCb;

    for(i = 0; i < BVDC_BvnError_eMaxCount; i++)
    {
        pIntCb = BVDC_P_GetBvnErrorCb(i);
        if(pIntCb == NULL)
            continue;

        BDBG_MSG(("Clear IntId %s(%d) L2Reg=0x%x Mask=0x%x",
            pIntCb->pchInterruptName, i, pIntCb->ulL2ClearReg, pIntCb->ulL2ClearMask));
        BREG_Write32(hVdc->hRegister, pIntCb->ulL2ClearReg, pIntCb->ulL2ClearMask);

        /* Clear interrupt count */
        hVdc->aulBvnErrCnt[i] = 0;
    }

    return;
}


/***************************************************************************
 *
 */
void BVDC_Dbg_GetBvnErrorStatus
    ( BVDC_Handle                  hVdc )
{
    uint32_t i;
    const BVDC_P_IntCbTbl *pIntCb;

    for(i = 0; i < BVDC_BvnError_eMaxCount; i++)
    {
        if(hVdc->aulBvnErrCnt[i] != 0)
        {
            pIntCb = BVDC_P_GetBvnErrorCb(i);
            if(pIntCb != NULL)
            {
                BDBG_ERR(("%s(%d): %d", pIntCb->pchInterruptName, i, hVdc->aulBvnErrCnt[i]));
            }
            else
            {
                BDBG_ASSERT(pIntCb != NULL);
            }
        }
    }
    return;
}

/***************************************************************************
 *
 */
void BVDC_Dbg_GetBvnErrorStatus_isr
    ( BVDC_Handle                  hVdc )
{
    uint32_t i;
    const BVDC_P_IntCbTbl *pIntCb;

    for(i = 0; i < BVDC_BvnError_eMaxCount; i++)
    {
        if(hVdc->aulBvnErrCnt[i] != 0)
        {
            pIntCb = BVDC_P_GetBvnErrorCb_isr(i);
            if(pIntCb != NULL)
            {
                BDBG_ERR(("%s(%d): %d", pIntCb->pchInterruptName, i, hVdc->aulBvnErrCnt[i]));
            }
            else
            {
                BDBG_ASSERT(pIntCb != NULL);
            }
        }
    }
    return;
}
#endif

/***************************************************************************
 *
 */
void BVDC_Dbg_Window_GetDebugStatus
    ( const BVDC_Window_Handle         hWindow,
      BVDC_Window_DebugStatus         *pDbgInfo )
{
    uint32_t ulBvnErrCount = 0;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(BVDC_P_STATE_IS_ACTIVE(hWindow))
    {
        uint32_t i;
        const BVDC_P_IntCbTbl *pIntCb;

        for(i = 0; i < BVDC_BvnError_eMaxCount; i++)
        {
            if(hWindow->hCompositor->hVdc->aulBvnErrCnt[i] != 0)
            {
                pIntCb = BVDC_P_GetBvnErrorCb(i);

                if(pIntCb != NULL)
                {
                    /* Retrieve error from Window */
                    if (((hWindow->eId == BVDC_P_WindowId_eComp0_V0) &&
                         (i == BVDC_BvnError_eCmp_0_V0)) ||
                        ((hWindow->eId == BVDC_P_WindowId_eComp0_V1) &&
                         (i == BVDC_BvnError_eCmp_0_V1)) ||
                        ((hWindow->eId == BVDC_P_WindowId_eComp1_V0) &&
                         (i == BVDC_BvnError_eCmp_1_V0)) ||
                        ((hWindow->eId == BVDC_P_WindowId_eComp1_V1) &&
                         (i == BVDC_BvnError_eCmp_1_V1)) ||
                        ((hWindow->eId == BVDC_P_WindowId_eComp0_G0) &&
                         (i == BVDC_BvnError_eCmp_0_G0)) ||
                        (((hWindow->eId >= BVDC_P_WindowId_eComp2_V0) &&
                          (hWindow->eId <= BVDC_P_WindowId_eComp6_V0)) &&
                         ((hWindow->eId - BVDC_P_WindowId_eComp2_V0) + BVDC_BvnError_eCmp_2_V0 == i)) ||
                        (((hWindow->eId >= BVDC_P_WindowId_eComp1_G0) &&
                          (hWindow->eId <= BVDC_P_WindowId_eComp6_G0)) &&
                         ((hWindow->eId - BVDC_P_WindowId_eComp1_G0) + BVDC_BvnError_eCmp_1_G0 == i)) ||
                        ((hWindow->eId == BVDC_P_WindowId_eComp0_G1) &&
                         (i == BVDC_BvnError_eCmp_0_G1)) ||
                        ((hWindow->eId == BVDC_P_WindowId_eComp0_G2) &&
                         (i == BVDC_BvnError_eCmp_0_G2)))
                    {
                        ulBvnErrCount += hWindow->hCompositor->hVdc->aulBvnErrCnt[i];
                    }

                    /* Retrieve error count only from the BVN block i associated with the display */
                    if ((hWindow->stCurResource.hCapture && (hWindow->stCurResource.hCapture->eId + BVDC_BvnError_eCap_0 == i)) ||
                        (hWindow->stCurResource.hPlayback && (hWindow->stCurResource.hPlayback->eId + BVDC_BvnError_eMfd_0 == i)) || /* includes MFD and VFD */
                        (hWindow->stCurResource.hScaler && (hWindow->stCurResource.hScaler->eId + BVDC_BvnError_eScl_0  == i)) ||
                        (hWindow->stCurResource.hDnr && (hWindow->stCurResource.hDnr->eId + BVDC_BvnError_eDnr_0  == i))
#if BVDC_P_SUPPORT_HSCL_VER
                     || (hWindow->stCurResource.hHscaler && (hWindow->stCurResource.hHscaler->eId + BVDC_BvnError_eHscl_0 == i))
#endif
                        )
                    {
                        ulBvnErrCount += hWindow->hCompositor->hVdc->aulBvnErrCnt[i];
                    }

                    if (hWindow->stCurResource.hMcvp)
                    {
                        if ((hWindow->stCurResource.hMcvp->eId + BVDC_BvnError_eMvp_0 == i) ||
                            (hWindow->stCurResource.hMcvp->hMcdi && (hWindow->stCurResource.hMcvp->hMcdi->eId + BVDC_BvnError_eMcdi_0 == i)) ||
                            (hWindow->stCurResource.hMcvp->hAnr && (hWindow->stCurResource.hMcvp->hAnr->eId + BVDC_BvnError_eMctf_0 == i)))
                        {
                            ulBvnErrCount += hWindow->hCompositor->hVdc->aulBvnErrCnt[i];
                        }
                    }
                }
                else
                {
                    BDBG_ASSERT(pIntCb != NULL);
                }
            }
        }
        pDbgInfo->ulNumErr = ulBvnErrCount;
    }

    {
        BVDC_Display_Handle hDisplay = hWindow->hCompositor->hDisplay;
        uint32_t ulLineNumber;

        if (!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg))
        {
            BVDC_P_Display_GetRasterLineNumber(hDisplay, &ulLineNumber);
            pDbgInfo->ulLineNumber = ulLineNumber;
        }
        else
        {
            pDbgInfo->ulLineNumber = BVDC_DBG_NOT_APPLICABLE;
        }
        pDbgInfo->ulVsyncCnt = hDisplay->ulVsyncCnt;
    }

    return;
}

/***************************************************************************
 *
 */
void BVDC_Dbg_Source_GetDebugStatus
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_DebugStatus         *pDbgInfo )
{
    uint32_t ulBvnErrCount = 0;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if(BVDC_P_STATE_IS_ACTIVE(hSource))
    {
        uint32_t i;
        const BVDC_P_IntCbTbl *pIntCb;

        for(i = 0; i < BVDC_BvnError_eMaxCount; i++)
        {
            if(hSource->hVdc->aulBvnErrCnt[i] != 0)
            {
                pIntCb = BVDC_P_GetBvnErrorCb(i);

                if(pIntCb != NULL)
                {
                    /* Retrieve error from Source */
                    if (((hSource->eId - BAVC_SourceId_eMpeg0) + BVDC_BvnError_eMfd_0) == i ||
                        ((hSource->eId - BAVC_SourceId_eVfd0)  + BVDC_BvnError_eVfd_0) == i ||
                        ((hSource->eId - BAVC_SourceId_eGfx0)  + BVDC_BvnError_eGfd_0) == i)
                    {
                        ulBvnErrCount += hSource->hVdc->aulBvnErrCnt[i];
                    }
                }
                else
                {
                    BDBG_ASSERT(pIntCb != NULL);
                }
            }
        }
        pDbgInfo->ulNumErr = ulBvnErrCount;
    }

    return;
}

#if !defined(B_REFSW_MINIMAL)
/***************************************************************************
 *
 */
BERR_Code BVDC_Dbg_MaskBvnErrorCb
    ( BVDC_Handle                  hVdc,
      BVDC_BvnModule               eBvnModule,
      uint32_t                     ulModuleIdx,
      bool                         bMaskOff )
{
    BVDC_BvnError       eBvnErrModule;
    BERR_Code eStatus = BERR_SUCCESS;

    if(BVDC_Bvn_eMaxCount <= eBvnModule)
    {
        eStatus = BERR_INVALID_PARAMETER;
        return (eStatus);
    }

    switch (eBvnModule)
    {
        case BVDC_Bvn_eRdc:
            eBvnErrModule = BVDC_BvnError_eRdc;
            BSTD_UNUSED(ulModuleIdx);
            break;

        case BVDC_Bvn_eMfd:
            if(ulModuleIdx >=(BVDC_BvnError_eVfd_0 - BVDC_BvnError_eMfd_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eMfd_0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eVfd:
            if(ulModuleIdx >=(BVDC_BvnError_eScl_0 - BVDC_BvnError_eVfd_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eVfd_0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eScl:
            if(ulModuleIdx >=(BVDC_BvnError_eDnr_0 - BVDC_BvnError_eScl_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eScl_0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eDnr:
            if(ulModuleIdx >=(BVDC_BvnError_eXsrc_0 - BVDC_BvnError_eDnr_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eDnr_0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eMvp:
            if(ulModuleIdx >=(BVDC_BvnError_eMcdi_0 - BVDC_BvnError_eMvp_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eMvp_0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eMcdi:
            if(ulModuleIdx >=(BVDC_BvnError_eMctf_0 - BVDC_BvnError_eMcdi_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eMcdi_0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eMctf:
            eBvnErrModule = BVDC_BvnError_eMctf_0;
            BSTD_UNUSED(ulModuleIdx);
            break;

        case BVDC_Bvn_eHscl:
            if(ulModuleIdx >=(BVDC_BvnError_eCap_0 - BVDC_BvnError_eHscl_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eHscl_0 + ulModuleIdx;
            break;
        case BVDC_Bvn_eCap:
            if(ulModuleIdx >=(BVDC_BvnError_eGfd_0 - BVDC_BvnError_eCap_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eCap_0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eGfd:
            if(ulModuleIdx >=(BVDC_BvnError_eCmp_0_V0 - BVDC_BvnError_eGfd_0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eGfd_0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eCmp_V0:
            if(ulModuleIdx >=(BVDC_BvnError_eCmp_0_V1 - BVDC_BvnError_eCmp_0_V0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eCmp_0_V0 + ulModuleIdx;
            break;

        case BVDC_Bvn_eCmp_V1:
            if(ulModuleIdx >=(BVDC_BvnError_eCmp_0_G0 - BVDC_BvnError_eCmp_0_V1))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eCmp_0_V1 + ulModuleIdx;
            break;

        case BVDC_Bvn_eCmp_G0:
            if(ulModuleIdx >=(BVDC_BvnError_eMaxCount - BVDC_BvnError_eCmp_0_G0))
            {
                eStatus = BERR_INVALID_PARAMETER;
                return (eStatus);
            }

            eBvnErrModule = BVDC_BvnError_eCmp_0_G0 + ulModuleIdx;
            break;
        default:
            eStatus = BERR_INVALID_PARAMETER;
            return (eStatus);
    }

    hVdc->abBvnErrMask[eBvnErrModule] = bMaskOff;
    return eStatus;
}


/***************************************************************************
 *
 */
BVDC_Source_Handle BVDC_Dbg_GetSourceHandle
    ( const BVDC_Handle                hVdc,
      BAVC_SourceId                    eSourceId )
{
    BVDC_Source_Handle hSrc = NULL;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    if(BVDC_P_STATE_IS_ACTIVE(hVdc->ahSource[eSourceId]))
    {
        hSrc = hVdc->ahSource[eSourceId];
    }

    return hSrc;
}
#endif


#if !defined(B_REFSW_MINIMAL) || defined(BVDC_SUPPORT_BVN_DEBUG)
/***************************************************************************
 *
 */
BVDC_Compositor_Handle BVDC_Dbg_GetCompositorHandle
    ( const BVDC_Handle                hVdc,
      BVDC_CompositorId                eCompositorId )
{
    BVDC_Compositor_Handle hCmp = NULL;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    if(BVDC_P_STATE_IS_ACTIVE(hVdc->ahCompositor[eCompositorId]))
    {
        hCmp = hVdc->ahCompositor[eCompositorId];
    }

    return hCmp;
}
#endif


#if !defined(B_REFSW_MINIMAL)
/***************************************************************************
 *
 */
BVDC_Window_Handle BVDC_Dbg_GetWindowHandle
    ( const BVDC_Handle                hVdc,
      BVDC_CompositorId                eCompositorId,
      BVDC_WindowId                    eWindowId )
{
    BVDC_Window_Handle hWin = NULL;
    BVDC_Compositor_Handle hCmp = NULL;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* Get the compositor */
    hCmp = BVDC_Dbg_GetCompositorHandle(hVdc, eCompositorId);

    /* Do we have valid compositor */
    if(hCmp)
    {
        BVDC_P_WindowId eWinId;
        BDBG_OBJECT_ASSERT(hCmp, BVDC_CMP);
        switch(eCompositorId)
        {
        case BVDC_CompositorId_eCompositor0:
            switch(eWindowId)
            {
            case BVDC_WindowId_eVideo0:
                eWinId = BVDC_P_WindowId_eComp0_V0;
                break;

            case BVDC_WindowId_eVideo1:
                eWinId = BVDC_P_WindowId_eComp0_V1;
                break;

            case BVDC_WindowId_eGfx0:
                eWinId = BVDC_P_WindowId_eComp0_G0;
                break;

            case BVDC_WindowId_eGfx1:
                eWinId = BVDC_P_WindowId_eComp0_G1;
                break;

            case BVDC_WindowId_eGfx2:
                eWinId = BVDC_P_WindowId_eComp0_G2;
                break;

            default:
                return NULL;
            }
            break;

        case BVDC_CompositorId_eCompositor1:
            switch(eWindowId)
            {
            case BVDC_WindowId_eVideo0:
                eWinId = BVDC_P_WindowId_eComp1_V0;
                break;

            case BVDC_WindowId_eVideo1:
                eWinId = BVDC_P_WindowId_eComp1_V1;
                break;

            case BVDC_WindowId_eGfx0:
                eWinId = BVDC_P_WindowId_eComp1_G0;
                break;

            default:
                return NULL;
            }
            break;

        case BVDC_CompositorId_eCompositor2:
            switch(eWindowId)
            {
            case BVDC_WindowId_eVideo0:
                eWinId = BVDC_P_WindowId_eComp2_V0;
                break;

            case BVDC_WindowId_eGfx0:
                eWinId = BVDC_P_WindowId_eComp2_G0;
                break;

            default:
                return NULL;
            }
            break;

        case BVDC_CompositorId_eCompositor3:
            switch(eWindowId)
            {
            case BVDC_WindowId_eVideo0:
                eWinId = BVDC_P_WindowId_eComp3_V0;
                break;

            case BVDC_WindowId_eGfx0:
                eWinId = BVDC_P_WindowId_eComp3_G0;
                break;

            default:
                return NULL;
            }
            break;

        case BVDC_CompositorId_eCompositor4:
            switch(eWindowId)
            {
            case BVDC_WindowId_eVideo0:
                eWinId = BVDC_P_WindowId_eComp4_V0;
                break;

            case BVDC_WindowId_eGfx0:
                eWinId = BVDC_P_WindowId_eComp4_G0;
                break;

            default:
                return NULL;
            }
            break;

        case BVDC_CompositorId_eCompositor5:
            switch(eWindowId)
            {
            case BVDC_WindowId_eVideo0:
                eWinId = BVDC_P_WindowId_eComp5_V0;
                break;

            case BVDC_WindowId_eGfx0:
                eWinId = BVDC_P_WindowId_eComp5_G0;
                break;

            default:
                return NULL;
            }
            break;

        case BVDC_CompositorId_eCompositor6:
            switch(eWindowId)
            {
            case BVDC_WindowId_eVideo0:
                eWinId = BVDC_P_WindowId_eComp6_V0;
                break;

            case BVDC_WindowId_eGfx0:
                eWinId = BVDC_P_WindowId_eComp6_G0;
                break;

            default:
                return NULL;
            }
            break;

        default:
            return NULL;
        }

        if(BVDC_P_STATE_IS_ACTIVE(hCmp->ahWindow[eWinId]))
        {
            hWin = hCmp->ahWindow[eWinId];
        }
    }

    return hWin;
}


/***************************************************************************
 *
 */
BVDC_Source_Handle BVDC_Dbg_Window_GetSourceHandle
    ( const BVDC_Window_Handle         hWindow )
{
    BVDC_Source_Handle hSrc = NULL;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(BVDC_P_STATE_IS_ACTIVE(hWindow) &&
       BVDC_P_STATE_IS_ACTIVE(hWindow->stCurInfo.hSource))
    {
        hSrc = hWindow->stCurInfo.hSource;
    }

    return hSrc;
}


/***************************************************************************
 *
 */
uint32_t BVDC_Dbg_Window_GetScalerStatus
    ( const BVDC_Window_Handle         hWindow,
      BVDC_Scaler_DebugStatus         *pStatus )
{
    BVDC_P_Scaler_Handle hScaler;
    uint32_t ulReg;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if(BVDC_P_STATE_IS_ACTIVE(hWindow))
    {
        hScaler = hWindow->stCurResource.hScaler;

        ulReg = BREG_Read32(hScaler->hReg, BCHP_SCL_0_BVB_IN_STATUS + hScaler->ulRegOffset);
#ifdef BCHP_SCL_0_BVB_IN_STATUS_ENABLE_ERROR_MASK
        pStatus->bEnableError = (ulReg & BCHP_SCL_0_BVB_IN_STATUS_ENABLE_ERROR_MASK) ? true : false;
#endif
        return ulReg;
    }

    return 0;
}


/***************************************************************************
 *
 */
BVDC_Compositor_Handle BVDC_Dbg_Window_GetCompositorHandle
    ( const BVDC_Window_Handle         hWindow )
{
    BVDC_Compositor_Handle hCmp = NULL;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(BVDC_P_STATE_IS_ACTIVE(hWindow) &&
       BVDC_P_STATE_IS_ACTIVE(hWindow->hCompositor))
    {
        hCmp = hWindow->hCompositor;
    }

    return hCmp;
}


/***************************************************************************
 *
 */
BVDC_Display_Handle BVDC_Dbg_Compositor_GetDisplayHandle
    ( const BVDC_Compositor_Handle     hCompositor )
{
    BVDC_Display_Handle hDis = NULL;

    /* Sanity checks */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    if(BVDC_P_STATE_IS_ACTIVE(hCompositor) &&
       BVDC_P_STATE_IS_ACTIVE(hCompositor->hDisplay))
    {
        hDis = hCompositor->hDisplay;
    }

    return hDis;
}
#endif

#if !defined(B_REFSW_MINIMAL) || defined(BVDC_SUPPORT_BVN_DEBUG)
/***************************************************************************
 *
 */
void BVDC_Dbg_Display_GetVecUcodeInfo
    ( const BVDC_Display_Handle     hDisplay,
      BVDC_Display_UcodeInfo       *pstUcodeInfo )
{
    BREG_Handle hRegister;
    uint32_t ulBase, ulTsIndex, ulCksumIndex;

    /* Sanity check */
    BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
    BDBG_ASSERT(pstUcodeInfo);

    /* Reset */
    pstUcodeInfo->ulAnalogTs = 0;
    pstUcodeInfo->ulAnalogCksum = 0;
    pstUcodeInfo->ulDviTs = 0;
    pstUcodeInfo->ulDviCksum = 0;
    pstUcodeInfo->ul656Ts = 0;
    pstUcodeInfo->ul656Cksum = 0;

    hRegister = hDisplay->hVdc->hRegister;

    if(BVDC_P_STATE_IS_ACTIVE(hDisplay))
    {
        if((hDisplay->bAnlgEnable) ||   /* if analog master */
            (!hDisplay->bAnlgEnable &&  /* or analog slave with DACs */
            (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
        {
            uint32_t ulOffset;
            if (hDisplay->bAnlgEnable)
            {
                ulOffset = hDisplay->stAnlgChan_0.ulItRegOffset;
            }
            else
            {
                ulOffset = (hDisplay->stAnlgChan_1.bEnable) ?
                    hDisplay->stAnlgChan_1.ulItRegOffset : hDisplay->stAnlgChan_0.ulItRegOffset;
            }

            ulBase = BCHP_IT_0_MICRO_INSTRUCTIONi_ARRAY_BASE + ulOffset;
            ulTsIndex = BVDC_P_RAM_TABLE_TIMESTAMP_IDX * sizeof(uint32_t);
            ulCksumIndex = BVDC_P_RAM_TABLE_CHECKSUM_IDX * sizeof(uint32_t);

            pstUcodeInfo->ulAnalogTs = BREG_Read32(hRegister, ulBase + ulTsIndex);
            pstUcodeInfo->ulAnalogCksum = BREG_Read32(hRegister, ulBase + ulCksumIndex);
        }

        if (hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
        {
#if (BVDC_P_ORTHOGONAL_VEC_VER <= BVDC_P_ORTHOGONAL_VEC_VER_1)
            ulBase = BCHP_DTRAM_0_DMC_INSTRUCTIONi_ARRAY_BASE + (BVDC_P_DVI_DTRAM_START_ADDR * sizeof(uint32_t));
#else
            ulBase =  BCHP_DVI_DTG_0_DMC_INSTRUCTIONi_ARRAY_BASE +
                (BVDC_P_DVI_DTRAM_START_ADDR * sizeof(uint32_t)) + hDisplay->stDviChan.ulDviRegOffset;
#endif
            ulTsIndex = BVDC_P_DTRAM_TABLE_TIMESTAMP_IDX * sizeof(uint32_t);
            ulCksumIndex = BVDC_P_DTRAM_TABLE_CHECKSUM_IDX * sizeof(uint32_t);

            pstUcodeInfo->ulDviTs = BREG_Read32(hRegister, ulBase + ulTsIndex);
            pstUcodeInfo->ulDviCksum = BREG_Read32(hRegister, ulBase + ulCksumIndex);
        }

#if (BCHP_ITU656_0_REG_START)
        if (hDisplay->st656Chan.bEnable)
        {
#if (BVDC_P_ORTHOGONAL_VEC_VER <= BVDC_P_ORTHOGONAL_VEC_VER_1)
            ulBase = BCHP_DTRAM_0_DMC_INSTRUCTIONi_ARRAY_BASE;
#else
            ulBase = BCHP_ITU656_DTG_0_DMC_INSTRUCTIONi_ARRAY_BASE;
#endif

            ulTsIndex = BVDC_P_DTRAM_TABLE_TIMESTAMP_IDX * sizeof(uint32_t);
            ulCksumIndex = BVDC_P_DTRAM_TABLE_CHECKSUM_IDX * sizeof(uint32_t);

            pstUcodeInfo->ul656Ts = BREG_Read32(hRegister, ulBase + ulTsIndex);
            pstUcodeInfo->ul656Cksum = BREG_Read32(hRegister, ulBase + ulCksumIndex);
        }
#endif
     }
}
#endif

#if (BVDC_BUF_LOG == 1)
/***************************************************************************
 * BVDC_SetBufLogStateAndDumpTrigger
 *
 * Set when to start logging multi-buffering events and how to notify user
 * the log can be dumped.
 */
BERR_Code BVDC_SetBufLogStateAndDumpTrigger
    ( BVDC_BufLogState                 eLogState,
      const BVDC_CallbackFunc_isr      pfCallback,
      void                             *pvParm1,
      int                              iParm2 )
{
    if (eLogState > BVDC_BufLogState_eAutomaticReduced)
    {
        BDBG_ERR(("Invalid log state %d ", (int) eLogState));
        return BERR_INVALID_PARAMETER;
    }

    if (((BVDC_BufLogState_eAutomatic == eLogState) || (BVDC_BufLogState_eAutomaticReduced == eLogState))
        && (NULL == pfCallback))
    {
        BDBG_ERR(("Callback function must be registered for eAutomatic or eAutomaticReduced mode"));
        return BERR_INVALID_PARAMETER;
    }

    BVDC_P_Buffer_SetLogStateAndDumpTrigger(eLogState, pfCallback, pvParm1, iParm2);

    return BERR_SUCCESS;
}

/***************************************************************************
 * BVDC_DumpBufLog
 *
 * Print out the captured multi-buffering events log.
 *
 */
void BVDC_DumpBufLog
    ( char                  *pLog,
      unsigned int           uiSizeToRead,
      unsigned int          *puiReadCount )
{
    BVDC_P_Buffer_DumpLog(pLog, uiSizeToRead, puiReadCount);
}

/***************************************************************************
 * BVDC_SetupManualTrigger
 *
 * Prepares the manual trigger of the multi-buffering event log
 *
 */
void BVDC_SetBufLogManualTrigger(void)
{
    BVDC_P_Buffer_SetManualTrigger();
}

BERR_Code BVDC_Window_EnableBufLog
    ( const BVDC_Window_Handle         hWindow,
      bool                             bEnable )
{
    if(!BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BVDC_P_Buffer_EnableBufLog(hWindow->eId, bEnable);
    return BERR_SUCCESS;
}
#endif


/* end of file */
