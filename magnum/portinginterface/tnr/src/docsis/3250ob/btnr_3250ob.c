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
#include "btnr_3250ob.h"

BDBG_MODULE(btnr_3250ob);

#define	DEV_MAGIC_ID			((BERR_TNR_ID<<16) | 0xFACE)


#define	CHK_RETCODE( rc, func )			\
do {										\
	if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
	{										\
		goto done;							\
	}										\
} while(0)


#define	HERTZ_TO_MHZ(x)			((float) ((float) x / 1000000.0))
#define	MHZ_TO_HERTZ(x)			(x * 1000000)





/*******************************************************************************
*
*	Private Module Handles
*
*******************************************************************************/
typedef struct BTNR_P_3250Ob_Handle		*BTNR_3250Ob_Handle;

typedef struct BTNR_P_3250Ob_Settings
{
	BQOB_Handle hQobDev;					/* Required to set LO Freq */
	unsigned long ifFreq;
	unsigned long rfFreq;
} BTNR_P_3250Ob_Settings;

typedef struct BTNR_P_3250Ob_Handle
{
	uint32_t magicId;					/* Used to check if structure is corrupt */

	BTNR_P_3250Ob_Settings settings;
} BTNR_P_3250Ob_Handle;



/*******************************************************************************
*
*	Default Module Settings
*
*******************************************************************************/
static const BTNR_3250Ob_Settings defDevSettings =
{
	BTNR_3250OB_SETTINGS_IFFREQ
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
static BERR_Code BTNR_3250Ob_Close(
	BTNR_Handle hDev					/* [in] Device handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BTNR_3250Ob_Close);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	hDev->magicId = 0x00;		/* clear it to catch inproper use */
	BKNI_Free( (void *) hDev->hDevImpl );
	BKNI_Free( (void *) hDev );

	BDBG_LEAVE(BTNR_3250Ob_Close);
	return( retCode );
}

static BERR_Code BTNR_3250Ob_SetRfFreq(
	BTNR_3250Ob_Handle hDev,			/* [in] Device handle */
	uint32_t rfFreq,					/* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode			/* [in] Requested tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	unsigned long loFreq;


	BDBG_ENTER(BTNR_3250Ob_SetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	BSTD_UNUSED(tunerMode);

	loFreq = hDev->settings.ifFreq + rfFreq;
	/* Now serialize this call to BQOB code, just in case others may be
	   trying to access BQOB  */
	BKNI_EnterCriticalSection();
	CHK_RETCODE( retCode, BQOB_SetLoFreq( hDev->settings.hQobDev, loFreq ) );
	BKNI_LeaveCriticalSection();
	hDev->settings.rfFreq = rfFreq;

done:
	BDBG_LEAVE(BTNR_3250Ob_SetRfFreq);
	return( retCode );
}

static BERR_Code BTNR_3250Ob_GetRfFreq(
	BTNR_3250Ob_Handle hDev,			/* [in] Device handle */
	uint32_t *rfFreq,					/* [output] Returns tuner freq., in Hertz */
	BTNR_TunerMode *tunerMode			/* [output] Returns tuner mode */
    )
{
	BERR_Code retCode = BERR_SUCCESS;


	BDBG_ENTER(BTNR_3250Ob_GetRfFreq);
	BDBG_ASSERT( hDev );
	BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

	*rfFreq = hDev->settings.rfFreq;
	*tunerMode = BTNR_TunerMode_eDigital;

	BDBG_LEAVE(BTNR_3250Ob_GetRfFreq);
	return( retCode );
}




/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/
BERR_Code BTNR_3250Ob_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BCHP_Handle hChip,					/* [in] Chip handle */
	BQOB_Handle hQob,					/* [in] QOB handle for Bcm3250 */
	const BTNR_3250Ob_Settings *pDefSettings /* [in] Default settings */
	)
{
	BERR_Code retCode = BERR_SUCCESS;
	BTNR_3250Ob_Handle h3250ObDev;
	BTNR_Handle hDev;


	BDBG_ENTER(BTNR_3250Ob_Open);
	BDBG_ASSERT( hQob );
	BSTD_UNUSED(hChip);

	hDev = NULL;
	/* Alloc memory from the system heap */
	h3250ObDev = (BTNR_3250Ob_Handle) BKNI_Malloc( sizeof( BTNR_P_3250Ob_Handle ) );
	if( h3250ObDev == NULL )
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("BTNR_3250Ob_Open: BKNI_malloc() failed"));
		goto done;
	}
	BKNI_Memset( h3250ObDev, 0x00, sizeof( BTNR_P_3250Ob_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_OUT_OF_SYSTEM_MEMORY;
		BDBG_ERR(("BTNR_3250Ob_Open: BKNI_malloc() failed"));
		BKNI_Free( h3250ObDev );
		goto done;
	}

	h3250ObDev->magicId = DEV_MAGIC_ID;
	h3250ObDev->settings.ifFreq = pDefSettings->ifFreq;
	h3250ObDev->settings.hQobDev = hQob;
	h3250ObDev->settings.rfFreq = 0;

	hDev->hDevImpl = (void *) h3250ObDev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_3250Ob_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_3250Ob_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) NULL;
	hDev->pClose = (BTNR_CloseFunc) BTNR_3250Ob_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc)NULL;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc)NULL;

done:
	*phDev = hDev;
	BDBG_LEAVE(BTNR_3250Ob_Open);
	return( retCode );
}

BERR_Code BTNR_3250Ob_GetDefaultSettings(
	BTNR_3250Ob_Settings *pDefSettings,	/* [output] Returns default setting */
	BCHP_Handle hChip					/* [in] Chip handle */
	)
{
	BERR_Code retCode = BERR_SUCCESS;

	
	BDBG_ENTER(BTNR_3250Ob_GetDefaultSettings);
	BSTD_UNUSED(hChip);

	*pDefSettings = defDevSettings;

	BDBG_LEAVE(BTNR_3250Ob_GetDefaultSettings);
	return( retCode );
}

