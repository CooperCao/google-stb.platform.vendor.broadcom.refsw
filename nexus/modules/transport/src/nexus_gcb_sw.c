/******************************************************************************
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
 ******************************************************************************/

#include "nexus_transport_module.h"
#include "nexus_playpump_impl.h"
#include "nexus_recpump_impl.h"

/* the number of chunks per second is constant, but the pid-filtered bitrate can vary greatly.
   need enough descriptors to keep feeding the playpump through low=bitrate sections */
#define NUM_PLAYPUMP_DESCS 2000
/* data size is driven by datarate through playpump and playback_ip buffering requirements
   e.g. to buffer 0.5 second of data, for 100Mbps stream will need 6.25MB and twice that in the recpump */
#define RECPUMP_DATA_SIZE  (3145728 * 4)
/* one entry is 48 bytes and need at least as many as number of playpump descriptors */
#define RECPUMP_INDEX_SIZE (18432 * 5)

BDBG_MODULE(nexus_gcb_sw);

#if 0
#define BDBG_MSG_TRACE_ITB(x) BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE_ITB(x)
#endif

#if 0
#define BDBG_MSG_TRACE_SUBMIT(x) BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE_SUBMIT(x)
#endif

#if 0
#define BDBG_MSG_TRACE_SUBMIT_SUMMARY(x) BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE_SUBMIT_SUMMARY(x)
#endif

#if 0
#define BDBG_MSG_TRACE_LOCK(x) BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE_LOCK(x)
#endif

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
}

static void overflow_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BDBG_ERR(("overflow %s", (const char *)context));
}

NEXUS_GcbSwHandle NEXUS_Gcb_P_Open(unsigned index, const NEXUS_ParserBandStartBondingGroupSettings *pSettings)
{
    NEXUS_Error rc;
    struct NEXUS_GcbSw *hGcb = NULL;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;

    if (pSettings->bondingPid > 0x1fff) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto error;
    }

    hGcb = BKNI_Malloc(sizeof(*hGcb));
    if (!hGcb) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    BKNI_Memset(hGcb, 0, sizeof(*hGcb));
    hGcb->index = index;
    hGcb->bondingPid = pSettings->bondingPid;
    hGcb->state.seqThreshold = -1;
    hGcb->startSettings = *pSettings;

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.numDescriptors = NUM_PLAYPUMP_DESCS;
    hGcb->playpump = NEXUS_Playpump_Open(pSettings->soft.playpumpIndex, &playpumpOpenSettings);

    if (hGcb->playpump==NULL) {
        BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }

    /* pacing */
    {
        NEXUS_PlaypumpSettings playpumpSettings;
        NEXUS_TimebaseSettings timebaseSettings;

        hGcb->pacing.timebaseFreerun = pSettings->soft.timebase;

        /* Timebase settings */
        NEXUS_Timebase_GetSettings(hGcb->pacing.timebaseFreerun, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
        rc = NEXUS_Timebase_SetSettings(hGcb->pacing.timebaseFreerun, &timebaseSettings);
        if (rc) {
            BERR_TRACE(rc); /* keep going */
        }

        NEXUS_Playpump_GetSettings(hGcb->playpump, &playpumpSettings);
        playpumpSettings.timestamp.resetPacing = true;
        playpumpSettings.timestamp.timebase = hGcb->pacing.timebaseFreerun;
        playpumpSettings.timestamp.pcrPacingPid = pSettings->soft.pcrPid;
        playpumpSettings.timestamp.pacing = true;
        playpumpSettings.timestamp.pacingMaxError = 0xffff;
        playpumpSettings.timestamp.resetPacing = true; /* for wraparound */
        rc = NEXUS_Playpump_SetSettings(hGcb->playpump, &playpumpSettings);
        if (rc) {
            BERR_TRACE(rc); /* keep going */
        }
    }

    return hGcb;

error:
    if (hGcb && hGcb->playpump) {
        NEXUS_Playpump_Close(hGcb->playpump);
    }
    if (hGcb && hGcb->pacing.timebaseFreerun) {
        NEXUS_Timebase_Close(hGcb->pacing.timebaseFreerun);
    }
    if (hGcb) {
        BKNI_Free(hGcb);
    }
    return NULL;
}

NEXUS_Error NEXUS_Gcb_P_Start(NEXUS_GcbSwHandle hGcb);
void NEXUS_Gcb_P_Stop(NEXUS_GcbSwHandle hGcb);
NEXUS_Error NEXUS_Gcb_P_RemoveParserBand(NEXUS_GcbSwHandle hGcb, NEXUS_ParserBand parserBand);

void NEXUS_Gcb_P_Close(NEXUS_GcbSwHandle hGcb)
{
    unsigned i;

    NEXUS_Gcb_P_Stop(hGcb);
    for (i=0; i<MAX_PARSERS; i++) {
        if (hGcb->parsers[i].recpump) {
            NEXUS_Gcb_P_RemoveParserBand(hGcb, hGcb->parsers[i].parserBandIndex);
        }
    }
    NEXUS_Playpump_Close(hGcb->playpump);
    if (hGcb->pacing.timebaseFreerun) {
        NEXUS_Timebase_Close(hGcb->pacing.timebaseFreerun);
    }
    BKNI_Free(hGcb);
}

static NEXUS_Error NEXUS_Gcb_P_ReplicatePidChannels(NEXUS_GcbSwHandle hGcb)
{
    NEXUS_P_HwPidChannel *pidChannel;
    unsigned pidList[MAX_PID_CHANNELS];
    unsigned i, j, numPids;
    NEXUS_Error rc;
    NEXUS_PlaypumpStatus playpumpStatus;

#if RECORD_ALLPASS
    return NEXUS_SUCCESS;
#endif

    NEXUS_Playpump_GetStatus(hGcb->playpump, &playpumpStatus);

    for (pidChannel = BLST_S_FIRST(&pTransport->pidChannels), numPids=0; pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link)) {
        if (pidChannel->status.playback && pidChannel->status.playbackIndex==playpumpStatus.index) {
            if (pidChannel->status.pid == hGcb->bondingPid) { continue; }
            pidList[numPids++] = pidChannel->status.pid;
            BDBG_MSG(("Found playpump pid %#x", pidChannel->status.pid));
        }
    }

    for (i=0; i<hGcb->numParsers; i++) {
        for (j=0; j<numPids; j++) {
            NEXUS_PidChannelHandle pid;
            pid = NEXUS_PidChannel_Open(hGcb->parsers[i].parserBandIndex, pidList[j], NULL);
            if (pid==NULL) {
                return BERR_TRACE(NEXUS_UNKNOWN);
            }
            rc = NEXUS_Recpump_AddPidChannel(hGcb->parsers[i].recpump, pid, NULL);
            if (rc) {
                return BERR_TRACE(NEXUS_UNKNOWN);
            }
            hGcb->parsers[i].pidChannelList[j] = pid;
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Gcb_P_AddParserBand(NEXUS_GcbSwHandle hGcb, NEXUS_ParserBandHandle band)
{
    unsigned parserIndex, index = -1, i;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpStatus recpumpStatus;
    NEXUS_PidChannelHandle bipPidChannel;
    NEXUS_PidChannelSettings pidChannelSettings;
    NEXUS_RecpumpPidChannelSettings addPidChannelSettings;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc;

    BDBG_ASSERT(band);
    parserIndex = band->hwIndex;

    NEXUS_ParserBand_P_GetSettings(band, &parserBandSettings);
    parserBandSettings.continuityCountEnabled = false; /* needed */
#if RECORD_ALLPASS
    parserBandSettings.acceptNullPackets = true;
    parserBandSettings.allPass = true;
#endif
    NEXUS_ParserBand_P_SetSettings(band, &parserBandSettings);

    for (i=0; i<MAX_PARSERS; i++) {
        if (hGcb->parsers[i].recpump) {
            if (hGcb->parsers[i].parserBandIndex==parserIndex) {
                BDBG_ERR(("%u: Parserband %u already added", hGcb->index, parserIndex));
                return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
    }

    /* determine an index for the parser */
    for (i=0; i<MAX_PARSERS; i++) {
        if (hGcb->parsers[i].recpump==NULL) {
            index = i;
            break;
        }
    }
    if (index >= MAX_PARSERS) {
        BDBG_ERR(("%u: Already added max supported # parserbands", hGcb->index));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

#if BAND_RECORD_DEBUG_MODE
    NEXUS_ParserBand_P_GetSettings(band, &parserBandSettings);
    parserBandSettings.forceRestamping = true;
    NEXUS_ParserBand_P_SetSettings(band, &parserBandSettings);
{
    char fname[128];
    BKNI_Snprintf(fname, sizeof(fname), "videos/band%u_tts.mpg", index);
    hGcb->parsers[index].recFile = fopen(fname, "wb");
    BDBG_ASSERT(hGcb->parsers[index].recFile);
}
#endif

    /* default buffer sizes are 3,145,728 and 18,432 */
    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    recpumpOpenSettings.data.bufferSize = RECPUMP_DATA_SIZE;
    recpumpOpenSettings.index.bufferSize = RECPUMP_INDEX_SIZE;
    recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    if (recpump==NULL) { return BERR_TRACE(NEXUS_UNKNOWN); }

    rc = NEXUS_Recpump_GetStatus(recpump, &recpumpStatus);
    if (rc) { BERR_TRACE(rc); }
    hGcb->parsers[index].bufferBase = recpumpStatus.data.bufferBase;
    hGcb->parsers[index].bufferSize = recpumpStatus.data.fifoSize;

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    /* dataready callback is required */
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = "data";
    recpumpSettings.index.dataReady.callback = dataready_callback;
    recpumpSettings.index.dataReady.context = "index";
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";
#if BAND_RECORD_DEBUG_MODE
    recpumpSettings.timestampType = NEXUS_TransportTimestampType_e32_Binary;
#endif
    rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannelSettings.continuityCountEnabled = false;
#if RECORD_ALLPASS
    NEXUS_ParserBand_GetAllPassPidChannelIndex(parserIndex, &pidChannelSettings.pidChannelIndex);
#endif
    bipPidChannel = NEXUS_PidChannel_Open(parserIndex, hGcb->bondingPid, &pidChannelSettings);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidTypeSettings.other.index = true; /* required for BIP pidchannel */
    NEXUS_Recpump_AddPidChannel(recpump, bipPidChannel, &addPidChannelSettings);

    hGcb->parsers[index].parserBandIndex = parserIndex;
    hGcb->parsers[index].recpump = recpump;
    hGcb->parsers[index].bipPidChannel = bipPidChannel;
    hGcb->numParsers++;
    hGcb->parsers[index].recpump->bipPidChannelNum = bipPidChannel->hwPidChannel->status.pidChannelIndex;
    BDBG_MSG(("[%u] parser %u, recpump %p, size %u, pidChannel %u", hGcb->index, parserIndex, (void*)recpump, (unsigned)hGcb->parsers[index].bufferSize, bipPidChannel->hwPidChannel->status.pidChannelIndex));

    return 0;
}

NEXUS_Error NEXUS_Gcb_P_RemoveParserBand(NEXUS_GcbSwHandle hGcb, NEXUS_ParserBand parserBand)
{
    NEXUS_ParserBandHandle band;
    NEXUS_PidChannelHandle pid;
    unsigned pb, index, i;

    BDBG_ASSERT(hGcb);
    band = NEXUS_ParserBand_Resolve_priv(parserBand);
    if (!band) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    pb = band->hwIndex;

    for (index=0; index<MAX_PARSERS; index++) {
        if (hGcb->parsers[index].recpump) {
            if (hGcb->parsers[index].parserBandIndex==pb) {
                break;
            }
        }
    }
    if (index>=MAX_PARSERS) {
        BDBG_ERR(("%u: Parserband %u not previously added", hGcb->index, pb));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    NEXUS_Recpump_RemoveAllPidChannels(hGcb->parsers[index].recpump);

    for (i=0; i<MAX_PID_CHANNELS; i++) {
        pid = hGcb->parsers[index].pidChannelList[i];
        if (pid) {
            NEXUS_PidChannel_Close(pid);
            hGcb->parsers[index].pidChannelList[i] = NULL;
        }
    }
    NEXUS_Recpump_Close(hGcb->parsers[index].recpump);
    NEXUS_PidChannel_Close(hGcb->parsers[index].bipPidChannel);

    hGcb->parsers[index].recpump = NULL;
    hGcb->parsers[index].parserBandIndex = 0;
    hGcb->parsers[index].bipPidChannel = NULL;
    hGcb->numParsers--;
    return NEXUS_SUCCESS;
}

/* parse ITB data and store in fifo */
static void NEXUS_Gcb_P_ProcessItb(NEXUS_GcbSwHandle hGcb)
{
    NEXUS_RecpumpHandle recpump;
    NEXUS_Error rc;
    const void *index_buffer, *index_buffer_wrap;
    size_t index_buffer_size, index_buffer_size_wrap;
    const uint32_t *pitb, *pitbw;
    unsigned i, seqNum;
    unsigned numItb;
    uintoff_t offsetCdb, offsetCdbNext;
    GcbChunk *chunk;

    for (i=0; i<hGcb->numParsers; i++) {
        recpump = hGcb->parsers[i].recpump;
        rc = NEXUS_Recpump_GetIndexBufferWithWrap(recpump, &index_buffer, &index_buffer_size, &index_buffer_wrap, &index_buffer_size_wrap);
        if (rc) { BERR_TRACE(rc); }

        pitb = index_buffer;
        numItb = 0;

        /* traverse the ITB and get the bundle locations and numbers */
        while ((index_buffer_size + index_buffer_size_wrap) >= ITB_PAIR_SIZE*2) { /* need at least two pairs per band, in order to figure out the chunk size */
            if (index_buffer_size < ITB_PAIR_SIZE) {
                BDBG_ASSERT(index_buffer_size==0); /* assume that ITBs always come in pairs and don't wrap in the middle */
                pitb = index_buffer_wrap;
                index_buffer_size = index_buffer_size_wrap;
                index_buffer_size_wrap = 0;
            }
            if (ITB_WORD0_GET_TYPE(pitb[0]) != 0x84) { BERR_TRACE(BERR_UNKNOWN); goto done; }
            seqNum = pitb[5];
            offsetCdb = pitb[3];
#if SUPPORT_40BIT_OFFSETS
            offsetCdb = GET_40BIT_OFFSET(pitb[2], pitb[3]);
#endif
            if (index_buffer_size >= ITB_PAIR_SIZE*2) {
                offsetCdbNext = pitb[(ITB_WORDS*2)+3];
#if SUPPORT_40BIT_OFFSETS
                offsetCdbNext = GET_40BIT_OFFSET(pitb[(ITB_WORDS*2)+2], pitb[(ITB_WORDS*2)+3]);
#endif
            }
            else { /* one ITB pair after wrap */
                pitbw = index_buffer_wrap;
                if (ITB_WORD0_GET_TYPE(pitbw[0]) != 0x84) { BERR_TRACE(BERR_UNKNOWN); goto done; }
                offsetCdbNext = pitbw[3];
#if SUPPORT_40BIT_OFFSETS
                offsetCdbNext = GET_40BIT_OFFSET(pitbw[2], pitbw[3]);
#endif
                pitb = pitbw;
            }

            if (BFIFO_WRITE_LEFT(&hGcb->parsers[i].chunkFifo)==0) { /* chunkFifo full */
                BDBG_MSG_TRACE_ITB(("[%u] chunkFifo full", i));
                break;
            }
            chunk = BFIFO_WRITE(&hGcb->parsers[i].chunkFifo);
            chunk->seqNum = seqNum;
            chunk->offset = offsetCdb;
            chunk->size = offsetCdbNext - offsetCdb;
            BFIFO_WRITE_COMMIT(&hGcb->parsers[i].chunkFifo, 1);

            pitb += ITB_WORDS*2;
            numItb++;
            if (index_buffer_size >= ITB_PAIR_SIZE) {
                index_buffer_size -= ITB_PAIR_SIZE;
            }
            else { break; }

#if 0 /* for debug. print all ITBs that come in and throw away */
            BDBG_MSG_TRACE_ITB(("[%u] seqNum %6u", i, seqNum));
            BFIFO_READ_COMMIT(&hGcb->parsers[i].chunkFifo, 1);
            {
                const void *data_buffer, *data_buffer_wrap;
                size_t data_buffer_size, data_buffer_size_wrap;
                rc = NEXUS_Recpump_GetDataBufferWithWrap(recpump, &data_buffer, &data_buffer_size, &data_buffer_wrap, &data_buffer_size_wrap);
                BDBG_ASSERT(!rc);
                NEXUS_Recpump_DataReadComplete(recpump, data_buffer_size+data_buffer_size_wrap);
            }
#endif
        }

        if (numItb) {
            unsigned numLeft = BFIFO_WRITE_LEFT(&hGcb->parsers[i].chunkFifo);
            chunk = BFIFO_READ(&hGcb->parsers[i].chunkFifo);
            BSTD_UNUSED(numLeft);
            BDBG_MSG_TRACE_ITB(("[%u] ITB +%u, seqNum %6u:%6u, offsetEnd %10u, fifo %2u/%2u", i, numItb, chunk->seqNum, seqNum, (unsigned)offsetCdbNext, CHUNK_FIFO_SIZE-numLeft, CHUNK_FIFO_SIZE));
            NEXUS_Recpump_IndexReadComplete(recpump, numItb*ITB_PAIR_SIZE);
        }
    }

done:
    return;
}

/* create and submit MCPB descriptor chain */
static void NEXUS_Gcb_P_Submit(NEXUS_GcbSwHandle hGcb)
{
    unsigned numChunks = MAX_DESC_LEN, numDesc;
    unsigned i, next, prevSeqNum = 0;
    NEXUS_Error rc;
    NEXUS_RecpumpHandle recpump;
    const void *data_buffer, *data_buffer_wrap;
    size_t data_buffer_size, data_buffer_size_wrap;
    uint32_t read, write, wrap, remain, chunk_start;
    GcbChunk *chunk;
    NEXUS_PlaypumpStatus playpumpStatus;
    size_t numDescConsumed;
    size_t numDescAvail, totalBytes;
    bool wait = false;

    if (hGcb->state.numDesc) {
        numDesc = hGcb->state.numDesc;
        totalBytes = hGcb->state.totalBytes;
        goto submit;
    }

    /* find how many descriptors we can submit */
    for (i=0; i<hGcb->numParsers; i++) {
        unsigned peek = BFIFO_READ_LEFT(&hGcb->parsers[i].chunkFifo);
        if (peek < numChunks) {
            numChunks = peek;
        }
    }
    numChunks *= hGcb->numParsers;

    #define MAX_CHUNKS 100 /* TODO: consider scaling this against playpumpOpenSettings.numDescriptors */
    if (numChunks > MAX_CHUNKS) {
        numChunks = MAX_CHUNKS - (MAX_CHUNKS%hGcb->numParsers);
    }

    for (i=0, numDesc=0, totalBytes=0; i<numChunks; i++) {
        next = hGcb->state.order[i%hGcb->numParsers];
        recpump = hGcb->parsers[next].recpump;
        chunk = BFIFO_READ(&hGcb->parsers[next].chunkFifo);

        if (i>0) {
            if (chunk->seqNum - prevSeqNum != 1) {
                BDBG_WRN(("[%u] Expected seqNum %u, got %u (%2u/%2u)", next, prevSeqNum+1, chunk->seqNum, i, numChunks));
                hGcb->state.state = sstate_reacq;
                #if 0 /* TODO: works from GCB perspective, but needs flush of decoder as well. otherwise decoders just don't consume any more */
                hGcb->state.state = sstate_restart;
                #endif
                /* we will submit upto and including chunk with (prevSeqNum), then find a point to resume */
                break;
            }
        }

        if (i<hGcb->numParsers) {
            rc = NEXUS_Recpump_GetDataBufferWithWrap(recpump, &(hGcb->parsers[next].data_buffer), &(hGcb->parsers[next].data_buffer_size), &(hGcb->parsers[next].data_buffer_wrap), &(hGcb->parsers[next].data_buffer_size_wrap));
            if (rc) { BERR_TRACE(rc); break; }
        }
        data_buffer = hGcb->parsers[next].data_buffer;
        data_buffer_size = hGcb->parsers[next].data_buffer_size;
        data_buffer_wrap = hGcb->parsers[next].data_buffer_wrap;
        data_buffer_size_wrap = hGcb->parsers[next].data_buffer_size_wrap;

        /* check if we're wrapping around */
        read = (uint8_t*)data_buffer - hGcb->parsers[next].bufferBase; /* the READ ptr offset, i.e. where we've consumed up to */

        if (data_buffer_wrap==0) {
            write = data_buffer_size + read; /* the WRITE ptr offset, i.e. where the buffer has filled up to */
        }
        else {
            write = data_buffer_size_wrap + ((uint8_t*)data_buffer_wrap - hGcb->parsers[next].bufferBase);
        }
        BDBG_ASSERT(chunk->offset >= hGcb->parsers[next].offsetAccum);
        chunk_start = chunk->offset - hGcb->parsers[next].offsetAccum; /* where the chunk is located inside the CDB, after accounting for CDB wrap */
        if (data_buffer_wrap==0) {
            BDBG_ASSERT(write >= chunk_start);
            remain = write - chunk_start; /* how many bytes are left to write before wrap */
            wrap = 0; /* the wrap point */
        }
        else {
            remain = data_buffer_size + read - chunk_start;
            wrap = read + data_buffer_size;
        }

        hGcb->state.desc[numDesc].addr = hGcb->parsers[next].bufferBase + chunk_start;
        if (remain >= chunk->size) {
            hGcb->state.desc[numDesc].length = chunk->size;
            if (remain==chunk->size) {
                hGcb->parsers[next].offsetAccum += wrap;
            }
        }
        else { /* wrap */
            hGcb->state.desc[numDesc].length = remain;
            numDesc++;
            BDBG_ASSERT(data_buffer_wrap==hGcb->parsers[next].bufferBase);
            hGcb->state.desc[numDesc].addr = hGcb->parsers[next].bufferBase;
            hGcb->state.desc[numDesc].length = chunk->size - remain;
            hGcb->parsers[next].offsetAccum += wrap;
            BDBG_MSG_TRACE_SUBMIT(("[%u] seqNum %6u, desc %8u - %8u (WRAP)", next, chunk->seqNum, (uint8_t*)hGcb->state.desc[numDesc-1].addr - hGcb->parsers[next].bufferBase, (uint8_t*)hGcb->state.desc[numDesc-1].addr + hGcb->state.desc[numDesc-1].length - hGcb->parsers[next].bufferBase));
        }
        BFIFO_READ_COMMIT(&hGcb->parsers[next].chunkFifo, 1);

        BDBG_MSG_TRACE_SUBMIT(("[%u] seqNum %6u, desc %8u - %8u. buff %8u %8u %8u %8u, r%2u%% w%2u%%", next, chunk->seqNum,
            (uint8_t*)hGcb->state.desc[numDesc].addr - hGcb->parsers[next].bufferBase, (uint8_t*)hGcb->state.desc[numDesc].addr + hGcb->state.desc[numDesc].length - hGcb->parsers[next].bufferBase,
            read, write, wrap, hGcb->parsers[next].bufferSize,
            read*100/hGcb->parsers[next].bufferSize,
            write*100/hGcb->parsers[next].bufferSize
            ));
        BDBG_ASSERT(chunk_start < hGcb->parsers[next].bufferSize);

        if (i >= numChunks - hGcb->numParsers) {
            uintoff_t descEnd = (uintoff_t)((uint8_t*)hGcb->state.desc[numDesc].addr + hGcb->state.desc[numDesc].length - hGcb->parsers[next].bufferBase);
            BDBG_MSG_TRACE_SUBMIT_SUMMARY(("[%u] seqNum %6u, descEnd %8u (%10u), buff %8u %8u %8u %8u, wdelta %8u", next, chunk->seqNum,
                (unsigned)descEnd,
                (unsigned)(descEnd + hGcb->parsers[next].offsetAccum),
                read, write, wrap, hGcb->parsers[next].bufferSize,
                (write >= hGcb->parsers[next].last_write_offset) ? write - hGcb->parsers[next].last_write_offset : 0));
            BSTD_UNUSED(descEnd);
            hGcb->parsers[next].last_write_offset = write;
        }

        numDesc++;
        totalBytes += chunk->size;
        prevSeqNum = chunk->seqNum;
        hGcb->parsers[next].submitSize += chunk->size;
    }

submit:
    /* submit new descriptors */
    rc = NEXUS_Playpump_GetStatus(hGcb->playpump, &playpumpStatus);
    if (rc) { BERR_TRACE(rc); }

    numDescAvail = playpumpStatus.descFifoSize - playpumpStatus.descFifoDepth;
    /* only submit if there is enough space */
    if (numDescAvail < numDesc) {
        wait = true;
        hGcb->state.numDesc = numDesc;
        hGcb->state.totalBytes = totalBytes;
    }
    if (numDescAvail < hGcb->state.minDescAvail) {
        hGcb->state.minDescAvail = numDescAvail;
    }

#if 0
    BDBG_MSG(("Playpump status: desc %3u/%3u/%3u (min %3u), bytes %7u:%7u %s", numDesc, (unsigned)numDescAvail, (unsigned)playpumpStatus.descFifoSize, hGcb->state.minDescAvail,
        (unsigned)totalBytes, (unsigned)playpumpStatus.fifoDepth,
        wait?"(WAIT)":""));
#endif
    if (wait) { goto done; }

    rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(hGcb->playpump, hGcb->state.desc, numDesc, &numDescConsumed);
    if (rc) { BERR_TRACE(rc); goto done; }

    for (i=0; i<hGcb->numParsers; i++) {
        size_t avail, submit;
        recpump = hGcb->parsers[i].recpump;
        submit = hGcb->parsers[i].submitSize;

        rc = NEXUS_Recpump_GetDataBufferWithWrap(recpump, &data_buffer, &data_buffer_size, &data_buffer_wrap, &data_buffer_size_wrap);
        if (rc) { BERR_TRACE(rc); }

        avail = data_buffer_size + data_buffer_size_wrap;
        if (avail <= hGcb->parsers[i].submitSize) {
            BDBG_WRN(("[%u] recpump depth (%u) less than advance amount %u", i, (uint32_t)avail, (uint32_t)hGcb->parsers[i].submitSize));
            submit = avail;
        }

        rc = NEXUS_Recpump_DataReadComplete(recpump, submit);
        if (rc) { BERR_TRACE(rc); }

        BDBG_MSG_TRACE_SUBMIT(("[%u] recpump read complete %u:%u:%u", i, submit, hGcb->parsers[i].submitSize, avail));
        hGcb->parsers[i].submitSize -= submit;
    }

    hGcb->state.numDesc = 0;
    hGcb->state.totalBytes = 0;

done:
    return;
}

/* assume unbonded data on one RAVE context. send it to playback right away */
#if UNBONDED_DEBUG_MODE
static void NEXUS_Gcb_P_SubmitDebug(NEXUS_GcbSwHandle hGcb)
{
    unsigned numDesc, numDescConsumed;
    NEXUS_Error rc;
    NEXUS_RecpumpHandle recpump;
    const void *data_buffer, *data_buffer_wrap;
    const void *index_buffer;
    size_t data_buffer_size, data_buffer_size_wrap;
    size_t index_buffer_size;
    NEXUS_PlaypumpScatterGatherDescriptor *desc;
#if BAND_RECORD_DEBUG_MODE
    int n;
#endif

    recpump = hGcb->parsers[0].recpump;
    rc = NEXUS_Recpump_GetDataBufferWithWrap(recpump, &data_buffer, &data_buffer_size, &data_buffer_wrap, &data_buffer_size_wrap);
    if (rc) { BERR_TRACE(rc); }

    desc = &hGcb->state.desc[0];
    if (data_buffer_size_wrap==0) {
        desc->addr = (void*)data_buffer;
        desc->length = data_buffer_size;
        numDesc = 1;
    }
    else {
        desc->addr = (void*)data_buffer;
        desc->length = data_buffer_size;
        desc++;
        desc->addr = (void*)data_buffer_wrap;
        desc->length = data_buffer_size_wrap;
        numDesc = 2;
    }

    rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(hGcb->playpump, hGcb->state.desc, numDesc, &numDescConsumed);
    if (rc) { BERR_TRACE(rc); goto done; }

    BDBG_MSG(("[%u] %u", 0, data_buffer_size+data_buffer_size_wrap));

#if BAND_RECORD_DEBUG_MODE
    if (data_buffer_size) {
        n = fwrite(data_buffer, 1, data_buffer_size, hGcb->parsers[0].recFile);
        if (n < 0) {BDBG_ERR(("fwrite error\n")); }
    }
    if (data_buffer_size_wrap) {
        n = fwrite(data_buffer_wrap, 1, data_buffer_size_wrap, hGcb->parsers[0].recFile);
        if (n < 0) {BDBG_ERR(("fwrite error\n")); }
    }
#endif

    rc = NEXUS_Recpump_DataReadComplete(recpump, data_buffer_size + data_buffer_size_wrap);
    if (rc) { BERR_TRACE(rc); }

    rc = NEXUS_Recpump_GetIndexBuffer(recpump, &index_buffer, &index_buffer_size);
    if (rc) { BERR_TRACE(rc); }
    rc = NEXUS_Recpump_IndexReadComplete(recpump, index_buffer_size);
    if (rc) { BERR_TRACE(rc); }

done:
    return;
}
#endif

#if BAND_RECORD_DEBUG_MODE
static void NEXUS_Gcb_P_SubmitDebug2(NEXUS_GcbSwHandle hGcb)
{
    unsigned numDesc, numDescConsumed;
    NEXUS_Error rc;
    NEXUS_RecpumpHandle recpump;
    const void *data_buffer, *data_buffer_wrap;
    size_t data_buffer_size, data_buffer_size_wrap;
    const void *index_buffer;
    size_t index_buffer_size;
    NEXUS_PlaypumpScatterGatherDescriptor *desc;
    unsigned i;
    int n;

    for (i=0; i<hGcb->numParsers; i++) {
        recpump = hGcb->parsers[i].recpump;
        rc = NEXUS_Recpump_GetDataBufferWithWrap(recpump, &data_buffer, &data_buffer_size, &data_buffer_wrap, &data_buffer_size_wrap);
        if (rc) { BERR_TRACE(rc); }

        if (data_buffer_size) {
            n = fwrite(data_buffer, 1, data_buffer_size, hGcb->parsers[i].recFile);
            if (n < 0) {BDBG_ERR(("[%u] fwrite error\n", i)); }
        }
        if (data_buffer_size_wrap) {
            n = fwrite(data_buffer_wrap, 1, data_buffer_size_wrap, hGcb->parsers[i].recFile);
            if (n < 0) {BDBG_ERR(("[%u] fwrite error\n", i)); }
        }

        BDBG_MSG(("[%u] %u %u", i, data_buffer_size, data_buffer_size_wrap));
        rc = NEXUS_Recpump_DataReadComplete(recpump, data_buffer_size + data_buffer_size_wrap);
        if (rc) { BERR_TRACE(rc); }

        NEXUS_Recpump_GetIndexBuffer(recpump, &index_buffer, &index_buffer_size);
        NEXUS_Recpump_IndexReadComplete(recpump, index_buffer_size);
    }
    return;
}
#endif

static void NEXUS_Gcb_P_Timer(void *arg)
{
    NEXUS_GcbSwHandle hGcb = arg;
    NEXUS_RecpumpHandle recpump;
    const void *data_buffer, *data_buffer_wrap;
    size_t data_buffer_size, data_buffer_size_wrap;

    NEXUS_Error rc;
    unsigned i;
    unsigned min = 0xFFFFFFFF, max = 0;

#if UNBONDED_DEBUG_MODE
    hGcb->state.state = sstate_lockMode;
    NEXUS_Gcb_P_SubmitDebug(hGcb);
    goto done;
#endif

#if BAND_RECORD_DEBUG_MODE
    hGcb->state.state = sstate_lockMode;
    NEXUS_Gcb_P_SubmitDebug2(hGcb);
    goto done;
#endif

    NEXUS_Gcb_P_ProcessItb(hGcb);

    if (hGcb->state.state==sstate_findInit) {
        GcbChunk *chunk;
        /* look for first seqNum and calculate trim to beginning of that chunk */
        for (i=0; i<hGcb->numParsers; i++) {
            if (BFIFO_READ_LEFT(&hGcb->parsers[i].chunkFifo)) {
                rc = NEXUS_Recpump_GetDataBuffer(hGcb->parsers[i].recpump, &data_buffer, &data_buffer_size);
                if (rc) { BERR_TRACE(rc); }

                chunk = BFIFO_READ(&hGcb->parsers[i].chunkFifo);
                hGcb->parsers[i].seqNum = chunk->seqNum;
                hGcb->parsers[i].submitSize = chunk->offset - hGcb->parsers[i].offsetAccum - (unsigned)((uint8_t*)data_buffer - hGcb->parsers[i].bufferBase);
                BDBG_MSG(("[%u] seqNum %u, CDB offset " BDBG_UINT64_FMT ", CDB advance %u", i, hGcb->parsers[i].seqNum, BDBG_UINT64_ARG(chunk->offset), hGcb->parsers[i].submitSize));
            }
        }

        for (i=0; i<hGcb->numParsers; i++) {
            if (hGcb->parsers[i].seqNum==-1) {
                BDBG_MSG(("Wait for seqNum on all bands"));
                hGcb->state.seqThreshold = -1;
                goto done;
            }
            if (hGcb->parsers[i].seqNum > hGcb->state.seqThreshold) {
                hGcb->state.seqThreshold = hGcb->parsers[i].seqNum;
                hGcb->state.seqThresholdParser = i;
            }
            if ((unsigned)hGcb->parsers[i].seqNum < min) {
                min = hGcb->parsers[i].seqNum;
            }
            if ((unsigned)hGcb->parsers[i].seqNum > max) {
                max = hGcb->parsers[i].seqNum;
            }
        }
        #define MAX_SEQNUM_SKEW 200
        if ((max<=min) || (max-min>MAX_SEQNUM_SKEW)) {
            BDBG_MSG(("Large seqNum skew detected (%u), attempting to rebase..", max-min));
            for (i=0; i<hGcb->numParsers; i++) {
                if (BFIFO_READ_LEFT(&hGcb->parsers[i].chunkFifo)==0) {
                    goto done;
                }
            }
            for (i=0; i<hGcb->numParsers; i++) {
                BFIFO_READ_COMMIT(&hGcb->parsers[i].chunkFifo, 1);
            }
            goto done;
        }

        BDBG_MSG(("Found initThreshold %u on band %u", hGcb->state.seqThreshold, hGcb->state.seqThresholdParser));
        hGcb->state.state = sstate_crossInit;
    }

    if (hGcb->state.state==sstate_reacq) {
        GcbChunk *chunk;
        unsigned priband;
        BDBG_MSG(("Reacquire"));

        min = 0xFFFFFFFF;
        max = 0;
        hGcb->state.seqThreshold = -1;
        hGcb->state.reacquire = true;

        for (i=0; i<hGcb->numParsers; i++) {
            if (BFIFO_READ_LEFT(&hGcb->parsers[i].chunkFifo)) {
                chunk = BFIFO_READ(&hGcb->parsers[i].chunkFifo);
                hGcb->parsers[i].seqNum = chunk->seqNum;
                BDBG_MSG(("[%u] seqNum %u", i, hGcb->parsers[i].seqNum));
                if (chunk->seqNum < min) {
                    min = chunk->seqNum;
                }
                if (chunk->seqNum > max) {
                    max = chunk->seqNum;
                }
            }
            else {
                hGcb->parsers[i].seqNum = -1;
                goto done;
            }
        }

        priband = hGcb->state.primaryParser;
        BDBG_ASSERT(hGcb->state.seqThresholdParser==priband);

        if ((max-min < hGcb->numParsers) && (min==(unsigned)(hGcb->parsers[hGcb->state.primaryParser].seqNum))) {
            /* this is a special case that allows us to wraparound cleanly */
            hGcb->state.seqThreshold = hGcb->parsers[priband].seqNum;
            BDBG_MSG(("Priband (%u) seqThreshold %u", priband, hGcb->state.seqThreshold));
            hGcb->state.state = sstate_crossPriBand;
        }
        else {
            hGcb->state.state = sstate_findInit;
        }
    }
    else if (hGcb->state.state==sstate_restart) { /* experimental code: start over from scratch */
        BDBG_MSG(("Restart"));
        NEXUS_Gcb_P_Stop(hGcb);
        NEXUS_Gcb_P_Start(hGcb);
        hGcb->state.state = sstate_findInit;
    }

cross_priband:
    if (hGcb->state.state==sstate_crossInit || hGcb->state.state==sstate_crossPriBand) {
        int seqNum;
        GcbChunk *chunk;

        for (i=0; i<hGcb->numParsers; i++) {
            if (hGcb->parsers[i].seqNum >= hGcb->state.seqThreshold) { continue; }

            seqNum = -1;
            while (BFIFO_READ_LEFT(&hGcb->parsers[i].chunkFifo)) {
                chunk = BFIFO_READ(&hGcb->parsers[i].chunkFifo);
                seqNum = chunk->seqNum;
                BDBG_MSG(("[%u] seqNum %u, offset " BDBG_UINT64_FMT ", size %u", i, seqNum, BDBG_UINT64_ARG(chunk->offset), chunk->size));
                if (seqNum > hGcb->state.seqThreshold) { break; }
                BFIFO_READ_COMMIT(&hGcb->parsers[i].chunkFifo, 1);
                hGcb->parsers[i].submitSize += chunk->size;
            }
            if (seqNum >= hGcb->state.seqThreshold) {
                BDBG_MSG(("[%u] cross seqThreshold (%u >= %u)", i, seqNum, hGcb->state.seqThreshold));
                hGcb->parsers[i].seqNum = seqNum;
            }
        }

        for (i=0; i<hGcb->numParsers; i++) {
            if (hGcb->parsers[i].seqNum < hGcb->state.seqThreshold) { goto done; }
        }

        if (hGcb->state.state==sstate_crossInit) {
            unsigned priband = 0;
            #if 0
            priband = hGcb->state.seqThresholdParser == (hGcb->numParsers-1) ? 0 : hGcb->state.seqThresholdParser+1; /* force a different priband for now for testing */
            #else
            priband = hGcb->state.primaryParser;
            #endif
            BDBG_MSG(("All parsers crossed initThreshold %u", hGcb->state.seqThreshold));

            if (priband != hGcb->state.seqThresholdParser) {
                BDBG_MSG(("Set new priband (%u) seqThreshold %u->%u", priband, hGcb->state.seqThreshold, hGcb->parsers[priband].seqNum));
                hGcb->state.seqThreshold = hGcb->parsers[priband].seqNum;
                hGcb->state.seqThresholdParser = priband;
                hGcb->state.state = sstate_crossPriBand;
                goto cross_priband;
            }
            else {
                BDBG_MSG(("Priband (%u) is already seqThreshold band", priband));
                hGcb->state.state = sstate_lockMode;
            }
        }
        else {
            BDBG_MSG(("All parsers crossed priBandThreshold %u", hGcb->state.seqThreshold));

            /* now advance the CDB to the starting point */
            for (i=0; i<hGcb->numParsers; i++) {
                chunk = BFIFO_READ(&hGcb->parsers[i].chunkFifo);
                seqNum = chunk->seqNum;
                BDBG_MSG(("[%u] seqNum ->%u, CDB offset " BDBG_UINT64_FMT ", CDB advance %u", i, seqNum, BDBG_UINT64_ARG(chunk->offset), hGcb->parsers[i].submitSize));

                recpump = hGcb->parsers[i].recpump;
                rc = NEXUS_Recpump_GetDataBufferWithWrap(recpump, &data_buffer, &data_buffer_size, &data_buffer_wrap, &data_buffer_size_wrap);
                if (rc) { BERR_TRACE(rc); }
                rc = NEXUS_Recpump_DataReadComplete(recpump, hGcb->parsers[i].submitSize);
                if (rc) { BERR_TRACE(rc); }
                hGcb->parsers[i].submitSize = 0;
            }

            hGcb->state.state = sstate_lockMode;
        }

        if (hGcb->state.reacquire) {
            BDBG_WRN(("Resume at seqNum %u", hGcb->state.seqThreshold));
            hGcb->state.reacquire = false;
        }

        /* set execution order */
        for (i=0; i<hGcb->numParsers; i++) {
            unsigned diff;
            BDBG_ASSERT(hGcb->parsers[i].seqNum >= hGcb->state.seqThreshold);
            diff = hGcb->parsers[i].seqNum - hGcb->state.seqThreshold;
            hGcb->state.order[diff] = i;
        }
        BDBG_MSG(("Exec order:"));
        for (i=0; i<hGcb->numParsers; i++) {
            unsigned band = hGcb->state.order[i];
            BDBG_MSG(("[%u] seqNum %u %s", band, hGcb->parsers[band].seqNum, band==hGcb->state.seqThresholdParser?"(PRIBAND)":""));
        }
    }

    if (hGcb->state.state==sstate_lockMode) {
        BDBG_MSG_TRACE_LOCK(("Locked"));
        NEXUS_Gcb_P_Submit(hGcb);
        goto done;
    }

done:
    hGcb->timer = NEXUS_ScheduleTimer(30, NEXUS_Gcb_P_Timer, hGcb);
}

void NEXUS_Gcb_P_CheckStart(void* param)
{
    NEXUS_GcbSwHandle hGcb = param;
    /* if NEXUS_ParserBand_StartBondingGroup called and no pidchannels opened */
    if (BLST_S_FIRST(&hGcb->playpump->pid_list)==NULL) {
        hGcb->checkTimer = NEXUS_ScheduleTimer(250, NEXUS_Gcb_P_CheckStart, hGcb);
        return;
    }

    NEXUS_Gcb_P_Start(hGcb);
}

NEXUS_Error NEXUS_Gcb_P_Start(NEXUS_GcbSwHandle hGcb)
{
    unsigned i;
    NEXUS_RecpumpHandle recpump;
    NEXUS_Error rc;

    if (BLST_S_FIRST(&hGcb->playpump->pid_list)==NULL) {
        /* should never occur, since NEXUS_ParserBand_StartBondingGroup is called first,
           then user requests pidchannels, then we really start */
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    hGcb->state.primaryParser = hGcb->parsers[0].parserBandIndex; /* master always in slot [0] */
    hGcb->state.numDesc = 0;
    hGcb->state.totalBytes = 0;
    hGcb->state.seqThreshold = -1;
    hGcb->state.seqThresholdParser = 0;
    hGcb->state.minDescAvail = 0xFFFFFFFF;

    /* user opens play pid channels via playpump
       these play pidchannels now need to be replicated on all gcb parsers */
    rc = NEXUS_Gcb_P_ReplicatePidChannels(hGcb);
    if (rc) {
        return BERR_TRACE(rc);
    }

    for (i=0; i<MAX_PARSERS; i++) {
        recpump = hGcb->parsers[i].recpump;
        if (recpump) {
            hGcb->parsers[i].seqNum = -1;
            hGcb->parsers[i].offsetAccum = 0;
            hGcb->parsers[i].submitSize = 0;
            BFIFO_INIT(&hGcb->parsers[i].chunkFifo, hGcb->parsers[i].chunks, CHUNK_FIFO_SIZE);
            NEXUS_Recpump_Start(recpump);
        }
    }

    NEXUS_Playpump_Start(hGcb->playpump);
    hGcb->timer = NEXUS_ScheduleTimer(100, NEXUS_Gcb_P_Timer, hGcb);
    return 0;
}

void NEXUS_Gcb_P_Stop(NEXUS_GcbSwHandle hGcb)
{
    unsigned i;
    if (hGcb->timer) {
        NEXUS_CancelTimer(hGcb->timer);
        hGcb->timer = NULL;
    }
    if (hGcb->checkTimer) {
        NEXUS_CancelTimer(hGcb->checkTimer);
        hGcb->checkTimer = NULL;
    }

    NEXUS_Playpump_Stop(hGcb->playpump);
    for (i=0; i<MAX_PARSERS; i++) {
        if (hGcb->parsers[i].recpump) {
            NEXUS_Recpump_Stop(hGcb->parsers[i].recpump);
        }
    }
    return;
}

NEXUS_Error NEXUS_Gcb_P_GetStatus(NEXUS_GcbSwHandle hGcb, NEXUS_ParserBandBondingGroupStatus *pStatus)
{
    BDBG_ASSERT(pStatus);
    pStatus->locked = (hGcb->state.state==sstate_lockMode);
    /* TODO: consider exposing last seen seqNum (from ProcessItb()) and last submitted seqNum (from Submit()) as public status */
    return NEXUS_SUCCESS;
}
