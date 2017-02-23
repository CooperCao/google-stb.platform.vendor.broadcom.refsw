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
/* Nexus example app: Play audio out to the HDMI input's Audio Return Channel. */

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_video_input.h"
#if NEXUS_HAS_CEC
#include "nexus_cec.h"
#endif
#if NEXUS_NUM_HDMI_INPUTS
#include "nexus_hdmi_input.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_output.h"
#include "nexus_audio_playback.h"
#if NEXUS_NUM_AUDIO_RETURN_CHANNEL
#include "nexus_audio_return_channel.h"
#endif
#endif
#include "blst_queue.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BKNI_EventHandle event;
bool deviceReady        = false ;
bool initiateArc        = false ;

bool    powerStatusValid = false;
uint8_t powerStatusValue;

#if NEXUS_HAS_HDMI_INPUT  &&  NEXUS_NUM_AUDIO_RETURN_CHANNEL && NEXUS_HAS_CEC
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
#endif


#if NEXUS_HAS_HDMI_INPUT  &&  NEXUS_NUM_AUDIO_RETURN_CHANNEL && NEXUS_HAS_CEC && NEXUS_NUM_CEC && NEXUS_HAS_AUDIO

BDBG_MODULE(audio_arc_output);


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
              pElem->idx, (void *)pElem,
              pElem->cecMessage.initiatorAddr,
              pElem->cecMessage.destinationAddr,
              pElem->cecMessage.length,
              msgBuffer, (void *)&pElem->cecMessage));
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
    NEXUS_CecHandle     hCec = (NEXUS_CecHandle)context;
    NEXUS_CecStatus     status;

    BSTD_UNUSED(param);
    BKNI_SetEvent(event);
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_LOG(("CEC Device Ready: Logical Address %d Acquired", status.logicalAddress )) ;

    deviceReady = true;
}


void msgReceived_callback(void *context, int param)
{
    NEXUS_CecHandle             hCec = (NEXUS_CecHandle)context;
    NEXUS_CecStatus             status;
    NEXUS_CecReceivedMessage    receivedMessage;
    NEXUS_Error                 rc ;
    uint8_t                     i, j ;
    char                        msgBuffer[128] ;

    BSTD_UNUSED(param);
    BKNI_SetEvent(event);
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
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Polling Message>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
        }

        else if (pMsg[0] == 0x83)    /* <Give Physical Address> */
        {
            NEXUS_CecMessage transmitMessage;

            checkRecvMsgSize(0x83, 1, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Give Physical Address>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Report Physical Address>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
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
        else if (pMsg[0] == 0xC0)    /* <InitiateARC> */
        {
            checkRecvMsgSize(0xC0, 1, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <InitiateARC>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            initiateArc = true;
        }
        else if (pMsg[0] == 0x84)    /* <Report Physical Address> */
        {
            checkRecvMsgSize(0x84, 4, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Report Physical Address> : 0x%02x 0x%02x 0x%02x",  pMsg[1], pMsg[2], pMsg[3] ));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
        }
        else if (pMsg[0] == 0x87)    /* <Device Vendor ID> */
        {
            checkRecvMsgSize(0x87, 4, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received  <Device Vendor ID> : 0x%02x 0x%02x 0x%02x",  pMsg[1], pMsg[2], pMsg[3] ));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
        }
        else if (pMsg[0] == 0x82)    /* <ActiveSource> */
        {
            checkRecvMsgSize(0x82, 3, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Active Source>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
        }
        else if (pMsg[0] == 0x9E)    /* <CEC Version> */
        {
            checkRecvMsgSize(0x9E, 2, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <CEC Version>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
        }
        else if (pMsg[0] == 0x90)    /* <Report Power Status> */
        {
            checkRecvMsgSize(0x90, 2, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Report Power Status>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            powerStatusValue = pMsg[1];
            powerStatusValid = true;
        }
        else if (pMsg[0] == 0x8C)    /* <Give Device Vendor ID> */
        {
            NEXUS_CecMessage transmitMessage;

            checkRecvMsgSize(0x8C, 1, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Give Device Vendor ID>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Device Vendor ID>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
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
        else if (pMsg[0] == 0x71)    /* <Give Audio Status> */
        {
            NEXUS_CecMessage transmitMessage;

            checkRecvMsgSize(0x71, 1, msgLen);
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Give Audio Status>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Report Audio Status>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
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
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Give System Audio Mode Status>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <System Audio Mode Status>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            transmitMessage.initiatorAddr = status.logicalAddress;
            transmitMessage.destinationAddr = 0x0;
            transmitMessage.length = 2;
            transmitMessage.buffer[0] = 0x7E;   /* <System Audio Mode Status> */
            transmitMessage.buffer[1] = 1;      /* 1 => On */

            rc = sendCecMessage(hCec, &transmitMessage);
            BDBG_ASSERT(!rc);
        }

        else if (pMsg[0] == 0x89)    /* <Vendor Command> */
        {
            NEXUS_CecMessage transmitMessage;

            /* checkRecvMsgSize(0x89, 1, msgLen); */
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Vendor Command>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Feature Abort>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
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
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Received <Vendor Command with ID>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Feature Abort>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
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
            BDBG_LOG((" "));
            BDBG_WRN(("****************************************************")) ;
            BDBG_WRN(("*  Received Unexpected Message: 0x%02x !!! ", pMsg[0]));
            BDBG_WRN(("****************************************************")) ;
            BDBG_LOG((" "));
        }
    }
}

void msgTransmitted_callback(void *context, int param)
{
    NEXUS_CecHandle     hCec = (NEXUS_CecHandle)context;
    NEXUS_CecStatus     status;

    BSTD_UNUSED(param);
    BKNI_SetEvent(event);
    NEXUS_Cec_GetStatus(hCec, &status);

    BDBG_LOG(("CEC Send Complete: Ack:%u", status.transmitMessageAcknowledged ));
}

static void playback_data_callback(void *pParam1, int param2)
{
    /*
    printf("Data callback - channel 0x%08x\n", (unsigned)pParam1);
    */
    BSTD_UNUSED(pParam1);
    BSTD_UNUSED(param2);

    BKNI_SetEvent(event);
}

int doSpdifOutput(void)
{
    BERR_Code errCode;

    NEXUS_PlatformConfiguration config;
    NEXUS_AudioPlaybackHandle playbackHandle;
    NEXUS_AudioReturnChannelHandle hArc;
    NEXUS_AudioPlaybackStartSettings playbackSettings;

    size_t bytesToPlay = 48000*4*2*60;    /* 48 kHz, 4 bytes/sample, 120 seconds (recommended unittest limit) */
    size_t bytesPlayed=0;
    size_t offset=0;
    int16_t *pBuffer;
    size_t bufferSize;

    /* 1KHz sine wave at 48 KHz (0dBFS 16-bit/sample) */
    static int16_t samples[48] =
    {
    0, 4276, 8480, 12539, 16383, 19947, 23169, 25995, 28377, 30272, 31650, 32486, 32767, 32486, 31650, 30272, 28377, 25995, 23169, 19947, 16383, 12539, 8480, 4276,
    0,-4277,-8481,-12540,-16384,-19948,-23170,-25996,-28378,-30273,-31651,-32487,-32767,-32487,-31651,-30273,-28378,-25996,-23170,-19948,-16384,-12540,-8481,-4277
    };

    NEXUS_Platform_GetConfiguration(&config);

    BKNI_Printf("Opening Playback\n");
    playbackHandle = NEXUS_AudioPlayback_Open(0, NULL);
    BDBG_ASSERT(playbackHandle);

    /* Connect DAC to playback */
    {
        NEXUS_AudioOutputSettings outputSettings;

        BKNI_Printf("Adding Playback to DAC\n");

        /* Change DAC settings here (if desired) */
        NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
        outputSettings.pll = 0;
        NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);

        /* Connect the Playback to the input of the DAC output */
        errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]),
                                             NEXUS_AudioPlayback_GetConnector(playbackHandle));
        BDBG_ASSERT(!errCode);
    }

    #if NEXUS_NUM_SPDIF_OUTPUTS
    {
        NEXUS_AudioOutputSettings outputSettings;

        BKNI_Printf("Adding Playback to SPDIF\n");

        /* Change Spdif output settings here (if desired) */
        NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]), &outputSettings);
        outputSettings.defaultSampleRate = 48000;
        outputSettings.nco = NEXUS_AudioOutputNco_eMax;
        errCode = NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]), &outputSettings);
        BDBG_ASSERT(!errCode);

        /* Connect the Playback to the input of the Spdif output */
        errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]),
                                   NEXUS_AudioPlayback_GetConnector(playbackHandle));
        BDBG_ASSERT(!errCode);
    }
    #endif

    #if NEXUS_NUM_AUDIO_RETURN_CHANNEL
    {
        NEXUS_AudioOutputSettings outputSettings;
        NEXUS_AudioReturnChannelSettings    arcSettings;

        BKNI_Printf("Opening ARC\n");
        NEXUS_AudioReturnChannel_GetDefaultSettings(&arcSettings);
        hArc = NEXUS_AudioReturnChannel_Open(0, &arcSettings);
        BDBG_ASSERT(hArc);

        BKNI_Printf("Adding Playback to ARC\n");

        /* Change Spdif output settings here (if desired) */
        NEXUS_AudioOutput_GetSettings(NEXUS_AudioReturnChannel_GetConnector(hArc), &outputSettings);
        outputSettings.defaultSampleRate = 48000;
        outputSettings.nco = NEXUS_AudioOutputNco_eMax;
        errCode = NEXUS_AudioOutput_SetSettings(NEXUS_AudioReturnChannel_GetConnector(hArc), &outputSettings);
        BDBG_ASSERT(!errCode);

        /* Connect the Playback to the input of the Spdif output */
        errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioReturnChannel_GetConnector(hArc),
                                   NEXUS_AudioPlayback_GetConnector(playbackHandle));
        BDBG_ASSERT(!errCode);
    }
    #endif


    NEXUS_AudioPlayback_GetDefaultStartSettings(&playbackSettings);
    playbackSettings.signedData = true;
    playbackSettings.dataCallback.callback = playback_data_callback;
    playbackSettings.dataCallback.context = playbackHandle;
    playbackSettings.dataCallback.param = (int)event;
    playbackSettings.sampleRate = 48000;
    playbackSettings.bitsPerSample = 16;
    playbackSettings.stereo = true;

    printf("Stream sample rate %d, %d bits per sample, %s\n", playbackSettings.sampleRate, playbackSettings.bitsPerSample, playbackSettings.stereo?"stereo":"mono");

    {
        NEXUS_AudioOutputSettings outputSettings;

        NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
        outputSettings.defaultSampleRate = playbackSettings.sampleRate;
        errCode = NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
        BDBG_ASSERT(!errCode);

        #if NEXUS_NUM_SPDIF_OUTPUTS
            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]), &outputSettings);
            outputSettings.defaultSampleRate = playbackSettings.sampleRate;
            errCode = NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]), &outputSettings);
            BDBG_ASSERT(!errCode);
        #endif
    }

    BKNI_Printf("Starting Playback\n" );

    playbackSettings.sampleRate = 48000;

    errCode = NEXUS_AudioPlayback_Start(playbackHandle, &playbackSettings);
    BDBG_ASSERT(!errCode);

    do
    {
        unsigned i;

        /* Check available buffer space */
        errCode = NEXUS_AudioPlayback_GetBuffer(playbackHandle, (void **)&pBuffer, &bufferSize);
        BDBG_ASSERT(!errCode);

        if (bufferSize)
        {
            /* Copy samples into buffer */
            bufferSize /= 4;
            for ( i=0; i<bufferSize; i++,bytesPlayed+=4 )
            {
                const int  step = 1;        /* use 1 for 1 KHz, 2 for 2 KHz, 4 for for 4 KHz, etc. */
                pBuffer[2*i] = pBuffer[(2*i)+1] = samples[offset];
                offset += step;
                if ( offset >= 48 )
                {
                    offset -= 48;
                }
            }
            bufferSize *= 4;

            errCode = NEXUS_AudioPlayback_WriteComplete(playbackHandle, bufferSize);
            BDBG_ASSERT(!errCode);

            if ((bytesPlayed/1024) % 100 == 0) {
                BDBG_WRN(("%d of %d KB played", bytesPlayed/1024, bytesToPlay/1024));
            }
        }
        else
        {
            errCode = BKNI_WaitForEvent(event, 5000);
            BDBG_ASSERT(!errCode);
            }
    } while (bytesPlayed < bytesToPlay);

    printf("Waiting for Playback buffer to empty\n");

    for (;;)
    {
        NEXUS_AudioPlaybackStatus status;
        NEXUS_AudioPlayback_GetStatus(playbackHandle, &status);
        if ( status.queuedBytes > 0 )
        {
            BKNI_Sleep(100);
        }
        else break;
    }

    printf("Stopping playback\n");
    BKNI_Printf("Stopping Playback\n" );

    NEXUS_AudioPlayback_Stop(playbackHandle);

    BKNI_Printf("Removing Playback from DAC\n");

    NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]),
                                  NEXUS_AudioPlayback_GetConnector(playbackHandle));

    #if NEXUS_NUM_SPDIF_OUTPUTS
        BKNI_Printf("Removing Playback from SPDIF\n");
        NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]),
                                   NEXUS_AudioPlayback_GetConnector(playbackHandle));
    #endif

    BKNI_Printf("Closing Playback\n");
    NEXUS_AudioPlayback_Close(playbackHandle);

    printf("doSpdifOutput: Returning\n");
    return 0;
}


int main(int argc, char **argv)
{
    NEXUS_HdmiOutputHandle hdmiOutput;
    NEXUS_HdmiOutputStatus hdmiStatus;

    NEXUS_CecHandle        hCec;

    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings ;
    unsigned hdmiInputIndex = 0;
    unsigned loopCount = 0 ;
    bool equalization = true;
    int curarg = 1;
    NEXUS_Error rc;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-port") && curarg+1<argc) {
            hdmiInputIndex = atoi(argv[++curarg]) ;
        }
        else if (!strcmp(argv[curarg], "-no_equalization")) {
            equalization = false;
        }
        else if (!strcmp(argv[curarg], "-loop") && curarg+1<argc) {
            loopCount = atoi(argv[++curarg]) ;
        }
        curarg++;
    }

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
    BKNI_CreateEvent(&event);

    /* Bring up all modules for a platform in a default configuration for this platform */

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

    hdmiOutput = platformConfig.outputs.hdmi[0] ;
    hCec = platformConfig.outputs.cec[0];

    {
        NEXUS_CecSettings cecSettings;

        NEXUS_Cec_GetSettings(hCec, &cecSettings);
        cecSettings.messageReceivedCallback.callback = msgReceived_callback ;
        cecSettings.messageReceivedCallback.context = hCec;

        cecSettings.messageTransmittedCallback.callback = msgTransmitted_callback;
        cecSettings.messageTransmittedCallback.context = hCec;

        cecSettings.logicalAddressAcquiredCallback.callback = deviceReady_callback ;
        cecSettings.logicalAddressAcquiredCallback.context = hCec;

        cecSettings.physicalAddress[0]= (hdmiStatus.physicalAddressA << 4) | hdmiStatus.physicalAddressB;
        cecSettings.physicalAddress[1]= (hdmiStatus.physicalAddressC << 4) | hdmiStatus.physicalAddressD;

        BDBG_LOG((" "));
        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG(("*  Using CEC Physical Address: %X.%X.%X.%X",
            (cecSettings.physicalAddress[0] & 0xF0) >> 4,
            (cecSettings.physicalAddress[0] & 0x0F),
            (cecSettings.physicalAddress[1] & 0xF0) >> 4,
            (cecSettings.physicalAddress[1] & 0x0F) )) ;
        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG((" "));

        cecSettings.deviceType = 5;     /* 3 => Tuner, 5 => Audio System */

        rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
        BDBG_ASSERT(!rc);

        /* Enable CEC core */
        NEXUS_Cec_GetSettings(hCec, &cecSettings);
            cecSettings.enabled = true;
            rc = NEXUS_Cec_SetSettings(hCec, &cecSettings);
        BDBG_ASSERT(!rc);
    }

    BDBG_LOG((" "));
    BDBG_LOG(("****************************************************")) ;
    BDBG_LOG(("*  Checking for CEC Logical Address"));
    BDBG_LOG(("****************************************************")) ;
    BDBG_LOG((" "));
    {
        unsigned            loops;
        NEXUS_CecStatus     cecStatus;

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

        BDBG_LOG((" "));
        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG(("*  CEC Logical Address is %u", cecStatus.logicalAddress));
        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG((" "));
    }

    {
        unsigned            loops;
        unsigned            powerUpCount;
        NEXUS_CecMessage    transmitMessage;

        for (powerUpCount=0 ; powerUpCount<5 ; powerUpCount++)
        {

            /*******************************************************************
             *  Send a <Give Device Power Status> message to the TV  and wait
             *  for the <Report Power Status> response.
             *******************************************************************/
            powerStatusValid = false;

            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Sending <Give Device Power Status>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            transmitMessage.destinationAddr = NEXUS_CecDeviceType_eAudioSystem;
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
                BDBG_LOG((" "));
                BDBG_LOG(("************************************************************************")) ;
                BDBG_LOG(("*  Got <Report Power Status>: power code is 0x%x => %s", powerStatusValue,
                                                        powerStatusValue==0x0 ? "On"        :
                                                        powerStatusValue==0x1 ? "Standby" :
                                                        powerStatusValue==0x2 ? "In transition: Standby -> On" :
                                                        powerStatusValue==0x3 ? "In transition: On -> Standby" :
                                                                                "<Reserved>"       ));
                BDBG_LOG(("************************************************************************")) ;
                BDBG_LOG((" "));
                if (powerStatusValid && powerStatusValue==0x0) break;   /* Great!  TV is powered up! */
            }
            else
            {
                BDBG_WRN((" "));
                BDBG_WRN(("************************************************************************")) ;
                BDBG_WRN(("  Timed out waiting for <Report Power Status> message.  Trying again...."));
                BDBG_WRN(("***********************************************************************")) ;
                BDBG_WRN((" "));
                continue;  /* Back to the top to send another <Give Device Power Status>. */
            }

            /*******************************************************************
             *  If the TV is in Standby, then turn it on by sending an
             *  <Image View On> message.
             *******************************************************************/
            if (powerUpCount == 0 )
            {
                powerStatusValid = false;

                BDBG_LOG((" "));
                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG(("*  Sending <Image View On>"));
                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG((" "));
                transmitMessage.destinationAddr = NEXUS_CecDeviceType_eAudioSystem;
                transmitMessage.length = 1;
                transmitMessage.buffer[0] = 0x04;   /* <Image View On> */

                rc = sendCecMessage(hCec, &transmitMessage);
                BDBG_ASSERT(!rc);
            }
            BKNI_Sleep(1000);
        }
    }

    {
        unsigned            requestInitiateArcCount;
        unsigned            loops;
        NEXUS_CecMessage    transmitMessage;

        for (requestInitiateArcCount=0 ; requestInitiateArcCount<5 ; requestInitiateArcCount++)
        {
            if (initiateArc) break;

            /*******************************************************************
             *  Now send a <Request ARC Initiation> message to the TV to ask it
             *  to send an <Initiate ARC> message to us.
             *******************************************************************/
            if (! initiateArc)
            {
                BDBG_LOG((" "));
                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG(("*  Sending <Request ARC Initiation>"));
                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG((" "));
                transmitMessage.destinationAddr = 5;
                transmitMessage.length = 1;
                transmitMessage.buffer[0] = 0xC3;   /* <Initiate ARC> */
                rc = sendCecMessage(hCec, &transmitMessage);
                BDBG_ASSERT(!rc);
            }

            /*******************************************************************
             *  Wait for the <Initiate ARC> message.
             *******************************************************************/
            BDBG_LOG((" "));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG(("*  Waiting for <Initiate ARC>"));
            BDBG_LOG(("****************************************************")) ;
            BDBG_LOG((" "));
            for (loops = 0; loops < 20; loops++)
            {
                if (initiateArc)
                    break;
                BDBG_LOG(("Waiting for <Initiate ARC>"));
                BKNI_Sleep(500);
            }

            if ( initiateArc )
            {
                BDBG_LOG((" "));
                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG(("*  Got <Initiate ARC>"));
                BDBG_LOG(("****************************************************")) ;
                BDBG_LOG((" "));
                break;
            }

            BDBG_ERR(("Timed out waiting for <Initiate ARC>."));
            BKNI_Sleep(1000);
        }   /* Loop back and send <Request ARC Initiation> again. */
    }

    /*******************************************************************
     *  So far, so good... The Amplifier wants us to turn on ARC output,
     *  so send the <Report ARC Initiated>, then start SPDIF output
     *  through the ARC output.
     *******************************************************************/
    {
        NEXUS_CecMessage    transmitMessage;

        BDBG_LOG((" "));
        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG(("*  Sending <Report ARC Initiated>"));
        BDBG_LOG(("****************************************************")) ;
        BDBG_LOG((" "));
        transmitMessage.destinationAddr = 5;
        transmitMessage.length = 1;
        transmitMessage.buffer[0] = 0xC1;   /* <Report ARC Initiated> */
        rc = sendCecMessage(hCec, &transmitMessage);
        BDBG_ASSERT(!rc);
    }


    doSpdifOutput();



    curarg = 0;
    while ((unsigned) curarg <= loopCount) {

           BKNI_Sleep(1000);

        if (loopCount)
            curarg++ ;
    }

    /*******************************************************************
     *  All done, time to cleanup and exit.
     *******************************************************************/
done:
    NEXUS_Platform_Uninit();
    BKNI_DestroyEvent(event);
    return 0;
}
#else
int main(void) {printf("no hdmi_input, ARC, CEC, or audio support\n");return 0;}
#endif
