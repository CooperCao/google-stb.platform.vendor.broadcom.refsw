/***************************************************************************
 *     Copyright (c) 2004-2013, Broadcom Corporation
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
#include "bstd.h"
#include "bkni.h"
#include "bvdc.h"
#include "brdc.h"

#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_hddvi_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_mad_priv.h"
#include "bvdc_mcvp_priv.h"
#include "bvdc_dnr_priv.h"
#include "bvdc_vnetcrc_priv.h"
#include "bvdc_vnet_priv.h"

#if (0 != BVDC_P_SUPPORT_VNET_CRC)
BDBG_MODULE(BVDC_VNETCRC);
BDBG_OBJECT_ID(BVDC_VNETCRC);

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_Create
 *
 * called by BVDC_Open only
 */
BERR_Code BVDC_P_VnetCrc_Create
	( BVDC_P_VnetCrc_Handle            *phVnetCrc,
	  BVDC_P_VnetCrcId                  eVnetCrcId,
	  BREG_Handle                       hRegister,
	  BVDC_P_Resource_Handle            hResource )
{
	BVDC_P_VnetCrcContext *pVnetCrc;
	BERR_Code  eResult = BERR_OUT_OF_SYSTEM_MEMORY;

	BDBG_ENTER(BVDC_P_VnetCrc_Create);

	/* in case creation failed */
	BDBG_ASSERT(phVnetCrc);
	*phVnetCrc = NULL;

	pVnetCrc = (BVDC_P_VnetCrcContext *)
		(BKNI_Malloc(sizeof(BVDC_P_VnetCrcContext)));
	if( pVnetCrc )
	{
		/* init the context */
		BKNI_Memset((void*)pVnetCrc, 0x0, sizeof(BVDC_P_VnetCrcContext));
		BDBG_OBJECT_SET(pVnetCrc, BVDC_VNETCRC);
		pVnetCrc->eId          = eVnetCrcId;
		pVnetCrc->hRegister    = hRegister;
		pVnetCrc->ulRegOffset  = 0;
		pVnetCrc->ulSrcMuxValue = BVDC_P_VNETCRC_SRC_NULL;
		pVnetCrc->hWindow      = NULL;

		/* init the SubRul sub-module */
		BVDC_P_SubRul_Init(&(pVnetCrc->SubRul), BVDC_P_VnetCrc_MuxAddr(pVnetCrc),
			0, BVDC_P_DrainMode_eNone, BVDC_P_VNET_CRC_PROBE_RATE, hResource);

		*phVnetCrc = pVnetCrc;
		eResult = BERR_SUCCESS;
	}

	BDBG_LEAVE(BVDC_P_VnetCrc_Create);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_Destroy
 *
 * called by BVDC_Close only
 */
BERR_Code BVDC_P_VnetCrc_Destroy
	( BVDC_P_VnetCrc_Handle                hVnetCrc )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BDBG_ENTER(BVDC_P_VnetCrc_Destroy);
	BDBG_OBJECT_ASSERT(hVnetCrc, BVDC_VNETCRC);

	BDBG_OBJECT_DESTROY(hVnetCrc, BVDC_VNETCRC);
	/* it is gone afterwards !!! */
	BKNI_Free((void*)hVnetCrc);

	BDBG_LEAVE(BVDC_P_VnetCrc_Destroy);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_AcquireConnect_isr
 *
 * It is called by BVDC_Window_Validate after changing from diable to
 * enabling .
 */
BERR_Code BVDC_P_VnetCrc_AcquireConnect_isr
	( BVDC_P_VnetCrc_Handle             hVnetCrc,
	  BVDC_Window_Handle                hWindow)
{
	BERR_Code  eResult = BERR_SUCCESS;

	BDBG_ENTER(BVDC_P_VnetCrc_AcquireConnect_isr);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hVnetCrc, BVDC_VNETCRC);

	hVnetCrc->hWindow = hWindow;

	BDBG_LEAVE(BVDC_P_VnetCrc_AcquireConnect_isr);
	return BERR_TRACE(eResult);
}


/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_ReleaseConnect_isr
 *
 * It is called after window decided that vnet crc is no-longer used by HW in
 * its vnet mode (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_VnetCrc_ReleaseConnect_isr
	( BVDC_P_VnetCrc_Handle            *phVnetCrc )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BDBG_ENTER(BVDC_P_VnetCrc_ReleaseConnect_isr);

	/* handle validation */
	BDBG_OBJECT_ASSERT(*phVnetCrc, BVDC_VNETCRC);
	BDBG_OBJECT_ASSERT((*phVnetCrc)->hWindow, BVDC_WIN);

	BVDC_P_Resource_ReleaseHandle_isr(
		BVDC_P_SubRul_GetResourceHandle_isr(&(*phVnetCrc)->SubRul),
		BVDC_P_ResourceType_eVnetCrc, (void *)(*phVnetCrc));
	(*phVnetCrc)->hWindow = NULL;
	*phVnetCrc = NULL;

	BDBG_LEAVE(BVDC_P_VnetCrc_ReleaseConnect_isr);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_UpdateVnetCrcData_isr
 *
 * called by BVDC_Window_BuildRul_isr at every src vsync. It reads vnet crc
 * data from HW into crc structure
 */
static void BVDC_P_VnetCrc_UpdateVnetCrcData_isr
	( BVDC_P_VnetCrc_Handle                hVnetCrc )
{
	uint32_t ulReg;

	BDBG_OBJECT_ASSERT(hVnetCrc, BVDC_VNETCRC);

	ulReg = BREG_Read32(hVnetCrc->hRegister, BCHP_VNET_B_CRC_Y_STATUS + hVnetCrc->ulRegOffset);
	hVnetCrc->ulCrcLuma = BCHP_GET_FIELD_DATA(ulReg, VNET_B_CRC_Y_STATUS, VALUE);
	ulReg = BREG_Read32(hVnetCrc->hRegister, BCHP_VNET_B_CRC_C_STATUS + hVnetCrc->ulRegOffset);
	hVnetCrc->ulCrcChroma = BCHP_GET_FIELD_DATA(ulReg, VNET_B_CRC_C_STATUS, VALUE);
}

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_VnetCrc_BuildRul_SetEnable_isr
	( BVDC_P_VnetCrc_Handle             hVnetCrc,
	  BVDC_P_ListInfo                  *pList,
	  bool                              bEnable,
	  bool                              bClear )
{
	BDBG_ENTER(BVDC_P_VnetCrc_SetEnable_isr);

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VNET_B_CRC_CTRL) + hVnetCrc->ulRegOffset;
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(VNET_B_CRC_CTRL, PROBE_RATE, BVDC_P_VNET_CRC_PROBE_RATE) |
		BCHP_FIELD_DATA(VNET_B_CRC_CTRL, CLEAR, bClear? 1: 0) |
		BCHP_FIELD_DATA(VNET_B_CRC_CTRL, ENABLE, bEnable? 1: 0);

	BDBG_MSG(("Set VnetCrc %s", bEnable ? "true" : "false"));

	BDBG_LEAVE(BVDC_P_VnetCrc_SetEnable_isr);
	return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_BuildRul_isr
 *
 * called by BVDC_Window_BuildRul_isr at every src vsync. It builds RUL for
 * vnet crc HW module and sample the crc data.
 *
 */
void BVDC_P_VnetCrc_BuildRul_isr
	( BVDC_P_VnetCrc_Handle            *phVnetCrc,
	  BVDC_P_ListInfo                  *pList,
	  BVDC_P_State                      eVnetState,
	  BVDC_P_PicComRulInfo             *pPicComRulInfo,
	  bool                              bEnable)
{
	uint32_t                 ulRulOpsFlags;
	BVDC_P_VnetCrc_Handle    hVnetCrc;

	/* handle validation */
	hVnetCrc = *phVnetCrc;
	BDBG_OBJECT_ASSERT(hVnetCrc, BVDC_VNETCRC);
	BDBG_OBJECT_ASSERT(hVnetCrc->hWindow, BVDC_WIN);

	ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(&(hVnetCrc->SubRul),
		pPicComRulInfo->eWin, eVnetState, pList->bLastExecuted);
	if (ulRulOpsFlags & BVDC_P_RulOp_eEnable)
	{
		/* join in vnet right after enabled. note: its src mux is initialed as disabled */
		if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
		{
			BVDC_P_VnetCrc_BuildRul_SetEnable_isr(hVnetCrc, pList, true, true);

			/* no harm with duplicated call */
			BVDC_P_SubRul_JoinInVnet_isr(&(hVnetCrc->SubRul), pList);
			hVnetCrc->ulCrcChroma = 0;
			hVnetCrc->ulCrcLuma = 0;
		}
		else
		{
			/* new config has been executed in HW */
			hVnetCrc->hWindow->stCurInfo.stDirty.stBits.bVnetCrc = 0;

			if (ulRulOpsFlags & BVDC_P_RulOp_eStatisInit)
			{
				BVDC_P_VnetCrc_BuildRul_SetEnable_isr(hVnetCrc, pList, true, true);
				BVDC_P_VnetCrc_UpdateVnetCrcData_isr(hVnetCrc);
			}
			else
			{
				BVDC_P_VnetCrc_BuildRul_SetEnable_isr(hVnetCrc, pList, true, false);
			}
		}
	}
	else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
	{
		BVDC_P_SubRul_DropOffVnet_isr(&(hVnetCrc->SubRul), pList);
		BVDC_P_VnetCrc_BuildRul_SetEnable_isr(hVnetCrc, pList, false, true);
	}

	BSTD_UNUSED(bEnable);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_VnetCrc_DecideVnetMode_isr
 *
 * called by BVDC_P_VnetCrc_AcquireConnect_isr or
 * BVDC_P_Window_DecideVnetMode_isr
 * return true if vnet reconfigure is needed
 */
bool BVDC_P_VnetCrc_DecideVnetMode_isr
	( BVDC_Window_Handle                   hWindow,
	  BVDC_P_VnetCrc_Handle                hVnetCrc,
	  BVDC_P_VnetMode                     *pVnetMode)
{
	BVDC_P_VnetPatch  eVnetPatchMode;
	uint32_t  ulSrcMuxValue;
	bool  bVnetCrcBeforeCap;
	bool  bRecfgVnet = true;

	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hVnetCrc, BVDC_VNETCRC);

	ulSrcMuxValue = BVDC_P_VNETCRC_SRC_NULL;
	eVnetPatchMode = BVDC_P_VnetPatch_eNone;
	bVnetCrcBeforeCap = pVnetMode->stBits.bSclBeforeCap;

	switch (hWindow->stCurInfo.stCbSettings.eCrcModule)
	{
	case BVDC_VnetModule_eSrc:
		eVnetPatchMode = BVDC_P_VnetPatch_eFreeCh;
		ulSrcMuxValue = BVDC_P_Source_PostMuxValue(hWindow->stCurInfo.hSource);
		bVnetCrcBeforeCap = BVDC_P_ON;
		break;

	case BVDC_VnetModule_eVfd:
		if(BVDC_P_VNET_USED_CAPTURE(*pVnetMode))
		{
			eVnetPatchMode = BVDC_P_VnetPatch_eFreeCh;
			ulSrcMuxValue = BVDC_P_Feeder_PostMuxValue(hWindow->stCurResource.hPlayback);
			bVnetCrcBeforeCap = BVDC_P_OFF;
		}
		break;

	case BVDC_VnetModule_eDnr:
#if (BVDC_P_SUPPORT_DNR)
		if(BVDC_P_VNET_USED_DNR(*pVnetMode))
		{
			ulSrcMuxValue = BVDC_P_Dnr_PostMuxValue(hWindow->stCurResource.hDnr);
		}
#endif

		break;
	case BVDC_VnetModule_eMad:
#if (BVDC_P_SUPPORT_MAD)
		if(BVDC_P_VNET_USED_MAD(*pVnetMode))
		{
			ulSrcMuxValue = BVDC_P_Mad_PostMuxValue(hWindow->stCurResource.hMad32);
		}
#endif
#if (BVDC_P_SUPPORT_MCVP)
		if(BVDC_P_VNET_USED_MCVP(*pVnetMode))
		{
			ulSrcMuxValue = BVDC_P_Mcvp_PostMuxValue(hWindow->stCurResource.hMcvp);
		}
#endif
		break;

	case BVDC_VnetModule_eScl:
		if(BVDC_P_VNET_USED_SCALER(*pVnetMode))
		{
			ulSrcMuxValue = BVDC_P_Scaler_PostMuxValue(hWindow->stCurResource.hScaler);
		}
		break;

	default:
		break;
	}

	if (BVDC_P_VNETCRC_SRC_NULL != ulSrcMuxValue)
	{
		pVnetMode->stBits.bUseVnetCrc = BVDC_P_ON;
		pVnetMode->stBits.bVnetCrcBeforeCap = (bVnetCrcBeforeCap)? BVDC_P_ON : BVDC_P_OFF;
	}

	if ((!BVDC_P_VNET_USED_VNETCRC(hWindow->stVnetMode)) &&
		(!BVDC_P_VNET_USED_VNETCRC(*pVnetMode)))
	{
		/* in this case BVDC_P_VnetCrc_BuildRul_isr will not invoked */
		hWindow->stCurInfo.stDirty.stBits.bVnetCrc = BVDC_P_OFF;
		bRecfgVnet = false;
	}

	if (hWindow->stCurInfo.stDirty.stBits.bVnetCrc)
	{
		hVnetCrc->eVnetPatchMode = eVnetPatchMode;
		hVnetCrc->ulSrcMuxValue = ulSrcMuxValue;
		BDBG_MSG(("Set vnet_crc vnet, win %d, vnetMode 0x%x, SrcMuxValue %d, patch %d",
			hWindow->eId, *(unsigned int *)pVnetMode, hVnetCrc->ulSrcMuxValue, hVnetCrc->eVnetPatchMode));
	}
	return bRecfgVnet;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_VnetCrc_UnsetVnet_isr
	( BVDC_P_VnetCrc_Handle                hVnetCrc )
{
	BVDC_P_SubRul_UnsetVnet_isr(&(hVnetCrc->SubRul));
	if (BVDC_P_VNETCRC_SRC_NULL == hVnetCrc->ulSrcMuxValue ||
		!hVnetCrc->hWindow->stCurInfo.stCbSettings.stMask.bCrc)
	{
		/* vnet crc will not be back to active afterwards */
		hVnetCrc->hWindow->stCurInfo.stDirty.stBits.bVnetCrc = 0;
		hVnetCrc->ulCrcChroma = 0;
		hVnetCrc->ulCrcLuma = 0;
	}
}

#else /* #if (0 != BVDC_P_SUPPORT_BOX_DETECT) */
/***************************************************************************/
/* No support for any hist block */

#include "bvdc_errors.h"

BDBG_MODULE(BVDC_VNETCRC);
BDBG_OBJECT_ID(BVDC_VNETCRC);

BERR_Code BVDC_P_VnetCrc_Create
	( BVDC_P_VnetCrc_Handle *           phVnetCrc,
	  BVDC_P_VnetCrcId                  eVnetCrcId,
	  BREG_Handle                       hRegister,
	  BVDC_P_Resource_Handle            hResource )
{
	BDBG_ASSERT(phVnetCrc);
	*phVnetCrc = NULL;
	BSTD_UNUSED(eVnetCrcId);
	BSTD_UNUSED(hRegister);
	BSTD_UNUSED(hResource);
	return BERR_SUCCESS;
}

BERR_Code BVDC_P_VnetCrc_Destroy
	( BVDC_P_VnetCrc_Handle             hVnetCrc )
{
	BSTD_UNUSED(hVnetCrc);
	return BERR_SUCCESS;
}

BERR_Code BVDC_P_VnetCrc_AcquireConnect_isr
	( BVDC_P_VnetCrc_Handle             hVnetCrc,
	  BVDC_Window_Handle                hWindow)
{
	BSTD_UNUSED(hVnetCrc);
	BSTD_UNUSED(hWindow);
	return BERR_SUCCESS;
}

BERR_Code BVDC_P_VnetCrc_ReleaseConnect_isr
	( BVDC_P_VnetCrc_Handle            *phVnetCrc )
{
	BSTD_UNUSED(phVnetCrc);
	return BERR_SUCCESS;
}

void BVDC_P_VnetCrc_BuildRul_isr
	( BVDC_P_VnetCrc_Handle            *phVnetCrc,
	  BVDC_P_ListInfo                  *pList,
	  BVDC_P_State                      eVnetState,
	  BVDC_P_PicComRulInfo             *pPicComRulInfo,
	  bool                              bEnable)
{
	BSTD_UNUSED(phVnetCrc);
	BSTD_UNUSED(pList);
	BSTD_UNUSED(eVnetState);
	BSTD_UNUSED(pPicComRulInfo);
	BSTD_UNUSED(bEnable);
	return;
}

bool BVDC_P_VnetCrc_DecideVnetMode_isr
	( BVDC_Window_Handle                   hWindow,
	  BVDC_P_VnetCrc_Handle                hVnetCrc,
	  BVDC_P_VnetMode                     *pVnetMode)
{
	BSTD_UNUSED(hWindow);
	BSTD_UNUSED(hVnetCrc);
	BSTD_UNUSED(pVnetMode);
	return false;
}

void BVDC_P_VnetCrc_UnsetVnet_isr
	( BVDC_P_VnetCrc_Handle                hVnetCrc )
{
	BSTD_UNUSED(hVnetCrc);
}

#endif  /* #if (0 != BVDC_P_SUPPORT_BOX_DETECT) */

/* End of file. */
