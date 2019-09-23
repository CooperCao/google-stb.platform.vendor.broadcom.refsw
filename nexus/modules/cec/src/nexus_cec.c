/***************************************************************************
* Copyright (C) 2018 Broadcom.
* The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to
* the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied),
* right to use, or waiver of any kind with respect to the Software, and
* Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
* THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
* IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
* RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
* IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
* A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
* ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
* THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
* INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
* RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
* HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
* EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
* WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
* FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* Module Description:
*
***************************************************************************/

#include "nexus_cec_module.h"
#include "nexus_cec.h"
#include "priv/nexus_core.h"

#include "priv/nexus_hdmi_output_priv.h"


BDBG_MODULE(nexus_cec);

static NEXUS_Cec g_cec;
static void NEXUS_Cec_P_AllocateLogicalAddress(NEXUS_CecHandle handle);
static void NEXUS_Cec_P_UpdatePhysicalAddress(void *pContext);

static struct {
    NEXUS_HdmiOutputHandle hdmiOutput; /* never dereferenced or used, only compared */
    bool connected; /* Tx phy is connected */
    NEXUS_HdmiOutputEdidRxHdmiVsdb vsdb;
} g_NEXUS_cecCache;

void NEXUS_CecModule_Print(void)
{
#if BDBG_DEBUG_BUILD
	BDBG_LOG(("CEC: %s, protocolVer=%#x",g_cec.opened ? "opened" : "closed", g_cec.status.cecVersion));
    if(g_cec.status.logicalAddress == 0xFF ) {
        BDBG_LOG((" logicalAddr=Unassigned, physicalAddr=%#x%x, deviceType=%d", g_cec.status.physicalAddress[0], g_cec.status.physicalAddress[1], g_cec.status.deviceType));
    }
    else {
        BDBG_LOG((" logicalAddr=%#x, physicalAddr=%#x%x, deviceType=%d", g_cec.status.logicalAddress, g_cec.status.physicalAddress[0], g_cec.status.physicalAddress[1], g_cec.status.deviceType));
    }
#endif
}

void NEXUS_Cec_P_Shutdown(void)
{
	if ( g_cec.opened )
	{
		BDBG_ERR(("Force closing CEC interface"));
		NEXUS_Cec_Close(&g_cec);
	}
}


void NEXUS_Cec_P_TransmittedCallback(void *pContext)
{
    NEXUS_CecHandle handle = (NEXUS_CecHandle)pContext;
    BCEC_MessageStatus stMessageStatus;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

    BCEC_GetTransmitMessageStatus(handle->cecHandle, &stMessageStatus);
    if ((!handle->cecSettings.disableLogicalAddressPolling)
    &&  (handle->searchState < NEXUS_CecLogicalAddrSearch_eReady))
    {
        if (!stMessageStatus.uiStatus)  /* NACK */
            handle->searchState = NEXUS_CecLogicalAddrSearch_eReportAddrFound;
        else                            /*  ACK */
        {
            if (handle->searchState == NEXUS_CecLogicalAddrSearch_eReportAddrFound)
                handle->searchState = NEXUS_CecLogicalAddrSearch_eReady ;
            else
                handle->searchState = NEXUS_CecLogicalAddrSearch_eNext;
        }
        NEXUS_Cec_P_AllocateLogicalAddress(handle);
    }
    else
    {
        NEXUS_TaskCallback_Fire(handle->messageTransmittedCallback);
    }
    handle->status.messageTransmitPending = false;
}


void NEXUS_Cec_P_ReceivedCallback(void *pContext)
{
	NEXUS_CecHandle handle = (NEXUS_CecHandle)pContext;
	BCEC_MessageStatus stMessageStatus;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

	BCEC_GetReceivedMessageStatus(handle->cecHandle, &stMessageStatus);
	handle->status.messageReceived = true;
	NEXUS_TaskCallback_Fire(handle->messageReceivedCallback);
	return;
}


/**
Summary:
Open a CEC interface
**/
NEXUS_CecHandle NEXUS_Cec_Open( /* attr{destructor=NEXUS_Cec_Close} */
	unsigned index,
	const NEXUS_CecSettings *pSettings
)
{
	BERR_Code errCode;
	NEXUS_Cec *pCec;
	BKNI_EventHandle cecTransmittedEvent;
	BKNI_EventHandle cecReceivedEvent;
	BCEC_Dependencies stCecDependencies;


	if (index > 0) {
		/* only one for now */
		BERR_TRACE(NEXUS_NOT_AVAILABLE);
		return NULL;
	}

	pCec = &g_cec;

	if ( pCec->opened )
	{
		BDBG_ERR(("CEC interface already opened"));
		return NULL;
	}

	NEXUS_OBJECT_INIT(NEXUS_Cec, pCec);

	pCec->messageTransmittedCallback = NEXUS_TaskCallback_Create(pCec, NULL);
	pCec->messageReceivedCallback = NEXUS_TaskCallback_Create(pCec, NULL);
	pCec->logicalAddressAcquiredCallback = NEXUS_TaskCallback_Create(pCec, NULL);

	/* must init CEC before allows CEC callback */
	pCec->logAddrSearchIndex = 0;
	pCec->status.physicalAddress[0] = 0xFF;
	pCec->status.physicalAddress[1] = 0xFF;
	pCec->status.logicalAddress = BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR;

	/* save dependencies */
	stCecDependencies.hChip = g_pCoreHandles->chp;
	stCecDependencies.hRegister = g_pCoreHandles->reg;
	stCecDependencies.hInterrupt = g_pCoreHandles->bint;
	stCecDependencies.hTmr = g_pCoreHandles->tmr;
	stCecDependencies.eCecHdmiPath = (BCEC_Hdmi) pSettings->cecController;

	errCode = BCEC_Open(&pCec->cecHandle, &stCecDependencies);
	if ( errCode )
	{
		errCode = BERR_TRACE(errCode);
		goto err_cec;
	}

	/* Transmit event */
	BCEC_GetEventHandle(pCec->cecHandle, BCEC_EventCec_eTransmitted, &cecTransmittedEvent) ;
	pCec->cecTransmittedEventCallback = NEXUS_RegisterEvent(cecTransmittedEvent, NEXUS_Cec_P_TransmittedCallback, pCec);
	if (!pCec->cecTransmittedEventCallback)
		goto err_cec;

	/* Receive event */
	BCEC_GetEventHandle(pCec->cecHandle, BCEC_EventCec_eReceived, &cecReceivedEvent) ;
	pCec->cecReceivedEventCallback = NEXUS_RegisterEvent(cecReceivedEvent, NEXUS_Cec_P_ReceivedCallback, pCec);
	if (!pCec->cecReceivedEventCallback)
		goto err_cec;

    errCode = BKNI_CreateEvent(&pCec->cecHotplugEvent);
    if (errCode != BERR_SUCCESS) {
        BERR_TRACE(errCode);
        goto err_cec;
    }
    pCec->cecHotplugEventCallback = NEXUS_RegisterEvent(pCec->cecHotplugEvent,
            NEXUS_Cec_P_UpdatePhysicalAddress, pCec);
    if (!pCec->cecHotplugEventCallback) {
        BERR_TRACE(NEXUS_UNKNOWN);
        goto err_cec;
    }

	/* Set appropriate CEC settings */
	errCode = NEXUS_Cec_SetSettings(pCec, pSettings);
	if (errCode)
		goto err_cec;


	pCec->opened = true;
	return pCec;


err_cec:

	NEXUS_Cec_Close(pCec);
	return NULL;
}


/**
Summary:
Close the Cec interface
**/
static void NEXUS_Cec_P_Finalizer(	NEXUS_CecHandle handle)
{
	NEXUS_Error rc = NEXUS_SUCCESS;
	NEXUS_OBJECT_ASSERT(NEXUS_Cec, handle);

    if (handle->cecHotplugEventCallback) {
        NEXUS_UnregisterEvent(handle->cecHotplugEventCallback);
    }
    if (handle->cecHotplugEvent) {
        BKNI_EventHandle event = handle->cecHotplugEvent;
        handle->cecHotplugEvent = NULL;
        BKNI_DestroyEvent(event);
    }
	if ( handle->cecTransmittedEventCallback ) {
		NEXUS_UnregisterEvent(handle->cecTransmittedEventCallback);
	}

	if ( handle->cecReceivedEventCallback ) {
		NEXUS_UnregisterEvent(handle->cecReceivedEventCallback);
	}

	rc = NEXUS_Cec_SetHdmiOutput(handle, NULL);
	if (rc) { BERR_TRACE(rc); }

	if (handle->cecHandle) {
		BCEC_Close(handle->cecHandle);
	}
	if (handle->messageTransmittedCallback) {
		NEXUS_TaskCallback_Destroy(handle->messageTransmittedCallback);
	}
	if (handle->messageReceivedCallback) {
		NEXUS_TaskCallback_Destroy(handle->messageReceivedCallback);
	}
	if (handle->logicalAddressAcquiredCallback) {
		NEXUS_TaskCallback_Destroy(handle->logicalAddressAcquiredCallback);
	}

	/* Wipe object w/ non-zero values. easier to see it core dumps. */
	NEXUS_OBJECT_DESTROY(NEXUS_Cec, handle);
	handle->opened = false;

	return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Cec, NEXUS_Cec_Close);

void NEXUS_Cec_GetDefaultSettings(
	NEXUS_CecSettings *pSettings /* [out] */
)
{
	BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->enabled = false;
    pSettings->disableLogicalAddressPolling = false;
    pSettings->logicalAddress = 0xFF;
	pSettings->physicalAddress[0]=0xFF;
	pSettings->physicalAddress[1]=0xFF;
	pSettings->deviceType = NEXUS_CecDeviceType_eTuner;
	pSettings->cecController = NEXUS_CecController_eTx;
	NEXUS_CallbackDesc_Init(&pSettings->messageTransmittedCallback);
	NEXUS_CallbackDesc_Init(&pSettings->messageReceivedCallback);
	NEXUS_CallbackDesc_Init(&pSettings->logicalAddressAcquiredCallback);

	return;
}


void NEXUS_Cec_GetSettings(
	NEXUS_CecHandle handle,
	NEXUS_CecSettings *pSettings /* [out] */
)
{
	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);
	*pSettings = handle->cecSettings;
}


NEXUS_Error NEXUS_Cec_SetSettings(
	NEXUS_CecHandle handle,
	const NEXUS_CecSettings *pSettings
)
{
	NEXUS_Error rc = 0;
	BCEC_Settings stCecSettings;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

    if ((pSettings->physicalAddress[0] != 0xFF)
    ||  (pSettings->physicalAddress[1] != 0xFF))
    {
        BDBG_WRN(("NEXUS_CecSettings.physicalAddress is now ignored. Using internally obtained value.")) ;
    }

    BKNI_EnterCriticalSection();
        /* use our global storage. don't call into HdmiOutput because it might be in standby. */
        if (!g_NEXUS_cecCache.connected)
        {
            BDBG_LOG(("No Rx device attached - physicalAddress not updated"));
            handle->status.physicalAddress[0] = 0xFF;
            handle->status.physicalAddress[1] = 0xFF;
        }
        else {
            handle->status.physicalAddress[0]= (g_NEXUS_cecCache.vsdb.physicalAddressA << 4) | g_NEXUS_cecCache.vsdb.physicalAddressB;
            handle->status.physicalAddress[1]= (g_NEXUS_cecCache.vsdb.physicalAddressC << 4) | g_NEXUS_cecCache.vsdb.physicalAddressD;
        }
    BKNI_LeaveCriticalSection();
	handle->status.deviceType = pSettings->deviceType;

    /* Disable polling for logical address NEXUS. Save provided logical address */
    if (pSettings->disableLogicalAddressPolling)
    {
        handle->status.logicalAddress = pSettings->logicalAddress;
    }
    else
    {
        if (pSettings->enabled
            && (handle->cecSettings.enabled != pSettings->enabled
             || handle->status.logicalAddress == BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR))
        {
            handle->searchState = NEXUS_CecLogicalAddrSearch_eInit;
            NEXUS_Cec_P_AllocateLogicalAddress(handle);
        }
    }

    BCEC_GetSettings(handle->cecHandle, &stCecSettings);
        stCecSettings.enable = pSettings->enabled;
        stCecSettings.eCecHdmiPath = (BCEC_Hdmi) pSettings->cecController;
        stCecSettings.CecLogicalAddr = handle->status.logicalAddress;
        BKNI_Memcpy(stCecSettings.CecPhysicalAddr, handle->status.physicalAddress,
            sizeof(handle->status.physicalAddress));
        stCecSettings.eDeviceType = handle->status.deviceType;
    rc = BCEC_SetSettings(handle->cecHandle, &stCecSettings);
    if (rc) return BERR_TRACE(rc);

	/* Set callbacks */
	NEXUS_TaskCallback_Set(handle->messageTransmittedCallback, &pSettings->messageTransmittedCallback);
	NEXUS_TaskCallback_Set(handle->messageReceivedCallback, &pSettings->messageReceivedCallback);
	NEXUS_TaskCallback_Set(handle->logicalAddressAcquiredCallback, &pSettings->logicalAddressAcquiredCallback);

    if (pSettings != &handle->cecSettings) {
        handle->cecSettings = *pSettings;
    }
	return rc;
}


NEXUS_Error NEXUS_Cec_GetStatus(
	NEXUS_CecHandle handle,
	NEXUS_CecStatus *pStatus /* [out] */
)
{
	NEXUS_Error rc = BERR_SUCCESS;
	BCEC_Settings stCecSettings;
	BCEC_MessageStatus stMessageStatus;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

	BCEC_GetSettings(handle->cecHandle, &stCecSettings);
	/* device logical address has not yet been established */
	if (stCecSettings.CecLogicalAddr == BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR)
		handle->status.ready = false;
	else
		handle->status.ready = true;


	/* Get device logical and physical address */
	/* logical and physical address */
	handle->status.logicalAddress = stCecSettings.CecLogicalAddr;
	BKNI_Memcpy(handle->status.physicalAddress, stCecSettings.CecPhysicalAddr,
			 sizeof(stCecSettings.CecPhysicalAddr));

	/* Device type */
	handle->status.deviceType = (NEXUS_CecDeviceType) stCecSettings.eDeviceType;

	/* CEC Version */
	handle->status.cecVersion = stCecSettings.cecVersion;

	/* message status */
	BCEC_GetTransmitMessageStatus(handle->cecHandle, &stMessageStatus);
	handle->status.transmitMessageAcknowledged =
							(stMessageStatus.uiStatus !=0);


	/* return status */
	*pStatus = handle->status;
	return rc;
}


NEXUS_Error NEXUS_Cec_ReceiveMessage(
	NEXUS_CecHandle handle,
	NEXUS_CecReceivedMessage *pReceivedMessage /* [out] */
)
{
	BAVC_HDMI_CEC_MessageData stMessageData;
	BCEC_MessageStatus stMessageStatus;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

	/* reset status */
	handle->status.messageReceived = false;

	BCEC_GetReceivedMessage(handle->cecHandle, &stMessageData);

	pReceivedMessage->data.initiatorAddr = stMessageData.initiatorAddr;
	pReceivedMessage->data.destinationAddr = stMessageData.destinationAddr;
	pReceivedMessage->data.length = stMessageData.messageLength;
	BKNI_Memcpy(&pReceivedMessage->data.buffer, &stMessageData.messageBuffer,
			(sizeof(uint8_t) * stMessageData.messageLength));

	/* message status */
	BCEC_GetReceivedMessageStatus(handle->cecHandle, &stMessageStatus);
	pReceivedMessage->receivedStatus.receivedMessageAcknowledged = (stMessageStatus.uiStatus !=0);
	pReceivedMessage->receivedStatus.endOfMessage = (stMessageStatus.uiEOM !=0);


	/* Re-enable the CEC core to receive the next incoming CEC message */
	BCEC_EnableReceive(handle->cecHandle);

	return NEXUS_SUCCESS;
}


void NEXUS_Cec_GetDefaultMessageData(
	NEXUS_CecMessage *pMessage /* [out] */
)
{
	BKNI_Memset(pMessage, 0, sizeof(NEXUS_CecMessage)) ;
	return;
}


NEXUS_Error NEXUS_Cec_TransmitMessage(
	NEXUS_CecHandle handle,
	const NEXUS_CecMessage *pMessage
)
{
	NEXUS_Error rc = NEXUS_SUCCESS;
	BAVC_HDMI_CEC_MessageData stCecMessageData;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

    if (handle->status.messageTransmitPending)
    {
        rc = NEXUS_NOT_AVAILABLE;
        goto done;
    }

    /* polling message request */
    if (handle->cecSettings.disableLogicalAddressPolling
    && (pMessage->initiatorAddr == pMessage->destinationAddr))
    {
        rc = BCEC_PingLogicalAddr(handle->cecHandle, pMessage->initiatorAddr);
        goto done;
    }

	/* prepare CEC message data */
	stCecMessageData.initiatorAddr = pMessage->initiatorAddr;
	stCecMessageData.destinationAddr = pMessage->destinationAddr;
	stCecMessageData.messageLength = pMessage->length;
	BKNI_Memcpy(stCecMessageData.messageBuffer, pMessage->buffer,
			sizeof(pMessage->buffer));

	/* transmit message */
	rc = BCEC_XmitMessage(handle->cecHandle, &stCecMessageData);

done:
    /* set Pending if message successfully sent */
    if (!rc)
        handle->status.messageTransmitPending = true;

    return rc;
}


static NEXUS_Error NEXUS_Cec_P_Send_ReportPhysicalAddress(NEXUS_CecHandle handle)
{

	BERR_Code rc ;
	NEXUS_CecMessage stMessage ;

	/**********************************
		CEC Message Buffer consists of:
			hexOpCode
			device physical address
			device type
	***********************************/

	/* CEC message opcode = 0x84 */
	stMessage.buffer[0] = BCEC_OpCode_ReportPhysicalAddress;

	/* [Device Physical Address] */
	stMessage.buffer[1] = handle->status.physicalAddress[0] ;
	stMessage.buffer[2] = handle->status.physicalAddress[1] ;

	/* Device Type */
	stMessage.buffer[3] = handle->cecSettings.deviceType ;

	/* Broadcast CEC message */
	stMessage.initiatorAddr = handle->cecSettings.logicalAddress ;
	stMessage.destinationAddr = BCEC_BROADCAST_ADDR ;
	stMessage.length = 4 ;

	rc = NEXUS_Cec_TransmitMessage(handle, &stMessage) ;
	return rc ;
}


static void NEXUS_Cec_P_UpdatePhysicalAddress(void *pContext)
{
    NEXUS_CecHandle hCEC = pContext;
    if (hCEC->opened) {
        NEXUS_Error rc = NEXUS_Cec_SetSettings(hCEC, &hCEC->cecSettings);
        if (rc) { BERR_TRACE(rc);}
    }
}

void NEXUS_Cec_P_StorePhysicalAddress_isr(NEXUS_HdmiOutputHandle hdmiOutput, const NEXUS_HdmiOutputEdidRxHdmiVsdb *vsdb)
{
    /* Store CEC phy address if HDMI had been opened */
    if (g_NEXUS_cecCache.hdmiOutput && g_NEXUS_cecCache.hdmiOutput != hdmiOutput)
		return;

    if (vsdb) {
        g_NEXUS_cecCache.connected = true;
        g_NEXUS_cecCache.vsdb = *vsdb;
    }
    else {
        g_NEXUS_cecCache.connected = false;
    }
    if (g_cec.opened && g_cec.cecHotplugEvent) {
        BKNI_SetEvent(g_cec.cecHotplugEvent);
    }
}

NEXUS_Error NEXUS_Cec_SetHdmiOutput(
    NEXUS_CecHandle handle,
    NEXUS_HdmiOutputHandle hdmiOutput /* attr{null_allowed=y} Pass NULL to remove/disconnected the link */
)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);
    /* don't need to ACQUIRE/RELEASE hdmiOutput because we will only compare the
    handle value after returning from this function */
    g_NEXUS_cecCache.hdmiOutput = hdmiOutput;
    return NEXUS_SUCCESS;
}

static const uint8_t g_logicalAddrArray[] =
{
	BAVC_HDMI_CEC_StbDevices_eSTB1,
	BAVC_HDMI_CEC_StbDevices_eSTB2,
	BAVC_HDMI_CEC_StbDevices_eSTB3,
	BAVC_HDMI_CEC_StbDevices_eSTB4
};


static void NEXUS_Cec_P_AllocateLogicalAddress(NEXUS_CecHandle handle)
{
	NEXUS_Error rc;
	uint8_t addr, lastIndex;
	BCEC_Settings stCecSettings;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

	lastIndex = sizeof(g_logicalAddrArray)/sizeof(g_logicalAddrArray[0]) - 1;
	switch (handle->searchState)
	{

	default:
	case NEXUS_CecLogicalAddrSearch_eInit:	/* ping first Address */

		handle->logAddrStartIndex = handle->logAddrSearchIndex;
		addr = g_logicalAddrArray[handle->logAddrSearchIndex];
		BDBG_MSG(("Starting search for CEC Logical Addr: %d...", addr)) ;
		handle->status.logicalAddress = BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR;
		rc = BCEC_PingLogicalAddr(handle->cecHandle, addr);
		if (rc) rc = BERR_TRACE(rc);

		break;

	case NEXUS_CecLogicalAddrSearch_eNext:
		handle->logAddrSearchIndex++;

		/* If at the end of the list, restart from beginning to make sure we poll through
			all STB-Device-Logical-Addresses */
		if (handle->logAddrSearchIndex > lastIndex) {
			handle->logAddrSearchIndex = 0;
		}

		/* At this point, we already went through all available logical addresses for
			STB devices. Device Unregistered  */
		if (handle->logAddrSearchIndex == handle->logAddrStartIndex)
		{
			BDBG_WRN(("All CEC Addrs used; device unregistered")) ;

			/* Reset to start from logical address #1 of STB device */
			handle->logAddrSearchIndex = 0;

			/* Update state */
			handle->searchState = NEXUS_CecLogicalAddrSearch_eReady;

			BCEC_GetSettings(handle->cecHandle, &stCecSettings);
				stCecSettings.enable = true;
				stCecSettings.CecLogicalAddr = BAVC_HDMI_CEC_StbDevices_eUnRegistered;
				BKNI_Memcpy(stCecSettings.CecPhysicalAddr, handle->status.physicalAddress,
					sizeof(handle->status.physicalAddress));
				stCecSettings.eDeviceType = handle->status.deviceType;
			rc = BCEC_SetSettings(handle->cecHandle, &stCecSettings);
			if (rc)
			{
				rc = BERR_TRACE(rc);
				return;
			}

			BCEC_EnableReceive(handle->cecHandle);

			NEXUS_TaskCallback_Fire(handle->logicalAddressAcquiredCallback);
		}
		else {
			addr = g_logicalAddrArray[handle->logAddrSearchIndex];
			BDBG_MSG(("Continuing search for CEC Logical Addr: %d...", addr)) ;
			rc = BCEC_PingLogicalAddr(handle->cecHandle, addr);
			if (rc) rc = BERR_TRACE(rc);
		}
		break;


	case NEXUS_CecLogicalAddrSearch_eReportAddrFound :

		handle->status.logicalAddress = g_logicalAddrArray[handle->logAddrSearchIndex];

		BDBG_MSG(("Report CEC Logical Addr: %d", handle->status.logicalAddress)) ;

		/* Get cec Settings. Ignore uninitialized error due to uninitialize Logical Address */
		BCEC_GetSettings(handle->cecHandle, &stCecSettings);

			stCecSettings.enable = true;
			stCecSettings.CecLogicalAddr = handle->status.logicalAddress;
			BKNI_Memcpy(stCecSettings.CecPhysicalAddr, handle->status.physicalAddress,
				sizeof(handle->status.physicalAddress));
			stCecSettings.eDeviceType = handle->status.deviceType;

		/* Apply logical address, physical Address and other cec settings */
		rc = BCEC_SetSettings(handle->cecHandle, &stCecSettings);
		if (rc)
		{
			rc = BERR_TRACE(rc);
			return;
		}
		else
		{
			/* Report Physical Address */
			rc = NEXUS_Cec_P_Send_ReportPhysicalAddress(handle) ;
			if (rc) rc = BERR_TRACE(rc);

			/* always enable receive after CEC msg is processed */
			BCEC_EnableReceive(handle->cecHandle);
		}

		break;

	case NEXUS_CecLogicalAddrSearch_eReady:
		NEXUS_TaskCallback_Fire(handle->logicalAddressAcquiredCallback);
		break ;
	}
}

NEXUS_Error NEXUS_CecModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Cec *pCec = &g_cec;
    BCEC_Settings cecSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pSettings);

    if(pCec->opened) {
	if(enabled) {
	    BCEC_GetSettings(pCec->cecHandle, &cecSettings);
	    cecSettings.enableAutoOn = true;
	    BCEC_SetSettings(pCec->cecHandle, &cecSettings);
	    rc = BCEC_Standby(pCec->cecHandle, NULL);
	} else {
	    rc = BCEC_Resume(pCec->cecHandle);
	    BCEC_GetSettings(pCec->cecHandle, &cecSettings);
	    cecSettings.enableAutoOn = false;
	    BCEC_SetSettings(pCec->cecHandle, &cecSettings);
	}
    }

    return rc;

#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}
