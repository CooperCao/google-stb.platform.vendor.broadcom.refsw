/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#ifndef BTNR_7125_PRIV_H__
#define BTNR_7125_PRIV_H__

#include "bchp.h"
#include "breg_mem.h"
#include "bint.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "berr_ids.h"
#include "btnr.h"
#include "bmem.h"
#include "breg_i2c.h"
#include "btnr_7125.h"
#include "bchp_ds_tuner_0.h"
#include "bchp_ds_tuner_1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BTNR_7125_P_UNIT_REG_OFFSET (BCHP_DS_TUNER_1_TUNER_TOP_CNTL0-BCHP_DS_TUNER_0_TUNER_TOP_CNTL0)

#define BTNR_7125_P_WriteField(h, Register, Field, Data, UnitNum) \
  BREG_Write32(h, (BCHP_##Register+(BTNR_7125_P_UNIT_REG_OFFSET*UnitNum)),\
   ((BREG_Read32(h, (BCHP_##Register+(BTNR_7125_P_UNIT_REG_OFFSET*UnitNum))) & \
  ~((uint32_t)BCHP_MASK(Register, Field))) | \
  BCHP_FIELD_DATA(Register, Field, Data)))

#define BTNR_7125_P_Write32(h, Register, Data, UnitNum) \
  BREG_Write32(h, (Register+(BTNR_7125_P_UNIT_REG_OFFSET*UnitNum)),Data)

#define BTNR_7125_P_ReadField(h, Register, Field, UnitNum) \
  ((((BREG_Read32(h, (BCHP_##Register+(BTNR_7125_P_UNIT_REG_OFFSET*UnitNum)))) & \
  BCHP_MASK(Register,Field)) >> BCHP_SHIFT(Register,Field)))

#define BTNR_7125_P_Read32(h, Register, UnitNum) \
  (BREG_Read32(h, (Register+(BTNR_7125_P_UNIT_REG_OFFSET*UnitNum))))

#ifdef TUNE_BBS_TEST
#define INIT_BBS_CLICK  0x1
#define TUNE_BBS_CLICK  0x2
#endif

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/

typedef struct BTNR_P_7125_Handle     *BTNR_7125_Handle;

typedef enum BTNR_LNA_ID_s
{
	BTNR_LNAID_3410 = 0,
	BTNR_LNAID_3412 = 1
}BTNR_LNA_ID_t;

typedef enum BTNR_OFFON_s
{
	BTNR_OFF = 0,
	BTNR_ON = 1
}BTNR_OFFON_t;

typedef enum SignalSource_s
{
	InternalTuner=0,
	ExternalTuner=1
} SignalSource_t;


typedef enum TNR_TuneStartMode_s
{
  TNR_TuneStartMode_Manual = 1, 
  TNR_TuneStartMode_ResetStatus = 2,
  TNR_TuneStartMode_DPMStatus = 4
} TNR_TuneStartMode_t;

typedef enum TNR_NexusStatusMode_s
{
  TNR_NexusStatusMode_EnableStatusForNexus = 1
} TNR_NexusStatusMode_t;

typedef enum TNR_NexusParamsMode_s
{
  TNR_NexusParamsMode_eDisable = 1
} TNR_NexusParamsMode_t;

typedef enum TNR_DeviceTechType_s
{
  TNR_DeviceTechType_e65nm = 0,
  TNR_DeviceTechType_e60nm = 1
} TNR_DeviceTechType_t;

/* internal Acquire settings */
typedef struct  BTNR_P_7125_AcquireSettings
{
	uint32_t												StartSturctureAddress;
	volatile TNR_TuneStartMode_t		BBSConnectMode;
	TNR_NexusParamsMode_t						NexusParamsDisable;					
	BTNR_7125_TunerAcquisitionType	TunerAcquisitionType;
	BTNR_7125_Bandwidth							TunerBw;
	uint32_t												TunerFreq;
	volatile TNR_NexusStatusMode_t  BBSNexusConnectMode;
	uint32_t			BBS_AcqWord0;					/*Beginning of 37 words of memory for BBS*/ 
	uint32_t			BBS_AcqWord1;
	uint32_t			BBS_AcqWord2;
	uint32_t			BBS_AcqWord3;
	uint32_t			BBS_LoopVGAGain_Init_Dwell;				 
	uint32_t			BBS_LoopVGAGain_VGA_Dwell;	 
	uint32_t			BBS_LoopVGAGain_notVGA_Dwell;
	uint32_t			BBS_LoopVGAGain_Display;
	uint32_t			BBS_Input_Source;
	uint32_t			BBS_Rerun_Init;
	uint32_t			BBS_THD_RF_AGC_Acq_BW;
	uint32_t 			BBS_THD_IF_AGC_Acq_BW;
	uint32_t 			BBS_THD_RF_AGC_Trk_BW;
	uint32_t			BBS_THD_IF_AGC_Trk_BW;
	uint32_t 			BBS_THD_DCOC_On;
	uint32_t 			BBS_THD_DCOC_Acq_BW;
	uint32_t 			BBS_THD_DCOC_Trk_BW;
	uint32_t 			BBS_THD_BCM3410_Auto;
	uint32_t 			BBS_THD_BCM3410_Gain_State;
	uint32_t 			BBS_THD_LNA_Boost_On;
	uint32_t 			BBS_THD_LNA_Tilt_On;
	uint32_t 			BBS_THD_LNA_Tilt_Freq;
	uint32_t 			BBS_DS_RF_AGC_Acq_BW;
	uint32_t 			BBS_DS_IF_AGC_Acq_BW;
	uint32_t 			BBS_DS_RF_AGC_Trk_BW;
	uint32_t 			BBS_DS_IF_AGC_Trk_BW;
	uint32_t 			BBS_DS_DCOC_On;
	uint32_t 			BBS_DS_DCOC_Acq_BW;
	uint32_t 			BBS_DS_DCOC_Trk_BW;
	uint32_t 			BBS_DS_BCM3410_Auto;
	uint32_t 			BBS_DS_BCM3410_Gain_State;
	uint32_t 			BBS_DS_LNA_Boost_On;
	uint32_t 			BBS_DS_LNA_Tilt_On;
	uint32_t 			BBS_DS_LNA_Tilt_Freq;		
	uint32_t			BBS_Cable_LNA_Address;
	uint32_t			BBS_Force_TunerAcquisitionType; /*End of 37 words of memory for BBS*/
	BTNR_OFFON_t		DoneLoadFlag;
	BTNR_OFFON_t		DoneFirstTimeFlag;
	BTNR_OFFON_t		ReRunInit;
	uint8_t				Ref_PLL_Status;
	uint8_t				Mixer_PLL_Status;
	uint8_t				LNA_Gain;
	BTNR_LNA_ID_t		LNA_ChipID;
	bool				RFbypassMode;	/*this is part of the PI but since it is bool it needs to be after the BBS stuff*/
	int32_t 			Min_AGC_Threshold;	   /* AGC thresholds to be set by VGA CAL:  Lower AGC Threshold */
	int32_t				Max_AGC_Threshold;    /* Upper Threshold */
} BTNR_P_7125_AcquireSettings_t;

/*structure for status parameters to/from Kaylin to C script	*/   
/*structure for status parameters to/from BBS to C script 		*/ 
typedef struct BTNR_7125_P_Status
{ 
  uint32_t	Tuner_Ref_Lock_Status; 		/*Beginning of 38 words of memory for BBS*/
  uint32_t	Tuner_Mixer_Lock_Status;
	uint32_t  Tuner_AGC_Status;
  uint32_t	TunerFreq;
  uint32_t	TunerMode;
  uint32_t	TunerBW;
  uint32_t	Cable_LNA_Gain;
  uint32_t	Cable_RF_Attenuation;
  uint32_t	Cable_RF_Tracking_Gain;
  uint32_t	Cable_RF_Tracking_BW;
  uint32_t	Cable_RF_Tracking_Roof;
  uint32_t	Terrestrial_LNA_Gain;
  uint32_t	Terrestrial_RFAGC_Gain;
  uint32_t	Terrestrial_LNA2_Gain;
	uint32_t	FGA_Gain;
  uint32_t	IF_VGA_Gain_Range;
  uint32_t	IF_VGA_Control_Value;
  uint32_t  LNA_AGC_LEVEL;
  uint32_t  RF_AGC_LEVEL;
  uint32_t  IF_AGC_LEVEL;
  uint32_t  IFDCVAL_I;
  uint32_t  IFDCVAL_Q;
	uint32_t	LNA_Gain_256db;
	uint32_t	Cable_HarmonicFilter;
	uint32_t	Cable_RF_Gain_256db;
	uint32_t	Cable_RF_TRKGain_256db;
	uint32_t	FGA_Gain_256db;
	int32_t		IF_PGA_Gain_256db; 
	uint32_t	IF_VGA_Gain_256db; 
	int32_t		ADC_IPGA_Gain_256db;  
	int32_t		ADC_QPGA_Gain_256db;
	int32_t		ADC_EXTPGA_Gain_256db;
	uint32_t	TotalGainToADC;
	int32_t		AGF_Gain_256db;
	int32_t		AGF2_Gain_256db;
	int32_t		InputPower_256db;
	uint32_t	LNA_Boost_Status;
	uint32_t	LNA_Tilt_Status;
	uint32_t  	DPM_Gain_256dbmV;  /*End of 38 words of memory for BBS*/
	TNR_DeviceTechType_t    Device_Tech_type;
}BTNR_7125_P_Status_t;

typedef struct BTNR_P_7125_Settings
{
    unsigned long rfFreq;
    int iRevLetter, iRevNumber, iType;      /* Saved chip information */
    BTNR_TunerMode tunerMode;
    bool powerSaver;
	unsigned unitNumber;
} BTNR_P_7125_Settings;

typedef struct BTNR_P_7125_Handle
{
    uint32_t magicId;                   /* Used to check if structure is corrupt */
	BREG_Handle hRegister;
    BTNR_P_7125_Settings settings;
	BTNR_P_7125_AcquireSettings_t *pTnrModeData;
	BTNR_7125_P_Status_t *pTnrStatus;
	BTNR_P_CallbackInfo InterruptCallbackInfo;
	BREG_I2C_Handle hI2cLNA;
	BMEM_Heap_Handle hHeap;
	BTMR_TimerHandle hTimer;
	BKNI_EventHandle hInterruptEvent;
} BTNR_P_7125_Handle;

/* function prototypes in btnr_7125_acquire.c*/
BERR_Code BTNR_7125_P_Init_TNR_Core0(BTNR_7125_Handle);
BERR_Code BTNR_7125_P_Tune_TNR_Core0(BTNR_7125_Handle);
BERR_Code BTNR_7125_P_Get_LockStatus_TNR_Core0(BTNR_7125_Handle);

/* function prototypes in btnr_7125_status.c*/
BERR_Code BTNR_7125_P_Reset_Status_TNR_Core0(BTNR_7125_Handle);
BERR_Code BTNR_7125_P_Get_Status_TNR_Core0(BTNR_7125_Handle);
BERR_Code BTNR_7125_P_Get_DPM(BTNR_7125_Handle);

/* function prototypes in btnr_7125_utils.c*/
BERR_Code BTNR_7125_P_PowerDn_TNR_Core0(BTNR_7125_Handle);
BERR_Code BTNR_7125_P_PowerUp_TNR_Core0(BTNR_7125_Handle);

void BTNR_7125_P_Set_Rf_ByPass_mode(BTNR_7125_Handle, bool);
void BTNR_7125_P_Get_LNA_Status(BTNR_7125_Handle);

/*******************************************************************************
*
*   Private Module Data
*
*******************************************************************************/
BERR_Code BTNR_7125_Close(
    BTNR_Handle hDev                   		/* [in] Device handle */
    );

BERR_Code BTNR_7125_SetRfFreq(
    BTNR_7125_Handle hDev,                /* [in] Device handle */
    uint32_t rfFreq,                    	/* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            	/* [in] Requested tuner mode */
    );

BERR_Code BTNR_7125_GetRfFreq(
    BTNR_7125_Handle hDev,                /* [in] Device handle */
    uint32_t *rfFreq,                   	/* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           	/* [output] Returns tuner mode */
    );

BERR_Code BTNR_P_7125_GetAgcRegVal(
    BTNR_7125_Handle hDev,                /* [in] Device handle */
    uint32_t regOffset,                 	/* [in] AGC register offset */
    uint32_t *agcVal                    	/* [out] output value */
    );

BERR_Code BTNR_P_7125_SetAgcRegVal(
	BTNR_7125_Handle hDev,                /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    );

BERR_Code BTNR_7125_GetInfo(
    BTNR_7125_Handle hDev,                /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             	/* [out] Tuner information */
    );

BERR_Code BTNR_7125_GetPowerSaver(
	BTNR_7125_Handle hDev,				/* [in] Device handle */
	BTNR_PowerSaverSettings *pwrSettings	/* [in] Power saver settings. */
	);

BERR_Code BTNR_7125_SetPowerSaver(
	BTNR_7125_Handle hDev,				/* [in] Device handle */
	BTNR_PowerSaverSettings *pwrSettings 	/* [in] Power saver settings. */
	);



#ifdef __cplusplus
    }
#endif
    
#endif

