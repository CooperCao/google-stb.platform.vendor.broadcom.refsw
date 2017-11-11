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
 *
 * Module Description:
 *  PVR record module
 *
 **************************************************************************/

#include "nexus_record_module.h"
#include "blst_slist.h"
#include "bfile_io.h"
#include "bcmindexer.h"
#include "nexus_file_pvr.h"
#if NEXUS_HAS_PLAYBACK
#include "priv/nexus_playback_notify.h"
#endif
#include "nexus_core_utils.h"

BDBG_MODULE(nexus_record);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

typedef struct NEXUS_Record_P_PidChannel {
    BLST_S_ENTRY(NEXUS_Record_P_PidChannel) link;
    NEXUS_PidChannelHandle  pidChn;
    NEXUS_RecordPidChannelSettings cfg;
} NEXUS_Record_P_PidChannel;

typedef struct NEXUS_Record_P_Playback {
    BLST_S_ENTRY(NEXUS_Record_P_Playback) link;
    NEXUS_PlaybackHandle playback;
} NEXUS_Record_P_Playback;

BDBG_OBJECT_ID(NEXUS_Record);
BDBG_OBJECT_ID(NEXUS_Record_P_Flow);

typedef struct NEXUS_Record_P_Flow {
    BDBG_OBJECT(NEXUS_Record_P_Flow)
    NEXUS_FileWriteHandle file;
    const void *buf;
    enum {
        Waiting,
        Writing, /* waiting for async File callback */
        Done,   /* brecord_stop has been recognized and we're exiting */
        Terminated, /* abnormal termination */
        Idle /* used for index write to recycle CPU cycles */
    } state;
    bool stopping;  /* inside NEXUS_Record_Stop, we call NEXUS_Recpump_StopData and process callbacks for remaining data. */
    bool partialWrite; /* set to true if last write was partial, this is used to lower reported priority */
    size_t size;
    union {
        struct {
            NEXUS_TimerHandle timer; /* timer used in the indexer write */
            BNAV_Indexer_Handle indexer; /* this field only used for scd record and only if we were told to use bcmindexer */
        } index;
        struct {
            size_t blockSize; /* size of the data */
            size_t blockOffset; /* current offset */
        } data;
    } info;
    NEXUS_RecordHandle record; /* link back to parent */
    NEXUS_CallbackHandler dataReady;
    NEXUS_CallbackHandler overflow;
    bool emptyBuffer;
    NEXUS_Time callbackTime;
} NEXUS_Record_P_Flow;


struct NEXUS_Record {
    BDBG_OBJECT(NEXUS_Record)
    BLST_S_HEAD(NEXUS_Record_P_PidChannels, NEXUS_Record_P_PidChannel) pid_list;
    BLST_S_HEAD(NEXUS_Record_P_Playbacks, NEXUS_Record_P_Playback) playback_list;
    NEXUS_RecordSettings cfg;
    bool started;
    NEXUS_Record_P_Flow data, index;
    NEXUS_RecordErrorType errorType;
    size_t entry_size;
    unsigned lastTimestamp;
    NEXUS_TaskCallbackHandle errorCallback, overflowCallback;
    NEXUS_TimerHandle processDataTimer;
    unsigned picturesIndexed;
    NEXUS_RecpumpStatus status;
    struct {
        uint64_t offset;
        unsigned timestamp;
    } priming;
};

static void NEXUS_Record_P_DataReadyCallback(void *context);
static void NEXUS_Record_P_IndexReadyCallback(void *context);
static void NEXUS_Record_P_Overflow(void *context);
static void NEXUS_Record_P_ProcessDataTimer(void *context);

static void
NEXUS_Record_P_InitFlow(NEXUS_RecordHandle record, NEXUS_Record_P_Flow *flow)
{
    flow->state = Idle;
    flow->stopping = false;
    flow->partialWrite = false;
    flow->info.index.timer = NULL;
    flow->info.index.indexer = NULL;
    flow->record = record;
    NEXUS_Time_Get(&flow->callbackTime);
    return;
}

void
NEXUS_Record_GetDefaultSettings(NEXUS_RecordSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->indexFormat = NEXUS_RecordIndexType_eBcm;
    NEXUS_CallbackDesc_Init(&pSettings->overflowCallback);
    NEXUS_CallbackDesc_Init(&pSettings->errorCallback);
    NEXUS_Recpump_GetDefaultSettings(&pSettings->recpumpSettings);
    pSettings->pollingTimer = 250; /* milliseconds */
    return;
}


NEXUS_RecordHandle
NEXUS_Record_Create(void)
{
    NEXUS_RecordHandle record;

    record = BKNI_Malloc(sizeof(*record));
    if(!record) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BDBG_OBJECT_INIT(record, NEXUS_Record);
#if NEXUS_HAS_PLAYBACK
    BLST_S_INIT(&record->playback_list);
#endif
    BLST_S_INIT(&record->pid_list);
    record->started = false;
    BDBG_OBJECT_INIT(&record->data, NEXUS_Record_P_Flow);
    BDBG_OBJECT_INIT(&record->index, NEXUS_Record_P_Flow);
    record->data.record = record;
    record->index.record = record;
    record->errorType = NEXUS_RecordErrorType_eNone;
    record->entry_size = 0;
    record->lastTimestamp = 0;
    record->errorCallback = NEXUS_TaskCallback_Create(record, NULL);
    record->overflowCallback = NEXUS_TaskCallback_Create(record, NULL);
    record->processDataTimer = NULL;
    NEXUS_CallbackHandler_Init(record->data.dataReady, NEXUS_Record_P_DataReadyCallback, &record->data);
    NEXUS_CallbackHandler_Init(record->index.dataReady, NEXUS_Record_P_IndexReadyCallback, &record->index);
    NEXUS_CallbackHandler_Init(record->data.overflow, NEXUS_Record_P_Overflow, record);
    NEXUS_CallbackHandler_Init(record->index.overflow, NEXUS_Record_P_Overflow, record);
    NEXUS_Record_GetDefaultSettings(&record->cfg);
    record->priming.offset = 0;
    return record;
err_alloc:
    return NULL;
}

void
NEXUS_Record_Destroy(NEXUS_RecordHandle record)
{
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    NEXUS_Record_Stop(record);
    NEXUS_Record_RemoveAllPidChannels(record);
    NEXUS_TaskCallback_Destroy(record->errorCallback);
    NEXUS_TaskCallback_Destroy(record->overflowCallback);
    NEXUS_CallbackHandler_Shutdown(record->data.dataReady);
    NEXUS_CallbackHandler_Shutdown(record->index.dataReady);
    NEXUS_CallbackHandler_Shutdown(record->data.overflow);
    NEXUS_CallbackHandler_Shutdown(record->index.overflow);
    BDBG_OBJECT_DESTROY(record, NEXUS_Record);
    BKNI_Free(record);
    return;
}

/* B_RECORD_ATOM_SIZE allows Record to divide up its writes into small enough pieces so that
it doesn't create a latency problem.
It must be a multiple of BIO_BLOCK_SIZE. It does not need to be a multiple of packet size (e.g. 188 bytes). */
#define B_RECORD_ATOM_SIZE  (47*BIO_BLOCK_SIZE)
/* We limit record size to prevent overflow. If a file transaction takes a long time, we
may overflow while we wait. Better to split it up into slightly smaller transactions so that we free
up space sooner. */
#define B_RECORD_WRITE_SIZE_LIMIT (3*B_RECORD_ATOM_SIZE)

/* XXX OS dependent, only used to get last disk/file error code */
#include <errno.h>

static void
NEXUS_Record_P_WriteDone(void *flow_, ssize_t size)
{
    NEXUS_Record_P_Flow *flow = flow_;
    NEXUS_RecordHandle record;
    NEXUS_Error rc;
    NEXUS_Record_P_Playback *play_item;

    BDBG_OBJECT_ASSERT(flow, NEXUS_Record_P_Flow);
    if (flow->state != Writing) {
        BDBG_ERR(("Received bad callback from File module"));
        return;
    }

    record = flow->record;
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    BDBG_ASSERT(record->cfg.recpump);
    BDBG_MSG_TRACE(("(%#x) completed writing %d(%u) bytes of data from %#x", (unsigned)flow->record, (int)size, flow->info.data.blockSize-flow->info.data.blockOffset, (unsigned)flow->buf+flow->info.data.blockOffset));

    if (size<0) {
        BDBG_ERR(("Can't write data to the file, terminating record: %d", errno));
        switch(errno) {
        case ENOSPC: record->errorType = NEXUS_RecordErrorType_eDiskFull;break;
        case EFBIG:  record->errorType = NEXUS_RecordErrorType_eMaxFileSize;break;
        default:  record->errorType = NEXUS_RecordErrorType_eUnknown;break;
        }
        flow->state = Terminated;
        /* Give recpump signal to stop feeding you data more data otherwise you will
           an overflow real quick. You loose data so its pretty much
           same as overflow, but the system will crash and you can recover from this*/
        NEXUS_Recpump_Stop(record->cfg.recpump);
        NEXUS_TaskCallback_Fire(record->errorCallback);
        return;
    }

#if NEXUS_HAS_PLAYBACK
    play_item=BLST_S_FIRST(&record->playback_list);
    if (play_item) {
        if (NEXUS_Module_TryLock(g_NEXUS_Record_P_ModuleState.cfg.modules.playback)) {
            for(play_item=BLST_S_FIRST(&record->playback_list);play_item;play_item=BLST_S_NEXT(play_item,link)) {
                NEXUS_Playback_RecordProgress_priv(play_item->playback);
            }
            NEXUS_Module_Unlock(g_NEXUS_Record_P_ModuleState.cfg.modules.playback);
        }
    }
#else
    BSTD_UNUSED(play_item);
#endif

    if (!flow->file) {
        BDBG_WRN(("Detected stop condition, record done"));
        flow->state = Done;
        return;
    }
    if(flow->info.data.blockOffset+size > flow->info.data.blockSize) {
        BDBG_ERR(("Wrote more data then expected: %u %d", (unsigned)(flow->info.data.blockSize - flow->info.data.blockOffset), (int)size));
    }
    flow->info.data.blockOffset += size;
    if(flow->info.data.blockOffset >= flow->info.data.blockSize) {
        flow->state = Waiting;
        rc = NEXUS_Recpump_DataReadComplete(record->cfg.recpump, flow->info.data.blockSize); /* completed write */
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
        /* if band hold enabled, call again in case write size was limited, to reduce buffer level away from bandhold threshold.
         * NOTE: we could keep consuming but we also like to reduce data ready callbacks to be efficient. */
        if(record->cfg.recpumpSettings.bandHold && (size == B_RECORD_WRITE_SIZE_LIMIT)) {
            NEXUS_Record_P_DataReadyCallback(flow);
        }
        return;
    } else {
        const uint8_t *buf = (const uint8_t *)flow->buf + flow->info.data.blockOffset;
        size = flow->info.data.blockSize - flow->info.data.blockOffset;
        flow->partialWrite = true;
        BDBG_MSG_TRACE(("(%#x) continue writing %u bytes of data from %#x", (unsigned)flow->record, (unsigned)size, (unsigned)buf));
        NEXUS_File_AsyncWrite(flow->file, buf, size, NEXUS_RecordModule, NEXUS_Record_P_WriteDone, flow);
        return;
    }
}

static void
NEXUS_Record_P_DataReadyCallback(void *context)
{
    NEXUS_Record_P_Flow *flow = context;
    const void *buffer;
    size_t size;
    NEXUS_Error rc;
    NEXUS_RecordHandle record;

    BDBG_OBJECT_ASSERT(flow, NEXUS_Record_P_Flow);
    if (flow->state == Writing) {
        return;
    }

    BDBG_OBJECT_ASSERT(flow->record, NEXUS_Record);
    record = flow->record;
    BDBG_ASSERT(flow->record->cfg.recpump);

    if(!flow->record->started) { goto done; }

    NEXUS_Time_Get(&flow->callbackTime);

    if (record->cfg.priming.data.fifoDepth) {
        const void *buffer2;
        size_t size2;
        rc = NEXUS_Recpump_GetDataBufferWithWrap(flow->record->cfg.recpump, &buffer, &size, &buffer2, &size2);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto done;}
        BSTD_UNUSED(buffer2);
        if (size + size2 > record->cfg.priming.data.fifoDepth) {
            size = size + size2 - record->cfg.priming.data.fifoDepth;
            /* TODO: support 130, 134, and 192 packet sizes */
            size -= size % B_RECORD_ATOM_SIZE;
            rc = NEXUS_Recpump_DataReadComplete(record->cfg.recpump, size);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}

            /* remember amount of data discarded */
            record->priming.offset += size;
        }
        goto done;
    }

    rc = NEXUS_Recpump_GetDataBuffer(flow->record->cfg.recpump, &buffer, &size);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto done;}

    flow->emptyBuffer = (size == 0);

    if ( size == 0 )
    {
        /* Possible with a stale callback */
        goto done;
    }

    if (!flow->file) {
        /* we're not writing this data to file. just complete all of it and wait for more. */
        rc = NEXUS_Recpump_DataReadComplete(flow->record->cfg.recpump, size);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
        flow->state = Waiting;
        return;
    }

#if DIRECT_IO_SUPPORT
    if (!flow->stopping) {
        unsigned truncAmount;

        /* If Recpump's data.atomSize is working, we will never get less that BIO_BLOCK_SIZE, even on wrap around. */
        BDBG_ASSERT(size >= BIO_BLOCK_SIZE);

        /* Direct I/O requires multiples of BIO_BLOCK_SIZE */
        truncAmount = size % BIO_BLOCK_SIZE;
        size -= truncAmount;
    }
#endif

    /* We limit record size to prevent overflow. If a file transaction takes a long time, we
    may overflow while we wait. Better to split it up into slightly smaller transactions so that we free
    up space sooner. */
    if (size>B_RECORD_WRITE_SIZE_LIMIT) {
        BDBG_MSG(("Limiting write size from %d to %d", (int)size, (int)B_RECORD_WRITE_SIZE_LIMIT));
        size = B_RECORD_WRITE_SIZE_LIMIT;
    }

    flow->buf = buffer;
    flow->partialWrite = false;
    flow->info.data.blockOffset = 0;
    flow->info.data.blockSize = size;
    BDBG_MSG_TRACE(("(%#x) writing %u bytes of data from %#x", (unsigned)flow->record, size, (unsigned)flow->buf));
    NEXUS_File_AsyncWrite(flow->file, flow->buf, size, NEXUS_RecordModule, NEXUS_Record_P_WriteDone, flow);
    flow->state = Writing;

done:
    return;
}

static void
NEXUS_Record_P_IdleTimer(void *context)
{
    NEXUS_Record_P_Flow *flow = context;
    NEXUS_RecordHandle record;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(flow, NEXUS_Record_P_Flow);

    BDBG_OBJECT_ASSERT(flow->record, NEXUS_Record);
    record = flow->record;
    BDBG_ASSERT(record->cfg.recpump);

    flow->info.index.timer = NULL;
    flow->state = Waiting;
    rc = NEXUS_Recpump_IndexReadComplete(record->cfg.recpump, flow->size);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
    return;
}

static void
NEXUS_Record_P_IndexReadyCallback(void *context)
{
    NEXUS_Record_P_Flow *flow = context;
    const void *buffer;
    size_t size;
    NEXUS_Error rc;
    NEXUS_RecordHandle record;
    unsigned sct_size;

    BDBG_OBJECT_ASSERT(flow, NEXUS_Record_P_Flow);
    BDBG_OBJECT_ASSERT(flow->record, NEXUS_Record);
    record = flow->record;
    BDBG_ASSERT(record->cfg.recpump);

    if(!record->started) { goto done; }
    sct_size = (record->entry_size == 6)?sizeof(BSCT_SixWord_Entry):sizeof(BSCT_Entry);

    NEXUS_Time_Get(&flow->callbackTime);

    if (record->cfg.priming.index.fifoDepth) {
        const void *buffer2;
        size_t size2;
        rc = NEXUS_Recpump_GetIndexBufferWithWrap(record->cfg.recpump, &buffer, &size, &buffer2, &size2);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto done;}
        BSTD_UNUSED(buffer2);

        if (size + size2 > record->cfg.priming.index.fifoDepth) {
            size = size + size2 - record->cfg.priming.index.fifoDepth;
            size -= size % sct_size;
            rc = NEXUS_Recpump_IndexReadComplete(record->cfg.recpump, size);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
        }
        goto done;
    }

    rc = NEXUS_Recpump_GetIndexBuffer(record->cfg.recpump, &buffer, &size);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto done;}

    flow->emptyBuffer = (size == 0);

    if ( size == 0 )
    {
        /* Possible with a stale callback */
        goto done;
    }

    BDBG_MSG_TRACE(("(%#x) got %d bytes of index from %#lx", (unsigned long)flow->record, (int)size, (unsigned long)buffer));
    if(flow->info.index.indexer) {
        unsigned sct_limit;
        size_t old_size;

        /* NAV data */
        if (size < sct_size) {
            BDBG_ERR(("Got buffer from record less then size of SCT table, ignore it"));
            goto done;
        }
        size -= size % sct_size;
        old_size = size;
        /* unless we are stopping, limit number of SCT entries recorded at a time
        to 25% of ITB so we still have a fifo if I/O has a delay. */
        sct_limit = record->status.index.fifoSize / 4;
        if (size>sct_limit && !flow->stopping) {
            size = sct_limit;
        }

#ifdef SCT_WRITE
        BDBG_MSG(("sct: addr %p size %d", buffer, size));
        fwrite(buffer, 1, size, sctfile);
#endif

        BDBG_MSG(("nav: addr %p size %d(%d)", buffer, (int)size, (int)old_size));
        /* nothing */
        BNAV_Indexer_Feed(flow->info.index.indexer, (void*)buffer, size/sct_size);
        flow->size = size;
        if(size<=old_size) {
            flow->state = Idle;
            rc = NEXUS_Recpump_IndexReadComplete(record->cfg.recpump, size);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
        } else {
            flow->state = Waiting;
            flow->info.index.timer = NEXUS_ScheduleTimer(10, NEXUS_Record_P_IdleTimer, flow);
        }
    } else {
        flow->state = Idle;
        rc = NEXUS_Recpump_IndexReadComplete(record->cfg.recpump, size);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
    }

done:
    return;
}


static int
NEXUS_Record_P_GetPriority(void *cntx)
{
    NEXUS_RecordHandle record = cntx;
    NEXUS_RecpumpStatus status;
    unsigned factor;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    BDBG_ASSERT(record->cfg.recpump);

    rc = NEXUS_Recpump_GetStatus(record->cfg.recpump, &status);
    if (rc || status.data.fifoSize == 0) {
        /* If recpump has been stopped, we report back lowest priority */
        return 0;
    }

    /* priority grows as quadrat of the fifo level */
    factor = status.data.fifoDepth/(status.data.fifoSize/256); /* 0 .. 256 - where 0 empty and 256 full */
    factor = (factor * factor) >> 6; /* priority is 0 ... 1024 */
    if(record->data.partialWrite) {
        factor = factor/16; /* scale down factor if  writer wasn't able to take all data (this prevents livelock case, where buffer is full, and writer can't take data, since it'd block all other writers */
    }
    BDBG_MSG_TRACE(("[%#lx] level %d%% priority %d", (unsigned long)record,
              status.data.fifoDepth*100/status.data.fifoSize, factor));
    /* when bandhold=true, no need to test it */
    if (status.data.fifoDepth*100/status.data.fifoSize >= 50 && !record->cfg.recpumpSettings.bandHold) {
        BDBG_WRN(("[%p] CDB level is %u%%. Overflow possible.", (void *)record,
            (unsigned)(status.data.fifoDepth*100/status.data.fifoSize)));
    }
    if (status.index.fifoSize && status.index.fifoDepth*100/status.index.fifoSize >= 50 && !record->cfg.recpumpSettings.bandHold) {
        BDBG_WRN(("[%p] ITB level is %u%%. Overflow possible.", (void *)record,
            (unsigned)(status.index.fifoDepth*100/status.index.fifoSize)));
    }
    return (int)factor;
}

static void
NEXUS_Record_P_Overflow(void *context)
{
    NEXUS_RecordHandle record = context;

    BDBG_OBJECT_ASSERT(record, NEXUS_Record);

    /* we have to record the case where we abort abnormally */
    if( record->index.state == Terminated || record->data.state == Terminated)
    {
        BDBG_WRN(("Terminating and ignoring overflow!"));
        return;
    }
    BDBG_WRN(("%#lx overflow event", (unsigned long)record));
#if B_VALIDATE_NAV_ENTRIES
    /* when debugging record, it's best to die on the first overflow. */
    BKNI_Fail();
#endif
    NEXUS_TaskCallback_Fire(record->overflowCallback);
    return;
}

/**
This is an aggressive debug mode for record.
It should not be used in a real system.
#define B_VALIDATE_NAV_ENTRIES 1
**/
#if B_VALIDATE_NAV_ENTRIES
#include "../src/bcmindexerpriv.h"
static bool validate_entry(BNAV_Entry *entry)
{
    /* some of these ranges are excessive, but we're mainly looking for corruptions which should have wildly
    different values */
    if (BNAV_get_frameType(entry) > 3) { /* I,B,P */
        BDBG_ERR(("invalid BNAV_get_frameType: %d", BNAV_get_frameType(entry)));
        return false;
    }
    if (BNAV_get_seqHdrStartOffset(entry) > 10 * 1000000) { /* GOP byte size */
        BDBG_ERR(("invalid BNAV_get_seqHdrStartOffset: %d", BNAV_get_seqHdrStartOffset(entry)));
        return false;
    }
    if (BNAV_get_refFrameOffset(entry) > 500) { /* # of pictures in a GOP */
        BDBG_ERR(("invalid BNAV_get_refFrameOffset: %d", BNAV_get_refFrameOffset(entry)));
        return false;
    }
    if (BNAV_get_version(entry) > 3) { /* could be 2 for MPEG, 3 for AVC */
        BDBG_ERR(("invalid BNAV_get_version: %d", BNAV_get_version(entry)));
        return false;
    }
    /* NOTE: we're not checking file offset. to do this right for N records, we would need a data structure.
    checking for frameSize should be sufficient. */
    if (BNAV_get_frameSize(entry) > 1000000) { /* largest I frame would be MPEG HD */
        BDBG_ERR(("invalid BNAV_get_frameSize: %d", BNAV_get_frameSize(entry)));
        return false;
    }
    return true;
}
#endif

/* synchronous write of index data, either SCT or NAV */
static unsigned long
NEXUS_Record_P_WriteNav(const void *p_bfr, unsigned long numEntries, unsigned long entrySize, void *fp )
{
    NEXUS_RecordHandle record = fp;
    NEXUS_FileWriteHandle index;
    BNAV_Indexer_Position position;
    ssize_t rc;
#if B_VALIDATE_NAV_ENTRIES
    bool fail = false;
    unsigned i;
    for (i=0;i<numEntries;i++) {
        if (!validate_entry(&((BNAV_Entry*)p_bfr)[i]))
            fail = true;
    }
#endif
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    index = record->index.file;

    BDBG_MSG(("bnav_write %p, %lu", (void *)p_bfr, entrySize*numEntries));
    record->picturesIndexed += numEntries;
    rc = index->write(index, p_bfr, entrySize*numEntries);

    if(BNAV_Indexer_GetPosition(record->index.info.index.indexer, &position)>=0){
        record->lastTimestamp = position.timestamp;
    }

#if B_VALIDATE_NAV_ENTRIES
    /* fail after the NAV entry is written. it should be on disk. */
    if (fail) BKNI_Fail();
#endif

    if (rc<0) {
        return 0;
    } else {
        return (unsigned long)rc;
    }
}

void
NEXUS_Record_GetSettings(NEXUS_RecordHandle record, NEXUS_RecordSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    BDBG_ASSERT(pSettings);
    *pSettings = record->cfg;
    return;
}

NEXUS_Error
NEXUS_Record_SetSettings(NEXUS_RecordHandle record, const NEXUS_RecordSettings *settings)
{
    NEXUS_RecpumpSettings cfg;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    BDBG_ASSERT(settings);

    if(record->started && (settings->recpump != record->cfg.recpump || settings->indexFormat != record->cfg.indexFormat) ) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_settings;
    }
    if(settings->recpump==NULL && BLST_S_FIRST(&record->pid_list)) {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_settings;
    }
    if((settings->priming.data.fifoDepth==0) != (settings->priming.index.fifoDepth==0)) {
        /* must prime for index and data together */
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_settings;
    }
    if(record->started && !record->cfg.priming.data.fifoDepth && settings->priming.data.fifoDepth) {
        /* can't re-enable priming after recording to file */
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_settings;
    }
    if(record->cfg.priming.data.fifoDepth && !settings->priming.data.fifoDepth) {
        if (record->index.info.index.indexer) {
            /* we start indexing how */
            BNAV_Indexer_SetPrimingStart(record->index.info.index.indexer, record->priming.offset);
        }
    }

    if(settings->recpump) {
        cfg = settings->recpumpSettings;
        NEXUS_Recpump_GetStatus(settings->recpump, &record->status);
        if (!record->status.openSettings.data.atomSize || record->status.openSettings.data.atomSize % BIO_BLOCK_SIZE != 0) {
            BDBG_ERR(("NEXUS_Record requires that NEXUS_RecpumpOpenSettings.data.atomSize be a multiple of %d", BIO_BLOCK_SIZE));
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err_settings;
        }
        if (record->status.openSettings.data.dataReadyThreshold < B_RECORD_ATOM_SIZE) {
            BDBG_WRN(("NEXUS_RecpumpOpenSettings.data.dataReadyThreshold should be at least %d for optimal NEXUS_Record performance", B_RECORD_ATOM_SIZE));
        }
        NEXUS_CallbackHandler_PrepareCallback(record->data.dataReady, cfg.data.dataReady);
        NEXUS_CallbackHandler_PrepareCallback(record->index.dataReady, cfg.index.dataReady);
        NEXUS_CallbackHandler_PrepareCallback(record->data.overflow, cfg.data.overflow);
        NEXUS_CallbackHandler_PrepareCallback(record->index.overflow, cfg.index.overflow);
        rc = NEXUS_Recpump_SetSettings(settings->recpump, &cfg);
        if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc);goto err_recpump_settings;}
    } else {
        NEXUS_Recpump_GetDefaultSettings(&cfg);
        rc = NEXUS_Recpump_SetSettings(record->cfg.recpump, &cfg); /* wipe out previous recpump settings */
        if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc);goto err_recpump_settings;}
    }

    NEXUS_TaskCallback_Set(record->errorCallback, &settings->errorCallback);
    NEXUS_TaskCallback_Set(record->overflowCallback, &settings->overflowCallback);

    record->cfg = *settings;

    if (record->cfg.pollingTimer && !record->processDataTimer && record->started) {
        record->processDataTimer = NEXUS_ScheduleTimer(record->cfg.pollingTimer, NEXUS_Record_P_ProcessDataTimer, record);
    }

    return NEXUS_SUCCESS;

err_recpump_settings:
err_settings:
    return rc;
}

void
NEXUS_Record_GetDefaultPidChannelSettings( NEXUS_RecordPidChannelSettings *pSettings )
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&pSettings->recpumpSettings);
    return;
}

#define NEXUS_RECPUMP_PID_IS_INDEX(PSETTINGS) \
    ( (((PSETTINGS)->pidType == NEXUS_PidType_eVideo) && ((PSETTINGS)->pidTypeSettings.video.index)) || \
      (((PSETTINGS)->pidType == NEXUS_PidType_eOther) && ((PSETTINGS)->pidTypeSettings.other.index)) \
    )

NEXUS_Error
NEXUS_Record_AddPidChannel(NEXUS_RecordHandle record, NEXUS_PidChannelHandle pidChannel, const NEXUS_RecordPidChannelSettings *pSettings )
{
    NEXUS_Error rc;
    NEXUS_Record_P_PidChannel *pid;
    NEXUS_RecordPidChannelSettings settings;

    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    BDBG_ASSERT(pidChannel);
    if(pSettings==NULL) {
        NEXUS_Record_GetDefaultPidChannelSettings(&settings);
        pSettings = &settings;
    }
    if (record->started && NEXUS_RECPUMP_PID_IS_INDEX(&pSettings->recpumpSettings)) {
        rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto err_settings;
    }
    if(!record->cfg.recpump) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_recpump;}
    rc = NEXUS_Recpump_AddPidChannel(record->cfg.recpump, pidChannel, &pSettings->recpumpSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_pidchn;}
    pid = BKNI_Malloc(sizeof(*pid));
    if(!pid) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_alloc;}
    pid->pidChn = pidChannel;
    pid->cfg = *pSettings;
    BLST_S_DICT_ADD(&record->pid_list, pid, NEXUS_Record_P_PidChannel, pidChn, link, err_duplicate);
    return NEXUS_SUCCESS;

err_duplicate:
    rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    BKNI_Free(pid);
err_alloc:
    {
        NEXUS_Error rc = NEXUS_Recpump_RemovePidChannel(record->cfg.recpump, pidChannel);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}
    }
err_pidchn:
err_recpump:
err_settings:
    return rc;
}

NEXUS_Error
NEXUS_Record_RemovePidChannel(NEXUS_RecordHandle record, NEXUS_PidChannelHandle pidChannel)
{
    NEXUS_Record_P_PidChannel *pid;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    BDBG_ASSERT(pidChannel);
    if(!record->cfg.recpump) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_recpump;}
    BLST_S_DICT_REMOVE(&record->pid_list, pid, pidChannel, NEXUS_Record_P_PidChannel, pidChn, link);
    if(!pid) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_pid;}
    BKNI_Free(pid);
    rc = NEXUS_Recpump_RemovePidChannel(record->cfg.recpump, pidChannel);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto err_remove;}
    return NEXUS_SUCCESS;

err_remove:
err_recpump:
err_pid:
    return rc;
}

void
NEXUS_Record_RemoveAllPidChannels(NEXUS_RecordHandle record)
{
    NEXUS_Record_P_PidChannel *pid;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(record, NEXUS_Record);

    if (!BLST_S_FIRST(&record->pid_list)) {
        return;
    }
    if(!record->cfg.recpump) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    while(NULL!=(pid=BLST_S_FIRST(&record->pid_list))) {
        BLST_S_REMOVE_HEAD(&record->pid_list, link);
        if(record->cfg.recpump) {
            rc=NEXUS_Recpump_RemovePidChannel(record->cfg.recpump, pid->pidChn);
            if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc);}
        }
        BKNI_Free(pid);
    }
    return;
}

/**
When timeshifting with low bit rate streams, we need to poll recpump and not wait for the dataReadyThreshold to be met.
**/
static void NEXUS_Record_P_ProcessDataTimer(void *context)
{
    NEXUS_RecordHandle record = context;
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    record->processDataTimer = NULL;

    if (record->started) {
        NEXUS_Time now;
        unsigned dataTimeDiff, indexTimeDiff;
        bool processedData, processedIndex;
        unsigned delay;

        processedData = processedIndex = false;
        NEXUS_Time_Get(&now);
        dataTimeDiff = NEXUS_Time_Diff(&now, &record->data.callbackTime);
        indexTimeDiff = NEXUS_Time_Diff(&now, &record->index.callbackTime);

        if (dataTimeDiff >= record->cfg.pollingTimer) {
            NEXUS_Record_P_DataReadyCallback(&record->data);
            processedData = true;
        }
        if (indexTimeDiff >= record->cfg.pollingTimer) {
            NEXUS_Record_P_IndexReadyCallback(&record->index);
            processedIndex = true;
        }

        if (processedData && processedIndex) {
            delay = record->cfg.pollingTimer;
        }
        else if (processedData) {
            delay = record->cfg.pollingTimer - indexTimeDiff;
        }
        else if (processedIndex) {
            delay = record->cfg.pollingTimer - dataTimeDiff;
        }
        else {
            delay = (dataTimeDiff > indexTimeDiff) ? (record->cfg.pollingTimer - dataTimeDiff) : (record->cfg.pollingTimer - indexTimeDiff);
        }

        record->processDataTimer = NEXUS_ScheduleTimer(delay, NEXUS_Record_P_ProcessDataTimer, record);
    }
}

void NEXUS_Record_GetDefaultAppendSettings( NEXUS_RecordAppendSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error
NEXUS_Record_Start(NEXUS_RecordHandle record, NEXUS_FileRecordHandle file)
{
    return NEXUS_Record_StartAppend(record, file, NULL);
}

NEXUS_Error
NEXUS_Record_StartAppend(NEXUS_RecordHandle record, NEXUS_FileRecordHandle file, const NEXUS_RecordAppendSettings *pAppendSettings)
{
    bool hasIndex;
    NEXUS_Record_P_PidChannel *pid, *indexPid;
    NEXUS_Error rc;
    bool fromPlayback = false;

    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    if (!record->cfg.recpump) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    /* it's possible to do data-only or index-only record */
    if (!file) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if (!file->data && !file->index) {return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

    record->picturesIndexed = 0;
    record->lastTimestamp = 0;
    record->errorType = NEXUS_RecordErrorType_eNone;

    for(indexPid=NULL, pid=BLST_S_FIRST(&record->pid_list);pid;pid=BLST_S_NEXT(pid, link)) {
        if (NEXUS_RECPUMP_PID_IS_INDEX(&pid->cfg.recpumpSettings)) {
            indexPid = pid;
            if(pid->cfg.recpumpSettings.pidTypeSettings.video.codec != NEXUS_VideoCodec_eH264) {
                break; /* stop here, unless H264 video, in which case there could be MVC/SVC pids */
            }
        }
    }
    hasIndex = indexPid && record->cfg.indexFormat != NEXUS_RecordIndexType_eNone && file->index;
    if(hasIndex) {
        NEXUS_PidChannelStatus  status;
        BDBG_ASSERT(indexPid && indexPid->pidChn);
        rc = NEXUS_PidChannel_GetStatus(indexPid->pidChn, &status);
        if(rc==NEXUS_SUCCESS) {
            hasIndex = status.transportType == NEXUS_TransportType_eTs;
            fromPlayback = status.playback;
        } else {
            rc = BERR_TRACE(rc);
            hasIndex = false;
        }
    }

    if (file->data) {
        bio_write_attach_priority(file->data, NEXUS_Record_P_GetPriority, record);
    }

    NEXUS_Record_P_InitFlow(record, &record->index);
    NEXUS_Record_P_InitFlow(record, &record->data);
    record->index.info.index.indexer = NULL;

#ifdef SCT_WRITE
    sctfile = fopen("stream.sct", "w+");
#endif

    if (hasIndex && record->cfg.indexFormat == NEXUS_RecordIndexType_eBcm) {
        BNAV_Indexer_Settings cfg;

        BNAV_Indexer_GetDefaultSettings(&cfg);

        cfg.writeCallback = NEXUS_Record_P_WriteNav;
        cfg.filePointer = record;
        if (record->cfg.recpumpSettings.timestampType == NEXUS_TransportTimestampType_eNone) {
            cfg.transportTimestampEnabled = false;
        }
        else{
            cfg.transportTimestampEnabled = true;
        }
        cfg.allowLargeTimeGaps = record->cfg.allowLargeTimeGaps;
        switch (record->cfg.timestampSource) {
        default:
        case NEXUS_RecordTimestampSource_eAuto:
            cfg.ptsBasedFrameRate = fromPlayback; /* if playback is not paced, but create timestamp with PTS's. if paced, this is not required, but should be equivalent. */
            break;
        case NEXUS_RecordTimestampSource_eWallClock:
            cfg.ptsBasedFrameRate = false;
            break;
        case NEXUS_RecordTimestampSource_ePts:
            cfg.ptsBasedFrameRate = true;
            break;
        }

        BDBG_ASSERT(indexPid);

        if (indexPid->cfg.recpumpSettings.pidType == NEXUS_PidType_eVideo) {
            /* This logic is duplicated in bmedia_player_nav.c's bmedia_player_nav_create. It is required for bcmindexer instead of passing the codec type. */
            switch (indexPid->cfg.recpumpSettings.pidTypeSettings.video.codec) {
            case NEXUS_VideoCodec_eH264_Svc:
            case NEXUS_VideoCodec_eH264_Mvc:
                record->entry_size = 6;
                cfg.videoFormat = BNAV_Indexer_VideoFormat_AVC_SVC;
                cfg.sctVersion = BSCT_Version6wordEntry;
                cfg.navVersion = BNAV_Version_AVC_Extended;
                break;
            case NEXUS_VideoCodec_eH264:
                record->entry_size = 6;
                cfg.videoFormat = BNAV_Indexer_VideoFormat_AVC;
                cfg.sctVersion = BSCT_Version6wordEntry;
                cfg.navVersion = BNAV_Version_AVC;
                break;
            case NEXUS_VideoCodec_eH265:
                record->entry_size = 6;
                cfg.videoFormat = BNAV_Indexer_VideoFormat_HEVC;
                cfg.sctVersion = BSCT_Version6wordEntry;
                cfg.navVersion = BNAV_Version_HEVC;
                break;
            case NEXUS_VideoCodec_eMpeg2:
                cfg.videoFormat = BNAV_Indexer_VideoFormat_MPEG2;
                cfg.sctVersion = BSCT_Version6wordEntry;
                record->entry_size = 6;
                break;
            case NEXUS_VideoCodec_eVc1:
                record->entry_size = 6;
                cfg.videoFormat = BNAV_Indexer_VideoFormat_VC1;
                cfg.sctVersion = BSCT_Version6wordEntry;
                cfg.navVersion = BNAV_Version_VC1_PES;
                break;
            case NEXUS_VideoCodec_eAvs:
                cfg.videoFormat = BNAV_Indexer_VideoFormat_AVS;
                cfg.sctVersion = BSCT_Version6wordEntry;
                cfg.navVersion = BNAV_Version_AVS;
                record->entry_size = 6;
                break;
            case NEXUS_VideoCodec_eUnknown:
                /* must be audio index, which is TimestampOnly */
                cfg.sctVersion = BSCT_Version6wordEntry;
                cfg.navVersion = BNAV_Version_TimestampOnly;
                record->entry_size = 6;
                break;
            default:
                BDBG_ERR(("Video Format not supported %#x", indexPid->cfg.recpumpSettings.pidTypeSettings.video.codec));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
        }
        else if (indexPid->cfg.recpumpSettings.pidType == NEXUS_PidType_eOther) {
            record->entry_size = 6;
            cfg.sctVersion = BSCT_Version6wordEntry;
            cfg.navVersion = BNAV_Version_TimestampOnly;
        }

        if (pAppendSettings) {
            cfg.append.offsetHi = pAppendSettings->startingOffset >> 32;
            cfg.append.offsetLo = pAppendSettings->startingOffset & 0xFFFFFFFF;
            cfg.append.timestamp = pAppendSettings->timestamp;
        }

        BNAV_Indexer_Open(&record->index.info.index.indexer, &cfg);
    }
    record->index.file = file->index;
    record->data.file = file->data;

    BDBG_ASSERT(record->cfg.recpump);
    BDBG_MSG(("Starting recpump..."));
    NEXUS_StartCallbacks(record->cfg.recpump);
    rc = NEXUS_Recpump_Start(record->cfg.recpump);
    if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_start;}
    record->started = true;

    BDBG_ASSERT(!record->processDataTimer);
    if (record->cfg.pollingTimer) {
        record->processDataTimer = NEXUS_ScheduleTimer(record->cfg.pollingTimer, NEXUS_Record_P_ProcessDataTimer, record);
    }

    return NEXUS_SUCCESS;

err_start:
    return rc;
}

void
NEXUS_Record_Stop(NEXUS_RecordHandle record)
{
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);

    if(!record->started) {
        return;
    }

    if (record->processDataTimer) {
        NEXUS_CancelTimer(record->processDataTimer);
        record->processDataTimer = NULL;
    }

    /* this may generate callbacks to save last recorded data */
    record->data.stopping = true;
    record->index.stopping = true;
    BDBG_ASSERT(record->cfg.recpump);
    NEXUS_Recpump_StopData(record->cfg.recpump);

    if (record->cfg.writeAllTimeout) {
        unsigned timeout;
        NEXUS_Error rc;
        NEXUS_RecpumpStatus status;
        for(timeout=0; timeout < record->cfg.writeAllTimeout; timeout+=10) {
           rc = NEXUS_Recpump_GetStatus(record->cfg.recpump, &status);
           BDBG_ASSERT(rc == NEXUS_SUCCESS);
           if (status.data.fifoDepth==0 && status.index.fifoDepth==0) {
                 break;
           }
           NEXUS_UnlockModule();
           BKNI_Sleep(10);
           NEXUS_LockModule();
        }
    }

    NEXUS_StopCallbacks(record->cfg.recpump);
    NEXUS_CallbackHandler_Stop(record->data.dataReady);
    NEXUS_CallbackHandler_Stop(record->index.dataReady);
    NEXUS_CallbackHandler_Stop(record->data.overflow);
    NEXUS_CallbackHandler_Stop(record->index.overflow);

    record->started = false;
    record->data.file = NULL; /* this is the signal to exit. all recpump callbacks will already have been processed */
    record->index.file = NULL; /* this is the signal to exit. all recpump callbacks will already have been processed */

    while (record->data.state == Writing) {
        BDBG_WRN(("Record writing something to disk, waiting for it: %d", record->data.state));
        NEXUS_UnlockModule();
        BKNI_Sleep(50); /* sleep 50 ms */
        NEXUS_LockModule();
    }
    while (record->index.state == Writing) {
        BDBG_WRN(("Record writing something to disk, waiting for it: %d", record->index.state));
        NEXUS_UnlockModule();
        BKNI_Sleep(50); /* sleep 50 ms */
        NEXUS_LockModule();
    }

    NEXUS_Recpump_Stop(record->cfg.recpump);

    if (record->index.info.index.indexer) {
        BNAV_Indexer_Close(record->index.info.index.indexer);
        record->index.info.index.indexer = NULL;
    }
    record->data.stopping = false;
    record->index.stopping = false;
    if(record->index.info.index.timer) {
        NEXUS_CancelTimer(record->index.info.index.timer);
        record->index.info.index.timer = NULL;
    }
#if NEXUS_HAS_PLAYBACK
    if(BLST_S_FIRST(&record->playback_list)) {
        BDBG_WRN(("NEXUS_Recpump_Stop: %p stopping with playback attached", (void *)record));
        NEXUS_Record_RemoveAllPlaybacks(record);
    }
#endif

    /* test that no callback reentrancy happened during this Stop call. NEXUS_CallbackHandler_Stop ensures that. */
    BDBG_ASSERT(record->data.state != Writing);
    BDBG_ASSERT(record->index.state != Writing);

    return;
}

NEXUS_Error
NEXUS_Record_AddPlayback(NEXUS_RecordHandle record, NEXUS_PlaybackHandle playback)
{
#if NEXUS_HAS_PLAYBACK
    NEXUS_Error rc;
    NEXUS_Record_P_Playback *play_item;

    if (!g_NEXUS_Record_P_ModuleState.cfg.modules.playback) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    play_item = BKNI_Malloc(sizeof(*play_item));
    if(!play_item) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    play_item->playback = playback;
    BLST_S_DICT_ADD(&record->playback_list, play_item, NEXUS_Record_P_Playback, playback, link, err_duplicate);
    NEXUS_Module_Lock(g_NEXUS_Record_P_ModuleState.cfg.modules.playback);
    NEXUS_Playback_AddRecordProgress_priv(play_item->playback);
    NEXUS_Module_Unlock(g_NEXUS_Record_P_ModuleState.cfg.modules.playback);

    return NEXUS_SUCCESS;
err_duplicate:
    rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
    BKNI_Free(play_item);
err_alloc:
    return rc;
#else
    BSTD_UNUSED(record);
    BSTD_UNUSED(playback);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error
NEXUS_Record_RemovePlayback( NEXUS_RecordHandle record, NEXUS_PlaybackHandle playback)
{
#if NEXUS_HAS_PLAYBACK
    NEXUS_Record_P_Playback *play_item;
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    BLST_S_DICT_REMOVE(&record->playback_list, play_item, playback, NEXUS_Record_P_Playback, playback, link);
    if(play_item==NULL) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}
    NEXUS_Module_Lock(g_NEXUS_Record_P_ModuleState.cfg.modules.playback);
    NEXUS_Playback_RemoveRecordProgress_priv(play_item->playback);
    NEXUS_Module_Unlock(g_NEXUS_Record_P_ModuleState.cfg.modules.playback);
    BKNI_Free(play_item);
    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(record);
    BSTD_UNUSED(playback);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

void
NEXUS_Record_RemoveAllPlaybacks( NEXUS_RecordHandle record)
{
#if NEXUS_HAS_PLAYBACK
    NEXUS_Record_P_Playback *play_item;
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);

    while(NULL!=(play_item=BLST_S_FIRST(&record->playback_list))) {
        BLST_S_REMOVE_HEAD(&record->playback_list, link);
        BKNI_Free(play_item);
    }
#else
    BSTD_UNUSED(record);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
    return;
}

NEXUS_Error NEXUS_Record_GetStatus(NEXUS_RecordHandle record, NEXUS_RecordStatus *pStatus)
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(record, NEXUS_Record);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (record->cfg.recpump) {
        rc = NEXUS_Recpump_GetStatus(record->cfg.recpump, &pStatus->recpumpStatus);
        if (rc) return BERR_TRACE(rc);
    }

    pStatus->error = record->errorType;
    pStatus->lastTimestamp = record->lastTimestamp;
    pStatus->picturesIndexed = record->picturesIndexed;

    if (record->index.info.index.indexer) {
        BNAV_Indexer_Status status;
        BNAV_Indexer_GetStatus(record->index.info.index.indexer, &status);
        pStatus->indexParsingErrors = status.parsingErrors;
    }

    return 0;
}
