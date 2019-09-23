/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_hdmi_output_module.h"
#include "bhdm_auto_i2c.h"
#if NEXUS_HAS_HDMI_INPUT
#include "priv/nexus_hdmi_input_priv.h"
#endif

BDBG_MODULE(nexus_hdmi_output_hotplug);

static void NEXUS_HdmiOutput_P_RxSenseTimerExpired(void *pContext);
static void NEXUS_HdmiOutput_P_RxPowerTimerExpired(void *pContext);

/* called when rxState goes to eDisconnected, or to ePoweredOn/Down */
static void NEXUS_HdmiOutput_P_FireHotplugCallbacks(NEXUS_HdmiOutputHandle hdmiOutput)
{
    /* notify HDR module */
    NEXUS_HdmiOutput_Dynrng_P_ConnectionChanged(hdmiOutput);

    /* notify application of hotplug status change */
    NEXUS_TaskCallback_Fire(hdmiOutput->hotplugCallback);
}

static void NEXUS_HdmiOutput_P_SetRxState(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_HdmiOutputState state)
{
    /* do not change any other state in this function. it's only meant to assign the rxState variable and print the change. */
#if BDBG_DEBUG_BUILD
    const char *g_rxStateStr[] = {"disconnected", "rxsense_check", "powered_down", "powered_on"};
    BDBG_LOG(("%s -> %s", g_rxStateStr[hdmiOutput->rxState], g_rxStateStr[state]));
#endif
    hdmiOutput->rxState = state;
}

void NEXUS_HdmiOutput_P_StopTimers(NEXUS_HdmiOutputHandle hdmiOutput )
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdmiOutput, hdmiOutput);
    if (hdmiOutput->rxSenseCheckTimer) {
        NEXUS_CancelTimer(hdmiOutput->rxSenseCheckTimer) ;
        hdmiOutput->rxSenseCheckTimer = NULL ;
    }
    if (hdmiOutput->powerTimer) {
        NEXUS_CancelTimer(hdmiOutput->powerTimer) ;
        hdmiOutput->powerTimer = NULL ;
    }
    if (hdmiOutput->postFormatChangeTimer) {
        NEXUS_CancelTimer(hdmiOutput->postFormatChangeTimer);
        hdmiOutput->postFormatChangeTimer = NULL;
    }
}

/* transition from any rxState to eDisconnected */
void NEXUS_HdmiOutput_P_SetDisconnectedState(NEXUS_HdmiOutputHandle hdmiOutput)
{
    if (hdmiOutput->rxState == NEXUS_HdmiOutputState_eDisconnected) {
        return;
    }

    NEXUS_HdmiOutput_P_StopTimers(hdmiOutput);

    /* reset cached edid settings */
    hdmiOutput->edidHdmiDevice = false ;
    BKNI_Memset(&hdmiOutput->edidVendorSpecificDB, 0, sizeof(BHDM_EDID_RxVendorSpecificDB)) ;

#if BHDM_HAS_HDMI_20_SUPPORT
    /* make sure all Auto I2c channels are disabled */
    BKNI_EnterCriticalSection() ;
        BHDM_AUTO_I2C_SetChannels_isr(hdmiOutput->hdmHandle, false) ;
    BKNI_LeaveCriticalSection() ;

    BHDM_SCDC_DisableScrambleTx(hdmiOutput->hdmHandle) ;
#endif

    NEXUS_HdmiOutput_P_SetTmdsSignalClock(hdmiOutput, false);
    NEXUS_HdmiOutput_P_SetTmdsSignalData(hdmiOutput, false);

#if NEXUS_HAS_HDMI_INPUT
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    /* Update hdcp2.x RxCaps settings */
    if (hdmiOutput->hdmiInput)
    {
        BERR_Code errCode;
        NEXUS_Hdcp2xReceiverIdListData stReceiverIdListData;

        /* Update RxCaps */
        NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
        errCode = NEXUS_HdmiInput_UpdateHdcp2xRxCaps_priv(hdmiOutput->hdmiInput, false);
        NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
        if (errCode != NEXUS_SUCCESS)
        {
            errCode = BERR_TRACE(errCode); /* keep going */
        }

        BKNI_Memset(&stReceiverIdListData, 0, sizeof(NEXUS_Hdcp2xReceiverIdListData));
        /* Clear outdated receiverIdList  */
        NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
        errCode = NEXUS_HdmiInput_LoadHdcp2xReceiverIdList_priv(hdmiOutput->hdmiInput,
         &stReceiverIdListData, false);
        NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
        if (errCode != BERR_SUCCESS)
        {
            errCode = BERR_TRACE(errCode); /* keep going */
        }
    }
#endif
#endif

#if NEXUS_HAS_SECURITY
    {
        BERR_Code errCode;
        errCode = NEXUS_HdmiOutput_P_DisableHdcpAuthentication(hdmiOutput);
        if (errCode) { errCode = BERR_TRACE(errCode) ; }
    }
#endif

    hdmiOutput->displaySettings.valid = false;

    /* notify Nexus Display of the cable removal to disable the HDMI Output */
    NEXUS_HdmiOutput_P_NotifyDisplay(hdmiOutput);

    NEXUS_HdmiOutput_P_SetRxState(hdmiOutput, NEXUS_HdmiOutputState_eDisconnected);
    NEXUS_HdmiOutput_P_HdcpPowerDown(hdmiOutput);
    /* if HdmiOutput disconnects from Display, don't fire callbacks */
    if (hdmiOutput->notifyDisplayEvent) {
        NEXUS_HdmiOutput_P_FireHotplugCallbacks(hdmiOutput);
    }
}

/*
read deviceAttached and rxSense HW state, but first process any forceDisconnect_isr SW state by moving into eDisconnected.
*/
void NEXUS_HdmiOutput_P_ProcessRxState(NEXUS_HdmiOutputHandle hdmiOutput, bool *deviceAttached, bool *rxSense)
{
    BKNI_EnterCriticalSection();
    if (hdmiOutput->forceDisconnect_isr) {
        hdmiOutput->forceDisconnect_isr = false;
        BKNI_LeaveCriticalSection();
        /* if we were disconnected at ISR time, we must go into eDisconnected state.
        However, afterwards we can check and possibly come out right away. */
        NEXUS_HdmiOutput_P_SetDisconnectedState(hdmiOutput);
    }
    else {
        BKNI_LeaveCriticalSection();
    }

    /* This is the only place where NEXUS_HdmiOutput determines deviceAttached and rxSense. */
    BHDM_GetReceiverSense(hdmiOutput->hdmHandle, deviceAttached, rxSense);
    if ( hdmiOutput->hdmSettings.bCrcTestMode ) {
        *deviceAttached = true;
        *rxSense        = true;
        BDBG_MSG(("CRC test mode, override deviceAttached & rxSense " ));
    }
}

/* transition from any rxState to eRxSenseCheck, ePoweredOn or ePoweredDown based on rxSense and current state */
static void NEXUS_HdmiOutput_P_SetConnectedState(NEXUS_HdmiOutputHandle hdmiOutput, bool rxSense)
{
    if (hdmiOutput->rxSenseCheckTimer)
    {
		NEXUS_CancelTimer(hdmiOutput->rxSenseCheckTimer) ;
		hdmiOutput->rxSenseCheckTimer = NULL ;
    }

    if (hdmiOutput->powerTimer)
    {
		NEXUS_CancelTimer(hdmiOutput->powerTimer) ;
		hdmiOutput->powerTimer = NULL ;
    }

    if (!rxSense && (hdmiOutput->rxState == NEXUS_HdmiOutputState_eDisconnected ||
            (hdmiOutput->rxState == NEXUS_HdmiOutputState_eRxSenseCheck && hdmiOutput->checkRxSenseCount < hdmiOutput->openSettings.maxRxSenseRetries)))
    {
        if (hdmiOutput->rxState == NEXUS_HdmiOutputState_eDisconnected) {
            NEXUS_HdmiOutput_P_SetRxState(hdmiOutput, NEXUS_HdmiOutputState_eRxSenseCheck);
        }
        hdmiOutput->rxSenseCheckTimer = NEXUS_ScheduleTimer(hdmiOutput->openSettings.rxSenseInterval,
            NEXUS_HdmiOutput_P_RxSenseTimerExpired, hdmiOutput) ;
        if (!hdmiOutput->rxSenseCheckTimer) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            /* this is a required timer, so we have to disconnect and exit */
            NEXUS_HdmiOutput_P_SetDisconnectedState(hdmiOutput) ;
            return;
        }
        hdmiOutput->checkRxSenseCount++ ;
        BDBG_MSG(("Check for RxSense try %2d of %2d; Elapsed time: %4dms",
            hdmiOutput->checkRxSenseCount, hdmiOutput->openSettings.maxRxSenseRetries,
            hdmiOutput->checkRxSenseCount * hdmiOutput->openSettings.rxSenseInterval)) ;
    }
    else {
        /* move to ePoweredOn or ePoweredDown */
        if (rxSense && hdmiOutput->rxState != NEXUS_HdmiOutputState_ePoweredOn) {
            /* Rx detected to be ON...
                make sure all TMDS data lines are ENABLED if under nexus control */
            NEXUS_HdmiOutput_P_SetTmdsSignalData(hdmiOutput, true);
            NEXUS_HdmiOutput_P_SetRxState(hdmiOutput, NEXUS_HdmiOutputState_ePoweredOn);
            /* notify Nexus Display of the cable insertion to re-enable HDMI Output */
            hdmiOutput->displaySettings.valid = false ;
            NEXUS_HdmiOutput_P_NotifyDisplay(hdmiOutput);

            if (hdmiOutput->notifyAudioEvent)
            {
                 BKNI_SetEvent(hdmiOutput->notifyAudioEvent);
            }

            NEXUS_HdmiOutput_P_CheckHdcpVersion(hdmiOutput);

#if NEXUS_HAS_HDMI_INPUT
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
            if (hdmiOutput->hdmiInput && (hdmiOutput->eHdcpVersion == BHDM_HDCP_Version_e2_2)) {
                NEXUS_Error rc;
                NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
                rc = NEXUS_HdmiInput_UpdateHdcp2xRxCaps_priv(hdmiOutput->hdmiInput, true);
                NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
                if (rc != NEXUS_SUCCESS) BERR_TRACE(rc); /* keep going */
            }
#endif
#endif
            NEXUS_HdmiOutput_P_FireHotplugCallbacks(hdmiOutput);
        }
        else if (!rxSense && hdmiOutput->rxState != NEXUS_HdmiOutputState_ePoweredDown) {
            NEXUS_HdmiOutput_P_SetRxState(hdmiOutput, NEXUS_HdmiOutputState_ePoweredDown);
            NEXUS_HdmiOutput_P_HdcpPowerDown(hdmiOutput);
            NEXUS_HdmiOutput_P_SetTmdsSignalData(hdmiOutput, false);
#if BHDM_HAS_HDMI_20_SUPPORT
            /* make sure all Auto I2c channels are disabled */
            BKNI_EnterCriticalSection() ;
                BHDM_AUTO_I2C_SetChannels_isr(hdmiOutput->hdmHandle, false) ;
            BKNI_LeaveCriticalSection() ;
#endif
            NEXUS_HdmiOutput_P_FireHotplugCallbacks(hdmiOutput);
        }

        /* enable power polling to check if/when a HDMI Rx is powered back on...
        a small number of receivers may not generate a Hotplug when powered on */
        if (hdmiOutput->openSettings.powerPollingInterval) {
            BDBG_MSG(("Start Rx power check polling...")) ;
            hdmiOutput->powerTimer = NEXUS_ScheduleTimer(hdmiOutput->openSettings.powerPollingInterval,
                NEXUS_HdmiOutput_P_RxPowerTimerExpired, hdmiOutput);
            if (!hdmiOutput->powerTimer) BERR_TRACE(NEXUS_NOT_INITIALIZED) ; /* keep going */
        }
    }

    BDBG_ASSERT(hdmiOutput->rxState != NEXUS_HdmiOutputState_eDisconnected);
}

static void NEXUS_HdmiOutput_P_RxSenseTimerExpired(void *pContext)
{
    NEXUS_HdmiOutputHandle hdmiOutput = (NEXUS_HdmiOutputHandle)pContext;
    bool deviceAttached, rxSense;

    BDBG_ASSERT(hdmiOutput->rxState == NEXUS_HdmiOutputState_eRxSenseCheck) ;
    hdmiOutput->rxSenseCheckTimer = NULL ;

    NEXUS_HdmiOutput_P_ProcessRxState(hdmiOutput, &deviceAttached, &rxSense);
    if (!deviceAttached) {
        NEXUS_HdmiOutput_P_SetDisconnectedState(hdmiOutput);
    }
    else {
        NEXUS_HdmiOutput_P_SetConnectedState(hdmiOutput, rxSense);
    }
}

static BERR_Code NEXUS_HdmiOutput_P_ReadEdid(NEXUS_HdmiOutputHandle hdmiOutput)
{

    BERR_Code errCode ;
    BHDM_Status hdmiStatus ;
    unsigned int i = 1 ;

    hdmiOutput->invalidEdidReported = false ;

    do
    {
        errCode = BHDM_EDID_Initialize(hdmiOutput->hdmHandle);
        if (errCode == BERR_SUCCESS)
        {
            break ;
        }

        /* Rx Device has been removed .  Abort */
        if (errCode == BHDM_NO_RX_DEVICE)
        {
            BDBG_WRN(("Device Disconnected while trying to read EDID")) ;
            goto done ;
        }

        if (errCode == BERR_TIMEOUT) {
            goto done;
        }

        /*
        this delay should really not be needed, but allow some time
        between EDID read attempts in case the Rx is still settling
        */
        BKNI_Sleep(100) ;
        if (i < hdmiOutput->openSettings.maxEdidRetries)
        {
            BDBG_WRN(("Error code %d reading EDID...  Retry %d of %d",
                errCode, i + 1, hdmiOutput->openSettings.maxEdidRetries)) ;
        }
    } while ( i++ <  hdmiOutput->openSettings.maxEdidRetries ) ;

    if ( i >= hdmiOutput->openSettings.maxEdidRetries )
    {
        BDBG_ERR(("Unable to read EDID after %d attempts", hdmiOutput->openSettings.maxEdidRetries ));
        goto done ;
    }

    /* cache EDID settings */
    errCode = BHDM_EDID_IsRxDeviceHdmi(hdmiOutput->hdmHandle,
        &hdmiOutput->edidVendorSpecificDB, &hdmiOutput->edidHdmiDevice) ;
    if (errCode)
    {
        BDBG_ERR(("Error determining Rx Device type")) ;
        goto done ;
    }

    NEXUS_HdmiOutput_TriggerCecCallback_priv(hdmiOutput);

done:
    BHDM_GetHdmiStatus(hdmiOutput->hdmHandle, &hdmiStatus) ;
    hdmiOutput->edidState = hdmiStatus.edidState ;

    return errCode;
}

void NEXUS_HdmiOutput_P_HotplugCallback(void *pContext)
{
    NEXUS_HdmiOutputHandle hdmiOutput = (NEXUS_HdmiOutputHandle)pContext;
    bool deviceAttached, rxSense;

    BDBG_OBJECT_ASSERT(hdmiOutput, NEXUS_HdmiOutput);

    NEXUS_HdmiOutput_P_ProcessRxState(hdmiOutput, &deviceAttached, &rxSense);
    if (!deviceAttached) {
        NEXUS_HdmiOutput_P_SetDisconnectedState(hdmiOutput);
    }
    else {
        NEXUS_HdmiOutput_P_SetTmdsSignalClock(hdmiOutput, true);

        /* EDID is not dependent on RxSense; it should be available to be read now */
        (void)NEXUS_HdmiOutput_P_ReadEdid(hdmiOutput) ;

        if (hdmiOutput->rxState == NEXUS_HdmiOutputState_eRxSenseCheck) {
            BDBG_WRN(("restarting rxSenseCheck countdown"));
            hdmiOutput->checkRxSenseCount = 0;
        }
        else if (hdmiOutput->rxState == NEXUS_HdmiOutputState_eDisconnected) {
            hdmiOutput->checkRxSenseCount = 0;
        }
        NEXUS_HdmiOutput_P_SetConnectedState(hdmiOutput, rxSense);
    }
}

void NEXUS_HdmiOutput_P_HotPlug_isr(void *context, int param, void *data)
{
    NEXUS_HdmiOutputHandle hdmiOutput = context;
    bool deviceAttached =  * (bool *) data ;

    BDBG_OBJECT_ASSERT(hdmiOutput, NEXUS_HdmiOutput);
    BSTD_UNUSED(param);
    BDBG_MSG(("Hotplug deviceAttached: %d", deviceAttached)) ;

    /* forceDisconnect_isr cannot be set true until task-time processing complete */
    if (!deviceAttached) {
        hdmiOutput->forceDisconnect_isr = true;
    }
    BKNI_SetEvent_isr(hdmiOutput->notifyHotplugEvent) ;
}

static void NEXUS_HdmiOutput_P_RxPowerTimerExpired(void *pContext)
{
    NEXUS_HdmiOutputHandle hdmiOutput = pContext;
    bool deviceAttached, rxSense;

    BDBG_OBJECT_ASSERT(hdmiOutput, NEXUS_HdmiOutput);
    BDBG_ASSERT(hdmiOutput->rxState == NEXUS_HdmiOutputState_ePoweredOn || hdmiOutput->rxState == NEXUS_HdmiOutputState_ePoweredDown);
    hdmiOutput->powerTimer = NULL;

    NEXUS_HdmiOutput_P_ProcessRxState(hdmiOutput, &deviceAttached, &rxSense);
    if (!deviceAttached) {
        NEXUS_HdmiOutput_P_SetDisconnectedState(hdmiOutput);
    }
    else {
        NEXUS_HdmiOutput_P_SetConnectedState(hdmiOutput, rxSense);
    }
}
