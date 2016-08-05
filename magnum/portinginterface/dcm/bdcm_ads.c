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

BDBG_MODULE(bdcm_ads);
/***************************************************************************
Summary:
    This structure represents a DOCSIS downstream channel. It shall
    encapsulate either a DOCSIS data or a QAM down stream channel.
    In general, there would be multiple downstream channels functioning
    as data and/or QAM channels per DOCSIS device.
****************************************************************************/
struct BDCM_AdsChannel
{
    BDCM_DeviceHandle hDevice;
    uint32_t channelNum;
    BRPC_DevId devId;
    BDCM_AdsCallbackFunc pCallback[BDCM_AdsCallback_eLast];
    void *pCallbackParam[BDCM_AdsCallback_eLast];
    unsigned long ifFreq; /* IF Frequency in Hertz */
    bool autoAcquire;     /* does auto-reacuire or not*/
    bool fastAcquire;
    BDCM_AdsLockStatus lockStatus;    /* current lock status */
    BKNI_MutexHandle mutex;              /* mutex to protect lock status*/
    uint32_t accCorrectedCount;          /* Accumulated corrected block count. Reset on every reset status */
    uint32_t accUncorrectedCount;        /* Accumulated un corrected block count. Reset on every reset status */
    uint32_t dsChannelPower;
};


BERR_Code BDCM_Ads_GetChannelDefaultSettings(BDCM_AdsSettings *pSettings)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(pSettings);
    BDBG_ENTER(BDCM_Ads_GetChannelDefaultSettings);
    BKNI_Memset(pSettings,0,sizeof(BDCM_AdsSettings));
    pSettings->ifFreq = BDCM_ADS_IFFREQ;
	pSettings->autoAcquire = false;
	pSettings->fastAcquire = false;
    BDBG_LEAVE(BDCM_Ads_GetChannelDefaultSettings);
    return retCode;
}

BDCM_AdsChannelHandle BDCM_Ads_OpenChannel(
    void  *handle,
    uint32_t channelNum,
    const struct BDCM_AdsSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDCM_DeviceHandle hDevice = (BDCM_DeviceHandle)handle;
    BDCM_AdsChannelHandle hChannel = NULL;
    BRPC_Param_ADS_OpenChannel	Param;
    BERR_Code retVal;

    BDBG_ASSERT(pSettings);
    BDBG_ENTER(BDCM_Ads_OpenChannel);

    if(channelNum >=hDevice->maxChannels || hDevice->hAds[channelNum])
    {
         retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
         BDBG_ERR(("%s either exceeded max channels or channel is already opened",__FUNCTION__));
         BDBG_ERR(("%s channelNum:maxChannels - %d:%d hDevice->hAds[%d]:%ul ",__FUNCTION__,
                   channelNum,hDevice->maxChannels,channelNum,(unsigned int)hDevice->hAds[channelNum]));
         goto done;
    }
    /* Alloc memory from the system heap */
    hChannel = (BDCM_AdsChannelHandle) BKNI_Malloc(sizeof(struct BDCM_AdsChannel));
    if( hChannel == NULL )
	{
  	    retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	    BDBG_ERR(("BDCM_Ads_OpenChannel: BKNI_malloc() failed"));
	    goto done;
	}
	BKNI_Memset(hChannel, 0x00, sizeof(struct BDCM_AdsChannel));


    Param.devId = ((pSettings->minVer <= 0x9) ? BRPC_DevId_DOCSIS_DS0 : BRPC_DevId_ECM_DS0) + channelNum;
    Param.ifFreq = pSettings->ifFreq;
	CHK_RETCODE(retCode, BRPC_CallProc(hDevice->hRpc, BRPC_ProcId_ADS_OpenChannel,
                                       (const uint32_t *)&Param, sizeInLong(Param),
                                       NULL, 0, &retVal));
	CHK_RETCODE( retCode, retVal );

    hChannel->devId = Param.devId;
	hChannel->hDevice = hDevice;
    hChannel->ifFreq = pSettings->ifFreq;
    CHK_RETCODE(retCode, BKNI_CreateMutex(&hChannel->mutex));
    hChannel->channelNum = channelNum;
    hDevice->hAds[channelNum] = hChannel;
done:
	if( retCode != BERR_SUCCESS )
	{
        if( hChannel != NULL )
	    {
	        BKNI_Free( hChannel );
            if(hChannel->mutex)
            {
                BKNI_DestroyMutex(hChannel->mutex);
            }
            hChannel = NULL;
	    }
	}
    else
    {
        BDBG_MSG(("%s: AdsChannel %u devID %u",__FUNCTION__,hChannel->channelNum,hChannel->devId));
    }
    BDBG_LEAVE(BDCM_Ads_OpenChannel);
	return hChannel;
}

BERR_Code BDCM_Ads_CloseChannel(BDCM_AdsChannelHandle hChannel)
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ADS_CloseChannel Param;
    BERR_Code retVal;
    BDCM_DeviceHandle hDevice = hChannel->hDevice;
    unsigned channelNum = hChannel->channelNum;

    BDBG_ENTER(BDCM_Ads_CloseChannel);
    BDBG_ASSERT(hChannel);
    Param.devId = hChannel->devId;
    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_ADS_CloseChannel,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
    CHK_RETCODE(retCode, retVal);
    BDBG_MSG(("%s: Channel %u devID %u",__FUNCTION__,channelNum,hChannel->devId));
    BKNI_DestroyMutex(hChannel->mutex);
    hDevice->hAds[channelNum] = NULL;
    BKNI_Free(hChannel) ;
done:
	BDBG_LEAVE(BDCM_Ads_CloseChannel);
	return( retCode );
}

BERR_Code BDCM_Ads_AcquireChannel(
    BDCM_AdsChannelHandle hChannel,
    BDCM_AdsInbandParam *ibParam
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ADS_Acquire Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Ads_AcquireChannel);
    BDBG_ASSERT(hChannel);
    BDBG_ASSERT(ibParam);

    BKNI_AcquireMutex(hChannel->mutex);
    hChannel->lockStatus = BDCM_AdsLockStatus_eUnlocked;
    BKNI_ReleaseMutex(hChannel->mutex);
    hChannel->accCorrectedCount = 0;
    hChannel->accUncorrectedCount = 0;
    BDBG_MSG(("%s: Channel %u devID %u",__FUNCTION__,hChannel->channelNum,hChannel->devId));
    BDBG_MSG(("%s: modType=%d, symbolRate=%d", __FUNCTION__, ibParam->modType, ibParam->symbolRate));
    Param.devId = hChannel->devId;
    Param.modType = ibParam->modType;
    Param.symbolRate = ibParam->symbolRate;
    Param.autoAcquire = hChannel->autoAcquire;
    Param.fastAcquire = hChannel->fastAcquire;
    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_ADS_Acquire,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
    CHK_RETCODE(retCode, retVal);
done:
	BDBG_LEAVE(BDCM_Ads_AcquireChannel);
	return( retCode );
}

BERR_Code BDCM_Ads_RequestAsyncChannelStatus(
    BDCM_AdsChannelHandle hChannel
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;
    BRPC_Param_ADS_GetDsChannelPower Param;
    BDBG_ENTER(BDCM_Ads_RequestAsyncChannelStatus);
    BDBG_ASSERT(hChannel);
    Param.devId = hChannel->devId;
    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_ADS_GetDsChannelPower,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL,0,&retVal));
    CHK_RETCODE(retCode, retVal);
done:
    BDBG_LEAVE(BDCM_Ads_RequestAsyncChannelStatus);
    return retCode;
}

BERR_Code BDCM_Ads_GetChannelStatus(
    BDCM_AdsChannelHandle hChannel,
    BDCM_AdsStatus *pStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ADS_GetStatus outParam;
    BRPC_Param_XXX_Get Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Ads_GetChannelStatus);
    BDBG_ASSERT(hChannel);
    BDBG_ASSERT(pStatus);

    pStatus->isFecLock = pStatus->isQamLock = false;
    Param.devId = hChannel->devId;

    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_ADS_GetStatus,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       (uint32_t *)&outParam,
                                       sizeInLong(outParam),
                                       &retVal));
    CHK_RETCODE(retCode, retVal);

    hChannel->accCorrectedCount += outParam.correctedCount;
    hChannel->accUncorrectedCount += outParam.uncorrectedCount;

    pStatus->isPowerSaverEnabled = outParam.isPowerSaverEnabled;
    pStatus->modType = outParam.modType;
    pStatus->ifFreq = outParam.ifFreq;
    pStatus->symbolRate = outParam.symbolRate;
    pStatus->isFecLock = outParam.isFecLock;
    pStatus->isQamLock = outParam.isQamLock;
    pStatus->correctedCount = outParam.correctedCount;
    pStatus->uncorrectedCount = outParam.uncorrectedCount;
    pStatus->snrEstimate = outParam.snrEstimate;
    pStatus->agcIntLevel = outParam.agcIntLevel;
    pStatus->agcExtLevel = outParam.agcExtLevel;
    pStatus->carrierFreqOffset = outParam.carrierFreqOffset;
    pStatus->carrierPhaseOffset = outParam.carrierPhaseOffset;
    pStatus->rxSymbolRate = outParam.rxSymbolRate;
    pStatus->interleaveDepth = outParam.interleaveDepth;
    pStatus->goodRsBlockCount = outParam.goodRsBlockCount;
    pStatus->berRawCount = outParam.berRawCount;
    pStatus->mainTap = outParam.mainTap;
    pStatus->equalizerGain = outParam.equalizerGain*100;
    pStatus->postRsBER = outParam.postRsBER;
    pStatus->elapsedTimeSec = outParam.elapsedTimeSec;
    pStatus->isSpectrumInverted = outParam.spectralInversion;
    pStatus->preRsBER = outParam.preRsBER;

    pStatus->accCorrectedCount = hChannel->accCorrectedCount;
    pStatus->accUncorrectedCount = hChannel->accUncorrectedCount;
    pStatus->dsChannelPower = hChannel->dsChannelPower;

done:
    BDBG_LEAVE(BDCM_Ads_GetChannelStatus);
    return retCode;
}

BERR_Code BDCM_Ads_ResetChannelStatus(BDCM_AdsChannelHandle hChannel)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ENTER(BDCM_Ads_ResetChannelStatus);
    BDBG_ASSERT( hChannel );
    hChannel->accCorrectedCount = 0;
    hChannel->accUncorrectedCount = 0;
    BDBG_LEAVE(BDCM_Ads_ResetChannelStatus);
	return retCode;
}

BERR_Code BDCM_Ads_GetChannelLockStatus(
    BDCM_AdsChannelHandle hChannel,
    BDCM_AdsLockStatus *pLockStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
#ifdef USE_RPC_GET_LOCK_STATUS
    BRPC_Param_ADS_GetLockStatus outParam;
    BRPC_Param_XXX_Get Param;
    BERR_Code retVal;
#endif

	BDBG_ENTER(BDCM_Ads_GetChannelLockStatus);
	BDBG_ASSERT(hChannel);
    BDBG_ASSERT( pLockStatus );

#ifndef USE_RPC_GET_LOCK_STATUS
	BKNI_AcquireMutex(hChannel->mutex);
	*pLockStatus = hChannel->lockStatus;
	BKNI_ReleaseMutex(hChannel->mutex);
#else
	*pLockStatus = false;
	Param.devId = hChannel->devId;
	CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_ADS_GetLockStatus,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       (uint32_t *)&outParam,
                                       sizeInLong(outParam),
                                       &retVal));
	CHK_RETCODE(retCode, retVal);
	*pLockStatus = (outParam.isQamLock == true) && (outParam.isFecLock == true);
done:
#endif
	BDBG_LEAVE(BDCM_Ads_GetChannelLockStatus);
	return retCode;
}

BERR_Code BDCM_Ads_GetChannelSoftDecision(
    BDCM_AdsChannelHandle hChannel,
    int16_t nbrToGet,
    int16_t *iVal,
    int16_t *qVal,
    int16_t *nbrGotten
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ADS_GetSoftDecisions outParam;
    BRPC_Param_XXX_Get Param;
    BERR_Code retVal;
    int cnt;
    unsigned int idx;

    BDBG_ASSERT(hChannel);
    BDBG_ASSERT(iVal);
    BDBG_ASSERT(qVal);
    BDBG_ASSERT(nbrGotten);

    BDBG_ENTER(BDCM_Ads_GetChannelSoftDecision);

    Param.devId = hChannel->devId;
    *nbrGotten = 0;
    for(cnt = nbrToGet; cnt > 0; cnt -= MX_IQ_PER_GET)
    {

        CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                           BRPC_ProcId_ADS_GetSoftDecision,
                                           (const uint32_t *)&Param,
                                           sizeInLong(Param),
                                           (uint32_t *)&outParam,
                                           sizeInLong(outParam),
                                           &retVal));
        CHK_RETCODE(retCode, retVal);
        BDBG_MSG(("%s nbrGotten%d",__FUNCTION__,outParam.nbrGotten));
        if(outParam.nbrGotten > (unsigned)cnt)
        {
            outParam.nbrGotten = cnt;
        }
		/* Copy one block at a time */
		for(idx = 0; idx < outParam.nbrGotten; idx++)
        {
            *iVal++ = (int16_t) outParam.iVal[idx];
            *qVal++ = (int16_t) outParam.qVal[idx];
            *nbrGotten += 1;
        }
    }
done:
    BDBG_LEAVE(BDCM_Ads_GetChannelSoftDecision);
    return retCode;
}

BERR_Code BDCM_Ads_EnableChannelPowerSaver(BDCM_AdsChannelHandle hChannel)
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ADS_EnablePowerSaver Param;
    BERR_Code retVal;
    BDBG_ENTER(BDCM_Ads_EnableChannelPowerSaver);
    BDBG_ASSERT(hChannel);
    Param.devId = hChannel->devId;
    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_ADS_EnablePowerSaver,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
    CHK_RETCODE(retCode, retVal);
done:
    BDBG_LEAVE(BDCM_Ads_EnableChannelPowerSaver);
    return retCode;
}


BERR_Code BDCM_Ads_ProcessChannelNotification(
    BDCM_AdsChannelHandle hChannel,
    unsigned int event
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Notification_Event event_code;
    BDCM_AdsLockStatus lockStatus;

    BDBG_ENTER(BDCM_Ads_ProcessChannelNotification);
    BDBG_ASSERT(hChannel);

    event_code = event>>16;

    switch (event_code)
    {
	    case BRPC_Notification_Event_LockStatusChanged:
	        lockStatus = (event & BRPC_Qam_Lock) && (event & BRPC_Fec_Lock);
            BDBG_MSG(("%s:BDCM_AdsCallback_eLockChange: lockStatus %d",__FUNCTION__,lockStatus));
            BKNI_AcquireMutex(hChannel->mutex);
            hChannel->lockStatus = lockStatus;
            BKNI_ReleaseMutex(hChannel->mutex);
            if( hChannel->pCallback[BDCM_AdsCallback_eLockChange] != NULL )
            {
                (hChannel->pCallback[BDCM_AdsCallback_eLockChange])(hChannel->pCallbackParam[BDCM_AdsCallback_eLockChange] );
            }
            break;
	    case BRPC_Notification_Event_DsChannelPower:
		    hChannel->dsChannelPower = BRPC_GET_DS_POWER(event);
		    break;
		default:
			BDBG_WRN(("unknown event code from DOCSIS %u",event_code));
			break;
	}

	BDBG_LEAVE(BDCM_Ads_ProcessChannelNotification);
	return retCode;
}

BERR_Code BDCM_Ads_InstallChannelCallback(
    BDCM_AdsChannelHandle hChannel,
    BDCM_AdsCallback callbackType,
    BDCM_AdsCallbackFunc pCallback,
    void *pParam
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ENTER(BDCM_Ads_InstallChannelCallback);
    BDBG_ASSERT(hChannel);

    switch(callbackType)
    {
        case BDCM_AdsCallback_eLockChange:
            hChannel->pCallback[callbackType] = pCallback;
            hChannel->pCallbackParam[callbackType] = pParam;
        break;
		default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }
    BDBG_LEAVE(BDCM_Ads_InstallChannelCallback);
    return retCode;
}



BERR_Code BDCM_Ads_SetScanParam(
    BDCM_AdsChannelHandle hChannel,
    BDCM_Ads_ChannelScanSettings *scanParam
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ADS_Scan Param;
    BERR_Code retVal;

    BDBG_ENTER(BDCM_Ads_SetScanParam);
    BDBG_ASSERT(hChannel);
    BDBG_ASSERT(scanParam);

    BKNI_AcquireMutex(hChannel->mutex);
    hChannel->lockStatus = BDCM_AdsLockStatus_eUnlocked;
    BKNI_ReleaseMutex(hChannel->mutex);
    hChannel->accCorrectedCount = 0;
    hChannel->accUncorrectedCount = 0;
    BDBG_MSG(("%s: Channel %u devID %u",__FUNCTION__,hChannel->channelNum,hChannel->devId));
    Param.devId = hChannel->devId;


     /* uint32_t ScanParams;        b31-b24	AI	QM	CO	TO	-	-	-	- */
     /* b23-b16 	A256 A128 A64 A32 A16 B1024 B256 B64 */
     /* b15-b00 	Reserved */
    Param.ScanParams = (uint32_t)(scanParam->AI<<31 | scanParam->QM <<30 | scanParam->CO<<29 | scanParam->TO<<28 |
                                    scanParam->A256 << 23 | scanParam->A128<<22 | scanParam->A64<<21 | scanParam->A32<<20 |
                                    scanParam->A16 <<19 | scanParam->B1024 <<18 | scanParam->B256<<17 |scanParam->B64<<16);
    BDBG_MSG(("scan parameter 0x%x\n", Param.ScanParams));

    Param.CarrierSearch = scanParam->carrierSearch;
    Param.UpperBaudSearch = scanParam->upperBaudSearch;
    Param.LowerBaudSearch = scanParam->lowerBaudSearch;

    BDBG_MSG(("scan Param.CarrierSearch 0x%x, Param.UpperBaudSearch 0x%x, Param.LowerBaudSearch 0x%x\n", Param.CarrierSearch, Param.UpperBaudSearch, Param.LowerBaudSearch));

    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                                       BRPC_ProcId_ADS_Scan,
                                       (const uint32_t *)&Param,
                                       sizeInLong(Param),
                                       NULL, 0, &retVal));
    CHK_RETCODE(retCode, retVal);
done:
	BDBG_LEAVE(BDCM_Ads_SetScanParam);
	return( retCode );
}


BERR_Code BDCM_Ads_GetScanStatus(
    BDCM_AdsChannelHandle hChannel,            /* [in] Device channel handle */
    BDCM_Ads_ScanStatus *pScanStatus        /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ADS_GetScanStatus outParam;
    BERR_Code retVal;
    BRPC_Param_XXX_Get Param;

    BDBG_ENTER(BDCM_Ads_GetScanStatus);
    BDBG_ASSERT(hChannel);
    BDBG_ASSERT( pScanStatus );

    BDBG_MSG(("%s: Channel %u devID %u",__FUNCTION__,hChannel->channelNum,hChannel->devId));
    Param.devId = hChannel->devId;

    BKNI_Memset( pScanStatus, 0x00, sizeof( BDCM_Ads_ScanStatus));

    Param.devId = hChannel->devId;
    CHK_RETCODE(retCode, BRPC_CallProc(hChannel->hDevice->hRpc,
                            BRPC_ProcId_ADS_GetScanStatus,
                            (const uint32_t *)&Param,
                            sizeInLong(Param),
                            (uint32_t *)&outParam,
                            sizeInLong(outParam),
                            &retVal));
    CHK_RETCODE(retCode, retVal);

    pScanStatus ->acquisitionStatus = outParam.AcquisitionStatus;
    pScanStatus ->autoAcquire = outParam.InvSpec_AutoAcq & 0x1;
    pScanStatus ->isSpectrumInverted = outParam.InvSpec_AutoAcq >> 1;
    pScanStatus ->carrierFreqOffset = outParam.RF_Offset;
    pScanStatus ->interleaver = outParam.Interleaver;
    pScanStatus ->symbolRate = outParam.symbolRate;
    pScanStatus ->modType = outParam.modType;

done:
    BDBG_LEAVE(BDCM_Ads_GetScanStatus);
    return( retCode );
}
