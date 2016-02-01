/***************************************************************************
 *     Copyright (c) 2005-2011, Broadcom Corporation
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
#include "bstd.h"
#include "bads.h"
#include "bads_priv.h"
#include "bads_7125_priv.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_ds_tuner_ref_0.h"
#include "bchp_ds_tuner_ref_1.h"
#include "bchp_ds_eq_0.h"
#include "bchp_ds_eq_1.h"

BDBG_MODULE(bads_7125_priv);

#define BADS_7125_P_USE_TIMER 1
/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/

/******************************************************************************
  BADS_7125_P_TimerFunc()
 ******************************************************************************/
BERR_Code BADS_7125_P_TimerFunc(void *myParam1, int myParam2)
{

    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;
    BADS_ChannelHandle hChnDev;
    BADS_7125_ChannelHandle h = NULL;
    unsigned i;
    BADS_AcquireState_t AcqState = BADS_Acq_Inactive; /* slow the timer period if not acquiring */
    BSTD_UNUSED(myParam2);

    BDBG_ENTER(BADS_7125_P_TimerFunc);
    hImplDev = (BADS_7125_Handle) myParam1;
    BDBG_ASSERT( hImplDev );

    BDBG_MSG(("%s",__FUNCTION__));

    for (i=0;i<MX_ADS_CHANNELS;i++)
    {
        hChnDev = (BADS_ChannelHandle)(hImplDev->hAdsChn[i]);
        if ( NULL == hChnDev )
            continue;
        h = (BADS_7125_ChannelHandle)(hChnDev->pImpl);
        BDBG_ASSERT( h );
        if (h->pAcquireParam->AcqState==BADS_Acq_Inactive)
            continue;
        if (h->pAcquireParam->AcqState>AcqState)
            AcqState = h->pAcquireParam->AcqState;

        BADS_7125_P_Get_LockStatus(hChnDev);

        if ( hImplDev->pCallback[i][BADS_Callback_eLockChange] != NULL )
        {
                BDBG_MSG(("AcqState = %u FecStatus = %u LockUpdate = %u",AcqState, h->pStatus->FecStatus,h->pAcquireParam->LockUpdate));
                if (((AcqState>=BADS_Acq_Acquiring)   && h->pStatus->FecStatus  && !h->pAcquireParam->LockUpdate) ||
                    ((AcqState==BADS_Acq_HasAcquired) && !h->pStatus->FecStatus && h->pAcquireParam->LockUpdate))
                {
                    h->pAcquireParam->LockUpdate = (h->pStatus->FecStatus != 0); /* remember the lock status we are reporting */
                    (hImplDev->pCallback[i][BADS_Callback_eLockChange])(hImplDev->pCallbackParam[i][BADS_Callback_eLockChange] );
                }
        }

        if (h->pAcquireParam->NexusStatusMode & BADS_NexusStatusMode_EnableStatusForNexus)
        {
            BADS_7125_P_Get_Status(hChnDev);
            h->pAcquireParam->NexusStatusMode &= (~BADS_NexusStatusMode_EnableStatusForNexus);
        }
        if (h->pAcquireParam->AcquireStartMode & BADS_AcquireStartMode_ResetStatus)
        {
            BDBG_MSG(("call reset"));
            BADS_7125_ResetStatus(hChnDev);
            h->pAcquireParam->AcquireStartMode &= (~BADS_AcquireStartMode_ResetStatus);
        }

        if (h->pAcquireParam->AcquireStartMode & BADS_AcquireStartMode_Acquire)
        {
            h->pAcquireParam->AcquireStartMode &= ~BADS_AcquireStartMode_Acquire;
            h->pStatus->FecStatus = 0;
            h->pStatus->ReAcquireCount = 0;
            BDBG_MSG(("call1 BADS_7125_ProcessInterruptEvent chn %u",i));
            BKNI_SetEvent(hImplDev->hInterruptEvent);
            return BERR_SUCCESS;
        }
        else
        {
            if ((h->pAcquireParam->AutoAcquireMode == 1) &&
                ((AcqState==BADS_Acq_HasAcquired) || (AcqState==BADS_Acq_ReAcquiring)))
            {   /* don't auto-acquire until acquired to begin with */
                if ( h->pStatus->FecStatus == 0)
                {
                    BDBG_MSG(("REACQUIRE: call2 BADS_7125_ProcessInterruptEvent chn %u",i));
                    BKNI_SetEvent(hImplDev->hInterruptEvent);
                    h->pAcquireParam->AcqState = BADS_Acq_ReAcquiring;
                }
            }
            if ( h->pStatus->FecStatus == 1)
            {
                BDBG_MSG(("chn %u HasAcquired",i));
                AcqState=h->pAcquireParam->AcqState=BADS_Acq_HasAcquired;
            }
        }
    }
    #if 0
    if (AcqState!=BADS_Acq_Acquiring)
    {
        BTMR_StopTimer_isr(hImplDev->hTimer);
        if (AcqState==BADS_Acq_HasAcquired)
        {   /* poll quarter second for loss of lock */
            BDBG_MSG(("Start timer for loss of lock"));
            BTMR_StartTimer_isr(hImplDev->hTimer, 250000);
        }
    }
    #endif
    BDBG_LEAVE(BADS_7125_P_TimerFunc);
    return retCode;
}

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BADS_7125_Open(
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
    BADS_7125_Handle hImplDev = NULL;

    BDBG_ENTER(BADS_7125_Open);

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


    hImplDev = (BADS_7125_Handle) BKNI_Malloc( sizeof( BADS_P_7125_Handle ) );
    if( hImplDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BADS_Open: BKNI_malloc() failed, impl"));
        goto done;
    }

    BKNI_Memset( hImplDev, 0x00, sizeof( BADS_P_7125_Handle ) );
    hImplDev->magicId = DEV_MAGIC_ID;
    hImplDev->hChip = hChip;
    hImplDev->hRegister = hRegister;
    hImplDev->hInterrupt = hInterrupt;
    hImplDev->mxChnNo = MX_ADS_CHANNELS;

    #if BADS_7125_P_USE_TIMER
    {
        /* Create timer for status lock check */
        BTMR_Settings sTimerSettings;
        BTMR_GetDefaultTimerSettings(&sTimerSettings);
        sTimerSettings.type = BTMR_Type_ePeriodic;
        sTimerSettings.cb_isr = (BTMR_CallbackFunc)BADS_7125_P_TimerFunc;
        sTimerSettings.pParm1 = (void*)hImplDev;
        sTimerSettings.parm2 = 0;
        sTimerSettings.exclusive = false;

        retCode = BTMR_CreateTimer (hDev->settings.hTmr, &hImplDev->hTimer, &sTimerSettings);
        if ( retCode != BERR_SUCCESS )
        {
            BDBG_ERR(("BADS_Open: Create Timer Failed"));
            retCode = BERR_TRACE(retCode);
            goto done;
        }
    }
    #else
    hImplDev->hTimer = NULL;
    #endif
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
    BDBG_LEAVE(BADS_7125_Open);
    return( retCode );
}

BERR_Code BADS_7125_Close(
    BADS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;

    BDBG_ENTER(BADS_7125_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    hImplDev = (BADS_7125_Handle) hDev->pImpl;

    if (hImplDev->hTimer)
    {
        BTMR_StopTimer(hImplDev->hTimer);
        BTMR_DestroyTimer(hImplDev->hTimer);
    }
    BKNI_DestroyEvent(hImplDev->hInterruptEvent);
    BKNI_Free( (void *)hDev->pImpl );
    hDev->magicId = 0x00;       /* clear it to catch improper use */
    BKNI_Free( (void *) hDev );


    BDBG_LEAVE(BADS_7125_Close);
    return( retCode );
}

BERR_Code BADS_7125_Init(
    BADS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;
    uint32_t chipVer;

    BDBG_ENTER(BADS_7125_Init);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    chipVer = BREG_Read32(hImplDev->hRegister, BCHP_SUN_TOP_CTRL_PROD_REVISION);
    hImplDev->verInfo.chipId = chipVer >> 16;
    hImplDev->verInfo.familyId = chipVer >> 16;
    hImplDev->verInfo.majVer = (chipVer >> 4) & 0xF;
    hImplDev->verInfo.minVer = chipVer & 0xF;
    BDBG_LEAVE(BADS_7125_Init);
    return( retCode );
}

BERR_Code BADS_7125_GetVersion(
    BADS_Handle hDev,                   /* [in] Device handle */
    BADS_Version *pVersion              /* [out] Returns version */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;

    BDBG_ENTER(BADS_7125_GetVersion);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    *pVersion = hImplDev->verInfo;      /* use saved data */

    BDBG_LEAVE(BADS_7125_GetVersion);
    return( retCode );
}

BERR_Code BADS_7125_GetTotalChannels(
    BADS_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;

    BDBG_ENTER(BADS_7125_GetTotalChannels);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    *totalChannels = hImplDev->mxChnNo; /* use saved data */

    BDBG_LEAVE(BADS_7125_GetTotalChannels);
    return( retCode );
}

BERR_Code BADS_7125_GetChannelDefaultSettings(
    BADS_Handle hDev,                       /* [in] Device handle */
    unsigned int channelNo,                 /* [in] Channel number to default setting for */
    BADS_ChannelSettings *pChnDefSettings   /* [out] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;

    BDBG_ENTER(BADS_7125_GetChannelDefaultSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    BDBG_MSG(("%s(%u), hImplDev->mxChnNo = %u",__FUNCTION__,channelNo, hImplDev->mxChnNo));
    if( channelNo < hImplDev->mxChnNo )
    {
        pChnDefSettings->ifFreq = 0;    /*The if freq for 7125 is set to 0 as the internal tuner does not spit out seperate if frequency. */
        pChnDefSettings->autoAcquire = true;
        pChnDefSettings->fastAcquire = false;
    }
    else
    {
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_LEAVE(BADS_7125_GetChannelDefaultSettings);
    return( retCode );
}

BERR_Code BADS_7125_OpenChannel(
    BADS_Handle hDev,                                   /* [in] Device handle */
    BADS_ChannelHandle *phChn,                          /* [out] Returns channel handle */
    unsigned int channelNo,                             /* [in] Channel number to open */
    const struct BADS_ChannelSettings *pChnDefSettings  /* [in] Channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;
    BADS_7125_ChannelHandle hImplChnDev = NULL;
    BADS_ChannelHandle hChnDev;
    uint32_t BufSrc;

    BDBG_ENTER(BADS_7125_OpenChannel);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    hChnDev = NULL;
    if( channelNo < hImplDev->mxChnNo )
    {
        if( hImplDev->hAdsChn[channelNo] == NULL )
        {
            BDBG_MSG(("BADS_7125_OpenChannel"));
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

            hImplChnDev = (BADS_7125_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_7125_ChannelHandle ) );
            if( hImplChnDev == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed, impl"));
                goto done;
            }

            hImplChnDev->chnNo = channelNo;
            hImplChnDev->hRegister = hImplDev->hRegister;

            hImplChnDev->pAcquireParam = (BADS_7125_P_AcquireParams_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(BADS_7125_P_AcquireParams_t), 0, 0 );
            BMEM_ConvertAddressToOffset(hDev->settings.hHeap, hImplChnDev->pAcquireParam, &BufSrc );
            BKNI_Memset(hImplChnDev->pAcquireParam, 0x00, sizeof( BADS_7125_P_AcquireParams_t ) );

            hImplChnDev->pStatus = (BADS_7125_P_Status_t *)BMEM_AllocAligned(hDev->settings.hHeap, sizeof(BADS_7125_P_Status_t), 0, 0 );
            BMEM_ConvertAddressToOffset(hDev->settings.hHeap, hImplChnDev->pStatus, &BufSrc );
            BKNI_Memset( hImplChnDev->pStatus, 0x00, sizeof( BADS_7125_P_Status_t ) );
            hImplChnDev->pAcquireParam->StatusStartAddress = BufSrc;

            if (pChnDefSettings != NULL)
            {
                hImplChnDev->settings.ifFreq = pChnDefSettings->ifFreq;
                hImplChnDev->pAcquireParam->AutoAcquireMode = hImplChnDev->settings.autoAcquire = pChnDefSettings->autoAcquire;
                hImplChnDev->pAcquireParam->fastAcquire = hImplChnDev->settings.fastAcquire = pChnDefSettings->fastAcquire;
            }
            else
            {
                hImplChnDev->pAcquireParam->AutoAcquireMode = hImplChnDev->settings.autoAcquire = 0;
                hImplChnDev->pAcquireParam->fastAcquire = hImplChnDev->settings.fastAcquire = 0;
                hImplChnDev->settings.ifFreq = 0;
            }
            hImplDev->hAdsChn[channelNo] = hChnDev;
            hChnDev->pImpl = hImplChnDev;

            *phChn = hChnDev;

            /*BADS_7125_P_Init_ADS(hChnDev);*/

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
        *phChn = NULL;
    }
    BDBG_LEAVE(BADS_7125_OpenChannel);
    return( retCode );
}

BERR_Code BADS_7125_CloseChannel(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;
    BADS_7125_ChannelHandle hImplChnDev;
    BADS_Handle hAds;
    unsigned int chnNo;

    BDBG_ENTER(BADS_7125_CloseChannel);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_7125_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    hAds = hChn->hAds;
    hImplDev = (BADS_7125_Handle) hAds->pImpl;
    BDBG_ASSERT( hImplDev );


    chnNo = hImplChnDev->chnNo;
    hChn->magicId = 0x00;       /* clear it to catch inproper use */
    BMEM_Free(hAds->settings.hHeap ,(void *)((BADS_P_7125_ChannelHandle *)(hChn->pImpl))->pAcquireParam);
    BMEM_Free(hAds->settings.hHeap,(void *)((BADS_P_7125_ChannelHandle *)(hChn->pImpl))->pStatus);
    BKNI_Free( hChn->pImpl );
    BKNI_Free( hChn );
    hImplDev->hAdsChn[chnNo] = NULL;

    BDBG_LEAVE(BADS_7125_CloseChannel);
    return( retCode );
}

BERR_Code BADS_7125_GetDevice(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Handle *phDev                  /* [out] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_7125_GetDevice);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *phDev = hChn->hAds;

    BDBG_LEAVE(BADS_7125_GetDevice);
    return( retCode );
}

BERR_Code BADS_7125_Acquire(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_InbandParam *ibParam           /* [in] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_ChannelHandle hImplChnDev;
    BADS_Handle hDev;
    BADS_7125_Handle hImplDev;

    BDBG_ENTER(BADS_7125_Acquire);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_7125_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    hDev = (BADS_Handle)(hChn->hAds);
    BDBG_ASSERT( hDev );
    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    BDBG_MSG(("BADS_7125_Acquire"));
    if (ibParam->symbolRate == 1)
    {
        /* a special symbolRate value to deactivate the timer when handing over control to
        DOCSIS */
        if (hImplDev->hTimer)
        {
            BDBG_MSG(("stop timer"));
            BTMR_StopTimer(hImplDev->hTimer);
        }
    }
    else if (ibParam->symbolRate == 2)
    {
        /* a special symbolRate value to deactivate the timer when taking over control from
        DOCSIS */
        if (hImplDev->hTimer)
        {
            BTMR_StopTimer(hImplDev->hTimer);
            BDBG_MSG(("start timer"));
            BTMR_StartTimer(hImplDev->hTimer, 25000);
        }
    }
    else
    {
        hImplChnDev->pAcquireParam->ModulationType  = ibParam->modType;
        hImplChnDev->pAcquireParam->SymbolRate      = ibParam->symbolRate;
        hImplChnDev->pAcquireParam->InvertSpectrum  = ibParam->invertSpectrum;
        hImplChnDev->pAcquireParam->spectrum        = ibParam->spectrum;
        hImplChnDev->pAcquireParam->EnableDpm       = ibParam->enableDpm;
        BKNI_Memset(hImplChnDev->pStatus, 0x00, sizeof( BADS_7125_P_Status_t ) ); /* Clear status structure upon new TuneAcquire command from host */
        BADS_7125_P_Acquire(hChn);
        hImplChnDev->pAcquireParam->AcqState        = BADS_Acq_Acquiring;
        hImplChnDev->pAcquireParam->LockUpdate      = false; /* first status update when we lock */

        if (hImplDev->hTimer)
        {
            BTMR_StopTimer(hImplDev->hTimer);
            BTMR_StartTimer(hImplDev->hTimer, 25000);   /* the timer is in Micro second */
        }
    }

    BDBG_LEAVE(BADS_7125_Acquire);
    return( retCode );
}

BERR_Code BADS_7125_GetStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Status *pStatus                /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_ChannelHandle hImplChnDev;

    BDBG_ENTER(BADS_7125_GetStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_7125_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    BADS_7125_P_Get_Status(hChn);

    pStatus->isPowerSaverEnabled = hImplChnDev->pStatus->PowerSave;                     /* Enable=1, Disable=0 */
    pStatus->modType = hImplChnDev->pStatus->ModulationType;                            /* Modulation type */
    pStatus->ifFreq = hImplChnDev->pStatus->IfFrequency;                                /* in Hertz, IF freq. */
    pStatus->symbolRate = hImplChnDev->pStatus->SymbolRate;                             /* in Baud */
    pStatus->symbolRateError = (int32_t) hImplChnDev->pStatus->TimingFrequencyError;    /* in Baud */
    pStatus->isFecLock = (hImplChnDev->pStatus->FecStatus==BADS_FECStatus_eLocked)?1:0; /* lock=1, unlock=0 */
    pStatus->isQamLock = (hImplChnDev->pStatus->QamStatus==BADS_QAMStatus_eLocked)?1:0; /* lock=1, unlock=0 */
    pStatus->correctedCount = hImplChnDev->pStatus->FEC_Corr_RS_Blocks;                 /* reset on every read */
    pStatus->uncorrectedCount = hImplChnDev->pStatus->FEC_UCorr_RS_Blocks;              /* reset on every read */
    pStatus->accCorrectedCount = hImplChnDev->pStatus->DS_CBERC2;                       /* reset on call to BADS_7125_P_Reset_Status() */
    pStatus->accUncorrectedCount = hImplChnDev->pStatus->DS_UERC2;                      /* reset on call to BADS_7125_P_Reset_Status() */
    pStatus->snrEstimate = hImplChnDev->pStatus->SNR;                                   /* in 1/256 dB */
    pStatus->agcIntLevel = 0;                                                           /* returns 0, moved to tuner*/
    pStatus->agcExtLevel = 0;                                                           /* returns 0, moved to tuner*/
    pStatus->carrierFreqOffset = hImplChnDev->pStatus->CarrierFrequencyError*1000;      /* in 1/1000 Hz */
    pStatus->carrierPhaseOffset = hImplChnDev->pStatus->CarrierPhaseError*1000;         /* in 1/1000 Hz */
    pStatus->rxSymbolRate = hImplChnDev->pStatus->SymbolRate;                           /* in Baud, received symbol rate */
    pStatus->interleaveDepth = (uint16_t)hImplChnDev->pStatus->InterleaverDepth;        /* use in Docsis */
    pStatus->goodRsBlockCount = hImplChnDev->pStatus->FEC_Clean_RS_Blocks;
    hImplChnDev->pStatus->FEC_Clean_RS_Blocks = 0;          /* reset on every read */
    pStatus->correctedBits = hImplChnDev->pStatus->DS_CERC2;
    pStatus->berRawCount = 0;                               /* reset on every read */
    pStatus->dsChannelPower = 0;                            /* in 10s of dBmV unit; OCAP DPM support for video channels */
    pStatus->mainTap = 0;                                   /* Channel main tap coefficient */
    pStatus->equalizerGain = 0;                             /* Channel equalizer gain value in dBm */
    /* OCAP required postRsBER for all DS channels. postRsBER and elapsedTimeSec will be reset on every channel change*/
    /* fpostRsBER normally between xxxe-6 to xxxe-12 float value, to send this small float number over rMagnum to host, we convert it to uint32 using the formula*/
    pStatus->postRsBER = 0;                       /* returns 0, NO FLOAT in 7125*/
                                        /* For Annex A  postRsBER = (11.224*uncorrectedCount)/(correctedCount+uncorrectedCount+cleanCount)*187*8)*/
                                        /* For Annex B  postRsBER = (11.224*uncorrectedCount)/(correctedCount+uncorrectedCount+cleanCount)*122*7)*/

    pStatus->elapsedTimeSec = 0;                              /* returns 0, not needed*/
    pStatus->isSpectrumInverted = hImplChnDev->pStatus->IsSpectrumInverted;             /* If 1, Spectrum is inverted relative to the AP bootup. */

    pStatus->cleanCount = hImplChnDev->pStatus->DS_NBERC2;          /* reset on every call to BADS_7125_P_Reset_Status() */
    pStatus->agcAGFLevel = hImplChnDev->pStatus->AGF_Level;         /* in 1/10 percent */
    pStatus->agcAGF2Level = hImplChnDev->pStatus->AGF2_Level;       /* in 1/10 percent */

    BDBG_LEAVE(BADS_7125_GetStatus);
    return( retCode );
}

BERR_Code BADS_7125_GetLockStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_LockStatus *pLockStatus         /* [out] Returns lock status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_ChannelHandle hImplChnDev;

    BDBG_ENTER(BADS_7125_GetLockStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_7125_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    BADS_7125_P_Get_LockStatus(hChn);
    *pLockStatus = hImplChnDev->pStatus->FecStatus;

    BDBG_LEAVE(BADS_7125_GetLockStatus);
    return( retCode );

}

BERR_Code BADS_7125_GetSoftDecision(
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
    BADS_7125_Handle hImplDev;
    BADS_Handle hDev;
    BADS_7125_ChannelHandle hImplChnDev;
    unsigned int chn;
    BDBG_ENTER(BADS_7125_GetSoftDecision);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hImplChnDev = (BADS_7125_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    hDev = (BADS_Handle)(hChn->hAds);
    BDBG_ASSERT( hDev );
    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    chn = hImplChnDev->chnNo;

    ReadReg = BADS_7125_P_Read32(hImplDev->hRegister, BCHP_DS_EQ_0_SOFT, chn);

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

    BDBG_LEAVE(BADS_7125_GetSoftDecision);
    return( retCode );
}


BERR_Code BADS_7125_EnablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_ChannelHandle hImplChnDev;


    BDBG_ENTER(BADS_7125_EnablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_7125_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    BADS_7125_P_PowerDn(hChn);

    BDBG_LEAVE(BADS_7125_EnablePowerSaver);
    return( retCode );
}

BERR_Code BADS_7125_DisablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_ChannelHandle hImplChnDev;


    BDBG_ENTER(BADS_7125_EnablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_7125_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    BADS_7125_P_PowerUp(hChn);

    BDBG_LEAVE(BADS_7125_EnablePowerSaver);
    return( retCode );
}

BERR_Code BADS_7125_ProcessNotification(
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

BERR_Code BADS_7125_InstallCallback(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Callback callbackType,         /* [in] Type of callback */
    BADS_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Handle hDev;
    BADS_7125_Handle hImplDev;
    BADS_7125_ChannelHandle hImplChnDev;
    unsigned int chn;

    BDBG_ENTER(BADS_7125_InstallCallback);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    hImplChnDev = (BADS_7125_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    hDev = (BADS_Handle)(hChn->hAds);
    BDBG_ASSERT( hDev );
    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    chn = hImplChnDev->chnNo;


    switch( callbackType )
    {
        case BADS_Callback_eLockChange:
        case BADS_Callback_eUpdateGain:
            hImplDev->pCallback[chn][callbackType] = pCallback;
            hImplDev->pCallbackParam[chn][callbackType] = pParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BDBG_LEAVE(BADS_7125_InstallCallback);
    return( retCode );
}

BERR_Code BADS_7125_SetDaisyChain(
    BADS_Handle hDev,       /* [in] Returns handle */
    bool enableDaisyChain   /* [in] Eanble/disable daisy chain. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;


    BDBG_ENTER(BADS_7125_SetDaisyChain);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
#if 0 /*MCW */
    if (enableDaisyChain)
        BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TUNER_REF_DB0_WDATA_01, DS_PD, 0);
    else
        BADS_7125_P_WriteField(hImplDev->hRegister, DS_0_TUNER_REF_DB0_WDATA_01, DS_PD, 1);
#else
    BDBG_ERR(("%s() not supported",__FUNCTION__));
#endif
    hImplDev->isDaisyChain = enableDaisyChain;

    BDBG_LEAVE(BADS_7125_SetDaisyChain);
    return( retCode );
}

BERR_Code BADS_7125_GetDaisyChain(
    BADS_Handle hDev,           /* [in] Returns handle */
    bool *isEnableDaisyChain    /* [in] Eanble/disable daisy chain. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;

    BDBG_ENTER(BADS_7125_GetDaisyChain);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    *isEnableDaisyChain = hImplDev->isDaisyChain;

    BDBG_LEAVE(BADS_7125_GetDaisyChain);
    return( retCode );
}

BERR_Code BADS_7125_GetInterruptEventHandle(
   BADS_Handle hDev,            /* [in] BADS handle */
   BKNI_EventHandle *phEvent /* [out] event handle */
)
{

    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;

    BDBG_ENTER(BADS_7125_GetInterruptEventHandle);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    *phEvent = hImplDev->hInterruptEvent;


    BDBG_LEAVE(BADS_7125_GetInterruptEventHandle);
    return( retCode );



}

BERR_Code BADS_7125_ProcessInterruptEvent(
    BADS_Handle hDev           /* [in] Returns handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_7125_Handle hImplDev;
    BADS_ChannelHandle hChnDev;
    BADS_7125_ChannelHandle h;
    unsigned i;

    BDBG_ENTER(BADS_7125_GetDaisyChain);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_7125_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    BDBG_MSG(("BADS_7125_ProcessInterruptEvent"));
    BKNI_ResetEvent(hImplDev->hInterruptEvent);
    for (i=0;i<MX_ADS_CHANNELS;i++)
    {
        hChnDev = (BADS_ChannelHandle)(hImplDev->hAdsChn[i]);
        BDBG_ASSERT( hChnDev );
        h = (BADS_7125_ChannelHandle)(hChnDev->pImpl);
        BDBG_ASSERT( h );

        if ((h->pAcquireParam->AcqState==BADS_Acq_ReAcquiring) &&
            (h->pStatus->FecStatus == 0))
        {
            if (hImplDev->hTimer)
            {
                BTMR_StopTimer(hImplDev->hTimer);
            }
            BDBG_MSG(("BADS_7125_ProcessInterruptEvent calling BADS_7125_P_Acquire(%u)",i));
            BADS_7125_P_Acquire(hChnDev);
            if (hImplDev->hTimer)
            {
                BTMR_StartTimer(hImplDev->hTimer, 25000);   /* the timer is in Micro second */
            }
        }
    }
    BDBG_LEAVE(BADS_7125_ProcessInterruptEvent);
    return( retCode );
}
