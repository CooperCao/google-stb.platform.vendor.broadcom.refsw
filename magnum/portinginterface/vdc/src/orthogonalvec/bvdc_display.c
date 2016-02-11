/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
 *  Main module for BVDC_Display_ functions.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"          /* standard types */
#include "bkni.h"          /* For malloc */

#include "bvdc.h"
#include "bvdc_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayvip_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_feeder_priv.h"

BDBG_MODULE(BVDC_DISP);

/*************************************************************************
 *	BVDC_Display_GetDefaultSettings
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetDefaultSettings
	( BVDC_DisplayId                   eDisplayId,
	  BVDC_Display_Settings           *pDefSettings )
{
	/* Clear out first */
	BKNI_Memset(pDefSettings, 0, sizeof(BVDC_Display_Settings));

	if(BVDC_DisplayId_eAuto == eDisplayId)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* default to analog master */
	pDefSettings->eMasterTg = (BVDC_DisplayTg)(eDisplayId % BVDC_DisplayTg_eTertIt);

	/* Heap NULL heap means using default main-heap.  If user would like
	 * to use a separate heap then this field should set accordingly */
	pDefSettings->hHeap = NULL;

	return BERR_SUCCESS;
}

/*************************************************************************
 *	BVDC_Display_Create
 *
 *	Creates a display handle. Assumes that it has not been created yet.
 *	Unlike the compositor handle, we do not know which display it is
 *	associated with until the user selects the video format.
 *
 *************************************************************************/
BERR_Code BVDC_Display_Create
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_Display_Handle             *phDisplay,
	  BVDC_DisplayId                   eDisplayId,
	  const BVDC_Display_Settings     *pDefSettings )
{
	BVDC_Display_Handle hDisplay;
	BVDC_DisplayId eId;
	uint32_t ulSrcId;
	BERR_Code err;

	BDBG_ENTER(BVDC_Display_Create);
	BDBG_ASSERT(phDisplay);
	BSTD_UNUSED(pDefSettings);

	/* Check internally if we created. */
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
	BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

	/* determine display ID */
	if(BVDC_DisplayId_eAuto == eDisplayId)
	{
		eId = (BVDC_DisplayId)hCompositor->eId;
	}
	else
	{
		eId = eDisplayId;
	}

	if(NULL == hCompositor->hVdc->ahDisplay[eId])
		BVDC_P_Display_Create(hCompositor->hVdc, &hCompositor->hVdc->ahDisplay[eId], eId);
	hDisplay = hCompositor->hVdc->ahDisplay[eId];
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Check if display is created or not. */
	if(BVDC_P_STATE_IS_ACTIVE(hDisplay) ||
	   BVDC_P_STATE_IS_CREATE(hDisplay) ||
	   BVDC_P_STATE_IS_DESTROY(hDisplay))
	{
		return BERR_TRACE(BVDC_ERR_DISPLAY_ALREADY_CREATED);
	}

	BDBG_MSG(("Creating Display[%d]", hDisplay->eId));
	BDBG_ASSERT(BVDC_P_STATE_IS_INACTIVE(hDisplay));

#if (BVDC_P_SUPPORT_STG)
	BVDC_P_ResetStgChanInfo(&hDisplay->stStgChan);
#endif
	BVDC_P_ResetDviChanInfo(&hDisplay->stDviChan);
	BVDC_P_Reset656ChanInfo(&hDisplay->st656Chan);
	BVDC_P_ResetAnalogChanInfo(&hDisplay->stAnlgChan_0);
	BVDC_P_ResetAnalogChanInfo(&hDisplay->stAnlgChan_1);
	hDisplay->bAnlgEnable = false;
	hDisplay->stDviChan.bEnable = false;

	/* determine display channel and the master timing generator */
	if(pDefSettings)
	{
		/* 7420 has no restriction on which can be the master. Any of
		 * IT_0, IT_2, IT3_3, DVI_DTG, and 656_DTG can be the master.
		 */
		BDBG_ASSERT(pDefSettings->eMasterTg < BVDC_DisplayTg_eUnknown);

#if !BVDC_P_SUPPORT_VEC_MUXING
		if(!hDisplay->bIsBypass &&
		   ((BVDC_DisplayTg)eId != pDefSettings->eMasterTg))
		{
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
#endif
		hDisplay->eMasterTg = pDefSettings->eMasterTg;

		/* Select specific flavor of 480P, output and sync modification */
		hDisplay->bArib480p     = pDefSettings->bArib480p;
		hDisplay->bModifiedSync = pDefSettings->bModifiedSync;
	}
	else
	{
		/* In case of auto, use one of the 3 analog master trigger */
		hDisplay->eMasterTg = (BVDC_DisplayTg)(eId % BVDC_DisplayTg_eTertIt);
		hDisplay->bArib480p = false;
	}

	switch(hDisplay->eMasterTg)
	{
		case BVDC_DisplayTg_eDviDtg:
			hDisplay->stDviChan.bEnable= true;
			break;

#if BVDC_P_SUPPORT_ITU656_OUT
		case BVDC_DisplayTg_e656Dtg:
			hDisplay->st656Chan.bEnable = true;
			break;
#endif

		case BVDC_DisplayTg_ePrimIt:
		case BVDC_DisplayTg_eSecIt:
		case BVDC_DisplayTg_eTertIt:
			hDisplay->bAnlgEnable = true;
			break;

#if (BVDC_P_SUPPORT_STG)
		case BVDC_DisplayTg_eStg0:
		case BVDC_DisplayTg_eStg1:
		case BVDC_DisplayTg_eStg2:
		case BVDC_DisplayTg_eStg3:
		case BVDC_DisplayTg_eStg4:
		case BVDC_DisplayTg_eStg5:

			if((hDisplay->eMasterTg - BVDC_DisplayTg_eStg0) >= BVDC_P_NUM_SHARED_STG)
			{
				BDBG_ERR(("System not supported STG_%d, %d available",
					hDisplay->eMasterTg - BVDC_DisplayTg_eStg0, BVDC_P_NUM_SHARED_STG));
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}

			hDisplay->stStgChan.bEnable = true;
			/* master mode STG id is assigned */
			hDisplay->stStgChan.ulStg   = hDisplay->eMasterTg - BVDC_DisplayTg_eStg0;
			break;
#endif
		default:
			break;
	}

	/* Allocate modules for each enabled channel */

	/* Prepare source (compositor) ID */
	ulSrcId = (uint32_t) hCompositor->eId;

	/* determine bHdCap and bSecamCap flag here */
	hDisplay->bHdCap = (ulSrcId == BVDC_CompositorId_eCompositor0) ? true : false;
	hDisplay->bSecamCap = ((BVDC_P_NUM_SHARED_SECAM == 0) || (BVDC_P_NUM_SHARED_SECAM < BVDC_P_MAX_DISPLAY_COUNT && hDisplay->bHdCap)) ? false: true;

	if (hDisplay->stDviChan.bEnable)
	{
		BDBG_MSG(("BVDC_Display_Create Display[%d] allocates resource for DVI", hDisplay->eId));
		/* TODO: Need to add support for dual HDMI master mode */
		BKNI_EnterCriticalSection();
		err = BVDC_P_AllocDviChanResources_isr(hCompositor->hVdc->hResource, hCompositor->hVdc->hRegister, eId,
			BVDC_Hdmi_0, &hDisplay->stDviChan, ulSrcId);
		BKNI_LeaveCriticalSection();

		if (err)
		{
			BDBG_ERR(("Failed to create Display[%d] for HDMI output. Short of DVI block. Check hardware capability.", eId));
			return err;
		}

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0
		switch(hDisplay->stDviChan.ulDvi)
		{
		case 0:
			hDisplay->ulHdmiPwrId = BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0;
			break;
#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY1
		case 1:
			hDisplay->ulHdmiPwrId = BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY1;
			break;
#endif
		default:
			BDBG_ERR(("Unsupported PWR HDMI %d", hDisplay->stDviChan.ulDvi));
			BDBG_ASSERT(0);
		}

		/* HDMI master mode, acquire PWR */
		if(hDisplay->ulHdmiPwrAcquire == 0)
		{
			BDBG_MSG(("HDMI master mode: Acquire BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY"));
			BCHP_PWR_AcquireResource(hCompositor->hVdc->hChip, hDisplay->ulHdmiPwrId);
			hDisplay->ulHdmiPwrAcquire++;
		}
#endif
	}

#if BVDC_P_SUPPORT_ITU656_OUT
	if (hDisplay->st656Chan.bEnable)
	{
		BDBG_MSG(("BVDC_Display_Create Display[%d] allocates resource for 656", hDisplay->eId));
		BKNI_EnterCriticalSection();
		err = BVDC_P_Alloc656ChanResources_isr(hCompositor->hVdc->hResource, eId, &hDisplay->st656Chan, ulSrcId);
		BKNI_LeaveCriticalSection();

		if (err)
		{
			BDBG_ERR(("Failed to create Display[%d] for 656 output. Short of 656 block. Check hardware capability.", eId));
			return err;
		}
#ifdef BCHP_PWR_RESOURCE_VDC_656_OUT
		if(hDisplay->ul656PwrAcquire == 0)
		{
			BDBG_MSG(("656 master mode: Acquire BCHP_PWR_RESOURCE_VDC_656_OUT"));
			BCHP_PWR_AcquireResource(hCompositor->hVdc->hChip, BCHP_PWR_RESOURCE_VDC_656_OUT);
			hDisplay->ul656PwrAcquire++;
		}
#endif
	}
#endif

#if (BVDC_P_SUPPORT_STG)
	if(hDisplay->stStgChan.bEnable)
	{
		BDBG_MSG(("BVDC_Display_Create Display[%d] allocates resource for Stg", hDisplay->eId));
		BKNI_EnterCriticalSection();
		err = BVDC_P_AllocStgChanResources_isr(hCompositor->hVdc->hResource, hDisplay);
		BKNI_LeaveCriticalSection();
		if (err)
		{
			BDBG_ERR(("Failed to create Display[%d] for STG output. Short of STG block. Check hardware capability.", eId));
			return err;
		}
#if BVDC_P_SUPPORT_VIP
		BDBG_MSG(("new Vip heap=%p, curHeap=%p, hVip=%p", (void *)hDisplay->stNewInfo.hVipHeap, (void *)hDisplay->stCurInfo.hVipHeap, (void *)hDisplay->hVip));
		if (hDisplay->stNewInfo.hVipHeap && !hDisplay->stCurInfo.hVipHeap && !hDisplay->hVip) {
			BDBG_MSG(("To allocate VIP resource..."));
			BVDC_P_Vip_AllocBuffer(hDisplay->hVdc->ahVip[hDisplay->stStgChan.ulStg], hDisplay);
			BVDC_P_Vip_Init(hDisplay->hVdc->ahVip[hDisplay->stStgChan.ulStg]);
		}
		/* Note, when disabled, VIP will be freed after ApplyChanges and isr programs RUL to put VIP back to auto drain mode */
#endif

#ifdef BCHP_PWR_RESOURCE_VDC_STG0
		BDBG_MSG(("STG master mode: Acquire BCHP_PWR_RESOURCE_VDC_STG %d", hDisplay->stStgChan.ulStg));
		BVDC_P_AcquireStgPwr(hDisplay);
#endif
	}
#endif

	if(hDisplay->bAnlgEnable)
	{
		BDBG_MSG(("BVDC_Display_Create Display[%d] allocates resource for AnlgChan_0", hDisplay->eId));
		BKNI_EnterCriticalSection();
		err = BVDC_P_AllocITResources(hCompositor->hVdc->hResource, eId * 2, &hDisplay->stAnlgChan_0, BVDC_P_HW_ID_INVALID);
		BKNI_LeaveCriticalSection();

		if(err)
		{
			BDBG_ERR(("Failed to create Display[%d] for %s output. Short of VEC hardware block. Check hardware capability.",
				      eId, hDisplay->bHdCap ? "component/VGA" : (hDisplay->bSecamCap ? "CVBS/S-Video/RF/SECAM" : "CVBS/S-Video/RF")));
			return err;
		}
	}

	/* Which heap to use? */
	hDisplay->hHeap = ((pDefSettings) && (pDefSettings->hHeap))
		? pDefSettings->hHeap : hCompositor->hVdc->hBufferHeap;

	/* link compsoitor and display */
	hDisplay->hCompositor = hCompositor;
	hCompositor->hDisplay = (BVDC_Display_Handle)hDisplay;

	/* Reinitialize context.  But not make it active until apply. */
	BVDC_P_Display_Init(hDisplay);

	/* Mark as create, awating for apply. */
	hDisplay->eState = BVDC_P_State_eCreate;
	*phDisplay = hDisplay;

	BDBG_LEAVE(BVDC_Display_Create);
	return BERR_SUCCESS;
}

/*************************************************************************
 *	BVDC_Display_Destroy
 *************************************************************************/
BERR_Code BVDC_Display_Destroy
	( BVDC_Display_Handle              hDisplay )
{

	BDBG_ENTER(BVDC_Display_Destroy);

	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

#if defined(BVDC_GFX_PERSIST)
	/* disable call back */
	hDisplay->stNewInfo.pfGenericCallback = NULL;
	hDisplay->stCurInfo.pfGenericCallback = NULL;
	goto done;
#endif

	if(BVDC_P_STATE_IS_DESTROY(hDisplay) ||
	   BVDC_P_STATE_IS_INACTIVE(hDisplay))
	{
		goto done;
	}

	if(BVDC_P_STATE_IS_ACTIVE(hDisplay))
	{
		hDisplay->eState = BVDC_P_State_eDestroy;
#if BVDC_P_SUPPORT_STG
		if(BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg))
			hDisplay->stNewInfo.bEnableStg = false;
#endif
	}

	if(BVDC_P_STATE_IS_CREATE(hDisplay))
	{
		hDisplay->eState = BVDC_P_State_eInactive;
	}

done:
	BDBG_LEAVE(BVDC_Display_Destroy);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *
 */
BERR_Code BVDC_Display_GetDisplayId
	( const BVDC_Display_Handle        hDisplay,
	  BVDC_DisplayId                  *peDisplayId )
{
	BDBG_ENTER(BVDC_Display_GetDisplayId);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(peDisplayId)
	{
		*peDisplayId = hDisplay->eId;
	}

	BDBG_LEAVE(BVDC_Display_GetDisplayId);
	return BERR_SUCCESS;
}
#endif


/*************************************************************************
 *	BVDC_Display_GetVbiPath
 *************************************************************************/
BERR_Code BVDC_Display_GetVbiPath
/* TODO: implement BVDC settings template logic */
	( const BVDC_Display_Handle        hDisplay,
	  BAVC_VbiPath                    *peVbiPath )
{
	BDBG_ENTER(BVDC_Display_GetVbiPath);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(peVbiPath)
	{
		*peVbiPath = (BVDC_P_HW_ID_INVALID != hDisplay->stAnlgChan_0.ulIt)
			? hDisplay->stAnlgChan_0.ulIt : BAVC_VbiPath_eUnknown;
	}

	BDBG_LEAVE(BVDC_Display_GetVbiPath);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetAnalogVbiPath
 *************************************************************************/
BERR_Code BVDC_Display_GetAnalogVbiPath
	( const BVDC_Display_Handle        hDisplay,
	  BAVC_VbiPath                    *peVbiPath )
{
	BDBG_ENTER(BVDC_Display_GetAnalogVbiPath);

	BDBG_LEAVE(BVDC_Display_GetAnalogVbiPath);

	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(peVbiPath);
	BDBG_ERR((" !!! Not supported feature!"));
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *	BVDC_P_Display_SetHdmiConfiguration
 *************************************************************************/
static BERR_Code BVDC_P_Display_SetHdmiConfiguration
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulHdmi,
	  BAVC_MatrixCoefficients          eHdmiOutput )
{
	bool bDirty;

	BDBG_ENTER(BVDC_P_Display_SetHdmiConfiguration);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Check for valid parameters */
#if (BVDC_P_SUPPORT_DVI_OUT == 1)
	if(ulHdmi != BVDC_Hdmi_0)
#elif (BVDC_P_SUPPORT_DVI_OUT == 2)
	if(ulHdmi != BVDC_Hdmi_0 && ulHdmi != BVDC_Hdmi_1)
#endif
	{
		BDBG_ERR(("Display[%d] handle invalid", hDisplay->eId));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	bDirty = (
		 (hDisplay->stCurInfo.ulHdmi != ulHdmi) ||
		 (hDisplay->stCurInfo.eHdmiOutput != eHdmiOutput) ||
		 (VIDEO_FORMAT_IS_4kx2k(hDisplay->stNewInfo.pFmtInfo->eVideoFmt) !=
		  VIDEO_FORMAT_IS_4kx2k(hDisplay->stCurInfo.pFmtInfo->eVideoFmt)) ||
		 (hDisplay->stNewInfo.bErrorLastSetting));
	hDisplay->stNewInfo.eHdmiOutput = eHdmiOutput;
	hDisplay->stNewInfo.ulHdmi      = ulHdmi;

	/* Trying to turn off (and release) Digital Timing Generator (DviDtg). Except
	 * the following cases where we still want the DviDtg to stay on to provide
	 * triggers:
	 *  (1) The DviDtg is the master mode
	 *  (2) The formats (e.g. 4kx2k) require DviDtg(or RMD) to be master mode */
	if((eHdmiOutput == BAVC_MatrixCoefficients_eUnknown) &&
	   (!BVDC_P_DISPLAY_USED_DVI(hDisplay->eMasterTg)) &&
	   (!VIDEO_FORMAT_IS_4kx2k(hDisplay->stNewInfo.pFmtInfo->eVideoFmt)))
	{
		hDisplay->stNewInfo.bEnableHdmi = false;
		if (bDirty)
		{
			hDisplay->stNewInfo.stDirty.stBits.bHdmiEnable = BVDC_P_DIRTY;
			if (hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi])
			{
				hDisplay->stNewInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_DIRTY;
			}
			BDBG_MSG(("Display[%d] disables HDMI", hDisplay->eId));
		}
	}
	else
	{
		/* HDMI is not yet enabled */
		hDisplay->stNewInfo.bEnableHdmi = true;
		if (bDirty)
		{
			hDisplay->stNewInfo.stDirty.stBits.bHdmiEnable = BVDC_P_DIRTY;
			hDisplay->stNewInfo.stDirty.stBits.bHdmiCsc    = BVDC_P_DIRTY;

#if (!BVDC_P_SUPPORT_SEAMLESS_ATTACH)
			hDisplay->stNewInfo.stDirty.stBits.bTiming = BVDC_P_DIRTY;
			hDisplay->stNewInfo.stDirty.stBits.bTimeBase = BVDC_P_DIRTY;
			hDisplay->stNewInfo.stDirty.stBits.bDacSetting = BVDC_P_DIRTY;
			hDisplay->stNewInfo.stDirty.stBits.bAcp        = BVDC_P_DIRTY;
			hDisplay->stNewInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_DIRTY;
			if (hDisplay->stCurInfo.bRgb || hDisplay->stCurInfo.bYPrPb)
			{
				hDisplay->stNewInfo.stDirty.stBits.bMpaaComp = BVDC_P_DIRTY;
			}
#else
			hDisplay->stNewInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_DIRTY;
#endif
		}
		BDBG_MSG(("Display[%d] enables HDMI", hDisplay->eId));
	}

	BDBG_LEAVE(BVDC_P_Display_SetHdmiConfiguration);
	return BERR_SUCCESS;
}

/*************************************************************************
 *	BVDC_Display_SetCustomVideoFormat
 *
 *	Sets video format
 *************************************************************************/
BERR_Code BVDC_Display_SetCustomVideoFormat
	( BVDC_Display_Handle              hDisplay,
	  const BFMT_VideoInfo            *pFmtInfo )
{
	BERR_Code eStatus = BERR_SUCCESS;
	bool      bFmtChange = false;

	BDBG_ENTER(BVDC_Display_SetCustomVideoFormat);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Valid parameters */
	if((NULL == pFmtInfo) ||
	   (pFmtInfo->eVideoFmt >= BFMT_VideoFmt_eMaxCount))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if(pFmtInfo->eVideoFmt == BFMT_VideoFmt_eDVI_1280x768p_Red)
	{
		BDBG_ERR(("Format Not Supported"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	if(!BVDC_P_IsVidfmtSupported(pFmtInfo->eVideoFmt))
	{
		BDBG_ERR(("Format not supported: %s", pFmtInfo->pchFormatStr));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

#if BVDC_P_SUPPORT_VIP
	if(hDisplay->stNewInfo.hVipHeap) {
		if(pFmtInfo->ulDigitalWidth > hDisplay->stNewInfo.stVipMemSettings.ulMaxWidth ||
		   pFmtInfo->ulDigitalHeight> hDisplay->stNewInfo.stVipMemSettings.ulMaxHeight)
		{
			BDBG_ERR(("Display format[%ux%u] is greater than VIP memory allocation[%ux%u]!",
				pFmtInfo->ulDigitalWidth, pFmtInfo->ulDigitalHeight,
				hDisplay->stNewInfo.stVipMemSettings.ulMaxWidth, hDisplay->stNewInfo.stVipMemSettings.ulMaxHeight));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
		if(hDisplay->hVdc->stBoxConfig.stVdc.astDisplay[hDisplay->eId].eMaxVideoFmt != BBOX_VDC_DISREGARD)
		{
			const BFMT_VideoInfo *pMaxFmt = BFMT_GetVideoFormatInfoPtr(hDisplay->hVdc->stBoxConfig.stVdc.astDisplay[hDisplay->eId].eMaxVideoFmt);
			if(pMaxFmt &&
			   ((pMaxFmt->ulDigitalWidth < pFmtInfo->ulDigitalWidth) || (pMaxFmt->ulDigitalHeight < pFmtInfo->ulDigitalHeight)))
			{
				BDBG_ERR(("Display format[%ux%u] is greater than VIP box mode max format[%ux%u]!",
					pFmtInfo->ulDigitalWidth, pFmtInfo->ulDigitalHeight,
					pMaxFmt->ulDigitalWidth, pMaxFmt->ulDigitalHeight));
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}
		}
	}
#endif

	/* Update new video format */
	hDisplay->stNewInfo.pFmtInfo = pFmtInfo;
	bFmtChange = (hDisplay->stCurInfo.pFmtInfo != pFmtInfo);
	/* specificlly for custom format
	 * a simple check sum calculation*/
	if(BVDC_P_IS_CUSTOMFMT(pFmtInfo->eVideoFmt))
	{
		hDisplay->stNewInfo.stCustomFmt = *pFmtInfo;
		hDisplay->stNewInfo.pFmtInfo = &hDisplay->stNewInfo.stCustomFmt;
		bFmtChange |= (hDisplay->stCurInfo.stCustomFmt.ulWidth != pFmtInfo->ulWidth) |
			(hDisplay->stCurInfo.stCustomFmt.ulDigitalWidth != pFmtInfo->ulDigitalWidth) |
			(hDisplay->stCurInfo.stCustomFmt.ulHeight != pFmtInfo->ulHeight) |
			(hDisplay->stCurInfo.stCustomFmt.ulDigitalHeight != pFmtInfo->ulDigitalHeight) |
			(hDisplay->stCurInfo.stCustomFmt.ulVertFreq != pFmtInfo->ulVertFreq) |
			(hDisplay->stCurInfo.stCustomFmt.bInterlaced != pFmtInfo->bInterlaced) ;
	}

	/* 1/1001 rate tracking */
	if(hDisplay->stCurInfo.ulVertFreq != pFmtInfo->ulVertFreq) {
		hDisplay->stNewInfo.stDirty.stBits.bSrcFrameRate = BVDC_P_DIRTY;
	}
	/* aspect ratio only */
	if(hDisplay->stCurInfo.stCustomFmt.eAspectRatio != pFmtInfo->eAspectRatio) {
		hDisplay->stNewInfo.stDirty.stBits.bAspRatio = BVDC_P_DIRTY;
	}

#if BVDC_P_SUPPORT_OSCL
	/* Certain chipsets rely on OSCL (output sclaer) within compositor to
	 * to achieve 1080p output. When in this mode, VEC is running at 1080p, but
	 * the whole BVN is configured as if the output format is 1080i.
	 * We use OSCL to convert the interlaced picture to frame then render it
	 * to VEC.
	 */
	if ((BFMT_VideoFmt_e1080p == pFmtInfo->eVideoFmt) ||
		(BFMT_VideoFmt_e1080p_50Hz== pFmtInfo->eVideoFmt))
	{
		hDisplay->hCompositor->stNewInfo.pFmtInfo =
			BFMT_GetVideoFormatInfoPtr((BFMT_VideoFmt_e1080p == pFmtInfo->eVideoFmt) ?
				BFMT_VideoFmt_e1080i : BFMT_VideoFmt_e1080i_50Hz);
		hDisplay->hCompositor->stNewInfo.bEnableOScl = true;
		if((bFmtChange) || (hDisplay->stNewInfo.bErrorLastSetting))
		{
			hDisplay->hCompositor->stNewInfo.stDirty.stBits.bOScl = BVDC_P_DIRTY;
		}
		hDisplay->stNewInfo.ulTriggerModuloCnt = 2;
	}
	else
	{
		hDisplay->hCompositor->stNewInfo.pFmtInfo = hDisplay->stNewInfo.pFmtInfo;
		hDisplay->hCompositor->stNewInfo.bEnableOScl = false;
		if((bFmtChange) || (hDisplay->stNewInfo.bErrorLastSetting))
		{
			hDisplay->hCompositor->stNewInfo.stDirty.stBits.bOScl = BVDC_P_DIRTY;
		}
		hDisplay->stNewInfo.ulTriggerModuloCnt = 1;
	}
#else
	hDisplay->hCompositor->stNewInfo.pFmtInfo = hDisplay->stNewInfo.pFmtInfo;
#endif
	hDisplay->stNewInfo.ulVertFreq = pFmtInfo->ulVertFreq;
	hDisplay->stNewInfo.eAspectRatio = pFmtInfo->eAspectRatio;
	if (BVDC_P_DISPLAY_USED_DIGTRIG(hDisplay->eMasterTg))
		hDisplay->stNewInfo.ulHeight = pFmtInfo->ulDigitalHeight >> pFmtInfo->bInterlaced;
	else
		hDisplay->stNewInfo.ulHeight = pFmtInfo->ulHeight >> pFmtInfo->bInterlaced;

	BVDC_P_Display_SetHdmiConfiguration(hDisplay, hDisplay->stNewInfo.ulHdmi, hDisplay->stNewInfo.eHdmiOutput);

	if((bFmtChange) || (hDisplay->stNewInfo.bErrorLastSetting))
	{
		/* When setting a new video format, most stuffs like time base,
		 * DAC, etc need to be programmed. */
		hDisplay->stNewInfo.stDirty.stBits.bTiming = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bTimeBase = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bDacSetting = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bAcp        = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.b3DSetting = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bAlignment = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bAspRatio = BVDC_P_DIRTY;
		if (hDisplay->bAnlgEnable)
		{
			hDisplay->stNewInfo.stDirty.stBits.bMpaaComp = BVDC_P_DIRTY;
		}
		if (hDisplay->stDviChan.bEnable)
		{
			hDisplay->stNewInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_DIRTY;
		}
	}
	BDBG_MSG(("pFmt=%p[%s], cur->pFmt=%p[%s], bFmtChange=%d",
		(void *)pFmtInfo,pFmtInfo->pchFormatStr,(void *)hDisplay->stCurInfo.pFmtInfo,hDisplay->stCurInfo.pFmtInfo->pchFormatStr, bFmtChange));

	BDBG_LEAVE(BVDC_Display_SetCustomVideoFormat);
	return eStatus;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetCustomVideoFormat
 *************************************************************************/
const BFMT_VideoInfo* BVDC_Display_GetCustomVideoFormat
	( const BVDC_Display_Handle        hDisplay )
{
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	return hDisplay->stCurInfo.pFmtInfo;
}
#endif

/*************************************************************************
 *	BVDC_Display_SetVideoFormat
 *
 *	Sets video format
 *************************************************************************/
BERR_Code BVDC_Display_SetVideoFormat
	( BVDC_Display_Handle              hDisplay,
	  BFMT_VideoFmt                    eVideoFormat )
{
	return BVDC_Display_SetCustomVideoFormat(hDisplay,
		BFMT_GetVideoFormatInfoPtr(eVideoFormat));
}

/*************************************************************************
 *	BVDC_Display_GetVideoFormat
 *************************************************************************/
BERR_Code BVDC_Display_GetVideoFormat
	( const BVDC_Display_Handle        hDisplay,
	  BFMT_VideoFmt                   *peVideoFormat )
{
	BDBG_ENTER(BVDC_Display_GetVideoFormat);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(peVideoFormat)
	{
		*peVideoFormat = hDisplay->stCurInfo.pFmtInfo->eVideoFmt;
	}

	BDBG_LEAVE(BVDC_Display_GetVideoFormat);
	return BERR_SUCCESS;
}

/*************************************************************************
 * BVDC_Display_SetDacConfiguration
 *
 * Enable Dac(s) to specific output, or disable Dac(s). Use the following
 *  defines to identify specific Dacs:
 * BVDC_Dac_0
 * BVDC_Dac_1
 * BVDC_Dac_2
 * BVDC_Dac_3
 * BVDC_Dac_4
 * BVDC_Dac_5
 *
 *  A DAC can only be used by one display at a time. A display is not using
 *  a specifc DAC if its corresponding state is BVDC_DacOutput_eUnused.
 *  A DAC that is unused by all displays is disabled. Initially, all DACs
 *  are unused for all displays.
 *************************************************************************/
BERR_Code  BVDC_Display_SetDacConfiguration
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulDacs,
	  BVDC_DacOutput                   eDacOutput )
{
	BVDC_P_DisplayInfo *pNewInfo;
	BVDC_P_DisplayInfo *pCurInfo;
	uint32_t            i;

	BDBG_ENTER(BVDC_Display_SetDacConfiguration);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

#if defined(BVDC_GFX_PERSIST)
	if (BVDC_DacOutput_eUnused == eDacOutput)
	{
		BVDC_Window_Handle  hWindow;

		switch (hDisplay->hCompositor->eId)
		{
		case BVDC_CompositorId_eCompositor0:
			hWindow = hDisplay->hCompositor->ahWindow[BVDC_P_WindowId_eComp0_G0];
			break;
		case BVDC_CompositorId_eCompositor1:
			hWindow = hDisplay->hCompositor->ahWindow[BVDC_P_WindowId_eComp1_G0];
			break;
		default:
			hWindow = hDisplay->hCompositor->ahWindow[BVDC_P_WindowId_eComp2_G0];
			break;
		}
		if ( NULL == hWindow ||
			 BVDC_P_State_eActive == hWindow->stCurInfo.eReaderState)
		{
			return BERR_SUCCESS;
		}
	}
#endif

	if(ulDacs > BVDC_Dac_0+BVDC_Dac_1+BVDC_Dac_2+BVDC_Dac_3+BVDC_Dac_4+BVDC_Dac_5+BVDC_Dac_6)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	pNewInfo = &hDisplay->stNewInfo;
	pCurInfo = &hDisplay->stCurInfo;


	/* Other than VDEC pass-through, bypass path does not support Dac outputs */
	if(hDisplay->bIsBypass && (eDacOutput != BVDC_DacOutput_eUnused))
	{
		return BERR_TRACE(BVDC_ERR_INVALID_DAC_SETTINGS);
	}

	if((hDisplay->stDviChan.bEnable || hDisplay->st656Chan.bEnable) && eDacOutput != BVDC_DacOutput_eUnused)
	{
		BDBG_ERR(("ANALOG slave mode not supported"));
		return BERR_TRACE(BVDC_ERR_INVALID_DAC_SETTINGS);
	}

	/* Update new outputs for each Dac */
	for(i=0; i < BVDC_P_MAX_DACS; i++)
	{
		if((ulDacs >> i) & 1)
		{
			pNewInfo->aDacOutput[i] = eDacOutput;
			if(eDacOutput != pCurInfo->aDacOutput[i] || pNewInfo->bErrorLastSetting)
			{
				pNewInfo->stDirty.stBits.bDacSetting = BVDC_P_DIRTY;
			}
		}
	}

	if(!hDisplay->bAnlgEnable)
	{
		if (pNewInfo->stDirty.stBits.bDacSetting == BVDC_P_DIRTY)
		{
			/* when adding a slave analog path */
			pNewInfo->stDirty.stBits.bTiming = BVDC_P_DIRTY;
			pNewInfo->stDirty.stBits.bTimeBase = BVDC_P_DIRTY;
			pNewInfo->stDirty.stBits.b3DSetting = BVDC_P_DIRTY;
			pNewInfo->stDirty.stBits.bAlignment = BVDC_P_DIRTY;
			pNewInfo->stDirty.stBits.bAcp = BVDC_P_DIRTY;
		}
	}

	if (pNewInfo->stDirty.stBits.bDacSetting == BVDC_P_DIRTY &&
		pCurInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] &&
		(eDacOutput == BVDC_DacOutput_eRed   || eDacOutput == BVDC_DacOutput_eY  ||
		 eDacOutput == BVDC_DacOutput_eGreen || eDacOutput == BVDC_DacOutput_ePr ||
		 eDacOutput == BVDC_DacOutput_eBlue  || eDacOutput == BVDC_DacOutput_ePb ||
		 (eDacOutput == BVDC_DacOutput_eUnused &&
		  (hDisplay->stCurInfo.bRgb || hDisplay->stCurInfo.bYPrPb))))
	{
		pNewInfo->stDirty.stBits.bMpaaComp = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetDacConfiguration);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetDacConfiguration
 *
 *	Returns a specific Dac output setting.
 *************************************************************************/
BERR_Code BVDC_Display_GetDacConfiguration
	( const BVDC_Display_Handle        hDisplay,
	  uint32_t                         ulDac,
	  BVDC_DacOutput                  *peDacOutput )
{
	uint32_t     i;
	BVDC_DacOutput eDacOutput = BVDC_DacOutput_eUnused;

	BDBG_ENTER(BVDC_Display_GetDacConfiguration);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if((!peDacOutput) || (ulDac > (BVDC_Dac_6 << 1)))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Get index to Dac table */
	for (i=0; i < BVDC_P_MAX_DACS; i++)
	{
		if((ulDac >> i) & 1)
		{
			eDacOutput = hDisplay->stCurInfo.aDacOutput[i];
			break;
		}
	}

	*peDacOutput = eDacOutput;
	BDBG_LEAVE(BVDC_Display_GetDacConfiguration);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *	BVDC_Display_SetHdmiConfiguration
 *************************************************************************/
BERR_Code BVDC_Display_SetHdmiConfiguration
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulHdmi,
	  BAVC_MatrixCoefficients          eHdmiOutput )
{
	BERR_Code err;

	BDBG_ENTER(BVDC_Display_SetHdmiConfiguration);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

#if defined(BVDC_GFX_PERSIST)
	if (BAVC_MatrixCoefficients_eUnknown == eHdmiOutput)
	{
		return BERR_SUCCESS;
	}
#endif

	if(eHdmiOutput > BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr)
	{
		BDBG_ERR(("DISP[%d] invalid eHdmiOutput=%d", hDisplay->eId, eHdmiOutput));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	err = BVDC_P_Display_SetHdmiConfiguration(hDisplay, ulHdmi, eHdmiOutput);

	BDBG_LEAVE(BVDC_Display_SetHdmiConfiguration);
	return err;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetHdmiConfiguration
 *
 *	Query the HDMI output for a specific HDMI
 *************************************************************************/
BERR_Code BVDC_Display_GetHdmiConfiguration
	( const BVDC_Display_Handle        hDisplay,
	  uint32_t                         ulHdmi,
	  BAVC_MatrixCoefficients         *peHdmiOutput )
{
	BDBG_ENTER(BVDC_Display_GetHdmiConfiguration);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

#if (BVDC_P_SUPPORT_DVI_OUT == 1)
	if(ulHdmi != BVDC_Hdmi_0)
#elif (BVDC_P_SUPPORT_DVI_OUT == 2)
	if(ulHdmi != BVDC_Hdmi_0 && ulHdmi != BVDC_Hdmi_1)
#endif
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if(peHdmiOutput)
	{
		*peHdmiOutput = hDisplay->stCurInfo.eHdmiOutput;
	}

	BDBG_LEAVE(BVDC_Display_GetHdmiConfiguration);
	return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_SetHdmiDropLines
 *
 *	Set the number of compositor video lines that Hdmi drops for a
 *  given format.
 *************************************************************************/
BERR_Code BVDC_Display_SetHdmiDropLines
	( const BVDC_Display_Handle        hDisplay,
	  uint32_t                         ulHdmi,
	  BFMT_VideoFmt                    eVideoFormat,
	  uint32_t                         ulHdmiDropLines )
{
	BDBG_ENTER(BVDC_Display_SetHdmiDropLines);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

#if (BVDC_P_SUPPORT_DVI_OUT == 1)
	if(ulHdmi != BVDC_Hdmi_0)
#elif (BVDC_P_SUPPORT_DVI_OUT == 2)
	if(ulHdmi != BVDC_Hdmi_0 && ulHdmi != BVDC_Hdmi_1)
#endif
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if(!VIDEO_FORMAT_SUPPORTS_DROP_LINE(eVideoFormat))
	{
		BDBG_ERR(("Dropping compositor lines is supported for this format!"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	switch(eVideoFormat)
	{
	case BFMT_VideoFmt_e720x482_NTSC:
	case BFMT_VideoFmt_e720x482_NTSC_J:
		if (ulHdmiDropLines > BVDC_P_480i_DROP_LINES_MAX)
		{
		BDBG_ERR(("Exceeds number of dropped lines supported by NTSC, NTSCJ."));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}

		if ((ulHdmiDropLines != 0) && (ulHdmiDropLines != 2))
		{
			BDBG_ERR(("NTSC, NTSCJ and PAL_M only support dropping of 0 or 2 lines."));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}

		break;

	case BFMT_VideoFmt_e720x483p:
		if (ulHdmiDropLines > BVDC_P_480p_DROP_LINES_MAX)
		{
			BDBG_ERR(("Exceeds number of dropped lines supported by 480p."));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
		break;

	default:
		/* Assert if format isn't handled yet. */
		BDBG_ASSERT(false);
		break;
	}

	hDisplay->stNewInfo.aulHdmiDropLines[eVideoFormat] = ulHdmiDropLines;

	if((hDisplay->stCurInfo.aulHdmiDropLines[eVideoFormat] != ulHdmiDropLines) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bHdmiDroplines = BVDC_P_DIRTY;

		/* DTRAM instructions will have to be reloaded.
		 */
		hDisplay->stNewInfo.stDirty.stBits.bTiming = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bTimeBase = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bDacSetting = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bAcp        = BVDC_P_DIRTY;

		if (hDisplay->bAnlgEnable)
		{
			hDisplay->stNewInfo.stDirty.stBits.bMpaaComp = BVDC_P_DIRTY;
		}

		hDisplay->stNewInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetHdmiDropLines);
	return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetHdmiDropLines
 *
 *	Get the number of compositor video lines that Hdmi drops for a given
 *  format.
 *************************************************************************/
BERR_Code BVDC_Display_GetHdmiDropLines
	( const BVDC_Display_Handle        hDisplay,
	  uint32_t                         ulHdmi,
	  BFMT_VideoFmt                    eVideoFormat,
	  uint32_t                        *pulHdmiDropLines )
{
	BDBG_ENTER(BVDC_Display_GetHdmiDropLines);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

#if (BVDC_P_SUPPORT_DVI_OUT == 1)
	if(ulHdmi != BVDC_Hdmi_0)
#elif (BVDC_P_SUPPORT_DVI_OUT == 2)
	if(ulHdmi != BVDC_Hdmi_0 && ulHdmi != BVDC_Hdmi_1)
#endif
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if (pulHdmiDropLines)
	{
		*pulHdmiDropLines = hDisplay->stCurInfo.aulHdmiDropLines[eVideoFormat];
	}

	BDBG_LEAVE(BVDC_Display_GetHdmiDropLines);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_SetHdmiSyncOnly
	( const BVDC_Display_Handle        hDisplay,
	  bool                             bSyncOnly )
{
	bool bCurSyncOnly;

	BVDC_Display_GetHdmiSyncOnly(hDisplay, &bCurSyncOnly);

	BDBG_ENTER(BVDC_Display_SetHdmiSyncOnly);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* only take effect if hdmi is enable */
	if(hDisplay->stNewInfo.bEnableHdmi == true)
	{
		BVDC_Display_SetMuteMode(hDisplay, BVDC_DisplayOutput_eDvo,
			                     bSyncOnly ? BVDC_MuteMode_eConst : BVDC_MuteMode_eDisable);
		if ((bCurSyncOnly != bSyncOnly) ||
			(hDisplay->stNewInfo.bErrorLastSetting))
		{
			hDisplay->stNewInfo.stDirty.stBits.bHdmiSyncOnly = BVDC_P_DIRTY;
		}
	}

	BDBG_LEAVE(BVDC_Display_SetHdmiSyncOnly);
	return BERR_SUCCESS;
}

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetHdmiSyncOnly
	( const BVDC_Display_Handle        hDisplay,
	  bool                            *pbSyncOnly )
{
	BVDC_MuteMode eMuteMode;

	BDBG_ENTER(BVDC_Display_GetHdmiSyncOnly);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(pbSyncOnly)
	{
		BVDC_Display_GetMuteMode(hDisplay, BVDC_DisplayOutput_eDvo, &eMuteMode);
		*pbSyncOnly = (eMuteMode == BVDC_MuteMode_eConst) ? true : false;
	}

	BDBG_LEAVE(BVDC_Display_GetHdmiSyncOnly);
	return BERR_SUCCESS;
}

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_SetHdmiXvYcc
	( const BVDC_Display_Handle        hDisplay,
	  bool                             bXvYcc )
{
	BDBG_ENTER(BVDC_Display_SetHdmiXvYcc);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	hDisplay->stNewInfo.bXvYcc = bXvYcc;
	if((hDisplay->stCurInfo.bXvYcc != bXvYcc) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bHdmiXvYcc = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetHdmiXvYcc);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetHdmiXvYcc
	( const BVDC_Display_Handle        hDisplay,
	  bool                            *pbXvYcc )
{
	BDBG_ENTER(BVDC_Display_GetHdmiXvYcc);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(pbXvYcc)
	{
		*pbXvYcc = hDisplay->stCurInfo.bXvYcc;
	}

	BDBG_LEAVE(BVDC_Display_GetHdmiXvYcc);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *
 *************************************************************************/
static BERR_Code BVDC_P_Display_SetHdmiSettings
	( const BVDC_Display_Handle          hDisplay,
	  const BVDC_P_Display_HdmiSettings *pHdmiSettings )
{
	BDBG_ENTER(BVDC_P_Display_SetHdmiSettings);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Current HW only supports up to 36-bit color depth */
	if(pHdmiSettings->eHdmiColorDepth > BAVC_HDMI_BitsPerPixel_e36bit)
	{
		return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
	}

	/* Current HW only supports none, 2x and 4x pixel repetition */
	if(pHdmiSettings->eHdmiPixelRepetition != BAVC_HDMI_PixelRepetition_eNone &&
	   pHdmiSettings->eHdmiPixelRepetition != BAVC_HDMI_PixelRepetition_e2x &&
	   pHdmiSettings->eHdmiPixelRepetition != BAVC_HDMI_PixelRepetition_e4x)
	{
		return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
	}

	if(pHdmiSettings->stSettings.eColorComponent >= BAVC_Colorspace_eFuture ||
	   pHdmiSettings->stSettings.eColorRange >= BAVC_ColorRange_eMax ||
	   pHdmiSettings->stSettings.eEotf >= BAVC_HDMI_DRM_EOTF_eMax)
	{
		return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
	}

	/* TODO: add check to only allow this API to be used for 4kx2k fmts and */
	/* with chips that have DSCL block */
#if !BVDC_P_SUPPORT_DSCL
	if(VIDEO_FORMAT_IS_4kx2k(pHdmiSettings->eHDMIFormat))
	{
		BDBG_ERR(("4kx2k output is not supported on this chipset"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
#endif

	/* Currently for chips with DSCL, we only support upscaling from */
	/* 1080p24/25/30 to 4kx2k@24/25/30 */
	if((!VIDEO_FORMAT_IS_4kx2k(pHdmiSettings->eHDMIFormat) && (pHdmiSettings->eHDMIFormat != BFMT_VideoFmt_eMaxCount)) ||
	   (VIDEO_FORMAT_IS_4kx2k(pHdmiSettings->eHDMIFormat) &&
	    (pHdmiSettings->eMatchingFormat != BFMT_VideoFmt_e1080p_24Hz &&
	     pHdmiSettings->eMatchingFormat != BFMT_VideoFmt_e1080p_25Hz &&
	     pHdmiSettings->eMatchingFormat != BFMT_VideoFmt_e1080p_30Hz &&
	     pHdmiSettings->eMatchingFormat != BFMT_VideoFmt_e1080p_50Hz &&
	     pHdmiSettings->eMatchingFormat != BFMT_VideoFmt_e1080p))) /* 60hz */
	{
		BDBG_ERR(("Invalid HDMI format or matching format"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	if(VIDEO_FORMAT_IS_4kx2k(pHdmiSettings->eHDMIFormat) && pHdmiSettings->eMatchingFormat != BFMT_VideoFmt_eMaxCount)
	{
		const BFMT_VideoInfo *pHdmiFmtInfo = BFMT_GetVideoFormatInfoPtr(pHdmiSettings->eHDMIFormat);
		const BFMT_VideoInfo *pMatchingFmtInfo = BFMT_GetVideoFormatInfoPtr(pHdmiSettings->eMatchingFormat);
		if(pHdmiFmtInfo->ulVertFreq != pMatchingFmtInfo->ulVertFreq)
		{
			BDBG_ERR(("Vert Freq does not match"));
			return BERR_TRACE(BERR_NOT_SUPPORTED);
		}
	}

	hDisplay->stNewInfo.pHdmiFmtInfo =
		(pHdmiSettings->eHDMIFormat != BFMT_VideoFmt_eMaxCount) ?
		BFMT_GetVideoFormatInfoPtr(pHdmiSettings->eHDMIFormat) : NULL;
	hDisplay->stNewInfo.stHdmiSettings = *pHdmiSettings;

	if((hDisplay->stCurInfo.stHdmiSettings.eHDMIFormat     != hDisplay->stNewInfo.stHdmiSettings.eHDMIFormat) ||
	   (hDisplay->stCurInfo.stHdmiSettings.eMatchingFormat != hDisplay->stNewInfo.stHdmiSettings.eMatchingFormat) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		/* set dirty bit to indicate HDMI format change */
		hDisplay->stNewInfo.stDirty.stBits.bHdmiSettings = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stHdmiSettings.stDirty.stBits.bHdmiRmChanged = BVDC_P_DIRTY;
	}

	if((hDisplay->stCurInfo.stHdmiSettings.eHdmiPixelRepetition != hDisplay->stNewInfo.stHdmiSettings.eHdmiPixelRepetition) ||
	   (hDisplay->stCurInfo.stHdmiSettings.eHdmiColorDepth != hDisplay->stNewInfo.stHdmiSettings.eHdmiColorDepth) ||
	   (hDisplay->stCurInfo.stHdmiSettings.stSettings.eColorComponent != hDisplay->stNewInfo.stHdmiSettings.stSettings.eColorComponent) ||
	   (hDisplay->stCurInfo.stHdmiSettings.stSettings.eColorRange != hDisplay->stNewInfo.stHdmiSettings.stSettings.eColorRange) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		/* Treat color component change as format change - need whole VEC path reset */
		hDisplay->stNewInfo.stDirty.stBits.bTiming = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bAcp    = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bTimeBase = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bDacSetting = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bHdmiSettings = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stHdmiSettings.stDirty.stBits.bHdmiRmChanged = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stHdmiSettings.stDirty.stBits.bHdmiColorComponent = BVDC_P_DIRTY;
	}
	if (hDisplay->stCurInfo.stHdmiSettings.stSettings.eEotf != hDisplay->stNewInfo.stHdmiSettings.stSettings.eEotf)
	{
		hDisplay->stNewInfo.stDirty.stBits.bHdmiSettings = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_P_Display_SetHdmiSettings);
	return BERR_SUCCESS;
}

/*************************************************************************
 *
 *************************************************************************/
static BERR_Code BVDC_P_Display_GetHdmiSettings
	( const BVDC_Display_Handle        hDisplay,
	  BVDC_P_Display_HdmiSettings     *pHdmiSettings )
{
	BDBG_ENTER(BVDC_P_Display_GetHdmiSettings);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(pHdmiSettings)
	{
		*pHdmiSettings = hDisplay->stNewInfo.stHdmiSettings;
	}

	BDBG_LEAVE(BVDC_P_Display_GetHdmiSettings);
	return BERR_SUCCESS;
}

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_SetHdmiSettings
	( const BVDC_Display_Handle        hDisplay,
	  const BVDC_Display_HdmiSettings *pHdmiSettings )
{
	BVDC_P_Display_HdmiSettings stHdmiSettings;
	BERR_Code eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Display_SetHdmiSettings);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	eStatus = BVDC_P_Display_GetHdmiSettings(hDisplay, &stHdmiSettings);
	stHdmiSettings.stSettings = *pHdmiSettings;
	eStatus = BVDC_P_Display_SetHdmiSettings(hDisplay, &stHdmiSettings);

	BDBG_LEAVE(BVDC_Display_SetHdmiSettings);
	return eStatus;
}

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetHdmiSettings
	( const BVDC_Display_Handle        hDisplay,
	  BVDC_Display_HdmiSettings       *pHdmiSettings )
{
	BDBG_ENTER(BVDC_Display_GetHdmiSettings);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(pHdmiSettings)
	{
		*pHdmiSettings = hDisplay->stCurInfo.stHdmiSettings.stSettings;
	}

	BDBG_LEAVE(BVDC_Display_GetHdmiSettings);
	return BERR_SUCCESS;
}

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_SetHdmiColorDepth
	( const BVDC_Display_Handle        hDisplay,
	  BAVC_HDMI_BitsPerPixel           eColorDepth )
{
	BVDC_P_Display_HdmiSettings stHdmiSettings;
	BERR_Code eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Display_SetHdmiColorDepth);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	eStatus = BVDC_P_Display_GetHdmiSettings(hDisplay, &stHdmiSettings);
	stHdmiSettings.eHdmiColorDepth = eColorDepth;
	eStatus = BVDC_P_Display_SetHdmiSettings(hDisplay, &stHdmiSettings);

	BDBG_LEAVE(BVDC_Display_SetHdmiColorDepth);
	return eStatus;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetHdmiColorDepth
	( const BVDC_Display_Handle        hDisplay,
	  BAVC_HDMI_BitsPerPixel          *eColorDepth )
{
	BDBG_ENTER(BVDC_Display_GetHdmiColorDepth);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(eColorDepth)
	{
		*eColorDepth = hDisplay->stCurInfo.stHdmiSettings.eHdmiColorDepth;
	}

	BDBG_LEAVE(BVDC_Display_GetHdmiColorDepth);
	return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_SetHdmiPixelRepetition
	( const BVDC_Display_Handle        hDisplay,
	  BAVC_HDMI_PixelRepetition        ePixelRepetition )
{
	BVDC_P_Display_HdmiSettings stHdmiSettings;
	BERR_Code eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Display_SetHdmiPixelRepetition);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	eStatus = BVDC_P_Display_GetHdmiSettings(hDisplay, &stHdmiSettings);
	stHdmiSettings.eHdmiPixelRepetition = ePixelRepetition;
	eStatus = BVDC_P_Display_SetHdmiSettings(hDisplay, &stHdmiSettings);

	BDBG_LEAVE(BVDC_Display_SetHdmiPixelRepetition);
	return eStatus;
}
#endif

#if !B_REFSW_MINIMAL
/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetHdmiPixelRepetition
	( const BVDC_Display_Handle        hDisplay,
	  BAVC_HDMI_PixelRepetition       *ePixelRepetition )
{
	BDBG_ENTER(BVDC_Display_GetHdmiPixelRepetition);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(ePixelRepetition)
	{
		*ePixelRepetition = hDisplay->stCurInfo.stHdmiSettings.eHdmiPixelRepetition;
	}

	BDBG_LEAVE(BVDC_Display_GetHdmiPixelRepetition);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_SetHdmiFormat
	( const BVDC_Display_Handle        hDisplay,
	  BFMT_VideoFmt                    eHDMIFormat,
	  BFMT_VideoFmt                    eMatchingFormat )
{
	BVDC_P_Display_HdmiSettings stHdmiSettings;
	BERR_Code eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Display_SetHdmiFormat);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	eStatus = BVDC_P_Display_GetHdmiSettings(hDisplay, &stHdmiSettings);
	stHdmiSettings.eHDMIFormat     = eHDMIFormat;
	stHdmiSettings.eMatchingFormat = eMatchingFormat;
	eStatus = BVDC_P_Display_SetHdmiSettings(hDisplay, &stHdmiSettings);

	BDBG_LEAVE(BVDC_Display_SetHdmiFormat);
	return eStatus;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetHdmiFormat
	( const BVDC_Display_Handle        hDisplay,
	  BFMT_VideoFmt                   *peHDMIFormat,
	  BFMT_VideoFmt                   *peMatchingFormat )
{
	BDBG_ENTER(BVDC_Display_GetHdmiFormat);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(peHDMIFormat)
	{
		*peHDMIFormat = hDisplay->stCurInfo.stHdmiSettings.eHDMIFormat;
	}
	if(peMatchingFormat)
	{
		*peMatchingFormat = hDisplay->stCurInfo.stHdmiSettings.eMatchingFormat;
	}

	BDBG_LEAVE(BVDC_Display_GetHdmiFormat);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *	BVDC_Display_Set656Configuration
 *************************************************************************/
BERR_Code BVDC_Display_Set656Configuration
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ul656Output,
	  bool                             bEnable )
{
	BDBG_ENTER(BVDC_Display_Set656Configuration);

#if (BVDC_P_SUPPORT_ITU656_OUT == 0)
	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(ul656Output);
	BSTD_UNUSED(bEnable);

	BDBG_ERR(("This chip does not support 656 output."));

	BDBG_LEAVE(BVDC_Display_Set656Configuration);

	return BERR_NOT_SUPPORTED;
#else

	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Check for valid parameters */
	if(ul656Output != BVDC_Itur656Output_0)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	hDisplay->stNewInfo.bEnable656 = bEnable;
	if((hDisplay->stCurInfo.bEnable656 != bEnable) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.b656Enable = BVDC_P_DIRTY;

#if (!BVDC_P_SUPPORT_SEAMLESS_ATTACH)
		if (hDisplay->stNewInfo.bEnable656)
		{
			/* Reset and reprogram master path as well */
			hDisplay->stNewInfo.stDirty.stBits.bTiming = BVDC_P_DIRTY;
			hDisplay->stNewInfo.stDirty.stBits.bTimeBase = BVDC_P_DIRTY;
			hDisplay->stNewInfo.stDirty.stBits.bDacSetting = BVDC_P_DIRTY;
			hDisplay->stNewInfo.stDirty.stBits.bAcp        = BVDC_P_DIRTY;

			if (hDisplay->bAnlgEnable)
			{
				hDisplay->stNewInfo.stDirty.stBits.bMpaaComp = BVDC_P_DIRTY;
			}

			if (hDisplay->stDviChan.bEnable)
			{
				hDisplay->stNewInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_DIRTY;
			}
		}
#endif
	}

	BDBG_LEAVE(BVDC_Display_Set656Configuration);

	return BERR_SUCCESS;
#endif
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_Get656Configuration
 *************************************************************************/
BERR_Code BVDC_Display_Get656Configuration
	( const BVDC_Display_Handle        hDisplay,
	  uint32_t                         ul656Output,
	  bool                            *pbEnable656Output )
{
	BDBG_ENTER(BVDC_Display_Get656Configuration);

#if (BVDC_P_SUPPORT_ITU656_OUT == 0)
	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(ul656Output);
	BSTD_UNUSED(pbEnable656Output);

	BDBG_ERR(("This chip does not support 656 output."));

	BDBG_LEAVE(BVDC_Display_Get656Configuration);
	return BERR_NOT_SUPPORTED;

#else
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(ul656Output != BVDC_Itur656Output_0)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if(pbEnable656Output)
	{
		*pbEnable656Output = hDisplay->stCurInfo.bEnable656;
	}

	BDBG_LEAVE(BVDC_Display_Get656Configuration);
	return BERR_SUCCESS;
#endif
}
#endif

/*************************************************************************
 *	BVDC_Display_RampStgResolution
 *************************************************************************/
BERR_Code BVDC_Display_RampStgResolution
	( BVDC_Display_Handle hDisplay,
	  uint32_t            ulResolutionRampCount )
{
	BDBG_ENTER(BVDC_Display_RampStgResolution);
#if (BVDC_P_SUPPORT_STG == 0)
	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(ulResolutionRampCount);

	BDBG_ERR(("This chip does not support STG"));

	BDBG_LEAVE(BVDC_Display_RampStgResolution);

	return BERR_NOT_SUPPORTED;
#else
	if (hDisplay->stNewInfo.bEnableStg)
	{
		hDisplay->stNewInfo.ulResolutionRampCount = ulResolutionRampCount;
		hDisplay->stNewInfo.stDirty.stBits.bTiming = BVDC_P_DIRTY;
	}
	BDBG_LEAVE(BVDC_Display_RampStgResolution);

	return BERR_SUCCESS;
#endif
}

/*************************************************************************
 *	BVDC_Display_SetStgConfiguration
 *************************************************************************/
BERR_Code BVDC_Display_SetStgConfiguration
	( BVDC_Display_Handle              hDisplay,
	  bool                             bEnable,
	  const BVDC_Display_StgSettings  *pStgSettings )
{
	BDBG_ENTER(BVDC_Display_SetStgConfiguration);

#if (BVDC_P_SUPPORT_STG == 0)
	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(bEnable);
	BSTD_UNUSED(pStgSettings);

	BDBG_ERR(("This chip does not support STG"));

	BDBG_LEAVE(BVDC_Display_SetStgConfiguration);

	return BERR_NOT_SUPPORTED;
#else
	/* Check for valid parameters */
	if(hDisplay ==NULL)
	{
		BDBG_LEAVE(BVDC_Display_SetStgConfiguration);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* @@@ About to remove condition for 7425 b0 slave mode removal*/
	if(BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg))
	{
		if(false == bEnable)
		{
			BDBG_ERR(("STG Master Mode cannot turn off trigger!!!"));
			BDBG_LEAVE(BVDC_Display_SetStgConfiguration);
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
	}

	hDisplay->stNewInfo.bEnableStg =
		(bEnable || BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg));

	if(pStgSettings)
	{
		if(pStgSettings->bNonRealTime &&
			(!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg)))
		{
			BDBG_ERR(("Display[%d] eMasterTg=%d is not STG, cannot be non-realtime!",
				hDisplay->eId, hDisplay->eMasterTg));
			BDBG_LEAVE(BVDC_Display_SetStgConfiguration);
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}

#if BVDC_P_SUPPORT_VIP
	if(pStgSettings->vip.hHeap)
	{
		if(hDisplay->stNewInfo.pFmtInfo->ulDigitalWidth > pStgSettings->vip.stMemSettings.ulMaxWidth ||
			hDisplay->stNewInfo.pFmtInfo->ulDigitalHeight> pStgSettings->vip.stMemSettings.ulMaxHeight)
		{
			BDBG_ERR(("Display format[%ux%u] is greater than VIP memory allocation[%ux%u]!",
				hDisplay->stNewInfo.pFmtInfo->ulDigitalWidth, hDisplay->stNewInfo.pFmtInfo->ulDigitalHeight,
				pStgSettings->vip.stMemSettings.ulMaxWidth, pStgSettings->vip.stMemSettings.ulMaxHeight));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
		if(hDisplay->hVdc->stBoxConfig.stVdc.astDisplay[hDisplay->eId].eMaxVideoFmt != BBOX_VDC_DISREGARD)
		{
			const BFMT_VideoInfo *pMaxFmt = BFMT_GetVideoFormatInfoPtr(hDisplay->hVdc->stBoxConfig.stVdc.astDisplay[hDisplay->eId].eMaxVideoFmt);
			BDBG_MSG(("box mode %u, display[%u] max fmt = %p[%u]", hDisplay->hVdc->stBoxConfig.stBox.ulBoxId, hDisplay->eId,
			pMaxFmt, hDisplay->hVdc->stBoxConfig.stVdc.astDisplay[hDisplay->eId].eMaxVideoFmt));
			if(pMaxFmt &&
			   ((pMaxFmt->ulDigitalWidth < pStgSettings->vip.stMemSettings.ulMaxWidth) || (pMaxFmt->ulDigitalHeight < pStgSettings->vip.stMemSettings.ulMaxHeight)))
			{
				BDBG_ERR(("VIP memory allocation[%ux%u] is greater than VIP box mode max format[%ux%u]!",
					pStgSettings->vip.stMemSettings.ulMaxWidth, pStgSettings->vip.stMemSettings.ulMaxHeight,
					pMaxFmt->ulDigitalWidth, pMaxFmt->ulDigitalHeight));
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}
		}
	}
#endif

		hDisplay->stNewInfo.bStgNonRealTime = pStgSettings->bNonRealTime;
		hDisplay->stNewInfo.ulStcSnapshotHiAddr = pStgSettings->ulStcSnapshotHiAddr;
		hDisplay->stNewInfo.ulStcSnapshotLoAddr = pStgSettings->ulStcSnapshotLoAddr;
#if (BVDC_P_SUPPORT_VIP)
		hDisplay->stNewInfo.hVipHeap        = pStgSettings->vip.hHeap;
		BKNI_Memcpy(&hDisplay->stNewInfo.stVipMemSettings, &pStgSettings->vip.stMemSettings, sizeof(BVDC_VipMemConfigSettings));
#endif
		if((bEnable != hDisplay->stCurInfo.bEnableStg) ||
		   (pStgSettings->bNonRealTime != hDisplay->stCurInfo.bStgNonRealTime) ||
		   (hDisplay->stNewInfo.bErrorLastSetting))
		{
			hDisplay->stNewInfo.stDirty.stBits.bStgEnable = BVDC_P_DIRTY;

			if (hDisplay->stNewInfo.bEnableStg)
			{
				/* Reset and reprogram master path as well */
				hDisplay->stNewInfo.stDirty.stBits.bTiming = BVDC_P_DIRTY;
				hDisplay->stNewInfo.stDirty.stBits.bTimeBase = BVDC_P_DIRTY;
				hDisplay->stNewInfo.stDirty.stBits.bDacSetting = BVDC_P_DIRTY;
				hDisplay->stNewInfo.stDirty.stBits.bAcp        = BVDC_P_DIRTY;
			}
		}

#if (BVDC_P_SUPPORT_VIP)
		if((pStgSettings->vip.hHeap != hDisplay->stCurInfo.hVipHeap) ||
		   (pStgSettings->vip.stMemSettings.ulMaxWidth != hDisplay->stCurInfo.stVipMemSettings.ulMaxWidth) ||
		   (pStgSettings->vip.stMemSettings.ulMaxHeight != hDisplay->stCurInfo.stVipMemSettings.ulMaxHeight) ||
		   (pStgSettings->ulStcSnapshotHiAddr != hDisplay->stCurInfo.ulStcSnapshotHiAddr) ||
		   (pStgSettings->ulStcSnapshotLoAddr != hDisplay->stCurInfo.ulStcSnapshotLoAddr) ||
		   (pStgSettings->vip.stMemSettings.bSupportInterlaced != hDisplay->stCurInfo.stVipMemSettings.bSupportInterlaced))
		{
			BDBG_MSG(("VIP change: heap=%p, %ux%u%c memc%u", (void *)pStgSettings->vip.hHeap, hDisplay->stNewInfo.stVipMemSettings.ulMaxWidth,
				hDisplay->stNewInfo.stVipMemSettings.ulMaxHeight, hDisplay->stNewInfo.stVipMemSettings.bSupportInterlaced?'i':'p',
				hDisplay->stNewInfo.stVipMemSettings.ulMemcId));
			hDisplay->stNewInfo.stDirty.stBits.bStgEnable = BVDC_P_DIRTY;
		}
#endif
	}

	BDBG_LEAVE(BVDC_Display_SetStgConfiguration);

	return BERR_SUCCESS;
#endif
}

/*************************************************************************
 *	BVDC_Display_GetStgConfiguration
 *************************************************************************/
BERR_Code BVDC_Display_GetStgConfiguration
	( const BVDC_Display_Handle        hDisplay,
	  bool                            *pbEnable,
	  BVDC_Display_StgSettings        *pStgSettings )
{
	BDBG_ENTER(BVDC_Display_GetStgConfiguration);

#if (BVDC_P_SUPPORT_STG == 0)
	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(pbEnable);
	BSTD_UNUSED(pStgSettings);

	BDBG_LEAVE(BVDC_Display_GetStgConfiguration);
	return BERR_NOT_SUPPORTED;

#else

	if(hDisplay == NULL)
	{
		BDBG_ERR(("Invalid parameter"));
		BDBG_LEAVE(BVDC_Display_GetStgConfiguration);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if(pbEnable)
	{
		*pbEnable = hDisplay->stCurInfo.bEnableStg;
	}

	if(pStgSettings)
	{
		pStgSettings->bNonRealTime = hDisplay->stCurInfo.bStgNonRealTime;
#if BVDC_P_SUPPORT_VIP
		pStgSettings->vip.hHeap = hDisplay->stCurInfo.hVipHeap;
		BKNI_Memcpy(&pStgSettings->vip.stMemSettings, &hDisplay->stCurInfo.stVipMemSettings, sizeof(BVDC_VipMemConfigSettings));
#endif
		pStgSettings->ulStcSnapshotLoAddr = hDisplay->stCurInfo.ulStcSnapshotLoAddr;
		pStgSettings->ulStcSnapshotHiAddr = hDisplay->stCurInfo.ulStcSnapshotHiAddr;
	}

	BDBG_LEAVE(BVDC_Display_GetStgConfiguration);
	return BERR_SUCCESS;
#endif
}


/*************************************************************************
 *	BVDC_Display_GetInterruptName
 *************************************************************************/
BERR_Code BVDC_Display_GetInterruptName
	( BVDC_Display_Handle             hDisplay,
	  const BAVC_Polarity             eFieldId,
	  BINT_Id                        *pInterruptName )
{
	BDBG_ENTER(BVDC_Display_GetInterruptName);

	if (!hDisplay)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BDBG_OBJECT_ASSERT(hDisplay->hCompositor, BVDC_CMP);

	if(pInterruptName)
	{
		*pInterruptName = BRDC_Slot_GetIntId(
			hDisplay->hCompositor->ahSlot[eFieldId == BAVC_Polarity_eBotField]);
	}

	BDBG_LEAVE(BVDC_Display_GetInterruptName);
	return BERR_SUCCESS;
}

/*************************************************************************
 *	BVDC_Display_SetRfmConfiguration
 *************************************************************************/
BERR_Code BVDC_Display_SetRfmConfiguration
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulRfmOutput,
	  BVDC_RfmOutput                   eRfmOutput,
	  uint32_t                         ulConstantValue )
{
	BDBG_ENTER(BVDC_Display_SetRfmConfiguration);

#if (BVDC_P_SUPPORT_RFM_OUTPUT == 0)
	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(ulRfmOutput);
	BSTD_UNUSED(eRfmOutput);
	BSTD_UNUSED(ulConstantValue);

	BDBG_ERR(("This chip does not support RF output."));
	BDBG_LEAVE(BVDC_Display_SetRfmConfiguration);
	return BERR_NOT_SUPPORTED;
#else
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(ulRfmOutput != BVDC_Rfm_0)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Bypass path does not support Rfm */
	if(hDisplay->bIsBypass)
	{
		return BERR_TRACE(BVDC_ERR_INVALID_RFM_PATH);
	}

	hDisplay->stNewInfo.eRfmOutput = eRfmOutput;
	hDisplay->stNewInfo.ulRfmConst =
		(eRfmOutput == BVDC_RfmOutput_eUnused) ? 0 : ulConstantValue;
	if((hDisplay->stCurInfo.eRfmOutput != eRfmOutput) ||
	   (hDisplay->stCurInfo.ulRfmConst != ulConstantValue) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bRfm = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bDacSetting = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bAcp        = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetRfmConfiguration);
	return BERR_SUCCESS;
#endif
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetRfmConfiguration
 *************************************************************************/
BERR_Code BVDC_Display_GetRfmConfiguration
	( const BVDC_Display_Handle        hDisplay,
	  uint32_t                         ulRfmOutput,
	  BVDC_RfmOutput                  *peRfmOutput,
	  uint32_t                        *pulConstantValue )
{
	BDBG_ENTER(BVDC_Display_GetRfmConfiguration);

#if (BVDC_P_SUPPORT_RFM_OUTPUT == 0)
	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(ulRfmOutput);
	BSTD_UNUSED(peRfmOutput);
	BSTD_UNUSED(pulConstantValue);

	BDBG_ERR(("This chip does not support RF output."));

	BDBG_LEAVE(BVDC_Display_GetRfmConfiguration);
	return BERR_NOT_SUPPORTED;
#else
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(ulRfmOutput != BVDC_Rfm_0)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if(peRfmOutput)
	{
		*peRfmOutput = hDisplay->stCurInfo.eRfmOutput;
	}

	if(pulConstantValue)
	{
		*pulConstantValue = hDisplay->stCurInfo.ulRfmConst;
	}

	BDBG_LEAVE(BVDC_Display_GetRfmConfiguration);
	return BERR_SUCCESS;
#endif
}
#endif

/*************************************************************************
 *	BVDC_Display_SetMpaa
 *************************************************************************/
BERR_Code BVDC_Display_SetMpaaDecimation
	( BVDC_Display_Handle              hDisplay,
	  BVDC_MpaaDeciIf                  eMpaaDeciIf,
	  uint32_t                         ulOutPorts,
	  bool                             bEnable )
{
	BDBG_ENTER(BVDC_Display_SetMpaaDecimation);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Bypass path does not support MPAA */
	if(hDisplay->bIsBypass)
	{
		return BERR_TRACE(BVDC_ERR_INVALID_MPAA_PATH);
	}

	if ((BVDC_MpaaDeciIf_eUnused <= eMpaaDeciIf) ||
		((~hDisplay->aulMpaaDeciIfPortMask[eMpaaDeciIf]) & ulOutPorts))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if (bEnable)
	{
		hDisplay->stNewInfo.aulEnableMpaaDeci[eMpaaDeciIf] |= ulOutPorts;
	}
	else
	{
		hDisplay->stNewInfo.aulEnableMpaaDeci[eMpaaDeciIf] &= ~ulOutPorts;
	}

	if((hDisplay->stCurInfo.aulEnableMpaaDeci[eMpaaDeciIf] != hDisplay->stNewInfo.aulEnableMpaaDeci[eMpaaDeciIf]) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		if (BVDC_MpaaDeciIf_eComponent == eMpaaDeciIf)
			hDisplay->stNewInfo.stDirty.stBits.bMpaaComp = BVDC_P_DIRTY;
		else /* BVDC_MpaaDeciIf_eHdmi == eMpaaDeciIf */
			hDisplay->stNewInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetMpaaDecimation);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetMpaaDecimation
 *************************************************************************/
BERR_Code BVDC_Display_GetMpaaDecimation
	( const BVDC_Display_Handle        hDisplay,
	  BVDC_MpaaDeciIf                  eMpaaDeciIf,
	  uint32_t                         ulOutPort,
	  bool                            *pbEnable )
{
	BDBG_ENTER(BVDC_Display_GetMpaaDecimation);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if ((BVDC_MpaaDeciIf_eUnused <= eMpaaDeciIf) ||
		((~hDisplay->aulMpaaDeciIfPortMask[eMpaaDeciIf]) & ulOutPort) ||
		(NULL == pbEnable))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	*pbEnable = hDisplay->stCurInfo.aulEnableMpaaDeci[eMpaaDeciIf] & ulOutPort;

	BDBG_LEAVE(BVDC_Display_GetMpaaDecimation);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *	BVDC_Display_SetAspectRatio
 *************************************************************************/
BERR_Code BVDC_Display_SetAspectRatio
	( BVDC_Display_Handle              hDisplay,
	  BFMT_AspectRatio                 eAspectRatio )
{
	BDBG_ENTER(BVDC_Display_SetAspectRatio);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	BDBG_OBJECT_ASSERT(hDisplay->hCompositor, BVDC_CMP);

	hDisplay->stNewInfo.eAspectRatio = eAspectRatio;
	if((hDisplay->stCurInfo.eAspectRatio != eAspectRatio) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bAspRatio = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetAspectRatio);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_SetSampleAspectRatio
 *************************************************************************/
BERR_Code BVDC_Display_GetSampleAspectRatio
	( BVDC_Display_Handle              hDisplay,
	  uint16_t                        *puiSampleAspectRatioX,
	  uint16_t                        *puiSampleAspectRatioY )
{
	BDBG_ENTER(BVDC_Display_GetSampleAspectRatio);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	BDBG_OBJECT_ASSERT(hDisplay->hCompositor, BVDC_CMP);

	if(puiSampleAspectRatioX)
	{
		*puiSampleAspectRatioX = hDisplay->stCurInfo.uiSampleAspectRatioX;
	}

	if(puiSampleAspectRatioY)
	{
		*puiSampleAspectRatioY = hDisplay->stCurInfo.uiSampleAspectRatioY;
	}

	BDBG_LEAVE(BVDC_Display_GetSampleAspectRatio);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *	BVDC_Display_SetSampleAspectRatio
 *************************************************************************/
BERR_Code BVDC_Display_SetSampleAspectRatio
	( BVDC_Display_Handle              hDisplay,
	  uint16_t                         uiSampleAspectRatioX,
	  uint16_t                         uiSampleAspectRatioY )
{
	BDBG_ENTER(BVDC_Display_SetSampleAspectRatio);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	BDBG_OBJECT_ASSERT(hDisplay->hCompositor, BVDC_CMP);

	hDisplay->stNewInfo.eAspectRatio = BFMT_AspectRatio_eSAR;
	hDisplay->stNewInfo.uiSampleAspectRatioX = uiSampleAspectRatioX;
	hDisplay->stNewInfo.uiSampleAspectRatioY = uiSampleAspectRatioY;
	if ((hDisplay->stCurInfo.eAspectRatio != BFMT_AspectRatio_eSAR) ||
	    (hDisplay->stCurInfo.uiSampleAspectRatioX != uiSampleAspectRatioX) ||
	    (hDisplay->stCurInfo.uiSampleAspectRatioY != uiSampleAspectRatioY) ||
	    (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bAspRatio = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetSampleAspectRatio);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_SetAspectRatioRectangle
 *************************************************************************/
BERR_Code BVDC_Display_SetAspectRatioCanvasClip
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulLeft,
	  uint32_t                         ulRight,
	  uint32_t                         ulTop,
	  uint32_t                         ulBottom )
{
	BDBG_ENTER(BVDC_Display_SetAspectRatioCanvasClip);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	BDBG_OBJECT_ASSERT(hDisplay->hCompositor, BVDC_CMP);

	hDisplay->stNewInfo.stAspRatRectClip.ulLeft   = ulLeft;
	hDisplay->stNewInfo.stAspRatRectClip.ulRight  = ulRight;
	hDisplay->stNewInfo.stAspRatRectClip.ulTop    = ulTop;
	hDisplay->stNewInfo.stAspRatRectClip.ulBottom = ulBottom;
	if ((hDisplay->stCurInfo.stAspRatRectClip.ulLeft   != ulLeft) ||
	    (hDisplay->stCurInfo.stAspRatRectClip.ulRight  != ulRight) ||
	    (hDisplay->stCurInfo.stAspRatRectClip.ulTop    != ulTop) ||
	    (hDisplay->stCurInfo.stAspRatRectClip.ulBottom != ulBottom) ||
	    (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bAspRatio = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetAspectRatioCanvasClip);
	return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetAspectRatio
 *************************************************************************/
BERR_Code BVDC_Display_GetAspectRatio
	( const BVDC_Display_Handle        hDisplay,
	  BFMT_AspectRatio                *peAspectRatio )
{
	BDBG_ENTER(BVDC_Display_GetAspectRatio);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	BDBG_OBJECT_ASSERT(hDisplay->hCompositor, BVDC_CMP);

	if(peAspectRatio)
	{
		*peAspectRatio = hDisplay->stCurInfo.eAspectRatio;
	}

	BDBG_LEAVE(BVDC_Display_GetAspectRatio);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *	BVDC_Display_SetDropFrame
 *************************************************************************/
BERR_Code BVDC_Display_SetDropFrame
	( BVDC_Display_Handle              hDisplay,
	  BVDC_Mode                        eDropFrame )
{
	BDBG_ENTER(BVDC_Display_SetDropFrame);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	hDisplay->stNewInfo.eDropFrame = eDropFrame;
	if((hDisplay->stCurInfo.eDropFrame != eDropFrame) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bDropFrame = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetDropFrame);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetDropFrame
 *************************************************************************/
BERR_Code BVDC_Display_GetDropFrame
	( const BVDC_Display_Handle        hDisplay,
	  BVDC_Mode                       *peDropFrame )
{
	BDBG_ENTER(BVDC_Display_GetDropFrame);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(peDropFrame)
	{
		*peDropFrame = hDisplay->stCurInfo.eDropFrame;
	}

	BDBG_LEAVE(BVDC_Display_GetDropFrame);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *	BVDC_Display_SetTimebase
 *************************************************************************/
BERR_Code BVDC_Display_SetTimebase
	( BVDC_Display_Handle              hDisplay,
	  BAVC_Timebase                    eTimeBase )
{
	BDBG_ENTER(BVDC_Display_SetTimebase);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Note: which timebase the bypass video 656 output is locked to is
	 * out of control of VDC PI; it's up to the upper level software to
	 * determine what timebase to use for vec 656 output clock by
	 * programming CLK_MISC register and VCXO_x_RM block; */
	if(hDisplay->bIsBypass)
	{
		BDBG_WRN(("Application needs to program the timebase for bypass video"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if(eTimeBase >= BAVC_Timebase_eMax)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	hDisplay->stNewInfo.eTimeBase = eTimeBase;
	if((hDisplay->stCurInfo.eTimeBase != eTimeBase) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bTimeBase = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetTimebase);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *	BVDC_Display_GetTimebase
 *************************************************************************/
BERR_Code BVDC_Display_GetTimebase
	( const BVDC_Display_Handle        hDisplay,
	  BAVC_Timebase                   *peTimeBase )
{
	BDBG_ENTER(BVDC_Display_GetTimebase);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Note: which timebase the bypass video 656 output is locked to is
	 * out of control of VDC PI; it's up to the upper level software to
	 * determine what timebase to use for vec 656 output clock by
	 * programming CLK_MISC register and VCXO_x_RM block; */
	if(hDisplay->bIsBypass)
	{
		BDBG_ERR(("Application needs to program the timebase for bypass video"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if(peTimeBase)
	{
		*peTimeBase = hDisplay->stCurInfo.eTimeBase;
	}

	BDBG_LEAVE(BVDC_Display_GetTimebase);
	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 *	BVDC_Display_SetCallbackSettings
 *************************************************************************/
BERR_Code BVDC_Display_SetCallbackSettings
	( BVDC_Display_Handle                  hDisplay,
	  const BVDC_Display_CallbackSettings *pSettings )
{
	BDBG_ENTER(BVDC_Display_SetCallbackSettings);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(!pSettings)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	hDisplay->stNewInfo.stCallbackSettings.stMask = pSettings->stMask;
	if((hDisplay->stCurInfo.stCallbackSettings.stMask.bCrc        != pSettings->stMask.bCrc) ||
	   (hDisplay->stCurInfo.stCallbackSettings.stMask.bRateChange != pSettings->stMask.bRateChange) ||
	   (hDisplay->stCurInfo.stCallbackSettings.stMask.bPerVsync   != pSettings->stMask.bPerVsync) ||
	   (hDisplay->stCurInfo.stCallbackSettings.stMask.bStgPictureId!= pSettings->stMask.bStgPictureId) ||
	   (hDisplay->stCurInfo.stCallbackSettings.stMask.bCableDetect!= pSettings->stMask.bCableDetect) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bCallback = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetCallbackSettings);
	return BERR_SUCCESS;
}

/*************************************************************************
 *	BVDC_Display_GetCallbackSettings
 *************************************************************************/
BERR_Code BVDC_Display_GetCallbackSettings
	( BVDC_Display_Handle                  hDisplay,
	  BVDC_Display_CallbackSettings       *pSettings )
{
	BDBG_ENTER(BVDC_Display_GetCallbackSettings);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if (pSettings)
	{
		*pSettings = hDisplay->stCurInfo.stCallbackSettings;
	}

	BDBG_LEAVE(BVDC_Display_GetCallbackSettings);
	return BERR_SUCCESS;
}

/*************************************************************************
 * BVDC_Display_InstallCallback
 *************************************************************************/
BERR_Code BVDC_Display_InstallCallback
	( BVDC_Display_Handle              hDisplay,
	  const BVDC_CallbackFunc_isr      pfCallback,
	  void                            *pvParm1,
	  int                              iParm2 )
{
	BDBG_ENTER(BVDC_Display_InstallCallback);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Store the new infos */
	hDisplay->stNewInfo.pfGenericCallback = pfCallback;
	hDisplay->stNewInfo.pvGenericParm1    = pvParm1;
	hDisplay->stNewInfo.iGenericParm2     = iParm2;
	if((hDisplay->stCurInfo.pfGenericCallback != pfCallback) ||
	   (hDisplay->stCurInfo.pvGenericParm1    != pvParm1)    ||
	   (hDisplay->stCurInfo.iGenericParm2     != iParm2)     ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bCallbackFunc = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_InstallCallback);
	return BERR_SUCCESS;
}

/*************************************************************************
 * BVDC_Display_EnableColorCorrection
 *************************************************************************/
BERR_Code BVDC_Display_EnableColorCorrection
	( BVDC_Display_Handle              hDisplay,
	  bool                             bEnable )
{
	BDBG_ENTER(BVDC_Display_EnableColorCorrection);

	BDBG_LEAVE(BVDC_Display_EnableColorCorrection);

	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(bEnable);

	BDBG_ERR((" !!! Not supported feature!"));

	return BERR_SUCCESS;
}

/*************************************************************************
 * BVDC_Display_SetColorCorrectionTable
 *************************************************************************/
BERR_Code BVDC_Display_SetColorCorrectionTable
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulGammaTableId,
	  uint32_t                         ulColorTempId )
{
	BDBG_ENTER(BVDC_Display_SetColorCorrectionTable);

	BDBG_LEAVE(BVDC_Display_SetColorCorrectionTable);


	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(ulGammaTableId);
	BSTD_UNUSED(ulColorTempId);

	BDBG_ERR((" !!! Not supported feature!"));

	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 * BVDC_Display_GetColorCorrectionTable
 *************************************************************************/
BERR_Code BVDC_Display_GetColorCorrectionTable
	( const BVDC_Display_Handle        hDisplay,
	  bool                            *pbEnable,
	  uint32_t                        *pulGammaTableId,
	  uint32_t                        *pulColorTempId )
{

	BDBG_ENTER(BVDC_Display_GetColorCorrectionTable);

	BDBG_LEAVE(BVDC_Display_GetColorCorrectionTable);

	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(pbEnable);
	BSTD_UNUSED(pulGammaTableId);
	BSTD_UNUSED(pulColorTempId);

	BDBG_ERR((" !!! Not supported feature!"));

	return BERR_SUCCESS;
}
#endif

/*************************************************************************
 * BVDC_Display_LoadColorCorrectionTable
 *************************************************************************/
BERR_Code BVDC_Display_LoadColorCorrectionTable
	( BVDC_Display_Handle              hDisplay,
	  const uint32_t                  *pulCcbTable )
{
	BDBG_ENTER(BVDC_Display_LoadColorCorrectionTable);
	BDBG_LEAVE(BVDC_Display_LoadColorCorrectionTable);

	BSTD_UNUSED(hDisplay);
	BSTD_UNUSED(pulCcbTable);

	BDBG_ERR((" !!! Not supported feature!"));

	return BERR_SUCCESS;
}

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_SetDvoConfiguration
	( BVDC_Display_Handle              hDisplay,
	  const BVDC_Display_DvoSettings  *pDvoSettings )
{
	BVDC_P_DisplayInfo *pNewInfo;

	BDBG_ENTER(BVDC_Display_SetDvoConfiguration);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	pNewInfo = &hDisplay->stNewInfo;

	pNewInfo->stDvoCfg = *pDvoSettings;
	if((hDisplay->stCurInfo.stDvoCfg.eDePolarity    != pDvoSettings->eDePolarity   ) ||
	   (hDisplay->stCurInfo.stDvoCfg.eHsPolarity    != pDvoSettings->eHsPolarity   ) ||
	   (hDisplay->stCurInfo.stDvoCfg.eVsPolarity    != pDvoSettings->eVsPolarity   ) ||
	   (hDisplay->stCurInfo.stDvoCfg.b8BitPanel     != pDvoSettings->b8BitPanel    ) ||
	   (hDisplay->stCurInfo.stDvoCfg.stSpreadSpectrum.bEnable != pDvoSettings->stSpreadSpectrum.bEnable) ||
	   (hDisplay->stCurInfo.stDvoCfg.stSpreadSpectrum.ulFrequency != pDvoSettings->stSpreadSpectrum.ulFrequency) ||
	   (hDisplay->stCurInfo.stDvoCfg.stSpreadSpectrum.ulDelta != pDvoSettings->stSpreadSpectrum.ulDelta) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		/* Changes these require the DVO timing block to reconfig */
		hDisplay->stNewInfo.stDirty.stBits.bHdmiEnable = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bHdmiCsc= BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetDvoConfiguration);
	return BERR_SUCCESS;

}

/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetDvoConfiguration
	( const BVDC_Display_Handle        hDisplay,
	  BVDC_Display_DvoSettings        *pDvoSettings )
{
	BDBG_ENTER(BVDC_Display_GetDvoConfiguration);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(pDvoSettings)
	{
		*pDvoSettings = hDisplay->stCurInfo.stDvoCfg;
	}

	BDBG_LEAVE(BVDC_Display_GetDvoConfiguration);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_SetDvoAttenuationRGB
	( BVDC_Display_Handle                hDisplay,
	  int32_t                            nAttentuationR,
	  int32_t                            nAttentuationG,
	  int32_t                            nAttentuationB,
	  int32_t                            nOffsetR,
	  int32_t                            nOffsetG,
	  int32_t                            nOffsetB )
{
	int32_t ulShiftBits;

	BDBG_ENTER(BVDC_Display_SetDvoAttenuationRGB);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	ulShiftBits = BVDC_P_CSC_DVO_CX_F_BITS - 11;

	hDisplay->stNewInfo.lDvoAttenuationR = nAttentuationR << ulShiftBits;
	hDisplay->stNewInfo.lDvoAttenuationG = nAttentuationG << ulShiftBits;
	hDisplay->stNewInfo.lDvoAttenuationB = nAttentuationB << ulShiftBits;
	hDisplay->stNewInfo.lDvoOffsetR      = nOffsetR       << ulShiftBits;
	hDisplay->stNewInfo.lDvoOffsetG      = nOffsetG       << ulShiftBits;
	hDisplay->stNewInfo.lDvoOffsetB      = nOffsetB       << ulShiftBits;
	if((hDisplay->stCurInfo.lDvoAttenuationR != nAttentuationR << ulShiftBits) ||
	   (hDisplay->stCurInfo.lDvoAttenuationG != nAttentuationG << ulShiftBits) ||
	   (hDisplay->stCurInfo.lDvoAttenuationB != nAttentuationB << ulShiftBits) ||
	   (hDisplay->stCurInfo.lDvoOffsetR      != nOffsetR       << ulShiftBits) ||
	   (hDisplay->stCurInfo.lDvoOffsetG      != nOffsetG       << ulShiftBits) ||
	   (hDisplay->stCurInfo.lDvoOffsetB      != nOffsetB       << ulShiftBits) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bHdmiCsc= BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetDvoAttenuationRGB);
	return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/*************************************************************************
 *
 *************************************************************************/
BERR_Code BVDC_Display_GetDvoAttenuationRGB
	( BVDC_Display_Handle                hDisplay,
	  int32_t                           *plAttenuationR,
	  int32_t                           *plAttenuationG,
	  int32_t                           *plAttenuationB,
	  int32_t                           *plOffsetR,
	  int32_t                           *plOffsetG,
	  int32_t                           *plOffsetB )
{
	int32_t ulShiftBits;

	BDBG_ENTER(BVDC_Display_GetDvoAttenuationRGB);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	ulShiftBits = BVDC_P_CSC_DVO_CX_F_BITS - 11;

	(*plAttenuationR) = hDisplay->stNewInfo.lDvoAttenuationR >> ulShiftBits;
	(*plAttenuationG) = hDisplay->stNewInfo.lDvoAttenuationG >> ulShiftBits;
	(*plAttenuationB) = hDisplay->stNewInfo.lDvoAttenuationB >> ulShiftBits;
	(*plOffsetR)      = hDisplay->stNewInfo.lDvoOffsetR      >> ulShiftBits;
	(*plOffsetG)      = hDisplay->stNewInfo.lDvoOffsetG      >> ulShiftBits;
	(*plOffsetB)      = hDisplay->stNewInfo.lDvoOffsetB      >> ulShiftBits;

	BDBG_LEAVE(BVDC_Display_GetDvoAttenuationRGB);
	return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Display_SetDvoColorMatrix
	( BVDC_Display_Handle              hDisplay,
	  bool                             bOverride,
	  const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
	  uint32_t                         ulShift )
{
	BDBG_ENTER(BVDC_Display_SetDvoColorMatrix);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* set new values */
	hDisplay->stNewInfo.bUserCsc = bOverride;
	if(bOverride)
	{
		uint32_t ulIndex;
		hDisplay->stNewInfo.ulUserShift = ulShift;
		for(ulIndex = 0; ulIndex < BVDC_CSC_COEFF_COUNT; ulIndex++)
		{
			hDisplay->stNewInfo.pl32_Matrix[ulIndex] = pl32_Matrix[ulIndex];
		}
	}

	/* Set display dirty bit */
	hDisplay->stNewInfo.stDirty.stBits.bHdmiCsc = BVDC_P_DIRTY;

	BDBG_LEAVE(BVDC_Display_SetDvoColorMatrix);
	return BERR_SUCCESS;
}
#endif

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Display_GetDvoColorMatrix
	( BVDC_Display_Handle              hDisplay,
	  bool                            *pbOverride,
	  int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
	  uint32_t                        *pulShift )
{
	BDBG_ENTER(BVDC_Display_GetDvoColorMatrix);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(pbOverride)
	{
		*pbOverride = hDisplay->stCurInfo.bUserCsc;
	}

	if(hDisplay->stCurInfo.bUserCsc)
	{
		uint32_t ulIndex;
		for(ulIndex = 0; ulIndex < BVDC_CSC_COEFF_COUNT; ulIndex++)
		{
			pl32_Matrix[ulIndex] = hDisplay->stCurInfo.pl32_Matrix[ulIndex];
		}

		if(pulShift)
		{
			*pulShift = hDisplay->stCurInfo.ulUserShift;
		}
	}
	else
	{
		BKNI_EnterCriticalSection();
		BVDC_P_Csc_ToMatrix_isr(pl32_Matrix, &hDisplay->stDvoCscMatrix.stCscCoeffs,
			BVDC_P_FIX_POINT_SHIFT);
		BKNI_LeaveCriticalSection();

		if(pulShift)
		{
			*pulShift = BVDC_P_FIX_POINT_SHIFT;
		}
	}

	BDBG_LEAVE(BVDC_Display_GetDvoColorMatrix);
	return BERR_SUCCESS;

}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Display_SetAlignment
	( BVDC_Display_Handle                           hDisplay,
	  BVDC_Display_Handle                           hTargetDisplay,
	  const BVDC_Display_AlignmentSettings         *pAlignSettings )
{
	BDBG_ENTER(BVDC_Display_SetAlignment);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);


	/* VEC alignment is per user request. As long as hTargetDisplay and
	 * pAlignSettings are not NULL we will start alignment process.
	 */

	/* set new values */
	if(hTargetDisplay && pAlignSettings)
	{
		hDisplay->stNewInfo.hTargetDisplay = hTargetDisplay;
		hDisplay->stNewInfo.stAlignCfg     = *pAlignSettings;
	}
	else
	{
		hDisplay->stNewInfo.hTargetDisplay = NULL;
	}

	/* Set display dirty bit */
	hDisplay->stNewInfo.stDirty.stBits.bAlignment = BVDC_P_DIRTY;

	BDBG_LEAVE(BVDC_Display_SetAlignment);
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Display_GetAlignment
	( const BVDC_Display_Handle                  hDisplay,
	  BVDC_Display_Handle                       *phTargetDisplay,
	  BVDC_Display_AlignmentSettings            *pAlignSettings )
{
	BDBG_ENTER(BVDC_Display_GetAlignment);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* set new values */
	if(pAlignSettings)
	{
		*pAlignSettings = hDisplay->stCurInfo.stAlignCfg;
	}

	if(phTargetDisplay)
	{
		*phTargetDisplay = hDisplay->stNewInfo.hTargetDisplay;
	}

	BDBG_LEAVE(BVDC_Display_GetAlignment);
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Display_SetOrientation
	( const BVDC_Display_Handle        hDisplay,
	  BFMT_Orientation                 eOrientation )
{
	BDBG_ENTER(BVDC_Display_SetOrientation);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	hDisplay->stNewInfo.eOrientation = eOrientation;
	hDisplay->hCompositor->stNewInfo.eOrientation = eOrientation;

	if((hDisplay->stCurInfo.eOrientation != eOrientation) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.b3DSetting = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bAspRatio = BVDC_P_DIRTY;

#if BVDC_P_SUPPORT_3D_VIDEO
		/* CMP_0_CMP_CTRL.BVB_VIDEO is shared by main, pip and graphics
		 * window on the CMP. Graphics has no delay, main and pip video
		 * window have different delay. So we treat display orientation
		 * change as format change to flush all the buffers in video
		 * windows to avoid BVN errors.
		 */
		hDisplay->stNewInfo.stDirty.stBits.bTiming = BVDC_P_DIRTY;
		hDisplay->stNewInfo.stDirty.stBits.bAcp    = BVDC_P_DIRTY;
#endif

	}

	BDBG_LEAVE(BVDC_Display_SetOrientation);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Display_GetOrientation
	( const BVDC_Display_Handle        hDisplay,
	  BFMT_Orientation                *peOrientation )
{
	BDBG_ENTER(BVDC_Display_GetOrientation);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(peOrientation)
	{
		*peOrientation = hDisplay->stCurInfo.eOrientation;
	}

	BDBG_LEAVE(BVDC_Display_GetOrientation);
	return BERR_SUCCESS;
}
#endif


/***************************************************************************
 *
 */
BERR_Code BVDC_Display_Set3dSourceBufferSelect
	( const BVDC_Display_Handle        hDisplay,
	  BVDC_3dSourceBufferSelect        e3dSrcBufSel )
{
	BDBG_ENTER(BVDC_Display_Set3dSourceBufferSelect);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

#if (!BVDC_P_MFD_SUPPORT_3D_VIDEO)
	if(e3dSrcBufSel != BVDC_3dSourceBufferSelect_eNormal)
	{
		BDBG_ERR(("3D video is not supported!"));
	}
#endif

	hDisplay->stNewInfo.e3dSrcBufSel = e3dSrcBufSel;
	if((hDisplay->stCurInfo.e3dSrcBufSel != e3dSrcBufSel) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.b3DSetting = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_Set3dSourceBufferSelect);
	return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Display_Get3dSourceBufferSelect
	( const BVDC_Display_Handle        hDisplay,
	  BVDC_3dSourceBufferSelect       *pe3dSrcBufSel )
{
	BDBG_ENTER(BVDC_Display_Get3dSourceBufferSelect);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(pe3dSrcBufSel)
	{
		*pe3dSrcBufSel = hDisplay->stCurInfo.e3dSrcBufSel;
	}

	BDBG_LEAVE(BVDC_Display_Get3dSourceBufferSelect);
	return BERR_SUCCESS;
}
#endif

static const bool aaDacOutput_ChVfUsage_Tbl[][3] =
{
/*   Ch0,   Ch1,   Ch2 */
	{true,  false, false}, /* BVDC_DacOutput_eSVideo_Luma */
	{false, true,  true }, /* BVDC_DacOutput_eSVideo_Chroma */
	{true,  true,  true }, /* BVDC_DacOutput_eComposite */
	{false, true,  false}, /* BVDC_DacOutput_eRed */
	{true,  false, false}, /* BVDC_DacOutput_eGreen */
	{false, false, true }, /* BVDC_DacOutput_eBlue */
	{true,  false, false}, /* BVDC_DacOutput_eY */
	{false, true,  false}, /* BVDC_DacOutput_ePr */
	{false, false, true }, /* BVDC_DacOutput_ePb */
	{true,  false, false}, /* BVDC_DacOutput_eHsync */
	{true,  false, false}, /* BVDC_DacOutput_eGreen_NoSync */
	{false, false, false}, /* BVDC_DacOutput_eVdec0 */
	{false, false, false}, /* BVDC_DacOutput_eIfdm0 */
	{false, false, false}, /* BVDC_DacOutput_eIfdm1 */
	{false, false, false}, /* BVDC_DacOutput_eFilteredCvbs */
	{false, false, false}  /* BVDC_DacOutput_eUnused */
};

/***************************************************************************
 *
 */
BERR_Code BVDC_Display_SetVfFilter
	( BVDC_Display_Handle              hDisplay,
	  BVDC_DisplayOutput               eDisplayOutput,
	  BVDC_DacOutput                   eDacOutput,
	  bool                             bOverride,
	  const uint32_t                  *paulFilterRegs,
	  uint32_t                         ulNumFilterRegs )
{
	bool *pabUserVfFilter = NULL;
	int32_t bUseCh0, bUseCh1, bUseCh2;

	BDBG_ENTER(BVDC_Display_SetVfFilter);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if((eDisplayOutput != BVDC_DisplayOutput_eComponent) &&
	   (eDisplayOutput != BVDC_DisplayOutput_eComposite) &&
	   (eDisplayOutput != BVDC_DisplayOutput_eSVideo))
	{
		BDBG_ERR(("VF filter can only be set on Component, Cvbs, or Svideo outputs."));
		return BERR_INVALID_PARAMETER;
	}

	if (bOverride && (ulNumFilterRegs < BVDC_P_CHROMA_TABLE_SIZE))
	{
		BDBG_ERR(("Incorrect VF filter table size %d.  Table size should be %d or greater for this platform. ", ulNumFilterRegs, BVDC_P_CHROMA_TABLE_SIZE));
		return BERR_INVALID_PARAMETER;
	}

	if (!BVDC_P_DISP_IS_VALID_DISPOUTPUT_AND_DAC(eDispOutput, eDacOutput))
	{
		BDBG_ERR(("Invalid display output and dac output combination."));
		return BERR_INVALID_PARAMETER;
	}

	bUseCh0 = aaDacOutput_ChVfUsage_Tbl[eDacOutput][0];
	bUseCh1 = aaDacOutput_ChVfUsage_Tbl[eDacOutput][1];
	bUseCh2 = aaDacOutput_ChVfUsage_Tbl[eDacOutput][2];

	if(!bUseCh0 && !bUseCh1 && !bUseCh2)
	{
		BDBG_ERR(("Invalid DAC Output."));
		return BERR_INVALID_PARAMETER;
	}

	if (eDisplayOutput == BVDC_DisplayOutput_eComponent)
	{
		pabUserVfFilter = hDisplay->stNewInfo.abUserVfFilterCo;
	}
	else
	{
		pabUserVfFilter = hDisplay->stNewInfo.abUserVfFilterCvbs;
	}

	/* Program CH0 VF */
	if (bUseCh0)
	{
		pabUserVfFilter[0] = bOverride;
		if(bOverride)
		{
			if (eDisplayOutput == BVDC_DisplayOutput_eComponent)
				BKNI_Memcpy(hDisplay->stNewInfo.aaulUserVfFilterCo[0], paulFilterRegs, BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
			else
				BKNI_Memcpy(hDisplay->stNewInfo.aaulUserVfFilterCvbs[0], paulFilterRegs, BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
		}
	}

	/* Program CH1 VF */
	if (bUseCh1)
	{
		pabUserVfFilter[1] = bOverride;
		if(bOverride)
		{
			if (eDisplayOutput == BVDC_DisplayOutput_eComponent)
				BKNI_Memcpy(hDisplay->stNewInfo.aaulUserVfFilterCo[1], paulFilterRegs, BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
			else
				BKNI_Memcpy(hDisplay->stNewInfo.aaulUserVfFilterCvbs[1], paulFilterRegs, BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
		}
	}

	/* Program CH2 VF */
	if (bUseCh2)
	{
		pabUserVfFilter[2] = bOverride;
		if(bOverride)
		{
			if (eDisplayOutput == BVDC_DisplayOutput_eComponent)
				BKNI_Memcpy(hDisplay->stNewInfo.aaulUserVfFilterCo[2], paulFilterRegs, BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
			else
				BKNI_Memcpy(hDisplay->stNewInfo.aaulUserVfFilterCvbs[2], paulFilterRegs, BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
		}
	}

	/* Set display dirty bit */
	hDisplay->stNewInfo.stDirty.stBits.bVfFilter = BVDC_P_DIRTY;

	BDBG_LEAVE(BVDC_Display_SetVfFilter);
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Display_GetVfFilter
	( BVDC_Display_Handle              hDisplay,
	  BVDC_DisplayOutput               eDisplayOutput,
	  BVDC_DacOutput                   eDacOutput,
	  bool                            *pbOverride,
	  uint32_t                        *paulFilterRegs,
	  uint32_t                         ulNumFilterRegs )
{
	BVDC_P_DisplayAnlgChan *pAnlgChan = NULL;
	uint32_t *paulUserVfFilter = NULL;
	int32_t lChannel;
	bool bOverride = false;

	BDBG_ENTER(BVDC_Display_GetVfFilter);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if((eDisplayOutput != BVDC_DisplayOutput_eComponent) &&
	   (eDisplayOutput != BVDC_DisplayOutput_eComposite) &&
	   (eDisplayOutput != BVDC_DisplayOutput_eSVideo))
	{
		BDBG_ERR(("VF filter can only be retrieved from Component, Cvbs, or Svideo outputs."));
		return BERR_INVALID_PARAMETER;
	}

	if (ulNumFilterRegs < BVDC_P_CHROMA_TABLE_SIZE)
	{
		BDBG_ERR(("Incorrect VF filter table size %d.  Table size should be %d or greater for this platform. ", ulNumFilterRegs, BVDC_P_CHROMA_TABLE_SIZE));
		return BERR_INVALID_PARAMETER;
	}

	if (!BVDC_P_DISP_IS_VALID_DISPOUTPUT_AND_DAC(eDispOutput, eDacOutput))
	{
		BDBG_ERR(("Invalid display output and dac output combination."));
		return BERR_INVALID_PARAMETER;
	}

	lChannel = aaDacOutput_ChVfUsage_Tbl[eDacOutput][0] ? 0 :
	           (aaDacOutput_ChVfUsage_Tbl[eDacOutput][1] ? 1 :
	            (aaDacOutput_ChVfUsage_Tbl[eDacOutput][2] ? 2 : BVDC_P_DISP_INVALID_VF_CH));

	if (lChannel == BVDC_P_DISP_INVALID_VF_CH)
	{
		BDBG_ERR(("Invalid DAC Output."));
		return BERR_INVALID_PARAMETER;
	}

	if (eDisplayOutput == BVDC_DisplayOutput_eComponent)
	{
		bOverride = hDisplay->stCurInfo.abUserVfFilterCo[lChannel];
		paulUserVfFilter = (uint32_t *)hDisplay->stCurInfo.aaulUserVfFilterCo[lChannel];
	}
	else
	{
		bOverride = hDisplay->stCurInfo.abUserVfFilterCvbs[lChannel];
		paulUserVfFilter = (uint32_t *)hDisplay->stCurInfo.aaulUserVfFilterCvbs[lChannel];
	}

	if(pbOverride)
	{
		*pbOverride = bOverride;
	}

	if(bOverride)
	{
		/* TODO: Is this necessary or can be merged with below? */
		BKNI_Memcpy(paulFilterRegs, paulUserVfFilter, BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
	}
	else
	{
		BVDC_P_Display_GetAnlgChanByOutput_isr(hDisplay, &hDisplay->stNewInfo, eDisplayOutput, &pAnlgChan);

		if(!pAnlgChan)
		{
			BDBG_ERR(("Display output does not exist.  Cannot get internal VF filter settings."));
			return BERR_INVALID_PARAMETER;
		}
		BKNI_EnterCriticalSection();
		BKNI_Memcpy(paulFilterRegs, pAnlgChan->apVfFilter[lChannel], BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
		BKNI_LeaveCriticalSection();
	}

	BDBG_LEAVE(BVDC_Display_GetVfFilter);
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Display_SetMuteMode
	( BVDC_Display_Handle              hDisplay,
	  BVDC_DisplayOutput               eDisplayOutput,
	  BVDC_MuteMode                    eMuteMode )
{
	bool bMute;

	BDBG_ENTER(BVDC_Display_SetMuteMode);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if (eMuteMode == BVDC_MuteMode_eConst)
	{
		bMute = true;
	}
	else if (eMuteMode == BVDC_MuteMode_eDisable)
	{
		bMute = false;
	}
	else
	{
		BDBG_ERR(("Selected mute mode not supported for output displays"));
		return BERR_INVALID_PARAMETER;
	}

	/* set new values */
	hDisplay->stNewInfo.abOutputMute[eDisplayOutput] = bMute;

	/* tie Cvbs and Svideo together */
	if ((eDisplayOutput == BVDC_DisplayOutput_eComposite) || (eDisplayOutput == BVDC_DisplayOutput_eSVideo))
	{
		hDisplay->stNewInfo.abOutputMute[BVDC_DisplayOutput_eSVideo] = bMute;
		hDisplay->stNewInfo.abOutputMute[BVDC_DisplayOutput_eComposite] = bMute;
	}

	/* Set display dirty bit */
	if((hDisplay->stNewInfo.abOutputMute[eDisplayOutput] != hDisplay->stCurInfo.abOutputMute[eDisplayOutput]) ||
	   (hDisplay->stNewInfo.bErrorLastSetting))
	{
		hDisplay->stNewInfo.stDirty.stBits.bOutputMute = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_Display_SetMuteMode);
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Display_GetMuteMode
	( BVDC_Display_Handle              hDisplay,
	  BVDC_DisplayOutput               eDisplayOutput,
	  BVDC_MuteMode                   *peMuteMode )
{
	bool bMute;

	BDBG_ENTER(BVDC_Display_GetMuteMode);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(peMuteMode)
	{
		bMute = hDisplay->stCurInfo.abOutputMute[eDisplayOutput];
		*peMuteMode = bMute ? BVDC_MuteMode_eConst : BVDC_MuteMode_eDisable;
	}

	BDBG_LEAVE(BVDC_Display_GetMuteMode);
	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Display_GetCapabilities
	( BVDC_Display_Handle              hDisplay,
	  BVDC_Display_Capabilities       *pCapabilities )
{
	BSTD_UNUSED(hDisplay);

	if(pCapabilities)
	{
		/* To make sure thing get initialize */
		BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));
		pCapabilities->pfIsVidfmtSupported = BVDC_P_IsVidfmtSupported;
	}

	return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Display_SetArtificialVsync
	( BVDC_Display_Handle              hDisplay,
	  bool                             bEnable,
	  uint32_t                         ulVsyncRegAddr,
	  uint32_t                         ulVsyncMask )
{
	BDBG_ENTER(BVDC_Display_SetArtificialVsync);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* set new values */
	hDisplay->stNewInfo.bArtificialVsync   = bEnable;
	hDisplay->stNewInfo.ulArtificialVsyncRegAddr = ulVsyncRegAddr;
	hDisplay->stNewInfo.ulArtificialVsyncMask = ulVsyncMask;

	/* Set display dirty bit */
	hDisplay->stNewInfo.stDirty.stBits.bMiscCtrl = BVDC_P_DIRTY;

	BDBG_LEAVE(BVDC_Display_SetArtificialVsync);
	return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_Display_GetArtificialVsync
	( const BVDC_Display_Handle        hDisplay,
	  bool                            *pbEnable,
	  uint32_t                        *pulVsyncRegAddr,
	  uint32_t                        *pulVsyncMask )
{
	BDBG_ENTER(BVDC_Display_GetArtificialVsync);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	if(pbEnable)
	{
		*pbEnable = hDisplay->stCurInfo.bArtificialVsync;
	}

	if(pulVsyncRegAddr)
	{
		*pulVsyncRegAddr = hDisplay->stCurInfo.ulArtificialVsyncRegAddr;
	}

	if(pulVsyncMask)
	{
		*pulVsyncMask = hDisplay->stCurInfo.ulArtificialVsyncMask;
	}

	BDBG_LEAVE(BVDC_Display_GetArtificialVsync);
	return BERR_SUCCESS;
}
#endif

/***************************************************************************
 * For new soft transcode buffer interface.
 * Extract a picture buffer from VDC encoder display to deliver to video encoder.
 */
void BVDC_Display_GetBuffer_isr
(   BVDC_Display_Handle        hDisplay,
    BAVC_EncodePictureBuffer  *pEncodePicture)
{
	BDBG_ENTER(BVDC_Display_GetBuffer_isr);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	BDBG_ASSERT(pEncodePicture);

	BKNI_Memset_isr((void*)pEncodePicture, 0, sizeof(BAVC_EncodePictureBuffer));
#if BVDC_P_SUPPORT_VIP
	if(hDisplay->hVip) {
		BVDC_P_Vip_GetBuffer_isr(hDisplay->hVip, pEncodePicture);
	}
#endif
	BDBG_LEAVE(BVDC_Display_GetBuffer_isr);
}

/***************************************************************************
 * For new soft transcode buffer interface.
 * Return the encoded picture back to VDC display.
 */
void BVDC_Display_ReturnBuffer_isr
(   BVDC_Display_Handle        hDisplay,
    const BAVC_EncodePictureBuffer  *pEncodePicture)
{
	BDBG_ENTER(BVDC_Display_ReturnBuffer_isr);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

#if BVDC_P_SUPPORT_VIP
	if(hDisplay->hVip) {
		BVDC_P_Vip_ReturnBuffer_isr(hDisplay->hVip, pEncodePicture);
	}
#else
	BSTD_UNUSED(pEncodePicture);
#endif

	BDBG_LEAVE(BVDC_Display_ReturnBuffer_isr);
}

/* End of File */
