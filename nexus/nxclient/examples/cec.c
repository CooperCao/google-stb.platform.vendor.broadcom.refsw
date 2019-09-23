/***************************************************************************
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
***************************************************************************/
#include "nxclient.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>

/* app based on nexus/examples/cec/cec_test.c */

#if NEXUS_HAS_CEC
#include "nexus_cec.h"

BDBG_MODULE(cec);
#include "nxapp_prompt.inc"

#define appHdmiOutputHpCbId 0

#define TV_ADDR 0x00                    /* logical address of the TV */



enum lastReqCmdStatus {
    NonePending,
    Pending
};

static NEXUS_CecHandle hCec;

static bool messageReceived = false;
static unsigned lastReqCmd = 0;
static enum lastReqCmdStatus lastReqCmdProcessed = NonePending;


void deviceReady_callback(void *context, int param)
{
    NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BKNI_SetEvent(context) ;

    /* print informative information */
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_WRN(("BCM%d Logical Address <%d> Acquired",
        BCHP_CHIP,
        status.logicalAddress));

    BDBG_WRN(("BCM%d Physical Address: %X.%X.%X.%X",
        BCHP_CHIP,
        (status.physicalAddress[0] & 0xF0) >> 4,
        (status.physicalAddress[0] & 0x0F),
        (status.physicalAddress[1] & 0xF0) >> 4,
        (status.physicalAddress[1] & 0x0F)));

}

static void msgReceived_callback(void *context, int param)
{
    NEXUS_CecStatus status;
    NEXUS_CecReceivedMessage receivedMessage;
    NEXUS_Error rc;
    uint8_t i, j;
    char msgBuffer[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)];

    BSTD_UNUSED(param);

    BKNI_SetEvent((BKNI_EventHandle)context);
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_WRN(("Message Received: %s", status.messageReceived ? "Yes" : "No"));
    messageReceived = status.messageReceived;

    rc = NEXUS_Cec_ReceiveMessage(hCec, &receivedMessage);
    BDBG_ASSERT(!rc);

    /* For debugging purposes */
    for (i = 0, j = 0; i <= receivedMessage.data.length && j<(sizeof(msgBuffer)-1); i++)
    {
        j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%02X ",
        receivedMessage.data.buffer[i]);
    }

    BDBG_WRN(("CEC Message Length %d Received: %s",
        receivedMessage.data.length, msgBuffer));

    BDBG_WRN(("Msg Recd Status from Phys/Logical Addrs: %X.%X.%X.%X / %d",
        (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
        (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
        status.logicalAddress));

    if(lastReqCmdProcessed == NonePending)
    {
        char *cmdMesg;
        bool gotReply = false;

        switch(lastReqCmd)
        {
            case NEXUS_CEC_OpGivePhysicalAddress:
                cmdMesg = "Physical Addr";
                if(receivedMessage.data.buffer[0] == NEXUS_CEC_OpReportPhysicalAddress)
                {
                    gotReply = true;
                }
                break;
            case NEXUS_CEC_OpGiveDeviceVendorID:
                cmdMesg = "Vendor ID";
                if(receivedMessage.data.buffer[0] == NEXUS_CEC_OpDeviceVendorID)
                {
                    gotReply = true;
                }
                break;
            case NEXUS_CEC_OpGetCECVersion:
                cmdMesg = "CEC Version";
                if(receivedMessage.data.buffer[0] == NEXUS_CEC_OpCECVersion)
                {
                    gotReply = true;
                }
                break;
            default:
                /* don't care message */
                goto done;
                break;
        }
        BDBG_LOG(("-------------------------"));
        if(gotReply == true)
        {
            /* First three characters in msgBuffer is command ID, which has been process in the above switch-case statement */
            BDBG_LOG(("deviceAddr=0x%02X reports %s: %s ", receivedMessage.data.initiatorAddr, cmdMesg, &msgBuffer[3]));
            lastReqCmdProcessed = Pending;
        }
        else
        {
            BDBG_LOG(("deviceAddr=0x%02X didn't reply to %s Request", TV_ADDR, cmdMesg));
        }
        BDBG_LOG(("-------------------------"));
    }
done:
    return;
}

static void msgTransmitted_callback(void *context, int param)
{
    NEXUS_CecStatus status;

    BSTD_UNUSED(context);
    BSTD_UNUSED(param);

    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_WRN(("Msg Xmit Status for Phys/Logical Addrs: %X.%X.%X.%X / %d",
        (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
        (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
        status.logicalAddress));

    BDBG_WRN(("Xmit Msg Acknowledged: %s",
        status.transmitMessageAcknowledged ? "Yes" : "No"));
    BDBG_LOG(("---->Xmit Msg Pending: %s",
        status.messageTransmitPending ? "Yes" : "No"));

}

int main(void)
{
    NEXUS_Error rc;
    BKNI_EventHandle event;
    BKNI_EventHandle cecDeviceReadyEvent;
    unsigned loops;
    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus cecStatus;
    NEXUS_CecMessage transmitMessage;

    rc = NxClient_Join(NULL);
    if (rc) return rc;

    BKNI_CreateEvent(&event);
    BKNI_CreateEvent(&cecDeviceReadyEvent) ;

    /* nxserver does not open CEC, so a client may. */
    NEXUS_Cec_GetDefaultSettings(&cecSettings);
    hCec = NEXUS_Cec_Open(0, &cecSettings);
    if (!hCec) {
        BDBG_ERR(("unable to open CEC"));
        return -1;
    }

    NEXUS_Cec_GetSettings(hCec, &cecSettings);
        cecSettings.messageReceivedCallback.callback = msgReceived_callback;
        cecSettings.messageReceivedCallback.context = event;

        cecSettings.messageTransmittedCallback.callback = msgTransmitted_callback;
        cecSettings.messageTransmittedCallback.context = event;

        cecSettings.logicalAddressAcquiredCallback.callback = deviceReady_callback;
        cecSettings.logicalAddressAcquiredCallback.context = cecDeviceReadyEvent;
    rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    BDBG_ASSERT(!rc);

    /* Enable CEC core */
    NEXUS_Cec_GetSettings(hCec, &cecSettings);
    cecSettings.enabled = true;
    rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    BDBG_ASSERT(!rc);

    BDBG_LOG(("*************************"));
    BDBG_LOG(("Wait for logical address before starting test..."));
    BDBG_LOG(("*************************"));

    rc = BKNI_WaitForEvent(cecDeviceReadyEvent, 10 * 1000);
    if (rc)
    {
        BDBG_ERR(("Error waiting for CEC..."));
        goto done;
    }


    NEXUS_Cec_GetStatus(hCec, &cecStatus);

    if (cecStatus.logicalAddress == 0xFF)
    {
        BDBG_ERR(("No CEC capable device found on HDMI output."));
        goto done;
    }

    transmitMessage.destinationAddr = 0;
    transmitMessage.length = 1;
    BDBG_LOG(("*************************"));
    BDBG_LOG(("Start Communication with device at logical addr:0x%02X (logical addr=0x00 is TV)",TV_ADDR));
    BDBG_LOG(("*************************"));

    BDBG_LOG((" "));
    BDBG_LOG(("*************************"));
    BDBG_LOG(("Request Device Physical Addr, (cmd=0x%02x)", NEXUS_CEC_OpGivePhysicalAddress));
    BDBG_LOG(("*************************"));
    transmitMessage.buffer[0] = lastReqCmd = NEXUS_CEC_OpGivePhysicalAddress;
    lastReqCmdProcessed = NonePending; /* get ready process request command */

    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    BDBG_ASSERT(!rc);
    nxapp_prompt("continue");

    BDBG_LOG(("*************************"));
    BDBG_LOG(("Request Device Vendor ID (cmd=0x%02x)", NEXUS_CEC_OpGiveDeviceVendorID));
    BDBG_LOG(("*************************"));
    transmitMessage.buffer[0] = lastReqCmd = NEXUS_CEC_OpGiveDeviceVendorID;
    lastReqCmdProcessed = NonePending; /* get ready process request command */
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    BDBG_ASSERT(!rc);
    nxapp_prompt("continue");

    BDBG_LOG(("*************************"));
    BDBG_LOG(("Request Device CEC Version (cmd=0x%02x)", NEXUS_CEC_OpGetCECVersion));
    BDBG_LOG(("*************************"));
    transmitMessage.buffer[0] = lastReqCmd = NEXUS_CEC_OpGetCECVersion;
    lastReqCmdProcessed = NonePending; /* get ready process request command */
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    BDBG_ASSERT(!rc);
    nxapp_prompt("continue");

    BDBG_LOG(("*************************"));
    BDBG_LOG(("Make sure TV is turned ON (cmd=0x%02X)", NEXUS_CEC_OpImageViewOn));
    BDBG_LOG(("Send <Image View On> message")); /* this is needed in case TV was already powered off */
    BDBG_LOG(("*************************"));
    transmitMessage.buffer[0] = lastReqCmd = NEXUS_CEC_OpImageViewOn;
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    BDBG_ASSERT(!rc);
    nxapp_prompt("continue");


    BDBG_LOG(("*************************"));
    BDBG_LOG(("Now Turn the TV OFF (cmd=0x%02X)", NEXUS_CEC_OpStandby));
    BDBG_LOG(("Send <Standby> message "));
    BDBG_LOG(("*************************"));
    transmitMessage.buffer[0] = lastReqCmd = NEXUS_CEC_OpStandby;
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    BDBG_ASSERT(!rc);
    nxapp_prompt("continue");


    BDBG_LOG(("*************************"));
    BDBG_LOG(("Turn the TV back ON (cmd=0x%02X)", NEXUS_CEC_OpImageViewOn));
    BDBG_LOG(("Send <Image View On> message"));
    BDBG_LOG(("*************************"));
    transmitMessage.buffer[0] = lastReqCmd = NEXUS_CEC_OpImageViewOn;
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    if (rc) {
        BDBG_WRN(("Error Transmitting <Image View On> Message %d", rc));
    }
    nxapp_prompt("continue");

    loops = 10;
    while (--loops)
    {
        BDBG_WRN(("Waiting for broadcast message from the TV..."));
        BKNI_WaitForEvent(event, 5 * 1000);
        if (!messageReceived)
        {
            BDBG_WRN(("CEC Message callback, but no msg received"));
        }
    }

done:
    BDBG_WRN(("*************************"));
    BDBG_WRN(("Wait 10 seconds for any additional messages"));
    BDBG_WRN(("*************************"));
    /* can't exit the app right away, otherwise the TV won't turn back on */
    BKNI_Sleep(10 * 1000);

    BDBG_WRN(("Done"));
    BKNI_DestroyEvent(event);
    NxClient_Uninit();

    return 0;
}
#else
int main(void)
{
    printf("CEC support not enabled\n");
    return 1;
}
#endif
