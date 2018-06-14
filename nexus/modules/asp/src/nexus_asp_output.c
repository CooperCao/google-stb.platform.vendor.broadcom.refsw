/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_asp_module.h"
#include "nexus_core_utils.h"
#include "nexus_client_resources.h"
#if NEXUS_HAS_SECURITY
#include "bhsm.h"
#include "bhsm_keyslot.h"
#include "priv/nexus_security_priv.h"
#include "nexus_security.h"
#endif
#include "basp_fw_api.h"
#include "basp.h"
#include "priv/nexus_core.h"
#include "bchp_xpt_rave.h"
#include "bxpt_capabilities.h"
#include "bchp_asp_mcpb.h"
#include "bchp_asp_mcpb_ch0.h"
#include "bchp_asp_mcpb_ch1.h"
#include "bchp_asp_edpkt_core.h"

BDBG_MODULE(nexus_asp_output);

#define FMT_PREFIX "%s:%d "
#define ARG_PREFIX , BSTD_FUNCTION, BSTD_LINE

/* Setting this to non-zero will read ASP messages by polling
 * instead of using BASP interrupt callbacks. */
#define  USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS   0

#define NEXUS_NUM_ASP_CHANNELS BASP_MAX_NUMBER_OF_CHANNEL
                                    /* TODO: move this to a chip specific header file such as: */
                                    /* platforms/97272/include/nexus_platform_features.h */
                                    /* Doesn't belong in platforms/common/include/nexus_platform_generic_features_priv.h */
#define NEXUS_ASP_BLOCK_SIZE 192
#define NEXUS_ASP_NUM_BLOCKS_IN_A_CHUNK 512

#define NEXUS_AspOutputState_toString(state)  (state==NEXUS_AspOutputState_eIdle            ? "Idle"      :     \
                                               state==NEXUS_AspOutputState_eConnected       ? "Connected" :     \
                                               state==NEXUS_AspOutputState_eStreaming       ? "Streaming" :     \
                                                                                              "Invalid!!!")

/* Per ASP Output Channel Structure. */
typedef struct NEXUS_AspOutput
{
    NEXUS_OBJECT(NEXUS_AspOutput);
    NEXUS_AspOutputState                state;
    bool                                finishRequested;
    bool                                finishCompleted;
    bool                                connectionReset;
    bool                                networkTimeout;
    bool                                remoteDoneSending;

    int                                 channelNum;
    BASP_ChannelHandle                  hChannel;
    NEXUS_AspStreamingProtocol          protocol;

    NEXUS_AspOutputCreateSettings       createSettings;
    NEXUS_AspOutputSettings             settings;
    NEXUS_AspOutputStartSettings        startSettings;
    NEXUS_AspOutputConnectHttpSettings  connectHttpSettings;
    NEXUS_AspTcpSettings                tcpSettings;

    NEXUS_AspBuffer                     writeFifo;
    NEXUS_AspBuffer                     reTransmitFifo;
    NEXUS_AspBuffer                     receiveFifo;
    NEXUS_AspBuffer                     miscBuffer;

    unsigned                            writeFifoLength;    /* # of bytes filled-into the write buffer. */
    unsigned char                       *pRcvdPayload;      /* Starting position of the last received payload */
    unsigned                            rcvdPayloadLength;

    NEXUS_AspOutputTcpStatus            initialStatus;
    NEXUS_AspOutputTcpStatus            currentStatus;

    NEXUS_IsrCallbackHandle             hEndOfStreamingCallback;
    NEXUS_IsrCallbackHandle             hRemoteDoneSendingCallback;
    NEXUS_IsrCallbackHandle             hBufferReadyCallback;
    NEXUS_IsrCallbackHandle             hHttpRequestDataReadyCallback;

    BKNI_EventHandle                    hMessageReceiveEvent;

    struct
    {
        unsigned                        finalSendSequenceNumber;
        unsigned                        finalRecvSequenceNumber;
    } tcpState;

    bool                                gotStartResponse;
    bool                                gotStopResponse;
    bool                                gotPayloadConsumedResponse;

    NEXUS_AspOutputDrmType              drmType;
    NEXUS_AspOutputDtcpIpSettings       dtcpIpSettings;
#ifdef NEXUS_HAS_SECURITY
    BHSM_KeyslotHandle                  hKeySlot;
    unsigned                            extKeyTableSlotIndex;
#endif
} NEXUS_AspOutput;

#if NEXUS_HAS_SECURITY
#define LOCK_SECURITY() NEXUS_Module_Lock(g_NEXUS_asp.settings.modules.security)
#define UNLOCK_SECURITY() NEXUS_Module_Unlock(g_NEXUS_asp.settings.modules.security)
#endif

#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
static void NEXUS_ProcessMsgFromFwCallbackByTimer(void *pContext);
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
static void NEXUS_AspOutput_ProcessMsgFromFwCallback_isr(void *pContext, int param);

#define APP_TS_PKT_LEN              188
#define APP_TS_PKTS_IN_IP_PKT       7
#define APP_NUM_IP_PKTS             10
#define APP_TCP_START_SEQ_NUM       1000
#define APP_TCP_START_ACK_NUM       512

#define MAX_ASP_MESSAGE_RESPONSE_TIME_IN_MS (1000)


static NEXUS_Error NEXUS_AspOutput_DoAspMessage(
    NEXUS_AspOutputHandle   hAspOutput,
    BASP_MessageType        messageType,
    BASP_Pi2Fw_Message      *pMessageToSend,
    bool                    *responseCompleteFlag)
{
    BERR_Code   rc = BERR_SUCCESS;
    NEXUS_Time  timeStart;
    NEXUS_Time  timeEnd;
    long        timeElapsed;

    static long timeMax = 0;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pMessageToSend);
    BDBG_ASSERT(responseCompleteFlag);

    NEXUS_Time_Get(&timeStart);

    /* Send StreamOut message to ASP. */
    BDBG_MSG((FMT_PREFIX "hAspOutput=%p Sending MessageType %u..."
              ARG_PREFIX, (void *)hAspOutput, messageType));

    *responseCompleteFlag = false;
    BKNI_ResetEvent(hAspOutput->hMessageReceiveEvent);

    rc = BASP_Channel_SendMessage( hAspOutput->hChannel,
                                    messageType,
                                    BASP_ResponseType_eAckRespRequired,
                                    pMessageToSend);
    if (rc != BERR_SUCCESS) {BERR_TRACE(rc); goto error;}

    BDBG_MSG((FMT_PREFIX "hAspOutput=%p MessageType %u sent..."
              ARG_PREFIX, (void *)hAspOutput, messageType));

    /* Wait for the response to the message. */
    for (;;)
    {
        rc = BKNI_WaitForEvent(hAspOutput->hMessageReceiveEvent, MAX_ASP_MESSAGE_RESPONSE_TIME_IN_MS);
        if (*responseCompleteFlag) { break; }
        if (rc != BERR_SUCCESS) {BERR_TRACE(rc); goto error;}

        BDBG_WRN((FMT_PREFIX "hAspOutput=%p MessageType %u, Event set, but no message response!"
                  ARG_PREFIX, (void *)hAspOutput, messageType));
    }

    NEXUS_Time_Get(&timeEnd);
    timeElapsed = NEXUS_Time_Diff(&timeEnd, &timeStart);
    if (timeElapsed > timeMax) {timeMax = timeElapsed; }

    BDBG_LOG((FMT_PREFIX "hAspOutput=%p MessageType %u response received, ET=%ld ms, Max ET=%ld ms."
              ARG_PREFIX, (void *)hAspOutput, messageType, timeElapsed, timeMax));

error:
    return rc;

}


static void updateStats(
    NEXUS_AspOutputHandle               hAspOutput,
    BREG_Handle                         hReg,
    int                                 channelNumber,
    NEXUS_AspOutputTcpStatus            *pInitialStats,
    NEXUS_AspOutputTcpStatus            *pStatus
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    /* XPT Rave  */
    {
        uint64_t validOffset, readOffset, baseOffset, endOffset;
        unsigned cdbSize, cdbDepth;

        readOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR);
        validOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_VALID_PTR);
        baseOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR);
        endOffset = BREG_Read64(hReg, BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR);
        cdbSize = endOffset - baseOffset;
        if (validOffset > readOffset)
        {
            cdbDepth = validOffset - readOffset;
        }
        else
        {
            cdbDepth = validOffset + cdbSize - readOffset;
        }
        pStatus->raveCtxDepthInBytes = cdbDepth;
        pStatus->raveCtxSizeInBytes = cdbSize;
    }

    {
#if 0
        /* EPKT Seq # */
        regAddr = 0x18440e0;    /* ASP_EPKT_CORE_CH00_CH_SEQ_NUM */
        regAddr = 0x18440e4;    /* ASP_EPKT_CORE_CH00_CH_RETX_SEQ_NUM */
        /* EPKT ReTx related  */
        checkRegContent(hReg, "RetxBufLevel", 0x18440e8, 0);
        checkRegContent(hReg, "RetxBufSeq", 0x18440ec, 0);
        checkRegContent64(hReg, "RetxBase", 0x18440f0, 0);
        checkRegContent64(hReg, "RetxEnd", 0x18440f8, 0);
        checkRegContent64(hReg, "RetxWrite", 0x1844100, 0);
        checkRegContent64(hReg, "RetxValid", 0x1844108, 0);
        checkRegContent64(hReg, "RetxRead", 0x1844110, 0);
#endif
    }

#if 0
        /* Dump the first 10 on CHIP descriptors that are used to feed data from Rave context to MCPB. */
        {
            int i;
            regAddr = 0x1859000;  /* ASP_MCPB_ON_CHIP_DESC_DATA */
            regExpected = 0;
            for (i=0; i<10; i++)
            {
                regAddr = 0x1859000 + i*4;  /* ASP_MCPB_ON_CHIP_DESC_DATA */
                checkRegContent(hReg, "AspMcpbOnChipDesc", regAddr, regExpected);
            }

            regAddr = 0x1851030;  /* ASP_MCPB_CH0_DMA_BUFF_WR_ADDR */
            regExpected = 0;
            checkRegContent(hReg, "AspMcpbCh0DmaWrAddr", regAddr, regExpected);

            /* Read DCPM_DATA_ADDR: it should be set to Valid pointer. */
            regAddr = 0x1851238; /* ASP_MCPB_CH0_DCPM_DATA_ADDR */
            regExpected = 0;
            checkRegContent(hReg, "AspMcpbCh0DpcmDataAddr", regAddr, regExpected);
        }
#endif

    /* MCPB  */
    {
        uint32_t mcpbChFieldOffset;
        uint32_t mcpbChannelSize = BCHP_ASP_MCPB_CH1_DMA_DESC_ADDR - BCHP_ASP_MCPB_CH0_DMA_DESC_ADDR;
        uint32_t newValue, curValue;

        /* Find offset for the ASP MCPB channel being used. */
        mcpbChFieldOffset = BCHP_ASP_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + (channelNumber * mcpbChannelSize);
        newValue = BREG_Read32(hReg, mcpbChFieldOffset);
        curValue = (uint32_t)(pStatus->mcpbConsumedInTsPkts & 0xFFFFFFFF);
        pStatus->mcpbConsumedInTsPkts += newValue - curValue;
        BDBG_MSG(( FMT_PREFIX "MCPBPktCounter value: new=%u cur=%u hi=%u lo=%u"
                   ARG_PREFIX, newValue, curValue, (uint32_t)(pStatus->mcpbConsumedInTsPkts >> 32), (uint32_t)(pStatus->mcpbConsumedInTsPkts&0xFFFFFFFF)));

        pStatus->mcpbConsumedInBytes = pStatus->mcpbConsumedInTsPkts * 188; /* Consider w/ timestamps. */
        pStatus->mcpbConsumedInIpPkts = pStatus->mcpbConsumedInTsPkts / 7; /* ASP Sends 7 TS packets in 1 IP packet. */

        mcpbChFieldOffset = BCHP_ASP_MCPB_CH0_ESCD_TCPIP_MAX_SEQUENCE_NUMBER + (channelNumber * mcpbChannelSize);
        pStatus->mcpbSendWindow = BREG_Read32(hReg, mcpbChFieldOffset) - BREG_Read32(hReg, mcpbChFieldOffset+4); /* max - cur seq # */

        pStatus->mcpbChEnabled = BREG_Read32(hReg, BCHP_ASP_MCPB_RUN_STATUS_0_31) & (1<<channelNumber);
        pStatus->mcpbDescFifoEmpty = BREG_Read32(hReg, BCHP_ASP_MCPB_ON_CHIP_DESC_FIFO_EMPTY_0_31) & (1<<channelNumber);

        mcpbChFieldOffset = BCHP_ASP_MCPB_CH0_DMA_DATA_BUFF_DEPTH_MONITOR + (channelNumber * mcpbChannelSize);
        pStatus->mcpbPendingBufferDepth = BREG_Read32(hReg, mcpbChFieldOffset);

        /* TODO: Add Retx related counters: ASP_MCPB_CH0_DMA_RETRANS_BUFF* */
        curValue = BREG_Read32(hReg, BCHP_ASP_MCPB_RETRANS_AV_PAUSE_STATUS_0_31);
        pStatus->mcpbAvPaused = curValue & (1<<channelNumber) ? true:false;

        curValue = BREG_Read32(hReg, BCHP_ASP_MCPB_DEBUG_14);
        pStatus->mcpbStalled = curValue >> 10 & 0x1;
    }

    /* EPKT  */
    {
        /* ASP_EPKT_CORE_CH00_CH_CONFIG_MISC */
    }

    /* EDPKT  */
    {
        uint32_t eDpktPendingPktsOffset;
        uint32_t value;
#if (BCHP_CHIP == 7278 && BCHP_VER == A0)
        pStatus->eDpktRxIpPkts = BREG_Read32(hReg, BCHP_ASP_EDPKT_CORE_DEBUG_STATUS_REG2);
#else
        pStatus->eDpktRxIpPkts = BREG_Read32(hReg, BCHP_ASP_EDPKT_CORE_RX_PKT_CNT);
#endif

        eDpktPendingPktsOffset = BCHP_ASP_EDPKT_CORE_CHANNEL_01_00_HEADER_COUNT_REG + ((channelNumber/2)*4);
        value = BREG_Read32(hReg, eDpktPendingPktsOffset);
        pStatus->eDpktPendingPkts = (channelNumber&0x1) ? (value >> 16) : (value & 0xffff);
    }


    /* Switch stats are common for all channels & are cummulative. */
    {
        pStatus->unimacTxUnicastIpPkts = BREG_Read32(hReg, 0x18414f0);
        pStatus->unimacTxUnicastIpBytes = BREG_Read32(hReg, 0x18414e8); /* TODO: consider 32bit wrap. */
        pStatus->nwSwRxFmAspInUnicastIpPkts = BREG_Read32(hReg, 0xf13ca0) - (pInitialStats ? pInitialStats->nwSwRxFmAspInUnicastIpPkts : 0);
        pStatus->nwSwTxToP0InUnicastIpPkts = BREG_Read32(hReg, 0xf100c0) - (pInitialStats ? pInitialStats->nwSwTxToP0InUnicastIpPkts : 0);
        pStatus->nwSwTxToHostInUnicastIpPkts = BREG_Read32(hReg, 0xf140c0) - (pInitialStats ? pInitialStats->nwSwTxToHostInUnicastIpPkts : 0);

        pStatus->nwSwTxToAspInUnicastIpPkts = BREG_Read32(hReg, 0xf138c0) - (pInitialStats ? pInitialStats->nwSwTxToAspInUnicastIpPkts : 0);
        pStatus->nwSwRxP0InUnicastIpPkts = BREG_Read32(hReg, 0xf104a0) - (pInitialStats ? pInitialStats->nwSwRxP0InUnicastIpPkts : 0);
        pStatus->unimacRxUnicastIpPkts = BREG_Read32(hReg, 0x1841468);
        pStatus->nwSwRxP8InUnicastIpPkts = BREG_Read32(hReg, 0xf144a0) - (pInitialStats ? pInitialStats->nwSwRxP8InUnicastIpPkts : 0);
        pStatus->nwSwRxP0InDiscards = BREG_Read32(hReg, 0xf10600) - (pInitialStats ? pInitialStats->nwSwRxP0InDiscards : 0);
    }
}


/************** Module level functions ***************/
static void unLockAndFreeBuffer(
    NEXUS_AspBuffer                         *pBuffer
    )
{
    BDBG_ASSERT(pBuffer);

    BDBG_MSG(( FMT_PREFIX "hNexusHeap=%p hMmaHeap=%p pBuffer=%p offset hi:lo=0x%x:0x%x"
               ARG_PREFIX, (void *)pBuffer->hNexusHeap, (void *)pBuffer->hMmaHeap, (void *)pBuffer->pBuffer, (uint32_t)(pBuffer->offset>>32), (uint32_t)pBuffer->offset));

    if (pBuffer->pBuffer)
    {
        BMMA_Unlock(pBuffer->hBlock, pBuffer->pBuffer);
        pBuffer->pBuffer = NULL;
    }

    if (pBuffer->hBlock)
    {
        BMMA_Free(pBuffer->hBlock);
        pBuffer->hBlock = NULL;
    }
} /* unLockAndFreeBuffer */


static NEXUS_Error allocateAndLockBuffer(
    const NEXUS_AspOutputCreateBufferSettings  *pBufferSettings,
    bool                                        needCpuAccess,
    unsigned                                    alignmentInBytes,
    NEXUS_AspBuffer                             *pBuffer
    )
{
    NEXUS_Error             rc;
    NEXUS_HeapHandle        hHeap;

    BDBG_ASSERT(pBufferSettings);
    BDBG_ASSERT(pBuffer);

    BKNI_Memset(pBuffer, 0, sizeof(*pBuffer));

    /* TODO: Add logic to allocate from user provided context. */
    if (pBufferSettings->memory)
    {
        BDBG_MSG(( FMT_PREFIX "pBufferSettings->memory is not yet supported!"
                   ARG_PREFIX));
        rc = NEXUS_NOT_SUPPORTED;
        goto error;
    }

    /* Select the heap to use for buffer allocation. */
    hHeap = NEXUS_P_DefaultHeap(pBufferSettings->heap, NEXUS_DefaultHeapType_eFull);
    if (!hHeap) hHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;

    pBuffer->hNexusHeap = hHeap;
    pBuffer->hMmaHeap = NEXUS_Heap_GetMmaHandle(hHeap);
    BDBG_ASSERT(pBuffer->hMmaHeap);

    /* Now allocate the buffer. */
    pBuffer->hBlock = BMMA_Alloc(pBuffer->hMmaHeap, pBufferSettings->size, alignmentInBytes, NULL);
    if (!pBuffer->hBlock) { rc=BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY); goto error; }

    /* Lock the buffer for CPU & Device access. */
    if (needCpuAccess)
    {
        pBuffer->pBuffer = BMMA_Lock(pBuffer->hBlock);
        BDBG_ASSERT(pBuffer->pBuffer);
    }

    pBuffer->offset = BMMA_LockOffset(pBuffer->hBlock);
    BDBG_ASSERT(pBuffer->offset);
    pBuffer->size = pBufferSettings->size;

    BDBG_MSG(( FMT_PREFIX "size=%u needCpuAccess=%s hNexusHeap=%p hMmaHeap=%p pBuffer=%p offset hi:lo=0x%x:0x%x"
               ARG_PREFIX, pBufferSettings->size, needCpuAccess?"Y":"N", (void *)pBuffer->hNexusHeap, (void *)pBuffer->hMmaHeap, (void *)pBuffer->pBuffer, (uint32_t)(pBuffer->offset>>32), (uint32_t)pBuffer->offset));
    rc = NEXUS_SUCCESS;
error:
    return (rc);

} /* allocateAndLockBuffer */



/******************** ASP Output Channel Specific API functions ***************/
void NEXUS_AspOutput_GetDefaultCreateSettings(NEXUS_AspOutputCreateSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->reTransmitFifo.size = 1 * 1024 * 1024;   /* 1MB of Default Retransmission Queue. */
    pSettings->receiveFifo.size = 1024 * 1024/2;        /* 1/2MB of Default Read Queue. */
    pSettings->writeFifo.size = 4 * 1024;               /* 4KB of Default Write Queue. */
    pSettings->miscBuffer.size = 4 * 1024;              /* 4KB of Default Write Queue. */
}

NEXUS_AspOutputHandle NEXUS_AspOutput_Create(
    const NEXUS_AspOutputCreateSettings *pSettings
    )
{
    int                     channelNum;
    NEXUS_Error             rc;
    NEXUS_AspOutputHandle  hAspOutput = NULL;

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p"
               ARG_PREFIX, (void*)hAspOutput));

    /* Check if we have a free ASP Channel, return error otherwise. */
    for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
    {
        if (g_NEXUS_asp.hAspChannelList[channelNum] == NULL &&      /* TODO: Eventually, this will be changed to hAspInputList. */
/*          g_NEXUS_asp.hAspInputList  [channelNum] == NULL &&         TODO: Uncomment this when we implement AspInput. */
            g_NEXUS_asp.hAspOutputList [channelNum] == NULL )
        {
            break;
        }
    }
    if (channelNum == BASP_MAX_NUMBER_OF_CHANNEL)
    {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        BDBG_MSG(( FMT_PREFIX "No free ASP Channel available, max=%d"
                   ARG_PREFIX, BASP_MAX_NUMBER_OF_CHANNEL));
       return NULL;
    }
#if 0
    /* TODO: work around for channel change issue for DTCP/IP. Currently, FW is not properly closing the channel for A0. */
    {
        static int chNum=0;
        channelNum = chNum;
        chNum++;
        if (chNum >= BASP_MAX_NUMBER_OF_CHANNEL) chNum = 0;
    }
#endif

#if 0
    /* TODO: need to add this when adding Multi-Process support. */
    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(asp, Count, NEXUS_ANY_ID);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }
#endif

    hAspOutput = BKNI_Malloc(sizeof(*hAspOutput));
    if (!hAspOutput)
    {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(hAspOutput, 0, sizeof *hAspOutput);
    NEXUS_OBJECT_INIT(NEXUS_AspOutput, hAspOutput);
    g_NEXUS_asp.hAspChannelList[channelNum] = (struct NEXUS_AspChannel*)hAspOutput;      /* TODO: Eventually, hAspChannelList will be removed. */
    g_NEXUS_asp.hAspOutputList[channelNum] = hAspOutput;
    hAspOutput->channelNum = channelNum;

    if (!pSettings)
    {
        NEXUS_AspOutput_GetDefaultCreateSettings(&hAspOutput->createSettings);
    }
    else
    {
        hAspOutput->createSettings = *pSettings;
    }

    /* Allocate Per Channel Context Memory. All of these memory blocks are for ASP FW/HW usage. */
    rc = allocateAndLockBuffer(&pSettings->reTransmitFifo, false /*needCpuAccess*/, BASP_EPKT_RETX_BUFFER_ALIGNMENT_IN_BYTES, &hAspOutput->reTransmitFifo);
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    rc = allocateAndLockBuffer(&pSettings->miscBuffer,     false /*needCpuAccess*/, BASP_MISC_BUFFER_ALIGNMENT_IN_BYTES,      &hAspOutput->miscBuffer);
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    rc = allocateAndLockBuffer(&pSettings->receiveFifo,    true  /*needCpuAccess*/, BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES,   &hAspOutput->receiveFifo); /* TODO: see if we can use a different rcv buffer for receiving HTTP Response as this wastes the virtual memory for all of the Rx buffer where as we only need it for the initial Response part. */
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    rc = allocateAndLockBuffer(&pSettings->writeFifo,      true  /*needCpuAccess*/, BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES,   &hAspOutput->writeFifo);
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }

    /* Create a Streaming Channel with the BASP base module. */
    {
        BASP_ChannelCreateSettings createSettings;

        BASP_Channel_GetDefaultCreateSettings(&createSettings);
        BDBG_MSG(( FMT_PREFIX "Calling BASP_Channel_Create()..."
                   ARG_PREFIX));
        createSettings.channelNumber = channelNum;
        rc = BASP_Channel_Create(g_NEXUS_asp.hContext, &createSettings, &hAspOutput->hChannel);
        if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    }

    hAspOutput->state = NEXUS_AspOutputState_eIdle;
    BDBG_MSG(( FMT_PREFIX "hAspOutput=%p BASP hChannel=%p channelNumber=%d"
               ARG_PREFIX, (void *)hAspOutput, (void *)hAspOutput->hChannel, hAspOutput->channelNum));

#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
    BDBG_MSG(( FMT_PREFIX "Using NEXUS TIMER instead of interrupts"
               ARG_PREFIX));

    if (!g_NEXUS_asp.hAspOutputTimer)
    {
        g_NEXUS_asp.timerIntervalInMs = 100;
        BDBG_MSG(( FMT_PREFIX "Starting NEXUS TIMER for %u ms"
                   ARG_PREFIX, g_NEXUS_asp.timerIntervalInMs ));
        g_NEXUS_asp.hAspOutputTimer = NEXUS_ScheduleTimer(g_NEXUS_asp.timerIntervalInMs, NEXUS_ProcessMsgFromFwCallbackByTimer, NULL);
        BDBG_ASSERT(g_NEXUS_asp.hAspOutputTimer);
    }
#else  /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
    BDBG_MSG(( FMT_PREFIX "Using BASP interrupts"
               ARG_PREFIX));
    {
        BASP_ChannelCallbacks callbacks;

        BASP_Channel_GetCallbacks(hAspOutput->hChannel, &callbacks);

        callbacks.messageReady.pCallback_isr = NEXUS_AspOutput_ProcessMsgFromFwCallback_isr;
        callbacks.messageReady.pParam1 = hAspOutput;
        callbacks.messageReady.param2 = 0;

        rc = BASP_Channel_SetCallbacks(hAspOutput->hChannel, &callbacks);
        if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    }
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */

    hAspOutput->hEndOfStreamingCallback = NEXUS_IsrCallback_Create(hAspOutput, NULL);
    if (hAspOutput->hEndOfStreamingCallback == NULL) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto error; }
    NEXUS_CallbackDesc_Init(&hAspOutput->settings.endOfStreaming);

    hAspOutput->hRemoteDoneSendingCallback = NEXUS_IsrCallback_Create(hAspOutput, NULL);
    if (hAspOutput->hRemoteDoneSendingCallback == NULL) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto error; }
    NEXUS_CallbackDesc_Init(&hAspOutput->settings.remoteDoneSending);

    hAspOutput->hBufferReadyCallback = NEXUS_IsrCallback_Create(hAspOutput, NULL);
    if (hAspOutput->hBufferReadyCallback == NULL) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto error; }
    NEXUS_CallbackDesc_Init(&hAspOutput->settings.bufferReady);

    hAspOutput->hHttpRequestDataReadyCallback = NEXUS_IsrCallback_Create(hAspOutput, NULL);
    if (hAspOutput->hHttpRequestDataReadyCallback == NULL) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto error; }
    NEXUS_CallbackDesc_Init(&hAspOutput->settings.httpRequestDataReady);

    rc = BKNI_CreateEvent(&hAspOutput->hMessageReceiveEvent);
    if (rc != BERR_SUCCESS) { BERR_TRACE(rc); goto error; }

#if 0
#include "bchp_xpt_rsbuff.h"
    /* Test only: Configure XPT to disable some thruput throttling bits so that we can push thru 1Gpbs thru it. */
    {
        /* XAC Disable. */
        /* This setting has adverse affect on outgoing thruput when you use mix of wget & media player clients. */
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RSBUFF_PBP_XAC_EN, 0);
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RSBUFF_IBP_XAC_EN, 0);
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RSBUFF_CTRL_PAUSE_EN_PBP, 0);
#define BCHP_XPT_OCXC_TOP_BUF_CONFIG 0x2207000
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_OCXC_TOP_BUF_CONFIG, 0);

        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RAVE_WRMASK_OPTIMIZATION_DIS_CX_0_31, 0xffffffff);
        BREG_Write32(g_pCoreHandles->reg, BCHP_XPT_RAVE_WRMASK_OPTIMIZATION_DIS_CX_32_47, 0xffffffff);
    }
#endif
    BKNI_Memset(&hAspOutput->dtcpIpSettings, 0, sizeof(hAspOutput->dtcpIpSettings));
    hAspOutput->dtcpIpSettings.pcpPayloadSize = NEXUS_ASP_NUM_BLOCKS_IN_A_CHUNK*NEXUS_ASP_BLOCK_SIZE;

    BDBG_MSG(( FMT_PREFIX "Returning hAspOutput=%p using channelNum=%d."
               ARG_PREFIX, (void*)hAspOutput, hAspOutput->channelNum ));

    return hAspOutput;
error:
    NEXUS_AspOutput_Destroy(hAspOutput);
    return NULL;
}


static void NEXUS_AspOutput_P_Finalizer(
    NEXUS_AspOutputHandle hAspOutput
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));
    BDBG_ASSERT(hAspOutput);

    NEXUS_OBJECT_ASSERT(NEXUS_AspOutput, hAspOutput);

    if (hAspOutput->hMessageReceiveEvent)
    {
        BKNI_DestroyEvent(hAspOutput->hMessageReceiveEvent);
        hAspOutput->hMessageReceiveEvent = NULL;
    }

    if (hAspOutput->hEndOfStreamingCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspOutput->hEndOfStreamingCallback);
        hAspOutput->hEndOfStreamingCallback = NULL;
    }
    if (hAspOutput->hRemoteDoneSendingCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspOutput->hRemoteDoneSendingCallback);
        hAspOutput->hRemoteDoneSendingCallback = NULL;
    }
    if (hAspOutput->hBufferReadyCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspOutput->hBufferReadyCallback);
        hAspOutput->hBufferReadyCallback = NULL;
    }
    if (hAspOutput->hHttpRequestDataReadyCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspOutput->hHttpRequestDataReadyCallback);
        hAspOutput->hHttpRequestDataReadyCallback = NULL;
    }

    g_NEXUS_asp.hAspChannelList[hAspOutput->channelNum] = NULL;      /* TODO: Eventually, hAspChannelList will be removed. */
    g_NEXUS_asp.hAspOutputList[hAspOutput->channelNum] = NULL;

    #if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
    /* Only cancel the timer if no other channels are active. */
    {
        int channelNum;

        for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
        {
            if (g_NEXUS_asp.hAspOutputList[channelNum]) { break; }
        }
        if (channelNum == BASP_MAX_NUMBER_OF_CHANNEL)
        {
            BDBG_MSG(("%s: No active ASP Channels, canceling the timer!", BSTD_FUNCTION));
            BDBG_ASSERT(g_NEXUS_asp.hAspOutputTimer);
            NEXUS_CancelTimer(g_NEXUS_asp.hAspOutputTimer);
            g_NEXUS_asp.hAspOutputTimer = NULL;
        }
    }

#else  /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
    /* Disable the messageReady callbacks for this channel. */
    if (hAspOutput->hChannel)
    {
        BERR_Code               rc;
        BASP_ChannelCallbacks   callbacks;

        BDBG_MSG(( FMT_PREFIX "Disabling BASP messageReady callbacks for channel %p!"
                   ARG_PREFIX, (void*)hAspOutput->hChannel));

        BASP_Channel_GetCallbacks(hAspOutput->hChannel, &callbacks);

        callbacks.messageReady.pCallback_isr = NULL;
        callbacks.messageReady.pParam1 = NULL;
        callbacks.messageReady.param2 = 0;

        rc = BASP_Channel_SetCallbacks(hAspOutput->hChannel, &callbacks);
        BERR_TRACE(rc);
    }
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */

    if (hAspOutput->hChannel) {BASP_Channel_Destroy(hAspOutput->hChannel);}

    unLockAndFreeBuffer(&hAspOutput->reTransmitFifo);
    unLockAndFreeBuffer(&hAspOutput->receiveFifo);
    unLockAndFreeBuffer(&hAspOutput->miscBuffer);
    unLockAndFreeBuffer(&hAspOutput->writeFifo);

    NEXUS_OBJECT_DESTROY(NEXUS_AspOutput, hAspOutput);
    BKNI_Free(hAspOutput);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AspOutput, NEXUS_AspOutput_Destroy);


/**
Summary:
Get Default StartSettings.
**/
void NEXUS_AspOutput_GetDefaultStartSettings(
    NEXUS_AspOutputStartSettings       *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->feedMode = NEXUS_AspOutputFeedMode_eAuto;
}


static void printStartSettings(
    const NEXUS_AspOutputStartSettings *pSettings
    )
{
    BDBG_ASSERT(pSettings);

    BDBG_MSG(( FMT_PREFIX "FeedMode=%s"
               ARG_PREFIX,
               pSettings->feedMode == NEXUS_AspOutputFeedMode_eAuto                  ? "eAuto" :
               pSettings->feedMode == NEXUS_AspOutputFeedMode_eHost                  ? "eHost" :
               pSettings->feedMode == NEXUS_AspOutputFeedMode_eAutoWithHostEncryption ? "eAutoWithHostEncrytion" :
                                                                                       "<undefined"
           ));
} /* printStartSettings */


static void printEthernetSettings(
    const NEXUS_AspEthernetSettings     *pEthernetSettings
    )
{
    BDBG_ASSERT(pEthernetSettings);

    BDBG_MSG((FMT_PREFIX "Eth: localMacAddress=%2x:%2x:%2x:%2x:%2x:%2x remoteMacAddress=%2x:%2x:%2x:%2x:%2x:%2x etherType=0x%x, vlanTag=0x%x"
              ARG_PREFIX, pEthernetSettings->localMacAddress[0],
                          pEthernetSettings->localMacAddress[1],
                          pEthernetSettings->localMacAddress[2],
                          pEthernetSettings->localMacAddress[3],
                          pEthernetSettings->localMacAddress[4],
                          pEthernetSettings->localMacAddress[5],
                          pEthernetSettings->remoteMacAddress[0],
                          pEthernetSettings->remoteMacAddress[1],
                          pEthernetSettings->remoteMacAddress[2],
                          pEthernetSettings->remoteMacAddress[3],
                          pEthernetSettings->remoteMacAddress[4],
                          pEthernetSettings->remoteMacAddress[5],
                          pEthernetSettings->etherType,
                          pEthernetSettings->vlanTag
                          ));

    BDBG_MSG((FMT_PREFIX "Switch: queueNumber=%u ingressBrcmTag=0x%x egressClassId=%u"
              ARG_PREFIX, pEthernetSettings->networkSwitch.queueNumber,
                          pEthernetSettings->networkSwitch.ingressBrcmTag,
                          pEthernetSettings->networkSwitch.egressClassId
                          ));

} /* printEthernetSettings */


static void printIpSettings(
    const NEXUS_AspIpSettings                 *pIpSettings
    )
{
    BDBG_ASSERT(pIpSettings);

    if (pIpSettings->ipVersion == NEXUS_AspIpVersion_e4)
    {
        BDBG_MSG((FMT_PREFIX "IP: localIpAddr=%d.%d.%d.%d remoteIpAddr=%d.%d.%d.%d dscp=%d ecn=%u initialIdentification=%u timeToLive=%u"
                  ARG_PREFIX, pIpSettings->ver.v4.localIpAddr>>24,
                              pIpSettings->ver.v4.localIpAddr>>16 & 0xff,
                              pIpSettings->ver.v4.localIpAddr>>8 & 0xff,
                              pIpSettings->ver.v4.localIpAddr & 0xff,
                              pIpSettings->ver.v4.remoteIpAddr>>24,
                              pIpSettings->ver.v4.remoteIpAddr>>16 & 0xff,
                              pIpSettings->ver.v4.remoteIpAddr>>8 & 0xff,
                              pIpSettings->ver.v4.remoteIpAddr & 0xff,
                              pIpSettings->ver.v4.dscp,
                              pIpSettings->ver.v4.ecn,
                              pIpSettings->ver.v4.initialIdentification,
                              pIpSettings->ver.v4.timeToLive
                              ));
    }

    printEthernetSettings(&pIpSettings->eth);

} /* printIpSettings */


static void printTcpSettings(
    const NEXUS_AspTcpSettings                 *pTcpSettings
    )
{
    BDBG_ASSERT(pTcpSettings);

    BDBG_MSG((FMT_PREFIX "TCP: localPort=%d remotePort=%d initialSendSequenceNumber=0x%x initialRecvSequenceNumber0x%x currentAckedNumber=0x%x"
              ARG_PREFIX, pTcpSettings->localPort,
                          pTcpSettings->remotePort,
                          pTcpSettings->initialSendSequenceNumber,
                          pTcpSettings->initialRecvSequenceNumber,
                          pTcpSettings->currentAckedNumber
                         ));
    BDBG_MSG((FMT_PREFIX "TCP: maxSegmentSize=%u windowScaleValue local=%u remote=%u remoteWindowSize=%u enableTimeStamps=%s timestampEchoValue=%u enableSack=%s"
              ARG_PREFIX, pTcpSettings->maxSegmentSize,
                          pTcpSettings->localWindowScaleValue,
                          pTcpSettings->remoteWindowScaleValue,
                          pTcpSettings->remoteWindowSize,
                          pTcpSettings->enableTimeStamps?"Y":"N",
                          pTcpSettings->timestampEchoValue,
                          pTcpSettings->enableSack?"Y":"N"
                          ));

    printIpSettings(&pTcpSettings->ip);

} /* printTcpSettings */


static void printConnectHttpSettings(
    const NEXUS_AspTcpSettings                  *pTcpSettings,
    const NEXUS_AspOutputConnectHttpSettings    *pSettings
    )
{
    BDBG_ASSERT(pTcpSettings);
    BDBG_ASSERT(pSettings);

    BDBG_MSG((FMT_PREFIX "transportType=%d maxBitRate=%u hRecpump=%p"
              ARG_PREFIX,
                pSettings->transportType,
                pSettings->maxBitRate,
                (void*)pSettings->hRecpump
                ));
    BDBG_MSG((FMT_PREFIX "HTTP: version=%d.%d enableChunkTransferEncoding=%s chunkSize=%u "
              ARG_PREFIX, pSettings->version.major,
                          pSettings->version.minor,
                          pSettings->enableChunkTransferEncoding?"Y":"N",
                          pSettings->chunkSize
                          ));


    printTcpSettings(pTcpSettings);

} /* printConnectHttpSettings */


#ifdef NEXUS_HAS_SECURITY
static BHSM_KeyslotHandle allocAndConfigKeySlot(
    NEXUS_AspOutputHandle  hAspOutput
    )
{
    BERR_Code rc;
    BHSM_KeyslotAllocateSettings keySlotSettings;
    BHSM_KeyslotHandle hKeySlot = NULL;
    BHSM_KeyslotSettings hsmKeyslotSettings;
    BHSM_KeyslotEntrySettings hsmEntrySettings;
    BHSM_KeyslotExternalKeyData extKeyTableData;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

   {
        BHSM_Handle hHsm;

        LOCK_SECURITY();
        NEXUS_Security_GetHsm_priv(&hHsm);
        BHSM_Keyslot_GetDefaultAllocateSettings(&keySlotSettings);
        keySlotSettings.owner = BHSM_SecurityCpuContext_eHost;
        keySlotSettings.slotType = BHSM_KeyslotType_eIvPerBlock;
        keySlotSettings.useWithDma = false;
        hKeySlot = BHSM_Keyslot_Allocate( hHsm, &keySlotSettings );
        UNLOCK_SECURITY();
        if (hKeySlot == NULL) {BERR_TRACE(NEXUS_UNKNOWN); goto error;}
    }

#define ASP_PID_CHANNEL_BASE 1024
    rc = BHSM_Keyslot_AddPidChannel( hKeySlot, ASP_PID_CHANNEL_BASE + hAspOutput->channelNum );
    if (rc != NEXUS_SUCCESS) {BERR_TRACE(rc); goto error;}

    BKNI_Memset( &hsmKeyslotSettings, 0, sizeof(hsmKeyslotSettings) );
    BHSM_Keyslot_GetSettings( hKeySlot, &hsmKeyslotSettings);
    hsmKeyslotSettings.regions.source[0] = true;
    hsmKeyslotSettings.regions.destinationRPipe[0] = true;
    hsmKeyslotSettings.regions.destinationGPipe[0] = true;
    rc = BHSM_Keyslot_SetSettings( hKeySlot, &hsmKeyslotSettings);
    if (rc != NEXUS_SUCCESS) {BERR_TRACE(rc); goto error;}

    BKNI_Memset( &hsmEntrySettings, 0, sizeof(hsmEntrySettings) );
    BHSM_Keyslot_GetEntrySettings( hKeySlot, BHSM_KeyslotBlockEntry_eCpsClear, &hsmEntrySettings);
    hsmEntrySettings.algorithm         = BHSM_CryptographicAlgorithm_eAes128;
    hsmEntrySettings.algorithmMode     = BHSM_CryptographicAlgorithmMode_eCbc;
    hsmEntrySettings.terminationMode   = BHSM_Keyslot_TerminationMode_eClear;
    hsmEntrySettings.external.key      = true;
    hsmEntrySettings.external.iv       = true;
    hsmEntrySettings.rPipeEnable       = true;
    hsmEntrySettings.gPipeEnable       = true;
    rc = BHSM_Keyslot_SetEntrySettings( hKeySlot, BHSM_KeyslotBlockEntry_eCpsClear, &hsmEntrySettings);
    if (rc != NEXUS_SUCCESS) {BERR_TRACE(rc); goto error;}

    rc = BHSM_Keyslot_GetEntryExternalKeySettings( hKeySlot, BHSM_KeyslotBlockEntry_eCpsClear, &extKeyTableData);
    if (rc != NEXUS_SUCCESS) {BERR_TRACE(rc); goto error;}
    hAspOutput->extKeyTableSlotIndex = extKeyTableData.slotIndex;

    BDBG_LOG((FMT_PREFIX "Done: extKeyTableSlotIndex=%u"
              ARG_PREFIX, hAspOutput->extKeyTableSlotIndex));
    return (hKeySlot);

error:
    if (hKeySlot != NULL)
    {
        LOCK_SECURITY();
        BDBG_LOG((FMT_PREFIX "Error! Freeing hKeySlot=%p"
                  ARG_PREFIX, (void*)hAspOutput->hKeySlot));
        BHSM_Keyslot_Free( hAspOutput->hKeySlot ); hAspOutput->hKeySlot = NULL;
        UNLOCK_SECURITY();
    }
    return NULL;

} /* allocAndConfigKeySlot */
#endif


void NEXUS_AspOutput_GetDefaultConnectHttpSettings(
    NEXUS_AspOutputConnectHttpSettings       *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(pSettings);
    if (!pSettings) return;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    {
        pSettings->transportType = NEXUS_TransportType_eUnknown;
        pSettings->maxBitRate = 40*1024*1024;

        pSettings->version.major = 1;
        pSettings->version.minor = 1;
        pSettings->enableChunkTransferEncoding = false;
#define B_ASP_BLOCK_SIZE            192
#define B_ASP_NUM_BLOCKS_IN_A_CHUNK 512
        pSettings->chunkSize = B_ASP_BLOCK_SIZE * B_ASP_NUM_BLOCKS_IN_A_CHUNK;
    }
} /* NEXUS_AspOutput_GetDefaultConnectHttpSettings */


void NEXUS_AspOutput_GetDefaultTcpSettings(
    NEXUS_AspTcpSettings       *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(pSettings);
    if (!pSettings) return;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
} /* NEXUS_AspOutput_GetDefaultTcpSettings */


NEXUS_Error NEXUS_AspOutput_ConnectHttp(
    NEXUS_AspOutputHandle                       hAspOutput,
    const NEXUS_AspTcpSettings                  *pTcpSettings,
    const NEXUS_AspOutputConnectHttpSettings    *pConnectHttpSettings
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pConnectHttpSettings);
    BDBG_ASSERT(pTcpSettings);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    printConnectHttpSettings(pTcpSettings, pConnectHttpSettings);

    if (hAspOutput->state != NEXUS_AspOutputState_eIdle)
    {
        BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_ConnectHttp() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

        return (NEXUS_NOT_SUPPORTED);
    }

    hAspOutput->connectHttpSettings = *pConnectHttpSettings;
    hAspOutput->tcpSettings         = *pTcpSettings;
    hAspOutput->protocol            = NEXUS_AspStreamingProtocol_eHttp;

    hAspOutput->state = NEXUS_AspOutputState_eConnected;

    return (NEXUS_SUCCESS);
}


/**
Summary:
Start an AspOutput.
Note: it will only starts streaming if NEXUS_AspOutputStartSettings.autoStartStreaming flag is set to true!
**/
NEXUS_Error NEXUS_AspOutput_Start(
    NEXUS_AspOutputHandle              hAspOutput,
    const NEXUS_AspOutputStartSettings *pSettings
    )
{
    NEXUS_Error                             nrc;
    BASP_Pi2Fw_Message                      msg;
    BASP_ChannelStartStreamOutMessage       *pStreamOut = NULL;
    BASP_ConnectionControlBlock             *pCcb = NULL;
    const NEXUS_AspIpSettings               *pIpSettings = NULL;
    const NEXUS_AspTcpSettings              *pTcpSettings = NULL;
    const NEXUS_AspEthernetSettings         *pEthSettings = NULL;
    static bool                             fwTimestampSynced = false;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pSettings);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    printStartSettings(pSettings);

    if (hAspOutput->state != NEXUS_AspOutputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_ConnectHttp() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

        nrc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }

    hAspOutput->startSettings = *pSettings;

    /* TODO: check for if (pSettings->autoStartStreaming) */

    if (hAspOutput->tcpSettings.connectionLost)
    {
        hAspOutput->connectionReset = true;
        hAspOutput->state = NEXUS_AspOutputState_eStreaming;

        BDBG_MSG((FMT_PREFIX "hAspOutput=%p Starting with connectionLost: connectionReset=true, firing hEndOfStreamingCallback!"
                  ARG_PREFIX, (void *)hAspOutput));
        BKNI_EnterCriticalSection();
        NEXUS_IsrCallback_Fire_isr(hAspOutput->hEndOfStreamingCallback);
        BKNI_LeaveCriticalSection();
        return (NEXUS_SUCCESS);
    }

    /* Build Start Message. */
    BKNI_Memset(&msg, 0, sizeof(msg));

    /* Ignore the message header fields, those are populated by the BASP base module. */

    /* Build message payload. */
    pStreamOut = &msg.MessagePayload.ChannelStartStreamOut;

    BDBG_MSG((FMT_PREFIX "!!!!!! TCP Retransmissions is %s!"
              ARG_PREFIX, g_NEXUS_asp.enableTcpRetrans?"enabled":"disabled"));
    pStreamOut->ui32RetransmissionEnable = g_NEXUS_asp.enableTcpRetrans;
    pStreamOut->ReTransmissionBuffer.ui32BaseAddrLo = (uint32_t)(hAspOutput->reTransmitFifo.offset & 0xFFFFFFFF);
    pStreamOut->ReTransmissionBuffer.ui32BaseAddrHi = (uint32_t)(hAspOutput->reTransmitFifo.offset>>32);
    pStreamOut->ReTransmissionBuffer.ui32Size = (uint32_t)hAspOutput->reTransmitFifo.size;

    pStreamOut->EthernetHeaderBuffer.ui32BaseAddrLo = (uint32_t)hAspOutput->miscBuffer.offset;
    pStreamOut->EthernetHeaderBuffer.ui32BaseAddrHi = (uint32_t)(hAspOutput->miscBuffer.offset>>32);
    pStreamOut->EthernetHeaderBuffer.ui32Size = (uint32_t)hAspOutput->miscBuffer.size;

    pStreamOut->ReceivePayloadBuffer.ui32BaseAddrLo = (uint32_t)(hAspOutput->receiveFifo.offset & 0xFFFFFFFF);
    pStreamOut->ReceivePayloadBuffer.ui32BaseAddrHi = (uint32_t)(hAspOutput->receiveFifo.offset>>32);
    pStreamOut->ReceivePayloadBuffer.ui32Size = (uint32_t)hAspOutput->receiveFifo.size;

    BMMA_FlushCache_isrsafe( hAspOutput->receiveFifo.hBlock, (char *)hAspOutput->receiveFifo.pBuffer, hAspOutput->receiveFifo.size );

    if (hAspOutput->writeFifoLength)
    {
        pStreamOut->ui32SendHttpResponsePktEnable = true;
        pStreamOut->HttpResponseBuffer.ui32BaseAddrLo = (uint32_t)(hAspOutput->writeFifo.offset & 0xFFFFFFFF);
        pStreamOut->HttpResponseBuffer.ui32BaseAddrHi = (uint32_t)(hAspOutput->writeFifo.offset>>32);
        pStreamOut->HttpResponseBuffer.ui32Size = hAspOutput->writeFifoLength;
    }

    BMMA_FlushCache_isrsafe( hAspOutput->writeFifo.hBlock, (char *)hAspOutput->writeFifo.pBuffer, hAspOutput->writeFifoLength );

    if (hAspOutput->connectHttpSettings.enableChunkTransferEncoding)
    {
        pStreamOut->ui32HttpType = BASP_HttpConnectionType_e11WithChunking;
        pStreamOut->ui32ChunkSize = hAspOutput->connectHttpSettings.chunkSize/NEXUS_ASP_BLOCK_SIZE;
    }
    else
    {
        pStreamOut->ui32HttpType = BASP_HttpConnectionType_e11WithoutChunking;
    }
    pStreamOut->ui32CongestionFlowControlEnable = 1 << BASP_DEBUG_HOOK_DUPLICATE_ACK_COND_BIT;

    if (g_NEXUS_asp.enableTcpCongestionControl)
    {
        BDBG_MSG((FMT_PREFIX "!!!!!! Enabling TCP Congestion Control Algorithm: Slow Start & Congestion Avoidance!"
                  ARG_PREFIX));
        pStreamOut->ui32CongestionFlowControlEnable |= 1 << BASP_DEBUG_HOOK_CONGESTION_CONTROL_ENABLE_BIT;
    }

    /* Populate the message's Connection Control Block. */
    pCcb = &pStreamOut->ConnectionControlBlock;

    /* Get RAVE CDB offset corresponding to the Recpump associated with this AspOutput (passed by the caller!). */
    {
        NEXUS_RecpumpStatus     status;
        uint64_t raveContextBaseAddress;

        if (hAspOutput->connectHttpSettings.hRecpump == NULL)
        {
            BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutputStartSettings.hRecpump must not be NULL"
                       ARG_PREFIX, (void*)hAspOutput));

            nrc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }

        nrc = NEXUS_Recpump_GetStatus(hAspOutput->connectHttpSettings.hRecpump, &status);
        if (nrc != NEXUS_SUCCESS) { BERR_TRACE(nrc); goto error;}

        raveContextBaseAddress = BASP_CVT_TO_ASP_REG_ADDR( BCHP_PHYSICAL_OFFSET + BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR +
                                      ((BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR) * status.rave.index));

        pStreamOut->ui32RaveContextBaseAddressLo = (uint32_t) (raveContextBaseAddress & 0xFFFFFFFF);
        pStreamOut->ui32RaveContextBaseAddressHi = (uint32_t) (raveContextBaseAddress >> 32);

        BDBG_MSG((FMT_PREFIX "hAspOutput=%p hRecpump=%p rave_ctx#=%d, baseAddr low=0x%x high=0x%x "
                  ARG_PREFIX, (void *)hAspOutput, (void *)hAspOutput->connectHttpSettings.hRecpump, status.rave.index,
                              pStreamOut->ui32RaveContextBaseAddressLo, pStreamOut->ui32RaveContextBaseAddressHi
                              ));
    }

    /* Fill-in the message's ASP MCPB related defaults for this Streaming out channel. */
    {
        BASP_McpbStreamOutConfig    *pMcpb = &pStreamOut->McpbStreamOutConfig;

        /* TODO: ATS: need to reconfigure bunch of these settings when 188 + 4 byte of Timestamps are present in the stream. */
        pMcpb->ASPPids.ui32NumASPPids = 1;              /* PID filtering is done at the XPT MCPB Config level, ASP MCPB will pass-thru all AV data from RAVE. Thus the value of 1. */
        pMcpb->ASPPids.aPidList[0].ui32ProgramType = 0;
        pMcpb->ASPPids.aPidList[0].ui32PidChannel = BXPT_P_PID_TABLE_SIZE + hAspOutput->channelNum;
        pMcpb->ASPPids.aPidList[0].ui32PidValue = 0;    /* All pass case. PID filtering is done by the XPT MCPB. */
        pMcpb->ui32PacingType = 0;  /* Pacing should happen at the XPT MCPB level, so this is just for debugging. */
        pMcpb->ui32SrcTimestampType = BASP_TransportTimeStampType_e302uMod300;
        /* TODO: this is a good option to enable if we have to see output timestamps. */
        pMcpb->ui32ForceRestamping = 1;
        pMcpb->ui32PcrPacingPid = 0;        /* N/A, again happens at the XPT MCPB level. */
        pMcpb->ui32ParityCheckDisable = 0;  /* N/A, again happens at the XPT MCPB level. */
        pMcpb->ui32AvgStreamBitrate = hAspOutput->connectHttpSettings.maxBitRate;
        pMcpb->ui32TimebaseIndex = 0;
        pMcpb->ui32ParserAllPassControl = 1;
#if 0
        pMcpb->ui32ParserStreamType = BASP_ParserStreamType_eMpeg;
        pMcpb->ui32PacketLen = 188;
#else
        /* Note: switching to Block mode for all container formats as DTCP/IP requires it. */
        pMcpb->ui32ParserStreamType = BASP_ParserStreamType_eBlock;
        pMcpb->ui32PacketLen = 192;
#endif
        pMcpb->ui32McpbTmeuErrorBoundLate = 0;
        /* TODO: Missing fields: outputAtsMode. */
    }

    /* Determine the pointers to various headers so that we can use them independent of the protocol. */
    if (hAspOutput->protocol == NEXUS_AspStreamingProtocol_eHttp)
    {
        pEthSettings = &hAspOutput->tcpSettings.ip.eth;
        pIpSettings  = &hAspOutput->tcpSettings.ip;
        pTcpSettings = &hAspOutput->tcpSettings;
    }
    else
    {
        BDBG_WRN((FMT_PREFIX "non-HTTP protocols are not yet supported!!!"
                  ARG_PREFIX));
        nrc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }

    /* Fill-in connection control block. */
    /* Ethernet & Network Switch related settings: needed for all protocols. */
    {
        /* if (pSettings->mode == NEXUS_AspStreamingMode_eOut) */
        {
            BKNI_Memcpy(pCcb->aui8SrcMacAddr,  pEthSettings->localMacAddress,  BASP_MAX_ENTRY_IN_MAC_ADDR);
            BKNI_Memcpy(pCcb->aui8DestMacAddr, pEthSettings->remoteMacAddress, BASP_MAX_ENTRY_IN_MAC_ADDR);
        }
        pCcb->ui32EtherType      = pEthSettings->etherType;
        pCcb->ui32IngressBrcmTag = pEthSettings->networkSwitch.ingressBrcmTag;
        pCcb->ui32EgressClassId  = pEthSettings->networkSwitch.egressClassId;
    }

    /* IP header related settings: needed for all protocols. */
    {
        pCcb->ui32IpVersion = pIpSettings->ipVersion == NEXUS_AspIpVersion_e6 ? 6 : 4;
        if (pIpSettings->ipVersion == NEXUS_AspIpVersion_e4)
        {
            pCcb->ui32Dscp          = pIpSettings->ver.v4.dscp;
            pCcb->ui32TimeToLive    = pIpSettings->ver.v4.timeToLive;
            pCcb->ai32SrcIpAddr[0]  = pIpSettings->ver.v4.localIpAddr;
            pCcb->ai32DestIpAddr[0] = pIpSettings->ver.v4.remoteIpAddr;
#if TODO
            /* FW currently doesn't define & use the protocol type & defaults it to TCP only. */
            /* Update this when this enum gets defined. */
            pCcb->ui32ProtocolType = pSettings->protocol;
#endif
        }
    }

    /* Set TCP header related settings if being used. */
    if (pTcpSettings)
    {
        pCcb->ui32SrcPort  = pTcpSettings->localPort;
        pCcb->ui32DestPort = pTcpSettings->remotePort;
        pCcb->ui32InitialSendSeqNumber     = pTcpSettings->initialSendSequenceNumber;
        pCcb->ui32InitialReceivedSeqNumber = pTcpSettings->initialRecvSequenceNumber;
        pCcb->ui32CurrentAckedNumber = pTcpSettings->currentAckedNumber;
        pCcb->ui32RemoteWindowSize   = pTcpSettings->remoteWindowSize;
        pCcb->ui32WindowAdvConst     = 0x1000; /* TODO: hardcoded as per HW team, need to further lookinto on this value. */
        pCcb->ui32RemoteWindowScaleValue = pTcpSettings->remoteWindowScaleValue;
        pCcb->ui32LocalWindowScaleValue  = pTcpSettings->localWindowScaleValue;

        if (g_NEXUS_asp.enableTcpSack) {pCcb->ui32SackEnable = pTcpSettings->enableSack;}

        pCcb->ui32TimeStampEnable = pTcpSettings->enableTimeStamps;
        BDBG_MSG((FMT_PREFIX "!!!!!! TCP Timestamps are %s!"
                  ARG_PREFIX, g_NEXUS_asp.enableTcpTimestamps? "enabled":"disabled"));
        if (g_NEXUS_asp.enableTcpTimestamps)
        {
            pCcb->ui32TimeStampEnable = 1;
            pCcb->ui32LocalTimeStampValue = pTcpSettings->senderTimestamp;
        }
        else
        {
            pCcb->ui32TimeStampEnable = 0;
        }
        pCcb->ui32MaxSegmentSize = pTcpSettings->maxSegmentSize;

        pCcb->ui32KaTimeout      = 7200000;
        pCcb->ui32KaInterval     = 75000;
        pCcb->ui32KaMaxProbes    = 9;
        pCcb->ui32RetxTimeout    = 1000;
        pCcb->ui32RetxMaxRetries = 13;
    }
    else
    {
        BDBG_WRN((FMT_PREFIX "non-TCP protocols are not yet supported!!!"
                  ARG_PREFIX));
        nrc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }


#if 0 /* ******************** Temporary **********************/
    pStreamOut->ui32SwitchQueueNumber = pEthSettings->networkSwitch.queueNumber; /* TODO: enable the loss-less behavior in switch & test this. */
#else /* ******************** Temporary **********************/
    pStreamOut->ui32SwitchQueueNumber = 0;  /* Currently, ASP will output packets to the queue 0 of its port. */
#endif /* ******************** Temporary **********************/

    BDBG_MSG((FMT_PREFIX "!!!!!! TODO: Overwriting the ui32SwitchQueueNumber(=%d) to 0 until ACH support is enabled in ASP & Network Switch"
              ARG_PREFIX, pStreamOut->ui32SwitchQueueNumber));
    pStreamOut->ui32SwitchSlotsPerEthernetPacket = 6; /* TODO get this number from FW/HW team & also make sure it gets programmed on each switch port. */
#if 0
    BASP_Msgqueue_Log(NULL, "StartMessage", &msg);
#endif

    /* TODO:
     * FW needs to program EDPKT's initial timestamp value that Linux had used for the connection.
     * Otherwise, receiving client will drop the incoming data packets as their timestamps will not match.
     * Until FW adds this logic, we are doing this in the software.
     */
    if (pTcpSettings->enableTimeStamps && !fwTimestampSynced)
    {
        uint64_t timestampValue;
        /* Set bit in EPKT Debug register to enable setting of the initial timestamp value. */
        BREG_Write32(g_pCoreHandles->reg, 0x18464b0, 0x2);

        /* Linux provides 32 bit timestamp value. This value needs to be programmed in the upper 32bits of the 48bit register. */
        timestampValue = (uint64_t)pTcpSettings->senderTimestamp <<16;
        BDBG_MSG((FMT_PREFIX "!!!!! TODO (move this to FW): Program starting timestamp value to EPKT: linux ts=0x%x 64bit shifted value: hi:lo=0x%x:0x%x ...."
                  ARG_PREFIX,
                    pTcpSettings->senderTimestamp, (uint32_t)(timestampValue>>32), (uint32_t)(timestampValue & 0xFFFFFFFF)));

        BREG_Write64(g_pCoreHandles->reg, 0x1846480, timestampValue ); /* Debug register to enable setting of the initial timestamp value. */
        BREG_Write32(g_pCoreHandles->reg, 0x18464b0, 0x0); /* Debug register to enable setting of the initial timestamp value. */

        /* Read the value back. */
        timestampValue = BREG_Read64(g_pCoreHandles->reg, 0x1846480); /* Debug register to enable setting of the initial timestamp value. */
        BDBG_MSG((FMT_PREFIX "timestampValue hi:lo=0x%x:0x%x read back ...."
                  ARG_PREFIX, (uint32_t)(timestampValue>>32), (uint32_t)(timestampValue & 0xFFFFFFFF) ));
        fwTimestampSynced = true;
    }

#ifdef NEXUS_HAS_SECURITY
    /* DTCP/IP related info. */
    if (hAspOutput->drmType == NEXUS_AspOutputDrmType_eDtcpIp)
    {
        /* if (pSettings->mode == NEXUS_AspStreamingMode_eOut) */
        {
            hAspOutput->hKeySlot = allocAndConfigKeySlot( hAspOutput );
            if (hAspOutput->hKeySlot == NULL) {nrc = BERR_TRACE(NEXUS_UNKNOWN); goto error;}

            pStreamOut->ui32DrmEnabled = true;
            pStreamOut->ui32PcpPayloadSize = hAspOutput->dtcpIpSettings.pcpPayloadSize/NEXUS_ASP_BLOCK_SIZE;
            if ( hAspOutput->connectHttpSettings.enableChunkTransferEncoding &&
                (pStreamOut->ui32ChunkSize < pStreamOut->ui32PcpPayloadSize ||
                pStreamOut->ui32ChunkSize % pStreamOut->ui32PcpPayloadSize) )
            {
                /* Chunk doesn't contain integral # of PCPs. FW doesn't support this right now! */
                BDBG_WRN((FMT_PREFIX "!!! pcpPayloadSize=%u is NOT integral multiple of httpChunkSize=%u, making them equal as FW only supports this mode!!"
                          ARG_PREFIX,
                        pStreamOut->ui32PcpPayloadSize, pStreamOut->ui32ChunkSize));
                pStreamOut->ui32PcpPayloadSize = pStreamOut->ui32ChunkSize;
            }
            else {
                BDBG_MSG((FMT_PREFIX "!!! pcpPayloadSize=%u httpChunkSize=%u!!"
                          ARG_PREFIX, pStreamOut->ui32PcpPayloadSize, pStreamOut->ui32ChunkSize));
            }
            pStreamOut->DtcpIpInfo.ui32EmiModes = hAspOutput->dtcpIpSettings.emiModes;
            pStreamOut->DtcpIpInfo.ui32ExchangeKeyLabel = hAspOutput->dtcpIpSettings.exchKeyLabel;

            /* Kx[0:1:2:3] -> Key[b3:b2:b1:b0] */
            pStreamOut->DtcpIpInfo.ui32ExchangeKeys[0] =
                (hAspOutput->dtcpIpSettings.exchKey[0] << 24) |
                (hAspOutput->dtcpIpSettings.exchKey[1] << 16) |
                (hAspOutput->dtcpIpSettings.exchKey[2] <<  8) |
                (hAspOutput->dtcpIpSettings.exchKey[3] <<  0) ;
            pStreamOut->DtcpIpInfo.ui32ExchangeKeys[1] =
                (hAspOutput->dtcpIpSettings.exchKey[4] << 24) |
                (hAspOutput->dtcpIpSettings.exchKey[5] << 16) |
                (hAspOutput->dtcpIpSettings.exchKey[6] <<  8) |
                (hAspOutput->dtcpIpSettings.exchKey[7] <<  0) ;
            pStreamOut->DtcpIpInfo.ui32ExchangeKeys[2] =
                (hAspOutput->dtcpIpSettings.exchKey[8] << 24) |
                (hAspOutput->dtcpIpSettings.exchKey[9] << 16) |
                (hAspOutput->dtcpIpSettings.exchKey[10] << 8) |
                (hAspOutput->dtcpIpSettings.exchKey[11] << 0) ;

            pStreamOut->DtcpIpInfo.ui32C_A2 = 0;

            /* Nonce[0:1:2:3] -> Nc[1][b0:b1:b2:b3], Nonce[4:5:6:7] -> Nc[1][b3:b2:b1:b0] */
            pStreamOut->DtcpIpInfo.ui32Nc[0] = /* Upper 32bits of the 64bit Nonce. */
                (hAspOutput->dtcpIpSettings.initialNonce[7] << 24) |
                (hAspOutput->dtcpIpSettings.initialNonce[6] << 16) |
                (hAspOutput->dtcpIpSettings.initialNonce[5] <<  8) |
                (hAspOutput->dtcpIpSettings.initialNonce[4] <<  0) ;
            pStreamOut->DtcpIpInfo.ui32Nc[1] = /* Lower 32bits of the 64bit Nonce. */
                (hAspOutput->dtcpIpSettings.initialNonce[3] << 24) |
                (hAspOutput->dtcpIpSettings.initialNonce[2] << 16) |
                (hAspOutput->dtcpIpSettings.initialNonce[1] <<  8) |
                (hAspOutput->dtcpIpSettings.initialNonce[0] <<  0) ;

            pStreamOut->DtcpIpInfo.ui32ASPKeySlot = hAspOutput->extKeyTableSlotIndex + 0x2; /* +2 offset for keySlot. */
        }
    }
#endif

#if 0
    {
        /* TODO: debug code to reset EDPKT stats upon every channel change. */
        BDBG_MSG((FMT_PREFIX "hAspOutput=%p Resetting the EDPKT .."
                  ARG_PREFIX, (void *)hAspOutput));
        BREG_Write32(g_pCoreHandles->reg, BCHP_ASP_EDPKT_CORE_TEST_REG0, 0x7c);
    }
#endif

    /* Send a StartStreamOut message to the ASP and wait for a response. */
    nrc = NEXUS_AspOutput_DoAspMessage( hAspOutput,
                                      BASP_MessageType_ePi2FwChannelStartStreamOut,
                                      &msg,
                                      &hAspOutput->gotStartResponse);

    if (nrc != NEXUS_SUCCESS) {BERR_TRACE(nrc); goto error;}

    /* Save initial stats as some of the stats are commulative & we need the initial values to calculate the current  */
    {
        updateStats(hAspOutput, g_pCoreHandles->reg, hAspOutput->channelNum, NULL, &hAspOutput->initialStatus);
        hAspOutput->currentStatus = hAspOutput->initialStatus;
    }

    /* TODO: Disable ACH logic in MCPB. */
    {
        unsigned value;
        value = BREG_Read32(g_pCoreHandles->reg, BCHP_ASP_MCPB_DIS_QUEUE_CHECK_IN_EPKT_MODE);
        value |= (1<<hAspOutput->channelNum);
        BREG_Write32(g_pCoreHandles->reg, BCHP_ASP_MCPB_DIS_QUEUE_CHECK_IN_EPKT_MODE, value);
    }

#if 0
    /* Disable TCP retransmissions: for debugging only! */
    {
        checkRegContent(g_pCoreHandles->reg, ">>>>>>>>>>>>>>>>>>> EPKT_ASP_EPKT_CORE_CH00_CH_CONFIG_MISC", 0x1844000, 0);
        checkRegContent(g_pCoreHandles->reg, ">>>>> SWITCH_CORE_TxFrameInDisc_P7", 0xf139a0, 0);
        BREG_Write32(g_pCoreHandles->reg, 0x1844000, 0x1);
    }
#endif

#if 0
    /* Configure EDPKT to receive packets w/ any BRCM tag: for debugging only! */
    {
        int i;
        for (i=0; i<256; i+=4)
        {
            BREG_Write32(g_pCoreHandles->reg, 0x1840200+i, 0);
        }
        /* Also configure ch0 to store all packet headers in the payload field. */
        {
            BREG_Write32(g_pCoreHandles->reg, 0x1840300, 0x1c6);
        }
        BDBG_WRN((FMT_PREFIX "!!!!!! Receiving all packets from Switch & recording them......"
                  ARG_PREFIX));
    }
#endif

    hAspOutput->state = NEXUS_AspOutputState_eStreaming;

    return (NEXUS_SUCCESS);

error:
    return (nrc);
}


NEXUS_Error NEXUS_AspOutput_Disconnect(
    NEXUS_AspOutputHandle                       hAspOutput,
    NEXUS_AspOutputDisconnectStatus             *pStatus
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pStatus);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    if (hAspOutput->state == NEXUS_AspOutputState_eStreaming)
    {
        BDBG_MSG(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_Disconnect() called when state is %s, calling NEXUS_AspOutput_Stop()"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));
        NEXUS_AspOutput_Stop(hAspOutput);
    }

    if (hAspOutput->state != NEXUS_AspOutputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_Disconnect() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

        return (NEXUS_NOT_SUPPORTED);
    }

    if (hAspOutput->protocol == NEXUS_AspStreamingProtocol_eHttp)
    {
        pStatus->tcpState.finalSendSequenceNumber = hAspOutput->tcpState.finalSendSequenceNumber;
#if 0 /* ******************** Temporary **********************/
        pStatus->tcpState.finalRecvSequenceNumber = hAspOutput->tcpState.finalRecvSequenceNumber;
#else /* ******************** Temporary **********************/
        /* TODO: until FW is fixed to return the correct Recv Seq #, we use the original value. */
        pStatus->tcpState.finalRecvSequenceNumber = hAspOutput->tcpState.finalRecvSequenceNumber;
#endif /* ******************** Temporary **********************/
    }

    hAspOutput->drmType = NEXUS_AspOutputDrmType_eNone;
    hAspOutput->state = NEXUS_AspOutputState_eIdle;

    return (NEXUS_SUCCESS);
}


/**
Summary:
Stop a AspOutput.

Description:
This API sends the Abort message to FW to immediately Stop NEXUS_AspOutput.
FW will NOT synchronize the protocol state (such as for TCP) in this case.
For that, caller should use NEXUS_AspOutput_Finish()
**/
void NEXUS_AspOutput_Stop(
    NEXUS_AspOutputHandle              hAspOutput
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    if (hAspOutput->state != NEXUS_AspOutputState_eStreaming)
    {
        BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_Stop() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));
        return;
    }

    if (hAspOutput->tcpSettings.connectionLost)
    {
        hAspOutput->state = NEXUS_AspOutputState_eConnected;
        return;
    }

    {
        BASP_Pi2Fw_Message pi2FwMessage;

        BKNI_Memset(&pi2FwMessage, 0, sizeof(pi2FwMessage));
        pi2FwMessage.MessagePayload.ChannelAbort.ui32Unused = 0;

        /* Send a ChannelAbort message to the ASP and wait for a response. */
        rc = NEXUS_AspOutput_DoAspMessage( hAspOutput,
                                         BASP_MessageType_ePi2FwChannelAbort,
                                          &pi2FwMessage,
                                          &hAspOutput->gotStopResponse);

        if (rc != NEXUS_SUCCESS) {BERR_TRACE(rc); goto error;}

        if (hAspOutput->protocol == NEXUS_AspStreamingProtocol_eHttp)
        {
            BDBG_MSG((FMT_PREFIX "hAspOutput=%p Abort Response Rcvd, seq#: final=%u initial=%u bytesSent=%u, ack#: final=%u initial=%u bytesRcvd=%u"
                      ARG_PREFIX, (void *)hAspOutput,
                                  hAspOutput->tcpState.finalSendSequenceNumber,
                                  hAspOutput->tcpSettings.initialSendSequenceNumber,
                                  hAspOutput->tcpState.finalSendSequenceNumber - hAspOutput->tcpSettings.initialSendSequenceNumber,
                                  hAspOutput->tcpState.finalRecvSequenceNumber,
                                  hAspOutput->tcpSettings.initialRecvSequenceNumber,
                                  hAspOutput->tcpState.finalRecvSequenceNumber - hAspOutput->tcpSettings.initialRecvSequenceNumber
                                  ));
        }
    }

#ifdef NEXUS_HAS_SECURITY
    if (hAspOutput->drmType == NEXUS_AspOutputDrmType_eDtcpIp)
    {
        if (hAspOutput->hKeySlot)
        {
            LOCK_SECURITY();
            BHSM_Keyslot_Free( hAspOutput->hKeySlot ); hAspOutput->hKeySlot = NULL;
            UNLOCK_SECURITY();
        }
        hAspOutput->hKeySlot = NULL;
    }
#endif
    hAspOutput->state = NEXUS_AspOutputState_eConnected;

error:
    return;
}


/**
Summary:
Finish an AspOutput.

Description:
This API initiates the normal finishing of ASP Channel.
FW will synchronize the protocol state (such as for TCP) in this case.
This may involve waiting for TCP ACKs for any pending data and thus may take time.
However, the API returns after sending the message to the FW.
Its completion will be notified via the statusChangedCallback.

For immediate aborting, caller should use NEXUS_AspOutput_Stop().
**/
void NEXUS_AspOutput_Finish(
    NEXUS_AspOutputHandle              hAspOutput
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    BDBG_ASSERT(hAspOutput);

    if (hAspOutput->state != NEXUS_AspOutputState_eStreaming)
    {
        BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_Finish() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));
        return;
    }

    if (hAspOutput->state == NEXUS_AspOutputState_eStreaming)
    {
        BASP_Pi2Fw_Message pi2FwMessage;

        BKNI_Memset(&pi2FwMessage, 0, sizeof(pi2FwMessage));
        pi2FwMessage.MessagePayload.ChannelAbort.ui32Unused = 0;

        /* Since the "ChannelStop" may take some time, don't wait here for the response message. */
        rc = BASP_Channel_SendMessage( hAspOutput->hChannel,
                                       BASP_MessageType_ePi2FwChannelStop,
                                       BASP_ResponseType_eAckRespRequired,
                                       &pi2FwMessage);
        if (rc != BERR_SUCCESS) {BERR_TRACE(rc); goto error;}
    }

#ifdef NEXUS_HAS_SECURITY
    if (hAspOutput->drmType == NEXUS_AspOutputDrmType_eDtcpIp)
    {
        if (hAspOutput->hKeySlot)
        {
            LOCK_SECURITY();
            BHSM_Keyslot_Free( hAspOutput->hKeySlot ); hAspOutput->hKeySlot = NULL;
            UNLOCK_SECURITY();
        }
        hAspOutput->hKeySlot = NULL;
    }
#endif

    hAspOutput->finishRequested = true;

error:
    return;
}


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error NEXUS_AspOutput_GetStatus(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputStatus              *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pStatus);

    pStatus->aspChannelIndex    = hAspOutput->channelNum;
    pStatus->state              = hAspOutput->state;
    pStatus->finishRequested    = hAspOutput->finishRequested;
    pStatus->finishCompleted    = hAspOutput->finishCompleted;
    pStatus->connectionReset    = hAspOutput->connectionReset;
    pStatus->networkTimeout     = hAspOutput->networkTimeout;
    pStatus->remoteDoneSending  = hAspOutput->remoteDoneSending;

    return (NEXUS_SUCCESS);
}


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error NEXUS_AspOutput_GetTcpStatus(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputTcpStatus           *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pStatus);

/*     hAspOutput->currentStatus.state = hAspOutput->state;  TODO: Should we add state to NEXUS_AspOutputTcpStatus? */
    updateStats(hAspOutput, g_pCoreHandles->reg, hAspOutput->channelNum, &hAspOutput->initialStatus, &hAspOutput->currentStatus);
#if 0
    determineAspHwPipeStatus(hAspOutput, g_pCoreHandles->reg, &hAspOutput->currentStatus, &pStatus->stats);
#endif
    *pStatus = hAspOutput->currentStatus;
/*      pStatus->aspChannelIndex = hAspOutput->channelNum;    TODO: Should we add aspChannelIndex to NEXUS_AspOutputTcpStatus? */

    /* ASP FW Stats: FW periodically updates the status buffer in DRAM. */
    {
        uint32_t newValue, curValue;
        BASP_FwStatusInfo *pFwStatusInfo = g_NEXUS_asp.statusBuffer.pBuffer;
        BASP_FwChannelInfo *pFwChannelInfo;

        BMMA_FlushCache_isrsafe( g_NEXUS_asp.statusBuffer.hBlock, (char *)pFwStatusInfo, sizeof(BASP_FwStatusInfo) );
        pFwChannelInfo = &(pFwStatusInfo->stChannelInfo[hAspOutput->channelNum]);

        pStatus->fwStats.congestionWindow = pFwChannelInfo->ui32CongestionWindow;
        pStatus->fwStats.receiveWindow = pFwChannelInfo->ui32ReceivedWindowSize;
        /* if (hAspOutput->startSettings.mode == NEXUS_AspStreamingMode_eOut)  */
        {
            pStatus->fwStats.sendWindow = pFwChannelInfo->ui32MaxSequenceNum - pFwChannelInfo->ui32SequenceNum;
        }
        pStatus->fwStats.sendSequenceNumber = pFwChannelInfo->ui32ReceivedAckSequenceNumber;
        pStatus->fwStats.rcvdAckNumber = pFwChannelInfo->ui32AckSequenceNumber;

        newValue = pFwChannelInfo->ui32NumOfIpPktsSent;
        curValue = (uint32_t)(pStatus->fwStats.pktsSent & 0xFFFFFFFF);
        pStatus->fwStats.pktsSent += (newValue - curValue);

        newValue = pFwChannelInfo->ui32NumReceivedPkts;
        curValue = (uint32_t)(pStatus->fwStats.pktsRcvd & 0xFFFFFFFF);
        pStatus->fwStats.pktsRcvd += (newValue - curValue);
        pStatus->fwStats.pktsDropped = pFwChannelInfo->ui32NumPktDropped;
        pStatus->fwStats.dataPktsDropped = pFwChannelInfo->ui32NumDataPktDropped;

        pStatus->fwStats.pktsRetx = pFwChannelInfo->ui32NumOfTotalRetx;
        pStatus->fwStats.retxSequenceNumber = pFwChannelInfo->ui32RetxSequenceNum;
        pStatus->fwStats.rcvdSequenceNumber = pFwChannelInfo->ui32ReceivedSequenceNumber;
    }

    pStatus->statsValid = true;
    return (NEXUS_SUCCESS);
}


NEXUS_Error NEXUS_AspOutput_GetHttpStatus(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputHttpStatus          *pStatus     /* [out] */
    )
{
    NEXUS_Error nrc;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pStatus);

    nrc = NEXUS_AspOutput_GetTcpStatus(hAspOutput, &pStatus->tcp);

    /* TODO: maintain HTTP stats. */
    return (nrc);
}


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void NEXUS_AspOutput_GetDtcpIpSettings(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputDtcpIpSettings      *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    BDBG_ASSERT(pSettings);
    *pSettings = hAspOutput->dtcpIpSettings;
}


/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to NEXUS_AspOutput_Connect().
**/
NEXUS_Error NEXUS_AspOutput_SetDtcpIpSettings(
    NEXUS_AspOutputHandle              hAspOutput,
    const NEXUS_AspOutputDtcpIpSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pSettings);

    hAspOutput->dtcpIpSettings = *pSettings;
    hAspOutput->drmType = NEXUS_AspOutputDrmType_eDtcpIp;
    BDBG_MSG((FMT_PREFIX "DTCP/IP Settings: pcpPayloadSize=%u exchKeyLabel=%u emiModes=%u "
              ARG_PREFIX,
                pSettings->pcpPayloadSize,
                pSettings->exchKeyLabel,
                pSettings->emiModes
            ));

    return (NEXUS_SUCCESS);
}


/**
Summary:
API to provide data to the firmware so that it can transmit it out on the network.

Note: one usage of this API is to allow caller to send HTTP Request or HTTP Response to the remote.
**/
static NEXUS_Error NEXUS_AspOutput_GetWriteBufferWithWrap(
    NEXUS_AspOutputHandle              hAspOutput,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pAmount,    /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                            *pAmount2    /* [out] size of the available space in the pBuffer2 that can be written to network. */
    )
{
    NEXUS_Error     rc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pAmount);
    BDBG_ASSERT(pBuffer2);
    BDBG_ASSERT(pAmount2);

    if (hAspOutput->state != NEXUS_AspOutputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_GetWriteBufferWithWrap() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));
        rc = NEXUS_NOT_SUPPORTED;
        goto error;
    }

    *pBuffer = hAspOutput->writeFifo.pBuffer;
    *pAmount = hAspOutput->writeFifo.size;
    *pBuffer2 = NULL;
    *pAmount2 = 0;

    BDBG_MSG((FMT_PREFIX "hAspOutput=%p: pBuffer=%p amount=%u"
              ARG_PREFIX, (void *)hAspOutput, *pBuffer, *pAmount));

error:
    return (rc);
}


static NEXUS_Error NEXUS_AspOutput_WriteComplete(
    NEXUS_AspOutputHandle              hAspOutput,
    unsigned                            amount       /* number of bytes written to the buffer. */
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    if (hAspOutput->state != NEXUS_AspOutputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_WriteComplete() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));
        return (BERR_TRACE(NEXUS_NOT_SUPPORTED));
    }

    hAspOutput->writeFifoLength = amount;
    BMMA_FlushCache_isrsafe( hAspOutput->writeFifo.hBlock, (char *)hAspOutput->writeFifo.pBuffer, amount );

    BDBG_MSG((FMT_PREFIX "hAspOutput=%p: amount=%u"
              ARG_PREFIX, (void *)hAspOutput, amount));

    return (NEXUS_SUCCESS);
}


NEXUS_Error NEXUS_AspOutput_SendHttpResponse(
    NEXUS_AspOutputHandle               hAspOutput,
    const void                          *pBuffer,       /* [in] attr{nelem=byteCount} pointer to HTTP response to be sent to network. */
    unsigned                            byteCount       /* [in] number of bytes in HTTP Response buffer. */
    )
{
    void        *pDst;
    void        *pDst2;
    unsigned    size;
    unsigned    size2;
    unsigned    count;

    NEXUS_Error  nrc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pBuffer);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    if (hAspOutput->state != NEXUS_AspOutputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspOutput=%p NEXUS_AspOutput_SendHttpResponse() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));
        return (BERR_TRACE(NEXUS_NOT_SUPPORTED));
    }

    nrc = NEXUS_AspOutput_GetWriteBufferWithWrap(hAspOutput, &pDst, &size, &pDst2, &size2);
    if (nrc != NEXUS_SUCCESS) {BERR_TRACE(nrc); goto error;}

    if (size + size2 < byteCount)
    {
        BDBG_ERR((FMT_PREFIX "HttpResponse is too long (%u bytes), only %u bytes available"
                  ARG_PREFIX, byteCount, size + size2 ));
        return (NEXUS_INVALID_PARAMETER);
    }

    count = (byteCount > size) ? size : byteCount;

    memcpy(pDst, pBuffer, count );

    if (byteCount > size)
    {
        memcpy(pDst2, (char *)pBuffer+count, byteCount-count );
    }

    nrc = NEXUS_AspOutput_WriteComplete(hAspOutput,  byteCount );
    if (nrc != NEXUS_SUCCESS) {BERR_TRACE(nrc); goto error;}

error:
    return nrc;
}


NEXUS_Error NEXUS_AspOutput_GetHttpRequestData(
    NEXUS_AspOutputHandle               hAspOutput,
    const void                          **pBuffer,      /* [out] attr{memory=cached} pointer to buffer containing data read from network. */
    unsigned                            *pByteCount     /* [out] number of bytes available in the data buffer pBuffer. */
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pByteCount);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    BSTD_UNUSED(hAspOutput);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pByteCount);
    return NEXUS_NOT_SUPPORTED;
}

NEXUS_Error NEXUS_AspOutput_HttpRequestDataConsumed(
    NEXUS_AspOutputHandle               hAspOutput,
    bool                                requestCompleted,     /* [in] false => Entire buffer is consumed. End of HttpRequest not found, more data is required. */
                                                              /*      true  => End of the HttpRequest has been found and consumed. No more data is required. */

    unsigned                            bytesConsumed         /* [in] If requestCompleted is true this is the number of bytes consumed from the current buffer.*/
    )                                                         /*      Else bytesConsumed must be equal to byte count returned by NEXUS_AspOutput_GetHttpRequestData. */
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    BSTD_UNUSED(hAspOutput);
    BSTD_UNUSED(requestCompleted);
    BSTD_UNUSED(bytesConsumed);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    return NEXUS_NOT_SUPPORTED;
}


NEXUS_Error NEXUS_AspOutput_GetBufferWithWrap(
    NEXUS_AspOutputHandle               hAspOutput,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pByteCount, /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                            *pByteCount2 /* [out] size of the available space in the pBuffer2 that can be written to network. */
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pByteCount);
    BDBG_ASSERT(pBuffer2);
    BDBG_ASSERT(pByteCount2);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    BSTD_UNUSED(hAspOutput);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pByteCount);
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pByteCount2);
    return NEXUS_NOT_SUPPORTED;
}


NEXUS_Error NEXUS_AspOutput_BufferSubmit(
    NEXUS_AspOutputHandle               hAspOutput,
    unsigned                            byteCount   /* number of bytes that AspOutput can send to the network. */
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(byteCount);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    BSTD_UNUSED(hAspOutput);
    BSTD_UNUSED(byteCount);
    return NEXUS_NOT_SUPPORTED;
}


/**
Summary:
API to Get Current Settings.
**/
void NEXUS_AspOutput_GetSettings(
    NEXUS_AspOutputHandle              hAspOutput,
    NEXUS_AspOutputSettings            *pSettings   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pSettings);

    *pSettings = hAspOutput->settings;
}


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error NEXUS_AspOutput_SetSettings(
    NEXUS_AspOutputHandle              hAspOutput,
    const NEXUS_AspOutputSettings      *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pSettings);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspOutput=%p state=%s"
               ARG_PREFIX, (void*)hAspOutput, NEXUS_AspOutputState_toString(hAspOutput->state)));

    hAspOutput->settings = *pSettings;

    NEXUS_IsrCallback_Set(hAspOutput->hEndOfStreamingCallback,       &hAspOutput->settings.endOfStreaming);
    NEXUS_IsrCallback_Set(hAspOutput->hRemoteDoneSendingCallback,    &hAspOutput->settings.remoteDoneSending);
    NEXUS_IsrCallback_Set(hAspOutput->hBufferReadyCallback,          &hAspOutput->settings.bufferReady);
    NEXUS_IsrCallback_Set(hAspOutput->hHttpRequestDataReadyCallback, &hAspOutput->settings.httpRequestDataReady);

    return (NEXUS_SUCCESS);
}


static void NEXUS_AspOutput_processMessage_isr(
    NEXUS_AspOutputHandle               hAspOutput,
    BASP_MessageType                    messageType,
    BASP_Fw2Pi_Message                  *pFw2PiMessage
    )
{

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);
    BDBG_ASSERT(pFw2PiMessage);

    BDBG_MSG((FMT_PREFIX "hAspOutput=%p msgType=%d pMessage=%p"
              ARG_PREFIX, (void *)hAspOutput, messageType, (void *)pFw2PiMessage));
    switch (messageType)
    {
        case BASP_MessageType_eFw2PiRstNotify:
        {
            if (hAspOutput->state == NEXUS_AspOutputState_eStreaming)
            {
                hAspOutput->connectionReset = true;

                BDBG_MSG((FMT_PREFIX "hAspOutput=%p Received RstNotify: connectionReset=true, firing hEndOfStreamingCallback!"
                          ARG_PREFIX, (void *)hAspOutput));
                NEXUS_IsrCallback_Fire_isr(hAspOutput->hEndOfStreamingCallback);
            }
            break;
        }
        case BASP_MessageType_eFw2PiFinNotify:  /* TODO: FIN handling has to be different. For now, just making it same as RST handling. */
        {
            if (hAspOutput->state == NEXUS_AspOutputState_eStreaming)
            {
                hAspOutput->remoteDoneSending = true;

                BDBG_MSG((FMT_PREFIX "hAspOutput=%p Received FinNotify: remoteDoneSending=true, firing hRemoteDoneSendingCallback!"
                          ARG_PREFIX, (void *)hAspOutput));
                NEXUS_IsrCallback_Fire_isr(hAspOutput->hRemoteDoneSendingCallback);
            }
            break;
        }
        case BASP_MessageType_eFw2PiPayloadNotify:
        {
            uint64_t offset;
            uint32_t relativeOffset;

            /* The PayloadNotify msg contains the offset into the receiveFifo. */
            offset = pFw2PiMessage->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32BaseAddrLo;
            offset |= (uint64_t)(pFw2PiMessage->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32BaseAddrHi) << 32;
            /* Now determine the relative offset into the receiveFifo. */
            relativeOffset = offset - hAspOutput->receiveFifo.offset;
            hAspOutput->pRcvdPayload = ((uint8_t *)hAspOutput->receiveFifo.pBuffer) + relativeOffset;
            hAspOutput->rcvdPayloadLength = pFw2PiMessage->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32Size;

            BMMA_FlushCache_isrsafe( hAspOutput->receiveFifo.hBlock, (char *)hAspOutput->receiveFifo.pBuffer, hAspOutput->rcvdPayloadLength );

            BDBG_MSG((FMT_PREFIX "hAspOutput=%p Rcvd PayloadNotify msg: offset hi:lo=0x%x:0x%x relativeOffset=%u pRcvdPayload=%p Length=%u, firing hBufferReadyCallback!"
                      ARG_PREFIX,
                        (void *)hAspOutput, (uint32_t)(offset>>32), (uint32_t)offset, relativeOffset, (void *)hAspOutput->pRcvdPayload, hAspOutput->rcvdPayloadLength ));

            NEXUS_IsrCallback_Fire_isr(hAspOutput->hBufferReadyCallback);
            break;
        }
        case BASP_MessageType_eFw2PiChannelAbortResp:
        {
            if (hAspOutput->state == NEXUS_AspOutputState_eStreaming)
            {
                BDBG_MSG((FMT_PREFIX "hAspOutput=%p Processing Abort Response message from Network Peer!"
                          ARG_PREFIX, (void *)hAspOutput));

                if (hAspOutput->protocol == NEXUS_AspStreamingProtocol_eHttp)
                {
                    hAspOutput->tcpState.finalSendSequenceNumber = pFw2PiMessage->Message.ResponsePayload.AbortResponse.ui32CurrentSeqNumber;
                    hAspOutput->tcpState.finalRecvSequenceNumber = pFw2PiMessage->Message.ResponsePayload.AbortResponse.ui32CurrentAckedNumber;
                }

                hAspOutput->gotStopResponse = true;
                BKNI_SetEvent(hAspOutput->hMessageReceiveEvent);
            }
            break;
        }
        case BASP_MessageType_eFw2PiChannelStartStreamOutResp:
            BDBG_MSG((FMT_PREFIX "hAspOutput=%p Received StartStreamOut response message from ASP!"
                      ARG_PREFIX, (void *)hAspOutput));
            hAspOutput->gotStartResponse = true;
            BKNI_SetEvent(hAspOutput->hMessageReceiveEvent);
            break;

        case BASP_MessageType_eFw2PiChannelStopResp:
            hAspOutput->gotStopResponse = true;
            hAspOutput->finishCompleted = true;

            BDBG_MSG((FMT_PREFIX "hAspOutput=%p Received Stop response: finishCompleted=true, firing hEndOfStreamingCallback!"
                      ARG_PREFIX, (void *)hAspOutput));
            NEXUS_IsrCallback_Fire_isr(hAspOutput->hEndOfStreamingCallback);
            break;

        case BASP_MessageType_eFw2PiPayloadConsumedResp:
            BDBG_MSG((FMT_PREFIX "hAspOutput=%p Received PayloadConsumed response message from ASP!"
                      ARG_PREFIX, (void *)hAspOutput));
            hAspOutput->gotPayloadConsumedResponse = true;
            BKNI_SetEvent(hAspOutput->hMessageReceiveEvent);
            break;

        case BASP_MessageType_eFw2PiChannelStartStreamInResp:
        case BASP_MessageType_eFw2PiChannelAbortWithRstResp:
        case BASP_MessageType_eFw2PiPerformanceGatheringResp:
        case BASP_MessageType_eFw2PiGenericSgTableFeedResp:
        case BASP_MessageType_eFw2PiFrameConsumedResp:
        case BASP_MessageType_eFw2PiGetDrmConstResp:

        case BASP_MessageType_eFw2PiFrameAvailable:
        case BASP_MessageType_eFw2PiFinComplete:
        case BASP_MessageType_eFw2PiGenericSgTableFeedCompletion:
        case BASP_MessageType_eFw2PiRtoNotify:
        default:
        {
            BDBG_WRN((FMT_PREFIX "!!!!!! TODO: hAspOutput=%p msgType=%d is NOT YET Handled by Nexus!!!"
                      ARG_PREFIX, (void *)hAspOutput, messageType));
            break;
        }
    }
}


#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS

static void NEXUS_ProcessMsgFromFwCallbackByTimer(void *pContext)
{
    int                     channelNum;
    bool                    foundChannel = false;

    BDBG_MSG((FMT_PREFIX "Entered by timer..."
              ARG_PREFIX ));

    g_NEXUS_asp.hAspOutputTimer = NULL;

    /* Loop through each channel. */
    for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
    {
        if (g_NEXUS_asp.hAspOutputList[channelNum] == NULL) { continue; }  /* This AspOutput not active, try next one. */

        foundChannel = true;

        pContext = g_NEXUS_asp.hAspOutputList[channelNum];

        BKNI_EnterCriticalSection();
        NEXUS_AspOutput_ProcessMsgFromFwCallback_isr(pContext, 0);
        BKNI_LeaveCriticalSection();
    }

    if (foundChannel)
    {
        BDBG_MSG((FMT_PREFIX "Starting NEXUS TIMER for %u ms"
                  ARG_PREFIX, g_NEXUS_asp.timerIntervalInMs ));
        g_NEXUS_asp.hAspOutputTimer = NEXUS_ScheduleTimer(g_NEXUS_asp.timerIntervalInMs, NEXUS_ProcessMsgFromFwCallbackByTimer, NULL);
        BDBG_ASSERT(g_NEXUS_asp.hAspOutputTimer);
    }
}
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */


static void NEXUS_AspOutput_ProcessMsgFromFwCallback_isr(void *pContext, int param)
{
    NEXUS_Error            rc;
    NEXUS_AspOutputHandle  hAspOutput = pContext;

    BDBG_OBJECT_ASSERT(hAspOutput, NEXUS_AspOutput);

    BSTD_UNUSED(param);

    /* Check if there is a response message or network generated event type message from ASP firmware. */
    while (true)
    {
        BASP_Fw2Pi_Message      fw2PiMessage;
        BASP_MessageType        msgType;
        unsigned                msgLen;

        BKNI_Memset_isr(&fw2PiMessage, 0, sizeof(fw2PiMessage));
        rc = BASP_Channel_ReadMessage_isr(hAspOutput->hChannel, &msgType, &fw2PiMessage, &msgLen);
        if (rc == BERR_NOT_AVAILABLE) {break;}
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR((FMT_PREFIX "BASP_Channel_ReadMessage_isr() for hAspOutput=%p failed! status=%u"
                      ARG_PREFIX, (void*) hAspOutput, rc ));
            BERR_TRACE(rc);
            return;
        }

        /* Make sure the message belongs to the channel that we asked for. */
        BDBG_ASSERT((unsigned)hAspOutput->channelNum == fw2PiMessage.MessageHeader.ui32ChannelIndex);

        NEXUS_AspOutput_processMessage_isr(hAspOutput, fw2PiMessage.MessageHeader.MessageType, &fw2PiMessage);
    }

    return;
}
