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
#include "bchp_xpt_mcpb_ch0.h"
#include "bchp_xpt_mcpb_ch1.h"
#include "bchp_asp_mcpb.h"
#include "bchp_asp_mcpb_ch0.h"
#include "bchp_asp_mcpb_ch1.h"
#include "bchp_asp_edpkt_core.h"

BDBG_MODULE(nexus_asp_input);

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

#define NEXUS_AspInputState_toString(state)  (state==NEXUS_AspInputState_eIdle            ? "Idle"      :     \
                                               state==NEXUS_AspInputState_eConnected       ? "Connected" :     \
                                               state==NEXUS_AspInputState_eStreaming       ? "Streaming" :     \
                                                                                              "Invalid!!!")

/* Per ASP input Channel Structure. */
typedef struct NEXUS_AspInput
{
    NEXUS_OBJECT(NEXUS_AspInput);
    NEXUS_AspInputState                state;
    bool                                finishRequested;
    bool                                finishCompleted;
    bool                                connectionReset;
    bool                                networkTimeout;
    bool                                remoteDoneSending;

    int                                 channelNum;
    BASP_ChannelHandle                  hChannel;
    NEXUS_AspStreamingProtocol          protocol;

    NEXUS_AspInputCreateSettings       createSettings;
    NEXUS_AspInputSettings             settings;
    NEXUS_AspInputStartSettings        startSettings;
    NEXUS_AspInputConnectHttpSettings  connectHttpSettings;
    NEXUS_AspTcpSettings               tcpSettings;

    NEXUS_AspBuffer                     writeFifo;
    NEXUS_AspBuffer                     reassembledFifo;
    NEXUS_AspBuffer                     reTransmitFifo;
    NEXUS_AspBuffer                     receiveFifo;
    NEXUS_AspBuffer                     miscBuffer;
    NEXUS_AspBuffer                     m2mDmaDescBuffer;

    unsigned                            writeFifoLength;    /* # of bytes filled-into the write buffer. */
    unsigned char                       *pRcvdPayload;      /* Starting position of the last received payload */
    unsigned                            rcvdPayloadLength;
    unsigned                            receiveFifoBytesConsumed;

    NEXUS_AspInputTcpStatus            initialStatus;
    NEXUS_AspInputTcpStatus            currentStatus;

    NEXUS_IsrCallbackHandle             hEndOfStreamingCallback;
    NEXUS_IsrCallbackHandle             hBufferReadyCallback;
    NEXUS_IsrCallbackHandle             hHttpResponseDataReadyCallback;

    BKNI_EventHandle                    hMessageReceiveEvent;

    struct
    {
        unsigned                        finalSendSequenceNumber;
        unsigned                        finalRecvSequenceNumber;
    } tcpState;

    bool                                gotStartResponse;
    bool                                gotStopResponse;
    bool                                gotPayloadConsumedResponse;

    NEXUS_AspInputDrmType              drmType;
    NEXUS_AspInputDtcpIpSettings       dtcpIpSettings;
#ifdef NEXUS_HAS_SECURITY
    BHSM_KeyslotHandle                  hKeySlot;
    unsigned                            extKeyTableSlotIndex;
#endif
} NEXUS_AspInput;

#if NEXUS_HAS_SECURITY
#define LOCK_SECURITY() NEXUS_Module_Lock(g_NEXUS_asp.settings.modules.security)
#define UNLOCK_SECURITY() NEXUS_Module_Unlock(g_NEXUS_asp.settings.modules.security)
#endif

#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
static void NEXUS_ProcessMsgFromFwCallbackByTimer(void *pContext);
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
static void NEXUS_AspInput_ProcessMsgFromFwCallback_isr(void *pContext, int param);

#define APP_TS_PKT_LEN              188
#define APP_TS_PKTS_IN_IP_PKT       7
#define APP_NUM_IP_PKTS             10
#define APP_TCP_START_SEQ_NUM       1000
#define APP_TCP_START_ACK_NUM       512

#define MAX_ASP_MESSAGE_RESPONSE_TIME_IN_MS (1000)


static NEXUS_Error NEXUS_AspInput_DoAspMessage(
    NEXUS_AspInputHandle    hAspInput,
    BASP_MessageType        messageType,
    BASP_Pi2Fw_Message      *pMessageToSend,
    bool                    *responseCompleteFlag)
{
    BERR_Code   rc = BERR_SUCCESS;
    NEXUS_Time  timeStart;
    NEXUS_Time  timeEnd;
    long        timeElapsed;

    static long timeMax = 0;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pMessageToSend);
    BDBG_ASSERT(responseCompleteFlag);

    NEXUS_Time_Get(&timeStart);

    /* Send StreamOut message to ASP. */
    BDBG_MSG((FMT_PREFIX "hAspInput=%p Sending MessageType %u..."
              ARG_PREFIX, (void *)hAspInput, messageType));

    *responseCompleteFlag = false;
    BKNI_ResetEvent(hAspInput->hMessageReceiveEvent);

    rc = BASP_Channel_SendMessage( hAspInput->hChannel,
                                    messageType,
                                    BASP_ResponseType_eAckRespRequired,
                                    pMessageToSend);
    if (rc != BERR_SUCCESS) {BERR_TRACE(rc); goto error;}

    BDBG_MSG((FMT_PREFIX "hAspInput=%p MessageType %u sent..."
              ARG_PREFIX, (void *)hAspInput, messageType));

    /* Wait for the response to the message. */
    for (;;)
    {
        rc = BKNI_WaitForEvent(hAspInput->hMessageReceiveEvent, MAX_ASP_MESSAGE_RESPONSE_TIME_IN_MS);
        if (*responseCompleteFlag) { break; }
        if (rc != BERR_SUCCESS) {BERR_TRACE(rc); goto error;}

        BDBG_WRN((FMT_PREFIX "hAspInput=%p MessageType %u, Event set, but no message response!"
                  ARG_PREFIX, (void *)hAspInput, messageType));
    }

    NEXUS_Time_Get(&timeEnd);
    timeElapsed = NEXUS_Time_Diff(&timeEnd, &timeStart);
    if (timeElapsed > timeMax) {timeMax = timeElapsed; }

    BDBG_LOG((FMT_PREFIX "hAspInput=%p MessageType %u response received, ET=%ld ms, Max ET=%ld ms."
              ARG_PREFIX, (void *)hAspInput, messageType, timeElapsed, timeMax));

error:
    return rc;

}


static void updateStats(
    NEXUS_AspInputHandle               hAspInput,
    BREG_Handle                         hReg,
    int                                 channelNumber,
    NEXUS_AspInputTcpStatus            *pInitialStats,
    NEXUS_AspInputTcpStatus            *pStatus
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

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

        pStatus->mcpbConsumedInBytes = pStatus->mcpbConsumedInTsPkts;
        pStatus->mcpbConsumedInIpPkts = pStatus->mcpbConsumedInTsPkts; /* Each MCPB local counter accounts for 1 ACK packet. */

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

    /* XPT MCPB Stats. */
    {
        uint32_t mcpbChFieldOffset;
        uint32_t mcpbChannelSize = BCHP_XPT_MCPB_CH1_RUN - BCHP_XPT_MCPB_CH0_RUN;
        uint32_t newValue, curValue;

        /* Find offset for the ASP MCPB channel being used. */
        mcpbChFieldOffset = BCHP_XPT_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER + (channelNumber * mcpbChannelSize);
        newValue = BREG_Read32(hReg, mcpbChFieldOffset);
        curValue = (uint32_t)(pStatus->xptMcpbConsumedInTsPkts & 0xFFFFFFFF);
        pStatus->xptMcpbConsumedInTsPkts += newValue - curValue;
        BDBG_MSG(("MCPBPktCounter value: new=%u cur=%u hi=%u lo=%u", newValue, curValue, (uint32_t)(pStatus->xptMcpbConsumedInTsPkts >> 32), (uint32_t)(pStatus->xptMcpbConsumedInTsPkts&0xFFFFFFFF)));

        pStatus->xptMcpbConsumedInBytes = pStatus->xptMcpbConsumedInTsPkts * 188; /* TODO: */
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
    const NEXUS_AspInputCreateBufferSettings  *pBufferSettings,
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



/******************** ASP input Channel Specific API functions ***************/
void NEXUS_AspInput_GetDefaultCreateSettings(NEXUS_AspInputCreateSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->reTransmitFifo.size = 8 * 1024;          /* KMB of Default Retransmission Queue. */
    pSettings->receiveFifo.size = 1024 * 1024;          /* 1MB of Default Read Queue. */
    pSettings->m2mDmaDescBuffer.size = 64*256;          /* 256 Descriptors each of 64B in size. */
    pSettings->reassembledFifo.size = 64 * 1024;        /* 64KB of Default Re-assembled Queue. */
    pSettings->writeFifo.size = 4 * 1024;               /* 4KB of Default Write Queue. */
    pSettings->miscBuffer.size = 4 * 1024;              /* 4KB of Default Write Queue. */
}

NEXUS_AspInputHandle NEXUS_AspInput_Create(
    const NEXUS_AspInputCreateSettings *pSettings
    )
{
    int                     channelNum;
    NEXUS_Error             rc;
    NEXUS_AspInputHandle    hAspInput = NULL;

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p"
               ARG_PREFIX, (void*)hAspInput));

    /* Check if we have a free ASP Channel, return error otherwise. */
    for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
    {
        if (g_NEXUS_asp.hAspChannelList[channelNum] == NULL &&      /* TODO: Eventually, this will be changed to hAspInputList. */
            g_NEXUS_asp.hAspInputList  [channelNum] == NULL &&
            g_NEXUS_asp.hAspInputList  [channelNum] == NULL )
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

    hAspInput = BKNI_Malloc(sizeof(*hAspInput));
    if (!hAspInput)
    {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(hAspInput, 0, sizeof *hAspInput);
#if 0
#endif
    NEXUS_OBJECT_INIT(NEXUS_AspInput, hAspInput);
    g_NEXUS_asp.hAspChannelList[channelNum] = (struct NEXUS_AspChannel*)hAspInput;      /* TODO: Eventually, hAspChannelList will be removed. */
    g_NEXUS_asp.hAspInputList[channelNum] = hAspInput;
    hAspInput->channelNum = channelNum;

    if (!pSettings)
    {
        NEXUS_AspInput_GetDefaultCreateSettings(&hAspInput->createSettings);
    }
    else
    {
        hAspInput->createSettings = *pSettings;
    }

    /* Allocate Per Channel Context Memory. All of these memory blocks are for ASP FW/HW usage. */
    rc = allocateAndLockBuffer(&pSettings->writeFifo,           true  /*needCpuAccess*/, BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES,   &hAspInput->writeFifo);
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    rc = allocateAndLockBuffer(&pSettings->reassembledFifo,     true  /*needCpuAccess*/, BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES,   &hAspInput->reassembledFifo);
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    rc = allocateAndLockBuffer(&pSettings->reTransmitFifo,      false /*needCpuAccess*/, BASP_EPKT_RETX_BUFFER_ALIGNMENT_IN_BYTES, &hAspInput->reTransmitFifo);
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    rc = allocateAndLockBuffer(&pSettings->receiveFifo,         true  /*needCpuAccess*/, BASP_RECEIVE_BUFFER_ALIGNMENT_IN_BYTES,   &hAspInput->receiveFifo); /* TODO: see if we can use a different rcv buffer for receiving HTTP Response as this wastes the virtual memory for all of the Rx buffer where as we only need it for the initial Response part. */
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    rc = allocateAndLockBuffer(&pSettings->miscBuffer,          false /*needCpuAccess*/, BASP_MISC_BUFFER_ALIGNMENT_IN_BYTES,      &hAspInput->miscBuffer);
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    rc = allocateAndLockBuffer(&pSettings->m2mDmaDescBuffer,    false /*needCpuAccess*/, BASP_MISC_BUFFER_ALIGNMENT_IN_BYTES,      &hAspInput->m2mDmaDescBuffer);
    if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }

    /* Create a Streaming Channel with the BASP base module. */
    {
        BASP_ChannelCreateSettings createSettings;

        BASP_Channel_GetDefaultCreateSettings(&createSettings);
        BDBG_MSG(( FMT_PREFIX "Calling BASP_Channel_Create()..."
                   ARG_PREFIX));
        createSettings.channelNumber = channelNum;
        rc = BASP_Channel_Create(g_NEXUS_asp.hContext, &createSettings, &hAspInput->hChannel);
        if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    }

    hAspInput->state = NEXUS_AspInputState_eIdle;
    BDBG_MSG(( FMT_PREFIX "hAspInput=%p BASP hChannel=%p channelNumber=%d"
               ARG_PREFIX, (void *)hAspInput, (void *)hAspInput->hChannel, hAspInput->channelNum));

#if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
    BDBG_MSG(( FMT_PREFIX "Using NEXUS TIMER instead of interrupts"
               ARG_PREFIX));

    if (!g_NEXUS_asp.hAspInputTimer)
    {
        g_NEXUS_asp.timerIntervalInMs = 100;
        BDBG_MSG(( FMT_PREFIX "Starting NEXUS TIMER for %u ms"
                   ARG_PREFIX, g_NEXUS_asp.timerIntervalInMs ));
        g_NEXUS_asp.hAspInputTimer = NEXUS_ScheduleTimer(g_NEXUS_asp.timerIntervalInMs, NEXUS_ProcessMsgFromFwCallbackByTimer, NULL);
        BDBG_ASSERT(g_NEXUS_asp.hAspInputTimer);
    }
#else  /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
    BDBG_MSG(( FMT_PREFIX "Using BASP interrupts"
               ARG_PREFIX));
    {
        BASP_ChannelCallbacks callbacks;

        BASP_Channel_GetCallbacks(hAspInput->hChannel, &callbacks);

        callbacks.messageReady.pCallback_isr = NEXUS_AspInput_ProcessMsgFromFwCallback_isr;
        callbacks.messageReady.pParam1 = hAspInput;
        callbacks.messageReady.param2 = 0;

        rc = BASP_Channel_SetCallbacks(hAspInput->hChannel, &callbacks);
        if (rc != NEXUS_SUCCESS) { BERR_TRACE(rc); goto error; }
    }
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */

    hAspInput->hEndOfStreamingCallback = NEXUS_IsrCallback_Create(hAspInput, NULL);
    if (hAspInput->hEndOfStreamingCallback == NULL) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto error; }
    NEXUS_CallbackDesc_Init(&hAspInput->settings.endOfStreaming);

    hAspInput->hBufferReadyCallback = NEXUS_IsrCallback_Create(hAspInput, NULL);
    if (hAspInput->hBufferReadyCallback == NULL) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto error; }
    NEXUS_CallbackDesc_Init(&hAspInput->settings.bufferReady);

    hAspInput->hHttpResponseDataReadyCallback = NEXUS_IsrCallback_Create(hAspInput, NULL);
    if (hAspInput->hHttpResponseDataReadyCallback == NULL) { rc = BERR_TRACE(NEXUS_UNKNOWN); goto error; }
    NEXUS_CallbackDesc_Init(&hAspInput->settings.httpResponseDataReady);

    rc = BKNI_CreateEvent(&hAspInput->hMessageReceiveEvent);
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
    BKNI_Memset(&hAspInput->dtcpIpSettings, 0, sizeof(hAspInput->dtcpIpSettings));
    hAspInput->dtcpIpSettings.pcpPayloadSize = NEXUS_ASP_NUM_BLOCKS_IN_A_CHUNK*NEXUS_ASP_BLOCK_SIZE;

    BDBG_MSG(( FMT_PREFIX "Returning hAspInput=%p using channelNum=%d."
               ARG_PREFIX, (void*)hAspInput, hAspInput->channelNum ));

    return hAspInput;
error:
    NEXUS_AspInput_Destroy(hAspInput);
    return NULL;
}

static void NEXUS_AspInput_P_Finalizer(
    NEXUS_AspInputHandle hAspInput
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));
    BDBG_ASSERT(hAspInput);

    NEXUS_OBJECT_ASSERT(NEXUS_AspInput, hAspInput);

    if (hAspInput->hMessageReceiveEvent)
    {
        BKNI_DestroyEvent(hAspInput->hMessageReceiveEvent);
        hAspInput->hMessageReceiveEvent = NULL;
    }

    if (hAspInput->hEndOfStreamingCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspInput->hEndOfStreamingCallback);
        hAspInput->hEndOfStreamingCallback = NULL;
    }
    if (hAspInput->hBufferReadyCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspInput->hBufferReadyCallback);
        hAspInput->hBufferReadyCallback = NULL;
    }
    if (hAspInput->hHttpResponseDataReadyCallback)
    {
        NEXUS_IsrCallback_Destroy(hAspInput->hHttpResponseDataReadyCallback);
        hAspInput->hHttpResponseDataReadyCallback = NULL;
    }

    g_NEXUS_asp.hAspChannelList[hAspInput->channelNum] = NULL;      /* TODO: Eventually, hAspChannelList will be removed. */
    g_NEXUS_asp.hAspInputList[hAspInput->channelNum] = NULL;

    #if USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS
    /* Only cancel the timer if no other channels are active. */
    {
        int channelNum;

        for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
        {
            if (g_NEXUS_asp.hAspInputList[channelNum]) { break; }
        }
        if (channelNum == BASP_MAX_NUMBER_OF_CHANNEL)
        {
            BDBG_MSG(("%s: No active ASP Channels, canceling the timer!", BSTD_FUNCTION));
            BDBG_ASSERT(g_NEXUS_asp.hAspInputTimer);
            NEXUS_CancelTimer(g_NEXUS_asp.hAspInputTimer);
            g_NEXUS_asp.hAspInputTimer = NULL;
        }
    }

#else  /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */
    /* Disable the messageReady callbacks for this channel. */
    if (hAspInput->hChannel)
    {
        BERR_Code               rc;
        BASP_ChannelCallbacks   callbacks;

        BDBG_MSG(( FMT_PREFIX "Disabling BASP messageReady callbacks for channel %p!"
                   ARG_PREFIX, (void*)hAspInput->hChannel));

        BASP_Channel_GetCallbacks(hAspInput->hChannel, &callbacks);

        callbacks.messageReady.pCallback_isr = NULL;
        callbacks.messageReady.pParam1 = NULL;
        callbacks.messageReady.param2 = 0;

        rc = BASP_Channel_SetCallbacks(hAspInput->hChannel, &callbacks);
        BERR_TRACE(rc);
    }
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */

    if (hAspInput->hChannel) {BASP_Channel_Destroy(hAspInput->hChannel);}

    unLockAndFreeBuffer(&hAspInput->writeFifo);
    unLockAndFreeBuffer(&hAspInput->reassembledFifo);
    unLockAndFreeBuffer(&hAspInput->reTransmitFifo);
    unLockAndFreeBuffer(&hAspInput->receiveFifo);
    unLockAndFreeBuffer(&hAspInput->miscBuffer);
    unLockAndFreeBuffer(&hAspInput->m2mDmaDescBuffer);

    NEXUS_OBJECT_DESTROY(NEXUS_AspInput, hAspInput);
    BKNI_Free(hAspInput);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AspInput, NEXUS_AspInput_Destroy);


/**
Summary:
Get Default StartSettings.
**/
void NEXUS_AspInput_GetDefaultStartSettings(
    NEXUS_AspInputStartSettings       *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->feedMode = NEXUS_AspInputFeedMode_eAuto;
}


static void printStartSettings(
    const NEXUS_AspInputStartSettings *pSettings
    )
{
    BDBG_ASSERT(pSettings);

    BDBG_MSG(( FMT_PREFIX "feedMode=%s"
               ARG_PREFIX,
               pSettings->feedMode == NEXUS_AspInputFeedMode_eAuto                   ? "eAuto" :
               pSettings->feedMode == NEXUS_AspInputFeedMode_eHost                   ? "eHost" :
               pSettings->feedMode == NEXUS_AspInputFeedMode_eAutoWithHostDecryption ? "eAutoWithHostDecryption" :
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
    const NEXUS_AspTcpSettings                 *pTcpSettings,
    const NEXUS_AspInputConnectHttpSettings    *pSettings
    )
{
    BDBG_ASSERT(pTcpSettings);
    BDBG_ASSERT(pSettings);

    BDBG_MSG((FMT_PREFIX "transportType=%d, hPlaypump=%p, hDma=%p"
              ARG_PREFIX,
                pSettings->transportType,
                (void*)pSettings->hPlaypump,
                (void*)pSettings->hDma
                ));

    printTcpSettings(pTcpSettings);

} /* printConnectHttpSettings */


#if 0 /* ******************** Temporary by Gary **********************/
#ifdef NEXUS_HAS_SECURITY
static BHSM_KeyslotHandle allocAndConfigKeySlot(
    NEXUS_AspInputHandle  hAspInput
    )
{
    BERR_Code rc;
    BHSM_KeyslotAllocateSettings keySlotSettings;
    BHSM_KeyslotHandle hKeySlot = NULL;
    BHSM_KeyslotSettings hsmKeyslotSettings;
    BHSM_KeyslotEntrySettings hsmEntrySettings;
    BHSM_KeyslotExternalKeyData extKeyTableData;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

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
    rc = BHSM_Keyslot_AddPidChannel( hKeySlot, ASP_PID_CHANNEL_BASE + hAspInput->channelNum );
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
    hAspInput->extKeyTableSlotIndex = extKeyTableData.slotIndex;

    BDBG_LOG((FMT_PREFIX "Done: extKeyTableSlotIndex=%u"
              ARG_PREFIX, hAspInput->extKeyTableSlotIndex));
    return (hKeySlot);

error:
    if (hKeySlot != NULL)
    {
        LOCK_SECURITY();
        BDBG_LOG((FMT_PREFIX "Error! Freeing hKeySlot=%p"
                  ARG_PREFIX, (void*)hAspInput->hKeySlot));
        BHSM_Keyslot_Free( hAspInput->hKeySlot ); hAspInput->hKeySlot = NULL;
        UNLOCK_SECURITY();
    }
    return NULL;

} /* allocAndConfigKeySlot */
#endif
#endif /* ******************** Temporary by Gary **********************/


void NEXUS_AspInput_GetDefaultConnectHttpSettings(
    NEXUS_AspInputConnectHttpSettings       *pSettings  /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->transportType = NEXUS_TransportType_eUnknown;
} /* NEXUS_AspInput_GetDefaultConnectHttpSettings */


void NEXUS_AspInput_GetDefaultTcpSettings(
    NEXUS_AspTcpSettings                    *pSettings  /* [out] */
    )
{
    BDBG_ASSERT(pSettings);
    if (!pSettings) return;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
} /* NEXUS_AspInput_GetDefaultTcpSettings */


NEXUS_Error NEXUS_AspInput_ConnectHttp(
    NEXUS_AspInputHandle                       hAspInput,
    const NEXUS_AspTcpSettings                 *pTcpSettings,
    const NEXUS_AspInputConnectHttpSettings    *pConnectHttpSettings
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pConnectHttpSettings);
    BDBG_ASSERT(pTcpSettings);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    printConnectHttpSettings(pTcpSettings, pConnectHttpSettings);

    if (hAspInput->state != NEXUS_AspInputState_eIdle)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_ConnectHttp() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

        return (NEXUS_NOT_SUPPORTED);
    }

    hAspInput->connectHttpSettings = *pConnectHttpSettings;
    hAspInput->tcpSettings         = *pTcpSettings;
    hAspInput->protocol            = NEXUS_AspStreamingProtocol_eHttp;

    hAspInput->state = NEXUS_AspInputState_eConnected;

    return (NEXUS_SUCCESS);
}


/**
Summary:
Start an AspInput.
Note: it will only starts streaming if NEXUS_AspInputStartSettings.autoStartStreaming flag is set to true!
**/
NEXUS_Error NEXUS_AspInput_Start(
    NEXUS_AspInputHandle              hAspInput,
    const NEXUS_AspInputStartSettings *pSettings
    )
{
    NEXUS_Error                             nrc;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pSettings);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    printStartSettings(pSettings);

    if (hAspInput->state != NEXUS_AspInputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_ConnectHttp() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

        nrc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }

    hAspInput->startSettings = *pSettings;

    /* TODO: check for if (pSettings->autoStartStreaming) */

    if (hAspInput->tcpSettings.connectionLost)
    {
        hAspInput->connectionReset = true;
        hAspInput->state = NEXUS_AspInputState_eStreaming;

        BDBG_MSG((FMT_PREFIX "hAspInput=%p Starting with connectionLost: connectionReset=true, firing hEndOfStreamingCallback!"
                  ARG_PREFIX, (void *)hAspInput));
        BKNI_EnterCriticalSection();
        NEXUS_IsrCallback_Fire_isr(hAspInput->hEndOfStreamingCallback);
        BKNI_LeaveCriticalSection();
        return (NEXUS_SUCCESS);
    }

    hAspInput->state = NEXUS_AspInputState_eStreaming;

    return (NEXUS_SUCCESS);

error:
    return (nrc);
}


static NEXUS_Error buildAndSendStartStreaminMessage(
    NEXUS_AspInputHandle              hAspInput
    )
{
    NEXUS_Error                             nrc;
    BASP_Pi2Fw_Message                      msg;
    BASP_ChannelStartStreamInMessage        *pStreamIn = NULL;
    BASP_ConnectionControlBlock             *pCcb = NULL;
    const NEXUS_AspIpSettings               *pIpSettings = NULL;
    const NEXUS_AspTcpSettings              *pTcpSettings = NULL;
    const NEXUS_AspEthernetSettings         *pEthSettings = NULL;
    static bool                             fwTimestampSynced = false;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    if (hAspInput->state != NEXUS_AspInputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_ConnectHttp() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

        nrc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
    }

    /* Build Start Message. */
    BKNI_Memset(&msg, 0, sizeof(msg));

    /* Ignore the message header fields, those are populated by the BASP base module. */

    /* Build message payload. */
    pStreamIn = &msg.MessagePayload.ChannelStartStreamIn;

    pStreamIn->ui32SendRstPktOnRetransTimeOutEnable = 0;
    pStreamIn->ui32KeepAliveTimerEnable = 1;
    pStreamIn->ui32DrmEnabled = 0;

    {
        NEXUS_PlaypumpStatus     status;

        BDBG_ASSERT(hAspInput->connectHttpSettings.hPlaypump);
        nrc = NEXUS_Playpump_GetStatus(hAspInput->connectHttpSettings.hPlaypump, &status);
        BDBG_ASSERT(nrc == NEXUS_SUCCESS);

        pStreamIn->ui32PlaybackChannelNumber = status.index;
    }
#define NEXUS_ASP_BASE_PID_CHANNEL_NUMBER 1024
    pStreamIn->ui32PidChannel = NEXUS_ASP_BASE_PID_CHANNEL_NUMBER + hAspInput->channelNum;

    BDBG_MSG(("%s:%p !!!!!! Setting up stream-in channel: pbChannel#=%u pidCh#=%u", BSTD_FUNCTION,
                (void *)hAspInput, pStreamIn->ui32PlaybackChannelNumber, pStreamIn->ui32PidChannel));
    /* Setup initial data: HTTP Get Request for StreamIn case. */
    pStreamIn->HttpRequestBuffer.ui32BaseAddrLo = (uint32_t) (hAspInput->writeFifo.offset & 0xFFFFFFFF);
    pStreamIn->HttpRequestBuffer.ui32BaseAddrHi = (uint32_t) (hAspInput->writeFifo.offset >> 32);
    pStreamIn->HttpRequestBuffer.ui32Size = hAspInput->writeFifoLength;

    BMMA_FlushCache_isrsafe( hAspInput->writeFifo.hBlock, (char *)hAspInput->writeFifo.pBuffer, hAspInput->writeFifoLength );

    /* TODO: take the retransmit buffer out. */
    pStreamIn->ReTransmissionBuffer.ui32BaseAddrLo = (uint32_t)(hAspInput->reTransmitFifo.offset & 0xFFFFFFFF);
    pStreamIn->ReTransmissionBuffer.ui32BaseAddrHi = (uint32_t)(hAspInput->reTransmitFifo.offset>>32);
    pStreamIn->ReTransmissionBuffer.ui32Size = (uint32_t)hAspInput->reTransmitFifo.size;

    pStreamIn->EthernetHeaderBuffer.ui32BaseAddrLo = (uint32_t)hAspInput->miscBuffer.offset;
    pStreamIn->EthernetHeaderBuffer.ui32BaseAddrHi = (uint32_t)(hAspInput->miscBuffer.offset>>32);
    pStreamIn->EthernetHeaderBuffer.ui32Size = (uint32_t)hAspInput->miscBuffer.size;

    pStreamIn->ReceivePayloadBuffer.ui32BaseAddrLo = (uint32_t)(hAspInput->receiveFifo.offset & 0xFFFFFFFF);
    pStreamIn->ReceivePayloadBuffer.ui32BaseAddrHi = (uint32_t)(hAspInput->receiveFifo.offset>>32);
    pStreamIn->ReceivePayloadBuffer.ui32Size = (uint32_t)hAspInput->receiveFifo.size;

    BMMA_FlushCache_isrsafe( hAspInput->receiveFifo.hBlock, (char *)hAspInput->receiveFifo.pBuffer, hAspInput->receiveFifo.size );

    pStreamIn->MemDmaMcpbDramDescriptorBuffer.ui32BaseAddrLo = (uint32_t)(hAspInput->m2mDmaDescBuffer.offset & 0xFFFFFFFF);
    pStreamIn->MemDmaMcpbDramDescriptorBuffer.ui32BaseAddrHi = (uint32_t)(hAspInput->m2mDmaDescBuffer.offset>>32);
    pStreamIn->MemDmaMcpbDramDescriptorBuffer.ui32Size = (uint32_t)hAspInput->m2mDmaDescBuffer.size;

    pCcb = &pStreamIn->ConnectionControlBlock;

    /* Determine the pointers to various headers so that we can use them independent of the protocol. */
    if (hAspInput->protocol == NEXUS_AspStreamingProtocol_eHttp)
    {
        pEthSettings = &hAspInput->tcpSettings.ip.eth;
        pIpSettings  = &hAspInput->tcpSettings.ip;
        pTcpSettings = &hAspInput->tcpSettings;
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

        BKNI_Memcpy(pCcb->aui8DestMacAddr, pEthSettings->localMacAddress, BASP_MAX_ENTRY_IN_MAC_ADDR);
        BKNI_Memcpy(pCcb->aui8SrcMacAddr, pEthSettings->remoteMacAddress, BASP_MAX_ENTRY_IN_MAC_ADDR);

        pCcb->ui32EtherType      = pEthSettings->etherType;
        pCcb->ui32IngressBrcmTag = pEthSettings->networkSwitch.ingressBrcmTag;
        pCcb->ui32EgressClassId  = pEthSettings->networkSwitch.egressClassId;
    }

    /* IP header related settings: needed for all protocols. */
    {
        pCcb->ui32IpVersion = pIpSettings->ipVersion == NEXUS_AspIpVersion_e6 ? 6 : 4;
        if (pIpSettings->ipVersion == NEXUS_AspIpVersion_e4)
        {
            pCcb->ui32Dscp = pIpSettings->ver.v4.dscp;
            pCcb->ui32TimeToLive = pIpSettings->ver.v4.timeToLive;
            pCcb->ai32DestIpAddr[0] = pIpSettings->ver.v4.localIpAddr;
            pCcb->ai32SrcIpAddr[0] = pIpSettings->ver.v4.remoteIpAddr;
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
        pCcb->ui32DestPort = pTcpSettings->localPort;
        pCcb->ui32SrcPort = pTcpSettings->remotePort;

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
        pStreamIn->ui32SwitchQueueNumber = pEthSettings->networkSwitch.queueNumber; /* TODO: enable the loss-less behavior in switch & test this. */
#else /* ******************** Temporary **********************/
        pStreamIn->ui32SwitchQueueNumber = 0;  /* Currently, ASP will output packets to the queue 0 of its port. */
#endif /* ******************** Temporary **********************/
        BDBG_MSG(("%s: !!!!!! TODO: Overwriting the ui32SwitchQueueNumber(=%d) to 0 until ACH support is enabled in ASP & Network Switch", BSTD_FUNCTION, pStreamIn->ui32SwitchQueueNumber));
        pStreamIn->ui32SwitchSlotsPerEthernetPacket = 6; /* TODO get this number from FW/HW team & also make sure it gets programmed on each switch port. */

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
    if (hAspInput->drmType == NEXUS_AspInputDrmType_eDtcpIp)
    {
        BDBG_ERR(("!!! FIXME: Ignoring DTCP/IP handling for Stream-in!!"));
    }
#endif

#if 0
    {
        /* TODO: debug code to reset EDPKT stats upon every channel change. */
        BDBG_MSG((FMT_PREFIX "hAspInput=%p Resetting the EDPKT .."
                  ARG_PREFIX, (void *)hAspInput));
        BREG_Write32(g_pCoreHandles->reg, BCHP_ASP_EDPKT_CORE_TEST_REG0, 0x7c);
    }
#endif

    /* Send a StartStreamIn message to the ASP and wait for a response. */
    nrc = NEXUS_AspInput_DoAspMessage( hAspInput,
                                      BASP_MessageType_ePi2FwChannelStartStreamIn,
                                      &msg,
                                      &hAspInput->gotStartResponse);

    if (nrc != NEXUS_SUCCESS) {BERR_TRACE(nrc); goto error;}

    /* Save initial stats as some of the stats are commulative & we need the initial values to calculate the current  */
    {
        updateStats(hAspInput, g_pCoreHandles->reg, hAspInput->channelNum, NULL, &hAspInput->initialStatus);
        hAspInput->currentStatus = hAspInput->initialStatus;
    }

    /* TODO: Disable ACH logic in MCPB. */
    {
        unsigned value;
        value = BREG_Read32(g_pCoreHandles->reg, BCHP_ASP_MCPB_DIS_QUEUE_CHECK_IN_EPKT_MODE);
        value |= (1<<hAspInput->channelNum);
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

    return (NEXUS_SUCCESS);

error:
    return (nrc);
}


NEXUS_Error NEXUS_AspInput_Disconnect(
    NEXUS_AspInputHandle                       hAspInput,
    NEXUS_AspInputDisconnectStatus             *pStatus
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pStatus);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    if (hAspInput->state == NEXUS_AspInputState_eStreaming)
    {
        BDBG_MSG(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_Disconnect() called when state is %s, calling NEXUS_AspInput_Stop()"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));
        NEXUS_AspInput_Stop(hAspInput);
    }

    if (hAspInput->state != NEXUS_AspInputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_Disconnect() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

        return (NEXUS_NOT_SUPPORTED);
    }

    if (hAspInput->protocol == NEXUS_AspStreamingProtocol_eHttp)
    {
        pStatus->tcpState.finalSendSequenceNumber = hAspInput->tcpState.finalSendSequenceNumber;
#if 0 /* ******************** Temporary **********************/
        pStatus->tcpState.finalRecvSequenceNumber = hAspInput->tcpState.finalRecvSequenceNumber;
#else /* ******************** Temporary **********************/
        /* TODO: until FW is fixed to return the correct Recv Seq #, we use the original value. */
        pStatus->tcpState.finalRecvSequenceNumber = hAspInput->tcpState.finalRecvSequenceNumber;
#endif /* ******************** Temporary **********************/
    }

    hAspInput->drmType = NEXUS_AspInputDrmType_eNone;
    hAspInput->state = NEXUS_AspInputState_eIdle;

    return (NEXUS_SUCCESS);
}


/**
Summary:
Stop a AspInput.

Description:
This API sends the Abort message to FW to immediately Stop NEXUS_AspInput.
FW will NOT synchronize the protocol state (such as for TCP) in this case.
For that, caller should use NEXUS_AspInput_Finish()
**/
void NEXUS_AspInput_Stop(
    NEXUS_AspInputHandle              hAspInput
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    if (hAspInput->state != NEXUS_AspInputState_eStreaming)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_Stop() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));
        return;
    }

    if (hAspInput->tcpSettings.connectionLost)
    {
        hAspInput->state = NEXUS_AspInputState_eConnected;
        return;
    }

    {
        BASP_Pi2Fw_Message pi2FwMessage;

        BKNI_Memset(&pi2FwMessage, 0, sizeof(pi2FwMessage));
        pi2FwMessage.MessagePayload.ChannelAbort.ui32Unused = 0;

        /* Send a ChannelAbort message to the ASP and wait for a response. */
        rc = NEXUS_AspInput_DoAspMessage( hAspInput,
                                         BASP_MessageType_ePi2FwChannelAbort,
                                          &pi2FwMessage,
                                          &hAspInput->gotStopResponse);

        if (rc != NEXUS_SUCCESS) {BERR_TRACE(rc); goto error;}

        if (hAspInput->protocol == NEXUS_AspStreamingProtocol_eHttp)
        {
            BDBG_MSG((FMT_PREFIX "hAspInput=%p Abort Response Rcvd, seq#: final=%u initial=%u bytesSent=%u, ack#: final=%u initial=%u bytesRcvd=%u"
                      ARG_PREFIX, (void *)hAspInput,
                                  hAspInput->tcpState.finalSendSequenceNumber,
                                  hAspInput->tcpSettings.initialSendSequenceNumber,
                                  hAspInput->tcpState.finalSendSequenceNumber - hAspInput->tcpSettings.initialSendSequenceNumber,
                                  hAspInput->tcpState.finalRecvSequenceNumber,
                                  hAspInput->tcpSettings.initialRecvSequenceNumber,
                                  hAspInput->tcpState.finalRecvSequenceNumber - hAspInput->tcpSettings.initialRecvSequenceNumber
                                  ));
        }
    }

#ifdef NEXUS_HAS_SECURITY
    if (hAspInput->drmType == NEXUS_AspInputDrmType_eDtcpIp)
    {
        if (hAspInput->hKeySlot)
        {
            LOCK_SECURITY();
            BHSM_Keyslot_Free( hAspInput->hKeySlot ); hAspInput->hKeySlot = NULL;
            UNLOCK_SECURITY();
        }
        hAspInput->hKeySlot = NULL;
    }
#endif
    hAspInput->state = NEXUS_AspInputState_eConnected;

error:
    return;
}


/**
Summary:
Finish an AspInput.

Description:
This API initiates the normal finishing of ASP Channel.
FW will synchronize the protocol state (such as for TCP) in this case.
This may involve waiting for TCP ACKs for any pending data and thus may take time.
However, the API returns after sending the message to the FW.
Its completion will be notified via the statusChangedCallback.

For immediate aborting, caller should use NEXUS_AspInput_Stop().
**/
void NEXUS_AspInput_Finish(
    NEXUS_AspInputHandle              hAspInput
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    BDBG_ASSERT(hAspInput);

    if (hAspInput->state != NEXUS_AspInputState_eStreaming)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_Finish() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));
        return;
    }

    if (hAspInput->state == NEXUS_AspInputState_eStreaming)
    {
        BASP_Pi2Fw_Message pi2FwMessage;

        BKNI_Memset(&pi2FwMessage, 0, sizeof(pi2FwMessage));
        pi2FwMessage.MessagePayload.ChannelAbort.ui32Unused = 0;

        /* Since the "ChannelStop" may take some time, don't wait here for the response message. */
        rc = BASP_Channel_SendMessage( hAspInput->hChannel,
                                       BASP_MessageType_ePi2FwChannelStop,
                                       BASP_ResponseType_eAckRespRequired,
                                       &pi2FwMessage);
        if (rc != BERR_SUCCESS) {BERR_TRACE(rc); goto error;}
    }

#ifdef NEXUS_HAS_SECURITY
    if (hAspInput->drmType == NEXUS_AspInputDrmType_eDtcpIp)
    {
        if (hAspInput->hKeySlot)
        {
            LOCK_SECURITY();
            BHSM_Keyslot_Free( hAspInput->hKeySlot ); hAspInput->hKeySlot = NULL;
            UNLOCK_SECURITY();
        }
        hAspInput->hKeySlot = NULL;
    }
#endif

    hAspInput->finishRequested = true;

error:
    return;
}


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error NEXUS_AspInput_GetStatus(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputStatus              *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pStatus);

    pStatus->aspChannelIndex    = hAspInput->channelNum;
    pStatus->state              = hAspInput->state;
    pStatus->connectionReset    = hAspInput->connectionReset;
    pStatus->networkTimeout     = hAspInput->networkTimeout;
    pStatus->remoteDoneSending  = hAspInput->remoteDoneSending;

    return (NEXUS_SUCCESS);
}


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error NEXUS_AspInput_GetTcpStatus(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputTcpStatus           *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pStatus);

/*     hAspInput->currentStatus.state = hAspInput->state;  TODO: Should we add state to NEXUS_AspInputTcpStatus? */
    updateStats(hAspInput, g_pCoreHandles->reg, hAspInput->channelNum, &hAspInput->initialStatus, &hAspInput->currentStatus);
#if 0
    determineAspHwPipeStatus(hAspInput, g_pCoreHandles->reg, &hAspInput->currentStatus, &pStatus->stats);
#endif
    *pStatus = hAspInput->currentStatus;
/*      pStatus->aspChannelIndex = hAspInput->channelNum;    TODO: Should we add aspChannelIndex to NEXUS_AspInputTcpStatus? */

    /* ASP FW Stats: FW periodically updates the status buffer in DRAM. */
    {
        uint32_t newValue, curValue;
        BASP_FwStatusInfo *pFwStatusInfo = g_NEXUS_asp.statusBuffer.pBuffer;
        BASP_FwChannelInfo *pFwChannelInfo;

        BMMA_FlushCache_isrsafe( g_NEXUS_asp.statusBuffer.hBlock, (char *)pFwStatusInfo, sizeof(BASP_FwStatusInfo) );
        pFwChannelInfo = &(pFwStatusInfo->stChannelInfo[hAspInput->channelNum]);

        pStatus->fwStats.congestionWindow = pFwChannelInfo->ui32CongestionWindow;
        pStatus->fwStats.receiveWindow = pFwChannelInfo->ui32ReceivedWindowSize;

        pStatus->fwStats.sendWindow = pFwChannelInfo->ui32SendWindow;

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
        pStatus->fwStats.descriptorsFedToXpt = pFwChannelInfo->ui32NumOfDescriptorSent;
        pStatus->fwStats.bytesFedToXpt = pFwChannelInfo->ui32NumBytesFedToDescriptors;
    }

    pStatus->statsValid = true;
    return (NEXUS_SUCCESS);
}


NEXUS_Error NEXUS_AspInput_GetHttpStatus(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputHttpStatus          *pStatus     /* [out] */
    )
{
    NEXUS_Error nrc;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pStatus);

    nrc = NEXUS_AspInput_GetTcpStatus(hAspInput, &pStatus->tcp);

    /* TODO: maintain HTTP stats. */
    return (nrc);
}


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void NEXUS_AspInput_GetDtcpIpSettings(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputDtcpIpSettings      *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

    BDBG_ASSERT(pSettings);
    *pSettings = hAspInput->dtcpIpSettings;
}


/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to NEXUS_AspInput_Connect().
**/
NEXUS_Error NEXUS_AspInput_SetDtcpIpSettings(
    NEXUS_AspInputHandle              hAspInput,
    const NEXUS_AspInputDtcpIpSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pSettings);

    hAspInput->dtcpIpSettings = *pSettings;
    hAspInput->drmType = NEXUS_AspInputDrmType_eDtcpIp;
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
API to provide an HTTP request to be sent out on the network.

Note: This API can only be called when NEXUS_AspInputStatus.state is _eConnected.

**/
NEXUS_Error NEXUS_AspInput_SendHttpRequest(
    NEXUS_AspInputHandle                hAspInput,
    const void                          *pBuffer,       /* [in] attr{nelem=byteCount} pointer to HTTP request to be sent to network. */
    unsigned                            byteCount       /* [in] number of bytes in HTTP Request buffer. */
    )
{
    NEXUS_Error  nrc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pBuffer);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p byteCount=%u state=%s"
               ARG_PREFIX, (void*)hAspInput, byteCount, NEXUS_AspInputState_toString(hAspInput->state)));

    if (hAspInput->state != NEXUS_AspInputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_SendHttpResponse() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));
        return (BERR_TRACE(NEXUS_NOT_SUPPORTED));
    }

    if (byteCount > hAspInput->writeFifo.size)
    {
        BDBG_ERR((FMT_PREFIX "HttpResponse is too long (%u bytes), only %u bytes available"
                  ARG_PREFIX, byteCount, hAspInput->writeFifo.size ));
        return (NEXUS_INVALID_PARAMETER);
    }

    BKNI_Memcpy(hAspInput->writeFifo.pBuffer, pBuffer, byteCount);
    hAspInput->writeFifoLength = byteCount;

    BDBG_LOG(( FMT_PREFIX "writeFifo=\"%.*s\""
               ARG_PREFIX, hAspInput->writeFifoLength, (char*)hAspInput->writeFifo.pBuffer));

    /* Build and send start streamin message. */
    nrc =  buildAndSendStartStreaminMessage(hAspInput);
    if (nrc != NEXUS_SUCCESS)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p buildAndSendStartStreaminMessage failed."
                   ARG_PREFIX, (void*)hAspInput));

        BERR_TRACE(nrc);
        goto error;
    }

error:
    return nrc;
}


/**
Summary:
API to receive the incoming HTTP Response (from remote) for host access.
**/
NEXUS_Error NEXUS_AspInput_GetHttpResponseData(
    NEXUS_AspInputHandle                hAspInput,
    const void                          **pBuffer,      /* [out] attr{memory=cached} pointer to buffer containing data read from network. */
    unsigned                            *pByteCount     /* [out] number of bytes available in the data buffer pBuffer. */
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pByteCount);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    if (hAspInput->state != NEXUS_AspInputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_GetHttpResponseData() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));
        return (BERR_TRACE(NEXUS_NOT_SUPPORTED));
    }

    if (hAspInput->pRcvdPayload)
    {
        *pBuffer = hAspInput->pRcvdPayload;
        *pByteCount = hAspInput->rcvdPayloadLength;

        BDBG_MSG((FMT_PREFIX "hAspInput=%p: pBuffer=%p byteCount=%u"
                  ARG_PREFIX, (void *)hAspInput, *pBuffer, *pByteCount));

        return (NEXUS_SUCCESS);
    }
    else
    {
        return (NEXUS_NOT_AVAILABLE);
    }
}


NEXUS_Error NEXUS_AspInput_HttpResponseDataConsumed(
    NEXUS_AspInputHandle                hAspInput,
    bool                                responseCompleted,    /* [in] false => Entire buffer is consumed. End of HttpResponse not found, more data is required. */
                                                              /*      true  => End of the HttpResponse has been found and consumed. No more data is required. */

    unsigned                            bytesConsumed         /* [in] If responseComplete is true this is the number of bytes consumed from the current buffer.*/
    )                                                         /*      Else bytesConsumed must be equal to byte count returned by NEXUS_AspInput_GetHttpResponseData. */
{
    BERR_Code rc = NEXUS_SUCCESS;
    BASP_Pi2Fw_Message pi2FwMessage;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    if (hAspInput->state != NEXUS_AspInputState_eConnected)
    {
        BDBG_ERR(( FMT_PREFIX "hAspInput=%p NEXUS_AspInput_HttpResponseDataConsumed() is invalid when state is %s"
                   ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));
        return (BERR_TRACE(NEXUS_NOT_SUPPORTED));
    }

    hAspInput->pRcvdPayload = NULL;

    BKNI_Memset(&pi2FwMessage, 0, sizeof(pi2FwMessage));

    if (responseCompleted)
    {
        pi2FwMessage.MessagePayload.PayloadConsumed.ui32RequireMoreData = false;
    }
    else
    {
        pi2FwMessage.MessagePayload.PayloadConsumed.ui32RequireMoreData = true;
    }
    pi2FwMessage.MessagePayload.PayloadConsumed.ui32NumberofBytesToSkip = bytesConsumed;

    /* Send a StartStreamOut message to the ASP and wait for a response. */
    rc = NEXUS_AspInput_DoAspMessage( hAspInput,
                                        BASP_MessageType_ePi2FwPayloadConsumed,
                                        &pi2FwMessage,
                                        &hAspInput->gotPayloadConsumedResponse);

    if (rc != NEXUS_SUCCESS) {BERR_TRACE(rc); goto error;}

    BDBG_MSG((FMT_PREFIX "hAspInput=%p PayloadConsumed Msg Response Rcvd"
              ARG_PREFIX, (void *)hAspInput));

error:
    return (rc);
}


NEXUS_Error NEXUS_AspInput_GetBuffer(
    NEXUS_AspInputHandle                hAspInput,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which has been received from the network. */
    unsigned                            *pByteCount  /* [out] number of bytes available in the pBuffer before the wrap. */
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pByteCount);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    BSTD_UNUSED(hAspInput);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pByteCount);
    return NEXUS_NOT_SUPPORTED;
}


NEXUS_Error NEXUS_AspInput_BufferComplete(
    NEXUS_AspInputHandle                hAspInput,
    unsigned                            byteCount   /* number of bytes processed. */
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(byteCount);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    BSTD_UNUSED(hAspInput);
    BSTD_UNUSED(byteCount);
    return NEXUS_NOT_SUPPORTED;
}


/**
Summary:
API to Get Current Settings.
**/
void NEXUS_AspInput_GetSettings(
    NEXUS_AspInputHandle              hAspInput,
    NEXUS_AspInputSettings            *pSettings   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pSettings);

    *pSettings = hAspInput->settings;
}


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error NEXUS_AspInput_SetSettings(
    NEXUS_AspInputHandle              hAspInput,
    const NEXUS_AspInputSettings      *pSettings
    )
{
    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pSettings);

    BDBG_MSG(( FMT_PREFIX "Entry. hAspInput=%p state=%s"
               ARG_PREFIX, (void*)hAspInput, NEXUS_AspInputState_toString(hAspInput->state)));

    hAspInput->settings = *pSettings;

    NEXUS_IsrCallback_Set(hAspInput->hEndOfStreamingCallback,        &hAspInput->settings.endOfStreaming);
    NEXUS_IsrCallback_Set(hAspInput->hBufferReadyCallback,           &hAspInput->settings.bufferReady);
    NEXUS_IsrCallback_Set(hAspInput->hHttpResponseDataReadyCallback, &hAspInput->settings.httpResponseDataReady);

    return (NEXUS_SUCCESS);
}


static void NEXUS_AspInput_processMessage_isr(
    NEXUS_AspInputHandle               hAspInput,
    BASP_MessageType                    messageType,
    BASP_Fw2Pi_Message                  *pFw2PiMessage
    )
{

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);
    BDBG_ASSERT(pFw2PiMessage);

    BDBG_MSG((FMT_PREFIX "hAspInput=%p msgType=%d pMessage=%p"
              ARG_PREFIX, (void *)hAspInput, messageType, (void *)pFw2PiMessage));
    switch (messageType)
    {
        case BASP_MessageType_eFw2PiRstNotify:
        {
            if (hAspInput->state == NEXUS_AspInputState_eStreaming)
            {
                hAspInput->connectionReset = true;

                BDBG_MSG((FMT_PREFIX "hAspInput=%p Received RstNotify: connectionReset=true, firing hEndOfStreamingCallback!"
                          ARG_PREFIX, (void *)hAspInput));
                NEXUS_IsrCallback_Fire_isr(hAspInput->hEndOfStreamingCallback);
            }
            break;
        }
        case BASP_MessageType_eFw2PiFinNotify:  /* TODO: FIN handling has to be different. For now, just making it same as RST handling. */
        {
            if (hAspInput->state == NEXUS_AspInputState_eStreaming)
            {
                hAspInput->remoteDoneSending = true;

                BDBG_MSG((FMT_PREFIX "hAspInput=%p Received FinNotify: remoteDoneSending=true, firing hEndOfStreamingCallback!"
                          ARG_PREFIX, (void *)hAspInput));
                NEXUS_IsrCallback_Fire_isr(hAspInput->hEndOfStreamingCallback);
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
            relativeOffset = offset - hAspInput->receiveFifo.offset;
            hAspInput->pRcvdPayload = ((uint8_t *)hAspInput->receiveFifo.pBuffer) + relativeOffset;
            hAspInput->rcvdPayloadLength = pFw2PiMessage->Message.MessagePayload.PayloadNotify.HttpResponseAddress.ui32Size;

            BMMA_FlushCache_isrsafe( hAspInput->receiveFifo.hBlock, (char *)hAspInput->receiveFifo.pBuffer, hAspInput->rcvdPayloadLength );

            BDBG_MSG((FMT_PREFIX "hAspInput=%p Rcvd PayloadNotify msg: offset hi:lo=0x%x:0x%x relativeOffset=%u pRcvdPayload=%p Length=%u, firing hHttpResponseDataReadyCallback!"
                      ARG_PREFIX,
                        (void *)hAspInput, (uint32_t)(offset>>32), (uint32_t)offset, relativeOffset, (void *)hAspInput->pRcvdPayload, hAspInput->rcvdPayloadLength ));

            NEXUS_IsrCallback_Fire_isr(hAspInput->hHttpResponseDataReadyCallback);
            /* Note... In the future, maybe the PayloadNotify message will also be used by the ASP firmware to notify us
             * about "bufferReady" events in the non-auto feedModes. */
            break;
        }
        case BASP_MessageType_eFw2PiChannelAbortResp:
        {
            if (hAspInput->state == NEXUS_AspInputState_eStreaming)
            {
                BDBG_MSG((FMT_PREFIX "hAspInput=%p Processing Abort Response message from Network Peer!"
                          ARG_PREFIX, (void *)hAspInput));

                if (hAspInput->protocol == NEXUS_AspStreamingProtocol_eHttp)
                {
                    hAspInput->tcpState.finalSendSequenceNumber = pFw2PiMessage->Message.ResponsePayload.AbortResponse.ui32CurrentSeqNumber;
                    hAspInput->tcpState.finalRecvSequenceNumber = pFw2PiMessage->Message.ResponsePayload.AbortResponse.ui32CurrentAckedNumber;
                }

                hAspInput->gotStopResponse = true;
                BKNI_SetEvent(hAspInput->hMessageReceiveEvent);
            }
            break;
        }
        case BASP_MessageType_eFw2PiChannelStartStreamInResp:
            BDBG_MSG((FMT_PREFIX "hAspInput=%p Received StartStreamIn response message from ASP!"
                      ARG_PREFIX, (void *)hAspInput));
            hAspInput->gotStartResponse = true;
            BKNI_SetEvent(hAspInput->hMessageReceiveEvent);
            break;

        case BASP_MessageType_eFw2PiChannelStopResp:
            hAspInput->gotStopResponse = true;
            hAspInput->finishCompleted = true;

            BDBG_MSG((FMT_PREFIX "hAspInput=%p Received Stop response: finishCompleted=true, firing hEndOfStreamingCallback!"
                      ARG_PREFIX, (void *)hAspInput));
            NEXUS_IsrCallback_Fire_isr(hAspInput->hEndOfStreamingCallback);
            break;

        case BASP_MessageType_eFw2PiPayloadConsumedResp:
            BDBG_MSG((FMT_PREFIX "hAspInput=%p Received PayloadConsumed response message from ASP!"
                      ARG_PREFIX, (void *)hAspInput));
            hAspInput->gotPayloadConsumedResponse = true;
            BKNI_SetEvent(hAspInput->hMessageReceiveEvent);
            break;

        case BASP_MessageType_eFw2PiChannelStartStreamOutResp:
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
            BDBG_WRN((FMT_PREFIX "!!!!!! TODO: hAspInput=%p msgType=%d is NOT YET Handled by Nexus!!!"
                      ARG_PREFIX, (void *)hAspInput, messageType));
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

    g_NEXUS_asp.hAspInputTimer = NULL;

    /* Loop through each channel. */
    for (channelNum=0; channelNum<BASP_MAX_NUMBER_OF_CHANNEL; channelNum++)
    {
        if (g_NEXUS_asp.hAspInputList[channelNum] == NULL) { continue; }  /* This AspInput not active, try next one. */

        foundChannel = true;

        pContext = g_NEXUS_asp.hAspInputList[channelNum];

        BKNI_EnterCriticalSection();
        NEXUS_AspInput_ProcessMsgFromFwCallback_isr(pContext, 0);
        BKNI_LeaveCriticalSection();
    }

    if (foundChannel)
    {
        BDBG_MSG((FMT_PREFIX "Starting NEXUS TIMER for %u ms"
                  ARG_PREFIX, g_NEXUS_asp.timerIntervalInMs ));
        g_NEXUS_asp.hAspInputTimer = NEXUS_ScheduleTimer(g_NEXUS_asp.timerIntervalInMs, NEXUS_ProcessMsgFromFwCallbackByTimer, NULL);
        BDBG_ASSERT(g_NEXUS_asp.hAspInputTimer);
    }
}
#endif   /* USE_NEXUS_TIMER_INSTEAD_OF_BASP_ISRS */


static void NEXUS_AspInput_ProcessMsgFromFwCallback_isr(void *pContext, int param)
{
    NEXUS_Error            rc;
    NEXUS_AspInputHandle  hAspInput = pContext;

    BDBG_OBJECT_ASSERT(hAspInput, NEXUS_AspInput);

    BSTD_UNUSED(param);

    /* Check if there is a response message or network generated event type message from ASP firmware. */
    while (true)
    {
        BASP_Fw2Pi_Message      fw2PiMessage;
        BASP_MessageType        msgType;
        unsigned                msgLen;

        BKNI_Memset_isr(&fw2PiMessage, 0, sizeof(fw2PiMessage));
        rc = BASP_Channel_ReadMessage_isr(hAspInput->hChannel, &msgType, &fw2PiMessage, &msgLen);
        if (rc == BERR_NOT_AVAILABLE) {break;}
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR((FMT_PREFIX "BASP_Channel_ReadMessage_isr() for hAspInput=%p failed! status=%u"
                      ARG_PREFIX, (void*) hAspInput, rc ));
            BERR_TRACE(rc);
            return;
        }

        /* Make sure the message belongs to the channel that we asked for. */
        BDBG_ASSERT((unsigned)hAspInput->channelNum == fw2PiMessage.MessageHeader.ui32ChannelIndex);

        NEXUS_AspInput_processMessage_isr(hAspInput, fw2PiMessage.MessageHeader.MessageType, &fw2PiMessage);
    }

    return;
}
