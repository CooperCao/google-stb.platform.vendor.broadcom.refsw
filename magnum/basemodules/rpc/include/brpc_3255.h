/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef __BRPC_3255_H
#define __BRPC_3255_H

#if __cplusplus
extern "C" {
#endif

#define MX_IQ_PER_GET	16		/* max number of QAM/QPSK soft decision results*/
#define MX_PREMABLE_SZ	(0x7f)	/* max size of premeable pattern in Upstream*/

typedef enum BRPC_DevId
{
	BRPC_DevId_3255			= 0,
	BRPC_DevId_3255_US0 	= 1,
	BRPC_DevId_3255_OB0		= 2,
	BRPC_DevId_3255_DS0		= 10,
	BRPC_DevId_3255_DS1		= 11,
	BRPC_DevId_3255_DS2		= 12,
	BRPC_DevId_3255_DS3		= 13,
	BRPC_DevId_3255_DS4		= 14,
	BRPC_DevId_3255_DS5		= 15,
	BRPC_DevId_3255_DS6		= 16,
	BRPC_DevId_3255_DS7		= 17,

	BRPC_DevId_3255_TNR0	= 20,
	BRPC_DevId_3255_TNR1	= 21,
	BRPC_DevId_3255_TNR2	= 22,
	BRPC_DevId_3255_TNR3	= 23,
	BRPC_DevId_3255_TNR4	= 24,
	BRPC_DevId_3255_TNR5	= 25,
	BRPC_DevId_3255_TNR6	= 26,
	BRPC_DevId_3255_TNR7	= 27,

	BRPC_DevId_3255_POD		= 30,
	#if (defined(VENDOR_GPIO_CONTROL) || defined(VENDOR_REQUEST))
	BRPC_DevId_3255_VEN     = 31,
	#endif

	BRPC_DevId_3255_TNR0_OOB= 40,

    /* Newer chip now has more channels, 16 to 40 channels. */
    /* Define another set of consecutive channels for these chips. */

    /* Reserved 50 channels from 50-99. */
	BRPC_DevId_ECM_DS0		= 50,
	BRPC_DevId_ECM_DS1		= 51,
	BRPC_DevId_ECM_DS2		= 52,
	BRPC_DevId_ECM_DS3		= 53,
	BRPC_DevId_ECM_DS4		= 54,
	BRPC_DevId_ECM_DS5		= 55,
	BRPC_DevId_ECM_DS6		= 56,
	BRPC_DevId_ECM_DS7		= 57,
	BRPC_DevId_ECM_DS8		= 58,
	BRPC_DevId_ECM_DS9		= 59,
	BRPC_DevId_ECM_DS10		= 60,
	BRPC_DevId_ECM_DS11		= 61,
	BRPC_DevId_ECM_DS12		= 62,
	BRPC_DevId_ECM_DS13		= 63,
	BRPC_DevId_ECM_DS14		= 64,
	BRPC_DevId_ECM_DS15		= 65,
	BRPC_DevId_ECM_DS16		= 66,
	BRPC_DevId_ECM_DS17		= 67,
	BRPC_DevId_ECM_DS18		= 68,
	BRPC_DevId_ECM_DS19		= 69,
	BRPC_DevId_ECM_DS20 	= 70,
	BRPC_DevId_ECM_DS21 	= 71,
	BRPC_DevId_ECM_DS22 	= 72,
	BRPC_DevId_ECM_DS23 	= 73,
	BRPC_DevId_ECM_DS24 	= 74,
	BRPC_DevId_ECM_DS25 	= 75,
	BRPC_DevId_ECM_DS26 	= 76,
	BRPC_DevId_ECM_DS27 	= 77,
	BRPC_DevId_ECM_DS28 	= 78,
	BRPC_DevId_ECM_DS29 	= 79,
	BRPC_DevId_ECM_DS30 	= 80,
	BRPC_DevId_ECM_DS31 	= 81,
	BRPC_DevId_ECM_DS32 	= 82,
	BRPC_DevId_ECM_DS33 	= 83,
	BRPC_DevId_ECM_DS34 	= 84,
	BRPC_DevId_ECM_DS35 	= 85,
	BRPC_DevId_ECM_DS36 	= 86,
	BRPC_DevId_ECM_DS37 	= 87,
	BRPC_DevId_ECM_DS38 	= 88,
	BRPC_DevId_ECM_DS39 	= 89,
	BRPC_DevId_ECM_DS40 	= 90,
	BRPC_DevId_ECM_DS41 	= 91,
	BRPC_DevId_ECM_DS42 	= 92,
	BRPC_DevId_ECM_DS43 	= 93,
	BRPC_DevId_ECM_DS44 	= 94,
	BRPC_DevId_ECM_DS45 	= 95,
	BRPC_DevId_ECM_DS46 	= 96,
	BRPC_DevId_ECM_DS47 	= 97,
	BRPC_DevId_ECM_DS48 	= 98,
	BRPC_DevId_ECM_DS_LAST	= 99,

    /* Reserved 50 channels from 100-149 */
    BRPC_DevId_ECM_TNR0     = 100,
	BRPC_DevId_ECM_TNR1     = 101,
	BRPC_DevId_ECM_TNR2     = 102,
	BRPC_DevId_ECM_TNR3     = 103,
	BRPC_DevId_ECM_TNR4     = 104,
	BRPC_DevId_ECM_TNR5     = 105,
	BRPC_DevId_ECM_TNR6     = 106,
	BRPC_DevId_ECM_TNR7     = 107,
	BRPC_DevId_ECM_TNR8     = 108,
	BRPC_DevId_ECM_TNR9     = 109,
	BRPC_DevId_ECM_TNR10    = 110,
	BRPC_DevId_ECM_TNR11    = 111,
	BRPC_DevId_ECM_TNR12    = 112,
	BRPC_DevId_ECM_TNR13    = 113,
	BRPC_DevId_ECM_TNR14    = 114,
	BRPC_DevId_ECM_TNR15    = 115,
	BRPC_DevId_ECM_TNR16    = 116,
	BRPC_DevId_ECM_TNR17    = 117,
	BRPC_DevId_ECM_TNR18    = 118,
	BRPC_DevId_ECM_TNR19    = 119,
    BRPC_DevId_ECM_TNR20    = 120,
	BRPC_DevId_ECM_TNR21    = 121,
	BRPC_DevId_ECM_TNR22    = 122,
	BRPC_DevId_ECM_TNR23    = 123,
	BRPC_DevId_ECM_TNR24    = 124,
	BRPC_DevId_ECM_TNR25    = 125,
	BRPC_DevId_ECM_TNR26    = 126,
	BRPC_DevId_ECM_TNR27    = 127,
	BRPC_DevId_ECM_TNR28    = 128,
	BRPC_DevId_ECM_TNR29    = 129,
	BRPC_DevId_ECM_TNR30    = 130,
	BRPC_DevId_ECM_TNR31    = 131,
	BRPC_DevId_ECM_TNR32    = 132,
	BRPC_DevId_ECM_TNR33    = 133,
	BRPC_DevId_ECM_TNR34    = 134,
	BRPC_DevId_ECM_TNR35    = 135,
	BRPC_DevId_ECM_TNR36    = 136,
	BRPC_DevId_ECM_TNR37    = 137,
	BRPC_DevId_ECM_TNR38    = 138,
	BRPC_DevId_ECM_TNR39    = 139,
    BRPC_DevId_ECM_TNR40    = 140,
	BRPC_DevId_ECM_TNR41    = 141,
	BRPC_DevId_ECM_TNR42    = 142,
	BRPC_DevId_ECM_TNR43    = 143,
	BRPC_DevId_ECM_TNR44    = 144,
	BRPC_DevId_ECM_TNR45    = 145,
	BRPC_DevId_ECM_TNR46    = 146,
	BRPC_DevId_ECM_TNR47    = 147,
	BRPC_DevId_ECM_TNR48    = 148,
    BRPC_DevId_ECM_TNR_LAST = 149,

	BRPC_ProcId_LastDevId
} BRPC_DevId;


typedef enum BRPC_ProcId
{
	BRPC_ProcId_Reserved = 0,
	BRPC_ProcId_InitSession = 1,

	BRPC_ProcId_ADS_GetVersion = 2,
	BRPC_ProcId_ADS_GetTotalChannels = 3,
	BRPC_ProcId_ADS_OpenChannel = 4,
	BRPC_ProcId_ADS_CloseChannel = 5,
	BRPC_ProcId_ADS_GetChannelDefaultSettings = 6,
	BRPC_ProcId_ADS_GetStatus = 7,
	BRPC_ProcId_ADS_Acquire = 8,
	BRPC_ProcId_ADS_EnablePowerSaver = 9,
	BRPC_ProcId_ADS_GetSoftDecision = 10,
	BRPC_ProcId_ADS_GetLockStatus = 11,

	BRPC_ProcId_TNR_Open = 12,
	BRPC_ProcId_TNR_Close = 13,
	BRPC_ProcId_TNR_GetVersion = 14,
	BRPC_ProcId_TNR_SetRfFreq = 15,

	BRPC_ProcId_TNR_Oob_Open = 16,
	BRPC_ProcId_TNR_Oob_Close = 17,
	BRPC_ProcId_TNR_Oob_SetRfFreq = 18,

	BRPC_ProcId_AOB_Open = 19,
	BRPC_ProcId_AOB_Close = 20,
	BRPC_ProcId_AOB_Acquire = 21,
	BRPC_ProcId_AOB_GetStatus = 22,
	BRPC_ProcId_AOB_GetLockStatus = 23,
	BRPC_ProcId_AOB_EnablePowerSaver = 24,
	BRPC_ProcId_AOB_GetSoftDecision = 25,

	BRPC_ProcId_AUS_Open = 26,
	BRPC_ProcId_AUS_Close = 27,
	BRPC_ProcId_AUS_SetOperationMode = 28,
	BRPC_ProcId_AUS_SetSymbolRate = 29,
	BRPC_ProcId_AUS_SetRfFreq = 30,
	BRPC_ProcId_AUS_SetPowerLevel = 31,
	BRPC_ProcId_AUS_GetStatus = 32,
	BRPC_ProcId_AUS_SetPreamblePattern = 34,
	BRPC_ProcId_AUS_SetBurstProfile = 35,
	BRPC_ProcId_AUS_SetRawPowerLevel = 36,
	BRPC_ProcId_AUS_TxStarVuePkt = 37,
	BRPC_ProcId_AUS_SetTransmitMode = 38,

#ifdef VENDOR_GPIO_CONTROL
	BRPC_ProcId_VEN_GpioControl = 39,
#endif
#ifdef VENDOR_REQUEST
	BRPC_ProcId_VEN_Request = 40,
#endif

	BRPC_ProcId_ADS_GetDsChannelPower = 41,
	    
	BRPC_ProcId_POD_CardOut = 43,
	BRPC_ProcId_POD_CardIn = 44,
	BRPC_ProcId_POD_CardInStart = 45,
	BRPC_ProcId_POD_CardApplyPower = 46,

	BRPC_ProcId_AOB_SetSpectrum = 50,
	BRPC_ProcId_TNR_GetAgcVal= 51,
	BRPC_ProcId_TNR_EnablePowerSaver= 52,
	BRPC_ProcId_TNR_GetPowerSaver= 53,
	BRPC_ProcId_AUS_GetTransmitMode = 54,
	BRPC_ProcId_ADS_GetBondingCapability = 55,
	BRPC_ProcId_ADS_ReserveChannel = 56,
	BRPC_ProcId_ECM_GetStatus = 57,
	BRPC_ProcId_ECM_PowerSaver = 58,
	BRPC_ProcId_ECM_Transit_Frontend_Control_to_Host = 59,
	BRPC_ProcId_ECM_Transit_Frontend_Control_to_Bnm = 60,

#if ((BCHP_CHIP == 7425) || (BCHP_CHIP == 7435) || (BCHP_CHIP == 7429) || (BCHP_CHIP == 74295) || (BCHP_CHIP == 7439))
	BRPC_ProcId_ECM_TSMF_GetFldVerifyConfig = 61,
	BRPC_ProcId_ECM_TSMF_SetFldVerifyConfig = 62,
	BRPC_ProcId_ECM_TSMF_EnableAutoMode = 63,
	BRPC_ProcId_ECM_TSMF_EnableSemiAutoMode = 64,
	BRPC_ProcId_ECM_TSMF_DisableTsmf = 65,
	BRPC_ProcId_ECM_TSMF_SetParserConfig = 66,

	BRPC_ProcId_ADS_SetParserBandId = 67,
	BRPC_ProcId_ECM_ReadXptBlock = 68,
	BRPC_ProcId_ECM_WriteXptBlock = 69,
	BRPC_ProcId_ECM_HostChannelsLockStatus = 70,
	BRPC_ProcId_ECM_DoLnaReConfig = 71,
	BRPC_ProcId_ECM_ReadDieTemperature = 72,
#endif
	BRPC_ProcId_ECM_GetSystemInfo = 302,
	BRPC_ProcId_ECM_GetWapStatus = 303,
	BRPC_ProcId_ECM_GetMtaStatus = 304,
	BRPC_ProcId_ECM_GetRouterStatus = 305,

	BRPC_ProcId_LastProcId
} BRPC_ProcId;


typedef struct BRPC_Param_InitSession
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_InitSession;


typedef struct BRPC_Param_XXX_Get
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_XXX_Get;

/* POD RPC parameters  */
typedef struct BRPC_Param_POD_CardOut
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_POD_CardOut;

typedef struct BRPC_Param_POD_CardIn
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_POD_CardIn;

typedef struct BRPC_Param_POD_CardInStart
{
	uint32_t devId;					/* device id, see BRPC_DevId */
} BRPC_Param_POD_CardInStart;

#define MCARD_3VOLT             0       /* use only for bcm97456-A0 MBV0002 ref desgin */
#define SCARD_3VOLT             1       /* Use on later MBV100x ref design, all platforms */
#define SCARD_5VOLT             2
#define MCARD_5VOLT             3
#define ENABLE_POD_OUTPINS      4       /* Works only on bcm3255-A3 or later */
#define DISABLE_POD_OUTPINS     5       /* Do nothing for earlier rev chip */
typedef struct BRPC_Param_POD_CardApplyPower
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t powerMode;				/* Mcard_3V, Scard_3V and Scard_5V */
} BRPC_Param_POD_CardApplyPower;

typedef struct BRPC_Param_TNR_GetVersion
{
	uint32_t manafactureId;
	uint32_t modelId;
	uint32_t majVer;
	uint32_t minVer;
} BRPC_Param_TNR_GetVersion;

typedef struct BRPC_Param_TNR_Open
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t ifFreq;					/* IF Frequency in Hertz */
} BRPC_Param_TNR_Open;


typedef struct BRPC_Param_TNR_Close
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_TNR_Close;

typedef struct BRPC_Param_TNR_GetAgcVal
{
    	/* Byte3-Byte2 = 16 bit chipid. Ex: 0x3412
       	Byte1 = T1T2|0SBT.  Out1Tilt(b7b6)/Out2Tilt(b5b4)|0(b3)/SuperBoost(b2)/Boost(b1)/Tilt(b0).  {0=OFF, 1=ON}
       	Byte0 = b000(b7-b5)/AGC gain value 0x0-0x1F(b4-b0)
     	+-------+-------+---------------+-------+
     	|  LnaChipId    | T1/T2|0/S/B/T |  AGC  |
     	+-------+-------+---------------+-------+
            B3      B2          B1         B0
    	*/
	uint32_t AgcVal;					/* AGC value*/
} BRPC_Param_TNR_GetAgcVal;

typedef struct BRPC_Param_TNR_EnablePowerSaver
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_TNR_EnablePowerSaver;

typedef struct BRPC_Param_TNR_GetPowerSaver
{
	uint32_t powersaver;      /* 1 : standby mode; 0 : non-standby mode*/
} BRPC_Param_TNR_GetPowerSaver;

/* 
  Added new BTNR_TunerMode_eDocsis. Usage:
  (1) To stop Docsis scanning or terminate Docsis & use DS0 for video channel, call this API to BRPC_DevId_3255_DS0 with BTNR_TunerMode_eDigital & a valid video freq.
  **note: shut down Docsis takes about 1-2sec**
  (2) To terminate DS0 video or start Docsis scanning, call this API to BRPC_DevId_3255_DS0 with BTNR_TunerMode_eDocsis & rfFreq=0.
  (3) To terminate DS0 video or ask Docsis to lock to a specific freq, call this API to BRPC_DevId_3255_DS0 with BTNR_TunerMode_eDocsis & rfFreq=desired_freq.
  **note: Starting Docsis from the Host does not need ADS_Acquire, thus no lock notification will be received by the Host
*/
typedef struct BRPC_Param_TNR_SetRfFreq
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t rfFreq;					/* RF Frequency in Hertz */
	uint32_t tunerMode;					/* tunerMode:	0: BTNR_TunerMode_eDigital
														1: BTNR_TunerMode_eAnalog
														2: BTNR_TunerMode_eDocsis(new)	*/
} BRPC_Param_TNR_SetRfFreq;


typedef struct BRPC_Param_TNR_OOB_Open
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t ifFreq; 					/* IF Frequency in Hertz: Use 1250000 to indicate using DS0 as OOB receiver.(shared with Docsis tuner) */ 
										/* Other value will be ignored & use external legacy tuner as OOB receiver. IfFreq is fixed during compile time */
} BRPC_Param_TNR_OOB_Open;


typedef struct BRPC_Param_TNR_OOB_Close
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_TNR_OOB_Close;


typedef struct BRPC_Param_TNR_OOB_SetRfFreq
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t rfFreq;					/* RF Frequency in Hertz */
} BRPC_Param_TNR_OOB_SetRfFreq;


typedef struct BRPC_Param_AOB_Open
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t enableFEC;			/* Use OOB FEC or not */
} BRPC_Param_AOB_Open;


typedef struct BRPC_Param_AOB_Close
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_AOB_Close;

typedef enum BRPC_AOB_ModulationType {
	BRPC_AOB_ModulationType_eAnnexAQpsk,
	BRPC_AOB_ModulationType_eDvs178Qpsk,
	BRPC_AOB_ModulationType_ePod_AnnexAQpsk,
	BRPC_AOB_ModulationType_ePod_Dvs178Qpsk
#if ((BCHP_CHIP == 7425) || (BCHP_CHIP == 7435) || (BCHP_CHIP == 7429) || (BCHP_CHIP == 74295))
    ,BRPC_AOB_ModulationType_eBERT_TEST_MODE
#endif
        
} BRPC_AOB_ModulationType;

typedef struct BRPC_Param_AOB_Acquire
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t modType;					/* Modulation type */
	uint32_t symbolRate;				/* in Baud */
} BRPC_Param_AOB_Acquire;


typedef struct BRPC_Param_AOB_GetLockStatus
{
	uint32_t isFecLock;					/* lock=1, unlock=0 */
	uint32_t isQamLock;					/* lock=1, unlock=0 */
} BRPC_Param_AOB_GetLockStatus;

typedef struct BRPC_Param_AOB_GetStatus
{
	uint32_t modType;					/* Modulation Type */
	uint32_t ifFreq;					/* !!Now use for DS Freq!! With FullBandCapture FEe, ifFreq is not used anymore. */
	uint32_t loFreq;					/* ret 0. With FullBandCapture FE, loFreq is not used anymore. */
	uint32_t sysXtalFreq;				/* in Hertz, Sys. Xtal freq. */
	uint32_t symbolRate;				/* in Baud */
	uint32_t isFecLock;					/* lock=1, unlock=0 */
	uint32_t isQamLock;					/* lock=1, unlock=0 */
	uint32_t snrEstimate;				/* in 1/256 db */
	uint32_t agcIntLevel;				/* in 1/10 percent */
	uint32_t agcExtLevel;				/* in 1/10 percent */
	uint32_t carrierFreqOffset;			/* ret 0. in 1/1000 Hz */
	uint32_t carrierPhaseOffset;		/* ret 0. in 1/1000 Hz */
	uint32_t uncorrectedCount;			/* 16bit value,not self-clearing  */
	uint32_t correctedCount;			/* 16bit value,not self-clearing */
	uint32_t berErrorCount;				/* not self-clearing */
	uint32_t fdcChannelPower;           /* !Only valid if using DS0 for OOB! in 1/10 dBmV unit; OCAP DPM support for OOB channel */
} BRPC_Param_AOB_GetStatus;

typedef enum BRPC_BAOB_SpectrumMode
{
	BRPC_BAOB_SpectrumMode_eAuto,
	BRPC_BAOB_SpectrumMode_eNoInverted,
	BRPC_BAOB_SpectrumMode_eInverted
} BRPC_BAOB_SpectrumMode;

typedef struct BRPC_Param_AOB_SetSpectrum
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t spectrum;					/* OOB DS specturm setting */
} BRPC_Param_AOB_SetSpectrum;

typedef struct BRPC_Param_AOB_EnablePowerSaver
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_AOB_EnablePowerSaver;

typedef struct BRPC_Param_AOB_GetSoftDecisions
{
	uint32_t iVal[MX_IQ_PER_GET];					/* Array of iVal */
	uint32_t qVal[MX_IQ_PER_GET];					/* Array of iVal */
	uint32_t	nbrGotten;							/* number bytes read*/
} BRPC_Param_AOB_GetSoftDecisions;


typedef struct BRPC_Param_AUS_Open
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t xtalFreq;				/* Crystal Freqency in Hertz */
} BRPC_Param_AUS_Open;


typedef struct BRPC_Param_AUS_Close
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_AUS_Close;

typedef struct BRPC_Param_AUS_SetSetTransmitMode
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t transmitMode;			/* 0: disable Transimitter
									   1: enable Transimitter */
} BRPC_Param_AUS_SetSetTransmitMode;

typedef enum BRPC_AUS_Operation_mode {
	BRPC_AUS_OperationMode_eAnnexA = 0,
	BRPC_AUS_OperationMode_eDvs178,
	BRPC_AUS_OperationMode_eDocsis,
	BRPC_AUS_OperationMode_ePod,
	BRPC_AUS_OperationMode_eTestCw,
	BRPC_AUS_OperationMode_eTestPn23,
	BRPC_AUS_OperationMode_ePod_AnnexA,
	BRPC_AUS_OperationMode_ePod_Dvs178
} BRPC_AUS_Operation_mode;

typedef struct BRPC_Param_AUS_SetOperationMode
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t operationMode;			/* operation Mode */
} BRPC_Param_AUS_SetOperationMode;

typedef struct BRPC_Param_AUS_SetSymbolRate
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t symbolRate;			/* symbol rate in Hz */
} BRPC_Param_AUS_SetSymbolRate;

typedef struct BRPC_Param_AUS_SetRfFreq
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t rfFreq;				/* RF Frequency in Hertz */
} BRPC_Param_AUS_SetRfFreq;

typedef struct BRPC_Param_AUS_SetPreamblePattern
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t patternSize;				/* preamble Pattern Size */
	uint32_t Pattern[MX_PREMABLE_SZ];	/* preamble Pattern */
} BRPC_Param_AUS_SetPreamblePattern;

typedef struct BRPC_Param_AUS_SetBurstProfile
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t burstBank;				/* Burst band number */
	uint32_t modulationType;		/* 1=QPSK, 2=16Qam */
	uint32_t diffEncodingOnOff;		/* 1=On, 2=Off */
	uint32_t preambleLength; 		/* 9:0 bits */
	uint32_t preambleValOffset;		/* 8:0 bits */
	uint32_t fecBytes;				/* 0-10 */
	uint32_t fecCodewordInfoBytes;	/* 0, 16-253 */
	uint32_t scramblerSeed;			/* 14:0 bits */
	uint32_t maxBurstSize;			/* TODO: */
	uint32_t guardTimeSize;			/* TODO: */
	uint32_t lastCodewordLength;	/* 1=fixed, 2=shortened */
	uint32_t scramblerOnOff;		/* 1=on, 2=off */
	uint32_t nbrPreambleDiffEncoding; /* Number of Preamble different encoding */
} BRPC_Param_AUS_SetBurstProfile;

typedef struct BRPC_Param_AUS_SetPowerLevel
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t powerLevel;			/* in hundredth of dBmV*/
} BRPC_Param_AUS_SetPowerLevel;


typedef struct BRPC_Param_AUS_SetRawPowerLevel
{
	uint32_t devId;
	uint32_t powerAmpGain;
	uint32_t powerAmpDac;
} BRPC_Param_AUS_SetRawPowerLevel;

typedef struct BRPC_AUS_Status {
	uint32_t powerSaveEnabled;
	BRPC_AUS_Operation_mode operationMode;
	uint32_t xtalFreq;
	uint32_t powerLevel;
	uint32_t rfFreq;
	uint32_t symbolRate;
} BRPC_AUS_Status;

typedef struct BRPC_Param_AUS_TxStarVuePkt
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t svBuffer[14];			/* (StarVue format: 1 byte sequence + 53 byte ATM cell) */
									/* only the first 54 bytes are valid data */
} BRPC_Param_AUS_TxStarVuePkt;

typedef struct BRPC_Param_ADS_GetVersion
{
	uint32_t majVer;
	uint32_t minVer;
} BRPC_Param_ADS_GetVersion;

typedef struct BRPC_Param_ADS_ReserveChannel
{
	uint32_t num2Reserve;				/* Number of bonded channels to reserve*/
} BRPC_Param_ADS_ReserveChannel;
typedef struct BRPC_Param_ADS_GetBondingCapability
{
	uint32_t maxNum;				/* Maximum number of bonded channels */
} BRPC_Param_ADS_GetBondingCapability;
typedef struct BRPC_Param_ADS_GetTotalChannels
{
	uint32_t totalChannels;				/* Total number of available channels */
} BRPC_Param_ADS_GetTotalChannels;

typedef struct BRPC_Param_ADS_OpenChannel
{
	uint32_t devId;						/* device id, see BRPC_DevId */
	uint32_t ifFreq; 					/* IF Frequency in Hertz */
} BRPC_Param_ADS_OpenChannel;

typedef struct BRPC_Param_ADS_CloseChannel
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_ADS_CloseChannel;

typedef struct BRPC_Param_ADS_GetLockStatus
{
	uint32_t isFecLock;					/* lock=1, unlock=0 */
	uint32_t isQamLock;					/* lock=1, unlock=0 */
} BRPC_Param_ADS_GetLockStatus;

typedef struct BRPC_Param_ADS_GetStatus
{
	uint32_t isPowerSaverEnabled;		/* Eanble=1, Disable=0 */
	uint32_t modType;					/* Modulation type */
	uint32_t ifFreq;					/* in Hertz, IF freq. */
	uint32_t symbolRate;				/* in Baud */
	uint32_t isFecLock;					/* lock=1, unlock=0 */
	uint32_t isQamLock;					/* lock=1, unlock=0 */
	uint32_t correctedCount;			/* reset on every read */
	uint32_t uncorrectedCount;			/* reset on every read */
	uint32_t snrEstimate;				/* in 1/256 dB */
	uint32_t agcIntLevel;				/* in 1/10 percent */
	uint32_t agcExtLevel;				/* in 1/10 percent */
	uint32_t carrierFreqOffset;			/* in 1/1000 Hz */
	uint32_t carrierPhaseOffset;		/* in 1/1000 Hz */
	uint32_t rxSymbolRate;				/* in Baud, received symbol rate */
	uint32_t interleaveDepth;			/* return index of 0-15 instead of i&j, see ITU Recommendation J.83	*/
										/* Table B.2/J.83 - Level 2 interleaving  */
	uint32_t goodRsBlockCount;			/* reset on every read */
	uint32_t berRawCount;   			/* reset on every read !!This is only useful for self-testing PN sequence data!! */
	uint32_t dsChannelPower;			/* Deprecated starting V3.0.1alpha4 release. Will always return 0. Please use new Async API to get channel Power */
	uint32_t mainTap;					/* Channel main tap value */
	uint32_t equalizerGain;				/* Channel equalizer gain value in dBm */
	/* OCAP required postRsBER for all DS channels. postRsBER and elapsedTimeSec will be reset on every channel change*/
	/* fpostRsBER normally between xxxe-6 to xxxe-12 float value, to send this small float number over rMagnum to host, we convert it to uint32 using the formula*/
	uint32_t postRsBER;					/* Converted floating point fpostRsBER --> uint32_t postRsBER for rMagnum transport: */
										/* int ipostRsBER = (int)( log(fpostRsBER) * 1000000.0 ); uint32_t postRsBER = (uint32_t)ipostRsBER; */
										/* Host side will need this to convert it back: int ipostRsBER = (int)postRsBER; float fpostRsBER = exp( (float)ipostRsBER/1000000.0 ); */
	uint32_t elapsedTimeSec;			/* postRsBER over this time */
	uint32_t spectralInversion;			/* spectral inversion, 1 inverted, 0 normal */
#if ((BCHP_CHIP == 7425) || (BCHP_CHIP == 7435) || (BCHP_CHIP == 7429) || (BCHP_CHIP == 74295))
	uint32_t preRsBER;                  /* Convert floating point fpreRsBER --> uint32_t preRsBER for rMagnum transport: */
	                                    /* int ipreRsBER = (int)( log(fpreRsBER) * 1000000.0 ); uint32_t preRsBER = (uint32_t)ipostRsBER; */
	                                    /* Host side will need this to convert it back: int ipreRsBER = (int)preRsBER; float fpreRsBER = exp( (float)ipreRsBER/1000000.0 ); */
#endif
#if 0
	uint32_t correctedBitCount;		    /* Added 10/09/2012: CERC, Corrected Bit Counter, reset on every read */
#endif
} BRPC_Param_ADS_GetStatus;


typedef enum BRPC_ADS_ModulationType{
	/* Most of these are currently not supported, are here for future use */
	BRPC_ADS_ModulationType_eAnnexAQam16,
	BRPC_ADS_ModulationType_eAnnexAQam32,
	BRPC_ADS_ModulationType_eAnnexAQam64,
	BRPC_ADS_ModulationType_eAnnexAQam128,
	BRPC_ADS_ModulationType_eAnnexAQam256,
	BRPC_ADS_ModulationType_eAnnexAQam512,
	BRPC_ADS_ModulationType_eAnnexAQam1024,
	BRPC_ADS_ModulationType_eAnnexAQam2048,
	BRPC_ADS_ModulationType_eAnnexAQam4096,
	BRPC_ADS_ModulationType_eAnnexBQam16,
	BRPC_ADS_ModulationType_eAnnexBQam32,
	BRPC_ADS_ModulationType_eAnnexBQam64,
	BRPC_ADS_ModulationType_eAnnexBQam128,
	BRPC_ADS_ModulationType_eAnnexBQam256,
	BRPC_ADS_ModulationType_eAnnexBQam512,
	BRPC_ADS_ModulationType_eAnnexBQam1024,
	BRPC_ADS_ModulationType_eAnnexBQam2048,
	BRPC_ADS_ModulationType_eAnnexBQam4096
#if BCHP_CHIP != 7405
	,BRPC_ADS_ModulationType_eAnnexCQam16,
	BRPC_ADS_ModulationType_eAnnexCQam32,
	BRPC_ADS_ModulationType_eAnnexCQam64,
	BRPC_ADS_ModulationType_eAnnexCQam128,
	BRPC_ADS_ModulationType_eAnnexCQam256,
	BRPC_ADS_ModulationType_eAnnexCQam512,
	BRPC_ADS_ModulationType_eAnnexCQam1024,
	BRPC_ADS_ModulationType_eAnnexCQam2048,
	BRPC_ADS_ModulationType_eAnnexCQam4096
#endif
} BRPC_ADS_ModulationType;

typedef struct BRPC_Param_ADS_Acquire
{
	uint32_t devId;			/* device id, see BRPC_DevId */
	uint32_t modType;		/* Modulation type */
	uint32_t symbolRate;	/* in Baud */
	uint32_t autoAcquire;	/* 0 : Acquire once and report lock/no-lock */ 
							/* 1 (not recomended) : Acquire once and report lock/no-lock plus.. */
							/* if no-lock or lost-lock, contineous re-acquire until lock */
							/* ***Auto re-acquire on a non-existing channel is a very resource intentive process */
	/* 71xx: For internal channel only */
	uint32_t fastAcquire;	/* 1: ~80ms faster. Use faster sweep rate & skip DCO loop (may give up some performance?)*/
							/* other value: Normal acquire time. */
} BRPC_Param_ADS_Acquire;

typedef struct BRPC_Param_ADS_EnablePowerSaver
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_ADS_EnablePowerSaver;

typedef struct BRPC_Param_ADS_GetSoftDecisions
{
	uint32_t iVal[MX_IQ_PER_GET];					/* Array of iVal */
	uint32_t qVal[MX_IQ_PER_GET];					/* Array of iVal */
	uint32_t	nbrGotten;							/* number bytes read*/
} BRPC_Param_ADS_GetSoftDecisions;

#ifdef VENDOR_GPIO_CONTROL
typedef struct BRPC_Param_VEN_GpioControl
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t Operation; 			/* GPIO_OPERATION enum */
	uint32_t RegAddress;			/* 1 of the 4 GPIO registers */
	uint32_t WriteValue;			/* value to be written for Write,OR,AND operation */
									/* not used for Read operation */
} BRPC_Param_VEN_GpioControl;

typedef struct BRPC_Param_VEN_GpioReadReply
{
	uint32_t ReadBackValue;			/* return Read back register Value for all operations */
} BRPC_Param_VEN_GpioReadReply;
#endif

#ifdef VENDOR_REQUEST
#define MAX_VEN_PARAM   32
typedef struct BRPC_Param_VEN_Request
{
	uint32_t devId;					/* device id, see BRPC_DevId */
	uint32_t Operation;				/* OPERATION enum */
	uint32_t NumOfParams;			/* Number of parameters to follows */
	uint32_t Params[MAX_VEN_PARAM];	/* Allow up to 8 paramters */
} BRPC_Param_VEN_Request;

typedef struct BRPC_Param_VEN_Reply
{
	uint32_t NumberOfReply;			/* Number of Reply parameters to follows */
	uint32_t Reply[MAX_VEN_PARAM];	/* allow up to 8 reply parameters */
} BRPC_Param_VEN_Reply;
#endif

typedef struct BRPC_Param_ECM_GetStatus
{
	uint32_t downstreamCenterFreq;                /* Docsis channel DS frequency */
	uint32_t downstreamPowerLevel;                /* Docsis channel DS power */
	                                              /* Deprecated starting V3.0.1alpah4 release. Will always return 0. Please use new Async API to get channel Power */
	uint32_t downstreamCarrierLock;               /* Docsis channel DS lock status */
	uint32_t channelScdmaStatus;                  /* Docsis channel DS scdma status */
	uint32_t upstreamModuType;                    /* US modulation type */
	uint32_t upstreamXmtCenterFreq;               /* US tramsmition frequency */
	uint32_t upstreamPowerLevel;                  /* US power */
	uint32_t upStreamSymbolrate;                  /* US Symbol rate */
	uint32_t ecmMacAddressHi;                     /* eCM Mac address */
	uint32_t ecmMacAddressLo;                     /* eCM mac address */
	uint32_t isEcmIpMode;                         /* eCM IPV6: 0 no IP address yet, 1 IPV4 2 IPV6 */
	uint32_t ecmIpAddress;                        /* eCM IPV4 address */
	uint32_t ecmIpv6Address0;                     /* eCM IPv6 address */
	uint32_t ecmIpv6Address1;                     /* eCM IPv6 address */
	uint32_t ecmIpv6Address2;                     /* eCM IPv6 address */
	uint32_t ecmIpv6Address3;                     /* eCM IPv6 address */
	uint32_t lastKnownGoodFreq;		      /* Last Frequency that Docsis successfully registered on */
	uint32_t snrEstimated;						  /* Docsis channel DS primary channel SNR estimate in 1/256 dB*/
}BRPC_Param_ECM_GetStatus;

#define MAX_NUM_SSID (32)

typedef enum BRPC_ECM_Standard
{
    BRPC_ECM_Standard_e1_x,
    BRPC_ECM_Standard_e2_x,
    BRPC_ECM_Standard_e3_x,
    BRPC_ECM_Standard_eMax
}BRPC_ECM_Standard;

typedef enum BRPC_WAP_WifiProtocolType
{
    BRPC_WAP_WifiProtocolType_eUnused,       	/* frequency not used                */
    BRPC_WAP_WifiProtocolType_e80211B,     		/* 802.11b  (2.4GHz only)            */
    BRPC_WAP_WifiProtocolType_e80211G,     		/* 802.11g  (2.4GHz only)            */
    BRPC_WAP_WifiProtocolType_e80211BG,   		/* B and G (2.4GHz only)             */
    BRPC_WAP_WifiProtocolType_e80211Mixed, 		/* B/G/N mixed mode (2.4GHz only)    */
    BRPC_WAP_WifiProtocolType_e80211N,     		/* 802.11n  (2.4GHz and 5.0GHz)      */
    BRPC_WAP_WifiProtocolType_e80211AC,    		/* 802.11ac (5.0GHz only)            */
    BRPC_WAP_WifiProtocolType_eMax
}  BRPC_WAP_WifiProtocolType;

typedef enum BRPC_WAP_WifiSecurityType
{
    BRPC_WAP_WifiSecurityType_eWPA,        		/* WIFI Protected Access (partial 802.11i)                  */
    BRPC_WAP_WifiSecurityType_eMixed, 			/* WIFI Protected Access (full 802.11i-2004)  CCMP+TKIP     */
    BRPC_WAP_WifiSecurityType_eWPA2AES,   		/* WIFI Protected Access (full 802.11i-2004)  CCMP/AES only */
    BRPC_WAP_WifiSecurityType_eMax
}BRPC_WAP_WifiSecurityType;

typedef struct BRPC_Param_ECM_WAP_SSID_Info
{
    char SSID[64];           	   				/* SSID, up to 32 characters, (as a NULL terminated string) */
    uint32_t ssidEnabled;             			/* non-0 if used, 0 if disabled  */
    uint32_t ssidChannelNo;             		/* Wireless channel */
    BRPC_WAP_WifiProtocolType protocol;         /* Wireless protocol */
    BRPC_WAP_WifiSecurityType security;         /* Security scheme */
	uint32_t macAddressHi;    					/* MAC address Hi per SSID */
	uint32_t macAddressLo;    					/* MAC address Lo per SSID */
}BRPC_Param_ECM_WAP_SSID_Info;

typedef struct BRPC_Param_ECM_GetWapStatus
{
	uint32_t numberSSID;           				/* Number of configured SSIDs  */
    uint32_t wapMacAddressHi;                   /* Wap base MAC address Hi */
    uint32_t wapMacAddressLo;    				/* Wap base MAC address Lo */
	BRPC_Param_ECM_WAP_SSID_Info ssidInfo[MAX_NUM_SSID];
}BRPC_Param_ECM_GetWapStatus;

typedef enum BRPC_ECM_WapInterfaceType
{
    BRPC_ECM_WapInterfaceType_eNone,        	/* No WAP */
    BRPC_ECM_WapInterfaceType_e2_4G, 			/* WAP 2.4GHz */
    BRPC_ECM_WapInterfaceType_e5G,   		    /* WAP 5GHz   */
	BRPC_ECM_WapInterfaceType_eDual,			/* WAP dual 2.4GHz and 5GHz */
    BRPC_ECM_WapInterfaceType_eMax
}BRPC_ECM_WapInterfaceType;


typedef struct BRPC_Param_ECM_GetSystemInfo
{
    char ecmMfctName[64];        				/* Manufacturer name, (as a NULL terminated string) */
    char ecmMfctOUI[64];         				/* The manufacturer Organisationally Unique Identifier (OUI), (as a NULL terminated string) */
    char ecmMfctDate[64];        				/* The manufacture date, (as a NULL terminated string) */
    char ecmSwVersion[64];       				/* The software version (as a NULL terminated string) */
    char ecmHwVersion[64];       				/* The hardware version (as a NULL terminated string)  */
    char ecmSerialNum[64];       				/* The serial number (as a NULL terminated string) */
    BRPC_ECM_Standard ecmStandard;			    /* docsis 1.x/2.0/3.0 */
	uint32_t ecmMacAddressHi;					/* eCM LAN Mac address */
	uint32_t ecmMacAddressLo;					/* eCM LAN mac address */
	uint32_t ecmIpMode;						  	/* eCM IPV6: 0 no IP address yet, 1 IPV4 2 IPV6 */
	uint32_t ecmIpAddress;						/* eCM LAN IPV4 address */
	uint32_t ecmIpv6Address0;					/* eCM LAN IPv6 address */
	uint32_t ecmIpv6Address1;					/* eCM LAN IPv6 address */
	uint32_t ecmIpv6Address2;					/* eCM LAN IPv6 address */
	uint32_t ecmIpv6Address3;					/* eCM LAN IPv6 address */
	BRPC_ECM_WapInterfaceType ecmWapInterface;	/* eCM WAP Interface type, 0, 2.4GHz, 5GHz, or dual band 2.4GHz and 5GHz */
    uint32_t ecmHasMtaInfo;                     /* eCM Multimedia Terminal Adaptor */
    uint32_t ecmHasRouterInfo;                  /* eCM Router */
}BRPC_Param_ECM_GetSystemInfo;

typedef struct BRPC_Param_ECM_WapInterfaceType
{
	BRPC_ECM_WapInterfaceType type;				/* which wap */
} BRPC_Param_ECM_WapInterfaceType;

typedef enum BRPC_ECM_MtaLineStatus
{
    BRPC_ECM_MtaLineStatus_eUnprovisioned,   /*!< Line is not allocated for use              */
    BRPC_ECM_MtaLineStatus_eOnhook,          /*!< Provisioned for use, but not busy          */
    BRPC_ECM_MtaLineStatus_eOffhook,         /*!< Phone is off hook, but no call in progress */
    BRPC_ECM_MtaLineStatus_eRinging,         /*!< On-hook, ringing due to incoming           */
    BRPC_ECM_MtaLineStatus_eActive,          /*!< Call in progress                           */
    BRPC_ECM_MtaLineStatus_eEnddef           /* Enum terminator */
}   BRPC_ECM_MtaLineStatus;


typedef struct BRPC_Param_ECM_MtaLineInfo
{
    BRPC_ECM_MtaLineStatus enStatus;         /*!< Line state                        */
    char                   szNumber[32];     /*!< Phone number of this line         */
    char                   szCallId[64];     /*!< Phone number of caller or callee  */
}   BRPC_Param_ECM_MtaLineInfo;


typedef struct BRPC_Param_ECM_GetMtaStatus
{
    int                        iActive;                   /*!< 0 for off, non-zero for active/on        */
    int                        iNumLines;                 /*!< Number of phones lines                   */
    BRPC_Param_ECM_MtaLineInfo pLines[8];                 /*!< List of per-line information             */
    uint32_t                   mtaExtIPv4Address;         /*!< IPv4 address of the external (WAN) side  */
    uint32_t                   mtaExtIPv6Address0;        /*!< IPv6 address of the external (WAN) side  */
    uint32_t                   mtaExtIPv6Address1;        /*!< IPv6 address of the external (WAN) side  */
    uint32_t                   mtaExtIPv6Address2;        /*!< IPv6 address of the external (WAN) side  */
    uint32_t                   mtaExtIPv6Address3;        /*!< IPv6 address of the external (WAN) side  */
    uint32_t                   mtaSipGatewayIPv4Address;  /*!< IPv4 address of the SIP gateway          */
    uint32_t                   mtaSipGatewayIPv6Address0; /*!< IPv6 address of the SIP gateway          */
    uint32_t                   mtaSipGatewayIPv6Address1; /*!< IPv6 address of the SIP gateway          */
    uint32_t                   mtaSipGatewayIPv6Address2; /*!< IPv6 address of the SIP gateway          */
    uint32_t                   mtaSipGatewayIPv6Address3; /*!< IPv6 address of the SIP gateway          */
}   BRPC_Param_ECM_GetMtaStatus;

typedef struct BRPC_Param_ECM_GetRouterStatus
{
    int                        iActive;                   /*!< 0 for off, non-zero for active           */
    uint32_t                   routerExtIPv4Address;      /*!< IPv4 address of the external (WAN) side  */
    uint32_t                   routerExtIPv6Address0;     /*!< IPv6 address of the external (WAN) side  */
    uint32_t                   routerExtIPv6Address1;     /*!< IPv6 address of the external (WAN) side  */
    uint32_t                   routerExtIPv6Address2;     /*!< IPv6 address of the external (WAN) side  */
    uint32_t                   routerExtIPv6Address3;     /*!< IPv6 address of the external (WAN) side  */
}   BRPC_Param_ECM_GetRouterStatus;

/*
API: BRPC_ProcId_ECM_GetSystemInfo
	-->inparams: NULL
	-->outparams: BRPC_Param_ECM_GetSystemInfo

API: BRPC_ProcId_ECM_GetWapStatus
	-->inparams: BRPC_Param_ECM_WapInterfaceType
	-->outparams: BRPC_Param_ECM_GetWapStatus
*/

/*
Host can use this API to put 3383 into low power mode. Currently supported modes:
BRPC_ECM_PowerMode_On: When 3383 is in low powermode, this will put 3383 back to full ON.

BRPC_ECM_PowerMode_Standby1: This mode will turn off the FullBand Capture(WB) block & transistion 
    to 1x1 narrowband data only mode. This will save approx 4 Watts of power.  
    When 3383 receives this mode from the host, it will send up a CM_STATUS msg to CMTS & expect the CMTS 
    to move this modem to a single channel CMTS.  Once the modem has moved to this new 1x1 CMTS, it will 
    switch from WB to NB & turn off WB. This feature requires a bonding CMTS that supports CM_STATUS 
    sent up by the modem.  This switchover will also stop any active video channels that are using 3383 WB.
    
    For testing purposes & if you do not have a bonding CMTS that supports CM_STATUS, there is a way to 
    test the switchover with the following special configuration to the modem.
    (1) set up non_vol to disable bonding: Use commands "CM/NonVol/CM DOCSIS 3.0 NonVol> enable us_bonding 0" & 
    "CM/NonVol/CM DOCSIS 3.0 NonVol> enable ds_bonding 0" & "CM/NonVol/CM DOCSIS 3.0 NonVol> write" & reboot
    (2) Boot up & range & register with a 1x1 CMTS.
    (3) host sends this Standby1 mode.
    (4) 3383 will switch from WB to NB with no loss of data & save approx 4 Watts.
*/
typedef enum BRPC_ECM_PowerMode{
	/* Most of these modes are not supported, except noted */
	BRPC_ECM_PowerMode_Unknown,
	BRPC_ECM_PowerMode_On,		/* Supported. From Standby1 to full on  */
	BRPC_ECM_PowerMode_Off,
	BRPC_ECM_PowerMode_Standby1,	/* Supported. From full on to Standby1 */
	BRPC_ECM_PowerMode_Standby2,
	BRPC_ECM_PowerMode_Standby3,
	BRPC_ECM_PowerMode_Standby4,
	BRPC_ECM_PowerMode_Last
} BRPC_ECM_PowerMode;

typedef struct BRPC_Param_ECM_PowerSaver
{
	uint32_t devId;				/* Set to BRPC_DevId_3255(0), see BRPC_DevId. Currently assume all external channels + LNA */
	uint32_t mode;				/* eCM power mode: BRPC_ECM_PowerMode  */
} BRPC_Param_ECM_PowerSaver;
typedef struct BRPC_Param_ECM_Transit_Frontend_Control_to_Host
{
	uint32_t devId;						/* Set to BRPC_DevId_3255(0), see BRPC_DevId. Currently assume all external channels + LNA */
} BRPC_Param_ECM_Transit_Frontend_Control_to_Host;
typedef struct BRPC_Param_ECM_Transit_Frontend_Control_to_Bnm
{
	uint32_t devId;						/* Set to BRPC_DevId_3255(0), see BRPC_DevId. Currently assume all external channels + LNA */
} BRPC_Param_ECM_Transit_Frontend_Control_to_Bnm;

typedef struct BRPC_Param_TSMF_Get
{
	uint32_t TsmfNum;
} BRPC_Param_TSMF_Get;

typedef struct BRPC_Param_ECM_TSMF_FldVerifyConfig
{
	uint32_t TsmfNum;

	uint32_t CrcChkDis;
	uint32_t RelTsStatusChkDis;
	uint32_t FrameTypeChkDis;
	uint32_t RelTsModeChkDis;
	uint32_t SyncChkFDis;
	uint32_t CCCkDis;
	uint32_t AdapChkDis;
	uint32_t ScChkDis;
	uint32_t TsPriorChkDis;
	uint32_t PusiChkDis;
	uint32_t TeiChkDis;
	uint32_t VersionChgMode;
} BRPC_Param_ECM_TSMF_FldVerifyConfig;

typedef struct BRPC_Param_ECM_TSMF_EnableAutoMode
{
	uint32_t eInputSel;
	uint32_t TSMFNum;
	uint32_t RelativeTsNo;
} BRPC_Param_ECM_TSMF_EnableAutoMode;

typedef struct BRPC_Param_ECM_TSMF_EnableSemiAutoMode
{
	uint32_t eInputSel;
	uint32_t TSMFNum;
	uint32_t SlotMapLo;
	uint32_t SlotMapHi;
	uint32_t RelativeTsNo;
} BRPC_Param_ECM_TSMF_EnableSemiAutoMode;

typedef struct BRPC_Param_ECM_TSMF_DisableTsmf
{
	uint32_t TSMFNum;
} BRPC_Param_ECM_TSMF_DisableTsmf;

typedef struct BRPC_Param_ECM_TSMF_SetParserConfig
{
	uint32_t uiParserBandNum;
	uint32_t TSMFNum;
	uint32_t TsmftoParserEn;
} BRPC_Param_ECM_TSMF_SetParserConfig;

typedef struct BRPC_Param_ADS_SetParserBandId
{
    uint32_t  devId;            /* device id DS0 to DSx, see BRPC_DevId */
    uint32_t  enable;           /* 0 or 1. Enabled bit for each mini parser */
    uint32_t  parserBand;       /* 4bit value. newly mapped parserband ID for devId */
} BRPC_Param_ADS_SetParserBandId;

typedef struct BRPC_Param_ECM_ReadXptBlock
{
    uint32_t startRegisterAddress; /* Offset from the begining of XPT block, 32bit aligned  */
    uint32_t endRegisterAddress;   /* Offset from the begining of XPT block, 32bit aligned */
    uint32_t returnRegisterValues[16]; /* array of uint32 to store read values. Max 16 32-bit word deep */ 
} BRPC_Param_ECM_ReadXptBlock;

typedef struct BRPC_Param_ECM_WriteXptBlock
{
    uint32_t startRegisterAddress; /* Offset from the begining of XPT block, 32bit aligned  */
    uint32_t endRegisterAddress;   /* Offset from the begining of XPT block, 32bit aligned */
    uint32_t writeRegisterValues[16]; /* array of uint32 to be written. Max 16 32-bit word deep */ 
} BRPC_Param_ECM_WriteXptBlock;

/* Host notfify eCM of Host control video channels lock status  */
typedef struct BRPC_Param_ECM_HostChannelsLockStatus
{
	uint32_t NonCmControlledVideoChLockStatus;  /* b31-b0 indicate host control video channels lock status. */
                						   		/* bit set=lock, clear=unlock. b0=ch0, b1=ch1, etc.... upto 32 channels can be reported*/
} BRPC_Param_ECM_HostChannelsLockStatus;

/* When CM receive this API, it will do LNA_reconfig(Doing CPPM) regardless, & it will glitch all locked channels */
/* LNA ReConfig can takes up to 1.5 seconds, so this is an async call, meaning when this API return, DoLnaReConfig is added to the Q and is not completed */
typedef struct BRPC_Param_ECM_DoLnaReConfig
{
    uint32_t devId;                                         /* Not used, just set to BRPC_DevId_3255(0), see BRPC_DevId. */
} BRPC_Param_ECM_DoLnaReConfig;

/* Read the FE chip temperature */
typedef struct BRPC_Param_ECM_ReadDieTemperature
{
    uint32_t TempInDot00DegC;        /* in unit 1/100 degC . Return chip temperature in C degree */
} BRPC_Param_ECM_ReadDieTemperature;

/* 					Async rMangum API calls:                   */
/*  Due to the nature of these API calls take longer time,     */
/*  the result will return in BRPC notification call below     */
/* 					                                           */
typedef struct BRPC_Param_ADS_GetDsChannelPower  /* OCAP DPM support for video channels */
{
	uint32_t devId;						/* device id, see BRPC_DevId */
} BRPC_Param_ADS_GetDsChannelPower;

/* 					BRPC notification data format:                   */
/*  Event code        total          32 bits                         */
/* |bit31 bit30.................. bit16|bit15 bit14...... bit0|      */
/* |BRPC_ADS_Notification_Event        |Event specific data   |      */
/* |BRPC_AOB_Notification_Event        |Event specific data   |      */
/* |BRPC_AUS_Notification_Event        |Event specific data   |      */
/* |BRPC_DS_Channel_Power_Event        | DPM in 1/10 dBbmV    |      */
typedef enum BRPC_Notification_Event
{
	BRPC_Notification_Event_No = 0,
	BRPC_Notification_Event_LockStatusChanged,
	BRPC_Notification_Event_EcmReset,
	BRPC_Notification_Event_EcmOperational,
	BRPC_Notification_Event_EcmRmagnumReady,	/* onetime notification to host from BNM when rMagnum is up during boot up */
	BRPC_Notification_Event_DsChannelPower,
	BRPC_Notification_Event_Last

} BRPC_Notification_Event;

typedef enum {
	BRPC_Qam_Lock = 0x8000,
    BRPC_Fec_Lock = 0x4000
#if 0
    ,BRPC_Acq_Done = 0x2000,         /* 1=indicate this is a lock_result of a ADS_Acquire. 0=unsolicited lock change notification */
    BRPC_Unused   = 0x1000
#endif
/*  BRPC_Freq_250khz = 0x0xxx   b11-b0=tuning freq in 250khz unit. Max freq=4095*250k=1,023,750000hz */
} BRPC_Param_ADS_LockStatus_Notification;


typedef enum {
	BRPC_OOB_Qam_Lock = 0x8000,
	BRPC_OOB_Fec_Lock = 0x4000
} BRPC_Param_AOB_LockStatus_Notification;

#define BRPC_GET_NOTIFICATION_EVENT(x)	(x>>16)
#define BRPC_GET_ADS_QAM_LOCKSTATUS(x)	(x&BRPC_Qam_Lock)
#define BRPC_GET_ADS_FEC_LOCKSTATUS(x)	(x&BRPC_Fec_Lock)
#define BRPC_GET_AOB_QAM_LOCKSTATUS(x)	(x&BRPC_OOB_Qam_Lock)
#define BRPC_GET_AOB_FEC_LOCKSTATUS(x)	(x&BRPC_OOB_Fec_Lock)
#define BRPC_GET_DS_POWER(x)			((int16_t)(x&0xffff))

#if __cplusplus
}
#endif

#endif
