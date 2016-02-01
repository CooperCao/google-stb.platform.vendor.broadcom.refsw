/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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

#include "bhdr.h"
#include "bhdr_priv.h"

#include "bhdr_fe.h"
#include "bhdr_fe_priv.h"

#include "bchp_hdmi_rx_fe_0.h"

#include "bhdm_cec_priv.h"


BDBG_MODULE(BHDR_FE) ;

#define	BHDR_CHECK_RC( rc, func )	          \
do                                                \
{										          \
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										      \
		goto done;							      \
	}										      \
} while(0)



static const BHDR_FE_Settings stDefaultFrontEndSettings =
{
	0  /* dummy */
} ;


static const BHDR_FE_ChannelSettings stDefaultFrontEndChannelSettings =
{
	0, /* FrontEndId */
	0 , /* uiCecAddr  */
	true, /* enable Equalizer */
	false, /* external switch */
	false, /* bHpdDisconnected; Hot Plug Detect is disconnected from 3549/56  */
} ;


typedef struct BHDR_FE_P_InterruptCbTable
{
	BINT_Id       IntrId;
	int               iParam2; 
	bool             enable ; /* debug purposes */
} BHDR_FE_P_InterruptCbTable ;


static const BHDR_FE_P_InterruptCbTable BHDR_FE_P_Intr[MAKE_INTR_ENUM(LAST)] =
{
	/* 15 */   { BCHP_INT_ID_DUPLICATE_FE_SELECT, BHDR_FE_INTR_eDUPLICATE_FE_SELECT, true},
	/* 16 */   { BCHP_INT_ID_MUX_0_IMPROPER_FE_SELECT_UPDATE, BHDR_FE_INTR_eMUX_0_IMPROPER_FE_SELECT_UPDATE, true},
	/* 17 */   { BCHP_INT_ID_MUX_1_IMPROPER_FE_SELECT_UPDATE, BHDR_FE_INTR_eMUX_1_IMPROPER_FE_SELECT_UPDATE, true},
} ;	



static const BHDR_FE_P_InterruptCbTable BHDR_FE_P_ChannelIntr0[MAKE_INTR_ENUM(LAST)] =
{
	/* 00 */   { BCHP_INT_ID_RX_HOTPLUG_UPDATE_0, BHDR_FE_CHN_INTR_eRX_HOTPLUG_UPDATE, true},
	/* 01 */   { BCHP_INT_ID_CEC_LOW_0, BHDR_FE_CHN_INTR_eCEC_LOW, true},
	/* 02 */   { BCHP_INT_ID_CEC_0, BHDR_FE_CHN_INTR_eCEC, true},
	/* 03 */   { BCHP_INT_ID_PLL_LOCK_0, BHDR_FE_CHN_INTR_ePLL_LOCK, true},
	/* 04 */   { BCHP_INT_ID_FREQ_CHANGE_0,  BHDR_FE_CHN_INTR_eFREQ_CHANGE, true},
} ;

static const BHDR_FE_P_InterruptCbTable BHDR_FE_P_ChannelIntr1[MAKE_INTR_ENUM(LAST)] =
{
	/* 05 */   { BCHP_INT_ID_RX_HOTPLUG_UPDATE_1, BHDR_FE_CHN_INTR_eRX_HOTPLUG_UPDATE, true},
	/* 06 */   { BCHP_INT_ID_CEC_LOW_1,  BHDR_FE_CHN_INTR_eCEC_LOW, true},
	/* 07 */   { BCHP_INT_ID_CEC_1, BHDR_FE_CHN_INTR_eCEC, true},
	/* 08 */   { BCHP_INT_ID_PLL_LOCK_1, BHDR_FE_CHN_INTR_ePLL_LOCK, true}, 
	/* 09 */   { BCHP_INT_ID_FREQ_CHANGE_1, BHDR_FE_CHN_INTR_eFREQ_CHANGE, true},
} ;	

static const BHDR_FE_P_InterruptCbTable *pInterrupts ;


typedef struct BHDR_FE_P_PLLSettingsLookupTable
{	
	uint8_t uiMinPixClock, uiMaxPixClock, uiP, uiN ;
} BHDR_FE_P_PLLSettingsLookupTable ;

static const  BHDR_FE_P_PLLSettingsLookupTable stPLLSettingsLookup[] =
{
	{ 0,  40, 1, 40},
	{ 41,  80, 2, 40},
	{ 81, 160, 4, 40},
	{161, 225, 6, 60}
} ;


static void BHDR_FE_P_InitializePhy(BHDR_FE_ChannelHandle hFeChannel) ;





/*******************************************************************************
BERR_Code BHDR_FE_GetDefaultSettings
Summary: Get the default settings for the HDMI device.
*******************************************************************************/
BERR_Code BHDR_FE_GetDefaultSettings(
	BCHP_Handle hChip,	
	BHDR_FE_Settings *pDefaultSettings  /* [in] pointer to memory to hold default settings */
) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(hChip) ;
	BDBG_ENTER(BHDR_FE_GetDefaultSettings) ;

	*pDefaultSettings = stDefaultFrontEndSettings ;

	BDBG_LEAVE(BHDR_FE_GetDefaultSettings) ;
	return rc ;
}


/******************************************************************************
BERR_Code BHDR_FE_Open
Summary: Open/Initialize the HDMI Rx Front End device .  The Front End can contain multiple channels
*******************************************************************************/
BERR_Code BHDR_FE_Open(
	BHDR_FE_Handle *phFrontEnd,       /* [in] HDMI Rx handle */
	BCHP_Handle hChip,                  /* Chip handle */
	BREG_Handle hRegister,              /* Register handle */
	BINT_Handle hInterrupt,             /* Interrupt handle */   
	const BHDR_FE_Settings  *pSettings /* [in] default HDMI settings */
) 
{
	BERR_Code      rc = BERR_SUCCESS;
	BHDR_FE_Handle hFrontEnd = NULL ;
	uint32_t Register ;
 	uint8_t i ;

	BDBG_ENTER(BHDR_FE_Open) ;

	/* verify parameters */
	BDBG_ASSERT( hChip );
	BDBG_ASSERT( hRegister );
	BDBG_ASSERT( hInterrupt );
	

#if BDBG_DEBUG_BUILD
	/* display version information */
	BDBG_MSG(("*****************************************")) ;
	BDBG_MSG(("HDMI Receiver Frontend ")) ;
	BDBG_MSG(("$brcm_Revision: $")) ;
 	BDBG_MSG(("Compiled: %s %s",  __DATE__, __TIME__)) ;
	BDBG_MSG(("*****************************************")) ;
#endif	

#if BHDR_POWER_MANAGEMENT
	BDBG_WRN(("Power Management is ENABLED")) ;
#else
	BDBG_WRN(("Power Management is DISABLED")) ;
#endif

	/* create HDMI Front End (port) handle */	
	hFrontEnd = (BHDR_FE_Handle) BKNI_Malloc(sizeof(BHDR_FE_P_Handle)) ;
	if (!hFrontEnd)
	{
		BDBG_ERR(("Unable to allocate memory for HDMI Front End (Port) Handle")) ;
		rc = BERR_OUT_OF_SYSTEM_MEMORY ;
		goto done ;
	}

	/* zero out memory associated with the HDMI Front End Handle before using */
	BKNI_Memset(hFrontEnd, 0, sizeof(BHDR_FE_P_Handle)) ;

	
	hFrontEnd->DeviceSettings = *pSettings ;

	hFrontEnd->hRegister = hRegister ;
	hFrontEnd->hInterrupt = hInterrupt ;
	hFrontEnd->hChip = hChip ;

	/* restore default front end configuration */	
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG) ;
	Register &= ~ BCHP_MASK(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;
	Register &= ~ BCHP_MASK(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;	
	Register |= 
		  BCHP_FIELD_DATA(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL, 0)
		|BCHP_FIELD_DATA(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL, 1) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG, Register) ;


	/* Take HDMI Rx Cores out of reset  */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_HDMI_RX_0_RESET_CONTROL) ;
	Register &= ~BCHP_MASK(DVP_HR_HDMI_RX_0_RESET_CONTROL, HDMI_RX_0_RESET) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_RX_0_RESET_CONTROL, Register) ;
	
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_HDMI_RX_FE_0_RESET_CONTROL) ;
	Register &= ~BCHP_MASK(DVP_HR_HDMI_RX_FE_0_RESET_CONTROL, HDMI_RX_FE_0_RESET) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_RX_FE_0_RESET_CONTROL, Register) ;
	
	/* Core 1 */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_HDMI_RX_1_RESET_CONTROL) ;
	Register &= ~BCHP_MASK(DVP_HR_HDMI_RX_1_RESET_CONTROL, HDMI_RX_1_RESET) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_RX_1_RESET_CONTROL, Register) ;	
	
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_HDMI_RX_FE_1_RESET_CONTROL) ;
	Register &= ~BCHP_MASK(DVP_HR_HDMI_RX_FE_1_RESET_CONTROL, HDMI_RX_FE_1_RESET) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_HDMI_RX_FE_1_RESET_CONTROL, Register) ;

	/* Create Events for use with Interrupts */

	BHDR_CHECK_RC(rc, BKNI_CreateEvent(
		&(hFrontEnd->BHDR_FE_Event_DUPLICATE_FE_SELECT))) ; 
	
	BHDR_CHECK_RC(rc, BKNI_CreateEvent(
		&(hFrontEnd->BHDR_FE_Event_MUX_0_IMPROPER_FE_SELECT_UPDATE))) ;
	
	BHDR_CHECK_RC(rc, BKNI_CreateEvent(
		&(hFrontEnd->BHDR_FE_Event_MUX_1_IMPROPER_FE_SELECT_UPDATE))) ;
	
	
	/* Register/enable interrupt callbacks */
	for (i = 0; i < MAKE_INTR_FE_ENUM(LAST) ; i++) 
	{
 		BHDR_CHECK_RC( rc, BINT_CreateCallback(	&(hFrontEnd->hCallback[i]), 
			hFrontEnd->hInterrupt, BHDR_FE_P_Intr[i].IntrId, 
			BHDR_FE_P_FrontEndIntr_isr, (void *) hFrontEnd, i ));
			
		/* clear interrupt callback */
		BHDR_CHECK_RC(rc, BINT_ClearCallback( hFrontEnd->hCallback[i])) ;

		/* skip interrupt if not enabled in table...  */	
		if (!BHDR_FE_P_Intr[i].enable)
			continue ;
			
		BHDR_CHECK_RC( rc, BINT_EnableCallback( hFrontEnd->hCallback[i] ) );
	}

	/* keep created pointer */
	*phFrontEnd = hFrontEnd ;

done:
	if( (rc != BERR_SUCCESS) && (hFrontEnd))
	{
		BKNI_Free(hFrontEnd) ;
		*phFrontEnd = NULL  ;
	}
	
	
	
	BDBG_LEAVE(BHDR_FE_Open) ;
	return rc ;
}



/***************************************************************************
BERR_Code BHDR_FE_Close
Summary: Close the Front End Channel 
****************************************************************************/
BERR_Code BHDR_FE_Close(BHDR_FE_Handle hFrontEnd) 
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;

	BDBG_ENTER(BHDR_FE_Close) ;
	BDBG_ASSERT(hFrontEnd) ;

	/* disable/destroy callbacks */
	for (i = 0; i < MAKE_INTR_FE_ENUM(LAST) ; i++) 
	{
 		BHDR_CHECK_RC(rc, BINT_DisableCallback(hFrontEnd->hCallback[i])) ;
			
		BHDR_CHECK_RC(rc, BINT_DestroyCallback(hFrontEnd->hCallback[i])) ;
	}	

	/* destroy events */
	BKNI_DestroyEvent(hFrontEnd->BHDR_FE_Event_DUPLICATE_FE_SELECT) ;
	BKNI_DestroyEvent(hFrontEnd->BHDR_FE_Event_MUX_0_IMPROPER_FE_SELECT_UPDATE) ;
	BKNI_DestroyEvent(hFrontEnd->BHDR_FE_Event_MUX_1_IMPROPER_FE_SELECT_UPDATE) ;
	

	/* free HDMI Front End (port) handle */
	BKNI_Free(hFrontEnd) ;
	hFrontEnd = (BHDR_FE_Handle) NULL ;


done:	
	BDBG_LEAVE(BHDR_FE_Close) ;
	return rc ;
}



/***************************************************************************
BERR_Code BHDR_FE_GetTotalChannels
Summary: Get the number of available channels for the Front End
****************************************************************************/
BERR_Code BHDR_FE_GetTotalChannels(
    BHDR_FE_Handle hFrontEnd, uint8_t *uiTotalChannels) 
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ASSERT(hFrontEnd) ;
	
	*uiTotalChannels = BHDR_FE_MAX_CHANNELS ;
	
	return rc ;
}



/*******************************************************************************
BERR_Code BHDR_GetDefaultSettings
Summary: Get the default settings for the HDMI device.
*******************************************************************************/
BERR_Code BHDR_FE_GetDefaultChannelSettings(
	BHDR_FE_Handle hFrontEnd, 
	BHDR_FE_ChannelSettings *pChSettings
) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(hFrontEnd) ;
	BDBG_ENTER(BHDR_FE_GetDefaultChannelSettings) ;

	*pChSettings = stDefaultFrontEndChannelSettings ;

	BDBG_LEAVE(BHDR_FE_GetDefaultChannelSettings) ;
	return rc ;
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_FE_OpenChannel(
	BHDR_FE_Handle hFrontEnd,       /* [in] HDMI Rx handle */
	BHDR_FE_ChannelHandle *phChannel, /* [out] Created channel handle */
	const BHDR_FE_ChannelSettings  *pChannelSettings /* [in] default HDMI settings */
) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BHDR_FE_ChannelHandle hFeChannel ;

	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint8_t i ;
	uint8_t  uiChannel ;
	
#if  BHDR_CONFIG_LOOP_FILTER_SETTINGS
	uint8_t 
		uiEqInputClkSampleNeg,
		uiLoopFilterHighThreshold, 
		uiLoopFilterLowThreshold ;
#endif

	BDBG_ASSERT(hFrontEnd) ;
	BDBG_ASSERT(pChannelSettings) ;
	BDBG_ENTER(BHDR_FE_OpenChannel) ;

	/* verify valid channel */
	uiChannel = pChannelSettings->uiChannel ;
	if (uiChannel >= BHDR_FE_P_eChannelMax)
	{
		rc = BHDR_INVALID_RESOURCE  ;
		goto done ;
	}

#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG
	BDBG_WRN(("HDMI_RX_FE_%d Block Configuration", uiChannel)) ;
	BDBG_WRN(("    Equalizer Enabled: %s", 
		pChannelSettings->bEnableEqualizer ? "YES" : "NO")) ;
	BDBG_WRN(("    External HDMI Switch: %s", 
		pChannelSettings->bExternalSwitch ? "YES" : "NO")) ; 
	BDBG_WRN(("    3549 HPD Pin Disconnected: %s", 
		pChannelSettings->bHpdDisconnected ? "YES" : "NO")) ;
#endif
	
	/* create/allocate HDMI Front End (port) handle */
	hFeChannel = (BHDR_FE_ChannelHandle) BKNI_Malloc(sizeof(BHDR_FE_P_ChannelHandle)) ;
	if (!hFeChannel)
	{
		BDBG_ERR(("Unable to allocate memory for HDMI Front End (Port) Handle")) ;
		rc = BERR_OUT_OF_SYSTEM_MEMORY ;
		goto done ;
	}

	/* zero out memory associated with the HDMI Front End Channel before using */
	BKNI_Memset(hFeChannel, 0, sizeof(BHDR_FE_P_ChannelHandle)) ;

	/* keep a copy of the opened channel settings */
	BKNI_Memcpy(&hFeChannel->settings, pChannelSettings, 
		sizeof(BHDR_FE_ChannelSettings)) ;		
	
	/* assign settings */
	hFeChannel->eChannel = uiChannel ;
	
	hFeChannel->ulOffset = ulOffset =  /* register offset */
		BHDR_FE_P_GET_REG_OFFSET(uiChannel) ;

	hRegister = hFeChannel->hRegister = hFrontEnd->hRegister ;

	/* Initialize/configure PHY */

	/* power up the analog core for use */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0 + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_0, REG_PWRDN) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0 + ulOffset, Register) ;
	
	/* disable hw phy reset controller */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_3 + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_3, 
		ENABLE_PHY_RESET_CONTROLLER) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_3 + ulOffset, Register) ;
	
	/* allow the Hot Plug to power down the PHY */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1 + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1,
		ALLOW_HOTPLUG_TO_POWER_DOWN_PHY) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1, 
		ALLOW_HOTPLUG_TO_POWER_DOWN_PHY, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_1 + ulOffset, Register) ;


	BHDR_FE_P_InitializePhy(hFeChannel) ;

	/* enable/disable equilzer as requested */
	BHDR_FE_EnableEqualization(hFeChannel, hFeChannel->settings.bEnableEqualizer) ;
	
	/* adjust defaults */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_LOOP_FILT_CTRL + ulOffset) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_LOOP_FILT_CTRL, bwsel_integ, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_LOOP_FILT_CTRL + ulOffset, Register) ;

	/* default for pll ctrl 0 */
	Register = 
		  BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, ANALOG_SPARE_5_0, 0)  /* 00 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, Rn_0, 0)                        /* 01 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, ANALOG_SPARE_5_1, 0) /* 02 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, BIASIN_EN, 1)		  /* 03 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, FREQ_DET_DIS, 0)         /* 04 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, ICPx, 0x1F)                   /* 05 - 09 */ 
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, Rz, 1)                            /* 10 - 12 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, Cz, 1)                            /* 13 - 14 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, Cp, 1)                            /* 15 - 16 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, Rn, 0)                            /* 17 - 18 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, Cn, 0)                            /* 19 - 20 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, LF_order, 0)                   /* 21 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, LFP_BW, 0)                    /* 22 - 23 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XF, 0)                 /* 24 - 26 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XS, 4)                 /* 29 - 27 */
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, ANALOG_SPARE_6, 0) ;    /* 30 - 37 */	
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_0 + ulOffset, Register) ;

#if  BHDR_CONFIG_LOOP_FILTER_SETTINGS
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_2 + ulOffset) ;
	Register &= ~ (
		  BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_2, LOOP_FILTER_PD_HIGH_THRESHOLD)
		| BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_2, LOOP_FILTER_PD_LOW_THRESHOLD)
		| BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_2, EQ_INPUT_CLK_SAMPLE_NEG)) ;
	
	uiEqInputClkSampleNeg = BHDR_CONFIG_EQ_INPUT_CLK_SAMPLE_NEG ;
	
	if (hFeChannel->settings.bExternalSwitch)
	{
		uiLoopFilterLowThreshold = 
			BHDR_CONFIG_LOOP_FILTER_PD_LOW_THRESHOLD_WITH_SWITCH ;
		uiLoopFilterHighThreshold =
			BHDR_CONFIG_LOOP_FILTER_PD_HIGH_THRESHOLD_WITH_SWITCH ;
	}
	else
	{
		uiLoopFilterLowThreshold = 
			BHDR_CONFIG_LOOP_FILTER_PD_LOW_THRESHOLD ;
		uiLoopFilterHighThreshold =
			BHDR_CONFIG_LOOP_FILTER_PD_HIGH_THRESHOLD ;
	}
	
	Register |= 
		  BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_2, 
		  	LOOP_FILTER_PD_HIGH_THRESHOLD, uiLoopFilterHighThreshold)
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_2, 
			LOOP_FILTER_PD_LOW_THRESHOLD, uiLoopFilterLowThreshold)
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_2, 
			EQ_INPUT_CLK_SAMPLE_NEG, uiEqInputClkSampleNeg) ;
	
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_2 + ulOffset, Register) ;
#endif

	/* adjust default HotPlug filter threshold */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, HOTPLUG_FILTER_THRESHOLD) ;
	Register |=  BCHP_FIELD_DATA(HDMI_RX_FE_0_HOTPLUG_CONTROL, HOTPLUG_FILTER_THRESHOLD, 0xFF) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL  + ulOffset, Register) ;
	

	/* initialize CEC core/support */
	BHDR_CEC_Initialize(hFeChannel) ;
	
	/* Register/enable interrupt callbacks for channel */
	switch (uiChannel)
	{
	case BHDR_FE_P_eChannel0 :
		pInterrupts = BHDR_FE_P_ChannelIntr0 ;
		break ;
	
	case BHDR_FE_P_eChannel1:
		pInterrupts = BHDR_FE_P_ChannelIntr1 ;
		break ;

	/* channel validity test done above.. should never execute here */
	default :
		pInterrupts = BHDR_FE_P_ChannelIntr0 ;
		BDBG_ERR(("Unknown Front End Channel ID %d", uiChannel)) ;
		BDBG_ASSERT(false) ;
	}
	
	for (i = 0; i < MAKE_INTR_FE_CHN_ENUM(LAST) ; i++) 
	{
 		BHDR_CHECK_RC( rc, BINT_CreateCallback(	&(hFeChannel->hCallback[i]), 
			hFrontEnd->hInterrupt, pInterrupts[i].IntrId, 
			BHDR_FE_P_ChannelIntr_isr, (void *) hFeChannel, i ));
			
		/* clear interrupt callback */
		BHDR_CHECK_RC(rc, BINT_ClearCallback( hFeChannel->hCallback[i])) ;

		/* skip interrupt if not enabled in table...  */	
		if (!pInterrupts[i].enable)
			continue ;

		/* enable interrupts if HPD signal is connected  */
		/* i.e. direct connection or switch with HPD connected */
		if (!hFeChannel->settings.bHpdDisconnected)
		{
			BHDR_CHECK_RC( rc, BINT_EnableCallback( hFeChannel->hCallback[i] )) ;
		}
	}

#if BHDR_CONFIG_HOTPLUG_PAD	
	/* Enable the internal hot plug pullup */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_PAD_CONTROL + ulOffset) ;
	Register |= BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_PAD_CONTROL, HOTPLUG_OUT_PUP) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_PAD_CONTROL  + ulOffset, Register) ;
#else
	BDBG_WRN(("*******************************************************")) ;
	BDBG_WRN(("HDMI Input may not work if Hot Plug Pad ECO has not been applied")) ;
	BDBG_WRN(("********************************************************")) ;
#endif	
	

	/* check if a device is attached and init the pixel count; ensures an update message */
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_STATUS + ulOffset) ;
	hFeChannel->bTxDeviceAttached = BCHP_GET_FIELD_DATA(Register, 
			HDMI_RX_FE_0_HOTPLUG_STATUS, RX_HOTPLUG_IN) ;

	hFeChannel->bPllLocked = false ;
	hFeChannel->PreviousbPllLocked = true ;
	hFeChannel->PreviousPixelClockCount = 1000 ;
	
	/* keep created Front End Channel and its settings */
	*phChannel = hFeChannel ;

	/* default HDMI Channel Power Settings to on */
	hFeChannel->stPowerSettings.bCecPowered = true ;
	hFeChannel->stPowerSettings.bHdmiRxPowered = true ;
	
		
done:
	BDBG_LEAVE(BHDR_FE_OpenChannel) ;
	return rc ;
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_FE_CloseChannel(
	BHDR_FE_ChannelHandle hFeChannel /* Created channel handle */
) 
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;

	BDBG_ENTER(BHDR_FE_CloseChannel) ;
	BDBG_ASSERT(hFeChannel) ;

	/* disable/destroy callbacks */
	for (i = 0; i < MAKE_INTR_FE_CHN_ENUM(LAST) ; i++) 
	{
 		BHDR_CHECK_RC( rc, BINT_DisableCallback(hFeChannel->hCallback[i])) ;
			
		BHDR_CHECK_RC(rc, BINT_DestroyCallback( hFeChannel->hCallback[i])) ;
	}	

	/* free HDMI Front End (port) handle */
	BKNI_Free(hFeChannel) ;
	hFeChannel = (BHDR_FE_ChannelHandle) NULL ;

done:
	BDBG_LEAVE(BHDR_FE_CloseChannel) ;
	return rc ;
}


/******************************************************************************
Summary: Software control of Phy Initialzation
*******************************************************************************/
static void BHDR_FE_P_InitializePhy(
	BHDR_FE_ChannelHandle hFeChannel) /* [out] Created channel handle */
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint8_t i ;
	bool bCalibrationDone = 0 ;
	
	BDBG_ENTER(BHDR_FE_P_InitializePhy) ;
	BDBG_ASSERT(hFeChannel) ;

	hRegister = hFeChannel->hRegister ;
	ulOffset = hFeChannel->ulOffset ;


	/* take phy out of Reset */
	Register = BREG_Read32(hRegister, 
		BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_3 + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_3,
		ENABLE_PHY_RESET_CONTROLLER) ;
	BREG_Write32(hRegister, 
		BCHP_HDMI_RX_FE_0_HDMI_RX_PHY_RESET_AND_POWER_CFG_3 + ulOffset, Register) ;

		
	/* power down the circuit */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset) ;	
	Register |= BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, BIAS_PD) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;

	/* pwr down analog channels */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;	
	Register |=
		 ( BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_POWER_DOWN) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_POWER_DOWN) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_POWER_DOWN)
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_POWER_DOWN)) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;


	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset) ;	
	Register |= BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, PD_DVICLK) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;

	
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;	
	Register &=
		 ~( BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET)) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;


	/* set up for CEC resistor calibration */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset) ;	
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, CAL50_26KB) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;
	
	
	/* Start Initialization */
	
	/* power up the bias circuit */
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, BIAS_PD) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;

	BKNI_Delay(10) ;

	/****************************/
	/* start CEC calibration  */
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, STRTCNTRB) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;
	
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_ANALOG_CFG_1, STRTCNTRB, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;

	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, STRTCNTRB) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;

	i = 1 ;
	do
	{

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_STATUS_1 + ulOffset) ;
		bCalibrationDone = BCHP_FIELD_DATA(HDMI_RX_FE_0_ANALOG_STATUS_1, CAL_DONE, Register) ;
		if (bCalibrationDone)
			break ;
		BKNI_Delay(1000) ;
		i++ ;
		
	} while (i < 10) ;

	if (!bCalibrationDone)
	{
		BDBG_WRN(("HDMI_RX_FE_%d CEC Resistor Calibration Timed Out!!!", 
			hFeChannel->eChannel)) ;
	}
	else
	{
		BDBG_MSG(("HDMI_RX_FE_%d CEC Resistor Calibration FINISHED in <= %d us ", 
			hFeChannel->eChannel, i * 1000)) ;
	}
		

	/****************************/
	/* start TMDS resistor calibration  */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset) ;
	Register |= BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, STRTCNTRB) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;
	
	Register |= BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, CAL50_26KB) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;
	
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, STRTCNTRB) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;


	i = 1 ;
	do
	{
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_STATUS_1 + ulOffset) ;
		bCalibrationDone = BCHP_FIELD_DATA(HDMI_RX_FE_0_ANALOG_STATUS_1, CAL_DONE, Register) ;
		if (bCalibrationDone)
			break ;
		BKNI_Delay(1000) ;
		i++ ;
		
	} while (i < 10) ;

	if (!bCalibrationDone)
	{
		BDBG_WRN(("HDMI_RX_FE_%d TMDS Calibration Timed Out!!!", 
			hFeChannel->eChannel)) ;
	}
	else
	{
		BDBG_MSG(("HDMI_RX_FE_%d TMDS Calibration FINISHED in <= %d us", 
			hFeChannel->eChannel, i * 1000)) ;
	}
		

	
	/* Power Up PLL */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;	
	Register &= ~BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_POWER_DOWN) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset) ;	
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, PD_DVICLK) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;

		
	BKNI_Delay(10) ;  

	/* reset pll */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;	
	
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_RESET, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;
		
	Register &= ~BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_RESET) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;


	/* pwr up analog channels */
	Register &=
		~( BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_POWER_DOWN) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_POWER_DOWN) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_POWER_DOWN)) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;

	BKNI_Delay(10) ;

	/* un reset the analog channels */
	Register |=
		  BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET, 1) 
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET, 1) 
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;

	BKNI_Delay(10) ;
	
	Register &=
		~( BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET)
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, PIX_CLK_EST_PIX_CLK_RESET)
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, PIX_CLK_EST_SYS_CLK_RESET)
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, RESET_HDMI_FE)		
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, RESET_INPUT_FIFO)) ;

	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;
	BDBG_LEAVE(BHDR_FE_P_InitializePhy) ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_SetHotPlug(BHDR_Handle hHDR, BHDR_HotPlugSignal eHotPlugSignal)           
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset  ;
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ASSERT(hHDR) ;
	hRegister = hHDR->hRegister ;

	/* get offset for Front End */
	ulOffset = hHDR->hFeChannel->ulOffset ;
	
	Register = BREG_Read32(hRegister, 	BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulOffset) ;

	if (eHotPlugSignal == BHDR_HotPlugSignal_eHigh)
	{
		BDBG_MSG(("HDMI_RX_FE_%d Force Hot Plug High (CONNECT Device)... ", hHDR->eCoreId)) ;
		Register &= ~BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, RX_HOTPLUG_OUT_FORCE_LOW) ; /* 0 */
		BKNI_EnterCriticalSection() ;
			BHDR_P_HotPlugConnect_isr(hHDR) ;
		BKNI_LeaveCriticalSection() ;
	}
	else
	{
		BDBG_MSG(("HDMI_RX_FE_%d Force Hot Plug Low (REMOVE Device)... ", hHDR->eCoreId)) ;
		Register |= BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, RX_HOTPLUG_OUT_FORCE_LOW) ; /* 1 */
		BKNI_EnterCriticalSection() ;
			BHDR_P_HotPlugRemove_isr(hHDR) ;
		BKNI_LeaveCriticalSection() ;
	}
	
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulOffset, Register) ;

	return rc ;
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_PulseHotPlug(BHDR_Handle hHDR)
{
	BREG_Handle  hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	
	BERR_Code rc = BERR_SUCCESS ;
	
	BDBG_ASSERT(hHDR) ;
	hRegister =hHDR->hRegister ;
	
	/* get offset for Front End Channel in use */
	ulOffset = hHDR->hFeChannel->ulOffset ;
		
	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL  + ulOffset) ;
	Register |= BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, RX_HOTPLUG_OUT_FORCE_LOW) ; /* 1 */
	BREG_Write32(hHDR->hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulOffset, Register) ;
	
	BKNI_Delay(2000) ;

	Register &= ~BCHP_MASK(HDMI_RX_FE_0_HOTPLUG_CONTROL, RX_HOTPLUG_OUT_FORCE_LOW) ; /* 0 */
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_CONTROL + ulOffset, Register) ;
	
	return rc ;
}



/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_FE_P_GetPllLockStatus_isr(BHDR_FE_ChannelHandle hFeChannel, bool *bLocked) 
{
	BREG_Handle hRegister  ;
	uint32_t ulOffset ;
	uint32_t Register ;
	
	uint8_t PllLock ;
	
	BDBG_ASSERT(hFeChannel) ;
	hRegister = hFeChannel->hRegister ;
	/* get offset for Front End */
	ulOffset = hFeChannel->ulOffset ;

	Register = BREG_Read32( hRegister, BCHP_HDMI_RX_FE_0_ANALOG_STATUS_2 + ulOffset) ;
	PllLock =	BCHP_GET_FIELD_DATA(Register, HDMI_RX_FE_0_ANALOG_STATUS_2, PLL_LOCK) ;
	hFeChannel->bPllLocked = PllLock ;

	*bLocked = false ;
	if (PllLock)
		*bLocked = true ;
}



/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_FE_P_GetPixelClockEstimate_isr(BHDR_FE_ChannelHandle hFeChannel,		
	uint32_t *EstimatedPixelClockRate 
)
{
	BREG_Handle hRegister ; 
	uint32_t ulOffset  ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t PixelClockCount ;
	uint32_t Register ;
	bool bPixelClockChange ;
	bool bPllLocked ;

	if (!hFeChannel)
		return BERR_NOT_INITIALIZED ;
	
	hRegister= hFeChannel->hRegister ;
	/* get offset for Front End */
	ulOffset = hFeChannel->ulOffset ;

	*EstimatedPixelClockRate = 0 ;

	/* first check if device is attached (or alternatively) if the PLL is locked... */
	BHDR_FE_P_GetPllLockStatus_isr(hFeChannel, &bPllLocked) ;
	
	/* warn (only once) if connected device is not active  */
	if (bPllLocked != hFeChannel->PreviousbPllLocked) 
	{
		if (!bPllLocked)
		{
			BDBG_WRN(("HDMI_RX_FE_%d Not Locked / Attached", hFeChannel->eChannel)) ;
			goto done ;
		}
	}
	
	if (!bPllLocked)
		goto done ;

	/* Now read the pixel count register and note changes */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PIX_CLK_CNT + ulOffset) ;
	PixelClockCount = BCHP_GET_FIELD_DATA(Register,  HDMI_RX_FE_0_PIX_CLK_CNT, PIX_CLK_CNT) ;
	
	if ((hFeChannel->PreviousPixelClockCount > PixelClockCount + 5) 
	|| (hFeChannel->PreviousPixelClockCount + 5 < PixelClockCount ))
	{
		bPixelClockChange = true ;
		hFeChannel->PreviousPixelClockCount = PixelClockCount ;
	}
	else
		bPixelClockChange = false ;

	/* warn (only once); Tx device is attached, but probably stopped transmitting */
	if ((bPixelClockChange) && (!PixelClockCount))
	{
		BDBG_WRN(("HDMI_RX_FE_%d Pixel Clock Count is 0; Unable to calculate pixel clock...",
			hFeChannel->eChannel)) ;
		goto done ;
	}


	/* do not calculate Pixel Clock if count = 0 */
	if (!PixelClockCount)
	{
		goto done ;
	}

	/* Calculate the pixel clock rate */
	*EstimatedPixelClockRate =  108000 * 1024 / PixelClockCount ;


	/* adjust PLL settings based on Pixel Clock */		
	if (bPixelClockChange)
	{
		uint8_t uiKvco_xs ;
		uint8_t uiKvco_xf ;
		uint8_t uiICPx  ;
		uint8_t index, uiFin ;
		uint32_t VCO, uiPixClock ;

		index = 0 ;
		uiPixClock = *EstimatedPixelClockRate / 1000 ;

		/* check if Pixel Clock Calculation result > 0 */
		if (!uiPixClock)
		{
			BDBG_WRN(("Calculated Pixel Clock Rate of 0...")) ;
			goto done ;
		}

		while (stPLLSettingsLookup[index].uiMaxPixClock)
		{
			if ((uiPixClock  >= stPLLSettingsLookup[index].uiMinPixClock)
			&& (uiPixClock <= stPLLSettingsLookup[index].uiMaxPixClock))
				break ;
			index++ ;
		}
	
		/* check if Pixel Clock is within PLL Setting table */
		/* use uiFin as temporary variable */
		uiFin = sizeof(stPLLSettingsLookup) / sizeof(*stPLLSettingsLookup) ;
		if (index >= uiFin)
		{
			BDBG_WRN(("Pixel Freq > 225; adjusting PLL for 225")) ;
			index = uiFin - 1 ;
		}

		uiFin = uiPixClock  / stPLLSettingsLookup[index].uiP ;
		VCO = uiFin * stPLLSettingsLookup[index].uiN ;

		BDBG_MSG(("%d <= %d <=%d; P= %d, N= %d",
			stPLLSettingsLookup[index].uiMinPixClock,
			uiPixClock,
			stPLLSettingsLookup[index].uiMaxPixClock,
			stPLLSettingsLookup[index].uiP,
			stPLLSettingsLookup[index].uiN)) ;

		BDBG_MSG(("Fin = %d  VCO = %d", uiFin, VCO)) ;
		
		
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_0 + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XS) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XF) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_0, ICPx) ;

	if ((VCO >= 1600) && (VCO <= 1650)) 
	{
		uiICPx = 0x10 ;
		uiKvco_xs = 4 ;
		uiKvco_xf = 4 ;
	}
	else if (VCO < 1600)
	{
		uiKvco_xs = 4 ;
		uiKvco_xf = 4 ;
		uiICPx = 0x1F ;
	}
	else
	{
		uiKvco_xs = 7 ;
		uiKvco_xf = 7 ;
		uiICPx = 0x8 ;
	}

	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XS, uiKvco_xs) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, KVCO_XF, uiKvco_xf) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_0, ICPx, uiICPx);	
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_0 + ulOffset, Register) ;

#if BHDR_CONFIG_DEBUG_INPUT_SIGNAL
	BDBG_WRN(("VCO: %d Xs: %d; Xf: %d; ICPx %#x", 
		VCO, uiKvco_xs, uiKvco_xf, uiICPx)) ;
#endif
	}
	
#if BDBG_DEBUG_BUILD
	/* Inform of changes */
	if (bPixelClockChange)
	{
		BDBG_WRN(("HDMI_RX_FE_%d Estimated Pixel (%d) Clock Rate: %d MHz; ", 
			hFeChannel->eChannel,
			PixelClockCount, *EstimatedPixelClockRate)) ;
	}
#endif

done:
	hFeChannel->PreviousbPllLocked = bPllLocked ;
	return rc ;
}



/******************************************************************************
void BHDR_FE_P_FrontEndIntr_isr
Summary: Handle interrupts from the HDMI core.
*******************************************************************************/
void BHDR_FE_P_FrontEndIntr_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] not used */
)
{
	BHDR_FE_ChannelHandle hFeChannel  ;
	hFeChannel = (BHDR_FE_ChannelHandle) pParam1 ;
	BDBG_ASSERT(hFeChannel) ;

	
	switch (parm2)
	{
	case MAKE_INTR_FE_ENUM(DUPLICATE_FE_SELECT) :
		BDBG_WRN(("3D Configuration: Multiple HDMI Rx assigned the same HDMI RX FE")) ;
		break ;

	case MAKE_INTR_FE_ENUM(MUX_0_IMPROPER_FE_SELECT_UPDATE) : 	
	case MAKE_INTR_FE_ENUM(MUX_1_IMPROPER_FE_SELECT_UPDATE) : 
#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG		
	{
		uint32_t Register ;
		uint8_t uiCore0_AssignedFe ;
		uint8_t uiCore1_AssignedFe ;
		
		Register = BREG_Read32(hFeChannel->hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG) ;
		uiCore0_AssignedFe = 
			BCHP_GET_FIELD_DATA(Register, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;		
		uiCore1_AssignedFe = 
			BCHP_GET_FIELD_DATA(Register, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;

		if ((uiCore0_AssignedFe != 0) || (uiCore1_AssignedFe != 1))
		{
			BDBG_WRN(("Special HDMI Rx Frontend Configuration")) ;
			BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_1", uiCore1_AssignedFe)) ;
			BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_0", uiCore0_AssignedFe)) ;
		}
		else
		{
			BDBG_WRN(("Normal HDMI Rx Frontend Configuration")) ;
		}
	}
#endif
		break ;
		
	default	:
		BDBG_ERR(("Unknown Interrupt ID:%d", parm2)) ;
	} ;		

	/* L2 interrupts are reset automatically */
}


static void BHDR_FE_P_ClearHdcpAuthentication_isr(BHDR_FE_ChannelHandle hFeChannel) 
{
	BREG_Handle hRegister ; 
	uint32_t ulOffset, Register  ;
	
	BDBG_ASSERT(hFeChannel) ;	
	hRegister= hFeChannel->hRegister ;

	ulOffset = hFeChannel->ulHdrOffset ;

	/* clear HDCP authenticated status in HDMI Rx Core */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG + ulOffset) ;

	Register &= ~ BCHP_MASK(HDMI_RX_0_HDCP_DEBUG, CLEAR_RX_AUTHENTICATED_P) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG + ulOffset , Register) ;
	
	Register |= BCHP_MASK(HDMI_RX_0_HDCP_DEBUG, CLEAR_RX_AUTHENTICATED_P) ; 
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_HDCP_DEBUG  + ulOffset , Register) ;
}


void BHDR_FE_P_ResetPll_isr(BHDR_FE_ChannelHandle hFeChannel) 
{

	BREG_Handle hRegister ; 
	uint32_t ulOffset, Register  ;
	
	BDBG_ASSERT(hFeChannel) ;
	hRegister= hFeChannel->hRegister ;
	/* get offset for Front End */
	ulOffset = hFeChannel->ulOffset ;

	/* pwr up analog channels */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;
	Register &=
		~( BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_POWER_DOWN) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_POWER_DOWN) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_POWER_DOWN)) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;

	BKNI_Delay(10) ;


	/* reset pll */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_RESET, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;
		
	Register &= ~BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_RESET) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;
#if 0			
	BKNI_Delay(1000) ; /* waiting for PLL Lock */

	/* un reset the analog channels */
	Register |=
		  BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET, 1) 
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET, 1) 
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;
	Register &=
		~( BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET) 
		  | BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET)) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;
#endif	
}



/******************************************************************************
void BHDR_FE_P_ChannelIntr_isr
Summary: Handle interrupts from the HDMI core.
*******************************************************************************/
void BHDR_FE_P_ChannelIntr_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] Interrupt ID */
)
{
	BHDR_FE_ChannelHandle hFeChannel  ;
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset   ;

	hFeChannel = (BHDR_FE_ChannelHandle) pParam1 ;
	BDBG_ASSERT(hFeChannel) ;

	hRegister = hFeChannel->hRegister ;

	/* get offset for Front End */
	ulOffset = hFeChannel->ulOffset ;

	
	switch (parm2)
	{
	case MAKE_INTR_FE_CHN_ENUM(RX_HOTPLUG_UPDATE) : 
	{
#define BHDR_P_HPD_SAMPLES 13
		uint8_t i, HpdStatus, HpdHigh, HpdLow, HighLowSamples[BHDR_P_HPD_SAMPLES] ;
		HpdHigh = HpdLow = 0 ;				
		for (i = 0 ; i < BHDR_P_HPD_SAMPLES; i++)
		{
			BKNI_Delay(50) ;
			
			Register = BREG_Read32( hRegister, BCHP_HDMI_RX_FE_0_HOTPLUG_STATUS + ulOffset) ;
			HpdStatus = BCHP_GET_FIELD_DATA(Register, 
					HDMI_RX_FE_0_HOTPLUG_STATUS, RX_HOTPLUG_IN) ;

			if (HpdStatus)
				HpdHigh++ ;
			else
				HpdLow++ ;
			HighLowSamples[i] = HpdStatus ;
		}

		if (HpdHigh > HpdLow)
			hFeChannel->bTxDeviceAttached = 1 ;
		else
			hFeChannel->bTxDeviceAttached = 0 ;
		
		BDBG_WRN(("HDMI_RX_FE_%d Hotplug; Connected: %d", 
			hFeChannel->eChannel, hFeChannel->bTxDeviceAttached)) ;

		/* report on inconsistent readings */		
		if ((HpdHigh) && (HpdLow))
		{
			BDBG_WRN(("_______________________________________________________")) ;
			BDBG_WRN(("HPD Status Samples: %d %d %d %d %d %d %d %d %d %d %d %d %d ", 
				HighLowSamples[0], HighLowSamples[1], HighLowSamples[2], 	
				HighLowSamples[3], HighLowSamples[4], HighLowSamples[5], 
				HighLowSamples[6], HighLowSamples[7], HighLowSamples[8], 
				HighLowSamples[9], HighLowSamples[10], HighLowSamples[11], 
				HighLowSamples[12])) ; 
			BDBG_WRN(("_______________________________________________________")) ;
		}
		
		if (hFeChannel->bTxDeviceAttached)
		{
			/* reset pll */
			BHDR_FE_P_ResetPll_isr(hFeChannel) ;
		}
		
		
		/* inform higher level of Connect/Disconnect interrupt */
		if (hFeChannel->pfHotPlugCallback_isr)
		{
			hFeChannel->pfHotPlugCallback_isr(hFeChannel->pvHotPlugParm1, 
				hFeChannel->iHotPlugParm2, &hFeChannel->bTxDeviceAttached) ;
		}
		else
		{
			BDBG_WRN(("No HotPlug callback installed...")) ;
		}

		Register = BREG_Read32(hRegister, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL) ;
		Register |= BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, HOTPLUG_CLR_INTERRUPT_DET) ;
		BREG_Write32(hRegister, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, Register) ;


		Register &= ~BCHP_MASK(DVP_HR_CEC_ENERGYSTAR_CNTRL, HOTPLUG_CLR_INTERRUPT_DET) ;
		BREG_Write32(hRegister, BCHP_DVP_HR_CEC_ENERGYSTAR_CNTRL, Register) ;

		/* clear HDCP authenticated status in HDMI Rx Core */
		BHDR_FE_P_ClearHdcpAuthentication_isr(hFeChannel) ;
		hFeChannel->cecConfiguration.FirstCecMessage = true ;
	
		break ;
	}

			
	case MAKE_INTR_FE_CHN_ENUM(CEC_LOW) :
		break ;
		    
	case MAKE_INTR_FE_CHN_ENUM(CEC) :
		BDBG_MSG(("HDMI_RX_FE_%d CEC Interrupt", hFeChannel->eChannel)) ;
		BHDR_FE_P_CEC_isr(hFeChannel) ;
		break ;
		
		
	case MAKE_INTR_FE_CHN_ENUM(PLL_LOCK) :   
		/* read the lock status directly here */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_STATUS_2 + ulOffset) ;
		hFeChannel->bPllLocked = (bool) BCHP_GET_FIELD_DATA(
			Register, HDMI_RX_FE_0_ANALOG_STATUS_2, PLL_LOCK) ;
		
		BDBG_MSG(("HDMI_RX_FE_%d PLL Lock: %d", 
			hFeChannel->eChannel, hFeChannel->bPllLocked)) ;
		
		/* clear HDCP authenticated status in HDMI Rx Core */
		BHDR_FE_P_ClearHdcpAuthentication_isr(hFeChannel) ;

		break ;
		
	case MAKE_INTR_FE_CHN_ENUM(FREQ_CHANGE) : 
		BDBG_MSG(("HDMI_RX_FE_%d Frequency Changed", hFeChannel->eChannel)) ;		
		BHDR_FE_P_ResetPll_isr(hFeChannel) ;
		
		/* clear HDCP authenticated status in HDMI Rx Core */
		BHDR_FE_P_ClearHdcpAuthentication_isr(hFeChannel) ;
		
		break ;

	default	:
		BDBG_ERR(("Unknown Interrupt ID:%d", parm2)) ;
	} ;		

	/* L2 interrupts are reset automatically */
}



/******************************************************************************
Summary: Attach the HDMI Rx Device to a Front End Channel 
*******************************************************************************/
BERR_Code BHDR_FE_AttachHdmiRxCore(
	BHDR_FE_ChannelHandle hFeChannel,
	BHDR_Handle hHDR)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	
	uint32_t RegValue ;
	BHDR_FE_P_Channel eNewFeChannel ;

	BHDR_P_HdrCoreId eCoreId ;
	
	uint8_t 
		uiCore0_AssignedFe, 
		uiCore1_AssignedFe ;

	BDBG_ENTER(BHDR_FE_AttachHdmiRxCore) ;
	
	BDBG_ASSERT(hHDR) ;
	BDBG_ASSERT(hFeChannel) ;

	hRegister = hFeChannel->hRegister ;

	/* verify specified channel does not exceed max channels */
	eNewFeChannel = hFeChannel->eChannel ;
	if (eNewFeChannel >= BHDR_FE_P_eChannelMax)
	{
		rc = BERR_INVALID_PARAMETER ;
		goto done ;
	}
	
	eCoreId = hHDR->eCoreId ;
	if (eCoreId >= BHDR_P_eHdrCoreIdMax)
	{
		BDBG_ERR(("Unknown HDMI Rx Core ID %d", eCoreId)) ;	
		goto done ;
	}

	/* verify change in FE channel configuration */
	
	RegValue = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG) ;
	uiCore0_AssignedFe = 
		BCHP_GET_FIELD_DATA(RegValue, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;		
	uiCore1_AssignedFe = 
		BCHP_GET_FIELD_DATA(RegValue, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;

#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG
	BDBG_WRN(("ATTACH HDMI_RX_%d to HDMI_RX_FE_%d", eCoreId, eNewFeChannel)) ;
	
	BDBG_WRN(("OLD DVP HR Config: ")) ;
	BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_1", uiCore1_AssignedFe)) ;
	BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_0", uiCore0_AssignedFe)) ;
#endif

	RegValue &= ~ BCHP_MASK(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;
	RegValue &= ~ BCHP_MASK(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;

	/* reconfigure Core to use new Front End */
	switch (eCoreId)
	{
	case BHDR_P_eHdrCoreId0 : 
		RegValue &= ~ BCHP_MASK(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;
		RegValue |= BCHP_FIELD_DATA(DVP_HR_DVP_HR_DATA_PATH_CFG, 
			HDMI_RX_0_SEL, eNewFeChannel) ;
		RegValue |= BCHP_FIELD_DATA(DVP_HR_DVP_HR_DATA_PATH_CFG, 
			HDMI_RX_1_SEL, uiCore1_AssignedFe) ;
		break ;
		
	case BHDR_P_eHdrCoreId1 : 
		RegValue &= ~ BCHP_MASK(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;
		RegValue |= BCHP_FIELD_DATA(DVP_HR_DVP_HR_DATA_PATH_CFG, 
			HDMI_RX_1_SEL, eNewFeChannel) ;
		RegValue |= BCHP_FIELD_DATA(DVP_HR_DVP_HR_DATA_PATH_CFG, 
			HDMI_RX_0_SEL, uiCore0_AssignedFe) ;
		break ;
		
	default :
		BDBG_ERR(("Unknown Channel ID %d", eNewFeChannel)) ;	
		break ;
	}

	/* assign new FrontEnd to HDMI Rx core */
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG, RegValue) ;


	RegValue = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG) ;
	uiCore0_AssignedFe = 
		BCHP_GET_FIELD_DATA(RegValue, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;		
	uiCore1_AssignedFe = 
		BCHP_GET_FIELD_DATA(RegValue, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;

#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG
	BDBG_WRN(("NEW DVP HR Config: ")) ;
	BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_1", uiCore1_AssignedFe)) ;
	BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_0", uiCore0_AssignedFe)) ;
#endif
	
	hHDR->hFeChannel = (BHDR_FE_ChannelHandle) hFeChannel ;

	/* remember FE Channel's assigned core */
	hFeChannel->uiHdrSel = eCoreId ;
	
	/* also remember the HDR register offset for updating HDR registers */
	hFeChannel->ulHdrOffset = hHDR->ulOffset ;

	hFeChannel->hHDR = hHDR ;

	BKNI_EnterCriticalSection() ;
		BHDR_P_HotPlugConnect_isr(hHDR);
	BKNI_LeaveCriticalSection() ;	
	
#if 0
	/* 20100420 Remove Interrupt Enable */
	/* Pulse the Hot Plug */
	BHDR_PulseHotPlug(hHDR) ;

	BKNI_EnterCriticalSection() ;
		BHDR_P_HotPlugConnect_isr(hHDR);
		
		if (!hFeChannel->settings.bHpdDisconnected)
		{
			BHDR_FE_P_EnableInterrupts_isr(hFeChannel, true) ;
			BHDR_P_EnableInterrupts_isr(hHDR, true) ;
		}
	BKNI_LeaveCriticalSection() ;	
#endif	
	
done:	
	BDBG_LEAVE(BHDR_FE_AttachHdmiRxCore) ;
	return rc ;
} 


/******************************************************************************
Summary: Detach the HDMI Rx Device from the Front End Channel 
*******************************************************************************/
BERR_Code BHDR_FE_DetachHdmiRxCore(
	BHDR_FE_ChannelHandle hFeChannel,
	BHDR_Handle hHDR) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	
	uint32_t RegValue ;

	uint8_t 
		uiCore0_AssignedFe, 
		uiCore1_AssignedFe ;

	
	BDBG_ENTER(BHDR_FE_DetachHdmiRxCore) ;

	BDBG_ASSERT(hHDR) ;
	BDBG_ASSERT(hFeChannel) ;

	hRegister = hHDR->hRegister ;

	
	/* verify change in FE channel configuration */
	
	RegValue = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG) ;
	uiCore0_AssignedFe = 
		BCHP_GET_FIELD_DATA(RegValue, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;		
	uiCore1_AssignedFe = 
		BCHP_GET_FIELD_DATA(RegValue, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;

#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG
	BDBG_WRN(("OLD DVP HR Config: ")) ;
	BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_1", uiCore1_AssignedFe)) ;
	BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_0", uiCore0_AssignedFe)) ;
	
	BDBG_WRN(("\n\nDETACH HDMI_RX_%d from HDMI_FE_%d", 
		hHDR->eCoreId, hHDR->hFeChannel->eChannel)) ;

	if (hHDR->hFeChannel->eChannel != hFeChannel->eChannel)
	{
		BDBG_WRN(("Requested detach of HDMI_RX_FE_%d does not match current HDMI_RX_FE_%d",
			hFeChannel->eChannel, hHDR->hFeChannel->eChannel)) ;
	}

#endif

	/* reconfigure Core to use new Front End */
	switch (hHDR->eCoreId)
	{
	case BHDR_P_eHdrCoreId0 :
		RegValue &= ~ BCHP_MASK(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;
		RegValue |= BCHP_FIELD_DATA(DVP_HR_DVP_HR_DATA_PATH_CFG, 
			HDMI_RX_0_SEL, 0) ;
		break ;
		
	case BHDR_P_eHdrCoreId1 :
		RegValue &= ~ BCHP_MASK(DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;
		RegValue |= BCHP_FIELD_DATA(DVP_HR_DVP_HR_DATA_PATH_CFG, 
			HDMI_RX_1_SEL, 1) ;
		break ;
		
	default :
		BDBG_ERR(("Unknown Core ID %d", hHDR->eCoreId)) ;	
		break ;
	}
	
	/* update new FrontEnd to HDMI Rx core */
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG, RegValue) ;


#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG
	RegValue = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_HR_DATA_PATH_CFG) ;
	uiCore0_AssignedFe = 
		BCHP_GET_FIELD_DATA(RegValue, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_0_SEL) ;
	uiCore1_AssignedFe = 
		BCHP_GET_FIELD_DATA(RegValue, DVP_HR_DVP_HR_DATA_PATH_CFG, HDMI_RX_1_SEL) ;

	BDBG_WRN(("NEW DVP HR Config: ")) ;
	BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_1", uiCore1_AssignedFe)) ;
	BDBG_WRN(("   HDMI_FE_%d -- routed to --> HDMI_RX_0", uiCore0_AssignedFe)) ;
#endif

	BDBG_LEAVE(BHDR_FE_DetachHdmiRxCore) ;
	return rc ;
} 

/******************************************************************************
Summary: Get the Front End Channel attached to the HDMI Rx Core  
*******************************************************************************/
BERR_Code BHDR_FE_GetAttachedHdmiRxCore(
	BHDR_FE_ChannelHandle hFeChannel,
	uint8_t *uiHdrSel)  
{
	BERR_Code rc = BERR_SUCCESS ;
	
	BDBG_ENTER(BHDR_FE_GetAttachedHdmiRxCore) ;
	BDBG_ASSERT(hFeChannel) ;

	*uiHdrSel = hFeChannel->uiHdrSel ;
	
	BDBG_LEAVE(BHDR_FE_GetAttachedHdmiRxCore) ;
	return rc ;
} 



/******************************************************************************
Summary: Enable/Disable Equalization 
*******************************************************************************/
BERR_Code BHDR_FE_EnableEqualization(BHDR_FE_ChannelHandle hFeChannel, bool bEnable) 
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset  ;
	BERR_Code rc = BERR_SUCCESS ;
	
	BDBG_ASSERT(hFeChannel) ;
	hRegister = hFeChannel->hRegister ;
	
	/* get offset for Front End */
	ulOffset = hFeChannel->ulOffset ;

#if BHDR_CONFIG_ENABLE_ADAPTIVE_EQUALIZATION
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset) ;
	Register &= ~ (
		  BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_FORCE_CW)
		| BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_COEF)) ;
	Register |= 
		  BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_FORCE_CW, 0)
		|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_COEF, 0x12) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset, Register) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_3 + ulOffset) ;
	Register &= ~ (
		  BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_3, LOOP_FILTER_COEFF_OUT1)
		| BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_3, LOOP_FILTER_COEFF_OUT2)) ;
	Register |= 
		  BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_3, LOOP_FILTER_COEFF_OUT1, 0x20)
		| BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_3, LOOP_FILTER_COEFF_OUT2, 0x60) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_3 + ulOffset, Register) ;
	
	/* enable or disable equalizer depending on setting */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset) ;
	Register &= ~ (
		   BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_PD_ZERO_FOR_NO_CHANGE)
		| BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_PHASE_ERROR_ZERO)) ;
					
	if (bEnable) 
	{
		Register |= 
			  BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_PD_ZERO_FOR_NO_CHANGE, 0)
			|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_PHASE_ERROR_ZERO, 0) ;
#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG
		BDBG_WRN(("HDMI_RX_FE_%d Equalizer is ENABLED; CTL_1 %X", hFeChannel->eChannel, Register)) ;
#endif
	}
	else
	{
		Register |= 
			  BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_PD_ZERO_FOR_NO_CHANGE, 1)
			|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_PHASE_ERROR_ZERO, 1) 
			|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_FORCE_CW, 0) 
			|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_OFFSET, 0x80) ;
#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG
		BDBG_WRN(("HDMI_RX_FE_%d Equalizer is DISABLED; CTL_1 %X", hFeChannel->eChannel, Register)) ;
#endif
	}		
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset , Register) ;
	
#else

	/* enable or disable equalizer depending on setting */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset) ;
	Register &= ~ (
		   BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_PD_ZERO_FOR_NO_CHANGE)
		| BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_PHASE_ERROR_ZERO)
		| BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_FORCE_CW)) ;
					
	if (bEnable) 
	{
		Register |= 
			  BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_PD_ZERO_FOR_NO_CHANGE, 0)
			|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_PHASE_ERROR_ZERO, 0)
			|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_FORCE_CW, 1) ;
	}
	else
	{
		Register |= 
			  BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_PD_ZERO_FOR_NO_CHANGE, 1)
			|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_PHASE_ERROR_ZERO, 1)
			|BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, EQ_FORCE_CW, 0) ;
		BDBG_WRN(("HDMI_RX_FE_%d Equalizer is DISABLED", hFeChannel->eChannel)) ;
	}		
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset , Register) ;

#endif
	
	return rc ;

}



/**************************************************************************
Summary: Register a callback function to be called when a CEC event occurs
**************************************************************************/
BERR_Code BHDR_FE_InstallCecCallback( 
	BHDR_FE_ChannelHandle hFeChannel,			/* [in] HDMI Front End Handle */
	const BHDR_FE_CallbackFunc pfCallback_isr, /* [in] cb for notification */
	void *pvParm1,  /* [in] the first argument (void *) passed to the callback function */ 
	int iParm2      /* [in] the second argument(int) passed to the callback function */ 
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_ENTER(BHDR_FE_InstallCecCallback);
	BDBG_ASSERT(hFeChannel) ;

	if (hFeChannel->pfCecCallback_isr)
	{
		BDBG_WRN(("HDMI_RX_FE_%d Callback handler already installed for CEC...overriding",
			hFeChannel->eChannel)) ;
	}	

	BKNI_EnterCriticalSection() ;
		hFeChannel->pfCecCallback_isr = pfCallback_isr ;
		hFeChannel->pvCecParm1 = pvParm1 ;
		hFeChannel->iCecParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;
	
	BDBG_LEAVE(BHDR_FE_InstallCecCallback) ;
	return rc ;
}


/**************************************************************************
Summary: Remove a previously registered callback function for CEC events
**************************************************************************/
BERR_Code BHDR_FE_UnInstallCecCallback( 
	BHDR_FE_ChannelHandle hFeChannel,                       /* [in] HDMI Rx Handle */
	const BHDR_FE_CallbackFunc pfCallback_isr /* [in] cb for CEC notification  */
) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_ENTER(BHDR_FE_UnInstallCecCallback);
	BDBG_ASSERT(hFeChannel) ;
	
	BSTD_UNUSED (pfCallback_isr) ;

	if (hFeChannel->pfCecCallback_isr == (BHDR_CallbackFunc) NULL)
	{
		BDBG_WRN(("No callback handler to uninstall for CEC callback")) ;
		goto done ;
	}

	BKNI_EnterCriticalSection() ;
		hFeChannel->pfCecCallback_isr = (BHDR_CallbackFunc) NULL ;
		hFeChannel->pvCecParm1 = (void *) NULL  ;
		hFeChannel->iCecParm2 = 0  ;
	BKNI_LeaveCriticalSection() ;
	
done:
	BDBG_LEAVE(BHDR_FE_UnInstallCecCallback) ;
	return rc ;
}



/**************************************************************************
Summary: Register a callback function to be called when a CEC event occurs
**************************************************************************/
BERR_Code BHDR_FE_InstallHotPlugCallback( 
	BHDR_FE_ChannelHandle hFeChannel,			/* [in] HDMI Front End Handle */
	const BHDR_FE_CallbackFunc pfCallback_isr, /* [in] cb for notification */
	void *pvParm1,  /* [in] the first argument (void *) passed to the callback function */ 
	int iParm2      /* [in] the second argument(int) passed to the callback function */ 
) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_ENTER(BHDR_FE_InstallHotPlugCallback);
	BDBG_ASSERT(hFeChannel) ;
	
	if (hFeChannel->pfHotPlugCallback_isr)
	{
		BDBG_WRN(("Callback handler already installed for Hot Plug...overriding")) ;
	}

	BKNI_EnterCriticalSection() ;
		hFeChannel->pfHotPlugCallback_isr = pfCallback_isr ;
		hFeChannel->pvHotPlugParm1 = pvParm1 ;
		hFeChannel->iHotPlugParm2 = iParm2 ;
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDR_FE_InstallHotPlugCallback) ;
	return rc ;
}



/**************************************************************************
Summary:Remove a previously registered callback function for Hot Plug events
**************************************************************************/
BERR_Code BHDR_FE_UnInstallHotPlugCallback( 
	BHDR_FE_ChannelHandle hFeChannel,                       /* [in] HDMI Rx Frontend Channel Handle */
	const BHDR_FE_CallbackFunc pfCallback_isr /* [in] cb for Hot Plug notification  */
) 
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_ENTER(BHDR_FE_UnInstallHotPlugCallback);
	BDBG_ASSERT(hFeChannel) ;
	
	BSTD_UNUSED(pfCallback_isr) ;

	if (hFeChannel->pfHotPlugCallback_isr == (BHDR_CallbackFunc) NULL)
	{
		BDBG_WRN(("No callback handler to uninstall for HotPlug callback")) ;
		goto done ;
	}

	BKNI_EnterCriticalSection() ;
		hFeChannel->pfHotPlugCallback_isr = (BHDR_CallbackFunc) NULL ;
		hFeChannel->pvHotPlugParm1 = (void *) NULL  ;
		hFeChannel->iHotPlugParm2 = 0  ;
	BKNI_LeaveCriticalSection() ;

done :	
	BDBG_LEAVE(BHDR_FE_UnInstallHotPlugCallback) ;
	return rc ;
}


/**************************************************************************
Summary: Power Down Core Sequence
**************************************************************************/
void BHDR_FE_P_PowerDownCore(
	BHDR_FE_ChannelHandle hFeChannel) 
{
	BREG_Handle hRegister ;
	uint32_t Register ;
		
	uint32_t ulOffset  ;
	
	BDBG_ASSERT(hFeChannel) ;
	hRegister = hFeChannel->hRegister ;
	/* get offset for Front End */
	ulOffset = hFeChannel->ulOffset ;
	
	/* power down the ADC */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_1, ADC_PWRDN) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_1, ADC_PWRDN, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset, Register) ;

	/* leave CEC powered if requested */
	if (!hFeChannel->stPowerSettings.bCecPowered)
	{
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset) ;
		Register |= BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, BIAS_PD) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;		
	}

	/* power down the analog core */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0 + ulOffset) ;
	Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_0, REG_PWRDN) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_ANALOG_CFG_0, REG_PWRDN, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0 + ulOffset, Register) ;

	/* power down the PLL and analog channels */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_POWER_DOWN, 1) ; 
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_POWER_DOWN, 1) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_POWER_DOWN, 1) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_POWER_DOWN, 1) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_RESET, 1) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET, 1) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET, 1) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;

#if BHDR_CONFIG_DEBUG_DISPLAY_FE_CONFIG
	BDBG_WRN(("HDMI RX_FE_%d Power Config: CEC: %s; PHY: %s ", 
		hFeChannel->eChannel,
		hFeChannel->stPowerSettings.bCecPowered ? "ENABLED" : "DISABLED",
		hFeChannel->stPowerSettings.bHdmiRxPowered ? "ENABLED" : "DISABLED")) ;
#endif
}


/**************************************************************************
Summary: Enable/Disable Frontend Interrupts
**************************************************************************/
void BHDR_FE_P_EnableInterrupts_isr(BHDR_FE_ChannelHandle hFeChannel, bool enable)
{
	BERR_Code rc  ;
	uint32_t ulOffset  ;
	uint8_t i ;

	BDBG_ENTER(BHDR_FE_P_EnableInterrupts_isr) ;

	if (hFeChannel->hHDR)
	{
		if (hFeChannel->hHDR->eUsage == BHDR_Usage_eSlave)
		{
			BDBG_WRN(("HDMI RX_FE_%d keep slave interrupts disabled",
				hFeChannel->eChannel));
			return ;
		}
	}

#if 0	
	/* do not enable/disable interrupts unless HPD signal is disconnected  */
	if (!hFeChannel->settings.bHpdDisconnected)
		return ;
#endif

	/* get offset for Front End */
	ulOffset = hFeChannel->ulOffset ;
	if (hFeChannel->eChannel == BHDR_FE_P_eChannel0)
		pInterrupts = BHDR_FE_P_ChannelIntr0 ;
	else
		pInterrupts = BHDR_FE_P_ChannelIntr1 ;
	
	for (i = 0; i < MAKE_INTR_FE_CHN_ENUM(LAST) ; i++) 
	{
		/* clear interrupt callback */
		rc =  BINT_ClearCallback_isr( hFeChannel->hCallback[i]) ;

		/* skip interrupt if not enabled in table...  */	
		if (!pInterrupts[i].enable)
			continue ;

	       if (enable)
			BINT_EnableCallback_isr( hFeChannel->hCallback[i] ) ;
		/* never disable Hotplug or CEC interrupts */
		else if ((i != MAKE_INTR_FE_CHN_ENUM(RX_HOTPLUG_UPDATE))
			&& (i != MAKE_INTR_FE_CHN_ENUM(CEC)))
		{
			BINT_DisableCallback_isr( hFeChannel->hCallback[i] ) ;
			BDBG_WRN(("CH%d Disable Interrupt %d; ", hFeChannel->eChannel, i)) ;
		}
		else
		{
			BDBG_WRN(("CH%d Keep Interrupt %d Enabled", hFeChannel->eChannel, i)) ; 
		}
	}
	BDBG_LEAVE(BHDR_FE_P_EnableInterrupts_isr) ;
}


/**************************************************************************
Summary:Set Power State  
**************************************************************************/
BERR_Code BHDR_FE_SetPowerState(
	BHDR_FE_ChannelHandle hFeChannel, BHDR_FE_ChannelPowerSettings * stPowerSettings) 
{
#if BHDR_POWER_MANAGEMENT
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t  	RegDvpFeResetControlAddr ;

	uint32_t ulOffset  ;
	BERR_Code rc = BERR_SUCCESS ;
	
	BDBG_ENTER(BHDR_FE_SetPowerState);
	
	BDBG_ASSERT(hFeChannel) ;
	hRegister = hFeChannel->hRegister ;

	if (BKNI_Memcmp(&hFeChannel->stPowerSettings, stPowerSettings,  
		sizeof(BHDR_FE_ChannelPowerSettings)) == 0)
	{
		BDBG_WRN(("No change in power settings...")) ;
		return rc ;
	}

	hFeChannel->stPowerSettings.bHdmiRxPowered = stPowerSettings->bHdmiRxPowered ;
	
	/* get offset for Front End */
	ulOffset = hFeChannel->ulOffset ;
	if (hFeChannel->eChannel == BHDR_FE_P_eChannel0)
	{
		RegDvpFeResetControlAddr = BCHP_DVP_HR_HDMI_RX_FE_0_RESET_CONTROL ;
		pInterrupts = BHDR_FE_P_ChannelIntr0 ;
	
	}
	else
	{
		RegDvpFeResetControlAddr = BCHP_DVP_HR_HDMI_RX_FE_1_RESET_CONTROL ;
		pInterrupts = BHDR_FE_P_ChannelIntr1 ;
	}
	
	if (stPowerSettings->bHdmiRxPowered)
	{
		/* unreset the DVP HR and HR Frontend */		
		Register = BREG_Read32(hRegister, RegDvpFeResetControlAddr) ;
		Register &= ~ BCHP_MASK(DVP_HR_HDMI_RX_FE_0_RESET_CONTROL, HDMI_RX_FE_0_RESET) ;
		BREG_Write32(hRegister, RegDvpFeResetControlAddr, Register) ;		

		/* power up analog core */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_0, REG_PWRDN) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0 + ulOffset, Register) ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_1, BIAS_PD) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_1 + ulOffset, Register) ;

		/* power up the PLL and analog channels */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset) ;

		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, CEC_POWER_DOWN) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_POWER_DOWN) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_POWER_DOWN) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_POWER_DOWN) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_POWER_DOWN) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_PLL_RESET) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_0_RESET) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_1_RESET) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_RESET_CONTROL, ANALOG_CHANNEL_2_RESET) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_RESET_CONTROL + ulOffset, Register) ;

		/* power up the ADC */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_1, ADC_PWRDN) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset, Register) ;

		/* enable the interrupts */
		BKNI_EnterCriticalSection();
			BHDR_FE_P_EnableInterrupts_isr(hFeChannel, true) ;
		BKNI_LeaveCriticalSection() ;
		
		BDBG_MSG(("HDMI_RX_FE_%d is powered ON", hFeChannel->eChannel)) ;
	}
	else /* power down */
	{
		/* disable the FrontEnd interrupts */
		if (!stPowerSettings->bCecPowered)
		{
			BKNI_EnterCriticalSection();
				BHDR_FE_P_EnableInterrupts_isr(hFeChannel, false) ;
			BKNI_LeaveCriticalSection() ;		
		}

		BHDR_FE_P_PowerDownCore(hFeChannel)  ;
	}


	/* if CEC/FrontEnd is to remain powered...  */
	/* i.e. Rx can be powered down while FrontEnd remains active */	 
	if (stPowerSettings->bCecPowered)
	{		
		/* power up the analog core */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0 + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_0, REG_PWRDN) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_0 + ulOffset, Register) ;
				
		/* enable the interrupts */
		BKNI_EnterCriticalSection();
			BHDR_FE_P_EnableInterrupts_isr(hFeChannel, true) ;
		BKNI_LeaveCriticalSection() ;
		
		BDBG_MSG(("HDMI_RX_FE_%d CEC/FrontEnd is ENABLED", hFeChannel->eChannel)) ;
	}
	else
	{
		BDBG_WRN(("HDMI_RX_FE_%d CEC/FrontEnd is DISABLED", hFeChannel->eChannel)) ;
	}		

#else
	if (!stPowerSettings->bHdmiRxPowered)
	{
		BDBG_WRN(("HDMI_RX_FE_%d CEC/FrontEnd is powered OFF", hFeChannel->eChannel)) ;
		BHDR_FE_P_PowerDownCore(hFeChannel)  ;
	}
	
#endif


	BKNI_Memcpy(&hFeChannel->stPowerSettings, stPowerSettings, 
		sizeof(BHDR_FE_ChannelPowerSettings)) ;
	
	BDBG_LEAVE(BHDR_FE_SetPowerState) ;
		
	return BERR_SUCCESS ;
}


/**************************************************************************
Summary:Get Power State  
**************************************************************************/
BERR_Code BHDR_FE_GetPowerState(
	BHDR_FE_ChannelHandle hFeChannel, BHDR_FE_ChannelPowerSettings * stPowerSettings) 
{
#if BHDR_POWER_MANAGEMENT
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_ENTER(BHDR_FE_GetPowerState);
	
	BKNI_Memcpy(stPowerSettings, &hFeChannel->stPowerSettings,
		sizeof(BHDR_FE_ChannelPowerSettings)) ;
	
	BDBG_LEAVE(BHDR_FE_GetPowerState) ;
		
	return rc ;
#else	
	BSTD_UNUSED(hFeChannel) ;
	BSTD_UNUSED(stPowerSettings) ;
	return BERR_SUCCESS ;
#endif
}


#if BHDR_CONFIG_DEBUG_EYE_DIAGRAM
BERR_Code BHDR_FE_DEBUG_GetEyeDiagramData(BHDR_Handle hHDR, uint8_t *padc_data) 
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t status;
	uint8_t adc_clk_dly;
	uint16_t i, j;
	uint16_t colcount = 2048;
	uint8_t adc_sample_num = 6;
	uint32_t adc_raw_data[4][2048];

	hRegister = hHDR->hRegister ;

	/* get offset for Front End */
	ulOffset = hHDR->hFeChannel->ulOffset ;
	
	/* Freeze the equalizer by zeroing out the phase error */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_PHASE_ERROR_ZERO, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset, Register) ;

	/*  set the ADC delay to 0 */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset) ;
	adc_clk_dly = (uint8_t) BCHP_GET_FIELD_DATA(Register, 
			HDMI_RX_FE_0_PLL_CTRL_1, ADCCLK_DLY) ;
	Register &= ~BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_1, ADCCLK_DLY) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset, Register) ;

	/* Capture the data */
	for(i = 0; i < colcount; i++)	
	{ 
		/* Start ADC capture and wait til done. */
		status = 0;

		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_3 + ulOffset) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_ANALOG_CFG_3, ADC_CAPTURE_BEGIN, 1) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_3 + ulOffset, Register) ;

		Register &= ~BCHP_MASK(HDMI_RX_FE_0_ANALOG_CFG_3, ADC_CAPTURE_BEGIN) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_CFG_3 + ulOffset, Register) ;

		while(status == 0)
		{
			Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_ADC_READBACK_0 + ulOffset) ;
			status = Register & BCHP_MASK(HDMI_RX_FE_0_ANALOG_ADC_READBACK_0, ADC_CAPTURE_DONE);
			BKNI_Delay(100) ;		
		}

		/* Read back captured data. */
		adc_raw_data[0][i] = 0x7FFFFFFF & BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_ADC_READBACK_0 + ulOffset);
		adc_raw_data[1][i] = 0x7FFFFFFF & BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_ADC_READBACK_1 + ulOffset);
		adc_raw_data[2][i] = 0x7FFFFFFF & BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_ADC_READBACK_2 + ulOffset);
		adc_raw_data[3][i] = 0x7FFFFFFF & BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_ANALOG_ADC_READBACK_3 + ulOffset);

		/* Rotate to the next point */
		Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset) ;
		Register &= ~BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_1, ADCCLK_DLY) ;
		Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_1, ADCCLK_DLY, i & 0x01F) ;
		BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset, Register) ;
		
	}

	/* Thaw the control loop */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_LOOP_FILT_CTRL + ulOffset) ;
	Register |= 
		BCHP_FIELD_DATA(HDMI_RX_FE_0_LOOP_FILT_CTRL, bwsel_prop, 1) |
		BCHP_FIELD_DATA(HDMI_RX_FE_0_LOOP_FILT_CTRL, bwsel_integ, 1);
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_LOOP_FILT_CTRL + ulOffset, Register) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_FE_0_PLL_CTRL_1, ADCCLK_DLY) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_FE_0_PLL_CTRL_1, ADCCLK_DLY, adc_clk_dly) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_PLL_CTRL_1 + ulOffset, Register) ;

	/* Unfreeze the equalizer */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1, LOOP_FILTER_PHASE_ERROR_ZERO) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_FE_0_DIGITAL_LOOP_FILTER_CTL_1 + ulOffset, Register) ;

	/*Postprocess the captured data to break out the individual samples.*/
	for(i = 0; i < colcount; i++)
	{
		for(j = 0; j < 24; j++)
		{
			*(padc_data + j*colcount + i) = (uint8_t)((adc_raw_data[j/adc_sample_num][i] >> ((j % adc_sample_num) * 5)) & 0x01F);
		}		
	}
	
	return rc ;
}
#endif

/* end of file */

