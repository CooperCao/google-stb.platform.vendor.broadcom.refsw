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

#include "bstd.h"
#include "bkni.h"
#include "bpvrlib_feed.h"

/* to import TRANS_DESC flags */
#include "bxpt_priv.h"
#if BXPT_HAS_TSMUX
#include "bxpt_tsmux.h"
#endif

BDBG_MODULE(bpvrlib_feed);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

/* save descriptors of the XPT playback */
#define B_PVR_LIB_FEED_SAVE_DESC 0
/* save data prior to sending data into the XPT playback */
#define B_PVR_LIB_FEED_SAVE_BEFORE 0
/* save data after XPT playback completed transfer */
#define B_PVR_LIB_FEED_SAVE_AFTER 0
#define B_PVR_LIB_FEED_CONTEXT_NAME 0

BDBG_OBJECT_ID(BPVRlib_Feed);

#if B_PVR_LIB_FEED_SAVE_BEFORE || B_PVR_LIB_FEED_SAVE_AFTER || B_PVR_LIB_FEED_SAVE_DESC
#include <stdio.h>
typedef struct b_pvr_feed_save{
    FILE *fout;
    unsigned no;
    const char *name;
} b_pvr_feed_save;

static void b_pvr_feed_save_init(b_pvr_feed_save *save, const char *name)
{
    save->name = name;
    save->fout = NULL;
    save->no = 0;
    return;
}

static void b_pvr_feed_save_open(b_pvr_feed_save *save)
{
    char name[64];
    BKNI_Snprintf(name, sizeof(name),
#if B_PVR_LIB_FEED_CONTEXT_NAME
  "videos/bpvrlib_feed_%#x_%s_%u.mpg", save,
#else
  "videos/bpvrlib_feed_%s_%u.mpg",
#endif
    save->name, save->no);
    save->fout = fopen(name, "w+b");
    return;
}

static void b_pvr_feed_save_close(b_pvr_feed_save *save)
{
    if(save->fout) {
        fclose(save->fout);
        save->fout = NULL;
        save->no++;
    }
    return;
}

static void b_pvr_feed_save_data(b_pvr_feed_save *save, const void *data, size_t len)
{
   if(save->fout) {
       BDBG_MSG(("D: %-8s: %#x:%u", save->name, (unsigned)data, (unsigned)len));
       fwrite(data, len, 1, save->fout);
   }
   return;
}

#endif /* B_PVR_LIB_FEED_SAVE_BEFORE || B_PVR_LIB_FEED_SAVE_AFTER */

#define B_PVRLIB_SMALL_DESC_BUG   BXPT_2_BYTE_MINIMUM_PLAYBACK_BUFFER

#define BFIFO_HEAD(name, type) struct name { type *bf_base; type *bf_last; type *bf_read; type *bf_write; int bf_wrap; }

#define BFIFO_INIT(fifo, base, size) do {(fifo)->bf_wrap=0; \
    (fifo)->bf_base=(fifo)->bf_read=(fifo)->bf_write=(base);\
    (fifo)->bf_last=(fifo)->bf_base+(size);}while(0)

#define BFIFO_WRITE(fifo) (fifo)->bf_write
#define BFIFO_READ(fifo) (fifo)->bf_read

#define BFIFO_WRITE_PEEK(fifo) \
    /* |====W---R===| */ ((unsigned)(((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_read - (fifo)->bf_write : ( \
    /* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_last - (fifo)->bf_write  : ( \
    /* |---RW---| */ (fifo)->bf_wrap ? 0 : (fifo)->bf_last - (fifo)->bf_write))))

#define BFIFO_READ_PEEK(fifo) \
    /* |====W---R===| */ ((unsigned)(((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_last - (fifo)->bf_read : ( \
    /* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_write - (fifo)->bf_read : ( \
    /* |---RW---| */ (fifo)->bf_wrap ? (fifo)->bf_last - (fifo)->bf_read:0))))

#define BFIFO_WRITE_COMMIT(fifo, size) do { \
    BDBG_ASSERT((size)>0 && BFIFO_WRITE_PEEK(fifo) >= (size));  \
    (fifo)->bf_write += (size); \
    if ((fifo)->bf_write >= (fifo)->bf_last) {(fifo)->bf_write = (fifo)->bf_base;(fifo)->bf_wrap++;}  \
    } while(0)

#define BFIFO_READ_COMMIT(fifo, size) do { \
    BDBG_ASSERT(BFIFO_READ_PEEK(fifo) >= (unsigned)(size)); \
    (fifo)->bf_read += (size); \
    if ((fifo)->bf_read >= (fifo)->bf_last) {(fifo)->bf_read = (fifo)->bf_base;(fifo)->bf_wrap--;}  \
    } while(0)

#define BFIFO_WRITE_LEFT(fifo)  \
    /* |====W---R===| */ (((fifo)->bf_write < (fifo)->bf_read) ? (fifo)->bf_read - (fifo)->bf_write : ( \
    /* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? ((fifo)->bf_read - (fifo)->bf_base) + ((fifo)->bf_last - (fifo)->bf_write)  : ( \
    /* |---RW---| */ (fifo)->bf_wrap ? 0 : (fifo)->bf_last - (fifo)->bf_base)))

#define BFIFO_READ_LEFT(fifo) \
    /* |====W---R===| */ (((fifo)->bf_write < (fifo)->bf_read) ? ((fifo)->bf_write - (fifo)->bf_base) + (fifo)->bf_last - (fifo)->bf_read : ( \
    /* |---R===W---| */ ((fifo)->bf_write > (fifo)->bf_read) ? (fifo)->bf_write - (fifo)->bf_read : ( \
    /* |---RW---| */ (fifo)->bf_wrap ? (fifo)->bf_last - (fifo)->bf_base:0)))

#define BFIFO_VALIDATE(fifo) do { \
        BDBG_ASSERT((fifo)->bf_wrap==0 || (fifo)->bf_wrap==1); \
        BDBG_ASSERT((fifo)->bf_read>=(fifo)->bf_base && (fifo)->bf_read<(fifo)->bf_last); \
        BDBG_ASSERT((fifo)->bf_write>=(fifo)->bf_base && (fifo)->bf_write<(fifo)->bf_last); \
    } while(0)

#define BFIFO_STATUS(DBG, header,fifo)  DBG((header ":(%p:%p) write (%u/%u/%p), read (%u/%u/%p)", (void *)(fifo)->bf_base, (void *)(fifo)->bf_last, (unsigned)BFIFO_WRITE_PEEK(fifo), (unsigned)BFIFO_WRITE_LEFT(fifo), (void *)BFIFO_WRITE(fifo), (unsigned)BFIFO_READ_PEEK(fifo), (unsigned)BFIFO_READ_LEFT(fifo), (void *)BFIFO_READ(fifo)))

BFIFO_HEAD(B_PVRlib_DescrFifo, BXPT_PvrDescriptor);


/* nc_ stands for Non Cached address, all other pointers are cached */
struct BPVRlib_Feed {
    BDBG_OBJECT(BPVRlib_Feed)
    bool active; /* set to true if hardware sending data */
#if !BXPT_HAS_MULTICHANNEL_PLAYBACK
    bool finished; /* true if finished bit is set */
    bool false_finished; /* true if finished bit is set and new descriptor was added, finished bit then might belong to the previous descriptor */
    const BXPT_PvrDescriptor *prev_hw_finished_desc; /* previous descriptor that was active in the hardware */
#else
    const BXPT_PvrDescriptor *prev_hw_completed_desc; /* previous descriptor that was completed by the hardware */
#endif
    const BXPT_PvrDescriptor *last_desc; /* last descriptor submitted to hardware */
    struct B_PVRlib_DescrFifo descFifo;
    size_t  ncompleted;
    uint8_t *databuf_ptr;
    BSTD_DeviceOffset databuf_offset;
    BMMA_Block_Handle descBlock;
    BXPT_PvrDescriptor *desc; /* cached pointer to the descriptors array */
    BXPT_PvrDescriptor *nc_desc; /* no-cached pointer to the descriptors array */
    BSTD_DeviceOffset off_desc; /* device offset for the descriptor */
    BPVRlib_Feed_Settings config;
    BINT_CallbackHandle hPlayInt;   /* cb Handle for playback interrupt */
    unsigned descScale;
#if B_PVRLIB_SMALL_DESC_BUG
#define B_PVRLIB_BOUNCE_BUF_SIZE    256
#define B_PVRLIB_SMALL_ADDR_TEST(a1,a2) (((a1)&0x1F) == ((a2)&0x1F) || ((a2)&0xF)==0)
    uint8_t *bounce_buffer;
    BSTD_DeviceOffset bounce_offset;
    size_t bounce_ptr;
    BSTD_DeviceOffset last_data_addr;
#else
#define B_PVRLIB_BOUNCE_BUF_SIZE    0
#endif
    BPVRlib_Feed_OffsetEntry offset_entries[8];
#if B_PVR_LIB_FEED_SAVE_BEFORE || B_PVR_LIB_FEED_SAVE_AFTER || B_PVR_LIB_FEED_SAVE_DESC
    struct {
#if B_PVR_LIB_FEED_SAVE_DESC
        b_pvr_feed_save desc;
#endif
#if B_PVR_LIB_FEED_SAVE_BEFORE
        b_pvr_feed_save before;
#endif
#if B_PVR_LIB_FEED_SAVE_AFTER
        b_pvr_feed_save after;
#endif
    } save;
#endif
};


#if B_PVR_LIB_FEED_SAVE_BEFORE || B_PVR_LIB_FEED_SAVE_AFTER || B_PVR_LIB_FEED_SAVE_DESC
static void b_pvr_feed_save_desc(BPVRlib_Feed_Handle feed, b_pvr_feed_save *save, const BXPT_PvrDescriptor *desc)
{
    void *cachedAddr;
    BSTD_DeviceOffset  data = desc->BufferStartAddr;
    uint32_t len = desc->BufferLength;
#if BXPT_HAS_MCPB_VER_3
    data |= ((uint64_t)desc->BufferStartAddrHi) << 32;
#endif

    if (len==0) { return; }

    cachedAddr = feed->config.offsetToCachedAddr(data);
    if (cachedAddr==NULL) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }

    feed->config.flushCache(cachedAddr, len);
    b_pvr_feed_save_data(save, cachedAddr, len);
}
#endif

static void BPVRlib_Feed_Priv_Update(BPVRlib_Feed_Handle feed);

/* no-op function */
static void* BPVRlib_Feed_OffsetToCachedAddr(uint64_t offset)
{
    BSTD_UNUSED(offset);
    return NULL;
}

/* no-op function */
static void BPVRlib_Feed_FlushCache(const void *pvAddr, size_t ulNumBytes)
{
    BSTD_UNUSED(pvAddr);
    BSTD_UNUSED(ulNumBytes);
    return;
}

void
BPVRlib_Feed_GetDefaultSettings(BPVRlib_Feed_Settings *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    config->offsetToCachedAddr = BPVRlib_Feed_OffsetToCachedAddr;
    config->flushCache = BPVRlib_Feed_FlushCache;
    return;
}

static void
BPVRlib_Feed_Priv_Reset(BPVRlib_Feed_Handle feed)
{
    feed->ncompleted = 0;
    feed->active = false;
#if !BXPT_HAS_MULTICHANNEL_PLAYBACK
    feed->false_finished = false;
    feed->finished = false;
    feed->prev_hw_finished_desc = NULL;
#else
    feed->prev_hw_completed_desc = NULL;
#endif
    feed->last_desc = NULL;
    BFIFO_INIT(&feed->descFifo, feed->desc, feed->config.numDesc*feed->descScale);
#if B_PVRLIB_SMALL_DESC_BUG
    feed->bounce_buffer = (uint8_t *)feed->nc_desc + feed->descScale*sizeof(*feed->desc)*feed->config.numDesc;
    feed->bounce_offset = feed->off_desc + feed->descScale*sizeof(*feed->desc)*feed->config.numDesc;
    feed->bounce_ptr = 0;
    feed->last_data_addr = 0;
#endif
    return;
}

BERR_Code
BPVRlib_Feed_Open(BPVRlib_Feed_Handle *pfeed, const BPVRlib_Feed_Settings *config)
{
    BERR_Code rc;
    BPVRlib_Feed_Handle feed;
    BINT_Id playInt;
    BXPT_Playback_ChannelSettings channelSettings;

    BDBG_ASSERT(pfeed);
    *pfeed = NULL;
    if(config == NULL) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_parameter;
    }
    if(config->xptHandle==NULL || config->xptPlayHandle==NULL) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_parameter;
    }
    if(config->mmaHeap==NULL || config->intHandle==NULL) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_parameter;
    }
    if( config->numDesc<2) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_parameter;
    }
#if !BXPT_HAS_TSMUX
    if(config->useExtndedDesc) {
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_parameter;
    }
#endif

    feed = BKNI_Malloc(sizeof(*feed));
    if(!feed) {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(feed, BPVRlib_Feed);
    feed->config = *config;
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    feed->descScale = 1;
#elif BXPT_HAS_TSMUX
    feed->descScale = config->useExtndedDesc ? 2: 1;
#else
    feed->descScale = 1;
    if(config->useExtndedDesc) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_desc;
    }
#endif
    feed->config.numDesc += feed->descScale; /* increase number of descriptors, since last descriptor can't be always used */

    feed->descBlock = BMMA_Alloc(config->mmaHeap, sizeof(*feed->desc)*feed->descScale*(feed->config.numDesc+B_PVRLIB_BOUNCE_BUF_SIZE), 16, NULL);
    if(!feed->descBlock) {
        rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err_desc;
    }
    feed->nc_desc = BMMA_Lock(feed->descBlock);
    if (!feed->nc_desc) { /* CPU inaccessible */
        rc = BERR_TRACE(BERR_UNKNOWN);
        goto err_cached;
    }

    feed->desc = feed->nc_desc;
    feed->off_desc = BMMA_LockOffset(feed->descBlock);
    BXPT_Playback_GetChannelSettings(config->xptPlayHandle, &channelSettings);
    channelSettings.descBlock = feed->descBlock;
    BXPT_Playback_SetChannelSettings(config->xptPlayHandle, &channelSettings);

    if (feed->config.bufferBlock) { /* can be NULL for s/g mode */
        feed->databuf_ptr = BMMA_Lock(feed->config.bufferBlock);
        if (!feed->databuf_ptr) { /* CPU inaccessible */
            rc = BERR_TRACE(BERR_UNKNOWN);
            goto err_cached;
        }
        feed->databuf_ptr += feed->config.bufferOffset;
        feed->databuf_offset = BMMA_LockOffset(feed->config.bufferBlock) + feed->config.bufferOffset;
    }

    BKNI_Memset(feed->desc, 0, sizeof(*feed->desc)*feed->descScale*(feed->config.numDesc+B_PVRLIB_BOUNCE_BUF_SIZE));
    BPVRlib_Feed_Priv_Reset(feed);
    playInt = BXPT_Playback_GetIntId(config->xptPlayHandle,BXPT_PbInt_eDone);
    feed->hPlayInt = NULL;
    if(config->descAvaliable_isr) {
        rc = BINT_CreateCallback(&feed->hPlayInt, feed->config.intHandle, playInt, config->descAvaliable_isr, config->applicationCnxt, config->applicationNumber);
        if(rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
            goto err_int_callback;
        }
    }
#if B_PVR_LIB_FEED_SAVE_DESC
    b_pvr_feed_save_init(&feed->save.desc,"desc");
#endif
#if B_PVR_LIB_FEED_SAVE_BEFORE
    b_pvr_feed_save_init(&feed->save.before,"before");
#endif
#if B_PVR_LIB_FEED_SAVE_AFTER
    b_pvr_feed_save_init(&feed->save.after,"after");
#endif

    *pfeed = feed;
    return BERR_SUCCESS;

err_int_callback:
err_cached:
    BMMA_Free(feed->descBlock);
err_desc:
    BKNI_Free(feed);
err_alloc:
err_parameter:
    return rc;
}


void
BPVRlib_Feed_Close(BPVRlib_Feed_Handle feed)
{
    BXPT_Playback_ChannelSettings channelSettings;
    BDBG_OBJECT_ASSERT(feed, BPVRlib_Feed);
#if B_PVR_LIB_FEED_SAVE_BEFORE
    b_pvr_feed_save_close(&feed->save.before);
#endif
#if B_PVR_LIB_FEED_SAVE_AFTER
    b_pvr_feed_save_close(&feed->save.after);
#endif
#if B_PVR_LIB_FEED_SAVE_DESC
    b_pvr_feed_save_init(&feed->save.desc,"desc");
#endif
    if(feed->hPlayInt) {
        BINT_DestroyCallback(feed->hPlayInt);
    }
    if (feed->config.bufferBlock) {
        BMMA_Unlock(feed->config.bufferBlock, feed->databuf_ptr);
        BMMA_UnlockOffset(feed->config.bufferBlock, feed->databuf_offset);
    }

    BXPT_Playback_GetChannelSettings(feed->config.xptPlayHandle, &channelSettings);
    channelSettings.descBlock = NULL;
    BXPT_Playback_SetChannelSettings(feed->config.xptPlayHandle, &channelSettings);

    BMMA_Unlock(feed->descBlock, feed->nc_desc);
    BMMA_UnlockOffset(feed->descBlock, feed->off_desc);
    BMMA_Free(feed->descBlock);

    BDBG_OBJECT_DESTROY(feed, BPVRlib_Feed);
    BKNI_Free(feed);
    return;
}


BERR_Code
BPVRlib_Feed_Start(BPVRlib_Feed_Handle feed)
{
    BERR_Code rc;
    BERR_Code erc;
    BDBG_OBJECT_ASSERT(feed, BPVRlib_Feed);
    rc = BXPT_Playback_StartChannel(feed->config.xptPlayHandle);
    if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_start; }

    if(feed->hPlayInt) {
        rc = BINT_EnableCallback(feed->hPlayInt);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_enable_int;}
    }
    BPVRlib_Feed_Priv_Reset(feed);
#if B_PVR_LIB_FEED_SAVE_DESC
    b_pvr_feed_save_open(&feed->save.desc);
#endif
#if B_PVR_LIB_FEED_SAVE_BEFORE
    b_pvr_feed_save_open(&feed->save.before);
#endif
#if B_PVR_LIB_FEED_SAVE_AFTER
    b_pvr_feed_save_open(&feed->save.after);
#endif
    return BERR_SUCCESS;

err_enable_int:
    erc = BXPT_Playback_StopChannel(feed->config.xptPlayHandle); /* use different 'erc' in the error path */
    if(erc!=BERR_SUCCESS) { erc = BERR_TRACE(erc); } /* keep on going */
err_start:
    return rc;
}


void
BPVRlib_Feed_Stop(BPVRlib_Feed_Handle feed)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(feed, BPVRlib_Feed);
#if B_PVR_LIB_FEED_SAVE_DESC
    b_pvr_feed_save_close(&feed->save.desc);
#endif
#if B_PVR_LIB_FEED_SAVE_BEFORE
    b_pvr_feed_save_close(&feed->save.before);
#endif
#if B_PVR_LIB_FEED_SAVE_AFTER
    b_pvr_feed_save_close(&feed->save.after);
#endif
    rc = BXPT_Playback_StopChannel(feed->config.xptPlayHandle);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
    }
    if(feed->hPlayInt) {
        BINT_DisableCallback(feed->hPlayInt);
    }
    return;
}

static void BPVRlib_Feed_P_SetBufferStartAddr(BXPT_PvrDescriptor *desc, BSTD_DeviceOffset offset)
{
    uint32_t offsetHi = offset >> 32;
#if BXPT_HAS_MCPB_VER_3
    desc->BufferStartAddrHi = offsetHi;
#else
    if(offsetHi!=0) {
        BDBG_ERR(("Truncating buffer address " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(offset)));
    }
#endif
    desc->BufferStartAddr = (uint32_t)offset;
    return;
}

static void BPVRlib_Feed_P_SetNextDescAddr(BXPT_PvrDescriptor *desc, BSTD_DeviceOffset offset)
{
    uint32_t offsetHi = offset >> 32;
#if BXPT_HAS_MCPB_VER_3
    desc->NextDescAddrHi = offsetHi;
#else
    if(offsetHi!=0) {
        BDBG_ERR(("Truncating descriptor address " BDBG_UINT64_FMT "", BDBG_UINT64_ARG(offset)));
    }
#endif
    desc->NextDescAddr = (uint32_t)offset;
    return;
}

static unsigned
BPVRlib_Feed_Priv_AddOffsetEntries(BPVRlib_Feed_Handle feed, const BPVRlib_Feed_OffsetEntry *entries, size_t nentries, bool last, bool extended)
{
    unsigned entries_left;
    unsigned entrySize;
    BERR_Code rc;
    BXPT_PvrDescriptor *prev_desc;
    BXPT_PvrDescriptor *first_desc=NULL;
    BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_AddOffsetEntries>: %p %p:%u %s", (void *)feed, (void *)entries, (unsigned)nentries, last?"LAST":""));

    entrySize = extended ? sizeof(BPVRlib_Feed_ExtendedOffsetEntry) : sizeof(*entries);

    for(prev_desc=NULL, entries_left=nentries;entries_left>0;) {
        unsigned i;
        unsigned nwrite_desc = BFIFO_WRITE_PEEK(&feed->descFifo);
        BXPT_PvrDescriptor *write_desc;
        BXPT_PvrDescriptor *desc;

        if(nwrite_desc==0 ||
                (nwrite_desc<=feed->descScale && BFIFO_WRITE_LEFT(&feed->descFifo)==(int)feed->descScale) /* we should always keep one entry in the FIFO empty since it's reserved for the 'last' dscriptor that is used for chaining */
                ) {
            break;
        }
        if(nwrite_desc>entries_left*feed->descScale) {
            nwrite_desc = entries_left*feed->descScale;
        }
        BDBG_ASSERT(nwrite_desc>0);
        write_desc = BFIFO_WRITE(&feed->descFifo);
#if !BXPT_HAS_MULTICHANNEL_PLAYBACK
        if(feed->finished) {
            feed->finished = false;
            feed->false_finished = true;
        }
        if(feed->false_finished && write_desc+(nwrite_desc-feed->descScale) == feed->prev_hw_finished_desc) {
            BDBG_WRN(("BPVRlib_Feed_Priv_AddOffsetEntries: %p preventing use of the last_finished_desc %p:%u", (void *)feed, (void *)feed->prev_hw_finished_desc, nwrite_desc));
            if(nwrite_desc==feed->descScale) {
                unsigned i;
                for(i=0;i<10;i++) {
                    BPVRlib_Feed_Priv_Update(feed);
                    if(!feed->false_finished) {
                        break;
                    }
                    BDBG_WRN(("BPVRlib_Feed_Priv_AddOffsetEntries: %p waiting(%u) to release last_finished_desc %p", (void *)feed, i, (void *)feed->prev_hw_finished_desc));
                    BKNI_Sleep(1);
                }
                /* try to use it anyway */
            } else {
                nwrite_desc-=feed->descScale;
            }
        }
#endif
        BDBG_ASSERT(write_desc);
        BFIFO_WRITE_COMMIT(&feed->descFifo, nwrite_desc);
        entries_left -= nwrite_desc/feed->descScale;
        if(first_desc==NULL) {
            first_desc = write_desc;
        }
        for( desc=write_desc, i=0; i<nwrite_desc; i+=feed->descScale, desc+=feed->descScale, entries=(void *)((uint8_t*)entries+entrySize) ) {
           desc->Flags = 0;
#if BXPT_HAS_TSMUX
            if(feed->config.useExtndedDesc) {
                BXPT_PvrDescriptor *desc8 = (void *)(feed->nc_desc + (desc - feed->desc));

                if(entrySize==sizeof(*entries)) {
                    BAVC_TsMux_DescConfig flags;

                    BKNI_Memset( &flags, 0, sizeof (flags) );
                    BXPT_Playback_ConfigTsMuxDesc(feed->config.xptPlayHandle, desc8, &flags);
                } else {
                    BXPT_Playback_ConfigTsMuxDesc(feed->config.xptPlayHandle, desc8, &((BPVRlib_Feed_ExtendedOffsetEntry *)entries)->flags);
                }
            }
#endif
            BPVRlib_Feed_P_SetBufferStartAddr(desc, entries->offset);
            desc->BufferLength = entries->len;
            if(prev_desc) {
                BPVRlib_Feed_P_SetNextDescAddr(prev_desc, feed->off_desc + ((uint8_t *)desc - (uint8_t *)feed->desc));
                if(i==0) { /* set NextDescAddr over the wrap */
                    BMMA_FlushCache(feed->descBlock, prev_desc, sizeof(*prev_desc));
                }
            }

            prev_desc = desc;
            BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_AddOffsetEntries: %p desc %u:%p %#lx(%#lx:%u)", (void *)feed, i, (void *)desc, (unsigned long)(feed->off_desc + ((uint8_t *)desc - (uint8_t *)feed->desc)), (unsigned long)entries->offset, (unsigned)entries->len));
#if B_PVRLIB_SMALL_DESC_BUG
            switch(entries->len) {
            case 0:
                desc->BufferLength = 2;
                BDBG_WRN(("BPVRlib_Feed_Priv_AddOffsetEntries: %#lx not supported S/G entry %#lx:%u, replaced with %#lx:%u", (unsigned long)feed, (unsigned long)entries->offset, (unsigned)entries->len, (unsigned long)desc->BufferStartAddr, (unsigned)desc->BufferLength));
                break;
            case 1:
                BDBG_WRN(("BPVRlib_Feed_Priv_AddOffsetEntries: %#lx S/G entry %#lx:%u(%#lx)", (unsigned long)feed, (unsigned long)entries->offset, (unsigned)entries->len, (unsigned long)feed->last_data_addr));
                if(B_PVRLIB_SMALL_ADDR_TEST(feed->last_data_addr, entries->offset)) {
                    void *addr;
                    BERR_Code rc = BMEM_Heap_ConvertOffsetToAddress(feed->config.xptPlayHandle->hMemory, entries->offset, &addr);
                    if(rc==BERR_SUCCESS) { /* XXX This path uses uncached addressses to copy contents of the descriptor to a bounce buffer */
                        for(;B_PVRLIB_SMALL_ADDR_TEST(feed->last_data_addr, feed->bounce_offset+(feed->bounce_ptr%B_PVRLIB_BOUNCE_BUF_SIZE));feed->bounce_ptr++) { }
                        feed->bounce_buffer[feed->bounce_ptr%B_PVRLIB_BOUNCE_BUF_SIZE]=*(uint8_t *)addr;
                        BPVRlib_Feed_P_SetBufferStartAddr(desc, feed->bounce_offset+(feed->bounce_ptr%B_PVRLIB_BOUNCE_BUF_SIZE));
                        feed->bounce_ptr++;
                    } else {
                        rc=BERR_TRACE(rc);
                        desc->BufferLength = 2;
                    }
                    BDBG_WRN(("BPVRlib_Feed_Priv_AddOffsetEntries: %#lx not supported S/G entry %#lx:%u, replaced with %#lx:%u", (unsigned long)feed, (unsigned long)entries->offset, (unsigned)entries->len, (unsigned long)desc->BufferStartAddr, (unsigned)desc->BufferLength));
                }
                break;
            default:
                break;
            }
            feed->last_data_addr = entries->offset + entries->len + 1;
#endif /* B_PVRLIB_SMALL_ADDR_TEST */
#if B_PVR_LIB_FEED_SAVE_DESC
            b_pvr_feed_save_data(&feed->save.desc, desc, feed->descScale*sizeof(*desc));
#endif
#if B_PVR_LIB_FEED_SAVE_BEFORE
            b_pvr_feed_save_desc(feed, &feed->save.before, desc);
#endif
        }
        prev_desc->NextDescAddr = TRANS_DESC_LAST_DESCR_IND;
        BMMA_FlushCache(feed->descBlock, write_desc, nwrite_desc*sizeof(*write_desc));
        feed->last_desc = prev_desc;
    }
    if(first_desc) {
        BXPT_PvrDescriptor *nc_prev_desc;
        BXPT_PvrDescriptor *nc_first_desc;
        BDBG_ASSERT(prev_desc!=NULL);
        /* convert first descriptor to uncached address */
        feed->active = true;
        nc_first_desc = feed->nc_desc + (first_desc - feed->desc);
        nc_prev_desc = feed->nc_desc + (prev_desc- feed->desc);
        if(last || nentries!=entries_left ) {
            prev_desc->Flags |= TRANS_DESC_INT_FLAG;
            BMMA_FlushCache(feed->descBlock, prev_desc, sizeof(*prev_desc));
        }
        BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_AddOffsetEntries: %p first:%p(%#lx) last:%p(%#lx)", (void *)feed, (void *)first_desc, (unsigned long)(feed->off_desc + ((uint8_t *)first_desc - (uint8_t *)feed->nc_desc)), (void *)prev_desc, (unsigned long)(feed->off_desc + ((uint8_t *)prev_desc - (uint8_t *)feed->nc_desc))));
        rc = BXPT_Playback_AddDescriptors(feed->config.xptPlayHandle, nc_prev_desc, nc_first_desc); /* last descriptor goes as second argument and first descriptor goes as third argument */
        if(rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
        }
    }
    BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_AddOffsetEntries<: %p %p:%u ->%u", (void *)feed, (void *)entries, (unsigned)nentries, (unsigned)nentries-entries_left));
    return nentries-entries_left;
}

static void
BPVRlib_Feed_Priv_Update(BPVRlib_Feed_Handle feed)
{
    BERR_Code rc;
    BXPT_PvrDescriptor *nc_hw_desc;
    BXPT_PvrDescriptor *hw_desc;
    unsigned hw_complete;
    unsigned nread_desc;

    BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update>: %p ncompleted:%u", (void *)feed, (unsigned)feed->ncompleted));
    if(!feed->active) {
        goto done;
    }
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    rc = BXPT_Playback_GetLastCompletedDescriptorAddress(feed->config.xptPlayHandle, &nc_hw_desc); /* [in] Handle for the playback channel */
#else
    rc = BXPT_Playback_GetCurrentDescriptorAddress(feed->config.xptPlayHandle, &nc_hw_desc);
#endif
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto error;
    }
    hw_desc = feed->desc + (nc_hw_desc - feed->nc_desc);  /* convert to cached address */
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    if (nc_hw_desc==NULL) { /* no descriptors have completed */
        goto done;
    }
    BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update: %p hw_desc:%p prev_hw_completed_desc:%p", (void *)feed, (void *)hw_desc, (void *)feed->prev_hw_completed_desc));
    if(hw_desc == feed->prev_hw_completed_desc) { /* there was no progress in HW */
        goto done;
    }
    feed->prev_hw_completed_desc = hw_desc;
#else
    if(hw_desc != feed->prev_hw_finished_desc) {
        feed->false_finished = false;
        feed->finished = false;
    }
#endif
    BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update: %p hw_desc:%p last_desc:%p %u", (void *)feed, (void *)hw_desc, (void *)feed->last_desc, (unsigned)(BFIFO_READ_LEFT(&feed->descFifo))));
    BFIFO_STATUS(BDBG_MSG_TRACE,"BPVRlib_Feed_Priv_Update: fifo", &feed->descFifo);
#if !BXPT_HAS_MULTICHANNEL_PLAYBACK
    if(feed->false_finished) {
        BDBG_MSG(("BPVRlib_Feed_Priv_Update: %p false finished desc:%p .. %p (%u)", (void *)feed, (void *)hw_desc, (void *)BFIFO_READ(&feed->descFifo), BFIFO_READ_PEEK(&feed->descFifo)));
        goto done;
    }
#endif
    if(hw_desc==feed->last_desc) { /* if we completed last descriptor test whether playback completed */
#if !BXPT_HAS_MULTICHANNEL_PLAYBACK
        BXPT_Playback_ChannelStatus status;
        rc = BXPT_Playback_GetChannelStatus(feed->config.xptPlayHandle, &status);
        if(rc!=BERR_SUCCESS) {
            rc=BERR_TRACE(rc);
        }
        BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update: %#lx hw_desc:%#lx %s", (unsigned long)feed, (unsigned long)hw_desc, status.Finished?"FINISHED":""));
        if(status.Finished) { /* last descriptor was completed */
            feed->finished = true;
            feed->prev_hw_finished_desc = hw_desc;
#else /* !BXPT_HAS_MULTICHANNEL_PLAYBACK */
        {
#endif /* !BXPT_HAS_MULTICHANNEL_PLAYBACK */
            hw_complete = BFIFO_READ_LEFT(&feed->descFifo);
#if B_PVR_LIB_FEED_SAVE_AFTER
            {
                unsigned i;
                struct B_PVRlib_DescrFifo descFifo = feed->descFifo; /* do a copy, so we don't interfere with actual FIFO */
                for(i=0; BFIFO_READ_PEEK(&descFifo)!=0;i++) {
                    const BXPT_PvrDescriptor *desc = BFIFO_READ(&descFifo);
                    b_pvr_feed_save_desc(feed, &feed->save.after, desc);
                    BFIFO_READ_COMMIT(&descFifo, feed->descScale);
                }
                BDBG_ASSERT(i==hw_complete/feed->descScale);
            }
#endif /* B_PVR_LIB_FEED_SAVE_AFTER */
            BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update: %#lx recycle all(%u) data from FIFO", (unsigned long)feed, hw_complete));
            feed->ncompleted += hw_complete/feed->descScale;
            /* update read pointer to walk through all descriptors */
            nread_desc = BFIFO_READ_PEEK(&feed->descFifo);
            if(nread_desc) {
                BDBG_ASSERT(nread_desc <= hw_complete);
                hw_complete -= nread_desc;
                BFIFO_READ_COMMIT(&feed->descFifo, nread_desc);
            } /* we could have a wrap */
            nread_desc = BFIFO_READ_PEEK(&feed->descFifo);
            if(nread_desc) {
                BFIFO_READ_COMMIT(&feed->descFifo, nread_desc);
            }
            BDBG_ASSERT(nread_desc == hw_complete); /* number of reclaimed descriptors shoild be the same as number of queued descriptors */
            feed->active = false;
            goto done;
        }
        /* last descriptor still in use, proceed as usual */
    }
    if(hw_desc >= feed->desc && hw_desc < (feed->desc + feed->descScale * (feed->config.numDesc + B_PVRLIB_BOUNCE_BUF_SIZE))) {
        const BXPT_PvrDescriptor *desc;

        desc = BFIFO_READ(&feed->descFifo);
        nread_desc = BFIFO_READ_PEEK(&feed->descFifo);

        if(hw_desc < desc) { /* |===H---R===| HW descriptor is after wraparound, consume all 'peek'ed data */
            BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update: %#lx [wrap] hw_desc:%#lx desc:%#lx nread_desc:%d", (unsigned long)feed, (unsigned long)hw_desc, (unsigned long)desc, nread_desc));
#if B_PVR_LIB_FEED_SAVE_AFTER
            {
                unsigned i;
                for(i=0;i<nread_desc;i++) {
                    b_pvr_feed_save_desc(feed, &feed->save.after, &desc[i]);
                }
            }
#endif /* B_PVR_LIB_FEED_SAVE_AFTER */
            feed->ncompleted += nread_desc/feed->descScale;
            BFIFO_READ_COMMIT(&feed->descFifo, nread_desc);
            nread_desc = BFIFO_READ_PEEK(&feed->descFifo);
            desc = BFIFO_READ(&feed->descFifo);
        }
        BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update: %#lx hw_desc:%#lx desc:%#lx nread_desc:%d", (unsigned long)feed, (unsigned long)hw_desc, (unsigned long)desc, nread_desc));
        if(hw_desc>=desc) { /* |-R===H-----| */
            hw_complete = hw_desc - desc;
            BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update: %#lx off:%d nread_desc:%d", (unsigned long)feed, hw_complete, nread_desc));
            if(hw_complete<=nread_desc) {
                BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update: %#lx completed:%u", (unsigned long)feed, (unsigned)hw_complete));
#if B_PVR_LIB_FEED_SAVE_AFTER
                {
                    unsigned i;
                    for(i=0;i<hw_complete;i++) {
                        b_pvr_feed_save_data_offset(feed, &feed->save.after, &desc[i]);
                    }
                }
#endif /* B_PVR_LIB_FEED_SAVE_AFTER */
                feed->ncompleted += hw_complete/feed->descScale;
                BFIFO_READ_COMMIT(&feed->descFifo, hw_complete);
            } else {
                BDBG_ERR(("BPVRlib_Feed_Priv_Update: %p underflow %u .. %u(%p..%p[%p..%p])", (void *)feed, (unsigned)hw_complete, (unsigned)nread_desc, (void *)desc, (void *)hw_desc, (void *)feed->desc, (void *)(feed->desc+feed->config.numDesc*feed->descScale)));
                goto error;
            }
        } else {
            BDBG_ERR(("BPVRlib_Feed_Priv_Update: %p descriptor out of sequence %p .. %p [%p..%p]", (void *)feed, (void *)desc, (void *)hw_desc, (void *)feed->desc, (void *)(feed->desc+feed->config.numDesc*feed->descScale)));
            goto error;
        }
    } else {
        BDBG_ERR(("BPVRlib_Feed_Priv_Update: %p descriptor out of range %p .. %p .. %p", (void *)feed, (void *)feed->desc, (void *)hw_desc, (void *)(feed->desc+feed->config.numDesc*feed->descScale)));
        goto error;
    }

error:
done:
    BFIFO_STATUS(BDBG_MSG_TRACE,"BPVRlib_Feed_Priv_Update: fifo", &feed->descFifo);
    BDBG_MSG_TRACE(("BPVRlib_Feed_Priv_Update<: %p ncompleted:%u", (void *)feed, (unsigned)feed->ncompleted));
    return;
}

BERR_Code
BPVRlib_Feed_AddOffsetEntries(BPVRlib_Feed_Handle feed, const BPVRlib_Feed_OffsetEntry *entries, size_t count, size_t *nconsumed)
{
    BDBG_MSG_TRACE(("BPVRlib_Feed_AddOffsetEntries: %p entries %p:%u", (void *)feed, (void *)entries, (unsigned)count));
    BDBG_OBJECT_ASSERT(feed, BPVRlib_Feed);

    BDBG_ASSERT(entries);
    BDBG_ASSERT(nconsumed);

    BPVRlib_Feed_Priv_Update(feed);
    *nconsumed = BPVRlib_Feed_Priv_AddOffsetEntries(feed, entries, count, true, false);
    return BERR_SUCCESS;
}

BERR_Code
BPVRlib_Feed_AddExtendedOffsetEntries(BPVRlib_Feed_Handle feed, const BPVRlib_Feed_ExtendedOffsetEntry *entries, size_t count, size_t *nconsumed)
{
    BDBG_MSG_TRACE(("BPVRlib_Feed_AddExtendedOffsetEntries: %p entries %p:%u", (void *)feed, (void *)entries, (unsigned)count));

    BDBG_OBJECT_ASSERT(feed, BPVRlib_Feed);

    BDBG_ASSERT(entries);
    BDBG_ASSERT(nconsumed);
    if(!feed->config.useExtndedDesc) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BPVRlib_Feed_Priv_Update(feed);
    *nconsumed = BPVRlib_Feed_Priv_AddOffsetEntries(feed, (void*)entries, count, true, true);
    return BERR_SUCCESS;
}

BERR_Code
BPVRlib_Feed_AddEntries(BPVRlib_Feed_Handle feed, const BPVRlib_Feed_Entry *entries /* pointer to array of entries */ , size_t nentries /* number of entries in the array */ , size_t *nconsumed)
{
    unsigned off;
    BPVRlib_Feed_OffsetEntry *offset_entries = feed->offset_entries;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_MSG_TRACE(("BPVRlib_Feed_AddEntries>: %p entries %p:%u", (void *)feed, (void *)entries, (unsigned)nentries));
    BDBG_OBJECT_ASSERT(feed, BPVRlib_Feed);
    BDBG_ASSERT(entries);
    BDBG_ASSERT(nconsumed);
    BDBG_ASSERT(feed->config.bufferBlock);

    *nconsumed=0;
    BPVRlib_Feed_Priv_Update(feed);

    for(off=0;;) {
        unsigned i;
        unsigned added;
        for(i=0;i<sizeof(feed->offset_entries)/sizeof(*feed->offset_entries) && off<nentries;off++,i++) {
            offset_entries[i].len = entries[off].len;
            offset_entries[i].offset = feed->databuf_offset + (unsigned)((uint8_t*)entries[off].addr - feed->databuf_ptr);
        }
        if(i==0) {
            break;
        }
        added = BPVRlib_Feed_Priv_AddOffsetEntries(feed, offset_entries, i, off==nentries, false);
        *nconsumed+=added;
        if(added!=i) {
            break;
        }
    }
    BDBG_MSG_TRACE(("BPVRlib_Feed_AddEntries<: %p entries %p:%u -> %u", (void *)feed, (void *)entries, (unsigned)nentries, (unsigned)*nconsumed));
    return rc;
}

BERR_Code
BPVRlib_Feed_GetCompleted(BPVRlib_Feed_Handle feed, size_t *ncompleted)
{
    BDBG_MSG_TRACE(("BPVRlib_Feed_GetCompleted>: %p", (void *)feed));

    BDBG_OBJECT_ASSERT(feed, BPVRlib_Feed);
    BDBG_ASSERT(ncompleted);
    BPVRlib_Feed_Priv_Update(feed);
    *ncompleted = feed->ncompleted;
    feed->ncompleted = 0;

    BDBG_MSG_TRACE(("BPVRlib_Feed_GetCompleted<: %p -> %u", (void *)feed, (unsigned)*ncompleted));
    return BERR_SUCCESS;
}

void
BPVRlib_Feed_GetStatus(BPVRlib_Feed_Handle feed, BPVRlib_Feed_Status *status)
{
    size_t freeDesc;
    BDBG_OBJECT_ASSERT(feed, BPVRlib_Feed);
    BDBG_ASSERT(status);

    BPVRlib_Feed_Priv_Update(feed);
    freeDesc =  BFIFO_WRITE_LEFT(&feed->descFifo);
    freeDesc /= feed->descScale;
#if !BXPT_HAS_MULTICHANNEL_PLAYBACK
    if(feed->false_finished) {
        if(freeDesc>0) {
            BDBG_MSG(("BPVRlib_Feed_GetStatus: %p correcting number of avaliable descriptors %u", (void *)feed, (unsigned)freeDesc));
            freeDesc --;
        }
    }
#endif
    if(freeDesc>0) {
        freeDesc --;
    }
    status->freeDesc = freeDesc;
    BDBG_MSG_TRACE(("BPVRlib_Feed_GetStatus: %p freeDesc:%u", (void *)feed, (unsigned)freeDesc));
    return;
}
