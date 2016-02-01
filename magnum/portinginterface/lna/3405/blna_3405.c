/***************************************************************************
 *     Copyright (c) 2003-2006, Broadcom Corporation
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
 * This module is the driver for BCM3405 LNA.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "blna.h"
#include "blna_priv.h"
#include "blna_3405.h"

BDBG_MODULE(blna_3405);

#define	CHK_RETCODE( rc, func )				\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


#define	DEV_MAGIC_ID			((BERR_LNA_ID<<16) | 0xFACE)

typedef enum Bcm3405Reg
{
	Bcm3405Reg_eAGC 			= 0x00,			/* 0x00 */
	Bcm3405Reg_eRESERVED_00,					/* 0x01 */
	Bcm3405Reg_eRESERVED_01,					/* 0x02 */
	Bcm3405Reg_eRESERVED_02,					/* 0x03 */
	Bcm3405Reg_eRESERVED_03,					/* 0x04 */
	Bcm3405Reg_eRESERVED_04,					/* 0x05 */
	Bcm3405Reg_eRESERVED_05,					/* 0x06 */
	Bcm3405Reg_eRESERVED_06,					/* 0x07 */
	Bcm3405Reg_eManualGain,						/* 0x08 */
	Bcm3405Reg_eRESERVED_07,					/* 0x09 */
	Bcm3405Reg_eAGC1,							/* 0x0A */
	Bcm3405Reg_eTM1,							/* 0x0B */
	Bcm3405Reg_eTM2,							/* 0x0C */
	Bcm3405Reg_eTM3,							/* 0x0D */
	Bcm3405Reg_eRFOUT1,							/* 0x0E */
	Bcm3405Reg_eRFOUT2,							/* 0x0F */
	Bcm3405Reg_eRFOUT3,							/* 0x10 */
	Bcm3405Reg_eOOB_BYPASS,						/* 0x11 */
	Bcm3405Reg_eAGC2,							/* 0x12 */
	Bcm3405Reg_eAGC3,							/* 0x13 */
	Bcm3405Reg_eAGC4,							/* 0x14 */
	Bcm3405Reg_ePowerDetector,					/* 0x15 */
	Bcm3405Reg_eRESERVED_08,					/* 0x16 */
	Bcm3405Reg_eI2C,							/* 0x17 */
	Bcm3405Reg_eExternalDrivers,				/* 0x18 */
	Bcm3405Reg_eSPARE_00,						/* 0x19 */
	Bcm3405Reg_eSPARE_01,						/* 0x1A */
	Bcm3405Reg_eSPARE_02,						/* 0x1B */
	Bcm3405Reg_eSPARE_03,						/* 0x1C */
	Bcm3405Reg_eSPARE_04,						/* 0x1D */
	Bcm3405Reg_eSPARE_05,						/* 0x1E */
	Bcm3405Reg_eSPARE_06,						/* 0x1F */
	Bcm3405Reg_eSPARE_07,						/* 0x20 */
	Bcm3405Reg_eSPARE_08,						/* 0x21 */
	Bcm3405Reg_eRESERVED_09,					/* 0x22 */
	Bcm3405Reg_eRESERVED_10,					/* 0x23 */
	Bcm3405Reg_eSPARE_09						/* 0x24 */
} Bcm3405Reg;

#define	BLNA_MN_AGC_OUTPUT_LEVEL			(0)
#define	BLNA_MX_AGC_OUTPUT_LEVEL			(63)
#define	BLNA_MN_MGC_OUTPUT_LEVEL			(1)
#define	BLNA_MX_MGC_OUTPUT_LEVEL			(70)

/*******************************************************************************
*
*	Design Notes:
*
*		TODO
*
*******************************************************************************/



/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/
typedef enum BLNA_3405_InitMode
{
	BLNA_3405_InitMode_eRaw,
	BLNA_3405_InitMode_eHalfCooked,
	BLNA_3405_InitMode_eCooked
} BLNA_3405_InitMode;
	
typedef struct BLNA_3405_P_Settings
{
	BLNA_3405_InitMode initMode;
	BLNA_GainMode gainMode;				/* See BLNA_GainMode for description */
	BREG_I2C_Handle hI2CReg;
	unsigned short i2cAddr;
	BLNA_InBandMode inBandPorts[BLNA_3405_MX_INBAND_PORTS];		/* InBand port mode */
	BLNA_OutOfBandMode outOfBandPorts[BLNA_3405_MX_OUTOFBAND_PORTS];	/* OutOfBand port mode */
	bool isExternalDriverEnabled[BLNA_3405_MX_EXTDRV]; /* Ext. Driver Enabled=1, otherwise disabled */
	unsigned int agcOutputLevel;		/* AGC Output Level, in 2/10 dB */
	int agcDeltaVal;					/* AGC Delta Value, in 2/10 dB */
	unsigned int manualGainVal;			/* If Manual Gain is enabled, manual gain value, in 2/10 dB */
} BLNA_3405_P_Settings;


typedef struct BLNA_P_3405_Handle		*BLNA_3405_Handle;

typedef struct BLNA_P_3405_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */

	BLNA_3405_P_Settings settings;
} BLNA_P_3405_Handle;

/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BLNA_3405_Settings defDevSettings =
{
	BLNA_3405_SETTINGS_I2CADDR,
	BLNA_3405_SETTINGS_GAINMODE,
	{ BLNA_3405_SETTINGS_INBANDPORTS, BLNA_3405_SETTINGS_INBANDPORTS, BLNA_3405_SETTINGS_INBANDPORTS },
	{ BLNA_3405_SETTINGS_OUTOFBANDPORTS, BLNA_3405_SETTINGS_OUTOFBANDPORTS },
	BLNA_3405_SETTINGS_AGCOUTPUTLEVEL,
	BLNA_3405_SETTINGS_AGCDELTALEVEL,
	BLNA_3405_SETTINGS_MANUALGAINVAL
};


/*******************************************************************************
*
*	Private Module Data
*
*******************************************************************************/


/*******************************************************************************
*
*	Private Module Functions
*
*******************************************************************************/
static BERR_Code BLNA_3405_Init( BLNA_3405_Handle hDev );
static void BLNA_3405_WriteReg(
	BLNA_3405_Handle hDev,
	unsigned bytOffset,
	uint8_t bytData
	)
{
	unsigned short chipAddr;
	unsigned char subAddr;
	unsigned char data;
	BERR_Code retCode;
	BLNA_3405_P_Settings *pLnaImplData;
	pLnaImplData = &hDev->settings;

	chipAddr = pLnaImplData->i2cAddr;		/* Must be 7 bit address */
	subAddr  = (unsigned char) bytOffset;
	data     = (unsigned char) bytData;


	retCode = BREG_I2C_Write( pLnaImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	if( retCode != BERR_SUCCESS )
	{
		/* This is a kludge to get around a problem on some board/Bcm3405 combination.
		   On some boards, the first write to Bcm3405 will fail, but after the first
		   failed write to Bcm3405, other writes to Bcm3405 will work.  On good
		   boards, the first write to Bcm3405 will work.  See PR 9154.
		*/
		retCode = BREG_I2C_Write( pLnaImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	}
	BDBG_ASSERT( retCode == BERR_SUCCESS );
}

static void BLNA_3405_ReadReg(
	BLNA_3405_Handle hDev,
	unsigned bytOffset,
	uint8_t *pbytData
	)
{
	unsigned short chipAddr;
	unsigned char subAddr;
	unsigned char data;
	BERR_Code retCode;

	BLNA_3405_P_Settings *pLnaImplData;
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	if( pbytData == NULL )
	{
		retCode = BERR_INVALID_PARAMETER;
		return ;
	}
	pLnaImplData = &hDev->settings;

	chipAddr = pLnaImplData->i2cAddr;		/* Must be 7 bit address */
	subAddr  = (unsigned char) bytOffset;

 	retCode = BREG_I2C_Read( pLnaImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	BDBG_ASSERT( retCode == BERR_SUCCESS );

	*pbytData = data;
}


static BERR_Code BLNA_3405_Close(
	BLNA_Handle hDev						/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BLNA_3405_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BLNA_3405_Close);
	return( retCode );
}



static BERR_Code BLNA_3405_EnableAutomaticGainControl(
	BLNA_3405_Handle hDev,					/* [in] Device handle */
	unsigned int agcOutputLevel,		/* [in] AGC Output Level, in 2/10 dB */
	int agcDeltaVal						/* [in] Delta value, specifics window, in 2/10 dB */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal, bAgc2, bAgc3, bAgc4;
	BLNA_3405_P_Settings *pLnaImplData;

	BDBG_ENTER(BLNA_3405_EnableAutomaticGainControl);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if ( ((signed int) agcOutputLevel < BLNA_MN_AGC_OUTPUT_LEVEL) || (agcOutputLevel > BLNA_MX_AGC_OUTPUT_LEVEL) || (agcDeltaVal < BLNA_MN_AGC_OUTPUT_LEVEL) ||
   	     ((signed int) (agcOutputLevel - agcDeltaVal) < BLNA_MN_AGC_OUTPUT_LEVEL) || 
       	 ((agcOutputLevel + agcDeltaVal) > BLNA_MX_AGC_OUTPUT_LEVEL) )
	{
		retCode = BLNA_ERR_OUTPUT_LEVEL_OUT_OF_RANGE;
	}

	if( hDev->settings.initMode == BLNA_3405_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3405_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eCooked )
	{	
		/* Write the output level and delta values */
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eAGC2, &bAgc2 );
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eAGC3, &bAgc3 );
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eAGC4, &bAgc4 );
	    bAgc2 &= 0xC0;
	    bAgc3 &= 0xC0;
	    bAgc4 &= 0xC0;
	    bAgc3 |= (uint8_t) agcOutputLevel;
	    bAgc2 |= (uint8_t) (agcOutputLevel + agcDeltaVal);
	    bAgc4 |= (uint8_t) (agcOutputLevel - agcDeltaVal);
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eAGC3, bAgc3 );
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eAGC2, bAgc2 );
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eAGC4, bAgc4 );

		/* Now, switch to AGC mode */
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eAGC1, &bVal );
		bVal &= 0x7F;
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eAGC1, bVal );
	}

	hDev->settings.agcOutputLevel = agcOutputLevel;
	hDev->settings.agcDeltaVal = agcDeltaVal;
	hDev->settings.gainMode = BLNA_GainMode_eAGC;

done:
	BDBG_LEAVE(BLNA_3405_EnableAutomaticGainControl);
	return( retCode );
}

static BERR_Code BLNA_3405_EnableManualGainControl(
	BLNA_3405_Handle hDev,					/* [in] Device handle */
	unsigned int gain					/* [in] Gain, in 2/10 dB */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;

	BDBG_ENTER(BLNA_3405_EnableManualGainControl);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if ( (gain < BLNA_MN_MGC_OUTPUT_LEVEL) || (gain > BLNA_MX_MGC_OUTPUT_LEVEL) )
	{
		retCode = BLNA_ERR_OUTPUT_LEVEL_OUT_OF_RANGE;
	}

	if( hDev->settings.initMode == BLNA_3405_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3405_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eCooked )
	{
		/* Disable AGC mode */
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eAGC1, &bVal );
		bVal |= 0x80;
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eAGC1, bVal );

		/* Write Manual gain value */
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eManualGain, &bVal );
		bVal &= 0x80;
		bVal |= (uint8_t) gain;
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eManualGain, bVal );

		/* Latch in Manual value */
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eAGC1, &bVal );
		bVal &= 0xFE;
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eAGC1, bVal );
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eAGC1, bVal+1 );
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eAGC1, bVal );
	}

	hDev->settings.manualGainVal = gain;
	hDev->settings.gainMode = BLNA_GainMode_eMGC;

done:
	BDBG_LEAVE(BLNA_3405_EnableManualGainControl);
	return( retCode );
}

static BERR_Code BLNA_3405_SetInBandMode(
	BLNA_3405_Handle hDev,					/* [in] Device handle */
	unsigned int portNbr,				/* [in] Port number, 0-2 */
	BLNA_InBandMode mode				/* [in] InBand mode */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;
	Bcm3405Reg regName;

	BDBG_ENTER(BLNA_3405_SetInBandMode);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	switch( mode )
	{
	case BLNA_InBandMode_eOpenCable:
		bVal = 0xFF;
		break;
	case BLNA_InBandMode_eAnalog:
		bVal = 0x7F;
		break;
	case BLNA_InBandMode_eDigital:
		bVal = 0x1F;
		break;
	case BLNA_InBandMode_ePowerDownRf:
		bVal = 0x00;
		break;
	default:
		retCode = BLNA_ERR_INBAND_MODE;
		goto done;
	}
	switch( portNbr )
	{
	case 0:
		regName = Bcm3405Reg_eRFOUT1;
		break;
	case 1:
		regName = Bcm3405Reg_eRFOUT2;
		break;
	case 2:
		regName = Bcm3405Reg_eRFOUT3;
		break;
	default:
		retCode = BLNA_ERR_INBAND_PORTNBR_OUT_OF_RANGE;
		goto done;
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3405_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eCooked )
	{	
		BLNA_3405_WriteReg( hDev, regName, bVal );
	}
	hDev->settings.inBandPorts[portNbr] = mode;

done:
	BDBG_LEAVE(BLNA_3405_SetInBandMode);
	return( retCode );
}

static BERR_Code BLNA_3405_SetOutOfBandMode(
	BLNA_3405_Handle hDev,					/* [in] Device handle */
	unsigned int portNbr,				/* [in] Port number, 0-1 */
	BLNA_OutOfBandMode mode				/* [in] OutOfBand mode */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal, oobVal, mask;

	BDBG_ENTER(BLNA_3405_SetOutOfBandMode);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	switch( mode )
	{
	case BLNA_OutOfBandMode_eMaxGain:
		oobVal = 0x0F;
		break;
	case BLNA_OutOfBandMode_eBypass:
		oobVal = 0x7;
		break;
	case BLNA_OutOfBandMode_eNormal:
		oobVal = 0x03;
		break;
	case BLNA_OutOfBandMode_ePowerDownRf:
		oobVal = 0x00;
		break;
	default:
		retCode = BLNA_ERR_OUTOFBAND_MODE;
		goto done;
	}
	switch( portNbr )
	{
	case 0:
		mask = 0x0F;
		oobVal = oobVal << 4;
		break;
	case 1:
		mask = 0xF0;
		break;
	default:
		retCode = BLNA_ERR_OUTOFBAND_PORTNBR_OUT_OF_RANGE;
		goto done;
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3405_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eCooked )
	{	
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eOOB_BYPASS, &bVal );
		bVal = (bVal & mask) | oobVal;
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eOOB_BYPASS, bVal );
	}
	hDev->settings.outOfBandPorts[portNbr] = mode;

done:
	BDBG_LEAVE(BLNA_3405_SetOutOfBandMode);
	return( retCode );
}

static BERR_Code BLNA_3405_EnableExternalDriver(
	BLNA_3405_Handle hDev,					/* [in] Device handle */
	unsigned int extNbr					/* [in] Extneral driver */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;
	uint8_t mask;

	BDBG_ENTER(BLNA_3405_EnableExternalDriver);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	switch( extNbr )
	{
	case 0:
		mask = 0x40;
		break;
	case 1:
		mask = 0x80;
		break;
	default:
		retCode = BLNA_ERR_EXTNBR_OUT_OF_RANGE;
		goto done;
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3405_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eCooked )
	{	
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eExternalDrivers, &bVal );
		bVal |= mask;
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eExternalDrivers, bVal );
	}
	hDev->settings.isExternalDriverEnabled[extNbr] = true;

done:
	BDBG_LEAVE(BLNA_3405_EnableExternalDriver);
	return( retCode );
}

static BERR_Code BLNA_3405_DisableExternalDriver(
	BLNA_3405_Handle hDev,					/* [in] Device handle */
	unsigned int extNbr					/* [in] Extneral driver */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;
	uint8_t mask;

	BDBG_ENTER(BLNA_3405_DisableExternalDriver);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	switch( extNbr )
	{
	case 0:
		mask = 0x40;
		break;
	case 1:
		mask = 0x80;
		break;
	default:
		retCode = BLNA_ERR_EXTNBR_OUT_OF_RANGE;
		goto done;
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3405_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eCooked )
	{	
		BLNA_3405_ReadReg( hDev, Bcm3405Reg_eExternalDrivers, &bVal );
		bVal &= ~mask;
		BLNA_3405_WriteReg( hDev, Bcm3405Reg_eExternalDrivers, bVal );
	}
	hDev->settings.isExternalDriverEnabled[extNbr] = false;

done:
	BDBG_LEAVE(BLNA_DisableExternalDriver);
	return( retCode );
}

static BERR_Code BLNA_3405_GetStatus(
	BLNA_3405_Handle hDev,					/* [in] Device handle */
	BLNA_Status *pStatus				/* [out] Returns status */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	unsigned int idx;

	BDBG_ENTER(BLNA_3405_GetStatus);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	if( pStatus == NULL)
	{
		retCode = BERR_INVALID_PARAMETER;
		goto done;
	}
	if( hDev->settings.initMode == BLNA_3405_InitMode_eCooked )
	{
		pStatus->gainMode = hDev->settings.gainMode;
	}
	else
	{
		pStatus->gainMode = BLNA_GainMode_eNotInitialized;
	}
	for( idx = 0; idx < BLNA_3405_MX_INBAND_PORTS; idx++ )
	{
		pStatus->inBandPorts[idx] = hDev->settings.inBandPorts[idx];
	}
	for( idx = 0; idx < BLNA_3405_MX_OUTOFBAND_PORTS; idx++ )
	{
		pStatus->outOfBandPorts[idx] = hDev->settings.outOfBandPorts[idx];
	}
	for( idx = 0; idx < BLNA_3405_MX_EXTDRV; idx++ )
	{
		pStatus->isExternalDriverEnabled[idx] = hDev->settings.isExternalDriverEnabled[idx];
	}
	pStatus->agcOutputLevel = hDev->settings.agcOutputLevel;
	pStatus->agcDeltaVal = hDev->settings.agcDeltaVal;
	pStatus->manualGainVal = hDev->settings.manualGainVal;
done:
	BDBG_LEAVE(BLNA_3405_GetStatus);
	return( retCode );
}

static BERR_Code BLNA_3405_GetLnaAgcRegVal(
	BLNA_3405_Handle hDev,					/* [in] Device handle */
	uint32_t regOffset,					/* [in] AGC register offset */
	uint32_t *agcVal					/* [out] output value */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;

	BDBG_ENTER(BLNA_3405_GetLnaAgcRegVal);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	if( agcVal == NULL)
	{
		retCode = BERR_INVALID_PARAMETER;
		goto done;
	}
	BLNA_3405_ReadReg( hDev, regOffset, &bVal );
	*agcVal = bVal;
done:
	BDBG_LEAVE(BLNA_3405_GetLnaAgcRegVal);
	return( retCode );
}

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
BERR_Code BLNA_3405_GetDefaultSettings(
	BLNA_3405_Settings *pDefSettings,			/* [out] Returns default setting */
	BCHP_Handle hChip						/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3405_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BLNA_3405_GetDefaultSettings);
	return( retCode );
}

BERR_Code BLNA_3405_Open(
	BLNA_Handle *phLna,					/* [out] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BLNA_3405_Settings *pDefSettings	/* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BLNA_Handle hDev;
	unsigned int idx;
 	BLNA_3405_Handle h3405Dev;
	BLNA_3405_P_Settings *pLnaImplData;


	BDBG_ENTER(BLNA_3405_Open);
	BDBG_ASSERT( hI2CReg );
	BSTD_UNUSED(hChip);

	hDev = NULL;
	if( pDefSettings == NULL)
	{
		retCode = BERR_INVALID_PARAMETER;
		BDBG_ERR(("BLNA_3431_Open: get a null settings\n"));
		goto done;
	}
	/* Alloc memory from the system heap */
	h3405Dev = (BLNA_3405_Handle) BKNI_Malloc( sizeof( BLNA_P_3405_Handle ) );
	if( h3405Dev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BLNA_3405_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset( h3405Dev, 0x00, sizeof( BLNA_P_3405_Handle ) );

	hDev = (BLNA_Handle) BKNI_Malloc( sizeof( BLNA_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BLNA_3405_Open: BKNI_malloc() failed\n"));
		BKNI_Free( h3405Dev );
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BLNA_P_Handle ) );

	h3405Dev->magicId = DEV_MAGIC_ID;
	pLnaImplData = &h3405Dev->settings;
	pLnaImplData->hI2CReg = hI2CReg;
	pLnaImplData->i2cAddr = pDefSettings->i2cAddr;
	if( (pDefSettings->i2cAddr & 0x80) )
	{
		BDBG_ERR(("I2C Address must be 7 bit format"));
		BDBG_ASSERT( !(pDefSettings->i2cAddr & 0x80) );
	}

	hDev->hDevImpl = (void *) h3405Dev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pEnableAutomaticGainControl = (BLNA_EnableAutomaticGainControlFunc) BLNA_3405_EnableAutomaticGainControl; 		/* ptr to get tuned frequency function */
	hDev->pEnableManualGainControl = (BLNA_EnableManualGainControlFunc) BLNA_3405_EnableManualGainControl;
	hDev->pSetInBandMode = (BLNA_SetInBandModeFunc)BLNA_3405_SetInBandMode;
	hDev->pSetOutOfBandMode = (BLNA_SetOutOfBandModeFunc) BLNA_3405_SetOutOfBandMode;
	hDev->pEnableExternalDriver = (BLNA_EnableExternalDriverFunc) BLNA_3405_EnableExternalDriver;
	hDev->pDisableExternalDriver = (BLNA_DisableExternalDriverFunc) BLNA_3405_DisableExternalDriver;
	hDev->pGetStatus = (BLNA_GetStatusFunc) BLNA_3405_GetStatus;
	hDev->pGetLnaAgcRegVal = (BLNA_GetLnaAgcRegValFunc) BLNA_3405_GetLnaAgcRegVal;
	hDev->pClose = (BLNA_CloseFunc) BLNA_3405_Close;	

	pLnaImplData->initMode = BLNA_3405_InitMode_eRaw;
	for( idx = 0; idx < BLNA_3405_MX_INBAND_PORTS; idx++ )
	{
		CHK_RETCODE( retCode, BLNA_3405_SetInBandMode( h3405Dev, idx, pDefSettings->inBandPorts[idx] ) );
	}
	for( idx = 0; idx < BLNA_3405_MX_OUTOFBAND_PORTS; idx++ )
	{
		CHK_RETCODE( retCode, BLNA_3405_SetOutOfBandMode( h3405Dev, idx, pDefSettings->outOfBandPorts[idx] ) );
	}
	for( idx = 0; idx < BLNA_3405_MX_EXTDRV; idx++ )
	{
		CHK_RETCODE( retCode, BLNA_3405_DisableExternalDriver( h3405Dev, idx ) );
	}
	if( pDefSettings->gainMode == BLNA_GainMode_eAGC )
	{
		CHK_RETCODE( retCode, BLNA_3405_EnableAutomaticGainControl( h3405Dev, pDefSettings->agcOutputLevel, pDefSettings->agcDeltaVal ) );
	}
	else if( pDefSettings->gainMode == BLNA_GainMode_eMGC )
	{
		CHK_RETCODE( retCode, BLNA_3405_EnableManualGainControl( h3405Dev, pDefSettings->manualGainVal ) );
	}
	pLnaImplData->initMode = BLNA_3405_InitMode_eHalfCooked;
done:
	if( retCode != BERR_SUCCESS )
	{
		if( hDev != NULL )
		{
			BKNI_Free( hDev );
			hDev = NULL;
		}
	}
	*phLna = hDev;

	BDBG_LEAVE(BLNA_3405_Open);
	return( retCode );
}

static BERR_Code BLNA_3405_Init(
	BLNA_3405_Handle hDev
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	unsigned int idx;
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	
	/* Read Only registers,
	BLNA_3405_WriteReg( hDev, 0x00, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x01, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x02, 0x00 );
	*/
	BLNA_3405_WriteReg( hDev, 0x03, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x04, 0x1A );
	BLNA_3405_WriteReg( hDev, 0x05, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x06, 0x04 );
	BLNA_3405_WriteReg( hDev, 0x07, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x08, 0x13 );
	BLNA_3405_WriteReg( hDev, 0x09, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x0A, 0x40 );
	BLNA_3405_WriteReg( hDev, 0x0B, 0x40 );
	BLNA_3405_WriteReg( hDev, 0x0C, 0x0D );
	/* Read Only register, BLNA_3405_WriteReg( hDev, 0x0D, 0x00 ); */
	BLNA_3405_WriteReg( hDev, 0x0E, 0xFF );
	BLNA_3405_WriteReg( hDev, 0x0F, 0xFF );
	BLNA_3405_WriteReg( hDev, 0x10, 0xFF );
	BLNA_3405_WriteReg( hDev, 0x11, 0x77 );
	BLNA_3405_WriteReg( hDev, 0x12, 0x66 );
	BLNA_3405_WriteReg( hDev, 0x13, 0xA3 );
	BLNA_3405_WriteReg( hDev, 0x14, 0xA0 );
	BLNA_3405_WriteReg( hDev, 0x15, 0xAA );
	BLNA_3405_WriteReg( hDev, 0x16, 0x45 );
	BLNA_3405_WriteReg( hDev, 0x17, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x18, 0x89 );
	BLNA_3405_WriteReg( hDev, 0x19, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x1A, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x1B, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x1C, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x1D, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x1E, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x1F, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x20, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x21, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x22, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x23, 0x00 );
	BLNA_3405_WriteReg( hDev, 0x24, 0x00 );

	/* Must set the isInitialize flag, before initializing the setting, otherwise
	   it will result in an infinite-recursive loop */
	hDev->settings.initMode = BLNA_3405_InitMode_eCooked;
	for( idx = 0; idx < BLNA_3405_MX_INBAND_PORTS; idx++ )
	{
		CHK_RETCODE( retCode, BLNA_3405_SetInBandMode( hDev, idx, hDev->settings.inBandPorts[idx] ) );
	}
	for( idx = 0; idx < BLNA_3405_MX_OUTOFBAND_PORTS; idx++ )
	{
		CHK_RETCODE( retCode, BLNA_3405_SetOutOfBandMode( hDev, idx, hDev->settings.outOfBandPorts[idx] ) );
	}
	for( idx = 0; idx < BLNA_3405_MX_EXTDRV; idx++ )
	{
		if( hDev->settings.isExternalDriverEnabled[idx] == true )
		{
			CHK_RETCODE( retCode, BLNA_3405_EnableExternalDriver( hDev, idx ) );
		}
		else
		{
			CHK_RETCODE( retCode, BLNA_3405_DisableExternalDriver( hDev, idx ) );
		}
	}
	if( hDev->settings.gainMode == BLNA_GainMode_eAGC )
	{
		CHK_RETCODE( retCode, BLNA_3405_EnableAutomaticGainControl( hDev, hDev->settings.agcOutputLevel, hDev->settings.agcDeltaVal ) );
	}
	else if( hDev->settings.gainMode == BLNA_GainMode_eMGC )
	{
		CHK_RETCODE( retCode, BLNA_3405_EnableManualGainControl( hDev, hDev->settings.manualGainVal ) );
	}

done:
	return( retCode );
}