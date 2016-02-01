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
 * This module is the driver for BCM3431 LNA.
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "blna.h"
#include "blna_priv.h"
#include "blna_3431.h"

BDBG_MODULE(blna_3431);

#define	CHK_RETCODE( rc, func ) do { if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) goto done; } while(0)

#define	DEV_MAGIC_ID			((BERR_LNA_ID<<16) | 0xFACE)

typedef enum Bcm3431Reg
{
	Bcm3431Reg_eAGC 			= 0x00,			/* 0x00 */		
	Bcm3431Reg_eRESERVED_00,					/* 0x01 */
	Bcm3431Reg_eRESERVED_01,					/* 0x02 */
	Bcm3431Reg_eRESERVED_02,					/* 0x03 */
	Bcm3431Reg_eRESERVED_03,					/* 0x04 */
	Bcm3431Reg_eRESERVED_04,					/* 0x05 */
	Bcm3431Reg_eRESERVED_05,					/* 0x06 */
	Bcm3431Reg_eRESERVED_06,					/* 0x07 */
	Bcm3431Reg_eWaitAgcFastUp_00,				/* 0x08 */
	Bcm3431Reg_eWaitAgcFastUp_01,				/* 0x09 */
	Bcm3431Reg_eWaitAgcSlowUp_00,				/* 0x0A */
	Bcm3431Reg_eWaitAgcSlowUp_01,				/* 0x0B */
	Bcm3431Reg_eWaitAgcSlowDown_00,				/* 0x0C */
	Bcm3431Reg_eWaitAgcSlowDown_01,				/* 0x0D */
	Bcm3431Reg_eWaitAgcFastDown_00,				/* 0x0E */
	Bcm3431Reg_eWaitAgcFastDown_01,				/* 0x0F */
	Bcm3431Reg_eSPARE_00,						/* 0x10 */
	Bcm3431Reg_eSPARE_01,						/* 0x11 */
	Bcm3431Reg_eWaitSample_00,					/* 0x12 */
	Bcm3431Reg_eWaitSample_01,					/* 0x13 */
	Bcm3431Reg_eWaitSample_02,					/* 0x14 */
	Bcm3431Reg_eStg1Gain,						/* 0x15 */
	Bcm3431Reg_eStg2Gain,						/* 0x16 */
	Bcm3431Reg_eStg1Ctrl,						/* 0x17 */
	Bcm3431Reg_eStg2Ctrl,						/* 0x18 */
	Bcm3431Reg_eCtrl_00,						/* 0x19 */
	Bcm3431Reg_eCtrl_01,						/* 0x1A */
	Bcm3431Reg_eCtrl_02,						/* 0x1B */
	Bcm3431Reg_eCtrl_03,						/* 0x1C */
	Bcm3431Reg_eCtrl_04,						/* 0x1D */
	Bcm3431Reg_eCtrl_05,						/* 0x1E */
	Bcm3431Reg_eCtrl_06,						/* 0x1F */
	Bcm3431Reg_eCtrl_07,						/* 0x20 */
	Bcm3431Reg_eCtrl_08,						/* 0x21 */
	Bcm3431Reg_eCtrl_09,						/* 0x22 */
	Bcm3431Reg_eCtrl_0a,						/* 0x23 */
	Bcm3431Reg_eCtrl_0b,						/* 0x24 */
	Bcm3431Reg_eCtrl_0c,						/* 0x25 */
	Bcm3431Reg_eExternalDrivers,				/* 0x26 */
	Bcm3431Reg_eMaxGain2,						/* 0x27 */
	Bcm3431Reg_eOutputCtrl,						/* 0x28 */
	Bcm3431Reg_eLna0Ctrl,						/* 0x29 */
	Bcm3431Reg_eLna1Ctrl,						/* 0x2A */
	Bcm3431Reg_eMaxGain1,						/* 0x2B */
	Bcm3431Reg_eMinGain1,						/* 0x2C */
	Bcm3431Reg_eCb_00,							/* 0x2D */
	Bcm3431Reg_eCb_01,							/* 0x2E */
	Bcm3431Reg_eCb_02,							/* 0x2F */
	Bcm3431Reg_eFastUpRampRate,					/* 0x30 */
	Bcm3431Reg_eSlowUpRampRate,					/* 0x31 */
	Bcm3431Reg_eSlowDownRampRate,				/* 0x32 */
	Bcm3431Reg_eFastDownRampRate,				/* 0x33 */
	Bcm3431Reg_eRampRateI2c,					/* 0x34 */
	Bcm3431Reg_eFbTp,							/* 0x35 */
	Bcm3431Reg_eRampDac1,						/* 0x36 */
	Bcm3431Reg_eRampDac2,						/* 0x37 */
	Bcm3431Reg_eRampDacInc,						/* 0x38 */
	Bcm3431Reg_eRampDac1I2c,					/* 0x39 */
	Bcm3431Reg_eRampDac2I2c,					/* 0x3A */
	Bcm3431Reg_eSPARE_02,						/* 0x3B */
	Bcm3431Reg_Revision_00,						/* 0x3C */
	Bcm3431Reg_Revision_01,						/* 0x3D */
	Bcm3431Reg_eSPARE_03						/* 0x3E */	
} Bcm3431Reg;


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
typedef enum BLNA_3431_InitMode
{
	BLNA_3431_InitMode_eRaw,
	BLNA_3431_InitMode_eHalfCooked,
	BLNA_3431_InitMode_eCooked
} BLNA_3431_InitMode;

typedef struct BLNA_3431_P_Settings
{
	BLNA_3431_InitMode initMode;
	BLNA_InBandMode inBandPorts[BLNA_MX_INBAND_PORTS];		/* InBand port mode */
	bool isExternalDriverEnabled[BLNA_MX_EXTDRV]; /* Ext. Driver Enabled=1, otherwise disabled */
	BREG_I2C_Handle hI2CReg;
	unsigned short i2cAddr;
} BLNA_3431_P_Settings;


typedef struct BLNA_P_3431_Handle		*BLNA_3431_Handle;

typedef struct BLNA_P_3431_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */
	
	BLNA_3431_P_Settings settings;
} BLNA_P_3431_Handle;

/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BLNA_3431_Settings defDevSettings =
{
	BLNA_3431_SETTINGS_I2CADDR,
	{ BLNA_3431_SETTINGS_INBANDPORTS, BLNA_3431_SETTINGS_INBANDPORTS, BLNA_3431_SETTINGS_INBANDPORTS }
	
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
static BERR_Code BLNA_3431_Init( BLNA_3431_Handle hDev );

static BERR_Code BLNA_3431_WriteReg(
							   BLNA_3431_Handle hDev,
							   unsigned bytOffset,
							   uint8_t bytData
							   )
{
	unsigned short chipAddr;
	unsigned char subAddr;
	unsigned char data;
	BERR_Code retCode;
	BLNA_3431_P_Settings *pLnaImplData;
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	
	pLnaImplData = &hDev->settings;
	
	chipAddr = pLnaImplData->i2cAddr;		/* Must be 7 bit address */
	subAddr  = (unsigned char) bytOffset;
	data     = (unsigned char) bytData;
	
	
	retCode = BREG_I2C_Write( pLnaImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	if( retCode != BERR_SUCCESS )
	{
	/* This is a kludge to get around a problem on some board/Bcm3431 combination.
	On some boards, the first write to Bcm3431 will fail, but after the first
	failed write to Bcm3431, other writes to Bcm3431 will work.  On good
	boards, the first write to Bcm3431 will work.  See PR 9154.
		*/
		retCode = BREG_I2C_Write( pLnaImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	}
	/*BDBG_ASSERT( retCode == BERR_SUCCESS );*/

    return retCode;
}

static BERR_Code BLNA_3431_ReadReg(
							  BLNA_3431_Handle hDev,
							  unsigned bytOffset,
							  uint8_t *pbytData
							  )
{
	unsigned short chipAddr;
	unsigned char subAddr;
	unsigned char data;
	BERR_Code retCode;	
	BLNA_3431_P_Settings *pLnaImplData;
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	if( pbytData == NULL )
	{
		retCode = BERR_INVALID_PARAMETER;
		return retCode;
	}

	pLnaImplData = &hDev->settings;
	
	chipAddr = pLnaImplData->i2cAddr;		/* Must be 7 bit address */
	subAddr  = (unsigned char) bytOffset;
	
	retCode = BREG_I2C_Read( pLnaImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	if( retCode != BERR_SUCCESS )
	{
	/* This is a kludge to get around a problem on some board/Bcm3431 combination.
	On some boards, the first write to Bcm3431 will fail, but after the first
	failed write to Bcm3431, other writes to Bcm3431 will work.  On good
	boards, the first write to Bcm3431 will work.  See PR 9154.
		*/
		retCode = BREG_I2C_Read( pLnaImplData->hI2CReg, chipAddr, subAddr, &data, 1 );
	}
	
	if( retCode == BERR_SUCCESS )
        *pbytData = data;

    return retCode;
}

static BERR_Code BLNA_3431_Close(
								 BLNA_Handle hDev						/* [in] Device handle */
								 )
{
	BERR_Code retCode = BERR_SUCCESS;
	
	
	BDBG_ENTER(BLNA_3431_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	
	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );
	
	BDBG_LEAVE(BLNA_3431_Close);
	return( retCode );
}

static BERR_Code BLNA_3431_SetInBandMode(
										 BLNA_3431_Handle hDev,					/* [in] Device handle */
										 unsigned int portNbr,				/* [in] Port number, 0-2 */
										 BLNA_InBandMode mode				/* [in] InBand mode */
										 )
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;
	uint8_t mask;
	
	BDBG_ENTER(BLNA_3431_SetInBandMode);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	switch( portNbr )
	{
	case 0:
		mask = 0x20;
		break;
	case 1:
		mask = 0x40;
		break;
	case 2:
		mask = 0x80;
		break;
	default:
		retCode = BLNA_ERR_EXTNBR_OUT_OF_RANGE;
		goto done;
	}
	
	switch( mode )
	{
	case BLNA_InBandMode_eOpenCable:
	case BLNA_InBandMode_eAnalog:
	case BLNA_InBandMode_eDigital:
	case BLNA_InBandMode_ePowerDownRf:
		break;
	default:
		retCode = BLNA_ERR_INBAND_MODE;
		goto done;
	}
	if( hDev->settings.initMode == BLNA_3431_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3431_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3431_InitMode_eCooked )
	{	
		BLNA_3431_ReadReg( hDev, Bcm3431Reg_eOutputCtrl, &bVal );
		if (mode == BLNA_InBandMode_ePowerDownRf)
		{
			bVal &= ~mask;
		}
		else
		{
			bVal |= mask;
		}
		BLNA_3431_WriteReg( hDev, Bcm3431Reg_eOutputCtrl, bVal );
	}
	hDev->settings.inBandPorts[portNbr] = mode;
done:
	BDBG_LEAVE(BLNA_3431_SetInBandMode);
	return( retCode );
	
}

static BERR_Code BLNA_3431_EnableExternalDriver(
												BLNA_3431_Handle hDev,					/* [in] Device handle */
												unsigned int extNbr					/* [in] Extneral driver */
												)
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;
	uint8_t mask;
	
	BDBG_ENTER(BLNA_3431_EnableExternalDriver);
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
	if( hDev->settings.initMode == BLNA_3431_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3431_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3431_InitMode_eCooked )
	{	
		BLNA_3431_ReadReg( hDev, Bcm3431Reg_eExternalDrivers, &bVal );
		bVal |= mask;
		BLNA_3431_WriteReg( hDev, Bcm3431Reg_eExternalDrivers, bVal );
	}
	hDev->settings.isExternalDriverEnabled[extNbr] = true;
	
done:
	BDBG_LEAVE(BLNA_3431_EnableExternalDriver);
	return( retCode );
}

static BERR_Code BLNA_3431_DisableExternalDriver(
												 BLNA_3431_Handle hDev,					/* [in] Device handle */
												 unsigned int extNbr					/* [in] Extneral driver */
												 )
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;
	uint8_t mask;
	
	BDBG_ENTER(BLNA_3431_DisableExternalDriver);
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
	if( hDev->settings.initMode == BLNA_3431_InitMode_eHalfCooked )
	{
		CHK_RETCODE( retCode, BLNA_3431_Init( hDev ) );
	}
	if( hDev->settings.initMode == BLNA_3431_InitMode_eCooked )
	{	
		BLNA_3431_ReadReg( hDev, Bcm3431Reg_eExternalDrivers, &bVal );
		bVal &= ~mask;
		BLNA_3431_WriteReg( hDev, Bcm3431Reg_eExternalDrivers, bVal );
	}
	hDev->settings.isExternalDriverEnabled[extNbr] = false;
	
done:
	BDBG_LEAVE(BLNA_DisableExternalDriver);
	return( retCode );
}

static BERR_Code BLNA_3431_GetStatus(
									 BLNA_3431_Handle hDev,					/* [in] Device handle */
									 BLNA_Status *pStatus				/* [out] Returns status */
									 )
{
	BERR_Code retCode = BERR_SUCCESS;
	unsigned int idx;
	
	BDBG_ENTER(BLNA_3431_GetStatus);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	if( pStatus == NULL)
	{
		retCode = BERR_INVALID_PARAMETER;
		goto done;
	}
	
	for( idx = 0; idx < BLNA_3431_MX_INBAND_PORTS; idx++ )
	{
		pStatus->inBandPorts[idx] = hDev->settings.inBandPorts[idx];
	}
	
	for( idx = 0; idx < BLNA_3431_MX_EXTDRV; idx++ )
	{
		pStatus->isExternalDriverEnabled[idx] = hDev->settings.isExternalDriverEnabled[idx];
	}
done:	
	BDBG_LEAVE(BLNA_3431_GetStatus);
	return( retCode );
}

static BERR_Code BLNA_3431_GetLnaAgcRegVal(
										   BLNA_3431_Handle hDev,					/* [in] Device handle */
										   uint32_t regOffset,					/* [in] AGC register offset */
										   uint32_t *agcVal					/* [out] output value */
										   )
{
	BERR_Code retCode = BERR_SUCCESS;
	uint8_t bVal;
	
	BDBG_ENTER(BLNA_3431_GetLnaAgcRegVal);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	if( agcVal == NULL)
	{
		retCode = BERR_INVALID_PARAMETER;
		goto done;
	}
	
	BLNA_3431_ReadReg( hDev, regOffset, &bVal );
	*agcVal = bVal;
done:	
	BDBG_LEAVE(BLNA_3431_GetLnaAgcRegVal);
	return( retCode );
}

BERR_Code BLNA_3431_GetDefaultSettings(
									   BLNA_3431_Settings *pDefSettings,			/* [out] Returns default setting */
									   BCHP_Handle hChip						/* [in] Chip handle */
									   )
{
	BERR_Code retCode = BERR_SUCCESS;	
	BDBG_ENTER(BLNA_3431_GetDefaultSettings);
	BSTD_UNUSED(hChip);
	
	*pDefSettings = defDevSettings;
	
	BDBG_LEAVE(BLNA_3431_GetDefaultSettings);
	return( retCode );
}

BERR_Code BLNA_3431_Open(
						 BLNA_Handle *phLna,					/* [out] Returns handle */
						 BCHP_Handle hChip,					/* [in] Chip handle */
						 BREG_I2C_Handle hI2cReg,			/* [in] I2C Register handle */
						 const BLNA_3431_Settings *pDefSettings	/* [in] Default settings */
						 )
{
	BERR_Code retCode = BERR_SUCCESS;
	BLNA_Handle hDev;
	unsigned int idx;
	BLNA_3431_Handle h3431Dev;
	BLNA_3431_P_Settings *pLnaImplData;
	
	
	BDBG_ENTER(BLNA_3431_Open);
	BDBG_ASSERT( hI2cReg );
	BSTD_UNUSED(hChip);
	
	hDev = NULL;
	if( pDefSettings == NULL)
	{
		retCode = BERR_INVALID_PARAMETER;
		BDBG_ERR(("BLNA_3431_Open: get a null settings\n"));
		goto done;
	}

	/* Alloc memory from the system heap */
	h3431Dev = (BLNA_3431_Handle) BKNI_Malloc( sizeof( BLNA_P_3431_Handle ) );
	if( h3431Dev == NULL || pDefSettings == NULL)
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BLNA_3431_Open: BKNI_malloc() failed\n"));
		goto done;
	}
	BKNI_Memset( h3431Dev, 0x00, sizeof( BLNA_P_3431_Handle ) );
	
	hDev = (BLNA_Handle) BKNI_Malloc( sizeof( BLNA_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BLNA_3431_Open: BKNI_malloc() failed\n"));
		BKNI_Free( h3431Dev );
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BLNA_P_Handle ) );
	
	h3431Dev->magicId = DEV_MAGIC_ID;
	pLnaImplData = &h3431Dev->settings;
	pLnaImplData->hI2CReg = hI2cReg;
	pLnaImplData->i2cAddr = pDefSettings->i2cAddr;
	if( (pDefSettings->i2cAddr & 0x80) )
	{
		BDBG_ERR(("I2C Address must be 7 bit format"));
		BDBG_ASSERT( !(pDefSettings->i2cAddr & 0x80) );
	}
	hDev->hDevImpl = (void *) h3431Dev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pEnableAutomaticGainControl = (BLNA_EnableAutomaticGainControlFunc) NULL; 		/* ptr to get tuned frequency function */
	hDev->pEnableManualGainControl = (BLNA_EnableManualGainControlFunc) NULL;
	hDev->pSetInBandMode = (BLNA_SetInBandModeFunc)BLNA_3431_SetInBandMode;
	hDev->pSetOutOfBandMode = (BLNA_SetOutOfBandModeFunc) NULL;
	hDev->pEnableExternalDriver = (BLNA_EnableExternalDriverFunc) BLNA_3431_EnableExternalDriver;
	hDev->pDisableExternalDriver = (BLNA_DisableExternalDriverFunc) BLNA_3431_DisableExternalDriver;
	hDev->pGetStatus = (BLNA_GetStatusFunc) BLNA_3431_GetStatus;
	hDev->pGetLnaAgcRegVal = (BLNA_GetLnaAgcRegValFunc) BLNA_3431_GetLnaAgcRegVal;
	hDev->pClose = (BLNA_CloseFunc) BLNA_3431_Close;
	for( idx = 0; idx < BLNA_3431_MX_INBAND_PORTS; idx++ )
	{
		CHK_RETCODE( retCode, BLNA_3431_SetInBandMode( h3431Dev, idx, pDefSettings->inBandPorts[idx] ) );
	}
	for( idx = 0; idx < BLNA_3431_MX_EXTDRV; idx++ )
	{
		CHK_RETCODE( retCode, BLNA_3431_DisableExternalDriver( h3431Dev, idx ) );
	}
	
	pLnaImplData->initMode = BLNA_3431_InitMode_eHalfCooked;
	
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
	BDBG_LEAVE(BLNA_3431_Open);
	return( retCode );
}

static BERR_Code BLNA_3431_Init(
								BLNA_3431_Handle hDev
								)
{
	BERR_Code retCode = BERR_SUCCESS;
#if 0	
	unsigned int idx;
#endif	
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x08, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x09, 0x01) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x0A, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x0B, 0x01) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x0C, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x0D, 0x01) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x0E, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x0F, 0x01) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x10, 0xF0) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x11, 0xF0) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x12, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x13, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x14, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x15, 0x27) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x16, 0x14) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x17, 0x59) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x18, 0x59) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x19, 0x50) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x1A, 0x01) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x1B, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x1C, 0x00) );
    /*	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x1D, 0x00) );*/
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x1E, 0xB8) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x1F, 0xF3) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x20, 0xF0) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x21, 0x6C) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x22, 0xA7) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x23, 0x4A) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x24, 0x48) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x25, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x26, 0x83) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x27, 0x14) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x28, 0x03) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x29, 0x88) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x2A, 0x88) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x2B, 0x27) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x2C, 0x01) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x2D, 0x49) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x2E, 0x11) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x2F, 0x11) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x30, 0x10) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x31, 0x04) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x32, 0x10) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x33, 0x40) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x34, 0x1F) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x35, 0x23) );
    /*	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x36, 0x00) );*/
    /*	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x37, 0x00) );*/
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x38, 0x01) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x39, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x3A, 0x00) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x3B, 0xF0) );
    /*	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x3C, 0x00) );*/
    /*	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x3D, 0x00) );*/
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x3E, 0xF0) );
	
	
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x19, 0x50) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x19, 0xB0) );
	CHK_RETCODE( retCode, BLNA_3431_WriteReg( hDev, 0x19, 0x50) );


	/* GAURAV : Commented out the below portion of code as it 
	   resulted in incorrect initialisation on 3520_3431_3418.*/	
#if 0
	/* Must set the isInitialize flag, before initializing the setting, otherwise
	   it will result in an infinite-recursive loop */
	hDev->settings.initMode = BLNA_3431_InitMode_eCooked;
	for( idx = 0; idx < BLNA_3431_MX_INBAND_PORTS; idx++ )
	{
		CHK_RETCODE( retCode, BLNA_3431_SetInBandMode( hDev, idx, hDev->settings.inBandPorts[idx] ) );
	}

	for( idx = 0; idx < BLNA_3431_MX_EXTDRV; idx++ )
	{
		if( hDev->settings.isExternalDriverEnabled[idx] == true )
		{
			CHK_RETCODE( retCode, BLNA_3431_EnableExternalDriver( hDev, idx ) );
		}
		else
		{
			CHK_RETCODE( retCode, BLNA_3431_DisableExternalDriver( hDev, idx ) );
		}
	}	
#endif	

done:
	return( retCode );
}
