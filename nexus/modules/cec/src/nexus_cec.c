/***************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

NEXUS_Error NEXUS_Cec_P_Shutdown(void)
{
	if ( g_cec.opened )
	{
		BDBG_ERR(("Force closing CEC interface"));
		NEXUS_Cec_Close(&g_cec);
	}

	return NEXUS_SUCCESS;
}


void NEXUS_Cec_P_TransmittedCallback(void *pContext)
{
	NEXUS_CecHandle handle = (NEXUS_CecHandle)pContext;
	BCEC_MessageStatus stMessageStatus;
	NEXUS_Error rc;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

	rc = BCEC_GetTransmitMessageStatus(handle->cecHandle, &stMessageStatus);
	if (rc) {
		BERR_TRACE(rc);
		return ;
	}

    if ((!handle->cecSettings.disableLogicalAddressPolling)
    &&  (handle->searchState < NEXUS_CecLogicalAddrSearch_eReady))
	{
		if (!stMessageStatus.uiStatus)
			handle->searchState = NEXUS_CecLogicalAddrSearch_eReady;
		else
			handle->searchState = NEXUS_CecLogicalAddrSearch_eNext;
		NEXUS_Cec_P_AllocateLogicalAddress(handle);
	}
	else
	{
		handle->status.messageTransmitPending = false;
		NEXUS_TaskCallback_Fire(handle->messageTransmittedCallback);
	}
}


void NEXUS_Cec_P_ReceivedCallback(void *pContext)
{
	NEXUS_CecHandle handle = (NEXUS_CecHandle)pContext;
	BCEC_MessageStatus stMessageStatus;
	NEXUS_Error rc;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

	rc = BCEC_GetReceivedMessageStatus(handle->cecHandle, &stMessageStatus);
	if (rc) {
		BERR_TRACE(rc);
		return ;
	}

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
	errCode = BCEC_GetEventHandle(pCec->cecHandle, BCEC_EventCec_eTransmitted, &cecTransmittedEvent) ;
	if (errCode)
		goto err_cec;
	pCec->cecTransmittedEventCallback = NEXUS_RegisterEvent(cecTransmittedEvent, NEXUS_Cec_P_TransmittedCallback, pCec);
	if (!pCec->cecTransmittedEventCallback)
		goto err_cec;

	/* Receive event */
	errCode = BCEC_GetEventHandle(pCec->cecHandle, BCEC_EventCec_eReceived, &cecReceivedEvent) ;
	if (errCode)
		goto err_cec;
	pCec->cecReceivedEventCallback = NEXUS_RegisterEvent(cecReceivedEvent, NEXUS_Cec_P_ReceivedCallback, pCec);
	if (!pCec->cecReceivedEventCallback)
		goto err_cec;


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

	/* save physical address & device type */
	BKNI_Memcpy(handle->status.physicalAddress, pSettings->physicalAddress, sizeof(pSettings->physicalAddress));
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

	handle->cecSettings = *pSettings;
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

	rc = BCEC_GetSettings(handle->cecHandle, &stCecSettings);
	/* device logical address has not yet been established */
	if (rc == BERR_NOT_INITIALIZED)
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
	rc = BCEC_GetTransmitMessageStatus(handle->cecHandle, &stMessageStatus);
	if (rc)
	{
		BDBG_ERR(("Error getting CEC message info"));
		return BERR_TRACE(rc);
	}
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
	NEXUS_Error rc = NEXUS_SUCCESS;
	BAVC_HDMI_CEC_MessageData stMessageData;
	BCEC_MessageStatus stMessageStatus;

	BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

	/* reset status */
	handle->status.messageReceived = false;

	rc = BCEC_GetReceivedMessage(handle->cecHandle, &stMessageData);
	if (rc)
	{
		BDBG_ERR(("Error receiving CEC Msg")) ;
		return BERR_TRACE(rc);
	}

	pReceivedMessage->data.initiatorAddr = stMessageData.initiatorAddr;
	pReceivedMessage->data.destinationAddr = stMessageData.destinationAddr;
	pReceivedMessage->data.length = stMessageData.messageLength;
	BKNI_Memcpy(&pReceivedMessage->data.buffer, &stMessageData.messageBuffer,
			(sizeof(uint8_t) * stMessageData.messageLength));

	/* message status */
	rc = BCEC_GetReceivedMessageStatus(handle->cecHandle, &stMessageStatus);
	if (rc)
	{
		BDBG_ERR(("Error getting CEC message info"));
		goto done;
	}
	pReceivedMessage->receivedStatus.receivedMessageAcknowledged = (stMessageStatus.uiStatus !=0);
	pReceivedMessage->receivedStatus.endOfMessage = (stMessageStatus.uiEOM !=0);


	/* Re-enable the CEC core to receive the next incoming CEC message */
	rc = BCEC_EnableReceive(handle->cecHandle);
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR(("Error enable CEC core to receive messages"));
		goto done;
	}

done:
	return rc;
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
    /* set Pending if message successfuly sent */
    if (!rc)
        handle->status.messageTransmitPending = true;

    return rc;
}


static void NEXUS_Cec_P_UpdatePhysicalAddress(void *pContext)
{
    NEXUS_CecHandle handle = pContext;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputStatus hdmiOutputStatus;
    NEXUS_CecSettings cecSettings;

    rc = NEXUS_HdmiOutput_GetStatus(handle->hdmiOutput, &hdmiOutputStatus);
    if (rc) {
        BDBG_ERR(("Error getting Hdmi_Output status to update CEC physical address"));
        rc = BERR_TRACE(rc) ;
        goto done;
    }

    NEXUS_Cec_GetSettings(handle, &cecSettings);
    if (!hdmiOutputStatus.connected)
    {
        BDBG_LOG(("No Rx device attached - physicalAddress not updated"));
        cecSettings.physicalAddress[0] = 0xFF;
        cecSettings.physicalAddress[1] = 0xFF;
    }
    else {
        cecSettings.physicalAddress[0]= (hdmiOutputStatus.physicalAddressA << 4) | hdmiOutputStatus.physicalAddressB;
        cecSettings.physicalAddress[1]= (hdmiOutputStatus.physicalAddressC << 4) | hdmiOutputStatus.physicalAddressD;
    }
    rc = NEXUS_Cec_SetSettings(handle, &cecSettings);
    if (rc) { BERR_TRACE(rc);}

done:
    return;
}


NEXUS_Error NEXUS_Cec_SetHdmiOutput(
    NEXUS_CecHandle handle,
    NEXUS_HdmiOutputHandle hdmiOutput /* attr{null_allowed=y} Pass NULL to remove/disconnected the link */
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Cec);

    if (hdmiOutput)
    {
        if (handle->hdmiOutput)
        {
            if (handle->hdmiOutput == hdmiOutput) {
                rc = NEXUS_SUCCESS;
                goto done;
            }
            else {
                NEXUS_Module_Lock(g_NEXUS_cecModuleSettings.hdmiOutput);
                NEXUS_HdmiOutput_SetCecHotplugHandler_priv(handle->hdmiOutput, NULL);
                NEXUS_Module_Unlock(g_NEXUS_cecModuleSettings.hdmiOutput);
                NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiOutput, handle->hdmiOutput);
            }
        }

        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_HdmiOutput, hdmiOutput);
        handle->hdmiOutput = hdmiOutput;

        /* first, update CEC physical address */
        NEXUS_Cec_P_UpdatePhysicalAddress(handle);

        if (!handle->cecHotplugEvent)
        {
            rc = BKNI_CreateEvent(&handle->cecHotplugEvent);
            if (rc != BERR_SUCCESS) {
                BDBG_ERR(( "Error creating cecHotplug Event" ));
                rc = BERR_TRACE(rc);
                NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiOutput, handle->hdmiOutput);
                handle->hdmiOutput = NULL;
                goto done;
            }
        }

        /* register event */
        if (handle->cecHotplugEventCallback) {
            NEXUS_UnregisterEvent(handle->cecHotplugEventCallback);
            handle->cecHotplugEventCallback = NULL;
        }

        /* register event */
        handle->cecHotplugEventCallback = NEXUS_RegisterEvent(handle->cecHotplugEvent,
                NEXUS_Cec_P_UpdatePhysicalAddress, handle);

        if (handle->cecHotplugEventCallback == NULL)
        {
            BDBG_ERR(( "NEXUS_RegisterEvent(cecHotplugEvent) failed!" ));
            rc = BERR_TRACE(NEXUS_OS_ERROR);

            if (handle->cecHotplugEvent) {
                BKNI_DestroyEvent(handle->cecHotplugEvent);
                handle->cecHotplugEvent = NULL;
            }
            NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiOutput, handle->hdmiOutput);
            handle->hdmiOutput = NULL;
            goto done;
        }

        NEXUS_Module_Lock(g_NEXUS_cecModuleSettings.hdmiOutput);
        NEXUS_HdmiOutput_SetCecHotplugHandler_priv(hdmiOutput, handle->cecHotplugEvent);
        NEXUS_Module_Unlock(g_NEXUS_cecModuleSettings.hdmiOutput);
    }
    else {
        if (!handle->hdmiOutput) {
            rc = NEXUS_SUCCESS;
            goto done;
        }

        NEXUS_Module_Lock(g_NEXUS_cecModuleSettings.hdmiOutput);
        NEXUS_HdmiOutput_SetCecHotplugHandler_priv(handle->hdmiOutput, NULL);
        NEXUS_Module_Unlock(g_NEXUS_cecModuleSettings.hdmiOutput);

        if (handle->cecHotplugEventCallback) {
            NEXUS_UnregisterEvent(handle->cecHotplugEventCallback);
            handle->cecHotplugEventCallback = NULL;
        }

        if (handle->cecHotplugEvent) {
            BKNI_DestroyEvent(handle->cecHotplugEvent);
            handle->cecHotplugEvent = NULL;
        }

        NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiOutput, handle->hdmiOutput);
        handle->hdmiOutput = NULL;
    }

done:
    return rc;

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

			rc = BCEC_EnableReceive(handle->cecHandle);
			if (rc) rc = BERR_TRACE(rc);

			NEXUS_TaskCallback_Fire(handle->logicalAddressAcquiredCallback);

			break;
		}
		else {
			addr = g_logicalAddrArray[handle->logAddrSearchIndex];
			BDBG_MSG(("Continuing search for CEC Logical Addr: %d...", addr)) ;
			rc = BCEC_PingLogicalAddr(handle->cecHandle, addr);
			if (rc) rc = BERR_TRACE(rc);
			break;
		}


	case NEXUS_CecLogicalAddrSearch_eReady:
		handle->status.logicalAddress = g_logicalAddrArray[handle->logAddrSearchIndex];

		BDBG_MSG(("Found CEC Logical Addr: %d", handle->status.logicalAddress)) ;

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
			rc = BCEC_ReportPhysicalAddress(handle->cecHandle);
			if (rc) rc = BERR_TRACE(rc);

			/* always enable receive after CEC msg is processed */
			rc = BCEC_EnableReceive(handle->cecHandle);
			if (rc) rc = BERR_TRACE(rc);

			NEXUS_TaskCallback_Fire(handle->logicalAddressAcquiredCallback);
		}
		break;
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
