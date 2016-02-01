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
 *     Private module for setting up the modular Vec
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "bfmt.h"

/* Note: Tricky here!  bavc.h needs bchp_rdc.h defininitions */
#include "bchp_rdc.h"

#include "bavc.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_common_priv.h"

#include "bchp_misc.h"
#include "bchp_video_enc.h"
#include "bchp_vbi_enc_prim.h"
#include "bchp_video_enc_decim.h"
#include "bchp_timer.h"

#include "bchp_sec_it.h"

/* Reset VEC */
#include "bchp_sun_top_ctrl.h"

#if BVDC_P_SUPPORT_ITU656_OUT
#include "bchp_csc.h"
#include "bchp_dtg.h"
#include "bchp_dvf.h"
#include "bchp_itu656.h"
#endif

#if BVDC_P_SUPPORT_DVI_OUT
#include "bchp_dvi_dtg.h"
#include "bchp_dvi_csc.h"
#include "bchp_dvi_dvf.h"
#endif

#if (BVDC_P_SUPPORT_ITU656_OUT || BVDC_P_SUPPORT_DVI_OUT)
#include "bchp_dtram.h"
#endif

#if (BVDC_P_SUPPORT_VBI_ENC_656)
	#if (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)
		#include "bchp_vbi_enc_656.h"
	#else
		#include "bchp_vbi_enc_656_ancil.h"
	#endif
#endif

#if BVDC_P_SUPPORT_SEC_VEC
#include "bchp_sec_csc_co.h"
#include "bchp_sec_src_co.h"
#include "bchp_sec_vf_co.h"
#include "bchp_vbi_enc_sec.h"
#endif

#if BVDC_P_SUPPORT_TER_VEC
#include "bchp_vbi_enc_tert.h"
#endif

#if BVDC_P_SUPPORT_PRM_VEC_CMPN_ONLY
#include "bchp_prim_src_co.h"
#include "bchp_prim_csc_co.h"
#include "bchp_prim_vf_co.h"
#endif

#if BVDC_P_SUPPORT_DVI_OUT
#if BVDC_P_SUPPORT_HDMI_OUT /* 7038/740x etc */
#include "bchp_hdmi_rm.h"
#elif BVDC_P_SUPPORT_DVPO /* 3563 */
#include "bchp_dvpo_rm_0.h"
#include "bchp_dvpo_0.h"
#include "bchp_lvds_phy_0.h"
#else /* 3560 */
#include "bchp_sparrow_rm.h"
#endif
#endif

#if BVDC_P_SUPPORT_DVI_65NM
#include "bchp_hdmi_tx_phy.h"
#endif

/* registers for component-only output path.
 * note: now assume only one of prm and sec support component-only output
 *       if this is not true, maybe we should use something like
 *       BVDC_P_VEC_VF_CO_OFFSET and BVDC_P_VEC_CSC_CO_OFFSET VF and VF_CO
 *       are identical duplicate of HW .
 */
#if BVDC_P_SUPPORT_PRM_VEC_CMPN_ONLY
#define BVDC_P_VEC_VF_MISC                 BCHP_PRIM_VF_MISC
#define BVDC_P_VEC_VF_CO_MISC              BCHP_PRIM_VF_CO_MISC
#define BVDC_P_VEC_VF_CO_FORMAT_ADDER      BCHP_PRIM_VF_CO_FORMAT_ADDER
#define BVDC_P_VEC_VF_CO_VF_RESET          BCHP_PRIM_VF_CO_VF_RESET
#define BVDC_P_VEC_VF_CO_NEG_SYNC_VALUES   BCHP_PRIM_VF_CO_NEG_SYNC_VALUES
#define BVDC_P_VEC_VF_CO_POS_SYNC_VALUES   BCHP_PRIM_VF_CO_POS_SYNC_VALUES
#define BVDC_P_VEC_VF_CO_FORMAT_ADDER      BCHP_PRIM_VF_CO_FORMAT_ADDER
#define BVDC_P_VEC_CSC_CO_CSC_MODE         BCHP_PRIM_CSC_CO_CSC_MODE
#define BVDC_P_VEC_CSC_CO_CSC_RESET        BCHP_PRIM_CSC_CO_CSC_RESET
#define BVDC_P_VEC_SRC_CO_SRC_CONTROL      BCHP_PRIM_SRC_CO_SRC_CONTROL
#define BVDC_P_VEC_PATH_SUPPORT_CO(path)   (path == BVDC_P_eVecPrimary)
#elif BVDC_P_SUPPORT_SEC_VEC_CMPN_ONLY
#define BVDC_P_VEC_VF_MISC                 BCHP_SEC_VF_MISC
#define BVDC_P_VEC_VF_CO_MISC              BCHP_SEC_VF_CO_MISC
#define BVDC_P_VEC_VF_CO_FORMAT_ADDER      BCHP_SEC_VF_CO_FORMAT_ADDER
#define BVDC_P_VEC_VF_CO_VF_RESET          BCHP_SEC_VF_CO_VF_RESET
#define BVDC_P_VEC_VF_CO_NEG_SYNC_VALUES   BCHP_SEC_VF_CO_NEG_SYNC_VALUES
#define BVDC_P_VEC_VF_CO_POS_SYNC_VALUES   BCHP_SEC_VF_CO_POS_SYNC_VALUES
#define BVDC_P_VEC_VF_CO_FORMAT_ADDER      BCHP_SEC_VF_CO_FORMAT_ADDER
#define BVDC_P_VEC_CSC_CO_CSC_MODE         BCHP_SEC_CSC_CO_CSC_MODE
#define BVDC_P_VEC_CSC_CO_CSC_RESET        BCHP_SEC_CSC_CO_CSC_RESET
#define BVDC_P_VEC_SRC_CO_SRC_CONTROL      BCHP_SEC_SRC_CO_SRC_CONTROL
#define BVDC_P_VEC_PATH_SUPPORT_CO(path)   (path == BVDC_P_eVecSecondary)
#else
#define BVDC_P_VEC_PATH_SUPPORT_CO(path)   (false)
#endif

#define BVDC_P_VF_THRESH                 1  /* HW reset value */
#define BVDC_P_VF_ENABLE                 1  /* HW reset value */

/***************** RM clock adjustment macroes *************/
#define BVDC_P_DISPLAY_RM_PHASE_INC_REG_OFFSET \
	((BCHP_PRIM_RM_PHASE_INC - BCHP_PRIM_RM_RATE_RATIO) / sizeof(uint32_t))

/* Dither settings for DISP */
#define BVDC_P_DITHER_DISP_CSC_LFSR_VALUE            (0xFFC01)
#define BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T0          (0x5)
#define BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T1          (0x3)
#define BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T2          (0x5)
/* Dither settings for DVI, 656 */
#define BVDC_P_DITHER_DISP_DVI_LFSR_VALUE            (0xEF83D)
#define BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T0          (0x0)
#define BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T1          (0x1)
#define BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T2          (0x6)

#define BVDC_P_DITHER_DISP_DVI_SCALE_10BIT           (0x1)
#define BVDC_P_DITHER_DISP_DVI_OFFSET_10BIT          (0x0)
#define BVDC_P_DITHER_DISP_DVI_SCALE_8BIT            (0x4)
#define BVDC_P_DITHER_DISP_DVI_OFFSET_8BIT           (0x1D)

/* Special settings for 656 dither */
#define BVDC_P_DITHER_DISP_CSC_SCALE_CH0             (0x1)
#define BVDC_P_DITHER_DISP_CSC_SCALE_CH1             (0x5)
#define BVDC_P_DITHER_DISP_CSC_SCALE_CH2             (0x5)

/* VCXO_RM in the following chipsets need to be reserved since vec core
   reset would reset VCXO_RM also! */
#ifdef BCHP_VCXO_0_RM_REG_START
#include "bchp_vcxo_0_rm.h"
#define BVDC_P_VCXO_RM_REG_COUNT \
	((BCHP_VCXO_0_RM_REG_END - BCHP_VCXO_0_RM_REG_START) / sizeof(uint32_t) + 1)
#endif
#ifdef BCHP_VCXO_1_RM_REG_START
#include "bchp_vcxo_1_rm.h"
#endif
#ifdef BCHP_VCXO_2_RM_REG_START
#include "bchp_vcxo_2_rm.h"
#endif

BDBG_MODULE(BVDC_DISP);
BDBG_OBJECT_ID(BVDC_DSP);

/* Forward declarations */
static const BVDC_P_RateInfo* BVDC_P_HdmiRmTableEx_isr
(
	const BFMT_VideoInfo                *pFmtInfo,
	BAVC_HDMI_PixelRepetition eHdmiPixelRepetition,
	const BAVC_VdcDisplay_Info          *pRateInfo
);


/*************************************************************************
 * BVDC_P_Display_Create
 *
 *************************************************************************/
BERR_Code BVDC_P_Display_Create
(
	BVDC_P_Context                  *pVdc,
	BVDC_Display_Handle             *phDisplay,
	BVDC_DisplayId                   eId
)
{
	BVDC_P_DisplayContext *pDisplay;

	BDBG_ENTER(BVDC_P_Display_Create);

	/* (1) Allocate display context */
	pDisplay = (BVDC_P_DisplayContext*)
		(BKNI_Malloc(sizeof(BVDC_P_DisplayContext)));
	if(!pDisplay)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Initialize to zero */
	BKNI_Memset((void*)pDisplay, 0x0, sizeof(BVDC_P_DisplayContext));
	BDBG_OBJECT_SET(pDisplay, BVDC_DSP);

	/* Initialize non-changing states.  These are not to be changed by runtime. */
	pDisplay->ulRdcVarAddr      = BRDC_AllocScratchReg(pVdc->hRdc);
	if(pDisplay->ulRdcVarAddr == 0)
	{
		BDBG_ERR(("Not enough scratch registers for display	format switch!"));
		BKNI_Free((void*)pDisplay);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	pDisplay->ulScratchTsAddr = BRDC_AllocScratchReg(pVdc->hRdc);
	if(pDisplay->ulScratchTsAddr == 0)
	{
		BDBG_ERR(("Not enough scratch registers for display Timestamp!"));
		BRDC_FreeScratchReg(pDisplay->hVdc->hRdc, pDisplay->ulRdcVarAddr);
		BKNI_Free((void*)pDisplay);
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	pDisplay->eId               = eId;
	pDisplay->hVdc              = (BVDC_Handle)pVdc;
	pDisplay->hTimer          = pVdc->hTimer;
	BTMR_GetTimerRegisters(pDisplay->hTimer, &pDisplay->stTimerReg);
	pDisplay->bIsBypass         = pVdc->pFeatures->bCmpBIsBypass &&
		(BVDC_DisplayId_eDisplay2 == eId);
	pDisplay->aulMpaaDeciIfPortMask[BVDC_MpaaDeciIf_eHdmi]      = BVDC_Hdmi_0;
	pDisplay->aulMpaaDeciIfPortMask[BVDC_MpaaDeciIf_eComponent] = BVDC_Cmpnt_0;

	/* CompB => Tertiary/Bypass, Comp0 => Secondary, Comp1 => Primary */
	if(pDisplay->eId == BVDC_DisplayId_eDisplay2)
	{
#if BVDC_P_SUPPORT_TER_VEC
		pDisplay->eVecPath    = BVDC_P_eVecTertiary;
		pDisplay->eVbiPath    = BAVC_VbiPath_eVec2;
		pDisplay->ulScratchVbiEncControl = BAVC_VBI_ENC_2_CTRL_SCRATCH;
		pDisplay->ulVbiEncOffset =
			(int32_t)(BCHP_VBI_ENC_TERT_Control - BCHP_VBI_ENC_PRIM_Control);
		pDisplay->lItOffset  =
			(int32_t)(BCHP_TERT_IT_IT_REV_ID    - BCHP_PRIM_IT_IT_REV_ID);
		pDisplay->lVfOffset  =
			(int32_t)(BCHP_TERT_VF_VF_REV_ID    - BCHP_PRIM_VF_VF_REV_ID);
		pDisplay->lSmOffset  =
			(int32_t)(BCHP_TERT_SM_SM_REV_ID    - BCHP_PRIM_SM_SM_REV_ID);
		pDisplay->lCscOffset =
			(int32_t)(BCHP_TERT_CSC_CSC_REV_ID  - BCHP_PRIM_CSC_CSC_REV_ID);
		pDisplay->lSrcOffset =
			(int32_t)(BCHP_TERT_SRC_SRC_REV_ID  - BCHP_PRIM_SRC_SRC_REV_ID);
		pDisplay->lRmOffset  =
			(int32_t)(BCHP_TERT_RM_CONTROL      - BCHP_PRIM_RM_CONTROL);
/* TODO: replace the following #elif statement with something more graceful. */
#elif (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)
		pDisplay->eVecPath    = BVDC_P_eVecBypass0;
		pDisplay->eVbiPath    = BAVC_VbiPath_eBypass0;
		pDisplay->ulScratchVbiEncControl = BAVC_VBI_ENC_BP_CTRL_SCRATCH;
		pDisplay->ulVbiEncOffset =
			(int32_t)(BCHP_VBI_ENC_656_Ancil_Control -
				BCHP_VBI_ENC_PRIM_Control);
#endif
	}

#if (BVDC_P_SUPPORT_SEC_VEC)
	else if(pDisplay->eId == BVDC_DisplayId_eDisplay1)
	{
		pDisplay->eVecPath = BVDC_P_eVecSecondary;
		/* BCHP_SEC_* is lower address */
		pDisplay->eVbiPath    = BAVC_VbiPath_eVec1;

		pDisplay->lItOffset  =
			(int32_t)(BCHP_SEC_IT_IT_REV_ID    - BCHP_PRIM_IT_IT_REV_ID);
		pDisplay->lVfOffset  =
			(int32_t)(BCHP_SEC_VF_VF_REV_ID    - BCHP_PRIM_VF_VF_REV_ID);
		pDisplay->lSmOffset  =
			(int32_t)(BCHP_SEC_SM_SM_REV_ID    - BCHP_PRIM_SM_SM_REV_ID);
		pDisplay->lCscOffset =
			(int32_t)(BCHP_SEC_CSC_CSC_REV_ID  - BCHP_PRIM_CSC_CSC_REV_ID);
		pDisplay->lSrcOffset =
			(int32_t)(BCHP_SEC_SRC_SRC_REV_ID  - BCHP_PRIM_SRC_SRC_REV_ID);
		pDisplay->lRmOffset  =
			(int32_t)(BCHP_SEC_RM_CONTROL      - BCHP_PRIM_RM_CONTROL);

		/* BCHP_VBI_ENC_PRIM_Control is lower address */
		pDisplay->ulScratchVbiEncControl = BAVC_VBI_ENC_1_CTRL_SCRATCH;
		pDisplay->ulVbiEncOffset =
			(int32_t)(BCHP_VBI_ENC_SEC_Control - BCHP_VBI_ENC_PRIM_Control);
	}
#endif
	else if(pDisplay->eId == BVDC_DisplayId_eDisplay0)
	{
		pDisplay->eVecPath    = BVDC_P_eVecPrimary;
		pDisplay->eVbiPath    = BAVC_VbiPath_eVec0;
		pDisplay->lItOffset  = 0;
		pDisplay->lVfOffset  = 0;
		pDisplay->lSmOffset  = 0;
		pDisplay->lCscOffset = 0;
		pDisplay->lSrcOffset = 0;
		pDisplay->lRmOffset  = 0;

		pDisplay->ulScratchVbiEncControl = BAVC_VBI_ENC_0_CTRL_SCRATCH;
		pDisplay->ulVbiEncOffset = 0;
	}

#if BVDC_P_SUPPORT_VBI_ENC_656
		pDisplay->ulScratchVbiEncControl_656 = BAVC_VBI_ENC_BP_CTRL_SCRATCH;
		pDisplay->ulVbiEncOffset_656 = 0;	/* Only one VBI/656 path */
#endif

	/* (2) create a AppliedDone event. */
	BKNI_CreateEvent(&pDisplay->hAppliedDoneEvent);

	/* (3) Save hDisplay in hVdc */
	pVdc->ahDisplay[pDisplay->eId] = (BVDC_Display_Handle)pDisplay;

	/* TODO: assign later
	pCompositor->hDisplay = (BVDC_Display_Handle)pDisplay;*/
	*phDisplay = (BVDC_Display_Handle)pDisplay;

	BDBG_LEAVE(BVDC_P_Display_Create);
	return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Display_Init
	( BVDC_Display_Handle              hDisplay )
{
	uint32_t i;
	uint32_t ulGammaTbl;
	uint32_t ulColorTempTbl;
	BVDC_P_CscCoeffs stIdentity = BVDC_P_MAKE_DVO_CSC
		(1.0000, 0.0000, 0.0000, 0.0000,
		 0.0000, 1.0000, 0.0000, 0.0000,
		 0.0000, 0.0000, 1.0000, 0.0000);
	uint32_t  ulLfsrCtrlT0, ulLfsrCtrlT1, ulLfsrCtrlT2, ulLfsrValue;

	BDBG_ENTER(BVDC_P_Display_Init);

	BKNI_Memset((void*)&hDisplay->stNewInfo, 0x0, sizeof(BVDC_P_DisplayInfo));
	BKNI_Memset((void*)&hDisplay->stCurInfo, 0x0, sizeof(BVDC_P_DisplayInfo));

	/* set default timebase for most common sand simplest usage:
	 * HD/SD simul displays  -> timebase 0 that tracks the single video input;
	 * Note:
	 *  There is no good default timebase for displays as it depends on the system usages.
	 *  NEXUS/App should configure display timebase according to the system config.
	 * Example system timebase config:
	 *  1) decoder 0 -> main windows of HD/SD simul displays -> timebase 0 that tracks decoder 0's live input PCR;
	 *  2) HDMI input -> PIP windows of HD/SD simul displays -> timebase 1 that tracks HDMI input's HSYNC time reference;
	 *  3) decoder 2 -> transcode window of encoder display 2 -> timebase 2 that is freerun if decoder 2's input comes from file playback. */
	hDisplay->stNewInfo.eTimeBase = BAVC_Timebase_e0;

	/* Default Dacs to unused */
	for(i=0; i < BVDC_P_MAX_DACS; i++)
	{
		hDisplay->stNewInfo.aDacOutput[i] = BVDC_DacOutput_eUnused;
	}

	/* Default CSC */
	hDisplay->stDvoCscMatrix.ulMin       = 0x0400;
	hDisplay->stDvoCscMatrix.ulMax       = 0x03ff;
	hDisplay->stDvoCscMatrix.stCscCoeffs = stIdentity;

	/* Default to off. */
	hDisplay->stNewInfo.bEnableHdmi = false;
	hDisplay->stNewInfo.bEnable656  = false;
	hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi] = 0;
	hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] = 0;

	/* Init format */
	hDisplay->stNewInfo.pFmtInfo   = BFMT_GetVideoFormatInfoPtr(hDisplay->hVdc->stSettings.eVideoFormat);
	hDisplay->stNewInfo.ulVertFreq = hDisplay->stNewInfo.pFmtInfo->ulVertFreq;
	hDisplay->stNewInfo.eAspectRatio = hDisplay->stNewInfo.pFmtInfo->eAspectRatio;
	BVDC_P_CalcuPixelAspectRatio_isr(
		hDisplay->stNewInfo.eAspectRatio,
		hDisplay->stNewInfo.uiSampleAspectRatioX,
		hDisplay->stNewInfo.uiSampleAspectRatioY,
		hDisplay->stNewInfo.pFmtInfo->ulDigitalWidth, hDisplay->stNewInfo.pFmtInfo->ulDigitalHeight,
		&hDisplay->stNewInfo.stAspRatRectClip,
		&hDisplay->ulPxlAspRatio,
		&hDisplay->ulPxlAspRatio_x_y,
		BFMT_Orientation_e2D);
	hDisplay->stNewInfo.stDirty.stBits.bAspRatio = BVDC_P_DIRTY;

	/* Set Hdmi default */
	hDisplay->stNewInfo.eHdmiOutput = BAVC_MatrixCoefficients_eUnknown;

	/* Initialize Dac output color space */
	hDisplay->stNewInfo.eOutputColorSpace = BVDC_P_Output_eUnknown;

	/* Initialize Rfm output */
	hDisplay->stNewInfo.eRfmOutput = BVDC_RfmOutput_eUnused;
	hDisplay->stCurInfo.eRfmOutput = BVDC_RfmOutput_eUnused;

	/* set default display input color space.
	 * NOTE: primary and secondary displays will always take HD color space input
	 * from compositors; while the bypass display input color space could be SD
	 * or HD depends on the VDEC source format. */
	hDisplay->stNewInfo.eCmpColorSpace = BVDC_P_CmpColorSpace_eHdYCrCb;
	/* Rate change callback */
	hDisplay->stNewInfo.pfGenericCallback = NULL;
	hDisplay->stNewInfo.pvGenericParm1    = NULL;
	hDisplay->stNewInfo.iGenericParm2     = 0;

	/* Current display rate info, update at least once at initialization */
	hDisplay->stNewInfo.stRateInfo.ulPixelClkRate    = 0;
	hDisplay->stNewInfo.stRateInfo.ulPixelClockRate  = 0;
	hDisplay->stNewInfo.stRateInfo.ulVertRefreshRate = 0;

	/* White balance */
	BVDC_P_Display_GetMaxTable(&ulGammaTbl, &ulColorTempTbl);
	hDisplay->stNewInfo.bCCEnable          = false;
	hDisplay->stNewInfo.bUserCCTable       = false;
	hDisplay->stNewInfo.ulGammaTableId     = ulGammaTbl;
	hDisplay->stNewInfo.ulColorTempId      = ulColorTempTbl;

	/* Dvo CSC adjustment values */
	hDisplay->stNewInfo.lDvoAttenuationR = BMTH_FIX_SIGNED_ITOFIX(1, BVDC_P_CSC_DVO_CX_I_BITS, BVDC_P_CSC_DVO_CX_F_BITS);
	hDisplay->stNewInfo.lDvoAttenuationG = BMTH_FIX_SIGNED_ITOFIX(1, BVDC_P_CSC_DVO_CX_I_BITS, BVDC_P_CSC_DVO_CX_F_BITS);
	hDisplay->stNewInfo.lDvoAttenuationB = BMTH_FIX_SIGNED_ITOFIX(1, BVDC_P_CSC_DVO_CX_I_BITS, BVDC_P_CSC_DVO_CX_F_BITS);
	hDisplay->stNewInfo.lDvoOffsetR      = 0;
	hDisplay->stNewInfo.lDvoOffsetG      = 0;
	hDisplay->stNewInfo.lDvoOffsetB      = 0;

	/* Rate manager info */
	hDisplay->stNewInfo.pulAnalogRateTable = NULL;
	/* hDisplay->stNewInfo.aAnalogRateInfo = {0, 0}; */

	/* VF filters */
	BKNI_Memset(hDisplay->stNewInfo.abUserVfFilterCo, false, sizeof(hDisplay->stNewInfo.abUserVfFilterCo));
	BKNI_Memset(hDisplay->stNewInfo.abUserVfFilterCvbs, false, sizeof(hDisplay->stNewInfo.abUserVfFilterCvbs));
	BKNI_Memset(hDisplay->stNewInfo.aaulUserVfFilterCo, 0, sizeof(hDisplay->stNewInfo.aaulUserVfFilterCo));
	BKNI_Memset(hDisplay->stNewInfo.aaulUserVfFilterCvbs, 0, sizeof(hDisplay->stNewInfo.aaulUserVfFilterCvbs));
	BKNI_Memset(hDisplay->apVfFilterCo, 0, sizeof(hDisplay->apVfFilterCo));
	BKNI_Memset(hDisplay->apVfFilterCvbs, 0, sizeof(hDisplay->apVfFilterCvbs));

	hDisplay->stNewInfo.pHdmiRateInfo = NULL;

	/* Dvo/Lvds spread spectrum */
	hDisplay->stNewInfo.stDvoCfg.stSpreadSpectrum.bEnable = false;
	hDisplay->stNewInfo.stDvoCfg.stSpreadSpectrum.ulFrequency = 23077;
	hDisplay->stNewInfo.stDvoCfg.stSpreadSpectrum.ulDelta = 10;

	/* misc/xvYcc parameters */
	hDisplay->stNewInfo.bXvYcc    = false;

	/* Other changable during runtime. */
	hDisplay->bUserAppliedChanges = true;

	/* Vec is not alive yet */
	hDisplay->eItState = BVDC_P_ItState_eNotActive;
	hDisplay->eState   = BVDC_P_State_eInactive;

	/* Vec Phase not being adjusted at startup */
	hDisplay->bVecPhaseInAdjust = false;
	hDisplay->bSetEventPending  = false;
	hDisplay->bRateManagerUpdatedPending = false;
	hDisplay->bRateManagerUpdated = false;

	/* Game mode off */
	hDisplay->hWinGameMode = NULL;
	hDisplay->pDvoRmInfo   = NULL;
	hDisplay->pRmTable     = NULL;
	hDisplay->bRmAdjusted = false;

	/* alignment off */
	hDisplay->ulCurrentTs     = 0;
	hDisplay->eNextPolarity   = BAVC_Polarity_eTopField;
	hDisplay->bAlignAdjusting = false;
	hDisplay->ulTsSampleCount = 0;
	hDisplay->ulTsSamplePeriod = 0;


	/* Dither init */
	ulLfsrCtrlT0 = BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T0;
	ulLfsrCtrlT1 = BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T1;
	ulLfsrCtrlT2 = BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T2;
	ulLfsrValue  = BVDC_P_DITHER_DISP_CSC_LFSR_VALUE;

	/* PRIM_CSC_DITHER */
	BVDC_P_Dither_Init(&hDisplay->stCscDither,
		ulLfsrCtrlT0, ulLfsrCtrlT1, ulLfsrCtrlT2, ulLfsrValue);

	/* DVI_CSC_DITHER */
	ulLfsrCtrlT0 = BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T0;
	ulLfsrCtrlT1 = BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T1;
	ulLfsrCtrlT2 = BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T2;
	ulLfsrValue  = BVDC_P_DITHER_DISP_DVI_LFSR_VALUE;
	BVDC_P_Dither_Init(&hDisplay->stDviDither,
		ulLfsrCtrlT0, ulLfsrCtrlT1, ulLfsrCtrlT2, ulLfsrValue);

	/* CSC_DITHER */
	BVDC_P_Dither_Init(&hDisplay->st656Dither,
		ulLfsrCtrlT0, ulLfsrCtrlT1, ulLfsrCtrlT2, ulLfsrValue);
	/* Special settings for 656. Default is always 1 */
	hDisplay->st656Dither.ulCh0Scale = BVDC_P_DITHER_DISP_CSC_SCALE_CH0;
	hDisplay->st656Dither.ulCh1Scale = BVDC_P_DITHER_DISP_CSC_SCALE_CH1;
	hDisplay->st656Dither.ulCh2Scale = BVDC_P_DITHER_DISP_CSC_SCALE_CH2;

	/* Reset done events */
	BKNI_ResetEvent(hDisplay->hAppliedDoneEvent);

	/* Same as new */
	hDisplay->stCurInfo = hDisplay->stNewInfo;

#if BVDC_P_SUPPORT_656_MASTER_MODE
	/* 656 in master mode */
	if(BVDC_DisplayTg_e656Dtg == hDisplay->eMasterTg)
	{
		/* The enabling 656 output logic requires this flag
		 * has different value in "stNewInfo" and "stCurInfo"
		 * so that 656 master mode can be jump started.
		 */
		hDisplay->stNewInfo.bEnable656 = true;
	}
#endif

#if BVDC_P_SUPPORT_DVO_MASTER_MODE
	/* DVI in master mode */
	if(BVDC_DisplayTg_eDviDtg == hDisplay->eMasterTg)
	{
		/* The enabling DVI output logic requires this flag
		 * has different value in "stNewInfo" and "stCurInfo"
		 * so that DVI master mode can be jump started.
		 */
		hDisplay->stNewInfo.bEnableHdmi = true;
	}
#endif

    hDisplay->ulVsyncCnt = 0;

    if (BVDC_P_DISPLAY_USED_DIGTRIG(hDisplay->eMasterTg))
    {
        if (BVDC_P_DISPLAY_USED_DVI(hDisplay->eMasterTg))
        {
            hDisplay->ulItLctrReg = BCHP_DVI_DTG_DTG_LCNTR;
        }
        else /* 656 out */
        {
            hDisplay->ulItLctrReg = BCHP_DTG_DTG_LCNTR;
        }
    }
    else
    {
        hDisplay->ulItLctrReg = BCHP_PRIM_IT_IT_LCNTR + hDisplay->lItOffset;
    }

	BDBG_LEAVE(BVDC_P_Display_Init);

	return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Display_Destroy
	( BVDC_Display_Handle              hDisplay )
{
	BDBG_ENTER(BVDC_P_Display_Destroy);
	if(!hDisplay)
	{
		BDBG_LEAVE(BVDC_P_Display_Destroy);
		return;
	}

	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	BDBG_OBJECT_ASSERT(hDisplay->hVdc, BVDC_VDC);

	/* At this point application should have disable all the
	 * callbacks &slots */

	/* [3] Remove display handle from main VDC handle */
	hDisplay->hVdc->ahDisplay[hDisplay->eId] = NULL;

	/* [2] Destroy event */
	BKNI_DestroyEvent(hDisplay->hAppliedDoneEvent);

	/* [1] Release RDC scratch register before release context */
	BRDC_FreeScratchReg(hDisplay->hVdc->hRdc, hDisplay->ulScratchTsAddr);
	BRDC_FreeScratchReg(hDisplay->hVdc->hRdc, hDisplay->ulRdcVarAddr);

	BDBG_OBJECT_DESTROY(hDisplay, BVDC_DSP);
	/* [0] Release context in system memory */
	BKNI_Free((void*)hDisplay);

	BDBG_LEAVE(BVDC_P_Display_Destroy);
	return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Display_GetDacSetting
 *
 *  Validate combinations that are supported. RGB, SDYPrPb, HDYPrPb, SVideo
 *  Mark the combinations found, so we don't repeat the search. Pass the
 *  updated array back. Return the output associated with the Dac settings
 *  in peOutput(ex: YQI, RGB, etc) for internal use.
 **************************************************************************/
static void BVDC_P_Display_GetDacSetting
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         uiIndex,
	  BVDC_P_Output                   *peOutput )
{
	BVDC_P_DisplayInfo *pNewInfo;

	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	pNewInfo = &hDisplay->stNewInfo;

	switch(pNewInfo->aDacOutput[uiIndex])
	{
		case BVDC_DacOutput_eComposite:
			*peOutput = BVDC_P_Output_eYQI;
			break;

		case BVDC_DacOutput_eSVideo_Luma:
		case BVDC_DacOutput_eSVideo_Chroma:
			if( BVDC_P_Display_FindDac_isr(
					hDisplay, BVDC_DacOutput_eSVideo_Luma) &&
				 BVDC_P_Display_FindDac_isr(
					hDisplay, BVDC_DacOutput_eSVideo_Chroma))
			{
				*peOutput = BVDC_P_Output_eYQI;
			}
			break;

		case BVDC_DacOutput_eRed:
		case BVDC_DacOutput_eGreen:
		case BVDC_DacOutput_eGreen_NoSync:
		case BVDC_DacOutput_eBlue:
			if( BVDC_P_Display_FindDac_isr(hDisplay, BVDC_DacOutput_eRed) &&
				(BVDC_P_Display_FindDac_isr(hDisplay, BVDC_DacOutput_eGreen) ||
				 BVDC_P_Display_FindDac_isr(
					hDisplay, BVDC_DacOutput_eGreen_NoSync)) &&
				 BVDC_P_Display_FindDac_isr(hDisplay, BVDC_DacOutput_eBlue) )
			{
				if (VIDEO_FORMAT_27Mhz(pNewInfo->pFmtInfo->ulPxlFreqMask))
				{
					*peOutput = BVDC_P_Output_eSDRGB;
				}
				else
				{
					*peOutput = BVDC_P_Output_eHDRGB;
				}
			}
			break;

		case BVDC_DacOutput_eY:
		case BVDC_DacOutput_ePr:
		case BVDC_DacOutput_ePb:
			if ( BVDC_P_Display_FindDac_isr(hDisplay, BVDC_DacOutput_eY) &&
				 BVDC_P_Display_FindDac_isr(hDisplay, BVDC_DacOutput_ePr) &&
				 BVDC_P_Display_FindDac_isr(hDisplay, BVDC_DacOutput_ePb))
			{
				/* BT.601 (480i,Pal,480p) */
				if (VIDEO_FORMAT_27Mhz(pNewInfo->pFmtInfo->ulPxlFreqMask))
				{
					*peOutput = BVDC_P_Output_eSDYPrPb;
				}
				else
				{	/* BT.709 (1080i,720p) */
					*peOutput = BVDC_P_Output_eHDYPrPb;
				}
			}
			break;

		case BVDC_DacOutput_eHsync:
			*peOutput = BVDC_P_Output_eHsync;
			break;

		default:
			*peOutput = BVDC_P_Output_eUnknown;
			break;
	};

	return;
}

/*************************************************************************
 *
 */
static void BVDC_P_Vec_Init_Misc_isr
	( BVDC_P_Context           *pVdc )
{
	uint32_t ulReg;

#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_0)
	uint32_t ulQdacAdj = pVdc->stSettings.aulDacBandGapAdjust[0];
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_2)
	/* TODO: Need to confirm if TDAC = DAC[0-2] and QDAC = DAC[3-6] or
	   TDAC = DAC[4-6] and QDAC = DAC[0-3] */
	uint32_t ulTdacAdj = pVdc->stSettings.aulDacBandGapAdjust[0];
	uint32_t ulQdacAdj = pVdc->stSettings.aulDacBandGapAdjust[3];
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_5)
	uint32_t ulTdacAdj = pVdc->stSettings.aulDacBandGapAdjust[0];
#elif ((BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_1) || \
       (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_3) || \
       (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_4))
	uint32_t ulTdac0Adj = pVdc->stSettings.aulDacBandGapAdjust[0];
	uint32_t ulTdac1Adj = pVdc->stSettings.aulDacBandGapAdjust[3];
#endif

	BDBG_ENTER(BVDC_P_Vec_Init_Misc_isr);

	/* MISC_OUT_CTRL: Default for Vec misc regs */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_OUT_CTRL)] &= ~(
#if (BVDC_P_MAX_DACS > 6)
		BCHP_MASK(MISC_OUT_CTRL, DAC6_SINC) |
#endif
#if (BVDC_P_MAX_DACS > 5)
		BCHP_MASK(MISC_OUT_CTRL, DAC5_SINC) |
#endif
#if (BVDC_P_MAX_DACS > 4)
		BCHP_MASK(MISC_OUT_CTRL, DAC4_SINC) |
#endif
#if (BVDC_P_MAX_DACS > 3)
		BCHP_MASK(MISC_OUT_CTRL, DAC3_SINC) |
#endif
#if (BVDC_P_MAX_DACS > 2)
		BCHP_MASK(MISC_OUT_CTRL, DAC2_SINC) |
#endif
		BCHP_MASK(MISC_OUT_CTRL, DAC1_SINC) |
		BCHP_MASK(MISC_OUT_CTRL, DAC0_SINC));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_OUT_CTRL)] |=  (
#if (BVDC_P_MAX_DACS > 6)
		BCHP_FIELD_ENUM(MISC_OUT_CTRL, DAC6_SINC ,ON) |
#endif
#if (BVDC_P_MAX_DACS > 5)
		BCHP_FIELD_ENUM(MISC_OUT_CTRL, DAC5_SINC ,ON) |
#endif
#if (BVDC_P_MAX_DACS > 4)
		BCHP_FIELD_ENUM(MISC_OUT_CTRL, DAC4_SINC ,ON) |
#endif
#if (BVDC_P_MAX_DACS > 3)
		BCHP_FIELD_ENUM(MISC_OUT_CTRL, DAC3_SINC ,ON) |
#endif
#if (BVDC_P_MAX_DACS > 2)
		BCHP_FIELD_ENUM(MISC_OUT_CTRL, DAC2_SINC ,ON) |
#endif
		BCHP_FIELD_ENUM(MISC_OUT_CTRL, DAC1_SINC ,ON) |
		BCHP_FIELD_ENUM(MISC_OUT_CTRL, DAC0_SINC ,ON));

#if BVDC_P_SUPPORT_RFM_OUTPUT
	/* MISC_OUT_MUX: Default for Vec misc regs */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_OUT_MUX)] &= ~(
		BCHP_MASK(MISC_OUT_MUX, RFM_SEL   ) );

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_OUT_MUX)] |=  (
		BCHP_FIELD_ENUM(MISC_OUT_MUX, RFM_SEL  , PRIM ) );
#endif

#if BCHP_MISC_SYNC_DELAY_REG
	/* MISC_SYNC_DELAY_REG: default for 7400 ext_sync */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_SYNC_DELAY_REG)] &= ~(
		BCHP_MASK(MISC_SYNC_DELAY_REG, EXT_SYNC0 ) |
		BCHP_MASK(MISC_SYNC_DELAY_REG, EXT_SYNC1 ) );

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_SYNC_DELAY_REG)] |=  (
		BCHP_FIELD_ENUM(MISC_SYNC_DELAY_REG, EXT_SYNC0 , SEC ) |
		BCHP_FIELD_ENUM(MISC_SYNC_DELAY_REG, EXT_SYNC1 , PRIM) );

	BREG_Write32(pVdc->hRegister, BCHP_MISC_SYNC_DELAY_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_SYNC_DELAY_REG)]);
#endif

	/* --- Setup Video_Enc --- */
	/* Disable source select */

	ulReg = BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_SRC_SEL, SOURCE, DISABLE);
	BREG_Write32(pVdc->hRegister, BCHP_VIDEO_ENC_DECIM_SRC_SEL, ulReg);

	ulReg = BCHP_FIELD_ENUM(VIDEO_ENC_PRIM_SRC_SEL, SOURCE, DISABLE);
	BREG_Write32(pVdc->hRegister, BCHP_VIDEO_ENC_PRIM_SRC_SEL, ulReg);

#if BVDC_P_SUPPORT_SEC_VEC
	ulReg = BCHP_FIELD_ENUM(VIDEO_ENC_SEC_SRC_SEL, SOURCE, DISABLE);
	BREG_Write32(pVdc->hRegister, BCHP_VIDEO_ENC_SEC_SRC_SEL, ulReg);
#endif

#if BVDC_P_SUPPORT_TER_VEC
	ulReg = BCHP_FIELD_ENUM(VIDEO_ENC_TERT_SRC_SEL, SOURCE, DISABLE);
	BREG_Write32(pVdc->hRegister, BCHP_VIDEO_ENC_TERT_SRC_SEL, ulReg);
#endif

/* T-DAC, and Q-DAC Settings */
#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_1)
	/* MISC_TDAC0_CTRL_REG: Default for Vec misc regs */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC0_CTRL_REG, CLK_EDGE) |
		BCHP_MASK(MISC_TDAC0_CTRL_REG, BG_PTATADJ) |
		BCHP_MASK(MISC_TDAC0_CTRL_REG, BG_CTATADJ));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_CTRL_REG)] |= (
		BCHP_FIELD_ENUM(MISC_TDAC0_CTRL_REG, CLK_EDGE, FALL) |
		BCHP_FIELD_DATA(MISC_TDAC0_CTRL_REG, BG_PTATADJ, ulTdac0Adj) |
		BCHP_FIELD_DATA(MISC_TDAC0_CTRL_REG, BG_CTATADJ, ulTdac0Adj));

	/* MISC_TDAC1_CTRL_REG: Default for Vec misc regs */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_CTRL_REG)]&= ~(
		BCHP_MASK(MISC_TDAC0_CTRL_REG, CLK_EDGE) |
		BCHP_MASK(MISC_TDAC0_CTRL_REG, BG_PTATADJ) |
		BCHP_MASK(MISC_TDAC0_CTRL_REG, BG_CTATADJ));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_CTRL_REG)] |= (
		BCHP_FIELD_ENUM(MISC_TDAC0_CTRL_REG, CLK_EDGE, FALL) |
		BCHP_FIELD_DATA(MISC_TDAC0_CTRL_REG, BG_PTATADJ, ulTdac1Adj) |
		BCHP_FIELD_DATA(MISC_TDAC0_CTRL_REG, BG_CTATADJ, ulTdac1Adj));

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC0_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_CTRL_REG)]);
	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC1_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_CTRL_REG)]);

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_2)
	/* MISC_TDAC_BG_CTRL_REG: power up band gap */
	/* Unused bit14 must be set */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC_BG_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, UNUSED)  |
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, PWRDN)  |
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, CORE_ADJ)  |
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, BANDGAP_BYP)  |
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, IREF_ADJ)  |
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, PWRDN_REFAMP) );

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC_BG_CTRL_REG)] |= (
		BCHP_FIELD_DATA(MISC_TDAC_BG_CTRL_REG, UNUSED, 0) |
		BCHP_FIELD_ENUM(MISC_TDAC_BG_CTRL_REG, PWRDN, PWRDN) |
		BCHP_FIELD_ENUM(MISC_TDAC_BG_CTRL_REG, CORE_ADJ, FOUR) |
		BCHP_FIELD_ENUM(MISC_TDAC_BG_CTRL_REG, BANDGAP_BYP, BANDGAP) |
		BCHP_FIELD_DATA(MISC_TDAC_BG_CTRL_REG, IREF_ADJ,  ulTdacAdj) |
		BCHP_FIELD_ENUM(MISC_TDAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN));

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC_BG_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC_BG_CTRL_REG)]);

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC1_CTRL_REG, PWRUP_BAIS)  |
		BCHP_MASK(MISC_TDAC1_CTRL_REG, PWRDN)  |
		BCHP_MASK(MISC_TDAC1_CTRL_REG, TCOBB)  |
		BCHP_MASK(MISC_TDAC1_CTRL_REG, DCWORD)  |
		BCHP_MASK(MISC_TDAC1_CTRL_REG, DCENBL));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_CTRL_REG)] |= (
		BCHP_FIELD_ENUM(MISC_TDAC1_CTRL_REG, PWRUP_BAIS, PWRDN)  |
		BCHP_FIELD_ENUM(MISC_TDAC1_CTRL_REG, PWRDN, PWRDN)  |
		BCHP_FIELD_ENUM(MISC_TDAC1_CTRL_REG, TCOBB, OFFSET)  |
		BCHP_FIELD_DATA(MISC_TDAC1_CTRL_REG, DCWORD, 0)  |
		BCHP_FIELD_ENUM(MISC_TDAC1_CTRL_REG, DCENBL, NORM));

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC1_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_CTRL_REG)]);

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC2_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_CTRL_REG)];

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC2_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC2_CTRL_REG)]);

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC3_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_CTRL_REG)];

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC3_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC3_CTRL_REG)]);

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_3)
	/* MISC_TDAC[0-1]_BG_CTRL_REG: power up band gap */
	/* Unused bit14 must be set */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_BG_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, UNUSED      ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN       ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, CORE_ADJ    ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, BANDGAP_BYP ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, IREF_ADJ    ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_BG_CTRL_REG)] |= (
		BCHP_FIELD_DATA(MISC_TDAC0_BG_CTRL_REG, UNUSED,       0         ) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN,        PWRDN     ) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, CORE_ADJ,     FOUR      ) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, BANDGAP_BYP,  BANDGAP   ) |
		BCHP_FIELD_DATA(MISC_TDAC0_BG_CTRL_REG, IREF_ADJ,     ulTdac0Adj) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_BG_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, UNUSED      ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN       ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, CORE_ADJ    ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, BANDGAP_BYP ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, IREF_ADJ    ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_BG_CTRL_REG)] |= (
		BCHP_FIELD_DATA(MISC_TDAC1_BG_CTRL_REG, UNUSED,       0         ) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN,        PWRDN     ) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, CORE_ADJ,     FOUR      ) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, BANDGAP_BYP,  BANDGAP   ) |
		BCHP_FIELD_DATA(MISC_TDAC1_BG_CTRL_REG, IREF_ADJ,     ulTdac1Adj) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRUP_BAIS) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRDN     ) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, TCOBB     ) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, DCWORD    ) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, DCENBL    ));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)] |= (
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRUP_BAIS, PWRDN ) |
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRDN,      PWRDN ) |
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, TCOBB,      OFFSET) |
		BCHP_FIELD_DATA(MISC_TDAC01_CTRL_REG, DCWORD,     0     ) |
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, DCENBL,     NORM  ));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC02_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC03_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC11_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC12_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC13_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC0_BG_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_BG_CTRL_REG)]);
	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC1_BG_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_BG_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC01_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC02_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC02_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC03_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC03_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC11_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC11_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC12_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC12_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC13_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC13_CTRL_REG)]);

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_4)
	/* MISC_TDAC[0-1]_BG_CTRL_REG: power up band gap */
	/* Unused bit14 must be set */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_BG_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, UNUSED      ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_CORE  ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, CORE_ADJ    ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, AUX_ADJ     ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, DBG_EN_AUX  ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_AUX   ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, IREF_ADJ    ) |
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_BG_CTRL_REG)] |= (
		BCHP_FIELD_DATA(MISC_TDAC0_BG_CTRL_REG, UNUSED,       0         ) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_CORE,   PWRDN     ) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, CORE_ADJ,     ZERO      ) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, AUX_ADJ,      TWO       ) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, DBG_EN_AUX,   ZERO      ) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_AUX,    POWER_DOWN) |
		BCHP_FIELD_DATA(MISC_TDAC0_BG_CTRL_REG, IREF_ADJ,     ulTdac0Adj) |
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_BG_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, UNUSED      ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_CORE  ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, CORE_ADJ    ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, AUX_ADJ     ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, DBG_EN_AUX  ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_AUX   ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, IREF_ADJ    ) |
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_BG_CTRL_REG)] |= (
		BCHP_FIELD_DATA(MISC_TDAC1_BG_CTRL_REG, UNUSED,       0         ) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_CORE,   PWRDN     ) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, CORE_ADJ,     ZERO      ) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, AUX_ADJ,      TWO       ) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, DBG_EN_AUX,   ZERO      ) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_AUX,    POWER_DOWN) |
		BCHP_FIELD_DATA(MISC_TDAC1_BG_CTRL_REG, IREF_ADJ,     ulTdac1Adj) |
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_TDAC01_CTRL_REG, UNUSED) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRDN_CORE) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, CLK_INV   ) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, TC_OBB    ) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, DC_DATA   ) |
		BCHP_MASK(MISC_TDAC01_CTRL_REG, DCENBL    ));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)] |= (
		BCHP_FIELD_DATA(MISC_TDAC0_BG_CTRL_REG, UNUSED,   1     ) |
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRDN_CORE, PWRDN ) |
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, CLK_INV,    NORM  ) |
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, TC_OBB,     OFFSET) |
		BCHP_FIELD_DATA(MISC_TDAC01_CTRL_REG, DC_DATA,    0     ) |
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, DCENBL,     NORM  ));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC02_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC03_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC11_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC12_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC13_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)];

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC0_BG_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC0_BG_CTRL_REG)]);
	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC1_BG_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC1_BG_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC01_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC01_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC02_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC02_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC03_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC03_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC11_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC11_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC12_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC12_CTRL_REG)]);

	BREG_Write32(pVdc->hRegister, BCHP_MISC_TDAC13_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_TDAC13_CTRL_REG)]);

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_5)
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC1_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_DAC1_CTRL_REG, DCENBL)  |
		BCHP_MASK(MISC_DAC1_CTRL_REG, DC_DATA)  |
		BCHP_MASK(MISC_DAC1_CTRL_REG, TC_OBB)  |
		BCHP_MASK(MISC_DAC1_CTRL_REG, CLK_INV)  |
		BCHP_MASK(MISC_DAC1_CTRL_REG, PWRDN_CORE)  |
		BCHP_MASK(MISC_DAC1_CTRL_REG, PWRDN_BIAS));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC1_CTRL_REG)] |= (
		BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, DCENBL, NORM)  |
		BCHP_FIELD_DATA(MISC_DAC1_CTRL_REG, DC_DATA, 0)  |
		BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, TC_OBB, OFFSET)  |
		BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, CLK_INV, NORM)  |
		BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, PWRDN_CORE, PWRDN)  |
		BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, PWRDN_BIAS, PWRDN));

	BREG_Write32(pVdc->hRegister, BCHP_MISC_DAC1_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC1_CTRL_REG)]);

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC2_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC1_CTRL_REG)];

	BREG_Write32(pVdc->hRegister, BCHP_MISC_DAC2_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC2_CTRL_REG)]);

#if (BVDC_P_MAX_DACS > 2)
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC3_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC1_CTRL_REG)];
	BREG_Write32(pVdc->hRegister, BCHP_MISC_DAC3_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC3_CTRL_REG)]);
#endif

	/* MISC_BG_CTRL_REG: power up band gap */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC_BG_CTRL_REG)] =
		BREG_Read32(pVdc->hRegister, BCHP_MISC_DAC_BG_CTRL_REG) & ~(
		BCHP_MASK(MISC_DAC_BG_CTRL_REG, PWRDN_REFAMP) |
		BCHP_MASK(MISC_DAC_BG_CTRL_REG, IREF_ADJ)  |
		BCHP_MASK(MISC_DAC_BG_CTRL_REG, BANDGAP_BYP)  |
		BCHP_MASK(MISC_DAC_BG_CTRL_REG, CORE_ADJ)  |
		BCHP_MASK(MISC_DAC_BG_CTRL_REG, PWRDN_CORE) );

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC_BG_CTRL_REG)] |= (
		BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN) |
		BCHP_FIELD_DATA(MISC_DAC_BG_CTRL_REG, IREF_ADJ, ulTdacAdj) |
		BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_REG, BANDGAP_BYP, BYP) |
		BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_REG, CORE_ADJ, FOUR) |
		BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_REG, PWRDN_CORE, PWRDN));

	BREG_Write32(pVdc->hRegister, BCHP_MISC_DAC_BG_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_DAC_BG_CTRL_REG)]);
#endif /* BVDC_P_SUPPORT_TDAC_VER_5 */

#if (BVDC_P_SUPPORT_QDAC_VER == BVDC_P_SUPPORT_QDAC_VER_1)
	/* MISC_QDAC_BG_CTRL_REG: power up band gap */
	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC_BG_CTRL_REG)] =
		BREG_Read32(pVdc->hRegister, BCHP_MISC_QDAC_BG_CTRL_REG) & ~(
		BCHP_MASK(MISC_QDAC_BG_CTRL_REG, PWRDN) |
		BCHP_MASK(MISC_QDAC_BG_CTRL_REG, CORE_ADJ)  |
		BCHP_MASK(MISC_QDAC_BG_CTRL_REG, BANDGAP_BYP)  |
		BCHP_MASK(MISC_QDAC_BG_CTRL_REG, IREF_ADJ)  |
		BCHP_MASK(MISC_QDAC_BG_CTRL_REG, PWRDN_REFAMP) );

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC_BG_CTRL_REG)] |= (
		BCHP_FIELD_ENUM(MISC_QDAC_BG_CTRL_REG, PWRDN, PWRUP) |
		BCHP_FIELD_ENUM(MISC_QDAC_BG_CTRL_REG, CORE_ADJ, FOUR) |
		BCHP_FIELD_ENUM(MISC_QDAC_BG_CTRL_REG, BANDGAP_BYP, BANDGAP) |
		BCHP_FIELD_DATA(MISC_QDAC_BG_CTRL_REG, IREF_ADJ, ulQdacAdj ) |
		BCHP_FIELD_ENUM(MISC_QDAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN));

	BREG_Write32(pVdc->hRegister, BCHP_MISC_QDAC_BG_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC_BG_CTRL_REG)]);

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC1_CTRL_REG)] &= ~(
		BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRUP_BAIS)  |
		BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRDN)  |
		BCHP_MASK(MISC_QDAC1_CTRL_REG, CLKINV)  |
		BCHP_MASK(MISC_QDAC1_CTRL_REG, TCOBB)  |
		BCHP_MASK(MISC_QDAC1_CTRL_REG, DCWORD)  |
		BCHP_MASK(MISC_QDAC1_CTRL_REG, DCENBL));

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC1_CTRL_REG)] |= (
		BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRUP_BAIS, PWRDN)  |
		BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRDN, PWRDN)  |
		BCHP_FIELD_DATA(MISC_QDAC1_CTRL_REG, CLKINV, 0)  |
		BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, TCOBB, OFFSET)  |
		BCHP_FIELD_DATA(MISC_QDAC1_CTRL_REG, DCWORD, 0)  |
		BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, DCENBL, NORM));

	BREG_Write32(pVdc->hRegister, BCHP_MISC_QDAC1_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC1_CTRL_REG)]);

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC2_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC1_CTRL_REG)];

	BREG_Write32(pVdc->hRegister, BCHP_MISC_QDAC2_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC2_CTRL_REG)]);

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC3_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC1_CTRL_REG)];

	BREG_Write32(pVdc->hRegister, BCHP_MISC_QDAC3_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC3_CTRL_REG)]);

	pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC4_CTRL_REG)] =
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC1_CTRL_REG)];

	BREG_Write32(pVdc->hRegister, BCHP_MISC_QDAC4_CTRL_REG,
		pVdc->aulMiscRegs[BVDC_P_DISP_GET_REG_IDX(MISC_QDAC4_CTRL_REG)]);
#endif

	BDBG_LEAVE(BVDC_P_Vec_Init_Misc_isr);
	return ;
}

/*************************************************************************
 *
 */
void BVDC_P_Vec_Update_OutMuxes_isr
	( BVDC_Handle           hVdc )
{
	uint32_t i, j;
	BVDC_Display_Handle hDisplay;
	bool bRfmSet = false;

	BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

	/* 1. Clear shared registers; */
#if BVDC_P_SUPPORT_RFM_OUTPUT
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_OUT_MUX) &= ~(
		BCHP_MASK(MISC_OUT_MUX, RFM_SEL ));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_OUT_CTRL) &= ~(
		BCHP_MASK(MISC_OUT_CTRL, RF_CONST));
#endif
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_OUT_MUX) &= ~(
#if (BVDC_P_MAX_DACS > 6)
		BCHP_MASK(MISC_OUT_MUX, DAC6_SEL ) |
#endif
#if (BVDC_P_MAX_DACS > 5)
		BCHP_MASK(MISC_OUT_MUX, DAC5_SEL ) |
#endif
#if (BVDC_P_MAX_DACS > 4)
		BCHP_MASK(MISC_OUT_MUX, DAC4_SEL ) |
#endif
#if (BVDC_P_MAX_DACS > 3)
		BCHP_MASK(MISC_OUT_MUX, DAC3_SEL ) |
#endif
#if (BVDC_P_MAX_DACS > 2)
		BCHP_MASK(MISC_OUT_MUX, DAC2_SEL ) |
#endif
		BCHP_MASK(MISC_OUT_MUX, DAC1_SEL ) |
		BCHP_MASK(MISC_OUT_MUX, DAC0_SEL ) );

	/* PR46546, PR46584: Power down unused DACs, and if all unused do it
	 * for bandgap too. */
#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_5)
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC1_CTRL_REG) &= ~(
		BCHP_MASK(MISC_DAC1_CTRL_REG, PWRDN_BIAS)|
		BCHP_MASK(MISC_DAC1_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC1_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, PWRDN_BIAS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, PWRDN_CORE, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC2_CTRL_REG) &= ~(
		BCHP_MASK(MISC_DAC2_CTRL_REG, PWRDN_BIAS)|
		BCHP_MASK(MISC_DAC2_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC2_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_DAC2_CTRL_REG, PWRDN_BIAS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_DAC2_CTRL_REG, PWRDN_CORE, PWRDN);

#if (BVDC_P_MAX_DACS > 2)
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC3_CTRL_REG) &= ~(
		BCHP_MASK(MISC_DAC3_CTRL_REG, PWRDN_BIAS)|
		BCHP_MASK(MISC_DAC3_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC3_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_DAC3_CTRL_REG, PWRDN_BIAS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_DAC3_CTRL_REG, PWRDN_CORE, PWRDN);
#endif

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC_BG_CTRL_REG) &= ~(
		BCHP_MASK(MISC_DAC_BG_CTRL_REG, PWRDN_CORE)|
		BCHP_MASK(MISC_DAC_BG_CTRL_REG, PWRDN_REFAMP));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC_BG_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_REG, PWRDN_CORE, PWRDN)|
		BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN);

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_4)
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC01_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC01_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRDN_CORE, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC02_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC02_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC02_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC02_CTRL_REG, PWRDN_CORE, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC03_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC03_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC03_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC03_CTRL_REG, PWRDN_CORE, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC0_BG_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_CORE)|
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_AUX)|
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC0_BG_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_CORE, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_AUX, POWER_DOWN)|
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC11_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC11_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC11_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC11_CTRL_REG, PWRDN_CORE, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC12_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC12_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC12_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC12_CTRL_REG, PWRDN_CORE, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC13_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC13_CTRL_REG, PWRDN_CORE));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC13_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC13_CTRL_REG, PWRDN_CORE, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_BG_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_CORE)|
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_AUX)|
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_BG_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_CORE, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_AUX, POWER_DOWN)|
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN);

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_3)
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC01_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC01_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC02_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC02_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC02_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC02_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC02_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC02_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC03_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC03_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC03_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC03_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC03_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC03_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC0_BG_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN)|
		BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC0_BG_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC11_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC11_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC11_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC11_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC11_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC11_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC12_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC12_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC12_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC12_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC12_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC12_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC13_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC13_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC13_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC13_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC13_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC13_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_BG_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN)|
		BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_BG_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN);

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_2)
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC1_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC1_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC1_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC1_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC2_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC2_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC2_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC2_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC2_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC2_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC3_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC3_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_TDAC3_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC3_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC3_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC3_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC_BG_CTRL_REG) &= ~(
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, PWRDN)|
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, PWRDN_REFAMP));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC_BG_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_TDAC_BG_CTRL_REG, PWRDN, PWRDN)|
		BCHP_FIELD_ENUM(MISC_TDAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN);
#endif /* BVDC_P_SUPPORT_TDAC_VER_5 */

#if (BVDC_P_SUPPORT_QDAC_VER == BVDC_P_SUPPORT_QDAC_VER_1)
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC1_CTRL_REG) &= ~(
		BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC1_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC2_CTRL_REG) &= ~(
		BCHP_MASK(MISC_QDAC2_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_QDAC2_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC2_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_QDAC2_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_QDAC2_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC3_CTRL_REG) &= ~(
		BCHP_MASK(MISC_QDAC3_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_QDAC3_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC3_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_QDAC3_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_QDAC3_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC4_CTRL_REG) &= ~(
		BCHP_MASK(MISC_QDAC4_CTRL_REG, PWRUP_BAIS)|
		BCHP_MASK(MISC_QDAC4_CTRL_REG, PWRDN));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC4_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_QDAC4_CTRL_REG, PWRUP_BAIS, PWRDN)|
		BCHP_FIELD_ENUM(MISC_QDAC4_CTRL_REG, PWRDN, PWRDN);

	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC_BG_CTRL_REG) &= ~(
		BCHP_MASK(MISC_QDAC_BG_CTRL_REG, PWRDN)|
		BCHP_MASK(MISC_TDAC_BG_CTRL_REG, PWRDN_REFAMP));
	BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC_BG_CTRL_REG) |=
		BCHP_FIELD_ENUM(MISC_QDAC_BG_CTRL_REG, PWRDN, PWRDN)|
		BCHP_FIELD_ENUM(MISC_QDAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_DOWN);

#endif

	/* 2. Set shared registers; */
	for(j = 0; j < BVDC_P_MAX_COMPOSITOR_COUNT; j++)
	{
		hDisplay = hVdc->ahDisplay[j];
		if(BVDC_P_STATE_IS_ACTIVE(hVdc->ahDisplay[j]) ||
		   BVDC_P_STATE_IS_CREATE(hVdc->ahDisplay[j]) ||
		   BVDC_P_STATE_IS_DESTROY(hVdc->ahDisplay[j]))
		{
			const BVDC_P_DisplayInfo *pNewInfo = &hDisplay->stNewInfo;

#if BVDC_P_SUPPORT_RFM_OUTPUT
			if(!bRfmSet)
			{
				/* MISC_OUT_MUX: RF output */
				if((BVDC_P_eVecPrimary   == pNewInfo->eRfmVecpath) ||
				   (BVDC_P_eVecSecondary == pNewInfo->eRfmVecpath) ||
				   (BVDC_P_eVecTertiary  == pNewInfo->eRfmVecpath))
				{
					/*BDBG_MSG(("RF is %s on Display[%d].",
						(BVDC_RfmOutput_eUnused != pNewInfo->eRfmOutput)
						? "Enable" : "Disable", hDisplay->eId));*/

					if(BVDC_RfmOutput_eCVBS == pNewInfo->eRfmOutput)
					{
	#if BVDC_P_SUPPORT_SEC_VEC
						BVDC_P_DISP_GET_REG_DATA(MISC_OUT_MUX) |=  (
							((BVDC_P_eVecPrimary == pNewInfo->eRfmVecpath) ?
							BCHP_FIELD_ENUM(MISC_OUT_MUX, RFM_SEL, PRIM ):
		#if BVDC_P_SUPPORT_TER_VEC
							((BVDC_P_eVecTertiary == pNewInfo->eRfmVecpath) ?
							BCHP_FIELD_ENUM(MISC_OUT_MUX, RFM_SEL, TERT ):
							BCHP_FIELD_ENUM(MISC_OUT_MUX, RFM_SEL, SEC  ))));
		#else
							BCHP_FIELD_ENUM(MISC_OUT_MUX, RFM_SEL, SEC  )));
		#endif /* BVDC_P_SUPPORT_TER_VEC */
	#else
						BVDC_P_DISP_GET_REG_DATA(MISC_OUT_MUX) |=
							BCHP_FIELD_ENUM(MISC_OUT_MUX, RFM_SEL, PRIM );
	#endif /* BVDC_P_SUPPORT_SEC_VEC */
					}
					else if(BVDC_RfmOutput_eConstant == pNewInfo->eRfmOutput)
					{
						BVDC_P_DISP_GET_REG_DATA(MISC_OUT_CTRL) |= (
							BCHP_FIELD_DATA(MISC_OUT_CTRL, RF_CONST, pNewInfo->ulRfmConst));
					}
				}

				/* only set it once; */
				if(BVDC_RfmOutput_eUnused != pNewInfo->eRfmOutput)
				{
					bRfmSet = true;
				}
			}
#endif /* BVDC_P_SUPPORT_RFM_OUTPUT */

			/* MISC_OUT_MUX: Set the Dac outputs */
			for(i = 0; i < BVDC_P_MAX_DACS; i++)
			{
				if((BVDC_P_eVecPrimary == hDisplay->eVecPath) &&
				   (BVDC_DacOutput_eUnused != pNewInfo->aDacOutput[i]))
				{
					/*BDBG_MSG(("Display[%d] setting DAC[%d] = 0x%08x", hDisplay->eId,
						i, BVDC_P_aaulPrimaryDacTable[pNewInfo->aDacOutput[i]][i])); */
					BVDC_P_DISP_GET_REG_DATA(MISC_OUT_MUX) |=
						BVDC_P_aaulPrimaryDacTable[pNewInfo->aDacOutput[i]][i];
				}
#if BVDC_P_SUPPORT_SEC_VEC
				else if((BVDC_P_eVecSecondary == hDisplay->eVecPath) &&
				        (BVDC_DacOutput_eUnused != pNewInfo->aDacOutput[i]))
				{
					/*BDBG_MSG(("Display[%d] setting DAC[%d] = 0x%08x", hDisplay->eId,
						i, BVDC_P_aaulSecondaryDacTable[pNewInfo->aDacOutput[i]][i])); */
					BVDC_P_DISP_GET_REG_DATA(MISC_OUT_MUX) |=
						BVDC_P_aaulSecondaryDacTable[pNewInfo->aDacOutput[i]][i];
				}
#endif
#if BVDC_P_SUPPORT_TER_VEC
				else if((BVDC_P_eVecTertiary == hDisplay->eVecPath) &&
				        (BVDC_DacOutput_eUnused != pNewInfo->aDacOutput[i]))
				{
					BVDC_P_DISP_GET_REG_DATA(MISC_OUT_MUX) |=
						BVDC_P_aaulTertiaryDacTable[pNewInfo->aDacOutput[i]][i];
				}
#endif
			}

#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_5)
			if(pNewInfo->abEnableDac[0])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC1_CTRL_REG) &= ~(
					BCHP_MASK(MISC_DAC1_CTRL_REG, PWRDN_BIAS)|
					BCHP_MASK(MISC_DAC1_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC1_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, PWRDN_BIAS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_DAC1_CTRL_REG, PWRDN_CORE, PWRUP);
			}
			if(pNewInfo->abEnableDac[1])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC2_CTRL_REG) &= ~(
					BCHP_MASK(MISC_DAC2_CTRL_REG, PWRDN_BIAS)|
					BCHP_MASK(MISC_DAC2_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC2_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_DAC2_CTRL_REG, PWRDN_BIAS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_DAC2_CTRL_REG, PWRDN_CORE, PWRUP);
			}
#if (BVDC_P_MAX_DACS > 2)
			if(pNewInfo->abEnableDac[2])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC3_CTRL_REG) &= ~(
					BCHP_MASK(MISC_DAC3_CTRL_REG, PWRDN_BIAS)|
					BCHP_MASK(MISC_DAC3_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC3_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_DAC3_CTRL_REG, PWRDN_BIAS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_DAC3_CTRL_REG, PWRDN_CORE, PWRUP);
			}
#endif
			/* At least one will turn on bandgap power!*/
			if(   (pNewInfo->abEnableDac[0])
			   || (pNewInfo->abEnableDac[1])
#if (BVDC_P_MAX_DACS > 2)
			   || (pNewInfo->abEnableDac[2])
#endif
			  )
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC_BG_CTRL_REG) &= ~(
					BCHP_MASK(MISC_DAC_BG_CTRL_REG, PWRDN_CORE)|
					BCHP_MASK(MISC_DAC_BG_CTRL_REG, PWRDN_REFAMP));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_DAC_BG_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_REG, PWRDN_CORE, PWRUP)|
					BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_UP);
			}
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_4)
			if(pNewInfo->abEnableDac[0])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC01_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC01_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC01_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC01_CTRL_REG, UNUSED, 1) |
					BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRDN_CORE, PWRUP);
			}
			if(pNewInfo->abEnableDac[1])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC02_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC02_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC02_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC02_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC02_CTRL_REG, UNUSED, 1) |
					BCHP_FIELD_ENUM(MISC_TDAC02_CTRL_REG, PWRDN_CORE, PWRUP);
			}
			if(pNewInfo->abEnableDac[2])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC03_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC03_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC03_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC03_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC03_CTRL_REG, UNUSED, 1) |
					BCHP_FIELD_ENUM(MISC_TDAC03_CTRL_REG, PWRDN_CORE, PWRUP);
			}
			/* At least one will turn on bandgap power!*/
			if(   (pNewInfo->abEnableDac[0])
			   || (pNewInfo->abEnableDac[1])
			   || (pNewInfo->abEnableDac[2])
			  )
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC0_BG_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_CORE)|
					BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_AUX)|
					BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC0_BG_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_CORE, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_AUX, POWER_UP)|
					BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP, POWER_UP);
			}

			if(pNewInfo->abEnableDac[3])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC11_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC11_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC11_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC11_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC11_CTRL_REG, UNUSED, 1) |
					BCHP_FIELD_ENUM(MISC_TDAC11_CTRL_REG, PWRDN_CORE, PWRUP);
			}
			if(pNewInfo->abEnableDac[4])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC12_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC12_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC12_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC12_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC12_CTRL_REG, UNUSED, 1) |
					BCHP_FIELD_ENUM(MISC_TDAC12_CTRL_REG, PWRDN_CORE, PWRUP);
			}
			if(pNewInfo->abEnableDac[5])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC13_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC13_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC13_CTRL_REG, PWRDN_CORE));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC13_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC13_CTRL_REG, UNUSED, 1) |
					BCHP_FIELD_ENUM(MISC_TDAC13_CTRL_REG, PWRDN_CORE, PWRUP);
			}
			/* At least one will turn on bandgap power!*/
			if(   (pNewInfo->abEnableDac[3])
			   || (pNewInfo->abEnableDac[4])
			   || (pNewInfo->abEnableDac[5])
			  )
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_BG_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_CORE)|
					BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_AUX)|
					BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_BG_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_CORE, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_AUX, POWER_UP)|
					BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP, POWER_UP);
			}

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_3)
			if(pNewInfo->abEnableDac[0])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC01_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC01_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC01_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC01_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC01_CTRL_REG, UNUSED, 1)|
					BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC01_CTRL_REG, PWRDN, PWRUP);
			}
			if(pNewInfo->abEnableDac[1])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC02_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC02_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC02_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC02_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC02_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC02_CTRL_REG, UNUSED, 1)|
					BCHP_FIELD_ENUM(MISC_TDAC02_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC02_CTRL_REG, PWRDN, PWRUP);
			}
			if(pNewInfo->abEnableDac[2])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC03_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC03_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC03_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC03_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC03_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC03_CTRL_REG, UNUSED, 1)|
					BCHP_FIELD_ENUM(MISC_TDAC03_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC03_CTRL_REG, PWRDN, PWRUP);
			}
			/* At least one will turn on bandgap power!*/
			if(   (pNewInfo->abEnableDac[0])
			   || (pNewInfo->abEnableDac[1])
			   || (pNewInfo->abEnableDac[2])
			  )
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC0_BG_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN)|
					BCHP_MASK(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC0_BG_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC0_BG_CTRL_REG, PWRDN_REFAMP, POWER_UP);
			}

			if(pNewInfo->abEnableDac[3])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC11_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC11_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC11_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC11_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC11_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC11_CTRL_REG, UNUSED, 1)|
					BCHP_FIELD_ENUM(MISC_TDAC11_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC11_CTRL_REG, PWRDN, PWRUP);
			}
			if(pNewInfo->abEnableDac[4])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC12_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC12_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC12_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC12_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC12_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC12_CTRL_REG, UNUSED, 1)|
					BCHP_FIELD_ENUM(MISC_TDAC12_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC12_CTRL_REG, PWRDN, PWRUP);
			}
			if(pNewInfo->abEnableDac[5])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC13_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC13_CTRL_REG, UNUSED) |
					BCHP_MASK(MISC_TDAC13_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC13_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC13_CTRL_REG) |=
					BCHP_FIELD_DATA(MISC_TDAC13_CTRL_REG, UNUSED, 1)|
					BCHP_FIELD_ENUM(MISC_TDAC13_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC13_CTRL_REG, PWRDN, PWRUP);
			}
			/* At least one will turn on bandgap power!*/
			if(   (pNewInfo->abEnableDac[3])
			   || (pNewInfo->abEnableDac[4])
			   || (pNewInfo->abEnableDac[5])
			  )
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_BG_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN)|
					BCHP_MASK(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_BG_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC1_BG_CTRL_REG, PWRDN_REFAMP, POWER_UP);
			}

#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_2)
			if(pNewInfo->abEnableDac[0])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC1_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC1_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC1_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_TDAC1_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC1_CTRL_REG, PWRDN, PWRUP);
			}
			if(pNewInfo->abEnableDac[1])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC2_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC2_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC2_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC2_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_TDAC2_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC2_CTRL_REG, PWRDN, PWRUP);
			}
			if(pNewInfo->abEnableDac[2])
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC3_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC3_CTRL_REG, PWRUP_BAIS)|
					BCHP_MASK(MISC_TDAC3_CTRL_REG, PWRDN));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC3_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_TDAC3_CTRL_REG, PWRUP_BAIS, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC3_CTRL_REG, PWRDN, PWRUP);
			}
			/* At least one will turn on bandgap power!*/
			if(   (pNewInfo->abEnableDac[0])
			   || (pNewInfo->abEnableDac[1])
			   || (pNewInfo->abEnableDac[2])
			  )
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC_BG_CTRL_REG) &= ~(
					BCHP_MASK(MISC_TDAC_BG_CTRL_REG, PWRDN)|
					BCHP_MASK(MISC_TDAC_BG_CTRL_REG, PWRDN_REFAMP));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_TDAC_BG_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_TDAC_BG_CTRL_REG, PWRDN, PWRUP)|
					BCHP_FIELD_ENUM(MISC_TDAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_UP);
			}
#endif

#if (BVDC_P_SUPPORT_QDAC_VER == BVDC_P_SUPPORT_QDAC_VER_1)
			/* 7400B0 MISC_QDAC[3-6]_CTRL_REG */
			if(pNewInfo->abEnableDac[3])
			{
				BVDC_P_DISP_GET_REG_DATA(MISC_QDAC1_CTRL_REG) &= ~(
					BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRUP_BAIS) |
					BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRDN));

				BVDC_P_DISP_GET_REG_DATA(MISC_QDAC1_CTRL_REG) |= (
					BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRUP_BAIS, PWRUP) |
					BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRDN, PWRUP));
			}
			if(pNewInfo->abEnableDac[4])
			{
				BVDC_P_DISP_GET_REG_DATA(MISC_QDAC2_CTRL_REG) &= ~(
					BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRUP_BAIS) |
					BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRDN));

				BVDC_P_DISP_GET_REG_DATA(MISC_QDAC2_CTRL_REG) |= (
					BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRUP_BAIS, PWRUP) |
					BCHP_FIELD_ENUM(MISC_QDAC2_CTRL_REG, PWRDN, PWRUP));
			}
			if(pNewInfo->abEnableDac[5])
			{
				BVDC_P_DISP_GET_REG_DATA(MISC_QDAC3_CTRL_REG) &= ~(
					BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRUP_BAIS) |
					BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRDN));

				BVDC_P_DISP_GET_REG_DATA(MISC_QDAC3_CTRL_REG) |= (
					BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRUP_BAIS, PWRUP) |
					BCHP_FIELD_ENUM(MISC_QDAC3_CTRL_REG, PWRDN, PWRUP));
			}
			if(pNewInfo->abEnableDac[6])
			{
				BVDC_P_DISP_GET_REG_DATA(MISC_QDAC4_CTRL_REG) &= ~(
					BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRUP_BAIS) |
					BCHP_MASK(MISC_QDAC1_CTRL_REG, PWRDN));

				BVDC_P_DISP_GET_REG_DATA(MISC_QDAC4_CTRL_REG) |= (
					BCHP_FIELD_ENUM(MISC_QDAC1_CTRL_REG, PWRUP_BAIS, PWRUP) |
					BCHP_FIELD_ENUM(MISC_QDAC4_CTRL_REG, PWRDN, PWRUP));
			}

			/* At least one will turn on bandgap power!*/
			if(   (pNewInfo->abEnableDac[3])
			   || (pNewInfo->abEnableDac[4])
			   || (pNewInfo->abEnableDac[5])
			   || (pNewInfo->abEnableDac[6])
			  )
			{
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC_BG_CTRL_REG) &= ~(
					BCHP_MASK(MISC_QDAC_BG_CTRL_REG, PWRDN)|
					BCHP_MASK(MISC_QDAC_BG_CTRL_REG, PWRDN_REFAMP));
				BVDC_P_VDC_GET_MISC_REG_DATA(MISC_QDAC_BG_CTRL_REG) |=
					BCHP_FIELD_ENUM(MISC_QDAC_BG_CTRL_REG, PWRDN, PWRUP)|
					BCHP_FIELD_ENUM(MISC_QDAC_BG_CTRL_REG, PWRDN_REFAMP, POWER_UP);
			}
#endif /* DAC enables */

		}
	}

	BSTD_UNUSED(bRfmSet);

	return ;
}


/*************************************************************************
 *  {secret}
 *	BVDC_P_Display_Validate
 *	Validate settings for Primary or Secondary display
 **************************************************************************/
static BERR_Code BVDC_P_Display_Validate
	( BVDC_Display_Handle           hDisplay )
{
	uint32_t                uiIndex;
	BVDC_P_Output           eOutput = BVDC_P_Output_eUnknown;
	BVDC_P_DisplayInfo      *pNewInfo, *pCurInfo;
	BFMT_VideoFmt           eNewVideoFmt;
	BERR_Code               eErr = BERR_SUCCESS;
	bool                    bYqi=false;
	bool                    bSdRgb=false;
	bool                    bHdRgb=false;
	bool                    bSdYprpb=false;
	bool                    bHdYprpb=false;
	bool                    bHsync=false;

	BDBG_ENTER(BVDC_P_Display_Validate);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	pNewInfo = &hDisplay->stNewInfo;
	pCurInfo = &hDisplay->stCurInfo;

	/* check alignment */
	if(pNewInfo->hTargetDisplay)
	{
		/* Check alignment chain */
		if(pNewInfo->hTargetDisplay->stNewInfo.hTargetDisplay)
		{
			BDBG_ERR(("Display alignment chain is invalid!Display%d->Display%d->Display%d",
				hDisplay->eId, pNewInfo->hTargetDisplay->eId,
				pNewInfo->hTargetDisplay->stNewInfo.hTargetDisplay->eId));
			return BERR_TRACE(BVDC_ERR_INVALID_MODE_PATH);
		}

		/* check timebase */
		if(pNewInfo->eTimeBase != pNewInfo->hTargetDisplay->stNewInfo.eTimeBase)
		{
			BDBG_ERR(("Bad alignment config: Display%d's timebase(%d) != target display%d's timebase(%d)!",
				hDisplay->eId, pNewInfo->eTimeBase,
				pNewInfo->hTargetDisplay->eId, pNewInfo->hTargetDisplay->stNewInfo.eTimeBase));
			return BERR_TRACE(BVDC_ERR_INVALID_MODE_PATH);
		}

		/* check default display frame rate */
		if((hDisplay->hVdc->stSettings.eDisplayFrameRate == BAVC_FrameRateCode_e60) ||
		   (hDisplay->hVdc->stSettings.eDisplayFrameRate == BAVC_FrameRateCode_e30) ||
		   (hDisplay->hVdc->stSettings.eDisplayFrameRate == BAVC_FrameRateCode_e24))
		{
			BDBG_ERR(("Bad VDC default display rate config for VEC locking: %d!",
				hDisplay->hVdc->stSettings.eDisplayFrameRate));
		}
	}

	/* Validate aspect ratio settings */
	if(BVDC_P_IS_UNKNOWN_ASPR(pNewInfo->eAspectRatio, pNewInfo->uiSampleAspectRatioX, pNewInfo->uiSampleAspectRatioY))
	{
		BDBG_ERR(("Invalid aspect ratio settings eAspectRatio: %d, uiSampleAspectRatioX: %d, uiSampleAspectRatioY: %d",
			pNewInfo->eAspectRatio, pNewInfo->uiSampleAspectRatioX, pNewInfo->uiSampleAspectRatioY));
		return BERR_TRACE(BVDC_ERR_INVALID_DISP_ASPECT_RATIO_RECT);
	}

	eNewVideoFmt = pNewInfo->pFmtInfo->eVideoFmt;

	/* Check for valid HDMI Video format */
	if ((pNewInfo->bEnableHdmi) &&
		(!VIDEO_FORMAT_IS_HDMI(eNewVideoFmt)))
	{
		BDBG_ERR(("Invalid HDMI video format."));
		return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
	}

	/* Check for valid 656out support */
	if ((pNewInfo->bEnable656) &&
		(VIDEO_FORMAT_IS_HD(eNewVideoFmt)))
	{
		return BERR_TRACE(BVDC_ERR_INVALID_656OUT_MODE);
	}

	/* Check for valid Dac combination */
	for(uiIndex=0; uiIndex < BVDC_P_MAX_DACS; uiIndex++)
	{
		if (pNewInfo->aDacOutput[uiIndex] != BVDC_DacOutput_eUnused)
		{	/* Search for valid Dac combinations */
			BVDC_P_Display_GetDacSetting((BVDC_Display_Handle)hDisplay, uiIndex, &eOutput);
			switch(eOutput)
			{
			case BVDC_P_Output_eYQI:
				if ( VIDEO_FORMAT_IS_HD(eNewVideoFmt) )
				{
					return BERR_TRACE(BVDC_ERR_VIDEOFMT_OUTPUT_MISMATCH);
				}
				bYqi = true;
				break;

			case BVDC_P_Output_eSDRGB:
				bSdRgb = true;
				break;

			case BVDC_P_Output_eHDRGB:
				bHdRgb = true;
				break;

			case BVDC_P_Output_eSDYPrPb:
				bSdYprpb = true;
				break;

			case BVDC_P_Output_eHDYPrPb:
				bHdYprpb = true;
				break;

			case BVDC_P_Output_eHsync:
				bHsync = true;
				break;

			case BVDC_P_Output_eUnknown:
			default:
				return BERR_TRACE(BVDC_ERR_INVALID_DAC_SETTINGS);
			};
		}
	}

	/* If Rfm is enabled from CVBS output, composite must be enabled at the same time. */
	if(BVDC_RfmOutput_eCVBS == pNewInfo->eRfmOutput)
	{
		if ( VIDEO_FORMAT_IS_HD(eNewVideoFmt) )
		{
			return BERR_TRACE(BVDC_ERR_VIDEOFMT_OUTPUT_MISMATCH);
		}
		bYqi = true;
	}

	/* Invalid combinations */
	if ((bYqi && bHdYprpb) ||
		(bHsync && (bYqi || (! BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath) &&
		(hDisplay->eVecPath == BVDC_P_eVecPrimary && !BVDC_P_SUPPORT_4_DACS_PRIM_VEC)))))
	{
		return BERR_TRACE(BVDC_ERR_INVALID_DAC_SETTINGS);
	}

	/* Reset num dacs used */
	pNewInfo->uiNumDacsOn = 0;

	/* How many Dacs are we using ? */
	for(uiIndex=0; uiIndex < BVDC_P_MAX_DACS; uiIndex++)
	{
		if (pNewInfo->aDacOutput[uiIndex] != BVDC_DacOutput_eUnused)
		{
			pNewInfo->uiNumDacsOn++;
		}
	}

	/* Initialize colorspace */
	pNewInfo->eOutputColorSpace   = BVDC_P_Output_eUnknown;
	pNewInfo->eCoOutputColorSpace = BVDC_P_Output_eUnknown;

	/* Update output color space for internal use */
	if (bYqi)
	{
		if(VIDEO_FORMAT_IS_NTSC_M(eNewVideoFmt))
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eYQI;
		}
		else if(VIDEO_FORMAT_IS_NTSC_J(eNewVideoFmt))
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eYQI_M;
		}
#if BVDC_P_SUPPORT_VEC_SECAM
		else if(VIDEO_FORMAT_IS_SECAM(eNewVideoFmt))
		{
			if ((eNewVideoFmt == BFMT_VideoFmt_eSECAM_L) ||
				(eNewVideoFmt == BFMT_VideoFmt_eSECAM_D) ||
				(eNewVideoFmt == BFMT_VideoFmt_eSECAM_K))
				pNewInfo->eOutputColorSpace = BVDC_P_Output_eYDbDr_LDK;
			else if ((eNewVideoFmt == BFMT_VideoFmt_eSECAM_B) ||
					 (eNewVideoFmt == BFMT_VideoFmt_eSECAM_G))
				pNewInfo->eOutputColorSpace = BVDC_P_Output_eYDbDr_BG;
			else /* BFMT_VideoFmt_eSECAM_H */
				pNewInfo->eOutputColorSpace = BVDC_P_Output_eYDbDr_H;
		}
#endif
		else if(eNewVideoFmt == BFMT_VideoFmt_ePAL_NC)
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eYUV_NC;
		}
		else if(eNewVideoFmt == BFMT_VideoFmt_ePAL_M)
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eYUV_M;
		}
		else if(eNewVideoFmt == BFMT_VideoFmt_ePAL_N)
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eYUV_N;
		}
		else /* (VIDEO_FORMAT_IS_PAL(eNewVideoFmt)) */
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eYUV;
		}
	}

	if (bHsync)
	{
		pNewInfo->eOutputColorSpace = BVDC_P_Output_eHsync;
	}

	if (bSdRgb)
	{
		if (BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
		{
			pNewInfo->eCoOutputColorSpace = BVDC_P_Output_eSDRGB;
		}
		else
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eSDRGB;
		}
	}
	else if (bSdYprpb)
	{
		if (BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
		{
			pNewInfo->eCoOutputColorSpace = BVDC_P_Output_eSDYPrPb;
		}
		else
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eSDYPrPb;
		}
	}
	else if (bHdYprpb)
	{
		if (BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
		{
			pNewInfo->eCoOutputColorSpace = BVDC_P_Output_eHDYPrPb;
		}
		else
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eHDYPrPb;
		}
	}
	else if (bHdRgb)
	{
		if (BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
		{
			pNewInfo->eCoOutputColorSpace = BVDC_P_Output_eHDRGB;
		}
		else
		{
			pNewInfo->eOutputColorSpace = BVDC_P_Output_eHDRGB;
		}
	}

	if( (eErr = BVDC_P_ValidateMacrovision(hDisplay)) != BERR_SUCCESS )
	{
		return BERR_TRACE(eErr);
	}

	/* Do not change DVI-DTG toggle for DVI-DTG toggle for custom formats. */
	if(!BVDC_P_IS_CUSTOMFMT(eNewVideoFmt))
	{
		const uint32_t* toggles = BVDC_P_GetDviDtgToggles_isr (pNewInfo);
		pNewInfo->stDvoCfg.eHsPolarity = toggles[0];
		pNewInfo->stDvoCfg.eVsPolarity = toggles[1];
	}

	/* evaluate bMultiRateAllow flag */
	/* To keep NTSC(cvbs/svideo) as 59.94hz and 480i can be 60.00hz or 59.94hz */
	/* Assumptions multiple of 24/15hz are capable of multirate capable display. */
	pNewInfo->bMultiRateAllow =
		(0 == (pNewInfo->pFmtInfo->ulVertFreq % (24 * BFMT_FREQ_FACTOR))) ||
		(0 == (pNewInfo->pFmtInfo->ulVertFreq % (15 * BFMT_FREQ_FACTOR)));

	pNewInfo->bMultiRateAllow &= !bYqi;

	if(pNewInfo->bMultiRateAllow != pCurInfo->bMultiRateAllow)
	{
		pNewInfo->stDirty.stBits.bRateChange = BVDC_P_DIRTY;
	}

	BDBG_LEAVE(BVDC_P_Display_Validate);
	return BERR_SUCCESS;
}

/*************************************************************************
 *  {secret}
 * BVDC_P_Display_ValidateChanges
 *
 * Validates user's new settings for all displays
 **************************************************************************/
BERR_Code BVDC_P_Display_ValidateChanges
	( BVDC_Display_Handle              ahDisplay[] )
{
	BERR_Code                 eStatus = BERR_SUCCESS;
	BVDC_Display_Handle       hDisplay=NULL;
	BVDC_P_DisplayInfo       *pNewPriInfo=NULL;
	BVDC_P_DisplayInfo       *pNewSecInfo=NULL;
	BVDC_P_DisplayInfo       *pNewTerInfo=NULL;
	BVDC_P_DisplayInfo       *pNewBpsInfo=NULL;
	BVDC_P_DisplayInfo       *pNewInfo=NULL;
	uint32_t                  uiIndex=0;
	uint32_t                  i;
	uint32_t                  uiNumRfmOut=0;
	uint32_t                  uiNumHdmiOut=0;
	uint32_t                  uiNum656Out=0;
	uint32_t                  ulNumDacOut[BVDC_P_MAX_DACS] = {0,};
	BVDC_P_VecPath            e656path = BVDC_P_eVecBypass0;
	BVDC_P_VecPath            eHdmipath = BVDC_P_eVecBypass0;
	BVDC_P_VecPath            eRfmpath = BVDC_P_eVecBypass0;
	uint32_t                  ulRfmConstant = 0;

	BDBG_ENTER(BVDC_P_Display_ValidateChanges);

	for (i=0; i < BVDC_P_MAX_DISPLAY_COUNT; i++)
	{
		/* Only validate the create or active onces. */
		if(BVDC_P_STATE_IS_ACTIVE(ahDisplay[i]) ||
		   BVDC_P_STATE_IS_CREATE(ahDisplay[i]))
		{
			hDisplay = ahDisplay[i];
			pNewInfo = &hDisplay->stNewInfo;

			/* Can't turn on/off sync only if hdmi is disabled. */
			pNewInfo->bSyncOnly &= pNewInfo->bEnableHdmi;

			if (hDisplay->eVecPath == BVDC_P_eVecBypass0)
			{
				pNewBpsInfo = pNewInfo;
			}
			else if (hDisplay->eVecPath == BVDC_P_eVecTertiary)
			{
				pNewTerInfo = pNewInfo;
			}
			else if (hDisplay->eVecPath == BVDC_P_eVecPrimary)
			{
				pNewPriInfo = pNewInfo;
			}
			else
			{
				pNewSecInfo = pNewInfo;
			}

			eStatus = BVDC_P_Display_Validate(hDisplay);
			if (eStatus != BERR_SUCCESS)
			{
				return BERR_TRACE(eStatus);
			}

			/* Look up analog rate manager info */
			if ((hDisplay->eVecPath != BVDC_P_eVecBypass0) ||
			    (pNewInfo->bEnableHdmi))
			{
				const uint32_t          *pTable;
				BAVC_VdcDisplay_Info     lRateInfo;
				eStatus = BVDC_P_GetRmTable_isr (pNewInfo, pNewInfo->pFmtInfo, &pTable, pNewInfo->bFullRate, &lRateInfo);

				if (eStatus != BERR_SUCCESS)
				{
					if (pNewInfo->bEnableHdmi)
					{
						BDBG_ERR(("disp[%d] - Digital pixel frequency for %s not yet supportted.",
							hDisplay->eId, pNewInfo->pFmtInfo->pchFormatStr));
						return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
					}
					else
					{
						BDBG_ERR(("disp[%d] - Analog pixel frequency for %s not yet supportted.",
							hDisplay->eId, pNewInfo->pFmtInfo->pchFormatStr));
						return BERR_TRACE(BVDC_ERR_FORMAT_NOT_SUPPORT_ANALOG_OUTPUT);
					}
				}

				pNewInfo->pulAnalogRateTable = pTable;
				pNewInfo->aAnalogRateInfo = lRateInfo;

				if (pNewInfo->bEnableHdmi)
				{
					const BVDC_P_RateInfo *pHdmiRateInfo;
					pHdmiRateInfo = BVDC_P_HdmiRmTableEx_isr(
						pNewInfo->pFmtInfo, pNewInfo->eHdmiPixelRepetition,
						&lRateInfo);
					if (pHdmiRateInfo == NULL)
					{
						BDBG_ERR(("disp[%d] - Digital pixel frequency for %s not yet supportted.",
							hDisplay->eId, pNewInfo->pFmtInfo->pchFormatStr));
						return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
					}
					pNewInfo->pHdmiRateInfo = pHdmiRateInfo;
				}
			}

			/* Will always need IT table entry */
			{
				const uint32_t *pTable;
				pTable = BVDC_P_GetItTable_isr(pNewInfo);
				if (pTable == NULL)
				{
					return BERR_TRACE (BERR_INVALID_PARAMETER);
				}
			}

			/* count how many displays enable a DAC? */
			for (uiIndex=0; uiIndex < BVDC_P_MAX_DACS; uiIndex++)
			{
				if(pNewInfo->aDacOutput[uiIndex] != BVDC_DacOutput_eUnused)
					ulNumDacOut[uiIndex]++;
			}

			/* 656output only supports NTSC and NTSC_J */
			if (pNewInfo->bEnable656)
			{
				if (!VIDEO_FORMAT_IS_525_LINES(pNewInfo->pFmtInfo->eVideoFmt)
					&&
					!VIDEO_FORMAT_IS_625_LINES(pNewInfo->pFmtInfo->eVideoFmt))
				{
					return BERR_TRACE(BVDC_ERR_INVALID_656OUT_USE);
				}
				e656path = hDisplay->eVecPath;
				uiNum656Out++;
			}

			/* Hdmi output path */
			if (pNewInfo->bEnableHdmi)
			{
				eHdmipath = hDisplay->eVecPath;
				uiNumHdmiOut++;
			}

			/* Rfm output path */
			if(pNewInfo->eRfmOutput != BVDC_RfmOutput_eUnused)
			{
				eRfmpath = hDisplay->eVecPath;
				ulRfmConstant = pNewInfo->ulRfmConst;
				uiNumRfmOut++;
			}
		}
	}

	/* Things to watch for:
	 *   - e656Vecpath
	 *   - eHdmiVecpath
	 *   - eRfmVecpath
	 *   - aDacOutput
	 *   - abEnableDac
	 *
	 * Are shared resources between the display contexts.  After
	 * validation they all should be the same.  Only one context (or its isr)
	 * should be building the RUL for settting these. */

	/* Only one Rfm Output active at a time. */
	if(uiNumRfmOut > 1)
	{
		return BERR_TRACE(BVDC_ERR_RFMOUT_MORE_THAN_ONE);
	}

	/* Only one 656/Hdmi active at a time.
	 * Need to save this since Misc bits are shared! */
	if(uiNum656Out > 1)
	{
		return BERR_TRACE(BVDC_ERR_656OUT_MORE_THAN_ONE);
	}

	if(uiNumHdmiOut > 1)
	{
		return BERR_TRACE(BVDC_ERR_HDMIOUT_MORE_THAN_ONE);
	}

	/* Are there any display conflicts with DAC settings? */
	for (uiIndex=0; uiIndex < BVDC_P_MAX_DACS; uiIndex++)
	{
		/* If multiple displays want to use a Dac, return error */
		if(ulNumDacOut[uiIndex] > 1)
			return BERR_TRACE(BVDC_ERR_INVALID_DAC_SETTINGS);
	}

	if (pNewBpsInfo)
	{
		pNewBpsInfo->e656Vecpath  = e656path;
		pNewBpsInfo->eHdmiVecpath = eHdmipath;
		pNewBpsInfo->eRfmVecpath  = eRfmpath;
		pNewBpsInfo->ulRfmConst   = ulRfmConstant;
	}
	if (pNewPriInfo)
	{
		pNewPriInfo->e656Vecpath  = e656path;
		pNewPriInfo->eHdmiVecpath = eHdmipath;
		pNewPriInfo->eRfmVecpath  = eRfmpath;
		pNewPriInfo->ulRfmConst   = ulRfmConstant;
	}
	if (pNewSecInfo)
	{
		pNewSecInfo->e656Vecpath  = e656path;
		pNewSecInfo->eHdmiVecpath = eHdmipath;
		pNewSecInfo->eRfmVecpath  = eRfmpath;
		pNewSecInfo->ulRfmConst   = ulRfmConstant;
	}
	if (pNewTerInfo)
	{
		pNewTerInfo->e656Vecpath  = e656path;
		pNewTerInfo->eHdmiVecpath = eHdmipath;
		pNewTerInfo->eRfmVecpath  = eRfmpath;
		pNewTerInfo->ulRfmConst   = ulRfmConstant;
	}

	/* Set DAC enable flags */
	for (uiIndex=0; uiIndex < BVDC_P_MAX_DACS; uiIndex++)
	{
		/* unused means disable */
		if (pNewPriInfo)
		{
			pNewPriInfo->abEnableDac[uiIndex] = ulNumDacOut[uiIndex] > 0;
		}
		if (pNewSecInfo)
		{
			pNewSecInfo->abEnableDac[uiIndex] = ulNumDacOut[uiIndex] > 0;
		}
		if (pNewTerInfo)
		{
			pNewTerInfo->abEnableDac[uiIndex] = ulNumDacOut[uiIndex] > 0;
		}
		if (pNewBpsInfo)
		{
			pNewBpsInfo->abEnableDac[uiIndex] = ulNumDacOut[uiIndex] > 0;
		}
	}

	BDBG_LEAVE(BVDC_P_Display_ValidateChanges);
	return eStatus;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Display_ApplyChanges_isr
 *
 *  New requests have been validated. New user settings will be applied
 *  at next Display isr.
 **************************************************************************/
void BVDC_P_Display_ApplyChanges_isr
	( BVDC_Display_Handle              hDisplay )
{
	uint32_t i;
	uint32_t ulReg;
	BVDC_P_DisplayInfo    *pCurInfo;
	BVDC_P_DisplayInfo    *pNewInfo;

	BDBG_ENTER(BVDC_P_Display_ApplyChanges_isr);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	BDBG_OBJECT_ASSERT(hDisplay->hCompositor, BVDC_CMP);

	/* To reduce the amount of typing */
	pNewInfo = &hDisplay->stNewInfo;
	pCurInfo = &hDisplay->stCurInfo;

	/* State transition for display/compositor. */
	if(BVDC_P_STATE_IS_CREATE(hDisplay))
	{
		BDBG_MSG(("Display%d activated.", hDisplay->eId));
		hDisplay->eState = BVDC_P_State_eActive;
		/* Re-enable callback in apply. */
		for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
		{
			BINT_ClearCallback_isr(hDisplay->hCompositor->ahCallback[i]);
			BINT_EnableCallback_isr(hDisplay->hCompositor->ahCallback[i]);
		}

		/* Assign Trigger to slot.  Effectively enable slots. */
		BVDC_P_Compositor_AssignTrigger_isr(hDisplay->hCompositor,
			hDisplay->eTopTrigger, hDisplay->eBotTrigger);

		/* set up top level vec muxing */
		if(!hDisplay->bIsBypass)
		/* slect which cmp goes to which display */
		{
			ulReg =
				BCHP_FIELD_ENUM(VIDEO_ENC_PRIM_SRC_SEL, SOURCE, S0_SOURCE+hDisplay->hCompositor->eId);

			BREG_Write32(hDisplay->hVdc->hRegister,
				BCHP_VIDEO_ENC_PRIM_SRC_SEL + hDisplay->eId * sizeof(uint32_t), ulReg);
		}
	}
	else if(BVDC_P_STATE_IS_DESTROY(hDisplay))
	{
		BDBG_MSG(("Display%d de-activated.", hDisplay->eId));

		/* disable triggers to complete shutdown display callbacks. */
		BVDC_P_Display_EnableTriggers_isr(hDisplay, false);

		/* Alignment slave is destroyed */
		if(pCurInfo->hTargetDisplay &&
		   pCurInfo->hTargetDisplay->ulAlignSlaves)
		{
			pCurInfo->hTargetDisplay->ulAlignSlaves--;
		}

		hDisplay->eState = BVDC_P_State_eInactive;
		for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
		{
			BRDC_Slot_Disable_isr(hDisplay->hCompositor->ahSlot[i]);
		}

		for(i = 0; i < BVDC_P_CMP_MAX_SLOT_COUNT; i++)
		{
			BINT_DisableCallback_isr(hDisplay->hCompositor->ahCallback[i]);
			BINT_ClearCallback_isr(hDisplay->hCompositor->ahCallback[i]);
		}
		if(!hDisplay->bIsBypass)
		/* slect which cmp goes to which display */
		{
			ulReg = BCHP_FIELD_ENUM(VIDEO_ENC_PRIM_SRC_SEL, SOURCE, DISABLE);

			BREG_Write32(hDisplay->hVdc->hRegister,
				BCHP_VIDEO_ENC_PRIM_SRC_SEL + hDisplay->eId * sizeof(uint32_t), ulReg);
		}
	}

	/**
	 * Reset IT state to not active for bypass, in order for apply
	 * changes to restart ISR.
	 */
	if((hDisplay->bIsBypass) &&
	   ((pNewInfo->bEnable656  && !pCurInfo->bEnable656) ||
	    (pNewInfo->bEnableHdmi && !pCurInfo->bEnableHdmi)))
	{
		hDisplay->eItState = BVDC_P_ItState_eNotActive;
		BDBG_MSG(("Display %d resets state to kick start", hDisplay->eId));
	}

	/* Requires reset/program IT block:
	 * 1) Switch format
	 * 2) DVI was off, now it is on (sync up DVI/Vec)
	 *    If the DVI block is started up after the "Master/Slave
	 *    Synchronization" signal is sent from Primary DTG, the two
	 *    paths  will be out of sync and the output will probably stall.
	 * 3) 656 is turned on after Vec is already active
	 *    Setting ITU656_PATH0_BVB_SEL also requires a syncronization
	 *    signal from the Primary DTG.
	 * 4) When run into/out of Macrovision Test01/02 configuration.
	 */
	if (
		(hDisplay->eItState != BVDC_P_ItState_eNotActive) &&
			((pNewInfo->stDirty.stBits.bTiming) ||
			(pNewInfo->eMacrovisionType >= BVDC_MacrovisionType_eTest01) ||
			(pCurInfo->eMacrovisionType >= BVDC_MacrovisionType_eTest01) ||
			(hDisplay->iDcsChange == 2) ||
			(!pCurInfo->bEnableHdmi && pNewInfo->bEnableHdmi) ||
			(!pCurInfo->bEnable656 && pNewInfo->bEnable656) ||
			(pCurInfo->aulHdmiDropLines[pNewInfo->pFmtInfo->eVideoFmt] !=
				pNewInfo->aulHdmiDropLines[pNewInfo->pFmtInfo->eVideoFmt])))
	{
		BDBG_MSG(("[ApplyChanges], %s", pNewInfo->pFmtInfo->pchFormatStr));
		hDisplay->eItState = BVDC_P_ItState_eSwitchMode;
	}

	/* Are the new settings different from the current ones */
	if((BVDC_P_IS_DIRTY(&(pNewInfo->stDirty))) ||
	   (pCurInfo->pfGenericCallback != pNewInfo->pfGenericCallback) ||
	   (pCurInfo->pvGenericParm1 != pNewInfo->pvGenericParm1) ||
	   (pCurInfo->bEnable656 != pNewInfo->bEnable656) ||
	   (pCurInfo->bEnableHdmi != pNewInfo->bEnableHdmi) ||
	   (pCurInfo->eHdmiOutput != pNewInfo->eHdmiOutput) ||
	   (pCurInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi] !=
	    pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi]) ||
	   (pCurInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] !=
	    pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent]) ||
	   (pCurInfo->eRfmOutput != pNewInfo->eRfmOutput) ||
	   (pCurInfo->ulRfmConst != pNewInfo->ulRfmConst) ||
	   (pCurInfo->bFullRate != pNewInfo->bFullRate) ||
	   (pCurInfo->eTimeBase != pNewInfo->eTimeBase) ||
	   (pCurInfo->bCCEnable != pNewInfo->bCCEnable) ||
	   (pCurInfo->bSyncOnly != pNewInfo->bSyncOnly) ||
	   (pCurInfo->bXvYcc != pNewInfo->bXvYcc) ||
	   (hDisplay->bMvChange) ||
	   (hDisplay->iDcsChange != 0) ||
	   (hDisplay->eItState == BVDC_P_ItState_eNotActive) ||
	   (hDisplay->hCompositor->stCurInfo.ulBgColorYCrCb !=
	    hDisplay->hCompositor->stNewInfo.ulBgColorYCrCb) ||
	   (pCurInfo->aulHdmiDropLines[pNewInfo->pFmtInfo->eVideoFmt] !=
	    pNewInfo->aulHdmiDropLines[pNewInfo->pFmtInfo->eVideoFmt]))
	{
		/* alignment */
		if(!pCurInfo->hTargetDisplay)
		{
			if(pNewInfo->hTargetDisplay)
			{
				pNewInfo->hTargetDisplay->ulAlignSlaves++;
				BDBG_MSG(("Display %d will be aligned to display %d with %d slaves.",
					hDisplay->eId, pNewInfo->hTargetDisplay->eId, pNewInfo->hTargetDisplay->ulAlignSlaves));
			}
		}
		else
		{
			if(!pNewInfo->hTargetDisplay) /* disables alignment */
			{
				BDBG_MSG(("Display%d to disable alignment.", hDisplay->eId));
				if(pCurInfo->hTargetDisplay->ulAlignSlaves)
				{
					pCurInfo->hTargetDisplay->ulAlignSlaves--;
				}
			}
		}

		/* re-calculate display pixel aspect ratio */
		if (pNewInfo->stDirty.stBits.bAspRatio)
		{
			uint32_t  ulPxlAspRatio_y_x;

			pCurInfo->eAspectRatio = pNewInfo->eAspectRatio;
			pCurInfo->uiSampleAspectRatioX = pNewInfo->uiSampleAspectRatioX;
			pCurInfo->uiSampleAspectRatioY = pNewInfo->uiSampleAspectRatioY;
			pCurInfo->stAspRatRectClip = pNewInfo->stAspRatRectClip;

			BVDC_P_CalcuPixelAspectRatio_isr(
				pCurInfo->eAspectRatio,
				pCurInfo->uiSampleAspectRatioX,
				pCurInfo->uiSampleAspectRatioY,
				pNewInfo->pFmtInfo->ulWidth,
				pNewInfo->pFmtInfo->ulHeight,
				&pCurInfo->stAspRatRectClip,
				&hDisplay->ulPxlAspRatio,
				&ulPxlAspRatio_y_x,
				BFMT_Orientation_e2D);

			/* inform window ApplyChanges  */
			hDisplay->hCompositor->bDspAspRatDirty = true;
		}

		/* Copying the new info to the current info.  Must be careful here
		 * of not globble current dirty bits set by source, but rather ORed
		 * them together. */
		BVDC_P_OR_ALL_DIRTY(&pNewInfo->stDirty, &pCurInfo->stDirty);

		/* Our new now becomes our current(hw-applied) */
		pCurInfo->stDirty = pNewInfo->stDirty;

		/* Clear dirty bits since it's already OR'ed into current.  Notes
		 * the it might not apply until next vysnc, so we're defering
		 * setting the event until next vsync. */
		BVDC_P_CLEAN_ALL_DIRTY(&pNewInfo->stDirty);

		/* Isr will set event to notify apply done. */
		BKNI_ResetEvent(hDisplay->hAppliedDoneEvent);
		hDisplay->bSetEventPending = true;
		hDisplay->bUserAppliedChanges = true;
	}

	/* No RUL has been executed yet, we've no triggers activated.  Must
	 * force the initial top RUL here! */
	if((BVDC_P_ItState_eNotActive == hDisplay->eItState) &&
	   ((!hDisplay->bIsBypass) ||
	    (pNewInfo->bEnable656) ||
	    (pNewInfo->bEnableHdmi)))
	{
		/* Start List */
		BVDC_P_ListInfo stList;

		/* Build Vec Top RUL and force it to execute immediately. */
		BRDC_List_Handle hList = BVDC_P_CMP_GET_LIST(hDisplay->hCompositor,
			BAVC_Polarity_eTopField);
		BRDC_Slot_Handle hSlot = BVDC_P_CMP_GET_SLOT(hDisplay->hCompositor,
			BAVC_Polarity_eTopField);

		/* Remove the first NOP in the Rul for 7038-B0 */
		BRDC_List_SetNumEntries_isr(hList, 0);
		BVDC_P_ReadListInfo_isr(&stList, hList);
		BVDC_P_Vec_BuildRul_isr(hDisplay, &stList, BAVC_Polarity_eTopField);
		BVDC_P_WriteListInfo_isr(&stList, hList);
		BRDC_Slot_SetList_isr(hSlot, hList);
		BRDC_Slot_Execute_isr(hSlot);
	}

	if(hDisplay->bIsBypass && (!pNewInfo->bEnable656) && (!pNewInfo->bEnableHdmi))
	{
		BKNI_SetEvent_isr(hDisplay->hAppliedDoneEvent);
	}

	BDBG_LEAVE(BVDC_P_Display_ApplyChanges_isr);
	return;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Display_AbortChanges
	( BVDC_Display_Handle              hDisplay )
{
	BDBG_ENTER(BVDC_P_Display_AbortChanges);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);

	/* Cancel the setting user set to the new state. */
	hDisplay->stNewInfo = hDisplay->stCurInfo;

	/* SW7425-4609: restore validation value */
	hDisplay->stNewInfo.ulVertFreq = hDisplay->stNewInfo.pFmtInfo->ulVertFreq;

	BDBG_LEAVE(BVDC_P_Display_AbortChanges);
	return;
}

/*************************************************************************
 *  {secret}
 * BVDC_P_Display_FindDac_isr
 *  Return true if found, false otherwise.
 **************************************************************************/
bool BVDC_P_Display_FindDac_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_DacOutput                   eDacOutput )
{
	uint32_t               uiIndex;
	BVDC_P_DisplayInfo     *pNewInfo;

	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	pNewInfo = &hDisplay->stNewInfo;

	/* Find the Dac output in the array */
	for(uiIndex=0; uiIndex < BVDC_P_MAX_DACS; uiIndex++)
	{
		if (pNewInfo->aDacOutput[uiIndex] == eDacOutput)
		{
			return true;
		}
	}
	return false;
}


/*************************************************************************
 *  {secret}
 * BVDC_P_Display_EnableTriggers_isr
 *  Re-enable trigger after vec reset.
 **************************************************************************/
void BVDC_P_Display_EnableTriggers_isr
	( BVDC_Display_Handle              hDisplay,
	  bool                             bEnable )
{
	uint32_t ulTrigger0;
	uint32_t ulTrigger1;

	BDBG_ENTER(BVDC_P_Display_EnableTriggers_isr);
	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
	switch(hDisplay->eMasterTg)
	{
	case BVDC_DisplayTg_ePrimIt:
	case BVDC_DisplayTg_eSecIt:
	case BVDC_DisplayTg_eTertIt:
		/* Re-enable triggers. */
		ulTrigger0 = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			BCHP_PRIM_IT_VEC_TRIGGER_0 + hDisplay->lItOffset);
		ulTrigger0 &= ~BCHP_PRIM_IT_VEC_TRIGGER_0_ENABLE_MASK;
		if(bEnable)
		{
			ulTrigger0 |= BCHP_PRIM_IT_VEC_TRIGGER_0_ENABLE_MASK;
		}
		BREG_Write32(hDisplay->hVdc->hRegister,
			BCHP_PRIM_IT_VEC_TRIGGER_0 + hDisplay->lItOffset, ulTrigger0);

		ulTrigger1 = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			BCHP_PRIM_IT_VEC_TRIGGER_1 + hDisplay->lItOffset);
		ulTrigger1 &= ~BCHP_PRIM_IT_VEC_TRIGGER_1_ENABLE_MASK;
		if(bEnable)
		{
			ulTrigger1 |= (hDisplay->stCurInfo.pFmtInfo->bInterlaced)
				? BCHP_PRIM_IT_VEC_TRIGGER_1_ENABLE_MASK : 0;
		}
		BREG_Write32(hDisplay->hVdc->hRegister,
			BCHP_PRIM_IT_VEC_TRIGGER_1 + hDisplay->lItOffset, ulTrigger1);
		break;

#if BVDC_P_SUPPORT_656_MASTER_MODE
	case BVDC_DisplayTg_e656Dtg: /* 656out in master mode */
		/* Re-enable triggers. */
		ulTrigger0 = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			BCHP_DTG_DTG_TRIGGER_0);
		ulTrigger0 &= ~BCHP_DVI_DTG_DTG_TRIGGER_0_ENABLE_MASK;
		if(bEnable)
		{
			ulTrigger0 |= BCHP_DVI_DTG_DTG_TRIGGER_0_ENABLE_MASK;
		}
		BREG_Write32(hDisplay->hVdc->hRegister,
			BCHP_DTG_DTG_TRIGGER_0, ulTrigger0);

		ulTrigger1 = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			BCHP_DTG_DTG_TRIGGER_1);
		ulTrigger1 &= ~BCHP_DVI_DTG_DTG_TRIGGER_1_ENABLE_MASK;
		if(bEnable)
		{
			ulTrigger1 |= (hDisplay->stCurInfo.pFmtInfo->bInterlaced)
				? BCHP_DVI_DTG_DTG_TRIGGER_1_ENABLE_MASK : 0;
		}
		BREG_Write32(hDisplay->hVdc->hRegister,
			BCHP_DTG_DTG_TRIGGER_1, ulTrigger1);
		break;
#endif

#if BVDC_P_SUPPORT_DVO_MASTER_MODE
	case BVDC_DisplayTg_eDviDtg: /* dvo in master mode */
		/* Re-enable triggers. */
		ulTrigger0 = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			BCHP_DVI_DTG_DTG_TRIGGER_0);
		ulTrigger0 &= ~BCHP_DVI_DTG_DTG_TRIGGER_0_ENABLE_MASK;
		if(bEnable)
		{
			ulTrigger0 |= BCHP_DVI_DTG_DTG_TRIGGER_0_ENABLE_MASK;
		}
		BREG_Write32(hDisplay->hVdc->hRegister,
			BCHP_DVI_DTG_DTG_TRIGGER_0, ulTrigger0);

		ulTrigger1 = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			BCHP_DVI_DTG_DTG_TRIGGER_1);
		ulTrigger1 &= ~BCHP_DVI_DTG_DTG_TRIGGER_1_ENABLE_MASK;
		if(bEnable)
		{
			ulTrigger1 |= (hDisplay->stCurInfo.pFmtInfo->bInterlaced)
				? BCHP_DVI_DTG_DTG_TRIGGER_1_ENABLE_MASK : 0;
		}
		BREG_Write32(hDisplay->hVdc->hRegister,
			BCHP_DVI_DTG_DTG_TRIGGER_1, ulTrigger1);
		break;
#endif
	default: break;
	}
	BDBG_LEAVE(BVDC_P_Display_EnableTriggers_isr);

	return;
}

static void BVDC_P_Display_SetFormatSwitchMarker_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList )
{
	/* This RUL sets a marker to indicate that we are switching
	 * format and VEC triggers are disabled. Upon detecting this marker,
	 * BVDC_P_CompositorDisplay_isr() will re-enable the triggers.
	 */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(hDisplay->ulRdcVarAddr);
	*pList->pulCurrent++ = 1;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Display_ResumeRmPhaseInc_isr
 **************************************************************************/
static void BVDC_P_Display_ResumeRmPhaseInc_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList )
{
	/* PRIM_RM_PHASE_INC (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_RM_PHASE_INC + hDisplay->lRmOffset);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_RM_PHASE_INC, PHASE_INC,
			hDisplay->pRmTable[BVDC_P_DISPLAY_RM_PHASE_INC_REG_OFFSET]);
}

#if (BVDC_P_SUPPORT_VEC_DITHER)
/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_Dither_isr
 *  Builds CSC Dither settings RUL for a display.
 **************************************************************************/
void BVDC_P_Vec_Build_Dither_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_DitherSetting            *pDither,
	  BVDC_P_ListInfo                 *pList )
{
	BSTD_UNUSED(hDisplay);

	/* PRIM_CSC_DITHER_CONTROL */
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_CONTROL, MODE,       pDither->ulMode     ) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_CONTROL, OFFSET_CH2, pDither->ulCh2Offset) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_CONTROL, SCALE_CH2,  pDither->ulCh2Scale ) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_CONTROL, OFFSET_CH1, pDither->ulCh1Offset) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_CONTROL, SCALE_CH1,  pDither->ulCh1Scale ) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_CONTROL, OFFSET_CH0, pDither->ulCh0Offset) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_CONTROL, SCALE_CH0,  pDither->ulCh0Scale );

	/* PRIM_CSC_DITHER_LFSR */
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_LFSR, T0, pDither->ulLfsrCtrlT0) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_LFSR, T1, pDither->ulLfsrCtrlT1) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_LFSR, T2, pDither->ulLfsrCtrlT2);

	/* PRIM_CSC_DITHER_LFSR_INIT */
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_LFSR_INIT, SEQ,   pDither->ulLfsrSeq  ) |
		BCHP_FIELD_DATA(PRIM_CSC_DITHER_LFSR_INIT, VALUE, pDither->ulLfsrValue);

}
#endif


/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_CSC_isr
 *  Builds CSC settings RUL for a display.
 **************************************************************************/
void BVDC_P_Vec_Build_CSC_isr
	( const BVDC_P_DisplayCscMatrix   *pCscMatrix,
	  BVDC_P_ListInfo                 *pList )
{
	BDBG_ASSERT(pCscMatrix);

	*pList->pulCurrent++ =
		BCHP_FIELD_ENUM(PRIM_CSC_CSC_MODE, CLAMP_MODE_C0, MIN_MAX) |
		BCHP_FIELD_ENUM(PRIM_CSC_CSC_MODE, CLAMP_MODE_C1, MIN_MAX) |
		BCHP_FIELD_ENUM(PRIM_CSC_CSC_MODE, CLAMP_MODE_C2, MIN_MAX) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_MODE, RANGE1, 0x005A) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_MODE, RANGE2, 0x007F);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_CSC_MIN_MAX, MIN, pCscMatrix->ulMin) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_MIN_MAX, MAX, pCscMatrix->ulMax);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C01_C00, COEFF_C0, pCscMatrix->stCscCoeffs.usY0) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C01_C00, COEFF_C1, pCscMatrix->stCscCoeffs.usY1);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C03_C02, COEFF_C2, pCscMatrix->stCscCoeffs.usY2) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C03_C02, COEFF_C3, pCscMatrix->stCscCoeffs.usYOffset);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C11_C10, COEFF_C0, pCscMatrix->stCscCoeffs.usCb0) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C11_C10, COEFF_C1, pCscMatrix->stCscCoeffs.usCb1);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C13_C12, COEFF_C2, pCscMatrix->stCscCoeffs.usCb2) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C13_C12, COEFF_C3, pCscMatrix->stCscCoeffs.usCbOffset);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C21_C20, COEFF_C0, pCscMatrix->stCscCoeffs.usCr0) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C21_C20, COEFF_C1, pCscMatrix->stCscCoeffs.usCr1);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C23_C22, COEFF_C2, pCscMatrix->stCscCoeffs.usCr2) |
		BCHP_FIELD_DATA(PRIM_CSC_CSC_COEFF_C23_C22, COEFF_C3, pCscMatrix->stCscCoeffs.usCrOffset);
}

#if BVDC_P_SUPPORT_DVI_OUT

/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_Build_DVI_isr
 *  Builds DTRAM, DVI_DTG, DVI_DVF
 **************************************************************************/
static void BVDC_P_Vec_Build_DVI_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  uint32_t                         ulHeight,
	  uint32_t                         ulTrig0Val,
	  uint32_t                         ulTrig1Val )
{
	const uint32_t *pDtRam;
#if (!BVDC_P_VEC_HAS_2_DIFFERENT_DVF)
	uint32_t upsample2x;
#endif
	const BVDC_P_DisplayInfo *pNewInfo = &hDisplay->stNewInfo;
	const BVDC_Display_DvoSettings *pDvoCfg = &pNewInfo->stDvoCfg;
	uint32_t ulIsSlave =
		(hDisplay->eMasterTg == BVDC_DisplayTg_e656Dtg) ? 0 : 1;
	uint32_t ulDviAutoRestart = ulIsSlave;

	/* Dtram[40..7f] is for DVI */
	pDtRam = BVDC_P_GetDtramTable_isr(pNewInfo, pNewInfo->pFmtInfo, hDisplay->bArib480p);
	BDBG_MSG(("Dtram microcode - timestamp: 0x%.8x", pDtRam[BVDC_P_DTRAM_TABLE_TIMESTAMP_IDX]));
	BDBG_MSG(("Dtram microcode - checksum:  0x%.8x", pDtRam[BVDC_P_DTRAM_TABLE_CHECKSUM_IDX]));

	/*-------------------------------*/
	/* NOTE: Reset Must happen first */
	/*-------------------------------*/
	/* VIDEO_ENC_SOFT_RESET_DVI */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_DVI);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_DVI_DVI_DVF, RESET, 1) |
		BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_DVI_DVI_DTG, RESET, 1);

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_DVI);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_DVI_DVI_DVF, RESET, 0) |
		BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_DVI_DVI_DTG, RESET, 0 );

	/* Load here! */
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DTRAM_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTRAM_DMC_INSTRUCTIONi_ARRAY_BASE +
		(BVDC_P_DVI_DTRAM_START_ADDR * sizeof(uint32_t)));
	BKNI_Memcpy((void*)pList->pulCurrent, (void*)pDtRam,
		BVDC_P_DTRAM_TABLE_SIZE * sizeof(uint32_t));
	pList->pulCurrent += BVDC_P_DTRAM_TABLE_SIZE;

	/* DVI_DTG_DTG_BVB (RW) */
	/* DVI_DTG_CCIR_PCL (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_CCIR_PCL);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, HACTIVE_ENABLE,  1) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, HACTIVE_SEL,     0) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, VACTIVE_ENABLE,  1) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, VACTIVE_SEL,     0) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, VBLANK_ENABLE,   1) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, VBLANK_SEL,      0) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, ODD_EVEN_ENABLE, 1) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, ODD_EVEN_SEL,    0);  /* nominal */

	/* DVI_DTG_DVI_PCL (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_DVI_PCL);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DIGITAL_HSYNC_ENABLE, 1) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DIGITAL_HSYNC_SEL,    0) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DVI_V_ENABLE,         1) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DVI_V_SEL,            1) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DVI_DE_ENABLE,        1) | /* nominal */
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DVI_DE_SEL,           3);  /* nominal */

	/* DVI_DTG_CNTL_PCL (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_CNTL_PCL);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_CNTL_PCL, NEW_LINE_CLR_SEL, 3 );  /* nominal */

	/* DVI_DTG_RAM_ADDR (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_RAM_ADDR);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_RAM_ADDR, MC_START_ADDR,
			BVDC_P_DVI_DTRAM_START_ADDR                       );  /* nominal */

	/* DVI_DTG_DTG_BVB_SIZE (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_DTG_BVB_SIZE);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_DTG_BVB_SIZE, HORIZONTAL, pNewInfo->pFmtInfo->ulWidth ) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_BVB_SIZE, VERTICAL, ulHeight                      );

	/* DVI_DTG_DTG_BVB_RSTATUS (RO) */
	/* DVI_DTG_DTG_BVB_CSTATUS (WO) */
	/* DVI_DTG_DTG_CTRL_STAT (RO) */
	/* DVI_DTG_DTG_LCNTR (RO) */
	/* DVI_DVF_DVF_REV_ID (RO) */
	/* DVI_DVF_DVF_CONFIG (RW) */
#if (!BVDC_P_VEC_HAS_2_DIFFERENT_DVF)
	if ((VIDEO_FORMAT_IS_ED(pNewInfo->pFmtInfo->eVideoFmt)) &&
		(BAVC_HDMI_PixelRepetition_e2x == pNewInfo->eHdmiPixelRepetition))
	{
		upsample2x = BCHP_DVI_DVF_DVF_CONFIG_UPSAMPLE2X_ON;
	}
	else
	{
		upsample2x = (VIDEO_FORMAT_IS_SD(pNewInfo->pFmtInfo->eVideoFmt)) ?
			BCHP_DVI_DVF_DVF_CONFIG_UPSAMPLE2X_ON :
			BCHP_DVI_DVF_DVF_CONFIG_UPSAMPLE2X_OFF ;
	}
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DVF_DVF_CONFIG);
	*pList->pulCurrent++ =
		BCHP_FIELD_ENUM(DVI_DVF_DVF_CONFIG, VBI_PREFERRED, OFF ) |
		BCHP_FIELD_ENUM(DVI_DVF_DVF_CONFIG, VBI_ENABLE,    OFF ) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_CONFIG, UPSAMPLE2X,  upsample2x  );
#endif

#if (BVDC_P_SUPPORT_DVO_MASTER_MODE)

	/* DVI_DTG_DTG_TRIGGER_0 (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_DTG_TRIGGER_0);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_DTG_TRIGGER_0, ENABLE,        0 ) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_TRIGGER_0, TRIGGER_VALUE, ulTrig0Val );

	/* DVI_DTG_DTG_TRIGGER_1 (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_DTG_TRIGGER_1);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_DTG_TRIGGER_1, ENABLE,        0 ) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_TRIGGER_1, TRIGGER_VALUE, ulTrig1Val );
#else
	BSTD_UNUSED(ulTrig0Val);
	BSTD_UNUSED(ulTrig1Val);
#endif

	/*-------------------------------*/
	/* NOTE: HW requires Conig last  */
	/*-------------------------------*/
	/* DVI_DTG_DTG_CONFIG (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_DTG_CONFIG);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, AUTO_RESTART, ulDviAutoRestart     ) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, RESTART_WIN,   31                  ) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, TOGGLE_DVI_DE, pDvoCfg->eDePolarity) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, TOGGLE_DVI_V,  pDvoCfg->eVsPolarity) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, TOGGLE_DVI_H,  pDvoCfg->eHsPolarity) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, TRIGGER_CNT_CLR_COND, 0            ) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, SLAVE_MODE,           ulIsSlave    ) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, MCS_ENABLE, 1                      );

#if (BVDC_P_SUPPORT_DVO_MASTER_MODE)
	/* NOTE: Set ulRdcVarAddr first time through, and allow switching
	 * of Vec State to Active by ulRdcVarAddr handlers.  TG reset transitions. */
	if(BVDC_DisplayTg_eDviDtg == hDisplay->eMasterTg)
	{
		/* Reset Marker and polarity record */
		BVDC_P_Display_SetFormatSwitchMarker_isr(hDisplay, pList);

		/* KLUDGE: This sent some sort of signal to HW, and we must need
		 * it for it to work properly. */
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTRAM_DTRAM_CONFIG);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(DTRAM_DTRAM_CONFIG, ARBITER_LATENCY, 0xb);
	}
#endif

	return;
}


/***************************************************************************
 *
 */
static void BVDC_P_Vec_Build_DVI_CSC_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList )
{
	const BVDC_P_DisplayCscMatrix *pCscMatrix = NULL;
	const BVDC_P_DisplayInfo *pNewInfo = &hDisplay->stNewInfo;

	/* Reset DVI_CSC */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_DVI);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_DVI_DVI_CSC, RESET, 1);

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_DVI);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_DVI_DVI_CSC, RESET, 0 );

	/* --- Setup DVI CSC --- */
	BVDC_P_Display_GetDviCscTable_isr(pNewInfo, &pCscMatrix);

	if(pNewInfo->bUserCsc)
	{
		BVDC_P_Csc_FromMatrixDvo_isr(&hDisplay->stDvoCscMatrix.stCscCoeffs,
			pNewInfo->pl32_Matrix, pNewInfo->ulUserShift,
			((pNewInfo->eHdmiOutput == BAVC_MatrixCoefficients_eHdmi_RGB) ||
			 (pNewInfo->eHdmiOutput == BAVC_MatrixCoefficients_eDvi_Full_Range_RGB))? true : false);

		hDisplay->stDvoCscMatrix.ulMin = pCscMatrix->ulMin;
		hDisplay->stDvoCscMatrix.ulMax = pCscMatrix->ulMax;
	}
	else
	{
		/* For BVDC_Display_GetDvoColorMatrix() */
		hDisplay->stDvoCscMatrix = *pCscMatrix;
	}

	if(!pNewInfo->abOutputMute[BVDC_DisplayOutput_eDvo])
	{
		BVDC_P_Csc_DvoApplyAttenuationRGB_isr(hDisplay->stNewInfo.lDvoAttenuationR,
		                                      hDisplay->stNewInfo.lDvoAttenuationG,
		                                      hDisplay->stNewInfo.lDvoAttenuationB,
		                                      hDisplay->stNewInfo.lDvoOffsetR,
		                                      hDisplay->stNewInfo.lDvoOffsetG,
		                                      hDisplay->stNewInfo.lDvoOffsetB,
		                                     &hDisplay->stDvoCscMatrix.stCscCoeffs);
	}
	else
	{
		uint8_t ucCh0, ucCh1, ucCh2;
		const BVDC_P_Compositor_Info *pCmpInfo = &hDisplay->hCompositor->stCurInfo;

		ucCh0 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 2);
		ucCh1 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 1);
		ucCh2 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 0);

		/* Swap ch0 and 1 of input color to match vec csc layout */
		BVDC_P_Csc_ApplyYCbCrColor_isr(&hDisplay->stDvoCscMatrix.stCscCoeffs, ucCh1, ucCh0, ucCh2);
	}

	/* DVI_CSC_CSC_MODE (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CSC_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_CSC_CSC_MODE);

	/* Setup CSC for Dac outputs */
	BVDC_P_Vec_Build_CSC_isr(&hDisplay->stDvoCscMatrix, pList);

#if (BVDC_P_SUPPORT_VEC_DITHER)
	/* DVI_CSC_DITHER */
	if(hDisplay->stNewInfo.stDvoCfg.b8BitPanel)
	{
		hDisplay->stDviDither.ulCh0Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_8BIT;
		hDisplay->stDviDither.ulCh1Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_8BIT;
		hDisplay->stDviDither.ulCh2Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_8BIT;
		hDisplay->stDviDither.ulCh0Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_8BIT;
		hDisplay->stDviDither.ulCh1Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_8BIT;
		hDisplay->stDviDither.ulCh2Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_8BIT;
	}
	else
	{
		hDisplay->stDviDither.ulCh0Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_10BIT;
		hDisplay->stDviDither.ulCh1Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_10BIT;
		hDisplay->stDviDither.ulCh2Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_10BIT;
		hDisplay->stDviDither.ulCh0Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_10BIT;
		hDisplay->stDviDither.ulCh1Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_10BIT;
		hDisplay->stDviDither.ulCh2Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_10BIT;
	}

	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DITHER_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_CSC_DITHER_CONTROL);
	BVDC_P_Vec_Build_Dither_isr(hDisplay, &hDisplay->stDviDither, pList);
#endif

	return;
}


/*************************************************************************
 *  {secret}
 *
 *  Builds the blank color of DVI or 656 output.  Use the compositor
 *  background color.
 **************************************************************************/
static void BVDC_P_Vec_Build_DVF_Color_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  uint32_t                         ulDvfRegAddr )
{
	uint8_t ucCh0, ucCh1, ucCh2;
	const BVDC_P_Compositor_Info *pCmpInfo = &hDisplay->hCompositor->stCurInfo;

	if((BAVC_MatrixCoefficients_eHdmi_RGB == hDisplay->stCurInfo.eHdmiOutput) ||
	   (BAVC_MatrixCoefficients_eDvi_Full_Range_RGB == hDisplay->stCurInfo.eHdmiOutput))
	{
		ucCh0 = pCmpInfo->ucGreen;
		ucCh1 = pCmpInfo->ucBlue;
		ucCh2 = pCmpInfo->ucRed;
	}
	else
	{
		ucCh0 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 2);
		ucCh1 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 1);
		ucCh2 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 0);
	}

	/* DVI_DVF_DVF_VALUES (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(ulDvfRegAddr);
	*pList->pulCurrent++ =
#if (!BVDC_P_VEC_HAS_2_DIFFERENT_DVF)
		BCHP_FIELD_DATA(DVI_DVF_DVF_VALUES, CH0_VBI_OFFSET, 0x1  ) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_VALUES, CH2_BLANK, ucCh2     ) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_VALUES, CH1_BLANK, ucCh1     ) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_VALUES, CH0_BLANK, ucCh0     );
#else
		BCHP_FIELD_DATA(DVI_DVF_DVI_DVF_VALUES, CH2_BLANK, ucCh2 ) |
		BCHP_FIELD_DATA(DVI_DVF_DVI_DVF_VALUES, CH1_BLANK, ucCh1 ) |
		BCHP_FIELD_DATA(DVI_DVF_DVI_DVF_VALUES, CH0_BLANK, ucCh0 );
#endif

	return;
}
#endif

#if BVDC_P_SUPPORT_ITU656_OUT
/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_Build_656_isr
 *  Build DTRAM, DTG, DVF, ITU656
 **************************************************************************/
static void BVDC_P_Vec_Build_656_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  uint32_t                         ulHeight,
	  uint32_t                         ulTrig0Val,
	  uint32_t                         ulTrig1Val )
{
	uint32_t ulNumRegs;
	const uint32_t *pTable=NULL;
	BVDC_P_DisplayInfo *pNewInfo = &hDisplay->stNewInfo;
	bool bDisplayRestart = (hDisplay->eItState == BVDC_P_ItState_eSwitchMode) ||
	   (hDisplay->eItState == BVDC_P_ItState_eNotActive);
	bool bIsMaster = (hDisplay->eMasterTg == BVDC_DisplayTg_e656Dtg);
#if !BVDC_P_SUPPORT_656_MASTER_MODE
	BSTD_UNUSED(ulTrig0Val);
	BSTD_UNUSED(ulTrig1Val);
#endif

	/* Since the vec IT reseted we want to reset the DTG as well. */
	if(bDisplayRestart)
	{
		BDBG_MSG(("Reset 656 DTG block."));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_ITU656);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_ITU656_ITU656_DTG, RESET, 1) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_ITU656_ITU656_CSC, RESET, 1) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_ITU656_ITU656_DVF, RESET, 1) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_ITU656_ITU656_FORMATTER, RESET, 1);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_ITU656);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_ITU656_ITU656_DTG, RESET, 0) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_ITU656_ITU656_CSC, RESET, 0) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_ITU656_ITU656_DVF, RESET, 0) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_ITU656_ITU656_FORMATTER, RESET, 0);
	}

	/* Setup VIDEO_ENC */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_ITU656_SRC_SEL);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(VIDEO_ENC_ITU656_SRC_SEL, SOURCE, hDisplay->hCompositor->eId);

	/* --- Setup DTRAM --- */
	/* Dtram[0..3f] is reserved for 656 */
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DTRAM_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTRAM_DMC_INSTRUCTIONi_ARRAY_BASE);

	/* get correct 656 dtram */
	pTable = BVDC_P_Get656DtramTable_isr(pNewInfo);

	BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
		BVDC_P_DTRAM_TABLE_SIZE * sizeof(uint32_t));
	pList->pulCurrent += BVDC_P_DTRAM_TABLE_SIZE;

#if BVDC_P_SUPPORT_656_MASTER_MODE /* bypass display */
	/* --- Setup 656 triggers for master mode --- */
	if (bDisplayRestart && bIsMaster)
	{
		*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(2);
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTG_DTG_TRIGGER_0);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(DVI_DTG_DTG_TRIGGER_0, TRIGGER_VALUE, ulTrig0Val) |
			BCHP_FIELD_DATA(DVI_DTG_DTG_TRIGGER_0, ENABLE, 0);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(DVI_DTG_DTG_TRIGGER_1, TRIGGER_VALUE, ulTrig1Val) |
			BCHP_FIELD_DATA(DVI_DTG_DTG_TRIGGER_1, ENABLE, 0);
	}
#endif

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTG_DTG_CONFIG);
	*pList->pulCurrent++ =
		/* master mode 656out should turn off AUTO_RESTART */
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, AUTO_RESTART,!bIsMaster) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, RESTART_WIN, 31) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, MCS_ENABLE, 1  ) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, SLAVE_MODE,  !bIsMaster) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, TRIGGER_CNT_CLR_COND, 0) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, TOGGLE_DVI_H, 0) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, TOGGLE_DVI_V, 0) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_CONFIG, TOGGLE_DVI_DE, 0);

	/* PR28786: Need to set DTG_DTG_BVB.BVB_DATA to DISP, and
	 * DTG_DTG_CONFIG.AUTO_RESTART to ON for 656 ouput to work
	 * correctly when switching from master to slave mode */
#if (BVDC_P_SUPPORT_MODULAR_VEC == 1)
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVF_DVF_BVB_CONFIG);
	*pList->pulCurrent++ =
		BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, PSYNC,     START_TOP) |
		BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, BVB_DATA , DISP);
#endif

	ulNumRegs = (BCHP_DTG_DTG_BVB_SIZE - BCHP_DTG_CCIR_PCL)/4 + 1;

	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(ulNumRegs);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTG_CCIR_PCL);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, HACTIVE_ENABLE,  1) |
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, HACTIVE_SEL,     0) |
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, VACTIVE_ENABLE,  1) |
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, VACTIVE_SEL,     0) |
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, VBLANK_ENABLE,   1) |
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, VBLANK_SEL,      0) |
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, ODD_EVEN_ENABLE, 1) |
		BCHP_FIELD_DATA(DVI_DTG_CCIR_PCL, ODD_EVEN_SEL,    0);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DIGITAL_HSYNC_ENABLE, 1) |
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DIGITAL_HSYNC_SEL,    0) |
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DVI_V_ENABLE,     0) |
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DVI_V_SEL,        0) |
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DVI_DE_ENABLE,    0) |
		BCHP_FIELD_DATA(DVI_DTG_DVI_PCL, DVI_DE_SEL,       0);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_CNTL_PCL, NEW_LINE_CLR_SEL, 3);

	/* DTG_RAM_ADDR = 0 for 656out */
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_RAM_ADDR, MC_START_ADDR, 0);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DTG_DTG_BVB_SIZE, HORIZONTAL,pNewInfo->pFmtInfo->ulWidth) |
		BCHP_FIELD_DATA(DVI_DTG_DTG_BVB_SIZE, VERTICAL, ulHeight);

	/* --- Setup DVF block --- */
	ulNumRegs = (BCHP_DVF_DVF_BVB_STATUS - BCHP_DVF_DVF_CONFIG)/4 + 1;

	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(ulNumRegs);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVF_DVF_CONFIG);

#if (!BVDC_P_VEC_HAS_2_DIFFERENT_DVF)
	*pList->pulCurrent++ =
		BCHP_FIELD_ENUM(DVI_DVF_DVF_CONFIG, VBI_PREFERRED, OFF)|
		BCHP_FIELD_ENUM(DVI_DVF_DVF_CONFIG, VBI_ENABLE,    OFF)|
		BCHP_FIELD_ENUM(DVI_DVF_DVF_CONFIG, UPSAMPLE2X,    ON) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_CONFIG, reserved0,     0) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_CONFIG, reserved_for_eco1, 0);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DVF_DVF_VALUES, CH0_VBI_OFFSET, 1) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_VALUES, CH2_BLANK,    128) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_VALUES, CH1_BLANK,    128) |
		BCHP_FIELD_DATA(DVI_DVF_DVF_VALUES, CH0_BLANK,    16);
#else
	*pList->pulCurrent++ =
		BCHP_FIELD_ENUM(DVI_DVF_DVI_DVF_CONFIG, UPSAMPLE2X,    ON) |
		BCHP_FIELD_DATA(DVI_DVF_DVI_DVF_CONFIG, reserved0,	   0) |
		BCHP_FIELD_DATA(DVI_DVF_DVI_DVF_CONFIG, reserved_for_eco1, 0);

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVI_DVF_DVI_DVF_VALUES, CH2_BLANK,	  128) |
		BCHP_FIELD_DATA(DVI_DVF_DVI_DVF_VALUES, CH1_BLANK,	  128) |
		BCHP_FIELD_DATA(DVI_DVF_DVI_DVF_VALUES, CH0_BLANK,	  16);
#endif

	/* bvb_status */
	*pList->pulCurrent++ = 0xffffffff;

	/*--- Setup ITU_656 ---*/
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_ITU656_CONFIG);
	if (hDisplay->bIsBypass)
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(ITU656_ITU656_CONFIG, FILTER_MODE, 0)|
			BCHP_FIELD_ENUM(ITU656_ITU656_CONFIG, INPUT_MODE, BYPASS)|
			BCHP_FIELD_ENUM(ITU656_ITU656_CONFIG, DATA_OUT_PATTERN, ZERO)|
			BCHP_FIELD_DATA(ITU656_ITU656_CONFIG, reserved0, 0);
	}
	else
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(ITU656_ITU656_CONFIG, FILTER_MODE, 0)|
			BCHP_FIELD_ENUM(ITU656_ITU656_CONFIG, INPUT_MODE, SHARED)|
			BCHP_FIELD_ENUM(ITU656_ITU656_CONFIG, DATA_OUT_PATTERN, ZERO)|
			BCHP_FIELD_DATA(ITU656_ITU656_CONFIG, reserved0, 0);
	}

	if (bDisplayRestart && bIsMaster)
	{
		/* Set ulRdcVarAddr first time through, and allow switching
		 * of Vec State to Active by ulRdcVarAddr handlers,
		 * init polarity record */
		BVDC_P_Display_SetFormatSwitchMarker_isr(hDisplay, pList);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTRAM_DTRAM_CONFIG);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(DTRAM_DTRAM_CONFIG, ARBITER_LATENCY, 0xb);
	}

	return;
}

/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_Build_656_CSC_isr
 *  Build CSC
 **************************************************************************/
static void BVDC_P_Vec_Build_656_CSC_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList )
{
	BVDC_P_DisplayInfo  *pNewInfo = &hDisplay->stNewInfo;
	const BVDC_P_DisplayCscMatrix *pCscMatrix = NULL;
	BVDC_P_DisplayCscMatrix stCscMatrix;

	/* --- Setup CSC ---*/
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CSC_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_CSC_CSC_MODE);

	BVDC_P_Display_Get656CscTable_isr(&hDisplay->stNewInfo,
		hDisplay->hCompositor->bIsBypass, &pCscMatrix);

	/* Setup CSC for Dac outputs */
	BVDC_P_Vec_Build_CSC_isr(pCscMatrix, pList);

	stCscMatrix = *pCscMatrix;

	/* Handle CSC mute */
	if (pNewInfo->abOutputMute[BVDC_DisplayOutput_e656])
	{
		uint8_t ucCh0, ucCh1, ucCh2;
		const BVDC_P_Compositor_Info *pCmpInfo = &hDisplay->hCompositor->stCurInfo;

		/* Blank color, use in CSC */
		ucCh0 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 2);
		ucCh1 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 1);
		ucCh2 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 0);

		/* Swap ch0 and 1 of input color to match vec csc layout */
		BVDC_P_Csc_ApplyYCbCrColor_isr(&stCscMatrix.stCscCoeffs, ucCh1, ucCh0, ucCh2);
	}

	/* Setup CSC for Dac outputs */
	BVDC_P_Vec_Build_CSC_isr(&stCscMatrix, pList);

#if (BVDC_P_SUPPORT_VEC_DITHER)
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DITHER_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_CSC_DITHER_CONTROL);
	BVDC_P_Vec_Build_Dither_isr(hDisplay, &hDisplay->st656Dither, pList);
#endif

	return;
}
#endif


#if (BVDC_P_SUPPORT_DVPO)
/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_BuildPllCfg_isr
 **************************************************************************/
static void BVDC_P_Vec_BuildPllCfg_isr
	( BVDC_P_ListInfo                 *pList,
	  BREG_Handle                      hReg,
	  const BVDC_Display_DvoSettings  *pDvoCfg,
	  const BVDC_P_RateInfo           *pRmInfo,
	  uint32_t                         ulRDiv,
	  uint32_t                         ulOffset )
{
	uint32_t ulPreDiv = 2;
	uint32_t ulReg, ulLvdsMode;

	/* Spread spectrum adjustment:
	 * When enable spread spectrum the OFFSET must be compenstate.  By
	 * 0.08% off the current offset.  Need to also make sure
	 * XPT_DPCR0_LOOP_CTRL.TIME_REF is correct setup for the correct source
	 * to have input/output locking. */
	if(pDvoCfg->stSpreadSpectrum.bEnable)
	{
		ulPreDiv  = 1;
		ulOffset  = (ulOffset / 2);
#if (BVDC_P_SUPPORT_HDMI_RM_VER < BVDC_P_HDMI_RM_VER_1)  /* pre-B2 */
		ulOffset += (ulOffset * 8)/10000;
#endif
	}

	/* Check LVDS mode
	 * Dual link:
	 *      DVPO_0_MISC_CONTROL_2.LVDS_MODE = 1
	 *      LVDS_PHY_0_LVDS_PLL_CFG. LINKDIV_CTRL = 0
	 * else
	 *      DVPO_0_MISC_CONTROL_2.LVDS_MODE = 0
	 *      LVDS_PHY_0_LVDS_PLL_CFG. LINKDIV_CTRL = 1
	 */
	ulReg  = BREG_Read32_isr(hReg, BCHP_DVPO_0_MISC_CONTROL_2);
	ulLvdsMode = BVDC_P_GET_FIELD(ulReg, DVPO_0_MISC_CONTROL_2, LVDS_MODE);

	/* LVDS_PHY_0_LVDS_PLL_CFG (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_LVDS_PLL_CFG);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CFG, LINKDIV_CTRL,            (ulLvdsMode != 1)      ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CFG, PLL_R_DIV,                ulRDiv                ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CFG, PLL_VCO_RANGE,            pRmInfo->ulVcoRange   ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CFG, PLL_M_DIV,                pRmInfo->ulMDiv       ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CFG, PLL_FEEDBACK_PRE_DIVIDER, pRmInfo->ulP2         ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CFG, PLL_INPUT_PRE_DIVIDER,    ulPreDiv              );

	/* DVPO_RM_0_OFFSET (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVPO_RM_0_OFFSET);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVPO_RM_0_OFFSET, OFFSET_ONLY, 0   ) |
		BCHP_FIELD_DATA(DVPO_RM_0_OFFSET, OFFSET, ulOffset);

	return;
}
#endif


/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_Build_RM_isr
 **************************************************************************/
static void BVDC_P_Vec_Build_RM_isr
	( BVDC_Display_Handle              hDisplay,
	  const uint32_t                  *pTable,
	  BVDC_P_ListInfo                 *pList )
{
	BDBG_ENTER(BVDC_P_Vec_Build_RM_isr);

	/* if switching mode, we must reset IT block before anything else to avoid deadlock */
	if((BVDC_P_ItState_eSwitchMode == hDisplay->eItState) ||
	   (BVDC_P_ItState_eNotActive  == hDisplay->eItState))
	{
		unsigned ulResetOffset = hDisplay->eId * sizeof(uint32_t);

		/* Reset IT */
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_PRIM + ulResetOffset);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_PRIM_PRIM_IT, RESET, 1) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_PRIM_PRIM_SM, RESET, 1) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_PRIM_PRIM_VF, RESET, 1) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_PRIM_PRIM_CSC, RESET, 1);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_PRIM + ulResetOffset);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_PRIM_PRIM_IT, RESET, 0) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_PRIM_PRIM_SM, RESET, 0) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_PRIM_PRIM_VF, RESET, 0) |
			BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_PRIM_PRIM_CSC, RESET, 0);

#if BVDC_P_SUPPORT_COMPONENT_ONLY
		if(BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
		{
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_SEC);
			*pList->pulCurrent++ =
				BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_SEC_SEC_CO_VF, RESET, 1) |
				BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_SEC_SEC_CO_CSC, RESET, 1)|
				BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_SEC_SEC_CO_SRC, RESET, 1);

			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_SOFT_RESET_SEC);
			*pList->pulCurrent++ =
				BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_SEC_SEC_CO_VF, RESET, 0) |
				BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_SEC_SEC_CO_CSC, RESET, 0)|
				BCHP_FIELD_DATA(VIDEO_ENC_SOFT_RESET_SEC_SEC_CO_SRC, RESET, 0);
		}
#endif
	}

	/* --- Setup RM --- */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_RM_CONTROL + hDisplay->lRmOffset);

	/* timebases by default:
	 * - primary Vec  (SD display) -> timebase 1;
	 * - secondary Vec(HD display) -> timebase 0; */
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_RM_CONTROL, RESET,       0) |
		BCHP_FIELD_DATA(PRIM_RM_CONTROL, INT_GAIN,    BVDC_P_RM_INTEGRAL_GAIN) |
		BCHP_FIELD_DATA(PRIM_RM_CONTROL, DIRECT_GAIN, BVDC_P_RM_DIRECT_GAIN  ) |
		BCHP_FIELD_DATA(PRIM_RM_CONTROL, DITHER,      0) |
		BCHP_FIELD_DATA(PRIM_RM_CONTROL, FREE_RUN,    0) |
		BCHP_FIELD_DATA(PRIM_RM_CONTROL, TIMEBASE,
			hDisplay->stNewInfo.eTimeBase);

	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_RM_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_RM_RATE_RATIO + hDisplay->lRmOffset);

	/* Setup RM_RATE_RATIO to RM_INTEGRATOR */
	BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
		BVDC_P_RM_TABLE_SIZE * sizeof(uint32_t));
	pList->pulCurrent += BVDC_P_RM_TABLE_SIZE;

	BDBG_LEAVE(BVDC_P_Vec_Build_RM_isr);
	return;
}

#if BVDC_P_SUPPORT_DVI_OUT
/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_Build_HdmiRM_isr
 **************************************************************************/
static void BVDC_P_Vec_Build_HdmiRM_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList )
{
#if ((!BVDC_P_SUPPORT_DVI_65NM) && (BVDC_P_SUPPORT_HDMI_OUT)) || (BVDC_P_SUPPORT_DVPO)
	uint32_t ulVcxoCtrl;
	bool bDisplayRestart =
		(hDisplay->eItState == BVDC_P_ItState_eNotActive) ||
		(hDisplay->eItState == BVDC_P_ItState_eSwitchMode);
#endif

#if ((BVDC_P_SUPPORT_DVI_65NM) && (BVDC_P_SUPPORT_HDMI_OUT)) || (BVDC_P_SUPPORT_DVPO)
	uint32_t ulSSReg;
	const BVDC_Display_DvoSettings *pDvoCfg;
#endif

	const BVDC_P_RateInfo *pRmInfo;
	const BVDC_P_DisplayInfo *pNewInfo = &hDisplay->stNewInfo;
	uint32_t ulScale, ulRange, ulHold, ulStep, ulFCW;
#if BVDC_P_SUPPORT_DVI_65NM
	uint32_t ulNdivMod, ulIcpx, ulCn;
#endif

	BDBG_ENTER(BVDC_P_Vec_Build_HdmiRM_isr);

	hDisplay->pDvoRmInfo = pRmInfo = BVDC_P_HdmiRmTableEx_isr(
		pNewInfo->pFmtInfo, pNewInfo->eHdmiPixelRepetition,
		&pNewInfo->stRateInfo);

	/* Spread sprectrum settings */
	ulHold  = 3; /* fixed value */
	ulScale = 7; /* fixed value */

	/* ulRange = 0.001 * FCW * (2**(-10-ulScale)) * ulDelta
	 * FCW = DVPO_RM_0_OFFSET
	 */
#if BVDC_P_SUPPORT_DVI_65NM
	ulFCW = hDisplay->pDvoRmInfo->ulOffset;
#else
	ulFCW = hDisplay->pDvoRmInfo->ulNDiv;
#endif

	/* ulRange computation is arranged in the following way so that
	 * there won't be overflow and we can still get accurate result
	 * while no floating point used.
	 *
	 */
	ulRange = ((ulFCW / 1310720) * pNewInfo->stDvoCfg.stSpreadSpectrum.ulDelta) / 100;

	/* ulStep = 1 / ( (1/ulFrequency) * 108000000 / ulRange / 512 / 4)
	 */
	ulStep = pNewInfo->stDvoCfg.stSpreadSpectrum.ulFrequency * ulRange / 52734;

#if BVDC_P_SUPPORT_HDMI_OUT
	/* HDMI_RM_CONTROL (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_CONTROL);
	*pList->pulCurrent++ =
		BCHP_FIELD_ENUM(HDMI_RM_CONTROL, RESET,       RESET_OFF  ) |
		BCHP_FIELD_DATA(HDMI_RM_CONTROL, INT_GAIN,    BVDC_P_RM_INTEGRAL_GAIN ) |
		BCHP_FIELD_DATA(HDMI_RM_CONTROL, DIRECT_GAIN, BVDC_P_RM_DIRECT_GAIN   ) |
		BCHP_FIELD_ENUM(HDMI_RM_CONTROL, DITHER,      DITHER_OFF ) |
		BCHP_FIELD_ENUM(HDMI_RM_CONTROL, FREE_RUN,    TIMEBASE   ) |
		BCHP_FIELD_DATA(HDMI_RM_CONTROL, TIMEBASE,    pNewInfo->eTimeBase);

	/* HDMI_RM_RATE_RATIO (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_RATE_RATIO);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(HDMI_RM_RATE_RATIO, MAX_ERR,     0 ) |
		BCHP_FIELD_DATA(HDMI_RM_RATE_RATIO, MIN_ERR,     0 ) |
		BCHP_FIELD_DATA(HDMI_RM_RATE_RATIO, DENOMINATOR, pRmInfo->ulDenominator);

	/* HDMI_RM_SAMPLE_INC (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_SAMPLE_INC);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(HDMI_RM_SAMPLE_INC, NUMERATOR,  pRmInfo->ulNumerator) |
		BCHP_FIELD_DATA(HDMI_RM_SAMPLE_INC, SAMPLE_INC, pRmInfo->ulSampleInc);

	/* HDMI_RM_PHASE_INC (RW) */
	/* Don't care!
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_PHASE_INC);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(HDMI_RM_PHASE_INC, PHASE_INC, 0 ); */

	/* HDMI_RM_INTEGRATOR (RW) */
	/* Don't care!
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_INTEGRATOR);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(HDMI_RM_INTEGRATOR, INTEGRATOR, 4291432974); */

#if BVDC_P_SUPPORT_DVI_65NM
	pDvoCfg = &pNewInfo->stDvoCfg;

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_OFFSET);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(HDMI_RM_OFFSET, OFFSET, pRmInfo->ulOffset);

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_FORMAT);
	*pList->pulCurrent++ = (
		BCHP_FIELD_DATA(HDMI_RM_FORMAT, SHIFT,        pRmInfo->ulShift) |
		BCHP_FIELD_DATA(HDMI_RM_FORMAT, STABLE_COUNT,            10000) );

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_HDMI_TX_PHY_PLL_CFG);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_PLL_CFG, PLL_RM_DIV, pRmInfo->ulRmDiv) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_PLL_CFG, PLL_VCO_RANGE, pRmInfo->ulVcoRange) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_PLL_CFG, PLL_PX_DIV, pRmInfo->ulPxDiv) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_PLL_CFG, PLL_FEEDBACK_PRE_DIVIDER, pRmInfo->ulFeedbackPreDiv) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_PLL_CFG, PLL_INPUT_PRE_DIVIDER, pRmInfo->ulInputPreDiv);

	/* LVDS_PHY_0_SPREAD_SPECTRUM (RW) */
	ulSSReg =
		BCHP_FIELD_DATA(HDMI_TX_PHY_SPREAD_SPECTRUM, LOAD,   0      ) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_SPREAD_SPECTRUM, ENABLE, 0      ) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_SPREAD_SPECTRUM, SCALE,  ulScale) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_SPREAD_SPECTRUM, RANGE,  ulRange) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_SPREAD_SPECTRUM, HOLD,   ulHold ) |
		BCHP_FIELD_DATA(HDMI_TX_PHY_SPREAD_SPECTRUM, STEP,   ulStep );

	/****************************************************************/
	/* Require specific protocol to follow in order to turn on/off. */
	/****************************************************************/
	/* (1) Disable spread spectrum and put in new changes. */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_SPREAD_SPECTRUM);
	*pList->pulCurrent++ = ulSSReg;

	/* (2) Clock in the new changes, LOAD: 0 -> 1 */
	ulSSReg |= (
		BCHP_FIELD_DATA(HDMI_TX_PHY_SPREAD_SPECTRUM, LOAD, 1));
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_SPREAD_SPECTRUM);
	*pList->pulCurrent++ = ulSSReg;

	/* (3) Clock in the new changes, LOAD: 1 -> 0 */
	ulSSReg &= ~(
		BCHP_MASK(HDMI_TX_PHY_SPREAD_SPECTRUM, LOAD));
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_SPREAD_SPECTRUM);
	*pList->pulCurrent++ = ulSSReg;

	/* (4) reenable the Spread Spectrum after making the changes */
	ulSSReg |= (
		BCHP_FIELD_DATA(HDMI_TX_PHY_SPREAD_SPECTRUM, ENABLE, pDvoCfg->stSpreadSpectrum.bEnable));
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_SPREAD_SPECTRUM);
	*pList->pulCurrent++ = ulSSReg;

	if ((VIDEO_FORMAT_IS_ED(pNewInfo->pFmtInfo->eVideoFmt)) &&
	    (BAVC_HDMI_PixelRepetition_e2x == pNewInfo->eHdmiPixelRepetition))
	{
		ulNdivMod = 2;
		ulIcpx = 10;
		ulCn = 0;
	}
	else
	{
		ulNdivMod = 4;
		ulIcpx = 3;
		ulCn = 1;
	}

	/* HDMI_TX_PHY_HDMI_TX_PHY_CTL_1 (RW) */
	BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
						 ~(BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, ICPX) |
						   BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, LF_ORDER) |
						   BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RZ) |
						   BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CZ) |
						   BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CP) |
						   BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN) |
						   BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CN)),
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, ICPX,
							ulIcpx) |
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, LF_ORDER, 1) |
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RZ, 1) |
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CZ, 1) |
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CP, 0) |
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, RN, 0) |
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_1, CN,
							ulCn),
						 BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_1);

	/* HDMI_TX_PHY_HDMI_TX_PHY_CTL_2 (RW) */
	BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
						 ~(BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, KVCO_XS) |
						   BCHP_MASK(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, NDIV_MOD)),
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, KVCO_XS, 4) |
						 BCHP_FIELD_DATA(HDMI_TX_PHY_HDMI_TX_PHY_CTL_2, NDIV_MOD,
							ulNdivMod),
						 BCHP_HDMI_TX_PHY_HDMI_TX_PHY_CTL_2);

	/* Do not need Kick start PLL on STB */
	hDisplay->ulKickStartDelay = (0); /* vsync until kick start */

#else

	/* HDMI_RM_FORMAT (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_FORMAT);
	*pList->pulCurrent++ = (
		BCHP_FIELD_DATA(HDMI_RM_FORMAT, STABLE_COUNT,               10000) |
		BCHP_FIELD_ENUM(HDMI_RM_FORMAT, OFFSET_ONLY,             DISABLE ) |
		BCHP_FIELD_DATA(HDMI_RM_FORMAT, RDIV_CTRL,        pRmInfo->ulRDiv) |
		BCHP_FIELD_DATA(HDMI_RM_FORMAT, OP_OFFSET,    pRmInfo->ulOpOffset) |
		BCHP_FIELD_DATA(HDMI_RM_FORMAT, OP_DITHER,                      0) );

	/* HDMI_RM_VCXO_CTRL (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
	*pList->pulCurrent++ = ulVcxoCtrl =
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, LOCK,        1       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, RSEN,        15      ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, SPARE,       0       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, CLK_PATTERN, 0       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, INT_RESETB,  1       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, DIV_RST,     0       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, FILRSTB,     1       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, CLK_EDGE,    0       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, PLL_PWRDN,   0       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, BYPASS,      0       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, BIT_CTRL,    1       ) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, MDIV,        pRmInfo->ulMDiv) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, NDIV,        pRmInfo->ulNDiv) |
		BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, PDIV,        pRmInfo->ulPDiv) |
		BCHP_FIELD_ENUM(HDMI_RM_VCXO_CTRL, NEGCW,       NEGATE  ) |
		BCHP_FIELD_ENUM(HDMI_RM_VCXO_CTRL, TCOMP,       CONVERT );

	/* Kick start PLL!  This is taken from hdmi code, hw requires this in
	 * order for hdmi to start up correctly! */
	if(bDisplayRestart)
	{
		ulVcxoCtrl &= ~(BCHP_MASK(HDMI_RM_VCXO_CTRL, FILRSTB));
		ulVcxoCtrl |=  (BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, FILRSTB,    0));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
		*pList->pulCurrent++ = ulVcxoCtrl;

		ulVcxoCtrl &= ~(BCHP_MASK(HDMI_RM_VCXO_CTRL, DIV_RST));
		ulVcxoCtrl |=  (BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, DIV_RST,    0));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
		*pList->pulCurrent++ = ulVcxoCtrl;

		ulVcxoCtrl &= ~(BCHP_MASK(HDMI_RM_VCXO_CTRL, DIV_RST));
		ulVcxoCtrl |=  (BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, DIV_RST,    1));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
		*pList->pulCurrent++ = ulVcxoCtrl;

		ulVcxoCtrl &= ~(BCHP_MASK(HDMI_RM_VCXO_CTRL, INT_RESETB));
		ulVcxoCtrl |=  (BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, INT_RESETB, 1));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
		*pList->pulCurrent++ = ulVcxoCtrl;

		ulVcxoCtrl &= ~(BCHP_MASK(HDMI_RM_VCXO_CTRL, INT_RESETB));
		ulVcxoCtrl |=  (BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, INT_RESETB, 0));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
		*pList->pulCurrent++ = ulVcxoCtrl;

		ulVcxoCtrl &= ~(BCHP_MASK(HDMI_RM_VCXO_CTRL, INT_RESETB));
		ulVcxoCtrl |=  (BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, INT_RESETB, 1));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
		*pList->pulCurrent++ = ulVcxoCtrl;

		ulVcxoCtrl &= ~(BCHP_MASK(HDMI_RM_VCXO_CTRL, DIV_RST));
		ulVcxoCtrl |=  (BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, DIV_RST,    0));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
		*pList->pulCurrent++ = ulVcxoCtrl;

		ulVcxoCtrl &= ~(BCHP_MASK(HDMI_RM_VCXO_CTRL, FILRSTB));
		ulVcxoCtrl |=  (BCHP_FIELD_DATA(HDMI_RM_VCXO_CTRL, FILRSTB,    1));
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_VCXO_CTRL);
		*pList->pulCurrent++ = ulVcxoCtrl;
	}
#endif /* modularvec */

#elif BVDC_P_SUPPORT_DVPO

	BSTD_UNUSED(ulVcxoCtrl);
	BSTD_UNUSED(bDisplayRestart);

	pDvoCfg = &pNewInfo->stDvoCfg;

	/* DVPO_RM_0_CONTROL (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVPO_RM_0_CONTROL);
	*pList->pulCurrent++ =
		BCHP_FIELD_ENUM(DVPO_RM_0_CONTROL, RESET,       RESET_OFF  ) |
		BCHP_FIELD_DATA(DVPO_RM_0_CONTROL, INT_GAIN,    BVDC_P_RM_INTEGRAL_GAIN ) |
		BCHP_FIELD_DATA(DVPO_RM_0_CONTROL, DIRECT_GAIN, BVDC_P_RM_DIRECT_GAIN   ) |
		BCHP_FIELD_ENUM(DVPO_RM_0_CONTROL, DITHER,      DITHER_OFF ) |
		BCHP_FIELD_ENUM(DVPO_RM_0_CONTROL, FREE_RUN,    TIMEBASE   ) |
		BCHP_FIELD_DATA(DVPO_RM_0_CONTROL, TIMEBASE,    pNewInfo->eTimeBase);

	/* DVPO_RM_0_RATE_RATIO (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVPO_RM_0_RATE_RATIO);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVPO_RM_0_RATE_RATIO, MAX_ERR,     0 ) |
		BCHP_FIELD_DATA(DVPO_RM_0_RATE_RATIO, MIN_ERR,     0 ) |
		BCHP_FIELD_DATA(DVPO_RM_0_RATE_RATIO, DENOMINATOR, pRmInfo->ulDenominator);

	/* DVPO_RM_0_SAMPLE_INC (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVPO_RM_0_SAMPLE_INC);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVPO_RM_0_SAMPLE_INC, NUMERATOR,  pRmInfo->ulNumerator) |
		BCHP_FIELD_DATA(DVPO_RM_0_SAMPLE_INC, SAMPLE_INC, pRmInfo->ulSampleInc);

	/* DVPO_RM_0_PHASE_INC (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVPO_RM_0_PHASE_INC);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(DVPO_RM_0_PHASE_INC, PHASE_INC, 0);

	/* DVPO_RM_0_FORMAT (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVPO_RM_0_FORMAT);
	*pList->pulCurrent++ = (
		BCHP_FIELD_DATA(DVPO_RM_0_FORMAT, SHIFT,        3    ) |
		BCHP_FIELD_DATA(DVPO_RM_0_FORMAT, STABLE_COUNT, 10000) );

	/* DVPO_RM_0_OFFSET (RW) */
	BVDC_P_Vec_BuildPllCfg_isr(pList, hDisplay->hVdc->hRegister,
		&pNewInfo->stDvoCfg, pRmInfo, pRmInfo->ulRDiv, pRmInfo->ulNDiv);

	/* LVDS_PHY_0_LVDS_PLL_CTL_1 (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_LVDS_PLL_CTL_1);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, test_buffer,         0 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, kvco_xs,             5 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, kvco_xf,             0 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, lpf_bw,              0 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, LF_order,            0 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, Cn,                  0 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, Rn,                  0 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, Cp,                  1 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, Cz,                  1 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, Rz,                  1 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, lcpx,                31) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, freq_det_dis,        0 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, AVDD1p2,             1 ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_1, pll_ctrl_1_reserved, 0 );

	/* LVDS_PHY_0_LVDS_PLL_CTL_2 (RW) */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_LVDS_PLL_CTL_2);
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_2, ndiv_mode,  2) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_2, txvcom_sel, 3) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_2, txbg_adjust,4) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_2, ptap_adj,   2) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_2, ctap_adj,   2) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_2, LDO_vout,   2) |
		BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_PLL_CTL_2, biasin_en,  1);

	/* LVDS_PHY_0_SPREAD_SPECTRUM (RW) */
	ulSSReg =
		BCHP_FIELD_DATA(LVDS_PHY_0_SPREAD_SPECTRUM, LOAD,   0      ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_SPREAD_SPECTRUM, ENABLE, 0      ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_SPREAD_SPECTRUM, SCALE,  ulScale) |
		BCHP_FIELD_DATA(LVDS_PHY_0_SPREAD_SPECTRUM, RANGE,  ulRange) |
		BCHP_FIELD_DATA(LVDS_PHY_0_SPREAD_SPECTRUM, HOLD,   ulHold ) |
		BCHP_FIELD_DATA(LVDS_PHY_0_SPREAD_SPECTRUM, STEP,   ulStep );

	/****************************************************************/
	/* Require specific protocol to follow in order to turn on/off. */
	/****************************************************************/
	/* (1) Disable spread spectrum and put in new changes. */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_SPREAD_SPECTRUM);
	*pList->pulCurrent++ = ulSSReg;

	/* (2) Clock in the new changes, LOAD: 1 -> 0 */
	ulSSReg |= (
		BCHP_FIELD_DATA(LVDS_PHY_0_SPREAD_SPECTRUM, LOAD, 1));
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_SPREAD_SPECTRUM);
	*pList->pulCurrent++ = ulSSReg;

	/* (3) Clock in the new changes, LOAD: 1 -> 0 */
	ulSSReg &= ~(
		BCHP_MASK(LVDS_PHY_0_SPREAD_SPECTRUM, LOAD));
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_SPREAD_SPECTRUM);
	*pList->pulCurrent++ = ulSSReg;

	/* (4) reenable the Spread Spectrum after making the changes */
	ulSSReg |= (
		BCHP_FIELD_DATA(LVDS_PHY_0_SPREAD_SPECTRUM, ENABLE, pDvoCfg->stSpreadSpectrum.bEnable));
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_SPREAD_SPECTRUM);
	*pList->pulCurrent++ = ulSSReg;

	/* Kick start PLL!  This is taken from hdmi code, hw requires this in
	 * order for hdmi to start up correctly!  This is needed after we
	 * enable sprectrum. */
	hDisplay->ulKickStartDelay = (2); /* vsync until kick start */

#else
	BSTD_UNUSED(pList);
#endif

	BDBG_LEAVE(BVDC_P_Vec_Build_HdmiRM_isr);
	return;
}
#else
#define BVDC_P_Vec_Build_HdmiRM_isr(hDisplay, pList)          BDBG_ASSERT(0)
#endif

/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_Build_IT_isr
 *	Builds IT and Video_Enc blocks (for mode switch)
 **************************************************************************/
static void BVDC_P_Vec_Build_IT_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  uint32_t                         ulHeight,
	  uint32_t                         ulTrig0Val,
	  uint32_t                         ulTrig1Val )
{
	const uint32_t      *pRamTbl = NULL;
	const uint32_t      *pTable  = NULL;
	uint32_t             ulItConfig = 0;
	BVDC_P_DisplayInfo  *pNewInfo = &hDisplay->stNewInfo;
	uint32_t             ulVecOffset = hDisplay->lItOffset;

	/* get correct ItTable, RamTable, and ItConfig */
	pTable     = BVDC_P_GetItTable_isr(pNewInfo);
	BDBG_ASSERT (pTable);
	pRamTbl    = BVDC_P_GetRamTable_isr(pNewInfo, hDisplay->bArib480p);
	BDBG_ASSERT (pRamTbl);
	ulItConfig = BVDC_P_GetItConfig_isr(pNewInfo);
	BDBG_ASSERT (ulItConfig);

	BDBG_MSG(("Analog vec microcode - timestamp: 0x%.8x", pRamTbl[BVDC_P_RAM_TABLE_TIMESTAMP_IDX]));
	BDBG_MSG(("Analog vec microcode - checksum:  0x%.8x", pRamTbl[BVDC_P_RAM_TABLE_CHECKSUM_IDX]));

	/* to support MV mode control byte change */
	if (VIDEO_FORMAT_SUPPORTS_MACROVISION(pNewInfo->pFmtInfo->eVideoFmt) &&
	    (BVDC_P_Get_Prot_Alt_isr() != BVDC_P_PROT_ALT_DCS ))
	{
		uint32_t ulAddr46Offset;
		uint32_t ulPcl4Offset;
		uint32_t ulAddr46;
		uint32_t ulPcl4;

		/* --- Setup IT block --- */
		*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_IT_TABLE_SIZE);
		*pList->pulCurrent++ =
			BRDC_REGISTER(BCHP_PRIM_IT_ADDR_0_3 + ulVecOffset);

		/* Setup IT_ADDR_0_3 to IT_PCL_5 */
		BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
			BVDC_P_IT_TABLE_SIZE * sizeof(uint32_t));

		ulAddr46Offset =
			(BCHP_PRIM_IT_ADDR_4_6 - BCHP_PRIM_IT_ADDR_0_3)/sizeof(uint32_t);
		ulPcl4Offset =
			(BCHP_PRIM_IT_PCL_4 - BCHP_PRIM_IT_ADDR_0_3)/sizeof(uint32_t);
		ulAddr46 = pTable[ulAddr46Offset];
		ulPcl4   = pTable[ulPcl4Offset];

		/* if toggle N0 control bits for CS(MC4/5) or BP(PCL_4) */
		if(!pNewInfo->stN0Bits.bBp)
		{
			ulPcl4 &= ~(BCHP_MASK(PRIM_IT_PCL_4, PSB_AND_TERM_2));
			ulPcl4 |= BCHP_FIELD_ENUM(PRIM_IT_PCL_4, PSB_AND_TERM_2, ZERO);
			*(pList->pulCurrent + ulPcl4Offset) = ulPcl4;
		}
		if(!pNewInfo->stN0Bits.bCs && (!(VIDEO_FORMAT_IS_SECAM(pNewInfo->pFmtInfo->eVideoFmt))))
		{
			ulAddr46 &= ~(
				BCHP_MASK(PRIM_IT_ADDR_4_6, MC_4_START_ADDR) |
				BCHP_MASK(PRIM_IT_ADDR_4_6, MC_5_START_ADDR));
			ulAddr46 |= (
				BCHP_FIELD_DATA(PRIM_IT_ADDR_4_6, MC_4_START_ADDR, 0xfd) |
				BCHP_FIELD_DATA(PRIM_IT_ADDR_4_6, MC_5_START_ADDR, 0xfd));

			*(pList->pulCurrent + ulAddr46Offset) = ulAddr46;
		}

		/* move pointer after adjustment */
		pList->pulCurrent += BVDC_P_IT_TABLE_SIZE;
	}
	else
	{
		/* --- Setup IT block --- */
		*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_IT_TABLE_SIZE);
		*pList->pulCurrent++ =
			BRDC_REGISTER(BCHP_PRIM_IT_ADDR_0_3 + ulVecOffset);

		/* Setup IT_ADDR_0_3 to IT_PCL_5 */
		BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
			BVDC_P_IT_TABLE_SIZE * sizeof(uint32_t));

		/* move pointer after adjustment */
		pList->pulCurrent += BVDC_P_IT_TABLE_SIZE;
	}

	/* Setup SEC_IT_BVB_SIZE */
	/* TODO: Does DCS require work here? */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_BVB_SIZE + ulVecOffset);
	if(pNewInfo->bWidthTrimmed && VIDEO_FORMAT_IS_NTSC(pNewInfo->pFmtInfo->eVideoFmt) &&
	   (pNewInfo->eMacrovisionType < BVDC_MacrovisionType_eTest01))
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(PRIM_IT_BVB_SIZE, HORIZONTAL,712) |
			BCHP_FIELD_DATA(PRIM_IT_BVB_SIZE, VERTICAL, ulHeight);
	}
	else
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(PRIM_IT_BVB_SIZE, HORIZONTAL,pNewInfo->pFmtInfo->ulWidth) |
			BCHP_FIELD_DATA(PRIM_IT_BVB_SIZE, VERTICAL, ulHeight);
	}

	/* Setup Vec triggers */
	/* Must be in the Rul, otherwise the Reset will destroy it */
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(2);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_VEC_TRIGGER_0 + ulVecOffset);

	/* Set 1st trigger */
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_IT_VEC_TRIGGER_0, TRIGGER_VALUE, ulTrig0Val) |
		BCHP_FIELD_DATA(PRIM_IT_VEC_TRIGGER_0, ENABLE, 0);

	/* Set 2nd trigger (for interlaced modes) */
	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_IT_VEC_TRIGGER_1, TRIGGER_VALUE, ulTrig1Val) |
		BCHP_FIELD_DATA(PRIM_IT_VEC_TRIGGER_1, ENABLE, 0);

	/* Write IT_MICRO_INSTRUCTIONS 0..127 */
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_RAM_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_MICRO_INSTRUCTIONi_ARRAY_BASE + ulVecOffset);

	BKNI_Memcpy((void*)pList->pulCurrent, (void*)pRamTbl,
		BVDC_P_RAM_TABLE_SIZE * sizeof(uint32_t));
	pList->pulCurrent += BVDC_P_RAM_TABLE_SIZE;
	hDisplay->ulCurShadowDCSKeyLow = hDisplay->stNewInfo.ulDCSKeyLow;

#if (BVDC_P_SUPPORT_HDMI_OUT || BVDC_P_SUPPORT_ITU656_OUT)
	/* Setup DTRAM_CONFIG. */
	if ((pNewInfo->bEnable656) || (pNewInfo->bEnableHdmi))
	{
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTRAM_DTRAM_CONFIG);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(DTRAM_DTRAM_CONFIG, ARBITER_LATENCY, 0xb);
	}
#endif

	/* This RUL contains the reset which produce the extra interrupts
	 * tell isr to ignore these and response to trigger executed rdc.
	 * In a way this also give the vec a chance to become stablize before
	 * starting the bvn front-end modules.
	 * init polarity record */
	BVDC_P_Display_SetFormatSwitchMarker_isr(hDisplay, pList);

	/* MUST be LAST one during initialization!!! */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_TG_CONFIG + ulVecOffset);
	*pList->pulCurrent++ = ulItConfig;

	return;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_CSC_SRC_SM_isr
 *  Adds CSC and SRC blocks to RUL for a display.
 *  Bypass - CSC
 *  Secondary - SEC_CSC(SVideo/composite),
 *              SEC_CSC_CO(component)
 *              SEC_SRC, SM
 **************************************************************************/
static void BVDC_P_Vec_Build_CSC_SRC_SM_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList )
{
	const BVDC_P_DisplayCscMatrix *pCscMatrix = NULL;
	const uint32_t          *pTable = NULL;
	BVDC_P_DisplayInfo      *pNewInfo =
	                         &hDisplay->stNewInfo;
	uint32_t                 ulSrcControl =
		BVDC_P_GetSrcControl_isr(pNewInfo->eOutputColorSpace);
	BVDC_P_DisplayCscMatrix  stCscMatrix;

	const BVDC_P_Compositor_Info *pCmpInfo = &hDisplay->hCompositor->stNewInfo;
	uint8_t ucCh0 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 2);
	uint8_t ucCh1 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 1);
	uint8_t ucCh2 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 0);

	/* Setup Main CSC */
	if( pNewInfo->eOutputColorSpace != BVDC_P_Output_eUnknown )
	{
		*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CSC_TABLE_SIZE);
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_CSC_CSC_MODE + hDisplay->lCscOffset);

		/* Get the color matrix table for CSC output */
		BVDC_P_Display_GetCscTable_isr(pNewInfo, pNewInfo->eOutputColorSpace, &pCscMatrix);

		stCscMatrix = *pCscMatrix;

		/* TODO: handle user csc */
		/* Handle CSC mute */
		if (((pNewInfo->abOutputMute[BVDC_DisplayOutput_eComponent]) &&
			 (pNewInfo->eCoOutputColorSpace != BVDC_P_Output_eUnknown) &&
			 (pNewInfo->eCoOutputColorSpace != BVDC_P_Output_eHsync)) ||
			(pNewInfo->abOutputMute[BVDC_DisplayOutput_eComposite]) ||
			(pNewInfo->abOutputMute[BVDC_DisplayOutput_eSVideo]))
		{
			/* Swap ch0 and 1 of input color to match vec csc layout */
			BVDC_P_Csc_ApplyYCbCrColor_isr(&stCscMatrix.stCscCoeffs, ucCh1, ucCh0, ucCh2);
		}

		/* Setup CSC for Dac outputs */
		BVDC_P_Vec_Build_CSC_isr(&stCscMatrix, pList);

#if (BVDC_P_SUPPORT_VEC_DITHER)
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DITHER_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_CSC_DITHER_CONTROL + hDisplay->lCscOffset);
	BVDC_P_Vec_Build_Dither_isr(hDisplay, &hDisplay->stCscDither, pList);
#endif

		/* Setup SRC */
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_SRC_SRC_CONTROL + hDisplay->lSrcOffset);
		*pList->pulCurrent++ = ulSrcControl;
	}

#if (BVDC_P_SUPPORT_COMPONENT_ONLY)
	/* Setup Component-only CSC */
	if((pNewInfo->eCoOutputColorSpace != BVDC_P_Output_eUnknown) &&
	   (pNewInfo->eCoOutputColorSpace != BVDC_P_Output_eHsync))
	{
		*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CSC_TABLE_SIZE);
		*pList->pulCurrent++ = BRDC_REGISTER(BVDC_P_VEC_CSC_CO_CSC_MODE);

		/* Get the color matrix table for CSC output */
		BVDC_P_Display_GetCscTable_isr(pNewInfo, pNewInfo->eCoOutputColorSpace, &pCscMatrix);

		stCscMatrix = *pCscMatrix;

		/* TODO: handle user csc */
		/* Handle CSC mute */
		if (pNewInfo->abOutputMute[BVDC_DisplayOutput_eComponent])
		{
			/* Swap ch0 and 1 of input color to match vec csc layout */
			BVDC_P_Csc_ApplyYCbCrColor_isr(&stCscMatrix.stCscCoeffs, ucCh1, ucCh0, ucCh2);
		}

		/* Setup CSC for Dac outputs */
		BVDC_P_Vec_Build_CSC_isr(&stCscMatrix, pList);

		/* Setup SRC_CO */
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BVDC_P_VEC_SRC_CO_SRC_CONTROL);
		*pList->pulCurrent++ = ulSrcControl;
	}
#endif

	/* --- Setup SM --- */
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_SM_TABLE_SIZE);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_SM_ENVELOPE_GENERATOR + hDisplay->lSmOffset);

	/* get the correct sm table */
	pTable = BVDC_P_GetSmTable_isr(pNewInfo, pNewInfo->eOutputColorSpace);

	/* SM_ENVELOPE_GENERATOR -> SM_COMP_CNTRL */
	BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
		BVDC_P_SM_TABLE_SIZE * sizeof(uint32_t));
	pList->pulCurrent += BVDC_P_SM_TABLE_SIZE;

	/* Setup SM_COMP_CONFIG */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_SM_COMP_CONFIG + hDisplay->lSmOffset);

	if(VIDEO_FORMAT_IS_NTSC(pNewInfo->pFmtInfo->eVideoFmt))
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, U_LINE_SEL, 0) |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, COMPOSITE_CLAMP_SEL, 1) |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, V_LINE_SEL, 0) |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, U_FIXED_LINE, 0) |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, COMPOSITE_OFFSET, 0);
	}
#if BVDC_P_SUPPORT_VEC_SECAM
	else if(VIDEO_FORMAT_IS_SECAM(pNewInfo->pFmtInfo->eVideoFmt))
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, U_LINE_SEL, 0)          |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, COMPOSITE_CLAMP_SEL, 1) |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, V_LINE_SEL,          1) |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, U_FIXED_LINE, 0)        |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, COMPOSITE_OFFSET, 0xd0);
	}
#endif
	else /* PAL */
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, U_LINE_SEL, 0)          |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, COMPOSITE_CLAMP_SEL, 1) |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, V_LINE_SEL,
			((pNewInfo->eOutputColorSpace == BVDC_P_Output_eYUV)   ||
			 (pNewInfo->eOutputColorSpace == BVDC_P_Output_eYUV_M) ||
			 (pNewInfo->eOutputColorSpace == BVDC_P_Output_eYUV_N) ||
			 (pNewInfo->eOutputColorSpace == BVDC_P_Output_eYUV_NC)))    |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, U_FIXED_LINE, 0)        |
			BCHP_FIELD_DATA(PRIM_SM_COMP_CONFIG, COMPOSITE_OFFSET, 0);
	}

#if BVDC_P_SUPPORT_VEC_SECAM
	/* Setup SM_FM_CONTROL */
	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_SM_FM_CONTROL + hDisplay->lSmOffset);
	if(VIDEO_FORMAT_IS_SECAM(pNewInfo->pFmtInfo->eVideoFmt))
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(PRIM_SM_FM_CONTROL, FINE_LUMA_DELAY,  0) |
			BCHP_FIELD_DATA(PRIM_SM_FM_CONTROL, GROSS_LUMA_DELAY, 0x18) |
			BCHP_FIELD_DATA(PRIM_SM_FM_CONTROL, FINE_SC_DELAY,    0x3) |
			BCHP_FIELD_DATA(PRIM_SM_FM_CONTROL, GROSS_SC_DELAY,   0x8) |
			BCHP_FIELD_ENUM(PRIM_SM_FM_CONTROL, ENABLE, ON);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_SM_FM_FMAMP + hDisplay->lSmOffset);
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(PRIM_SM_FM_FMAMP, LOWER_LIMIT,  64) |
			BCHP_FIELD_DATA(PRIM_SM_FM_FMAMP, UPPER_LIMIT, 192) |
			BCHP_FIELD_DATA(PRIM_SM_FM_FMAMP, SLOPE_ADJUST,  3) |
			BCHP_FIELD_DATA(PRIM_SM_FM_FMAMP, SCALE, 92);
	}
	else
	{
		*pList->pulCurrent++ =
			BCHP_FIELD_DATA(PRIM_SM_FM_CONTROL, GROSS_LUMA_DELAY, 0x18) |
			BCHP_FIELD_DATA(PRIM_SM_FM_CONTROL, GROSS_SC_DELAY,   0x18);
	}
#endif
	return;
}


/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_Build_VF_MISC_isr
 *	Builds either: SEC_VF_MISC, SEC_VF_CO_MISC or PRIM_VF_MISC register
 *  Required for Videoformat or colorspace change. Also required EVERY
 *  VSYNC, if a certain hardware bug exists.
 **************************************************************************/
static uint32_t BVDC_P_Vec_Get_VF_MISC_isr
	( BVDC_P_DisplayInfo                  *pNewInfo,
	  bool                                bArib480p,
	  const uint32_t                      *pVfValuesTable,
	  const BVDC_P_Display_ShaperSettings *pShaperSettings
)
{
	uint32_t             ulVfMiscRegIdx, ulVfMiscRegValue;

	ulVfMiscRegIdx =
		(BCHP_PRIM_VF_MISC - BCHP_PRIM_VF_FORMAT_ADDER) / sizeof(uint32_t);
	ulVfMiscRegValue =
		pVfValuesTable[ulVfMiscRegIdx] &
			~BCHP_MASK(PRIM_VF_MISC, BVB_SAV_REMOVE);
	ulVfMiscRegValue |=
		BCHP_FIELD_DATA(
			PRIM_VF_MISC, BVB_SAV_REMOVE, pShaperSettings->ulSavRemove);

	/* SECAM is a special case. This bitfield value does not follow colorspace
	 * like other formats. */
	if (VIDEO_FORMAT_IS_SECAM(pNewInfo->pFmtInfo->eVideoFmt))
	{
		ulVfMiscRegValue &=
			~BCHP_FIELD_DATA(PRIM_VF_MISC, BVB_LINE_REMOVE_TOP, 1);
	}

	/* PAL line 23 is reserved for WSS vbi data; Cx and beyond can drop the
     * 1st active video line from BVN; */
	else if (VIDEO_FORMAT_IS_625_LINES(pNewInfo->pFmtInfo->eVideoFmt))
	{
		ulVfMiscRegValue |=
			BCHP_FIELD_DATA(PRIM_VF_MISC, BVB_LINE_REMOVE_TOP, 1);
	}

	/* Also for PAL-N. */
	else if ((pNewInfo->pFmtInfo->eVideoFmt == BFMT_VideoFmt_ePAL_N) &&
			 ((pNewInfo->eMacrovisionType ==
				BVDC_MacrovisionType_eNoProtection) ||
			 (pNewInfo->eMacrovisionType ==
				BVDC_MacrovisionType_eTest01) ||
			 (pNewInfo->eMacrovisionType ==
				BVDC_MacrovisionType_eTest02)) )
	{
		ulVfMiscRegValue |=
			BCHP_FIELD_DATA(PRIM_VF_MISC, BVB_LINE_REMOVE_TOP, 1);
	}
	/* Also special case for ARIB NTSC */
	else if(VIDEO_FORMAT_IS_NTSC(pNewInfo->pFmtInfo->eVideoFmt) &&
		    (true  == bArib480p) &&
            (false == pNewInfo->bWidthTrimmed))
	{
		ulVfMiscRegValue |= (
			BCHP_FIELD_DATA(PRIM_VF_MISC, BVB_LINE_REMOVE_TOP   , 1) |
			BCHP_FIELD_DATA(PRIM_VF_MISC, BVB_LINE_REMOVE_BOTTOM, 1) );
	}

	return ulVfMiscRegValue;
}


/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_Build_VF_isr
 *	Builds either: SEC_VF, SEC_VF_CO or PRIM_VF
 *  Required for Videoformat or colorspace change
 **************************************************************************/
static void BVDC_P_Vec_Build_VF_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  uint32_t                         ulFormatAdder,
	  BVDC_P_Output                    eOutputColorSpace )
{
	BVDC_P_Display_ShaperSettings stShaperSettings;
	uint32_t             ulNumRegs;
	uint32_t             ulVfMiscRegIdx, ulVfMiscRegValue;
	uint32_t             ulVfSTRegIdx,   ulVfSTRegValue;
	uint32_t            *pulFmtAdder;
	uint32_t            *pulPosSync;
	uint32_t             ulVal;
	const uint32_t     **apVfFilter;
	const uint32_t      *pTable=0;
	BVDC_P_DisplayInfo  *pNewInfo = &hDisplay->stNewInfo;
	uint32_t             i=0;
	uint32_t             work[BVDC_P_VF_TABLE_SIZE];

	/* Channel band-limiting filter
	                    Ch0    Ch1    Ch2
	YQI (NTSC,NTSC_J)   6.0    0.6    1.3
	YUV (M,N,NC)        4.2    1.3    1.3
	YUV (B,B1,D1,G,H)   5.0    1.3    1.3
	YUV (I)             5.5    1.3    1.3
	YUV (D,K,K1)        6.0    1.3    1.3
	RGB                 6.75   6.75   6.75
	YPrPb               6.75   3.375  3.375 */

	/* get the correct channel filters */
	for (i=0; i < BVDC_P_VEC_CH_NUM; i++)
	{
		if(BVDC_P_DISP_IS_COMPONENT(eOutputColorSpace))
		{
			BVDC_P_GetChFilters_isr(pNewInfo, eOutputColorSpace, &hDisplay->apVfFilterCo[0], &hDisplay->apVfFilterCo[1], &hDisplay->apVfFilterCo[2]);
			if(pNewInfo->abUserVfFilterCo[i])
			{
				hDisplay->apVfFilterCo[i] = (const uint32_t *)pNewInfo->aaulUserVfFilterCo[i];
			}
			apVfFilter = hDisplay->apVfFilterCo;
		}
		else
		{
			BVDC_P_GetChFilters_isr(pNewInfo, eOutputColorSpace, &hDisplay->apVfFilterCvbs[0], &hDisplay->apVfFilterCvbs[1], &hDisplay->apVfFilterCvbs[2]);
			if(pNewInfo->abUserVfFilterCvbs[i])
			{
				hDisplay->apVfFilterCvbs[i] = (const uint32_t *)pNewInfo->aaulUserVfFilterCvbs[i];
			}
			apVfFilter = hDisplay->apVfFilterCvbs;
		}
	}

	/* get the correct vf table */
	BVDC_P_FillVfTable_isr(
		pNewInfo, eOutputColorSpace, work, NULL, &stShaperSettings);
	pTable = &work[0];

	/* prepare for setting vec BVB left cut */
#if !BVDC_P_PR14712_FIXED
	stShaperSettings.ulSavRemove = 0;
#endif

	/* Prepare for setting *_VF_MISC and SEC_VF_CO_MISC, if it exists. */
	ulVfMiscRegIdx =
		(BCHP_PRIM_VF_MISC - BCHP_PRIM_VF_FORMAT_ADDER) / sizeof(uint32_t);
	ulVfMiscRegValue =
		BVDC_P_Vec_Get_VF_MISC_isr (
		    pNewInfo, hDisplay->bArib480p, pTable, &stShaperSettings);

	/* Prepare for settng the _VF_SYNC_TRANS_0 register */
	ulVfSTRegIdx =
		(BCHP_PRIM_VF_SYNC_TRANS_0 - BCHP_PRIM_VF_FORMAT_ADDER) /
			sizeof(uint32_t);
	ulVfSTRegValue =
		BVDC_P_Display_Modify_SYNC_TRANS_0_isr (pNewInfo, pTable[ulVfSTRegIdx]);

#if BVDC_P_SUPPORT_COMPONENT_ONLY
	/* When using Secondary Vec, must make sure Vec consumes
	 * pixels at the same rate for both SEC_VF and SEC_VF_CO */
	if (BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
	{
		uint32_t ulOtherMiscReg =
			(ulFormatAdder == BVDC_P_VEC_VF_CO_FORMAT_ADDER) ?
				BVDC_P_VEC_VF_MISC : BVDC_P_VEC_VF_CO_MISC;

		if((ulOtherMiscReg == BVDC_P_VEC_VF_CO_MISC) ||
		   (pNewInfo->eOutputColorSpace != BVDC_P_Output_eHsync))
		{
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(ulOtherMiscReg);
			*pList->pulCurrent++ = ulVfMiscRegValue;
		}
	}
#endif

	ulNumRegs = (BCHP_PRIM_VF_SHAPER - BCHP_PRIM_VF_FORMAT_ADDER)/4+1 ;

	/* Setup VF_FORMAT_ADDER -> VF_BVB_STATUS */
	*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(ulNumRegs);
	*pList->pulCurrent++ = BRDC_REGISTER(ulFormatAdder);

	/* Save RUL table entry for VF_FORMAT_ADDER. It will be modified later. */
	pulFmtAdder = pList->pulCurrent;

	/* Save RUL table entry for VF_POS_SYNC_VALUES. It will be modified
	 * later. */
	pulPosSync =
		pList->pulCurrent +
			(BCHP_PRIM_VF_POS_SYNC_VALUES - BCHP_PRIM_VF_FORMAT_ADDER)/4;

	/* Setup VF_FORMAT_ADDER -> SEC_VF_SYNC_TRANS_1 */
	BKNI_Memcpy((void*)pList->pulCurrent, (void*)(pTable),
		(BVDC_P_VF_TABLE_SIZE) * sizeof(uint32_t));
	*(pList->pulCurrent + ulVfMiscRegIdx) = ulVfMiscRegValue;     /* override */
	*(pList->pulCurrent + ulVfSTRegIdx) = ulVfSTRegValue;   /* posible ovride */
	pList->pulCurrent += BVDC_P_VF_TABLE_SIZE;

	/* Setup Channel VF filters */
	for (i=0; i < BVDC_P_VEC_CH_NUM; i++)
	{
		BKNI_Memcpy((void*)pList->pulCurrent, apVfFilter[i],
			BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
		pList->pulCurrent += BVDC_P_CHROMA_TABLE_SIZE;
	}

	/* BVB_STATUS */
	*pList->pulCurrent++ = 0xff;

	*pList->pulCurrent++ =
		BCHP_FIELD_DATA(PRIM_VF_SHAPER, SAV_REPLICATE, stShaperSettings.ulSavReplicate) |
		BCHP_FIELD_DATA(PRIM_VF_SHAPER, EAV_PREDICT,   stShaperSettings.ulEavPredict) |
		BCHP_FIELD_DATA(PRIM_VF_SHAPER, THRESHOLD,     BVDC_P_VF_THRESH)    |
		BCHP_FIELD_DATA(PRIM_VF_SHAPER, CONTROL,       BVDC_P_VF_ENABLE);

		/* Programming note: ulNumRegs values have been placed in the RUL. The
		 * last BRDC_OP_IMMS_TO_REGS is fulfilled.
		 */

	/* FORMAT_ADDER */
	ulVal = pTable[0];

	/* SD with RGB? */
	if((VIDEO_FORMAT_IS_SD(pNewInfo->pFmtInfo->eVideoFmt)) &&
		(eOutputColorSpace == BVDC_P_Output_eSDRGB))
	{
		ulVal = BVDC_P_GetFmtAdderValue_isr(pNewInfo);
	}

	if(!pNewInfo->stN0Bits.bPsAgc)
	{
		ulVal &= ~BCHP_MASK(PRIM_VF_FORMAT_ADDER, SECOND_NEGATIVE_SYNC);
	}

#if DCS_SUPPORT
	if (hDisplay->ulNewShadowDCSKeyLow != 0x0)
	{
		ulVal &= ~BCHP_MASK       (PRIM_VF_FORMAT_ADDER, C0_POSITIVESYNC);
		ulVal |=  BCHP_FIELD_DATA (PRIM_VF_FORMAT_ADDER, C0_POSITIVESYNC, 1);
	}
#endif

	/* Alternate sync arrangement? */
	if (!(hDisplay->bModifiedSync) &&
		(pNewInfo->eOutputColorSpace == BVDC_P_Output_eHDYPrPb))
	{
		switch (pNewInfo->pFmtInfo->eVideoFmt)
		{
		case BFMT_VideoFmt_e720p:
		case BFMT_VideoFmt_e720p_24Hz:
		case BFMT_VideoFmt_e720p_25Hz:
		case BFMT_VideoFmt_e720p_30Hz:
		case BFMT_VideoFmt_e720p_50Hz:
		case BFMT_VideoFmt_e1080i:
		case BFMT_VideoFmt_e1080i_50Hz:
		case BFMT_VideoFmt_e1250i_50Hz:
		case BFMT_VideoFmt_e1080p:
		case BFMT_VideoFmt_e1080p_24Hz:
		case BFMT_VideoFmt_e1080p_25Hz:
		case BFMT_VideoFmt_e1080p_30Hz:
		case BFMT_VideoFmt_e1080p_50Hz:
			ulVal &= ~(
				BCHP_MASK       (PRIM_VF_FORMAT_ADDER,    C2_POSITIVESYNC) |
				BCHP_MASK       (PRIM_VF_FORMAT_ADDER,    C1_POSITIVESYNC) |
				BCHP_MASK       (PRIM_VF_FORMAT_ADDER, ADD_SYNC_TO_OFFSET) |
				BCHP_MASK       (PRIM_VF_FORMAT_ADDER,             OFFSET) );
			ulVal |= (
				BCHP_FIELD_DATA (PRIM_VF_FORMAT_ADDER,    C2_POSITIVESYNC,     0) |
				BCHP_FIELD_DATA (PRIM_VF_FORMAT_ADDER,    C1_POSITIVESYNC,     0) |
				BCHP_FIELD_DATA (PRIM_VF_FORMAT_ADDER, ADD_SYNC_TO_OFFSET,     0) |
				BCHP_FIELD_DATA (PRIM_VF_FORMAT_ADDER,             OFFSET, 0x1EE));
			break;
		default:
			break;
		}
	}

	/* Edit the VF_FORMAT_ADDER RUL entry */
	*pulFmtAdder = ulVal;

	/* NTSC/PAL could have sync reduction */
	if((VIDEO_FORMAT_IS_SD(pNewInfo->pFmtInfo->eVideoFmt)) &&
		(eOutputColorSpace != BVDC_P_Output_eUnknown))
	{
		/* Note: for RGB with external sync,
		 * need to remove sync signals from G channel; */
	    bool bDacOutput_Green_NoSync =
			(BVDC_P_Display_FindDac_isr(
				hDisplay, BVDC_DacOutput_eGreen_NoSync));

		ulVal = (
#if (BVDC_P_SUPPORT_COMPONENT_ONLY)
			(eOutputColorSpace == pNewInfo->eCoOutputColorSpace) &&
#endif
			BVDC_P_Display_FindDac_isr(hDisplay, BVDC_DacOutput_eHsync)) ?
			0 :
			BVDC_P_Macrovision_GetNegSyncValue_isr(
				pNewInfo, eOutputColorSpace, bDacOutput_Green_NoSync);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
#if BVDC_P_SUPPORT_COMPONENT_ONLY
		*pList->pulCurrent++ = (eOutputColorSpace == pNewInfo->eCoOutputColorSpace)
			? BRDC_REGISTER(BVDC_P_VEC_VF_CO_NEG_SYNC_VALUES)
			: BRDC_REGISTER(BCHP_PRIM_VF_NEG_SYNC_VALUES + hDisplay->lVfOffset);
#else
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_VF_NEG_SYNC_VALUES);
#endif
		*pList->pulCurrent++ = ulVal;
	}

#if DCS_SUPPORT /** { **/
	/* A new VF table was loaded, so need to override
	 * Pos. sync. values for DCS. */
	if (hDisplay->ulNewShadowDCSKeyLow != 0x0)
	{
		uint32_t ulVal;
		BVDC_P_DisplayInfo *pDispInfo = &hDisplay->stNewInfo;
#if (BVDC_P_SUPPORT_COMPONENT_ONLY) /** { **/
		if (ulFormatAdder == BVDC_P_VEC_VF_CO_FORMAT_ADDER)
		{
			ulVal =
				BVDC_P_GetCoPosSyncValueDcs_isr(
					pDispInfo->pFmtInfo->eVideoFmt,
					pDispInfo->eCoOutputColorSpace,
					hDisplay->ulNewShadowDCSKeyLow);
		}
		else
		{
			ulVal =
				BVDC_P_GetPosSyncValueDcs_isr(
					pDispInfo->pFmtInfo->eVideoFmt,
					pDispInfo->eCoOutputColorSpace,
					hDisplay->ulNewShadowDCSKeyLow);
		}
#else /** } BVDC_P_SUPPORT_COMPONENT_ONLY { **/
		ulVal =
			BVDC_P_GetPosSyncValueDcs_isr(
				pDispInfo->pFmtInfo->eVideoFmt,
				pDispInfo->eCoOutputColorSpace,
				hDisplay->ulNewShadowDCSKeyLow);
#endif /** } BVDC_P_SUPPORT_COMPONENT_ONLY **/

		/*
		 * Programming note: not a mistake. Each of the above three function
		 * calls needs the same value pDispInfo->eCoOutputColorSpace.
		 */

		/* Update VEC path chosen above */
		*pulPosSync = ulVal;
	}
#endif /** } DCS_SUPPORT **/

	return;
}
 /*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_MISC_isr
 *
 *  Adds Vec MISC and VIDEO_ENC block to RUL
 **************************************************************************/
static void BVDC_P_Vec_Build_MISC_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList )
{
	BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
	BVDC_P_DisplayInfo  *pNewInfo = &hDisplay->stNewInfo;

	/* Add In/Out Muxes update to RUL. */
	if ((pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] !=
	     pCurInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent]) ||
		(pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi] !=
		 pCurInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi]))
	{

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ =
			BRDC_REGISTER(BCHP_VIDEO_ENC_PRIM_SRC_SEL + hDisplay->eId * sizeof(uint32_t));

		if (pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent])
		{
			*pList->pulCurrent++ = BCHP_FIELD_ENUM(VIDEO_ENC_PRIM_SRC_SEL, SOURCE, DECIM_SOURCE);
		}
		else
		{
			*pList->pulCurrent++ =
				BCHP_FIELD_DATA(VIDEO_ENC_PRIM_SRC_SEL, SOURCE, hDisplay->hCompositor->eId);
		}

		/* Enable mpaa if either component or hdmi mpaa is selected */
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_DECIM_DECIM_CONTROL);

		if (pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] ||
		    pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi])
		{
			/* TODO: For vdec source, passthr_count is equal to num_vbi
			         No vdec in 7400B0 */
			/* DECIMATE_RATIO: The MPAA standard specifies the resolution to be
			 * not greater than 520000 pixels in a frame */
			if((pNewInfo->pFmtInfo->ulWidth * pNewInfo->pFmtInfo->ulHeight)/2 < 520000)
			{
				*pList->pulCurrent++ =
					BCHP_FIELD_DATA(VIDEO_ENC_DECIM_DECIM_CONTROL, PASSTHROUGH_COUNT, 0) |
					BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_DECIM_CONTROL, DECIMATE_RATIO, BY2)  |
					BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_DECIM_CONTROL, DECIMATE_SAMPLING_EN, ON) |
					BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_DECIM_CONTROL, DECIMATE_FILTER_EN, ON);
			}
			else
			{
				*pList->pulCurrent++ =
					BCHP_FIELD_DATA(VIDEO_ENC_DECIM_DECIM_CONTROL, PASSTHROUGH_COUNT, 0) |
					BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_DECIM_CONTROL, DECIMATE_RATIO, BY4)  |
					BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_DECIM_CONTROL, DECIMATE_SAMPLING_EN, ON) |
					BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_DECIM_CONTROL, DECIMATE_FILTER_EN, ON);
			}

			/* Decim controls Mpaa for all Vec outputs */
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_DECIM_SRC_SEL);
			*pList->pulCurrent++ =
				BCHP_FIELD_DATA(VIDEO_ENC_DECIM_SRC_SEL, SOURCE, hDisplay->hCompositor->eId);
		}
		else
		{
			*pList->pulCurrent++ =
				BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_DECIM_CONTROL, DECIMATE_SAMPLING_EN, OFF) |
				BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_DECIM_CONTROL, DECIMATE_FILTER_EN, OFF);

			/* Decim controls Mpaa for all Vec outputs */
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_DECIM_SRC_SEL);
			*pList->pulCurrent++ = BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_SRC_SEL, SOURCE, DISABLE);
		}
	}

#if BVDC_P_SUPPORT_ITU656_OUT
	/* MISC_ITU656_IN_MUX: 656 output */
	if(pNewInfo->bEnable656 != pCurInfo->bEnable656)
	{
		BDBG_MSG(("ITUR-656 is %s on Display[%d].",
			pNewInfo->bEnable656 ? "Enable" : "Disable", hDisplay->eId));

		/* MISC_ITU656_IN_MUX: Default for Vec misc regs */
		BVDC_P_DISP_GET_REG_DATA(MISC_ITU656_MASTER_SEL) &= ~(
			BCHP_MASK(MISC_ITU656_MASTER_SEL, FREERUN     ) |
			BCHP_MASK(MISC_ITU656_MASTER_SEL, SELECT     ));

		if((BVDC_P_eVecPrimary   == pNewInfo->e656Vecpath) ||
		   (BVDC_P_eVecSecondary == pNewInfo->e656Vecpath) ||
		   (BVDC_P_eVecTertiary  == pNewInfo->e656Vecpath))
		{
			BVDC_P_DISP_GET_REG_DATA(MISC_ITU656_MASTER_SEL) |=  (
				BCHP_FIELD_DATA(MISC_ITU656_MASTER_SEL, FREERUN, 0) |
				BCHP_FIELD_DATA(MISC_ITU656_MASTER_SEL, SELECT, pNewInfo->e656Vecpath));
		}
		BVDC_P_DISP_WRITE_TO_RUL(MISC_ITU656_MASTER_SEL, pList->pulCurrent);

		if (!pNewInfo->bEnable656)
		{
			/* Decim controls Mpaa for all Vec outputs */
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_ITU656_SRC_SEL);
			*pList->pulCurrent++ = BCHP_FIELD_ENUM(VIDEO_ENC_ITU656_SRC_SEL, SOURCE, DISABLE);
		}
	}
#endif

#if BVDC_P_SUPPORT_DVI_OUT
	/* MISC_DVI_IN_MUX: Hdmi or Dvi output */
	if((pNewInfo->bSyncOnly != pCurInfo->bSyncOnly) ||
	   (pNewInfo->bEnableHdmi != pCurInfo->bEnableHdmi) ||
	   (pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi] !=
	    pCurInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi]))
	{
		BDBG_MSG(("HDMI is %s on Display[%d].",
			pNewInfo->bEnableHdmi ? "Enable" : "Disable", hDisplay->eId));

		/* MISC_DVI_IN_MUX: Default for Vec misc regs */
		BVDC_P_DISP_GET_REG_DATA(MISC_DVI_MASTER_SEL) &= ~(
			BCHP_MASK(MISC_DVI_MASTER_SEL, FREERUN     ) |
			BCHP_MASK(MISC_DVI_MASTER_SEL, SELECT     ));

		BVDC_P_DISP_GET_REG_DATA(MISC_DVI_MASTER_SEL) |=  (
			BCHP_FIELD_DATA(MISC_DVI_MASTER_SEL, SELECT, (pNewInfo->eHdmiVecpath)) |
			BCHP_FIELD_DATA(MISC_DVI_MASTER_SEL, FREERUN,
				(BVDC_DisplayTg_eDviDtg == hDisplay->eMasterTg)));

		BVDC_P_DISP_WRITE_TO_RUL(MISC_DVI_MASTER_SEL, pList->pulCurrent);

		/* Select DVI source */
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_DVI_SRC_SEL);

		if((!pNewInfo->bEnableHdmi) ||
		   ( pNewInfo->bSyncOnly))
		{
			*pList->pulCurrent++ =
				BCHP_FIELD_ENUM(VIDEO_ENC_DVI_SRC_SEL, SOURCE, DISABLE);
		}
		else
		{
			if (pNewInfo->aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi])
			{
				*pList->pulCurrent++ =
					BCHP_FIELD_ENUM(VIDEO_ENC_DVI_SRC_SEL, SOURCE, DECIM_SOURCE);
			}
			else
			{
				*pList->pulCurrent++ =
					BCHP_FIELD_DATA(VIDEO_ENC_DVI_SRC_SEL, SOURCE, hDisplay->hCompositor->eId);
			}
		}
	}
#endif

	BVDC_P_DISP_WRITE_TO_RUL(MISC_OUT_MUX,        pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_OUT_CTRL,       pList->pulCurrent);
#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_5)
	BVDC_P_DISP_WRITE_TO_RUL(MISC_DAC1_CTRL_REG,  pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_DAC2_CTRL_REG,  pList->pulCurrent);
#if (BVDC_P_MAX_DACS > 2)
	BVDC_P_DISP_WRITE_TO_RUL(MISC_DAC3_CTRL_REG,  pList->pulCurrent);
#endif
	BVDC_P_DISP_WRITE_TO_RUL(MISC_DAC_BG_CTRL_REG,pList->pulCurrent);
#elif (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_3)
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC01_CTRL_REG,  pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC02_CTRL_REG,  pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC03_CTRL_REG,  pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC0_BG_CTRL_REG,pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC11_CTRL_REG,  pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC12_CTRL_REG,  pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC13_CTRL_REG,  pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC1_BG_CTRL_REG,pList->pulCurrent);
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_2)
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC1_CTRL_REG,	pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC2_CTRL_REG,	pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC3_CTRL_REG,	pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_TDAC_BG_CTRL_REG,pList->pulCurrent);
#endif

#if (BVDC_P_SUPPORT_QDAC_VER == BVDC_P_SUPPORT_QDAC_VER_1)
	BVDC_P_DISP_WRITE_TO_RUL(MISC_QDAC1_CTRL_REG, pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_QDAC2_CTRL_REG, pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_QDAC3_CTRL_REG, pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_QDAC4_CTRL_REG, pList->pulCurrent);
	BVDC_P_DISP_WRITE_TO_RUL(MISC_QDAC_BG_CTRL_REG,pList->pulCurrent);
#endif

	return ;
}


#if (BVDC_P_SUPPORT_MODULAR_VEC == 1)
/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_BuildPSync_isr
 *
 * Read the PSYNC from IT's status and program to PSYNC.
 **************************************************************************/
static void BVDC_P_Vec_BuildPSync_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  uint32_t                         ulVfRegAddr )
{
	/* PSYNC needs to match with the internal MC field polarity.  For the case of
	 * PAL it's crucial because depend on TOP/BOT it will removed the extra line
	 * for WSS, but since interrupt lacency can cause the RUL to programm the
	 * PSYNC with old polarity it will cause the VEC to incorrect removed the
	 * line and resulted in stall.
	 *
	 * Solution here is for RDC to read the polarity status and program it on
	 * the fly.  So even with T/B rul the PSYNC will get programmed correctly. */

	/* Extract out current field id: x {0, 1} */
	*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_VEC_CTRL_STAT + hDisplay->lItOffset);

	*pList->pulCurrent++ = BRDC_OP_VAR_AND_IMM_TO_VAR(BRDC_Variable_0, BRDC_Variable_1);
	*pList->pulCurrent++ = BCHP_PRIM_IT_VEC_CTRL_STAT_COMP_ODD_EVEN_MASK;

	*pList->pulCurrent++ = BRDC_OP_VAR_ROR_TO_VAR(
		BRDC_Variable_1, BCHP_PRIM_IT_VEC_CTRL_STAT_COMP_ODD_EVEN_SHIFT, BRDC_Variable_2);

	/* psync = ( x << 1) | ( x ^ 1 ) */
	/* BRDC_Variable_3 = BRDC_Variable_2 ror 31; */
	*pList->pulCurrent++ = BRDC_OP_VAR_ROR_TO_VAR(
		BRDC_Variable_2, 31, BRDC_Variable_3);

	/* BRDC_Variable_1 = BRDC_Variable_2 xor 1 */
	*pList->pulCurrent++ = BRDC_OP_VAR_XOR_IMM_TO_VAR(
		BRDC_Variable_2, BRDC_Variable_1);
	*pList->pulCurrent++ = 1;

	/* BRDC_Variable_2 = BRDC_Variable_3 or BRDC_Variable_1 */
	*pList->pulCurrent++ = BRDC_OP_VAR_OR_VAR_TO_VAR(
		BRDC_Variable_3, BRDC_Variable_1, BRDC_Variable_2);

	*pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
	*pList->pulCurrent++ = BRDC_REGISTER(ulVfRegAddr);

	return;
}
#endif

/*************************************************************************
 *  {secret}
 *	BVDC_P_Vec_BuildVsync_isr
 *
 *	Adds Vec settings required for Vsync updates:
 *	VF_AGC_VALUES (for macrovision)
 *	IT_BVB_CONFIG[PSYNC] or DTG_BVB[PSYNC]
 **************************************************************************/
static void BVDC_P_Vec_BuildVsync_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldPolarity )
{
	uint32_t             ulPicSync;
	uint32_t             ulVbiCtrl;
	BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
	BVDC_P_DisplayInfo  *pNewInfo = &hDisplay->stNewInfo;

    /* Update VSYNC counter */
    hDisplay->ulVsyncCnt++;

	/* Compute picture sync */
#if (BVDC_P_SUPPORT_MODULAR_VEC == 1)
	if(BAVC_Polarity_eTopField == eFieldPolarity)
	{
		ulPicSync =
			BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, PSYNC, START_TOP) |
			BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, BVB_DATA , DISP);
	}
	else if(BAVC_Polarity_eBotField == eFieldPolarity)
	{
		ulPicSync =
			BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, PSYNC, START_BOT) |
			BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, BVB_DATA , DISP);
	}
	else
	{
		ulPicSync =
			BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, PSYNC, START_PROG) |
			BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, BVB_DATA , DISP);
	}
#else
	ulPicSync = 0;
#endif

	/* If it has automode then we'd use the automod instead. */
#ifdef BCHP_PRIM_VF_BVB_CONFIG_PSYNC_SEL_AUTO
	ulPicSync |=
		BCHP_FIELD_ENUM(PRIM_VF_BVB_CONFIG, PSYNC_SEL, AUTO);
#endif

	/* program vbi enc control */
	ulVbiCtrl =
		BREG_Read32_isr(
			hDisplay->hVdc->hRegister, hDisplay->ulScratchVbiEncControl);
	if(hDisplay->ulPrevVbiEncControlValue != ulVbiCtrl)
	{
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ =
			BRDC_REGISTER(BCHP_VBI_ENC_PRIM_Control + hDisplay->ulVbiEncOffset);
		*pList->pulCurrent++ = ulVbiCtrl;
		hDisplay->ulPrevVbiEncControlValue = ulVbiCtrl;
	}
#if BVDC_P_SUPPORT_VBI_ENC_656
	ulVbiCtrl =
		BREG_Read32_isr(
			hDisplay->hVdc->hRegister, hDisplay->ulScratchVbiEncControl_656);
	if(hDisplay->ulPrevVbiEncControlValue_656 != ulVbiCtrl)
	{
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ =
			BRDC_REGISTER(BCHP_VBI_ENC_656_Ancil_Control +
			hDisplay->ulVbiEncOffset_656);
		*pList->pulCurrent++ = ulVbiCtrl;
		hDisplay->ulPrevVbiEncControlValue_656 = ulVbiCtrl;
	}
#endif

#if BVDC_P_SUPPORT_ITU656_OUT
	if(pCurInfo->bEnable656)
	{
		/* Setup DTG_DTG_BVB (656 master mode) */
#if (BVDC_P_SUPPORT_MODULAR_VEC == 1)
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVF_DVF_BVB_CONFIG);
		*pList->pulCurrent++ = ulPicSync;
#endif  /* modular vec */
	}
#endif

#if BVDC_P_SUPPORT_DVI_OUT
	if(pCurInfo->bEnableHdmi)
	{
	#if BVDC_P_SUPPORT_DVPO /* 3548 */
		uint32_t ulNDiv;
		const BVDC_P_RateInfo *pRmInfo = hDisplay->pDvoRmInfo;
	#endif

		/* Setup DVI_DTG_DTG_BVB (dvo master mode) */
#if (BVDC_P_SUPPORT_MODULAR_VEC == 1)
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DVF_DVF_BVB_CONFIG);
		*pList->pulCurrent++ = ulPicSync;
#endif  /* modular vec */

	#if BVDC_P_SUPPORT_DVPO /* clock adjustment */
		/* adjust DVO RM clock if:
		   1. one video window turns on game mode delay control and
		   2. the window is not reconfig;
		 */
		if(hDisplay->hWinGameMode && pRmInfo &&
		   BVDC_P_STATE_IS_ACTIVE(hDisplay->hWinGameMode) &&
		   hDisplay->hWinGameMode->stCurInfo.stGameDelaySetting.bEnable &&
		   BVDC_P_IS_CLEAN(&hDisplay->hWinGameMode->stCurInfo.stDirty))
		{
			#define BVDC_P_GAMEM_MODE_DELAY_STEP         (5000)
			#define BVDC_P_GAMEM_MODE_DVO_ADJ_MAX_STEP      (1)
			/* set rate */
			if(hDisplay->hWinGameMode->bAdjGameModeClock)
			{
				int32_t lAdjCnt;

				if(hDisplay->hWinGameMode->bFastAdjClock)
				{
					lAdjCnt = hDisplay->hWinGameMode->lCurGameModeLag / BVDC_P_GAMEM_MODE_DELAY_STEP +
						((hDisplay->hWinGameMode->lCurGameModeLag > 0) ? 1 : -1);
					lAdjCnt = (lAdjCnt > BVDC_P_GAMEM_MODE_DVO_ADJ_MAX_STEP)?
						BVDC_P_GAMEM_MODE_DVO_ADJ_MAX_STEP : lAdjCnt;
					BDBG_ASSERT(pRmInfo->ulRDiv + lAdjCnt > 0);
				}
				else
				{
					lAdjCnt = (hDisplay->hWinGameMode->lCurGameModeLag > 0)? 1 : -1;
				}

				/* Note: adjust Ndiv will be coarse since it changes VCO clock */
				if(lAdjCnt > 1 || lAdjCnt < -1 ||
					hDisplay->hWinGameMode->bCoarseAdjClock)
				{
					ulNDiv  = (uint32_t)(pRmInfo->ulRDiv + lAdjCnt) * (pRmInfo->ulNDiv / pRmInfo->ulRDiv);
				}
				else
				{
					ulNDiv = pRmInfo->ulNDiv;
				}

				BVDC_P_Vec_BuildPllCfg_isr(pList, hDisplay->hVdc->hRegister,
					&pCurInfo->stDvoCfg, pRmInfo,
					pRmInfo->ulRDiv + lAdjCnt, ulNDiv);

				hDisplay->bRmAdjusted = true;
				BDBG_MSG(("Game mode delay: %d[usec]; to accelarate DVO RM? %d;",
					hDisplay->hWinGameMode->ulCurBufDelay, lAdjCnt));
			}
			else if(!hDisplay->hWinGameMode->bCoarseAdjClock && hDisplay->bRmAdjusted)/* restore */
			{
				BVDC_P_Vec_BuildPllCfg_isr(pList, hDisplay->hVdc->hRegister,
					&pCurInfo->stDvoCfg, pRmInfo,
					pRmInfo->ulRDiv, pRmInfo->ulNDiv);
				hDisplay->bRmAdjusted = false;
			}
		}
		else if(hDisplay->bRmAdjusted)/* restore */
		{
			BVDC_P_Vec_BuildPllCfg_isr(pList, hDisplay->hVdc->hRegister,
				&pCurInfo->stDvoCfg, pRmInfo,
				pRmInfo->ulRDiv, pRmInfo->ulNDiv);
			hDisplay->bRmAdjusted = false;
		}

		/* PR35122, PR53584: Add a LVDS DRESET on LVDS PLL / Rate Manager changes. */
		if(hDisplay->ulKickStartDelay)
		{
			hDisplay->ulKickStartDelay--;
			if(!hDisplay->ulKickStartDelay)
			{
				/* LVDS_PHY_0_LVDS_PLL_RESET_CTL (RW) */
				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG() ;
				*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_LVDS_RESET_CTL) ;
				*pList->pulCurrent++ =
					BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_RESET_CTL, LVDS_ARESET, 1) ;

				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG() ;
				*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_LVDS_RESET_CTL) ;
				*pList->pulCurrent++ =
					BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_RESET_CTL, LVDS_DRESET, 1) ;

				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG() ;
				*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_LVDS_RESET_CTL) ;
				*pList->pulCurrent++ =
					BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_RESET_CTL, LVDS_DRESET, 0) ;

				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG() ;
				*pList->pulCurrent++ = BRDC_REGISTER(BCHP_LVDS_PHY_0_LVDS_RESET_CTL) ;
				*pList->pulCurrent++ =
					BCHP_FIELD_DATA(LVDS_PHY_0_LVDS_RESET_CTL, LVDS_ARESET, 0) ;
			}
		}
	#endif
	}
#endif

	/* TODO: support slave analog + master digital tg */
	if (!hDisplay->bIsBypass) /* Primary or Secondary path */
	{
		bool bChangeMV = false;

		/* to clear VBI_ENC_XXX_Pass_Through.COUNT */
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_VBI_ENC_PRIM_Pass_Through + hDisplay->ulVbiEncOffset);
		*pList->pulCurrent++ = 0;

		/* set up game mode VEC RM */
		if(hDisplay->hWinGameMode && hDisplay->pRmTable &&
		   BVDC_P_STATE_IS_ACTIVE(hDisplay->hWinGameMode) &&
		   hDisplay->hWinGameMode->stCurInfo.stGameDelaySetting.bEnable &&
		   BVDC_P_IS_CLEAN(&hDisplay->hWinGameMode->stCurInfo.stDirty))
		{
			/* set rate */
			if(hDisplay->hWinGameMode->bAdjGameModeClock)
			{
				int32_t lAdj = (int32_t)hDisplay->pRmTable[BVDC_P_DISPLAY_RM_PHASE_INC_REG_OFFSET] /
					(1000 * ((hDisplay->hWinGameMode->lCurGameModeLag>0)? 1 : -1));

				/* PRIM_RM_PHASE_INC (RW) */
				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
				*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_RM_PHASE_INC + hDisplay->lRmOffset);
				*pList->pulCurrent++ =
					BCHP_FIELD_DATA(PRIM_RM_PHASE_INC, PHASE_INC,
						hDisplay->pRmTable[BVDC_P_DISPLAY_RM_PHASE_INC_REG_OFFSET] + lAdj);

				hDisplay->bRmAdjusted = true;
				BDBG_MSG(("Game mode delay: %d[usec]; to accelarate VEC%d RM? %d;",
					hDisplay->hWinGameMode->ulCurBufDelay, hDisplay->eVecPath, lAdj));
			}
			else if(hDisplay->bRmAdjusted) /* restore */
			{
				BVDC_P_Display_ResumeRmPhaseInc_isr(hDisplay, pList);
				hDisplay->bRmAdjusted = false;
			}
		}
		else if(hDisplay->bRmAdjusted) /* restore */
		{
			BVDC_P_Display_ResumeRmPhaseInc_isr(hDisplay, pList);
			hDisplay->bRmAdjusted = false;
		}

#if (BVDC_P_SUPPORT_MODULAR_VEC == 1)
		/* Setup IT_BVB_CONFIG */
		if((BAVC_Polarity_eFrame != eFieldPolarity) &&
		   (VIDEO_FORMAT_IS_625_LINES(pCurInfo->pFmtInfo->eVideoFmt)))
		{
			BVDC_P_Vec_BuildPSync_isr(hDisplay, pList, BCHP_PRIM_VF_BVB_CONFIG + hDisplay->lVfOffset);
		}
		else
		{
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_VF_BVB_CONFIG + hDisplay->lVfOffset);
			*pList->pulCurrent++ = ulPicSync;
		}
#endif

#if BVDC_P_SUPPORT_COMPONENT_ONLY
#if (BVDC_P_SUPPORT_MODULAR_VEC == 1)
		if (BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
		{
			if((BAVC_Polarity_eFrame != eFieldPolarity) &&
			   (VIDEO_FORMAT_IS_625_LINES(pCurInfo->pFmtInfo->eVideoFmt)))
			{
				BVDC_P_Vec_BuildPSync_isr(hDisplay, pList, BCHP_SEC_VF_CO_BVB_CONFIG);
			}
			else
			{
				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
				*pList->pulCurrent++ = BRDC_REGISTER(BCHP_SEC_VF_CO_BVB_CONFIG);
				*pList->pulCurrent++ = ulPicSync;
			}
		}
#endif
#endif

		/* Update VF_POS_SYNC_VALUES for every frame for AGC cycling.
		 * Note: progressive format will not have Bottom Field interrupt. */
		if(pCurInfo->eMacrovisionType != BVDC_MacrovisionType_eNoProtection)
		{
			uint32_t ulVal = BVDC_P_GetPosSyncValue_isr(hDisplay, &pList->pulCurrent,
				hDisplay->eVecPath);

			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ =
				BRDC_REGISTER(BCHP_PRIM_VF_POS_SYNC_VALUES + hDisplay->lVfOffset);
			*pList->pulCurrent++ = ulVal;

#if (BVDC_P_SUPPORT_COMPONENT_ONLY)
			/* don't forget SEC_VF_CO! */
			if(pCurInfo->eCoOutputColorSpace != BVDC_P_Output_eUnknown)
			{
				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
				*pList->pulCurrent++ = BRDC_REGISTER(BVDC_P_VEC_VF_CO_POS_SYNC_VALUES);
				*pList->pulCurrent++ = ulVal;
			}
#endif
		}

		/* Determine if new IT register settings are required */
		if(hDisplay->bMvChange &&
		   (pCurInfo->pFmtInfo->eVideoFmt == pNewInfo->pFmtInfo->eVideoFmt) &&
		   (hDisplay->eItState != BVDC_P_ItState_eSwitchMode) &&
		    hDisplay->bUserAppliedChanges)
		{
			bChangeMV = true;
		}
		else if (hDisplay->iDcsChange == 1)
		{
			bChangeMV = true;
		}

		/* Update some IT registers if MV type changed */
		if (bChangeMV)
		{
			BDBG_MSG(("Disp %d field %d to change mv type: %d -> %d",
				hDisplay->eId, eFieldPolarity,
				pCurInfo->eMacrovisionType, pNewInfo->eMacrovisionType));
			BVDC_P_ChangeMvType_isr(hDisplay, &pList->pulCurrent);
			hDisplay->bMvChange = false;
		}
		hDisplay->iDcsChange = 0;

		/* reload 480i microcode to handle 704-sample vs. 720-sample analog output;
		 * TODO: add PAL trim support; */
		if(pCurInfo->stDirty.stBits.bWidthTrim && VIDEO_FORMAT_IS_NTSC(pCurInfo->pFmtInfo->eVideoFmt) &&
		   (pCurInfo->eMacrovisionType < BVDC_MacrovisionType_eTest01))
		{
			BVDC_P_Display_ShaperSettings stShaper;
			uint32_t        ulVfMiscRegValue;
			const uint32_t *pRamTbl =
				BVDC_P_GetRamTable_isr(pCurInfo, hDisplay->bArib480p);

			/* Write IT_MICRO_INSTRUCTIONS 0..255 on-the-fly */
			*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_RAM_TABLE_SIZE);
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_MICRO_INSTRUCTIONi_ARRAY_BASE + hDisplay->lItOffset);

			BKNI_Memcpy((void*)pList->pulCurrent, (void*)pRamTbl,
				BVDC_P_RAM_TABLE_SIZE * sizeof(uint32_t));
			pList->pulCurrent += BVDC_P_RAM_TABLE_SIZE;
			hDisplay->ulCurShadowDCSKeyLow = hDisplay->stNewInfo.ulDCSKeyLow;

			/* adjust BVB_SIZE */
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_BVB_SIZE + hDisplay->lItOffset);
			*pList->pulCurrent++ =
				BCHP_FIELD_DATA(PRIM_IT_BVB_SIZE, HORIZONTAL, pCurInfo->bWidthTrimmed? 712 : 720) |
				BCHP_FIELD_DATA(PRIM_IT_BVB_SIZE, VERTICAL, pCurInfo->pFmtInfo->ulHeight / BVDC_P_FIELD_PER_FRAME);

			/* get the correct vf left cut */
			BVDC_P_FillVfTable_isr(
				pCurInfo, BVDC_P_Output_eUnknown, NULL, NULL, &stShaper);

			/* set the BVB_SAV_REMOVE to left cut */
#if BVDC_P_PR14712_FIXED
			if(pCurInfo->bWidthTrimmed)
			{
				stShaper.ulSavRemove = 8;
			}
#else
			stShaper.ulSavRemove = 0;
#endif
			ulVfMiscRegValue =
				BCHP_FIELD_DATA(PRIM_VF_MISC, reserved0,      0) |
				BCHP_FIELD_ENUM(PRIM_VF_MISC, VF_ENABLE,      ON) |
				BCHP_FIELD_DATA(PRIM_VF_MISC, SUM_OF_TAPS,    0) |
				BCHP_FIELD_ENUM(PRIM_VF_MISC, UPSAMPLE2X,     ON) |
				BCHP_FIELD_DATA(PRIM_VF_MISC, BVB_SAV_REMOVE, stShaper.ulSavRemove) |
				BCHP_FIELD_DATA(PRIM_VF_MISC, VBI_PREFERRED,  1) |
#ifdef BVDC_P_WSE_VER3
				BCHP_FIELD_ENUM(PRIM_VF_MISC, VBI_ENABLE,    OFF) |
#else
				BCHP_FIELD_ENUM(PRIM_VF_MISC, VBI_ENABLE,     ON) |
#endif
				BCHP_FIELD_ENUM(PRIM_VF_MISC, C2_RAMP,        SYNC_TRANS_1) |
				BCHP_FIELD_ENUM(PRIM_VF_MISC, C1_RAMP,        SYNC_TRANS_1) |
				BCHP_FIELD_ENUM(PRIM_VF_MISC, C0_RAMP,        SYNC_TRANS_0);

		    if ((true  == hDisplay->bArib480p) &&
			    (false == pCurInfo->bWidthTrimmed))
			{
				ulVfMiscRegValue |= (
					BCHP_FIELD_DATA(PRIM_VF_MISC, BVB_LINE_REMOVE_TOP   , 1) |
					BCHP_FIELD_DATA(PRIM_VF_MISC, BVB_LINE_REMOVE_BOTTOM, 1) );
			}

			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_VF_MISC + hDisplay->lVfOffset);
			*pList->pulCurrent++ = ulVfMiscRegValue;

#if BVDC_P_SUPPORT_COMPONENT_ONLY
			/* When using Secondary Vec, must make sure Vec consumes
			 * pixels at the same rate for both SEC_VF and SEC_VF_CO */
			if (BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
			{
				*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
				*pList->pulCurrent++ = BRDC_REGISTER(BVDC_P_VEC_VF_CO_MISC);
				*pList->pulCurrent++ = ulVfMiscRegValue;
			}
#endif
			/* clear the duirty bit */
			pCurInfo->stDirty.stBits.bWidthTrim = BVDC_P_CLEAN;
		}
	}

#ifdef BVDC_P_WSE_VER3 /** { **/
	/* Work-around for a hardware bug */
	{
		uint32_t       ulVfMiscRegIdx;
		uint32_t       ulVfMiscRegValue;
		const uint32_t *pTable;
		uint32_t aulTable[BVDC_P_VF_TABLE_SIZE];
		BVDC_P_Display_ShaperSettings stShaperSettings;

		BVDC_P_FillVfTable_isr(
			pCurInfo, pCurInfo->eOutputColorSpace,
			&aulTable[0], NULL, &stShaperSettings);
		ulVfMiscRegIdx =
			(BCHP_PRIM_VF_MISC - BCHP_PRIM_VF_FORMAT_ADDER) / sizeof(uint32_t);
		ulVfMiscRegValue =
			BVDC_P_Vec_Get_VF_MISC_isr (
				pCurInfo, hDisplay->bArib480p, &aulTable[0], &stShaperSettings);
		ulVfMiscRegValue &= ~BCHP_FIELD_ENUM(PRIM_VF_MISC, VBI_ENABLE, ON);
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ =
			BRDC_REGISTER(BCHP_PRIM_VF_MISC + hDisplay->lVfOffset);
		*pList->pulCurrent++ = ulVfMiscRegValue;
		ulVfMiscRegValue |= BCHP_FIELD_ENUM(PRIM_VF_MISC, VBI_ENABLE, ON);
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ =
			BRDC_REGISTER(BCHP_PRIM_VF_MISC + hDisplay->lVfOffset);
		*pList->pulCurrent++ = ulVfMiscRegValue;
#if BVDC_P_SUPPORT_COMPONENT_ONLY /** { **/
		/* When using Secondary Vec, must make sure Vec consumes
		 * pixels at the same rate for both SEC_VF and SEC_VF_CO */
		if ((BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath)) &&
		    (pCurInfo->eCoOutputColorSpace != BVDC_P_Output_eSDRGB) &&
		    (pCurInfo->eCoOutputColorSpace != BVDC_P_Output_eHDRGB))
		{
			ulVfMiscRegValue &= ~BCHP_FIELD_ENUM(PRIM_VF_MISC, VBI_ENABLE, ON);
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BVDC_P_VEC_VF_CO_MISC);
			*pList->pulCurrent++ = ulVfMiscRegValue;
			ulVfMiscRegValue |= BCHP_FIELD_ENUM(PRIM_VF_MISC, VBI_ENABLE, ON);
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BVDC_P_VEC_VF_CO_MISC);
			*pList->pulCurrent++ = ulVfMiscRegValue;
		}
#endif /** } BVDC_P_SUPPORT_COMPONENT_ONLY **/
		if (VIDEO_FORMAT_IS_625_LINES(pCurInfo->pFmtInfo->eVideoFmt))
		{
			pTable = BVDC_P_GetItTable_isr(pCurInfo);
			ulVfMiscRegValue = pTable[5];
			ulVfMiscRegValue &=
				~BCHP_MASK       (PRIM_IT_STACK_reg_6_7, REG_7        );
			ulVfMiscRegValue |=
				 BCHP_FIELD_DATA (PRIM_IT_STACK_reg_6_7, REG_7, 0xFFFF);
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ =
				BRDC_REGISTER(BCHP_PRIM_IT_STACK_reg_6_7 + hDisplay->lItOffset);
				*pList->pulCurrent++ = ulVfMiscRegValue;
		}
	}
#endif /** } BVDC_P_WSE_VER3 **/

	/* Display callback installed? */
	if (pCurInfo->pfGenericCallback)
	{
		BVDC_Display_CallbackData *pCbData = &hDisplay->stCallbackData;

		/* Notify external modules of rate manager updated. */
		if((hDisplay->bRateManagerUpdated) &&
		   (hDisplay->stCallbackSettings.stMask.bRateChange))
		{
			pCbData->stMask.bRateChange = 1;
			pCbData->sDisplayInfo = pCurInfo->stRateInfo;
			pCurInfo->pfGenericCallback(pCurInfo->pvGenericParm1,
				pCurInfo->iGenericParm2, (void*)pCbData);
			/* Clear rate change flag */
			hDisplay->bRateManagerUpdated = false;
		}

#if BVDC_P_SUPPORT_CMP_CRC
		if (hDisplay->stCallbackSettings.stMask.bCrc)
		{
			uint32_t ulReg;

			/* Get new CRC YCrCb values */
			ulReg = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			                        BCHP_CMP_0_CRC_Y_STATUS + hDisplay->hCompositor->ulRegOffset);
			pCbData->ulCrcLuma = BVDC_P_GET_FIELD(ulReg, CMP_0_CRC_Y_STATUS, VALUE);

			ulReg = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			                        BCHP_CMP_0_CRC_CR_STATUS + hDisplay->hCompositor->ulRegOffset);
			pCbData->ulCrcCr = BVDC_P_GET_FIELD(ulReg, CMP_0_CRC_CR_STATUS, VALUE);

			ulReg = BREG_Read32_isr(hDisplay->hVdc->hRegister,
			                        BCHP_CMP_0_CRC_CB_STATUS + hDisplay->hCompositor->ulRegOffset);
			pCbData->ulCrcCb= BVDC_P_GET_FIELD(ulReg, CMP_0_CRC_CB_STATUS, VALUE);

			pCbData->eId = hDisplay->eId;
			pCbData->stMask.bCrc = 1;

			/* Callback application with the above data */
			pCurInfo->pfGenericCallback(pCurInfo->pvGenericParm1,
			                            eFieldPolarity, (void*)pCbData);
		}
#endif
		/* Clear mask bits */
		pCbData->stMask.bRateChange = 0;
		pCbData->stMask.bCrc = 0;
	}

	/* Set event now. */
	if((hDisplay->bSetEventPending) &&
	   (BVDC_P_ItState_eActive == hDisplay->eItState))
	{
		hDisplay->bSetEventPending = false;
		BKNI_SetEvent_isr(hDisplay->hAppliedDoneEvent);
	}

	return;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Display_UpdateTimestamp_isr
 *  Update delta between two displays timestamps; delta>0 needs to speedup; otherwise slowdown;
 **************************************************************************/
static int32_t BVDC_P_Display_UpdateTimestamp_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_DisplayContext           *pTargetDisplay,
	  BAVC_Polarity                    eFieldPolarity )
{
	int32_t lDeltaTs;
	uint32_t ulTargetTs = pTargetDisplay->ulCurrentTs;
	uint32_t ulVsyncInterval =
		BVDC_P_DISPLAY_ONE_VSYNC_INTERVAL(pTargetDisplay->stCurInfo.pFmtInfo->ulVertFreq);
	uint32_t ulMyVsyncInterval =
		BVDC_P_DISPLAY_ONE_VSYNC_INTERVAL(hDisplay->stCurInfo.pFmtInfo->ulVertFreq);

	/* 0) update the display timestamps & cadence */

	/*  note: my Ts may be triggered (N-1) myVsyncs earlier depending on my alignment
	    period N number! So to get my current Ts, need to add (N-1)* myVsyncIntervals; */
	hDisplay->ulCurrentTs   = BREG_Read32_isr(hDisplay->hVdc->hRegister, hDisplay->ulScratchTsAddr)
		+ (hDisplay->ulTsSamplePeriod - 1) * ulMyVsyncInterval;
	hDisplay->eNextPolarity = eFieldPolarity;

	/* adjust only when delta is within one Vsync interval;
	 * TODO: we assumed timer is 30-bit range, so the addition won't wraparound;
	 *       but if timer is 32-bit, need to handle wraparound case! */
	if((hDisplay->ulCurrentTs + BVDC_P_DISPLAY_ALIGNMENT_THRESHOLD >  ulTargetTs) &&
	   (hDisplay->ulCurrentTs <  ulTargetTs + ulVsyncInterval + BVDC_P_DISPLAY_ALIGNMENT_THRESHOLD))
	{
		/* 1) compute delta between this display and the target display in auto mode; */
		if(BVDC_AlignmentDirection_eAuto == hDisplay->stCurInfo.stAlignCfg.eDirection)
		{
			/* 1.1) interlaced/interlaced case; */
			if(hDisplay->stCurInfo.pFmtInfo->bInterlaced &&
			    pTargetDisplay->stCurInfo.pFmtInfo->bInterlaced)
			{
				/* same polarity */
				if (hDisplay->eNextPolarity == pTargetDisplay->eNextPolarity)
				{
					/* positive delta: speedup; */
					lDeltaTs = hDisplay->ulCurrentTs - ulTargetTs;
				}
				else /* opposite polarity */
				{
					/* negative delta: slow down; */
					lDeltaTs = (int32_t)(hDisplay->ulCurrentTs - ulTargetTs) - (int32_t)ulVsyncInterval;
				}
			}
			/* 1.2) progressive/interlaced case */
			else
			{
				/* accelerate or decelerate whichever converge quicker; */
				if(hDisplay->ulCurrentTs > ulTargetTs + ulVsyncInterval/2)
				{
					/* negative delta: slow down; */
					lDeltaTs =(int32_t)(hDisplay->ulCurrentTs - ulTargetTs) - (int32_t)ulVsyncInterval;
				}
				else
				{
					/* positive delta: speed up; */
					lDeltaTs = (int32_t)(hDisplay->ulCurrentTs - ulTargetTs);
				}
			}
		}
		else /* 2) manual mode */
		{
			lDeltaTs = (hDisplay->stCurInfo.stAlignCfg.eDirection == BVDC_AlignmentDirection_eFaster) ?
				(int32_t)(hDisplay->ulCurrentTs - ulTargetTs) :
				((int32_t)(hDisplay->ulCurrentTs - ulTargetTs) - (int32_t)ulVsyncInterval);
		}

		/* align behind the target */
		lDeltaTs -= BVDC_P_DISPLAY_ALIGNMENT_OFFSET;
	}
	else
	{
		lDeltaTs = 0; /* ignore isr dis-ordering; */
	}

	/*BDBG_MSG(("hDisplay->ulCurrentTs=%d, ulTargetTs=%d", hDisplay->ulCurrentTs,ulTargetTs));*/
	return lDeltaTs;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Display_Alignment_isr
 **************************************************************************/
static void BVDC_P_Display_Alignment_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_DisplayContext           *pTargetDisplay,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldPolarity )
{
	int32_t lDeltaTs = BVDC_P_Display_UpdateTimestamp_isr(hDisplay, pTargetDisplay, eFieldPolarity);

	/* 1) build RUL to snapshot display timestamp; */
	*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
	*pList->pulCurrent++ = BRDC_REGISTER(hDisplay->stTimerReg.status);

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_1);
	*pList->pulCurrent++ = BCHP_TIMER_TIMER0_STAT_COUNTER_VAL_MASK;

	*pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_2);

	*pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
	*pList->pulCurrent++ = BRDC_REGISTER(hDisplay->ulScratchTsAddr);

	/* 2) adjust RM clock for alignment; */
	if(!hDisplay->bIsBypass) /* analog path */
	{
		/* set rate: TODO: when to stop? */
		if ((lDeltaTs > BVDC_P_DISPLAY_ALIGNMENT_THRESHOLD) ||
		    (lDeltaTs < -(int32_t)BVDC_P_DISPLAY_ALIGNMENT_THRESHOLD))
		{
			#define BVDC_P_DISPLAY_ALIGNMENT_STEP_SCALE       (6)  /* 1/6 delta step size */
			/* variable step size */
			int32_t lAdj = (int32_t)(hDisplay->pRmTable[BVDC_P_DISPLAY_RM_PHASE_INC_REG_OFFSET] /
				BVDC_P_DISPLAY_ONE_VSYNC_INTERVAL(hDisplay->stCurInfo.pFmtInfo->ulVertFreq)) * lDeltaTs /
				BVDC_P_DISPLAY_ALIGNMENT_STEP_SCALE;

			/* PRIM_RM_PHASE_INC (RW) */
			*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_RM_PHASE_INC + hDisplay->lRmOffset);
			*pList->pulCurrent++ =
				BCHP_FIELD_DATA(PRIM_RM_PHASE_INC, PHASE_INC,
					((int32_t)hDisplay->pRmTable[BVDC_P_DISPLAY_RM_PHASE_INC_REG_OFFSET] + lAdj));
			BDBG_MSG(("Display align delta: %d[cycles]; to accelarate VEC%d RM? %d;",
				lDeltaTs, hDisplay->eVecPath, lAdj));
			hDisplay->bAlignAdjusting = true;
		}
		else if(hDisplay->bAlignAdjusting)
		{
			BVDC_P_Display_ResumeRmPhaseInc_isr(hDisplay, pList);
			hDisplay->bAlignAdjusting = false;
		}
	}

}

/* Due to alignment of MFD, or multiple windows sharing a MPEG src, MAD might
 * have to process extra lines after CMP completed the field. If RDC trigers
 * right after it, MAD has little time to complete the job, and cmd error will
 * occur.  Therefore, we need to delay the RDC triger a little bit. However,
 * for the panel we use in vdc team, the biggest BVDC_P_TRIGGER_DELAY could 3.
 * Number bigger than that leads to (ulTopTrigVal > pFmtInfo->ulScanHeight)
 * and the triger line is forced to 1. */
#define BVDC_P_EARLY_TRIGGER_DELAY               (2)

/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_BuildRul
 *  Build the necessary Vec blocks.
 **************************************************************************/
void BVDC_P_Vec_BuildRul_isr
	( BVDC_Display_Handle              hDisplay,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldPolarity )
{
	BVDC_P_Display_DirtyBits *pCurDirty;
	uint32_t             ulTopTrigVal, ulBotTrigVal;
	BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
	BVDC_P_DisplayInfo  *pNewInfo = &hDisplay->stNewInfo;
	uint32_t             ulHeight;
	bool                 bDisplayRestart =
		(hDisplay->eItState == BVDC_P_ItState_eNotActive) ||
		(hDisplay->eItState == BVDC_P_ItState_eSwitchMode);

	/* TODO: Building RUL should be off the current context.  New context
	 * should be use for validations, and once it's passed validation it will
	 * be used.  We'd like to narrow down of accessing the NEW context in
	 * isr context. */

	/* Display dirty bits. */
	pCurDirty = &pCurInfo->stDirty;

	/* Compute number of lines per field/frame base on format! */
	ulHeight = (pNewInfo->pFmtInfo->bInterlaced) ?
		pNewInfo->pFmtInfo->ulHeight / BVDC_P_FIELD_PER_FRAME:
		pNewInfo->pFmtInfo->ulHeight;

	/* Alignment:
	    1) neither displays are in format switch;
	    2) both displays are in the mutual-lockable vertical refresh rate;
	    3) we only support slower slave aligned with faster master now;
	  */
	if(!bDisplayRestart &&
	    hDisplay->pRmTable &&
	    BVDC_P_IS_CLEAN(pCurDirty) &&
	    pCurInfo->hTargetDisplay &&
	    BVDC_P_IS_CLEAN(&pCurInfo->hTargetDisplay->stCurInfo.stDirty) &&
	    (pCurInfo->hTargetDisplay->eItState == BVDC_P_ItState_eActive) &&
	    ((pCurInfo->ulVertFreq == pCurInfo->hTargetDisplay->stCurInfo.ulVertFreq) ||
	     (pCurInfo->ulVertFreq == 2997 && pCurInfo->hTargetDisplay->stCurInfo.ulVertFreq==5994) ||
	     (pCurInfo->ulVertFreq == 2397 && pCurInfo->hTargetDisplay->stCurInfo.ulVertFreq==5994) ||
	     (pCurInfo->ulVertFreq == 2500 && pCurInfo->hTargetDisplay->stCurInfo.ulVertFreq==5000)))
	{
		/* slave->master vsync TS sampling periods for display alignment:
			72->60: sample once every 6 vsyncs; not required yet;
			60->50: sample once every 6 vsyncs; not required yet;
			50->60: sample once every 5 vsyncs; not required yet;
			60->24: sample once every 5 vsyncs; not required yet;
			48->60: sample once every 4 vsyncs; not required yet;
			24->60: sample once every 2 vsyncs;
			60->30: sample once every 2 vsyncs; not required yet;
			50->25: sample once every 2 vsyncs; not required yet;
			30->60: sample once every 1 vsyncs;
			25->50: sample once every 1 vsyncs;
			50->50: sample once every 1 vsyncs;
			60->60: sample once every 1 vsyncs;
			24->24: sample once every 1 vsyncs; not required;
			30->30: sample once every 1 vsyncs; not required;
		 */
		if(pCurInfo->ulVertFreq == 2397 && pCurInfo->hTargetDisplay->stCurInfo.ulVertFreq==5994)
		{
			hDisplay->ulTsSamplePeriod = 2;
		}
		else
		{
			hDisplay->ulTsSamplePeriod = 1;
		}

		/* counting down TS sampling vsyncs */
		if(hDisplay->ulTsSampleCount == 0)
		{
			hDisplay->ulTsSampleCount = hDisplay->ulTsSamplePeriod-1;
			BVDC_P_Display_Alignment_isr(hDisplay, (BVDC_P_DisplayContext*)pCurInfo->hTargetDisplay, pList, eFieldPolarity);
		}
		else
		{
			hDisplay->ulTsSampleCount--;
		}
	}
	else if(hDisplay->ulAlignSlaves) /* update timestamp for target display */
	{
		hDisplay->eNextPolarity = eFieldPolarity;

		hDisplay->ulCurrentTs   = BREG_Read32_isr(hDisplay->hVdc->hRegister, hDisplay->ulScratchTsAddr);

		/* 1) build RUL to snapshot display timestamp; */
		*pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
		*pList->pulCurrent++ = BRDC_REGISTER(hDisplay->stTimerReg.status);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_1);
		*pList->pulCurrent++ = BCHP_TIMER_TIMER0_STAT_COUNTER_VAL_MASK;

		*pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_2);

		*pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
		*pList->pulCurrent++ = BRDC_REGISTER(hDisplay->ulScratchTsAddr);
	}
	else if(!hDisplay->bIsBypass && hDisplay->pRmTable && hDisplay->bAlignAdjusting)
	{
		BVDC_P_Display_ResumeRmPhaseInc_isr(hDisplay, pList);
		hDisplay->bAlignAdjusting = false;
	}

	/* Users can switch streams, without notifying us. Need to get the
	 * input color space & frame rate here, Adjusm. */
	BVDC_P_Compositor_GetOutputInfo_isr(hDisplay->hCompositor, &pNewInfo->bFullRate);

	/* Compute Display Trigger Values. Rules of thumb are:
	   1) Trigger must locate in the VBI region to avoid partial picture update and BVN stall;
	   2) Leave as much headroom as possible for RUL to complete execution within VBI
	      region and for video data to pre-fill the BVN pipeline;
	   3) Top trigger fires a top RUL to program BVN for display of a top field or a progressive
	      frame, so top trigger should locate BEFORE the start of a progressive frame or the
	      start of a top field picture;
	   4) Bottom trigger fires a bottom RUL to program BVN for display of a bottom field, so
	      bottom trigger should locate BEFORE the start of an interlaced bottom field picture;
	   Note: DTV custom formats reposition the triggers to the end of active video region
	            to leave more room for long RULs to execute;
	            Settop products need more thorough tests to make sure all display formats
	            ulTopActive values to match microcode implementation; */
#if BVDC_P_SUPPORT_EARLY_TRIGGER
	/* DTV projects come here: position trigger as early as possible; */
	/* for progressive format, this value is the 1st line after the active frame; */
#if BFMT_LEGACY_3DTV_SUPPORT
	if(VIDEO_FORMAT_IS_CUSTOM_1080P3D(pNewInfo->pFmtInfo->eVideoFmt))
	{
		ulTopTrigVal = 500;
		ulBotTrigVal = 1625;
	}
	else
#endif
	{
		ulTopTrigVal = BVDC_P_EARLY_TRIGGER_DELAY + pNewInfo->pFmtInfo->ulTopActive +
			(pNewInfo->pFmtInfo->ulHeight >> pNewInfo->pFmtInfo->bInterlaced);

		/* to avoid trigger value overbound; */
		if(ulTopTrigVal > pNewInfo->pFmtInfo->ulScanHeight)
		{
			ulTopTrigVal = 1;/* line counter starts from 1; */
		}

		/* For interlaced format, end of frame means end of bottom picture; */
		if(pNewInfo->pFmtInfo->bInterlaced)
		{
			/* bottom trigger is after end of top picture; */
			ulBotTrigVal = ulTopTrigVal;
			/* top trigger is after the end of bottom picture; */
			ulTopTrigVal = ((pNewInfo->pFmtInfo->ulScanHeight + 1) / 2) + ulBotTrigVal;

			/* avoid overbound as well; */
			if(ulTopTrigVal > pNewInfo->pFmtInfo->ulScanHeight)
				ulTopTrigVal = 1;
		}
		/* For progressive format, bottom trigger is disabled; */
		else
		{
			ulBotTrigVal = 0;
		}
	}
#else
	ulTopTrigVal = BVDC_P_TRIGGER_LINE;
	ulBotTrigVal = (pNewInfo->pFmtInfo->bInterlaced)
		? (((pNewInfo->pFmtInfo->ulScanHeight + 1) / 2) + BVDC_P_TRIGGER_LINE) : 0;
#endif

	/* Setup RM block:
	 * (1) must be !bypass, or master timing generator.
	 * AND
	 * (a) eFramerate changes
	 * (b) eVideoFmt, eTimebase, RateChange changes; on applychanges()
	 * (c) bDisplayRestart
	 * (d) missed rul */

	/* Notify external modules that Rate Manager has been changed. */
	if(hDisplay->bRateManagerUpdatedPending && pList->bLastExecuted)
	{
		hDisplay->bRateManagerUpdatedPending = false;
		hDisplay->bRateManagerUpdated = true;
	}

	if(((!hDisplay->bIsBypass) ||
	    (BVDC_DisplayTg_eDviDtg == hDisplay->eMasterTg)) && /* (1) */
	   ((pCurInfo->bFullRate != pNewInfo->bFullRate) ||   /* (a) */
	   (((pCurInfo->stDirty.stBits.bTiming) ||
	     (pCurInfo->stDirty.stBits.bRateChange) ||
	     (pCurInfo->eTimeBase != pNewInfo->eTimeBase)) &&
	    (hDisplay->bUserAppliedChanges)) ||                 /* (b) */
	   (bDisplayRestart) ||                                 /* (c) */
	   (!pList->bLastExecuted)))                            /* (d) */
	{
		/* get the correct rm table and update RateInfo */
		BVDC_P_GetRmTable_isr (pNewInfo, pNewInfo->pFmtInfo, &hDisplay->pRmTable, pNewInfo->bFullRate, &pNewInfo->stRateInfo);
		pNewInfo->ulVertFreq = pNewInfo->stRateInfo.ulVertRefreshRate;

		if((pCurInfo->stRateInfo.ulPixelClkRate !=
		    pNewInfo->stRateInfo.ulPixelClkRate) ||
		   (pCurInfo->stRateInfo.ulVertRefreshRate !=
		    pNewInfo->stRateInfo.ulVertRefreshRate) ||
		   (pCurInfo->eTimeBase != pNewInfo->eTimeBase))
		{
			pCurInfo->stRateInfo = pNewInfo->stRateInfo;
			hDisplay->bRateManagerUpdatedPending = true;
		}

		/* Note: when switching mode, the IT block needs to be reset
		 * before anything else! */
		if(!hDisplay->bIsBypass)
		{
			BVDC_P_Vec_Build_RM_isr(hDisplay, hDisplay->pRmTable, pList);
			BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_RM_isr", hDisplay->eId));
			BDBG_MSG(("\tVEC's RM PxlClk = %sMHz, RefRate = %d (1/%dth Hz), MultiRate=%d",
				BVDC_P_GetRmString_isr(pNewInfo, pNewInfo->pFmtInfo), pNewInfo->ulVertFreq, BFMT_FREQ_FACTOR,
				pNewInfo->bMultiRateAllow));
		}

		/* Build HDMI Rate manager as well. */
		if(pNewInfo->bEnableHdmi)
		{
			if((bDisplayRestart) ||
			   (hDisplay->bRateManagerUpdatedPending))
			{
				BVDC_P_Vec_Build_HdmiRM_isr(hDisplay, pList);
				BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_HdmiRM_isr", hDisplay->eId));
				BDBG_MSG(("\tHDMI/LVDS's RM PxlClk = %sMHz, RefRate = %d (1/%dth Hz)",
					hDisplay->pDvoRmInfo->pchRate, pNewInfo->ulVertFreq, BFMT_FREQ_FACTOR));
			}
		}

		/* Update frame rate! */
		pCurInfo->bFullRate = pNewInfo->bFullRate;
	}

	/* Setup CSC, SRC, SM blocks.
	 * Note, when source dynamically changes color space, vec needs to load different
	 * csc matrix to compensate the compositor csc truncation errors;
	 * ASSUME: non-bypass display's input color space doesn't change. */
	if (((bDisplayRestart) ||
	     ((pCurInfo->eCmpColorSpace != pNewInfo->eCmpColorSpace) &&
		  /* PR51169: fix for when both input colorspace and format changed but dac not yet
		     updated through applychanges() */
		  (pCurInfo->pFmtInfo->eVideoFmt == pNewInfo->pFmtInfo->eVideoFmt)) ||
		 (pCurInfo->stDirty.stBits.bOutputMute) ||
	     (hDisplay->bUserAppliedChanges)) &&
	    (!hDisplay->bIsBypass))
	{
		BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_CSC_SRC_SM_isr", hDisplay->eId));
		BVDC_P_Vec_Build_CSC_SRC_SM_isr(hDisplay, pList);
	}

	if((hDisplay->bUserAppliedChanges) || bDisplayRestart)
	{
		/* Build MISC block */
		BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_MISC_isr", hDisplay->eId));
		BVDC_P_Vec_Build_MISC_isr(hDisplay, pList);
	}

#if BVDC_P_SUPPORT_ITU656_OUT
	if(pNewInfo->bEnable656)
	{
		/* re-program 656 DTG/DVF blocks if format switch */
		if((hDisplay->bUserAppliedChanges) || bDisplayRestart)
		{
			BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_656_isr", hDisplay->eId));
			BVDC_P_Vec_Build_656_isr(hDisplay, pList, ulHeight,
				ulTopTrigVal, ulBotTrigVal);
		}

		/* reload 656 csc matrix if source dynamic change, or format switch; */
		if((pCurInfo->eCmpColorSpace != pNewInfo->eCmpColorSpace) ||
		   (pCurInfo->stDirty.stBits.bOutputMute) ||
		   bDisplayRestart)
		{
			BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_656_CSC_isr", hDisplay->eId));
			BVDC_P_Vec_Build_656_CSC_isr(hDisplay, pList);
		}
	}
#endif

#if BVDC_P_SUPPORT_DVI_OUT
	if(pNewInfo->bEnableHdmi)
	{
		/* reload HDMI/DVO csc matrix if source dynamic change, user change or
		 * format switch;  Or custom matrix. */
		if((bDisplayRestart) ||
		   (pCurInfo->stDirty.stBits.bCscAdjust) ||
		   (pCurInfo->stDirty.stBits.bOutputMute) ||
		   (pCurInfo->eCmpColorSpace != pNewInfo->eCmpColorSpace))
		{
			BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_DVI_CSC_isr", hDisplay->eId));
			BVDC_P_Vec_Build_DVI_CSC_isr(hDisplay, pList);
		}
	}
#endif

	/* save the dynamic changed new input color space after all the CSC matrices are updated; */
	pCurInfo->eCmpColorSpace = pNewInfo->eCmpColorSpace;

	/* Do we need to build the other blocks?
	 * 1) ApplyChanges
	 * 2) Initial state, Vec is not active */
	if(hDisplay->bUserAppliedChanges || bDisplayRestart)
	{
#if BVDC_P_SUPPORT_DVI_OUT
		/* Setup DVI blocks */
		if(pNewInfo->bEnableHdmi)
		{
			if(bDisplayRestart)
			{
				BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_DVI_isr", hDisplay->eId));
				BVDC_P_Vec_Build_DVI_isr(hDisplay, pList, ulHeight, ulTopTrigVal, ulBotTrigVal);
			}
			BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_DVF_Color_isr", hDisplay->eId));

#if (!BVDC_P_VEC_HAS_2_DIFFERENT_DVF)
			BVDC_P_Vec_Build_DVF_Color_isr(hDisplay, pList, BCHP_DVI_DVF_DVF_VALUES);
#else
			BVDC_P_Vec_Build_DVF_Color_isr(hDisplay, pList, BCHP_DVI_DVF_DVI_DVF_VALUES);
#endif
			if(pCurDirty->stBits.bCcbAdjust)
			{
				BVDC_P_Vec_BuildCCBRul_isr(hDisplay, pList);
			}
		}
#endif
		if(hDisplay->bIsBypass)
		{
			goto BVDC_P_Vec_BuildRul_isr_UpdateDisplayState;
		}

		/* Setup VF */
		BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_VF_isr", hDisplay->eId));
		BVDC_P_Vec_Build_VF_isr(hDisplay, pList,
			BCHP_PRIM_VF_FORMAT_ADDER + hDisplay->lVfOffset,
			pNewInfo->eOutputColorSpace);

#if BVDC_P_SUPPORT_COMPONENT_ONLY
		/* Setup SEC_VF_CO */
		if (BVDC_P_VEC_PATH_SUPPORT_CO(hDisplay->eVecPath))
		{
			BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_VF_CO_isr", hDisplay->eId));
			BVDC_P_Vec_Build_VF_isr(hDisplay, pList,
			BVDC_P_VEC_VF_CO_FORMAT_ADDER,
				pNewInfo->eCoOutputColorSpace);
		}
#endif

		/* MUST be last!! */
		/* Switch modes if mode changes or for the first time */
		if(bDisplayRestart)
		{
			/* Reset/program IT block. Required for new format switch */
			BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_Build_IT_isr", hDisplay->eId));
			BVDC_P_Vec_Build_IT_isr(hDisplay, pList, ulHeight,
				ulTopTrigVal, ulBotTrigVal);
		}
		else
		{
			/* Necessary settings for every Vsync to avoid flash */
			BVDC_P_Vec_BuildVsync_isr(hDisplay, pList, eFieldPolarity);
		}

		/* program PCL_0 for separate h/v sync outputs for DTV application. */
		BVDC_P_Vec_Build_SyncPCL_isr(hDisplay, pList);

BVDC_P_Vec_BuildRul_isr_UpdateDisplayState:
		/* We're done with the new changes, reset our flag */
		if(hDisplay->bUserAppliedChanges)
		{
			/* Our new now becomes our current(hw-applied) */
			hDisplay->stCurInfo = hDisplay->stNewInfo;
			hDisplay->bUserAppliedChanges = false;
		}
	}
	else
	{
		/* Necessary settings for every Vsync */
		BVDC_P_Vec_BuildVsync_isr(hDisplay, pList, eFieldPolarity);

		/* Clear dirty bits when done building RUL!
		 * TODO: eventually move all the build rul into this condition. */
		if(BVDC_P_IS_DIRTY(pCurDirty))
		{
			BDBG_MSG(("pCurDirty  = 0x%08x", (*pCurDirty).aulInts[0]));
			BDBG_MSG(("bCscAdjust = %d, bUserCsc = %d",
				pCurDirty->stBits.bCscAdjust, pCurInfo->bUserCsc));
			BDBG_MSG(("bRateChange = %d, eDropFrame = %d",
				pCurDirty->stBits.bRateChange, pCurInfo->eDropFrame));
			BDBG_MSG(("bWidthTrim = %d, bWidthTrimmed = %d",
				pCurDirty->stBits.bWidthTrim, pCurInfo->bWidthTrimmed));
			BVDC_P_CLEAN_ALL_DIRTY(pCurDirty);
		}
	}

	return;
}

/***************************************************************************
 *
 * BVDC_Display_GetItUcodeInfo
 * Return info about VEC IT microcode, from actual hardware registers.
 ***************************************************************************/
BERR_Code BVDC_Display_GetItUcodeInfo
	( BVDC_Display_Handle hDisplay,
	  BVDC_Display_ItUcodeInfo* pInfo )
{
	BREG_Handle hReg;
	uint32_t iOffset;

	BDBG_ENTER (BVDC_Display_GetItUcodeInfo);

	if (!hDisplay)
	{
		return BERR_TRACE (BERR_INVALID_PARAMETER);
	}

	iOffset = hDisplay->lItOffset;
	hReg = hDisplay->hVdc->hRegister;

	pInfo->ulAnalogTimestamp =
		BREG_Read32 (
			hReg,
			BCHP_PRIM_IT_MICRO_INSTRUCTIONi_ARRAY_BASE +
				(sizeof(uint32_t)*BVDC_P_RAM_TABLE_TIMESTAMP_IDX) +
				iOffset);
	pInfo->ulAnalogChecksum =
		BREG_Read32 (
			hReg,
			BCHP_PRIM_IT_MICRO_INSTRUCTIONi_ARRAY_BASE +
				(sizeof(uint32_t)*BVDC_P_RAM_TABLE_CHECKSUM_IDX) +
				iOffset);
	pInfo->ulI656Timestamp =
		BREG_Read32 (
			hReg,
			BCHP_DTRAM_DMC_INSTRUCTIONi_ARRAY_BASE +
				(sizeof(uint32_t)*BVDC_P_DTRAM_TABLE_TIMESTAMP_IDX));
	pInfo->ulI656Checksum =
		BREG_Read32 (
			hReg,
			BCHP_DTRAM_DMC_INSTRUCTIONi_ARRAY_BASE +
				(sizeof(uint32_t)*BVDC_P_DTRAM_TABLE_CHECKSUM_IDX));
	pInfo->ulDviTimestamp =
		BREG_Read32 (
			hReg,
			BCHP_DTRAM_DMC_INSTRUCTIONi_ARRAY_BASE +
				(sizeof(uint32_t)*BVDC_P_DTRAM_TABLE_SIZE) +
				(sizeof(uint32_t)*BVDC_P_DTRAM_TABLE_TIMESTAMP_IDX));
	pInfo->ulDviChecksum =
		BREG_Read32 (
			hReg,
			BCHP_DTRAM_DMC_INSTRUCTIONi_ARRAY_BASE +
			(sizeof(uint32_t)*BVDC_P_DTRAM_TABLE_SIZE) +
			(sizeof(uint32_t)*BVDC_P_DTRAM_TABLE_CHECKSUM_IDX));

	BDBG_LEAVE (BVDC_Display_GetItUcodeInfo);
	return BERR_SUCCESS;
}

void BVDC_P_ResetVec
	( BVDC_P_Context                  *pVdc )
{
	uint32_t ulVbiPrimReg;
#if BVDC_P_SUPPORT_SEC_VEC
	uint32_t ulVbiSecReg;
#endif
#if BVDC_P_SUPPORT_TER_VEC
	uint32_t ulVbiTerReg;
#endif
#if (BVDC_P_SUPPORT_VBI_ENC_656)
	uint32_t ulVbiAncilReg;
#endif

#ifdef BCHP_VCXO_0_RM_REG_START
	uint32_t i;
	uint32_t ulVcxoRm0[BVDC_P_VCXO_RM_REG_COUNT];
#endif
#ifdef BCHP_VCXO_1_RM_REG_START
	uint32_t ulVcxoRm1[BVDC_P_VCXO_RM_REG_COUNT];
#endif
#ifdef BCHP_VCXO_2_RM_REG_START
	uint32_t ulVcxoRm2[BVDC_P_VCXO_RM_REG_COUNT];
#endif

	/* prepare for software reset */
	BKNI_EnterCriticalSection();
	/* before reset, get regsiters partially owned by VBI module */
	ulVbiPrimReg  = BREG_Read32(pVdc->hRegister, BCHP_VBI_ENC_PRIM_Control);
#if (BVDC_P_SUPPORT_VBI_ENC_656)
	ulVbiAncilReg = BREG_Read32(pVdc->hRegister, BCHP_VBI_ENC_656_Ancil_Control);
#endif

#if BVDC_P_SUPPORT_SEC_VEC
	ulVbiSecReg   = BREG_Read32(pVdc->hRegister, BCHP_VBI_ENC_SEC_Control);
#endif

#if BVDC_P_SUPPORT_TER_VEC
	ulVbiTerReg   = BREG_Read32(pVdc->hRegister, BCHP_VBI_ENC_TERT_Control);
#endif

	/* save VCXO_RM settings for 740x since it's moved to vec core; */
#ifdef BCHP_VCXO_0_RM_REG_START
	for(i = 0; i < BVDC_P_VCXO_RM_REG_COUNT; i++)
	{
		ulVcxoRm0[i] = BREG_Read32(pVdc->hRegister, BCHP_VCXO_0_RM_REG_START +
			i * sizeof(uint32_t));
	}
#endif

#ifdef BCHP_VCXO_1_RM_REG_START
	for(i = 0; i < BVDC_P_VCXO_RM_REG_COUNT; i++)
	{
		ulVcxoRm1[i] = BREG_Read32(pVdc->hRegister, BCHP_VCXO_1_RM_REG_START +
			i * sizeof(uint32_t));
	}
#endif

#ifdef BCHP_VCXO_2_RM_REG_START
	for(i = 0; i < BVDC_P_VCXO_RM_REG_COUNT; i++)
	{
		ulVcxoRm2[i] = BREG_Read32(pVdc->hRegister, BCHP_VCXO_2_RM_REG_START +
			i * sizeof(uint32_t));
	}
#endif

	/* Software Reset entire VEC block!  This will reset RM  */
	BREG_AtomicUpdate32_isr(pVdc->hRegister, BCHP_SUN_TOP_CTRL_SW_RESET,
		BCHP_MASK(       SUN_TOP_CTRL_SW_RESET, vec_sw_reset),
		BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, vec_sw_reset, 1 ));

	BREG_AtomicUpdate32_isr(pVdc->hRegister, BCHP_SUN_TOP_CTRL_SW_RESET,
		BCHP_MASK(       SUN_TOP_CTRL_SW_RESET, vec_sw_reset),
		BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, vec_sw_reset, 0 ));

	/* PR 9338:
	   The VBI module owns most of the fields from these registers.
		All fields owned by VBI should be restored and the pass_through field
		of PRIM/SEC vbi encoders are set, while 656 vbi encoder's pass_through
		count is cleared. */
	ulVbiPrimReg  |= BCHP_VBI_ENC_PRIM_Control_ENABLE_PASS_THROUGH_MASK;
	BREG_Write32(pVdc->hRegister, BCHP_VBI_ENC_PRIM_Control, ulVbiPrimReg);
	BREG_Write32(pVdc->hRegister, BAVC_VBI_ENC_0_CTRL_SCRATCH, ulVbiPrimReg);
#if (BVDC_P_SUPPORT_VBI_ENC_656)
	ulVbiAncilReg &= ~BCHP_VBI_ENC_656_Ancil_Control_PASS_THROUGH_COUNT_MASK;
	BREG_Write32(pVdc->hRegister, BCHP_VBI_ENC_656_Ancil_Control, ulVbiAncilReg);
	BREG_Write32(pVdc->hRegister, BAVC_VBI_ENC_BP_CTRL_SCRATCH, ulVbiAncilReg);
#endif

#if (BVDC_P_SUPPORT_TER_VEC)
	ulVbiTerReg |= BCHP_VBI_ENC_PRIM_Control_ENABLE_PASS_THROUGH_MASK;
	BREG_Write32(pVdc->hRegister, BCHP_VBI_ENC_TERT_Control, ulVbiTerReg);
	BREG_Write32(pVdc->hRegister, BAVC_VBI_ENC_2_CTRL_SCRATCH, ulVbiTerReg);
#endif

#if (BVDC_P_SUPPORT_SEC_VEC)
	ulVbiSecReg   |= BCHP_VBI_ENC_PRIM_Control_ENABLE_PASS_THROUGH_MASK;
	BREG_Write32(pVdc->hRegister, BCHP_VBI_ENC_SEC_Control, ulVbiSecReg);
	BREG_Write32(pVdc->hRegister, BAVC_VBI_ENC_1_CTRL_SCRATCH, ulVbiSecReg);
#endif

	/* restore VCXO_RM settings for 740x since it's moved to vec core; */
#ifdef BCHP_VCXO_0_RM_REG_START
	for(i = 0; i < BVDC_P_VCXO_RM_REG_COUNT; i++)
	{
		BREG_Write32(pVdc->hRegister, BCHP_VCXO_0_RM_REG_START +
			i * sizeof(uint32_t), ulVcxoRm0[i]);
	}
#endif

#ifdef BCHP_VCXO_1_RM_REG_START
	for(i = 0; i < BVDC_P_VCXO_RM_REG_COUNT; i++)
	{
		BREG_Write32(pVdc->hRegister, BCHP_VCXO_1_RM_REG_START +
			i * sizeof(uint32_t), ulVcxoRm1[i]);
	}
#endif

#ifdef BCHP_VCXO_2_RM_REG_START
	for(i = 0; i < BVDC_P_VCXO_RM_REG_COUNT; i++)
	{
		BREG_Write32(pVdc->hRegister, BCHP_VCXO_2_RM_REG_START +
			i * sizeof(uint32_t), ulVcxoRm2[i]);
	}
#endif
	BKNI_LeaveCriticalSection();

	/* init VEC MISC block registers */
	BVDC_P_Vec_Init_Misc_isr(pVdc);

	return;
}

/*************************************************************************
 *  {secret}
 *	BVDC_P_HdmiRmTableEx_isr
 **************************************************************************/
static const BVDC_P_RateInfo* BVDC_P_HdmiRmTableEx_isr
(
	const BFMT_VideoInfo                *pFmtInfo,
	BAVC_HDMI_PixelRepetition eHdmiPixelRepetition,
	const BAVC_VdcDisplay_Info          *pRateInfo
)
{
	const BVDC_P_RateInfo *pRmInfo;

	BDBG_ENTER(BVDC_P_HdmiRmTableEx_isr);

	/* DVO master mode custom formats */
	if(BVDC_P_IS_CUSTOMFMT(pFmtInfo->eVideoFmt))
	{
		/* If rate table is not present it i's a single rate format.
		 * Assumptions multiple of 24/15hz are capable of multirate capable
		 * display. */
		bool bFullRate = (
			(0 == (pRateInfo->ulVertRefreshRate % (24 * BFMT_FREQ_FACTOR))) ||
			(0 == (pRateInfo->ulVertRefreshRate % (15 * BFMT_FREQ_FACTOR))))
			? true : false;

		/* User must also supply both rates */
		bFullRate &= (
			(pFmtInfo->pCustomInfo->pDvoRmTbl0) &&
			(pFmtInfo->pCustomInfo->pDvoRmTbl1));

		/* Use the detected source rate when applicable. */
		pRmInfo = (bFullRate)
			? pFmtInfo->pCustomInfo->pDvoRmTbl0
			: pFmtInfo->pCustomInfo->pDvoRmTbl1;
	}
	else
	{
		/* Lookup the correct table! */
		pRmInfo =
			BVDC_P_HdmiRmTable_isr(pFmtInfo->eVideoFmt,
				pRateInfo->ulPixelClkRate, 0, eHdmiPixelRepetition, 0);
	}

	BDBG_LEAVE(BVDC_P_HdmiRmTableEx_isr);
	return pRmInfo;
}

BERR_Code BVDC_P_Display_GetRasterLineNumber
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                        *pulRasterLineNumber )
{
    BREG_Handle hReg;

	BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
    hReg = hDisplay->hVdc->hRegister;

    *pulRasterLineNumber = BREG_Read32(hReg, hDisplay->ulItLctrReg);

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 * BVDC_P_Display_GetFieldPolarity_isr
 * Determines the next field polarity based on the LCNTR register of the given
 * timing generator for non-STG displays; otherwise, just use eFieldPolarity.
 * The polarity is then stored in BRDC_Variable_0.
 *
 ***************************************************************************/
BERR_Code BVDC_P_Display_GetFieldPolarity_isr
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                       **ppulRulCur,
	  BAVC_Polarity                    eFieldPolarity )
{
	uint32_t  *pulRulCur = *ppulRulCur;
    BERR_Code err = BERR_SUCCESS;

    if (BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg))
    {
        *pulRulCur++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_0);
        *pulRulCur++ = eFieldPolarity;
    }
    else
    {
        BFMT_VideoInfo videoFmtInfo;
        uint32_t ulFactor;

        videoFmtInfo = *((const BFMT_VideoInfo*)BFMT_GetVideoFormatInfoPtr_isr(hDisplay->stCurInfo.pFmtInfo->eVideoFmt));
        ulFactor = videoFmtInfo.ulScanHeight/2;

        /* for interlaced format, polarity_record = (bot)? 0 : 1 */
        *pulRulCur++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
        *pulRulCur++ = BRDC_REGISTER(hDisplay->ulItLctrReg);
        /* If LCNTR (trigger line) < scan height/2, top field; otherwise, bottom field.
         * Subtract LCNTR reg value from scan height/2 using 2's complement addition. If
         * LCNTR > scan height/2, the resulting 2's complement addition will
         * overflow, resulting in number that will have a 1 at the MSB. If LCNTR <
		 * scan height/2, the MSB will be 0. So we can use this MSB to determine whether
		 * it is a top (0) or bottom (1) field. */
        *pulRulCur++ = BRDC_OP_NOT_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1);

        *pulRulCur++ = BRDC_OP_VAR_SUM_IMM_TO_VAR(BRDC_Variable_1, BRDC_Variable_2);
        *pulRulCur++ = ulFactor + 1;

        *pulRulCur++ = BRDC_OP_VAR_AND_IMM_TO_VAR(BRDC_Variable_2, BRDC_Variable_1);
        *pulRulCur++ = 0x80000000;

        *pulRulCur++ = BRDC_OP_VAR_ROR_TO_VAR(BRDC_Variable_1, 31, BRDC_Variable_0);
    }

    /* reset RUL buffer pointer */
	*ppulRulCur = pulRulCur;

    return err;
}
