/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "nexus_frontend_module.h"
#include "priv/nexus_frontend_mtsif_priv.h"
#include "priv/nexus_core.h"
#if NEXUS_HAS_MXT
#include "bmxt.h"
#endif

#include "nexus_frontend_device_impl.h"

BDBG_MODULE(nexus_frontend);

#define BDBG_MSG_TRACE(x) /*BDBG_MSG(x)*/

#define MAX_PB_PER_MTSIF_FRONTEND 4

struct NEXUS_FrontendDeviceList g_frontendDeviceList;

struct NEXUS_FrontendList g_frontendList;

struct NEXUS_TunerList g_tunerList;

static NEXUS_FrontendModuleStatistics g_NEXUS_FrontendModuleStatistics;

/***************************************************************************
Summary:
    Return a list of capabilities for a given frontend object.
Description:
    This call returns a list of capabilities for a frontend object.  Because
    many frontends support multiple types of modulation schemes, this
    allows the application to distinguish the capabilities of one tuner
    versus another.  If a tuner has multiple capabilities, only one of
    the modes may be used at any one time.
See Also:
    NEXUS_Frontend_Get
 ***************************************************************************/
void NEXUS_Frontend_GetCapabilities(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendCapabilities *pCapabilities   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pCapabilities);

    *pCapabilities = handle->capabilities;
}

NEXUS_Error NEXUS_Frontend_GetInputBandStatus(NEXUS_FrontendHandle handle, NEXUS_InputBandStatus *pStatus)
{

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if (NEXUS_Frontend_P_CheckDeviceOpen(handle)) {
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

#if NEXUS_HAS_MXT
{
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendDeviceHandle device;
    BMXT_InputBandStatus ibStatus;
    NEXUS_Error rc;

    device = handle->pGenericDeviceHandle;
    NEXUS_Frontend_GetUserParameters(handle, &userParams);

    rc = BMXT_GetInputBandStatus(device->mtsifConfig.mxt, &ibStatus);
    if (rc) {
        return BERR_TRACE(rc);
    }
    pStatus->syncCount = ibStatus.syncCount[NEXUS_FRONTEND_USER_PARAM1_GET_INPUT_BAND(userParams.param1)];
}
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(pStatus);
    return NEXUS_NOT_SUPPORTED;
#endif
}

NEXUS_Error NEXUS_Frontend_ReadSoftDecisions( NEXUS_FrontendHandle handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length, size_t *pNumRead )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != handle->pDeviceHandle);
    BDBG_ASSERT(NULL != pDecisions);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->readSoftDecisions )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_ReadSoftDecisions(handle->pParentFrontend, pDecisions, length, pNumRead);
        }
        else if (handle->getSoftDecisions)
        {
            /* impl Read with Get */
            int rc;
            rc = NEXUS_Frontend_GetSoftDecisions(handle, pDecisions, length);
            if (rc) return BERR_TRACE(rc);
            *pNumRead = length;
            return 0;
        }
        else
        {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->readSoftDecisions(handle->pDeviceHandle, pDecisions, length, pNumRead);
    }
}

NEXUS_Error NEXUS_Frontend_GetSoftDecisions( NEXUS_FrontendHandle handle, NEXUS_FrontendSoftDecision *pDecisions, size_t length )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != handle->pDeviceHandle);
    BDBG_ASSERT(NULL != pDecisions);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getSoftDecisions )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetSoftDecisions(handle->pParentFrontend, pDecisions, length);
        }
        else if (handle->readSoftDecisions)
        {
            /* impl Get with Read */
            while (length) {
                size_t numRead;
                int rc;
                rc = NEXUS_Frontend_ReadSoftDecisions(handle, pDecisions, length, &numRead);
                if (rc) return BERR_TRACE(rc);
                length -= numRead;
            }
            return 0;
        }
        else
        {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getSoftDecisions(handle->pDeviceHandle, pDecisions, length);
    }
}

NEXUS_Error NEXUS_Frontend_GetUserParameters( NEXUS_FrontendHandle handle, NEXUS_FrontendUserParameters *pParams )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pParams);
    *pParams = handle->userParameters;
    if (handle->userParameters.isMtsif) {
        NEXUS_FRONTEND_USER_PARAM1_SET_INPUT_BAND(pParams->param1, handle->mtsif.inputBand);
        NEXUS_FRONTEND_USER_PARAM1_SET_MTSIF_TX(pParams->param1, handle->mtsif.txOut);
        NEXUS_FRONTEND_USER_PARAM1_SET_DAISYCHAIN_MTSIF_TX(pParams->param1, handle->mtsif.daisyTxOut);
        NEXUS_FRONTEND_USER_PARAM1_SET_DAISYCHAIN_OVERRIDE(pParams->param1, handle->mtsif.daisyOverride);
    }
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Frontend_SetUserParameters( NEXUS_FrontendHandle handle, const NEXUS_FrontendUserParameters *pParams )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pParams);
    handle->userParameters = *pParams;
    if (pParams->isMtsif) {
        handle->mtsif.inputBand = NEXUS_FRONTEND_USER_PARAM1_GET_INPUT_BAND(pParams->param1);
        handle->mtsif.txOut = NEXUS_FRONTEND_USER_PARAM1_GET_MTSIF_TX(pParams->param1);
        handle->mtsif.daisyTxOut = NEXUS_FRONTEND_USER_PARAM1_GET_DAISYCHAIN_MTSIF_TX(pParams->param1);
        handle->mtsif.daisyOverride = NEXUS_FRONTEND_USER_PARAM1_GET_DAISYCHAIN_OVERRIDE(pParams->param1);
    }
    if (handle->postSetUserParameters) {
        handle->postSetUserParameters(handle, pParams);
    }
    return BERR_SUCCESS;
}

#if NEXUS_HAS_MXT
struct NEXUS_P_MtsifPidChannel {
    BLST_S_ENTRY(NEXUS_P_MtsifPidChannel) link;
    unsigned pid;
    unsigned hostPb, demodPb;
    unsigned hostIndex, demodIndex;
    unsigned mtsifTxSel;
    enum {
        enabled,
        pendingEnable, /* pidchannel has been opened on host, but no demodPB -> hostPB connection has been established */
        disabled
    } state;
    BMXT_Handle mxt;
};
typedef struct NEXUS_P_MtsifPidChannel *NEXUS_P_MtsifPidChannelHandle;

static void NEXUS_Frontend_P_PidChannelCallback(void *arg);
static void NEXUS_Frontend_P_SetPid(NEXUS_P_MtsifPidChannelHandle pidChannel);
static void NEXUS_Frontend_P_SetPendingPids(void);
#if NEXUS_TRANSPORT_EXTENSION_TBG
static void NEXUS_Frontend_P_TbgConfig(void *arg);
#endif

#define MAX_MTSIF_PID_CHANNELS_PER_CALLBACK 32
typedef struct NEXUS_FrontendHostMtsifConfig {
    struct {
        unsigned demodPb; /* PB number used on demod */
        bool connected; /* if connection specified in NEXUS_ParserBandSettings has been established (via tune) */
        NEXUS_FrontendDeviceMtsifConfig *deviceConfig; /* device that is feeding the host PB */
    } hostPbSettings[NEXUS_NUM_PARSER_BANDS];

    BLST_S_HEAD(NEXUS_Frontend_P_MtsifPidChannels, NEXUS_P_MtsifPidChannel) mtsifPidChannels;
    bool demodPidChannelUsed[BMXT_MAX_NUM_PIDCHANNELS];
    struct NEXUS_MtsifPidChannelSettings *pidSettings;

    BKNI_EventHandle pidChannelEvent;
    NEXUS_EventCallbackHandle pidChannelEventCallback;
} NEXUS_FrontendHostMtsifConfig;
static struct NEXUS_FrontendHostMtsifConfig g_NEXUS_Frontend_P_HostMtsifConfig;
#endif

void NEXUS_Frontend_P_Init(void)
{
#if NEXUS_HAS_MXT
    BKNI_Memset(&g_NEXUS_Frontend_P_HostMtsifConfig, 0, sizeof(g_NEXUS_Frontend_P_HostMtsifConfig));
#endif
}

void NEXUS_Frontend_P_EnablePidFiltering(void) {
#if NEXUS_HAS_MXT
    /* single callback for all frontend devices */
    if (g_NEXUS_Frontend_P_HostMtsifConfig.pidChannelEventCallback==NULL) {
        NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
        NEXUS_TransportModule_GetPidchannelEvent(&g_NEXUS_Frontend_P_HostMtsifConfig.pidChannelEvent);
        NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
        if (g_NEXUS_Frontend_P_HostMtsifConfig.pidChannelEvent==NULL) {
            BDBG_ERR(("Unable to get transport pidchannel event"));
            return;
        }

        g_NEXUS_Frontend_P_HostMtsifConfig.pidChannelEventCallback = NEXUS_RegisterEvent(g_NEXUS_Frontend_P_HostMtsifConfig.pidChannelEvent, NEXUS_Frontend_P_PidChannelCallback, NULL);

        g_NEXUS_Frontend_P_HostMtsifConfig.pidSettings = BKNI_Malloc(sizeof(struct NEXUS_MtsifPidChannelSettings)*MAX_MTSIF_PID_CHANNELS_PER_CALLBACK);
        if (!g_NEXUS_Frontend_P_HostMtsifConfig.pidSettings) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            return;
        }
    }
#endif
}

void NEXUS_Frontend_P_CloseAllMtsifPidChannels(void)
{
#if NEXUS_HAS_MXT
    NEXUS_P_MtsifPidChannelHandle p;
    while (NULL!=(p=BLST_S_FIRST(&g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels))) {
        BLST_S_REMOVE_HEAD(&(g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels), link);
        g_NEXUS_Frontend_P_HostMtsifConfig.demodPidChannelUsed[p->demodIndex] = false;
        BKNI_Free(p);
    }
#endif
}

void NEXUS_Frontend_P_Uninit(void)
{
#if NEXUS_HAS_MXT
    if (BLST_S_FIRST(&g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels)) {
        NEXUS_Frontend_P_CloseAllMtsifPidChannels();
        BDBG_MSG(("Clean up stale demod pidchannels"));
    }

    if (g_NEXUS_Frontend_P_HostMtsifConfig.pidChannelEventCallback) {
        NEXUS_Module_Lock(g_NEXUS_frontendModule);
        NEXUS_UnregisterEvent(g_NEXUS_Frontend_P_HostMtsifConfig.pidChannelEventCallback);
        NEXUS_Module_Unlock(g_NEXUS_frontendModule);
    }

    if (g_NEXUS_Frontend_P_HostMtsifConfig.pidSettings) {
        BKNI_Free(g_NEXUS_Frontend_P_HostMtsifConfig.pidSettings);
    }
#endif
}

void NEXUS_Frontend_GetDefaultAcquireSettings( NEXUS_FrontendAcquireSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static void NEXUS_Frontend_UninstallCallbacks_priv(NEXUS_FrontendHandle handle)
{
    NEXUS_FrontendDeviceHandle deviceHandle = NULL;
    BDBG_ASSERT(NULL != handle);

    deviceHandle = NEXUS_Frontend_GetDevice(handle);
    if(deviceHandle && (deviceHandle->openFailed || deviceHandle->abortThread)) goto done;

    if(handle->uninstallCallbacks) {
        handle->uninstallCallbacks(handle->pDeviceHandle);
    }
    else if ( handle->pParentFrontend && handle->pParentFrontend->uninstallCallbacks ) {
        NEXUS_Frontend_UninstallCallbacks_priv(handle->pParentFrontend);
    }

done:
    return;
}

NEXUS_Error NEXUS_Frontend_GetBertStatus(NEXUS_FrontendHandle handle, NEXUS_FrontendBertStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getBertStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetBertStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getBertStatus(handle->pDeviceHandle, pStatus);
    }
}


NEXUS_FrontendHandle NEXUS_Frontend_P_Create(void *pDeviceHandle)
{
    NEXUS_FrontendHandle handle;

    BDBG_ASSERT(NULL != pDeviceHandle);
    handle = BKNI_Malloc(sizeof(NEXUS_Frontend));
    if (!handle) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_Frontend, handle);
    handle->pDeviceHandle = pDeviceHandle;
    /* must be append for implicit index to work */
    BLST_SQ_INSERT_TAIL(&g_frontendList.frontends, handle, link);
    NEXUS_OBJECT_REGISTER(NEXUS_Frontend, handle, Create);

    handle->connector = NEXUS_FrontendConnector_Create();
    if (!handle->connector) {
        goto error;
    }

    return handle;

error:
    NEXUS_Frontend_Close(handle);
    return NULL;
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_P_Create(void)
{
    NEXUS_FrontendDeviceHandle handle;

    handle = BKNI_Malloc(sizeof(NEXUS_FrontendDevice));
    if (!handle) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_FrontendDevice, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_FrontendDevice, handle, Create);
    handle->tripwire = 0x54329876;
    BDBG_MSG(("Creating device handle %p...", (void *)handle));

    return handle;
}

NEXUS_FrontendHandle NEXUS_Frontend_Acquire( const NEXUS_FrontendAcquireSettings *pSettings )
{
    NEXUS_FrontendHandle frontend;
    unsigned index;
    for (frontend = BLST_SQ_FIRST(&g_frontendList.frontends), index = 0; frontend; frontend = BLST_SQ_NEXT(frontend, link), index++) {
        BDBG_OBJECT_ASSERT(frontend, NEXUS_Frontend);
        if (frontend->acquired) continue;
        switch (pSettings->mode) {
        case NEXUS_FrontendAcquireMode_eByCapabilities:
            if ((!pSettings->capabilities.qam || frontend->capabilities.qam) &&
                (!pSettings->capabilities.vsb || frontend->capabilities.vsb) &&
                (!pSettings->capabilities.ofdm || frontend->capabilities.ofdm) &&
                (!pSettings->capabilities.outOfBand || frontend->capabilities.outOfBand) &&
                (!pSettings->capabilities.ifd || frontend->capabilities.ifd) &&
                (!pSettings->capabilities.satellite || frontend->capabilities.satellite))
            {
                frontend->acquired = true;
                NEXUS_OBJECT_REGISTER(NEXUS_FrontendConnector, frontend->connector, Acquire);
                return frontend;
            }
            break;
        case NEXUS_FrontendAcquireMode_eByIndex:
            if (pSettings->index == NEXUS_ANY_ID || pSettings->index == index) {
                frontend->acquired = true;
                NEXUS_OBJECT_REGISTER(NEXUS_FrontendConnector, frontend->connector, Acquire);
                return frontend;
            }
            break;
        default:
            BERR_TRACE(NEXUS_INVALID_PARAMETER); return NULL;
        }
    }
    return NULL;
}

void NEXUS_Frontend_Release( NEXUS_FrontendHandle frontend )
{
    NEXUS_FrontendDeviceHandle deviceHandle = NULL;
    BDBG_OBJECT_ASSERT(frontend, NEXUS_Frontend);

    NEXUS_Frontend_UninstallCallbacks_priv(frontend);


    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if(deviceHandle == NULL) goto done;

    if(!deviceHandle->openFailed && !deviceHandle->abortThread){
        NEXUS_Frontend_Untune(frontend);
    }
    if(frontend->acquired){
        frontend->acquired = false;
        NEXUS_OBJECT_UNREGISTER(NEXUS_FrontendConnector, frontend->connector, Release);
    }
done:
    return;
}

/***************************************************************************
Summary:
    Close a frontend handle
***************************************************************************/

static void NEXUS_Frontend_P_Release(NEXUS_FrontendHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    if(handle->acquired)
        NEXUS_Frontend_Release(handle);

    NEXUS_OBJECT_UNREGISTER(NEXUS_Frontend, handle, Destroy);
}

void NEXUS_Frontend_P_Destroy(NEXUS_FrontendHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BLST_SQ_REMOVE(&g_frontendList.frontends, handle, NEXUS_Frontend, link);
    if (handle->connector) {
        NEXUS_FrontendConnector_Destroy(handle->connector);
    }
    BDBG_OBJECT_DESTROY(handle, NEXUS_Frontend);
    BKNI_Free(handle);
    return;
}

static void NEXUS_Frontend_P_Finalizer(NEXUS_FrontendHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != handle->close);
    if ( handle->numExtensions > 0 )
    {
        BDBG_ERR(("Can not close a frontend with open extensions."));
        BDBG_ASSERT(handle->numExtensions == 0);
    }

#if (0==(NEXUS_HOST_CONTROL_EXTERNAL_FRONTEND_STANDBY+NEXUS_HOST_CONTROL_INTERNAL_FRONTEND_STANDBY))
    /* These standby use cases need to separate untune and close. Otherwise... */
    /* untune before closing. may result in power savings. */
    {
        NEXUS_FrontendDeviceHandle deviceHandle = NULL;
        deviceHandle = NEXUS_Frontend_GetDevice(handle);

        if(deviceHandle && !deviceHandle->openFailed && !deviceHandle->abortThread){
            NEXUS_Frontend_Untune(handle);
        }
    }
#endif

#ifdef NEXUS_FRONTEND_CUSTOMER_EXTENSION
    NEXUS_Frontend_P_UninitExtension(handle);
#endif
    if(handle->pGenericDeviceHandle && (handle->pGenericDeviceHandle->openPending || handle->pGenericDeviceHandle->openFailed))
    BDBG_WRN(("NEXUS_Frontend_P_Finalizer: Application aborted prematurely on the previous run. Waiting for device initialization thread to exit."));

    handle->close(handle);

    goto done; /* This bypasses a compile warning if the earlier standby block is not compiled. */

done:
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Frontend, NEXUS_Frontend_Close);

void NEXUS_Frontend_Untune( NEXUS_FrontendHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot untune."));
        return;
    }
    handle->tuned = false;

#if 0
    /* if app has disconnected mtsif, this will release mapping */
    NEXUS_Frontend_ReapplyTransportSettings(handle);
#endif

    if ( handle->untune )
    {
        handle->untune(handle->pDeviceHandle);
    }
    else if ( handle->pParentFrontend && handle->pParentFrontend->untune )
    {
        NEXUS_Frontend_Untune(handle->pParentFrontend);
    }
    else
    {
        /* This is a simple way to reset the app callback. In the future, we may want
                a chip-specific way of reseting state. */
        if ( handle->capabilities.qam )
        {
            NEXUS_FrontendQamSettings settings;
            NEXUS_Frontend_GetDefaultQamSettings(&settings);
            (void)NEXUS_Frontend_TuneQam(handle, &settings);
        }
        else if ( handle->capabilities.vsb )
        {
            NEXUS_FrontendVsbSettings settings;
            NEXUS_Frontend_GetDefaultVsbSettings(&settings);
            (void)NEXUS_Frontend_TuneVsb(handle, &settings);
        }
        else if ( handle->capabilities.satellite )
        {
            NEXUS_FrontendSatelliteSettings settings;
            NEXUS_Frontend_GetDefaultSatelliteSettings(&settings);
            settings.frequency=950000000UL;
            (void)NEXUS_Frontend_TuneSatellite(handle, &settings);
        }
        else if ( handle->capabilities.ofdm )
        {
            NEXUS_FrontendOfdmSettings settings;
            NEXUS_Frontend_GetDefaultOfdmSettings(&settings);
            (void)NEXUS_Frontend_TuneOfdm(handle, &settings);
        }
        else if ( handle->capabilities.analog )
        {
            NEXUS_FrontendAnalogSettings settings;
            NEXUS_Frontend_GetDefaultAnalogSettings(&settings);
            (void)NEXUS_Frontend_TuneAnalog(handle, &settings);
        }
        else if ( handle->capabilities.outOfBand )
        {
            NEXUS_FrontendOutOfBandSettings settings;
            NEXUS_Frontend_GetDefaultOutOfBandSettings(&settings);
            (void)NEXUS_Frontend_TuneOutOfBand(handle, &settings);
        }
        else if ( handle->capabilities.upstream )
        {
            /* no untune for upstream*/
        }
        else
        {
            BDBG_ERR(("Unable to untune. No tuner capabilities found."));
        }
    }
}

void NEXUS_Frontend_P_SetTuned(NEXUS_FrontendHandle handle)
{
    NEXUS_FrontendHandle f;
    unsigned cnt = 0;

    handle->tuned = true;

    for (f=BLST_SQ_FIRST(&g_frontendList.frontends);f;f=BLST_SQ_NEXT(f,link)) {
        if (f->tuned) cnt++;
    }
    if (cnt > g_NEXUS_FrontendModuleStatistics.maxTunedFrontends) {
        g_NEXUS_FrontendModuleStatistics.maxTunedFrontends = cnt;
    }
}

/***************************************************************************
Summary:
    Reset status values for a frontend
***************************************************************************/
void NEXUS_Frontend_ResetStatus(NEXUS_FrontendHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot reset status."));
        return;
    }

    NEXUS_Time_Get(&handle->resetStatusTime);
    if ( handle->resetStatus )
    {
        handle->resetStatus(handle->pDeviceHandle);
    }
    else if ( handle->pParentFrontend )
    {
        NEXUS_Frontend_ResetStatus(handle->pParentFrontend);
    }
}

/***************************************************************************
Summary:
    Get the Synchronous lock status of a frontend.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetFastStatus(NEXUS_FrontendHandle handle, NEXUS_FrontendFastStatus *pStatus)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getFastStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetFastStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getFastStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Retrieve the chip family id, chip id, chip version and firmware version.
 ***************************************************************************/
void NEXUS_Frontend_GetType(NEXUS_FrontendHandle handle,NEXUS_FrontendType *pType)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot get type."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( NULL == handle->getType )
    {
        if ( handle->pParentFrontend )
        {
            NEXUS_Frontend_GetType(handle->pParentFrontend, pType);
        }
        else
        {
            BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        handle->getType(handle->pDeviceHandle, pType);
    }
}

void NEXUS_Frontend_GetDefaultSpectrumSettings(NEXUS_FrontendSpectrumSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->dataReadyCallback);
}


NEXUS_Error NEXUS_Frontend_RequestSpectrumData(NEXUS_FrontendHandle handle, const NEXUS_FrontendSpectrumSettings *settings)
{
    BDBG_ASSERT(NULL != handle);

    if (!settings || !settings->data) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (!NEXUS_P_CpuAccessibleAddress(settings->data)) {
        /* this pointer must have driver-side mapping */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestSpectrumData )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_RequestSpectrumData(handle->pParentFrontend, settings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->requestSpectrumData(handle->pDeviceHandle, settings);
    }
}

void NEXUS_Frontend_P_GetDefaultExtensionSettings(NEXUS_FrontendExtensionSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/* Functions to override.  The close function is required, all other functions are optional.
    If you do not want to extend a function, leave it in the default state and the parent's handler will be called instead. */
NEXUS_FrontendHandle NEXUS_Frontend_P_CreateExtension(const NEXUS_FrontendExtensionSettings *pSettings )
{
    NEXUS_Frontend *pFrontend;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->close);
    BDBG_ASSERT(NULL != pSettings->parent);
    NEXUS_ASSERT_MODULE();

    pFrontend = NEXUS_Frontend_P_Create(pSettings->pDeviceHandle);
    if ( NULL == pFrontend )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    if (pSettings->parent->registerExtension) {
        (*pSettings->parent->registerExtension)(pSettings->parent, pFrontend);
    }

    pFrontend->capabilities = pSettings->parent->capabilities;
    pFrontend->userParameters = pSettings->parent->userParameters;
    pFrontend->pParentFrontend = pSettings->parent;
    pFrontend->close = pSettings->close;
    pFrontend->tuneAnalog = pSettings->tuneAnalog;
    pFrontend->tuneOutOfBand = pSettings->tuneOutOfBand;
    pFrontend->tuneQam = pSettings->tuneQam;
    pFrontend->tuneUpstream = pSettings->tuneUpstream;
    pFrontend->tuneVsb = pSettings->tuneVsb;
    pFrontend->tuneSatellite = pSettings->tuneSatellite;
    pFrontend->getDiseqcSettings = pSettings->getDiseqcSettings;
    pFrontend->setDiseqcSettings = pSettings->setDiseqcSettings;
    pFrontend->sendDiseqcMessage = pSettings->sendDiseqcMessage;
    pFrontend->getDiseqcReply = pSettings->getDiseqcReply;
    pFrontend->sendDiseqcAcw = pSettings->sendDiseqcAcw;
    pFrontend->resetDiseqc = pSettings->resetDiseqc;
    pFrontend->tuneOfdm = pSettings->tuneOfdm;
    pFrontend->untune = pSettings->untune;

    pSettings->parent->numExtensions++;

    return pFrontend;
}

void NEXUS_Frontend_P_DestroyExtension(NEXUS_FrontendHandle handle)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != handle->pParentFrontend);
    BDBG_ASSERT(handle->pParentFrontend->numExtensions > 0);
    handle->pParentFrontend->numExtensions--;

    if (handle->pParentFrontend->registerExtension) {
        (*handle->pParentFrontend->registerExtension)(handle->pParentFrontend, NULL);
    }

    NEXUS_Frontend_P_Destroy(handle);
}

#if NEXUS_HAS_MXT
NEXUS_Error NEXUS_Frontend_P_InitMtsifConfig(NEXUS_FrontendDeviceMtsifConfig *pConfig, const BMXT_Settings *mxtSettings)
{
    BMXT_ParserConfig parserConfig;
    unsigned i;
    NEXUS_Error rc;

    if (pConfig->mxt==NULL) {
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }
    /* TODO: we could move BMXT_Open() here as well, but some platforms (e.g. 7366) would need to pass in an extra BREG handle to this function */

    if (pConfig->numDemodIb || pConfig->numDemodPb) {
        BDBG_WRN(("Unexpected non-zero members in MTSIF config")); /* this is a helpful warning to indicate that platform frontend code is not memset'ing the NEXUS_FrontendDeviceMtsifConfig's parent struct to 0 */
    }

    pConfig->pidfilter = mxtSettings->enablePidFiltering;
    pConfig->numDemodIb = BMXT_GetNumResources(mxtSettings->chip, mxtSettings->chipRev, BMXT_ResourceType_eInputBand);
    pConfig->numDemodPb = BMXT_GetNumResources(mxtSettings->chip, mxtSettings->chipRev, BMXT_ResourceType_eParser);

    for (i=0; i<pConfig->numDemodPb; i++) {
        rc = BMXT_GetParserConfig(pConfig->mxt, i, &parserConfig);
        if (rc) { return BERR_TRACE(rc); }

        /* turn off demod parsers */
        parserConfig.Enable = false;

        /* cache other settings */
        pConfig->demodPbSettings[i].errorInputIgnore = parserConfig.ErrorInputIgnore;
        pConfig->demodPbSettings[i].timestampMode = (unsigned)parserConfig.TsMode;
        pConfig->demodPbSettings[i].dssMode = parserConfig.DssMode;
        pConfig->demodPbSettings[i].mtsifTxSel = parserConfig.mtsifTxSelect;
        pConfig->demodPbSettings[i].allPass = parserConfig.AllPass;
        pConfig->demodPbSettings[i].acceptNull = parserConfig.AcceptNulls;

        rc = BMXT_SetParserConfig(pConfig->mxt, i, &parserConfig);
        if (rc) { return BERR_TRACE(rc); }
    }

    return NEXUS_SUCCESS;
}

#if NEXUS_TRANSPORT_EXTENSION_TSMF
static NEXUS_Error NEXUS_Frontend_P_SetTsmfInput(void *mxt, unsigned tsmfInput, unsigned tsmfIndex, const NEXUS_TsmfSettings *tsmfSettings)
{
    NEXUS_Error rc;
    BDBG_CASSERT(sizeof(NEXUS_TsmfFieldVerifyConfig) == sizeof(BMXT_TSMFFldVerifyConfig));
    BDBG_CASSERT(NEXUS_TsmfVersionChangeMode_eFrameChangeVer == (NEXUS_TsmfVersionChangeMode)BMXT_TSMFVersionChgMode_eFrameChgVer);

    if(tsmfSettings->sourceType==NEXUS_TsmfSourceType_eMtsifRx)
       return NEXUS_SUCCESS;

    rc = BMXT_TSMF_SetFldVerifyConfig(mxt, tsmfIndex, (const BMXT_TSMFFldVerifyConfig *)&(tsmfSettings->fieldVerifyConfig));
    if (rc) {rc = BERR_TRACE(rc);} /* keep going */

    if (tsmfSettings->enabled) {
        BMXT_TSMFInputSel inputSelect = (BMXT_TSMFInputSel) tsmfInput;
        BDBG_MSG(("TSMF: IB%u -> TSMF%u (rel %u)", tsmfInput, tsmfIndex, tsmfSettings->relativeTsNum));

        if (tsmfSettings->semiAutomaticMode) {
            rc = BMXT_TSMF_EnableSemiAutoMode(mxt, inputSelect, tsmfIndex, tsmfSettings->slotMapLo, tsmfSettings->slotMapHi, tsmfSettings->relativeTsNum);
            if (rc) { return BERR_TRACE(rc); }
        }
        else {
            rc = BMXT_TSMF_EnableAutoMode(mxt, inputSelect, tsmfIndex, tsmfSettings->relativeTsNum);
            if (rc) { return BERR_TRACE(rc); }
        }
    }
    else {
        BDBG_MSG(("TSMF: TSMF%u disabled", tsmfIndex));
        rc = BMXT_TSMF_DisableTsmf(mxt, tsmfIndex);
        if (rc) { return BERR_TRACE(rc); }
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Frontend_P_SetTsmfOutput(void *mxt, unsigned tsmfOutput, unsigned tsmfIndex, bool enabled)
{
    NEXUS_Error rc;
    BDBG_MSG(("TSMF: TSMF%u -> PB%u (enable %u)", tsmfIndex, tsmfOutput, enabled));
    rc = BMXT_TSMF_SetParserConfig(mxt, tsmfOutput, tsmfIndex, enabled);
    if (rc) { return BERR_TRACE(rc); }
    return NEXUS_SUCCESS;
}
#endif

static void NEXUS_Frontend_P_MxtTimer(void* context)
{
    NEXUS_FrontendDeviceMtsifConfig *pConfig = context;
    BMXT_Handle mxt = pConfig->mxt;
    unsigned i;
    uint32_t status = BMXT_ReadIntStatusRegister(mxt, BMXT_IntReg_eFE_INTR_STATUS0);

    BDBG_MSG_TRACE(("NEXUS_Frontend_P_MxtTimer: status %#x", status));
    if (status) {
        unsigned lengthError = 0, transportError = 0; /* remap frontend PB to backend PB */
        /* bits 0~15 are length error for parser 0 through 15. bits 16~31 are transport error for parser 0 through 15 */
        for (i=0; i<16; i++) {
            if (i<pConfig->numDemodPb) {
                if ((status>>i)&0x1) {
                    lengthError |= 1 << pConfig->demodPbSettings[i].virtualNum;
                }
                if ((status>>(i+16))&0x1) {
                    transportError |= 1 << pConfig->demodPbSettings[i].virtualNum;
                }
            }
        }
        NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
        NEXUS_ParserBand_P_MtsifErrorStatus_priv(lengthError, transportError);
        NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
    }

    pConfig->timer = NEXUS_ScheduleTimer(1000, NEXUS_Frontend_P_MxtTimer, pConfig);
}

#if NEXUS_DUMP_MTSIF
static void NEXUS_Frontend_P_DumpMtsifConfig(const NEXUS_FrontendDeviceMtsifConfig *pConfig);
#endif

/* configure the demod transport core to route a given frontend to one or more parser bands on host */
static NEXUS_Error NEXUS_Frontend_P_ConfigMtsifConfig(NEXUS_FrontendHandle frontend, bool enabled)
{
    unsigned i;
    unsigned demodIb, demodPb, demodMtsifTx, demodDaisyMtsifTx, daisyOverride;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendDeviceMtsifConfig *pConfig = &frontend->pGenericDeviceHandle->mtsifConfig;
    NEXUS_FrontendDeviceMtsifConfig *pChainedConfig = frontend->pGenericDeviceHandle->chainedConfig; /* points to a chained MTSIF config, if it exists */
    BMXT_Handle mxt = pConfig->mxt;
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct NEXUS_MtsifParserBandSettings *mtsifConnections = NULL;
    unsigned num;
    bool setTimer = false;
    mtsifConnections = BKNI_Malloc(sizeof(*mtsifConnections)*MAX_PB_PER_MTSIF_FRONTEND);
    if (!mtsifConnections) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto done; }

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    demodIb = frontend->mtsif.inputBand;
    demodMtsifTx = frontend->mtsif.txOut;
    demodDaisyMtsifTx = frontend->mtsif.daisyTxOut;
    daisyOverride = frontend->mtsif.daisyOverride;

    if (!userParams.isMtsif) {
        rc = NEXUS_SUCCESS; goto done;
    }
    if (mxt==NULL) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done;
    }
    if (pConfig->numDemodIb==0 || pConfig->numDemodIb>BMXT_MAX_NUM_INPUT_BANDS || pConfig->numDemodPb==0 || pConfig->numDemodPb>BMXT_MAX_NUM_PARSERS) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done;
    }

    /* one device but two mtsif configs. this is a special case that requires setting the pointers to the correct config for this frontend handle */
    if (userParams.pParam2) { /* TODO: use a nexus_frontend_mtsif_priv.h macro to handle this case */
        if (frontend->pGenericDeviceHandle->chainedConfig==NULL) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done;
        }
        if (frontend->pGenericDeviceHandle->chainedConfig->mxt==NULL) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto done;
        }

        /* swap configs */
        pChainedConfig = pConfig;
        pConfig = frontend->pGenericDeviceHandle->chainedConfig;
        mxt = pConfig->mxt;

        /* HAB/RPC-chained, but not MTSIF-chained */
        if (!pConfig->slave && !pChainedConfig->slave) {
            pChainedConfig = NULL;
        }
    }

    /* 1. find all host PBs whose MTSIF source is equal to this frontend handle. we must map those PBs to this frontend */
    if (enabled) {
        NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
        NEXUS_ParserBand_GetMtsifConnections_priv(frontend->connector, mtsifConnections, MAX_PB_PER_MTSIF_FRONTEND, &num);
        NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);
    }
    else {
        num = 0;
    }
    if (pChainedConfig && (num > 1)) {
        BDBG_WRN(("Daisy-chained MTSIF configurations only support 1-to-1 mappings. Only the first host PB will receive demod data"));
        num = 1;
    }

    for (i=0; i<num; i++) {
        unsigned hostPb = mtsifConnections[i].index;
        const NEXUS_ParserBandSettings *pSettings = &mtsifConnections[i].settings;

        /* 2. check all current connections for existing connections or conflicts */
        for (demodPb=0; demodPb<pConfig->numDemodPb; demodPb++) {
            if (pConfig->demodPbSettings[demodPb].enabled) {
                if (pConfig->demodPbSettings[demodPb].virtualNum==hostPb) {
                    if (pConfig->demodPbSettings[demodPb].inputBandNum==demodIb) {
                        /* specified FE-> PB connection already exists due to tuning again without untune */
                        BDBG_MSG(("MTSIF exist:      IB%u -> PB%u -> PB%u (TX%u, %p:%p)", demodIb, demodPb, hostPb, pConfig->demodPbSettings[demodPb].mtsifTxSel, (void *)frontend, (void *)mxt));
                        break;
                    }
                    else {
                        /* mark the existing connection as NULL so we override it below
                           typical example: FE0 -> PB0 then FE1 -> PB0. FE0 is still tuned, but data not routed anywhere */
                        pConfig->demodPbSettings[demodPb].enabled = false;
                        pConfig->demodPbSettings[demodPb].connector = NULL;
                        BDBG_MSG(("MTSIF override:   IB%u -> PB%u -> PB%u (TX%u, %p:%p)", pConfig->demodPbSettings[demodPb].inputBandNum, demodPb, hostPb, demodMtsifTx, (void *)frontend, (void *)mxt));
                        goto set_mapping; /* must use same demodPB */
                    }
                }
            }
        }

        if (demodPb < pConfig->numDemodPb) { /* connection already exists. skip over picking a free demod PB */
            if (pConfig->demodPbSettings[demodPb].mtsifTxSel != demodMtsifTx) { /* allow MTSIF_TX change on-the-fly */
                goto set_mapping;
            }
            goto apply_settings;
        }

        /* 3. pick a free demod PB */
        if (pChainedConfig) {
            demodPb = hostPb; /* 1-to-1 mapping only */
        }
        else {
            for (demodPb=0; demodPb<pConfig->numDemodPb; demodPb++) {
                if (pConfig->demodPbSettings[demodPb].enabled==false) {
                    break;
                }
            }
        }

set_mapping:
        if (demodPb < pConfig->numDemodPb) {
            BMXT_ParserConfig parserConfig;

            /* 4a. map demod IB->demod PB */
            BMXT_GetParserConfig(mxt, demodPb, &parserConfig);
            pConfig->demodPbSettings[demodPb].inputBandNum = parserConfig.InputBandNumber = demodIb;
            pConfig->demodPbSettings[demodPb].enabled = parserConfig.Enable = true;
            pConfig->demodPbSettings[demodPb].connector = frontend->connector;
            pConfig->demodPbSettings[demodPb].mtsifTxSel = parserConfig.mtsifTxSelect = demodMtsifTx;
            BMXT_SetParserConfig(mxt, demodPb, &parserConfig);

            /* if slave, then we need to change the passthrough routing on master */
            if (pConfig->slave && !daisyOverride) {
                BMXT_Handle mxt_master;
                BDBG_ASSERT(pChainedConfig); /* if slave, then by definition, pChainedConfig is non-NULL */
                mxt_master = pChainedConfig->mxt;
                BMXT_GetParserConfig(mxt_master, hostPb, &parserConfig);
                #if 0 /* no need to do this as we check for conflicts later and force a disable */
                BDBG_ASSERT(parserConfig.Enable == false);
                #endif
                pChainedConfig->demodPbSettings[hostPb].mtsifTxSel = parserConfig.mtsifTxSelect = demodDaisyMtsifTx;
                BMXT_SetParserConfig(mxt_master, hostPb, &parserConfig);
                BDBG_MSG(("MTSIF passthrough: setting PB%u and TX%u on master", hostPb, demodDaisyMtsifTx));
            }

            /* 4b. set virtual PB num of demod PB equal to host PB num */
            BMXT_GetParserConfig(mxt, demodPb, &parserConfig);
            pConfig->demodPbSettings[demodPb].virtualNum = parserConfig.virtualParserNum = hostPb;
            BMXT_SetParserConfig(mxt, demodPb, &parserConfig);
            BDBG_MSG(("MTSIF connect:    IB%u -> PB%u -> PB%u (TX%u, %p:%p) %s", demodIb, demodPb, hostPb, parserConfig.mtsifTxSelect,
                (void *)frontend, (void *)mxt, pChainedConfig?(pConfig->slave?"(chain slave) ":"(chain master)"):""));

            if ( (g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].connected) && /* if this hostPb is already being fed */
                (g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].deviceConfig != pConfig) )
            {
                NEXUS_FrontendDeviceMtsifConfig *otherConfig = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].deviceConfig;
                BMXT_Handle otherMxt;
                unsigned otherDemodPb;
                BDBG_ASSERT(otherConfig);

                otherMxt = otherConfig->mxt;
                otherDemodPb = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].demodPb;

                BMXT_GetParserConfig(otherMxt, otherDemodPb, &parserConfig);
                otherConfig->demodPbSettings[otherDemodPb].enabled = parserConfig.Enable = false;
                BMXT_SetParserConfig(otherMxt, otherDemodPb, &parserConfig);

                BDBG_WRN(("MTSIF conflict: host PB%u is already receiving data from another MTSIF device (%p). Forcing disable of demod PB%u (IB%u -> PB%u -> PB%u)",
                    hostPb, (void*)otherMxt, otherDemodPb, parserConfig.InputBandNumber, otherDemodPb, parserConfig.virtualParserNum));
            }

            g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].demodPb = demodPb;
            g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].connected = true;
            g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].deviceConfig = pConfig;
        }

apply_settings:
        /* did not find free demod PB */
        if (demodPb>=pConfig->numDemodPb) {
            BDBG_ERR(("No demod PB available to connect: %p:%p IB%u -> PB? -> PB%u", (void *)frontend, (void *)mxt, demodIb, hostPb));
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto done;
        }

        /* grab the settings of the output PB on the host, and apply them to the demod PB */
        if ( (pConfig->demodPbSettings[demodPb].errorInputIgnore != pSettings->teiIgnoreEnabled) ||
             (pConfig->demodPbSettings[demodPb].timestampMode != (NEXUS_IS_DSS_MODE(pSettings->transportType) ? BMXT_ParserTimestampMode_eBinary : BMXT_ParserTimestampMode_eMod300)) ||
             (pConfig->demodPbSettings[demodPb].dssMode != NEXUS_IS_DSS_MODE(pSettings->transportType)) )
        {
            BMXT_ParserConfig mxtParserConfig;
            BDBG_MSG(("MTSIF PB settings: demod PB%u <- host PB%u (DSS %u)", demodPb, hostPb, NEXUS_IS_DSS_MODE(pSettings->transportType)));
            BMXT_GetParserConfig(mxt, demodPb, &mxtParserConfig);
            /* translate NEXUS_ParserBandSettings to BMXT_ParserConfig.
               note the ParserBandSettings have already been applied to host; we'll just apply them to the demod as well.
               some may be needed, some not. */
            mxtParserConfig.ErrorInputIgnore = pSettings->teiIgnoreEnabled;
            mxtParserConfig.TsMode = NEXUS_IS_DSS_MODE(pSettings->transportType) ? BMXT_ParserTimestampMode_eBinary : BMXT_ParserTimestampMode_eMod300;
            mxtParserConfig.DssMode = NEXUS_IS_DSS_MODE(pSettings->transportType);
            /* ignore allPass and acceptNullPackets for demod. demod is always in allpass + acceptnull mode. */
            BMXT_SetParserConfig(mxt, demodPb, &mxtParserConfig);

            if (pConfig->demodPbSettings[demodPb].dssMode != NEXUS_IS_DSS_MODE(pSettings->transportType)) {
                BMXT_InputBandConfig mxtInputBandConfig;
                BDBG_MSG(("MTSIF IB settings: demod IB%u <- host PB%u (DSS %u)", demodIb, hostPb, NEXUS_IS_DSS_MODE(pSettings->transportType)));
                BMXT_GetInputBandConfig(mxt, demodIb, &mxtInputBandConfig);
                mxtInputBandConfig.DssMode = mxtParserConfig.DssMode;
                BMXT_SetInputBandConfig(mxt, demodIb, &mxtInputBandConfig);
            }

            pConfig->demodPbSettings[demodPb].errorInputIgnore = mxtParserConfig.ErrorInputIgnore;
            pConfig->demodPbSettings[demodPb].timestampMode = mxtParserConfig.TsMode;
            pConfig->demodPbSettings[demodPb].dssMode = mxtParserConfig.DssMode;
        }

        if ( pConfig->pidfilter &&
            ((pConfig->demodPbSettings[demodPb].allPass != pSettings->allPass) ||
            (pConfig->demodPbSettings[demodPb].acceptNull != pSettings->acceptNullPackets)) )
        {
            BMXT_ParserConfig mxtParserConfig;
            BDBG_MSG(("MTSIF PB settings: demod PB%u <- host PB%u (allPass %u, acceptNull %u)", demodPb, hostPb, pSettings->allPass, pSettings->acceptNullPackets));
            BMXT_GetParserConfig(mxt, demodPb, &mxtParserConfig);
            pConfig->demodPbSettings[demodPb].allPass = mxtParserConfig.AllPass = pSettings->allPass;
            pConfig->demodPbSettings[demodPb].acceptNull = mxtParserConfig.AcceptNulls = pSettings->acceptNullPackets;
            BMXT_SetParserConfig(mxt, demodPb, &mxtParserConfig);
        }

        if (pSettings->teiError.callback || pSettings->lengthError.callback) {
            setTimer = true;
        }

#if NEXUS_TRANSPORT_EXTENSION_TSMF
        if (!mtsifConnections[i].tsmf.valid) continue;
        /* set input: connect this frontend to all (could be more than one) TSMF whose input matches this frontend */
        NEXUS_Frontend_P_SetTsmfInput(mxt, demodIb, mtsifConnections[i].tsmf.hwIndex, &mtsifConnections[i].tsmf.settings);

        /* set output: connect this TSMF to all demod PB that are connected to host PBs whose input is this TSMF (yes, it's complicated...) */
        demodPb = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[mtsifConnections[i].index].demodPb; /* demod TSMF output is the demod PB, not the host PB */
        NEXUS_Frontend_P_SetTsmfOutput(mxt, demodPb, mtsifConnections[i].tsmf.hwIndex, mtsifConnections[i].tsmf.settings.enabled);
#endif

    }

    if (enabled && pConfig->pidfilter) {
        NEXUS_Frontend_P_SetPendingPids();
    }

    /* find any previously-established connections that must now be disconnected. disconnects are only implicitly requested, so we must search */
    for (demodPb=0; demodPb<pConfig->numDemodPb; demodPb++) {

        if (!pConfig->demodPbSettings[demodPb].enabled) { continue; }
        if (pConfig->demodPbSettings[demodPb].connector != frontend->connector) {
            continue; /* if PB is being fed by a different frontend, then the new connection has already been established, so no need to disconnect the old one */
        }

        for (i=0; i<num; i++) {
            if (mtsifConnections[i].index == pConfig->demodPbSettings[demodPb].virtualNum) { break; } /* connection is still valid */
        }
        if (i == num) {
            /* connection no longer valid, so disconnect */
            BMXT_ParserConfig parserConfig;
            unsigned hostPb = pConfig->demodPbSettings[demodPb].virtualNum;
            BMXT_GetParserConfig(mxt, demodPb, &parserConfig);
            pConfig->demodPbSettings[demodPb].enabled = parserConfig.Enable = false;
            pConfig->demodPbSettings[demodPb].connector = NULL;
            BMXT_SetParserConfig(mxt, demodPb, &parserConfig);
            BDBG_MSG(("MTSIF disconnect: IB%u -> PB%u -> PB%u (%p:%p)", demodIb, demodPb, hostPb, (void *)frontend, (void *)mxt));
            g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].demodPb = 0;
            g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].connected = false;
            g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].deviceConfig = 0;
#if NEXUS_HAS_TSMF
            /* disable/reset the parser's TSMF config. otherwise, a new frontend -> host_PB connection may inadvertently get TSMF-filtered */
            BMXT_TSMF_SetParserConfig(mxt, demodPb, 0 /* don't care */, false);
#endif
        }
    }

    if (setTimer) {
        if (!pConfig->timer) {
            pConfig->timer = NEXUS_ScheduleTimer(1000, NEXUS_Frontend_P_MxtTimer, pConfig);
        }
    }
    else {
        if (pConfig->timer) {
            NEXUS_CancelTimer(pConfig->timer);
            pConfig->timer = NULL;
        }
    }

#if NEXUS_DUMP_MTSIF
    NEXUS_Frontend_P_DumpMtsifConfig(pConfig);
    if (pChainedConfig) {
        NEXUS_Frontend_P_DumpMtsifConfig(pChainedConfig);
    }
#endif

done:
    if (mtsifConnections) { BKNI_Free(mtsifConnections); }
    return rc;
}

NEXUS_Error NEXUS_Frontend_P_SetMtsifConfig(NEXUS_FrontendHandle frontend)
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(frontend, NEXUS_Frontend);
    rc = NEXUS_Frontend_P_ConfigMtsifConfig(frontend, true);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

void NEXUS_Frontend_P_UnsetMtsifConfig(NEXUS_FrontendHandle frontend)
{

    BDBG_OBJECT_ASSERT(frontend, NEXUS_Frontend);
    (void)NEXUS_Frontend_P_ConfigMtsifConfig(frontend, false);
}
#else
NEXUS_Error NEXUS_Frontend_P_InitMtsifConfig(NEXUS_FrontendDeviceMtsifConfig *pConfig, const void *mxtSettings)
{
    BSTD_UNUSED(pConfig);
    BSTD_UNUSED(mxtSettings);
    return 0;
}
NEXUS_Error NEXUS_Frontend_P_SetMtsifConfig(NEXUS_FrontendHandle frontend)
{
    BDBG_OBJECT_ASSERT(frontend, NEXUS_Frontend);
    return 0;
}

void NEXUS_Frontend_P_UnsetMtsifConfig(NEXUS_FrontendHandle frontend)
{
    BDBG_OBJECT_ASSERT(frontend, NEXUS_Frontend);
}
#endif /* NEXUS_HAS_MXT */

#if NEXUS_DUMP_MTSIF
void NEXUS_Frontend_P_PostDumpMtsifConfig(void *context)
{
#if NEXUS_HAS_MXT
    unsigned i;
    NEXUS_FrontendDeviceMtsifConfig *pConfig = context;
    BMXT_Handle mxt = pConfig->mxt;
    BMXT_InputBandStatus ibStatus;
    BDBG_MSG(("PostDumpMtsifConfig %p (%s)", (void *)mxt, pConfig->slave?"slave":"master"));

    /* IB sync count */
    for (i=0; i<2; i++) {
        BMXT_GetInputBandStatus(mxt, &ibStatus);
        BDBG_MSG(("demod IB: %02x %02x %02x %02x   %02x %02x %02x %02x   %02x %02x %02x %02x   %02x %02x %02x %02x",
            ibStatus.syncCount[0], ibStatus.syncCount[1], ibStatus.syncCount[2], ibStatus.syncCount[3],
            ibStatus.syncCount[4], ibStatus.syncCount[5], ibStatus.syncCount[6], ibStatus.syncCount[7],
            ibStatus.syncCount[8], ibStatus.syncCount[9], ibStatus.syncCount[10], ibStatus.syncCount[11],
            ibStatus.syncCount[12], ibStatus.syncCount[13], ibStatus.syncCount[14], ibStatus.syncCount[15]));
    }
#else
    BSTD_UNUSED(context);
#endif
}

/* debug function to dump all known info */
void NEXUS_Frontend_P_DumpMtsifConfig(const NEXUS_FrontendDeviceMtsifConfig *pConfig)
{
#if NEXUS_HAS_MXT && NEXUS_NUM_PARSER_BANDS
    BMXT_Handle mxt = pConfig->mxt;
    BMXT_ParserConfig parserConfig;
    unsigned i;
    const unsigned numDemodPb = pConfig->numDemodPb>16 ? 16:pConfig->numDemodPb;

    BDBG_ASSERT(pConfig);
    BDBG_ASSERT(mxt);

    BDBG_MSG(("DumpMtsifConfig %p (%s)", (void *)mxt, pConfig->slave?"slave":"master"));
    /* print out real and cached demod PB settings */
    for (i=0; i<numDemodPb; i++) {
        BMXT_GetParserConfig(mxt, i, &parserConfig);
        BDBG_MSG(("demod PB[%2u]: en %u:%u, IB %2u:%2u, virt %2u:%2u, TX %u", i,
            parserConfig.Enable, pConfig->demodPbSettings[i].enabled,
            parserConfig.InputBandNumber, pConfig->demodPbSettings[i].inputBandNum,
            parserConfig.virtualParserNum, pConfig->demodPbSettings[i].virtualNum,
            parserConfig.mtsifTxSelect));
    }

#if 0
    /* print out host PB settings */
    {
        struct NEXUS_MtsifParserBandSettings mtsifConnections[MAX_PB_PER_MTSIF_FRONTEND];
        unsigned i, num;

        NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
        NEXUS_ParserBand_GetMtsifConnections_priv(frontend->connector, mtsifConnections, MAX_PB_PER_MTSIF_FRONTEND, &num);
        NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);

        for (i=0; i<num; i++) {
            BDBG_MSG(("host PB[%2u]: %u:%#lx", mtsifConnections[i].index, mtsifConnections[i].settings.sourceType,
                mtsifConnections[i].settings.sourceType==NEXUS_ParserBandSourceType_eMtsif ? mtsifConnections[i].settings.sourceTypeSettings.mtsif : NULL));
        }
    }
#endif

    /* print MTSIF status */
    {
        BMXT_MtsifStatus mtsifStatus;
        BMXT_GetMtsifStatus(mxt, &mtsifStatus);
        for (i=0; i<2; i++) {
            if (mtsifStatus.tx[i].enabled) {
                BDBG_MSG(("MTSIF_TX[%u]: enabled, polarity %u, ifWidth %u", i, mtsifStatus.tx[i].clockPolarity, mtsifStatus.tx[i].interfaceWidth));
            }
        }
        for (i=0; i<1; i++) {
            if (mtsifStatus.rx[i].enabled) {
                BDBG_MSG(("MTSIF_RX[%u]: enabled, polarity %u, ifWidth %u", i, mtsifStatus.rx[i].clockPolarity, mtsifStatus.rx[i].interfaceWidth));
            }
        }
    }

    #if 0
    /* print the IB sync count later since it's only useful after the Tune */
    NEXUS_ScheduleTimer(4000, NEXUS_Frontend_P_PostDumpMtsifConfig, pConfig);
    #endif
#else
    BSTD_UNUSED(pConfig);
#endif
}
#endif

NEXUS_Error NEXUS_Frontend_ReapplyTransportSettings(NEXUS_FrontendHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    /* call the device-specific function because the cached MTSIF-config struct is per-device */
    if (NULL == handle->reapplyTransportSettings) {
        if (handle->pParentFrontend) {
            return NEXUS_Frontend_ReapplyTransportSettings(handle->pParentFrontend);
        }
        else {
            return BERR_TRACE(BERR_SUCCESS);
        }
    }
    else {
        return handle->reapplyTransportSettings(handle->pDeviceHandle);
    }
}

NEXUS_Error NEXUS_Frontend_WriteRegister(NEXUS_FrontendHandle handle, unsigned address, uint32_t value )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if (NULL == handle->writeRegister) {
        if (handle->pParentFrontend) {
            return NEXUS_Frontend_WriteRegister(handle->pParentFrontend, address, value);
        }
        else {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else {
        return handle->writeRegister(handle->pDeviceHandle, address, value);
    }
}

NEXUS_Error NEXUS_Frontend_ReadRegister(NEXUS_FrontendHandle handle, unsigned address,  uint32_t *value )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if (NULL == handle->readRegister) {
        if (handle->pParentFrontend) {
            return NEXUS_Frontend_ReadRegister(handle->pParentFrontend, address, value);
        }
        else {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else {
        return handle->readRegister(handle->pDeviceHandle, address, value);
    }
}

NEXUS_FrontendHandle NEXUS_Frontend_OpenStub( unsigned index )
{
    BSTD_UNUSED(index);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}

NEXUS_Error NEXUS_Frontend_Standby_priv(NEXUS_FrontendHandle handle, bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if(handle->standby)
    {
        return handle->standby(handle->pDeviceHandle, enabled, pSettings);
    }
    else if ( handle->pParentFrontend && handle->pParentFrontend->standby ) {
        return NEXUS_Frontend_Standby_priv(handle->pParentFrontend, enabled, pSettings);
    }
    else
    {
        /* Frontend does not have a standby api. This is a valid case */
        return NEXUS_SUCCESS;
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}

NEXUS_Error NEXUS_Frontend_GetTemperature(NEXUS_FrontendHandle handle, NEXUS_FrontendTemperature *pTemp)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Frontend, handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getTemperature)
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetTemperature(handle->pParentFrontend, pTemp);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getTemperature(handle->pDeviceHandle, pTemp);
    }
}

NEXUS_Error NEXUS_Frontend_TransmitDebugPacket(NEXUS_FrontendHandle handle, NEXUS_FrontendDebugPacketType type, const uint8_t *pBuffer, size_t size)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->transmitDebugPacket)
    {
        if (handle->pParentFrontend)
        {
            return NEXUS_Frontend_TransmitDebugPacket(handle->pParentFrontend, type, pBuffer, size);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->transmitDebugPacket(handle->pDeviceHandle, type, pBuffer, size);
    }
}

NEXUS_FrontendConnectorHandle NEXUS_Frontend_GetConnector( NEXUS_FrontendHandle handle )
{
    NEXUS_OBJECT_ASSERT(NEXUS_Frontend, handle);
    return handle->connector;
}

#if defined(NEXUS_FRONTEND_DOCSIS) && defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)
NEXUS_Error NEXUS_FrontendDevice_P_GetDocsisLnaDeviceAgcValue(NEXUS_FrontendDeviceHandle handle,
                                                              uint32_t *agcValue)
{
    BDBG_ASSERT(NULL != handle);
    if ( NULL == handle->getDocsisLnaDeviceAgcValue)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->getDocsisLnaDeviceAgcValue(handle->pDevice, agcValue);
    }
}
NEXUS_Error NEXUS_FrontendDevice_P_SetHostChannelLockStatus(NEXUS_FrontendDeviceHandle handle,
                                                            unsigned channelNum,bool locked)
{
    BDBG_ASSERT(NULL != handle);
    if ( NULL == handle->getDocsisLnaDeviceAgcValue)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->setHostChannelLockStatus(handle->pDevice,channelNum,locked);
    }
}
#endif

NEXUS_Error NEXUS_FrontendDevice_P_GetInternalGain(NEXUS_FrontendDeviceHandle handle, const NEXUS_GainParameters *params, NEXUS_InternalGainSettings *pSettings)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getInternalGain)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->getInternalGain(handle->pDevice, params, pSettings);
    }
}

NEXUS_Error NEXUS_FrontendDevice_P_SetExternalGain(NEXUS_FrontendDeviceHandle handle, const NEXUS_ExternalGainSettings *pSettings)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->setExternalGain)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->setExternalGain(handle->pDevice, pSettings);
    }
}

void NEXUS_FrontendDevice_GetDefaultLinkSettings(NEXUS_FrontendDeviceLinkSettings *pSettings)
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->rfInput = NEXUS_FrontendDeviceRfInput_eDaisy;
}

NEXUS_FrontendDeviceHandle NEXUS_Frontend_GetDevice(NEXUS_FrontendHandle handle)
{
    BDBG_ASSERT(NULL != handle);

    if(NULL == handle->pGenericDeviceHandle)
    {
        BDBG_WRN(("Device handle not initialized."));
        return NULL;
    }
    else
    {
        return handle->pGenericDeviceHandle;
    }

}

NEXUS_Error NEXUS_FrontendDevice_Link(NEXUS_FrontendDeviceHandle parentHandle, NEXUS_FrontendDeviceHandle childHandle, const NEXUS_FrontendDeviceLinkSettings *pSettings)
{
    NEXUS_Error rc=NEXUS_SUCCESS;

    /* parent or child can override how the link works, child first */
    if (childHandle->deviceLink) {
        rc = childHandle->deviceLink(parentHandle, childHandle, pSettings);
    } else if (parentHandle->deviceLink) {
        rc = parentHandle->deviceLink(parentHandle, childHandle, pSettings);
    } else {
        /* otherwise, standard chained config */
        BLST_D_INSERT_HEAD(&parentHandle->deviceChildList, childHandle, link);
        childHandle->parent = parentHandle;
        if (pSettings) {
            childHandle->linkSettings = *pSettings;

            if (pSettings->mtsif==NEXUS_FrontendDeviceMtsifOutput_eDaisy) {
                parentHandle->chainedConfig = &childHandle->mtsifConfig;
                childHandle->chainedConfig = &parentHandle->mtsifConfig;
                childHandle->mtsifConfig.slave = true;
            }
        }
    }

    return rc;
}

void NEXUS_FrontendDevice_Unlink(NEXUS_FrontendDeviceHandle parentHandle,NEXUS_FrontendDeviceHandle childHandle)
{
    NEXUS_FrontendDeviceHandle tempChildHandle = NULL;
    if(childHandle){
        BLST_D_REMOVE(&parentHandle->deviceChildList, childHandle, link);
    }
    else {
        /* Remove all the children of this device. */
        for ( tempChildHandle = BLST_D_FIRST(&parentHandle->deviceChildList); NULL != tempChildHandle; tempChildHandle = BLST_D_NEXT(tempChildHandle, link) )
        {
            BLST_D_REMOVE(&parentHandle->deviceChildList, tempChildHandle, link);
            tempChildHandle->parent = NULL;
        }
    }

    /* Remove the device from its parent's list of children. */
    if(parentHandle->parent != NULL){
        BLST_D_REMOVE(&parentHandle->parent->deviceChildList, parentHandle, link);
    }
}

static void NEXUS_FrontendDevice_P_Release(NEXUS_FrontendDeviceHandle handle)
{
    if (handle->tripwire == 0x54329876) {
        BDBG_MSG(("Released device handle %p is fine...", (void *)handle));
    } else {
        BDBG_WRN(("device handle %p might have been corrupted...", (void *)handle));
    }

    NEXUS_OBJECT_UNREGISTER(NEXUS_FrontendDevice, handle, Destroy);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_FrontendDevice, NEXUS_FrontendDevice_Close);

static void  NEXUS_FrontendDevice_P_Finalizer(NEXUS_FrontendDeviceHandle handle)
{
    BDBG_ASSERT(NULL != handle);

    if ( NULL == handle->close)
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }
    else
    {
        handle->close(handle->pDevice);
    }
}
NEXUS_Error NEXUS_FrontendDevice_GetStatus(NEXUS_FrontendDeviceHandle handle, NEXUS_FrontendDeviceStatus *pStatus)
{
    if(handle != NULL){
        if ( NULL == handle->getStatus)
        {
            BDBG_WRN(("Device status not supported for this frontend device."));
            return NEXUS_SUCCESS;
        }
        else
        {
            return handle->getStatus(handle->pDevice, pStatus);
        }
    }
    else{
        /* This is to handle the scenario where the device api is not supported. */
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
        return NEXUS_SUCCESS;
    }
}

NEXUS_Error NEXUS_FrontendDevice_P_Standby(NEXUS_FrontendDeviceHandle handle, const NEXUS_StandbySettings *pSettings)
{
    BDBG_ASSERT(NULL != handle);

    if((handle->mode != NEXUS_StandbyMode_eDeepSleep) && (NEXUS_FrontendDevice_P_CheckOpen(handle))){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->standby)
    {
        BDBG_WRN(("Device standby not supported for this frontend device."));
        return NEXUS_SUCCESS;
    }
    else
    {
        return handle->standby(handle->pDevice, pSettings);
    }
}

void NEXUS_FrontendDevice_GetDefaultRecalibrateSettings(NEXUS_FrontendDeviceRecalibrateSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->cppm.powerLevelChange);
    NEXUS_CallbackDesc_Init(&pSettings->cppm.calibrationComplete);
}

NEXUS_Error NEXUS_FrontendDevice_Recalibrate(NEXUS_FrontendDeviceHandle handle, const NEXUS_FrontendDeviceRecalibrateSettings *pSettings)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->recalibrate)
    {
        BDBG_WRN(("Device recalibration not supported for this frontend device."));
        return NEXUS_SUCCESS;
    }
    else
    {
         return handle->recalibrate(handle->pDevice, pSettings);
    }
}

void NEXUS_FrontendDevice_GetCapabilities(NEXUS_FrontendDeviceHandle handle, NEXUS_FrontendDeviceCapabilities *pCapabilities)
{
    BDBG_ASSERT(NULL != handle);

    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));

#if NEXUS_FRONTEND_PASSTHROUGH
    pCapabilities->numTuners = NEXUS_MAX_FRONTENDS;
    return;
#endif

    if (!handle->nonblocking.getCapabilities) {
        if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
            BDBG_ERR(("Device open failed. Cannot get device capabilities."));
            BERR_TRACE(NEXUS_NOT_INITIALIZED);
            return;
        }
    }

    if ( NULL == handle->getCapabilities)
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        handle->getCapabilities(handle->pDevice, pCapabilities);
    }
}

NEXUS_Error NEXUS_FrontendDevice_GetSatelliteCapabilities(NEXUS_FrontendDeviceHandle handle, NEXUS_FrontendSatelliteCapabilities *pCapabilities)
{
    BDBG_ASSERT(NULL != handle);

    if (!handle->nonblocking.getSatelliteCapabilities) {
        if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
            BDBG_ERR(("Device open failed. Cannot get device satellite capabilities."));
            return BERR_TRACE(NEXUS_NOT_INITIALIZED);
        }
    }

    if ( NULL == handle->getSatelliteCapabilities)
    {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    else
    {
        return handle->getSatelliteCapabilities(handle->pDevice, pCapabilities);
    }
}

void NEXUS_FrontendDevice_GetTunerCapabilities(NEXUS_FrontendDeviceHandle handle, unsigned tunerIndex, NEXUS_FrontendCapabilities *pCapabilities)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot get device tuner capabilities.."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( NULL == handle->getTunerCapabilities)
    {
        BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        handle->getTunerCapabilities(handle->pDevice, tunerIndex, pCapabilities);
    }
}

NEXUS_Error NEXUS_FrontendDevice_LinkAmplifier(NEXUS_FrontendDeviceHandle handle, NEXUS_AmplifierHandle amplifierHandle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != amplifierHandle);
#if NEXUS_AMPLIFIER_SUPPORT
    handle->amplifier = amplifierHandle;
#endif
    return rc;
}

#if NEXUS_AMPLIFIER_SUPPORT
NEXUS_Error NEXUS_FrontendDevice_P_GetAmplifierStatus(NEXUS_FrontendDeviceHandle handle, NEXUS_AmplifierStatus *pStatus)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getAmplifierStatus)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->getAmplifierStatus(handle->pDevice, pStatus);
    }
}

NEXUS_Error NEXUS_FrontendDevice_P_SetAmplifierStatus(NEXUS_FrontendDeviceHandle handle, const NEXUS_AmplifierStatus *pStatus)
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->setAmplifierStatus)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->setAmplifierStatus(handle->pDevice, pStatus);
    }
}
#endif

NEXUS_Error NEXUS_FrontendDevice_P_CheckOpen(NEXUS_FrontendDeviceHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

iterate:
    if(handle->openFailed || handle->abortThread){
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;
    }
    else if(handle->openPending){
        rc = BKNI_Sleep(50);
        if (rc) {rc = BERR_TRACE(rc); goto done;}
        goto iterate;
    } else if (handle->delayedInitializationRequired) {
        if (handle->delayedInit(handle) != NEXUS_SUCCESS) {
            rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;
        }
        handle->delayedInitializationRequired = false;
    }
done:
    if(rc && (handle->openPending || handle->openFailed )){
        BDBG_WRN(("NEXUS_FrontendDevice_P_CheckOpen: Application aborted prematurely. Waiting for device initialization thread to exit."));
        handle->abortThread = true;
        NEXUS_FrontendDevice_Unlink(handle, NULL);
    }
    return rc;
}

NEXUS_Error NEXUS_Frontend_P_CheckDeviceOpen(NEXUS_FrontendHandle handle)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_FrontendDeviceHandle deviceHandle = NULL;

    deviceHandle = NEXUS_Frontend_GetDevice(handle);
    if(deviceHandle == NULL) goto done;

    rc = NEXUS_FrontendDevice_P_CheckOpen(deviceHandle);
done:
    return rc;
}

BDBG_FILE_MODULE(nexus_frontend_proc);
void NEXUS_FrontendModule_P_Print(void)
{
#if BDBG_DEBUG_BUILD
    NEXUS_FrontendHandle frontend;
    NEXUS_FrontendDeviceHandle device;
    unsigned index;
    NEXUS_Error errCode;
    bool handleFound = false;
    NEXUS_FrontendDeviceHandle tempHandle = NULL, deviceHandle = NULL;
    NEXUS_FrontendCapabilities frontendCapabilities;
    NEXUS_FrontendQamStatus *pQamStatus = NULL;
    NEXUS_FrontendSatelliteStatus *pSatStatus = NULL;
    NEXUS_FrontendVsbStatus *pVsbStatus = NULL;
    NEXUS_FrontendOfdmStatus *pOfdmStatus = NULL;

    pQamStatus = BKNI_Malloc(sizeof(*pQamStatus));
    if (NULL == pQamStatus) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}
    pSatStatus = BKNI_Malloc(sizeof(*pSatStatus));
    if (NULL == pSatStatus) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}
    pVsbStatus = BKNI_Malloc(sizeof(*pVsbStatus));
    if (NULL == pVsbStatus) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}
    pOfdmStatus = BKNI_Malloc(sizeof(*pOfdmStatus));
    if (NULL == pOfdmStatus) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;}

    BLST_D_INIT(&g_frontendDeviceList.deviceList);

    /* TODO: frontend list should be moved per device */
    for (frontend = BLST_SQ_FIRST(&g_frontendList.frontends), index = 0; frontend; frontend = BLST_SQ_NEXT(frontend, link), index++) {
        handleFound = false;
        deviceHandle = NEXUS_Frontend_GetDevice(frontend);
         if (deviceHandle != NULL) {
            for (tempHandle = BLST_D_FIRST(&g_frontendDeviceList.deviceList); tempHandle; tempHandle = BLST_D_NEXT(tempHandle, node)) {
                if (tempHandle == deviceHandle) {
                    handleFound = true;
                }
            }
            if (!handleFound) {
                if (BLST_D_FIRST(&g_frontendDeviceList.deviceList)==NULL) {
                    BLST_D_INSERT_HEAD(&g_frontendDeviceList.deviceList, deviceHandle, node);
                }
                else {
                    BLST_D_INSERT_AFTER(&g_frontendDeviceList.deviceList, BLST_D_FIRST(&g_frontendDeviceList.deviceList), deviceHandle, node);
                }
            }
        }
#if 0 /* MXT regdump */
{
        void BMXT_P_RegDump(BMXT_Handle handle);
        if (deviceHandle) {
            BMXT_Handle mxt = deviceHandle->mtsifConfig.mxt;
            BMXT_P_RegDump(mxt);
            goto done;
        }
}
#endif
        BDBG_MODULE_LOG(nexus_frontend_proc, ("frontend %2d: %p:%p, acquired %c", index, (void *)frontend, (void *)deviceHandle, frontend->acquired?'y':'n'));
        NEXUS_Frontend_GetCapabilities(frontend, &frontendCapabilities);
        if (frontend->acquired && frontendCapabilities.qam) {
            NEXUS_FrontendQamStatus *pStatus = pQamStatus;
            int modulation;
            BDBG_ASSERT(pStatus);

            BDBG_MODULE_LOG(nexus_frontend_proc, (" Cable/QAM"));
            /* must use sync call to for status, can't modify the application async callback setup */
            errCode = NEXUS_Frontend_GetQamStatus(frontend, pStatus);
            if (errCode) {
                BDBG_MODULE_LOG(nexus_frontend_proc, ("    Error reading status, errCode=%d", errCode));
                continue;
            }
            modulation = 0;
            if (pStatus->settings.mode != NEXUS_FrontendQamMode_eAuto_64_256) {
                modulation = 16 << pStatus->settings.mode; /* use as multiple by 2 */
                BDBG_MODULE_LOG(nexus_frontend_proc, ("    freq=%d hz, modulation=Qam-%d, annex=%c",
                    pStatus->settings.frequency, modulation, pStatus->settings.annex+'A'));
            }
            else {
                BDBG_MODULE_LOG(nexus_frontend_proc, ("    freq=%dhz, modulation=auto, annex=%c",
                    pStatus->settings.frequency, pStatus->settings.annex+'A'));
            }
            BDBG_MODULE_LOG(nexus_frontend_proc, ("    lockStatus=%s, snr=%ddB (est), fecCorrected=%d, fecUncorrected=%d",
                (pStatus->fecLock)? "locked": "unlocked",
                pStatus->snrEstimate/100, pStatus->fecCorrected, pStatus->fecUncorrected));
        }
        if (frontend->acquired && frontendCapabilities.satellite) {
            NEXUS_FrontendSatelliteStatus *pStatus = pSatStatus;
            int modulation;
            static const char *satModulation[] = {"dss", "dcii", "qpskTurbo", "8pskTubro", "turbo", "qpskLdpc", "8pskLdpc", "ldpc", "blindAcquistion", "unknown" };
            BDBG_ASSERT(pStatus);

            /* must use sync call to for status, can't modify the application async callback setup */
            BDBG_MODULE_LOG(nexus_frontend_proc, (" Satellite"));
            errCode = NEXUS_Frontend_GetSatelliteStatus(frontend, pStatus);
            if (errCode) {
                BDBG_MODULE_LOG(nexus_frontend_proc, ("    Error reading status, errCode=%d", errCode));
                continue;
            }
            modulation = (pStatus->mode < (sizeof(satModulation)/sizeof(char *))? pStatus->mode: ((sizeof(satModulation)/sizeof(char *))-1));
            BDBG_MODULE_LOG(nexus_frontend_proc, ("    freq=%dhz, modulation=%s, codeRate=%d/%d %s",
                pStatus->settings.frequency, satModulation[modulation],
                pStatus->settings.codeRate.numerator, pStatus->settings.codeRate.denominator,
                (pStatus->settings.bertEnable)? "bert":""));
            BDBG_MODULE_LOG(nexus_frontend_proc, ("    lockStatus=%s, snr=%ddB (est), fecCorrected=%d, fecUncorrected=%d",
                (pStatus->demodLocked)? "locked": "unlocked",
                pStatus->snrEstimate/100, pStatus->fecCorrected, pStatus->fecUncorrected));
        }
        if (frontend->acquired && frontendCapabilities.vsb) {
            NEXUS_FrontendVsbStatus *pStatus = pVsbStatus;
            BDBG_ASSERT(pStatus);

            /* must use sync call to for status, can't modify the application async callback setup */
            BDBG_MODULE_LOG(nexus_frontend_proc, (" ATSC/VSB"));
            errCode = NEXUS_Frontend_GetVsbStatus(frontend, pStatus);
            if (errCode) {
                BDBG_MODULE_LOG(nexus_frontend_proc, ("    Error reading status, errCode=%d", errCode));
                continue;
            }
            BDBG_MODULE_LOG(nexus_frontend_proc, ("    freq=%dhz, modulation=%d",
                pStatus->settings.frequency, (pStatus->settings.mode == NEXUS_FrontendVsbMode_e8)? 8: 16));
            BDBG_MODULE_LOG(nexus_frontend_proc, ("    lockStatus=%s, snr=%ddB (est), fecCorrected=%d, fecUncorrected=%d",
                (pStatus->fecLock)? "locked": "unlocked",
                pStatus->snrEstimate/100, pStatus->fecCorrected, pStatus->fecUncorrected));
        }
        if (frontend->acquired && frontendCapabilities.ofdm) {
            NEXUS_FrontendOfdmStatus *pStatus = pOfdmStatus;
            int mode, modulation;
            static const char *ofdmModulation[] = {"qpsk", "Qam-16", "Qam-64", "Qam-256", "dqpsk", "unknown" };
            static const char *ofdmMode[] = {"DVB-T", "DVB-T2", "DVB-C2", "ISDBT", "unknown" };
            BDBG_ASSERT(pStatus);

            /* must use sync call to for status, can't modify the application async callback setup */
            errCode = NEXUS_Frontend_GetOfdmStatus(frontend, pStatus);
            if (errCode) {
                BDBG_MODULE_LOG(nexus_frontend_proc, (" OFDM"));
                BDBG_MODULE_LOG(nexus_frontend_proc, ("    Error reading status, errCode=%d", errCode));
                pStatus = NULL;
                continue;
            }
            mode = (pStatus->settings.mode < (sizeof(ofdmMode)/sizeof(char *)))? pStatus->settings.mode: ((sizeof(ofdmMode)/sizeof(char *))-1);
            modulation = (pStatus->modulation < (sizeof(ofdmModulation)/sizeof(char *)))? pStatus->modulation: ((sizeof(ofdmModulation)/sizeof(char *)-1));
            BDBG_MODULE_LOG(nexus_frontend_proc, (" %s", ofdmMode[mode]));
            BDBG_MODULE_LOG(nexus_frontend_proc, ("    freq=%dhz, modulation=%s",
                pStatus->settings.frequency, ofdmModulation[modulation]));
            BDBG_MODULE_LOG(nexus_frontend_proc, ("    lockStatus=%s, snr=%ddB, fecCorrected=%d, fecUncorrected=%d",
                (pStatus->fecLock)? "locked": "unlocked",
                pStatus->snr/100, pStatus->fecCorrectedBlocks, pStatus->fecUncorrectedBlocks));
        }
    }

    for (device = BLST_D_FIRST(&g_frontendDeviceList.deviceList); device; device = BLST_D_NEXT(device, node)) {
        BDBG_MODULE_LOG(nexus_frontend_proc, ("device %p (%#x)", (void *)device, device->familyId));

        if (device->parent) {
            BDBG_MODULE_LOG(nexus_frontend_proc, ("  parent %#x: %p", device->parent->familyId, (void *)device->parent));
        }
#if NEXUS_HAS_MXT
        {
            unsigned j;
            NEXUS_FrontendDeviceMtsifConfig *pConfig = &device->mtsifConfig;
            NEXUS_FrontendDeviceMtsifConfig *pChainedConfig = device->chainedConfig;
            if (pChainedConfig && pChainedConfig->slave) {
                BDBG_MODULE_LOG(nexus_frontend_proc, ("  chain %p -> %p -> host", (void *)pChainedConfig, (void *)pConfig));
            }
            for (j=0; j<pConfig->numDemodPb; j++) {
                if (pConfig->demodPbSettings[j].enabled) {
                    /* lookup frontend using connector */
                    for (frontend = BLST_SQ_FIRST(&g_frontendList.frontends); frontend; frontend = BLST_SQ_NEXT(frontend, link)) {
                        if (frontend->connector == pConfig->demodPbSettings[j].connector) break;
                    }
                    BDBG_MODULE_LOG(nexus_frontend_proc, ("  frontend %p -> IB%u -> PB%u -> PB%u (TX%u)",
                        (void *)frontend, pConfig->demodPbSettings[j].inputBandNum, j, pConfig->demodPbSettings[j].virtualNum, pConfig->demodPbSettings[j].mtsifTxSel));
                }
            }
        }
#endif
    }

#if NEXUS_HAS_MXT && (defined BCHP_XPT_FE_MTSIF_RX1_PKT_BAND0_BAND31_DETECT)
    for (index=0; index<BXPT_NUM_MTSIF; index++) {
        uint32_t step = BCHP_XPT_FE_MTSIF_RX1_PKT_BAND0_BAND31_DETECT - BCHP_XPT_FE_MTSIF_RX0_PKT_BAND0_BAND31_DETECT;
        uint32_t addr = BCHP_XPT_FE_MTSIF_RX0_PKT_BAND0_BAND31_DETECT + step*index;
        BREG_Write32(g_pCoreHandles->reg, addr, 0);
    }
    BKNI_Sleep(1);
    for (index=0; index<BXPT_NUM_MTSIF; index++) {
        uint32_t step = BCHP_XPT_FE_MTSIF_RX1_PKT_BAND0_BAND31_DETECT - BCHP_XPT_FE_MTSIF_RX0_PKT_BAND0_BAND31_DETECT;
        uint32_t addr = BCHP_XPT_FE_MTSIF_RX0_PKT_BAND0_BAND31_DETECT + step*index;
        BDBG_MODULE_LOG(nexus_frontend_proc, ("MTSIF_RX%u BAND_DETECT = %08x", index, BREG_Read32(g_pCoreHandles->reg, addr)));
    }
#endif

done:
    for (tempHandle = BLST_D_FIRST(&g_frontendDeviceList.deviceList); tempHandle; tempHandle = BLST_D_NEXT(tempHandle, node)) {
        BLST_D_REMOVE(&g_frontendDeviceList.deviceList, tempHandle, node);
    }

    if (pQamStatus) { BKNI_Free(pQamStatus); }
    if (pSatStatus) { BKNI_Free(pSatStatus); }
    if (pVsbStatus) { BKNI_Free(pVsbStatus); }
    if (pOfdmStatus) { BKNI_Free(pOfdmStatus); }
#endif
}


#if NEXUS_HAS_SPI
#include "nexus_spi.h"
#endif

NEXUS_Error NEXUS_FrontendDevice_Probe(const NEXUS_FrontendDeviceOpenSettings *pSettings, NEXUS_FrontendProbeResults *pResults)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned chipId=0;
    NEXUS_GpioHandle gpioHandle = NULL;

#if NEXUS_FRONTEND_PASSTHROUGH
    pResults->chip.familyId = 0x9999;
    return rc;
#endif

    if (pSettings->reset.enable) {
        NEXUS_GpioSettings gpioSettings;
        BDBG_MSG(("Setting GPIO %d high",pSettings->reset.pin));
        NEXUS_Gpio_GetDefaultSettings(pSettings->reset.type, &gpioSettings);
        gpioSettings.mode = NEXUS_GpioMode_eOutputPushPull;
        gpioSettings.value = pSettings->reset.value;
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        gpioHandle = NEXUS_Gpio_Open(pSettings->reset.type, pSettings->reset.pin, &gpioSettings);
        BDBG_ASSERT(NULL != gpioHandle);

        BKNI_Sleep(500);
    }

    /* Read chipId based on i2c (if provided), SPI (if provided), or chip ID (if neither provided).
     * For i2c/spi, read two bytes. If the resulting id is below the lowest known frontend, read one more byte,
     * e.g. for 45216.
     */
    if (pSettings->i2cDevice) {
        BREG_I2C_Handle i2cHandle;
        uint8_t buf[5];
        uint8_t subAddr;

        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NULL);
        BDBG_MSG(("i2c handle: %p, i2cAddress: 0x%x",(void*)i2cHandle,pSettings->i2cAddress));
        buf[0]= 0x0;
        subAddr = 0x1;
        BREG_I2C_WriteNoAddr(i2cHandle, pSettings->i2cAddress, (uint8_t *)&subAddr, 1);
        BREG_I2C_ReadNoAddr(i2cHandle, pSettings->i2cAddress, buf, 1);
        chipId = buf[0];

        subAddr = 0x2;
        BREG_I2C_WriteNoAddr(i2cHandle, pSettings->i2cAddress, (uint8_t *)&subAddr, 1);
        BREG_I2C_ReadNoAddr(i2cHandle, pSettings->i2cAddress, buf, 1);
        chipId = (chipId << 8) | buf[0];

        if (chipId < 0x3000) {
            subAddr = 0x3;
            BREG_I2C_WriteNoAddr(i2cHandle, pSettings->i2cAddress, (uint8_t *)&subAddr, 1);
            BREG_I2C_ReadNoAddr(i2cHandle, pSettings->i2cAddress, buf, 1);
            chipId = (chipId << 8) | buf[0];
        }

        BDBG_MSG(("chip family ID = 0x%04x", chipId));
    }
#if NEXUS_HAS_SPI
#define NUM_SPI_BYTES 8
#define NUM_SPI_ADDRESSES 3
    else if (pSettings->spiDevice) {
        uint8_t wData[NUM_SPI_BYTES], rData[NUM_SPI_BYTES];
        NEXUS_Error rc;
        uint8_t spiAddr[NUM_SPI_ADDRESSES] = { 0x20, 0x40, 0x24 };
        unsigned i = 0;

        BDBG_MSG(("Probing for chip at SPI %p",(void *)pSettings->spiDevice));

        BKNI_Memset(wData, 0, NUM_SPI_BYTES);

        while (i < NUM_SPI_ADDRESSES && (chipId == 0 || chipId == 0xFFFF)) {

            wData[0] = spiAddr[i]<<1;
#if DEBUG_SPI_READS
            {
                int i;
                for (i=0; i < 2; i++) {
                    BDBG_MSG(("wData[%d]: 0x%02x",i,wData[i]));
                }
            }
#endif

            rc = NEXUS_Spi_Read(pSettings->spiDevice, wData, rData, NUM_SPI_BYTES);
            if(rc) {rc = BERR_TRACE(rc);}

#if DEBUG_SPI_READS
            {
                int i;
                for (i=0; i < NUM_SPI_BYTES; i++) {
                    BDBG_MSG(("rData[%d]: 0x%02x",i,rData[i]));
                }
            }
#endif

            chipId = (rData[3] << 8) | (rData[4]);
            if (chipId < 0x3000) {
                chipId = (chipId << 8) | (rData[5]);
            }

            BDBG_MSG(("chip family ID = 0x%04x", chipId));
            i++;
        }

    }
#endif
    else {
        BCHP_Info chipInfo;
        BCHP_GetInfo(g_pCoreHandles->chp, &chipInfo);
        chipId = chipInfo.familyId;
    }

    if (chipId == 0xffff)
        chipId = 0;
    pResults->chip.familyId = chipId;

    if (pSettings->reset.enable) {
        if (gpioHandle)
            NEXUS_Gpio_Close(gpioHandle);
    }
    return rc;
}

void NEXUS_FrontendDevice_GetDefaultOpenSettings(NEXUS_FrontendDeviceOpenSettings *pSettings)
{
    int i;
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    for (i=0; i < NEXUS_MAX_MTSIF; i++) {
        pSettings->mtsif[i].enabled = true;
    }
    pSettings->cable.enabled = true;
    pSettings->terrestrial.enabled = true;
    pSettings->satellite.enabled = true;
    pSettings->loadAP = true;
    NEXUS_CallbackDesc_Init(&pSettings->updateGainCallback);
}

NEXUS_FrontendDeviceHandle NEXUS_FrontendDevice_Open(unsigned index, const NEXUS_FrontendDeviceOpenSettings *pSettings)
{
#if NEXUS_FRONTEND_PASSTHROUGH
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    return NEXUS_FrontendDevice_P_OpenPassthrough();
#else
    unsigned i;
    NEXUS_FrontendProbeResults probe;

    if (!NEXUS_FrontendDevice_Probe(pSettings, &probe)) {
        for (i=0;g_frontends[i].chipid;i++) {
            BDBG_MSG(("NEXUS_FrontendDevice_Open: %x %x | %d %d | %p",g_frontends[i].chipid,probe.chip.familyId,g_frontends[i].chipid,probe.chip.familyId,(void *)(unsigned long)g_frontends[i].openDevice));
            if (g_frontends[i].chipid == probe.chip.familyId) {
                return g_frontends[i].openDevice(index, pSettings);
            }
        }
    }
    {
        unsigned familyId = probe.chip.familyId;
        if ((familyId == 0x45316) || (familyId == 0x45304) || (familyId == 0x45302)) familyId = 0x45308;
        if (familyId == 0x3465) familyId = 0x3466;
        BDBG_WRN(("Detected frontend %x does not have an entry in the frontend table. Was Nexus built with NEXUS_FRONTEND_%x=y set?", familyId, familyId));
    }
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
#endif
}

void NEXUS_FrontendDevice_GetDefaultSettings(
    NEXUS_FrontendDeviceSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error NEXUS_FrontendDevice_GetSettings(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_FrontendDeviceSettings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getSettings)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->getSettings(handle->pDevice, pSettings);
    }
}

NEXUS_Error NEXUS_FrontendDevice_SetSettings(
    NEXUS_FrontendDeviceHandle handle,
    const NEXUS_FrontendDeviceSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->setSettings)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->setSettings(handle->pDevice, pSettings);
    }
}

NEXUS_Error NEXUS_FrontendDevice_GetAmplifierStatus(
    NEXUS_FrontendDeviceHandle handle,
    NEXUS_FrontendDeviceAmplifierStatus *pStatus
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_FrontendDevice_P_CheckOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDeviceAmplifierStatus)
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else
    {
        return handle->getDeviceAmplifierStatus(handle->pDevice, pStatus);
    }
}

void NEXUS_Frontend_GetDefaultOpenSettings(NEXUS_FrontendChannelSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendHandle NEXUS_Frontend_Open(const NEXUS_FrontendChannelSettings *pSettings)
{
#if NEXUS_FRONTEND_PASSTHROUGH
    BSTD_UNUSED(g_frontends);
    return NEXUS_Frontend_P_OpenPassthrough(pSettings->device);
#else
    unsigned i;

    if (pSettings->device->familyId) {
        for (i=0;g_frontends[i].chipid;i++) {
            if (g_frontends[i].chipid == pSettings->device->familyId) {
                BDBG_MSG(("NEXUS_Frontend_Open: %x %x | %d %d | %p",g_frontends[i].chipid,pSettings->device->familyId,g_frontends[i].chipid,pSettings->device->familyId,(void *)(unsigned long)g_frontends[i].openChannel));
                return g_frontends[i].openChannel(pSettings);
            }
        }
    }
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
#endif
}


void NEXUS_FrontendDevice_GetWakeupSettings(NEXUS_FrontendDeviceHandle handle, NEXUS_TransportWakeupSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    if(handle != NULL){
        if (NULL == handle->getWakeupSettings)
        {
            BDBG_WRN(("Getting or setting wakeup packet settings is not supported for this frontend device."));
        }
        else
        {
            handle->getWakeupSettings(handle, pSettings);
            return;
        }
    }
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_FrontendDevice_SetWakeupSettings(NEXUS_FrontendDeviceHandle handle, const NEXUS_TransportWakeupSettings *pSettings)
{
    if(handle != NULL){
        if (NULL == handle->setWakeupSettings)
        {
            BDBG_WRN(("Getting or setting wakeup packet settings is not supported for this frontend device."));
        }
        else
        {
            return handle->setWakeupSettings(handle, pSettings);
        }
    }
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_FrontendModule_GetStatistics( NEXUS_FrontendModuleStatistics *pStats )
{
    *pStats = g_NEXUS_FrontendModuleStatistics;
}

#if NEXUS_HAS_MXT
static NEXUS_Error NEXUS_Frontend_P_FindDemodPidChannelIndex(unsigned first, unsigned bound, unsigned *index)
{
    unsigned i;
    for (i=first; i<bound; i++) {
        if (i>=BMXT_MAX_NUM_PIDCHANNELS) {
            break;
        }
        if (g_NEXUS_Frontend_P_HostMtsifConfig.demodPidChannelUsed[i]==false) {
            *index = i;
            return NEXUS_SUCCESS;
        }
    }

    return NEXUS_NOT_AVAILABLE;
}

static NEXUS_FrontendDeviceHandle NEXUS_FrontendConnector_P_GetDevice(NEXUS_FrontendConnectorHandle connector)
{
    NEXUS_FrontendHandle frontend = NULL;
    for (frontend = BLST_SQ_FIRST(&g_frontendList.frontends); frontend; frontend = BLST_SQ_NEXT(frontend, link)) {
        if (frontend->connector == connector) break;
    }
    if (frontend) {
        return frontend->pGenericDeviceHandle;
    }
    else {
        return NULL;
    }
}

static void NEXUS_Frontend_P_PidChannelCallback(void *arg)
{
    unsigned num, i, demodIndex;
    unsigned hostIndex, hostPb;
    NEXUS_P_MtsifPidChannelHandle pidChannel;
    NEXUS_Error rc;
    struct NEXUS_MtsifPidChannelSettings *settings = g_NEXUS_Frontend_P_HostMtsifConfig.pidSettings;
    bool exists;
    NEXUS_P_MtsifPidChannelHandle p, prev;
    NEXUS_FrontendDeviceHandle device;

    BSTD_UNUSED(arg);

    do {
        NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
        NEXUS_TransportModule_GetMtsifPidChannels_priv(settings, MAX_MTSIF_PID_CHANNELS_PER_CALLBACK, &num);
        NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);

        for (i=0; i<num; i++) {
            device = NEXUS_FrontendConnector_P_GetDevice(settings[i].frontend);
            if (device==NULL || device->mtsifConfig.pidfilter==false) {
                continue;
            }
            hostIndex = settings[i].status.pidChannelIndex;
            hostPb = settings[i].status.parserBand;
            exists = false;

            /* check if exists / find insertion point in order of hostIndex */
            for (prev=NULL,p=BLST_S_FIRST(&(g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels));p;prev=p,p=BLST_S_NEXT(p,link)) {
                if (p->hostIndex == hostIndex) {
                    exists = true;
                    break;
                }
                if (hostIndex < p->hostIndex) break;
            }

            if (settings[i].state == NEXUS_MtsifPidChannelState_eChanged) {
                if (!exists) {
                    rc = NEXUS_Frontend_P_FindDemodPidChannelIndex(hostIndex, BMXT_MAX_NUM_PIDCHANNELS, &demodIndex);
                    if (rc) {
                        rc = NEXUS_Frontend_P_FindDemodPidChannelIndex(BMXT_MAX_NUM_PARSERS, hostIndex, &demodIndex);
                        if (rc) {
                            BDBG_ERR(("Out of demod pidchannels (host pidchannel index %u)", hostIndex));
                            continue;
                        }
                    }
                    pidChannel = BKNI_Malloc(sizeof(*pidChannel));
                    if (!pidChannel) {
                        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                        continue;
                    }
                    BKNI_Memset(pidChannel, 0, sizeof(*pidChannel));

                    if (prev) {
                        BLST_S_INSERT_AFTER(&(g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels), prev, pidChannel, link);
                    }
                    else {
                        BLST_S_INSERT_HEAD(&(g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels), pidChannel, link);
                    }
                    g_NEXUS_Frontend_P_HostMtsifConfig.demodPidChannelUsed[demodIndex] = true;
                }
                else {
                    demodIndex = p->demodIndex;
                    pidChannel = p;
                }
                BDBG_ASSERT(pidChannel);
                pidChannel->pid = settings[i].status.pid;
                pidChannel->hostPb = hostPb;
                pidChannel->hostIndex = hostIndex;
                pidChannel->demodIndex = demodIndex;

                if (g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].connected) { /* demodPB -> hostPB already connected */
                    NEXUS_FrontendDeviceMtsifConfig *config = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[pidChannel->hostPb].deviceConfig;
                    if (exists && pidChannel->mxt && (pidChannel->mxt != config->mxt)) {
                        pidChannel->state = disabled;
                        BDBG_MSG(("MTSIF pidChannel %3u:%3u device change %p:%p", hostIndex, pidChannel->demodIndex, (void*)pidChannel->mxt, (void*)config->mxt));
                        NEXUS_Frontend_P_SetPid(pidChannel);
                    }
                    pidChannel->state = enabled;
                    pidChannel->demodPb = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPb].demodPb;
                    pidChannel->mtsifTxSel = config->demodPbSettings[pidChannel->demodPb].mtsifTxSel;
                    pidChannel->mxt = config->mxt;
                    BDBG_MSG(("MTSIF pidChannel %3u:%3u, PB%2u:%2u %s", hostIndex, pidChannel->demodIndex, hostPb, pidChannel->demodPb, exists?"(override)":""));
                    NEXUS_Frontend_P_SetPid(pidChannel);
                }
                else {
                    if (exists && pidChannel->mxt) {
                        pidChannel->state = disabled;
                        BDBG_MSG(("MTSIF pidChannel %3u:%3u device change %p", hostIndex, pidChannel->demodIndex, (void*)pidChannel->mxt));
                        NEXUS_Frontend_P_SetPid(pidChannel);
                    }
                    pidChannel->state = pendingEnable;
                    BDBG_MSG(("MTSIF pidChannel %3u:%3u, PB%2u:(pending) %s", hostIndex, pidChannel->demodIndex, hostPb, exists?"(override)":""));
                }
            }
            else if (settings[i].state == NEXUS_MtsifPidChannelState_eClosed) {
                if (exists) {
                    BDBG_MSG(("MTSIF pidChannel %3u:%3u close", p->hostIndex, p->demodIndex));
                    if (p->mxt) {
                        p->state = disabled;
                        NEXUS_Frontend_P_SetPid(p);
                    }
                    BLST_S_REMOVE(&(g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels), p, NEXUS_P_MtsifPidChannel, link);
                    g_NEXUS_Frontend_P_HostMtsifConfig.demodPidChannelUsed[p->demodIndex] = false;
                    BKNI_Free(p);
                }
            }
        }
    }while (num==MAX_MTSIF_PID_CHANNELS_PER_CALLBACK);

#if NEXUS_TRANSPORT_EXTENSION_TBG
    NEXUS_Frontend_P_TbgConfig(NULL);
#endif

    return;
}

static void NEXUS_Frontend_P_SetPid(NEXUS_P_MtsifPidChannelHandle pidChannel)
{
    BMXT_PidChannelSettings pidSettings;
    unsigned hostPb = pidChannel->hostPb;

    BDBG_ASSERT(hostPb < NEXUS_NUM_PARSER_BANDS);
    BDBG_ASSERT(pidChannel->mxt);

    pidSettings.pid = pidChannel->pid;
    pidSettings.enable = (pidChannel->state==enabled);
    pidSettings.inputSelect = pidChannel->demodPb;
    pidSettings.mtsifTxSelect = pidChannel->mtsifTxSel;
    BDBG_MSG(("Set pidchannel[%3u]: pid %x, enable %u, demodPB%u, TX%u", pidChannel->demodIndex, pidSettings.pid, pidSettings.enable, pidSettings.inputSelect, pidSettings.mtsifTxSelect));
    BMXT_ConfigPidChannel(pidChannel->mxt, pidChannel->demodIndex, &pidSettings);
}

static void NEXUS_Frontend_P_SetPendingPids(void)
{
    NEXUS_P_MtsifPidChannelHandle pidChannel;
    for (pidChannel = BLST_S_FIRST(&(g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels)); pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link)) {
        if (pidChannel->state==pendingEnable) {
            NEXUS_FrontendDeviceMtsifConfig *config = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[pidChannel->hostPb].deviceConfig;
            pidChannel->demodPb = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[pidChannel->hostPb].demodPb;
            pidChannel->mtsifTxSel = config->demodPbSettings[pidChannel->demodPb].mtsifTxSel;
            pidChannel->mxt = config->mxt;
            pidChannel->state = enabled;
            NEXUS_Frontend_P_SetPid(pidChannel);
        }
    }
}
#endif

#if NEXUS_TRANSPORT_EXTENSION_TBG
static void NEXUS_Frontend_P_TbgConfig(void *arg)
{
    BMXT_Tbg_GlobalConfig tbgGlobalConfig;
    BMXT_Tbg_ParserConfig tbgParserConfig;
    BMXT_Handle mxt = NULL;
    unsigned i, demodPb, hostPriPb, demodPriPb, demodPidIndex;
    NEXUS_P_MtsifPidChannelHandle pidChannel;
    struct NEXUS_TbgHostParserSettings pSettings;
    BSTD_UNUSED(arg);

    NEXUS_Module_Lock(g_NEXUS_frontendModuleSettings.transport);
    NEXUS_Tbg_GetHostParserSettings_priv(&pSettings);
    NEXUS_Module_Unlock(g_NEXUS_frontendModuleSettings.transport);

    if (pSettings.enabled==false) {
        return;
    }

    for (i=0; i<NEXUS_NUM_PARSER_BANDS; i++) {
        if (!pSettings.band[i].changed ||
            !g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[i].connected ||
            g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[i].deviceConfig==NULL ||
            g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[i].deviceConfig->pidfilter==false)
        {
            continue;
        }
        mxt = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[i].deviceConfig->mxt;
        hostPriPb = pSettings.band[i].primaryParserBandIndex;
        demodPb = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[i].demodPb;

        if (hostPriPb!=NEXUS_ParserBand_eInvalid)
        {
            BDBG_ASSERT(hostPriPb < NEXUS_NUM_PARSER_BANDS);
            demodPriPb = g_NEXUS_Frontend_P_HostMtsifConfig.hostPbSettings[hostPriPb].demodPb;
            for (pidChannel = BLST_S_FIRST(&(g_NEXUS_Frontend_P_HostMtsifConfig.mtsifPidChannels)); pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link)) {
                if (pidChannel->hostIndex==pSettings.band[i].unmappedPidChIndex) {
                    break;
                }
            }
            demodPidIndex = pidChannel ? pidChannel->demodIndex : 0;
            if (pidChannel==NULL || demodPb >= NEXUS_NUM_PARSER_BANDS || demodPriPb >= NEXUS_NUM_PARSER_BANDS) {
                BDBG_WRN(("TBG: Invalid parser setting"));
                continue;
            }
            BMXT_Tbg_GetGlobalConfig(mxt, &tbgGlobalConfig);
            tbgGlobalConfig.markerTag = pSettings.markerTag;
            BMXT_Tbg_SetGlobalConfig(mxt, &tbgGlobalConfig);

            BDBG_MSG(("TBG: demodPB%2u pri%2u <- hostPB%2u pri%2u (pid %u:%u)", demodPb, demodPriPb, i, pSettings.band[i].primaryParserBandIndex, demodPidIndex, pidChannel->hostIndex));
            BMXT_Tbg_GetParserConfig(mxt, demodPb, &tbgParserConfig);
            tbgParserConfig.enable = true;
            tbgParserConfig.primaryBandNum = demodPriPb;
            tbgParserConfig.unmappedPidChNum = demodPidIndex;
            tbgParserConfig.unmappedMarkerPktAcceptEn = true;
            tbgParserConfig.btpGenDis = true;
            BMXT_Tbg_SetParserConfig(mxt, demodPb, &tbgParserConfig);
        }
        else {
            BDBG_MSG(("TBG: demodPB%2u disable", demodPb));
            BMXT_Tbg_GetParserConfig(mxt, demodPb, &tbgParserConfig);
            tbgParserConfig.enable = false;
            tbgParserConfig.unmappedMarkerPktAcceptEn = false;
            tbgParserConfig.btpGenDis = false;
            BMXT_Tbg_SetParserConfig(mxt, demodPb, &tbgParserConfig);
        }
    }
}
#endif
