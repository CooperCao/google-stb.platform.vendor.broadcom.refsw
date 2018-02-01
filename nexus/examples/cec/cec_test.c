/***************************************************************************
*  Copyright (C) 2008-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_platform.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>

#if NEXUS_HAS_CEC && NEXUS_NUM_CEC && NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_cec.h"

BDBG_MODULE(cec_test);

NEXUS_HdmiOutputHandle hdmiOutput;
NEXUS_CecHandle hCec;
bool deviceReady = false ;
bool messageReceived = false;

void deviceReady_callback(void *context, int param)
{
	NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_WRN(("BCM%d Logical Address <%d> Acquired", 
        BCHP_CHIP,        
        status.logicalAddress)) ;

    BDBG_WRN(("BCM%d Physical Address: %X.%X.%X.%X",
        BCHP_CHIP,
        (status.physicalAddress[0] & 0xF0) >> 4, 
        (status.physicalAddress[0] & 0x0F),
        (status.physicalAddress[1] & 0xF0) >> 4, 
        (status.physicalAddress[1] & 0x0F))) ;

    if ((status.physicalAddress[0] != 0xFF)
    && (status.physicalAddress[1] != 0xFF))
    {
	    BDBG_WRN(("CEC Device Ready!")) ;
	       deviceReady = true ;
    }
}



void msgReceived_callback(void *context, int param)
{
    NEXUS_CecStatus status;
    NEXUS_CecReceivedMessage receivedMessage;
    NEXUS_Error rc ;
    uint8_t i, j ;
    char msgBuffer[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)];

    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_WRN(("Message Received: %s", status.messageReceived ? "Yes" : "No")) ;
	messageReceived = status.messageReceived;

    rc = NEXUS_Cec_ReceiveMessage(hCec, &receivedMessage);
    BDBG_ASSERT(!rc);
	
    /* For debugging purposes */
    for (i = 0, j = 0; i <= receivedMessage.data.length && j<(sizeof(msgBuffer)-1); i++)
    {
        j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%02X ", 
        receivedMessage.data.buffer[i]) ;
    }           

    BDBG_WRN(("CEC Message Length %d Received: %s", 
        receivedMessage.data.length, msgBuffer)) ;

    BDBG_WRN(("Msg Recd Status from Phys/Logical Addrs: %X.%X.%X.%X / %d", 
        (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
        (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
        status.logicalAddress)) ;
}

void msgTransmitted_callback(void *context, int param)
{
	NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_WRN(("Msg Xmit Status for Phys/Logical Addrs: %X.%X.%X.%X / %d", 
        (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
        (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
        status.logicalAddress)) ;

    BDBG_WRN(("Xmit Msg Acknowledged: %s", 
		status.transmitMessageAcknowledged ? "Yes" : "No")) ;
    BDBG_WRN(("Xmit Msg Pending: %s", 
		status.messageTransmitPending ? "Yes" : "No")) ;
}

int main(int argc, char **argv)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    BKNI_EventHandle event;
    unsigned loops;
    NEXUS_HdmiOutputStatus status;

    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus cecStatus;
    NEXUS_CecMessage transmitMessage;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    hdmiOutput = platformConfig.outputs.hdmi[0];
    hCec = platformConfig.outputs.cec[0];

    BDBG_WRN(("waiting for hdmi connected..."));
    for (loops = 0; loops < 10; loops++) {
        rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
        BDBG_ASSERT(!rc);
        if (status.connected) {
            break;
        }
        BKNI_Sleep(1000);
    }
    if (!status.connected) 
        return 0;

    BDBG_WRN(("********************")) ;
    BDBG_WRN(("HDMI is connected"));
    BDBG_WRN(("********************")) ;

	
    NEXUS_Cec_GetSettings(hCec, &cecSettings);
        cecSettings.messageReceivedCallback.callback = msgReceived_callback ;
        cecSettings.messageReceivedCallback.context = event;
        
        cecSettings.messageTransmittedCallback.callback = msgTransmitted_callback;
        cecSettings.messageTransmittedCallback.context = event;
        
        cecSettings.logicalAddressAcquiredCallback.callback = deviceReady_callback ;
        cecSettings.logicalAddressAcquiredCallback.context = event;

        cecSettings.physicalAddress[0]= (status.physicalAddressA << 4) 
            | status.physicalAddressB;
        cecSettings.physicalAddress[1]= (status.physicalAddressC << 4) 
            | status.physicalAddressD;
	
    rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    BDBG_ASSERT(!rc);


    /* Enable CEC core */
    NEXUS_Cec_GetSettings(hCec, &cecSettings);
        cecSettings.enabled = true;
        rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    BDBG_ASSERT(!rc);


    BDBG_WRN(("*************************")) ;
    BDBG_WRN(("Wait for logical address before starting test..."));
    BDBG_WRN(("*************************\n\n")) ;
    for (loops = 0; loops < 10; loops++) {
		if (deviceReady)
			break;
        BKNI_Sleep(1000);
    }

    if (cecStatus.logicalAddress == 0xFF)
    {
        BDBG_ERR(("No CEC capable device found on HDMI output."));
        goto done ;
    }

    transmitMessage.destinationAddr = 0;
    transmitMessage.length = 1;
        
    
    BDBG_WRN(("*************************")) ;
    BDBG_WRN(("Make sure TV is turned ON")); 
    BDBG_WRN(("Send <Image View On> message")); /* this is needed in case TV was already powered off */
    BDBG_WRN(("*************************")) ;
    transmitMessage.buffer[0] = 0x04;
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    BDBG_ASSERT(!rc);  
    printf("Press <ENTER> to continue\n");
    getchar();


    BDBG_WRN(("*************************")) ;
    BDBG_WRN(("Now Turn the TV OFF ")); 
    BDBG_WRN(("Send <Standby> message "));
    BDBG_WRN(("*************************")) ;
    transmitMessage.buffer[0] = 0x36;
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);  
    BDBG_ASSERT(!rc);
    printf("Press <ENTER> to continue\n");
    getchar();


    BDBG_WRN(("*************************")) ;
    BDBG_WRN(("Turn the TV back ON")); 
    BDBG_WRN(("Send <Image View On> message")); 
    BDBG_WRN(("*************************")) ;
    transmitMessage.buffer[0] = 0x04;
    rc = NEXUS_Cec_TransmitMessage(hCec, &transmitMessage);
    if (rc)
    {
		BDBG_WRN(("Error Transmitting <Image View On> Message %d", rc)) ;
    }
    printf("Press <ENTER> to continue\n");
    getchar();
    
    loops = 10;
    while (--loops) 
    {
        BDBG_WRN(("Waiting for broadcast message from the TV...\n"));
        BKNI_WaitForEvent(event, 5 * 1000);
        if (!messageReceived) 
        {
            BDBG_WRN(("CEC Message callback, but no msg received"));
        }
        
    }

done:   
    BDBG_WRN(("*************************")) ;
    BDBG_WRN(("Wait 10 seconds for any additional messages"));
    BDBG_WRN(("*************************")) ;
    /* can't exit the app right away, otherwise the TV won't turn back on */
    BKNI_Sleep(10 * 1000);

    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();

    return 0;
}
#else

int main(int argc, char **argv)
{
    BSTD_UNUSED(argc);
    printf("%s Support for CEC has been disabled; use 'NEXUS_CEC_SUPPORT=y' to enable",
		argv[0]) ;
    return 0 ;
}

#endif

