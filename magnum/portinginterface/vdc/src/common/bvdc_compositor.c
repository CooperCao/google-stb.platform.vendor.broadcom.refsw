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
#include "bstd.h"                     /* standard types */
#include "bpxl.h"                     /* Pixel format */
#include "bkni.h"                     /* For malloc */
#include "bvdc.h"                     /* Video display */
#include "bvdc_compositor_priv.h"
#include "bvdc_priv.h"


BDBG_MODULE(BVDC_CMP);


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Compositor_GetDefaultSettings
    ( BVDC_CompositorId                eCompositorId,
      BVDC_Compositor_Settings        *pDefSettings )
{
    BSTD_UNUSED(eCompositorId);
    BSTD_UNUSED(pDefSettings);

    return BERR_SUCCESS;
}
#endif

/***************************************************************************
 * Create a compositor.
 *
 * If eCompositorId is not create this function will create an compositor
 * handle and pass it back to user.  It also keep track of it in hVdc.
 */
BERR_Code BVDC_Compositor_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Compositor_Handle          *phCompositor,
      BVDC_CompositorId                eCompositorId,
      const BVDC_Compositor_Settings  *pDefSettings )
{
    BDBG_ENTER(BVDC_Compositor_Create);
    BSTD_UNUSED(pDefSettings);
    BDBG_ASSERT(phCompositor);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    if(!hVdc->pFeatures->abAvailCmp[eCompositorId])
    {
        BDBG_ERR(("Compositor[%d] not supported on this chipset.", eCompositorId));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if(hVdc->ahCompositor[eCompositorId]!=NULL)
    {
        /* Reinitialize context.  But not make it active until apply. */
        *phCompositor = hVdc->ahCompositor[eCompositorId];
        BVDC_P_Compositor_Init(*phCompositor);

        /* Mark as create, awating for apply. */
        hVdc->ahCompositor[eCompositorId]->eState = BVDC_P_State_eCreate;
        BDBG_MSG(("cmp[%d] has been created", eCompositorId));
        BDBG_LEAVE(BVDC_Compositor_Create);
        return BERR_SUCCESS;
    }

    BVDC_P_Compositor_Create(hVdc, &hVdc->ahCompositor[eCompositorId], (BVDC_CompositorId)eCompositorId);

    BDBG_OBJECT_ASSERT(hVdc->ahCompositor[eCompositorId], BVDC_CMP);
    /* Check if compositor is created or not. */
    if(BVDC_P_STATE_IS_ACTIVE(hVdc->ahCompositor[eCompositorId]) ||
       BVDC_P_STATE_IS_CREATE(hVdc->ahCompositor[eCompositorId]) ||
       BVDC_P_STATE_IS_DESTROY(hVdc->ahCompositor[eCompositorId]))
    {
        return BERR_TRACE(BVDC_ERR_COMPOSITOR_ALREADY_CREATED);
    }

    BDBG_MSG(("Creating compositor%d", hVdc->ahCompositor[eCompositorId]->eId));
    BDBG_ASSERT(BVDC_P_STATE_IS_INACTIVE(hVdc->ahCompositor[eCompositorId]));

    /* Reinitialize context.  But not make it active until apply. */
    *phCompositor = hVdc->ahCompositor[eCompositorId];
    BVDC_P_Compositor_Init(*phCompositor);

    /* Mark as create, awating for apply. */
    hVdc->ahCompositor[eCompositorId]->eState = BVDC_P_State_eCreate;

    BDBG_LEAVE(BVDC_Compositor_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Compositor_Destroy
    ( BVDC_Compositor_Handle           hCompositor )
{
    uint32_t i;

    BDBG_ENTER(BVDC_Compositor_Destroy);

#if defined(BVDC_GFX_PERSIST)
    goto done;
#endif

    /* Return if trying to free a NULL handle. */
    if(!hCompositor)
    {
        goto done;
    }

    /* check parameters */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* Check to see if there are any active windows. */
    for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hCompositor->ahWindow[i]))
        {
            BDBG_ERR(("Active window %d has not been released.",
                      hCompositor->ahWindow[i]->eId));
            BDBG_ERR(("The window state is %d.",
                      hCompositor->ahWindow[i]->eState));
            return BERR_TRACE(BERR_LEAKED_RESOURCE);
        }
    }

    if(BVDC_P_STATE_IS_DESTROY(hCompositor) ||
       BVDC_P_STATE_IS_INACTIVE(hCompositor))
    {
        goto done;
    }

    if(BVDC_P_STATE_IS_ACTIVE(hCompositor))
    {
        hCompositor->eState = BVDC_P_State_eDestroy;
    }

    if(BVDC_P_STATE_IS_CREATE(hCompositor))
    {
        hCompositor->eState = BVDC_P_State_eInactive;
    }

done:
    BDBG_LEAVE(BVDC_Compositor_Destroy);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Compositor_GetMaxWindowCount
    ( const BVDC_Compositor_Handle     hCompositor,
      uint32_t                        *pulVideoWindowCount,
      uint32_t                        *pulGfxWindowCount )
{
    BDBG_ENTER(BVDC_Compositor_GetMaxWindowCount);
    /* check parameters */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* set return value */
    if(pulVideoWindowCount)
    {
        *pulVideoWindowCount = hCompositor->pFeatures->ulMaxVideoWindow;
    }

    if(pulGfxWindowCount)
    {
        *pulGfxWindowCount = hCompositor->pFeatures->ulMaxGfxWindow;
    }

    BDBG_LEAVE(BVDC_Compositor_GetMaxWindowCount);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Compositor_SetBackgroundColor
    ( BVDC_Compositor_Handle           hCompositor,
      uint8_t                          ucRed,
      uint8_t                          ucGreen,
      uint8_t                          ucBlue )
{
    unsigned int uiARGB;

    BDBG_ENTER(BVDC_Compositor_SetBackgroundColor);
    /* check parameters */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* set new value */
    hCompositor->stNewInfo.ucRed   = ucRed;
    hCompositor->stNewInfo.ucGreen = ucGreen;
    hCompositor->stNewInfo.ucBlue  = ucBlue;
    uiARGB = BPXL_MAKE_PIXEL(BPXL_eA8_R8_G8_B8,
        0x00, ucRed, ucGreen, ucBlue);

    /* Convert base on current output matrixCoeffs
     * TODO: add covert for BT2020 */
    if((BVDC_P_MatrixCoeffs_eBt2020_NCL != hCompositor->eOutMatrixCoeffs) &&
       (BVDC_P_MatrixCoeffs_eBt2020_CL != hCompositor->eOutMatrixCoeffs) &&
       (BVDC_P_MatrixCoeffs_eBt709 != hCompositor->eOutMatrixCoeffs) &&
       (BVDC_P_MatrixCoeffs_eSmpte240M != hCompositor->eOutMatrixCoeffs))
    {
        BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8,
            uiARGB, (unsigned int*)&hCompositor->stNewInfo.ulBgColorYCrCb);
    }
    else
    {
        BPXL_ConvertPixel_RGBtoHdYCbCr_isr(
            BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8,
            uiARGB, (unsigned int*)&hCompositor->stNewInfo.ulBgColorYCrCb);
    }

    if(hCompositor->hDisplay)
    {
    /* Trigger HDMI sync only and mute dirty bits */
        if(hCompositor->hDisplay->stNewInfo.bEnableHdmi)
        {
            hCompositor->hDisplay->stNewInfo.stDirty.stBits.bHdmiSyncOnly = BVDC_P_DIRTY;
        }
        hCompositor->hDisplay->stNewInfo.stDirty.stBits.bOutputMute = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_Compositor_SetBackgroundColor);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Compositor_GetBackgroundColor
    ( const BVDC_Compositor_Handle     hCompositor,
      uint8_t                         *pucRed,
      uint8_t                         *pucGreen,
      uint8_t                         *pucBlue )
{
    BDBG_ENTER(BVDC_Compositor_GetBackgroundColor);
    /* check parameters */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* set return value */
    if(pucRed)
    {
        *pucRed   = hCompositor->stCurInfo.ucRed;
    }

    if(pucGreen)
    {
        *pucGreen = hCompositor->stCurInfo.ucGreen;
    }

    if(pucBlue)
    {
        *pucBlue  = hCompositor->stCurInfo.ucBlue;
    }

    BDBG_LEAVE(BVDC_Compositor_GetBackgroundColor);
    return BERR_SUCCESS;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
**************************************************************************/
BERR_Code BVDC_Compositor_SetOsdConfiguration
    ( BVDC_Compositor_Handle     hCompositor,
      const BVDC_OsdSettings     *pOsdSettings )
{
    BSTD_UNUSED(hCompositor);
    BSTD_UNUSED(pOsdSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
**************************************************************************/
BERR_Code BVDC_Compositor_GetOsdConfiguration
    ( BVDC_Compositor_Handle     hCompositor,
      BVDC_OsdSettings           *pOsdSettings )
{
    BSTD_UNUSED(hCompositor);
    BSTD_UNUSED(pOsdSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif


/***************************************************************************
**************************************************************************/
BERR_Code BVDC_Compositor_SetColorClip
    ( BVDC_Compositor_Handle          hCompositor,
      const BVDC_ColorClipSettings   *pstColorClipSettings )
{
    BDBG_ENTER(BVDC_Compositor_SetColorClip);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* only support compositor 0 */
    if(hCompositor->eId != BVDC_CompositorId_eCompositor0)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pstColorClipSettings->eColorClipMode > BVDC_ColorClipMode_Both)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hCompositor->stNewInfo.stColorClipSettings = *pstColorClipSettings;

    hCompositor->stNewInfo.stDirty.stBits.bColorClip = BVDC_P_DIRTY;

    BDBG_LEAVE(BVDC_Compositor_SetColorClip);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
**************************************************************************/
BERR_Code BVDC_Compositor_GetColorClip
    ( BVDC_Compositor_Handle          hCompositor,
      BVDC_ColorClipSettings         *pstColorClipSettings )
{
    BDBG_ENTER(BVDC_Compositor_GetColorClip);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    if(pstColorClipSettings)
    {
        *pstColorClipSettings = hCompositor->stCurInfo.stColorClipSettings;
    }

    BDBG_LEAVE(BVDC_Compositor_GetColorClip);
    return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Compositor_GetCapabilities
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_Compositor_Capabilities    *pCapabilities )
{
    BSTD_UNUSED(hCompositor);
    BSTD_UNUSED(pCapabilities);

    return BERR_SUCCESS;
}
#endif

/* End of File */
