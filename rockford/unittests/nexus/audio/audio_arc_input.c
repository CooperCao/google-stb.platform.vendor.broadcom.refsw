/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/
/* Nexus example app: Play audio received from the Audio Return Channel. */

#include "nexus_platform.h"
#if NEXUS_HAS_CEC && NEXUS_NUM_CEC && NEXUS_HAS_AUDIO && NEXUS_NUM_SPDIF_INPUTS
#include "bstd.h"
#include "bkni.h"
#include "blst_queue.h"
#include <stdlib.h>
#include "nexus_hdmi_output.h"
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include "nexus_cec.h"
#include "nexus_spdif_input.h"


BDBG_MODULE(audio_arc_input);

NEXUS_HdmiOutputHandle hdmiOutput;
NEXUS_CecHandle        hCec;
bool deviceReady        = false ;
bool reportArcInitiated = false ;

bool    activeSourceValid = false ;
uint8_t activeSourcePhysicalAddress[2];

bool    cecVersionValid = false;
uint8_t cecVersionValue;

bool    powerStatusValid = false;
uint8_t powerStatusValue;

bool    systemAudioMode = false;


/* Define the CEC transmit queue.  */
#define CEC_TX_QUEUE_ELEM_COUNT  (7)

struct cecTxQueue
{
    BLST_Q_ENTRY(cecTxQueue) cecTxQueueLink;
    int  idx;
    NEXUS_CecMessage cecMessage;
};

static struct cecTxQueue   cecTxQueue[CEC_TX_QUEUE_ELEM_COUNT];
static BLST_Q_HEAD(cecTxQueueFreeHead, cecTxQueue) cecTxQueueFreeHead = BLST_Q_INITIALIZER(cecTxQueueFreeHead);
static BLST_Q_HEAD(cecTxQueueWaitingHead, cecTxQueue) cecTxQueueWaitingHead = BLST_Q_INITIALIZER(cecTxQueueWaitingHead);
static BLST_Q_HEAD(cecTxQueueBusyHead, cecTxQueue) cecTxQueueBusyHead = BLST_Q_INITIALIZER(cecTxQueueBusyHead);


void printTxQueueElem(NEXUS_CecHandle handle, const struct cecTxQueue *pElem, const char *pPrefix)
{
    unsigned i, j;
    char msgBuffer[128] ;

    BSTD_UNUSED(handle);

    for (i = 0, j = 0; i < pElem->cecMessage.length ; i++)
    {
        j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%02X ",
        pElem->cecMessage.buffer[i]) ;
    }

    BDBG_WRN (("CEC Send: %s %u (%p): From:%2u To:%2u Length:%u Data: %s  pMessage: %p",
              pPrefix,
              pElem->idx, (void*)pElem,
              pElem->cecMessage.initiatorAddr,
              pElem->cecMessage.destinationAddr,
              pElem->cecMessage.length,
              msgBuffer, (void*)&pElem->cecMessage));
}

NEXUS_Error sendCecMessage(NEXUS_CecHandle handle, const NEXUS_CecMessage *pMessage)
{
    struct cecTxQueue   *pElem;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    /* See if a message pointer was passed.  A null pointer indicates that a prior
     * send has completed. */
    if (pMessage)
    {
        /* We got a message to send... add it to the transmit queue. */
        struct cecTxQueue   *pElem;

        {  /* Start Critical Section */
            BKNI_EnterCriticalSection();
            pElem = BLST_Q_FIRST(&cecTxQueueFreeHead);  /* get the first free element */

            if (!pElem)
            {
                BDBG_ERR(("sendCecMessage: cecTxQueue overflow!!!" ));
                BDBG_ASSERT(0);
            }

            /* Relink element from free list to waiting list. */
            BLST_Q_REMOVE(&cecTxQueueFreeHead, pElem, cecTxQueueLink);
            pElem->cecMessage = *pMessage;  /* copy the message into the element. */
            /*  printTxQueueElem(handle,pElem,"Adding"); */
            BLST_Q_INSERT_TAIL(&cecTxQueueWaitingHead, pElem, cecTxQueueLink );
            BKNI_LeaveCriticalSection();
        }  /* End Critical Section */

    }
    else
    {
        /* No message to send. This means the prior send has completed. Remove
         * prior sent message from the busy list. Then we'll check to see
         * if we have something to send (below) */
        struct cecTxQueue   *pElem;

        {  /* Start Critical Section */
            BKNI_EnterCriticalSection();
            pElem = BLST_Q_FIRST(&cecTxQueueBusyHead);

            /* If something is in the busy list, then it's finished being transmitted
             * so we can move that element back to the free list.   */
            if ( pElem)
            {
                /* Relink the element from the busy list to the free list. */
                BLST_Q_REMOVE(&cecTxQueueBusyHead, pElem, cecTxQueueLink);
                /* printTxQueueElem(handle,pElem,"Deleting"); */
                BLST_Q_INSERT_TAIL(&cecTxQueueFreeHead, pElem, cecTxQueueLink );
            }
            BKNI_LeaveCriticalSection();
        }  /* End Critical Section */

        /* But if there was nothing in the busy list, then we are probably getting notified
         * of somebody elses send completion... so there's nothing to do here except for
         * maybe print a warning message. */
        if ( ! pElem)
        {
            BDBG_WRN(("CEC Send: Send complete callback, but nothing being sent."));
        }
    }

    /* If the busy list is empty, then look for a queued message to send.
     * If there's a current message in the busy list, we'll come back here later
     * when that message finishes. */
    pElem = NULL;

    {  /* Start Critical Section */
        BKNI_EnterCriticalSection();
        if ( BLST_Q_EMPTY(&cecTxQueueBusyHead))     /* If no send in progress */
        {
            /* Now set up to transmit the oldest message from the transmit queue. */
            pElem = BLST_Q_FIRST(&cecTxQueueWaitingHead);
            if (pElem)
            {
                BLST_Q_REMOVE(&cecTxQueueWaitingHead, pElem, cecTxQueueLink);

                /* Leave the critical section while printing*/
                BKNI_LeaveCriticalSection();
                printTxQueueElem(handle,pElem,"Sending");
                BKNI_EnterCriticalSection();

                BLST_Q_INSERT_TAIL(&cecTxQueueBusyHead, pElem, cecTxQueueLink );
            }
        }
        BKNI_LeaveCriticalSection();
    }  /* End Critical Section */

    /* If pElem is non-NULL, then we've moved an element to the busy list
     * and it's okay for us to try to send it.  */
    if (pElem)
    {
        errCode = NEXUS_Cec_TransmitMessage(handle, &pElem->cecMessage);
        if (errCode != NEXUS_SUCCESS)
        {
            /* The send failed, probably because there is another message
             * being sent by someone else.  Don't panic, just link the
             * element back to the head of the waiting list, and we'll
             * try again the current send completes.     */
            BDBG_ERR(("CEC Send: Send failed!!! Leaving element in transmit queue"));

            {  /* Start Critical Section */
                BKNI_EnterCriticalSection();
                BLST_Q_REMOVE(&cecTxQueueBusyHead, pElem, cecTxQueueLink);

                /* Leave the critical section while printing*/
                BKNI_LeaveCriticalSection();
                printTxQueueElem(handle,pElem,"Requeing");
                BKNI_EnterCriticalSection();

                BLST_Q_INSERT_HEAD(&cecTxQueueWaitingHead, pElem, cecTxQueueLink );
                BKNI_LeaveCriticalSection();
            }  /* End Critical Section */

        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error checkRecvMsgSize(uint8_t opcode, unsigned expectedLen, unsigned actualLen)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    if (actualLen != expectedLen)
    {
        BDBG_ERR(("CEC Recv: Wrong message length for opcode 0x%02x: expected %u bytes received %u", opcode, expectedLen, actualLen ));
        errCode = NEXUS_INVALID_PARAMETER;
    }
    return (errCode);
}


void deviceReady_callback(void *context, int param)
{
    NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_LOG(("CEC Device Ready: Logical Address %d Acquired", status.logicalAddress )) ;

    deviceReady = true;
}


void msgReceived_callback(void *context, int param)
{
    NEXUS_CecStatus             status;
    NEXUS_CecReceivedMessage    receivedMessage;
    NEXUS_Error                 rc ;
    uint8_t                     i, j ;
    char                        msgBuffer[128] ;

    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    NEXUS_Cec_GetStatus(hCec, &status);

    if ( ! status.messageReceived)
    {
        BDBG_ERR(("CEC Recv: No message!!!")) ;
        return;
    }

    BKNI_Memset(&receivedMessage, 0xEE, sizeof receivedMessage);  /* fill buffer with recognizable pattern (debugging) */

    rc = NEXUS_Cec_ReceiveMessage(hCec, &receivedMessage);
    BDBG_ASSERT(!rc);

    /* For debugging purposes */
    msgBuffer[0] = 0;   /* Set initial null terminator in case of zero length message. */
    for (i = 0, j = 0; i < receivedMessage.data.length ; i++)
    {
        j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%02X ",
        receivedMessage.data.buffer[i]) ;
    }

    BDBG_LOG(("CEC Recv: From:%2u To:%2u Length:%u Data: %s  Ack:%u EOM:%u ",
            receivedMessage.data.initiatorAddr,
            receivedMessage.data.destinationAddr,
            receivedMessage.data.length,
            msgBuffer,
            receivedMessage.receivedStatus.receivedMessageAcknowledged,
            receivedMessage.receivedStatus.endOfMessage ));


    /*******************************************************************
     *  Do any processing that might be needed for the received message.
     *******************************************************************/
    {
        uint8_t    *pMsg = &receivedMessage.data.buffer[0];
        unsigned msgLen = receivedMessage.data.length;

        if (msgLen == 0)
        {

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Polling Message>"));
            BDBG_LOG(("****************************************************")) ;

        }

        else if (pMsg[0] == 0x83)    /* <Give Physical Address> */
        {
            NEXUS_CecMessage transmitMessage;

            checkRecvMsgSize(0x83, 1, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Give Physical Address>"));
            BDBG_LOG(("****************************************************")) ;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Report Physical Address>"));
            BDBG_LOG(("****************************************************")) ;

            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0xF;
            transmitMessage.length = 4;
            transmitMessage.buffer[0] = 0x84;   /* <Report Physical Address> */
            transmitMessage.buffer[1] = 0x00;
            transmitMessage.buffer[2] = 0x01;
            transmitMessage.buffer[3] = 0x05;

            rc = sendCecMessage(hCec, &transmitMessage);
            BDBG_ASSERT(!rc);
            }
        else if (pMsg[0] == 0xC1)    /* <Report ARC Initiated> */
        {
            checkRecvMsgSize(0xC1, 1, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <ARC Initiated>"));
            BDBG_LOG(("****************************************************")) ;

            reportArcInitiated = true;
        }
        else if (pMsg[0] == 0x82)    /* <ActiveSource> */
        {
            checkRecvMsgSize(0x82, 3, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Active Source>"));
            BDBG_LOG(("****************************************************")) ;

            activeSourcePhysicalAddress[0] = pMsg[1];
            activeSourcePhysicalAddress[1] = pMsg[2];
            activeSourceValid = true;
        }
        else if (pMsg[0] == 0x9E)    /* <CEC Version> */
        {
            checkRecvMsgSize(0x9E, 2, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <CEC Version>"));
            BDBG_LOG(("****************************************************")) ;

            cecVersionValue = pMsg[1];
            cecVersionValid = true;
        }
        else if (pMsg[0] == 0x90)    /* <Report Power Status> */
        {
            checkRecvMsgSize(0x90, 2, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Report Power Status>"));
            BDBG_LOG(("****************************************************")) ;

            powerStatusValue = pMsg[1];
            powerStatusValid = true;
        }
        else if (pMsg[0] == 0x8C)    /* <Give Device Vendor ID> */
        {
            NEXUS_CecMessage transmitMessage;

            checkRecvMsgSize(0x8C, 1, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Give Device Vendor ID>"));
            BDBG_LOG(("****************************************************")) ;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Device Vendor ID>"));
            BDBG_LOG(("****************************************************")) ;

            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0xf;
            transmitMessage.length = 4;
            transmitMessage.buffer[0] = 0x87;   /* <Device Vendor ID> */
            transmitMessage.buffer[1] = 0x18;   /* 18-C0-86 Broadcom Corp. */
            transmitMessage.buffer[2] = 0xC0;
            transmitMessage.buffer[3] = 0x86;

            rc = sendCecMessage(hCec, &transmitMessage);
            BDBG_ASSERT(!rc);
            }
        else if (pMsg[0] == 0xC3)    /* <Request ARC Initiation> */
        {
            NEXUS_CecMessage transmitMessage;

            checkRecvMsgSize(0xC3, 1, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Request ARC Initiation>"));
            BDBG_LOG(("****************************************************")) ;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Initiate ARC>"));
            BDBG_LOG(("****************************************************")) ;

            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0x0;
            transmitMessage.length = 1;
            transmitMessage.buffer[0] = 0xC0;   /* <Initiate ARC> */

            rc = sendCecMessage(hCec, &transmitMessage);
            BDBG_ASSERT(!rc);
        }
        else if (pMsg[0] == 0x70)    /* <System Audio Mode Request> */
        {
            NEXUS_CecMessage transmitMessage;

            if (msgLen != 1 && msgLen != 3) checkRecvMsgSize(0x70, 3, msgLen);

            if (msgLen == 3)    systemAudioMode = true;
            else                systemAudioMode = false;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <System Audio Mode Request>"));
            BDBG_LOG(("****************************************************")) ;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Set System Audio Mode: %s>", systemAudioMode ? "True" : "False"));
            BDBG_LOG(("****************************************************")) ;


            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0xF;
            transmitMessage.length = 2;
            transmitMessage.buffer[0] = 0x72;   /* <Set System Audio Mode> */
            transmitMessage.buffer[1] = systemAudioMode;   /* 01 => System Audio Status="On" */

            rc = sendCecMessage(hCec, &transmitMessage);
            BDBG_ASSERT(!rc);
        }
        else if (pMsg[0] == 0x71)    /* <Give Audio Status> */
        {
            NEXUS_CecMessage transmitMessage;

            checkRecvMsgSize(0x71, 1, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Give Audio Status>"));
            BDBG_LOG(("****************************************************")) ;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Report Audio Status>"));
            BDBG_LOG(("****************************************************")) ;

            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0x0;
            transmitMessage.length = 2;
            transmitMessage.buffer[0] = 0x7A;   /* <Report Audio Status> */
            transmitMessage.buffer[1] = 0x32;   /* Unmuted, 50% volume */

            rc = sendCecMessage(hCec, &transmitMessage);
            BDBG_ASSERT(!rc);
        }
        else if (pMsg[0] == 0x7D)    /* <Give System Audio Mode Status> */
        {
            NEXUS_CecMessage transmitMessage;

            checkRecvMsgSize(0x7D, 1, msgLen);

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Give System Audio Mode Status>"));
            BDBG_LOG(("****************************************************")) ;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <System Audio Mode Status>"));
            BDBG_LOG(("****************************************************")) ;

            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0x0;
            transmitMessage.length = 2;
            transmitMessage.buffer[0] = 0x7E;   /* <System Audio Mode Status> */
            transmitMessage.buffer[1] = systemAudioMode;   /* 1 => On */

            rc = sendCecMessage(hCec, &transmitMessage);
            BDBG_ASSERT(!rc);
        }

        else if (pMsg[0] == 0x89)    /* <Vendor Command> */
        {
            NEXUS_CecMessage transmitMessage;

            /* checkRecvMsgSize(0x89, 1, msgLen); */

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Vendor Command>"));
            BDBG_LOG(("****************************************************")) ;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Feature Abort>"));
            BDBG_LOG(("****************************************************")) ;

            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0x0;
            transmitMessage.length = 3;
            transmitMessage.buffer[0] = 0x00;   /* <Feature Abort> */
            transmitMessage.buffer[1] = 0x89;   /* [Feature Opcode] */
            transmitMessage.buffer[2] = 0x00;   /* [Abort Reason] */

            rc = sendCecMessage(hCec, &transmitMessage);
            /* BDBG_ASSERT(!rc);   */
        }

        else if (pMsg[0] == 0xA0)    /* <Vendor Command With ID> */
        {
            NEXUS_CecMessage transmitMessage;

            /* checkRecvMsgSize(0x89, 1, msgLen); */

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Vendor Command with ID>"));
            BDBG_LOG(("****************************************************")) ;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Feature Abort>"));
            BDBG_LOG(("****************************************************")) ;

            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0x0;
            transmitMessage.length = 3;
            transmitMessage.buffer[0] = 0x00;   /* <Feature Abort> */
            transmitMessage.buffer[1] = 0xA0;   /* [Feature Opcode] */
            transmitMessage.buffer[2] = 0x00;   /* [Abort Reason] */

            rc = sendCecMessage(hCec, &transmitMessage);
            /* BDBG_ASSERT(!rc);   */
        }
        else
        {

            BDBG_WRN(("****************************************************")) ;
            BDBG_WRN(("*  Received Unexpected Message: 0x%02x !!! ", pMsg[0]));
            BDBG_WRN(("****************************************************")) ;

        }
    }
}

void msgTransmitted_callback(void *context, int param)
{
    NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_LOG(("CEC Send Complete: Ack:%u", status.transmitMessageAcknowledged ));

    /* Call sendCecMessage (with a null message) so that it can send the
     * next message (if one is waiting in the transmit queue). */
    sendCecMessage(hCec, NULL);
}

int doSpdifInput(void)
{
    NEXUS_SpdifInputHandle spdifInput;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

    spdifInput = NEXUS_SpdifInput_Open(0, NULL);

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.input = NEXUS_SpdifInput_GetConnector(spdifInput);
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.autoConfigTimebase = false;
    audioProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    #if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                               NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                               NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));
    #endif

    BDBG_LOG(("Starting Audio Decoder"));
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);


    BDBG_LOG(("****************************************************")) ;
    BDBG_LOG(("*  Press any key to exit"));
    BDBG_LOG(("****************************************************")) ;

    getchar();

    BDBG_LOG(("Stopping decoder"));
    NEXUS_AudioDecoder_Stop(audioDecoder);

    BDBG_LOG(("Removing inputs from outputs"));
    #if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
    #endif

    BDBG_LOG(("Shutting down decoder connectors"));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

    BDBG_LOG(("Shutting down spdif input connector"));
    NEXUS_AudioInput_Shutdown(NEXUS_SpdifInput_GetConnector(spdifInput));

    BDBG_LOG(("Closing stc channel"));
    NEXUS_StcChannel_Close(audioProgram.stcChannel);

    BDBG_LOG(("Closing decoder"));
    NEXUS_AudioDecoder_Close(audioDecoder);

    BDBG_LOG(("Closing spdif input"));
    NEXUS_SpdifInput_Close(spdifInput);

    return 0;
}

int main(int argc, char **argv)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    unsigned hdmiOutputIndex = 0;
    BKNI_EventHandle event;
    unsigned loops;
    NEXUS_HdmiOutputStatus status;

    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus cecStatus;

    BDBG_LOG(("Initializing the CEC transmit queue (cecTxQueue)."));
    /* Initialize the cecTxQueue elements. */
    {
        int i;

        for (i=0 ; i<CEC_TX_QUEUE_ELEM_COUNT ; i++)
        {
            struct cecTxQueue   *pElem;

            pElem = &cecTxQueue[i];
            pElem->idx = i;     /* Set an index value (mostly for friendly debug prints). */

            /* Then link the element to the end of the free list. */
            /* BDBG_WRN(("Adding element %u (%p) to Free list", pElem->idx, pElem)); */
            BLST_Q_INSERT_TAIL(&cecTxQueueFreeHead, pElem, cecTxQueueLink );
        }
    }

    if (argc > 1) hdmiOutputIndex = atoi(argv[1]);

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    hdmiOutput = platformConfig.outputs.hdmi[0];
    hCec = platformConfig.outputs.cec[0];


    BDBG_LOG(("****************************************************")) ;
    BDBG_LOG(("*  Checking for HDMI Output connection"));
    BDBG_LOG(("****************************************************")) ;

    for (loops = 0; loops < 10; loops++)
    {
        rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &status);
        if ( !rc && status.connected) break;
        BDBG_LOG(("Waiting for HDMI connection"));
        BKNI_Sleep(500);
    }
    if ( rc || !status.connected)
    {
        BDBG_ERR(("Timed out waiting for HDMI Output connection.  Giving up."));
        goto done ;
    }


    BDBG_LOG(("****************************************************")) ;
    BDBG_LOG(("*  HDMI is connected"));
    BDBG_LOG(("****************************************************")) ;


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


    BDBG_LOG(("****************************************************")) ;
    BDBG_LOG(("*  Using CEC Physical Address: %X.%X.%X.%X",
        (cecSettings.physicalAddress[0] & 0xF0) >> 4,
        (cecSettings.physicalAddress[0] & 0x0F),
        (cecSettings.physicalAddress[1] & 0xF0) >> 4,
        (cecSettings.physicalAddress[1] & 0x0F) )) ;
    BDBG_LOG(("****************************************************")) ;


    cecSettings.deviceType = 5;     /* 3 => Tuner, 5 => Audio System */

    rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    BDBG_ASSERT(!rc);

    /* Enable CEC core */
    NEXUS_Cec_GetSettings(hCec, &cecSettings);
        cecSettings.enabled = true;
        rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
    BDBG_ASSERT(!rc);


    BDBG_LOG(("****************************************************")) ;
    BDBG_LOG(("*  Checking for CEC Logical Address"));
    BDBG_LOG(("****************************************************")) ;

    for (loops = 0; loops < 10; loops++) {
        if (deviceReady) break;
        BDBG_LOG(("Waiting for CEC Logical Address"));
        BKNI_Sleep(500);
    }

    if ( !deviceReady )
    {
        BDBG_ERR(("Timed out waiting for CEC Logical Address.  Giving up."));
        goto done ;
    }

    rc = NEXUS_Cec_GetStatus(hCec, &cecStatus);
    BDBG_ASSERT(!rc);

    if (cecStatus.logicalAddress == 0xFF)
    {
        BDBG_ERR(("No CEC capable device found on HDMI output."));
        goto done ;
    }


    BDBG_LOG(("****************************************************")) ;
    BDBG_LOG(("*  CEC Logical Address is %u", cecStatus.logicalAddress));
    BDBG_LOG(("****************************************************")) ;


    {
        NEXUS_CecMessage transmitMessage;
        unsigned            powerUpCount;
        unsigned            initiateArcCount;

        transmitMessage.initiatorAddr = cecStatus.logicalAddress;  /* not used for transmitting. */

        for (powerUpCount=0 ; powerUpCount<5 ; powerUpCount++)
        {

            /*******************************************************************
             *  Send a <Give Device Power Status> message to the TV  and wait
             *  for the <Report Power Status> response.
             *******************************************************************/
            powerStatusValid = false;


            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Give Device Power Status>"));
            BDBG_LOG(("****************************************************")) ;

            transmitMessage.destinationAddr = 0x0;
            transmitMessage.length = 1;
            transmitMessage.buffer[0] = 0x8F;   /* <Give Device Power Status> */

            rc = sendCecMessage(hCec, &transmitMessage);
            BDBG_ASSERT(!rc);

            for (loops = 0; loops < 10; loops++) {
                if ( powerStatusValid ) break;
                BDBG_LOG(("Waiting for <Report Power Status>"));
                BKNI_Sleep(500);
            }

            if ( powerStatusValid)
            {

                BDBG_LOG(("************************************************************************")) ;
                BDBG_LOG(("*  Got <Report Power Status>: power code is 0x%x => %s", powerStatusValue,
                                                        powerStatusValue==0x0 ? "On"        :
                                                        powerStatusValue==0x1 ? "Standby" :
                                                        powerStatusValue==0x2 ? "In transition: Standby -> On" :
                                                        powerStatusValue==0x3 ? "In transition: On -> Standby" :
                                                                                "<Reserved>"       ));
                BDBG_LOG(("************************************************************************")) ;

                if (powerStatusValid && powerStatusValue==0x0) break;   /* Great!  TV is powered up! */
            }
            else
            {

                BDBG_WRN(("************************************************************************")) ;
                BDBG_WRN(("  Timed out waiting for <Report Power Status> message.  Continuing anyway."));
                BDBG_WRN(("***********************************************************************")) ;

                continue;  /* Back to the top to send another <Give Device Power Status>. */
            }

            /*******************************************************************
             *  If the TV is in Standby, then turn it on by sending an
             *  <Image View On> message.
             *******************************************************************/
            if (powerUpCount == 0 )
            {
                powerStatusValid = false;


                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG(("*  Sending <Image View On>"));
                BDBG_LOG(("****************************************************")) ;

                transmitMessage.destinationAddr = 0x0;
                transmitMessage.length = 1;
                transmitMessage.buffer[0] = 0x04;   /* <Image View On> */

                rc = sendCecMessage(hCec, &transmitMessage);
                BDBG_ASSERT(!rc);
            }
            BKNI_Sleep(1000);
        }

        /*******************************************************************
         *  Start out with a sanity check of the CEC interface.  We'll
         *  send a few messages and make sure we can get back a reasonable
         *  response.
         *
         *  Send a <Request Active Source> message and wait for the
         *  <Active Source> response.
         *******************************************************************/
        BKNI_Sleep(1000);

        activeSourceValid = false;


        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG(("*  Sending <Request Active Source>"));
        BDBG_LOG(("****************************************************")) ;

        transmitMessage.destinationAddr = 0xF;
        transmitMessage.length = 1;
        transmitMessage.buffer[0] = 0x85;   /* <Request Active Source> */

        rc = sendCecMessage(hCec, &transmitMessage);
        BDBG_ASSERT(!rc);

        for (loops = 0; loops < 10; loops++) {
            if ( activeSourceValid ) break;
            BDBG_LOG(("Waiting for <Active Source>"));
            BKNI_Sleep(500);
        }

        if ( activeSourceValid)
        {

            BDBG_LOG(("************************************************************************")) ;
            BDBG_LOG(("*  Got <Active Source>: Physical address: 0x%02x 0x%02x",
                                    activeSourcePhysicalAddress[0],
                                    activeSourcePhysicalAddress[1]));
            BDBG_LOG(("************************************************************************")) ;

        }
        else
        {

            BDBG_WRN(("************************************************************************")) ;
            BDBG_WRN(("  Timed out waiting for <Active Source> message.  Continuing anyway."));
            BDBG_WRN(("***********************************************************************")) ;

        }

        /*******************************************************************
         *  Send a <Get CEC Version> message and wait for the
         *  <CEC Version> response.
         *******************************************************************/
        cecVersionValid = false;


        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG(("*  Sending <Get CEC Version>"));
        BDBG_LOG(("****************************************************")) ;

        transmitMessage.destinationAddr = 0x0;
        transmitMessage.length = 1;
        transmitMessage.buffer[0] = 0x9F;   /* <Get CEC Version> */

        rc = sendCecMessage(hCec, &transmitMessage);
        BDBG_ASSERT(!rc);

        for (loops = 0; loops < 10; loops++) {
            if ( cecVersionValid ) break;
            BDBG_LOG(("Waiting for <CEC Version>"));
            BKNI_Sleep(500);
        }

        if ( cecVersionValid)
        {

            BDBG_LOG(("************************************************************************")) ;
            BDBG_LOG(("*  Got <CEC Version>: version code is 0x%x => %s", cecVersionValue,
                                                    cecVersionValue==0x4 ? "Version 1.3a"        :
                                                    cecVersionValue==0x5 ? "Version 1.4 or 1.4a" :
                                                                           "<Reserved>"       ));
            BDBG_LOG(("************************************************************************")) ;

        }
        else
        {
            BDBG_WRN(("************************************************************************")) ;
            BDBG_WRN(("  Timed out waiting for <CEC Version> message.  Continuing anyway."));
            BDBG_WRN(("***********************************************************************")) ;
        }

        /*******************************************************************
         *  Okay, done with the sanity check, now let's get down to work.
         *  Send a <Set System Audio Mode> directly to the TV to make sure.
         *  it supports the command.                                                              .
         *******************************************************************/

        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG(("*  Sending <Set System Audio Mode> [On]"));
        BDBG_LOG(("****************************************************")) ;

        transmitMessage.destinationAddr = 0;
        transmitMessage.length = 2;
        transmitMessage.buffer[0] = 0x72;   /* <Set System Audio Mode> */
        transmitMessage.buffer[1] = 0x01;   /* 01 => System Audio Status="On" */

        rc = sendCecMessage(hCec, &transmitMessage);
        BDBG_ASSERT(!rc);

        BKNI_Sleep(500);

        /*******************************************************************
         *  Now do a broadcast of the <Set System Audio Mode> to all
         *  devices.  This should cause the TV to mute it's speakers.
         *******************************************************************/

        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG(("*  Broadcasting <Set System Audio Mode> [On]"));
        BDBG_LOG(("****************************************************")) ;

        transmitMessage.destinationAddr = 0xf;
        transmitMessage.length = 2;
        transmitMessage.buffer[0] = 0x72;   /* <Set System Audio Mode> */
        transmitMessage.buffer[1] = 0x01;   /* 01 => System Audio Status="On" */

        rc = sendCecMessage(hCec, &transmitMessage);
        BDBG_ASSERT(!rc);

        BKNI_Sleep(500);

        for (initiateArcCount=0 ; initiateArcCount<5 ; initiateArcCount++)
        {
            if (reportArcInitiated) break;

            /*******************************************************************
             *  Now send an <Initiate ARC> message to the TV to ask it to
             *  start outputting on the Audio Return Channel.  The TV
             *  should respond with a <Report ARC Initiated> message.
             *******************************************************************/
            if (! reportArcInitiated)
            {

                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG(("*  Sending <Initiate ARC>"));
                BDBG_LOG(("****************************************************")) ;

                transmitMessage.destinationAddr = 0;
                transmitMessage.length = 1;
                transmitMessage.buffer[0] = 0xC0;   /* <Initiate ARC> */
                rc = sendCecMessage(hCec, &transmitMessage);
                BDBG_ASSERT(!rc);
            }

            /*******************************************************************
             *  Wait for the <Report ARC Initiated> message.
             *******************************************************************/

            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Waiting for <Report ARC Initiated>"));
            BDBG_LOG(("****************************************************")) ;

            for (loops = 0; loops < 20; loops++)
            {
                if (reportArcInitiated)
                    break;
                BDBG_LOG(("Waiting for <Report ARC Initiated>"));
                BKNI_Sleep(500);
            }

            if ( reportArcInitiated )
            {

                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG(("*  Got <Report ARC Initiated>"));
                BDBG_LOG(("****************************************************")) ;

                break;
            }

            BDBG_ERR(("Timed out waiting for <Report ARC Initiated>."));
            BKNI_Sleep(1000);
        }   /* Loop back and send <Initiate ARC> again. */

        /*******************************************************************
         *  So far, so good... The TV should be sending its audio to us
         *  over the ARC.  Now set up our audio to capture the SPDIF input
         *  and play it to our outputs
         *******************************************************************/

        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG(("*  Starting SPDIF Input"));
        BDBG_LOG(("****************************************************")) ;


        doSpdifInput();
    }

    /*******************************************************************
     *  All done, time to cleanup and exit.
     *******************************************************************/
done:

    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void) {printf("no Audio ARC support\n");return 0;}
#endif
