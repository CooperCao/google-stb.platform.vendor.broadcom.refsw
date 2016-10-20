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
#include "nexus_frontend_module.h"
#include "nexus_platform_features.h"
#include "priv/nexus_transport_priv.h"
#include "bads.h"
#include "bads_3255.h"
#include "btnr.h"
#include "btnr_3255ib.h"
#include "brpc.h"
#include "brpc_3255.h"
#include "brpc_socket.h"
#include "brpc_plat.h"
#if NEXUS_HAS_MXT
#include "bmxt.h"
#endif

#if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
#include "baob.h"
#include "baus.h"
#include "btnr_3255ob.h"
#endif

BDBG_MODULE(nexus_frontend_3255);

BDBG_OBJECT_ID(NEXUS_3255Device);

#define sizeInLong(x)   (sizeof(x)/sizeof(uint32_t))

void b_convert_ipaddr(uint8_t *c, uint32_t b)
{
    c[0] = (b>>24)&0xff;
    c[1] = (b>>16)&0xff;
    c[2] = (b>>8)&0xff;
    c[3] =  b&0xff;
}

/***************************************************************************
Summary
DOCSIS QAM video status data type
***************************************************************************/
typedef struct NEXUS_3255QamChannelStatus
{
    NEXUS_TaskCallbackHandle statusCallback;
    BADS_Status status;
    unsigned fecCorrected;      /* accumulated FEC corrected count*/
    unsigned fecUnCorrected;    /* accumulated FEC uncorrected count*/
    unsigned totalSymbol;       /* accumulated Total symbol count*/
    unsigned unlockCount;       /* unlock count */
    unsigned acquireCount;      /* QAM acquiring total count */
    unsigned failureCount;      /* QAM acquiring failure count */
    unsigned failureFreq;       /* failure QAM frequency */
    NEXUS_Time lockTime;        /* QAM locked time*/
    bool    locked;
} NEXUS_3255QamChannelStatus;

/***************************************************************************
Summary
DOCSIS device handle
***************************************************************************/
typedef struct NEXUS_3255Device
{
    BDBG_OBJECT(NEXUS_3255Device)
    BADS_Handle ads;
#if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
    BAUS_Handle aus;
    BTNR_Handle oobTnr;
    BAOB_Handle aob;
#endif
    BRPC_Handle rpc_handle;
    NEXUS_ThreadHandle rpc_notification;
    unsigned rpc_notification_count;
    NEXUS_ThreadHandle heartbeat;
    BKNI_EventHandle heartbeatEvent;
#if NEXUS_POWER_MANAGEMENT
    BKNI_MutexHandle rpcMutex;
#endif
    bool heartbeat_enabled;
    NEXUS_FrontendHandle frontendHandle[NEXUS_MAX_FRONTENDS];
    bool rpc_notification_enabled;
    bool isOOBsupported;
    unsigned numChannels;
    unsigned numDocsisChannels;
    unsigned numOfQamFrontends;
    unsigned numOfFrontends;
    unsigned oobFrontendIndex;
    NEXUS_3255DeviceSettings deviceSettings;
    NEXUS_3255ChannelCapabilities  *channelCapabilities;
    NEXUS_3255DeviceStatus deviceStatus;
    NEXUS_TaskCallbackHandle eCMStatusCallback;
    unsigned numOfTsmfParsers;
    unsigned numOfUsedTsmfParsers;
    uint32_t NonCmControlledVideoChLockStatus;
    NEXUS_StandbyMode currentPowerState;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
} NEXUS_3255Device;

/**********************************************************************************
 Summary:
 DOCSIS channel handle
 *********************************************************************************/
typedef struct NEXUS_3255Channel
{
    unsigned chn_num; /* channel number */
    unsigned frontendIndex;
    unsigned tsmfParserIndex;
    BADS_ChannelHandle ads_chn;
    BTNR_Handle tnr;
    NEXUS_CallbackHandler lockDriverCBHandler;
    NEXUS_TaskCallbackHandle lockAppCallback;
    NEXUS_TaskCallbackHandle lockDriverCallback;
    NEXUS_TaskCallbackHandle asyncStatusAppCallback;
    NEXUS_TimerHandle retuneTimer;
    NEXUS_3255QamChannelStatus ads_status;
    NEXUS_FrontendQamSettings last_ads;
    NEXUS_FrontendOutOfBandSettings last_aob;
    NEXUS_FrontendUpstreamSettings  last_aus;
    bool tune_started;
    NEXUS_3255ChannelSettings channelSettings;
    NEXUS_TsmfSettings tsmfSettings;
    NEXUS_3255DeviceHandle deviceHandle; /* 3255 device*/
} NEXUS_3255Channel;

/*************************************************************************************
Summary:
DOCSIS Channel Handle
***************************************************************************************/
typedef struct NEXUS_3255Channel *NEXUS_3255ChannelHandle;

/***************************************************************************
Summary:
    Mapped to NEXUS_Frontend->getType. Invoked when
    NEXUS_Frontend_GetType API is invoked.
************************************************************************/
static void NEXUS_Frontend_P_GetType3255Channel(void *handle,NEXUS_FrontendType *type)
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BERR_Code retVal, retCode;
    BRPC_Param_ADS_GetVersion outVerParam;
    BRPC_Param_XXX_Get Param;

    BDBG_ASSERT(handle);
    deviceHandle = channelHandle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BKNI_Memset(type, 0, sizeof(NEXUS_FrontendType));

    retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ADS_GetVersion, (const uint32_t *)&Param, sizeof(Param)/4, (uint32_t *)&outVerParam, sizeInLong(outVerParam), &retVal);
    if(retCode != BERR_SUCCESS || retVal != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: unable to get version",__FUNCTION__));
        goto error;
    }
    /* assigning chip ID as family ID */
    type->chip.familyId = outVerParam.majVer >> 16;
    type->chip.id = outVerParam.majVer >> 16;
    type->chip.version.major = outVerParam.majVer & 0x0000000f;
    type->chip.version.minor = 0;
    BDBG_MSG(("%s %#x  %#x ",__FUNCTION__,type->chip.familyId,type->chip.version.major));
error:
    return;
}

/*********************************************************************************************
Summary:
 Private APIs that are hooked to the front end API for OOB upstream and downstream channels
**********************************************************************************************/
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
static void NEXUS_Frontend_P_3255OobChannelLockChange(void *pParam)
{
    NEXUS_TaskCallbackHandle callback = (NEXUS_TaskCallbackHandle)pParam;

    BDBG_MSG(("3255 OOB lock event change"));

    if ( callback )
    {
        NEXUS_TaskCallback_Fire(callback);
    }
}

static void NEXUS_Frontend_P_3255OobChannelUnTune(void *handle)
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    channelHandle->tune_started = false;

    NEXUS_Frontend_P_UnsetMtsifConfig(deviceHandle->frontendHandle[channelHandle->frontendIndex]);
}

static NEXUS_Error NEXUS_Frontend_P_3255OobChannelTune(
    void *handle,
    const NEXUS_FrontendOutOfBandSettings *pSettings)
{
    BERR_Code rc;
    BAOB_ModulationType modType;
    BAOB_SpectrumMode spectrum;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT(NULL != pSettings);
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);

    rc = NEXUS_Frontend_P_SetMtsifConfig(deviceHandle->frontendHandle[channelHandle->frontendIndex]);
    if (rc) { return BERR_TRACE(rc); }

    switch (pSettings->mode)
    {
    case NEXUS_FrontendOutOfBandMode_eAnnexAQpsk:
        modType = BAOB_ModulationType_eAnnexAQpsk;
        break;
    case NEXUS_FrontendOutOfBandMode_eDvs178Qpsk:
        modType = BAOB_ModulationType_eDvs178Qpsk;
        break;
    case NEXUS_FrontendOutOfBandMode_ePod_AnnexAQpsk:
        modType = BAOB_ModulationType_ePod_AnnexAQpsk;
        break;
    case NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk:
        modType = BAOB_ModulationType_ePod_Dvs178Qpsk;
        break;
    default:
        return BERR_INVALID_PARAMETER;
    }

    switch (pSettings->spectrum)
    {
    case NEXUS_FrontendOutOfBandSpectrum_eAuto:
        spectrum = BAOB_SpectrumMode_eAuto;
        break;
    case NEXUS_FrontendOutOfBandSpectrum_eInverted:
        spectrum = BAOB_SpectrumMode_eInverted;
        break;
    case NEXUS_FrontendOutOfBandSpectrum_eNonInverted:
        spectrum = BAOB_SpectrumMode_eNoInverted;
        break;
    default:
        return BERR_INVALID_PARAMETER;
    }

    NEXUS_TaskCallback_Set(channelHandle->lockAppCallback, &(pSettings->lockCallback));

    if ( channelHandle->tnr)
    {
        rc = BTNR_SetTunerRfFreq(channelHandle->tnr, pSettings->frequency, BTNR_TunerMode_eDigital);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
    }

    rc = BAOB_Set_Spectrum(deviceHandle->aob, spectrum);
    if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    if (BAOB_Acquire(deviceHandle->aob, modType, pSettings->symbolRate)) { return BERR_TRACE(rc);}

    channelHandle->tune_started = true;
    channelHandle->last_aob = *pSettings;
    return BERR_SUCCESS;

}

static NEXUS_Error NEXUS_Frontend_P_Get3255OobChannelStatus(
    void *handle,
    NEXUS_FrontendOutOfBandStatus *pStatus)
{
    BERR_Code  rc;
    struct BAOB_Status st;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);

    rc = BAOB_GetStatus(deviceHandle->aob,  &st);
    if (rc!=BERR_SUCCESS){ return BERR_TRACE(rc);}
    pStatus->ifFreq = st.ifFreq;
    pStatus->isFecLocked = st.isFecLock;
    pStatus->isQamLocked = st.isQamLock;
    pStatus->symbolRate = st.symbolRate;
    pStatus->snrEstimate = st.snrEstimate*100/256;
    pStatus->agcIntLevel = st.agcIntLevel;
    pStatus->agcExtLevel = st.agcExtLevel;
    pStatus->carrierFreqOffset = st.carrierFreqOffset;
    pStatus->carrierPhaseOffset = st.carrierPhaseOffset;
    pStatus->correctedCount = st.correctedCount;
    pStatus->uncorrectedCount = st.uncorrectedCount;
    pStatus->fdcChannelPower = st.fdcChannelPower;
    pStatus->berErrorCount = st.berErrorCount;

    pStatus->settings = channelHandle->last_aob;
    BDBG_MSG((" OOB STATUS : fec_lock %d \t qam_lock %d \t  agcIntLevel %d \t st.agcExtLevel %d "
              "\t snr_estimate %d \n\t fec_corr_cnt %d \t fec_uncorr_cnt %d\t ber_estimate %d"
              "\t fdcChannelPower %d",st.isFecLock, st.isQamLock, st.agcIntLevel, st.agcExtLevel,
              st.snrEstimate,st.correctedCount, st.uncorrectedCount, st.berErrorCount, st.fdcChannelPower));

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_3255UsChannelTune(
    void *handle,
    const NEXUS_FrontendUpstreamSettings *pSettings)
{
    BERR_Code rc;
    BAUS_OperationMode mode;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;


    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT(NULL != pSettings);
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);

    rc = NEXUS_Frontend_P_SetMtsifConfig(deviceHandle->frontendHandle[channelHandle->frontendIndex]);
    if (rc) { return BERR_TRACE(rc); }

    switch (pSettings->mode)
    {
    case NEXUS_FrontendUpstreamMode_eAnnexA:
        mode = BAUS_OperationMode_eAnnexA;
        break;
    case NEXUS_FrontendUpstreamMode_eDvs178:
        mode = BAUS_OperationMode_eDvs178;
        break;
    case NEXUS_FrontendUpstreamMode_ePodAnnexA :
        mode = BAUS_OperationMode_ePodAnnexA;
        break;
    case NEXUS_FrontendUpstreamMode_ePodDvs178:
        mode = BAUS_OperationMode_ePodDvs178;
        break;
    case NEXUS_FrontendUpstreamMode_eDocsis:
        mode = BAUS_OperationMode_eDocsis;
        break;
    default:
        return BERR_NOT_SUPPORTED;
    }
#if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
    rc = BAUS_SetOperationMode(deviceHandle->aus, mode);
    if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    if (mode != BAUS_OperationMode_eDocsis) {
        rc = BAUS_SetRfFreq(deviceHandle->aus, pSettings->frequency);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

        rc = BAUS_SetSymbolRate(deviceHandle->aus, pSettings->symbolRate);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

        rc = BAUS_SetPowerLevel(deviceHandle->aus, pSettings->powerLevel);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
    }
#endif
    channelHandle->last_aus = *pSettings;
    return BERR_SUCCESS;

}
static NEXUS_Error NEXUS_Frontend_P_Get3255UsChannelStatus(
    void *handle,
    NEXUS_FrontendUpstreamStatus *pStatus)
{
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    BERR_Code  rc;
    struct BAUS_Status st;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);
    rc = BAUS_GetStatus(deviceHandle->aus,  &st);
    if (rc!=BERR_SUCCESS){ return BERR_TRACE(rc);}

    switch (st.operationMode)
    {
        case BAUS_OperationMode_ePodAnnexA:
            pStatus->mode  = NEXUS_FrontendUpstreamMode_ePodAnnexA;
            break;
        case BAUS_OperationMode_ePodDvs178:
            pStatus->mode  = NEXUS_FrontendUpstreamMode_ePodDvs178;
            break;
        case BAUS_OperationMode_eAnnexA :
            pStatus->mode  = NEXUS_FrontendUpstreamMode_eAnnexA;
            break;
        case BAUS_OperationMode_eDvs178:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eDvs178;
            break;
        case BAUS_OperationMode_eDocsis:
            pStatus->mode = NEXUS_FrontendUpstreamMode_eDocsis;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pStatus->frequency = st.rfFreq;
    pStatus->symbolRate = st.symbolRate;
    pStatus->powerLevel = st.powerLevel;
    pStatus->sysXtalFreq = st.sysXtalFreq;

    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return NEXUS_NOT_SUPPORTED;
#endif
}
#endif

/*********************************************************************************************
Summary:
 Private APIs that are hooked to the front end API for QAM video channels
**********************************************************************************************/
static void NEXUS_Frontend_P_3255QamChannelLockChange(void *pParam)
{
    NEXUS_TaskCallbackHandle callback = (NEXUS_TaskCallbackHandle)pParam;

    BDBG_MSG(("3255 lock event change"));

    if ( callback )
    {
        NEXUS_TaskCallback_Fire(callback);
    }
}

static void NEXUS_Frontend_P_3255_Reacquire(void *context)
{

    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)context;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BADS_ModulationType qam_mode;
    BERR_Code rc;
    BADS_InbandParam params;
    NEXUS_FrontendQamSettings *pSettings = &channelHandle->last_ads;
    BADS_LockStatus lockStatus = BADS_LockStatus_eUnlocked;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT(NULL != pSettings);

    channelHandle->retuneTimer = NULL;

    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return;

    rc = BADS_GetLockStatus(channelHandle->ads_chn, &lockStatus);
    if (rc!=BERR_SUCCESS) { BERR_TRACE(rc); return; }
    if (lockStatus == BADS_LockStatus_eUnlocked)
    {
        BDBG_MSG(("%s() unlocked, reacquiring.",__FUNCTION__));
        BKNI_Memset(&params, 0, sizeof(params));

        if ( pSettings->annex == NEXUS_FrontendQamAnnex_eA )
        {
            switch ( pSettings->mode )
            {
            case NEXUS_FrontendQamMode_e16:
                qam_mode = BADS_ModulationType_eAnnexAQam16;
                break;
            case NEXUS_FrontendQamMode_e32:
                qam_mode = BADS_ModulationType_eAnnexAQam32;
                break;
            case NEXUS_FrontendQamMode_e64:
                qam_mode = BADS_ModulationType_eAnnexAQam64;
                break;
            case NEXUS_FrontendQamMode_e128:
                qam_mode = BADS_ModulationType_eAnnexAQam128;
                break;
            case NEXUS_FrontendQamMode_e256:
                qam_mode = BADS_ModulationType_eAnnexAQam256;
                break;
            default:
                return;
            }
        }
        else if ( pSettings->annex == NEXUS_FrontendQamAnnex_eB )
        {
            switch ( pSettings->mode )
            {
            case NEXUS_FrontendQamMode_e64:
                qam_mode = BADS_ModulationType_eAnnexBQam64;
                break;
            case NEXUS_FrontendQamMode_e256:
                qam_mode = BADS_ModulationType_eAnnexBQam256;
                break;
            default:
                return;
            }
        }
        else
        {
            BERR_TRACE(BERR_INVALID_PARAMETER);return;
        }

        if ( pSettings->symbolRate )
        {
            params.symbolRate = pSettings->symbolRate;
        }
        else
        {
            params.symbolRate = NEXUS_Frontend_P_GetDefaultQamSymbolRate(pSettings->mode, pSettings->annex);
        }

        params.modType = qam_mode;
        rc = BADS_Acquire(channelHandle->ads_chn, &params);
        if (rc!=BERR_SUCCESS) { BERR_TRACE(rc); return; }
        channelHandle->tune_started = true;
    }

}

static void NEXUS_Frontend_P_Check3255ChannelReacquireStatus(void *context)
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)context;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BERR_Code rc;
    NEXUS_FrontendQamSettings *pSettings = &channelHandle->last_ads;
    BADS_LockStatus lockStatus = BADS_LockStatus_eUnlocked;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT(NULL != pSettings);

    BDBG_MSG(("%s() calling firing lockAppCallback.",__FUNCTION__));
    NEXUS_TaskCallback_Fire(channelHandle->lockAppCallback);

    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return;

    rc = BADS_GetLockStatus(channelHandle->ads_chn, &lockStatus);
    if (rc!=BERR_SUCCESS) { BERR_TRACE(rc); return; }

    if ((lockStatus == BADS_LockStatus_eUnlocked) && pSettings->autoAcquire)
    {
        if( channelHandle->retuneTimer )
        {
            NEXUS_CancelTimer(channelHandle->retuneTimer);
        }
        BDBG_MSG(("%s() unlocked, schedule a timer to reacquire.",__FUNCTION__));
        channelHandle->retuneTimer= NEXUS_ScheduleTimer(2000, NEXUS_Frontend_P_3255_Reacquire, context);
    }
}

static NEXUS_Error NEXUS_Frontend_P_3255QamChannelTune(
    void *handle,
    const NEXUS_FrontendQamSettings *pSettings)
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BADS_ModulationType qam_mode;
    BERR_Code rc;
    BADS_InbandParam params;
#if 0
    BTNR_PowerSaverSettings pwrSetting;
#endif
    unsigned chn_num = channelHandle->chn_num;
    NEXUS_CallbackDesc callbackDesc;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_MSG(("TuneQAM docsis channel %u",chn_num));
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);

    rc = NEXUS_Frontend_P_SetMtsifConfig(deviceHandle->frontendHandle[channelHandle->frontendIndex]);
    if (rc) { return BERR_TRACE(rc); }

    BKNI_Memset(&params, 0, sizeof(params));

    if ( pSettings->annex == NEXUS_FrontendQamAnnex_eA )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e16:
            qam_mode = BADS_ModulationType_eAnnexAQam16;
            break;
        case NEXUS_FrontendQamMode_e32:
            qam_mode = BADS_ModulationType_eAnnexAQam32;
            break;
        case NEXUS_FrontendQamMode_e64:
            qam_mode = BADS_ModulationType_eAnnexAQam64;
            break;
        case NEXUS_FrontendQamMode_e128:
            qam_mode = BADS_ModulationType_eAnnexAQam128;
            break;
        case NEXUS_FrontendQamMode_e256:
            qam_mode = BADS_ModulationType_eAnnexAQam256;
            break;
        default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else if ( pSettings->annex == NEXUS_FrontendQamAnnex_eB )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e64:
            qam_mode = BADS_ModulationType_eAnnexBQam64;
            break;
        case NEXUS_FrontendQamMode_e256:
            qam_mode = BADS_ModulationType_eAnnexBQam256;
            break;
        default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else if ( pSettings->annex == NEXUS_FrontendQamAnnex_eC )
    {
        switch ( pSettings->mode )
        {
        case NEXUS_FrontendQamMode_e64:
            qam_mode = BADS_ModulationType_eAnnexCQam64;
            break;
        case NEXUS_FrontendQamMode_e256:
            qam_mode = BADS_ModulationType_eAnnexCQam256;
            break;
        default:
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( pSettings->symbolRate )
    {
        params.symbolRate = pSettings->symbolRate;
    }
    else
    {
        params.symbolRate = NEXUS_Frontend_P_GetDefaultQamSymbolRate(pSettings->mode, pSettings->annex);
    }

    callbackDesc.param = chn_num;
    NEXUS_CallbackHandler_PrepareCallback(channelHandle->lockDriverCBHandler, callbackDesc);
    NEXUS_TaskCallback_Set(channelHandle->lockDriverCallback, &callbackDesc);
    NEXUS_TaskCallback_Set(channelHandle->lockAppCallback, &(pSettings->lockCallback));
    NEXUS_TaskCallback_Set(channelHandle->asyncStatusAppCallback, &(pSettings->asyncStatusReadyCallback));

    if (channelHandle->tnr)
    {
#if 0
        pwrSetting.enable = false;
        rc = BTNR_SetPowerSaver(channelHandle->tnr, &pwrSetting);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
#endif
        rc = BTNR_SetTunerRfFreq(channelHandle->tnr, pSettings->frequency, BTNR_TunerMode_eDigital);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
    }

    params.modType = qam_mode;
    rc = BADS_Acquire(channelHandle->ads_chn, &params);
    if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    channelHandle->tune_started = true;
    if (channelHandle->retuneTimer)
        NEXUS_CancelTimer(channelHandle->retuneTimer);
    channelHandle->retuneTimer = NULL;
    channelHandle->last_ads = *pSettings;

    return BERR_SUCCESS;
}

static void NEXUS_Frontend_P_3255QamChannelUnTune(void *handle)
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);

    NEXUS_Frontend_P_UnsetMtsifConfig(deviceHandle->frontendHandle[channelHandle->frontendIndex]);

    channelHandle->tune_started = false;
    if (channelHandle->retuneTimer)
        NEXUS_CancelTimer(channelHandle->retuneTimer);
    channelHandle->retuneTimer = NULL;
    return;
}
#if 0
NEXUS_Error NEXUS_Frontend_3255_GetQamLockStatus(
    NEXUS_FrontendHandle frontHandle,
    bool *locked)
{
    BERR_Code rc;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)frontHandle->pDeviceHandle;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)channelHandle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    *locked = false;
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);
    rc = BADS_GetLockStatus(channelHandle->ads_chn, locked);
    if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    return BERR_SUCCESS;
}
#endif

NEXUS_Error NEXUS_Frontend_P_3255_GetFastStatus(
   void *handle,
   NEXUS_FrontendFastStatus *pStatus
   )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    unsigned chn_num = channelHandle->chn_num;
    bool isLocked=false;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);

    BKNI_Memset(pStatus,0,sizeof(*pStatus));
    if(deviceHandle->channelCapabilities[chn_num].channelType == NEXUS_3255ChannelType_eInBand)
    {
        if(channelHandle->tune_started)
        {
            BADS_LockStatus adsLockStatus = BADS_LockStatus_eUnlocked;
            BADS_GetLockStatus(channelHandle->ads_chn,&adsLockStatus);
            isLocked = (adsLockStatus == BADS_LockStatus_eLocked)? true:false;
        }
        else
        {
            BDBG_WRN(("app didn't send the tune command yet for QAM channel %u",chn_num));
            rc = NEXUS_NOT_INITIALIZED;
            goto error;
        }
    }
    else
    {
#if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
        if(deviceHandle->channelCapabilities[chn_num].channelType == NEXUS_3255ChannelType_eOutOfBand)
        {
            if(channelHandle->tune_started)
            {
                BAOB_GetLockStatus(deviceHandle->aob,&isLocked);
            }
            else
            {
                BDBG_WRN(("app didn't send the tune command yet for OOB channel"));
                rc = NEXUS_NOT_INITIALIZED;
                goto error;
            }
        }
        else
#endif
        {
            BDBG_ERR(("getFastStatus not supported for channel type %u",deviceHandle->channelCapabilities[chn_num].channelType));
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

static NEXUS_Error NEXUS_Frontend_P_Get3255QamChannelStatus(
    void *handle,
    NEXUS_FrontendQamStatus *pStatus)
{
    BERR_Code rc;
    BADS_Status st;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BADS_Version version;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);


    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);
    rc = BADS_GetStatus(channelHandle->ads_chn,  &st);
    if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}
    rc = BADS_GetVersion(deviceHandle->ads, &version);
    if (rc) return BERR_TRACE(rc);

    pStatus->fecLock = st.isFecLock;
    pStatus->receiverLock = st.isQamLock;
    pStatus->symbolRate = st.rxSymbolRate;
    pStatus->ifAgcLevel = st.agcIntLevel;
    pStatus->rfAgcLevel = st.agcExtLevel;
    pStatus->carrierFreqOffset = st.carrierFreqOffset;
    pStatus->carrierPhaseOffset = st.carrierPhaseOffset;

    if (st.isQamLock == false)
    {
        st.dsChannelPower = 0;
        st.mainTap = 0;
        st.equalizerGain = 0;
        st.interleaveDepth = 0;
    }
    pStatus->dsChannelPower = (version.minVer <= 1) ? st.dsChannelPower : 0;
    pStatus->mainTap = st.mainTap;
    pStatus->equalizerGain = st.equalizerGain;
    pStatus->interleaveDepth = st.interleaveDepth;

    if (st.isFecLock == false ||st.isQamLock == false)
    {
        st.snrEstimate = 0;
        st.correctedCount = 0;
        st.uncorrectedCount = 0;
        st.berRawCount = 0;
        st.goodRsBlockCount = 0;
        st.postRsBER = 0;
        st.elapsedTimeSec = 0;
    }

    pStatus->snrEstimate = st.snrEstimate*100/256;
    pStatus->fecCorrected = st.correctedCount;
    pStatus->fecUncorrected = st.uncorrectedCount;
    pStatus->berEstimate = st.berRawCount;
    pStatus->goodRsBlockCount = st.goodRsBlockCount;
    pStatus->postRsBer = st.postRsBER;
    pStatus->postRsBerElapsedTime = st.elapsedTimeSec;
    pStatus->spectrumInverted = st.isSpectrumInverted;
    pStatus->settings = channelHandle->last_ads;

    BDBG_MSG((" STATUS : fec_lock %d \t qam_lock %d \t  agcIntLevel %d \t st.agcExtLevel %d "
              "\t snr_estimate %d \t fec_corr_cnt %d \t fec_uncorr_cnt %d\t ber_estimate %d",
              st.isFecLock, st.isQamLock, st.agcIntLevel, st.agcExtLevel, st.snrEstimate,
              st.correctedCount, st.uncorrectedCount, st.berRawCount));

    return BERR_SUCCESS;
}


static NEXUS_Error NEXUS_Frontend_P_Get3255ChannelSoftDecisions(
    void *handle,
    NEXUS_FrontendSoftDecision *pDecisions,
    size_t length)
{
    int i;
    BERR_Code rc;
    int16_t *data_i, *data_q;
    int16_t return_length;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);

    if ((data_i = BKNI_Malloc(length*sizeof(int16_t))) == NULL) {
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }
    if ((data_q = BKNI_Malloc(length*sizeof(int16_t))) == NULL) {
        BKNI_Free(data_i);
        return BERR_OUT_OF_SYSTEM_MEMORY;
    }

    rc = BADS_GetSoftDecision(channelHandle->ads_chn, (int16_t)length,  data_i, data_q, &return_length);
    if (rc!=BERR_SUCCESS) {
        BKNI_Free(data_i);
        BKNI_Free(data_q);
        return_length = length;
        return BERR_TRACE(rc);
    }

    for(i=0;i<return_length;i++) {
        pDecisions[i].i =  data_i[i]*2;
        pDecisions[i].q = data_q[i]*2;
    }

    BKNI_Free(data_i);
    BKNI_Free(data_q);

    return BERR_SUCCESS;

}

/**************************************************************************************
  Private APIs for managing a DOCSIS device
 **************************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_Init3255DeviceRPC(NEXUS_3255DeviceHandle deviceHandle)
{
    BRPC_OpenSocketImplSettings socketSettings;
    BRPC_Param_InitSession Param;
    BERR_Code errCode, retCode, retVal;
    unsigned count;

    BRPC_GetDefaultOpenSocketImplSettings(&socketSettings);
    socketSettings.timeout = deviceHandle->deviceSettings.rpcTimeout;

    if (NEXUS_GetEnv("no_3255") != NULL)
    {
        socketSettings.rpc_disabled = true;
        BDBG_WRN(("**********DOCSIS is NOT used!**********"));
    }
    errCode = BRPC_Open_SocketImpl(&deviceHandle->rpc_handle, &socketSettings);
    if ( errCode != BERR_SUCCESS ) return NEXUS_UNKNOWN;
    /* wait for 3255 to bootup and setup Rmagnum interface*/
    Param.devId = BRPC_DevId_3255;
    count = 0;
    if (NEXUS_GetEnv("no_3255") == NULL)
    {
       while (1) {
            #if BDBG_DEBUG_BUILD
            if (count%20==0)BDBG_WRN((" waiting for 3255 booting up!"));
            #endif
            retCode = BRPC_CallProc(deviceHandle->rpc_handle,
                                    BRPC_ProcId_InitSession,
                                    (const uint32_t *)&Param,
                                    sizeof(Param)/4,
                                    NULL, 0, &retVal);
            if (retCode == BERR_SUCCESS/*&& retVal == BERR_SUCCESS*/) break;
            BKNI_Sleep(10);
            if (count++ >= 1000)
            {
                BDBG_ERR((" timeout in waiting for 3255"));
                BRPC_Close_SocketImpl(deviceHandle->rpc_handle);
                return NEXUS_UNKNOWN;
            };
        }
    }
    BDBG_WRN((" RPC is initialized between 3255 and 740x"));
    BRPC_Close_SocketImpl(deviceHandle->rpc_handle);

    /* reopen RPC socket to apply normal timeout value*/
    socketSettings.timeout = 3000; /*3000ms for reliable connection*/
    errCode = BRPC_Open_SocketImpl(&deviceHandle->rpc_handle, &socketSettings);
    if ( errCode != BERR_SUCCESS ) return NEXUS_UNKNOWN;
    BDBG_WRN(("Device Handle %p, RPC handle %p", (void *)deviceHandle, (void *)deviceHandle->rpc_handle));
    return NEXUS_SUCCESS;
}

static NEXUS_3255ChannelHandle NEXUS_Frontend_P_Get3255ChannelHandle(
    NEXUS_3255DeviceHandle deviceHandle,
    unsigned chn_num)
{
    unsigned i=0;
    bool foundChannelHandle=false;
    NEXUS_3255ChannelHandle channelHandle;

    BDBG_MSG(("deviceHandle->numOfQamFrontends %u deviceHandle %p",i, (void*)deviceHandle));
    for(i=0;i< deviceHandle->numOfQamFrontends && deviceHandle->frontendHandle[i];i++)
    {
        channelHandle = deviceHandle->frontendHandle[i]->pDeviceHandle;
        if(channelHandle->chn_num == chn_num)
        {
            BDBG_MSG(("found channel handle %u",chn_num));
            foundChannelHandle = true;
            break;
        }
    }

    if(foundChannelHandle)
    {
        return channelHandle;
    }
    else
    {
        return NULL;
    }
}

static void NEXUS_Frontend_P_Process3255DeviceRpcNotification(
    uint32_t device_id,
    uint32_t event,
    void * arg)
{
    unsigned id;
    unsigned rpc_event;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)arg;
    NEXUS_3255ChannelHandle channelHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_MSG(("NEXUS_Frontend_P_3255DeviceProcessNotification deviceId %x event %u deviceHandle %p",device_id,event,(void *)deviceHandle));
    deviceHandle->rpc_notification_count++;

    switch (device_id)
    {
        case BRPC_DevId_3255_DS0:
        case BRPC_DevId_3255_DS1:
        case BRPC_DevId_3255_DS2:
        case BRPC_DevId_3255_DS3:
        case BRPC_DevId_3255_DS4:
        case BRPC_DevId_3255_DS5:
        case BRPC_DevId_3255_DS6:
        case BRPC_DevId_3255_DS7:
            id = device_id - BRPC_DevId_3255_DS0;
            rpc_event = BRPC_GET_NOTIFICATION_EVENT(event);
            BDBG_MSG(("channel id %u",id));
            channelHandle = NEXUS_Frontend_P_Get3255ChannelHandle(deviceHandle,id);
            if (channelHandle)
            {
                if (rpc_event == BRPC_Notification_Event_DsChannelPower)
                {
                    BDBG_MSG((" got DS power notification"));
                    channelHandle->ads_status.status.dsChannelPower = BRPC_GET_DS_POWER(event);
                    if (channelHandle->ads_status.statusCallback)
                        NEXUS_TaskCallback_Fire(channelHandle->ads_status.statusCallback);
                    if (channelHandle->asyncStatusAppCallback)
                        NEXUS_TaskCallback_Fire(channelHandle->asyncStatusAppCallback);
                }
                else
                {
                    if (channelHandle->tune_started == true) /* only handle notification when tuner is active*/
                    {
                        BADS_ProcessNotification(channelHandle->ads_chn, event);
                    }
                }
            }
            break;
        case BRPC_DevId_ECM_DS0:
        case BRPC_DevId_ECM_DS1:
        case BRPC_DevId_ECM_DS2:
        case BRPC_DevId_ECM_DS3:
        case BRPC_DevId_ECM_DS4:
        case BRPC_DevId_ECM_DS5:
        case BRPC_DevId_ECM_DS6:
        case BRPC_DevId_ECM_DS7:
        case BRPC_DevId_ECM_DS8:
        case BRPC_DevId_ECM_DS9:
        case BRPC_DevId_ECM_DS10:
        case BRPC_DevId_ECM_DS11:
        case BRPC_DevId_ECM_DS12:
        case BRPC_DevId_ECM_DS13:
        case BRPC_DevId_ECM_DS14:
        case BRPC_DevId_ECM_DS15:
            id = device_id - BRPC_DevId_ECM_DS0;
            rpc_event = BRPC_GET_NOTIFICATION_EVENT(event);
            BDBG_MSG(("channel id %u",id));
            channelHandle = NEXUS_Frontend_P_Get3255ChannelHandle(deviceHandle,id);
            if (channelHandle)
            {
                if (rpc_event == BRPC_Notification_Event_DsChannelPower)
                {
                    BDBG_MSG((" got DS power notification"));
                    channelHandle->ads_status.status.dsChannelPower = BRPC_GET_DS_POWER(event);
                    if (channelHandle->ads_status.statusCallback)
                        NEXUS_TaskCallback_Fire(channelHandle->ads_status.statusCallback);
                    if (channelHandle->asyncStatusAppCallback)
                        NEXUS_TaskCallback_Fire(channelHandle->asyncStatusAppCallback);
                }
                else
                {
                    if (channelHandle->tune_started == true) /* only handle notification when tuner is active*/
                    {
                        BADS_ProcessNotification(channelHandle->ads_chn, event);
                    }
                }
            }
            break;
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    case BRPC_DevId_3255_OB0:
            if(deviceHandle->frontendHandle[deviceHandle->oobFrontendIndex])
            {
                channelHandle = deviceHandle->frontendHandle[deviceHandle->oobFrontendIndex]->pDeviceHandle;
                if (deviceHandle->aob != NULL && channelHandle->tune_started)
                    BAOB_ProcessNotification(deviceHandle->aob, event);
            }
            break;
#endif
    case BRPC_DevId_3255:
            rpc_event = BRPC_GET_NOTIFICATION_EVENT(event);
            if (rpc_event == BRPC_Notification_Event_EcmReset) {
                BDBG_WRN((" got eCM reset notification"));
                deviceHandle->deviceStatus = NEXUS_3255DeviceStatus_eReset;
                NEXUS_TaskCallback_Fire(deviceHandle->eCMStatusCallback);
            }
            else if (rpc_event == BRPC_Notification_Event_EcmOperational) {
                  /* The ECM opertaional notification is sent after Docsis is registered*/
                  /* not very useful for Rmagnum bring up*/
                BDBG_MSG((" got eCM registeration notification"));
            }
            else if (rpc_event == BRPC_Notification_Event_EcmRmagnumReady)
            {
                BDBG_WRN((" got eCM Rmagnum ready  notification"));
                BKNI_SetEvent(deviceHandle->heartbeatEvent);
            }
            break;
        case BRPC_DevId_3255_US0:
        case BRPC_DevId_3255_TNR0:
        case BRPC_DevId_3255_TNR1:
        case BRPC_DevId_3255_TNR2:
        case BRPC_DevId_3255_TNR3:
        case BRPC_DevId_3255_TNR4:
        case BRPC_DevId_3255_TNR5:
        case BRPC_DevId_3255_TNR6:
        case BRPC_DevId_3255_TNR7:
        case BRPC_DevId_ECM_TNR0:
        case BRPC_DevId_ECM_TNR1:
        case BRPC_DevId_ECM_TNR2:
        case BRPC_DevId_ECM_TNR3:
        case BRPC_DevId_ECM_TNR4:
        case BRPC_DevId_ECM_TNR5:
        case BRPC_DevId_ECM_TNR6:
        case BRPC_DevId_ECM_TNR7:
        case BRPC_DevId_ECM_TNR8:
        case BRPC_DevId_ECM_TNR9:
        case BRPC_DevId_ECM_TNR10:
        case BRPC_DevId_ECM_TNR11:
        case BRPC_DevId_ECM_TNR12:
        case BRPC_DevId_ECM_TNR13:
        case BRPC_DevId_ECM_TNR14:
        case BRPC_DevId_ECM_TNR15:
        default:
            BDBG_WRN((" unknown notification from 3255 device %d", device_id));
    }
}

static void NEXUS_Frontend_3255DeviceRpcNotificationThread(void *arg)
{
    uint32_t device_id, event;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)arg;
    /*TODO:: use infinite select() instead of polling*/
    while(deviceHandle->rpc_notification_enabled) {
#if NEXUS_POWER_MANAGEMENT
        BKNI_AcquireMutex(deviceHandle->rpcMutex);
        BDBG_MSG(("check_rpc(): Enter notified Thread"));
#endif
        BRPC_CheckNotification(deviceHandle->rpc_handle,  &device_id, &event, 100);
#if NEXUS_POWER_MANAGEMENT
        BDBG_MSG(("check_rpc(): End notified Thread"));
        BKNI_ReleaseMutex(deviceHandle->rpcMutex);
#endif
        if (BRPC_GET_NOTIFICATION_EVENT(event)) {
            BDBG_MSG(("check_rpc(): notified by server (device_id = %08x) event is %x", device_id, event));
            NEXUS_LockModule();
            NEXUS_Frontend_P_Process3255DeviceRpcNotification(device_id, event, arg);
            NEXUS_UnlockModule();
        }
        BKNI_Sleep(100);
    }
}


static NEXUS_Error NEXUS_Frontend_P_Open3255Channels(NEXUS_3255DeviceHandle deviceHandle)
{
    BERR_Code rc;
    NEXUS_3255ChannelHandle channelHandle;
    unsigned int i;
    BADS_Settings ads_cfg;
    BADS_ChannelSettings chn_cfg;
    BTNR_3255Ib_Settings tnr3255_cfg;
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    BTNR_3255Ob_Settings tnrOb3255_cfg;
    BAOB_Settings aob_cfg;
    BAUS_Settings aus_cfg;
#endif

    rc = BADS_3255_GetDefaultSettings( &ads_cfg, NULL);
    if ( rc != BERR_SUCCESS ) goto err_init;

    ads_cfg.hGeneric = deviceHandle->rpc_handle;
    rc = BADS_Open(&deviceHandle->ads, NULL, NULL, NULL, &ads_cfg);
    if ( rc != BERR_SUCCESS ) goto err_init;

    rc = BADS_Init(deviceHandle->ads);
    if ( rc != BERR_SUCCESS ) goto err_init;

    for (i=0;i<deviceHandle->numOfQamFrontends;i++)
    {
        NEXUS_3255ChannelHandle channelHandle = deviceHandle->frontendHandle[i]->pDeviceHandle;
        rc = BTNR_3255Ib_GetDefaultSettings(&tnr3255_cfg, NULL);
        if (rc != BERR_SUCCESS) goto err_init;
        tnr3255_cfg.hGeneric = deviceHandle->rpc_handle;
        tnr3255_cfg.ifFreq = BTNR_3255Ib_SETTINGS_IFFREQ;
        tnr3255_cfg.devId += channelHandle->chn_num;
        rc =  BTNR_3255Ib_Open(&channelHandle->tnr, NULL, NULL, NULL, &tnr3255_cfg);
        if (rc != BERR_SUCCESS) goto err_init;
    }

    /* Configure ADS channels */
    for (i=0;i<deviceHandle->numOfQamFrontends;i++)
    {
        channelHandle = deviceHandle->frontendHandle[i]->pDeviceHandle;
        rc = BADS_GetChannelDefaultSettings( deviceHandle->ads, i, &chn_cfg);
        if (rc!=BERR_SUCCESS) goto err_init;
        chn_cfg.autoAcquire = deviceHandle->deviceSettings.autoAcquire;
        chn_cfg.fastAcquire = deviceHandle->deviceSettings.fastAcquire;
        rc = BADS_OpenChannel( deviceHandle->ads, &channelHandle->ads_chn, channelHandle->chn_num, &chn_cfg);
        if (rc!=BERR_SUCCESS) goto err_init;
    }

#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
                channelHandle = deviceHandle->frontendHandle[deviceHandle->oobFrontendIndex]->pDeviceHandle;
    rc = BTNR_3255Ob_GetDefaultSettings(&tnrOb3255_cfg, NULL);
    if (rc!=BERR_SUCCESS) goto err_init;
    tnrOb3255_cfg.hGeneric = deviceHandle->rpc_handle;

/* BTNR_3255OB_SETTINGS_IFFREQ is deprecated (incorrectly assigns IF_FREQ based on host chip)
   use NEXUS_3255_OOB_TUNER_IFFREQ instead (to be defined in nexus_platform_features.h) */
#ifndef NEXUS_3255_OOB_TUNER_IFFREQ
#define NEXUS_3255_OOB_TUNER_IFFREQ BTNR_3255OB_SETTINGS_IFFREQ
#endif

    tnrOb3255_cfg.ifFreq = NEXUS_3255_OOB_TUNER_IFFREQ;
    rc = BTNR_3255Ob_Open(&channelHandle->tnr, NULL, &tnrOb3255_cfg);
    if (rc!=BERR_SUCCESS) goto err_init;

    rc = BAOB_GetDefaultSettings( &aob_cfg, NULL);
    if (rc!=BERR_SUCCESS) goto err_init;
    aob_cfg.hGeneric = deviceHandle->rpc_handle;
    aob_cfg.enableFEC = deviceHandle->deviceSettings.enableFEC;
    rc = BAOB_Open(&deviceHandle->aob, NULL, NULL, NULL, &aob_cfg);
    if (rc!=BERR_SUCCESS) goto err_init;

    rc = BAUS_GetDefaultSettings(&aus_cfg, NULL);
    if (rc!=BERR_SUCCESS) goto err_init;
    aus_cfg.xtalFreq = BAUS_SETTINGS_XTALFREQ;
    aus_cfg.hGeneric = deviceHandle->rpc_handle;;
    rc = BAUS_Open( &deviceHandle->aus, NULL, NULL, &aus_cfg);
    if (rc!=BERR_SUCCESS) goto err_init;

    deviceHandle->isOOBsupported = true;
#else
    deviceHandle->isOOBsupported = false;
#endif
                deviceHandle->deviceStatus = NEXUS_3255DeviceStatus_eOperational;
    return BERR_SUCCESS;

err_init:
    for ( i = 0; i < deviceHandle->numOfQamFrontends && NULL != deviceHandle->frontendHandle[i]; i++)
    {
        channelHandle= deviceHandle->frontendHandle[i]->pDeviceHandle;
        BTNR_Close(channelHandle->tnr);
    }
    for ( i = 0; i < deviceHandle->numOfQamFrontends && NULL != deviceHandle->frontendHandle[i]; i++)
    {
        channelHandle= deviceHandle->frontendHandle[i]->pDeviceHandle;
        BADS_CloseChannel(channelHandle->ads_chn);
    }
    if (deviceHandle->ads)
    {
        BADS_Close(deviceHandle->ads);
    }
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    channelHandle = deviceHandle->frontendHandle[deviceHandle->oobFrontendIndex]->pDeviceHandle;
    if (channelHandle->tnr)
      BTNR_Close(channelHandle->tnr);
    if (deviceHandle->aus)
    {
        BAUS_Close(deviceHandle->aus);
    }
    if (deviceHandle->aob)
    {
        BAOB_Close(deviceHandle->aob);
    }
#endif
    deviceHandle->deviceStatus = NEXUS_3255DeviceStatus_eUninitialized;
    return rc;
}

static void NEXUS_Frontend_P_Close3255Channels(NEXUS_3255DeviceHandle deviceHandle)
{
    unsigned i;
    NEXUS_3255ChannelHandle channelHandle;
    BDBG_MSG(("NEXUS_Frontend_P_Close3255Channels >>"));
    /* all channels are closed, close everthing*/
    for ( i = 0; i < deviceHandle->numOfQamFrontends && deviceHandle->frontendHandle[i]; i++)
    {
        channelHandle = deviceHandle->frontendHandle[i]->pDeviceHandle;
        if(channelHandle->tnr)
        {
            BTNR_Close(channelHandle->tnr);
        }
        channelHandle->tnr = NULL;
    }

    for ( i = 0; i < deviceHandle->numOfQamFrontends && deviceHandle->frontendHandle[i]; i++)
    {
        channelHandle = deviceHandle->frontendHandle[i]->pDeviceHandle;
        if(channelHandle->ads_chn)
        {
            BADS_CloseChannel(channelHandle->ads_chn);
        }
        channelHandle->ads_chn = NULL;
    }

    if (deviceHandle->ads)
    {
        BADS_Close(deviceHandle->ads);
        deviceHandle->ads = NULL;
    }
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    if(deviceHandle->frontendHandle[deviceHandle->oobFrontendIndex])
    {
        channelHandle = deviceHandle->frontendHandle[deviceHandle->oobFrontendIndex]->pDeviceHandle;
        if (channelHandle->tnr)
        {
            BTNR_Close(channelHandle->tnr);
            channelHandle->tnr = NULL;
        }
        if (deviceHandle->aus)
        {
            BAUS_Close(deviceHandle->aus);
            deviceHandle->aus = NULL;
        }
        if (deviceHandle->aob)
        {
            BAOB_Close(deviceHandle->aob);
            deviceHandle->aob = NULL;
        }
    }
#endif

    deviceHandle->deviceStatus = NEXUS_3255DeviceStatus_eUninitialized;
    BDBG_MSG(("NEXUS_Frontend_P_Close3255Channels <<"));
    return;
}

static void NEXUS_Frontend_P_3255DeviceHeartbeatThread(void * arg)
{
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)arg;
    BERR_Code retVal, retCode;
    NEXUS_Error errCode;
    BRPC_Param_ADS_GetVersion outVerParam;
    BRPC_Param_XXX_Get Param;
    bool need_restart;
    unsigned i;
    int count;

    Param.devId = BRPC_DevId_3255;

    deviceHandle->rpc_notification_count = 0; /* clear notification count*/

    while(deviceHandle->heartbeat_enabled == true)
    {
        /* BDBG_MSG((" check eCM's heartbeat every 2 seconds")); */
        need_restart = true;

        /* to avoid false alarm that eCM is dead or reset:
           (1) use double RPC calls to verify eCM status
           (2) eCM notification also indicates eCM is alive
        */
        if (deviceHandle->deviceStatus == NEXUS_3255DeviceStatus_eOperational)
        {

#if NEXUS_POWER_MANAGEMENT
            BKNI_AcquireMutex(deviceHandle->rpcMutex);
            BDBG_MSG(("check_rpc(): Enter 1 heartbeat Thread"));
#endif
            retCode = BRPC_CallProc(deviceHandle->rpc_handle,
                                    BRPC_ProcId_ADS_GetVersion,
                                    (const uint32_t *)&Param,
                                     sizeof(Param)/4,
                                    (uint32_t *)&outVerParam,
                                    sizeInLong(outVerParam), &retVal);
#if NEXUS_POWER_MANAGEMENT
            BDBG_MSG(("check_rpc(): End 1 heartbeat Thread"));
            BKNI_ReleaseMutex(deviceHandle->rpcMutex);
#endif
            need_restart = (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS) && (!deviceHandle->rpc_notification_count);
            if (need_restart == true && deviceHandle->deviceStatus == NEXUS_3255DeviceStatus_eOperational) /* do one more test */
            {
                BKNI_Sleep(100);
#if NEXUS_POWER_MANAGEMENT
                BKNI_AcquireMutex(deviceHandle->rpcMutex);
                BDBG_MSG(("check_rpc(): Enter 2 heartbeat Thread"));
#endif
                retCode = BRPC_CallProc(deviceHandle->rpc_handle,
                                        BRPC_ProcId_ADS_GetVersion,
                                        (const uint32_t *)&Param,
                                        sizeof(Param)/4,
                                        (uint32_t *)&outVerParam,
                                        sizeInLong(outVerParam),
                                        &retVal);
#if NEXUS_POWER_MANAGEMENT
                BDBG_MSG(("check_rpc(): End 2 heartbeat Thread"));
                BKNI_ReleaseMutex(deviceHandle->rpcMutex);
#endif
                need_restart = (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS)&&(!deviceHandle->rpc_notification_count);
                if (need_restart) deviceHandle->deviceStatus = NEXUS_3255DeviceStatus_eReset;
            }
        }
        while (need_restart == true && deviceHandle->heartbeat_enabled == true)
        {
            BDBG_ERR((" Delay 10 seconds and start polling eCM "));
            retCode = BKNI_WaitForEvent(deviceHandle->heartbeatEvent, 10000);
            if (deviceHandle->heartbeat_enabled == false) break;
            /*  delay max 10 seconds or wait for EcmRmagnumReady event then start polling eCM */
            BKNI_ResetEvent(deviceHandle->heartbeatEvent);
            count = 0;
            if (NEXUS_GetEnv("no_3255") == NULL)
            {
                while (deviceHandle->heartbeat_enabled == true) {
                    BDBG_MSG((" waiting for eCM booting up!"));
#if NEXUS_POWER_MANAGEMENT
                    BKNI_AcquireMutex(deviceHandle->rpcMutex);
                    BDBG_MSG(("check_rpc(): Enter 3 heartbeat Thread"));
#endif
                    retCode = BRPC_CallProc(deviceHandle->rpc_handle,
                                            BRPC_ProcId_InitSession,
                                            (const uint32_t *)&Param,
                                            sizeof(Param)/4,
                                            NULL, 0, &retVal);
#if NEXUS_POWER_MANAGEMENT
                    BDBG_MSG(("check_rpc(): End 3 heartbeat Thread"));
                    BKNI_ReleaseMutex(deviceHandle->rpcMutex);
#endif
                    need_restart = (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS);
                    if (need_restart == false ||  ++count >= 5) break;
                    BKNI_Sleep(100);
                }
            }
            if (need_restart == true)
            {
                BDBG_ERR((" eCM is dead! "));
                NEXUS_LockModule();
                deviceHandle->deviceStatus = NEXUS_3255DeviceStatus_eFailed;
                /* ask application to manually reset eCM*/
                NEXUS_TaskCallback_Fire(deviceHandle->eCMStatusCallback);
                NEXUS_UnlockModule();
            }
            else
            {
                BDBG_ERR((" eCM's heartbeat is back. Re-initialize ADS/AOB/AUS "));
                NEXUS_LockModule();
                NEXUS_Frontend_P_Close3255Channels(deviceHandle);
                /* coverity[freed_arg] */
                errCode = NEXUS_Frontend_P_Open3255Channels(deviceHandle);
                if (errCode)
                {
                    BDBG_ERR((" Error in Re-initialize ADS/AOB/AUS, what to do? "));
                    NEXUS_UnlockModule();
                    continue;
                }
                else
                {
                    for ( i = 0; i < deviceHandle->numOfQamFrontends && NULL!=deviceHandle->frontendHandle[i]; i++)
                    {
                        NEXUS_3255ChannelHandle channelHandle=deviceHandle->frontendHandle[i]->pDeviceHandle;
                        BADS_InstallCallback(channelHandle->ads_chn, BADS_Callback_eLockChange,
                            (BADS_CallbackFunc)NEXUS_Frontend_P_3255QamChannelLockChange, (void*)channelHandle->lockDriverCallback);
                    }
                    #if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
                    if (deviceHandle->aob)
                    {
                        NEXUS_3255ChannelHandle channelHandle=deviceHandle->frontendHandle[deviceHandle->oobFrontendIndex]->pDeviceHandle;
                        BAOB_InstallCallback(deviceHandle->aob, BAOB_Callback_eLockChange,
                            (BAOB_CallbackFunc)NEXUS_Frontend_P_3255OobChannelLockChange, (void*)channelHandle->lockAppCallback);
                    }
                    #endif
                }
                NEXUS_TaskCallback_Fire(deviceHandle->eCMStatusCallback);
                NEXUS_UnlockModule();
            }
        }
        /* BDBG_MSG((" Done checking eCM heartbeat ")); */
next_try:
        NEXUS_LockModule();
        deviceHandle->rpc_notification_count = 0; /* clear notification count*/
        NEXUS_UnlockModule();
        retCode = BKNI_WaitForEvent(deviceHandle->heartbeatEvent, 2000);
        if (deviceHandle->heartbeat_enabled == false) break;
        if (deviceHandle->rpc_notification_count) {
            BDBG_MSG((" Got %d RPC notification, no need to poll",deviceHandle->rpc_notification_count ));
            goto next_try;
        }
    }

}

/**********************************************************************************
 Summary:
 Private API for closing a frontend handle. It's hooked up to the main
 close funtion pointer in the frontend handle during frontend open.
 **********************************************************************************/
static void NEXUS_Frontend_P_Close3255Channel(NEXUS_FrontendHandle handle)
{
    NEXUS_3255DeviceHandle deviceHandle;
    NEXUS_3255ChannelHandle channelHandle;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    channelHandle =(NEXUS_3255ChannelHandle) handle->pDeviceHandle;
    deviceHandle = channelHandle->deviceHandle;
    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);

    /* uninstall lock change callback */
    if (channelHandle->ads_chn)
    {
        BERR_Code rc;
        rc = BADS_InstallCallback(channelHandle->ads_chn, BADS_Callback_eLockChange, NULL, NULL);
        if (rc) {rc = BERR_TRACE(rc);}
    }
    else
    {
        #if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
        if (deviceHandle->aob)
        {
        BERR_Code rc;
            rc = BAOB_InstallCallback(deviceHandle->aob, BAOB_Callback_eLockChange, NULL, NULL);
            if (rc) { rc = BERR_TRACE(rc);}
        }
        #endif
    }

    if (channelHandle->retuneTimer)
    {
            /* in case channel tuned with autoAcquire wasn't untuned prior to close. */
         NEXUS_CancelTimer(channelHandle->retuneTimer);
         channelHandle->retuneTimer = NULL;
    }

    if(channelHandle->lockDriverCallback)
    {
        NEXUS_CallbackHandler_Shutdown(channelHandle->lockDriverCBHandler);
    }
    if ( NULL != channelHandle->lockDriverCallback)
    {
        NEXUS_TaskCallback_Destroy(channelHandle->lockDriverCallback);
        channelHandle->lockDriverCallback = NULL;
    }
    if ( NULL != channelHandle->lockAppCallback)
    {
        NEXUS_TaskCallback_Destroy(channelHandle->lockAppCallback);
        channelHandle->lockAppCallback = NULL;
    }
    if ( NULL != channelHandle->asyncStatusAppCallback)
    {
        NEXUS_TaskCallback_Destroy(channelHandle->asyncStatusAppCallback);
        channelHandle->asyncStatusAppCallback = NULL;
    }

    if(NULL != channelHandle->ads_status.statusCallback)
    {
        NEXUS_TaskCallback_Destroy(channelHandle->ads_status.statusCallback);
        channelHandle->ads_status.statusCallback = NULL;
    }

    NEXUS_Frontend_P_Destroy(handle);
    deviceHandle->frontendHandle[channelHandle->frontendIndex] = NULL;
    if(channelHandle->tnr)
    {
        BTNR_Close(channelHandle->tnr);
        channelHandle->tnr = NULL;
    }

    if(channelHandle->ads_chn)
    {
        BADS_CloseChannel(channelHandle->ads_chn);
        channelHandle->ads_chn = NULL;
    }
    BKNI_Free(channelHandle);
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3255GetTemperature(void *handle, NEXUS_FrontendTemperature *pTemperature)
{
    NEXUS_Error ret = NEXUS_SUCCESS;

    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;
    BRPC_Param_ECM_ReadDieTemperature outParam;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BDBG_ASSERT(deviceHandle);

    retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ECM_ReadDieTemperature,
        NULL, 0, (uint32_t *)&outParam, sizeInLong(outParam), &retVal);

    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" Unable to get the 3383 Die Temperature via RPC to 3383"));
        return NEXUS_INVALID_PARAMETER;
    }

    pTemperature->temperature  = outParam.TempInDot00DegC;

    return ret;
}

static NEXUS_Error NEXUS_Frontend_P_3255Standby(void *handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)channelHandle->deviceHandle;
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_ECM_PowerSaver Param;
    BERR_Code retVal;
    BSTD_UNUSED(enabled);

    BDBG_WRN(("Channel Handle %p , Device Handle %p, RPC handle %p", (void *)channelHandle, (void *)deviceHandle, (void *)deviceHandle->rpc_handle));

    BDBG_ASSERT( deviceHandle->rpc_handle);

    if (deviceHandle->currentPowerState == pSettings->mode)
    {
        BDBG_ERR(("3255 Device already in standby mode: %d", pSettings->mode));
        return NEXUS_SUCCESS;
    }

    Param.devId = BRPC_DevId_3255;

    if ((pSettings->mode == NEXUS_StandbyMode_ePassive) || (pSettings->mode == NEXUS_StandbyMode_eDeepSleep))
        Param.mode = BRPC_ECM_PowerMode_Standby1;
    else if ((pSettings->mode == NEXUS_StandbyMode_eActive) || (pSettings->mode == NEXUS_StandbyMode_eOn))
        Param.mode = BRPC_ECM_PowerMode_On;
    else
    {
        BDBG_ERR((" Unsupported standby mode"));
        return NEXUS_INVALID_PARAMETER;
    }

    retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ECM_PowerSaver,
            (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" Unable to transistion to Standby mode "));
        return NEXUS_INVALID_PARAMETER;
    }

    deviceHandle->currentPowerState = pSettings->mode;

    return NEXUS_SUCCESS;
}

/**********************************************************************************
 Summary:
 Private API for getting QAM status aynchronously. It's hooked up to the main
 getQamAsyncStatus funtion pointer in the frontend handle during frontend open.
 **********************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_3255_GetAsyncQamStatus(
    void *handle,
    NEXUS_FrontendQamStatus *pStatus)
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BADS_Status *pSt;


    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);

    pSt = &channelHandle->ads_status.status;
    pStatus->fecLock = pSt->isFecLock;
    pStatus->receiverLock = pSt->isQamLock;
    pStatus->symbolRate = pSt->rxSymbolRate;
    pStatus->ifAgcLevel = pSt->agcIntLevel;
    pStatus->rfAgcLevel = pSt->agcExtLevel;
    pStatus->carrierFreqOffset = pSt->carrierFreqOffset;
    pStatus->carrierPhaseOffset = pSt->carrierPhaseOffset;

    pStatus->dsChannelPower = pSt->dsChannelPower;
    pStatus->mainTap = pSt->mainTap;
    pStatus->equalizerGain = pSt->equalizerGain;
    pStatus->interleaveDepth = pSt->interleaveDepth;

    pStatus->snrEstimate = pSt->snrEstimate*100/256;
    pStatus->fecCorrected = pSt->correctedCount;
    pStatus->fecUncorrected = pSt->uncorrectedCount;
    pStatus->berEstimate = pSt->berRawCount;
    pStatus->goodRsBlockCount = pSt->goodRsBlockCount;
    pStatus->postRsBer = pSt->postRsBER;
    pStatus->postRsBerElapsedTime = pSt->elapsedTimeSec;
    pStatus->spectrumInverted = pSt->isSpectrumInverted;
    pStatus->settings = channelHandle->last_ads;

    BDBG_MSG((" STATUS : fec_lock %d \t qam_lock %d \t  agcIntLevel %d \t agcExtLevel %d \t snr_estimate %d "
              "\t fec_corr_cnt %d \t fec_uncorr_cnt %d\t ber_estimate %d",pStatus->fecLock, pStatus->receiverLock,
               pStatus->ifAgcLevel, pStatus->rfAgcLevel, pStatus->snrEstimate,pStatus->fecCorrected,
               pStatus->fecUncorrected, pStatus->berEstimate));
    BDBG_MSG((" STATUS DS: ds power %d in 1/10 dBmV", pStatus->dsChannelPower));
    return NEXUS_SUCCESS;

}

/**********************************************************************************
 Summary:
 Private API for requesting QAM status aynchronously. It's hooked up to the main
 requestQamAsyncStatus funtion pointer in the frontend handle during frontend open.
 **********************************************************************************/
static NEXUS_Error NEXUS_Frontend_P_3255_RequestAsyncQamStatus(
    void *handle)
{
    BERR_Code rc;
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    unsigned chn_num = channelHandle->chn_num;
    BADS_Version version;
    NEXUS_3255QamChannelStatus *qamChannelStatus;
    BRPC_Param_ADS_GetDsChannelPower param;
    BERR_Code retCode = BERR_SUCCESS;
    BERR_Code retVal;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);

    BDBG_MSG((" start NEXUS_Frontend_3255_GetQamStatus"));
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);

    qamChannelStatus = &channelHandle->ads_status;

    rc = BADS_GetStatus(channelHandle->ads_chn,  &qamChannelStatus->status);
    if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    BDBG_MSG((" STATUS : fec_lock %d \t qam_lock %d \t  agcIntLevel %d \t st.agcExtLevel %d \t "
              "snr_estimate %d \t fec_corr_cnt %d \t fec_uncorr_cnt %d\t ber_estimate %d",
              qamChannelStatus->status.isFecLock,qamChannelStatus->status.isQamLock,
              qamChannelStatus->status.agcIntLevel,qamChannelStatus->status.agcExtLevel,
              qamChannelStatus->status.snrEstimate,qamChannelStatus->status.correctedCount,
              qamChannelStatus->status.uncorrectedCount,qamChannelStatus->status.berRawCount));
    BDBG_MSG((" end NEXUS_Frontend_3255_GetQamStatus"));

    rc = BADS_GetVersion(deviceHandle->ads, &version);
    if (rc) return BERR_TRACE(rc);

    BDBG_ASSERT( deviceHandle->rpc_handle);
    param.devId = ((version.minVer <= 0x9) ? BRPC_DevId_3255_DS0 : BRPC_DevId_ECM_DS0) + chn_num;
    retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ADS_GetDsChannelPower,
                            (const uint32_t *)&param, sizeInLong(param), NULL, 0, &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" NEXUS_Frontend_3255_GetDSPower Fail"));
        return NEXUS_NOT_SUPPORTED;
    }
    return NEXUS_SUCCESS;
}

/*****************************************************************************************
 Summary:
 Private APIs for mapping the parser bands of a DOCSIS channel.
 (Applicable for DOCSIS devices that are connected to host transport using MTSIF interface)
 ****************************************************************************************/
NEXUS_Error NEXUS_Frontend_P_3255_ReapplyTransportSettings(void *handle)
{
#if NEXUS_HAS_MXT
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    NEXUS_Error rc;

    rc = NEXUS_Frontend_P_SetMtsifConfig(deviceHandle->frontendHandle[channelHandle->frontendIndex]);
    if (rc) { return BERR_TRACE(rc); }
#else
    BSTD_UNUSED(handle);
#endif
    return NEXUS_SUCCESS;
}

void NEXUS_Frontend_GetDefault3255DeviceSettings(
    NEXUS_3255DeviceSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->autoAcquire = false;
    pSettings->fastAcquire = false;
    pSettings->enableFEC = false; /* CableCARD will do FEC for OOB*/
    pSettings->rpcTimeout = 50;
    return;
}


NEXUS_3255DeviceHandle NEXUS_Frontend_Open3255Device(
    unsigned index,
    const NEXUS_3255DeviceSettings *pSettings
    )
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    BERR_Code errCode;
    unsigned i, num_ch, bonded_ch;
    BADS_Settings ads_cfg;
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    BTNR_3255Ob_Settings tnrOb3255_cfg;
    BAOB_Settings aob_cfg;
    BAUS_Settings aus_cfg;
#endif
    NEXUS_ThreadSettings thread_settings;
    NEXUS_3255DeviceHandle deviceHandle=NULL;
#if NEXUS_POWER_MANAGEMENT
    BERR_Code result;
#endif

    BDBG_MSG(("NEXUS_Frontend_Open3255Device >>>"));
    /*
     * Assuming that there is only one docsis device
     */
    BSTD_UNUSED(index);

    pFrontendDevice = NEXUS_FrontendDevice_P_Create();
    if (NULL == pFrontendDevice)
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;;
    }

    deviceHandle = BKNI_Malloc(sizeof(NEXUS_3255Device));
    if ( NULL == deviceHandle )
    {
        BKNI_Free(pFrontendDevice);
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    BKNI_Memset(deviceHandle, 0, sizeof(NEXUS_3255Device));
    BDBG_OBJECT_SET(deviceHandle, NEXUS_3255Device);

    deviceHandle->deviceSettings = *pSettings;
    deviceHandle->pGenericDeviceHandle = pFrontendDevice;
    pFrontendDevice->pDevice = (void *) deviceHandle;
    if (NEXUS_Frontend_P_Init3255DeviceRPC(deviceHandle) ) goto err_rpc;
    BDBG_WRN(("Device Handle %p, RPC handle %p", (void *)deviceHandle, (void *)deviceHandle->rpc_handle));
    errCode = BADS_3255_GetDefaultSettings( &ads_cfg, NULL);
    if ( errCode != BERR_SUCCESS ) goto err_init;

    ads_cfg.hGeneric = deviceHandle->rpc_handle;
    errCode = BADS_Open(&deviceHandle->ads, NULL, NULL, NULL, &ads_cfg);
    if ( errCode != BERR_SUCCESS ) goto err_init;

    errCode = BADS_Init(deviceHandle->ads);
    if ( errCode != BERR_SUCCESS ) goto err_init;

    /* get total ADS channel number*/
    errCode = BADS_GetTotalChannels(deviceHandle->ads, &num_ch);
    if (errCode!=BERR_SUCCESS) goto err_init;
    deviceHandle->numChannels = num_ch;
    BDBG_MSG(("docsis numChannels %u",num_ch));

#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    deviceHandle->channelCapabilities = BKNI_Malloc(sizeof(NEXUS_3255ChannelCapabilities)*(num_ch+1));
#else
    deviceHandle->channelCapabilities = BKNI_Malloc(sizeof(NEXUS_3255ChannelCapabilities)*num_ch);
#endif
    if(!deviceHandle->channelCapabilities) goto err_init;

    /* get number of bonded channels */
    errCode = BADS_GetBondingCapability(deviceHandle->ads, &bonded_ch);
    if (errCode!=BERR_SUCCESS) goto err_init;
    if(bonded_ch==num_ch)
    {
        BDBG_WRN(("DOCSIS operating in legacy mode. So using all the DOCSIS channels for data"));
    }
    else
    {
        if (bonded_ch == 0) BDBG_MSG(("no docsis bonded channel exists"));
        else BDBG_MSG(("docsis bonded channels 0-%u",bonded_ch-1));
    }
    deviceHandle->numDocsisChannels = bonded_ch;
    for(i=0;i<num_ch;i++)
    {
        if(i<bonded_ch)
        {
            deviceHandle->channelCapabilities[i].channelType = NEXUS_3255ChannelType_eDocsis;
        }
        else
        {
            deviceHandle->channelCapabilities[i].channelType = NEXUS_3255ChannelType_eInBand;
        }
    }

    /* Initialize non-Docsis video channel lock status */
    deviceHandle->NonCmControlledVideoChLockStatus = 0;

    /* Set the Initial Power state of the Docsis frontend device to "On" */
    deviceHandle->currentPowerState = NEXUS_StandbyMode_eOn;

#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    errCode = BTNR_3255Ob_GetDefaultSettings(&tnrOb3255_cfg, NULL);
    if (errCode!=BERR_SUCCESS) goto oob_done;
    tnrOb3255_cfg.hGeneric = deviceHandle->rpc_handle;

    /* BTNR_3255OB_SETTINGS_IFFREQ is deprecated (incorrectly assigns IF_FREQ based on host chip)
      use NEXUS_3255_OOB_TUNER_IFFREQ instead (to be defined in nexus_platform_features.h) */
#ifndef NEXUS_3255_OOB_TUNER_IFFREQ
    #define NEXUS_3255_OOB_TUNER_IFFREQ BTNR_3255OB_SETTINGS_IFFREQ
#endif
    tnrOb3255_cfg.ifFreq = NEXUS_3255_OOB_TUNER_IFFREQ;
    errCode = BTNR_3255Ob_Open(&deviceHandle->oobTnr, NULL, &tnrOb3255_cfg);
    if (errCode!=BERR_SUCCESS) goto oob_done;

    errCode = BAOB_GetDefaultSettings( &aob_cfg, NULL);
    if (errCode!=BERR_SUCCESS) goto oob_done;
    aob_cfg.hGeneric = deviceHandle->rpc_handle;
    aob_cfg.enableFEC = deviceHandle->deviceSettings.enableFEC;
    errCode = BAOB_Open(&deviceHandle->aob, NULL, NULL, NULL, &aob_cfg);
    if (errCode!=BERR_SUCCESS)
     {
        BDBG_ASSERT(0);
        goto oob_done;
    }

    errCode = BAUS_GetDefaultSettings(&aus_cfg, NULL);
    if (errCode!=BERR_SUCCESS) goto oob_done;
    aus_cfg.xtalFreq = BAUS_SETTINGS_XTALFREQ;
    aus_cfg.hGeneric = deviceHandle->rpc_handle;;
    errCode = BAUS_Open( &deviceHandle->aus, NULL, NULL, &aus_cfg);
    if (errCode!=BERR_SUCCESS) goto oob_done;

    deviceHandle->isOOBsupported = true;
    deviceHandle->channelCapabilities[num_ch].channelType = NEXUS_3255ChannelType_eOutOfBand;
oob_done:
#else
    deviceHandle->isOOBsupported = false;
#endif

#if NEXUS_HAS_MXT
    {
        /* open MXT */
        BMXT_Settings mxtSettings;
        BADS_Version version;
        uint16_t chipId;
        uint8_t chipRev;

        /* Retrieve Chip Id and Rev */
        errCode = BADS_GetVersion(deviceHandle->ads, &version);
        if (errCode) goto err_rpc;
        BDBG_WRN(("BADS_GetVersion: major 0x%x minor 0x%x", version.majVer, version.minVer));

        if (version.minVer <= 0x9)
        {
            BMXT_3383_GetDefaultSettings(&mxtSettings);
        }
        else
        {
            BMXT_3384_GetDefaultSettings(&mxtSettings);
            mxtSettings.MtsifTxCfg[0].TxClockPolarity = 0;
        }
        for (i=0; i<BMXT_NUM_MTSIF; i++) {
            mxtSettings.MtsifTxCfg[i].Enable = pSettings->mtsif;
            NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
            mxtSettings.MtsifTxCfg[i].Encrypt = NEXUS_TransportModule_P_IsMtsifEncrypted();
            NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        }
        mxtSettings.hRpc = deviceHandle->rpc_handle;

        chipId = (uint16_t)(version.majVer >> 16);

        if (chipId == 0x3383)
            mxtSettings.chip = BMXT_Chip_e3383;
        else if (chipId == 0x3843)
            mxtSettings.chip = BMXT_Chip_e3384;
        else
            goto err_rpc;

        chipRev = (uint8_t)(version.majVer & 0x0000000f);

        switch (chipRev)
        {
            case 0:
                mxtSettings.chipRev = BMXT_ChipRev_eA0;
                deviceHandle->numOfTsmfParsers = 2;
                break;
            case 1:
                mxtSettings.chipRev = BMXT_ChipRev_eA1;
                deviceHandle->numOfTsmfParsers = 2;
                break;
            case 2:
                mxtSettings.chipRev = BMXT_ChipRev_eB0;
                deviceHandle->numOfTsmfParsers = 8;
                break;
            default:
                goto err_rpc;
        }

        errCode = BMXT_Open(&deviceHandle->pGenericDeviceHandle->mtsifConfig.mxt, NULL, NULL, &mxtSettings);
        if ( errCode != BERR_SUCCESS ) goto err_init;

        errCode = NEXUS_Frontend_P_InitMtsifConfig(&deviceHandle->pGenericDeviceHandle->mtsifConfig, &mxtSettings);
        if (errCode!=BERR_SUCCESS) goto err_init;
    }
#endif

        /* Create Docsis status callback */
    deviceHandle->eCMStatusCallback = NEXUS_TaskCallback_Create(deviceHandle, NULL);
    if ( NULL == deviceHandle->eCMStatusCallback )
    {
            goto err_init;
        }
        /* start RPC notification thread*/
    deviceHandle->rpc_notification_enabled = true;
    if (NEXUS_GetEnv("no_3255") != NULL){
        BDBG_WRN(("Disabling RPC notification since 3255 is not used"));
            deviceHandle->rpc_notification_enabled = false;
    }
#if NEXUS_POWER_MANAGEMENT
    result = BKNI_CreateMutex(&deviceHandle->rpcMutex);
    if(BERR_SUCCESS == result)
    {
      BDBG_MSG(("NEXUS_Frontend_Open3255Device--CreateMutex success >>>"));
    }else
      BDBG_MSG(("NEXUS_Frontend_Open3255Device--CreateMutex Failed >>>"));
#endif

    NEXUS_Thread_GetDefaultSettings(&thread_settings);
        /* TODO:: set correct thread priority*/
    deviceHandle->rpc_notification = NEXUS_Thread_Create("rpc_notification",
                                                         NEXUS_Frontend_3255DeviceRpcNotificationThread,
                                                         (void*)deviceHandle,
                                                         &thread_settings);
    if (deviceHandle->rpc_notification == NULL)
    {
            BDBG_ERR((" can't create RPC notification thread"));
            goto err_init;
        }
        /* start 3255 heartbeat monitoring thread*/
    deviceHandle->heartbeat_enabled= true;
        NEXUS_Thread_GetDefaultSettings(&thread_settings);
        /* TODO:: set correct thread priority*/
    BKNI_CreateEvent(&deviceHandle->heartbeatEvent);
    BKNI_ResetEvent(deviceHandle->heartbeatEvent);
    deviceHandle->deviceStatus = NEXUS_3255DeviceStatus_eOperational;
    deviceHandle->heartbeat = NEXUS_Thread_Create("3255_Heartbeat",
                                                  NEXUS_Frontend_P_3255DeviceHeartbeatThread,
                                                  (void*)deviceHandle,
                                                  &thread_settings);

    if (deviceHandle->heartbeat == NULL)
    {
            BDBG_ERR((" can't create 3255_Heartbeat thread"));
            goto err_init;
    }

    BDBG_WRN(("Device Handle %p, RPC handle %p", (void *)deviceHandle, (void *)deviceHandle->rpc_handle));
    BDBG_MSG(("NEXUS_Frontend_Open3255Device %p>>>", (void *)deviceHandle));
    return deviceHandle;

err_init:
    if (deviceHandle->ads)
    {
        BADS_Close(deviceHandle->ads);
    }
    if(deviceHandle->channelCapabilities)
    {
        BKNI_Free(deviceHandle->channelCapabilities);
    }
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    if (deviceHandle->aus)
    {
        BAUS_Close(deviceHandle->aus);
    }
    if (deviceHandle->aob)
    {
        BAOB_Close(deviceHandle->aob);
    }
     if (deviceHandle->oobTnr)
     {
         BTNR_Close(deviceHandle->oobTnr);
     }
#endif
    deviceHandle->deviceStatus = NEXUS_3255DeviceStatus_eUninitialized;
    if (deviceHandle->heartbeat)
    {
        NEXUS_UnlockModule();       /* Need to unlock because heartbeat thread might be blocked on our lock. */
        deviceHandle->heartbeat_enabled = false;
        BKNI_SetEvent(deviceHandle->heartbeatEvent);
        BKNI_Sleep(100);
        NEXUS_Thread_Destroy(deviceHandle->heartbeat);
        NEXUS_LockModule();         /* The heartbeat thread should be gone by now.  Get the module lock again. */
    }

    if (deviceHandle->rpc_notification)
    {
        NEXUS_UnlockModule();       /* Need to unlock because rpc_notification thread might be blocked on our lock. */
        deviceHandle->rpc_notification_enabled = false;
        BKNI_Sleep(600);        /* time for task to finish */
        NEXUS_Thread_Destroy(deviceHandle->rpc_notification);
        NEXUS_LockModule();         /* The rpc_notification thread should be gone by now.  Get the module lock again. */
    }

    if (deviceHandle->heartbeatEvent)
        BKNI_DestroyEvent(deviceHandle->heartbeatEvent);

    if (deviceHandle->eCMStatusCallback)
            NEXUS_TaskCallback_Destroy(deviceHandle->eCMStatusCallback);

        BRPC_Close_SocketImpl(deviceHandle->rpc_handle);
err_rpc:
    BKNI_Free(deviceHandle);
err_malloc:
    return NULL;
}

void NEXUS_Frontend_Close3255Device(NEXUS_3255DeviceHandle deviceHandle)
{

    BDBG_ASSERT(deviceHandle);
    if(deviceHandle->channelCapabilities)
    {
        BKNI_Free(deviceHandle->channelCapabilities);
    }
    if (deviceHandle->ads)
    {
        BADS_Close(deviceHandle->ads);
    }
#if defined(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT)
    if (deviceHandle->aus)
    {
        BAUS_Close(deviceHandle->aus);
    }
    if (deviceHandle->aob)
    {
        BAOB_Close(deviceHandle->aob);
    }
#endif

#if NEXUS_HAS_MXT
    if (deviceHandle->pGenericDeviceHandle->mtsifConfig.mxt) {
        BMXT_Close(deviceHandle->pGenericDeviceHandle->mtsifConfig.mxt);
    }
#endif

    if (deviceHandle->heartbeat)
    {
        NEXUS_UnlockModule();       /* Need to unlock because heartbeat thread might be blocked on our lock. */
        deviceHandle->heartbeat_enabled = false;
        BKNI_SetEvent(deviceHandle->heartbeatEvent);
        BKNI_Sleep(100);
        NEXUS_Thread_Destroy(deviceHandle->heartbeat);
        NEXUS_LockModule();         /* The heartbeat thread should be gone by now.  Get the module lock again. */
        BKNI_DestroyEvent(deviceHandle->heartbeatEvent);
    }
    NEXUS_UnlockModule();           /* Need to unlock because rpc_notification thread might be blocked on our lock. */
    deviceHandle->rpc_notification_enabled = false;
    BKNI_Sleep(600); /* time for task to finish */
    NEXUS_Thread_Destroy(deviceHandle->rpc_notification);
#if NEXUS_POWER_MANAGEMENT
    BKNI_DestroyMutex(deviceHandle->rpcMutex);
#endif
    NEXUS_LockModule();             /* The rpc_notification thread should be gone by now.  Get the module lock again. */
    NEXUS_TaskCallback_Destroy(deviceHandle->eCMStatusCallback);
    BRPC_Close_SocketImpl(deviceHandle->rpc_handle);
    if (deviceHandle->pGenericDeviceHandle)
        BKNI_Free(deviceHandle->pGenericDeviceHandle);
    BDBG_OBJECT_DESTROY(deviceHandle, NEXUS_3255Device);
    BKNI_Free(deviceHandle);
    return;
}

NEXUS_Error NEXUS_Frontend_Get3255DeviceCapabilities(
    NEXUS_3255DeviceHandle handle,
    NEXUS_3255DeviceCapabilities *pCapabilities
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_ASSERT((handle));
    pCapabilities->numOfDocsisChannels = handle->numDocsisChannels;
    pCapabilities->numOfQamChannels = handle->numChannels-handle->numDocsisChannels;
    pCapabilities->totalChannels = handle->numChannels;
    pCapabilities->isOobChannelSupported = handle->isOOBsupported;
    if(handle->isOOBsupported)
    {
        pCapabilities->totalChannels+=1;
    }
    pCapabilities->numOfTsmfParsers = handle->numOfTsmfParsers;
    return rc;
}

NEXUS_Error NEXUS_Frontend_Get3255DeviceGpioPinSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255DeviceGpioPin pin,
    NEXUS_3255DeviceGpioPinSettings *pSettings
    )
{
    /*TODO*/
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pin);
    BSTD_UNUSED(pSettings);
    return NEXUS_NOT_SUPPORTED;
}


NEXUS_Error NEXUS_Frontend_Set3255DeviceGpioPinSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255DeviceGpioPin pin,
    const NEXUS_3255DeviceGpioPinSettings *pSettings
    )
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle->pDeviceHandle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_POD_CardApplyPower Param;
    BERR_Code retVal;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT( deviceHandle->rpc_handle);
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);
    switch (pin) {
        case NEXUS_3255DeviceGpioPin_eOob:
            Param.devId = BRPC_DevId_3255;
            Param.powerMode = (pSettings->mode == NEXUS_GpioMode_eOutputPushPull) ?
                               ENABLE_POD_OUTPINS : DISABLE_POD_OUTPINS;
            BDBG_MSG((" NEXUS_3255GpioPin_eOob %s", (Param.powerMode == ENABLE_POD_OUTPINS) ? "enable" : "disable" ));
            retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_POD_CardApplyPower,
                                    (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal);
            if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
            {
                BDBG_ERR((" NEXUS_3255GpioPin_eOob Fail"));
                return NEXUS_INVALID_PARAMETER;
            }
            break;
        default:
            BDBG_WRN((" unknown NEXUS_3255GpioPin"));
            return NEXUS_NOT_SUPPORTED;
    }
    return NEXUS_SUCCESS;

}

void NEXUS_Frontend_GetDefault3255ChannelSettings(
    NEXUS_3255ChannelSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->channelNumber = 0;
    pSettings->enableTsmfSupport = false;
    return;
}

static NEXUS_Error NEXUS_Frontend_P_3255TransmitDebugPacket(void* handle, NEXUS_FrontendDebugPacketType type,const uint8_t *pBuffer, size_t size)
{
#if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BERR_Code retCode = BERR_SUCCESS;

    /* The Starvue transmit packet size is fixed at 54 bytes */
    if ((handle == NULL) || (pBuffer == NULL) || (size != 54) || (type != NEXUS_FrontendDebugPacketType_eOob))
    {
        BDBG_ERR(("Parameters is not valid"));
        return NEXUS_INVALID_PARAMETER;
    }
    BDBG_ASSERT(channelHandle);
    BDBG_ASSERT(deviceHandle);
    BDBG_ASSERT(deviceHandle->aus);

    retCode = BAUS_TransmitStarvuePkt(deviceHandle->aus, (uint8_t *)pBuffer, size);
    if (retCode != BERR_SUCCESS){
        BDBG_ERR(("NOT SUCCESS"));
        rc = (retCode = BERR_OUT_OF_DEVICE_MEMORY)? NEXUS_OUT_OF_DEVICE_MEMORY : NEXUS_INVALID_PARAMETER;
    }
    return rc;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(type);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(size);
    return NEXUS_NOT_SUPPORTED;
#endif
}

NEXUS_FrontendHandle NEXUS_Frontend_Open3255Channel(
    NEXUS_3255DeviceHandle deviceHandle,
    const NEXUS_3255ChannelSettings *pSettings
    )
{
    BERR_Code errCode, rc;
    NEXUS_FrontendHandle frontendHandle;
    unsigned int chn_num;
    NEXUS_3255ChannelHandle channelHandle;
    BTNR_3255Ib_Settings tnr3255_cfg;
    BADS_ChannelSettings chn_cfg;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(pSettings->channelNumber <= deviceHandle->numChannels);

    chn_num = pSettings->channelNumber;
    channelHandle = (NEXUS_3255ChannelHandle)BKNI_Malloc(sizeof(NEXUS_3255Channel));

    if ( NULL == channelHandle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_init;
    }
    BKNI_Memset((void *)channelHandle,0,sizeof(NEXUS_3255Channel));
    BKNI_Memcpy((void *)&channelHandle->channelSettings,(void*)pSettings,sizeof(*pSettings));
    channelHandle->deviceHandle = deviceHandle;
    BDBG_WRN(("Handle %p, Device Handle %p num %x", (void *)channelHandle, (void *)channelHandle->deviceHandle, chn_num));
    deviceHandle->frontendHandle[deviceHandle->numOfFrontends] = frontendHandle = NEXUS_Frontend_P_Create(channelHandle);
    /* Establish device capabilities */
    if ( deviceHandle->channelCapabilities[pSettings->channelNumber].channelType == NEXUS_3255ChannelType_eInBand)
    {
        rc = BTNR_3255Ib_GetDefaultSettings(&tnr3255_cfg, NULL);
        if (rc != BERR_SUCCESS) goto err_init;
        tnr3255_cfg.hGeneric = deviceHandle->rpc_handle;
        tnr3255_cfg.ifFreq = BTNR_3255Ib_SETTINGS_IFFREQ;
        tnr3255_cfg.devId += chn_num;
        rc =  BTNR_3255Ib_Open(&channelHandle->tnr, NULL, NULL, NULL, &tnr3255_cfg);
        if (rc != BERR_SUCCESS) goto err_init;

        rc = BADS_GetChannelDefaultSettings( deviceHandle->ads, chn_num, &chn_cfg);
        if (rc!=BERR_SUCCESS) goto err_init;
        chn_cfg.autoAcquire = deviceHandle->deviceSettings.autoAcquire;
        chn_cfg.fastAcquire = deviceHandle->deviceSettings.fastAcquire;
        rc = BADS_OpenChannel( deviceHandle->ads,&channelHandle->ads_chn, chn_num, &chn_cfg);
        if (rc!=BERR_SUCCESS) goto err_init;
    if ( NULL == frontendHandle )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_init;
    }
        frontendHandle->pGenericDeviceHandle = deviceHandle->pGenericDeviceHandle;
        frontendHandle->capabilities.docsis = false;
        frontendHandle->capabilities.qam = true;
        frontendHandle->capabilities.outOfBand = true;
        frontendHandle->capabilities.upstream = true;
        BKNI_Memset(frontendHandle->capabilities.qamModes, true, sizeof(frontendHandle->capabilities.qamModes));
        /* bind functions*/
        frontendHandle->tuneQam = NEXUS_Frontend_P_3255QamChannelTune;
        frontendHandle->getQamStatus = NEXUS_Frontend_P_Get3255QamChannelStatus;
        frontendHandle->getSoftDecisions = NEXUS_Frontend_P_Get3255ChannelSoftDecisions;
        frontendHandle->untune = NEXUS_Frontend_P_3255QamChannelUnTune;
        frontendHandle->getTemperature = NEXUS_Frontend_P_3255GetTemperature;
        frontendHandle->standby = NEXUS_Frontend_P_3255Standby;
        frontendHandle->requestQamAsyncStatus = NEXUS_Frontend_P_3255_RequestAsyncQamStatus;
        frontendHandle->getQamAsyncStatus = NEXUS_Frontend_P_3255_GetAsyncQamStatus;
        frontendHandle->getFastStatus = NEXUS_Frontend_P_3255_GetFastStatus;
        frontendHandle->transmitDebugPacket= NEXUS_Frontend_P_3255TransmitDebugPacket;
        BDBG_MSG(("deviceHandle->numOfQamFrontends %u",deviceHandle->numOfQamFrontends));
        deviceHandle->numOfQamFrontends++;
    }
    #if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
    else
    {
        frontendHandle->pGenericDeviceHandle = deviceHandle->pGenericDeviceHandle;
        channelHandle->tnr = deviceHandle->oobTnr;
        frontendHandle->capabilities.docsis = false;
        frontendHandle->capabilities.qam = false;
        frontendHandle->capabilities.outOfBand = true;
        frontendHandle->capabilities.upstream = true;
        BKNI_Memset(frontendHandle->capabilities.outOfBandModes, true, sizeof(frontendHandle->capabilities.outOfBandModes));
        BKNI_Memset(frontendHandle->capabilities.upstreamModes, true, sizeof(frontendHandle->capabilities.upstreamModes));
        /* bind functions*/
        frontendHandle->tuneOutOfBand = NEXUS_Frontend_P_3255OobChannelTune;
        frontendHandle->getOutOfBandStatus = NEXUS_Frontend_P_Get3255OobChannelStatus;
        frontendHandle->untune = NEXUS_Frontend_P_3255OobChannelUnTune;
        frontendHandle->tuneUpstream = NEXUS_Frontend_P_3255UsChannelTune;
        frontendHandle->getUpstreamStatus = NEXUS_Frontend_P_Get3255UsChannelStatus;
        frontendHandle->getTemperature = NEXUS_Frontend_P_3255GetTemperature;
        frontendHandle->standby = NEXUS_Frontend_P_3255Standby;
        deviceHandle->oobFrontendIndex = deviceHandle->numOfFrontends;
        frontendHandle->transmitDebugPacket= NEXUS_Frontend_P_3255TransmitDebugPacket;
    }
    #endif

    if(deviceHandle->deviceSettings.mtsif)
    {
        if(pSettings->enableTsmfSupport && deviceHandle->numOfUsedTsmfParsers < deviceHandle->numOfTsmfParsers)
        {
            channelHandle->tsmfParserIndex = deviceHandle->numOfUsedTsmfParsers;
            deviceHandle->numOfUsedTsmfParsers++;
        }
        else
        {
            if(deviceHandle->numOfTsmfParsers &&
               deviceHandle->numOfUsedTsmfParsers >= deviceHandle->numOfTsmfParsers)
            {
                BDBG_WRN(("User requested TSMF parsing can't be enabled because all the available parsers are already acquired"));
                channelHandle->channelSettings.enableTsmfSupport = false;
            }
            else
            {
                BDBG_MSG(("TSMF parsing disabled for this channel"));
            }
        }
        frontendHandle->reapplyTransportSettings = NEXUS_Frontend_P_3255_ReapplyTransportSettings;
    }

    frontendHandle->close = NEXUS_Frontend_P_Close3255Channel;
    frontendHandle->getType = NEXUS_Frontend_P_GetType3255Channel;
    /* Create app callback */
    channelHandle->lockAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
    if ( NULL == channelHandle->lockAppCallback)
    {
        goto err_app_callback;
    }
    /* Create async status app callback */
    channelHandle->asyncStatusAppCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
    if ( NULL == channelHandle->asyncStatusAppCallback)
    {
        goto err_app_callback;
    }
        /* install callback to  notify of lock/unlock change */
    if ( deviceHandle->channelCapabilities[pSettings->channelNumber].channelType == NEXUS_3255ChannelType_eInBand)
    {
        /* Create driver callback for re-acquire check */
        channelHandle->lockDriverCallback = NEXUS_TaskCallback_Create(frontendHandle, NULL);
        if ( NULL == channelHandle->lockDriverCallback)
        {
            goto err_driver_callback;
        }

        rc = BADS_InstallCallback(channelHandle->ads_chn, BADS_Callback_eLockChange,
                                  (BADS_CallbackFunc)NEXUS_Frontend_P_3255QamChannelLockChange, (void*)channelHandle->lockDriverCallback);
        if (rc!=BERR_SUCCESS) goto err_driver_callback;
        NEXUS_CallbackHandler_Init(channelHandle->lockDriverCBHandler, NEXUS_Frontend_P_Check3255ChannelReacquireStatus, (void *)channelHandle);
    }
    #if NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT
    else
    {
        rc = BAOB_InstallCallback(deviceHandle->aob, BAOB_Callback_eLockChange,
                                  (BAOB_CallbackFunc)NEXUS_Frontend_P_3255OobChannelLockChange, (void*)channelHandle->lockAppCallback);
        if (rc!=BERR_SUCCESS) goto err_app_callback;
    }
    #endif
    /* save channel number in channelHandle*/
    channelHandle->frontendIndex = deviceHandle->numOfFrontends;
    deviceHandle->numOfFrontends++;
    channelHandle->chn_num = chn_num;
    channelHandle->deviceHandle = deviceHandle;

    /* Success */
    return frontendHandle;

err_driver_callback:
    if (channelHandle->lockDriverCallback) {
        NEXUS_TaskCallback_Destroy(channelHandle->lockDriverCallback);
    }
err_app_callback:
    if (channelHandle->lockAppCallback) {
        NEXUS_TaskCallback_Destroy(channelHandle->lockAppCallback);
    }
    if (channelHandle->asyncStatusAppCallback) {
        NEXUS_TaskCallback_Destroy(channelHandle->asyncStatusAppCallback);
    }
err_init:
    if(deviceHandle->frontendHandle[deviceHandle->numOfFrontends])
    {
        NEXUS_Frontend_P_Destroy(deviceHandle->frontendHandle[deviceHandle->numOfFrontends]);
        deviceHandle->frontendHandle[deviceHandle->numOfFrontends] = NULL;
    }
    if(channelHandle->tnr)
    {
        BTNR_Close(channelHandle->tnr);
    }
    if(channelHandle->ads_chn)
    {
        BADS_CloseChannel(channelHandle->ads_chn);
    }
    if(channelHandle)
    {
        BKNI_Free(channelHandle);
    }
    return NULL;
}

NEXUS_Error NEXUS_Frontend_Set3255ChannelSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_3255ChannelSettings *pSettings
    )
{
    NEXUS_3255ChannelHandle channelHandle = handle->pDeviceHandle;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)channelHandle->deviceHandle;
    NEXUS_Error ret = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BKNI_Memcpy((void *)&channelHandle->channelSettings,(void *)pSettings,sizeof(*pSettings));
    if (deviceHandle->eCMStatusCallback)
        NEXUS_TaskCallback_Set(deviceHandle->eCMStatusCallback, &(pSettings->docsisStateChange));

    if(deviceHandle->deviceSettings.mtsif)
    {
        if(pSettings->enableTsmfSupport &&
           deviceHandle->numOfUsedTsmfParsers < deviceHandle->numOfTsmfParsers &&
           channelHandle->frontendIndex!=deviceHandle->oobFrontendIndex)
        {
            channelHandle->tsmfParserIndex = deviceHandle->numOfUsedTsmfParsers;
            deviceHandle->numOfUsedTsmfParsers++;
            BDBG_MSG(("DOCSIS ch %u TsmfParser Index %u frontendIndex %u",channelHandle->chn_num,deviceHandle->numOfUsedTsmfParsers,channelHandle->frontendIndex));
        }
        else
        {
            if(deviceHandle->numOfTsmfParsers &&
               deviceHandle->numOfUsedTsmfParsers >= deviceHandle->numOfTsmfParsers)
            {
                BDBG_WRN(("User requested TSMF parsing can't be enabled because all the available parsers are already acquired"));
                channelHandle->channelSettings.enableTsmfSupport = false;
            }
            else
            {
                BDBG_MSG(("TSMF parsing disabled for this channel"));
            }
        }
    }
    return ret;
}

void NEXUS_Frontend_Get3255ChannelSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_3255ChannelSettings *pSettings
    )
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle->pDeviceHandle;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)channelHandle->deviceHandle;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);

    BDBG_ASSERT(NULL != pSettings);
    *pSettings = channelHandle->channelSettings;
}

NEXUS_Error NEXUS_Frontend_Get3255ChannelCapabilities(
    NEXUS_3255DeviceHandle handle,
    unsigned channel,
    NEXUS_3255ChannelCapabilities *pCapabilities
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_ASSERT(handle);
    BKNI_Memcpy((void *)pCapabilities,(void*)&handle->channelCapabilities[channel],
                sizeof(NEXUS_3255ChannelCapabilities));
    return rc;
}

NEXUS_Error NEXUS_Frontend_GetDocsisChannelStatus(
    NEXUS_FrontendHandle handle,
    unsigned docsisChannel,
    NEXUS_DocsisChannelStatus *pStatus
    )
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle->pDeviceHandle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_XXX_Get Param;
    BRPC_Param_ECM_GetStatus outParam;
    BERR_Code retVal;
    BADS_Version version;
#if 0
    BRPC_Param_ADS_GetDsChannelPower param;
#endif

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT( deviceHandle->rpc_handle);

    if (docsisChannel >= deviceHandle->numDocsisChannels)
    {
        BDBG_ERR(("Invalid Docsis Index: %u", docsisChannel));
        return NEXUS_INVALID_PARAMETER;
    }

    BKNI_Memset(pStatus, 0, sizeof(NEXUS_DocsisChannelStatus));
    pStatus->deviceStatus = deviceHandle->deviceStatus;
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_SUCCESS);

    retVal = BADS_GetVersion(deviceHandle->ads, &version);
    if (retVal) return NEXUS_NOT_SUPPORTED;

    Param.devId = ((version.minVer <= 0x9) ? BRPC_DevId_3255_DS0 : BRPC_DevId_ECM_DS0) + docsisChannel;
    BDBG_MSG((" NEXUS_Frontend_3255_GetDocsisStatus"));
    retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ECM_GetStatus, (const uint32_t *)&Param,
                            sizeInLong(Param), (uint32_t *)&outParam, sizeInLong(outParam), &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" NEXUS_Frontend_3255_GetDocsisStatus Fail"));
        return NEXUS_NOT_SUPPORTED;
    }

    pStatus->downstreamCenterFreq = outParam.downstreamCenterFreq;
    pStatus->downstreamPowerLevel = (version.minVer <=1) ? outParam.downstreamPowerLevel : 0;
    pStatus->downstreamCarrierLock = outParam.downstreamCarrierLock;
    pStatus->channelScdmaStatus = (NEXUS_3255ChannelScdmaType)outParam.channelScdmaStatus;
    pStatus->upstreamModType = (NEXUS_3255ChannelUsModType)outParam.upstreamModuType;
    pStatus->upstreamXmtCenterFreq = outParam.upstreamXmtCenterFreq;
    pStatus->upstreamPowerLevel = outParam.upstreamPowerLevel;
    pStatus->upStreamSymbolrate = outParam.upStreamSymbolrate;
    pStatus->lastKnownGoodFreq = outParam.lastKnownGoodFreq;
    pStatus->snrEstimate = outParam.snrEstimated;

    pStatus->macAddress[0] = (outParam.ecmMacAddressHi>>24)&0xff;
    pStatus->macAddress[1] = (outParam.ecmMacAddressHi>>16)&0xff;
    pStatus->macAddress[2] = (outParam.ecmMacAddressHi>>8)&0xff;
    pStatus->macAddress[3] = (outParam.ecmMacAddressHi)&0xff;
    pStatus->macAddress[4] = (outParam.ecmMacAddressLo>>8)&0xff;
    pStatus->macAddress[5] = (outParam.ecmMacAddressLo)&0xff;

    if (outParam.isEcmIpMode)
    {
        b_convert_ipaddr((uint8_t *)pStatus->ipv4Address,outParam.ecmIpAddress);
        b_convert_ipaddr((uint8_t *)&pStatus->ipv6Address[0],outParam.ecmIpv6Address0);
        b_convert_ipaddr((uint8_t *)&pStatus->ipv6Address[4],outParam.ecmIpv6Address1);
        b_convert_ipaddr((uint8_t *)&pStatus->ipv6Address[8],outParam.ecmIpv6Address2);
        b_convert_ipaddr((uint8_t *)&pStatus->ipv6Address[12],outParam.ecmIpv6Address3);
    }
    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Docsis_GetSystemInfo(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisSystemInfo *pSystemInfo)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)(handle->pDevice);
    BRPC_Param_ECM_GetSystemInfo systemInfo;
    BRPC_Param_XXX_Get Param;
    BERR_Code retVal;
    BADS_Version version;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT( deviceHandle->rpc_handle);

    BKNI_Memset(pSystemInfo, 0, sizeof(NEXUS_DocsisSystemInfo));
    BKNI_Memset(&systemInfo, 0, sizeof(BRPC_Param_ECM_GetSystemInfo));

    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_SUCCESS);

    retVal = BADS_GetVersion(deviceHandle->ads, &version);
    if (retVal) return NEXUS_NOT_SUPPORTED;

    Param.devId = ((version.minVer <= 0x9) ? BRPC_DevId_3255_DS0 : BRPC_DevId_ECM_DS0) + 0;
    BDBG_MSG((" NEXUS_Docsis_GetSystemInfo"));

    retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ECM_GetSystemInfo, (const uint32_t *)&Param,
                            sizeInLong(Param), (uint32_t *)&systemInfo, sizeInLong(systemInfo), &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" NEXUS_Docsis_GetSystemInfo Fail"));
        return NEXUS_NOT_SUPPORTED;
    }
    BKNI_Memcpy(pSystemInfo->ecmMfctName,systemInfo.ecmMfctName,64);
    pSystemInfo->ecmMfctName[63] = '\0';
    BKNI_Memcpy(pSystemInfo->ecmMfctOUI,systemInfo.ecmMfctOUI,64);
    pSystemInfo->ecmMfctOUI[63] = '\0';
    BKNI_Memcpy(pSystemInfo->ecmMfctDate,systemInfo.ecmMfctDate,64);
    pSystemInfo->ecmMfctDate[63] = '\0';
    BKNI_Memcpy(pSystemInfo->ecmSwVersion,systemInfo.ecmSwVersion,64);
    pSystemInfo->ecmSwVersion[63] = '\0';
    BKNI_Memcpy(pSystemInfo->ecmHwVersion,systemInfo.ecmHwVersion,64);
    pSystemInfo->ecmHwVersion[63] = '\0';
    BKNI_Memcpy(pSystemInfo->ecmSerialNum,systemInfo.ecmSerialNum,64);
    pSystemInfo->ecmSerialNum[63] = '\0';

    switch(systemInfo.ecmStandard)
    {
    case 0:
        pSystemInfo->ecmStandard = NEXUS_DocsisStandard_e1_x;
        break;
    case 1:
        pSystemInfo->ecmStandard = NEXUS_DocsisStandard_e2_x;
        break;
    case 2:
        pSystemInfo->ecmStandard = NEXUS_DocsisStandard_e3_x;
        break;
    default:
        pSystemInfo->ecmStandard = NEXUS_DocsisStandard_eMax;
        break;
    }

    switch(systemInfo.ecmIpMode)
    {
    case 0:
        pSystemInfo->ecmIpMode = NEXUS_EcmIpMode_eNone;
        break;
    case 1:
        /*IPV4 LAN address*/
        pSystemInfo->ecmIpMode = NEXUS_EcmIpMode_eV4;
        break;
    case 2:
        /*IPV4 LAN address*/
        pSystemInfo->ecmIpMode = NEXUS_EcmIpMode_eV6;
        break;
    default:
        pSystemInfo->ecmIpMode = NEXUS_EcmIpMode_eMax;
        break;
    }
    /* LAN mac address*/
    pSystemInfo->ecmMacAddress[0] = (systemInfo.ecmMacAddressHi>>24)&0xff;
    pSystemInfo->ecmMacAddress[1] = (systemInfo.ecmMacAddressHi>>16)&0xff;
    pSystemInfo->ecmMacAddress[2] = (systemInfo.ecmMacAddressHi>>8)&0xff;
    pSystemInfo->ecmMacAddress[3] = (systemInfo.ecmMacAddressHi)&0xff;
    pSystemInfo->ecmMacAddress[4] = (systemInfo.ecmMacAddressLo>>24)&0xff;
    pSystemInfo->ecmMacAddress[5] = (systemInfo.ecmMacAddressLo>>16)&0xff;
    pSystemInfo->ecmMacAddress[6] = (systemInfo.ecmMacAddressLo>>8)&0xff;
    pSystemInfo->ecmMacAddress[7] = (systemInfo.ecmMacAddressLo)&0xff;
    /* IP address*/
    if (systemInfo.ecmIpMode == NEXUS_EcmIpMode_eV4 || systemInfo.ecmIpMode == NEXUS_EcmIpMode_eV6)
    {
        b_convert_ipaddr(pSystemInfo->ecmIpAddress, systemInfo.ecmIpAddress);
        b_convert_ipaddr(&pSystemInfo->ecmIpv6Address[0],systemInfo.ecmIpv6Address0);
        b_convert_ipaddr(&pSystemInfo->ecmIpv6Address[4],systemInfo.ecmIpv6Address1);
        b_convert_ipaddr(&pSystemInfo->ecmIpv6Address[8],systemInfo.ecmIpv6Address2);
        b_convert_ipaddr(&pSystemInfo->ecmIpv6Address[12],systemInfo.ecmIpv6Address3);
    }
    switch(systemInfo.ecmWapInterface)
    {
    case 0:
        pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_eNone;
        break;
    case 1:
        pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_e2_4g;
        break;
    case 2:
        pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_e5g;
        break;
    case 3:
        pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_eDual;
        break;
    default:
        pSystemInfo->ecmWapInterface = NEXUS_DocsisWapInterfaceType_eMax;
        break;
    }
    pSystemInfo->ecmMtaInfoAvailable = systemInfo.ecmHasMtaInfo;
    pSystemInfo->ecmRouterInfoAvailable = systemInfo.ecmHasRouterInfo;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Docsis_GetWapStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisWapInterfaceType wapIfType,
    NEXUS_DocsisWapStatus *pWapStatus)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle) (handle->pDevice);
    BRPC_Param_ECM_GetWapStatus *pDcmWapStatus = NULL;
    BRPC_Param_ECM_WapInterfaceType Param;
    BERR_Code retVal;
    BADS_Version version;
    uint32_t i = 0;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT( deviceHandle->rpc_handle);

    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_SUCCESS);

    retVal = BADS_GetVersion(deviceHandle->ads, &version);
    if (retVal) return NEXUS_NOT_SUPPORTED;

    BKNI_Memset(pWapStatus, 0, sizeof(NEXUS_DocsisWapStatus));
    /* use malloc to avoid kernel stack overflow*/
    pDcmWapStatus = (BRPC_Param_ECM_GetWapStatus *)BKNI_Malloc(sizeof(BRPC_Param_ECM_GetWapStatus));
    if(!pDcmWapStatus)
    {
        BDBG_ERR(("Out of system memory"));
        return NEXUS_OUT_OF_SYSTEM_MEMORY;
    }
    BKNI_Memset(pDcmWapStatus, 0, sizeof(BRPC_Param_ECM_GetWapStatus));
    Param.type = wapIfType;
    BDBG_MSG((" NEXUS_Docsis_GetWapStatus"));

    retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ECM_GetWapStatus, (const uint32_t *)&Param,
                            sizeInLong(Param), (uint32_t *)pDcmWapStatus, sizeInLong(BRPC_Param_ECM_GetWapStatus), &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" NEXUS_Docsis_GetWapStatus Fail"));
        BKNI_Free(pDcmWapStatus);
        return NEXUS_NOT_SUPPORTED;
    }

    if (pDcmWapStatus->numberSSID > NEXUS_DOCSIS_MAX_SSID)
    {
        pDcmWapStatus->numberSSID = NEXUS_DOCSIS_MAX_SSID;
    }

    pWapStatus->numSsid = pDcmWapStatus->numberSSID;
    pWapStatus->wapMacAddress[0] = (pDcmWapStatus->wapMacAddressHi>>24)&0xff;
    pWapStatus->wapMacAddress[1] = (pDcmWapStatus->wapMacAddressHi>>16)&0xff;
    pWapStatus->wapMacAddress[2] = (pDcmWapStatus->wapMacAddressHi>>8)&0xff;
    pWapStatus->wapMacAddress[3] = (pDcmWapStatus->wapMacAddressHi)&0xff;
    pWapStatus->wapMacAddress[4] = (pDcmWapStatus->wapMacAddressLo>>24)&0xff;
    pWapStatus->wapMacAddress[5] = (pDcmWapStatus->wapMacAddressLo>>16)&0xff;
    pWapStatus->wapMacAddress[6] = (pDcmWapStatus->wapMacAddressLo>>8)&0xff;
    pWapStatus->wapMacAddress[7] = (pDcmWapStatus->wapMacAddressLo)&0xff;

    for (i=0; i < pDcmWapStatus->numberSSID; i++){
        NEXUS_DocsisWapSsidInfo *pSsidInfo = &pWapStatus->ssidInfo[i];
        BKNI_Memcpy(&pSsidInfo->ssid[0],&pDcmWapStatus->ssidInfo[i].SSID[0],64);
        pSsidInfo->ssid[63] = '\0';
        pSsidInfo->ssidEnabled = pDcmWapStatus->ssidInfo[i].ssidEnabled;
        pSsidInfo->ssidChannelNo = pDcmWapStatus->ssidInfo[i].ssidChannelNo;
        /* map wap protocol to nexus*/
        switch(pDcmWapStatus->ssidInfo[i].protocol)
        {
        case 0:
            pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_eUnused;
            break;
        case 1:
            pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211b;
            break;
        case 2:
            pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211g;
            break;
        case 3:
            pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211Bg;
            break;
        case 4:
            pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211Mixed;
            break;
        case 5:
            pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211n;
            break;
        case 6:
            pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_e80211Ac;
            break;
        default:
            pSsidInfo->protocol = NEXUS_DocsisWapWifiProtocolType_eMax;
            break;
        }
        /* map wap security scheme to nexus*/
        switch(pDcmWapStatus->ssidInfo[i].security)
        {
        case 0:
            pSsidInfo->security = NEXUS_DocsisWapWifiSecurityType_eWpa;
            break;
        case 1:
            pSsidInfo->security = NEXUS_DocsisWapWifiSecurityType_eMixed;
            break;
        case 2:
            pSsidInfo->security = NEXUS_DocsisWapWifiSecurityType_eWpa2Aes;
            break;
        default:
            pSsidInfo->security = NEXUS_DocsisWapWifiSecurityType_eMax;
            break;
        }
        /* wap mac address*/
        pSsidInfo->macAddress[0] = (pDcmWapStatus->ssidInfo[i].macAddressHi>>24)&0xff;
        pSsidInfo->macAddress[1] = (pDcmWapStatus->ssidInfo[i].macAddressHi>>16)&0xff;
        pSsidInfo->macAddress[2] = (pDcmWapStatus->ssidInfo[i].macAddressHi>>8)&0xff;
        pSsidInfo->macAddress[3] = (pDcmWapStatus->ssidInfo[i].macAddressHi)&0xff;
        pSsidInfo->macAddress[4] = (pDcmWapStatus->ssidInfo[i].macAddressLo>>24)&0xff;
        pSsidInfo->macAddress[5] = (pDcmWapStatus->ssidInfo[i].macAddressLo>>16)&0xff;
        pSsidInfo->macAddress[6] = (pDcmWapStatus->ssidInfo[i].macAddressLo>>8)&0xff;
        pSsidInfo->macAddress[7] = (pDcmWapStatus->ssidInfo[i].macAddressLo)&0xff;
    }
    BKNI_Free(pDcmWapStatus);

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Docsis_GetMtaStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisMtaStatus *pMtaStatus)

{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle) (handle->pDevice);
    BRPC_Param_ECM_GetMtaStatus dcmMtaStatus;
    BERR_Code retVal;
    BADS_Version version;
    int i = 0;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT( deviceHandle->rpc_handle);

    BKNI_Memset(pMtaStatus, 0, sizeof(NEXUS_DocsisMtaStatus));
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_SUCCESS);

    retVal = BADS_GetVersion(deviceHandle->ads, &version);
    if (retVal) return NEXUS_NOT_SUPPORTED;

    BKNI_Memset(&dcmMtaStatus, 0, sizeof(BRPC_Param_ECM_GetMtaStatus));

    BDBG_MSG((" NEXUS_Docsis_GetMtaStatus"));

    retCode = BRPC_CallProc(deviceHandle->rpc_handle,
                            BRPC_ProcId_ECM_GetMtaStatus,
                            NULL, 0,
                            (uint32_t *)&dcmMtaStatus,
                            sizeInLong(BRPC_Param_ECM_GetMtaStatus), &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" NEXUS_Docsis_GetMtaStatus Fail"));
        return NEXUS_NOT_SUPPORTED;
    }

    pMtaStatus->mtaEnabled = dcmMtaStatus.iActive;
    pMtaStatus->mtaNumLines = dcmMtaStatus.iNumLines;
    for (i=0; i< dcmMtaStatus.iNumLines; i++)
    {
        pMtaStatus->mtaLinesInfo[i].mtaLineStatus = dcmMtaStatus.pLines[i].enStatus;
        BKNI_Memcpy(pMtaStatus->mtaLinesInfo[i].mtaNumber, dcmMtaStatus.pLines[i].szNumber,32);
        BKNI_Memcpy(pMtaStatus->mtaLinesInfo[i].mtaCallerId, dcmMtaStatus.pLines[i].szCallId,64);
    }
    b_convert_ipaddr(pMtaStatus->mtaExtIpv4Address, dcmMtaStatus.mtaExtIPv4Address);
    b_convert_ipaddr(&pMtaStatus->mtaExtIpv6Address[0], dcmMtaStatus.mtaExtIPv6Address0);
    b_convert_ipaddr(&pMtaStatus->mtaExtIpv6Address[4], dcmMtaStatus.mtaExtIPv6Address1);
    b_convert_ipaddr(&pMtaStatus->mtaExtIpv6Address[8], dcmMtaStatus.mtaExtIPv6Address2);
    b_convert_ipaddr(&pMtaStatus->mtaExtIpv6Address[12], dcmMtaStatus.mtaExtIPv6Address3);

    b_convert_ipaddr(pMtaStatus->mtaSipGatewayIpv4Address, dcmMtaStatus.mtaSipGatewayIPv4Address);
    b_convert_ipaddr(&pMtaStatus->mtaSipGatewayIpv6Address[0],dcmMtaStatus.mtaSipGatewayIPv6Address0);
    b_convert_ipaddr(&pMtaStatus->mtaSipGatewayIpv6Address[4], dcmMtaStatus.mtaSipGatewayIPv6Address1);
    b_convert_ipaddr(&pMtaStatus->mtaSipGatewayIpv6Address[8], dcmMtaStatus.mtaSipGatewayIPv6Address2);
    b_convert_ipaddr(&pMtaStatus->mtaSipGatewayIpv6Address[12], dcmMtaStatus.mtaSipGatewayIPv6Address3);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Docsis_GetRouterStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_DocsisRouterStatus *pRouterStatus)
{
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle) (handle->pDevice);
    BRPC_Param_ECM_GetRouterStatus dcmRouterStatus;
    BERR_Code retVal;
    BADS_Version version;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT( deviceHandle->rpc_handle);

    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_SUCCESS);

    retVal = BADS_GetVersion(deviceHandle->ads, &version);
    if (retVal) return NEXUS_NOT_SUPPORTED;

    BKNI_Memset(pRouterStatus, 0, sizeof(NEXUS_DocsisRouterStatus));

    BKNI_Memset(&dcmRouterStatus, 0, sizeof(NEXUS_DocsisRouterStatus));

    BDBG_MSG((" NEXUS_Docsis_GetRouterStatus"));

    retCode = BRPC_CallProc(deviceHandle->rpc_handle,
                            BRPC_ProcId_ECM_GetRouterStatus,
                            NULL, 0,
                            (uint32_t *)&dcmRouterStatus,
                            sizeInLong(BRPC_Param_ECM_GetRouterStatus), &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" NEXUS_Docsis_GetRouterStatus Fail"));
        return NEXUS_NOT_SUPPORTED;
    }

    pRouterStatus->routerEnabled = dcmRouterStatus.iActive;

    b_convert_ipaddr(pRouterStatus->routerExtIpv4Address, dcmRouterStatus.routerExtIPv4Address);
    b_convert_ipaddr(&pRouterStatus->routerExtIpv6Address[0],dcmRouterStatus.routerExtIPv6Address0);
    b_convert_ipaddr(&pRouterStatus->routerExtIpv6Address[4],dcmRouterStatus.routerExtIPv6Address1);
    b_convert_ipaddr(&pRouterStatus->routerExtIpv6Address[8],dcmRouterStatus.routerExtIPv6Address2);
    b_convert_ipaddr(&pRouterStatus->routerExtIpv6Address[12],dcmRouterStatus.routerExtIPv6Address3);

    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Process vendor specific request to Docsis
See Also:

 ***************************************************************************/
NEXUS_Error NEXUS_Docsis_GetVsi(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_DocsisVsiRequest *pVsiRequest,
    NEXUS_DocsisVsi  *pVsi)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pVsiRequest);
    BSTD_UNUSED(pVsi);
#ifdef VENDOR_REQUEST
    BERR_Code retCode = BERR_SUCCESS;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle) (handle->pDevice);
    NEXUS_DocsisVsi dcmVsi;
    BERR_Code retVal;
    BADS_Version version;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT( deviceHandle->rpc_handle);

    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_SUCCESS);

    retVal = BADS_GetVersion(deviceHandle->ads, &version);
    if (retVal) return NEXUS_NOT_SUPPORTED;

    BKNI_Memset(pVsi, 0, sizeof(NEXUS_DocsisVsi));

    BKNI_Memset(&dcmVsi, 0, sizeof(NEXUS_DocsisVsi));

    BDBG_MSG((" NEXUS_Docsis_GetVsi"));

    retCode = BRPC_CallProc(deviceHandle->rpc_handle,
                            BRPC_ProcId_VEN_Request,
                            (const uint32_t *)pVsiRequest,
                            sizeInLong(NEXUS_DocsisVsiRequest),
                            (uint32_t *)&dcmVsi,
                            sizeInLong(NEXUS_DocsisVsi), &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" NEXUS_Docsis_GetVsi Fail"));
        return NEXUS_NOT_SUPPORTED;
    }

    BKNI_Memcpy(pVsi, &dcmVsi, sizeof(NEXUS_DocsisVsi));
    return NEXUS_SUCCESS;

#else
    BDBG_ERR(("NEXUS_Docsis_GetVsi Not Supported!"));
    return NEXUS_NOT_SUPPORTED;

#endif
}

NEXUS_Error NEXUS_Frontend_Get3255ChannelAgcConfig(
    NEXUS_FrontendHandle handle,
    NEXUS_3255ChannelAgcConfig *pStatus
    )
{
    NEXUS_3255ChannelHandle channelHandle = (NEXUS_3255ChannelHandle)handle->pDeviceHandle;
    NEXUS_3255DeviceHandle deviceHandle = channelHandle->deviceHandle;
    BERR_Code retCode = BERR_SUCCESS;
    BRPC_Param_XXX_Get Param;
    BRPC_Param_TNR_GetAgcVal outParam;
    BERR_Code retVal;
    BADS_Version version;

    BDBG_OBJECT_ASSERT(deviceHandle, NEXUS_3255Device);
    BDBG_ASSERT( deviceHandle->rpc_handle);
    if (deviceHandle->deviceStatus != NEXUS_3255DeviceStatus_eOperational) return (NEXUS_NOT_SUPPORTED);

    retVal = BADS_GetVersion(deviceHandle->ads, &version);
    if (retVal) return NEXUS_NOT_SUPPORTED;

    Param.devId = ((version.minVer <= 0x9) ? BRPC_DevId_3255_TNR0 : BRPC_DevId_ECM_TNR0) + channelHandle->chn_num;
    retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_TNR_GetAgcVal, (const uint32_t *)&Param,
                            sizeInLong(Param), (uint32_t *)&outParam, sizeInLong(outParam), &retVal);
    if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
    {
        BDBG_ERR((" NEXUS_Frontend_3255_GetTnrAgc Fail"));
        return NEXUS_NOT_SUPPORTED;
    }


    /********************************************************************************************************
    The format of 32 bit agcGain passed by 3383

    B3-B2: 16 bit chipid. Ex: 0x3412
    B1: b7b6=Output1Tilt, b5b4=Output2Tilt, b3=0, b2=SuperBoost, b1= Boost, b0=Tilt {0=OFF, 1=ON}
    B0:  Lna gain value from 0-0x1F (RDB indicates a 6 bit value but a valid gain value is only 5 bits)
    +---------------+----------------+-----------------+--------------+
    |            LnaChipId           |  T1/T2/0/S/B/T  |      AGC     |
    +---------------+----------------+-----------------+--------------+
            B3             B2                B1               B0


    Example:  Host receives LNA reading such of 0x3412561f
    Break it down:
    LnaChipId = 3412
    Output1 Stage2 Tilt: 1
    Output2 Stage2 Tilt: 1
    SuperBoost = ON
    Boost = ON
    Stage1 Tilt = OFF
    AGC = 0x1F  (max value)
    **********************************************************************************************************/
    pStatus->agcValue = (0x1f & outParam.AgcVal);
    pStatus->lnaChipId = (outParam.AgcVal >> 16);
    pStatus->output1TiltGain = ((outParam.AgcVal >> 14) & 0x3);
    pStatus->output2TiltGain = ((outParam.AgcVal >> 12) & 0x3);
    pStatus->superBoostEnabled = (outParam.AgcVal & 0x00000400)?true:false;
    pStatus->gainBoostEnabled = (outParam.AgcVal & 0x00000200)?true:false;
    pStatus->tiltEnabled = (outParam.AgcVal & 0x00000100)?true:false;
    BDBG_MSG((" %s  AGC is 0x%x LNA Chip ID 0x%x Tilt %d Boost %d SuperBoost %d Output1 Stage2 Tilt %d Output2 Stage2 Tilt %d",
              __FUNCTION__, pStatus->agcValue, pStatus->lnaChipId, pStatus->tiltEnabled, pStatus->gainBoostEnabled,
              pStatus->superBoostEnabled, pStatus->output1TiltGain, pStatus->output2TiltGain));
    return NEXUS_SUCCESS;

}


NEXUS_3255DeviceHandle NEXUS_Frontend_Get3255DeviceHandle(
    NEXUS_FrontendHandle handle
    )
{
    NEXUS_3255ChannelHandle channelHandle = handle->pDeviceHandle;
    NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)channelHandle->deviceHandle;

    return deviceHandle;
}

void NEXUS_Frontend_Get3255LockStatusForHostChannels(
    NEXUS_3255DeviceHandle handle,
    uint32_t *pLockStatus
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_3255Device);

    *pLockStatus = handle->NonCmControlledVideoChLockStatus;
}


NEXUS_Error NEXUS_Frontend_Set3255LockStatusForHostChannels(
    NEXUS_3255DeviceHandle handle,
    uint32_t lockStatus
    )
{
        NEXUS_Error ret = NEXUS_SUCCESS;
        BERR_Code retCode = BERR_SUCCESS;
        BERR_Code retVal;
        BRPC_Param_ECM_HostChannelsLockStatus Param;
        NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)handle;

        Param.NonCmControlledVideoChLockStatus = lockStatus;

        retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ECM_HostChannelsLockStatus,
                                                        (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal);

        if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
        {
                BDBG_ERR((" Unable to send the HostChannelsLockStatus through RPC to 3383 "));
                return NEXUS_INVALID_PARAMETER;
        }

    deviceHandle->NonCmControlledVideoChLockStatus = lockStatus;

    return ret;
}

NEXUS_Error NEXUS_Frontend_Config3255Lna(
    NEXUS_3255DeviceHandle handle
    )
{
        NEXUS_Error ret = NEXUS_SUCCESS;
        BERR_Code retCode = BERR_SUCCESS;
        BERR_Code retVal;
    BRPC_Param_ECM_DoLnaReConfig Param;
        NEXUS_3255DeviceHandle deviceHandle = (NEXUS_3255DeviceHandle)handle;

    Param.devId = BRPC_DevId_3255;

        retCode = BRPC_CallProc(deviceHandle->rpc_handle, BRPC_ProcId_ECM_DoLnaReConfig,
                                                        (const uint32_t *)&Param, sizeInLong(Param), NULL, 0, &retVal);

        if (retCode != BERR_SUCCESS || retVal != BERR_SUCCESS )
        {
                BDBG_ERR((" Unable to send the ECM_DoLnaReConfig through RPC to 3383 "));
                return NEXUS_INVALID_PARAMETER;
        }
    return ret;
}
