/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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

#ifndef BTNR_3418_SMARTTUNE_H__
#define BTNR_3418_SMARTTUNE_H__


#ifdef __cplusplus
extern "C" {
#endif

/* LO1 Mod 48, LO1 Mod 24, 2LO1-3LO2, LO1+RF-2LO2.
   Always keep NUMOFSPURTYPES as the last element of the enumeration. */

typedef enum eDAC_Const {eDAC_MIN=0, eDAC_MAX=0x3F, eDAC_SEL_REG=0x1C, eDAC_PWR_DET=6,
						 eDAC_CTRL_REG=0x1D, eDAC_FORCE_VGA=3} eDAC_Const;
typedef enum eAGC_Const {eAGC_MAX=0x1F} eAGC_Const;
typedef enum eVGA_Const {eVGA_MAX=0x3F} eVGA_Const;
typedef enum eDAC_Device {eDAC_CAL, eDAC_CTAT, eDAC_PTAT,
					  eDAC_PL, eDAC_PH, eDAC_NMOS, eDAC_PMOS} eDAC_Device;


typedef struct CPLL1
{
	int 			m_iVCO; 		/* Last Selected VCO. */
	int 			m_iN;			/* Integer portion of feedback divider. */
	int 			m_iFrac;		/* Fractional part of feedback divider. */
	int 			m_iOffset;		/* Offset control bits. */
	unsigned long	m_nFreq;		/* Frequency for LO1.	*/ 
	unsigned long	m_nVCO12XOvr;	/* Crossover frequency from VCO1 to VCO2 (MHz) */
	unsigned long	m_nVCO23XOvr;	/* Crossover frequency from VCO2 to VCO3 (MHz). */

} CPLL1;


typedef struct CPLL2
{
	int  m_iN;
	int  m_iM;
	long m_nFreq; /* shouldn't it be unsigned?? */

} CPLL2;


typedef struct CDAC
{
	bool m_bForceVga;	/* true if forcing the VGA */
	int  m_iForceVal;	/* Force value for VGA. */
	int  m_iAGCVal; 	/* AGC value. */
} CDAC;


#define BCM3418_INPUTSIGNALTYPE_DIGITAL  1
#define BCM3418_INPUTSIGNALTYPE_ANALOG	 2

typedef struct BTNR_P_3418_Settings
{
	CPLL1			m_cPLL1;
	CPLL2			m_cPLL2;
	CDAC			m_cDAC;
		
	bool			m_bInitTuner;
	int 			m_iAgcSettleTime, m_iForceSettleTime;
	int 			iRevLetter;
	int 			iRevNumber, iType;		/* Saved chip information */
	
	unsigned long	ifFreq; /* IF2 - Second IF: 43750000 (US), 36125000 (EURO).*/
	unsigned long	xtalFreq; /* Crystal frequency in MHz. The default is 24000000. */
	BREG_I2C_Handle hI2CReg;
	unsigned short	i2cAddr;

	
	long			m_nGainControlType; 	/* Gain Control Type. TGC_AGC_CONTINUOUS(1),TGC_AGC_HOLD_WHEN_TUNE(2),TGC_MANUAL_GAIN(4) */
	bool			m_bFreezedAGC, m_bPowerUp;
	long			m_nInitForceVGA;

	/*Norminal SAW center frequency and bandwidth in Hz.
		For US (IF2 = 43750000, 44000000, & 47250000 Hz), SAW CF = 1216000000, and SAW BW = 17000000
		For EU (IF2 = 36125000 Hz), SAW CF = 1220000000, and SAW BW = 27000000 */
	long			m_nSAWBW;
	long			m_nSAWFX;
	int 			m_iAGCCap;

	int 			m_nInputSignalType;

	int 			enableAgcGain;
	unsigned long	rfFreq;
	BTNR_TunerMode	tunerMode;
	bool			isInitialized;
	bool			alwaysUse3418A2Init;
	unsigned char * register_shadow;
} BTNR_P_3418_Settings;



void BTNR_P_3418_initialize( BTNR_P_3418_Settings *pTnrImplData );
void BTNR_P_3418_initDigital( BTNR_P_3418_Settings *pTnrImplData );
void BTNR_P_3418_initAnalog( BTNR_P_3418_Settings *pTnrImplData );
void BTNR_P_3418_tune( BTNR_P_3418_Settings *pTnrImplData, long nRF );
/* Not Used */void BTNR_P_3418_GetTemperature( BTNR_P_3418_Settings *pTnrImplData, long *pnTemp);
void BTNR_P_3418_PowerDown( BTNR_P_3418_Settings *pTnrImplData );
void BTNR_P_3418_Freeze( BTNR_P_3418_Settings *pTnrImplData );
void BTNR_P_3418_UnFreeze( BTNR_P_3418_Settings *pTnrImplData );

#ifdef __cplusplus
}
#endif
 
#endif

