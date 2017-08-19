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
#include "btnr_3117ob.h"
#include "btnr_31xxob_priv.h"

BDBG_MODULE(btnr_3117ob);

#define	DEV_MAGIC_ID					((BERR_TNR_ID<<16) | 0xFACE)
#define	BTNR_31XXOb_SETTINGS_I2CADDR	(0x66)			/* 7bit addr */
#define	BTNR_31XXOb_IF_FREQ	            44000000			/* 7bit addr */

/*******************************************************************************
*
*	Public Module Functions
*
*******************************************************************************/

BERR_Code BTNR_3117Ob_Open(
	BTNR_Handle *phDev,					/* [output] Returns handle */
	BTNR_3117Ob_Settings *pSettings, /* [Input] settings structure */
	BHAB_Handle hHab                  /* [Input] Hab Handle */   
	)
{
	BERR_Code retCode = BERR_SUCCESS;
 	BTNR_31xxOb_Handle h3117ObDev;
	BTNR_P_31xxOb_Settings *pTnrImplData;
	BTNR_Handle hDev;


	BDBG_ENTER(BTNR_3117Ob_Open);
	BDBG_ASSERT( hHab );	
	
	hDev = NULL;
	/* Alloc memory from the system heap */
	h3117ObDev = (BTNR_31xxOb_Handle) BKNI_Malloc( sizeof( BTNR_P_31xxOb_Handle ) );
	if( h3117ObDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3117_Open: BKNI_malloc() failed"));
		goto done;
	}
	BKNI_Memset( h3117ObDev, 0x00, sizeof( BTNR_P_31xxOb_Handle ) );

	hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
	if( hDev == NULL )
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("BTNR_3117Ob_Open: BKNI_malloc() failed"));
		BKNI_Free( h3117ObDev );
		goto done;
	}
	BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

	h3117ObDev->magicId = DEV_MAGIC_ID;
	pTnrImplData = &h3117ObDev->settings;
	pTnrImplData->rfFreq = 0;
	pTnrImplData->tunerMode = BTNR_TunerMode_eDigital;
	pTnrImplData->ifFreq = pSettings->ifFreq;

	h3117ObDev->hHab = hHab;
	hDev->hDevImpl = (void *) h3117ObDev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_31xxOb_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_31xxOb_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_31xxOb_GetAgcRegVal;
	hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_31xxOb_SetAgcRegVal;
	hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_31xxOb_GetInfo;
	hDev->pClose = (BTNR_CloseFunc) BTNR_31xxOb_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_31xxOb_GetPowerSaver;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_31xxOb_SetPowerSaver;

done:
	*phDev = hDev;
	BDBG_LEAVE(BTNR_3117Ob_Open);
	return( retCode );
}

BERR_Code BTNR_3117Ob_GetDefaultSettings(
	BTNR_3117Ob_Settings *pDefSettings	/* [out] Returns default setting */
	)
{
	BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
	pDefSettings->i2cAddr = BTNR_31XXOb_SETTINGS_I2CADDR;
	pDefSettings->ifFreq = BTNR_31XXOb_IF_FREQ;

	return BERR_SUCCESS;
}

