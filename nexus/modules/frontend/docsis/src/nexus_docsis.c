/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/

#include "nexus_frontend_module.h"
#include "nexus_platform_features.h"
#include "nexus_docsis_priv.h"
#include "priv/nexus_transport_priv.h"

BDBG_MODULE(nexus_docsis);
BDBG_OBJECT_ID(NEXUS_DocsisDevice);
BDBG_OBJECT_ID(NEXUS_DocsisChannel);

#define EXTERNAL_FRONTEND_NUMBER_OFFSET 16

#if NEXUS_HAS_MXT
BMXT_ChipRev NEXUS_DOCSIS_P_GetChipRev(uint8_t rawChipRev)
{
    BMXT_ChipRev chipRev = BMXT_ChipRev_eMax;
    switch(rawChipRev)
    {
    case 0x0:
        {
            BDBG_WRN(("DOCSIS Chip Rev: A0"));
            chipRev = BMXT_ChipRev_eA0;
        }
        break;
    case 0x1:
        {
            BDBG_WRN(("DOCSIS Chip Rev: A1"));
            chipRev = BMXT_ChipRev_eA1;
        }
        break;
    case 0x2:
        {
            BDBG_WRN(("DOCSIS Chip Rev: A2"));
            chipRev = BMXT_ChipRev_eA2;
        }
        break;
    case 0x8:
        {
            BDBG_WRN(("DOCSIS Chip Rev: B0"));
            chipRev = BMXT_ChipRev_eB0;
        }
        break;
	case 0x10: /*Case of 3390 */
        {
            BDBG_WRN(("3390 DOCSIS Chip Rev: B0"));
            chipRev = BMXT_ChipRev_eB0;
        }
        break;
    case 0x9:
        {
            BDBG_WRN(("DOCSIS Chip Rev: B1"));
            chipRev = BMXT_ChipRev_eB1;
        }
        break;
    case 0xa:
        {
            BDBG_WRN(("DOCSIS Chip Rev: B2"));
            chipRev = BMXT_ChipRev_eB2;
        }
        break;
    default:
        BDBG_ERR(("invalid DOCSIS Chip Revision"));
    }
    return chipRev;
}
#endif

void NEXUS_Docsis_GetDefaultOpenDeviceSettings(NEXUS_DocsisOpenDeviceSettings *pOpenSettings)
{
    BKNI_Memset(pOpenSettings,0,sizeof(*pOpenSettings));
    pOpenSettings->rpcTimeOut = 50;
    return;
}

NEXUS_FrontendDeviceHandle NEXUS_Docsis_OpenDevice(
   unsigned index,
   const NEXUS_DocsisOpenDeviceSettings *pOpenSettings)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t numDsChannels, numDataChannels;
    BDCM_DeviceSettings deviceSettings;
    NEXUS_ThreadSettings threadSettings;
    NEXUS_FrontendDeviceHandle hFrontendDevice=NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    uint16_t chipId;
    uint8_t rawChipRev;
    bool has_3128 = false, mtsif_chained = false;

    BDBG_ASSERT(pOpenSettings);
    BSTD_UNUSED(index);

	hFrontendDevice = NEXUS_FrontendDevice_P_Create();
	if (!hFrontendDevice)
	{
		retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto errorAlloc;
	}

    hDevice = BKNI_Malloc(sizeof(NEXUS_DocsisDevice));
    if (!hDevice )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto errorAlloc;
    }
    BKNI_Memset(hDevice, 0, sizeof(NEXUS_DocsisDevice));
    BDBG_OBJECT_SET(hDevice, NEXUS_DocsisDevice);

    deviceSettings.rpcTimeout= pOpenSettings->rpcTimeOut;
    hDevice->hDocsis = BDCM_OpenDevice(&deviceSettings);
    if(!hDevice->hDocsis) goto errorAlloc;

    retCode = BDCM_InitDevice(hDevice->hDocsis);
    if ( retCode != BERR_SUCCESS ) goto errorInit;

    /*
     * DOCSIS device is active. Close the device
     * and open it back with 3 seconds RPC timeout
     * for reliable connection between host
     * and DOCSIS
     */
    BDCM_CloseDevice(hDevice->hDocsis);

    deviceSettings.rpcTimeout= DOCSIS_OPERATIONAL_RPC_TIMEOUT;
    hDevice->hDocsis = BDCM_OpenDevice(&deviceSettings);
    if(!hDevice->hDocsis) goto errorInit;

    /* Retrieve Chip Id and Rev */
    retCode = BDCM_GetDeviceVersion(hDevice->hDocsis,&hDevice->version);
    if (retCode) goto errorInit;

    chipId = (uint16_t)(hDevice->version.majVer >> 16);
	#if NEXUS_USE_3390_VMS
	rawChipRev = (uint8_t)(hDevice->version.majVer & 0x000000ff);
	#else
    rawChipRev = (uint8_t)(hDevice->version.majVer & 0x0000000f);
	#endif
    BDBG_WRN(("DOCSIS ChipId:0x%x rawChipRev::0x%x",chipId, rawChipRev));
    BDBG_WRN(("DOCSIS Version major:0x%x minor:0x%x", hDevice->version.majVer, hDevice->version.minVer));

#ifdef NEXUS_FRONTEND_REVERSE_RMAGNUM_SUPPORT
    BDBG_WRN(("Reverse RMagnum enabled."));
    hDevice->hasMxt = false;
    hDevice->dataTuner = pOpenSettings->dataTuner[0];
    hDevice->pGenericDeviceHandle = hFrontendDevice;
    BKNI_CreateEvent(&hDevice->tunerStatusEvent);
    BKNI_ResetEvent(hDevice->tunerStatusEvent);
    BSTD_UNUSED(numDsChannels);
    BSTD_UNUSED(numDataChannels);
    BSTD_UNUSED(mtsif_chained);
#else
    retCode = BDCM_GetDeviceTotalDsChannels(hDevice->hDocsis, &numDsChannels);
    if (retCode!=BERR_SUCCESS) goto errorInit;

    if (numDsChannels >> EXTERNAL_FRONTEND_NUMBER_OFFSET) {
        has_3128 = true;
#if BCHP_CHIP==7445 || BCHP_CHIP==7439
        mtsif_chained = 1; /* 3128 via MTSIF can be daisy-chained or connected directly to host */ /* TODO: not a good practice of (BCHP_CHIP==7445), need from platfrom info passed here to decide*/
#endif
        BDBG_WRN(("numDSChannels:0x%x, chained", numDsChannels));
        numDsChannels &= 0xff;
    }

    hDevice->numChannels = numDsChannels;
    hDevice->numDsChannels = numDsChannels;
    retCode = BDCM_GetDeviceBondingCapability(hDevice->hDocsis, &numDataChannels);
    if (retCode!=BERR_SUCCESS) goto errorInit;

    BDBG_WRN(("numDSChannels:0x%x", numDsChannels));
    BDBG_WRN(("numDataChannels:0x%x",numDataChannels));
    BDBG_WRN(("numVideoChannels:0x%x",numDsChannels-numDataChannels));
    hDevice->numDataChannels = numDataChannels;
    hDevice->pGenericDeviceHandle = hFrontendDevice;

#if NEXUS_HAS_MXT
    {
        /* open MXT */
        BMXT_Settings mxtSettings;
        BMXT_ChipRev chipRev;
        unsigned i;
        hDevice->hasMxt = true;
        if (hDevice->version.minVer <= 0x9) {
            BMXT_3383_GetDefaultSettings(&mxtSettings);
        }
        else {
            BMXT_3384_GetDefaultSettings(&mxtSettings);
            mxtSettings.MtsifTxCfg[0].TxClockPolarity = 0;
            mxtSettings.MtsifTxCfg[1].TxClockPolarity = 0;
        }

        for (i=0; i<BMXT_MAX_NUM_MTSIF_TX; i++) {
            mxtSettings.MtsifTxCfg[i].Enable = true;
            NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
            mxtSettings.MtsifTxCfg[i].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
            NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        }

        if (mtsif_chained) {
            mxtSettings.MtsifRxCfg[0].Enable = true; /* must enable MTSIF_RX for 3128 bypass */
            mxtSettings.MtsifRxCfg[0].Decrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
            mxtSettings.MtsifRxCfg[0].RxClockPolarity = 1;
        }

        mxtSettings.hRpc = hDevice->hDocsis->hRpc;
        switch(chipId) {
            case 0x3383: mxtSettings.chip = BMXT_Chip_e3383; break;
            case 0x3843: mxtSettings.chip = BMXT_Chip_e3384; break;
            case 0x3384: mxtSettings.chip = BMXT_Chip_e3384; break;
			case 0x3390: mxtSettings.chip = BMXT_Chip_e3390; break;
            case 0x7145:
            case 0x3385:
                mxtSettings.chip = BMXT_Chip_e7145; break; /* TODO: is chipId 0x7145 for 7145? */
            default:
                BDBG_ERR(("DOCSIS chipId:0x%x not handled", chipId));
                goto errorInit;
        }
        chipRev = NEXUS_DOCSIS_P_GetChipRev(rawChipRev);
        switch (chipRev) {
            case BMXT_ChipRev_eA0: mxtSettings.chipRev = BMXT_ChipRev_eA0; break;
            case BMXT_ChipRev_eA1: mxtSettings.chipRev = BMXT_ChipRev_eA1; break;
            case BMXT_ChipRev_eA2: mxtSettings.chipRev = BMXT_ChipRev_eA2; break;
            case BMXT_ChipRev_eB0: mxtSettings.chipRev = BMXT_ChipRev_eB0; break;
            /* TODO: does this correctly return A0 for 7145? */
            default:
                BDBG_ERR(("DOCSIS rawChip:0x%x capabilities not known", rawChipRev));
                goto errorInit;
        }
        hDevice->numTsmfParsers = BMXT_GetNumResources(mxtSettings.chip, mxtSettings.chipRev, BMXT_ResourceType_eTsmf);

        {
            BMXT_Handle mxt_docsis, mxt_3128;

            retCode = BMXT_Open(&mxt_docsis, NULL, NULL, &mxtSettings);
            if (retCode!=BERR_SUCCESS) goto errorInit;

            hDevice->pGenericDeviceHandle->mtsifConfig.mxt = mxt_docsis;

            retCode = NEXUS_Frontend_P_InitMtsifConfig(&hDevice->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
            if (retCode!=BERR_SUCCESS) goto errorInit;

			#if NEXUS_USE_3390_VMS
			for (i=0; i < hDevice->numTsmfParsers; i++)
			{
				BMXT_ParserConfig pConfig;
                BMXT_GetParserConfig(mxt_docsis, i, &pConfig);
				/* In 3390, first 8 channels always go to MTSIF0 */
				if (i < hDevice->numTsmfParsers/2)
					pConfig.mtsifTxSelect = 0;
				else
					pConfig.mtsifTxSelect = 1;
                BMXT_SetParserConfig(mxt_docsis, i, &pConfig);
            }
			#endif

            if (has_3128) {
                BMXT_3128_GetDefaultSettings(&mxtSettings);
                mxtSettings.chipRev = BMXT_ChipRev_eC0;
                mxtSettings.hVirtual = mxt_docsis; /* 3128 MXT handle is always virtual */
                mxtSettings.MtsifTxCfg[0].TxClockPolarity = 1; /* for 3128-family-based chips, TX polarity has the opposite meaning as usual */
                mxtSettings.MtsifTxCfg[0].Enable = true;
                NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
                mxtSettings.MtsifTxCfg[0].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
                NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);

                retCode = BMXT_Open(&mxt_3128, NULL, NULL, &mxtSettings);
                if (retCode != BERR_SUCCESS) goto errorInit;

                /* one FrontendDevice but two MtsifConfigs */
                hDevice->pGenericDeviceHandle->chainedConfig = BKNI_Malloc(sizeof(NEXUS_FrontendDeviceMtsifConfig));
                if (hDevice->pGenericDeviceHandle->chainedConfig==NULL) {
                    goto errorInit;
                }
                BKNI_Memset(hDevice->pGenericDeviceHandle->chainedConfig, 0, sizeof(NEXUS_FrontendDeviceMtsifConfig));

                hDevice->pGenericDeviceHandle->chainedConfig->mxt = mxt_3128;

                retCode = NEXUS_Frontend_P_InitMtsifConfig(hDevice->pGenericDeviceHandle->chainedConfig, &mxtSettings);
                if (retCode!=BERR_SUCCESS) goto errorInit;

                /* we are always RPC-chained, but MTSIF-chained is optional */
                if (mtsif_chained) {
                    hDevice->pGenericDeviceHandle->chainedConfig->slave = true;
                }
            }
        }
    }
#else
    hDevice->hasMxt = false;
#endif
#endif

    hDevice->statusCallback = NEXUS_TaskCallback_Create(hDevice, NULL);
    if (!hDevice->statusCallback )
    {
        goto errorInit;
    }

    hDevice->notificationEnabled = true;
    NEXUS_Thread_GetDefaultSettings(&threadSettings);
    hDevice->notificationThread = NEXUS_Thread_Create("docsis_notification",
                                                      NEXUS_Docsis_P_NotificationThread,
                                                      (void*)hDevice,
                                                      &threadSettings);
    if (!hDevice->notificationThread)
    {
        BDBG_ERR((" can't create DOCSIS notification thread"));
        goto errorInit;
    }

    hDevice->heartBeatEnabled = true;
    NEXUS_Thread_GetDefaultSettings(&threadSettings);
    BKNI_CreateEvent(&hDevice->heartBeatEvent);
    BKNI_ResetEvent(hDevice->heartBeatEvent);
    hDevice->status.state = NEXUS_DocsisDeviceState_eOperational;
    hDevice->heartBeatThread = NEXUS_Thread_Create("docsis_heartbeat",
                                                  NEXUS_Docsis_P_HeartBeatThread,
                                                  (void*)hDevice,
                                                  &threadSettings);

    if (!hDevice->heartBeatThread)
    {
        BDBG_ERR((" can't create DOCSIS_Heartbeat thread"));
        goto errorInit;
    }
    hFrontendDevice->pDevice = (void *) hDevice;
    hFrontendDevice->application = NEXUS_FrontendDeviceApplication_eCable;
    hFrontendDevice->close = NEXUS_Docsis_P_CloseDevice;
    hFrontendDevice->getStatus = NEXUS_Docsis_P_GetStatus;
    #if defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)
    hFrontendDevice->getDocsisLnaDeviceAgcValue = NEXUS_Docsis_P_GetDocsisLnaDeviceAgcValue;
    hFrontendDevice->setHostChannelLockStatus = NEXUS_Docsis_P_SetHostChannelLockStatus;
    #endif
    #if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    hDevice->outOfBandChannelEnabled = true;
    hDevice->numChannels +=1;
    hDevice->upStreamEnabled = true;
    hDevice->numChannels +=1;
    #endif
    BDBG_MSG(("hFrontendDevice %#lx hDevice %#lx, RPC handle %#lx",
              (unsigned long) hFrontendDevice,
              (unsigned long) hDevice,
              (unsigned long) hDevice->hDocsis->hRpc));
    return hFrontendDevice;
errorInit:
    hDevice->status.state = NEXUS_DocsisDeviceState_eUninitialized;

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

    if (hDevice->notificationThread)
    {
        /* unlock module to unblock the DOCSIS notification thread */
        NEXUS_UnlockModule();
        hDevice->notificationEnabled = false;
        /* time for task to finish */
        BKNI_Sleep(600);
        NEXUS_Thread_Destroy(hDevice->notificationThread);
        /* lock the module */
        NEXUS_LockModule();
    }

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

    #if NEXUS_HAS_MXT
    if (has_3128) {
        if (hDevice->pGenericDeviceHandle->chainedConfig->mxt)
        {
            BMXT_Close(hDevice->pGenericDeviceHandle->chainedConfig->mxt);
        }
        if (hDevice->pGenericDeviceHandle->chainedConfig)
        {
            BKNI_Free(hDevice->pGenericDeviceHandle->chainedConfig);
        }
    }

    if (hDevice->pGenericDeviceHandle->mtsifConfig.mxt)
    {
        BMXT_Close(hDevice->pGenericDeviceHandle->mtsifConfig.mxt);
    }
    #endif

    if(hDevice->hDocsis)
    {
        BDCM_CloseDevice(hDevice->hDocsis);
    }
errorAlloc:
    if(hDevice)
    {
        BKNI_Free(hDevice);
    }
    if(hFrontendDevice)
    {
        BKNI_Free(hFrontendDevice);
    }
    BDBG_ERR(("hDOCSISDevice is null"));
    return NULL;
}

void NEXUS_Docsis_GetDeviceCapabilities(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisDeviceCapabilities *pCapabilities
    )
{
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pCapabilities);

    BKNI_Memset(pCapabilities, 0, sizeof(NEXUS_DocsisDeviceCapabilities));

    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);
    pCapabilities->numDataChannels = hDevice->numDataChannels;
    pCapabilities->numQamChannels = hDevice->numDsChannels-hDevice->numDataChannels;
    pCapabilities->totalChannels = hDevice->numChannels;
    pCapabilities->numTsmfParsers = hDevice->numTsmfParsers;
    pCapabilities->isMtsif = hDevice->hasMxt;
    if(hDevice->outOfBandChannelEnabled)
    {
        pCapabilities->numOutOfBandChannels =1;
    }
    if(hDevice->upStreamEnabled)
    {
        pCapabilities->numUpStreamChannels =1;
    }
    return;
}

void NEXUS_Docsis_GetDeviceSettings(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisDeviceSettings *pSettings
    )
{
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);
    pSettings->stateChange = hDevice->settings.stateChange;
    return;
}

NEXUS_Error NEXUS_Docsis_SetDeviceSettings(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_DocsisDeviceSettings *pSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);
    if (hDevice->statusCallback)
    {
        NEXUS_TaskCallback_Set(hDevice->statusCallback, &(pSettings->stateChange));
    }
    return rc;
}


NEXUS_Error NEXUS_Docsis_GetDeviceStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisDeviceStatus *pStatus
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pStatus);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);

    pStatus->state = hDevice->status.state;
    return rc;
}

void NEXUS_Docsis_GetDefaultOpenChannelSettings(
    NEXUS_DocsisOpenChannelSettings *pOpenSettings
    )
{
    BDBG_ASSERT(pOpenSettings);
    BKNI_Memset(pOpenSettings,0,sizeof(*pOpenSettings));
    pOpenSettings->channelNum = 0;
    pOpenSettings->autoAcquire = false;
    pOpenSettings->fastAcquire = false;
    pOpenSettings->enableFEC = false; /* cable card does the FEC by default */
    pOpenSettings->channelType = NEXUS_DocsisChannelType_eQam;
    return;
}

NEXUS_FrontendHandle NEXUS_Docsis_OpenChannel(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_DocsisOpenChannelSettings *pOpenSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_FrontendHandle hFrontend=NULL;
    NEXUS_DocsisChannelHandle hChannel=NULL;
    NEXUS_DocsisDeviceHandle hDevice=NULL;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pOpenSettings);

    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);
    BDBG_ASSERT(pOpenSettings->channelNum<= hDevice->numChannels);

    hChannel = (NEXUS_DocsisChannelHandle)BKNI_Malloc(sizeof(NEXUS_DocsisChannel));
    if (!hChannel)
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto errorAlloc;
    }
    BKNI_Memset((void *)hChannel,0,sizeof(NEXUS_DocsisChannel));
    BDBG_OBJECT_SET(hChannel, NEXUS_DocsisChannel);
    BKNI_Memcpy((void *)&hChannel->settings,(void*)pOpenSettings,sizeof(*pOpenSettings));
    hChannel->hDevice = hDevice;

    BDBG_MSG(("channelType %u channelNum :%u",pOpenSettings->channelType,pOpenSettings->channelNum));

    if (pOpenSettings->channelType == NEXUS_DocsisChannelType_eQam)
    {
        BDCM_TnrSettings tnrSettings;
        BDCM_AdsSettings adsSettings;
        BDBG_MSG(("QAM channel"));
        if(pOpenSettings->channelNum < hDevice->numDataChannels
           && hDevice->hDsChannel[pOpenSettings->channelNum])
        {
            BDBG_ERR(("%s:Either dsChannel is already used or it's a data channel",__FUNCTION__));
            goto error;
        }

        hFrontend = NEXUS_Frontend_P_Create(hChannel);
        if(!hFrontend)
        {
            retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto error;
        }

        BDCM_Tnr_GetDefaultSettings(&tnrSettings);
        tnrSettings.ifFreq = BDCM_TNR_IFFREQ;
        tnrSettings.type = BDCM_TnrType_eAds;
        tnrSettings.adsTunerNum = pOpenSettings->channelNum;
        tnrSettings.minVer = hDevice->version.minVer;
        hChannel->hTnr = BDCM_Tnr_Open(hDevice->hDocsis, &tnrSettings);
        if (!hChannel->hTnr)
        {
            BDBG_ERR(("%s: tuner open failed",__FUNCTION__));
            goto error;
        }

        BDCM_Ads_GetChannelDefaultSettings(&adsSettings);
        adsSettings.autoAcquire = hChannel->settings.autoAcquire;
        adsSettings.fastAcquire = hChannel->settings.fastAcquire;
        adsSettings.minVer = hDevice->version.minVer;
        hChannel->qam = BDCM_Ads_OpenChannel(hDevice->hDocsis,
                                                hChannel->settings.channelNum,
                                                &adsSettings);
        if (!hChannel->qam)
        {
            BDBG_ERR(("%s: demod open failed",__FUNCTION__));
            goto error;
        }

        hChannel->lockDriverCallback = NEXUS_TaskCallback_Create(hFrontend, NULL);
        if (!hChannel->lockDriverCallback)
        {
            BDBG_ERR(("%s:lockDriverCallback create failed",__FUNCTION__));
            goto error;
        }

        hChannel->lockAppCallback = NEXUS_TaskCallback_Create(hFrontend, NULL);
        if (!hChannel->lockAppCallback)
        {
             BDBG_ERR(("%s: lockAppCallback create failed",__FUNCTION__));
             goto error;
        }

        hChannel->asyncStatusAppCallback = NEXUS_TaskCallback_Create(hFrontend, NULL);
        if (!hChannel->asyncStatusAppCallback)
        {
            BDBG_ERR(("%s: asyncStatusAppCallback create failed",__FUNCTION__));
            goto error;
        }

        BDCM_Ads_InstallChannelCallback(hChannel->qam, BDCM_AdsCallback_eLockChange,
                                           (BDCM_AdsCallbackFunc)NEXUS_Docsis_P_QamLockStatus,
                                           (void*)hChannel->lockDriverCallback);

        NEXUS_CallbackHandler_Init(hChannel->lockDriverCBHandler, NEXUS_Docsis_P_CheckQamTuneStatus,(void *)hChannel);

        hFrontend->capabilities.docsis = false;
        hFrontend->capabilities.qam = true;
        hFrontend->capabilities.outOfBand = false;
        hFrontend->capabilities.upstream = false;
        BKNI_Memset(hFrontend->capabilities.qamModes, true, sizeof(hFrontend->capabilities.qamModes));

        hFrontend->close = NEXUS_Docsis_P_CloseChannel;
        /*
         * BCM3384 doesn't support deep sleep mode (S3) so disable the standby mode for now.
         */
        hFrontend->standby = NULL; /*NEXUS_Docsis_P_ChannelStandby;*/
        hFrontend->tuneQam = NEXUS_Docsis_P_TuneQam;
        hFrontend->getQamStatus = NEXUS_Docsis_P_GetQamStatus;
        hFrontend->requestQamAsyncStatus = NEXUS_Docsis_P_RequestQamAsyncStatus;
        hFrontend->getQamAsyncStatus = NEXUS_Docsis_P_GetQamAsyncStatus;
        hFrontend->getFastStatus = NEXUS_Docsis_P_GetFastStatus;
        hFrontend->getSoftDecisions= NEXUS_Docsis_P_GetSoftDecisions;
        hFrontend->untune = NEXUS_Docsis_P_Untune;
        hFrontend->getQamScanStatus = NEXUS_Docsis_P_GetQamScanStatus;
        if(hDevice->hasMxt)
        {
            hFrontend->reapplyTransportSettings = NEXUS_Docsis_P_ReapplyChannelTransportSettings;
        }
        hChannel->dsChannelNum = pOpenSettings->channelNum;
        hDevice->hDsChannel[pOpenSettings->channelNum] = hFrontend;
        hFrontend->pDeviceHandle = hChannel;
        hFrontend->pGenericDeviceHandle = handle;
        BDBG_MSG(("QAM channel %u configured",hChannel->dsChannelNum));
    }
    else
    {
        if(pOpenSettings->channelType == NEXUS_DocsisChannelType_eOutOfBand)
        {
            /* DOCSIS OOB channel */
            BDCM_AobSettings aobSettings;
            BDCM_TnrSettings tnrSettings;
            BDBG_MSG(("OOB channel"));
            if(!hDevice->outOfBandChannelEnabled)
            {
                BDBG_ERR(("DOCSIS device doesn't have an OOB channel"));
                goto error;
            }

            hFrontend = NEXUS_Frontend_P_Create(hChannel);
            if(!hFrontend)
            {
              retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
              goto error;
            }
            BDCM_Tnr_GetDefaultSettings(&tnrSettings);
            tnrSettings.type = BDCM_TnrType_eAob;
            tnrSettings.ifFreq = BDCM_TNR_IFFREQ;
            tnrSettings.minVer = hDevice->version.minVer;
            hChannel->hTnr = BDCM_Tnr_Open(hDevice->hDocsis,&tnrSettings);
            if(!hChannel->hTnr)
            {
                BDBG_ERR(("%s: OOB demod open failed",__FUNCTION__));
                goto error;
            }
            BDCM_Aob_GetChannelDefaultSettings(&aobSettings);
            aobSettings.enableFEC = hChannel->settings.enableFEC;
            aobSettings.ifFreq = BDCM_AOB_IFFREQ;
            aobSettings.spectrum = BDCM_AobSpectrumMode_eAuto;
            hChannel->outOfBand = BDCM_Aob_OpenChannel(hDevice->hDocsis,&aobSettings);
            if(!hChannel->outOfBand)
            {
                BDBG_ERR(("%s: OOB demod open failed",__FUNCTION__));
                goto error;
            }

            hChannel->lockAppCallback = NEXUS_TaskCallback_Create(hFrontend, NULL);
            if (!hChannel->lockAppCallback)
            {
                 BDBG_ERR(("%s: lockAppCallback create failed",__FUNCTION__));
                 goto error;
            }

            BDCM_Aob_InstallChannelCallback(hChannel->outOfBand, BDCM_AobCallback_eLockChange,
                        (BDCM_AobCallbackFunc)NEXUS_Docsis_P_OobLockStatus, (void*)hChannel->lockAppCallback);
            hFrontend->capabilities.docsis = false;
            hFrontend->capabilities.qam = false;
            hFrontend->capabilities.outOfBand = true;
            hFrontend->capabilities.upstream = false;
            BKNI_Memset(hFrontend->capabilities.outOfBandModes, true, sizeof(hFrontend->capabilities.outOfBandModes));

            hFrontend->close = NEXUS_Docsis_P_CloseChannel;
            /*
             * BCM3384 doesn't support deep sleep mode (S3) so disable the standby mode for now.
             */
            hFrontend->standby = NULL; /*NEXUS_Docsis_P_ChannelStandby;*/
            hFrontend->tuneOutOfBand = NEXUS_Docsis_P_TuneOutOfBand;
            hFrontend->getOutOfBandStatus = NEXUS_Docsis_P_GetOutOfBandStatus;
            hFrontend->getFastStatus = NEXUS_Docsis_P_GetFastStatus;
            hFrontend->untune = NEXUS_Docsis_P_Untune;

            hDevice->hOutOfBandChannel = hFrontend;
            hFrontend->pDeviceHandle = hChannel;
            hFrontend->pGenericDeviceHandle = handle;
            BDBG_MSG(("OOB channel configured "));
        }
        else
        {
            if(pOpenSettings->channelType == NEXUS_DocsisChannelType_eUpstream)
            {
                BDCM_AusSettings ausSettings;
                BDBG_MSG(("US channel"));
                if(!hDevice->upStreamEnabled)
                {
                    BDBG_ERR(("DOCSIS device doesn't have an upStream channel"));
                    goto error;
                }

                hFrontend = NEXUS_Frontend_P_Create(hChannel);
                if(!hFrontend)
                {
                    retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                    goto error;
                }

                BDCM_Aus_GetChannelDefaultSettings(&ausSettings);
                ausSettings.xtalFreq = BDCM_AUS_XTALFREQ;
                hChannel->upStream =  BDCM_Aus_OpenChannel(hDevice->hDocsis,&ausSettings);
                if(!hChannel->upStream)
                {
                    BDBG_ERR(("%s: US demod open failed",__FUNCTION__));
                    goto error;
                }


                hFrontend->capabilities.docsis = false;
                hFrontend->capabilities.qam = false;
                hFrontend->capabilities.outOfBand = false;
                hFrontend->capabilities.upstream = true;
                BKNI_Memset(hFrontend->capabilities.upstreamModes, true, sizeof(hFrontend->capabilities.upstreamModes));

                hFrontend->close = NEXUS_Docsis_P_CloseChannel;
                /*
                 * BCM3384 doesn't support deep sleep mode (S3) so disable the standby mode for now.
                 */
                hFrontend->standby = NULL; /*NEXUS_Docsis_P_ChannelStandby;*/
                hFrontend->tuneUpstream = NEXUS_Docsis_P_TuneUpstream;
                hFrontend->untune = NEXUS_Docsis_P_Untune;
                hFrontend->getUpstreamStatus = NEXUS_Docsis_P_GetUpstreamStatus;
                hFrontend->transmitDebugPacket= NEXUS_Docsis_P_TransmitDebugPacket;

                hChannel->dsChannelNum = 0;
                hDevice->hUpStreamChannel = hFrontend;
                hFrontend->pDeviceHandle = hChannel;
                hFrontend->pGenericDeviceHandle = handle;
                BDBG_MSG(("US channel configured"));
            }
            else
            {
                if(pOpenSettings->channelNum >= hDevice->numDataChannels)
                {
                    BDBG_ERR(("%s: invalid DOCSIS data channel index",__FUNCTION__));
                }

                hFrontend = NEXUS_Frontend_P_Create(hChannel);
                if(!hFrontend)
                {
                    retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
                    goto error;
                }
                hFrontend->capabilities.docsis = true;
                hFrontend->capabilities.qam = false;
                hFrontend->capabilities.outOfBand = false;
                hFrontend->capabilities.upstream = false;
                hFrontend->close = NEXUS_Docsis_P_CloseChannel;
                hChannel->dsChannelNum = pOpenSettings->channelNum;
                hDevice->hDsChannel[pOpenSettings->channelNum] = hFrontend;
                hFrontend->pDeviceHandle = hChannel;
                hFrontend->pGenericDeviceHandle = handle;
                BDBG_MSG(("data channel %u configured",hChannel->dsChannelNum));
            }
        }
    }
    hFrontend->getType = NEXUS_Docsis_P_GetType;
    BDBG_MSG(("hDevice %#lx hChannel %#lx hFrontend %#lx",(long unsigned int)hDevice,(long unsigned int)hChannel,(long unsigned int)hFrontend));
    hFrontend->pGenericDeviceHandle = hDevice->pGenericDeviceHandle;
    return hFrontend;

error:
    if (hChannel->asyncStatusAppCallback)
    {
        NEXUS_TaskCallback_Destroy(hChannel->asyncStatusAppCallback);
    }
    if (hChannel->lockAppCallback)
    {
        NEXUS_TaskCallback_Destroy(hChannel->lockAppCallback);
    }
    if (hChannel->lockDriverCallback)
    {
        NEXUS_TaskCallback_Destroy(hChannel->lockDriverCallback);
    }
    if(hChannel->hTnr)
    {
        BDCM_Tnr_Close(hChannel->hTnr);
    }
    if(hChannel->qam)
    {
        BDCM_Ads_CloseChannel(hChannel->qam);
    }
    if(hChannel->outOfBand)
    {
        BDCM_Aob_CloseChannel(hChannel->outOfBand);
    }
    if(hChannel->upStream)
    {
        BDCM_Aus_CloseChannel(hChannel->upStream);
    }
    if(hFrontend)
    {
        NEXUS_Frontend_P_Destroy(hFrontend);
    }
    if(hChannel)
    {
        BKNI_Free(hChannel);
    }
errorAlloc:
    return NULL;
}

NEXUS_Error NEXUS_Docsis_EnableCableCardOutOfBandPins(
    NEXUS_FrontendDeviceHandle handle,
    bool enabled
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    BDBG_ASSERT(handle);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);
    retCode = BDCM_EnableCableCardOutOfBandPins(hDevice->hDocsis,enabled);
    return retCode;
}

NEXUS_Error NEXUS_Docsis_ConfigureDeviceLna(
    NEXUS_FrontendDeviceHandle handle
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_DocsisDeviceHandle hDevice=NULL;
    BDBG_ASSERT(handle);
    hDevice = (NEXUS_DocsisDeviceHandle)handle->pDevice;
    BDBG_OBJECT_ASSERT(hDevice, NEXUS_DocsisDevice);
    retCode = BDCM_ConfigureDeviceLna(hDevice->hDocsis);
    return retCode;
}
