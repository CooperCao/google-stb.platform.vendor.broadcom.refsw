/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/




#include "bdcm_device.h"

BDBG_MODULE(bdcm_aob);

/***************************************************************************
Summary:
    This structure represents a DOCSIS out-of-band channel.
    In general, there would be only one out of band channel per
    DOCSIS device.
****************************************************************************/
struct BDCM_AobChannel
{
    BDCM_DeviceHandle hDevice;
    BRPC_DevId devId;
    BDCM_AobCallbackFunc pCallback[BDCM_AobCallback_eLast];
    void *pCallbackParam[BDCM_AobCallback_eLast];
    bool enableFEC;                                                /* enable OOB FEC*/
    BDCM_AobSpectrumMode spectrum;                              /* current specturm setting*/
    bool isLock;                                                   /* current lock status */
    BKNI_MutexHandle mutex;                                        /* mutex to protect lock status*/
};

BERR_Code BDCM_Aob_GetChannelDefaultSettings(
    BDCM_AobSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(pSettings);
    BDBG_ENTER(BDCM_Aob_GetChannelDefaultSettings);
    BKNI_Memset(pSettings,0,sizeof(*pSettings));
    pSettings->xtalFreq = BDCM_AOB_XTALFREQ;
    pSettings->ifFreq = BDCM_AOB_IFFREQ;
    pSettings->spectrum = BDCM_AobSpectrumMode_eNoInverted;
    pSettings->enableFEC = false;
    BDBG_LEAVE(BDCM_Aob_GetChannelDefaultSettings);
    return retCode;
}

BDCM_AobChannelHandle BDCM_Aob_OpenChannel(
    void *handle,
    const BDCM_AobSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDCM_AobChannelHandle hChannel=NULL;
    BDCM_DeviceHandle hDevice = (BDCM_DeviceHandle)(handle);
    BRPC_Param_AOB_Open	Param;
    BERR_Code retVal;
    BDBG_ASSERT(hDevice);

    BDBG_ENTER(BDCM_Aob_OpenChannel);

    if(hDevice->hAob)
    {
       BDBG_WRN(("%s: OOB channel already opened %p",__FUNCTION__,(void*)hDevice->hAob));
       return NULL;
    }
    /* Alloc memory from the system heap */
    hChannel = (BDCM_AobChannelHandle)BKNI_Malloc(sizeof(struct BDCM_AobChannel));
    if(hChannel == NULL)
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("%s: BKNI_malloc() failed",__FUNCTION__));
        goto done;
    }
    BKNI_Memset(hChannel, 0x00, sizeof(struct BDCM_AobChannel));

    hChannel->hDevice = hDevice;
    hChannel->devId = BRPC_DevId_DOCSIS_OB0;
    hChannel->enableFEC = pSettings->enableFEC;
    hChannel->spectrum = pSettings->spectrum;

    Param.devId = hChannel->devId;
    Param.enableFEC = hChannel->enableFEC;

    CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc,
                                       BRPC_ProcId_AOB_Open,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
    CHK_RETCODE(retCode, retVal);

    CHK_RETCODE(retCode, BDCM_Aob_SetChannelSpectrum(hChannel, hChannel->spectrum));

    CHK_RETCODE(retCode, BKNI_CreateMutex(&hChannel->mutex));
done:
    if( retCode != BERR_SUCCESS )
    {
        if( hChannel != NULL )
		{
            BKNI_Free( hChannel );
            if (hChannel->mutex)
            {
                BKNI_DestroyMutex(hChannel->mutex);
            }
            hChannel = NULL;
        }
    }

    BDBG_LEAVE(BDCM_Aob_OpenChannel);
    return hChannel;
}

BERR_Code BDCM_Aob_CloseChannel(BDCM_AobChannelHandle hChannel)
{
	BERR_Code retCode = BERR_SUCCESS;
	BRPC_Param_AOB_Close Param;
	BERR_Code retVal;

    BDBG_ENTER(BDCM_Aob_CloseChannel);
	BDBG_ASSERT( hChannel );
    BDBG_MSG(("%s OOB channel devID %d",__FUNCTION__,hChannel->devId));
	Param.devId = hChannel->devId;
	CHK_RETCODE(retCode,BRPC_CallProc(hChannel->hDevice->hRpc,
                                      BRPC_ProcId_AOB_Close,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      NULL, 0, &retVal));
	CHK_RETCODE(retCode, retVal);
	BKNI_DestroyMutex(hChannel->mutex);
    hChannel->hDevice->hAob = NULL;
    BKNI_Free((void *) hChannel);
done:
	BDBG_LEAVE(BDCM_Aob_CloseChannel);
	return retCode;
}


BERR_Code BDCM_Aob_AcquireChannel(
    BDCM_AobChannelHandle hChannel,
    BDCM_AobModulationType modType,
    uint32_t symbolRate
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_AOB_Acquire Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Aob_AcquireChannel);
    BDBG_ASSERT(hChannel);

    BDBG_MSG(("%s: modType=%d, symbolRate=%d", __FUNCTION__, modType, symbolRate));
    BKNI_AcquireMutex(hChannel->mutex);
    hChannel->isLock = false;
    BKNI_ReleaseMutex(hChannel->mutex);

    Param.devId = hChannel->devId;
    Param.modType = modType;
    Param.symbolRate = symbolRate;
    CHK_RETCODE(retCode,BRPC_CallProc(hChannel->hDevice->hRpc,
                                      BRPC_ProcId_AOB_Acquire,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      NULL, 0, &retVal));
    CHK_RETCODE(retCode, retVal);
done:
    BDBG_LEAVE(BDCM_Aob_AcquireChannel);
    return retCode;
}

BERR_Code BDCM_Aob_GetChannelStatus(
    BDCM_AobChannelHandle hChannel,
    BDCM_AobStatus *pStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_AOB_GetStatus outParam;
    BRPC_Param_XXX_Get Param;
    BERR_Code retVal;

    BDBG_ENTER(BAOB_Aob_GetChannelStatus);
    BDBG_ASSERT(hChannel);

    pStatus->isFecLock = pStatus->isQamLock = false;
    Param.devId = hChannel->devId;
    BDBG_MSG(("Param.devId %d",Param.devId));
    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_AOB_GetStatus,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       (uint32_t *)&outParam,
                                       sizeInLong(outParam),
                                       &retVal) );
    CHK_RETCODE( retCode, retVal);
    pStatus->modType = outParam.modType;
    pStatus->symbolRate = outParam.symbolRate;
    pStatus->ifFreq = outParam.ifFreq;
    pStatus->loFreq = outParam.loFreq;
    pStatus->sysXtalFreq = outParam.sysXtalFreq;
    pStatus->isFecLock = outParam.isFecLock;
    pStatus->isQamLock = outParam.isQamLock;
    pStatus->snrEstimate = outParam.snrEstimate;
    pStatus->agcIntLevel = outParam.agcIntLevel;
    pStatus->agcExtLevel = outParam.agcExtLevel;
    pStatus->carrierFreqOffset = outParam.carrierFreqOffset;
    pStatus->carrierPhaseOffset = outParam.carrierPhaseOffset;
    pStatus->uncorrectedCount = outParam.uncorrectedCount;
    pStatus->correctedCount = outParam.correctedCount;
    pStatus->berErrorCount = outParam.berErrorCount;
    pStatus->fdcChannelPower= outParam.fdcChannelPower;
    BDBG_MSG(("%s outParam.modType:%d  outParam.ifFreq:%d "
              "outParam.symbolRate:%d outParam.fdcChannelPower:%d"
              "outParam.snrEstimate:%d",__FUNCTION__,
              outParam.modType,outParam.ifFreq, outParam.symbolRate,
              outParam.fdcChannelPower, outParam.snrEstimate));
done:
    BDBG_LEAVE(BAOB_Aob_GetChannelStatus);
    return retCode;
}

BERR_Code BDCM_Aob_GetChannelLockStatus(
    BDCM_AobChannelHandle hChannel,
    bool *isLock
    )
{
    BERR_Code retCode = BERR_SUCCESS;
#ifdef USE_RPC_GET_LOCK_STATUS
    BRPC_Param_AOB_GetLockStatus outParam;
    BRPC_Param_XXX_Get Param;
    BERR_Code retVal;
#endif
    BDBG_ENTER(BDCM_Aob_GetChannelLockStatus);
    BDBG_ASSERT(hChannel);
    BDBG_ASSERT(isLock);

    *isLock = false;
#ifdef USE_RPC_GET_LOCK_STATUS
    Param.devId = hChannel->devId;
    CHK_RETCODE(retCode,BRPC_CallProc(hChannel->hDevice->hRpc,
                                      BRPC_ProcId_AOB_GetLockStatus,
                                      (const uint32_t *)&Param,
                                      sizeInLong(Param),
                                      (uint32_t *)&outParam,
                                      sizeInLong(outParam),
                                      &retVal));
    CHK_RETCODE(retCode, retVal);
    if (hChannel->enableFEC)
    {
        *isLock = (outParam.isQamLock == true) && (outParam.isFecLock == true);
    }
    else
    {
        *isLock = outParam.isQamLock;
    }
#else
    BKNI_AcquireMutex(hChannel->mutex);
    *isLock = hChannel->isLock;
    BKNI_ReleaseMutex(hChannel->mutex);
#endif

#ifdef USE_RPC_GET_LOCK_STATUS
done:
#endif
	BDBG_LEAVE(BDCM_Aob_GetChannelLockStatus);
    return retCode;
}

BERR_Code BDCM_Aob_EnableChannelPowerSaver(BDCM_AobChannelHandle hChannel)
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_AOB_EnablePowerSaver Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Aob_EnableChannelPowerSaver);
    BDBG_ASSERT(hChannel);

    Param.devId = hChannel->devId;
    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_AOB_EnablePowerSaver,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0,
                                       &retVal));
    CHK_RETCODE(retCode, retVal);
done:
    BDBG_LEAVE(BDCM_Aob_EnableChannelPowerSaver);
    return( retCode );
}

BERR_Code BDCM_Aob_GetChannelSpectrum(
    BDCM_AobChannelHandle hChannel,
    BDCM_AobSpectrumMode *spectrum
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BDCM_Aob_GetChannelSpectrum);
    BDBG_ASSERT(hChannel);
    BDBG_ASSERT(spectrum);

    *spectrum = hChannel->spectrum;

    BDBG_LEAVE(BDCM_Aob_GetChannelSpectrum);
    return retCode;
}


BERR_Code BDCM_Aob_SetChannelSpectrum(
    BDCM_AobChannelHandle hChannel,
    BDCM_AobSpectrumMode spectrum
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_AOB_SetSpectrum Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Aob_SetChannelSpectrum);
    BDBG_ASSERT(hChannel);

    Param.devId = hChannel->devId;
    Param.spectrum = spectrum;
    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_AOB_SetSpectrum,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
    CHK_RETCODE(retCode, retVal);
    hChannel->spectrum = spectrum;
done:
    BDBG_LEAVE(BDCM_Aob_SetChannelSpectrum);
    return retCode;
}


BERR_Code BDCM_Aob_ProcessChannelNotification(
    BDCM_AobChannelHandle hChannel,
    unsigned int event
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Notification_Event event_code;
    bool isFECLock, isQAMLock;

    BDBG_ENTER(BDCM_Aob_ProcessChannelNotification);
    BDBG_ASSERT(hChannel);

    event_code = event>>16;

    switch (event_code)
    {
        case BRPC_Notification_Event_LockStatusChanged:
            isQAMLock = BRPC_GET_AOB_QAM_LOCKSTATUS(event);
            isFECLock = BRPC_GET_AOB_FEC_LOCKSTATUS(event);
            BKNI_AcquireMutex(hChannel->mutex);
			if (hChannel->enableFEC)
            {
                hChannel->isLock = isQAMLock && isFECLock;
            }
            else
            {
                hChannel->isLock = isQAMLock;
            }
            BKNI_ReleaseMutex(hChannel->mutex);
            if( hChannel->pCallback[BDCM_AobCallback_eLockChange] != NULL )
            {
                (hChannel->pCallback[BDCM_AobCallback_eLockChange])(hChannel->pCallbackParam[BDCM_AobCallback_eLockChange] );
            }
            BDBG_MSG(("%s: AOB LockStatusChanged from DOCSIS: isLock? %d",__FUNCTION__,hChannel->isLock));
            break;
        default:
            BDBG_WRN(("%s: unknown AOB event code from DOCSIS %x",__FUNCTION__,event_code));
            break;
    }

    BDBG_LEAVE(BDCM_Aob_ProcessChannelNotification);
    return retCode;
}

BERR_Code BDCM_Aob_InstallChannelCallback(
    BDCM_AobChannelHandle hChannel,
    BDCM_AobCallback callbackType,
    BDCM_AobCallbackFunc pCallback,
    void *pParam
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BDCM_Aob_InstallChannelCallback);
    BDBG_ASSERT(hChannel);

    switch( callbackType )
    {
        case BDCM_AobCallback_eLockChange:
            hChannel->pCallback[callbackType] = pCallback;
            hChannel->pCallbackParam[callbackType] = pParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BDBG_LEAVE(BDCM_Aob_InstallChannelCallback);
    return retCode;
}
