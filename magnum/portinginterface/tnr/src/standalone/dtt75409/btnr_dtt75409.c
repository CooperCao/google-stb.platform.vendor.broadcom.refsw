/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_dtt75409.h"

BDBG_MODULE(btnr_dtt75409);

#define DEV_MAGIC_ID			((BERR_TNR_ID<<16) | 0xFACE)

#define CHK_RETCODE( rc, func ) 		\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/

typedef struct BTNR_P_DTT75409_Settings
{
	unsigned long ifFreq;
	BREG_I2C_Handle hI2CReg;
	unsigned short i2cAddr;

	unsigned long rfFreq;
	BTNR_TunerMode tunerMode;
	bool isInitialized;
} BTNR_P_DTT75409_Settings;

typedef struct BTNR_P_DTT75409_Handle		*BTNR_DTT75409_Handle;

typedef struct BTNR_P_DTT75409_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */

	BTNR_P_DTT75409_Settings settings;
} BTNR_P_DTT75409_Handle;

/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BTNR_DTT75409_Settings defDevSettings =
{
	BTNR_DTT75409_SETTINGS_IFFREQ,
	BTNR_DTT75409_SETTINGS_I2CADDR
};

/*******************************************************************************
*
*	Private Module Data
*
*******************************************************************************/
static BERR_Code BTNR_DTT75409_Close(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BTNR_DTT75409_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BTNR_DTT75409_Close);
	return( retCode );
}

static BERR_Code BTNR_DTT75409_SetRfFreq(
	BTNR_DTT75409_Handle hDev,				/* [in] Device handle */
	uint32_t rfFreq,					/* [in] Requested tuner freq., in Hertz */
	BTNR_TunerMode tunerMode			/* [in] Requested tuner mode */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_DTT75409_Settings *pTnrImplData;
	uint8_t tnr[5];
	unsigned long n_cnt;
	unsigned long fosc;
	
	BDBG_ENTER(BTNR_DTT75409_SetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;
	if( pTnrImplData->isInitialized == false )
	{
		/* If required */
		pTnrImplData->isInitialized = true;
	}

	fosc = (rfFreq + pTnrImplData->ifFreq)/ 1000000;
	n_cnt = (rfFreq + pTnrImplData->ifFreq) /  (1000000 / 16);
	
	tnr[0] = 0x7F & (n_cnt >> 8);
    tnr[1] = 0xFF & n_cnt;
    tnr[2] = 0x8B; /* 0xB8 for RF AGC disabled */

	if ( fosc < 121 )
    {
        tnr[3] = 0x05;
    }
    else if (fosc < 141 )
        tnr[3] = 0x45;
    else if (fosc < 166 )
        tnr[3] = 0x85;
    else if (fosc < 182 )
        tnr[3] = 0xc5;
    else if (fosc < 286 )
        tnr[3] = 0x06;
    else if (fosc < 386 )
        tnr[3] = 0x46;
    else if (fosc < 446 )
        tnr[3] = 0x86;
    else if (fosc < 466 )
        tnr[3] = 0xc6;
    else if (fosc < 506 )
        tnr[3] = 0x0c;
    else if (fosc < 761 )
        tnr[3] = 0x4c;
    else if (fosc < 846 )
        tnr[3] = 0x8c;
    else if (fosc < 905 )
        tnr[3] = 0xcc;

	tnr[4] = 0xE3;
 
    CHK_RETCODE( retCode, BREG_I2C_WriteNoAddr( pTnrImplData->hI2CReg, pTnrImplData->i2cAddr, tnr, sizeof(tnr) ) );

	tnr[4] = 0xC3;

	CHK_RETCODE( retCode, BREG_I2C_WriteNoAddr( pTnrImplData->hI2CReg, pTnrImplData->i2cAddr, tnr, sizeof(tnr) ) );

	pTnrImplData->rfFreq = rfFreq;
	pTnrImplData->tunerMode = tunerMode;

done:
	BDBG_LEAVE(BTNR_DTT75409_SetRfFreq);
	return( retCode );
}

static BERR_Code BTNR_DTT75409_GetRfFreq(
	BTNR_DTT75409_Handle hDev,				/* [in] Device handle */
	uint32_t *rfFreq,					/* [output] Returns tuner freq., in Hertz */
	BTNR_TunerMode *tunerMode			/* [output] Returns tuner mode */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_P_DTT75409_Settings *pTnrImplData;


	BDBG_ENTER(BTNR_DTT75409_GetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	pTnrImplData = &hDev->settings;

	*rfFreq = pTnrImplData->rfFreq;
	*tunerMode = pTnrImplData->tunerMode;

	BDBG_LEAVE(BTNR_DTT75409_GetRfFreq);
	return( retCode );
}

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/

BERR_Code BTNR_DTT75409_Open(
	BTNR_Handle *phDev, 				/* [output] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BREG_I2C_Handle hI2CReg,			/* [in] I2C Register handle */
	const BTNR_DTT75409_Settings *pDefSettings /* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_DTT75409_Handle hDTT75409Dev;
	BTNR_P_DTT75409_Settings *pTnrImplData;
	BTNR_Handle hDev;


	BDBG_ENTER(BTNR_DTT75409_Open);
	BDBG_ASSERT( hI2CReg );
	BSTD_UNUSED(hChip);

	hDev = NULL;
	/* Alloc memory from the system heap */
	hDTT75409Dev = (BTNR_DTT75409_Handle) BKNI_Malloc( sizeof( BTNR_P_DTT75409_Handle ) );
	if( hDTT75409Dev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_DTT75409_Open: BKNI_malloc() failed"));
		goto done;
	}
	BKNI_Memset( hDTT75409Dev, 0x00, sizeof( BTNR_P_DTT75409_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_DTT75409_Open: BKNI_malloc() failed"));
		BKNI_Free( hDTT75409Dev );
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

	hDTT75409Dev->magicId = DEV_MAGIC_ID;
	pTnrImplData = &hDTT75409Dev->settings;
	pTnrImplData->hI2CReg = hI2CReg;
	pTnrImplData->ifFreq = pDefSettings->ifFreq;
	pTnrImplData->i2cAddr = pDefSettings->i2cAddr;
	if( (pDefSettings->i2cAddr & 0x80) )
	{
		BDBG_ERR(("I2C Address must be 7 bit format"));
		BDBG_ASSERT( !(pDefSettings->i2cAddr & 0x80) );
		BKNI_Free( hDTT75409Dev );
		goto done;
	}

	pTnrImplData->rfFreq = 0;
	pTnrImplData->tunerMode = BTNR_TunerMode_eDigital;
	pTnrImplData->isInitialized = false;

	hDev->hDevImpl = (void *) hDTT75409Dev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_DTT75409_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_DTT75409_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) NULL;
	hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) NULL;
	hDev->pClose = (BTNR_CloseFunc) BTNR_DTT75409_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc)NULL;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc)NULL;


done:
	*phDev = hDev;
	BDBG_LEAVE(BTNR_DTT75409_Open);
	return( retCode );
}

BERR_Code BTNR_DTT75409_GetDefaultSettings(
	BTNR_DTT75409_Settings *pDefSettings,	/* [output] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	
	BDBG_ENTER(BTNR_DTT75409_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BTNR_DTT75409_GetDefaultSettings);
	return( retCode );
}

