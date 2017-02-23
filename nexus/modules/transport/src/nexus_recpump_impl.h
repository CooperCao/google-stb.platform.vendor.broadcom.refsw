/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#ifndef NEXUS_RECPUMP_IMPL_H__
#define NEXUS_RECPUMP_IMPL_H__

#include "nexus_transport_module.h"
#include "priv/nexus_core.h"
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
#include "nexus_dma.h"
#endif

/*
security & dma options
1) NEXUS_ENCRYPTED_DVR_WITH_M2M - nexus uses M2M DMA to decrypt
2) otherwise - nexus uses keyslots to decrypt
There is no current option for NEXUS_HAS_DMA-only or internal hooks for SW-based decrypt (ala playpump external crypto)
*/

struct NEXUS_RecpumpBufferAlloc {
    struct {
        void *buffer;
        BMMA_DeviceOffset offset;
        BMMA_Heap_Handle mmaHeap; /* if non-NULL, then the block is internally-allocated */
        BMMA_Block_Handle block;
    } data, index;
};

struct NEXUS_RecpumpFlow {
    bool pending; /* if true, then a callback has been sent and a complete is pending */
    bool irqEnabled; /* only accessed in the ISR context */
    NEXUS_RecpumpHandle recpump; /* link back to parent */
    BINT_CallbackHandle irq, overflow_irq;
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventCallback;
    NEXUS_TaskCallbackHandle dataReadyCallback;
    NEXUS_IsrCallbackHandle overflowCallback;
    size_t bufferSize;
    void *bufferBase;
    bool flushableBuffer;
    unsigned lastGetBuffer; /* remember total size (size + size2) returned by last GetBuffer. this validates the WriteComplete call. */
    unsigned dataReadyThreshold; /* copy from Settings for use in flow */
    unsigned atomSize; /* copy from Settings for use in flow */
    uint64_t bytesRecorded;
};

typedef struct NEXUS_Recpump_P_PidChannel {
    struct {
        bool enabled;
        NEXUS_RecpumpTpitFilter filter;
        unsigned index;
    } tpit;
    NEXUS_RecpumpAddPidChannelSettings settings;
    NEXUS_PidChannelHandle  pidChn;
    BLST_S_ENTRY(NEXUS_Recpump_P_PidChannel) link;
    unsigned playback;
} NEXUS_Recpump_P_PidChannel;

struct NEXUS_Recpump {
    NEXUS_OBJECT(NEXUS_Recpump);
    unsigned tindex; /* index of pTransport->recpump[] */
    NEXUS_RecpumpOpenSettings openSettings;
    NEXUS_RecpumpSettings settings;
    BLST_S_HEAD(NEXUS_Recpump_P_PidChannels, NEXUS_Recpump_P_PidChannel) pid_list;
    BXPT_RaveIdx_Handle tpitIdx;
    unsigned tpitCount;
    BXPT_RaveIdx_Handle scdIdx;
    unsigned scdPidCount;
    bool indexing; /* was indexing started? */
    bool playback;

    NEXUS_TransportType inputTransportType;

    BXPT_RaveCx_Handle rave_rec;
    unsigned rave_rec_index;
    BXPT_RaveCx_Handle extra_rave_rec;
    struct NEXUS_RecpumpBufferAlloc rave_rec_mem, extra_rave_rec_mem;
    
    struct NEXUS_RecpumpFlow data, index;
    enum {
        Ready,
        Done,   /* besttop_recpump_stop has been recognized and we're exiting */
        Terminated  /* abnormal termination */
    } rave_state;
    bool started; /* true if Start was called */
    bool actuallyStarted; /* true if actually started - which requires Start to be called and at least one pid channel */
    bool dataStopped; /* true if StopData was called. sets to false on start */

    struct NEXUS_Rave_P_ErrorCounter raveErrors;
#if NEXUS_ENCRYPTED_DVR_WITH_M2M
    bool cryptoActive; /* true if data encryption is required */
    /*
     * if crypto active there is also staging area in the FIFO - area where data is recored but not yet encrypted
     *      |---------|++++++++|==========|~~~~~~~~~~~~|-------------|
     *   FifoStart  CdbRead  CryptoTail CryptoHead CdbWrite    FifoEnd(Wrap)
     *   Legend:
     *     '-' - no data
     *     '+' - recorded and encrypted data (could be consumed by an application)
     *     '=' - recorded that currently being encrypted
     *     '~' - recorded that is not yet encrypted
     *
     *      Wrap of encrypted data
     *      |++++++++|==========|~~~~~~~~~~~~|-------------|+++++++++++++++|
     *   FifoStart CryptoTail CryptoHead CdbWrite        CdbRead      FifoEnd(Wrap)
     *
     *      Wrap of data to encrypt
     *      |========|~~~~~~~~~~~~|-------------|++++++++++++++|===========|
     *   FifoStart CryptoHead CdbWrite        CdbRead      CryptoTail   FifoEnd(Wrap)
     */
    struct {
        void *head, *tail; /* tail ... head -> distance of currently outstanding DMA transaction, it could wrap over the buffer boundary, tail==head  means no DMA outstanding */
        NEXUS_DmaJobBlockSettings blocks[2];
        NEXUS_DmaJobHandle job; /* job is only created when crypto started */
        size_t packetSize;
        BKNI_EventHandle event;
        NEXUS_EventCallbackHandle callback;
    } crypto;
#endif
#if BXPT_NUM_TSIO
    BINT_CallbackHandle tsioDmaEndIrq;
    NEXUS_IsrCallbackHandle tsioDmaEndCallback;
#endif
    unsigned bipPidChannelNum;
    uint64_t elapsedRaveTime;  /* time rave has been recording in 1.26 uSec increments */
};

#endif
