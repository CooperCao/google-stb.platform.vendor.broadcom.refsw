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
#include "bdbg.h"
#include "bmem.h"
#include "btnr_priv.h"
#include "btmr.h"
#include "btnr_ob_3x7x.h"
#include "btnr_3x7xob_priv.h"
#include "bchp_ufe.h"
#include "bchp_ufe_afe.h"
#include "bchp_sdadc.h"
#include "bchp_ufe_misc2.h"
#include "btnr_ob_tune.h"
#include "btnr_ob_struct.h"



BDBG_MODULE(btnr_ob_3x7x);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)

/******************************************************************************
* BTNR_Ob_3x7x_Open()
******************************************************************************/
BERR_Code BTNR_Ob_3x7x_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
    BTNR_Ob_3x7x_Settings *pSettings, /* [Input] settings structure */
	BREG_Handle hRegister
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_Ob_3x7x_Handle h3x7xDev;
    BTNR_Handle hDev;
	uint32_t BufSrc;
	void *cached_ptr, *tmpAddress;

    BDBG_ENTER(BTNR_Ob_3x7x_Open);

	BDBG_MSG(("BTNR_Ob_3x7x_Open"));

    hDev = NULL;
    /* Alloc memory from the system heap */
    h3x7xDev = (BTNR_Ob_3x7x_Handle) BKNI_Malloc( sizeof( BTNR_Ob_P_3x7x_Handle ) );
    if( h3x7xDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_Ob_3x7x_Open: BKNI_malloc() failed"));
        goto done;
    }
	BKNI_Memcpy(&h3x7xDev->settings, pSettings, sizeof(*pSettings));

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_Ob_3x7x_Open: BKNI_malloc() failed"));
        BKNI_Free( h3x7xDev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

	tmpAddress = (BTNR_Ob_3x7x_TuneParams_t *)BMEM_AllocAligned(pSettings->hHeap, sizeof(BTNR_Ob_3x7x_TuneParams_t), 0, 0 );
	if (tmpAddress == NULL )
	{
			BDBG_ERR(("BTNR_Ob_Open: BKNI_malloc() failed"));
			goto done;
	}
	BMEM_ConvertAddressToCached(pSettings->hHeap, tmpAddress, &cached_ptr);
	BKNI_Memset(cached_ptr, 0x00, sizeof( BTNR_Ob_3x7x_TuneParams_t ) );
	h3x7xDev->pTunerParams = cached_ptr;
	BMEM_FlushCache(pSettings->hHeap, h3x7xDev->pTunerParams, sizeof( BTNR_Ob_3x7x_TuneParams_t ) );
	BMEM_ConvertAddressToOffset(pSettings->hHeap, tmpAddress, &BufSrc );
	h3x7xDev->pTunerParams->BTNR_Ob_BBS_Params.BBSAddress = BufSrc;

	tmpAddress = (BTNR_Ob_3x7x_TuneStatus_t *)BMEM_AllocAligned(pSettings->hHeap, sizeof(BTNR_Ob_3x7x_TuneStatus_t), 0, 0 );
	if (tmpAddress == NULL )
	{
			BDBG_ERR(("BTNR_Ob_Open: BKNI_malloc() failed"));
			goto done;
	}
	BMEM_ConvertAddressToCached(pSettings->hHeap, tmpAddress, &cached_ptr);
	BKNI_Memset(cached_ptr, 0x00, sizeof( BTNR_Ob_3x7x_TuneStatus_t ) );
	h3x7xDev->pTunerStatus = cached_ptr;
	BMEM_FlushCache(pSettings->hHeap, h3x7xDev->pTunerStatus, sizeof( BTNR_Ob_3x7x_TuneStatus_t ) );

	h3x7xDev->magicId = DEV_MAGIC_ID;
	h3x7xDev->hHeap = pSettings->hHeap;
	h3x7xDev->hRegister = hRegister;

	hDev->hDevImpl = (void *) h3x7xDev;
	hDev->magicId = DEV_MAGIC_ID;
	hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_Ob_3x7x_SetRfFreq;
	hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_Ob_3x7x_GetRfFreq;
	hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) NULL;
	hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) NULL;
	hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_Ob_3x7x_GetInfo;
	hDev->pClose = (BTNR_CloseFunc) BTNR_Ob_3x7x_Close;
	hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_Ob_3x7x_GetPowerSaver;
	hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_Ob_3x7x_SetPowerSaver;
	hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_Ob_3x7x_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_Ob_3x7x_SetSettings;

	BKNI_CreateEvent(&(h3x7xDev->hInterruptEvent));

	h3x7xDev->pTunerStatus->PowerStatus = BTNR_Ob_ePower_Off;
done:
    *phDev = hDev;
    BDBG_LEAVE(BTNR_Ob_3x7x_Open);
    return( retCode );
}


/******************************************************************************
* BTNR_Ob_3x7x_GetDefaultSettings
******************************************************************************/
BERR_Code BTNR_Ob_3x7x_GetDefaultSettings(
    BTNR_Ob_3x7x_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);
    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
    return BERR_SUCCESS;
}


/******************************************************************************
  BTNR_Ob_3x7x_P_TimerFunc()
 ******************************************************************************/
BERR_Code BTNR_Ob_3x7x_P_TimerFunc(void *myParam1, int myParam2)
{
    BTNR_Handle hDev = (BTNR_Handle)myParam1;
	BTNR_Ob_P_3x7x_Handle *p3x7x = (BTNR_Ob_P_3x7x_Handle *)(hDev->hDevImpl);
	BSTD_UNUSED(myParam2);
	BKNI_SetEvent(p3x7x->hInterruptEvent);
	return BERR_SUCCESS;
}

/******************************************************************************
  BTNR_Ob_3x7x_GetInterruptEventHandle()
 ******************************************************************************/
BERR_Code BTNR_Ob_3x7x_GetInterruptEventHandle(BTNR_Handle h, BKNI_EventHandle* hEvent)
{
    *hEvent = ((BTNR_Ob_P_3x7x_Handle *)(h->hDevImpl))->hInterruptEvent;
    return BERR_SUCCESS;
}

/******************************************************************************
  BTNR_Ob_3x7x_ProcessInterruptEvent()
 ******************************************************************************/
BERR_Code BTNR_Ob_3x7x_ProcessInterruptEvent(BTNR_Handle hTnrDev)
{

  BTNR_Ob_P_3x7x_Handle *hDev = (BTNR_Ob_P_3x7x_Handle *)(hTnrDev->hDevImpl);

  BMEM_FlushCache(hDev->hHeap, hDev->pTunerParams, sizeof( BTNR_Ob_3x7x_TuneParams_t ) );

  if (hDev->pTunerStatus->PowerStatus != BTNR_Ob_ePower_On)
  {
	BDBG_ERR(("BTNR_Ob_3x7x_ProcessInterruptEvent: power is still off  "));
	return BERR_NOT_INITIALIZED;
  }

  BDBG_MSG(("BTNR_Ob_3x7x_ProcessInterruptEvent %0x  ", hDev->pTunerParams->BTNR_Ob_BBS_Params.BBSConnectMode ));

  if (hDev->pTunerParams->BTNR_Ob_BBS_Params.BBSConnectMode & BTNR_Ob_BBSConnectMode_Tune)
  {
	hDev->pTunerParams->BTNR_Ob_BBS_Params.BBSConnectMode &= (~BTNR_Ob_BBSConnectMode_Tune);
	BTNR_Ob_3x7x_SetRfFreq(hDev, hDev->pTunerParams->BTNR_Ob_BBS_Params.rfFreq, BTNR_TunerMode_eDigital);
  }

  return BERR_SUCCESS;

}

/******************************************************************************
  BTNR_3x7x_Get_RF_Status()
 ******************************************************************************/
BERR_Code BTNR_Ob_3x7x_Get_RF_Status(BTNR_Handle hTnrDev, BTNR_Ob_3x7x_RfStatus_t *RfCallbackStatus)
{
	BTNR_Ob_P_3x7x_Handle *p3x7x = (BTNR_Ob_P_3x7x_Handle *)(hTnrDev->hDevImpl);

    if (p3x7x->pTunerStatus->PowerStatus != BTNR_Ob_ePower_On)
    {
		BDBG_ERR(("BTNR_Ob_3x7x_Get_RF_Status: power is still off  "));
		return BERR_NOT_INITIALIZED;
    }
	/* BTNR_P_Oob_TunerStatus(p3x7x); */
    RfCallbackStatus->Total_Mix_After_ADC = p3x7x->pTunerStatus->Total_Mix_After_ADC;
    RfCallbackStatus->PreADC_Gain_x256db  = p3x7x->pTunerStatus->PreADC_Gain_x256db;
    RfCallbackStatus->PostADC_Gain_x256db = p3x7x->pTunerStatus->PostADC_Gain_x256db;
    RfCallbackStatus->External_Gain_x256db = p3x7x->pTunerStatus->External_Gain_x256db;

	return BERR_SUCCESS;
}










