/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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

#ifndef BTNR_3420_PRIV_H__
#define BTNR_3420_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct BTNR_P_3420_Settings
{
	unsigned long ifFreq;
	unsigned long xtalFreq;
	BREG_I2C_Handle hI2CReg;
	unsigned short i2cAddr;

	int iRevLetter, iRevNumber, iType;		/* Saved chip information */
	int enableAgcGain;
	unsigned long rfFreq;

	
	unsigned short AnnexMode;	/* only used for Docsis tuner */	
	unsigned long DSFreq;		/* only used for Docsis tuner */
	unsigned char TunerType;
	bool PowerTune;				/* only used for Docsis tuner */
	BTNR_TunerMode tunerMode;
	bool isInitialized;
} BTNR_P_3420_Settings;

BERR_Code BTNR_P_3420_tune( BTNR_P_3420_Settings *pTnrImplData, int lFreq );
BERR_Code BTNR_P_3420_initialize( BTNR_P_3420_Settings *pTnrImplData );


#ifdef __cplusplus
}
#endif
 
#endif



