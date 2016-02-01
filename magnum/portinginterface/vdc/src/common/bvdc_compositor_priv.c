/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#include "bvdc_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bchp_bmisc.h"
#include "bvdc_pep_priv.h"
#include "bchp_mmisc.h"
#include "bvdc_feeder_priv.h"

#if BVDC_P_SUPPORT_OSCL
#include "bvdc_oscl_priv.h"
#endif

#if (BVDC_P_SUPPORT_NEW_SW_INIT) /* beyond 7420 */
#define BVDC_P_MAKE_CMP(pCmp, id)                                     \
{                                                                     \
	(pCmp)->ulCoreResetAddr = BCHP_BMISC_SW_INIT;                     \
	(pCmp)->ulCoreResetMask = BCHP_BMISC_SW_INIT_CMP_##id##_MASK;     \
}
#else
#define BVDC_P_MAKE_CMP(pCmp, id)                                     \
{                                                                     \
	(pCmp)->ulCoreResetAddr = BCHP_BMISC_SOFT_RESET;                  \
	(pCmp)->ulCoreResetMask = BCHP_BMISC_SOFT_RESET_CMP_##id##_MASK;  \
}
#endif

/* Dither settings for COMP */
#define BVDC_P_DITHER_CMP_LFSR_VALUE                 (0xE0F82)
#define BVDC_P_DITHER_CMP_V0_LFSR_CTRL_T0            (0x6)
#define BVDC_P_DITHER_CMP_V0_LFSR_CTRL_T1            (0x3)
#define BVDC_P_DITHER_CMP_V0_LFSR_CTRL_T2            (0x6)
#define BVDC_P_DITHER_CMP_V1_LFSR_CTRL_T0            (0x4)
#define BVDC_P_DITHER_CMP_V1_LFSR_CTRL_T1            (0x3)
#define BVDC_P_DITHER_CMP_V1_LFSR_CTRL_T2            (0x6)

BDBG_MODULE(BVDC_CMP);
BDBG_FILE_MODULE(BVDC_CMP_SIZE);
BDBG_FILE_MODULE(BVDC_CMP_CSC);
BDBG_FILE_MODULE(repeatpolarity);
BDBG_OBJECT_ID(BVDC_CMP);

/* SW7250-211/ SW7364-291
 CMP0 RDB HW config register bit CORE_BVB_WIDTH_10 should be 1 instead 0*/
#if (((BCHP_CHIP==7364) && (BCHP_VER <= BCHP_VER_B0)) ||\
	 ((BCHP_CHIP==7250) && (BCHP_VER == BCHP_VER_B0)))
#define BVDC_P_CMP_HW_CONFIGURATION_WORKAROUND         (1)
#else
#define BVDC_P_CMP_HW_CONFIGURATION_WORKAROUND         (0)
#endif


/* INDEX: by compositor id. */
static const BVDC_P_Compositor_Features s_aHydraCompositorFeatures
	[BVDC_P_MAX_COMPOSITOR_COUNT] =
{
	/* Compositor 0 (Primary) */
	{BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT,
	 BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT,
	 BVDC_P_CMP_0_MAX_WINDOW_COUNT},

	/* Compositor 1 (Secondary) */
	{BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT,
	 BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT,
	 BVDC_P_CMP_1_MAX_WINDOW_COUNT},

	/* Compositor Bypass */
	{BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT,
	 BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT,
	 BVDC_P_CMP_2_MAX_WINDOW_COUNT},

	/* VICE */
	{BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT,
	 BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT,
	 BVDC_P_CMP_3_MAX_WINDOW_COUNT},

	/* VICE */
	{BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT,
	 BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT,
	 BVDC_P_CMP_4_MAX_WINDOW_COUNT},

	/* VICE */
	{BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT,
	 BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT,
	 BVDC_P_CMP_5_MAX_WINDOW_COUNT},

	/* VICE */
	{BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT,
	 BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT,
	 BVDC_P_CMP_6_MAX_WINDOW_COUNT}
};


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Compositor_Create
	( BVDC_Handle                      hVdc,
	  BVDC_Compositor_Handle          *phCompositor,
	  BVDC_CompositorId                eCompositorId )
{
	uint32_t i;
	BVDC_P_CompositorContext *pCompositor;
	BERR_Code eStatus = BERR_SUCCESS;
#ifdef BCHP_CMP_0_HW_CONFIGURATION
	uint32_t ulHwCfg;
#endif

	BDBG_ENTER(BVDC_P_Compositor_Create);
	BDBG_ASSERT(phCompositor);
	BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

	/* (1) Alloc the context. */
	pCompositor = (BVDC_P_CompositorContext*)
		(BKNI_Malloc(sizeof(BVDC_P_CompositorContext)));
	if(!pCompositor)
	{
		eStatus = BERR_OUT_OF_SYSTEM_MEMORY;
		goto BVDC_P_Compositor_Create_Done;
	}

	/* Clear out the context and set defaults. */
	BKNI_Memset((void*)pCompositor, 0x0, sizeof(BVDC_P_CompositorContext));
	BDBG_OBJECT_SET(pCompositor, BVDC_CMP);

	/* Initialize non-changing states.  These are not to be changed by runtime. */
	pCompositor->eId          = eCompositorId;
	pCompositor->pFeatures    = &s_aHydraCompositorFeatures[eCompositorId];
	pCompositor->ulRegOffset  = BVDC_P_CMP_GET_REG_OFFSET(eCompositorId);
	pCompositor->hVdc         = hVdc;
	pCompositor->bIsBypass    = (
		(hVdc->pFeatures->bCmpBIsBypass) &&
		(BVDC_CompositorId_eCompositor2 == pCompositor->eId));

#ifdef BCHP_CMP_0_HW_CONFIGURATION
	ulHwCfg = BREG_Read32(hVdc->hRegister,
		BCHP_CMP_0_HW_CONFIGURATION + pCompositor->ulRegOffset);
#ifdef BCHP_CMP_0_HW_CONFIGURATION_V0_Ma_CSC_Present_SHIFT
	pCompositor->bSupportMACsc[0] = BVDC_P_GET_FIELD(
		ulHwCfg, CMP_0_HW_CONFIGURATION, V0_Ma_CSC_Present);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_V0_NL_LUT_Present_SHIFT
	pCompositor->bSupportNLCsc[0] = BVDC_P_GET_FIELD(
		ulHwCfg, CMP_0_HW_CONFIGURATION, V0_NL_LUT_Present);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_V1_Ma_CSC_Present_SHIFT
	pCompositor->bSupportMACsc[1] = BVDC_P_GET_FIELD(
		ulHwCfg, CMP_0_HW_CONFIGURATION, V1_Ma_CSC_Present);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_V1_NL_LUT_Present_SHIFT
	pCompositor->bSupportNLCsc[1] = BVDC_P_GET_FIELD(
		ulHwCfg, CMP_0_HW_CONFIGURATION, V1_NL_LUT_Present);
#endif
#ifdef BCHP_CMP_0_HW_CONFIGURATION_CORE_BVB_WIDTH_10_SHIFT
	pCompositor->bIs10BitCore = BVDC_P_GET_FIELD(
		ulHwCfg, CMP_0_HW_CONFIGURATION, CORE_BVB_WIDTH_10);
#if BVDC_P_CMP_HW_CONFIGURATION_WORKAROUND
	if(BVDC_CompositorId_eCompositor0 == pCompositor->eId)
		pCompositor->bIs10BitCore = true;
#endif
#endif

#if (!defined(BCHP_CMP_0_HW_CONFIGURATION_V0_Ma_CSC_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V0_NL_LUT_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V1_Ma_CSC_Present_SHIFT) && \
     !defined(BCHP_CMP_0_HW_CONFIGURATION_V1_NL_LUT_Present_SHIFT))
	BSTD_UNUSED(ulHwCfg);
#endif
#if (BCHP_CHIP==7439) && (BCHP_VER <= BCHP_VER_B1)
	/* TODO: the following info in HW_CONFIGURATION is not right, create HW jira */
	pCompositor->bSupportNLCsc[0] = true;
	pCompositor->bSupportNLCsc[1] = true;
	if (pCompositor->eId == BVDC_CompositorId_eCompositor0)
		pCompositor->bSupportMACsc[1] = true;
#endif
#endif

	/* all windows should have been disconnected. */
	for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
	{
		pCompositor->ahWindow[i] = NULL;
	}

	/* (2) Create RDC List */
	for(i = 0; i < BVDC_P_CMP_MAX_LIST_COUNT; i++)
	{
		eStatus = BRDC_List_Create(hVdc->hRdc, BVDC_P_MAX_ENTRY_PER_RUL,
			&pCompositor->ahList[i]);
		if(BERR_SUCCESS != eStatus)
		{
			goto BVDC_P_Compositor_Create_Done;
		}
	}

	/* (3) Create RDC Slot */
	for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
	{
		BRDC_SlotId eSlotId;
		BRDC_Slot_Settings  stSlotSettings;

		eStatus = BRDC_Slot_Create(hVdc->hRdc, &pCompositor->ahSlot[i]);
		if(BERR_SUCCESS != eStatus)
		{
			goto BVDC_P_Compositor_Create_Done;
		}
		BRDC_Slot_UpdateLastRulStatus_isr(pCompositor->ahSlot[i], pCompositor->ahList[i], true);
		eStatus = BRDC_Slot_GetId(pCompositor->ahSlot[i], &eSlotId);
		if(BERR_SUCCESS != eStatus)
		{
			goto BVDC_P_Compositor_Create_Done;
		}

		/* Enable RDC priority */
		if(pCompositor->eId == BVDC_CompositorId_eCompositor0)
		{
			stSlotSettings.bHighPriority = true;
			eStatus = BRDC_Slot_SetConfiguration(pCompositor->ahSlot[i], &stSlotSettings);
			if(BERR_SUCCESS != eStatus)
			{
				goto BVDC_P_Compositor_Create_Done;
			}
		}
		BDBG_MSG(("Compositor[%d] uses slot[%d]", pCompositor->eId, eSlotId));
	}

	/* (4) Create Slot interrupt callback */
	for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
	{
		eStatus = BINT_CreateCallback(&pCompositor->ahCallback[i], hVdc->hInterrupt,
			BRDC_Slot_GetIntId(pCompositor->ahSlot[i]),
			BVDC_P_CompositorDisplay_isr, (void*)pCompositor, i);
		if(BERR_SUCCESS != eStatus)
		{
			goto BVDC_P_Compositor_Create_Done;
		}
	}

	switch(pCompositor->eId)
	{
		case BVDC_CompositorId_eCompositor0:
			BVDC_P_MAKE_CMP(pCompositor, 0);
			break;
#if (BVDC_P_CMP_1_MAX_WINDOW_COUNT)
		case BVDC_CompositorId_eCompositor1:
			BVDC_P_MAKE_CMP(pCompositor, 1);
			break;
#endif
#if (BVDC_P_CMP_2_MAX_WINDOW_COUNT)
		case BVDC_CompositorId_eCompositor2:
			BVDC_P_MAKE_CMP(pCompositor, 2);
			break;
#endif
#if (BVDC_P_CMP_3_MAX_WINDOW_COUNT)
		case BVDC_CompositorId_eCompositor3:
			BVDC_P_MAKE_CMP(pCompositor, 3);
			break;
#endif
#if (BVDC_P_CMP_4_MAX_WINDOW_COUNT)
		case BVDC_CompositorId_eCompositor4:
			BVDC_P_MAKE_CMP(pCompositor, 4);
			break;
#endif
#if (BVDC_P_CMP_5_MAX_WINDOW_COUNT)
		case BVDC_CompositorId_eCompositor5:
			BVDC_P_MAKE_CMP(pCompositor, 5);
			break;
#endif
#if (BVDC_P_CMP_6_MAX_WINDOW_COUNT)
		case BVDC_CompositorId_eCompositor6:
			BVDC_P_MAKE_CMP(pCompositor, 6);
			break;
#endif
		default:
			BDBG_ERR(("Need to handle BVDC_CompositorId_eCompositor%d", pCompositor->eId));
			BDBG_ASSERT(0);
			break;
	}


	/* (5) Added this compositor to hVdc */
	hVdc->ahCompositor[eCompositorId] = (BVDC_Compositor_Handle)pCompositor;

	/* All done. now return the new fresh context to user. */
	*phCompositor = (BVDC_Compositor_Handle)pCompositor;

BVDC_P_Compositor_Create_Done:
	BDBG_LEAVE(BVDC_P_Compositor_Create);

	if((BERR_SUCCESS != eStatus) && (NULL != pCompositor))
	{
		for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
		{
			if(NULL != pCompositor->ahCallback[i])
				BINT_DestroyCallback(pCompositor->ahCallback[i]);
		}
		for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
		{
			if(NULL != pCompositor->ahSlot[i])
				BRDC_Slot_Destroy(pCompositor->ahSlot[i]);
		}
		for(i = 0; i < BVDC_P_CMP_MAX_LIST_COUNT; i++)
		{
			if(NULL != pCompositor->ahList[i])
				BRDC_List_Destroy(pCompositor->ahList[i]);
		}

		BDBG_OBJECT_DESTROY(pCompositor, BVDC_CMP);
		BKNI_Free((void*)pCompositor);
	}

	return BERR_TRACE(eStatus);
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Compositor_Destroy
	( BVDC_Compositor_Handle           hCompositor )
{
	uint32_t i;

	BDBG_ENTER(BVDC_P_Compositor_Destroy);
	if(!hCompositor)
	{
		BDBG_LEAVE(BVDC_P_Compositor_Destroy);
		return;
	}

	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
	BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

	/* At this point application should have disable all the
	 * callbacks &slots */

	/* [5] Removed this compositor from hVdc */
	hCompositor->hVdc->ahCompositor[hCompositor->eId] = NULL;

	/* Child windows for this compositor. */
	if(BVDC_CompositorId_eCompositor0 == hCompositor->eId)
	{
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V0]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V1]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_G0]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_G1]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp0_G2]);
	}
	else if(BVDC_CompositorId_eCompositor1 == hCompositor->eId)
	{
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp1_V0]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp1_V1]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp1_G0]);
	}
	else if(BVDC_CompositorId_eCompositor2 == hCompositor->eId)
	{
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp2_V0]);
		if(!hCompositor->hVdc->pFeatures->bCmpBIsBypass)
		{
			BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp2_G0]);
		}
	}
	else if(BVDC_CompositorId_eCompositor3 == hCompositor->eId)
	{
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp3_V0]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp3_G0]);
	}
	else if(BVDC_CompositorId_eCompositor4 == hCompositor->eId)
	{
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp4_V0]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp4_G0]);
	}
	else if(BVDC_CompositorId_eCompositor5 == hCompositor->eId)
	{
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp5_V0]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp5_G0]);
	}
	else if(BVDC_CompositorId_eCompositor6 == hCompositor->eId)
	{
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp6_V0]);
		BVDC_P_Window_Destroy(hCompositor->ahWindow[BVDC_P_WindowId_eComp6_G0]);
	}

	/* [4] Create Slot interrupt callback */
	for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
	{
		BINT_DestroyCallback(hCompositor->ahCallback[i]);
	}

	/* [3] Destroy RDC Slot */
	for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
	{
		BRDC_Slot_Destroy(hCompositor->ahSlot[i]);
	}

	/* [2] Destroy RDC List */
	for(i = 0; i < BVDC_P_CMP_MAX_LIST_COUNT; i++)
	{
		BRDC_List_Destroy(hCompositor->ahList[i]);
	}

	BDBG_OBJECT_DESTROY(hCompositor, BVDC_CMP);
	/* [1] Release context in system memory */
	BKNI_Free((void*)hCompositor);

	BDBG_LEAVE(BVDC_P_Compositor_Destroy);
	return;
}


/***************************************************************************
 * {private}
 *
 * By application calling BVDC_Compositor_Create().
 */
void BVDC_P_Compositor_Init
	( BVDC_Compositor_Handle           hCompositor )
{
	uint32_t i;
	BVDC_P_Compositor_Info *pNewInfo;
	BVDC_P_Compositor_Info *pCurInfo;

	BDBG_ENTER(BVDC_P_Compositor_Init);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
	BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

	/* Init to default state. */
	hCompositor->bInitial            = true;
	hCompositor->eState              = BVDC_P_State_eInactive;
	hCompositor->ulActiveVideoWindow = 0;
	hCompositor->ulActiveGfxWindow   = 0;
	hCompositor->bUserAppliedChanges = false;
	hCompositor->hSyncLockWin        = NULL;
	hCompositor->hSyncLockSrc        = NULL;
	hCompositor->hForceTrigPipSrc    = NULL;
	hCompositor->ulSlip2Lock         = 0;


	/* used for NRT mode transcode: default true to freeze STC and not get encoded */
	hCompositor->bIgnorePicture = true;
	hCompositor->bStallStc      = true;
	hCompositor->bMute          = true;
	/* used for xcode channel change */
	hCompositor->bChannelChange = true;
	hCompositor->bGfxChannelChange = true;


	/* reinitialized ruls. */
	for(i = 0; i < BVDC_P_CMP_MAX_LIST_COUNT; i++)
	{
		BRDC_List_SetNumEntries_isr(hCompositor->ahList[i], 0);
		BVDC_P_BuildNoOpsRul_isr(hCompositor->ahList[i]);
	}

	/* Assign fresh new no-op list. */
	for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
	{
		/* Initialized rul indexing. */
		hCompositor->aulRulIdx[i] = 0;
		BRDC_Slot_SetList_isr(hCompositor->ahSlot[i],
			hCompositor->ahList[i * BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT]);
	}

	/* Compositor's output framerate from selected master window */
	hCompositor->eSrcFRateCode = hCompositor->hVdc->stSettings.eDisplayFrameRate;
	hCompositor->bFullRate     = BVDC_P_IS_FULL_FRAMRATE(hCompositor->eSrcFRateCode);

	/* Clear out shadow registers. */
	BKNI_Memset((void*)hCompositor->aulRegs, 0x0, sizeof(hCompositor->aulRegs));

	/* Default compositor settings. */
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_CTRL) &= ~(
		BCHP_MASK(CMP_0_CANVAS_CTRL, ENABLE) |
		BCHP_MASK(CMP_0_CANVAS_CTRL, ENABLE_CTRL));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_CANVAS_CTRL, ENABLE, ENABLE) |
		BCHP_FIELD_ENUM(CMP_0_CANVAS_CTRL, ENABLE_CTRL, STOP_ON_FIELD_COMPLETION));

	/* Default blender settings, no blending */
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_0_CTRL) &= ~(
		BCHP_MASK(CMP_0_BLEND_0_CTRL, BLEND_SOURCE));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_0_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));

	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_1_CTRL) &= ~(
		BCHP_MASK(CMP_0_BLEND_1_CTRL, BLEND_SOURCE));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_1_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_BLEND_1_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 2
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_2_CTRL) &= ~(
		BCHP_MASK(CMP_0_BLEND_2_CTRL, BLEND_SOURCE));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_2_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_BLEND_2_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 3
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_3_CTRL) &= ~(
		BCHP_MASK(CMP_0_BLEND_3_CTRL, BLEND_SOURCE));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_3_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_BLEND_3_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 4
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_4_CTRL) &= ~(
		BCHP_MASK(CMP_0_BLEND_4_CTRL, BLEND_SOURCE));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_BLEND_4_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_BLEND_4_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));
#endif
#endif
#endif

	/* PR 11368: Configuring compositors for multi-tap filtering, for
	 * eliminating distortion when using all-pass bandwidth filters. */
	BVDC_P_CMP_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
		BCHP_MASK(CMP_0_V0_SURFACE_CTRL, FILT_CTRL));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
#if BVDC_P_SUPPORT_CMP_TEN_TAP_422_TO_444
		BCHP_FIELD_ENUM(CMP_0_V0_SURFACE_CTRL, FILT_CTRL, TEN_TAPS_FILTERING));
#else
		BCHP_FIELD_ENUM(CMP_0_V0_SURFACE_CTRL, FILT_CTRL, MULTI_TAPS_FILTERING));
#endif

#if BVDC_P_SUPPORT_CMP_DERINGING
	BVDC_P_CMP_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
		BCHP_MASK(CMP_0_V0_SURFACE_CTRL, DERING_EN));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_V0_SURFACE_CTRL, DERING_EN, ENABLE));
#endif

#if BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1
	BVDC_P_CMP_GET_REG_DATA(CMP_0_V1_SURFACE_CTRL) &= ~(
		BCHP_MASK(CMP_0_V1_SURFACE_CTRL, FILT_CTRL));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_V1_SURFACE_CTRL) |=  (
#if BVDC_P_SUPPORT_CMP_TEN_TAP_422_TO_444
		BCHP_FIELD_ENUM(CMP_0_V1_SURFACE_CTRL, FILT_CTRL, TEN_TAPS_FILTERING));
#else
		BCHP_FIELD_ENUM(CMP_0_V1_SURFACE_CTRL, FILT_CTRL, MULTI_TAPS_FILTERING));
#endif  /* BVDC_P_SUPPORT_CMP_TEN_TAP_422_TO_444 */

#if BVDC_P_SUPPORT_CMP_DERINGING
	BVDC_P_CMP_GET_REG_DATA(CMP_0_V1_SURFACE_CTRL) &= ~(
		BCHP_MASK(CMP_0_V1_SURFACE_CTRL, DERING_EN));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_V1_SURFACE_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_V1_SURFACE_CTRL, DERING_EN, ENABLE));
#endif

#endif  /* BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1 */

	/* disable range clipping */
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CMP_OUT_CTRL) &= ~(
		BCHP_MASK(CMP_0_CMP_OUT_CTRL, OUT_TO_DS_CTRL) |
		BCHP_MASK(CMP_0_CMP_OUT_CTRL, OUT_TO_VEC_CTRL) |
		BCHP_MASK(CMP_0_CMP_OUT_CTRL, CLIP_CTRL));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CMP_OUT_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_CMP_OUT_CTRL, OUT_TO_DS_CTRL, DISABLE) |
		BCHP_FIELD_ENUM(CMP_0_CMP_OUT_CTRL, OUT_TO_VEC_CTRL, ENABLE) |
		BCHP_FIELD_ENUM(CMP_0_CMP_OUT_CTRL, CLIP_CTRL, DISABLE));

#if BVDC_P_SUPPORT_CMP_CRC
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CRC_CTRL) &= ~(
		BCHP_MASK(CMP_0_CRC_CTRL, INIT_VALUE) |
		BCHP_MASK(CMP_0_CRC_CTRL, PROBE_RATE) |
		BCHP_MASK(CMP_0_CRC_CTRL, CLEAR) |
		BCHP_MASK(CMP_0_CRC_CTRL, ENABLE));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CRC_CTRL) |=  (
		BCHP_FIELD_DATA(CMP_0_CRC_CTRL, INIT_VALUE, 0) |
		BCHP_FIELD_ENUM(CMP_0_CRC_CTRL, PROBE_RATE, ONE_PICTURE_PERIOD) |
		BCHP_FIELD_ENUM(CMP_0_CRC_CTRL, CLEAR,      CLEAR) |
		BCHP_FIELD_DATA(CMP_0_CRC_CTRL, ENABLE,     1));
#endif

	/* Initial new/current public states */
	pNewInfo = &hCompositor->stNewInfo;
	pCurInfo = &hCompositor->stCurInfo;

	/* Clear out user's states. */
	BKNI_Memset((void*)pNewInfo, 0x0, sizeof(BVDC_P_Compositor_Info));
	BKNI_Memset((void*)pCurInfo, 0x0, sizeof(BVDC_P_Compositor_Info));
	BDBG_CASSERT(sizeof(pNewInfo->stDirty.stBits) <= sizeof(pNewInfo->stDirty.aulInts));

	/* default values. */
	pNewInfo->pFmtInfo = BFMT_GetVideoFormatInfoPtr(
		hCompositor->hVdc->stSettings.eVideoFormat);

	pNewInfo->ulBgColorYCrCb = BVDC_P_YCRCB_BLACK;

	pNewInfo->stLumaRect.stRegion.ulLeft   = 0;
	pNewInfo->stLumaRect.stRegion.ulRight  = 0;
	pNewInfo->stLumaRect.stRegion.ulTop    = 0;
	pNewInfo->stLumaRect.stRegion.ulBottom = 0;
	pNewInfo->bLumaRectUserSet             = false;

	pNewInfo->stColorClipSettings.ulCrYSlopeA = 0;
	pNewInfo->stColorClipSettings.ulCrYSlopeB = 0;
	pNewInfo->stColorClipSettings.ulCbYSlopeA = 0;
	pNewInfo->stColorClipSettings.ulCbYSlopeB = 0;
	pNewInfo->stColorClipSettings.ulCrJoint = 0;
	pNewInfo->stColorClipSettings.ulCbJoint = 0;
	pNewInfo->stColorClipSettings.eColorClipMode = BVDC_ColorClipMode_None;

	/* Make sure it a non-null pointer for window creation. */
	pCurInfo->pFmtInfo = BFMT_GetVideoFormatInfoPtr(
		hCompositor->hVdc->stSettings.eVideoFormat);

	BDBG_LEAVE(BVDC_P_Compositor_Init);
	return;
}


/* Miscellaneous functions. */
/***************************************************************************
 * {private}
 *
 * Determined if new user settings are valid.  It validates all new
 * settings of windows within this compositor.
 */
static BERR_Code BVDC_P_Compositor_Validate
	( const BVDC_Compositor_Handle     hCompositor )
{
	uint32_t i, j, k, ulPhaseAdjCount, ulMasterCount, ulGameModeCount;
	uint32_t aulBlenderZOrder[BVDC_P_CMP_MAX_BLENDER];
	/*BVDC_P_WindowId aeBlenderWinId[BVDC_P_CMP_MAX_BLENDER];*/
	uint32_t ulVidWinCnt = 0;
	BERR_Code eStatus = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	/* Defer validations until display handle is created. */
	if(!hCompositor->hDisplay)
	{
		return BERR_SUCCESS;
	}

	/* Get the new validated format from BVDC_P_Display! */
#if BVDC_P_SUPPORT_OSCL
	/* Certain chipsets rely on OSCL (output sclaer) within compositor to
	 * to achieve 1080p output. When in this mode, VEC is running at 1080p, but
	 * the whole BVN is configured as if the output format is 1080i.
	 * We use OSCL to convert the interlaced picture to frame then render it
	 * to VEC.
	 */
	if ((BFMT_VideoFmt_e1080p == hCompositor->hDisplay->stNewInfo.pFmtInfo->eVideoFmt) ||
		(BFMT_VideoFmt_e1080p_50Hz== hCompositor->hDisplay->stNewInfo.pFmtInfo->eVideoFmt))
	{
		hCompositor->stNewInfo.pFmtInfo =
			BFMT_GetVideoFormatInfoPtr((BFMT_VideoFmt_e1080p == hCompositor->hDisplay->stNewInfo.pFmtInfo->eVideoFmt) ?
				BFMT_VideoFmt_e1080i : BFMT_VideoFmt_e1080i_50Hz);
	}
	else
	{
		hCompositor->stNewInfo.pFmtInfo = hCompositor->hDisplay->stNewInfo.pFmtInfo;
	}

#else
	hCompositor->stNewInfo.pFmtInfo = hCompositor->hDisplay->stNewInfo.pFmtInfo;
#endif

	/* keep track of which blender get used */
	for(i = 0; i < BVDC_P_CMP_MAX_BLENDER; i++)
	{
		hCompositor->abBlenderUsed[i] = false;
		aulBlenderZOrder[i] = 0;
		hCompositor->aeBlenderWinId[i] = BVDC_P_WindowId_eUnknown;
	}
	ulPhaseAdjCount = 0;
	ulMasterCount = 0;
	ulGameModeCount = 0;

	/* Make sure all the changes to these windows are valid. */
	for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
	{
		if(BVDC_P_STATE_IS_ACTIVE(hCompositor->ahWindow[i]) ||
		   BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]))
		{
			uint8_t ucZOrder = hCompositor->ahWindow[i]->stNewInfo.ucZOrder;

			for(j=0; j<BVDC_P_CMP_MAX_BLENDER-1;j++)
			{
				if(hCompositor->abBlenderUsed[j]==false) break;

				if(ucZOrder < aulBlenderZOrder[j])
				{
					/* shift everything down by 1 */
					for(k=BVDC_P_CMP_MAX_BLENDER-2; k>=j; k--)
					{
						if(hCompositor->abBlenderUsed[k])
						{
							aulBlenderZOrder[k+1] = aulBlenderZOrder[k];
							hCompositor->abBlenderUsed[k+1] = true;
							hCompositor->aeBlenderWinId[k+1] = hCompositor->aeBlenderWinId[k];
							hCompositor->ahWindow[hCompositor->aeBlenderWinId[k+1]]->ulBlenderId = k+1;
						}

						if(k==j) break;
					}
					break;
				}
			}

			hCompositor->abBlenderUsed[j] = true;
			aulBlenderZOrder[j] = ucZOrder;
			hCompositor->aeBlenderWinId[j] = hCompositor->ahWindow[i]->eId;
			hCompositor->ahWindow[hCompositor->aeBlenderWinId[j]]->ulBlenderId = j;

			/* Window validation is where the bulk of the work going to be. */
			eStatus = BVDC_P_Window_ValidateChanges(hCompositor->ahWindow[i],
				hCompositor->stNewInfo.pFmtInfo);
			if(BERR_SUCCESS != eStatus)
			{
				return BERR_TRACE(eStatus);
			}

			if(BVDC_P_WIN_IS_VIDEO_WINDOW(hCompositor->ahWindow[i]->eId))
			{
				ulVidWinCnt++;
			}

			if(hCompositor->ahWindow[i]->stNewInfo.bUseSrcFrameRate)
			{
				ulMasterCount++;
			}

			if(hCompositor->ahWindow[i]->stNewInfo.stGameDelaySetting.bEnable)
			{
				ulGameModeCount++;
			}
		}
	}

	/* Check number of video windows in a given compositor.  */
	if(ulVidWinCnt > BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT)
	{
		return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
	}

	/* Check phase adjustment mode settings.  Only one analog source
	 * can be adjust with a given compositor.  */
	if(ulPhaseAdjCount > 1)
	{
		return BERR_TRACE(BVDC_ERR_MULTI_PHASE_ADJUST_VALUES);
	}

	/* Check master frame rate settings.  Only one video source
	 * can be master of a given compositor.  */
	if(ulMasterCount > 1)
	{
		BDBG_ERR(("CMP[%d] Can not enable multiple master framerate tracking windows",
			hCompositor->eId));
		return BERR_TRACE(BVDC_ERR_INVALID_FRAMERATE_USE);
	}

	/* Check game mode settings.  Only one video source
	 * can be in game mode of a given compositor.  */
	if(ulGameModeCount > 1)
	{
		return BERR_TRACE(BVDC_ERR_INVALID_MODE_PATH);
	}

	return eStatus;
}


/***************************************************************************
 * {private}
 *
 * Determined if new user settings are valid.  It validates all new
 * settings of windows within this compositor.
 */
BERR_Code BVDC_P_Compositor_ValidateChanges
	( const BVDC_Compositor_Handle     ahCompositor[] )
{
	BERR_Code eStatus;
	int i;

	BDBG_ENTER(BVDC_P_Compositor_ValidateChanges);


	for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
	{
		if(BVDC_P_STATE_IS_ACTIVE(ahCompositor[i]) ||
		   BVDC_P_STATE_IS_CREATE(ahCompositor[i]) ||
		   BVDC_P_STATE_IS_DESTROY(ahCompositor[i]))
		{
			eStatus = BVDC_P_Compositor_Validate(ahCompositor[i]);
			if(BERR_SUCCESS != eStatus)
			{
				return BERR_TRACE(eStatus);
			}
		}
	}

	BDBG_LEAVE(BVDC_P_Compositor_ValidateChanges);
	return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 * All new window/compositor states are good.  Now we only need to apply
 * those changes when building the next RUL.
 */
void BVDC_P_Compositor_ApplyChanges_isr
	( BVDC_Compositor_Handle           hCompositor )
{
	BVDC_P_Compositor_Info *pNewInfo;
	BVDC_P_Compositor_Info *pCurInfo;
	BVDC_P_Compositor_DirtyBits *pNewDirty;
	BVDC_P_Compositor_DirtyBits *pCurDirty;
	BVDC_ClipRect *pNewRec;
	BVDC_ClipRect *pCurRec;
	int i;

	BDBG_ENTER(BVDC_P_Compositor_ApplyChanges_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
	BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

	/* To reduce the amount of typing */
	pNewInfo = &hCompositor->stNewInfo;
	pCurInfo = &hCompositor->stCurInfo;

	pNewDirty = &pNewInfo->stDirty;
	pCurDirty = &pCurInfo->stDirty;

	/* State transition for display/compositor. */
	if(BVDC_P_STATE_IS_CREATE(hCompositor))
	{
		BDBG_MSG(("Compositor%d activated.", hCompositor->eId));
		hCompositor->eState = BVDC_P_State_eActive;
		BVDC_P_SET_ALL_DIRTY(pNewDirty);

		/* Check to make sure if this compositor need to handle checking for
		 * source lost. */
		if(!hCompositor->hVdc->hCmpCheckSource)
		{
			hCompositor->hVdc->hCmpCheckSource = hCompositor;
		}
	}
	else if(BVDC_P_STATE_IS_DESTROY(hCompositor))
	{
		BDBG_MSG(("Compositor%d de-activated", hCompositor->eId));
		hCompositor->eState = BVDC_P_State_eInactive;

		/* Check to make sure if this compositor need to delegate handle
		 * checking for source lost to another compositor. */
		if(hCompositor == hCompositor->hVdc->hCmpCheckSource)
		{
			hCompositor->hVdc->hCmpCheckSource = NULL;
			for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
			{
				if(BVDC_P_STATE_IS_ACTIVE(hCompositor->hVdc->ahCompositor[i]) &&
				   hCompositor->hVdc->ahCompositor[i]->hDisplay &&
				   BVDC_P_STATE_IS_ACTIVE(hCompositor->hVdc->ahCompositor[i]->hDisplay))
				{
					hCompositor->hVdc->hCmpCheckSource = hCompositor->hVdc->ahCompositor[i];
				}
			}
		}
	}
	else if(BVDC_P_STATE_IS_INACTIVE(hCompositor))
	{
		goto done;
	}

	/* Compositor is always re-enable every vsync. */
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_CTRL) &= ~(
		BCHP_MASK(CMP_0_CANVAS_CTRL, ENABLE));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_CTRL) |=  (
		BCHP_FIELD_ENUM(CMP_0_CANVAS_CTRL, ENABLE, ENABLE));

	/* Applychanges for each windows. */
	for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
	{
		if(BVDC_P_STATE_IS_ACTIVE(hCompositor->ahWindow[i]) ||
		   BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
		   BVDC_P_STATE_IS_DESTROY(hCompositor->ahWindow[i]))
		{
			BVDC_P_Window_ApplyChanges_isr(hCompositor->ahWindow[i]);
		}
	}
	hCompositor->bDspAspRatDirty = false;

	/* defer compositor size -- number of lines. */
	/* set bgcolor */
	BVDC_P_CMP_SET_REG_DATA(CMP_0_BG_COLOR, pNewInfo->ulBgColorYCrCb);

	pNewRec = &pNewInfo->stLumaRect.stRegion;
	pCurRec = &pCurInfo->stLumaRect.stRegion;

	/* Any changes in compositor property. */
	if((BVDC_P_IS_DIRTY(pNewDirty)) ||
	   (pCurInfo->pFmtInfo != pNewInfo->pFmtInfo) ||
	   (pCurInfo->eOrientation != pNewInfo->eOrientation) ||
	   (pCurInfo->ulBgColorYCrCb != pNewInfo->ulBgColorYCrCb) ||
	   (pCurRec->ulLeft   != pNewRec->ulLeft) ||
	   (pCurRec->ulRight  != pNewRec->ulRight) ||
	   (pCurRec->ulTop    != pNewRec->ulTop) ||
	   (pCurRec->ulBottom != pNewRec->ulBottom))
	{
		/* Must be careful here not to globble current dirty bits set by compositor,
		 * but rather OR them together. */
		BVDC_P_OR_ALL_DIRTY(pNewDirty, pCurDirty);

		/* Update current info here to let windows wr/rd sides see consistent info. */
		hCompositor->stCurInfo = hCompositor->stNewInfo ;

		/* Clear dirty bits since they're already OR'ed into current. */
		if ((NULL!=hCompositor->hDisplay) &&
			(BVDC_P_DISPLAY_USED_STG(hCompositor->hDisplay->eMasterTg)))
		{
			if(BVDC_P_DISPLAY_NODELAY(hCompositor->hDisplay->pStgFmtInfo, hCompositor->hDisplay->stCurInfo.pFmtInfo))
			{
				BVDC_P_CLEAN_ALL_DIRTY(pNewDirty);
				hCompositor->bUserAppliedChanges = true;
			}
		}
		else
		{
			BVDC_P_CLEAN_ALL_DIRTY(pNewDirty);
			hCompositor->bUserAppliedChanges = true;
		}
	}

done:
	BDBG_LEAVE(BVDC_P_Compositor_ApplyChanges_isr);
	return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Compositor_AbortChanges
	( BVDC_Compositor_Handle           hCompositor )
{
	int i;

	BDBG_ENTER(BVDC_P_Compositor_AbortChanges);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	/* Cancel the setting user set to the new state. */
	hCompositor->stNewInfo = hCompositor->stCurInfo;

	/* Applychanges for each windows. */
	for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
	{
		if(BVDC_P_STATE_IS_ACTIVE(hCompositor->ahWindow[i]) ||
		   BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
		   BVDC_P_STATE_IS_DESTROY(hCompositor->ahWindow[i]))
		{
			BDBG_OBJECT_ASSERT(hCompositor->ahWindow[i], BVDC_WIN);
			hCompositor->ahWindow[i]->stNewInfo = hCompositor->ahWindow[i]->stCurInfo;
		}
	}

	BDBG_LEAVE(BVDC_P_Compositor_AbortChanges);
	return;
}


/***************************************************************************
 * {private}
 *
 * Configure a color space conversion table inside a compositor.
 */
static void BVDC_P_Window_BuildCscRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_WindowId                    eWinInCmp,
	  uint32_t                         ulR_i,
	  BVDC_P_CscCfg                   *pCscCfg,
	  bool                             bBypassVideoProcessings,
	  BVDC_P_ListInfo                 *pList)
{
	uint32_t ulCscStartReg;
	uint32_t uMatrixBlockSize;
	const BVDC_P_CscCoeffs *pCscCoeffs;

#if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC
	uint32_t ulNumNLCtrlCnvBits =  BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R1_SHIFT - BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_SHIFT;
	uint32_t ulNewNLCtrl =
		(((pCscCfg->bNLXvYcc) ? 1 : 0) << (BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_XVYCC_R0_SHIFT + ulR_i)) |
		((BVDC_P_NL_CSC_CTRL_SEL_BYPASS != pCscCfg->ulNLCnv)?
		 ((1 << (BCHP_CMP_0_V0_NL_CSC_CTRL_NL_CSC_R0_SHIFT + ulR_i)) |
		  (pCscCfg->ulNLCnv << (BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_SHIFT + ulR_i * ulNumNLCtrlCnvBits))) : 0);
	uMatrixBlockSize = BVDC_P_REGS_ENTRIES(CMP_0_V0_R0_MC_COEFF_C00, CMP_0_V0_R0_MC_COEFF_C23);
#elif (BVDC_P_SUPPORT_CSC_MAT_COEF_VER >= 2)
	uMatrixBlockSize = BVDC_P_REGS_ENTRIES(CMP_0_V0_R0_MC_COEFF_C00, CMP_0_V0_R0_MC_COEFF_C23);
	#if (BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1)
	ulCscStartReg = (1==eWinInCmp)? BCHP_CMP_0_V1_R0_MC_COEFF_C00 : BCHP_CMP_0_V0_R0_MC_COEFF_C00;
	#else
	ulCscStartReg = BCHP_CMP_0_V0_R0_MC_COEFF_C00;
	BSTD_UNUSED(eWinInCmp);
	#endif
	ulCscStartReg += (ulR_i * uMatrixBlockSize * sizeof(int32_t));
#elif (BVDC_P_SUPPORT_CSC_MAT_COEF_VER == 1)
	uMatrixBlockSize = BVDC_P_REGS_ENTRIES(CMP_0_V0_COEFF_C00, CMP_0_V0_COEFF_C23);
	ulCscStartReg = (1 == ulR_i) ? (BCHP_CMP_0_SEC_V0_COEFF_C00) :
	#if (BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1)
		((1==eWinInCmp)? BCHP_CMP_0_V1_COEFF_C00 : BCHP_CMP_0_V0_COEFF_C00);
	#else
		BCHP_CMP_0_V0_COEFF_C00;
	BSTD_UNUSED(eWinInCmp);
	#endif
#else
	uMatrixBlockSize = BVDC_P_REGS_ENTRIES(CMP_0_V0_COEFF_C01_C00, CMP_0_V0_COEFF_C23_C22);
	ulCscStartReg = (1 == ulR_i) ? (BCHP_CMP_0_SEC_V0_COEFF_C01_C00) :
	#if (BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1)
		((1==eWinInCmp)? BCHP_CMP_0_V1_COEFF_C01_C00 : BCHP_CMP_0_V0_COEFF_C01_C00);
	#else
		BCHP_CMP_0_V0_COEFF_C01_C00;
	BSTD_UNUSED(eWinInCmp);
	#endif
#endif

	BDBG_ENTER(BVDC_P_Window_BuildCscRul_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

#if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC
	if (hCompositor->bSupportNLCsc[eWinInCmp])
	{
		/* Programming NL_CSC_CTRL
		 */
		ulCscStartReg = (1==eWinInCmp)? BCHP_CMP_0_V1_NL_CSC_CTRL : BCHP_CMP_0_V0_NL_CSC_CTRL;
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		hCompositor->ulNLCscCtrl[eWinInCmp] |= ulNewNLCtrl; /* |= prev call for R = 0, 1, ..., (ulR_i - 1) */
		*pList->pulCurrent++ = BRDC_REGISTER(ulCscStartReg + hCompositor->ulRegOffset);
		*pList->pulCurrent++ = (bBypassVideoProcessings)? 0 : hCompositor->ulNLCscCtrl[eWinInCmp];
		ulCscStartReg = (1==eWinInCmp)? BCHP_CMP_0_V1_R0_MA_COEFF_C00 : BCHP_CMP_0_V0_R0_MA_COEFF_C00;
		ulCscStartReg += (ulR_i * uMatrixBlockSize * sizeof(int32_t));
		BDBG_MODULE_MSG(BVDC_CMP_CSC,("cmp[%d]win[%d]cscSlot[%d] CMP_CSC_NLCtrl: 0x%x",
			hCompositor->eId, eWinInCmp, ulR_i, hCompositor->ulNLCscCtrl[eWinInCmp]));

		if (hCompositor->bSupportMACsc[eWinInCmp])
		{
			/* Programming MA
			 */
			pCscCoeffs = pCscCfg->pCscMA;
			*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS( uMatrixBlockSize );
			*pList->pulCurrent++ = BRDC_REGISTER( ulCscStartReg + hCompositor->ulRegOffset);
			BDBG_MODULE_MSG(BVDC_CMP_CSC,("cmp[%d]win[%d]cscSlot[%d] CMP_CSC_MA:", hCompositor->eId, eWinInCmp, ulR_i));
			BVDC_P_Csc_Print_isr(pCscCoeffs);

			/* [ c00, c01 c02 c03 ] */
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C00, COEFF_MUL, pCscCoeffs->usY0);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C01, COEFF_MUL, pCscCoeffs->usY1);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C02, COEFF_MUL, pCscCoeffs->usY2);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C03, COEFF_ADD, pCscCoeffs->usYOffset);

			/* [ c10, c11 c12 c13 ] */
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C10, COEFF_MUL, pCscCoeffs->usCb0);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C11, COEFF_MUL, pCscCoeffs->usCb1);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C12, COEFF_MUL, pCscCoeffs->usCb2);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C13, COEFF_ADD, pCscCoeffs->usCbOffset);

			/* [ c20, c21 c22 c23 ] */
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C20, COEFF_MUL, pCscCoeffs->usCr0);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C21, COEFF_MUL, pCscCoeffs->usCr1);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C22, COEFF_MUL, pCscCoeffs->usCr2);
			*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MA_COEFF_C23, COEFF_ADD, pCscCoeffs->usCrOffset);
		}

		/* update ulCscStartReg for MC */
		ulCscStartReg += (BCHP_CMP_0_V0_R0_MC_COEFF_C00 - BCHP_CMP_0_V0_R0_MA_COEFF_C00);
	}
	else
	{
		ulCscStartReg = (1==eWinInCmp)? BCHP_CMP_0_V1_R0_MC_COEFF_C00 : BCHP_CMP_0_V0_R0_MC_COEFF_C00;
		ulCscStartReg += (ulR_i * uMatrixBlockSize * sizeof(int32_t));
	}
#else
	BSTD_UNUSED(bBypassVideoProcessings);
#endif /* #if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC */

	/* Programming MC
	 */
	pCscCoeffs = &pCscCfg->stCscMC;
	BDBG_MODULE_MSG(BVDC_CMP_CSC,("cmp[%d]win[%d]cscSlot[%d] CMP_CSC_MC:", hCompositor->eId, eWinInCmp, ulR_i));
	BVDC_P_Csc_Print_isr(pCscCoeffs);

	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS( uMatrixBlockSize );
	*pList->pulCurrent++ = BRDC_REGISTER( ulCscStartReg + hCompositor->ulRegOffset);

#if (BVDC_P_SUPPORT_CSC_MAT_COEF_VER >= 2)
	/* [ c00, c01 c02 c03 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C00, COEFF_MUL, pCscCoeffs->usY0);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C01, COEFF_MUL, pCscCoeffs->usY1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C02, COEFF_MUL, pCscCoeffs->usY2);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C03, COEFF_ADD, pCscCoeffs->usYOffset);

	/* [ c10, c11 c12 c13 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C10, COEFF_MUL, pCscCoeffs->usCb0);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C11, COEFF_MUL, pCscCoeffs->usCb1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C12, COEFF_MUL, pCscCoeffs->usCb2);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C13, COEFF_ADD, pCscCoeffs->usCbOffset);

	/* [ c20, c21 c22 c23 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C20, COEFF_MUL, pCscCoeffs->usCr0);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C21, COEFF_MUL, pCscCoeffs->usCr1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C22, COEFF_MUL, pCscCoeffs->usCr2);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_R0_MC_COEFF_C23, COEFF_ADD, pCscCoeffs->usCrOffset);

#elif (BVDC_P_SUPPORT_CSC_MAT_COEF_VER == 1)
	/* [ c00, c01 c02 c03 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C00, COEFF_MUL, pCscCoeffs->usY0);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C01, COEFF_MUL, pCscCoeffs->usY1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C02, COEFF_MUL, pCscCoeffs->usY2);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C03, COEFF_ADD, pCscCoeffs->usYOffset);

	/* [ c10, c11 c12 c13 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C10, COEFF_MUL, pCscCoeffs->usCb0);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C11, COEFF_MUL, pCscCoeffs->usCb1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C12, COEFF_MUL, pCscCoeffs->usCb2);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C13, COEFF_ADD, pCscCoeffs->usCbOffset);

	/* [ c20, c21 c22 c23 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C20, COEFF_MUL, pCscCoeffs->usCr0);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C21, COEFF_MUL, pCscCoeffs->usCr1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C22, COEFF_MUL, pCscCoeffs->usCr2);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C23, COEFF_ADD, pCscCoeffs->usCrOffset);
#else
	/* [ c00, c01 c02 c03 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C01_C00, COEFF_C0, pCscCoeffs->usY0) |
	                       BCHP_FIELD_DATA(CMP_0_V0_COEFF_C01_C00, COEFF_C1, pCscCoeffs->usY1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C03_C02, COEFF_C2, pCscCoeffs->usY2) |
	                       BCHP_FIELD_DATA(CMP_0_V0_COEFF_C03_C02, COEFF_C3, pCscCoeffs->usYOffset);

	/* [ c10, c11 c12 c13 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C11_C10, COEFF_C0, pCscCoeffs->usCb0) |
	                       BCHP_FIELD_DATA(CMP_0_V0_COEFF_C11_C10, COEFF_C1, pCscCoeffs->usCb1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C13_C12, COEFF_C2, pCscCoeffs->usCb2) |
	                       BCHP_FIELD_DATA(CMP_0_V0_COEFF_C13_C12, COEFF_C3, pCscCoeffs->usCbOffset);

	/* [ c20, c21 c22 c23 ] */
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C21_C20, COEFF_C0, pCscCoeffs->usCr0) |
	                       BCHP_FIELD_DATA(CMP_0_V0_COEFF_C21_C20, COEFF_C1, pCscCoeffs->usCr1);
	*pList->pulCurrent++ = BCHP_FIELD_DATA(CMP_0_V0_COEFF_C23_C22, COEFF_C2, pCscCoeffs->usCr2) |
	                       BCHP_FIELD_DATA(CMP_0_V0_COEFF_C23_C22, COEFF_C3, pCscCoeffs->usCrOffset);
#endif

	BDBG_LEAVE(BVDC_P_Window_BuildCscRul_isr);
	return;
}


#if (BVDC_P_SUPPORT_COLOR_CLIP)
/*************************************************************************
 *  {secret}
 *	BVDC_P_ColorClip_BuildRul_isr
 *	Builds Color Clip block
 **************************************************************************/
static void BVDC_P_Compositor_BuildColorClipRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList)
{
	BVDC_ColorClipSettings *pColorClipSettings;

	BDBG_ENTER(BVDC_P_Compositor_BuildColorClipRul_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	pColorClipSettings = &hCompositor->stCurInfo.stColorClipSettings;

		BDBG_MSG(("Color Clip Mode = %d", pColorClipSettings->eColorClipMode));

	/* Build RUL */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_COLOR_CLIP_CTRL);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, CLIP_COUNT_RESET, 0) |
		BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, ENABLE,
			(pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_None) ? 0 : 1) |
		BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, WHITE_DISABLE,
			(pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_White ||
			 pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_Both) ? 0 : 1) |
		BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, BLACK_DISABLE,
			(pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_Black ||
			 pColorClipSettings->eColorClipMode == BVDC_ColorClipMode_Both) ? 0 : 1) |
		BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, EXTENDED_WHITE_RANGE,
			pColorClipSettings->bExtendedWhite ? 1 : 0) |
		BCHP_FIELD_DATA(CMP_0_COLOR_CLIP_CTRL, EXTENDED_BLACK_RANGE,
			pColorClipSettings->bExtendedBlack ? 1 : 0);

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_CB_LUMA_SLOPE);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(CMP_0_CB_LUMA_SLOPE, SLOPE_B, (pColorClipSettings->ulCbYSlopeB >> 8) & 0x7FF) |
		BCHP_FIELD_DATA(CMP_0_CB_LUMA_SLOPE, SLOPE_A, (pColorClipSettings->ulCbYSlopeA >> 8) & 0x7FF);

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_CR_LUMA_SLOPE);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(CMP_0_CR_LUMA_SLOPE, SLOPE_B, (pColorClipSettings->ulCrYSlopeB >> 8) & 0x7FF) |
		BCHP_FIELD_DATA(CMP_0_CR_LUMA_SLOPE, SLOPE_A, (pColorClipSettings->ulCrYSlopeA >> 8) & 0x7FF);

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_CB_CR_SLOPE_JOINT);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(CMP_0_CB_CR_SLOPE_JOINT, CB_SLOPE_JOINT, pColorClipSettings->ulCbJoint) |
		BCHP_FIELD_DATA(CMP_0_CB_CR_SLOPE_JOINT, CR_SLOPE_JOINT, pColorClipSettings->ulCrJoint);

	BDBG_LEAVE(BVDC_P_ColorClip_BuildRul_isr);
	return;
}
#endif

/***************************************************************************
 * {private}
 *
 * This function adds the CMP_x_Vx registers into pList.  This functions
 * assumes that the shadowed-registers have been updated, and dirty bits
 * are set approriately.
 */

static void BVDC_P_Compositor_BuildRul_Graphics_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList)
{
	/* Gwin0 */
	if(BVDC_P_CMP_COMPARE_FIELD_DATA(CMP_0_G0_SURFACE_CTRL, ENABLE, 1))
	{
		BVDC_P_CMP_BLOCK_WRITE_TO_RUL(CMP_0_G0_SURFACE_SIZE, CMP_0_G0_CANVAS_OFFSET, pList->pulCurrent);
#if BVDC_P_SUPPORT_3D_VIDEO
		BVDC_P_CMP_WRITE_TO_RUL(CMP_0_G0_CANVAS_X_OFFSET_R, pList->pulCurrent);
#endif
	}

#if (BVDC_P_SUPPORT_CSC_MAT_COEF_VER >= 2)
	if(hCompositor->stCurInfo.stDirty.stBits.bColorClip || hCompositor->bInitial)
	{
		/* CMP color clip configuration is only supported by compositor 0 of certain
		 * chipsets.
		 */
#if (BVDC_P_SUPPORT_COLOR_CLIP)
		if(hCompositor->eId == BVDC_CompositorId_eCompositor0)
			BVDC_P_Compositor_BuildColorClipRul_isr(hCompositor, pList);
#endif
		hCompositor->stCurInfo.stDirty.stBits.bColorClip = BVDC_P_CLEAN;
	}
#endif
}

#ifndef BVDC_FOR_BOOTUPDATER
/***************************************************************************
 * {private}
 *
 * This function adds the CMP_x_Vx registers into pList.  This functions
 * assumes that the shadowed-registers have been updated, and dirty bits
 * are set approriately.
 */

static void BVDC_P_Compositor_BuildRul_Video_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList,
	  BVDC_P_WindowId                  eVId,     /* window index */
	  BVDC_WindowId                    eWinInCmp) /* 0 or 1 */
{
	uint32_t ulV0V1Offset = 0;

#if (BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT > 1)
	if(hCompositor->pFeatures->ulMaxVideoWindow > 1)
		ulV0V1Offset = eWinInCmp*(BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_V0_SURFACE_SIZE);
#else
	BSTD_UNUSED(eWinInCmp);
#endif

#if BVDC_P_SUPPORT_WIN_CONST_COLOR
		/* window constant color settings */
	BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_CONST_COLOR, ulV0V1Offset, pList->pulCurrent);
#endif

	if(BVDC_P_CMP_OFFSET_COMPARE_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, ulV0V1Offset, SURFACE_ENABLE, 1))
	{
		if (((BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE, ulV0V1Offset, HSIZE) +
			  BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, X_OFFSET)) >
			 (BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE, ulV0V1Offset, HSIZE))) ||
			((BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE, ulV0V1Offset, VSIZE) +
			  BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, Y_OFFSET)) >
			 (BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE, ulV0V1Offset, VSIZE))))
		{
			uint32_t  ulDspW, ulCutLeft, ulDspH, ulCutTop;

			BDBG_ERR(("Invalid CMP V%d disp size [H:%d, V:%d] vs surf size [H:%d, V:%d] - [X:%d, Y:%d]",
					  (uint32_t)eVId,
					  BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset, HSIZE),
					  BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset, VSIZE),
					  BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,   ulV0V1Offset, HSIZE),
					  BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,   ulV0V1Offset, VSIZE),
					  BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, X_OFFSET),
					  BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, Y_OFFSET)));

			ulDspW    = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset, HSIZE);
			ulCutLeft = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, X_OFFSET);
			ulDspH    = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset, VSIZE);
			ulCutTop  = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset, Y_OFFSET);

			if ((BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset,HSIZE) +
				 BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset,X_OFFSET)) >
				(BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,   ulV0V1Offset,HSIZE)))
			{
				ulDspW = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,ulV0V1Offset, HSIZE);
				ulCutLeft = 0;
			}

			if ((BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE,   ulV0V1Offset,VSIZE) +
				 BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset,Y_OFFSET)) >
				(BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE,   ulV0V1Offset,VSIZE)))
			{
				ulDspH = BVDC_P_CMP_OFFSET_GET_FIELD_NAME(CMP_0_V0_SURFACE_SIZE, ulV0V1Offset,VSIZE);
				ulCutTop = 0;
			}

			BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_DISPLAY_SIZE, ulV0V1Offset) =
				BCHP_FIELD_DATA(CMP_0_V0_DISPLAY_SIZE, HSIZE, ulDspW) |
				BCHP_FIELD_DATA(CMP_0_V0_DISPLAY_SIZE, VSIZE, ulDspH);
			BVDC_P_CMP_OFFSET_GET_REG_DATA(CMP_0_V0_SURFACE_OFFSET, ulV0V1Offset) =
				BCHP_FIELD_DATA(CMP_0_V0_SURFACE_OFFSET, X_OFFSET, ulCutLeft) |
				BCHP_FIELD_DATA(CMP_0_V0_SURFACE_OFFSET, Y_OFFSET, ulCutTop);
		}

		BVDC_P_CMP_OFFSET_BLOCK_WRITE_TO_RUL(CMP_0_V0_SURFACE_SIZE, CMP_0_V0_CANVAS_OFFSET, ulV0V1Offset, pList->pulCurrent);

#if BVDC_P_SUPPORT_3D_VIDEO
		BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_CANVAS_X_OFFSET_R, ulV0V1Offset, pList->pulCurrent);
#endif

#if (BVDC_P_SUPPORT_DITHER)
		BVDC_P_CMP_OFFSET_BLOCK_WRITE_TO_RUL(CMP_0_V0_MC_DITHER_CTRL, CMP_0_V0_MC_DITHER_LFSR_CTRL, ulV0V1Offset, pList->pulCurrent);
#endif

#if BVDC_P_SUPPORT_CMP_CLEAR_RECT_V0
		if(hCompositor->ulMosaicAdjust[eVId] || hCompositor->bInitial)
		{
			if(hCompositor->ahWindow[eVId]->stCurInfo.bClearRect)
			{
				BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_RECT_COLOR, ulV0V1Offset, pList->pulCurrent);
				BVDC_P_CMP_OFFSET_BLOCK_WRITE_TO_RUL(CMP_0_V0_RECT_TOP_CTRL, CMP_0_V0_RECT_ENABLE_MASK, ulV0V1Offset, pList->pulCurrent);
				BVDC_P_CMP_OFFSET_RECT_BLOCK_WRITE_TO_RUL(CMP_0_V0_RECT_SIZEi_ARRAY_BASE,
					hCompositor->ahWindow[eVId]->stCurInfo.ulMosaicCount, ulV0V1Offset, pList->pulCurrent);
				BVDC_P_CMP_OFFSET_RECT_BLOCK_WRITE_TO_RUL(CMP_0_V0_RECT_OFFSETi_ARRAY_BASE,
					hCompositor->ahWindow[eVId]->stCurInfo.ulMosaicCount, ulV0V1Offset, pList->pulCurrent);
			}
			else
			{
				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
				*pList->pulCurrent++ = BRDC_REGISTER(BCHP_CMP_0_V0_RECT_TOP_CTRL + hCompositor->ulRegOffset + ulV0V1Offset);
				*pList->pulCurrent++ = 0;
			}

			if(pList->bLastExecuted && hCompositor->ulMosaicAdjust[eVId])
			{
				hCompositor->ulMosaicAdjust[eVId]--;
			}
		}
#endif

		if(hCompositor->ulCscAdjust[eVId])
		{
			bool bBypassVideoProcessings = false;
			BVDC_Window_Handle hWindow = hCompositor->ahWindow[eVId];
			BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
			bBypassVideoProcessings = hWindow->stSettings.bBypassVideoProcessings ||
				(hCompositor->eTransferCharacteristics == BAVC_TransferCharacteristics_eArib_STD_B67) ||
				(hCompositor->eTransferCharacteristics == BAVC_TransferCharacteristics_eSmpte_ST_2084);

			if(hCompositor->bCscCompute[eVId])
			{
#if BVDC_P_SUPPORT_CMP_MOSAIC_CSC
				if(hCompositor->ahWindow[eVId]->stCurInfo.bClearRect)
				{
					BVDC_P_Window_CalculateMosaicCsc_isr(hCompositor->ahWindow[eVId]);
				}
				else
#endif
					BVDC_P_Window_CalculateCsc_isr(hCompositor->ahWindow[eVId]);
				hCompositor->bCscCompute[eVId] = false;
			}
			if(hCompositor->bCscDemoCompute[eVId])
			{
				BVDC_P_Window_SetSecCscDemo_isr(hCompositor->ahWindow[eVId]);
				hCompositor->bCscDemoCompute[eVId] = false;
			}

			hCompositor->ulNLCscCtrl[eWinInCmp] = 0;
#if BVDC_P_SUPPORT_CMP_MOSAIC_CSC
			/* TODO: Add dirty bits */
			/* Mosaic mode with CMP mosaic cscs. */
			if(hCompositor->ahWindow[eVId]->stCurInfo.bClearRect)
			{
				uint32_t i;
				for (i = 0; i < BVDC_P_SUPPORT_CMP_MOSAIC_CSC; i++)
				{
					BVDC_P_Window_BuildCscRul_isr(hCompositor, eWinInCmp, i, &hWindow->astMosaicCscList[i], bBypassVideoProcessings, pList);
				}

				/* Write mosaic csc indices. */
				BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_RECT_CSC_INDEX_0 , ulV0V1Offset, pList->pulCurrent);
				if(hCompositor->ahWindow[eVId]->stSettings.ulMaxMosaicRect > 8)
				{
					BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_RECT_CSC_INDEX_1 , ulV0V1Offset, pList->pulCurrent);
				}
			}
			else
#endif
			{
				BVDC_P_Window_BuildCscRul_isr(hCompositor, eWinInCmp, 0, &hWindow->stPriCsc, bBypassVideoProcessings, pList);
			}

			{
				BVDC_P_Window_BuildCscRul_isr(hCompositor, eWinInCmp, 1, &hWindow->stSecCsc, bBypassVideoProcessings, pList);
			}

			if(pList->bLastExecuted)
			{
				hCompositor->ulCscAdjust[eVId]--;
			}
		}
	}

	/* Color key */
	if( hCompositor->ulColorKeyAdjust[eVId] )
	{
		if(pList->bLastExecuted)
		{
			hCompositor->ulColorKeyAdjust[eVId]--;
		}
		/* CMP_0_V0_LUMA_KEYING, CMP_0_V0_CB_KEYING and CMP_0_V0_CR_KEYING
		 * are not always together */
		BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_LUMA_KEYING, ulV0V1Offset, pList->pulCurrent);
		BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_CB_KEYING,   ulV0V1Offset, pList->pulCurrent);
		BVDC_P_CMP_OFFSET_WRITE_TO_RUL(CMP_0_V0_CR_KEYING,   ulV0V1Offset, pList->pulCurrent);
	}
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/***************************************************************************
 * {private}
 *
 * This function adds the CMP_x registers into pList.  This functions
 * assumes that the shadowed-registers have been updated, and dirty bits
 * are set approriately.
 */
static void BVDC_P_Compositor_BuildRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldId )
{
	BVDC_P_Window_DirtyBits *pV0CurDirty;
	BVDC_P_WindowId eV0Id, eV1Id = BVDC_P_WindowId_eComp0_V1;
	uint32_t i;

	BDBG_ENTER(BVDC_P_Compositor_BuildRul_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
	BSTD_UNUSED(eFieldId);

	if(hCompositor->bIsBypass)
	{
		/* Bypass compositor (dummy one) has nothing to program except
		 * video path controlling. */
		pV0CurDirty = &(hCompositor->ahWindow[BVDC_P_WindowId_eComp2_V0]->stCurInfo.stDirty);

		/* CSC already handle in 656out clear dirty bit now. */
		pV0CurDirty->stBits.bCscAdjust = BVDC_P_CLEAN;
		pV0CurDirty->stBits.bCabAdjust = BVDC_P_CLEAN;
		pV0CurDirty->stBits.bLabAdjust = BVDC_P_CLEAN;
		pV0CurDirty->stBits.bTntAdjust = BVDC_P_CLEAN;
		return;
	}

	eV0Id = BVDC_P_CMP_GET_V0ID(hCompositor);

	if(hCompositor->pFeatures->ulMaxVideoWindow > 1)
	{
		eV1Id = hCompositor->eId? BVDC_P_WindowId_eComp1_V1 : BVDC_P_WindowId_eComp0_V1;
	}

	/* reset */
	if(hCompositor->bInitial)
	{
		/* Make sure these get re-programm when reset. */
		hCompositor->ulCscAdjust[eV0Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
		hCompositor->ulColorKeyAdjust[eV0Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
		hCompositor->bCscDemoCompute[eV0Id] = true;

		if(hCompositor->pFeatures->ulMaxVideoWindow > 1)
		{
			hCompositor->ulCscAdjust[eV1Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
			hCompositor->ulColorKeyAdjust[eV1Id] = BVDC_P_RUL_UPDATE_THRESHOLD;
			hCompositor->bCscDemoCompute[eV1Id] = true;
		}
		/*remove the software init after 7425*/
#if (!BVDC_P_SUPPORT_3D_VIDEO)
		BVDC_P_BUILD_RESET(pList->pulCurrent,
			hCompositor->ulCoreResetAddr, hCompositor->ulCoreResetMask);
#endif

#if BVDC_P_SUPPORT_CMP_CRC
		/* Also make sure to enable CRC */
		BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CRC_CTRL, pList->pulCurrent);
#endif

		BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CMP_OUT_CTRL, pList->pulCurrent);
	}

#if BVDC_P_SUPPORT_OSCL
	/* Build OSCL RUL if it presents */
	if (BVDC_CompositorId_eCompositor0 == hCompositor->eId)
	{
		BVDC_P_OSCL_BuildRul_isr(hCompositor, pList, eFieldId);
	}
#endif

#ifndef BVDC_FOR_BOOTUPDATER
	if(eV0Id == BVDC_P_WindowId_eComp0_V0)
	{
		BVDC_P_Pep_BuildRul_isr(hCompositor->ahWindow[eV0Id], pList, hCompositor->bInitial);
	}
#endif

	/* canvas, bgcolor and blender setting */
	BVDC_P_CMP_BLOCK_WRITE_TO_RUL(CMP_0_CANVAS_SIZE, CMP_0_BG_COLOR, pList->pulCurrent);
	BVDC_P_CMP_WRITE_TO_RUL(CMP_0_BLEND_0_CTRL, pList->pulCurrent);

#ifndef BVDC_FOR_BOOTUPDATER
#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 1
	if(hCompositor->pFeatures->ulMaxWindow > 1)
	{
		BVDC_P_CMP_WRITE_TO_RUL(CMP_0_BLEND_1_CTRL, pList->pulCurrent);
	}
#endif

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 2
	if(hCompositor->pFeatures->ulMaxWindow > 2)
	{
		BVDC_P_CMP_WRITE_TO_RUL(CMP_0_BLEND_2_CTRL, pList->pulCurrent);
	}
#endif

#if BVDC_P_CMP_0_MAX_WINDOW_COUNT > 3
	if(hCompositor->pFeatures->ulMaxWindow > 3)
	{
		BVDC_P_CMP_WRITE_TO_RUL(CMP_0_BLEND_3_CTRL, pList->pulCurrent);
	}
#endif


#if BVDC_P_SUPPORT_3D_VIDEO
	BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CMP_CTRL, pList->pulCurrent);
#endif

	/* TODO:  Optimize with dirty bits */
#if (BVDC_P_SUPPORT_CMP_LUMA_AVG_VER > 0)
	BVDC_P_CMP_WRITE_TO_RUL(CMP_0_LUMA_AVG_CTRL,           pList->pulCurrent);
	BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CMP_LUMA_AVG_WIN_OFFSET, pList->pulCurrent);
	BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CMP_LUMA_AVG_WIN_SIZE,   pList->pulCurrent);
#endif

#if BVDC_P_SUPPORT_CMP_DEMO_MODE
	BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CSC_DEMO_SETTING, pList->pulCurrent);
#endif

	/* Vwin0 */
	for(i=eV0Id; i< (eV0Id + hCompositor->pFeatures->ulMaxVideoWindow); i++)
	{
		BVDC_P_Compositor_BuildRul_Video_isr(hCompositor, pList, (BVDC_P_WindowId)i, (BVDC_WindowId)(i - eV0Id));
	}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

	/* Gwin0 */
	BVDC_P_Compositor_BuildRul_Graphics_isr(hCompositor, pList);

	if(hCompositor->bInitial)
	{
		hCompositor->bInitial = false;
	}

	BDBG_LEAVE(BVDC_P_Compositor_BuildRul_isr);
	return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Compositor_BuildSyncLockRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldId )
{
	int i;
	bool abForceSlot[BVDC_P_MAX_SOURCE_COUNT];
	BDBG_ENTER(BVDC_P_Compositor_BuildSyncLockRul_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	hCompositor->hForceTrigPipSrc = NULL;

	/* to handle multiple windows coming from the same mpg source */
	for(i = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
	{
		abForceSlot[i] = false;
	}

	/* Add an entry into the RUL with sole purpose of force trigger the
	 * the source slot.  Take advantage of the way register layout, each
	 * slot immediate trigger is 16 bytes apart.
	 *
	 * There are two cases where we want to force trigger the source's
	 * slot RUL.
	 *
	 * (1) The mpeg sync-lock source the LOCKED to this compositor.
	 * (2) The other mpeg source's that hasn't connected to any compositor. */
	for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
	{
		BVDC_Source_Handle hSource;

		/* SKIP: If it's just created or inactive no need to build ruls. */
		if(!hCompositor->ahWindow[i] ||
			BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
			BVDC_P_STATE_IS_INACTIVE(hCompositor->ahWindow[i]))
		{
			continue;
		}

		BDBG_OBJECT_ASSERT(hCompositor->ahWindow[i], BVDC_WIN);
		if((hCompositor->ahWindow[i]->bUserAppliedChanges) &&
		   (hCompositor->ahWindow[i]->stCurInfo.hSource == NULL))
		{
			hSource = hCompositor->ahWindow[i]->stNewInfo.hSource;
		}
		else
		{
			hSource = hCompositor->ahWindow[i]->stCurInfo.hSource;
		}
		BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

		/* Force trigger mpeg slot:
		 * 1) MPEG Sync-lock source required force trigger.
		 * 2) MPEG PIP source not locking to any compositor yet and not triggered by MTG. */
		if((BVDC_P_SRC_IS_MPEG(hSource->eId)) &&
		   (!abForceSlot[hSource->eId]) &&
#if BVDC_P_SUPPORT_MTG
		   (!hSource->bMtgSrc) &&
#endif
		   ((hSource->hSyncLockCompositor == hCompositor) ||
		    (hSource->hSyncLockCompositor == NULL)))
		{
			/* Note, we invert the field polarity if it's locked with a captured mpeg src
			 *   to adapt to 2-field multi-buffering, which puts read pointer one field
			 *   behind write pointer; the trigger field swap only makes sense with
			 *   active window and interlaced formats;
			 * PR43082: This flag needs to reflect the total BVN delay of the sync-locked
			 * window to achieve precise lipsync goal with HD/SD simul mode; */
			/* FIELDSWAP 1) detection; */
			if(BVDC_P_STATE_IS_ACTIVE(hCompositor->hSyncLockWin))
			{
				hSource->bFieldSwap = (hCompositor->hSyncLockWin->ulTotalBvnDelay & 1);
				BDBG_MSG(("CMP[%d], SRC[%d], WIN[%d], bFieldSwap[%d], ulTotalBvnDelay[%d] pol[%u]",
					hCompositor->eId,
					hSource->eId,
					hCompositor->hSyncLockWin->eId,
					hSource->bFieldSwap,
					hCompositor->hSyncLockWin->ulTotalBvnDelay, eFieldId));
			}

			/* Force source slot to execute. */
			if((hSource->hSyncLockCompositor == NULL) && (hSource->ulTransferLock))
			{
				BDBG_MSG(("Don't force trigger the PIP window yet"));
				BVDC_P_BUILD_NO_OPS(pList->pulCurrent);

				/* Note: this is a backup to clear the transfer lock semaphore, in case
				   the critical source _isr, responsible to clear semaphore, is called
				   too early (_isr call disorder issue) such that it doesn't see the
				   semaphore, and source disconnection display _isr sets semaphore later
				   and stops building force trigger RUL for that mpeg source; that mpeg
				   source could lose driving interrupt forever! */
				if(--hSource->ulTransferLock == 0)
				{
					/* clean up source slots to prepare for the trigger transfer */
					BVDC_P_Source_CleanupSlots_isr(hSource);
				}
				BDBG_MSG(("CMP[%d] decrements SRC[%d]'s ulTransferLock semaphore count %d",
					hCompositor->eId, hSource->eId, hSource->ulTransferLock));
			}
			else
			{
				abForceSlot[hSource->eId] = true;
				/* FIELDSWAP 3) force field swapped trigger; */
				/* interlaced non real time transcode mode*/
#if BVDC_P_SUPPORT_STG
				if((eFieldId != BAVC_Polarity_eFrame) &&
					(BVDC_P_DISPLAY_NRT_STG(hCompositor->hDisplay)))
				{
					bool bToggleSrcPolarity = hSource->bFieldSwap;
#if BVDC_P_STG_NRT_CADENCE_WORKAROUND /* if stg hw cannot repeat pol for ignore pic, accounted here */
					/* if stg hw cannot repeat cadence, after workaround forced repeat src cadence, toggle display cadence here */
					/* NOTE: the src cadence toggle flag is derived at end of last round of NRT src/disp isr pair! */
					bToggleSrcPolarity ^= hSource->bToggleCadence;
					BDBG_MSG(("Cmp[%u] to toggle pol cadence = %u [%u]", hCompositor->eId, hSource->bToggleCadence, bToggleSrcPolarity));
#endif
					*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
					BDBG_MODULE_MSG(repeatpolarity,("use src[%d] ignore flag", hSource->eId));
					*pList->pulCurrent++ = BRDC_REGISTER(hSource->ulScratchPolReg);
					*pList->pulCurrent++ = BRDC_OP_VAR_XOR_IMM_TO_VAR(BRDC_Variable_0, BRDC_Variable_1);

					if((bToggleSrcPolarity? BVDC_P_NEXT_POLARITY(eFieldId) : eFieldId) == BAVC_Polarity_eTopField) {
						*pList->pulCurrent++ = 0;
					} else /* bottom */ {
						*pList->pulCurrent++ = 1;
					}
					*pList->pulCurrent++ = BRDC_OP_COND_SKIP(BRDC_Variable_1);
					*pList->pulCurrent++ = 5;/* if bottom(1) slot, skip 5 dwords */
					BVDC_P_BUILD_IMM_EXEC_OPS(pList->pulCurrent, hSource->aulImmTriggerAddr[BAVC_Polarity_eTopField]);
					*pList->pulCurrent++ = BRDC_OP_SKIP();/* continue by skip 3 dwords bot RUL; */
					*pList->pulCurrent++ = 3;/* 3-dword for immediate trigger RUL */
					BVDC_P_BUILD_IMM_EXEC_OPS(pList->pulCurrent, hSource->aulImmTriggerAddr[BAVC_Polarity_eBotField]);

				} else
#endif
					BVDC_P_BUILD_IMM_EXEC_OPS(pList->pulCurrent, hSource->aulImmTriggerAddr[
						hSource->bFieldSwap ? BVDC_P_NEXT_POLARITY(eFieldId) : eFieldId]);

				/* PsF: source refresh rate is frame rate */
				/* The refresh rate of this source is driven by refresh rate
				 * of this display */
				if(!(hSource->stCurInfo.bPsfEnable || hSource->stCurInfo.bForceFrameCapture))
				{
					/* this value may differentiate 59.94 vs 60 for HD formats that support
					    multiple frame rates. */
					hSource->ulVertFreq = hCompositor->hDisplay->stCurInfo.ulVertFreq;
				}
				hSource->ulDispVsyncFreq = hCompositor->hDisplay->stCurInfo.ulVertFreq;
			}

			/* Assume:
			   1) two mpg sources at most in system;
			   2) mpg source can only be sync slipped if it's in the PIP of a sync-locked
			      compositor; */
			if(hSource->hSyncLockCompositor == NULL)
			{
				hCompositor->hForceTrigPipSrc = hSource;
			}
		}
	}

	BDBG_LEAVE(BVDC_P_Compositor_BuildSyncLockRul_isr);
	return;
}


/***************************************************************************
 * {private}
 *
 */
bool BVDC_P_Compositor_BuildSyncSlipRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldId,
	  bool                             bBuildCanvasCtrl )
{
	int i;
	bool bBuilldBothTopBotSlots = false;

	BDBG_ENTER(BVDC_P_Compositor_BuildSyncSlipRul_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
	BDBG_OBJECT_ASSERT(hCompositor->hDisplay, BVDC_DSP);

	/* Now update windows state (including setInfo) for next picture to be displayed. */
	BVDC_P_Compositor_WindowsReader_isr(hCompositor, eFieldId, pList);

	/* Build sync slip Rul.  Basically this build playback portion of
	 * RUL.  Most likely it will be the VEC, CMP, VFDs, GFD, [SCLs], and
	 * VNET.  If no window is active (just bgcolor) it will just build the
	 * VEC, CMP, and VNET. */
	BVDC_P_Vec_BuildRul_isr(hCompositor->hDisplay, pList, eFieldId);

	/* Build Compositor RUL */
	BVDC_P_Compositor_BuildRul_isr(hCompositor, pList, eFieldId);

	/* Build related video backend blocks that associated with active windows.
	 * Only build the READER portion of the windows. */
	for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
	{
		/* SKIP: If it's just created or inactive no need to build ruls. */
		if(!hCompositor->ahWindow[i] ||
		   BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
		   BVDC_P_STATE_IS_INACTIVE(hCompositor->ahWindow[i]))
		{
			continue;
		}

		BDBG_OBJECT_ASSERT(hCompositor->ahWindow[i], BVDC_WIN);
		/* we need to keep building the sliped window and the newly
		 * transfered locked window on the consistent vec trigger; */
		if(!hCompositor->ahWindow[i]->bSyncLockSrc || hCompositor->ulSlip2Lock)
		{
			/* Note: sync-slipped window reader shutdown needs to build both slots to
			 * make both top/bot slots consistent in case next time the isr is not
			 * serviced on time, the staled playback RUL could unmute when the
			 * capture side is disconnecting vnet;  */
			if(!hCompositor->ahWindow[i]->bSyncLockSrc &&
			   BVDC_P_SRC_IS_VIDEO(hCompositor->ahWindow[i]->stCurInfo.hSource->eId))
			{
				bBuilldBothTopBotSlots |= (BVDC_P_State_eShutDownRul == hCompositor->ahWindow[i]->stCurInfo.eReaderState);
			}

			/* Only need to build the READER part of the slip. */
			BVDC_P_Window_BuildRul_isr(hCompositor->ahWindow[i], pList, eFieldId,
				false, /* writer*/
				true,  /* reader */
				false  /* CanvasCtrl */ );
		}
	}

	/* convas should not be enabled until all surfaces are configured and enabled,
	 * notice that syncLock window RUL is built after BuildSyncSlipRul_isr is called */
	if (bBuildCanvasCtrl)
	{
		BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CANVAS_CTRL, pList->pulCurrent);
	}

	BDBG_LEAVE(BVDC_P_Compositor_BuildSyncSlipRul_isr);
	return bBuilldBothTopBotSlots;
}

/***************************************************************************
 * {private}
 *
 * to be called for syncLock case since syncLock window RUL is built after
 * BuildSyncSlipRul_isr is called
 */
void BVDC_P_Compositor_BuildConvasCtrlRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList )
{
	BDBG_ENTER(BVDC_P_Compositor_BuildConvasCtrlRul_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	BVDC_P_CMP_WRITE_TO_RUL(CMP_0_CANVAS_CTRL, pList->pulCurrent);

	BDBG_LEAVE(BVDC_P_Compositor_BuildConvasCtrlRul_isr);
}

/***************************************************************************
 * {private}
 *
 */
#if !BVDC_P_ORTHOGONAL_VEC
BERR_Code BVDC_P_Compositor_GetOutputInfo_isr
	( const BVDC_Compositor_Handle     hCompositor,
	  bool                            *pbFullRate )
{
	BDBG_ENTER(BVDC_P_Compositor_GetOutputInfo_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	if(pbFullRate)
	{
		if(hCompositor->hDisplay->stCurInfo.eDropFrame == BVDC_Mode_eOff)
		{
			hCompositor->bFullRate = true;
		}
		else if(hCompositor->hDisplay->stCurInfo.eDropFrame == BVDC_Mode_eOn)
		{
			hCompositor->bFullRate = false;
		}
		*pbFullRate = hCompositor->bFullRate;
	}

	BDBG_LEAVE(BVDC_P_Compositor_GetOutputInfo_isr);
	return BERR_SUCCESS;
}
#endif

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Compositor_AssignTrigger_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BRDC_Trigger                     eTopTrigger,
	  BRDC_Trigger                     eBotTrigger )
{
	BDBG_ENTER(BVDC_P_Compositor_AssignTrigger_isr);
	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	BRDC_Slot_ExecuteOnTrigger_isr(hCompositor->ahSlot[BAVC_Polarity_eTopField],
		eTopTrigger, true);
	BRDC_Slot_ExecuteOnTrigger_isr(hCompositor->ahSlot[BAVC_Polarity_eBotField],
		eBotTrigger, true);

	BDBG_LEAVE(BVDC_P_Compositor_AssignTrigger_isr);
	return BERR_SUCCESS;
}

/**************************************************************************
 * This function will call the READER of all the windows hold by this compositor,
 * and determine the input color space to VEC, also select the video window
 * color space conversion matrix if necessary.
 */
#ifndef BVDC_FOR_BOOTUPDATER
void BVDC_P_Compositor_WindowsReader_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BAVC_Polarity                    eNextFieldId,
	  BVDC_P_ListInfo                 *pList )
{
	int                       i;
	uint32_t                  ulVSize, ulHSize;
	BVDC_Window_Handle        hWindow;
	bool                      bWidthTrim = true;
	BVDC_DisplayTg            eMasterTg;
	bool                      bDTg;
	bool                      bBgCsc = false;
	BFMT_Orientation          eOrientation;
#if (BVDC_P_DCX_3D_WORKAROUND)
	bool                      bEnableDcxm = false;
#endif

#if BVDC_P_ORTHOGONAL_VEC
	BVDC_P_Display_SrcInfo    stSrcInfo;
#endif
	bool bOutputXvYcc =       hCompositor->hDisplay->stCurInfo.bXvYcc;
	const BFMT_VideoInfo     *pFmtInfo;

	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

	if(!hCompositor->bIsBypass)
	{
		BFMT_VideoFmt eDspFmt = hCompositor->stCurInfo.pFmtInfo->eVideoFmt;

		if (VIDEO_FORMAT_IS_UHD(eDspFmt))
		{
			hCompositor->eCmpColorSpace =
				(BAVC_MatrixCoefficients_eItu_R_BT_709 == hCompositor->hDisplay->stCurInfo.eHdmiOutput)?
				BVDC_P_CmpColorSpace_eHdYCrCb : BVDC_P_CmpColorSpace_eUhdYCrCb;
		}
		else if (BVDC_P_IS_CUSTOMFMT(eDspFmt))
		{
			/* May need to select something else in the future if
			 * custom transcoding formats or customer is something else.
			 * The selection is backward compatible. */
			/* customize output color space according to format info:
			 *     - if vsync rate is 25/50Hz, choose HD or PAL SD by format height;
			 *        else, choose HD or NTSC SD by format height; */
			if(hCompositor->stCurInfo.pFmtInfo->ulVertFreq == 25*BFMT_FREQ_FACTOR ||
			   hCompositor->stCurInfo.pFmtInfo->ulVertFreq == 50*BFMT_FREQ_FACTOR)
			{
			    hCompositor->eCmpColorSpace = (hCompositor->stCurInfo.pFmtInfo->ulDigitalHeight > 576) ?
			        BVDC_P_CmpColorSpace_eHdYCrCb : BVDC_P_CmpColorSpace_ePalSdYCrCb;
			}
			else
			{
			    hCompositor->eCmpColorSpace = (hCompositor->stCurInfo.pFmtInfo->ulDigitalHeight > 480) ?
			        BVDC_P_CmpColorSpace_eHdYCrCb : BVDC_P_CmpColorSpace_eNtscSdYCrCb;
			}
			BDBG_MSG(("CMP%u fmt[%ux%u%c%u] csc = %u", hCompositor->eId, hCompositor->stCurInfo.pFmtInfo->ulDigitalWidth,
			    hCompositor->stCurInfo.pFmtInfo->ulDigitalHeight, hCompositor->stCurInfo.pFmtInfo->bInterlaced?'i':'p',
			    hCompositor->stCurInfo.pFmtInfo->ulVertFreq/BFMT_FREQ_FACTOR, hCompositor->eCmpColorSpace));
		}
		else
		{
			BAVC_MatrixCoefficients  eMatrixCoeff;
			eMatrixCoeff = BAVC_GetDefaultMatrixCoefficients_isrsafe(eDspFmt, bOutputXvYcc);
			hCompositor->eCmpColorSpace =
				( (eMatrixCoeff == BAVC_MatrixCoefficients_eXvYCC_601)?         BVDC_P_CmpColorSpace_eXvYcc_601   :
				  (eMatrixCoeff == BAVC_MatrixCoefficients_eSmpte_170M)?        BVDC_P_CmpColorSpace_eNtscSdYCrCb :
				  (eMatrixCoeff == BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG)? BVDC_P_CmpColorSpace_ePalSdYCrCb  :
				  BVDC_P_CmpColorSpace_eHdYCrCb );
		}
	}
	else
	{
		/* set up cmp colorspace for bypass 656.
		   the color space conversion would be done inside 656 encoder. */
		for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
		{
			hWindow = hCompositor->ahWindow[i];
			BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

			if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
			{
				switch(hWindow->hBuffer->pCurReaderBuf->eMatrixCoefficients)
				{
				/* HD */
				case BAVC_MatrixCoefficients_eItu_R_BT_709:
					hCompositor->eCmpColorSpace = BVDC_P_CmpColorSpace_eHdYCrCb;
					break;
				/* 1953 NTSC SD */
				case BAVC_MatrixCoefficients_eFCC:
					hCompositor->eCmpColorSpace = BVDC_P_CmpColorSpace_eFcc;
					break;
				/* PAL SD */
				case BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG:
					hCompositor->eCmpColorSpace = BVDC_P_CmpColorSpace_eNtscSdYCrCb;
					break;
				/* NTSC SD */
				case BAVC_MatrixCoefficients_eSmpte_170M:
					hCompositor->eCmpColorSpace = BVDC_P_CmpColorSpace_ePalSdYCrCb;
					break;
				/* 1987 ATSC HD */
				case BAVC_MatrixCoefficients_eSmpte_240M:
					hCompositor->eCmpColorSpace = BVDC_P_CmpColorSpace_eSmpte_240M;
					break;
				default:
					hCompositor->eCmpColorSpace = BVDC_P_CmpColorSpace_eHdYCrCb;
				}

				break;
			}
			else
			{
				hCompositor->eCmpColorSpace =
					VIDEO_FORMAT_27Mhz(hCompositor->stCurInfo.pFmtInfo->ulPxlFreqMask)
					? BVDC_P_CmpColorSpace_eNtscSdYCrCb : BVDC_P_CmpColorSpace_eHdYCrCb;
			}
		}
	}

	/* second pass: to adjust non-vbi-pass-thru window position;
	   Note: adjustment is done in the second pass in case vwin0 has no pass-thru,
	         but vwin1 has; */
	for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
	{
		/* SKIP: If it's just created or inactive no need to program. */
		if(!hCompositor->ahWindow[i] ||
			BVDC_P_STATE_IS_CREATE(hCompositor->ahWindow[i]) ||
			BVDC_P_STATE_IS_INACTIVE(hCompositor->ahWindow[i]))
		{
			continue;
		}

		hWindow = hCompositor->ahWindow[i];
		BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
		BVDC_P_Window_Reader_isr(hCompositor->ahWindow[i], eNextFieldId, pList);
#if (BVDC_P_DCX_3D_WORKAROUND)
		if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
		{
			if(hWindow->pCurReaderNode)
				bEnableDcxm = hWindow->pCurReaderNode->bEnableDcxm;
		}
#endif

		/* turn off 704-sample feature if any window is larger than 704 width */
		if(BVDC_P_WIN_GET_FIELD_NAME(CMP_0_V0_DISPLAY_SIZE, HSIZE) > 704)
		{
			bWidthTrim = false;
		}
	}

	/* Turn off 704-sample feature if DCS. */
	/* TODO: remove this restriction */
#if DCS_SUPPORT
	if (
		VIDEO_FORMAT_SUPPORTS_DCS (
			hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
	{
		bWidthTrim = false;
	}
#endif

	/* We do not have a 704-sample ARIB array, so this is another case where we
	 * have to remove this feature */
	if (hCompositor->hDisplay->bArib480p)
	{
		bWidthTrim = false;
	}

	/* set up display sourceinfo */
	/* detect SD 704-sample input: black at both edges, all windows are <= 704 width; */
	bWidthTrim = (
		(bWidthTrim) &&
		(hCompositor->stCurInfo.ulBgColorYCrCb == BVDC_P_YCRCB_BLACK) &&
		(VIDEO_FORMAT_IS_SD(hCompositor->stCurInfo.pFmtInfo->eVideoFmt)));

#if BVDC_P_ORTHOGONAL_VEC
	bBgCsc = (hCompositor->hDisplay->stCurInfo.eCmpColorSpace != hCompositor->eCmpColorSpace);
	stSrcInfo.bWidthTrimmed = bWidthTrim;
	stSrcInfo.eCmpColorSpace = hCompositor->eCmpColorSpace;

	if(hCompositor->hDisplay->stCurInfo.eDropFrame == BVDC_Mode_eOff)
	{
		hCompositor->bFullRate = true;
	}
	else if(hCompositor->hDisplay->stCurInfo.eDropFrame == BVDC_Mode_eOn)
	{
		hCompositor->bFullRate = false;
	}

	/* auto track source if enabled or last used */
	stSrcInfo.bFullRate = hCompositor->bFullRate;

	BVDC_P_Display_SetSourceInfo_isr(hCompositor->hDisplay, &stSrcInfo);
#else
	bBgCsc = (hCompositor->hDisplay->stNewInfo.eCmpColorSpace != hCompositor->eCmpColorSpace);
	if (hCompositor->hDisplay->stCurInfo.bWidthTrimmed != bWidthTrim)
	{
		hCompositor->hDisplay->stCurInfo.stDirty.stBits.bWidthTrim = BVDC_P_DIRTY;
	}
	hCompositor->hDisplay->stCurInfo.bWidthTrimmed = bWidthTrim;
	hCompositor->hDisplay->stNewInfo.bWidthTrimmed = bWidthTrim;
	hCompositor->hDisplay->stNewInfo.eCmpColorSpace = hCompositor->eCmpColorSpace;
#endif

	/* Update BG colorspace to match base on src/disp changes */
	if(bBgCsc)
	{
		uint32_t ulBgColorYCrCb;
		unsigned int uiARGB = BPXL_MAKE_PIXEL(BPXL_eA8_R8_G8_B8, 0x00,
			hCompositor->stCurInfo.ucRed,
			hCompositor->stCurInfo.ucGreen,
			hCompositor->stCurInfo.ucBlue);

		if((BVDC_P_CmpColorSpace_eHdYCrCb != hCompositor->eCmpColorSpace) &&
		   (BVDC_P_CmpColorSpace_eSmpte_240M != hCompositor->eCmpColorSpace))
		{
			BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8,
				uiARGB, (unsigned int*)&ulBgColorYCrCb);
		}
		else
		{
			BPXL_ConvertPixel_RGBtoHdYCbCr_isr(
				BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8,
				uiARGB, (unsigned int*)&ulBgColorYCrCb);
		}

		hCompositor->stNewInfo.ulBgColorYCrCb = ulBgColorYCrCb;
		hCompositor->stCurInfo.ulBgColorYCrCb = ulBgColorYCrCb;
		BVDC_P_CMP_SET_REG_DATA(CMP_0_BG_COLOR, ulBgColorYCrCb);
	}

	/* set compositor size -- number of lines. */
	eMasterTg = hCompositor->hDisplay->eMasterTg;
	bDTg      =  BVDC_P_DISPLAY_USED_DIGTRIG(eMasterTg);
	pFmtInfo = hCompositor->stCurInfo.pFmtInfo;

#if BVDC_P_SUPPORT_STG
	pFmtInfo = (BVDC_P_DISPLAY_USED_STG(eMasterTg) &&
		(hCompositor->hDisplay->pStgFmtInfo !=NULL))?
			hCompositor->hDisplay->pStgFmtInfo:hCompositor->stCurInfo.pFmtInfo;

	eOrientation = BVDC_P_DISPLAY_USED_STG(eMasterTg)?
		hCompositor->eDspOrientation: hCompositor->stCurInfo.eOrientation;
#else
	eOrientation = hCompositor->stCurInfo.eOrientation;
#endif

	ulHSize   = bDTg ? pFmtInfo->ulDigitalWidth :pFmtInfo->ulWidth;
	ulVSize   = bDTg ? pFmtInfo->ulDigitalHeight:pFmtInfo->ulHeight;
	ulVSize >>= pFmtInfo->bInterlaced;

	/* Handle 3d case */
	if(!BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt))
	{
		if(eOrientation == BFMT_Orientation_e3D_LeftRight)
		{
			ulHSize = ulHSize / 2;
		}
		else if(eOrientation == BFMT_Orientation_e3D_OverUnder)
		{
			ulVSize = ulVSize / 2;
		}
	}

	BDBG_MODULE_MSG(BVDC_CMP_SIZE,("Canvas[%d] %4d x %4d orientation %d bDtg%u pFmt%p",
	    hCompositor->eId,  ulHSize,ulVSize, eOrientation, bDTg, (void *)pFmtInfo));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CANVAS_SIZE) = (
		BCHP_FIELD_DATA(CMP_0_CANVAS_SIZE, HSIZE, ulHSize) |
		BCHP_FIELD_DATA(CMP_0_CANVAS_SIZE, VSIZE, ulVSize));

#if BVDC_P_SUPPORT_3D_VIDEO
	{
		if(BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt))
			eOrientation = pFmtInfo->eOrientation;

#if (BVDC_P_DCX_3D_WORKAROUND)
		if(bEnableDcxm)
			eOrientation = BFMT_Orientation_e2D;
#endif

		BVDC_P_CMP_GET_REG_DATA(CMP_0_CMP_CTRL) = (
			BCHP_FIELD_DATA(CMP_0_CMP_CTRL, BVB_VIDEO, eOrientation));
	}
#endif

	/* Luma Average for this compositor. */
#if (BVDC_P_SUPPORT_CMP_LUMA_AVG_VER > 0)
	{
		BVDC_P_Rect stRect;

		BVDC_P_CalculateRect_isr(&hCompositor->stCurInfo.stLumaRect.stRegion,
			ulHSize,
			(bDTg? pFmtInfo->ulDigitalHeight:
			pFmtInfo->ulHeight),
			pFmtInfo->bInterlaced,
			&stRect);

	BVDC_P_CMP_GET_REG_DATA(CMP_0_LUMA_AVG_CTRL) = (
		BCHP_FIELD_DATA(CMP_0_LUMA_AVG_CTRL, V0_LUMA_AVG_CTRL,      1) |
		BCHP_FIELD_DATA(CMP_0_LUMA_AVG_CTRL, CMP_OUT_LUMA_AVG_CTRL, 1));

	BVDC_P_CMP_GET_REG_DATA(CMP_0_CMP_LUMA_AVG_WIN_OFFSET) = (
		BCHP_FIELD_DATA(CMP_0_CMP_LUMA_AVG_WIN_OFFSET, X_OFFSET, stRect.lLeft) |
		BCHP_FIELD_DATA(CMP_0_CMP_LUMA_AVG_WIN_OFFSET, Y_OFFSET, stRect.lTop));
	BVDC_P_CMP_GET_REG_DATA(CMP_0_CMP_LUMA_AVG_WIN_SIZE) = (
		BCHP_FIELD_DATA(CMP_0_CMP_LUMA_AVG_WIN_SIZE, HSIZE, stRect.ulWidth) |
		BCHP_FIELD_DATA(CMP_0_CMP_LUMA_AVG_WIN_SIZE, VSIZE, stRect.ulHeight));
}
#endif

	BDBG_LEAVE(BVDC_P_Compositor_WindowsReader_isr);
	return;
}

void BVDC_P_Compositor_SetMBoxMetaData_isr
	( const BVDC_P_PictureNode        *pPicture,
	  BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_WindowId                  eId)
{
	BDBG_ENTER(BVDC_P_Compositor_SetMBoxMetaData);
	BDBG_OBJECT_ASSERT(hCompositor,     BVDC_CMP);

	if(NULL!=pPicture)
	{
		hCompositor->ulOrigPTS               = pPicture->ulOrigPTS;

		hCompositor->uiHorizontalPanScan     = pPicture->iHorzPanScan;
		hCompositor->uiVerticalPanScan       = pPicture->iVertPanScan;

		hCompositor->ulDisplayHorizontalSize = pPicture->ulDispHorzSize;
		hCompositor->ulDisplayVerticalSize   = pPicture->ulDispVertSize;

		hCompositor->ePictureType            = pPicture->ePictureType;

		hCompositor->eSourcePolarity         = pPicture->eSrcPolarity;
		hCompositor->bPictureRepeatFlag      = pPicture->stFlags.bPictureRepeatFlag;
		hCompositor->bIgnorePicture          = pPicture->bIgnorePicture;
		hCompositor->bStallStc               = pPicture->bStallStc;
		hCompositor->bLast                   = pPicture->bLast;
		hCompositor->bChannelChange          = pPicture->bChannelChange;
		hCompositor->bMute                   = pPicture->bMute;
		hCompositor->ulStgPxlAspRatio_x_y    = pPicture->ulStgPxlAspRatio_x_y;
		hCompositor->ulDecodePictureId       = pPicture->ulDecodePictureId;
		hCompositor->bValidAfd               = pPicture->bValidAfd;
		hCompositor->ulAfd                   = pPicture->ulAfd;
		hCompositor->eBarDataType            = pPicture->eBarDataType;
		hCompositor->ulTopLeftBarValue       = pPicture->ulTopLeftBarValue;
		hCompositor->ulBotRightBarValue      = pPicture->ulBotRightBarValue;
		hCompositor->bPreChargePicture       = pPicture->bPreChargePicture;
		hCompositor->bEndofChunk             = pPicture->bEndofChunk;
		hCompositor->ulChunkId               = pPicture->ulChunkId;
		hCompositor->eDspOrientation         = pPicture->eDispOrientation;
		if(BVDC_P_CMP_GET_V0ID(hCompositor)==eId)
		{
#if (BVDC_P_ORTHOGONAL_VEC)
			bool bPrevPassThrough = (hCompositor->eTransferCharacteristics >=BAVC_TransferCharacteristics_eSmpte_ST_2084);
			bool bPassThrough     = (pPicture->eTransferCharacteristics >= BAVC_TransferCharacteristics_eSmpte_ST_2084);

			hCompositor->hDisplay->stNewInfo.stDirty.stBits.bHdmiCsc =
				(bPrevPassThrough!= bPassThrough)?BVDC_P_DIRTY: BVDC_P_CLEAN;
#endif
			hCompositor->eTransferCharacteristics = pPicture->eTransferCharacteristics;

		}

		/*stg path is controlled in bvdc_displaystg_priv.c */
		if(!BVDC_P_DISPLAY_USED_STG(hCompositor->hDisplay->eMasterTg))
		{
			hCompositor->ulPicId += (!hCompositor->bIgnorePicture);
		}
	}

	BDBG_LEAVE(BVDC_P_Compositor_SetMBoxMetaData);
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/* End of file. */
