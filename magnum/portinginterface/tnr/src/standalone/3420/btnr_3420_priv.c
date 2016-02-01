/***************************************************************************
 *	   Copyright (c) 2003-2011, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_3420.h"
#include "btnr_3420_priv.h"


BDBG_MODULE(btnr_3420_priv);

#define BCM3420_HWID	(0x2)
#define ANNEX_A 		(0x00)
#define ANNEX_B 		(0x01)
#define ANNEX_A_IF_FREQ (5000000)
#define ANNEX_B_IF_FREQ (4000000)
#define TUNER_A_I2C_ADDR 0x61

typedef struct _BCM3420_struct
{
	uint32_t nLowFreq;
	uint32_t nHighFreq;
	unsigned short wBits;
} BCM3420_struct;

/* 5/06/05: Updated Optimized3420FreqMap */
#define NUM_3420_ELEMENTS 27

static const BCM3420_struct BCM3420_Array[NUM_3420_ELEMENTS] = {
   {  49000,  50125, 0x0A43},
   {  50125,  58500, 0x0A63},
   {  58500,  69250, 0x0983},
   {  69250,  83625, 0x09A3},
   {  83625,  97500, 0x09C3},
   {  97500, 100250, 0x09C5},
   { 100250, 117000, 0x09E5},
   { 117000, 138500, 0x0905},
   { 138500, 167250, 0x0925},
   { 167250, 187000, 0x0945},
   { 187000, 200500, 0x0949},
   { 200500, 234000, 0x096B},
   { 234000, 277000, 0x088B},
   { 277000, 325000, 0x08A9},
   { 325000, 334500, 0x08AF},
   { 334500, 401000, 0x08CF},
   { 401000, 468000, 0x08ED},
   { 468000, 535000, 0x040D},
   { 535000, 554000, 0x080D},
   { 554000, 638000, 0x0430},
   { 638000, 669000, 0x0830},
   { 669000, 720000, 0x0450},
   { 720000, 802000, 0x0850},
   { 802000, 835000, 0x0470},
   { 835000, 885000, 0x0470},
   { 885000, 911000, 0x0870},
   { 911000, 999000, 0x0C70}
};

BERR_Code BTNR_P_3420_tune( BTNR_P_3420_Settings *pTnrImplData, int lFreq )
{
	BERR_Code rc = BERR_SUCCESS;
    BTNR_P_3420_Settings *p_tunerData = pTnrImplData;
	uint8_t Seq1[7];
    uint8_t Seq2[6];
	uint32_t  nVal, i, nIndex, nFlo, nMult=0;
	unsigned short wBits;

	nFlo = lFreq + p_tunerData->ifFreq;
	nIndex = 0;
	for (i=0; i<NUM_3420_ELEMENTS; i++)
	{
		if ((nFlo > (BCM3420_Array[i].nLowFreq*1000)) && 
			(nFlo <= (BCM3420_Array[i].nHighFreq*1000)))
		{
			nIndex = i;
			break;
		}
	}
	wBits = BCM3420_Array[nIndex].wBits;
	switch( wBits & 0x380 )
	{
		case 0:
			nMult = 1;
			break;
		case 0x80:
			nMult = 2;
			break;
		case 0x0100:
			nMult = 4;
			break;
		case 0x0180:
			nMult = 8;
			break;
		case 0x0200:
			nMult = 16;
			break;
	}

	nVal	  = (nFlo / 125000) * nMult;
	
	Seq1[0] = (nVal & 0x1f80) >> 7;
    Seq1[1] = (nVal & 0x7f) << 1;
    Seq1[2] = 0x00;
    Seq1[3] = 0x16; /* check it bytSeq[5] = 0x06;  turn on 16Mhz output(b4) to drive the other tuner */
    Seq1[4] = (wBits & 0x3fc) >> 2;

	if (p_tunerData->PowerTune)
	{
		Seq1[5] = 0x4b | ((wBits & 0x03) << 4);
		Seq1[6] = 0x0c;
		Seq2[1] = 0x34; /* was 0x24 before */
	}
	else
	{
		Seq1[5] = 0x4f | ((wBits & 0x03) << 4);
		Seq1[6] = 0x3c;
		Seq2[1] = 0x3c;
	}
		
	Seq2[0] = 0x0a;
    Seq2[2] = 0xff;
    Seq2[3] = 0x60;
    Seq2[4] = 0x00;

    if (p_tunerData->ifFreq == ANNEX_B_IF_FREQ)
    	Seq2[5] = 0x3b;
    else
        Seq2[5] = 0x39;
	rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x0, Seq1, sizeof(Seq1));
	if( rc != BERR_SUCCESS)	goto done;
	rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x70, Seq2, sizeof(Seq2));
	if( rc != BERR_SUCCESS)	goto done;
	
	if (p_tunerData->PowerTune)
		Seq1[6] = 0x3c;
	else
		Seq2[3] = 0x40;
		
	
	rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x0, Seq1, sizeof(Seq1));
	if( rc != BERR_SUCCESS)	goto done;
	rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x70, Seq2, sizeof(Seq2));
	if( rc != BERR_SUCCESS)	goto done;
	
	if (!p_tunerData->PowerTune)
		BKNI_Delay(2000);
	

	if (p_tunerData->PowerTune)
    {
    	Seq2[3] = 0x40;
		rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x0, Seq1, sizeof(Seq1));
		if( rc != BERR_SUCCESS) goto done;
		rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x70, Seq2, sizeof(Seq2));
		if( rc != BERR_SUCCESS) goto done;
	    BKNI_Delay(2000);
    }

	
	Seq1[3] = ((wBits & 0xc00) >> 10) | 0x10 ;	/*turn on 16Mhz output(b4) to drive the other tuner*/
	rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x0, Seq1, sizeof(Seq1));
	if( rc != BERR_SUCCESS)	goto done;
	rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x70, Seq2, sizeof(Seq2));
	if( rc != BERR_SUCCESS)	goto done;
	
	
	if (p_tunerData->PowerTune)
            BKNI_Sleep(560);
    else
            BKNI_Sleep(40);

    if (p_tunerData->PowerTune)
            Seq1[5] = 0x4f | ((wBits & 0x03) << 4);
    else
            Seq2[1] = 0x34;

	rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x0, Seq1, sizeof(Seq1));
	if( rc != BERR_SUCCESS)	goto done;
	rc = BREG_I2C_Write(p_tunerData->hI2CReg, p_tunerData->i2cAddr, 0x70, Seq2, sizeof(Seq2));

done:
	return rc;
}

BERR_Code BTNR_P_3420_initialize( BTNR_P_3420_Settings *pTnrImplData )
{
	BERR_Code rc = BERR_SUCCESS;
	BTNR_P_3420_Settings *p_tunerData = pTnrImplData;

	p_tunerData->PowerTune = 1; /* check it */
	
	/* p_tunerData->i2cAddr = TUNER_A_I2C_ADDR; */
	
	/* Powertune Tuner once here */
	p_tunerData->isInitialized = true;

	/* initialize the tuner to 0 freq. */
	rc = BTNR_P_3420_tune( p_tunerData, 0 );

	return rc;
}

