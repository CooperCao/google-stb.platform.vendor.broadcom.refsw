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

BDBG_MODULE(BVDC_DISP);


/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate RamTable for display modes, which
 *  supports Macrovision (480i,PAL,480p). Should never get here otherwise!
 **************************************************************************/
const uint32_t* BVDC_P_GetRamTable_isr
(
	const BVDC_P_DisplayInfo     *pDispInfo,
	bool                          bArib480p
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

#if !B_REFSW_MINIMAL
BERR_Code BVDC_P_ChangeMvType_isr
(
	BVDC_P_DisplayContext  *pDisplay,
	uint32_t              **ppulRul )
{
	BSTD_UNUSED(pDisplay);
	BSTD_UNUSED(ppulRul);
	return BERR_SUCCESS;
}
#endif

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
 *	Returns SEC_VF_POS_SYNC_VALUES value
 **************************************************************************/
uint32_t BVDC_P_GetPosSyncValue_isr
(
	BVDC_P_DisplayContext  *pDisplay,
	uint32_t              **ppulRul
)
{
	uint32_t    ulVfPosSync = 0;
	BSTD_UNUSED(ppulRul);
	BSTD_UNUSED(pDisplay);

	return(ulVfPosSync);
}


/*************************************************************************
 *  {secret}
 *	Returns SEC_VF_NEG_SYNC_VALUES value
 *	The amplitude of the neg sync pulses reduces for NTSC/PAL.
 *************************************************************************/
void BVDC_P_Macrovision_GetNegSyncValue_isr
	( BVDC_P_DisplayInfo              *pDispInfo,
	  BVDC_P_Output                    eOutputColorSpace,
	  bool                             bDacOutput_Green_NoSync,
	  uint32_t*                        ulRegVal,
	  uint32_t*                        ulRegValEx)
{
	uint32_t ulValue1, ulValue2;
	uint32_t ulValue;
	uint32_t ulValueEx;
	uint32_t ulValue0 = BVDC_P_NEG_SYNC_TIP_VALUE;

	BSTD_UNUSED(pDispInfo);

	if ((BVDC_P_Output_eSDRGB == eOutputColorSpace) && bDacOutput_Green_NoSync)
	{
		ulValue0 =
			BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(BVDC_P_DAC_OUTPUT_SYNC_LEVEL);
	}

	/* 525/60 CVBS/Svideo-Y outputs use 714/286 picture/sync ratio;
	   525/60 YPbPr/RGB and 625/50 outputs use 700/300 picture/sync ratio */
	if((BVDC_P_Output_eYQI	 == eOutputColorSpace) ||
	   (BVDC_P_Output_eYQI_M == eOutputColorSpace) ||
	   (BVDC_P_Output_eYUV_M == eOutputColorSpace) ||
	   (BVDC_P_Output_eYUV_N == eOutputColorSpace))
	{
		ulValue1 = BVDC_P_DAC_OUTPUT_NTSC_SYNC_LEVEL; /* 286 mv */
	}
	else
	{
		ulValue1 = BVDC_P_DAC_OUTPUT_SYNC_LEVEL;      /* 300 mv */
	}
	ulValue2 = ulValue1;

	/* Convert from voltage to register bits */
	ulValue1 = BVDC_P_NEG_SYNC_AMPLITUDE_VALUE (ulValue1);
	ulValue2 = BVDC_P_NEG_SYNC_AMPLITUDE_VALUE (ulValue2);

	/* Format for hardware registers */
#if (BVDC_P_SUPPORT_VEC_VF_VER < 2)
	ulValue =
		BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE2, ulValue2) |
		BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE1, ulValue1) |
		BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE0, ulValue0) ;
	ulValueEx = 0;
#else
	ulValue =
		BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE1, ulValue1) |
		BCHP_FIELD_DATA(VF_0_NEG_SYNC_VALUES, VALUE0, ulValue0) ;
	ulValueEx =
		BCHP_FIELD_DATA(VF_0_NEG_SYNC_AMPLITUDE_EXTN, VALUE2, ulValue2);
#endif

	/* Return computed values */
	*ulRegVal   = ulValue;
	*ulRegValEx = ulValueEx;
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
	uint32_t ulTable[BVDC_P_VF_TABLE_SIZE];

	BVDC_P_FillVfTable_isr(
		pDispInfo, BVDC_P_Output_eSDRGB, ulTable, NULL, NULL);

	return ulTable[0];
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

/* End of File */
