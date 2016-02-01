/******************************************************************************
 *    (c)2011-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
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
 *****************************************************************************/

#ifndef _BTNR_STRUCT_H__
#define _BTNR_STRUCT_H__

#if __cplusplus
extern "C" {
#endif

/******************************************
 *BTNR_3x7x_Tuner_Power_Mode_t definition
 *****************************************/
typedef enum BTNR_Tuner_Power_Mode_s
{
BTNR_Tuner_Power_Mode_eOff        = 0,
BTNR_Tuner_Power_Mode_eMini_Power = 1,
BTNR_Tuner_Power_Mode_eUHF_Power = 2,
BTNR_Tuner_Power_Mode_eVHF_Power = 3,
BTNR_Tuner_Power_Mode_eUHF_VHF_Power = 4,
BTNR_Tuner_Power_Mode_eLna_Daisy_Power = 5,
BTNR_Tuner_Power_Mode_eOn        = 0xFFU
}BTNR_Tuner_Power_Mode_t;

typedef struct BTNR_3x7x_Tuner_Power_Mode_s
{
	BTNR_Tuner_Power_Mode_t Tuner_Power_Mode;
}BTNR_3x7x_Tuner_Power_Mode_t;

/******************************************
 *BTNR_3x7x_TuneType_t definition
 *****************************************/
typedef enum BTNR_TuneType_s
{
  BTNR_TuneType_eInitTune = 1,
  BTNR_TuneType_eTune = 2,
  BTNR_TuneType_eMiniTune = 3
} BTNR_TuneType_t;

typedef struct BTNR_3x7x_TuneType_s
{
	BTNR_TuneType_t   TuneType;
} BTNR_3x7x_TuneType_t;

/******************************************
 *BTNR_3x7x_Acquire_Params_t definition
 *****************************************/
typedef enum BTNR_TunerApplicationMode_s
{
  BTNR_TunerApplicationMode_eCable = 1,
  BTNR_TunerApplicationMode_eTerrestrial = 2
} BTNR_TunerApplicationMode_t;

typedef enum BTNR_LPF_Bandwidth_s
{
  BTNR_LPF_Bandwidth_e8MHz = 1,
  BTNR_LPF_Bandwidth_e7MHz = 2,
  BTNR_LPF_Bandwidth_e6MHz = 3,
  BTNR_LPF_Bandwidth_e5MHz = 4,
  BTNR_LPF_Bandwidth_e1_7MHz = 5,
	BTNR_LPF_Bandwidth_eVariable = 6
} BTNR_LPF_Bandwidth_t;

typedef enum BTNR_Standard_s
{
  BTNR_Standard_eDVBT = 1,
  BTNR_Standard_eISDBT = 2,
  BTNR_Standard_eQAM = 3,
  BTNR_Standard_eT2 =4
} BTNR_Standard_t;

typedef struct BTNR_Acquire_Params_s
{
	BTNR_TunerApplicationMode_t   Application;
	BTNR_Standard_t           Standard;
	BTNR_LPF_Bandwidth_t	  LPF_Bandwidth;
	uint32_t				  LPF_Variable_Bandwidth;
	uint32_t				  RF_Freq;

    uint32_t                  Tune_After_Set_Params;
} BTNR_3x7x_Acquire_Params_t;


/******************************************
 *BTNR_DPM_Params_t definition
 *****************************************/
typedef enum BTNR_DPM_Target_s
{
  BTNR_DPM_Target_eDaisyUHF=0,
  BTNR_DPM_Target_eRFVGA=1,
  BTNR_DPM_Target_eDaisyUHF_RFVGA=2
}BTNR_DPM_Target_t;

typedef enum BTNR_DPM_Params_OffOn_s
{
  BTNR_DPM_Params_eDisable=0,
  BTNR_DPM_Params_eEnable=1
}BTNR_DPM_Params_OffOn_t;

typedef struct BTNR_3x7x_DPM_Params_s
{
	BTNR_DPM_Params_OffOn_t   DPM_Enable;
	BTNR_DPM_Target_t         DPM_Target;
	uint32_t				  DPM_Freq;
} BTNR_3x7x_DPM_Params_t;

/******************************************
 *BTNR_3x7x_LoopThru_Params_t definition
 *****************************************/
typedef enum BTNR_LoopThru_Source_s
{
  BTNR_LoopThru_Source_eVHF=0,
  BTNR_LoopThru_Source_eDAC=1,
  BTNR_LoopThru_Source_eVHF_DAC=2
}BTNR_LoopThru_Source_t;

typedef enum BTNR_LoopThru_Params_OffOn_s
{
  BTNR_LoopThru_Params_eDisable=0,
  BTNR_LoopThru_Params_eEnable=1
}BTNR_LoopThru_Params_OffOn_t;

typedef struct BTNR_3x7x_LoopThru_Params_s
{
	BTNR_LoopThru_Params_OffOn_t LoopThru_Enable;
	BTNR_LoopThru_Source_t       LoopThru_Source;
} BTNR_3x7x_LoopThru_Params_t;

/******************************************
 *BTNR_3x7x_Daisy_Params_t definition
 *****************************************/
typedef enum BTNR_Daisy_Source_s
{
  BTNR_Daisy_Source_eUHF=0,
  BTNR_Daisy_Source_eVHF=1,
  BTNR_Daisy_Source_eUHF_VHF=2
}BTNR_Daisy_Source_t;

typedef enum BTNR_Daisy_Params_OffOn_s
{
  BTNR_Daisy_Params_eDisable=0,
  BTNR_Daisy_Params_eEnable=1
}BTNR_Daisy_Params_OffOn_t;

typedef struct BTNR_3x7x_Daisy_Params_s
{
	BTNR_Daisy_Params_OffOn_t Daisy_Enable;
	BTNR_Daisy_Source_t       Daisy_Source;
} BTNR_3x7x_Daisy_Params_t;

/******************************************
 *BTNR_3x7x_Gain_Params_t definition
 *****************************************/
typedef enum BTNR_Gain_Params_OffOn_s
{
  BTNR_Gain_Params_eDisable=0,
  BTNR_Gain_Params_eEnable=1
}BTNR_Gain_Params_OffOn_t;

typedef struct BTNR_3x7x_Gain_Params_s
{
	int32_t						ExternalGain_Total;			/*1/256 dBm [15:0]*/
	int32_t						ExternalGain_Bypassable;	/*1/256 dBm [15:0]*/
	int32_t						InternalGain_To_LT;			/*1/256 dBm [15:0]*/
	int32_t						InternalGain_To_Daisy;		/*1/256 dBm [15:0]*/
	uint32_t					Frequency;					/*Hz [31:0]*/
	BTNR_Gain_Params_OffOn_t	Gain_Bypassed;				/*Bool*/
}BTNR_3x7x_Gain_Params_t;

/****************************************************
 *BTNR_3x7x_Local_Params_t definition: must be initialized
 ***************************************************/
typedef enum BTNR_3x7x_TunerRfInputMode
{
  BTNR_3x7x_TunerRfInputMode_eOff = 0, /* Tuner Rf is off. */
  BTNR_3x7x_TunerRfInputMode_eExternalLna=1,  /* Tuner Rf input through UHF path. This Rf path does not use internal LNA. */
  BTNR_3x7x_TunerRfInputMode_eInternalLna=2, /* Tuner Rf input through UHF path. This Rf path does uses internal LNA. */
  BTNR_3x7x_TunerRfInputMode_eStandardIf=3,  /* 44 MHz or 36 MHz */
  BTNR_3x7x_TunerRfInputMode_eLowIf=4,  /*4 MHz to 5 MHz. */
  BTNR_3x7x_TunerRfInputMode_eBaseband=5,
  BTNR_3x7x_TunerRfInputMode_InternalLna_Daisy=6
}BTNR_3x7x_TunerRfInputMode_t;


 typedef enum BTNR_Local_Params_SmartTune_s
{
  BTNR_Local_Params_SmartTune_FreqPlanDefault=0,
  BTNR_Local_Params_SmartTune_FreqPlanA=1
}BTNR_Local_Params_SmartTune_t;

typedef struct BTNR_3x7x_Local_Params_s
{
	uint32_t								TunerCapCntl;
	int32_t									RF_Offset;	  /*This is set by scan*/
	uint32_t								Symbol_Rate;	/*This is set by scan*/
	int32_t									Total_Mix_After_ADC;
	int16_t									PostADC_Gain_x256db;
#ifndef LEAP_BASED_CODE
	BTNR_TunerApplicationMode_t				TunerApplication;
	BTNR_3x7x_TunerRfInputMode_t			RfInputMode;
	BTNR_Tuner_Power_Mode_t					Tuner_Power_Mode;
	uint32_t								TunerBBSaddress;
#endif
	BTNR_Local_Params_SmartTune_t   SmartTune;
	uint32_t								RevId;
	uint32_t								TuneComple;
	uint8_t									hardware_tune;
	uint32_t								VCO_max;
	uint32_t								freq_HRM_max;
}BTNR_3x7x_Local_Params_t;

/****************************************************
 *BTNR_3x7x_Internal_Params_t definition: must be initialized
 ***************************************************/
typedef enum BTNR_Internal_Params_SDADC_Input_s
{
	BTNR_Internal_Params_SDADC_Input_eTuner=0,
	BTNR_Internal_Params_SDADC_Input_eTuner_wTestOut=1,
	BTNR_Internal_Params_SDADC_Input_eExtReal=2,
	BTNR_Internal_Params_SDADC_Input_eExtIQ=3
} BTNR_Internal_Params_SDADC_Input_t;

typedef enum BTNR_Internal_Params_TunerRFFIL_s
{
  BTNR_Internal_Params_TunerRFFIL_eTRKFIL=0,
  BTNR_Internal_Params_TunerRFFIL_eMOCATRAP=1,
  BTNR_Internal_Params_TunerRFFIL_eTRKBYPASS=2
}BTNR_Internal_Params_TunerRFFIL_t;

typedef enum BTNR_Internal_Params_OffOn_s
{
  BTNR_Internal_Params_eDisable=0,
  BTNR_Internal_Params_eEnable=1
}BTNR_Internal_Params_OffOn_t;

typedef enum BTNR_Internal_Params_TunerBias_s
{
	BTNR_Internal_Params_TunerBias_eSensitivity=0,
	BTNR_Internal_Params_TunerBias_eACI=1,
	BTNR_Internal_Params_TunerBias_eLINEAR=2
} BTNR_Internal_Params_TunerBias_t;

typedef struct BTNR_3x7x_Internal_Params_s
{
	BTNR_Internal_Params_OffOn_t       LNA_Enable;
	BTNR_Internal_Params_SDADC_Input_t SDADC_Input;
	BTNR_Internal_Params_TunerRFFIL_t  RFFIL_Select;
	BTNR_Internal_Params_OffOn_t       HRC_Enable;
	BTNR_Internal_Params_TunerBias_t   Bias_Select;
	uint32_t						   IF_Freq;
	bool						       RFInputModeComplete;
	bool						       PowerUpDone;
	bool							   TunerInitComplete;
}BTNR_3x7x_Internal_Params_t;

typedef enum BTNR_BBSConnectMode_s
{
  BTNR_BBSConnectMode_Tune = 1,
  BTNR_BBSConnectMode_ResetStatus = 2,
  BTNR_BBSConnectMode_EnableStatus = 4,
  BTNR_BBSConnectMode_RfMode = 8,
  BTNR_BBSConnectMode_EnableLoop = 16,
  BTNR_BBSConnectMode_EnableDaisy = 32,
  BTNR_BBSConnectMode_EnableDPM = 64,
  BTNR_BBSConnectMode_SetExternalGain = 128
} BTNR_BBSConnectMode_t;

/****************************************************
 *BTNR_3x7x_BBS_Params_t definition:
 ***************************************************/
typedef struct BTNR_3x7x_BBS_Params_t
{
	uint32_t							StartSturctureAddress;
	volatile BTNR_BBSConnectMode_t		BBSConnectMode;
	BTNR_TunerApplicationMode_t			Application;
	BTNR_Standard_t						Standard;
	BTNR_LPF_Bandwidth_t				LPF_Bandwidth;
    BTNR_3x7x_TunerRfInputMode_t		BTNR_RF_Input_Mode;
	BTNR_3x7x_LoopThru_Params_t			BTNR_LoopThru_Params;
	BTNR_3x7x_Daisy_Params_t			BTNR_Daisy_Params;
	BTNR_3x7x_DPM_Params_t				BTNR_DPM_Params;
	BTNR_3x7x_Gain_Params_t				BTNR_Gain_Params;
}BTNR_3x7x_BBS_Params_t;

/****************************************************
 *BTNR_3x7x_TuneParams_t definition:
 ***************************************************/
/*This is the main structure by the tuning functions*/
typedef struct BTNR_3x7x_TuneParams_s
{
#ifndef LEAP_BASED_CODE
	BTNR_3x7x_BBS_Params_t		 BTNR_BBS_Params;
#endif
	BTNR_3x7x_Tuner_Power_Mode_t BTNR_TunePowerMode;
	BTNR_3x7x_TuneType_t         BTNR_TuneType;
	BTNR_3x7x_Acquire_Params_t   BTNR_Acquire_Params;
	BTNR_3x7x_DPM_Params_t       BTNR_DPM_Params;
	BTNR_3x7x_LoopThru_Params_t  BTNR_LoopThru_Params;
	BTNR_3x7x_Daisy_Params_t     BTNR_Daisy_Params;
	BTNR_3x7x_Gain_Params_t		 BTNR_Gain_Params;
    BTNR_3x7x_TunerRfInputMode_t BTNR_RF_Input_Mode;
	BTNR_3x7x_Internal_Params_t  BTNR_Internal_Params;
	BTNR_3x7x_Local_Params_t     BTNR_Local_Params;
}BTNR_3x7x_TuneParams_t;

typedef enum BTNR_PowerStatus_s
{
    BTNR_ePower_Off = 0,
    BTNR_ePower_On  = 1,
    BTNR_ePower_Unknown = 0xFF
}  BTNR_PowerStatus_t;

typedef enum BADS_3x7x_Status_UnlockLock_s
{
  BTNR_Status_eUnlock=0,
  BTNR_Status_eLock=1
}BTNR_3x7x_Status_UnlockLock_t;

/****************************************************
 *BTNR_3x7x_TuneStatus_t definition:
 ***************************************************/
/*This is the main structure by the tuning status*/
typedef struct BTNR_3x7x_TuneStatus_s
{
  uint32_t      Lock;
	BTNR_3x7x_Status_UnlockLock_t		Tuner_Ref_Lock_Status;
	BTNR_3x7x_Status_UnlockLock_t		Tuner_Phy_Lock_Status;
	BTNR_3x7x_Status_UnlockLock_t		Tuner_Mixer_Lock_Status;
	BTNR_PowerStatus_t                  PowerStatus;
	uint32_t                            Tuner_Ref_Freq;
	uint32_t                            Tuner_RefPll_Freq;
	uint32_t                            Tuner_PhyPll1_Freq;
	uint32_t                            Tuner_PhyPll2_Freq;
	uint32_t                            Tuner_PhyPll3_Freq;
	uint32_t                            Tuner_PhyPll4_Freq;
	uint32_t                            Tuner_PhyPll5_Freq;
	uint32_t                            Tuner_PhyPll6_Freq;
	uint32_t							Tuner_LNA_Gain_Code;
	uint32_t							Tuner_RFVGA_Gain_Code;
	uint32_t							Tuner_RF_Freq;
	uint32_t							Tuner_RF_Freq_pre;
	int32_t								EstChannelPower_dbm_i32;
	int16_t								Tuner_PreADC_Gain_x256db;
	int16_t								External_Gain_x256db;
	int8_t								Tuner_RFFIL_band;
	int8_t								Tuner_RFFIL_tune;
	int8_t								RFAGC_indirect_addr;
	uint32_t							RFAGC_indirect_data;
	uint8_t								Tuner_Change_Settings;
	uint8_t								LO_index;
	uint8_t								RFAGC_dither;
	uint8_t								External_FGLNA_Mode;
	uint8_t								AGC_BW;
	uint8_t								LO_LDO;
	int32_t								g_IQ_phs_DEG;
	int32_t								g_IQ_phs_AMP;
	uint8_t								dly;
	uint16_t							cval;
	int16_t								TNR_backoff;
	uint8_t								UFE_AGC_BW;
	uint8_t								MAX_gain_code;
	uint8_t								AGC_TOP;
	uint8_t								rev;
	uint8_t								CodeRate;
	uint8_t								QAM;
	int16_t								Tuner_Sensitivity;
	uint8_t								LNA_TOP;
	uint8_t								RFAGC_MAX;
	uint8_t								TOP_DOWN_ACI;
	uint8_t								TOP_DOWN_ACI_BOOST;
	uint8_t								TOP_UP_ACI;
	uint8_t								TOP_UP_ACI_BOOST;
	uint8_t								TOP_DOWN_LIN;
	uint8_t								TOP_DOWN_LIN_BOOST;
	uint8_t								TOP_UP_LIN;
	uint8_t								TOP_UP_LIN_BOOST;
	uint8_t								RFVGA_byp;
	uint16_t							LNA_atten;
	uint16_t							VGA_atten;
	int32_t								SNR_margin;
	uint8_t								REFPLL;
	int8_t								LNAAGC_count;
	uint8_t								ACI_near;
	uint8_t								ACI_far;
	uint8_t								LNAAGC_FRZ;
} BTNR_3x7x_TuneStatus_t;

#ifdef __cplusplus
}
#endif

#endif /* _BTNR_STRUCT_H__ */





