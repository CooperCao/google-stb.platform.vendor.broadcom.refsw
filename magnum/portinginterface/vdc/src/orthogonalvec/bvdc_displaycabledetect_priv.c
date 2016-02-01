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
#include "bvdc_display_priv.h"

BDBG_MODULE(BVDC_DISP);

#define BVDC_P_PLUGIN_THRESHOLD    0xAA
#define BVDC_P_PLUGOUT_THRESHOLD   0x21C
#define BVDC_P_PLUGOUT_SLEEP       4
#define BVDC_P_PLUGIN_SLEEP        1

#if (BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND)
/* This is AnalyseADC_Data function which returns true if connected, otherwise
 * false */
static bool BVDC_P_IsDacConnected_isr
	( BVDC_Display_Handle              hDisplay )
{
	uint32_t ulAdcData = 0;
	bool bConnected = false;

	/* TODO: need to determine worst case waiting here */
	while(ulAdcData == 0)
		ulAdcData = BREG_Read32_isr(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_ADC_DATA_0);

	if (ulAdcData <= BVDC_P_PLUGIN_THRESHOLD)
	{
		bConnected = true;
	}
	else if (ulAdcData > BVDC_P_PLUGOUT_THRESHOLD)
	{
		bConnected = false;
	}

	BDBG_MSG(("ADC OUT: 0x%x - STATUS: %s",
		ulAdcData, bConnected ? "Connected" : "Disconnected"));
	return bConnected;
}

static void BVDC_P_SyncOffOtherDacs_isr
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulDac )
{
	uint32_t ulDetectSyncCtrl = 0; /* TODO: reset value? */

	/* TODO: add support up to MAX_DAC */
	switch(ulDac)
	{
		case 0:
			ulDetectSyncCtrl =
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[3]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[2]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[1]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[0]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_EN,     1   );
			break;
		case 1:
			ulDetectSyncCtrl =
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[3]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[2]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[1]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[0]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_EN,     1   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_EN,     0   );
			break;
		case 2:
			ulDetectSyncCtrl =
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[3]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[2]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[1]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[0]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_EN,     1   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_EN,     0   );
			break;
		case 3:
			ulDetectSyncCtrl =
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[3]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[2]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[1]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[0]) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_EN,     1   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_EN,     0   ) |
				BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_EN,     0   );
			break;
		default:
			break;
	}

	BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_DETECT_SYNC_CTRL_0, ulDetectSyncCtrl);
}

static bool BVDC_P_PlugIn_isr
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulDac )
{
	if(hDisplay->hVdc->ulPlugInWait == 0)
	{
		BDBG_MSG(("Dac %d Starts Plug in", ulDac));
		BVDC_P_SyncOffOtherDacs_isr(hDisplay, ulDac);
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_ADC_CTRL_0,
			BCHP_FIELD_DATA(MISC_ADC_CTRL_0, RESETB,   1      ) |
			BCHP_FIELD_DATA(MISC_ADC_CTRL_0, CTRL,     0      ) |
			BCHP_FIELD_ENUM(MISC_ADC_CTRL_0, MUX_MODE, SCALED ) |
			BCHP_FIELD_DATA(MISC_ADC_CTRL_0, CH_SEL,   ulDac  ) |
			BCHP_FIELD_DATA(MISC_ADC_CTRL_0, PWRDN,    0      ));
#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_11)
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_INST_BIAS_CTRL_0,
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, GAIN_ADJ,       hDisplay->hVdc->stSettings.aulDacBandGapAdjust[ulDac] ) |
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, GAIN_OVERRIDE,  1    ) |
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, REG_ADJ,        4    ) |
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, BG_ADJ,         6    ) |
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, PWRDN,          0    ));
#elif (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_9)
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_BIAS_CTRL_0,
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC3_SCALE_LP,  0             ) | /* optimmal */
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC3_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[3]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC2_SCALE_LP,  0             ) | /* optimmal */
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC2_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[2]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC1_SCALE_LP,  0             ) | /* optimmal */
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC1_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[1]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC0_SCALE_LP,  0             ) | /* optimmal */
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC0_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[0]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, GAIN_ADJ,       hDisplay->hVdc->stSettings.aulDacBandGapAdjust[ulDac]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, GAIN_OVERRIDE,  1             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, REG_ADJ,        4             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, BG_ADJ,         6             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, PWRDN,          0             ));
#endif
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_DETECT_EN_0,
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, USE_STEP_DLY,   0) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, SW_CALIBRATE,   1) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, CALIBRATE,      1) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, PLUGOUT_DETECT, 0) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, PLUGIN_DETECT,  0) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, AUTO_DETECT,    0));
#if 0
		return BVDC_P_IsDacConnected_isr(hDisplay);
#else
		hDisplay->hVdc->ulPlugInWait = BVDC_P_PLUGIN_SLEEP;
		return false; /* start plug in sleeping, still not connected */
	}
	else
	{
		hDisplay->hVdc->ulPlugInWait--;
		if(hDisplay->hVdc->ulPlugInWait == 0)
		{
			/* Done plug out sleeping */
			return BVDC_P_IsDacConnected_isr(hDisplay);
		}
		else
		{
			BDBG_MSG(("Dac %d Waits", ulDac));
			return false; /* still plug in sleeping, still not connected */
		}
#endif
	}
}

static bool BVDC_P_PlugOut_isr
	( BVDC_Display_Handle              hDisplay,
	  uint32_t                         ulDac )
{
	if(hDisplay->hVdc->ulPlugOutWait == 0)
	{
		BDBG_MSG(("Dac %d Starts Plug out", ulDac));
		BVDC_P_SyncOffOtherDacs_isr(hDisplay, ulDac);
#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_11)
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_INST_BIAS_CTRL_0,
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, GAIN_ADJ,       hDisplay->hVdc->stSettings.aulDacBandGapAdjust[ulDac] ) |
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, GAIN_OVERRIDE,  1    ) |
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, REG_ADJ,        4    ) |
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, BG_ADJ,         6    ) |
			BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, PWRDN,          0    ));
#elif (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_9)
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_BIAS_CTRL_0,
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC3_SCALE_LP,  0             ) | /* optimmal */
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC3_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[3]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC2_SCALE_LP,  0             ) | /* optimmal */
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC2_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[2]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC1_SCALE_LP,  0             ) | /* optimmal */
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC1_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[1]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC0_SCALE_LP,  0             ) | /* optimmal */
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC0_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[0]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, GAIN_ADJ,       hDisplay->hVdc->stSettings.aulDacBandGapAdjust[ulDac]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, GAIN_OVERRIDE,  1             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, REG_ADJ,        4             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, BG_ADJ,         6             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, PWRDN,          0             ));
#endif
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_DETECT_EN_0,
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, USE_STEP_DLY,   0) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, SW_CALIBRATE,   0) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, CALIBRATE,      0) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, PLUGOUT_DETECT, 1) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, PLUGIN_DETECT,  0) |
			BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, AUTO_DETECT,    1));
		hDisplay->hVdc->ulPlugOutWait = BVDC_P_PLUGOUT_SLEEP;
		return true; /* start plug out sleeping, still connected */
	}
	else
	{
		hDisplay->hVdc->ulPlugOutWait--;
		if(hDisplay->hVdc->ulPlugOutWait == 0)
		{
			uint32_t tmp;
			tmp = BREG_Read32_isr(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_CABLE_STATUS_0);
			if(tmp & (1<<ulDac))
			{
				BDBG_MSG(("Dac%d: BCHP_MISC_DAC_CABLE_STATUS_0 = 0x%x: Connected", ulDac, tmp));
				return true;
			}
			else
			{
				BDBG_MSG(("Dac%d: BCHP_MISC_DAC_CABLE_STATUS_0 = 0x%x: Not Connected", ulDac, tmp));
				return false;
			}
		}
		else
		{
			BDBG_MSG(("Dac %d Waits", ulDac));
			return true; /* still plug out sleeping, still connected */
		}
	}
}

void BVDC_P_CableDetect_isr
	( BVDC_Display_Handle              hDisplay )
{
	/* This function will be called every vsync, currently only display0 */
	/* runs this.  TODO: add function pointer and assign to first display */
	/* created, and implement the hand off functionality when the display */
	/* with cable detect running gets destroyed */

	bool bStatus = false;
	uint32_t ulNextDac = hDisplay->hVdc->ulDacDetect;
	BVDC_DacConnectionState eStatus = BVDC_DacConnectionState_eDisconnected;

	if(hDisplay->hVdc->ulPlugOutWait == 0)
	{
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_CABLE_STATUS_0, 0xF);
	}
	BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_DETECT_SYNC_CTRL_0, hDisplay->hVdc->ulDacDetectSyncCtrl);
	if(hDisplay->hVdc->aulDacSyncEn[hDisplay->hVdc->ulDacDetect])
	{
		/* if current detected DAC with sync */
		if(hDisplay->hVdc->aeDacStatus[hDisplay->hVdc->ulDacDetect] == BVDC_DacConnectionState_eDisconnected)
		{
			/*BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_CABLE_STATUS_0, 0x1F);*/
			/* if current status is not connected, do Plug in */
			bStatus = BVDC_P_PlugIn_isr(hDisplay, hDisplay->hVdc->ulDacDetect);
			if(hDisplay->hVdc->ulPlugInWait == 0)
			{
				/* advance to next dac detect */
				ulNextDac = (hDisplay->hVdc->ulDacDetect + 1) % (BVDC_P_MAX_DACS);
			}
		}
		else
		{
			/* if current status is connected, do Plug out */
			bStatus = BVDC_P_PlugOut_isr(hDisplay, hDisplay->hVdc->ulDacDetect);
			if(hDisplay->hVdc->ulPlugOutWait == 0)
			{
				/* Done with plug out, advance to next dac detect */
				ulNextDac = (hDisplay->hVdc->ulDacDetect + 1) % (BVDC_P_MAX_DACS);
			}
		}

		eStatus = bStatus ? BVDC_DacConnectionState_eConnected : BVDC_DacConnectionState_eDisconnected;
	}
	else
	{
		uint32_t source = hDisplay->hVdc->aulDacSyncSource[hDisplay->hVdc->ulDacDetect];
		eStatus = hDisplay->hVdc->aeDacStatus[source];
		BDBG_MSG(("SOURCE OF SYNC for DAC %d is %d stat is %s", hDisplay->hVdc->ulDacDetect, source,
			(hDisplay->hVdc->aeDacStatus[source] == BVDC_DacConnectionState_eConnected) ? "Connected" : "Not Connected"));
		/* advance to next dac detect */
		ulNextDac = (hDisplay->hVdc->ulDacDetect + 1) % (BVDC_P_MAX_DACS);
		BDBG_MSG(("Dac%d Not Sync => Next Dac: %d",
			hDisplay->hVdc->ulDacDetect, ulNextDac));
	}

	if(hDisplay->hVdc->aeDacStatus[hDisplay->hVdc->ulDacDetect] != eStatus)
	{
		BDBG_ERR(("Dac%d: %s -> %s",
			hDisplay->hVdc->ulDacDetect,
			(hDisplay->hVdc->aeDacStatus[hDisplay->hVdc->ulDacDetect] == BVDC_DacConnectionState_eConnected) ? "Connected" : "Not Connected",
			(eStatus == BVDC_DacConnectionState_eConnected) ? "Connected" : "Not Connected"));
	}

	hDisplay->hVdc->aeDacStatus[hDisplay->hVdc->ulDacDetect] = eStatus;
	hDisplay->hVdc->ulDacDetect = ulNextDac;

	if(hDisplay->hVdc->ulPlugOutWait == 0 && hDisplay->hVdc->ulPlugInWait == 0)
	{
		uint32_t ulRegister;
		/* Restoring DAC status and Enabling Corresponding Low-Power Modes */
		ulRegister = BREG_Read32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_DETECT_EN_0);
		ulRegister &= ~(BCHP_MASK(MISC_DAC_DETECT_EN_0, AUTO_DETECT));
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_DETECT_EN_0, ulRegister);
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_CABLE_STATUS_0, 0xF);
#if 0
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_CABLE_STATUS_0,
			BCHP_FIELD_DATA(MISC_DAC_CABLE_STATUS_0, DAC0_CONNECTED, (hDisplay->hVdc->aeDacStatus[0] == BVDC_DacConnectionState_eConnected) ? 1 : 0) |
			BCHP_FIELD_DATA(MISC_DAC_CABLE_STATUS_0, DAC1_CONNECTED, (hDisplay->hVdc->aeDacStatus[1] == BVDC_DacConnectionState_eConnected) ? 1 : 0) |
			BCHP_FIELD_DATA(MISC_DAC_CABLE_STATUS_0, DAC2_CONNECTED, (hDisplay->hVdc->aeDacStatus[2] == BVDC_DacConnectionState_eConnected) ? 1 : 0) |
			BCHP_FIELD_DATA(MISC_DAC_CABLE_STATUS_0, DAC3_CONNECTED, (hDisplay->hVdc->aeDacStatus[3] == BVDC_DacConnectionState_eConnected) ? 1 : 0));
#endif

#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_11)
		/* BCHP_MISC_DAC_x_SCALE_CTRL  */
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_0_SCALE_CTRL,
			BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_LP,  (hDisplay->hVdc->aeDacStatus[0] == BVDC_DacConnectionState_eConnected) ? 0 : 1 ) |
			BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[0]));
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_1_SCALE_CTRL,
			BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_LP,  (hDisplay->hVdc->aeDacStatus[1] == BVDC_DacConnectionState_eConnected) ? 0 : 1 ) |
			BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[1]));
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_2_SCALE_CTRL,
			BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_LP,  (hDisplay->hVdc->aeDacStatus[2] == BVDC_DacConnectionState_eConnected) ? 0 : 1 ) |
			BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[2]));
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_3_SCALE_CTRL,
			BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_LP,  (hDisplay->hVdc->aeDacStatus[3] == BVDC_DacConnectionState_eConnected) ? 0 : 1 ) |
			BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[3]));
#elif (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_9)
		BREG_Write32(hDisplay->hVdc->hRegister, BCHP_MISC_DAC_BIAS_CTRL_0,
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC3_SCALE_LP, (hDisplay->hVdc->aeDacStatus[3] == BVDC_DacConnectionState_eConnected) ? 0 : 1 ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC3_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[3]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC2_SCALE_LP, (hDisplay->hVdc->aeDacStatus[2] == BVDC_DacConnectionState_eConnected) ? 0 : 1 ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC2_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[2]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC1_SCALE_LP, (hDisplay->hVdc->aeDacStatus[1] == BVDC_DacConnectionState_eConnected) ? 0 : 1 ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC1_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[1]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC0_SCALE_LP, (hDisplay->hVdc->aeDacStatus[0] == BVDC_DacConnectionState_eConnected) ? 0 : 1 ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC0_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[0]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, GAIN_ADJ,       hDisplay->hVdc->stSettings.aulDacBandGapAdjust[0]) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, GAIN_OVERRIDE,  1             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, REG_ADJ,        4             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, BG_ADJ,         6             ) |
				BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, PWRDN,          0             ));
#endif
	}
}

#endif

