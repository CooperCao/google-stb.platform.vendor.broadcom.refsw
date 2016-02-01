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
#include "bvdc_buffer_priv.h"

#if (BVDC_P_SUPPORT_XSRC)
#include "bchp_mmisc.h"
#include "bvdc_xsrc_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_vnet_priv.h"
#include "bchp_xsrc_0.h"


BDBG_MODULE(BVDC_XSRC);
BDBG_OBJECT_ID(BVDC_XSRC);


#define BVDC_P_MAKE_XSRC(pXsrc, id, channel_init)                                 \
{                                                                                 \
	(pXsrc)->ulResetRegAddr  = BCHP_MMISC_SW_INIT;                                \
	(pXsrc)->ulResetMask     = BCHP_MMISC_SW_INIT_XSRC_##id##_MASK;               \
	(pXsrc)->ulVnetResetAddr = BCHP_##channel_init;                               \
	(pXsrc)->ulVnetResetMask = BCHP_##channel_init##_XSRC_##id##_MASK;            \
	(pXsrc)->ulVnetMuxAddr   = BCHP_VNET_F_XSRC_##id##_SRC;                       \
	(pXsrc)->ulVnetMuxValue  = BCHP_VNET_B_CAP_0_SRC_SOURCE_XSRC_##id;            \
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Xsrc_Create
	( BVDC_P_Xsrc_Handle            *phXsrc,
	  BVDC_P_XsrcId                  eXsrcId,
	  BVDC_P_Resource_Handle         hResource,
	  BREG_Handle                    hReg
)
{
	BVDC_P_XsrcContext *pXsrc;

	BDBG_ENTER(BVDC_P_Xsrc_Create);

	BDBG_ASSERT(phXsrc);

	/* Use: to see messages */
	/* BDBG_SetModuleLevel("BVDC_XSRC", BDBG_eMsg); */

	/* The handle will be NULL if create fails. */
	*phXsrc = NULL;

	/* Alloc the context. */
	pXsrc = (BVDC_P_XsrcContext*)
		(BKNI_Malloc(sizeof(BVDC_P_XsrcContext)));
	if(!pXsrc)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Clear out the context and set defaults. */
	BKNI_Memset((void*)pXsrc, 0x0, sizeof(BVDC_P_XsrcContext));
	BDBG_OBJECT_SET(pXsrc, BVDC_XSRC);

	pXsrc->eId          = eXsrcId;
	pXsrc->hReg         = hReg;
	pXsrc->ulRegOffset  = 0;

#ifdef BCHP_XSRC_1_REG_START
	pXsrc->ulRegOffset = (eXsrcId - BVDC_P_XsrcId_eXsrc0) *
		(BCHP_XSRC_1_REG_START - BCHP_XSRC_0_REG_START);
#endif

	/* Init to the default filter coeffficient tables. */
	BVDC_P_GetFirCoeffs_isr(&pXsrc->pHorzFirCoeffTbl, NULL);
	BVDC_P_GetChromaFirCoeffs_isr(&pXsrc->pChromaHorzFirCoeffTbl, NULL);

	switch(eXsrcId)
	{
		case BVDC_P_XsrcId_eXsrc0:
#ifdef BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_XSRC_0_MASK
			BVDC_P_MAKE_XSRC(pXsrc, 0, MMISC_VNET_B_CHANNEL_SW_INIT);
#elif BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_XSRC_0_MASK
			BVDC_P_MAKE_XSRC(pXsrc, 0, MMISC_VNET_B_CHANNEL_SW_INIT_1);
#endif
			break;
		case BVDC_P_XsrcId_eXsrc1:
#ifdef BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_XSRC_1_MASK
			BVDC_P_MAKE_XSRC(pXsrc, 1, MMISC_VNET_B_CHANNEL_SW_INIT);
#elif BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_XSRC_1_MASK
			BVDC_P_MAKE_XSRC(pXsrc, 1, MMISC_VNET_B_CHANNEL_SW_INIT_1);
#endif
			break;
		case BVDC_P_XsrcId_eUnknown:
		default:
			BDBG_ERR(("Need to handle BVDC_P_Xsrc_eXsrc%d", eXsrcId));
			BDBG_ASSERT(0);
	}

	/* init the SubRul sub-module */
	BVDC_P_SubRul_Init(&(pXsrc->SubRul), pXsrc->ulVnetMuxAddr,
		pXsrc->ulVnetMuxValue, BVDC_P_DrainMode_eBack, 0, hResource);

	/* All done. now return the new fresh context to user. */
	*phXsrc = (BVDC_P_Xsrc_Handle)pXsrc;

	BDBG_LEAVE(BVDC_P_Xsrc_Create);
	return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Xsrc_Destroy
	( BVDC_P_Xsrc_Handle             hXsrc )
{
	BDBG_ENTER(BVDC_P_Xsrc_Destroy);
	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);

	BDBG_OBJECT_DESTROY(hXsrc, BVDC_XSRC);
	/* Release context in system memory */
	BKNI_Free((void*)hXsrc);

	BDBG_LEAVE(BVDC_P_Xsrc_Destroy);
	return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Xsrc_Init_isr
	( BVDC_P_Xsrc_Handle            hXsrc )
{
	uint32_t  ulReg;
	uint32_t  ulTaps;

	BDBG_ENTER(BVDC_P_Xsrc_Init_isr);
	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);

	hXsrc->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;

	/* Clear out shadow registers. */
	BKNI_Memset((void*)hXsrc->aulRegs, 0x0, sizeof(hXsrc->aulRegs));

	hXsrc->ulPrevSrcWidth = 0xffffffff;
	/* one value is enough to cause re-setinfo
	hXsrc->lPrevXsrcCutLeft = 0xffffffff;
	hXsrc->ulPrevXsrcCutWidth = 0xffffffff;
	hXsrc->ulPrevOutWidth = 0xffffffff;
	hXsrc->ulPrevSrcHeight = 0xffffffff;*/

	ulReg = BREG_Read32(hXsrc->hReg, BCHP_XSRC_0_HW_CONFIGURATION + hXsrc->ulRegOffset);

	hXsrc->bDeringing = BCHP_GET_FIELD_DATA(ulReg, XSRC_0_HW_CONFIGURATION, DERINGING) ? true : false;
	hXsrc->bHscl      = BCHP_GET_FIELD_DATA(ulReg, XSRC_0_HW_CONFIGURATION, HSCL_CORE) ? true : false;
#if BVDC_P_XSRC_SUPPORT_CCA
	hXsrc->bCca       = BCHP_GET_FIELD_DATA(ulReg, XSRC_0_HW_CONFIGURATION, CCA)       ? true : false;
#endif
	ulTaps            = BCHP_GET_FIELD_DATA(ulReg, XSRC_0_HW_CONFIGURATION, HORIZ_TAPS);
	switch(ulTaps)
	{
		case BCHP_XSRC_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_8_TAPS:
			hXsrc->ulHorzTaps = BVDC_P_CT_8_TAP;
			break;
		case BCHP_XSRC_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_12_TAPS:
			hXsrc->ulHorzTaps = BVDC_P_CT_12_TAP;
			break;
		case BCHP_XSRC_0_HW_CONFIGURATION_HORIZ_TAPS_HORIZ_16_TAPS:
			hXsrc->ulHorzTaps = BVDC_P_CT_16_TAP;
			break;
		default: break;
	}

#if BVDC_P_XSRC_SUPPORT_DERINE
	if(hXsrc->bDeringing)
	{
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_DERINGING) &=  ~(
			BCHP_MASK(XSRC_0_DERINGING, HORIZ_CHROMA_PASS_FIRST_RING ) |
			BCHP_MASK(XSRC_0_DERINGING, HORIZ_LUMA_PASS_FIRST_RING ) |
			BCHP_MASK(XSRC_0_DERINGING, HORIZ_CHROMA_DERINGING ) |
			BCHP_MASK(XSRC_0_DERINGING, HORIZ_LUMA_DERINGING ));

		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_DERINGING) |=  (
			BCHP_FIELD_ENUM(XSRC_0_DERINGING, HORIZ_CHROMA_PASS_FIRST_RING, ENABLE ) |
			BCHP_FIELD_ENUM(XSRC_0_DERINGING, HORIZ_LUMA_PASS_FIRST_RING, ENABLE ) |
			BCHP_FIELD_ENUM(XSRC_0_DERINGING, HORIZ_CHROMA_DERINGING, ON ) |
			BCHP_FIELD_ENUM(XSRC_0_DERINGING, HORIZ_LUMA_DERINGING,   ON ));
	}
#endif

#if BVDC_P_XSRC_SUPPORT_CCA
	if(hXsrc->bCca)
	{
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_CCA_CONFIG) &=  ~(
			BCHP_MASK(XSRC_0_CCA_CONFIG, CCA_CONFIG ));

		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_CCA_CONFIG) |=  (
			BCHP_FIELD_ENUM(XSRC_0_CCA_CONFIG, CCA_CONFIG, DEFAULT ));
	}
#endif

	/* always dither: no harm to 8-bit source; */
	BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_DITHER_LFSR_INIT) &= ~(
		BCHP_MASK(XSRC_0_HORIZ_DITHER_LFSR_INIT, SEQ) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_LFSR_INIT, VALUE));
	BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_DITHER_LFSR_INIT) |=  (
		BCHP_FIELD_ENUM(XSRC_0_HORIZ_DITHER_LFSR_INIT, SEQ,   ONCE_PER_SOP) |
		BCHP_FIELD_DATA(XSRC_0_HORIZ_DITHER_LFSR_INIT, VALUE,            0));

	BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_DITHER_CTRL) &=  ~(
		BCHP_MASK(XSRC_0_HORIZ_DITHER_CTRL, MODE      ) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_CTRL, OFFSET_CH2) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_CTRL, SCALE_CH2 ) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_CTRL, OFFSET_CH1) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_CTRL, SCALE_CH1 ) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_CTRL, OFFSET_CH0) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_CTRL, SCALE_CH0 ));
	BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_DITHER_CTRL) |=  (
		BCHP_FIELD_ENUM(XSRC_0_HORIZ_DITHER_CTRL, MODE,   DITHER) |
		BCHP_FIELD_DATA(XSRC_0_HORIZ_DITHER_CTRL, OFFSET_CH2,  0) |
		BCHP_FIELD_DATA(XSRC_0_HORIZ_DITHER_CTRL, SCALE_CH2,   1) |
		BCHP_FIELD_DATA(XSRC_0_HORIZ_DITHER_CTRL, OFFSET_CH1,  0) |
		BCHP_FIELD_DATA(XSRC_0_HORIZ_DITHER_CTRL, SCALE_CH1,   1) |
		BCHP_FIELD_DATA(XSRC_0_HORIZ_DITHER_CTRL, OFFSET_CH0,  0) |
		BCHP_FIELD_DATA(XSRC_0_HORIZ_DITHER_CTRL, SCALE_CH0,   1));

	BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_DITHER_LFSR_CTRL) &= ~(
		BCHP_MASK(XSRC_0_HORIZ_DITHER_LFSR_CTRL, T0) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_LFSR_CTRL, T1) |
		BCHP_MASK(XSRC_0_HORIZ_DITHER_LFSR_CTRL, T2));
	BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_DITHER_LFSR_CTRL) |=  (
		BCHP_FIELD_ENUM(XSRC_0_HORIZ_DITHER_LFSR_CTRL, T0, B3) |
		BCHP_FIELD_ENUM(XSRC_0_HORIZ_DITHER_LFSR_CTRL, T1, B8) |
		BCHP_FIELD_ENUM(XSRC_0_HORIZ_DITHER_LFSR_CTRL, T2, B12));

	hXsrc->ulSrcHrzAlign  = 2;

	/* Initialize state. */
	hXsrc->bInitial = true;

	BDBG_LEAVE(BVDC_P_Xsrc_Init_isr);
	return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Xsrc_BuildRul_SetEnable_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  BVDC_P_ListInfo              *pList )
{
	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);

	/* Add scaler registers to RUL using block write */
	if(hXsrc->ulUpdateAll)
	{
		hXsrc->ulUpdateAll--;
		/* optimize scaler mute RUL */
#if BVDC_P_XSRC_SUPPORT_HSCL
		BVDC_P_XSRC_BLOCK_WRITE_TO_RUL(
			hXsrc, XSRC_0_HORIZ_CONTROL, XSRC_0_DEST_PIC_SIZE,
			pList->pulCurrent);
		BVDC_P_XSRC_BLOCK_WRITE_TO_RUL(
			hXsrc, XSRC_0_SRC_PIC_HORIZ_PAN_SCAN,
			XSRC_0_HORIZ_DEST_PIC_REGION_3_END, pList->pulCurrent);
#else
		BVDC_P_XSRC_BLOCK_WRITE_TO_RUL(
			hXsrc, XSRC_0_BVB_IN_SIZE, XSRC_0_SRC_PIC_SIZE, pList->pulCurrent);
#endif
		BVDC_P_XSRC_WRITE_TO_RUL(
			hXsrc, XSRC_0_VIDEO_3D_MODE, pList->pulCurrent);

#if BVDC_P_XSRC_SUPPORT_DERINE
		if(hXsrc->bDeringing)
		{
			BVDC_P_XSRC_WRITE_TO_RUL(
				hXsrc, XSRC_0_DERINGING, pList->pulCurrent);
		}
#endif

#if BVDC_P_XSRC_SUPPORT_CCA
		if(hXsrc->bCca)
		{
			BVDC_P_XSRC_WRITE_TO_RUL(
				hXsrc, XSRC_0_CCA_CONFIG, pList->pulCurrent);
		}
#endif

		BVDC_P_XSRC_BLOCK_WRITE_TO_RUL(
			hXsrc, XSRC_0_HORIZ_DITHER_LFSR_INIT, XSRC_0_HORIZ_DITHER_CTRL,
			pList->pulCurrent);

#if BVDC_P_XSRC_SUPPORT_HSCL
		BVDC_P_XSRC_BLOCK_WRITE_TO_RUL(
			hXsrc, XSRC_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01,
			XSRC_0_HORIZ_FIR_CHROMA_COEFF_PHASE7_04_05, pList->pulCurrent);
#endif
		BVDC_P_XSRC_WRITE_TO_RUL(
			hXsrc, XSRC_0_TOP_CONTROL, pList->pulCurrent);
	}

	BVDC_P_XSRC_WRITE_TO_RUL(
		hXsrc, XSRC_0_ENABLE, pList->pulCurrent);
}



/***************************************************************************
 * {private}
 *
 * BVDC_P_Xsrc_AcquireConnect_isr
 *
 * It is called by BVDC_Window_Validate after changing from diable mad to
 * enable mad, .
 */
BERR_Code BVDC_P_Xsrc_AcquireConnect_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  BVDC_Source_Handle            hSource )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BDBG_ENTER(BVDC_P_Xsrc_AcquireConnect_isr);

	hXsrc->hSource = hSource;

	/* init xsrc */
	BVDC_P_Xsrc_Init_isr(hXsrc);

	BDBG_LEAVE(BVDC_P_Xsrc_AcquireConnect_isr);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Xsrc_ReleaseConnect_isr
 *
 * It is called after window decided that mad is no-longer used by HW in its
 * vnet mode (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_Xsrc_ReleaseConnect_isr
	( BVDC_P_Xsrc_Handle        *phXsrc )
{
	BERR_Code  eResult = BERR_SUCCESS;

	BDBG_ENTER(BVDC_P_Xsrc_ReleaseConnect_isr);

	/* handle validation */
	if (NULL != *phXsrc)
	{
		BDBG_OBJECT_ASSERT(*phXsrc, BVDC_XSRC);

		BVDC_P_Resource_ReleaseHandle_isr(
			BVDC_P_SubRul_GetResourceHandle_isr(&(*phXsrc)->SubRul),
			BVDC_P_ResourceType_eXsrc, (void *)(*phXsrc));

		/* this makes win to stop calling Xsrc code */
		*phXsrc = NULL;
	}

	BDBG_LEAVE(BVDC_P_Xsrc_ReleaseConnect_isr);
	return BERR_TRACE(eResult);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_Xsrc_BuildRul_DrainVnet_isr
 *
 * called by BVDC_P_Xsrc_BuildRul_isr after resetting to drain the module and
 * its pre-patch FreeCh or LpBack
 */
static void BVDC_P_Xsrc_BuildRul_DrainVnet_isr
	( BVDC_P_Xsrc_Handle          hXsrc,
	  BVDC_P_ListInfo            *pList,
	  bool                        bNoCoreReset)
{
	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);
	/* reset sub and connect the module to a drain */
	BVDC_P_SubRul_Drain_isr(&(hXsrc->SubRul), pList,
	    bNoCoreReset?0:hXsrc->ulResetRegAddr,
	    bNoCoreReset?0:hXsrc->ulResetMask,
	    bNoCoreReset?0:hXsrc->ulVnetResetAddr,
	    bNoCoreReset?0:hXsrc->ulVnetResetMask);
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Xsrc_BuildRul_isr
	( const BVDC_P_Xsrc_Handle      hXsrc,
	  BVDC_P_ListInfo              *pList,
	  BVDC_P_State                  eVnetState,
	  BVDC_P_PicComRulInfo         *pPicComRulInfo )
{
	uint32_t  ulRulOpsFlags;

	BDBG_ENTER(BVDC_P_Xsrc_BuildRul_isr);
	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);

	/* currently this is only for vnet building / tearing-off*/

	ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(
		&(hXsrc->SubRul), pPicComRulInfo->eWin, eVnetState, pList->bLastExecuted);

	if ((0 == ulRulOpsFlags) ||
		(ulRulOpsFlags & BVDC_P_RulOp_eReleaseHandle))
		return;
	else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
	{
		BVDC_P_SubRul_DropOffVnet_isr(&(hXsrc->SubRul), pList);
		BVDC_P_Xsrc_SetEnable_isr(hXsrc, false);
		BVDC_P_XSRC_WRITE_TO_RUL(
			hXsrc, XSRC_0_ENABLE, pList->pulCurrent);
	}

	/* If rul failed to execute last time we'd re reprogrammed possible
	 * missing registers. */
	if((!pList->bLastExecuted)|| (hXsrc->bInitial))
	{
		hXsrc->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
	}

	/* reset */
	if(hXsrc->bInitial)
	{
		hXsrc->bInitial = false;
	}

	if (ulRulOpsFlags & BVDC_P_RulOp_eEnable)
	{
		BVDC_P_Xsrc_BuildRul_SetEnable_isr(hXsrc, pList);

		/* join in vnet after enable. note: its src mux is initialed as disabled */
		if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
		{
			BVDC_P_SubRul_JoinInVnet_isr(&(hXsrc->SubRul), pList);
		}
	}

	if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
	{
		BVDC_P_Xsrc_BuildRul_DrainVnet_isr(hXsrc, pList, pPicComRulInfo->bNoCoreReset);
	}

	BDBG_LEAVE(BVDC_P_Xsrc_BuildRul_isr);
	return;
}

/***************************************************************************
 * {private}
 *
 */
#if BVDC_P_XSRC_SUPPORT_HSCL
static void BVDC_P_Xsrc_SetFirCoeff_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  const uint32_t               *pulHorzFirCoeff )
{
	int i;

	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);
	/* write horiz entries into registers */
	for(i = 0; (pulHorzFirCoeff) && (*pulHorzFirCoeff != BVDC_P_XSRC_LAST); i++)
	{
		hXsrc->aulRegs[BVDC_P_XSRC_GET_REG_IDX(XSRC_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01) + i] =
			*pulHorzFirCoeff;
		pulHorzFirCoeff++;
	}
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Xsrc_SetChromaFirCoeff_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  const uint32_t               *pulHorzFirCoeff )
{
	int i;

	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);
	/* write 32 hor entries into registers */
	for(i = 0; (pulHorzFirCoeff) && (*pulHorzFirCoeff != BVDC_P_XSRC_LAST); i++)
	{
		hXsrc->aulRegs[BVDC_P_XSRC_GET_REG_IDX(XSRC_0_HORIZ_FIR_CHROMA_COEFF_PHASE0_00_01) + i] =
			*pulHorzFirCoeff;
		pulHorzFirCoeff++;
	}
}
#endif

/***************************************************************************
 * {private}
 *
 * The following notes illustrate the capabilities of the hscaler.  It
 * mainly shows different modes scaler operates to achieve the desire
 * scaling ratio with the best quality.
 *
 * Sx = in/out.  For example src=1920 dst=720 then Sx = 1920/720 or 2.66.
 * Hence this is what Sx means:
 *   Sx >  1 Scale down
 *   Sx <  1 Scale up
 *   Sx == 1 Unity.
 *
 * Actuall Sx tells how much we're scaling down.  For example
 * Sx = 4 means we're scaling down 4 times.
 *
 * There is no hard-wired filter in XSRC, so Sx is the value that goes into
 * horizontal FIR ratio register.
 *
 * Sx must be [32.0, 0.125].
 *
 *
 * [[ Conclusion ]]
 *  With the above information the theoretical scaling capacities are:
 *
 *  Sx = 32:1 to 1:32
 */
void BVDC_P_Xsrc_SetInfo_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  BVDC_Window_Handle            hWindow,
	  const BVDC_P_PictureNodePtr   pPicture )
{
	uint32_t ulSrcHSize;               /* really scaled src width in pixel unit */
	uint32_t ulDstVSize;               /* Dst height, in row unit */
	uint32_t ulAlgnSrcHSize;           /* src width into the 1st one of half band or FIR, pixel unit */
	uint32_t ulBvbInHSize;             /* input bvb width in pixel unit */
	uint32_t ulNrmHrzStep;              /* Total horizontal src step per dest pixel, U11.21 */
	uint32_t ulFirHrzStep = 0;          /* FIR hrz src step per dest pixel, HW reg fmt, for coeff select */
	int32_t  lHrzPhsAccInit = 0;
	uint32_t ulMaxX;
	BVDC_P_Rect  *pXsrcIn, *pXsrcOut;
#if BVDC_P_XSRC_SUPPORT_HSCL
	uint32_t ulDstHSize;               /* Dst width in pixel unit */
	const BVDC_P_FirCoeffTbl *pHorzFirCoeff;
#endif

	BDBG_ENTER(BVDC_P_Xsrc_SetInfo_isr);
	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
	pXsrcIn = pPicture->pXsrcIn;
	pXsrcOut = pPicture->pXsrcOut;


	ulDstVSize = pXsrcOut->ulHeight >> (BAVC_Polarity_eFrame!=pPicture->eSrcPolarity);
	/* any following info changed -> re-calculate SCL settings */
	if((hXsrc->ulPrevSrcWidth != pXsrcIn->ulWidth) ||
	   (hXsrc->ulPrevOutWidth != pXsrcOut->ulWidth) ||
	   (hXsrc->ulPrevSrcHeight != ulDstVSize) ||  /* no vrt scl */
#if (BVDC_P_SUPPORT_3D_VIDEO)
		(pPicture->eOrigSrcOrientation != hXsrc->ePrevSrcOrientation)    ||
		(pPicture->eDispOrientation  != hXsrc->ePrevDispOrientation)   ||
#endif
	   !BVDC_P_XSRC_COMPARE_FIELD_DATA(hXsrc, XSRC_0_ENABLE, SCALER_ENABLE, 1))
	{
		/* for next "dirty" check */
		hXsrc->ulPrevSrcWidth = pXsrcIn->ulWidth;
		hXsrc->ulPrevOutWidth = pXsrcOut->ulWidth;
		hXsrc->ulPrevSrcHeight = pXsrcIn->ulHeight >> (BAVC_Polarity_eFrame!=pPicture->eSrcPolarity);

		hXsrc->ePrevSrcOrientation  = pPicture->eOrigSrcOrientation;
		hXsrc->ePrevDispOrientation = pPicture->eDispOrientation;

		hXsrc->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;

		/* -----------------------------------------------------------------------
		 * 1). Init some regitsers first, they might be modified later basing on
		 * specific conditions
		 */
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_ENABLE) =
			BCHP_FIELD_ENUM(XSRC_0_ENABLE, SCALER_ENABLE, ON);

		/* Always re-enable after set info. */
		/* Horizontal scaler settings (and top control)!  Choose scaling order,
		 * and how much to decimate data. */
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_TOP_CONTROL) =
#if (BVDC_P_SUPPORT_XSRC_VER < BVDC_P_SUPPORT_XSRC_VER_2)
			BCHP_FIELD_ENUM(XSRC_0_TOP_CONTROL, UPDATE_SEL,   UPDATE_BY_PICTURE) |
#endif
			BCHP_FIELD_ENUM(XSRC_0_TOP_CONTROL, ENABLE_CTRL,  ENABLE_BY_PICTURE);

#if BVDC_P_XSRC_SUPPORT_HSCL
		/* scaler panscan will be combined with init phase */
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_SRC_PIC_HORIZ_PAN_SCAN) &= ~(
			BCHP_MASK(XSRC_0_SRC_PIC_HORIZ_PAN_SCAN, OFFSET));

		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_CONTROL) &= ~(
			BCHP_MASK(XSRC_0_HORIZ_CONTROL, FIR_ENABLE          ) |
			BCHP_MASK(XSRC_0_HORIZ_CONTROL, MASK_HSCL_LONG_LINE ) |
			BCHP_MASK(XSRC_0_HORIZ_CONTROL, MASK_HSCL_SHORT_LINE) |
			BCHP_MASK(XSRC_0_HORIZ_CONTROL, STALL_DRAIN_ENABLE  ));

		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_CONTROL) |=  (
			 BCHP_FIELD_ENUM(XSRC_0_HORIZ_CONTROL, FIR_ENABLE,          ON ) |
			BCHP_FIELD_ENUM(XSRC_0_HORIZ_CONTROL, MASK_HSCL_LONG_LINE,  ON ) |
			BCHP_FIELD_ENUM(XSRC_0_HORIZ_CONTROL, MASK_HSCL_SHORT_LINE, ON ) |
			BCHP_FIELD_ENUM(XSRC_0_HORIZ_CONTROL, STALL_DRAIN_ENABLE,   OFF));
#endif
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_VIDEO_3D_MODE) =
			BCHP_FIELD_DATA(XSRC_0_VIDEO_3D_MODE, BVB_VIDEO,
			BVDC_P_VNET_USED_XSRC_AT_WRITER(pPicture->stVnetMode) ?
			pPicture->eOrigSrcOrientation : pPicture->eDispOrientation);

		/* -----------------------------------------------------------------------
		 * 2). Need to calculate the horizontal scaling factors before src width
		 * alignment and init phase can be decided
		 */

#if BVDC_P_XSRC_SUPPORT_HSCL
		/* output size */
		ulDstHSize = pXsrcOut->ulWidth;
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_DEST_PIC_SIZE) &= ~(
			BCHP_MASK(XSRC_0_DEST_PIC_SIZE, HSIZE) |
			BCHP_MASK(XSRC_0_DEST_PIC_SIZE, VSIZE));
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_DEST_PIC_SIZE) |=  (
			BCHP_FIELD_DATA(XSRC_0_DEST_PIC_SIZE, HSIZE, ulDstHSize) |
			BCHP_FIELD_DATA(XSRC_0_DEST_PIC_SIZE, VSIZE, ulDstVSize));
#endif
		/* the src size really scaled and output */
		ulSrcHSize = pXsrcIn->ulWidth;

		/* the src size that get into the first scaler sub-modules (e.g. HW half-band
		 * filter if it is scaled down a lot): it includes the FIR_LUMA_SRC_PIC_OFFSET,
		 * but not the XSRC_0_PIC_OFFSET, it has to be rounded-up for alignment */
		ulMaxX = (ulSrcHSize << 6);
		ulAlgnSrcHSize = ((ulMaxX + ((1<< 6) - 1)) >>  6);
		ulAlgnSrcHSize = BVDC_P_ALIGN_UP(ulAlgnSrcHSize, hXsrc->ulSrcHrzAlign);

		/* Init the input/output horizontal size of FIRs */
		/*ulFirSrcHSize = ulSrcHSize;*/

		/* XSRC only do linear scaling */
		/*if(0 == pPicture->ulNonlinearSclOutWidth)*/
		{
			/* Horizantal step HW reg uses U5.17 in older arch, U5.26 after smooth non-linear is
			 * suported. Since CPU uses 32 bits int, calc step with 26 bits frac needs special
			 * handling (see the delta calcu in the case of nonlinear scaling). It is the step
			 * delta and internal step accum reg, not the initial step value, that really need 26
			 * frac bits, therefore we use 21 bits for trade off */
			ulNrmHrzStep = pPicture->ulXsrcNrmHrzSrcStep;    /* U11.21 */
			ulFirHrzStep = /*ulHrzStep =*/ BVDC_P_SCL_H_STEP_NRM_TO_SPEC(ulNrmHrzStep); /* U4.17, U5.17, U5.26 */
			/*ulFirHrzStepInit = ulFirHrzStep;*/

#if BVDC_P_XSRC_SUPPORT_HSCL
			/* set step size and region_0 end */
			BVDC_P_XSRC_SET_HORZ_RATIO(hXsrc, ulFirHrzStep);
			BVDC_P_XSRC_SET_HORZ_REGION02(hXsrc, 0, ulDstHSize, 0);
#endif
		}

		/* -----------------------------------------------------------------------
		 * 3). Now we can set src size, offset(=0) and bvb size
		 */
		ulBvbInHSize = pXsrcIn->ulWidth;

		/* in older chips, align ulBvbInHSize up if ulAlgnSrcHSize has been aligned
		 * up due to half-band.
		 * note: align ulBvbInHSize up might cause "short line" error, that is OK
		 * and scl input module would patch. If we don't align up, SCL might hang */
		ulBvbInHSize  = BVDC_P_MAX(ulBvbInHSize, ulAlgnSrcHSize);

		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_BVB_IN_SIZE) &= ~(
			BCHP_MASK(XSRC_0_BVB_IN_SIZE, HSIZE) |
			BCHP_MASK(XSRC_0_BVB_IN_SIZE, VSIZE));
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_BVB_IN_SIZE) |=  (
			BCHP_FIELD_DATA(XSRC_0_BVB_IN_SIZE, HSIZE, ulBvbInHSize) |
			BCHP_FIELD_DATA(XSRC_0_BVB_IN_SIZE, VSIZE, ulDstVSize));

		/* SRC_PIC_SIZE should include FIR_LUMA_SRC_PIC_OFFSET and align */
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_SRC_PIC_SIZE) &= ~(
			BCHP_MASK(XSRC_0_SRC_PIC_SIZE, HSIZE) |
			BCHP_MASK(XSRC_0_SRC_PIC_SIZE, VSIZE));
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_SRC_PIC_SIZE) |=  (
			BCHP_FIELD_DATA(XSRC_0_SRC_PIC_SIZE, HSIZE, ulAlgnSrcHSize) |
			BCHP_FIELD_DATA(XSRC_0_SRC_PIC_SIZE, VSIZE, ulDstVSize));

#if BVDC_P_XSRC_SUPPORT_HSCL
		/* -----------------------------------------------------------------------
		 * 4). set coeffs for horizontal
		 */
		pHorzFirCoeff = BVDC_P_SelectFirCoeff_isr(hXsrc->pHorzFirCoeffTbl,
			0, BVDC_P_CtInput_eAny, BVDC_P_CtOutput_eAny,
			pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirHrzStep,
			hXsrc->ulHorzTaps, ulSrcHSize, ulDstHSize);
		BDBG_MSG(("Luma H Coeffs  : %s", pHorzFirCoeff->pchName));
		BVDC_P_Xsrc_SetFirCoeff_isr(hXsrc, pHorzFirCoeff->pulCoeffs);

		pHorzFirCoeff = BVDC_P_SelectFirCoeff_isr(hXsrc->pChromaHorzFirCoeffTbl,
			0, BVDC_P_CtInput_eAny, BVDC_P_CtOutput_eAny,
			pPicture->PicComRulInfo.eSrcOrigPolarity, ulFirHrzStep,
			hXsrc->ulHorzTaps, ulSrcHSize, ulDstHSize);
		BDBG_MSG(("Chroma H Coeffs: %s", pHorzFirCoeff->pchName));
		BVDC_P_Xsrc_SetChromaFirCoeff_isr(hXsrc, pHorzFirCoeff->pulCoeffs);

		/* -----------------------------------------------------------------------
		 * 5). set init phase for horizontal (zero crop)
		 */
		/* Compute the phase accumulate intial value in S11.6 or S5.26 */
		lHrzPhsAccInit = BVDC_P_FIXED_A_MINUS_FIXED_B(
			BVDC_P_H_INIT_PHASE_RATIO_ADJ(ulFirHrzStep) / 2,
			BVDC_P_H_INIT_PHASE_0_POINT_5);

		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_FIR_INIT_PHASE_ACC) &= ~(
			BCHP_MASK(XSRC_0_HORIZ_FIR_INIT_PHASE_ACC, SIZE));
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_FIR_INIT_PHASE_ACC) |=  (
			BCHP_FIELD_DATA(XSRC_0_HORIZ_FIR_INIT_PHASE_ACC, SIZE,
			lHrzPhsAccInit));

		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_FIR_INIT_PHASE_ACC_R) &= ~(
			BCHP_MASK(XSRC_0_HORIZ_FIR_INIT_PHASE_ACC_R, SIZE));
		BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_HORIZ_FIR_INIT_PHASE_ACC_R) |=  (
			BCHP_FIELD_DATA(XSRC_0_HORIZ_FIR_INIT_PHASE_ACC_R, SIZE,
			lHrzPhsAccInit));
#endif
	}

	/* Printing out ratio in float format would be nice, but PI
	 * code does permit float. */
	if(BVDC_P_RUL_UPDATE_THRESHOLD == hXsrc->ulUpdateAll)
	{
		BDBG_MSG(("-------------------------"));
		BDBG_MSG(("Xsrc[%d]         : %dx%d to %dx%d", hXsrc->eId,
			pXsrcIn->ulWidth,  pXsrcIn->ulHeight,
			pXsrcOut->ulWidth, pXsrcOut->ulHeight));
		BDBG_MSG(("ulFirHrzStep      : %-8x", ulFirHrzStep));
		BDBG_MSG(("H_InitPhase       : %-8x", lHrzPhsAccInit));
	}

	BDBG_LEAVE(BVDC_P_Xsrc_SetInfo_isr);
	return;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Xsrc_SetEnable_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  bool                          bEnable )
{
	BDBG_OBJECT_ASSERT(hXsrc, BVDC_XSRC);

	if(!BVDC_P_XSRC_COMPARE_FIELD_DATA(
		hXsrc, XSRC_0_ENABLE, SCALER_ENABLE, bEnable))
	{
		hXsrc->ulUpdateAll = BVDC_P_RUL_UPDATE_THRESHOLD;
	}

	/* Turn on/off the scaler. */
	BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_ENABLE) &= ~(
		BCHP_MASK(XSRC_0_ENABLE, SCALER_ENABLE));

	BVDC_P_XSRC_GET_REG_DATA(hXsrc, XSRC_0_ENABLE) |=  (bEnable
		? BCHP_FIELD_ENUM(XSRC_0_ENABLE, SCALER_ENABLE, ON)
		: BCHP_FIELD_ENUM(XSRC_0_ENABLE, SCALER_ENABLE, OFF));

	return;
}
#else
BERR_Code BVDC_P_Xsrc_Create
	( BVDC_P_Xsrc_Handle           *phXsrc,
	  BVDC_P_XsrcId                 eXsrcId,
	  BVDC_P_Resource_Handle        hResource,
	  BREG_Handle                   hReg )
{
	BSTD_UNUSED(phXsrc);
	BSTD_UNUSED(eXsrcId);
	BSTD_UNUSED(hResource);
	BSTD_UNUSED(hReg);
	return BERR_SUCCESS;
}

void BVDC_P_Xsrc_Destroy
	( BVDC_P_Xsrc_Handle            hXsrc )
{
	BSTD_UNUSED(hXsrc);
	return;
}

void BVDC_P_Xsrc_Init_isr
	( BVDC_P_Xsrc_Handle            hXsrc )
{
	BSTD_UNUSED(hXsrc);
	return;
}

void BVDC_P_Xsrc_BuildRul_SetEnable_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  BVDC_P_ListInfo              *pList )
{
	BSTD_UNUSED(hXsrc);
	BSTD_UNUSED(pList);
}

void BVDC_P_Xsrc_SetInfo_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  BVDC_Window_Handle            hWindow,
	  const BVDC_P_PictureNodePtr   pPicture )
{
	BSTD_UNUSED(hXsrc);
	BSTD_UNUSED(hWindow);
	BSTD_UNUSED(pPicture);
	return;
}

void BVDC_P_Xsrc_SetEnable_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  bool                          bEnable )
{
	BSTD_UNUSED(hXsrc);
	BSTD_UNUSED(bEnable);
	return;
}
#endif  /* #if (BVDC_P_SUPPORT_XSRC) || (BVDC_P_SUPPORT_XSRC_MAD_HARD_WIRED) */

#if !(BVDC_P_SUPPORT_XSRC)

BERR_Code BVDC_P_Xsrc_AcquireConnect_isr
	( BVDC_P_Xsrc_Handle            hXsrc,
	  BVDC_Source_Handle            hSource )
{
	BSTD_UNUSED(hXsrc);
	BSTD_UNUSED(hSource);
	return BERR_SUCCESS;
}

BERR_Code BVDC_P_Xsrc_ReleaseConnect_isr
	( BVDC_P_Xsrc_Handle           *phXsrc )
{
	BSTD_UNUSED(phXsrc);
	return BERR_SUCCESS;
}

void BVDC_P_Xsrc_BuildRul_isr
	( const BVDC_P_Xsrc_Handle      hXsrc,
	  BVDC_P_ListInfo              *pList,
	  BVDC_P_State                  eVnetState,
	  BVDC_P_PicComRulInfo         *pPicComRulInfo )
{
	BSTD_UNUSED(hXsrc);
	BSTD_UNUSED(pList);
	BSTD_UNUSED(eVnetState);
	BSTD_UNUSED(pPicComRulInfo);
	return;
}

#endif /* #if !(BVDC_P_SUPPORT_XSRC) */

/* End of file. */
