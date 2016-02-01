/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nexus_transport_module.h"
#include "nexus_playpump_impl.h"

BDBG_MODULE(nexus_playpump_priv);

#define B_PLAYPUMP_CAPTURE_DATA 0
/* maximum size of playback chunk */
/* should be less then BATOM_VEC_MAX_SIZE */
#define B_PLAY_CHUNK_SIZE_MAX   (60*1024)
#define B_PLAY_MAX_HW_QUEUE 16 /* max number of elements queued in the HW */

#if B_PLAYPUMP_CAPTURE_DATA 
#include <stdio.h>

static void b_playpump_p_capture_data(NEXUS_PlaypumpHandle pump, const char *name, const void *addr, size_t len)
{
    static FILE *fp = NULL;

    if(fp==NULL) {
        fp = fopen(name, "w+b");
    }
    if(fp) {
        unsigned char header[12];

        header[0] = ((uint32_t)addr)>>24;
        header[1] = ((uint32_t)addr)>>16;
        header[2] = ((uint32_t)addr)>>8;
        header[3] = ((uint32_t)addr);

        header[4] = ((uint32_t)len)>>24;
        header[5] = ((uint32_t)len)>>16;
        header[6] = ((uint32_t)len)>>8;
        header[7] = ((uint32_t)len);

        header[8] = ((uint32_t)pump)>>24;
        header[9] = ((uint32_t)pump)>>16;
        header[10] = ((uint32_t)pump)>>8;
        header[11] = ((uint32_t)pump);
        fwrite(header, 1, sizeof(header), fp);
        fwrite(addr, 1, len, fp);
        fflush(fp);
    }
}

#else /* B_PLAYPUMP_CAPTURE_DATA  */
#define b_playpump_p_capture_data(pump, name, addr, length)
#endif

/* send next chunk to the playback channel */
bool
b_play_next(NEXUS_PlaypumpHandle p)
{
    BERR_Code rc;
    struct bpvr_queue_item *item;
    size_t receiver_space;
    BPVRlib_Feed_OffsetEntry entry;
    size_t nentries;
    bool active=false;

restart:
    if(p->state.queued_in_hw>=B_PLAY_MAX_HW_QUEUE) {
        return true;
    }
    for(;;) {
#if B_HAS_MEDIA
        if (p->state.packetizer==b_play_packetizer_media) {
            if(b_pump_demux_is_congested(p->demux)) {
                return true;
            }
        } 
#endif
        if (BFIFO_READ_PEEK(&p->pendingFifo)==0) {
            if(p->state.queued_in_hw==0) {
                BDBG_MSG(("underflow of the playback buffer"));
            }
            return active;
        }

        active = true;

        item = BFIFO_READ(&p->pendingFifo);
        if(item == p->state.last_item) {
            p->state.last_item = NULL; /* clear last_item that might go into the HW */
        }
        if(p->state.current.length>0) {
            break;
        }

        if (item->desc.length==0) {
            /* no payload in a chunk, recycling it */
            BFIFO_READ_COMMIT(&p->pendingFifo, 1);
#if B_HAS_MEDIA
            if (p->state.packetizer==b_play_packetizer_media) {
                /* in media mode even empty entries shall be queued up */
                b_pump_demux_enqueue(p->demux, item);
            } else
#endif
            if (p->state.packetizer==b_play_packetizer_crypto) {
                /* in crypto mode even empty entries shall be queued up */
                b_pump_crypto_enqueue(p->crypto, item);
            } else {
                /* don't send and not release entry without payload */
            }
        } else {
            /* bingo!, chunk with payload, handle it */
            if(p->settings.mode==NEXUS_PlaypumpMode_eSegment) {
                BDBG_MSG_FLOW(("playpump %#lx: chunk %#lx:%u left %u", (unsigned long)p, (unsigned long)item->desc.addr, (unsigned)item->desc.length, (unsigned)p->state.segment_left));
                if (p->state.segment_left==0) {
                    /* we must see new segment here, validate chunk first */
                    NEXUS_PlaypumpSegment desc;
                    BKNI_Memcpy(&desc, item->desc.addr, sizeof(desc));/* since itam->addr might be not 32 bit alligned, copy it first */
                    if (item->desc.length < sizeof(desc) || desc.signature != NEXUS_PLAYPUMP_SEGMENT_SIGNATURE || desc.length < sizeof(desc)) {
                        BDBG_WRN(("playpump %p: invalid segment at %p (%u:%#x)", (void *)p, (void *)item->desc.addr, (unsigned)item->desc.length, desc.signature));
                        goto remove_inalid_chunk;
                    }
                    BDBG_MSG_FLOW(("playpump %p: segment %p:%u", (void *)p, (void *)desc.offset, (unsigned)desc.length));
                    p->state.current.length = item->desc.length - sizeof(desc);
                    p->state.current.addr = (uint8_t *)item->desc.addr + sizeof(desc);
                    p->state.segment_left = desc.length - sizeof(desc);
#if B_HAS_MEDIA
                    if (p->state.packetizer==b_play_packetizer_media) {
                        b_pump_demux_descriptor(p->demux, &desc);
                    }
#endif
                    if (p->state.current.length==0) {
                        /* no payload left in a chunk, recycling it */
                        BFIFO_READ_COMMIT(&p->pendingFifo, 1);
#if B_HAS_MEDIA
                        if (p->state.packetizer==b_play_packetizer_media) {
                            /* in media mode even empty entries shall be queued up */
                            b_pump_demux_enqueue(p->demux, item);
                        } else
#endif
                        if (p->state.packetizer==b_play_packetizer_crypto) {
                            /* in crypto mode even empty entries shall be queued up */
                            b_pump_crypto_enqueue(p->crypto, item);
                        } else {
                            BFIFO_READ_COMMIT(&p->activeFifo, 1);
                        }
                    }
                } else {
                    p->state.current = item->desc;
                }
                if (p->state.segment_left < p->state.current.length ) {
                    BDBG_WRN(("playpump %#lx: unexpected out of sequence data at %#lx %u:%u, aborting", (unsigned long)p, (unsigned long)p->state.current.addr, (unsigned)p->state.current.length, (unsigned)p->state.segment_left));
remove_inalid_chunk:
                    /* try to recover here */
                    BFIFO_READ_COMMIT(&p->pendingFifo, 1);
#if B_HAS_MEDIA
                    if (p->state.packetizer==b_play_packetizer_media) {
                        /* in media mode even empty entries shall be queued up */
                        b_pump_demux_enqueue(p->demux, item);
                    } else
#endif
                    if (p->state.packetizer==b_play_packetizer_crypto) {
                        /* in crypto mode even empty entries shall be queued up */
                        b_pump_crypto_enqueue(p->crypto, item);
                    } else {
                        if(p->state.queued_in_hw==0) {
                            BFIFO_READ_COMMIT(&p->fifo, p->state.current.length);
                            BFIFO_READ_COMMIT(&p->activeFifo, 1);
                        } else {
                            return active; /* waiting for HW to consume all entries and only then consume empty entry */
                        }
                    }
                    p->state.current.length = 0;
                    continue;
                }
                p->state.segment_left -= p->state.current.length;
            } else {
                p->state.current = item->desc;
                break;
            }
        }
    }
    p->state.active.addr = p->state.current.addr;
    if (p->state.packetizer==b_play_packetizer_media) {
        receiver_space = B_PLAY_CHUNK_SIZE_MAX;
    } else {
        receiver_space = p->state.current.length;
    }
    if (p->state.current.length > receiver_space) {
        p->state.active.length = receiver_space;
        /* keep chunk in the queue */
        p->state.current.length -= receiver_space;
        p->state.current.addr = (uint8_t *)p->state.current.addr + receiver_space;
    } else {
        p->state.active.length = p->state.current.length;
        p->state.current.length = 0; /* force to load new entry from queue */
    }
    BDBG_MSG_FLOW(("sending chunk %#x(%#x:%u) %#x(%d)[%#x]", item, item->desc.addr, item->desc.length, (unsigned)p->state.active.addr, p->state.active.length, p->state.current.length, (unsigned)BFIFO_READ(&p->fifo)));
    p->state.bytes_played += p->state.active.length;

    /* remove entry before sending it to playback */
    if (p->state.current.length==0) { /* chunk is completed */
        BFIFO_READ_COMMIT(&p->pendingFifo, 1); /* remove chunk from pending fifo */
#if B_HAS_MEDIA
        if (p->state.packetizer==b_play_packetizer_media) {
            b_pump_demux_enqueue(p->demux, item);
        } else
#endif
        if (p->state.packetizer==b_play_packetizer_crypto) {
            b_pump_crypto_enqueue(p->crypto, item);
        }
    }

#if B_HAS_MEDIA
    if (p->state.packetizer==b_play_packetizer_media) {
        if(!b_pump_demux_feed(p->demux, item, p->state.active.addr, p->state.active.length)) {
            BDBG_WRN(("reverting aborted media write"));
            p->state.current.length += p->state.active.length;
            p->state.current.addr = p->state.active.addr;
        } else if (b_pump_demux_is_empty(p->demux)) {
            /* no data accumulated in the playback HW */
            /* we need to send more data into the packetizer */
            goto restart;
        }
    } else
#endif /* B_HAS_MEDIA */
    if (p->state.packetizer==b_play_packetizer_crypto) {
        if(!b_pump_crypto_feed(p->crypto, item, p->state.active.addr, p->state.active.length)) {
            BDBG_WRN(("reverting aborted crypto write"));
            p->state.current.length += p->state.active.length;
            p->state.current.addr = p->state.active.addr;
        } else if (b_pump_crypto_is_empty(p->crypto)) {
            /* no data accumulated in the playback HW */
            /* we need to send more data into the packetizer */
            goto restart;
        }
    } else {
        /* flush cache and convert address */
        entry.len = p->state.active.length;
        entry.offset = NEXUS_AddrToOffset(p->state.active.addr);
        if ( false == p->settings.dataNotCpuAccessible ) {
            NEXUS_FlushCache(p->state.active.addr, p->state.active.length);
        }
        rc = BPVRlib_Feed_AddOffsetEntries(p->play_feed, &entry, 1, &nentries);
        if (rc!=BERR_SUCCESS) {
            BDBG_ERR(("BPVRlib_Feed_AddOffsetEntries: returned error %#x, ignored", rc));
        } else {
            BDBG_ASSERT(nentries==1);
            p->state.queued_in_hw++;
            BDBG_MSG_FLOW(("queued %u:%u", p->state.queued_in_hw, B_PLAY_MAX_HW_QUEUE));
            if(p->state.current.length == 0) {
                goto restart;
            }
        }
    }
    return active;
}

void
b_playpump_p_reset(NEXUS_PlaypumpHandle p)
{

#if B_SAVE_PLAYPUMP
    b_playpump_p_data_flush();
#endif

    BFIFO_INIT(&p->fifo, p->buf, p->openSettings.fifoSize);

    BFIFO_INIT(&p->activeFifo, p->item_mem, p->openSettings.numDescriptors);
    BFIFO_INIT(&p->pendingFifo, p->item_mem, p->openSettings.numDescriptors);
    p->state.active.length = 0;
    p->state.current.length = 0;
    p->state.segment_left = 0;
    p->state.queued_in_hw = 0;
    p->state.last_item = NULL;
    return;
}

static BERR_Code
b_playpump_bound(NEXUS_PlaypumpHandle p, const NEXUS_PlaypumpScatterGatherDescriptor *pSgDesc)
{
    if (p->boundsHeapAddr && p->boundsHeapSize) {
        if (pSgDesc->addr < p->boundsHeapAddr || 
            ((uint8_t *)pSgDesc->addr+pSgDesc->length > (uint8_t *)p->boundsHeapAddr+p->boundsHeapSize))
        {
            /*BDBG_WRN(("%#lx: descriptor bound violation %#lx:%#x %#lx:%#x", p,
                pSgDesc->addr, pSgDesc->length, p->boundsHeapAddr, p->boundsHeapSize));*/
            return BERR_INVALID_PARAMETER;
        }
    }
    return BERR_SUCCESS;
}

BERR_Code
b_playpump_p_add_request(NEXUS_PlaypumpHandle p, size_t skip, size_t amount_used, const NEXUS_PlaypumpScatterGatherDescriptor *pSgDesc)
{
    struct bpvr_queue_item *item;
    unsigned nFree;

    nFree = BFIFO_WRITE_PEEK(&p->activeFifo);
    if (nFree==0) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    if (pSgDesc) {
        NEXUS_ASSERT_FIELD(NEXUS_PlaypumpDesc, addr,   NEXUS_PlaypumpScatterGatherDescriptor, addr);
        NEXUS_ASSERT_FIELD(NEXUS_PlaypumpDesc, length, NEXUS_PlaypumpScatterGatherDescriptor, length);
        NEXUS_ASSERT_STRUCTURE(NEXUS_PlaypumpDesc, NEXUS_PlaypumpScatterGatherDescriptor);
    
        for(; amount_used>=sizeof(*pSgDesc); amount_used-=sizeof(*pSgDesc),pSgDesc++) {
            if (b_playpump_bound(p, pSgDesc)!=BERR_SUCCESS) {
                continue;
            }
            item = BFIFO_WRITE(&p->activeFifo);
            BKNI_Memcpy(&item->desc, pSgDesc, sizeof(*pSgDesc)); /* pSgDesc might not be 32-bit alligned */
            item->sg = true;
            item->skip = 0;
#if B_HAS_MEDIA
            item->ref_cnt = 0;
#endif
            BFIFO_WRITE_COMMIT(&p->activeFifo,1);
            BFIFO_WRITE_COMMIT(&p->pendingFifo,1);
            BDBG_MSG_FLOW(("queuing up chunk %#x %#x:%u:%u", (unsigned)item, (unsigned)item->desc.addr, item->skip, item->desc.length));
        }
        BDBG_ASSERT(amount_used==0);
    }
    else {
        switch(p->settings.mode) {
        case NEXUS_PlaypumpMode_eFifo:
        case NEXUS_PlaypumpMode_eSegment:
            BDBG_ASSERT(p->state.last_addr == BFIFO_WRITE(&p->fifo));
#if B_SAVE_PLAYPUMP
            b_playpump_p_data(BFIFO_WRITE(&p->fifo)+skip, amount_used);
#endif
            if(skip==0 && amount_used>0 && p->state.packetizer==b_play_packetizer_none && p->settings.mode == NEXUS_PlaypumpMode_eFifo  && p->state.last_item!=NULL) {
                size_t threshold;
                threshold = (2*p->openSettings.fifoSize)/ p->openSettings.numDescriptors; /* limit size of the merged chunk */
                item = p->state.last_item;
                if(item->desc.length < threshold && (uint8_t*)item->desc.addr + item->desc.length == BFIFO_WRITE(&p->fifo)) {
                    BDBG_MSG_FLOW(("merged up chunk %#x %#x:%u:%u->%u", (unsigned)item, (unsigned)item->desc.addr, item->skip, item->desc.length, item->desc.length+amount_used));
                    item->desc.length += amount_used;
                    BFIFO_WRITE_COMMIT(&p->fifo, amount_used);
                    break;
                }
            }
            item = BFIFO_WRITE(&p->activeFifo);
            BDBG_ASSERT(item);
            item->desc.addr = BFIFO_WRITE(&p->fifo) + skip;
            if (skip+amount_used>0) {
                BFIFO_WRITE_COMMIT(&p->fifo, skip+amount_used);
            }

            item->desc.length = amount_used;
            b_playpump_p_capture_data(p, "add_request.dat", item->desc.addr, item->desc.length);

            item->sg = false;        
            item->skip = skip;
            item->ref_cnt = 0;
            p->state.last_item = item;
            BFIFO_WRITE_COMMIT(&p->activeFifo,1);
            BFIFO_WRITE_COMMIT(&p->pendingFifo,1);
            BDBG_MSG_FLOW(("queuing up chunk %#x %#x:%u:%u", (unsigned)item, (unsigned)item->desc.addr, item->skip, item->desc.length));
            break;
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    
    if(p->state.active.length==0) {
        /* initiate new transaction */
        b_play_next(p);
    }
    return 0;
}

void
b_playpump_p_do_read_callback(NEXUS_PlaypumpHandle p)
{
    /* non-blocking get of buffer space */
    if (BFIFO_WRITE_PEEK(&p->fifo)==0 && p->buf) {
        /* If it fails, or if there's no space available, don't send a callback */
        BDBG_MSG_FLOW(("no space available"));
        return;
    }
    if (BFIFO_WRITE_PEEK(&p->activeFifo)==0) {
        BDBG_MSG_FLOW(("no chunks avaliable"));
        return;
    }

    /* call back to the user with the space available */
    NEXUS_TaskCallback_Fire(p->dataCallback);
}


/* this function called when playback completed single transaction*/
void
b_playpump_p_xpt_event(void *p_)
{
    NEXUS_PlaypumpHandle p = p_;
    BERR_Code rc;
    size_t complete;

    if(p->state.muxInput) {
        return;
    }

#if B_HAS_MEDIA
    if(p->state.packetizer==b_play_packetizer_media) {
        b_pump_demux_reclaim(p->demux);
        return;
    } else
#endif
    if(p->state.packetizer==b_play_packetizer_crypto) {
        b_pump_crypto_reclaim(p->crypto);
        return;
    }

    rc = BPVRlib_Feed_GetCompleted(p->play_feed, &complete);
    if (rc!=BERR_SUCCESS) {
        BDBG_ERR(("BPVRlib_Feed_GetCompleted returned %d, complete %u", rc, (unsigned)complete));
        return;
    }
    if (complete==0) {
        BDBG_MSG_FLOW(("BPVRlib_Feed_GetCompleted returned %d, complete %u", rc, (unsigned)complete));
        return ;
    }
    BDBG_MSG_FLOW(("chunk completed %#x:%u (%u)", p->state.active.addr, p->state.active.length, complete));
    if (complete > p->state.queued_in_hw) {
        BDBG_ERR(("play event without active transaction"));
        return;
    }
    p->state.queued_in_hw -= complete;
    for(;complete>0;) {
        struct bpvr_queue_item *item = BFIFO_READ(&p->activeFifo);
        BDBG_MSG_FLOW(("recycling %#lx %u:%u %#lx", (unsigned)item->desc.addr, item->skip, item->desc.length, item->skip+(unsigned long)BFIFO_READ(&p->fifo)));
        if(item->desc.length>0) {  /* don't count "empty" transactions */
            complete--;
        }
        if(!item->sg) {
            size_t recycle = item->skip + item->desc.length;
            BFIFO_READ_COMMIT(&p->fifo, recycle);
        }
        BFIFO_READ_COMMIT(&p->activeFifo, 1); /* recycle chunk */
    }
    p->state.active.length = 0;
    b_play_next(p); /* send more data to the playback */
    b_playpump_p_do_read_callback(p_); /* try to get more data into the playback buffer */
    return;
}

void b_playpump_p_throttle_timer(void *play)
{
    NEXUS_PlaypumpHandle p = play;
    p->throttle_timer = NULL;
#if B_HAS_MEDIA
    if(p->state.packetizer==b_play_packetizer_media) {
        if (b_pump_demux_is_empty(p->demux)) {
            BDBG_MSG_FLOW(("timer event"));
            if(b_pump_demux_is_congested(p->demux) || !b_play_next(p)) {
                b_pump_demux_advance(p->demux);
            }
        }
    } else
#endif
    if(p->state.packetizer==b_play_packetizer_crypto) {
        if (b_pump_crypto_is_empty(p->crypto)) {
            BDBG_MSG_FLOW(("timer event"));
            b_play_next(p);
        }
    } else if(p->state.active.length==0) {
        BDBG_MSG_FLOW(("timer event"));
        /* initiate new transaction */
        b_play_next(p);
    }
    p->throttle_timer = NEXUS_ScheduleTimer(B_THROTTLE_TIMEOUT, b_playpump_p_throttle_timer, p);  /* schedulle timer after 30 ms */
}

#if B_HAS_CRYPTO
BERR_Code
b_playpump_p_set_encryption(NEXUS_PlaypumpHandle play, const bencryption_params *encryption)
{
    if(play->crypto) {
        play->crypto_remainder = 0;
        bcrypto_close(play->crypto);
        play->crypto  = NULL;
    }
    if (encryption->type != bencryption_type_none) {
        if (encryption->use_cps) {
            bstream_set_encryption(&play->stream,encryption);
        } else {
            bcrypto_settings settings;

            BDBG_MSG(("Encryption is set for this playpump "));
            if (encryption->key_length != 64 && encryption->key_length != 128 ) {
                BDBG_ERR(("Record encryption supports 64/128 bit key only (length is %d)",encryption->key_length));
                return BERR_TRACE(NEXUS_UNKNOWN);
            }

            bcrypto_settings_init(&settings);
            settings.hardware_ctrl = bcrypto_hardware_m2m;
            settings.encryption.type = encryption->type;
            memcpy(settings.encryption.key, encryption->key, encryption->key_length / 8);
            settings.encryption.key_length = encryption->key_length;
            settings.encryption.blockmode = bcrypto_blockmode_ecb;
            settings.operation = bcrypto_operation_decrypt;
            if (b_is_dss_mode(play->stream.storage.original_mpeg.mpeg_type)) {
                settings.data_format = bcrypto_data_format_dss;
                play->crypto_packet_size = 130;
            } else {
                settings.data_format = bcrypto_data_format_ts;
                play->crypto_packet_size = 188;
            }
            play->crypto = bcrypto_open(&settings);
            play->crypto_remainder = 0;
        }
    } else {
        if (encryption->use_cps) {
            bstream_set_encryption(&play->stream,encryption);
        }
    }
    return 0;
}
#endif

void
NEXUS_P_Playpump_DescAvail_isr(void *p, int unused)
{
    NEXUS_PlaypumpHandle play = p;
    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(play, NEXUS_Playpump);
    if(!play->state.muxInput) {
        BKNI_SetEvent_isr(play->descEvent);
    }
    return;
}



#if B_SAVE_PLAYPUMP
#include <stdio.h>
static FILE *playpump_data=NULL;

void
b_playpump_p_data(const void *data, size_t len)
{
    if(playpump_data==NULL) {
        char filename[20];
        static int filename_count = 0;
        sprintf(filename, "pump_data%d.mpg", filename_count++);
        playpump_data = fopen(filename, "wb");
    }
    BDBG_ASSERT(playpump_data);
    fwrite(data, 1, len, playpump_data);
    return;
}

void
b_playpump_p_data_flush(void)
{
    if (playpump_data) {
        fclose(playpump_data);
        playpump_data=NULL;
    }
    return;
}
#endif /* B_SAVE_PLAYPUMP */

void
b_pid_map_init(b_pid_map *map, uint16_t base)
{
    BDBG_ASSERT(map);
    map->base_pid = base;
    BKNI_Memset(map->map, 0, sizeof(map->map));
    return;
}

uint16_t
b_pid_map_alloc(b_pid_map *map)
{
    unsigned i;
    BDBG_ASSERT(map);
    for(i=0;i<sizeof(map->map)/sizeof(*map->map);i++) {
        if(!map->map[i]) {
            map->map[i]=true;
            return i+map->base_pid;
        }
    }
    return 0;
}

void
b_pid_map_free(b_pid_map *map, uint16_t pid)
{
    BDBG_ASSERT(map);
    if(pid<map->base_pid || pid >= map->base_pid + sizeof(map->map)/sizeof(*map->map)) {
        BDBG_ERR(("b_pid_map_free: %#lx %#x freeing invalid pid", (unsigned long)map, (unsigned)pid));
        return;
    }
    map->map[pid-map->base_pid]=false;
    return;
}

NEXUS_Error 
NEXUS_Playpump_AddExtendedOffsetEntries_priv(NEXUS_PlaypumpHandle playpump, const BPVRlib_Feed_ExtendedOffsetEntry *entries, size_t count, size_t *nconsumed)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(playpump, NEXUS_Playpump);
    return BPVRlib_Feed_AddExtendedOffsetEntries(playpump->play_feed, entries, count, nconsumed);
}

NEXUS_Error 
NEXUS_Playpump_GetCompleted_priv(NEXUS_PlaypumpHandle playpump, size_t *ncompleted)
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(playpump, NEXUS_Playpump);
    return BPVRlib_Feed_GetCompleted(playpump->play_feed, ncompleted);
}

