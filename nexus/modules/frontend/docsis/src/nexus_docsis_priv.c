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

#include "nexus_frontend_module.h"
#include "nexus_docsis_priv.h"
#include "bdcm_rtnr.h"

BDBG_MODULE(nexus_docsis_priv);

NEXUS_DocsisChannelHandle NEXUS_Docsis_P_GetChannelHandle(
    NEXUS_DocsisDeviceHandle hDevice,
    BRPC_DevId notificationId)
{
    NEXUS_DocsisChannelHandle hChannel=NULL;
    int dsChannelNum = -1;
    if(BRPC_DevId_DOCSIS_OB0 == notificationId && hDevice->hOutOfBandChannel)
    {
        hChannel = (NEXUS_DocsisChannelHandle)(hDevice->hOutOfBandChannel->pDeviceHandle);
        goto end;
    }

    if(notificationId >= BRPC_DevId_ECM_DS0 && notificationId < BRPC_DevId_ECM_DS_LAST)
    {
        dsChannelNum = notificationId - BRPC_DevId_ECM_DS0;
    }
    else
    {
        if(notificationId >= BRPC_DevId_DOCSIS_DS0 && notificationId <= BRPC_DevId_DOCSIS_DS7)
        {
            dsChannelNum = notificationId - BRPC_DevId_DOCSIS_DS0;
        }
        else
        {
            dsChannelNum = -1;
        }

    }
    if(dsChannelNum>=0)
    {
        unsigned i=0;
        for(i=hDevice->numDataChannels;i< hDevice->numDsChannels;i++)
        {
            if(hDevice->hDsChannel[i])
            {
               hChannel = (NEXUS_DocsisChannelHandle) hDevice->hDsChannel[i]->pDeviceHandle;
            }
            if(hChannel && hChannel->dsChannelNum == (unsigned)dsChannelNum)
            {
                break;
            }
            else
            {
                hChannel = NULL;
            }
        }
    }
end:
    return hChannel;
}

#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
static void NEXUS_Docsis_P_TuneComplete(void *context, int param)
{
    NEXUS_DocsisDevice *pDevice = (NEXUS_DocsisDevice *)context;
    BERR_Code retCode;

    BSTD_UNUSED(param);

    BDBG_MSG(("NEXUS_Docsis_P_TuneComplete received."));

    retCode = BDCM_Rtnr_TuneAck(pDevice->hDocsis);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR((" BDCM_Rtnr_TuneAck fail"));
    }
}

static void NEXUS_Docsis_P_TuneAsyncStatusReady(void *context, int param)
{
    BSTD_UNUSED(param);

    BDBG_MSG(("NEXUS_Docsis_P_TuneAsyncStatusReady called."));
    BKNI_SetEvent((BKNI_EventHandle)context);
}
#endif

void NEXUS_Docsis_P_ProcessNotification(
    uint32_t notificationId,
    uint32_t event,
    void * arg)
{
    unsigned rpcEvent;
    NEXUS_DocsisDeviceHandle hDevice = (NEXUS_DocsisDeviceHandle)arg;
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    NEXUS_TunerHandle hDataTuner = hDevice->dataTuner;
    NEXUS_TunerTuneSettings tunerTuneSettings;
    NEXUS_TunerSettings tunerSettings;
    NEXUS_TunerStatus tunerStatus;
    int dacGain = 0;
    NEXUS_Error rc;
    BERR_Code retCode;
#else
    NEXUS_DocsisChannelHandle hChannel;
#endif

    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);
    BDBG_MSG(("notificationId %u event %#lx hDevice %#lx",
              notificationId,(long unsigned int)event,(long unsigned int)hDevice));
    hDevice->notificationCount++;
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    rpcEvent = BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT(event);
#else
    rpcEvent = BRPC_GET_NOTIFICATION_EVENT(event);
    hChannel = NEXUS_Docsis_P_GetChannelHandle(hDevice,notificationId);
#endif
    switch(rpcEvent)
    {
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    case BRPC_Notification_Event_TuneInit:
        {
            BDBG_ERR(("Get Notification BRPC_Notification_Event_TuneInit"));
            dacGain = BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_TUNEINIT_GAIN(event);
            BDBG_ERR(("TuneInit: dacGain[%d]", dacGain));
        }
        break;

    case BRPC_Notification_Event_Tune:
        {
            BDBG_ERR(("Get Notification BRPC_Notification_Event_Tune"));

            NEXUS_Tuner_GetSettings(hDataTuner, &tunerSettings);
            NEXUS_Tuner_GetDefaultTuneSettings(NEXUS_TunerMode_eQam, &tunerTuneSettings);

            if(BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_TUNE_ANNEX_MODE(event))
            {
                tunerSettings.bandwidth = NEXUS_FrontendQamBandwidth_e8Mhz;
                tunerSettings.ifFrequency = 5000000;
                tunerTuneSettings.modeSettings.qam.annex = NEXUS_FrontendQamAnnex_eA;
            }
            else
            {
                tunerSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
                tunerSettings.ifFrequency = 4000000;
                tunerTuneSettings.modeSettings.qam.annex = NEXUS_FrontendQamAnnex_eB;
            }

            tunerSettings.dacAttenuation = dacGain*256;
            rc = NEXUS_Tuner_SetSettings(hDataTuner, &tunerSettings);
            if(rc){rc = BERR_TRACE(rc);}

            tunerTuneSettings.mode = NEXUS_TunerMode_eQam;
            tunerTuneSettings.modeSettings.qam.qamMode = NEXUS_FrontendQamMode_e256;
            tunerTuneSettings.frequency = BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT_TUNE_RF_FREQ(event)*1000;
            tunerTuneSettings.tuneCompleteCallback.callback = NEXUS_Docsis_P_TuneComplete;
            tunerTuneSettings.tuneCompleteCallback.context = hDevice;
            tunerTuneSettings.asyncStatusReadyCallback.callback = NEXUS_Docsis_P_TuneAsyncStatusReady;
            tunerTuneSettings.asyncStatusReadyCallback.context = hDevice->tunerStatusEvent;
            BDBG_ERR(("Tune: freq[%d], annex[%d], qammode[%d]", tunerTuneSettings.frequency,
                tunerTuneSettings.modeSettings.qam.annex,
                tunerTuneSettings.modeSettings.qam.qamMode));
            rc = NEXUS_Tuner_Tune(hDataTuner, &tunerTuneSettings);
            if(rc){rc = BERR_TRACE(rc);}
        }
        break;

    case BRPC_Notification_Event_WfeRssi:
        {
            BDBG_ERR(("Get Notification BRPC_Notification_Event_WfeRssi"));

            rc = NEXUS_Tuner_RequestAsyncStatus(hDataTuner);
            if(rc){rc = BERR_TRACE(rc);}

            rc = BKNI_WaitForEvent(hDevice->tunerStatusEvent, 1000);
            if(rc){rc = BERR_TRACE(rc);}

            rc = NEXUS_Tuner_GetAsyncStatus(hDataTuner, &tunerStatus);
            if(rc){rc = BERR_TRACE(rc);}

            retCode = BDCM_Rtnr_WfeRssiAck(hDevice->hDocsis, tunerStatus.rssi, tunerStatus.adcPgaGain);
            if (retCode != BERR_SUCCESS)
            {
                BDBG_ERR((" BDCM_Rtnr_WfeRssiAck fail"));
            }
        }
        break;
#else
    case BRPC_Notification_Event_LockStatusChanged:
        {
            BDBG_MSG(("DOCSIS lock changed event notification"));
            if (hChannel && hChannel->tuneStarted)
            {
                switch(hChannel->settings.channelType)
                {
                case NEXUS_DocsisChannelType_eQam:
                    {
                        BDBG_MSG((" QAM channel:%u -> firing DOCSIS sync status callback",
                                  hChannel->dsChannelNum));
                        BDCM_Ads_ProcessChannelNotification(hChannel->qam,event);
                    }
                    break;
                case NEXUS_DocsisChannelType_eOutOfBand:
                    {
                        BDBG_MSG((" OOB channel -> firing DOCSIS sync status callback"));
                        BDCM_Aob_ProcessChannelNotification(hChannel->outOfBand,event);
                    }
                    break;
                default:
                    BDBG_MSG(("lock state change event not handled"));
                 }
            }
        }
        break;
    case BRPC_Notification_Event_DsChannelPower:
        {
            BDBG_MSG(("DOCSIS DS Channel event notification"));
            BDCM_Ads_ProcessChannelNotification(hChannel->qam,event);
            if (hChannel && hChannel->asyncStatusAppCallback)
            {
                BDBG_MSG(("firing DOCSIS async status callback"));
                NEXUS_TaskCallback_Fire(hChannel->asyncStatusAppCallback);
            }
        }
        break;
#endif
    case BRPC_Notification_Event_EcmReset:
        {
            BDBG_MSG(("DOCSIS reset event notification"));
            hDevice->status.state = NEXUS_DocsisDeviceState_eReset;
            if(hDevice && hDevice->statusCallback)
            {
                BDBG_MSG(("firing DOCSIS status callback"));
                NEXUS_TaskCallback_Fire(hDevice->statusCallback);
            }
        }
        break;
    case BRPC_Notification_Event_EcmOperational:
        {
            BDBG_MSG(("DOCSIS operational event notification"));
        }
        break;
    case BRPC_Notification_Event_EcmRmagnumReady:
        {
            BDBG_MSG(("DOCSIS ready event notification"));
            if(hDevice && hDevice->heartBeatEvent)
            {
                BKNI_SetEvent(hDevice->heartBeatEvent);
            }
        }
        break;

    default:
        BDBG_MSG(("DOCSIS unknown event notification"));
    }
    return;
}

void NEXUS_Docsis_P_NotificationThread(void *arg)
{
    uint32_t notificationId, event;
    NEXUS_DocsisDeviceHandle hDevice = (NEXUS_DocsisDeviceHandle)arg;
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    BERR_Code retCode;
    retCode = BDCM_Rtnr_SetDevId(hDevice->hDocsis, BRPC_DevId_DOCSIS);
    if(retCode)
    {
        BDBG_ERR((" BDCM_Rtnr_SetDevId fail"));
        return;
    }
#endif
    while(hDevice->notificationEnabled)
    {
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
        BRPC_CheckNotification(hDevice->hDocsis->hRpc,  &notificationId, &event, 1000);
        if (event && BRPC_GET_NOTIFICATION_REVERSE_RMAGNUM_EVENT(event))
#else
        BRPC_CheckNotification(hDevice->hDocsis->hRpc,  &notificationId, &event, 100);
        if (BRPC_GET_NOTIFICATION_EVENT(event))
#endif
        {
            BDBG_MSG(("notification event available from DOCSIS"));
            NEXUS_LockModule();
            NEXUS_Docsis_P_ProcessNotification(notificationId, event, arg);
            NEXUS_UnlockModule();
        }
    }
    return;
}

/*
 * DOCSIS device sends a heart beat event every 2 seconds.
 * In case, the heart event isn't sent to the host,
 * then this thread shall wait for the DOCSIS device to
 * become active. After DOCSIS device is reactivated,
 * the DOCSIS channels are re-initialized.
 */
void NEXUS_Docsis_P_HeartBeatThread(void * arg)
{
    NEXUS_DocsisDeviceHandle hDevice = (NEXUS_DocsisDeviceHandle)arg;
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDCM_Version version;
	BRPC_Param_XXX_Get Param;
	bool restart;
    unsigned initTryCount=0;
    Param.devId = BRPC_DevId_DOCSIS;

	hDevice->notificationCount = 0; /* clear notification count*/

	while(hDevice->heartBeatEnabled == true)
	{
        restart = true;
        if (hDevice->status.state == NEXUS_DocsisDeviceState_eOperational)
		{
		    retCode = BDCM_GetDeviceVersion(hDevice->hDocsis, &version);
		    restart = (retCode != BERR_SUCCESS) && (!hDevice->notificationCount);
			if (restart)
			{
				hDevice->status.state = NEXUS_DocsisDeviceState_eReset;
                BDBG_WRN(("DOCSIS device in reset state"));
			}
		}
        while (restart && hDevice->heartBeatEnabled)
        {
            retCode = BKNI_WaitForEvent(hDevice->heartBeatEvent, 10000);
            if (!hDevice->heartBeatEnabled) break;
            BKNI_ResetEvent(hDevice->heartBeatEvent);
            initTryCount = 0;
            while (hDevice->heartBeatEnabled == true)
            {
                retCode = BDCM_InitDevice(hDevice->hDocsis);
                restart = (retCode != BERR_SUCCESS);
                if (!restart ||  ++initTryCount >= 5) break;
                BKNI_Sleep(100);
            }
            if (restart)
            {
                BDBG_WRN((" DOCSIS is dead! "));
                NEXUS_LockModule();
                hDevice->status.state = NEXUS_DocsisDeviceState_eFailed;
                NEXUS_TaskCallback_Fire(hDevice->statusCallback);
                NEXUS_UnlockModule();
            }
            else
            {
                BDBG_ERR((" DOCSIS heartbeat is back"));
                BDBG_ERR((" Attempting to reset all the activated channels"));
                NEXUS_LockModule();
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
                retCode = BDCM_Rtnr_SetDevId(hDevice->hDocsis, BRPC_DevId_DOCSIS);
                if (retCode != BERR_SUCCESS)
                {
                    BDBG_ERR((" BDCM_Rtnr_SetDevId fail"));
                    return;
                }
                hDevice->status.state = NEXUS_DocsisDeviceState_eOperational;
#else

                rc = NEXUS_Docsis_P_Reset(hDevice);
#endif
                if (rc!=NEXUS_SUCCESS)
                {
                    BDBG_ERR(("failed to reset the activated channels after DOCSIS reset"));
                    NEXUS_UnlockModule();
                    continue;
                }
                NEXUS_TaskCallback_Fire(hDevice->statusCallback);
                NEXUS_UnlockModule();
                BDBG_ERR(("resetting ofchannels succeeded"));
            }
        }
checkHearBeatNotification:
        NEXUS_LockModule();
        hDevice->notificationCount = 0; /* clear notification count*/
        NEXUS_UnlockModule();
        BKNI_WaitForEvent(hDevice->heartBeatEvent, 2000);
        if (!hDevice->heartBeatEnabled)
            break;
        if (hDevice->notificationCount)
        {
            BDBG_MSG(("DOCSIS device active - notificationCount:%u",hDevice->notificationCount));
            goto checkHearBeatNotification;
        }

    }
    return;
}

NEXUS_Error NEXUS_Docsis_P_Reset(NEXUS_DocsisDeviceHandle hDevice)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_FrontendHandle hFrontend = NULL;
    for (i=hDevice->numDataChannels;i < hDevice->numDsChannels;i++)
    {
        hFrontend = hDevice->hDsChannel[i];
        if(hFrontend)
        {
            hChannel = (NEXUS_DocsisChannelHandle)hFrontend->pDeviceHandle;
            BDBG_MSG(("resetting DS channel %u",hChannel->dsChannelNum));
            if(hChannel->hTnr)
            {
                BDCM_TnrSettings tnrSettings;
                BDCM_Tnr_Close(hChannel->hTnr);
                hChannel->hTnr = NULL;
                BDCM_Tnr_GetDefaultSettings(&tnrSettings);
                tnrSettings.ifFreq = BDCM_TNR_IFFREQ;
                tnrSettings.type = BDCM_TnrType_eAds;
                tnrSettings.adsTunerNum = hChannel->dsChannelNum;
                tnrSettings.minVer = hDevice->version.minVer;
                hChannel->hTnr = BDCM_Tnr_Open(hDevice->hDocsis,
                                               &tnrSettings);
                if (!hChannel->hTnr)
                {
                    BDBG_WRN(("%s: QAM tuner open failed dsChannel:%u",
                              __FUNCTION__,hChannel->settings.channelNum));
                    rc = NEXUS_NOT_INITIALIZED;
                    break;
                }
            }
            if(hChannel->qam)
            {
                BDCM_AdsSettings adsSettings;
                BDCM_Ads_CloseChannel(hChannel->qam);
                hChannel->qam = NULL;
                BDCM_Ads_GetChannelDefaultSettings(&adsSettings);
                adsSettings.autoAcquire = hChannel->settings.autoAcquire;
                adsSettings.fastAcquire = hChannel->settings.fastAcquire;
                adsSettings.minVer = hDevice->version.minVer;
		        hChannel->qam = BDCM_Ads_OpenChannel(hDevice->hDocsis,
                                                        hChannel->settings.channelNum,
                                                        &adsSettings);
                if(!hChannel->qam)
                {
                    BDBG_WRN(("%s: QAM demod open failed dsChannel:%u",
                              __FUNCTION__,hChannel->settings.channelNum));
                    rc = NEXUS_NOT_INITIALIZED;
                    break;
                }
                BDCM_Ads_InstallChannelCallback(hChannel->qam, BDCM_AdsCallback_eLockChange,
									       (BDCM_AdsCallbackFunc)NEXUS_Docsis_P_QamLockStatus,
                                           (void*)hChannel->lockDriverCallback);
             }

        }
    }

    if(rc!=NEXUS_SUCCESS)
    {
        BDBG_WRN(("reset DS channel %u",i));
        goto error;
    }
    if(hDevice->hOutOfBandChannel)
    {
        hFrontend = hDevice->hOutOfBandChannel;
        hChannel = (NEXUS_DocsisChannelHandle)hFrontend->pDeviceHandle;
        BDBG_MSG(("resetting OOB channel"));
        if(hChannel->hTnr)
        {
            BDCM_TnrSettings tnrSettings;
            BDCM_Tnr_Close(hChannel->hTnr);
            hChannel->hTnr = NULL;
            BDCM_Tnr_GetDefaultSettings(&tnrSettings);
            tnrSettings.type = BDCM_TnrType_eAob;
            tnrSettings.ifFreq = BDCM_TNR_IFFREQ;
            tnrSettings.minVer = hDevice->version.minVer;
            hChannel->hTnr = BDCM_Tnr_Open(hDevice->hDocsis,
                                           &tnrSettings);
            if(!hChannel->hTnr)
            {
                BDBG_ERR(("%s: OOB demod open failed",__FUNCTION__));
                rc = NEXUS_NOT_INITIALIZED;
                goto error;
            }

        }
        if(hChannel->outOfBand)
        {
            BDCM_AobSettings aobSettings;
            BDCM_Aob_CloseChannel(hChannel->outOfBand);
            hChannel->outOfBand = NULL;
            BDCM_Aob_GetChannelDefaultSettings(&aobSettings);
            aobSettings.enableFEC = false; /* cable card does the FEC*/
            aobSettings.ifFreq = BDCM_AOB_IFFREQ;
            aobSettings.spectrum = BDCM_AobSpectrumMode_eAuto;
            hChannel->outOfBand = BDCM_Aob_OpenChannel(hDevice->hDocsis,
                                                          &aobSettings);
            if(!hChannel->outOfBand)
            {
                BDBG_ERR(("%s: OOB demod open failed",__FUNCTION__));
                rc = NEXUS_NOT_INITIALIZED;
                goto error;
            }
            BDCM_Aob_InstallChannelCallback(hChannel->outOfBand, BDCM_AobCallback_eLockChange,
                        (BDCM_AobCallbackFunc)NEXUS_Docsis_P_OobLockStatus, (void*)hChannel->lockAppCallback);
        }

    }
    if(hChannel->upStream)
    {
        hFrontend = hDevice->hUpStreamChannel;
        hChannel = (NEXUS_DocsisChannelHandle)hFrontend->pDeviceHandle;
        BDBG_MSG(("resetting US channel"));
        if(hChannel->upStream)
        {
            BDCM_AusSettings ausSettings;
            BDCM_Aus_CloseChannel(hChannel->upStream);
            hChannel->outOfBand = NULL;
            BDCM_Aus_GetChannelDefaultSettings(&ausSettings);
            ausSettings.xtalFreq = BDCM_AUS_XTALFREQ;
            hChannel->upStream =  BDCM_Aus_OpenChannel(hDevice->hDocsis,
                                                          &ausSettings);
            if(hChannel->upStream)
            {
                BDBG_ERR(("%s: US demod open failed",__FUNCTION__));
                rc = NEXUS_NOT_INITIALIZED;
                goto error;
            }

        }
    }
    hDevice->status.state = NEXUS_DocsisDeviceState_eOperational;
    return rc;
error:
    hDevice->status.state = NEXUS_DocsisDeviceState_eUninitialized;
    return rc;
}

void NEXUS_Docsis_P_CloseDevice(void *handle)
{
    NEXUS_DocsisDeviceHandle hDevice = (NEXUS_DocsisDeviceHandle)handle;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);
#if NEXUS_HAS_MXT
    if (hDevice->pGenericDeviceHandle->chainedConfig)
    {
        if (hDevice->pGenericDeviceHandle->chainedConfig->mxt)
        {
            BMXT_Close(hDevice->pGenericDeviceHandle->chainedConfig->mxt);
        }
        BKNI_Free(hDevice->pGenericDeviceHandle->chainedConfig);
    }
    if (hDevice->pGenericDeviceHandle->mtsifConfig.mxt) {
        BMXT_Close(hDevice->pGenericDeviceHandle->mtsifConfig.mxt);
    }
#endif
	if (hDevice->heartBeatThread)
	{
        /* unlock module to unblock the DOCSIS heart beat thread */
		NEXUS_UnlockModule();
		hDevice->heartBeatEnabled = false;
		BKNI_SetEvent(hDevice->heartBeatEvent);
		BKNI_Sleep(100);
		NEXUS_Thread_Destroy(hDevice->heartBeatThread);
        /* lock the module */
		NEXUS_LockModule();
	}
    /* unlock module to unblock the DOCSIS notification thread */
	NEXUS_UnlockModule();
	hDevice->notificationEnabled = false;
    /* time for task to finish */
	BKNI_Sleep(600);
	NEXUS_Thread_Destroy(hDevice->notificationThread);
    /* lock the module */
	NEXUS_LockModule();
    if (hDevice->heartBeatEvent)
    {
        BKNI_DestroyEvent(hDevice->heartBeatEvent);
    }
#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    if (hDevice->tunerStatusEvent)
    {
        BKNI_DestroyEvent(hDevice->tunerStatusEvent);
    }
#endif
    if (hDevice->statusCallback)
    {
        NEXUS_TaskCallback_Destroy(hDevice->statusCallback);
    }
    if(hDevice->hDocsis)
    {
        BDCM_CloseDevice(hDevice->hDocsis);
    }

    BKNI_Free(hDevice->pGenericDeviceHandle);
	BDBG_OBJECT_DESTROY(hDevice, NEXUS_DocsisDevice);

	BKNI_Free(hDevice);
	return;
}


NEXUS_Error NEXUS_Docsis_P_GetStatus(
    void *handle,
     NEXUS_FrontendDeviceStatus *pStatus)
{
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDCM_DeviceTemperature deviceTemp;
    hDevice = (NEXUS_DocsisDeviceHandle)handle;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);
    BDBG_ASSERT(pStatus);

    retCode =  BDCM_GetDeviceTemperature(hDevice->hDocsis,
                                            &deviceTemp);
    if(!retCode)
    {
        /* convert 100'th to 1000'th units */
        pStatus->temperature = deviceTemp.temperature * 10;
    }
    else
    {
        rc = NEXUS_UNKNOWN;
    }
    return rc;
}

NEXUS_Error NEXUS_Docsis_P_GetDocsisLnaDeviceAgcValue(
    void *handle,
     uint32_t *agcVal)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    hDevice = (NEXUS_DocsisDeviceHandle)handle;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);
    BDBG_ASSERT(agcVal);
    *agcVal = 0;
    retCode = BDCM_GetLnaDeviceAgcValue(hDevice->hDocsis,hDevice->version,agcVal);
    return retCode;
}

NEXUS_Error NEXUS_Docsis_P_SetHostChannelLockStatus(
    void *handle,
    unsigned channelNum,
    bool locked)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    uint32_t hostChannelLockStatus=0;
    hDevice = (NEXUS_DocsisDeviceHandle)handle;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    retCode =  BDCM_GetDeviceHostChannelsStatus(hDevice->hDocsis,&hostChannelLockStatus);
    BDBG_ERR(("current host channels lock status %u",hostChannelLockStatus));

    if(locked)
    {
        hostChannelLockStatus |= (1<<channelNum);
    }
    else
    {
        hostChannelLockStatus &= ~(1<<channelNum);
    }
    retCode =  BDCM_SetDeviceHostChannelsStatus(hDevice->hDocsis,hostChannelLockStatus);
    BDBG_ERR(("changed host channels lock status %u",hostChannelLockStatus));
    return retCode;
}

void NEXUS_Docsis_P_CloseChannel(NEXUS_FrontendHandle handle)
{
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDBG_OBJECT_ASSERT(handle,NEXUS_Frontend);
    hChannel = (NEXUS_DocsisChannelHandle)handle->pDeviceHandle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
	hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    if (hChannel->retuneTimer)
    {
         NEXUS_CancelTimer(hChannel->retuneTimer);
         hChannel->retuneTimer = NULL;
    }
    if(hChannel->lockDriverCallback)
    {
        NEXUS_CallbackHandler_Shutdown(hChannel->lockDriverCBHandler);
        NEXUS_TaskCallback_Destroy(hChannel->lockDriverCallback);
        hChannel->lockDriverCallback = NULL;
    }

    if (hChannel->lockAppCallback)
    {
        NEXUS_TaskCallback_Destroy(hChannel->lockAppCallback);
        hChannel->lockAppCallback = NULL;
    }

    if (hChannel->asyncStatusAppCallback)
    {
        NEXUS_TaskCallback_Destroy(hChannel->asyncStatusAppCallback);
        hChannel->asyncStatusAppCallback = NULL;
    }

    if(hChannel->qam)
    {
        BDCM_Ads_CloseChannel(hChannel->qam);
        BDCM_Ads_InstallChannelCallback(hChannel->qam,
                                           BDCM_AdsCallback_eLockChange,
                                           NULL, NULL);
        hChannel->qam = NULL;
        hDevice->hDsChannel[hChannel->dsChannelNum] = NULL;
    }

    if(hChannel->outOfBand)
    {
        BDCM_Aob_InstallChannelCallback(hChannel->outOfBand,
                                           BDCM_AobCallback_eLockChange,
                                           NULL, NULL);
        BDCM_Aob_CloseChannel(hChannel->outOfBand);
        hChannel->outOfBand = NULL;
    }

    if(hChannel->upStream)
    {
        BDCM_Aus_CloseChannel(hChannel->upStream);
        hChannel->upStream = NULL;
    }

    if(hChannel->settings.channelType == NEXUS_DocsisChannelType_eData)
    {
        hDevice->hDsChannel[hChannel->dsChannelNum] = NULL;
    }

    if(hChannel->hTnr)
    {
        BDCM_Tnr_Close(hChannel->hTnr);
        hChannel->hTnr = NULL;
    }

    NEXUS_Frontend_P_Destroy(handle);
    BKNI_Free(hChannel);
    return;
}

NEXUS_Error NEXUS_Docsis_P_GetFastStatus(
   void *handle,
   NEXUS_FrontendFastStatus *pStatus
   )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    bool isLocked=false;

    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);
    BKNI_Memset(pStatus,0,sizeof(*pStatus));
    if(hChannel->settings.channelType == NEXUS_DocsisChannelType_eQam)
    {
        if(hChannel->tuneStarted)
        {
            BDCM_AdsLockStatus adsLockStatus = BDCM_AdsLockStatus_eUnlocked;
            BDCM_Ads_GetChannelLockStatus(hChannel->qam,&adsLockStatus);
            isLocked = (adsLockStatus == BDCM_AdsLockStatus_eLocked)? true:false;
        }
        else
        {
            BDBG_WRN(("app didn't send the tune command yet for QAM channel %u",hChannel->dsChannelNum));
            rc = NEXUS_NOT_INITIALIZED;
            goto error;
        }
    }
    else
    {
        if(hChannel->settings.channelType == NEXUS_DocsisChannelType_eOutOfBand)
        {
            if(hChannel->tuneStarted)
            {
                BDCM_Aob_GetChannelLockStatus(hChannel->outOfBand,&isLocked);
            }
            else
            {
                BDBG_WRN(("app didn't send the tune command yet for OOB channel"));
                rc = NEXUS_NOT_INITIALIZED;
                goto error;
            }
        }
        else
        {
            BDBG_ERR(("getFastStatus not supported for channel type %u",hChannel->settings.channelType));
            rc = NEXUS_NOT_SUPPORTED;
            goto error;
        }
    }

    if(isLocked)
    {
        pStatus->lockStatus = NEXUS_FrontendLockStatus_eLocked;
        pStatus->acquireInProgress = false;
    }
    else
    {
        pStatus->lockStatus = NEXUS_FrontendLockStatus_eUnlocked;
        pStatus->acquireInProgress = true;
    }

error:
    return rc;
}

void NEXUS_Docsis_P_Untune(
    void *handle
    )
{
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    NEXUS_FrontendHandle hFrontend=NULL;
    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    hChannel->tuneStarted = false;
    switch(hChannel->settings.channelType)
    {
    case NEXUS_DocsisChannelType_eQam:
        hFrontend = hDevice->hDsChannel[hChannel->dsChannelNum];
        if(hChannel->retuneTimer)
        {
            NEXUS_CancelTimer(hChannel->retuneTimer);
            hChannel->retuneTimer = NULL;
        }

        {
            BERR_Code retCode = BERR_SUCCESS;

            if(hChannel->hTnr)
            {
                retCode = BDCM_Tnr_SetRfFreq(hChannel->hTnr,
                                                0,
                                                BDCM_TnrMode_eDigital);
                if (retCode != BERR_SUCCESS)
                {
                    BDBG_WRN(("DCM tuner set freq failed!"));
                    return;
                }
            }
        }

        break;
    case NEXUS_DocsisChannelType_eOutOfBand:
        hFrontend = hDevice->hOutOfBandChannel;
        break;
    case NEXUS_DocsisChannelType_eUpstream:
        hFrontend = hDevice->hUpStreamChannel;
        break;
    case NEXUS_DocsisChannelType_eData:
        BDBG_MSG(("data channel untune not handled"));
        break;
    default:
        BDBG_WRN(("untune: unknown channel type"));
    }

    if (hFrontend) {
        NEXUS_Frontend_P_UnsetMtsifConfig(hFrontend);
    }
    return;
}

NEXUS_Error NEXUS_Docsis_P_TuneQam(
    void *handle,
    const NEXUS_FrontendQamSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    BDCM_AdsModulationType qamMode;
    BDCM_AdsInbandParam params;
    unsigned channelNum;
    NEXUS_CallbackDesc callbackDesc;

    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    BDBG_ASSERT(pSettings);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);
    channelNum = hChannel->dsChannelNum;
    BDBG_MSG(("TuneQAM docsis channel %u",channelNum));

    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_NOT_SUPPORTED);
    }

    rc = NEXUS_Frontend_P_SetMtsifConfig(hDevice->hDsChannel[hChannel->dsChannelNum]);
    if (rc) { return BERR_TRACE(rc); }

    BKNI_Memset(&params, 0, sizeof(params));
    switch(pSettings->annex)
    {
    case NEXUS_FrontendQamAnnex_eA:
        {
            switch ( pSettings->mode )
            {
            case NEXUS_FrontendQamMode_e16:
                qamMode = BDCM_AdsModulationType_eAnnexAQam16;
                break;
            case NEXUS_FrontendQamMode_e32:
                qamMode = BDCM_AdsModulationType_eAnnexAQam32;
                break;
            case NEXUS_FrontendQamMode_e64:
                qamMode = BDCM_AdsModulationType_eAnnexAQam64;
                break;
            case NEXUS_FrontendQamMode_e128:
                qamMode = BDCM_AdsModulationType_eAnnexAQam128;
                break;
            case NEXUS_FrontendQamMode_e256:
                qamMode = BDCM_AdsModulationType_eAnnexAQam256;
            break;
            default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
        break;
    case NEXUS_FrontendQamAnnex_eB:
        {
            switch ( pSettings->mode )
            {
            case NEXUS_FrontendQamMode_e64:
                qamMode = BDCM_AdsModulationType_eAnnexBQam64;
                break;
            case NEXUS_FrontendQamMode_e256:
                qamMode = BDCM_AdsModulationType_eAnnexBQam256;
                break;
            default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
        break;
    case NEXUS_FrontendQamAnnex_eC:
        {
            switch ( pSettings->mode )
            {
            case NEXUS_FrontendQamMode_e64:
                qamMode = BDCM_AdsModulationType_eAnnexCQam64;
                break;
            case NEXUS_FrontendQamMode_e256:
                qamMode = BDCM_AdsModulationType_eAnnexCQam256;
                break;
            default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
        }
        break;
    default:
        BDBG_WRN(("invalid annex mode %u",pSettings->annex));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(pSettings->symbolRate)
    {
        params.symbolRate = pSettings->symbolRate;
    }
    else
    {
        params.symbolRate = NEXUS_Frontend_P_GetDefaultQamSymbolRate(pSettings->mode,
                                                                     pSettings->annex);
    }

    callbackDesc.param = channelNum;
    NEXUS_CallbackHandler_PrepareCallback(hChannel->lockDriverCBHandler, callbackDesc);
    NEXUS_TaskCallback_Set(hChannel->lockDriverCallback, &callbackDesc);
    NEXUS_TaskCallback_Set(hChannel->lockAppCallback, &(pSettings->lockCallback));
    NEXUS_TaskCallback_Set(hChannel->asyncStatusAppCallback, &(pSettings->asyncStatusReadyCallback));

    if(hChannel->hTnr)
    {
        retCode = BDCM_Tnr_SetRfFreq(hChannel->hTnr,
                                        pSettings->frequency,
                                        BDCM_TnrMode_eDigital);
        if (retCode != BERR_SUCCESS)
        {
            return BERR_TRACE(retCode);
        }
    }
    params.modType = qamMode;

    {
        BDCM_Ads_ChannelScanSettings scanParam;

        /* Scan Parameters */
        if((pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eScan) /*|| (pSettings->acquisitionMode == NEXUS_FrontendQamAcquisitionMode_eAuto)*/){
            BKNI_Memset(&scanParam, 0, sizeof(scanParam));
            scanParam.QM = true;
            scanParam.TO = true;
            if( pSettings->spectrumMode == NEXUS_FrontendQamSpectrumMode_eAuto) scanParam.AI = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e16]) scanParam.A16 = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e32]) scanParam.A32= true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e64]) scanParam.A64 = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e128]) scanParam.A128 = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e256]) scanParam.A256 = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e512]) scanParam.A512 = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e1024]) scanParam.A1024 = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e64]) scanParam.B64 = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e256]) scanParam.B256 = true;
            if(pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e1024]) scanParam.B1024 = true;
            if(pSettings->scan.frequencyOffset){
                scanParam.CO = true;
                scanParam.carrierSearch = pSettings->scan.frequencyOffset/256;
            }
            scanParam.upperBaudSearch = pSettings->scan.upperBaudSearch;
            scanParam.lowerBaudSearch = pSettings->scan.lowerBaudSearch;

            retCode = BDCM_Ads_SetScanParam(hChannel->qam, &scanParam );
            if (retCode!=BERR_SUCCESS)
            {
                return BERR_TRACE(retCode);
            }
        }
    }

    retCode = BDCM_Ads_AcquireChannel(hChannel->qam,
                                         &params);
    if (retCode!=BERR_SUCCESS)
    {
        return BERR_TRACE(retCode);
    }

    hChannel->tuneStarted = true;
    if (hChannel->retuneTimer)
    {
        NEXUS_CancelTimer(hChannel->retuneTimer);
        hChannel->retuneTimer = NULL;
    }
    hChannel->qamSettings = *pSettings;
    return retCode;
}

NEXUS_Error NEXUS_Docsis_P_GetQamStatus(
    void *handle,
    NEXUS_FrontendQamStatus *pStatus
    )
{
	BERR_Code retCode;
    BDCM_AdsStatus adsStatus;
	BDCM_Version version;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    BDBG_ASSERT(pStatus);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if(hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_NOT_SUPPORTED);
    }

    retCode = BDCM_Ads_GetChannelStatus(hChannel->qam,
                                           &adsStatus);
    if (retCode != BERR_SUCCESS) { return BERR_TRACE(retCode);}

    retCode = BDCM_GetDeviceVersion(hDevice->hDocsis,
                                       &version);
    if (retCode) return BERR_TRACE(retCode);

    pStatus->fecLock = adsStatus.isFecLock;
    pStatus->receiverLock = adsStatus.isQamLock;
    pStatus->symbolRate = adsStatus.rxSymbolRate;
    pStatus->ifAgcLevel = adsStatus.agcIntLevel;
    pStatus->rfAgcLevel = adsStatus.agcExtLevel;
    pStatus->carrierFreqOffset = adsStatus.carrierFreqOffset;
    pStatus->carrierPhaseOffset = adsStatus.carrierPhaseOffset;

    if (adsStatus.isQamLock == false)
    {
        adsStatus.dsChannelPower = 0;
        adsStatus.mainTap = 0;
        adsStatus.equalizerGain = 0;
        adsStatus.interleaveDepth = 0;
    }

    pStatus->dsChannelPower = adsStatus.dsChannelPower;
    pStatus->mainTap = adsStatus.mainTap;
    pStatus->equalizerGain = adsStatus.equalizerGain;
    pStatus->interleaveDepth = adsStatus.interleaveDepth;

    if (adsStatus.isFecLock == false ||adsStatus.isQamLock == false)
    {
        adsStatus.snrEstimate = 0;
        adsStatus.correctedCount = 0;
        adsStatus.uncorrectedCount = 0;
        adsStatus.berRawCount = 0;
        adsStatus.goodRsBlockCount = 0;
        adsStatus.postRsBER = 0;
        adsStatus.elapsedTimeSec = 0;
    }

    pStatus->snrEstimate = adsStatus.snrEstimate*100/256;
    pStatus->fecCorrected = adsStatus.correctedCount;
    pStatus->fecUncorrected = adsStatus.uncorrectedCount;
    pStatus->berEstimate = adsStatus.berRawCount;
    pStatus->goodRsBlockCount = adsStatus.goodRsBlockCount;
    pStatus->postRsBer = adsStatus.postRsBER;
    pStatus->postRsBerElapsedTime = adsStatus.elapsedTimeSec;
    pStatus->spectrumInverted = adsStatus.isSpectrumInverted;
    pStatus->viterbiErrorRate = adsStatus.preRsBER;
    pStatus->errorRateUnits = NEXUS_FrontendErrorRateUnits_eNaturalLog;
    pStatus->settings = hChannel->qamSettings;

    BDBG_MSG(("DOCSIS QAM status"));
    BDBG_MSG(("isFecLock:%d",adsStatus.isFecLock));
    BDBG_MSG(("isQamLock:%d",adsStatus.isQamLock));
    BDBG_MSG(("agcIntLevel:%d",adsStatus.agcIntLevel));
    BDBG_MSG(("agcExtLevel:%d",adsStatus.agcExtLevel));
    BDBG_MSG(("snrEstimate:%d",adsStatus.snrEstimate));
    BDBG_MSG(("correctedCount:%d",adsStatus.correctedCount));
    BDBG_MSG(("uncorrectedCount:%d",adsStatus.uncorrectedCount));
    BDBG_MSG(("berRawCount:%d",adsStatus.berRawCount));
    return retCode;
}

NEXUS_Error NEXUS_Docsis_P_GetQamAsyncStatus(
    void *handle,
    NEXUS_FrontendQamStatus *pStatus
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    rc = NEXUS_Docsis_P_GetQamStatus(handle,pStatus);
    return rc;
}

NEXUS_Error NEXUS_Docsis_P_RequestQamAsyncStatus(
    void *handle
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    retCode = BDCM_Ads_RequestAsyncChannelStatus(hChannel->qam);
    return retCode;
}

void NEXUS_Docsis_P_AutoTuneQam(void *handle)
{
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDCM_AdsModulationType qamMode;
    NEXUS_FrontendQamSettings *pSettings = NULL;
    BDCM_AdsLockStatus adsLockStatus;
    BDCM_AdsInbandParam params;
    BERR_Code retCode = BERR_SUCCESS;

    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    pSettings = &hChannel->qamSettings;
    adsLockStatus = BDCM_AdsLockStatus_eUnlocked;

    hChannel->retuneTimer = NULL;

    if(hDevice->status.state != NEXUS_DocsisDeviceState_eOperational) {return;}

    retCode = BDCM_Ads_GetChannelLockStatus(hChannel->qam,
                                               &adsLockStatus);
    if (retCode!=BERR_SUCCESS)
    {
        BERR_TRACE(retCode);
        return;
    }

    if (adsLockStatus == BDCM_AdsLockStatus_eUnlocked)
    {
        BDBG_MSG(("unlocked -> auto acquiring channel %u",hChannel->dsChannelNum));
        BKNI_Memset(&params, 0, sizeof(params));
        switch(pSettings->annex)
        {
        case NEXUS_FrontendQamAnnex_eA:
            {
                switch ( pSettings->mode )
                {
                case NEXUS_FrontendQamMode_e16:
                    qamMode = BDCM_AdsModulationType_eAnnexAQam16;
                    break;
                case NEXUS_FrontendQamMode_e32:
                    qamMode = BDCM_AdsModulationType_eAnnexAQam32;
                    break;
                case NEXUS_FrontendQamMode_e64:
                    qamMode = BDCM_AdsModulationType_eAnnexAQam64;
                    break;
                case NEXUS_FrontendQamMode_e128:
                    qamMode = BDCM_AdsModulationType_eAnnexAQam128;
                    break;
                case NEXUS_FrontendQamMode_e256:
                    qamMode = BDCM_AdsModulationType_eAnnexAQam256;
                break;
                default:
                return;
                }
            }
            break;
        case NEXUS_FrontendQamAnnex_eB:
            {
                switch ( pSettings->mode )
                {
                case NEXUS_FrontendQamMode_e64:
                    qamMode = BDCM_AdsModulationType_eAnnexBQam64;
                    break;
                case NEXUS_FrontendQamMode_e256:
                    qamMode = BDCM_AdsModulationType_eAnnexBQam256;
                    break;
                default:
                return;
                }
            }
            break;
        case NEXUS_FrontendQamAnnex_eC:
            {
                switch ( pSettings->mode )
                {
                case NEXUS_FrontendQamMode_e64:
                    qamMode = BDCM_AdsModulationType_eAnnexCQam64;
                    break;
                case NEXUS_FrontendQamMode_e256:
                    qamMode = BDCM_AdsModulationType_eAnnexCQam256;
                    break;
                default:
                return;
                }
            }
            break;
        default:
            BDBG_WRN(("invalid annex mode %u",pSettings->annex));
            return;
        }

        if(pSettings->symbolRate)
        {
            params.symbolRate = pSettings->symbolRate;
        }
        else
        {
            params.symbolRate = NEXUS_Frontend_P_GetDefaultQamSymbolRate(pSettings->mode,
                                                                         pSettings->annex);
        }

        params.modType = qamMode;

        retCode = BDCM_Ads_AcquireChannel(hChannel->qam,
                                             &params);
        if (retCode!=BERR_SUCCESS)
        {
            BERR_TRACE(retCode); return;
        }
        hChannel->tuneStarted = true;
    }
    return;
}

void NEXUS_Docsis_P_QamLockStatus(void *pParam)
{
    NEXUS_TaskCallbackHandle lockStatusCallback = (NEXUS_TaskCallbackHandle)pParam;
    if(lockStatusCallback)
    {
        NEXUS_TaskCallback_Fire(lockStatusCallback);
    }
    return;
}

void NEXUS_Docsis_P_CheckQamTuneStatus(void *handle)
{
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    NEXUS_FrontendQamSettings *pSettings = NULL;
    BDCM_AdsLockStatus adsLockStatus;
    BERR_Code retCode = BERR_SUCCESS;

    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    pSettings = &hChannel->qamSettings;
    adsLockStatus = BDCM_AdsLockStatus_eUnlocked;

    BDBG_MSG(("firing QAM channel %u lockAppCallback",hChannel->dsChannelNum));
    NEXUS_TaskCallback_Fire(hChannel->lockAppCallback);
    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational) { return; };

    retCode = BDCM_Ads_GetChannelLockStatus(hChannel->qam,
                                               &adsLockStatus);
    if (retCode != BERR_SUCCESS)
    {
        BERR_TRACE(retCode);
        return;
    }

    if ((adsLockStatus == BDCM_AdsLockStatus_eUnlocked) && pSettings->autoAcquire
    && pSettings->acquisitionMode != NEXUS_FrontendQamAcquisitionMode_eScan
    )
    {
        if( hChannel->retuneTimer )
        {
            NEXUS_CancelTimer(hChannel->retuneTimer);
        }
        BDBG_MSG((" QAM channel %u unlocked - timer scheduled to reacquire",
                  hChannel->dsChannelNum));
        hChannel->retuneTimer= NEXUS_ScheduleTimer(2000,
                                                   NEXUS_Docsis_P_AutoTuneQam,
                                                   handle);
    }
    return;
}


NEXUS_Error NEXUS_Docsis_P_TuneOutOfBand(
    void *handle,
    const NEXUS_FrontendOutOfBandSettings *pSettings
    )
{

    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDCM_AobModulationType aobMod;
    BDCM_AobSpectrumMode aobSpectrum;

    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    BDBG_ASSERT(pSettings);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    if(hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_NOT_SUPPORTED);
    }

    rc = NEXUS_Frontend_P_SetMtsifConfig(hDevice->hOutOfBandChannel);
    if (rc) { return BERR_TRACE(rc); }

    switch (pSettings->mode)
    {
    case NEXUS_FrontendOutOfBandMode_eAnnexAQpsk:
        aobMod = BDCM_AobModulationType_eAnnexAQpsk;
        break;
    case NEXUS_FrontendOutOfBandMode_eDvs178Qpsk:
        aobMod = BDCM_AobModulationType_eDvs178Qpsk;
        break;
    case NEXUS_FrontendOutOfBandMode_ePod_AnnexAQpsk:
        aobMod = BDCM_AobModulationType_ePod_AnnexAQpsk;
        break;
    case NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk:
        aobMod = BDCM_AobModulationType_ePod_Dvs178Qpsk;
        break;
    default:
        return BERR_INVALID_PARAMETER;
    }

    switch (pSettings->spectrum)
    {
    case NEXUS_FrontendOutOfBandSpectrum_eAuto:
        aobSpectrum = BDCM_AobSpectrumMode_eAuto;
        break;
    case NEXUS_FrontendOutOfBandSpectrum_eInverted:
        aobSpectrum = BDCM_AobSpectrumMode_eInverted;
        break;
    case NEXUS_FrontendOutOfBandSpectrum_eNonInverted:
        aobSpectrum = BDCM_AobSpectrumMode_eNoInverted;
        break;
    default:
        return BERR_INVALID_PARAMETER;
    }

    NEXUS_TaskCallback_Set(hChannel->lockAppCallback, &(pSettings->lockCallback));

    if (hChannel->hTnr)
    {
        retCode = BDCM_Tnr_SetRfFreq(hChannel->hTnr,
                                     pSettings->frequency,
                                     BDCM_TnrMode_eDigital);
        if (retCode !=BERR_SUCCESS)
        {
            return BERR_TRACE(retCode);
        }
    }

    retCode = BDCM_Aob_SetChannelSpectrum(hChannel->outOfBand,
                                          aobSpectrum);
    if (retCode != BERR_SUCCESS)
    {
        return BERR_TRACE(retCode);
    }

    retCode = BDCM_Aob_AcquireChannel(hChannel->outOfBand,
                                         aobMod,
                                         pSettings->symbolRate);
    if (retCode != BERR_SUCCESS)
    {
        return BERR_TRACE(retCode);
    }
    hChannel->tuneStarted = true;
    hChannel->outOfBandSettings = *pSettings;
    return retCode;
}

void NEXUS_Docsis_P_OobLockStatus(
	void *pParam
	)
{
    NEXUS_TaskCallbackHandle lockStatusCallback = (NEXUS_TaskCallbackHandle)pParam;
    if(lockStatusCallback)
    {
        NEXUS_TaskCallback_Fire(lockStatusCallback);
    }
    return;
}

NEXUS_Error NEXUS_Docsis_P_GetOutOfBandStatus(
    void *handle,
    NEXUS_FrontendOutOfBandStatus *pStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDCM_AobStatus aobStatus;
    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    BDBG_ASSERT(pStatus);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_NOT_SUPPORTED);
    }

    retCode = BDCM_Aob_GetChannelStatus(hChannel->outOfBand,
                                        &aobStatus);
    if (retCode !=BERR_SUCCESS)
    {
        return BERR_TRACE(retCode);
    }
    pStatus->isFecLocked = aobStatus.isFecLock;
    pStatus->isQamLocked = aobStatus.isQamLock;
    pStatus->symbolRate = aobStatus.symbolRate;
    pStatus->snrEstimate = aobStatus.snrEstimate*100/256;
    pStatus->agcIntLevel = aobStatus.agcIntLevel;
    pStatus->agcExtLevel = aobStatus.agcExtLevel;
    pStatus->carrierFreqOffset = aobStatus.carrierFreqOffset;
    pStatus->carrierPhaseOffset = aobStatus.carrierPhaseOffset;
    pStatus->correctedCount = aobStatus.correctedCount;
    pStatus->uncorrectedCount = aobStatus.uncorrectedCount;
    pStatus->fdcChannelPower = aobStatus.fdcChannelPower;
    pStatus->berErrorCount = aobStatus.berErrorCount;

    pStatus->settings = hChannel->outOfBandSettings;

    BDBG_MSG(("OOB Status"));
    BDBG_MSG(("FEC lock:%d",aobStatus.isFecLock));
    BDBG_MSG(("QAM lock :%d",aobStatus.isQamLock));
    BDBG_MSG(("AGC internal level:%d",aobStatus.agcExtLevel));
    BDBG_MSG(("AGC externale level:%d",aobStatus.snrEstimate));
    BDBG_MSG(("FEC corrected count :%d",aobStatus.correctedCount));
    BDBG_MSG(("FEF uncorrected count :%d",aobStatus.uncorrectedCount));
    BDBG_MSG(("BER Count:%d",aobStatus.berErrorCount));
    BDBG_MSG(("channel power:%d",aobStatus.fdcChannelPower));
    return retCode;
}

NEXUS_Error NEXUS_Docsis_P_TuneUpstream(
    void *handle,
    const NEXUS_FrontendUpstreamSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDCM_AusOperationMode ausMode;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice = NULL;

    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    BDBG_ASSERT(pSettings);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    if (hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_NOT_SUPPORTED);
    }

    rc = NEXUS_Frontend_P_SetMtsifConfig(hDevice->hUpStreamChannel);
    if (rc) { return BERR_TRACE(rc); }

    switch (pSettings->mode)
    {
    case NEXUS_FrontendUpstreamMode_eAnnexA:
        ausMode = BDCM_AusOperationMode_eAnnexA;
        break;
    case NEXUS_FrontendUpstreamMode_eDvs178:
        ausMode = BDCM_AusOperationMode_eDvs178;
        break;
    case NEXUS_FrontendUpstreamMode_ePodAnnexA :
        ausMode = BDCM_AusOperationMode_ePodAnnexA;
        break;
    case NEXUS_FrontendUpstreamMode_ePodDvs178:
        ausMode = BDCM_AusOperationMode_ePodDvs178;
        break;
    case NEXUS_FrontendUpstreamMode_eDocsis:
        ausMode = BDCM_AusOperationMode_eDocsis;
        break;
    default:
        return BERR_NOT_SUPPORTED;
    }

    retCode = BDCM_Aus_SetChannelOperationMode(hChannel->upStream,
                                                  ausMode);
    if (retCode != BERR_SUCCESS)
    {
        return BERR_TRACE(retCode);
    }

    if (ausMode != BDCM_AusOperationMode_eDocsis)
    {
        retCode = BDCM_Aus_SetChannelRfFreq(hChannel->upStream,
                                               pSettings->frequency);
        if(retCode != BERR_SUCCESS)
        {
            return BERR_TRACE(retCode);
        }

        retCode = BDCM_Aus_SetChannelSymbolRate(hChannel->upStream,
                                                   pSettings->symbolRate);
        if(retCode != BERR_SUCCESS)
        {
            return BERR_TRACE(retCode);
        }

        retCode = BDCM_Aus_SetChannelPowerLevel(hChannel->upStream,
                                                  pSettings->powerLevel);
        if(retCode != BERR_SUCCESS)
         {
             return BERR_TRACE(retCode);
         }
    }
    hChannel->upStreamSettings = *pSettings;
    return retCode;
}

NEXUS_Error NEXUS_Docsis_P_GetUpstreamStatus(
    void *handle,
    NEXUS_FrontendUpstreamStatus *pStatus
    )
{

    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    BERR_Code  retCode = BERR_SUCCESS;
    struct BDCM_AusStatus ausStatus;
    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    BDBG_ASSERT(pStatus);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if(hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_NOT_SUPPORTED);
    }

    retCode = BDCM_Aus_GetChannelStatus(hChannel->upStream,
                                           &ausStatus);
    if(retCode != BERR_SUCCESS)
    {
        return BERR_TRACE(retCode);
    }

    switch (ausStatus.operationMode)
    {
        case BDCM_AusOperationMode_ePodAnnexA:
            pStatus->mode  = NEXUS_FrontendUpstreamMode_ePodAnnexA;
            break;
        case BDCM_AusOperationMode_ePodDvs178:
            pStatus->mode  = NEXUS_FrontendUpstreamMode_ePodDvs178;
            break;
        case BDCM_AusOperationMode_eAnnexA :
            pStatus->mode  = NEXUS_FrontendUpstreamMode_eAnnexA;
            break;
        case BDCM_AusOperationMode_eDvs178:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eDvs178;
            break;
        case BDCM_AusOperationMode_eDocsis:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eDocsis;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pStatus->frequency = ausStatus.rfFreq;
    pStatus->symbolRate = ausStatus.symbolRate;
    pStatus->powerLevel = ausStatus.powerLevel;
    pStatus->sysXtalFreq = ausStatus.sysXtalFreq;

    BDBG_MSG(("US Status"));
    BDBG_MSG(("operation mode %d",ausStatus.operationMode));
    BDBG_MSG(("RF freq %lu",ausStatus.rfFreq));
    BDBG_MSG(("symbol rate %lu",ausStatus.symbolRate));
    BDBG_MSG(("power level %d",ausStatus.powerLevel));
    BDBG_MSG(("crystal freq %d",ausStatus.sysXtalFreq));
    return retCode;
}

NEXUS_Error NEXUS_Docsis_P_TransmitDebugPacket(
   void *handle,
   NEXUS_FrontendDebugPacketType type,
   const uint8_t *pBuffer,
   size_t size)
{
     BERR_Code retCode = BERR_SUCCESS;
     NEXUS_Error rc = NEXUS_SUCCESS;
     NEXUS_DocsisChannelHandle hChannel = NULL;
     NEXUS_DocsisDeviceHandle hDevice = NULL;

     hChannel = (NEXUS_DocsisChannelHandle) handle;
     BDBG_OBJECT_ASSERT(hChannel, NEXUS_DocsisChannel);
     BDBG_ASSERT(pBuffer);
     hDevice = hChannel->hDevice;
     BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);

     if ((size != 54) || (type != NEXUS_FrontendDebugPacketType_eOob))
     {
         return NEXUS_INVALID_PARAMETER;
     }

     retCode = BDCM_Aus_TransmitChannelStarvuePkt(hChannel->upStream,
                                                     (uint8_t *)pBuffer,
                                                     size);
     if (retCode != BERR_SUCCESS)
     {
         BDBG_WRN(("transmission of debug packet failed"));
         rc = (retCode = BERR_OUT_OF_DEVICE_MEMORY)?
              NEXUS_OUT_OF_SYSTEM_MEMORY : NEXUS_INVALID_PARAMETER;
     }
     return rc;
}


NEXUS_Error NEXUS_Docsis_P_GetSoftDecisions(
    void *handle,
    NEXUS_FrontendSoftDecision *pDecisions,
    size_t length)
{
     BERR_Code retCode = BERR_SUCCESS;
     int16_t *data_i, *data_q;
     int16_t returnLength=0;
     int i=0;
     NEXUS_DocsisChannelHandle hChannel = NULL;
     NEXUS_DocsisDeviceHandle hDevice = NULL;

     hChannel = (NEXUS_DocsisChannelHandle) handle;
     BDBG_OBJECT_ASSERT(hChannel, NEXUS_DocsisChannel);
     BDBG_ASSERT(pDecisions);
     hDevice = hChannel->hDevice;
     BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);

    if(hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_NOT_SUPPORTED);
    }

    data_i = BKNI_Malloc(length*sizeof(int16_t));
    if(!data_i)
    {
        return NEXUS_OUT_OF_SYSTEM_MEMORY;
    }

    data_q = BKNI_Malloc(length*sizeof(int16_t));
    if(!data_q)
    {
        BKNI_Free(data_i);
        return NEXUS_OUT_OF_SYSTEM_MEMORY;
    }

    retCode = BDCM_Ads_GetChannelSoftDecision(hChannel->qam,
                                                 (int16_t)length,
                                                 data_i, data_q,
                                                 &returnLength);
    if (retCode != BERR_SUCCESS)
    {
        returnLength = length;
        goto error;
    }

    for(i=0;i<returnLength;i++)
    {
        pDecisions[i].i =  data_i[i]*2;
        pDecisions[i].q = data_q[i]*2;
    }

error:
    BKNI_Free(data_i);
    BKNI_Free(data_q);
    return retCode;
}

NEXUS_Error NEXUS_Docsis_P_ReapplyChannelTransportSettings(void *handle)
{
#if NEXUS_HAS_MXT
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    NEXUS_FrontendHandle hFrontend = NULL;

    hChannel = (NEXUS_DocsisChannelHandle) handle;
    BDBG_OBJECT_ASSERT(hChannel, NEXUS_DocsisChannel);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);

    switch(hChannel->settings.channelType)
    {
    case NEXUS_DocsisChannelType_eQam:
        hFrontend = hDevice->hDsChannel[hChannel->dsChannelNum];
        break;
    case NEXUS_DocsisChannelType_eOutOfBand:
        hFrontend = hDevice->hOutOfBandChannel;
        break;
    case NEXUS_DocsisChannelType_eUpstream:
        hFrontend = hDevice->hUpStreamChannel;
        break;
    case NEXUS_DocsisChannelType_eData:
        BDBG_MSG(("MTSIF/TSMF configuration not required for data channel %u",
                  hChannel->dsChannelNum));
        break;
        default:
        BDBG_WRN(("%s:unknown channel type",__FUNCTION__));
    }

    if (hFrontend) {
        rc = NEXUS_Frontend_P_SetMtsifConfig(hFrontend);
        if (rc) { return BERR_TRACE(rc); }
    }
#else
    BSTD_UNUSED(handle);
#endif
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Docsis_P_ChannelStandby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice = NULL;
    BDCM_DevicePowerMode powerMode;
    hChannel = (NEXUS_DocsisChannelHandle) handle;
    BDBG_OBJECT_ASSERT(hChannel, NEXUS_DocsisChannel);
    BDBG_ASSERT(pSettings);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);

    BSTD_UNUSED(enabled);

   if (hDevice->standbyMode == pSettings->mode)
    {
        BDBG_WRN(("no change in the standby mode %d",pSettings->mode));
    	return NEXUS_SUCCESS;
    }

   if ((pSettings->mode == NEXUS_StandbyMode_ePassive) || (pSettings->mode == NEXUS_StandbyMode_eDeepSleep))
   {
       powerMode = BRPC_ECM_PowerMode_Standby1;
   }
   else
   {
       if ((pSettings->mode == NEXUS_StandbyMode_eActive) || (pSettings->mode == NEXUS_StandbyMode_eOn))
       {
           powerMode = BRPC_ECM_PowerMode_On;
       }
       else
       {
           BDBG_ERR((" unsupported standby mode"));
           return NEXUS_INVALID_PARAMETER;
       }
   }

   retCode = BDCM_SetDevicePowerMode(hDevice->hDocsis,powerMode);

   if(retCode != BERR_SUCCESS)
   {
       BDBG_ERR(("unable to transistion to standby mode %u",pSettings->mode));
       return NEXUS_INVALID_PARAMETER;
   }
   hDevice->standbyMode = pSettings->mode;
   return retCode;
}

void NEXUS_Docsis_P_GetType(void *handle,
                            NEXUS_FrontendType *type)
{
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);
    BKNI_Memset(type, 0, sizeof(NEXUS_FrontendType));
    /* assigning chip ID as family ID */
    type->chip.familyId = hDevice->version.majVer >> 16;
    type->chip.id = hDevice->version.majVer >> 16;
    /* refer to NEXUS_DOCSIS_P_GetChipRev for revID definitions*/
    type->chip.version.major = hDevice->version.majVer & 0x0000000f;
    type->chip.version.minor = 0;
    return;
}

NEXUS_Error NEXUS_Docsis_P_GetQamScanStatus(
    void *handle,
    NEXUS_FrontendQamScanStatus *pStatus
    )
{
    BERR_Code retCode;
    BDCM_Ads_ScanStatus adsScanStatus;
    NEXUS_DocsisChannelHandle hChannel = NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    hChannel = (NEXUS_DocsisChannelHandle)handle;
    BDBG_OBJECT_ASSERT(hChannel,NEXUS_DocsisChannel);
    BDBG_ASSERT(pStatus);
    hDevice = hChannel->hDevice;
    BDBG_OBJECT_ASSERT(hDevice,NEXUS_DocsisDevice);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if(hDevice->status.state != NEXUS_DocsisDeviceState_eOperational)
    {
        return (NEXUS_NOT_SUPPORTED);
    }

    retCode = BDCM_Ads_GetScanStatus(hChannel->qam, &adsScanStatus);
    if (retCode != BERR_SUCCESS) { return BERR_TRACE(retCode);}

    pStatus->symbolRate = adsScanStatus.symbolRate;
    pStatus->frequencyOffset = adsScanStatus.carrierFreqOffset;

    switch(adsScanStatus.modType){
        case BDCM_AdsModulationType_eAnnexAQam16:
        case BDCM_AdsModulationType_eAnnexBQam16:
        case BDCM_AdsModulationType_eAnnexCQam16:
            pStatus->mode = NEXUS_FrontendQamMode_e16;
            break;
        case BDCM_AdsModulationType_eAnnexAQam32:
        case BDCM_AdsModulationType_eAnnexBQam32:
        case BDCM_AdsModulationType_eAnnexCQam32:
            pStatus->mode = NEXUS_FrontendQamMode_e32;
            break;
        case BDCM_AdsModulationType_eAnnexAQam64:
        case BDCM_AdsModulationType_eAnnexBQam64:
        case BDCM_AdsModulationType_eAnnexCQam64:
            pStatus->mode = NEXUS_FrontendQamMode_e64;
            break;
        case BDCM_AdsModulationType_eAnnexAQam128:
        case BDCM_AdsModulationType_eAnnexBQam128:
        case BDCM_AdsModulationType_eAnnexCQam128:
            pStatus->mode = NEXUS_FrontendQamMode_e128;
            break;
        case BDCM_AdsModulationType_eAnnexAQam256:
        case BDCM_AdsModulationType_eAnnexBQam256:
        case BDCM_AdsModulationType_eAnnexCQam256:
            pStatus->mode = NEXUS_FrontendQamMode_e256;
            break;
        case BDCM_AdsModulationType_eAnnexAQam512:
        case BDCM_AdsModulationType_eAnnexBQam512:
        case BDCM_AdsModulationType_eAnnexCQam512:
            pStatus->mode = NEXUS_FrontendQamMode_e512;
            break;
        case BDCM_AdsModulationType_eAnnexAQam1024:
        case BDCM_AdsModulationType_eAnnexBQam1024:
        case BDCM_AdsModulationType_eAnnexCQam1024:
            pStatus->mode = NEXUS_FrontendQamMode_e1024;
            break;
        case BDCM_AdsModulationType_eAnnexAQam2048:
        case BDCM_AdsModulationType_eAnnexBQam2048:
        case BDCM_AdsModulationType_eAnnexCQam2048:
            pStatus->mode = NEXUS_FrontendQamMode_e2048;
            break;
        case BDCM_AdsModulationType_eAnnexAQam4096:
        case BDCM_AdsModulationType_eAnnexBQam4096:
        case BDCM_AdsModulationType_eAnnexCQam4096:
            pStatus->mode = NEXUS_FrontendQamMode_e4096;
            break;
        default:
            pStatus->mode = NEXUS_FrontendQamMode_eMax;
            break;
        }

    if(adsScanStatus.modType <= BDCM_AdsModulationType_eAnnexAQam4096)
        pStatus->annex =NEXUS_FrontendQamAnnex_eA;
    else if(adsScanStatus.modType <= BDCM_AdsModulationType_eAnnexBQam4096)
        pStatus->annex =NEXUS_FrontendQamAnnex_eB;
    else if(adsScanStatus.modType <= BDCM_AdsModulationType_eAnnexCQam4096)
        pStatus->annex =NEXUS_FrontendQamAnnex_eC;
    else
        pStatus->annex =NEXUS_FrontendQamAnnex_eMax;


    pStatus->interleaver = adsScanStatus.interleaver;
    pStatus->spectrumInverted = adsScanStatus.isSpectrumInverted;
    pStatus->acquisitionStatus = adsScanStatus.acquisitionStatus;

    BDBG_MSG(("DOCSIS scan status"));
    return retCode;
}
