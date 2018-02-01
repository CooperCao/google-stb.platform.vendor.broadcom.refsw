/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bads.h"
#include "bads_priv.h"
#include "bads_7550_api.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_thd_core.h"
#include "bchp_ds_tuner_ref.h"
#include "bchp_ds_eq.h"
#include "bchp_ds.h"
#include "bchp_gfd_1.h"


BDBG_MODULE(bads_soc_priv);

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/

/******************************************************************************
  BADS_Soc_P_TimerFunc_isr()
 ******************************************************************************/
BERR_Code BADS_Soc_P_TimerFunc_isr(BADS_Soc_Handle hImplDev )
{
	BDBG_ASSERT( hImplDev );
	BKNI_SetEvent(hImplDev->hInterruptEvent);
	return BERR_SUCCESS;
}
/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BADS_Soc_Open(
    BADS_Handle *pAds,                          /* [out] Returns handle */
    BCHP_Handle hChip,                          /* [in] Chip handle */
    BREG_Handle hRegister,                      /* [in] Register handle */
    BINT_Handle hInterrupt,                     /* [in] Interrupt handle */
    const struct BADS_Settings *pDefSettings    /* [in] Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Handle hDev;
    unsigned int chnIdx;
	BTMR_Settings sTimerSettings;
    BADS_Soc_Handle hImplDev = NULL;

    BDBG_ENTER(BADS_Soc_Open);

    /* Alloc memory from the system heap */
    hDev = (BADS_Handle) BKNI_Malloc( sizeof( BADS_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
        goto done;
    }
    
    BKNI_Memset( hDev, 0x00, sizeof( BADS_P_Handle ) );
    hDev->magicId = DEV_MAGIC_ID;
	BKNI_Memcpy(&hDev->settings, pDefSettings, sizeof(*pDefSettings));


    hImplDev = (BADS_Soc_Handle) BKNI_Malloc( sizeof( BADS_P_Soc_Handle ) );
    if( hImplDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BADS_Open: BKNI_malloc() failed, impl"));
        goto done;
    }

    BKNI_Memset( hImplDev, 0x00, sizeof( BADS_P_Soc_Handle ) );
    hImplDev->magicId = DEV_MAGIC_ID;
    hImplDev->hChip = hChip;
    hImplDev->hRegister = hRegister;
    hImplDev->hInterrupt = hInterrupt;
    hImplDev->mxChnNo = MX_ADS_CHANNELS;
    
    /* Create timer for status lock check */
    BTMR_GetDefaultTimerSettings(&sTimerSettings);
    sTimerSettings.type = BTMR_Type_ePeriodic;
    sTimerSettings.cb_isr = (BTMR_CallbackFunc)BADS_Soc_P_TimerFunc_isr;
    sTimerSettings.pParm1 = (void*)hImplDev;
    sTimerSettings.parm2 = 0;
    sTimerSettings.exclusive = false;

    retCode = BTMR_CreateTimer (hDev->settings.hTmr, &hImplDev->hTimer, &sTimerSettings);
    if ( retCode != BERR_SUCCESS )
    {
        BDBG_ERR(("BTHD_Open: Create Timer Failed"));
        retCode = BERR_TRACE(retCode);
        goto done;
    }	

	BKNI_CreateEvent(&(hImplDev->hInterruptEvent));



    for( chnIdx = 0; chnIdx < MX_ADS_CHANNELS; chnIdx++ )
    {
        hImplDev->hAdsChn[chnIdx] = NULL;
    }
    hDev->pImpl = hImplDev;
    *pAds = hDev;

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            BKNI_Free( hDev );
        }
        if( hImplDev != NULL )
        {
            BKNI_Free( hImplDev );
        }       
        *pAds = NULL;
    }
    BDBG_LEAVE(BADS_Soc_Open);
    return( retCode );
}

BERR_Code BADS_Soc_Close(
    BADS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	BADS_Soc_Handle hImplDev;

    BDBG_ENTER(BADS_Soc_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
	hImplDev = (BADS_Soc_Handle) hDev->pImpl;

	BTMR_DestroyTimer(hImplDev->hTimer);
	BKNI_DestroyEvent(hImplDev->hInterruptEvent);
    BKNI_Free( (void *)hDev->pImpl );
    hDev->magicId = 0x00;       /* clear it to catch improper use */
    BKNI_Free( (void *) hDev );


    BDBG_LEAVE(BADS_Soc_Close);
    return( retCode );
}

BERR_Code BADS_Soc_Init(
    BADS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;
    uint16_t chipVer;
    
    BDBG_ENTER(BADS_Soc_Init);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    chipVer = BREG_Read32(hImplDev->hRegister, BCHP_SUN_TOP_CTRL_PROD_REVISION);
    hImplDev->verInfo.majVer = chipVer >> 8;
    hImplDev->verInfo.minVer = chipVer & 0xFF;

    BDBG_LEAVE(BADS_Soc_Init);
    return( retCode );
}

BERR_Code BADS_Soc_GetVersion(
    BADS_Handle hDev,                   /* [in] Device handle */
    BADS_Version *pVersion              /* [out] Returns version */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;

    BDBG_ENTER(BADS_Soc_GetVersion);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    *pVersion = hImplDev->verInfo;      /* use saved data */

    BDBG_LEAVE(BADS_Soc_GetVersion);
    return( retCode );
}

BERR_Code BADS_Soc_GetTotalChannels(
    BADS_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;

    BDBG_ENTER(BADS_Soc_GetTotalChannels);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    *totalChannels = hImplDev->mxChnNo; /* use saved data */

    BDBG_LEAVE(BADS_Soc_GetTotalChannels);
    return( retCode );
}


BERR_Code BADS_Soc_OpenChannel(
    BADS_Handle hDev,                                   /* [in] Device handle */
    BADS_ChannelHandle *phChn,                          /* [out] Returns channel handle */
    unsigned int channelNo,                             /* [in] Channel number to open */
    const struct BADS_ChannelSettings *pChnDefSettings  /* [in] Channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;
    BADS_Soc_ChannelHandle hImplChnDev = NULL;
    BADS_ChannelHandle hChnDev;
	uint32_t BufSrc;

    BDBG_ENTER(BADS_Soc_OpenChannel);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    
    hChnDev = NULL;
    if( channelNo < hImplDev->mxChnNo )
    {
        if( hImplDev->hAdsChn[channelNo] == NULL )
        {
		    BDBG_MSG(("BADS_Soc_OpenChannel"));
            /* Alloc memory from the system heap */
            hChnDev = (BADS_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_ChannelHandle ) );
            if( hChnDev == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed"));
                goto done;
            }
            
            BKNI_Memset( hChnDev, 0x00, sizeof( BADS_P_ChannelHandle ) );
            hChnDev->magicId = DEV_MAGIC_ID;
            hChnDev->hAds = hDev;

            hImplChnDev = (BADS_Soc_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_Soc_ChannelHandle ) );
            if( hImplChnDev == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed, impl"));
                goto done;
            }
            
            hImplChnDev->chnNo = channelNo;
         
			hImplChnDev->pAcquireParam = (BADS_Soc_P_AcquireParams_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(BADS_Soc_P_AcquireParams_t), 0, 0 );
			BMEM_ConvertAddressToOffset(hDev->settings.hHeap, hImplChnDev->pAcquireParam, &BufSrc );
			BKNI_Memset(hImplChnDev->pAcquireParam, 0x00, sizeof( BADS_Soc_P_AcquireParams_t ) );
			BREG_Write32(hImplDev->hRegister, BCHP_GFD_1_SCRATCH1, BufSrc);
			hImplChnDev->pStatus = (BADS_Soc_P_Status_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(BADS_Soc_P_Status_t), 0, 0 );
			BMEM_ConvertAddressToOffset(hDev->settings.hHeap, hImplChnDev->pStatus, &BufSrc );
			BKNI_Memset( hImplChnDev->pStatus, 0x00, sizeof( BADS_Soc_P_Status_t ) );
			hImplChnDev->pAcquireParam->StatusStartAddress = BufSrc;
			if (pChnDefSettings != NULL) 
				hImplChnDev->pAcquireParam->IfFrequency = pChnDefSettings->ifFreq;
			if (pChnDefSettings->autoAcquire == true) 
				hImplChnDev->pAcquireParam->AutoAcquireMode = 1;
			else
				hImplChnDev->pAcquireParam->AutoAcquireMode = 0;

			if (pChnDefSettings->fastAcquire == true) 
				hImplChnDev->pAcquireParam->fastAcquire = 1;
			else
				hImplChnDev->pAcquireParam->fastAcquire = 0;

            if (pChnDefSettings != NULL) 
			{
				hImplChnDev->settings.ifFreq = pChnDefSettings->ifFreq;
				hImplChnDev->settings.autoAcquire = pChnDefSettings->autoAcquire;
				hImplChnDev->settings.fastAcquire = pChnDefSettings->fastAcquire;
			}
            hImplDev->hAdsChn[channelNo] = hChnDev;
            hChnDev->pImpl = hImplChnDev;

            *phChn = hChnDev;

			/*BADS_Soc_P_Init_ADS(hChnDev);*/

        }
        else
        {
            retCode = BERR_TRACE(BADS_ERR_NOTAVAIL_CHN_NO);
        }
    }
    else
    {
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hChnDev != NULL )
        {
            BKNI_Free( hChnDev );
            hImplDev->hAdsChn[channelNo] = NULL;
        }
        if( hImplChnDev != NULL )
        {
            BKNI_Free( hImplChnDev );
        }
        *phChn = NULL;
    }
    BDBG_LEAVE(BADS_Soc_OpenChannel);
    return( retCode );
}

BERR_Code BADS_Soc_CloseChannel(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;
    BADS_Soc_ChannelHandle hImplChnDev;
    BADS_Handle hAds;
    unsigned int chnNo;
    
    BDBG_ENTER(BADS_Soc_CloseChannel);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Soc_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    hAds = hChn->hAds;
    hImplDev = (BADS_Soc_Handle) hAds->pImpl;
    BDBG_ASSERT( hImplDev );
   

    chnNo = hImplChnDev->chnNo;
    hChn->magicId = 0x00;       /* clear it to catch inproper use */
	BMEM_Free(hAds->settings.hHeap ,(void *)((BADS_P_Soc_ChannelHandle *)(hChn->pImpl))->pAcquireParam);
	BMEM_Free(hAds->settings.hHeap,(void *)((BADS_P_Soc_ChannelHandle *)(hChn->pImpl))->pStatus);
    BKNI_Free( hChn->pImpl );
    BKNI_Free( hChn );
    hImplDev->hAdsChn[chnNo] = NULL;



    BDBG_LEAVE(BADS_Soc_CloseChannel);
    return( retCode );
}

BERR_Code BADS_Soc_GetDevice(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Handle *phDev                  /* [out] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_Soc_GetDevice);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *phDev = hChn->hAds;

    BDBG_LEAVE(BADS_Soc_GetDevice);
    return( retCode );
}

BERR_Code BADS_Soc_Acquire(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_InbandParam *ibParam           /* [in] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_ChannelHandle hImplChnDev;
	BADS_Handle hDev;
	BADS_Soc_Handle hImplDev;

    BDBG_ENTER(BADS_Soc_Acquire);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Soc_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	hDev = (BADS_Handle)(hChn->hAds);
	BDBG_ASSERT( hDev );
	hImplDev = (BADS_Soc_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );
	BDBG_MSG(("BADS_Soc_Acquire"));
	hImplChnDev->pAcquireParam->ModulationType = ibParam->modType;	
	hImplChnDev->pAcquireParam->SymbolRate		= ibParam->symbolRate;			
	hImplChnDev->pAcquireParam->InvertSpectrum	= ibParam->invertSpectrum;
	hImplChnDev->pAcquireParam->spectrum	    = ibParam->spectrum; 
	hImplChnDev->pAcquireParam->EnableDpm      = ibParam->enableDpm;
       hImplChnDev->pAcquireParam->LockUpdate = true;
	BKNI_Memset(hImplChnDev->pStatus, 0x00, sizeof( BADS_Soc_P_Status_t ) ); /* Clear status structure upon new TuneAcquire command from host */
	BADS_Soc_P_Acquire(hChn);

    if (!hImplDev->timerStopped)
    {
        BTMR_StopTimer(hImplDev->hTimer);  /* the timer is in Micro second */
    }
    BTMR_StartTimer(hImplDev->hTimer, 25000);   /* the timer is in Micro second */

    BDBG_LEAVE(BADS_Soc_Acquire);
    return( retCode );
}

BERR_Code BADS_Soc_GetStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Status *pStatus                /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_ChannelHandle hImplChnDev;
    
    BDBG_ENTER(BADS_Soc_GetStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Soc_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

	BADS_Soc_P_Get_Status(hChn);

	pStatus->isPowerSaverEnabled = 0;													/* Enable=1, Disable=0 */
	pStatus->modType = hImplChnDev->pStatus->ModulationType;							/* Modulation type */
	pStatus->ifFreq = hImplChnDev->pStatus->IfFrequency;								/* in Hertz, IF freq. */
	pStatus->symbolRate = hImplChnDev->pStatus->SymbolRate;								/* in Baud */
	pStatus->symbolRateError = (int32_t) hImplChnDev->pStatus->TimingFrequencyError;	/* in Baud */
	pStatus->isFecLock = (hImplChnDev->pStatus->FecStatus==BADS_FECStatus_eLocked)?1:0;	/* lock=1, unlock=0 */
	pStatus->isQamLock = (hImplChnDev->pStatus->QamStatus==BADS_QAMStatus_eLocked)?1:0;	/* lock=1, unlock=0 */
	pStatus->correctedCount = hImplChnDev->pStatus->FEC_Corr_RS_Blocks;							/* reset on every call to BADS_Soc_P_Reset_Status() */
	pStatus->uncorrectedCount = hImplChnDev->pStatus->FEC_UCorr_RS_Blocks;						/* reset on every call to BADS_Soc_P_Reset_Status() */
	pStatus->snrEstimate = hImplChnDev->pStatus->SNR;									/* in 1/256 dB */
	pStatus->agcIntLevel = 0;																					/* returns 0, moved to tuner*/
	pStatus->agcExtLevel = 0;																					/* returns 0, moved to tuner*/
	pStatus->carrierFreqOffset = hImplChnDev->pStatus->CarrierFrequencyError*1000;		/* in 1/1000 Hz */
	pStatus->carrierPhaseOffset = hImplChnDev->pStatus->CarrierPhaseError*1000;			/* in 1/1000 Hz */
	pStatus->rxSymbolRate = hImplChnDev->pStatus->SymbolRate;							/* in Baud, received symbol rate */
	pStatus->interleaveDepth = (uint16_t)hImplChnDev->pStatus->InterleaverDepth;		/* use in Docsis */
	pStatus->goodRsBlockCount = 0;							/* reset on every read */
	pStatus->berRawCount = 0;   							/* reset on every read */
	pStatus->dsChannelPower = 0;							/* in 10s of dBmV unit; OCAP DPM support for video channels */
	pStatus->mainTap = 0;									/* Channel main tap coefficient */
	pStatus->equalizerGain = 0;								/* Channel equalizer gain value in dBm */
	/* OCAP required postRsBER for all DS channels. postRsBER and elapsedTimeSec will be reset on every channel change*/
	/* fpostRsBER normally between xxxe-6 to xxxe-12 float value, to send this small float number over rMagnum to host, we convert it to uint32 using the formula*/
	pStatus->postRsBER = 0;						  /* returns 0, NO FLOAT in Soc*/
	                                    /* For Annex A  postRsBER = (11.224*uncorrectedCount)/(correctedCount+uncorrectedCount+cleanCount)*187*8)*/
	                                    /* For Annex B  postRsBER = (11.224*uncorrectedCount)/(correctedCount+uncorrectedCount+cleanCount)*122*7)*/

	pStatus->elapsedTimeSec = 0;							  /* returns 0, not needed*/
	pStatus->isSpectrumInverted = hImplChnDev->pStatus->IsSpectrumInverted;				/* If 1, Spectrum is inverted relative to the AP bootup. */
	
	pStatus->cleanCount = hImplChnDev->pStatus->FEC_Clean_RS_Blocks;							/* reset on every call to BADS_Soc_P_Reset_Status() */
	pStatus->agcAGFLevel = hImplChnDev->pStatus->AGF_Level;										  /* in 1/10 percent */
	pStatus->agcAGF2Level = hImplChnDev->pStatus->AGF2_Level;										/* in 1/10 percent */
	pStatus->correctedBits = hImplChnDev->pStatus->FEC_Corr_Bits;							/* reset on every call to BADS_Soc_P_Reset_Status() */

    BDBG_LEAVE(BADS_Soc_GetStatus);
    return( retCode );
}

BERR_Code BADS_Soc_GetLockStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_LockStatus *pLockStatus         /* [out] Returns lock status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_ChannelHandle hImplChnDev;

    BDBG_ENTER(BADS_Soc_GetLockStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Soc_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

	BADS_Soc_P_Get_LockStatus(hChn);
	*pLockStatus = hImplChnDev->pStatus->FecStatus;

    BDBG_LEAVE(BADS_Soc_GetLockStatus);
    return( retCode );

}

BERR_Code BADS_Soc_GetSoftDecision(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	uint32_t ReadReg;
	int16_t i;
	BADS_Soc_Handle hImplDev;
    BADS_Handle hDev;
    BADS_Soc_ChannelHandle hImplChnDev;
    BDBG_ENTER(BADS_Soc_GetSoftDecision);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hImplChnDev = (BADS_Soc_ChannelHandle) hChn->pImpl;
	BDBG_ASSERT( hImplChnDev );
	hDev = (BADS_Handle)(hChn->hAds);
	BDBG_ASSERT( hDev );
	hImplDev = (BADS_Soc_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );

    ReadReg = BREG_Read32(hImplDev->hRegister, BCHP_DS_EQ_SOFT);

    for (i = 0; i < nbrToGet ; i++)
    {
        iVal[i] = ((ReadReg & 0xffff0000) >> 16) >> 4;
        qVal[i] = (ReadReg & 0x0000ffff) >> 4;

		if (iVal[i] & 0x800)
            iVal[i] -= 0x1000;
        if (qVal[i] & 0x800)
            qVal[i] -= 0x1000;
    }
    *nbrGotten = nbrToGet;  

    BDBG_LEAVE(BADS_Soc_GetSoftDecision);
    return( retCode );
}


BERR_Code BADS_Soc_EnablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_ChannelHandle hImplChnDev;


    BDBG_ENTER(BADS_Soc_EnablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Soc_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

	BADS_Soc_P_PowerDn(hChn);

    BDBG_LEAVE(BADS_Soc_EnablePowerSaver);
    return( retCode );
}

BERR_Code BADS_Soc_ProcessNotification(
    BADS_ChannelHandle hChn,                /* [in] Device channel handle */
    unsigned int event                      /* [in] Event code and event data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hChn);
    BSTD_UNUSED(event);

    BDBG_WRN(("Not supported for this frontend chip."));
    return( retCode );
}

BERR_Code BADS_Soc_InstallCallback(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Callback callbackType,         /* [in] Type of callback */
    BADS_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
	BADS_Handle hDev = (BADS_Handle)(hChn->hAds);
	BADS_Soc_Handle hImplDev = (BADS_Soc_Handle) hDev->pImpl;
    BDBG_ENTER(BADS_Soc_InstallCallback);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );



    switch( callbackType )
    {
        case BADS_Callback_eLockChange:
            hImplDev->pCallback[callbackType] = pCallback;
            hImplDev->pCallbackParam[callbackType] = pParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BDBG_LEAVE(BADS_Soc_InstallCallback);
    return( retCode );
}

BERR_Code BADS_Soc_SetDaisyChain(
    BADS_Handle hDev,       /* [in] Returns handle */
    bool enableDaisyChain   /* [in] Eanble/disable daisy chain. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;


    BDBG_ENTER(BADS_Soc_SetDaisyChain);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );
    if (enableDaisyChain)
		BADS_Soc_P_WriteField(hImplDev->hRegister, DS_TUNER_REF_DB0_WDATA_01, DS_PD, 0); 
	else
		BADS_Soc_P_WriteField(hImplDev->hRegister, DS_TUNER_REF_DB0_WDATA_01, DS_PD, 1);
    hImplDev->isDaisyChain = enableDaisyChain;

    BDBG_LEAVE(BADS_Soc_SetDaisyChain);
    return( retCode );
}

BERR_Code BADS_Soc_GetDaisyChain(
    BADS_Handle hDev,           /* [in] Returns handle */
    bool *isEnableDaisyChain    /* [in] Eanble/disable daisy chain. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;
    
    BDBG_ENTER(BADS_Soc_GetDaisyChain);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );
    
    *isEnableDaisyChain = hImplDev->isDaisyChain;

    BDBG_LEAVE(BADS_Soc_GetDaisyChain);
    return( retCode );
}

BERR_Code BADS_Soc_ResetStatus(
    BADS_ChannelHandle hChannel           /* [in] Returns handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    
    BDBG_ENTER(BADS_Soc_ResetStatus);
    BDBG_ASSERT( hChannel );
    BDBG_ASSERT( hChannel->magicId == DEV_MAGIC_ID );
	BADS_Soc_P_Reset_Status(hChannel);

    BDBG_LEAVE(BADS_Soc_ResetStatus);
    return( retCode );
}

BERR_Code BADS_Soc_GetInterruptEventHandle(
   BADS_Handle hDev,            /* [in] BADS handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
)
{

	BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;
    
    BDBG_ENTER(BADS_Soc_GetInterruptEventHandle);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );
	*phEvent = hImplDev->hInterruptEvent;


    BDBG_LEAVE(BADS_Soc_GetInterruptEventHandle);
    return( retCode );



}

BERR_Code BADS_Soc_ProcessInterruptEvent(
    BADS_Handle hDev           /* [in] Returns handle */
    )
{
	BERR_Code retCode = BERR_SUCCESS;
	static uint32_t lock_status = 2;
    BADS_Soc_Handle hImplDev;
	BADS_ChannelHandle hChnDev;
	BADS_Soc_ChannelHandle h;


    BDBG_ENTER(BADS_Soc_P_TimerFunc);
	hImplDev = (BADS_Soc_Handle) hDev->pImpl;
	BDBG_ASSERT( hImplDev );
	hChnDev = (BADS_ChannelHandle)(hImplDev->hAdsChn[0]);
	BDBG_ASSERT( hChnDev );
	h = (BADS_Soc_ChannelHandle)(hChnDev->pImpl);
    BDBG_ASSERT( h );

	BTMR_StopTimer(hImplDev->hTimer);  /* the timer is in Micro second */
	BKNI_ResetEvent(hImplDev->hInterruptEvent);
	BADS_Soc_P_Get_LockStatus(hChnDev);

/*	if(( hImplDev->pCallback[BADS_Callback_eLockChange] != NULL ) && (lock_status != h->pStatus->FecStatus))*/
    if ( hImplDev->pCallback[BADS_Callback_eLockChange] != NULL )
	{
            if (h->pStatus->FecStatus == 1)
            {
                if (h->pAcquireParam->LockUpdate)
                {
                    h->pAcquireParam->LockUpdate = false;
                    BKNI_EnterCriticalSection();
                    (hImplDev->pCallback[BADS_Callback_eLockChange])(hImplDev->pCallbackParam[BADS_Callback_eLockChange] );
                    BKNI_LeaveCriticalSection();
                }
            }
            else
            {
                if (lock_status != h->pStatus->FecStatus)
                {
                    h->pAcquireParam->LockUpdate = true;
                    BKNI_EnterCriticalSection();
                    (hImplDev->pCallback[BADS_Callback_eLockChange])(hImplDev->pCallbackParam[BADS_Callback_eLockChange] );
                    BKNI_LeaveCriticalSection();
                }
            }
	}
	lock_status = h->pStatus->FecStatus;

	if (h->pAcquireParam->NexusStatusMode & BADS_NexusStatusMode_EnableStatusForNexus)
	{
		BADS_Soc_P_Get_Status(hChnDev);
	    h->pAcquireParam->NexusStatusMode &= (~BADS_NexusStatusMode_EnableStatusForNexus);
	}
	if (h->pAcquireParam->AcquireStartMode & BADS_AcquireStartMode_ResetStatus)
	{
		BDBG_MSG(("call reset"));
		BADS_Soc_P_Reset_Status(hChnDev);
	    h->pAcquireParam->AcquireStartMode &= (~BADS_AcquireStartMode_ResetStatus);
	}

	if (h->pAcquireParam->AcquireStartMode & BADS_AcquireStartMode_Acquire)
	{
		h->pAcquireParam->AcquireStartMode &= ~BADS_AcquireStartMode_Acquire;
		h->pStatus->FecStatus = 0;
		h->pStatus->ReAcquireCount = 0;
		BADS_Soc_P_Acquire(hChnDev);
	}
	else
	{
		if (h->pAcquireParam->AutoAcquireMode == 1)
		{
			if (lock_status == 0)
			{
				BADS_Soc_P_Acquire(hChnDev);
			}
		}
	}
	BTMR_StartTimer(hImplDev->hTimer, 25000);   /* the timer is in Micro second */
	BDBG_LEAVE(BADS_Soc_P_TimerFunc);
	return retCode;
}



BERR_Code BADS_Soc_Untune(
    BADS_Handle hDev           /* [in] Returns handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;

    BDBG_ENTER(BADS_Soc_Untune);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;  
    BDBG_ASSERT( hImplDev );

    BTMR_StopTimer(hImplDev->hTimer);  /* the timer is in Micro second */
    hImplDev->timerStopped = true;

    BDBG_LEAVE(BADS_Soc_Untune);
    return( retCode );
}

/***************************************************************************
Summary:
    This function returns the default settings for ADS module.

Description:
    This function is responsible for returns the default setting for 
    BADS module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BADS_Leap_Open()

****************************************************************************/

BERR_Code BADS_Soc_GetChannelDefaultSettings(
    BADS_Handle hDev,                       /* [in] Device handle */
    unsigned int channelNo,                 /* [in] Channel number to default setting for */
    BADS_ChannelSettings *pChnDefSettings   /* [out] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Soc_Handle hImplDev;

    BDBG_ENTER(BADS_Soc_GetChannelDefaultSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Soc_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    switch( channelNo )
    {
        case 0:
        if( channelNo < hImplDev->mxChnNo )
        {
            pChnDefSettings->ifFreq = 0;    /*The if freq for Soc is set to 0 as the internal tuner does not spit out seperate if frequency. */
            pChnDefSettings->autoAcquire = true;
            pChnDefSettings->fastAcquire = false;
            break;
        }
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BDBG_LEAVE(BADS_Soc_GetChannelDefaultSettings);
    return( retCode );
}