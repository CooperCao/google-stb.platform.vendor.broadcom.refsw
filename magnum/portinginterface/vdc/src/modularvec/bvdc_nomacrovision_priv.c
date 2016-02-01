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
 *		Private module for No Macrovision support. Do not include
 * bvdc_macrovision_priv.c, if this file is included in your project.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"          /* standard types */
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bchp_prim_vf.h"

BDBG_MODULE(BVDC_DISP);


/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate RamTable for display modes, which
 *  supports Macrovision (480i,PAL,480p). Should never get here otherwise!
 **************************************************************************/
const uint32_t* BVDC_P_GetRamTable_isr
(
	const BVDC_P_DisplayInfo    *pDispInfo,
	bool                         bArib480p
)
{
	return BVDC_P_GetRamTableSub_isr(pDispInfo, bArib480p);
}

/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate ItTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetItTable_isr
(
	const BVDC_P_DisplayInfo     *pDispInfo
)
{
	return BVDC_P_GetItTableSub_isr(pDispInfo);
}

BERR_Code BVDC_P_ChangeMvType_isr
(
	BVDC_P_DisplayContext  *pDisplay,
	uint32_t              **ppulRul )
{
	BSTD_UNUSED(pDisplay);
	BSTD_UNUSED(ppulRul);
	return BERR_SUCCESS;
}

/*************************************************************************
 *  {secret}
 **************************************************************************/
uint32_t BVDC_P_GetItConfig_isr
(
	const BVDC_P_DisplayInfo     *pDispInfo
)
{
	return BVDC_P_GetItConfigSub_isr(pDispInfo);
}

/*************************************************************************
 *  {secret}
 *	Just a stub here to match with bvdc_macrovision_priv.c
 **************************************************************************/
uint32_t BVDC_P_GetAgc
(
	BVDC_P_DisplayContext  *pDisplay,
	BFMT_VideoFmt           eVideoFmt,
	uint32_t              **ppRul,
	BVDC_P_VecPath          eVecPath
)
{
	BSTD_UNUSED(pDisplay);
	BSTD_UNUSED(eVideoFmt);
	BSTD_UNUSED(ppRul);
	BSTD_UNUSED(eVecPath);
	return 0;
}

/*************************************************************************
 *  {secret}
 *	Returns SEC_VF_NEG_SYNC_VALUES value
 *	Handle case of RGB with no sync on green channel.
 **************************************************************************/
uint32_t BVDC_P_Macrovision_GetNegSyncValue_isr
(
	BVDC_P_DisplayInfo     *pDispInfo,
	BVDC_P_Output           eOutputColorSpace,
	bool                    bDacOutput_Green_NoSync
)
{
	uint32_t ulValue;
	uint32_t ulValue0 = BVDC_P_NEG_SYNC_TIP_VALUE;

	BSTD_UNUSED(pDispInfo);

	if (((BVDC_P_Output_eSDRGB == eOutputColorSpace) ||
	     (BVDC_P_Output_eHDRGB == eOutputColorSpace))
		&& bDacOutput_Green_NoSync)
	{
		ulValue0 =
			BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(BVDC_P_DAC_OUTPUT_SYNC_LEVEL);
	}

    /* 525/60 CVBS/Svideo-Y outputs use 714/286 picture/sync ratio;
	   525/60 YPbPr/RGB and 625/50 outputs use 700/300 picture/sync ratio */
	if((BVDC_P_Output_eYQI   == eOutputColorSpace) ||
	   (BVDC_P_Output_eYQI_M == eOutputColorSpace) ||
	   (BVDC_P_Output_eYUV_M == eOutputColorSpace) ||
	   (BVDC_P_Output_eYUV_N == eOutputColorSpace))
	{
		ulValue =
			BCHP_FIELD_DATA(PRIM_VF_NEG_SYNC_VALUES, VALUE2,
				BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(BVDC_P_DAC_OUTPUT_NTSC_SYNC_LEVEL)) | /* 286 mv */
			BCHP_FIELD_DATA(PRIM_VF_NEG_SYNC_VALUES, VALUE1,
				BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(BVDC_P_DAC_OUTPUT_NTSC_SYNC_LEVEL)) | /* 286 mv */
			BCHP_FIELD_DATA(PRIM_VF_NEG_SYNC_VALUES, VALUE0, ulValue0);
	}
	else
	{
		ulValue = /* 700mV/300mV pix/sync ratio */
			(BCHP_FIELD_DATA(PRIM_VF_NEG_SYNC_VALUES, VALUE2,
				BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(BVDC_P_DAC_OUTPUT_SYNC_LEVEL)) | /* 300 mv */
			 BCHP_FIELD_DATA(PRIM_VF_NEG_SYNC_VALUES, VALUE1,
				BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(BVDC_P_DAC_OUTPUT_SYNC_LEVEL)) | /* 300 mv */
			 BCHP_FIELD_DATA(PRIM_VF_NEG_SYNC_VALUES, VALUE0, ulValue0));
	}

	return ulValue;
}

/*************************************************************************
 *  {secret}
 *	Returns SEC_VF_POS_SYNC_VALUES value
 **************************************************************************/
uint32_t BVDC_P_GetPosSyncValue_isr
(
	BVDC_P_DisplayContext  *pDisplay,
	uint32_t              **ppulRul,
	BVDC_P_VecPath          eVecPath
)
{
	uint32_t    ulVfPosSync;
	BVDC_P_DisplayInfo *pDispInfo = &pDisplay->stCurInfo;
	BSTD_UNUSED(ppulRul);
	BSTD_UNUSED(eVecPath);

	if (VIDEO_FORMAT_IS_HD(pDispInfo->pFmtInfo->eVideoFmt))
	{
		ulVfPosSync =
			BCHP_FIELD_DATA(PRIM_VF_POS_SYNC_VALUES, VALUE1, 0x0eb) |
			BCHP_FIELD_DATA(PRIM_VF_POS_SYNC_VALUES, VALUE0, 0x0eb);
	}
	else if (VIDEO_FORMAT_IS_625_LINES(pDispInfo->pFmtInfo->eVideoFmt))
	{
		ulVfPosSync =
			BCHP_FIELD_DATA(PRIM_VF_POS_SYNC_VALUES, VALUE1, 0x230) |
			BCHP_FIELD_DATA(PRIM_VF_POS_SYNC_VALUES, VALUE0, 0x296);
	}
	else
	{
		ulVfPosSync =
			BCHP_FIELD_DATA(PRIM_VF_POS_SYNC_VALUES, VALUE1, 0x230) |
			BCHP_FIELD_DATA(PRIM_VF_POS_SYNC_VALUES, VALUE0, 0x2b1);
	}

	return(ulVfPosSync);
}

/*************************************************************************
 *  {secret}
 *	Returns SEC_CO_VF_POS_SYNC_VALUES value
 **************************************************************************/
uint32_t BVDC_P_GetCoPosSyncValue_isr
(
	BVDC_P_DisplayContext  *pDisplay,
	uint32_t              **ppulRul,
	BVDC_P_VecPath          eVecPath
)
{
	return BVDC_P_GetPosSyncValue_isr (pDisplay, ppulRul, eVecPath);
}

/*************************************************************************
 *  {secret}
 *  This function is for SD RGB output only.
 *	Returns SEC_VF_FORMAT_ADDER value
 **************************************************************************/
uint32_t BVDC_P_GetFmtAdderValue_isr
(
	BVDC_P_DisplayInfo     *pDispInfo
)
{
	uint32_t aulTable[BVDC_P_VF_TABLE_SIZE];
	BVDC_P_FillVfTable_isr (
		pDispInfo, BVDC_P_Output_eSDRGB, &aulTable[0], NULL, NULL);
	return aulTable[0];
}

/*************************************************************************
 *  {secret}
 *  This function is to validate the macrovision settings.
 **************************************************************************/
BERR_Code BVDC_P_ValidateMacrovision
(
	BVDC_P_DisplayContext   *pDisplay
)
{
	BSTD_UNUSED(pDisplay);
	return BERR_SUCCESS;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Build_SyncPCL_isr
 *  Build the IT PCL_0 setting for h/v sync outputs;
 **************************************************************************/
void BVDC_P_Vec_Build_SyncPCL_isr
	( BVDC_P_DisplayContext           *pDisplay,
	  BVDC_P_ListInfo                 *pList )
{
	BDBG_ENTER(BVDC_P_Vec_Build_SyncPCL_isr);

	/* PR13177: For DTV application, RGB + Hsync are Dac outputs, Vsync is
	Ext digital output. */
	if(BVDC_P_Display_FindDac_isr(pDisplay, BVDC_DacOutput_eHsync))
	{
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_PCL_0 + pDisplay->lItOffset);
		*pList->pulCurrent++ =
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, VBI_DATA_ACTIVE_ENABLE,         ON) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, VBI_DATA_ACTIVE_MUX_SELECT ,    3) |
#if BVDC_P_SUPPORT_EXT_SYNC_PCL_0
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, EXT_VSYNC_ENABLE,               ON) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, EXT_VSYNC_MUX_SELECT,           0) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, EXT_HSYNC_ENABLE,               ON) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, EXT_HSYNC_MUX_SELECT,           0) |
#endif
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_4 ,            ZERO) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_3 ,            ZERO) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_2 ,            ZERO) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_1 ,            ZERO) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_0 ,            ONE) | /* take Hsync only */
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, NEGSYNC_MUX_E_SELECT,           0) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, NEGSYNC_MUX_D_SELECT,           0) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, NEGSYNC_MUX_4_SELECT,           0) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, NEGSYNC_MUX_3_SELECT,           0);
	}
	else /* normal setting */
	{
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_PRIM_IT_PCL_0 + pDisplay->lItOffset);
		*pList->pulCurrent++ =
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, VBI_DATA_ACTIVE_ENABLE,         ON) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, VBI_DATA_ACTIVE_MUX_SELECT ,    3) |
#if BVDC_P_SUPPORT_EXT_SYNC_PCL_0
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, EXT_VSYNC_ENABLE,               ON) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, EXT_VSYNC_MUX_SELECT,           0) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, EXT_HSYNC_ENABLE,               ON) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, EXT_HSYNC_MUX_SELECT,           0) |
#endif
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_4 ,            ZERO) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_3 ,            ZERO) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_2 ,            LINE_A) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_1 ,            LINE_B) |
			BCHP_FIELD_ENUM(PRIM_IT_PCL_0, NEGSYNC_AND_TERM_0 ,            LINE_C) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, NEGSYNC_MUX_E_SELECT,           0) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, NEGSYNC_MUX_D_SELECT,           3) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, NEGSYNC_MUX_4_SELECT,           0) |
			BCHP_FIELD_DATA(PRIM_IT_PCL_0, NEGSYNC_MUX_3_SELECT,           2);
	}

	BDBG_LEAVE(BVDC_P_Vec_Build_SyncPCL_isr);
	return;
}

BVDC_P_Prot_Alt BVDC_P_Get_Prot_Alt_isr (void)
{
	return BVDC_P_PROT_Alt_None;
}


uint32_t BVDC_P_Display_Modify_SYNC_TRANS_0_isr (
	BVDC_P_DisplayInfo               *pNewInfo,
	uint32_t                         ulVfSTRegVal)
{
	BSTD_UNUSED (pNewInfo);

	return ulVfSTRegVal;
}
#ifdef BVDC_P_DISPLAY_DUMP
void BVDC_Display_MV_DumpAll_aulVfTable (void)
{
}
void BVDC_Display_MV_DumpAll_aulItTable (void)
{
}
void BVDC_Display_MV_DumpAll_ulItConfig (void)
{
}
void BVDC_Display_MV_DumpTables (void)
{
}
#endif

/* End of File */
