/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
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

	/* Convert base on current colorspace output */
	if((BVDC_P_CmpColorSpace_eHdYCrCb != hCompositor->eCmpColorSpace) &&
	   (BVDC_P_CmpColorSpace_eSmpte_240M != hCompositor->eCmpColorSpace))
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
#if BVDC_P_ORTHOGONAL_VEC
	/* Trigger HDMI sync only and mute dirty bits */
		if(hCompositor->hDisplay->stNewInfo.bEnableHdmi)
		{
			hCompositor->hDisplay->stNewInfo.stDirty.stBits.bHdmiSyncOnly = BVDC_P_DIRTY;
		}
#endif
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

/***************************************************************************
*
**************************************************************************/
BERR_Code BVDC_Compositor_SetLumaStatsConfiguration
	( BVDC_Compositor_Handle           hCompositor,
	  const BVDC_LumaSettings         *pLumaSettings )
{
	BERR_Code             err = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Compositor_SetLumaStatsConfiguration);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	/* TODO: support LUMA AVG for CMP_1 */

	if(hCompositor->eId == BVDC_CompositorId_eCompositor0)
	{
		if(pLumaSettings)
		{
			hCompositor->stNewInfo.stLumaRect = *pLumaSettings;
			hCompositor->stNewInfo.bLumaRectUserSet = true;
		}
		else
		{
			hCompositor->stNewInfo.bLumaRectUserSet = false;
		}
	}
	else
	{
		err = BERR_TRACE(BERR_NOT_SUPPORTED);
		BDBG_ERR(("Luma Rect set is not supported for compositor[%d]", hCompositor->eId));
	}

	BDBG_LEAVE(BVDC_Compositor_SetLumaStatsConfiguration);
	return err;
}


/***************************************************************************
*
**************************************************************************/
BERR_Code BVDC_Compositor_GetLumaStatsConfiguration
	( const BVDC_Compositor_Handle     hCompositor,
	  BVDC_LumaSettings               *pLumaSettings )
{
	BDBG_ENTER(BVDC_Compositor_GetLumaStatsConfiguration);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	if(pLumaSettings)
	{
		*pLumaSettings = hCompositor->stCurInfo.stLumaRect;
	}

	BDBG_LEAVE(BVDC_Compositor_GetLumaStatsConfiguration);
	return BERR_SUCCESS;
}

/***************************************************************************
*
**************************************************************************/
BERR_Code BVDC_Compositor_GetLumaStatus
	( const BVDC_Compositor_Handle     hCompositor,
	  BVDC_LumaStatus                 *pLumaStatus )
{
	BDBG_ENTER(BVDC_Compositor_GetLumaStatus);

	/* TODO: support LUMA AVG for CMP_1 */

	/* check parameters */
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	if(hCompositor->eId != BVDC_CompositorId_eCompositor0)
	{
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	/* set return value */
	if(pLumaStatus)
	{
#if (BVDC_P_SUPPORT_CMP_LUMA_AVG_VER > 0)
		uint32_t i;
		uint32_t ulReg;
		BREG_Handle hReg = hCompositor->hVdc->hRegister;

		/* Read luma suma from hw and return it to user. */
		ulReg = BREG_Read32(hReg, BCHP_CMP_0_CMP_LUMA_SUM);
		pLumaStatus->ulSum =
			BCHP_GET_FIELD_DATA(ulReg, CMP_0_CMP_LUMA_SUM, LUMA_SUM);

		ulReg = BREG_Read32(hReg, BCHP_CMP_0_CMP_PIXEL_COUNT);
		pLumaStatus->ulPixelCnt =
			BCHP_GET_FIELD_DATA(ulReg, CMP_0_CMP_PIXEL_COUNT, PIXEL_COUNT);

		/* Compositor does not min/max/histogram. */
		pLumaStatus->ulMax = 0;
		pLumaStatus->ulMin = 0;
		for(i = 0; i < BVDC_LUMA_HISTOGRAM_COUNT; i++)
		{
			pLumaStatus->aulHistogram[i] = 0;
		}
#endif
	}

	BDBG_LEAVE(BVDC_Compositor_GetLumaStatus);
	return BERR_SUCCESS;
}


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
