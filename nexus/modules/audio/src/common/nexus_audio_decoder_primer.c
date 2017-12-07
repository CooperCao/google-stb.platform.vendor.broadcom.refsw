/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_audio_module.h"
#include "priv/nexus_pid_channel_priv.h"

#if BAPE_DSP_SUPPORT
#include "bdsp.h"
#endif

BDBG_MODULE(nexus_audio_decoder_primer);

#ifdef DEBUG_PRIMER
#define BDBG_MSG_TRACE(X) BDBG_MSG(X) 
#else
#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */
#endif

#define TIMER_INTERVAL 30 /* mSec */

struct NEXUS_AudioDecoderPrimer
{
    NEXUS_OBJECT(NEXUS_AudioDecoderPrimer);
    NEXUS_AudioDecoderHandle audioDecoder; /* set for a decoder that's been modified which must be unmodified. */
    NEXUS_AudioDecoderStartSettings startSettings;
    NEXUS_AudioDecoderPrimerSettings settings;
    int div2ms; /* constant to convert PTS units to millisecond units. 45 for MPEG, 27000 for DSS */
    NEXUS_RaveHandle rave;
    NEXUS_TimerHandle timer;
    
    BAVC_XptContextMap cx_map;
    bool active;
    bool overflow;
    bool full;
    struct {
        BSTD_DeviceOffset base, end;
    } cdb, itb;
    BSTD_DeviceOffset sitb_read; /* shadow read register */
    bool playback; /* true for playback TSM, not playback feed */
    struct {
        unsigned cnt, itb_valid;
    } stuck;
    
    /* audio doesn't have a "GOP", but we reuse the term to stay in sync with video primer. 
    in this case, "GOP" is the set of audio frames delimited by a coded PTS. */
#define MAX_GOPS 100
    struct {
        BSTD_DeviceOffset cdb_read;
        BSTD_DeviceOffset itb_read;
        uint32_t pts;
        uint32_t pcr_offset;
    } gops[MAX_GOPS];
    unsigned next_gop; /* next_gop-1 is the last GOP seen by ITB */
    unsigned consumed_gop; /* the GOP that we've consumed up to */

    /* last base_address and pcr_offset ITB's seen */
    BSTD_DeviceOffset cdb_base_entry; /* address in cdb pointed by last base_address ITB. if 0, none seen. */
    BSTD_DeviceOffset itb_base_entry; /* address of base_address itb */
    uint32_t pcr_offset;     /* only valid if pcr_offset_set is true */
    bool pcr_offset_set;
    uint32_t last_pts;
};

struct itb_entry_t {
    uint32_t word0;
    uint32_t word1;
    uint32_t word2;
    uint32_t word3;
};
#define ITB_SIZE sizeof(struct itb_entry_t)

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_audioModuleData.internalSettings.modules.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_audioModuleData.internalSettings.modules.transport)

static void reset_primer(NEXUS_AudioDecoderPrimerHandle primer);
static void NEXUS_AudioDecoderPrimer_P_AcquireStartResources(NEXUS_AudioDecoderPrimerHandle primer);
static void NEXUS_AudioDecoderPrimer_P_ReleaseStartResources(NEXUS_AudioDecoderPrimerHandle primer);

static unsigned nexus_audiodecoder_p_cdb_depth(NEXUS_AudioDecoderPrimerHandle primer)
{
    BSTD_DeviceOffset valid, read;
    valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Valid);
    read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Read);
    if (valid != primer->cdb.base) valid++;
    if (read != primer->cdb.base) read++;
    return (valid>=read?valid-read:(valid-primer->cdb.base)+(primer->cdb.end-read)) * 100 / (primer->cdb.end-primer->cdb.base);
}

static unsigned nexus_audiodecoder_p_itb_depth(NEXUS_AudioDecoderPrimerHandle primer)
{
    BSTD_DeviceOffset valid, read;
    valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
    read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read);
    if (valid != primer->itb.base) valid++;
    if (read != primer->itb.base) read++;
    return (valid>=read?valid-read:(valid-primer->itb.base)+(primer->itb.end-read)) * 100 / (primer->itb.end-primer->itb.base);
}

#ifdef DEBUG_PRIMER
static void NEXUS_AudioDecoder_P_PrintItb2(NEXUS_AudioDecoderPrimerHandle primer, BSTD_DeviceOffset from, BSTD_DeviceOffset to)
{
    struct itb_entry_t *from_itb;
    struct itb_entry_t *to_itb;
    BERR_Code rc;
    unsigned count;

    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);

    from_itb = NEXUS_OffsetToCachedAddr(from);
    BDBG_ASSERT(from_itb);
    to_itb = NEXUS_OffsetToCachedAddr(to);
    BDBG_ASSERT(to_itb);
    BDBG_ASSERT(from_itb <= to_itb);
    NEXUS_FlushCache(from_itb, (to_itb-from_itb)*sizeof(struct itb_entry_t));
    count = 0;
    while (from_itb < to_itb) {
        BDBG_MSG(("primer: %p ITB %p: %08x %08x %08x %08x", (void*)primer, (void*)from_itb, from_itb->word0, from_itb->word1, from_itb->word2, from_itb->word3));
        from_itb++;
        if (++count == 20) break;
    }
}

static void NEXUS_AudioDecoder_P_PrintItb(NEXUS_AudioDecoderPrimerHandle primer)
{
    BSTD_DeviceOffset itb_valid, itb_read;
    itb_valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
    itb_read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read);

    if (itb_valid < itb_read) {
        BSTD_DeviceOffset itb_wrap;
        itb_wrap = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Wrap);
        BDBG_MSG(("PrintItb " BDBG_UINT64_FMT "->" BDBG_UINT64_FMT ", " BDBG_UINT64_FMT "->" BDBG_UINT64_FMT,
            BDBG_UINT64_ARG(itb_read), BDBG_UINT64_ARG(itb_wrap), BDBG_UINT64_ARG(primer->itb.base), BDBG_UINT64_ARG(itb_valid)));
        NEXUS_AudioDecoder_P_PrintItb2(primer, itb_read, itb_wrap);
        NEXUS_AudioDecoder_P_PrintItb2(primer, primer->itb.base, itb_valid);
    }
    else {
        BDBG_MSG(("PrintItb " BDBG_UINT64_FMT "->" BDBG_UINT64_FMT,
            BDBG_UINT64_ARG(itb_read), BDBG_UINT64_ARG(itb_valid)));
        NEXUS_AudioDecoder_P_PrintItb2(primer, itb_read, itb_valid);
    }
}

static void NEXUS_AudioDecoder_P_DumpRegisters(NEXUS_AudioDecoderPrimerHandle primer)
{
    BSTD_DeviceOffset valid, read;
    unsigned depth;

    valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
    read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read);
    depth = (valid>=read?valid-read:(valid-primer->itb.base)+(primer->itb.end-read)) * 100 / (primer->itb.end-primer->itb.base);
    BDBG_MSG(("ITB: valid=" BDBG_UINT64_FMT " read=" BDBG_UINT64_FMT " base=" BDBG_UINT64_FMT " end=" BDBG_UINT64_FMT " depth=%d%%",
        BDBG_UINT64_ARG(valid), BDBG_UINT64_ARG(read), BDBG_UINT64_ARG(primer->itb.base), BDBG_UINT64_ARG(primer->itb.end),
        depth));

    valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Valid);
    read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Read);
    BDBG_MSG(("CDB: valid=" BDBG_UINT64_FMT " read=" BDBG_UINT64_FMT " base=" BDBG_UINT64_FMT " end=" BDBG_UINT64_FMT " depth=%d%%",
        BDBG_UINT64_ARG(valid), BDBG_UINT64_ARG(read), BDBG_UINT64_ARG(primer->cdb.base), BDBG_UINT64_ARG(primer->cdb.end),
        nexus_audiodecoder_p_cdb_depth(primer)));
}
#endif

static void clear_next_gop(NEXUS_AudioDecoderPrimerHandle primer)
{
    BKNI_Memset(&primer->gops[primer->next_gop], 0, sizeof(primer->gops[primer->next_gop]));
    if (primer->pcr_offset_set) {
        primer->gops[primer->next_gop].pcr_offset = primer->pcr_offset;
    }
}

static void NEXUS_AudioDecoder_P_PrimerCompare(NEXUS_AudioDecoderPrimerHandle primer, uint32_t serialStc, unsigned * i, int * gop_index, int * min_diff)
{
    uint32_t stc = serialStc + primer->gops[*i].pcr_offset; /* Serial STC + offset = STC */
    int diff = primer->gops[*i].pts + primer->settings.ptsOffset - stc;

#if 0
    BDBG_MSG_TRACE(("%p: eval%d stc=%#x pts=%#x at " BDBG_UINT64_FMT "/" BDBG_UINT64_FMT,
        (void*)primer, *i, stc, primer->gops[*i].pts,
        BDBG_UINT64_ARG(primer->gops[*i].cdb_read),
        BDBG_UINT64_ARG(primer->gops[*i].itb_read)));
#endif
    if (diff <= 0) {
        if (*gop_index == -1 || diff > *min_diff) {
            *gop_index = *i;
            *min_diff = diff;
        }
    }

    *i = (*i + 1) % MAX_GOPS;
    BDBG_MSG_TRACE(("%p: eval%d stc=%#x pts=%#x diff=%d index=%d min=%d at " BDBG_UINT64_FMT "/" BDBG_UINT64_FMT,
        (void*)primer, *i, stc, primer->gops[*i].pts, diff, *gop_index, *min_diff,
        BDBG_UINT64_ARG(primer->gops[*i].cdb_read),
        BDBG_UINT64_ARG(primer->gops[*i].itb_read)));
}

static void NEXUS_AudioDecoder_P_SetReadPtr(NEXUS_AudioDecoderPrimerHandle primer)
{
    struct itb_entry_t *itb;
    BSTD_DeviceOffset itb_read;

    if (!primer->playback) {
        /* back up and write one pcr_offset ITB */
        itb_read = primer->gops[primer->consumed_gop].itb_read;
        if (itb_read - ITB_SIZE > primer->itb.base) {
            itb_read -= ITB_SIZE;
        }
        else {
            /* there's no room for the PCR_OFFSET entry without setting the WRAP pointer, etc. just wait for a little more
            data to come. */
            return;
        }
        itb=NEXUS_OffsetToCachedAddr(itb_read);
        itb->word0 = 0x22800000; /* pcr offset, marked valid */
        itb->word1 = primer->gops[primer->consumed_gop].pcr_offset;
        itb->word2 = 0;
        itb->word3 = 0;
    }

    {
        BSTD_DeviceOffset temp = primer->gops[primer->consumed_gop].cdb_read;
        BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Read, temp);

        /* ITB_READ has inclusive semantics (that is, it points to the last byte read), so a conversion is necessary. Nexus has exclusive logic. */
        temp = primer->gops[primer->consumed_gop].itb_read;
        if (temp != primer->itb.base) temp--;
        BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read, temp);
    }
}

static void NEXUS_AudioDecoder_P_PrimerSetRave(NEXUS_AudioDecoderPrimerHandle primer)
{
    uint32_t serialStc;
    int gop_index = -1;
    int min_diff = 0; /* doesn't need to be -INT_MAX, because first gop eval'd always reseeds */
    unsigned i;

    if (!primer->active) return;

    if (primer->playback) {
        /* In playback, HW pcr offset will be zero and mosaic will use SW pcr offset.
        So, we get GetStc which returns SerialStc + HW or SW pcr offset. */
        NEXUS_StcChannelStatus stcStatus;
        int rc;
        rc = NEXUS_StcChannel_GetStatus(primer->startSettings.stcChannel, &stcStatus);
        if (rc || !stcStatus.stcValid) return;
        serialStc = stcStatus.stc;
    }
    else {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_GetSerialStc_priv(primer->startSettings.stcChannel, &serialStc);
        UNLOCK_TRANSPORT();
    }

    i = primer->consumed_gop;
    /* this part handles the full case */
    if ((i == primer->next_gop) && primer->full) {
        NEXUS_AudioDecoder_P_PrimerCompare(primer, serialStc, &i, &gop_index, &min_diff);
    }
    /* this part handles the rest; notice the i == next_gop condition is
     * only allowed once above. since we are not actually updating the
     * read pointer here, full will always be full, so it can't be used
     * here to handle the one case where i == primer->next_gop and full == true,
     * otherwise it will wrap around over and over again */
    while (i != primer->next_gop) {
        NEXUS_AudioDecoder_P_PrimerCompare(primer, serialStc, &i, &gop_index, &min_diff);
    }
    
    if (gop_index == -1) {
#if 0
        BDBG_MSG_TRACE(("%p: no action. Serial STC=%08x, closest diff %5d, %d %d", (void*)primer, serialStc, min_diff, primer->consumed_gop, primer->next_gop));
#endif
        return;
    }

    primer->consumed_gop = gop_index;
    BDBG_MSG_TRACE(("%p: SetRave PTS=%08x STC=%08x (%08x %08x), diff %5d, CDB %u%% ITB %u%%", (void*)primer,
        primer->gops[primer->consumed_gop].pts, serialStc + primer->gops[primer->consumed_gop].pcr_offset, serialStc, primer->gops[primer->consumed_gop].pcr_offset, min_diff,
        nexus_audiodecoder_p_cdb_depth(primer), nexus_audiodecoder_p_itb_depth(primer)));
    if (min_diff/45 < -10000) {
        BDBG_WRN(("%p: flush primer on very late PTS: %d", (void *)primer, min_diff));
        NEXUS_AudioDecoderPrimer_Flush(primer);
        return;
    }
    /* update the rave read pointer */
    NEXUS_AudioDecoder_P_SetReadPtr(primer);

    /* update the primer cache read pointer */
    primer->consumed_gop = (primer->consumed_gop + 1) % MAX_GOPS;
    /* both read and write set full
     * no harm in setting here even for live, as it's not used in live */
    if (primer->consumed_gop != primer->next_gop)
    {
        primer->full = false;
    }
}

static void NEXUS_AudioDecoder_P_PrimerProcessItb(NEXUS_AudioDecoderPrimerHandle primer, BSTD_DeviceOffset itb_valid)
{
    struct itb_entry_t * pitb;
    struct itb_entry_t * pitb_end;
    uint8_t type;    

    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);

    pitb = NEXUS_OffsetToCachedAddr(primer->sitb_read);
    if (!pitb) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    pitb_end = NEXUS_OffsetToCachedAddr(itb_valid);
    if (!pitb_end) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    NEXUS_FlushCache (pitb, (pitb_end-pitb)*sizeof(struct itb_entry_t));

    while (pitb < pitb_end) {
        if (primer->playback && primer->full)
        {
            BDBG_MSG_TRACE(("primer cache full; no more entries processed"));
            return;
        }

        BDBG_MSG_TRACE(("%p: w0=%08x %08x %08x %08x", (void*)primer, pitb->word0, pitb->word1, pitb->word2, pitb->word3));
        /* determine entry type */
        type = pitb->word0 >> 24;
        switch(type)
        {
        case 0x22: /* pcr offset */
            if (((pitb->word0 >> 23) & 0x1) == 0) {
                BDBG_MSG(("%p: received invalid pcr offset : %#x", (void *)primer, pitb->word1));
            }
            else {
                primer->pcr_offset = pitb->word1;
                primer->pcr_offset_set = true;
            }
            break;

        case 0x21: /* pts */
            {
                uint32_t pts = pitb->word1;
                /* don't store PTS unless it meets a time threshold. this prevents the audio primer gops[] from 
                overflowing - if it overflows there will be an extended period of no sound when switching from
                 primer to audio decoder */      
                int diff = 0; /* units of milliseconds, early = positive, late = negative */
                if (primer->last_pts) {
                    diff = pts > primer->last_pts ? (pts - primer->last_pts) : -1 * (primer->last_pts - pts);
                    if (primer->div2ms == 27000) diff /= 600;
                    diff /= 45; /* convert from 50KHz to milliseconds */

                    /* 2 second early/10 second late threshold */
                    if (diff > 2000 || diff < -10000) {
                        BDBG_WRN(("%p: reset primer on PTS discontinuity: %#x, %#x, diff %d", (void *)primer, pts, primer->last_pts, diff));
                        reset_primer(primer);
                    }
                }
                
                if (primer->pcr_offset_set && primer->cdb_base_entry && (!primer->last_pts || diff > 200)) {
                    primer->last_pts = primer->gops[primer->next_gop].pts = pts;
                    primer->gops[primer->next_gop].pcr_offset = primer->pcr_offset;
                    primer->gops[primer->next_gop].cdb_read = primer->cdb_base_entry;
                    primer->gops[primer->next_gop].itb_read = primer->itb_base_entry;
                    primer->next_gop = (primer->next_gop + 1) % MAX_GOPS;
                    if (primer->next_gop == primer->consumed_gop) {
                        if (primer->playback) {
                            BDBG_MSG(("%p: primer cache full nxtGop=%d consGop=%d diff=%d" , (void *)primer,
                                primer->next_gop , primer->consumed_gop, diff ));
                            primer->full = true;
                        }
                        else
                        {
                            /* If you get overflow, there is a high chance of dropped audio data, resulting in no sound */
                            BDBG_ERR(("%p: primer cache overflow nxtGop=%d consGop=%d diff=%d" , (void *)primer,
                                primer->next_gop , primer->consumed_gop, diff ));

                            /* BDBG_ASSERT(primer->next_gop != primer->consumed_gop); */
                        }
                    }
                }
            }
            break;
#define CDB_OVERFLOW 0x10000
#define ITB_OVERFLOW 0x20000
        case 0x20: /* base_address  */
        case 0x28: /* base_address offset entry (for 40 bit addressing) */
            primer->cdb_base_entry = pitb->word1;
            if (type == 0x28) {
                primer->cdb_base_entry += primer->cdb.base;
            }
            primer->itb_base_entry = NEXUS_AddrToOffset(pitb);
            if (!primer->itb_base_entry) {
                 BERR_TRACE(NEXUS_INVALID_PARAMETER);
                 return;
            }

            if(0 != ( (CDB_OVERFLOW|ITB_OVERFLOW) & pitb->word2)){

                BDBG_ERR(("%p: %s %s RAVE signaled overflow", (void *)primer, (CDB_OVERFLOW & pitb->word2) ? "CDB":"" ,
                                                            (ITB_OVERFLOW & pitb->word2) ? "ITB":""));
                primer->overflow = true;
            }
            break;
        }
        pitb++;
    }

    /* need to ensure that we get a pcr offset if we have valid data we are processing */
    if (!primer->playback && !primer->pcr_offset_set && (itb_valid - primer->sitb_read) > 0) {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_SetPcrOffsetContextAcquireMode_priv(primer->startSettings.stcChannel);
        UNLOCK_TRANSPORT();
    }

    primer->sitb_read = NEXUS_AddrToOffset(pitb);
    if (!primer->sitb_read) {
         BERR_TRACE(NEXUS_INVALID_PARAMETER);
         return;
    }
}

static void NEXUS_AudioDecoder_P_PrimerCallback(void *context)
{
    NEXUS_AudioDecoderPrimerHandle primer = context;
    BSTD_DeviceOffset itb_valid, itb_wrap;

    primer->timer = NULL;
    
    /* convert ITB_Valid and ITB_Wrap to standard ring buffer semantics in this function. instead of pointing to the last byte of the ITB it should point
    to the first byte of the next ITB. Note that RAVE has an exception when valid == base. */
    itb_valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
    if (itb_valid != primer->itb.base) itb_valid++;

    /* fifo watchdog: if CDB fills, flush CDB and ITB */
    if (primer->playback) {
        if (itb_valid == primer->stuck.itb_valid) {
            unsigned cdb_depth = nexus_audiodecoder_p_cdb_depth(primer);
            unsigned itb_depth = nexus_audiodecoder_p_itb_depth(primer);
            if (cdb_depth > 95 || itb_depth > 95) {
                if (++primer->stuck.cnt == 16) {
                    BDBG_WRN(("flushing audio primer %p after stuck full for 500 msec: CDB %u%% ITB %u%%", (void *)primer, cdb_depth, itb_depth));
                    NEXUS_AudioDecoderPrimer_Flush(primer);
                }
            }
            else {
                primer->stuck.cnt = 0;
            }
        }
        else {
            primer->stuck.cnt = 0;
            primer->stuck.itb_valid = itb_valid;
        }
    }

    /* check for itb wrap around and handle it */
    if (itb_valid < primer->sitb_read) {
        /* for this to be true, the HW must have wrapped. */
        itb_wrap = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Wrap);
        if (!itb_wrap) goto resched;
        itb_wrap++;
        NEXUS_AudioDecoder_P_PrimerProcessItb(primer, itb_wrap);
        /* if we consumed up to wrap point then wrap */
        if (primer->sitb_read >= itb_wrap) {
            primer->sitb_read = primer->itb.base;
            NEXUS_AudioDecoder_P_PrimerProcessItb(primer, itb_valid);
        }
    }
    else {
        NEXUS_AudioDecoder_P_PrimerProcessItb(primer, itb_valid);
    }
    
    NEXUS_AudioDecoder_P_PrimerSetRave(primer);

resched:
    if (primer->active) {
        primer->timer = NEXUS_ScheduleTimer(TIMER_INTERVAL, NEXUS_AudioDecoder_P_PrimerCallback, primer);
    }
}

/* TEMP */
static NEXUS_Error NEXUS_AudioDecoder_P_GetRaveSettings(NEXUS_RaveOpenSettings *pRaveOpenSettings, const NEXUS_AudioDecoderOpenSettings *pSettings)
{
    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(pRaveOpenSettings);
    UNLOCK_TRANSPORT();
    
    BAPE_Decoder_GetDefaultCdbItbConfig(NULL /*unused*/, &pRaveOpenSettings->config);

    if ( !pSettings || pSettings->fifoSize == 0 )
    {
        /* NOTE: Don't automatically increase CDB/ITB for IP Settop internally. */
    }
    else
    {
        /* Make ITB proportional to CDB */
        pRaveOpenSettings->config.Itb.Length = 1024*((pRaveOpenSettings->config.Itb.Length/1024) * (pSettings->fifoSize/1024))/(pRaveOpenSettings->config.Cdb.Length/1024);
        BDBG_ASSERT(0 != pRaveOpenSettings->config.Itb.Length);
        pRaveOpenSettings->config.Cdb.Length = pSettings->fifoSize;
    }

    return 0;
}

NEXUS_AudioDecoderPrimerHandle NEXUS_AudioDecoderPrimer_Open( NEXUS_AudioDecoderHandle audioDecoder )
{
    /* do not set primer->audioDecoder based on open-settings only */
    return NEXUS_AudioDecoderPrimer_Create(&audioDecoder->openSettings);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioDecoderPrimer, NEXUS_AudioDecoderPrimer_Close);

NEXUS_AudioDecoderPrimerHandle NEXUS_AudioDecoderPrimer_Create( const NEXUS_AudioDecoderOpenSettings *pSettings )
{
    NEXUS_AudioDecoderPrimerHandle primer;
    NEXUS_RaveOpenSettings raveOpenSettings;
    NEXUS_Error rc = 0;

    primer = BKNI_Malloc(sizeof(*primer));
    if (!primer) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_AudioDecoderPrimer, primer);
    
    rc = NEXUS_AudioDecoder_P_GetRaveSettings(&raveOpenSettings, pSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    #if BAPE_DSP_SUPPORT
    raveOpenSettings.config.Cdb.Alignment = BDSP_ADDRESS_ALIGN_CDB;
    raveOpenSettings.config.Itb.Alignment = BDSP_ADDRESS_ALIGN_ITB;
    #endif

    LOCK_TRANSPORT();
    primer->rave = NEXUS_Rave_Open_priv(&raveOpenSettings);
    if (!primer->rave) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
    }
    UNLOCK_TRANSPORT();
    if (rc) goto error; /* already traced above */
    return primer;
    
error:
    NEXUS_AudioDecoderPrimer_Close(primer);
    return NULL;
}

static void nexus_audiodecoderprimer_replace(NEXUS_AudioDecoderPrimerHandle primer, NEXUS_AudioDecoderHandle audioDecoder)
{
    BDBG_ASSERT(!primer->audioDecoder);
    BDBG_ASSERT(!audioDecoder->primer);
    audioDecoder->primer = primer;
    primer->audioDecoder = audioDecoder;
    audioDecoder->savedRaveContext = audioDecoder->raveContext;
    audioDecoder->raveContext = primer->rave;
}

static void nexus_audiodecoderprimer_restore(NEXUS_AudioDecoderPrimerHandle primer)
{
    if (primer->audioDecoder) {
        BDBG_ASSERT(primer->audioDecoder->primer == primer);
        primer->audioDecoder->raveContext = primer->audioDecoder->savedRaveContext;
        primer->audioDecoder->savedRaveContext = NULL;
        primer->audioDecoder->primer = NULL;
        primer->audioDecoder = NULL;
    }
}

static void NEXUS_AudioDecoderPrimer_P_Finalizer( NEXUS_AudioDecoderPrimerHandle primer )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDecoderPrimer, primer);
    if (primer->active) {
        NEXUS_AudioDecoderPrimer_Stop(primer);
    }

    nexus_audiodecoderprimer_restore(primer);

    if (primer->rave) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Close_priv(primer->rave);
        UNLOCK_TRANSPORT();
    }    
    NEXUS_OBJECT_DESTROY(NEXUS_AudioDecoderPrimer, primer);
    BKNI_Free(primer);
}

/* don't wipe out CDB; only wipe out primer state, but keep pcr_offset. */
static void reset_primer(NEXUS_AudioDecoderPrimerHandle primer)
{
    primer->next_gop = 0;
    primer->consumed_gop = 0;
    primer->last_pts = 0;
    primer->full = false;
    primer->overflow = false;
    primer->stuck.cnt = 0;
    clear_next_gop(primer);
}

void NEXUS_AudioDecoderPrimer_Flush( NEXUS_AudioDecoderPrimerHandle primer )
{
    BSTD_DeviceOffset valid;
    
    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);

    /* If playback wrapped, rave was flushed, so reset primers pointers - all the data is gone */
    primer->cdb_base_entry = 0;
    primer->itb_base_entry = 0;
    BKNI_Memset(primer->gops, 0, sizeof(primer->gops));
    primer->pcr_offset = 0;
    primer->pcr_offset_set = primer->playback;
    reset_primer(primer);

    /* cx map pointers aren't good until we're started */
    if (primer->active)
    {
        /* empty the buffer */
        valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Valid);
        BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Read, valid);

        /* ITB_READ and ITB_VALID both have inclusive semantics, so we can copy directly to flush. */
        valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
        BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read, valid);
    }
}

static NEXUS_Error NEXUS_AudioDecoder_P_StartPrimer( NEXUS_AudioDecoderPrimerHandle primer, bool initial )
{
    NEXUS_Error rc;
    NEXUS_PidChannelStatus pidChannelStatus;
    NEXUS_RaveStatus raveStatus;

    if ( primer->startSettings.pidChannel ) {
        rc = NEXUS_PidChannel_GetStatus(primer->startSettings.pidChannel, &pidChannelStatus);
        if (rc) return BERR_TRACE(rc);

        /* playback flag is really playbackTsm flag in both structs */        
        if (primer->startSettings.stcChannel) {
            NEXUS_StcChannelSettings stcSettings;
            NEXUS_StcChannel_GetSettings(primer->startSettings.stcChannel, &stcSettings);
            primer->playback = (stcSettings.mode != NEXUS_StcChannelMode_ePcr) && !primer->startSettings.input;
        }
        else {
            primer->playback = false;
        }
    }
    else {
        BDBG_MSG(("%p: no pidchannel set", (void *)primer ));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    primer->div2ms = NEXUS_IS_DSS_MODE(pidChannelStatus.transportType) ? 27000 : 45;

    if ( initial ) {
        /* Reconfiguring Rave from scratch wipes out any data in rave - don't do it */
        rc = NEXUS_AudioDecoder_P_ConfigureRave(primer->rave, &primer->startSettings, &pidChannelStatus, primer->playback);
        if (!rc) {
            /* Don't need this because stop has already disabled RAVE ?? */
            LOCK_TRANSPORT();
            rc = NEXUS_Rave_GetStatus_priv(primer->rave, &raveStatus);
            if (!rc) {
                NEXUS_Rave_Disable_priv(primer->rave);
                NEXUS_Rave_Flush_priv(primer->rave);
            }
            UNLOCK_TRANSPORT();
        }
        if (rc) return BERR_TRACE(rc);
    }
    else {
        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(primer->rave, &raveStatus);
        UNLOCK_TRANSPORT();
        if (rc) return BERR_TRACE(rc);
    }

    primer->cx_map    = raveStatus.xptContextMap;
    primer->itb.base  = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Base);
    primer->itb.end   = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_End)+1;
    primer->cdb.base  = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Base);
    primer->cdb.end   = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_End)+1;
    primer->sitb_read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read);
    if (primer->sitb_read & 0xF) {
        /* READ ptr can come from HW as inclusive, but we treat it as exclusive */
        primer->sitb_read++;
        BDBG_ASSERT(primer->sitb_read % 0x10 == 0);
    }

    if ( initial ) {
        primer->cdb_base_entry = 0;
        primer->itb_base_entry = 0;
        BKNI_Memset(primer->gops, 0, sizeof(primer->gops));
        primer->pcr_offset = 0;
    }
    primer->active = true;
    primer->pcr_offset_set = primer->playback;

    reset_primer(primer);

    BDBG_ASSERT(!primer->timer);
    primer->timer = NEXUS_ScheduleTimer(TIMER_INTERVAL, NEXUS_AudioDecoder_P_PrimerCallback, primer);
    if (!primer->timer) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    LOCK_TRANSPORT();
    NEXUS_Rave_Enable_priv(primer->rave);

    if (!primer->playback) {
        /* set the pcr offset context into acquire mode, so that we get at least one pcr offset entry in the ITB */
        NEXUS_StcChannel_SetPcrOffsetContextAcquireMode_priv(primer->startSettings.stcChannel);
    }
    UNLOCK_TRANSPORT();

    NEXUS_PidChannel_ConsumerStarted(primer->startSettings.pidChannel ); /* needed to unpause playback & stuff like that */

    return 0;
}

NEXUS_Error NEXUS_AudioDecoderPrimer_Start( NEXUS_AudioDecoderPrimerHandle primer, const NEXUS_AudioDecoderStartSettings *pStartSettings )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);
    BDBG_MSG(("%p: Start", (void *)primer));

    if (primer->active) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (!pStartSettings || !pStartSettings->pidChannel || !pStartSettings->stcChannel) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    primer->startSettings = *pStartSettings;

    NEXUS_AudioDecoderPrimer_P_AcquireStartResources(primer);

    rc = NEXUS_AudioDecoder_P_StartPrimer(primer, true);
    if (rc)
    {
        BERR_TRACE(rc);
        NEXUS_AudioDecoderPrimer_P_ReleaseStartResources(primer);
    }

    return rc;
}

static void NEXUS_AudioDecoderPrimer_P_AcquireStartResources(NEXUS_AudioDecoderPrimerHandle primer)
{
    NEXUS_OBJECT_ACQUIRE(primer, NEXUS_PidChannel, primer->startSettings.pidChannel);
    if (primer->startSettings.stcChannel)
    {
        NEXUS_OBJECT_ACQUIRE(primer, NEXUS_StcChannel, primer->startSettings.stcChannel);
    }
    if (primer->startSettings.stcChannel) {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_EnablePidChannel_priv(primer->startSettings.stcChannel, primer->startSettings.pidChannel);
        UNLOCK_TRANSPORT();
    }
}

static void NEXUS_AudioDecoderPrimer_P_ReleaseStartResources(NEXUS_AudioDecoderPrimerHandle primer)
{
    if (primer->startSettings.stcChannel) {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_DisablePidChannel_priv(primer->startSettings.stcChannel, primer->startSettings.pidChannel);
        UNLOCK_TRANSPORT();
    }
    NEXUS_OBJECT_RELEASE(primer, NEXUS_PidChannel, primer->startSettings.pidChannel);
    if (primer->startSettings.stcChannel)
    {
        NEXUS_OBJECT_RELEASE(primer, NEXUS_StcChannel, primer->startSettings.stcChannel);
    }
}

static void NEXUS_AudioDecoderPrimer_P_DisableRave( NEXUS_AudioDecoderPrimerHandle primer )
{
    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(primer->rave);
    NEXUS_Rave_RemovePidChannel_priv(primer->rave); /* this is required for StartPrimer with a different pid */
    NEXUS_Rave_Flush_priv(primer->rave);
    UNLOCK_TRANSPORT();
}

void NEXUS_AudioDecoderPrimer_Stop( NEXUS_AudioDecoderPrimerHandle primer )
{
    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);
    BDBG_MSG(("%p: Stop", (void *)primer));

    if (primer->active==false) {
        BDBG_ERR(("%p: Primer already stopped", (void *)primer));
        return;
    }

    primer->active = false;

    NEXUS_AudioDecoderPrimer_P_DisableRave(primer);
    NEXUS_AudioDecoderPrimer_P_ReleaseStartResources(primer);

    if (primer->timer) {
        NEXUS_CancelTimer(primer->timer);
        primer->timer = NULL;
    }

    nexus_audiodecoderprimer_restore(primer);
}

/* if NEXUS_AudioDecoder_Stop is called after NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode,
we just disconnect and don't restart primer. This is essentially StopPrimerAndStopDecode. */
void NEXUS_AudioDecoderPrimer_P_DecodeStopped(NEXUS_AudioDecoderPrimerHandle primer)
{
    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);
    nexus_audiodecoderprimer_restore(primer);
    NEXUS_AudioDecoderPrimer_P_DisableRave(primer);
}

NEXUS_Error NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( NEXUS_AudioDecoderPrimerHandle primer , NEXUS_AudioDecoderHandle audioDecoder )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);
    BDBG_MSG(("%p: StopPrimerAndStartDecode %p", (void*)primer, (void*)audioDecoder));

    if (audioDecoder->started) {
        BDBG_ERR(("AudioDecoder already started"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if (!primer->active) {
        BDBG_ERR(("%p: cannot StopPrimerAndStartDecode on stopped primer", (void *)primer));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    nexus_audiodecoderprimer_replace(primer, audioDecoder);

    primer->active = false;
    if (primer->timer) {
        NEXUS_CancelTimer(primer->timer);
        primer->timer = NULL;
    }

#ifdef DEBUG_PRIMER
    NEXUS_AudioDecoder_P_PrintItb(primer);
    /* NEXUS_AudioDecoder_P_DumpRegisters(primer); */
#endif

    rc = NEXUS_AudioDecoder_Start(audioDecoder, &primer->startSettings);
    if (rc!=NEXUS_SUCCESS) {
        rc = BERR_TRACE(rc);
        nexus_audiodecoderprimer_restore(primer);
        /* fall through to release resources for primer, as primer has stopped */
    }

    /* NEXUS_AudioDecoder_Start acquires the resources, so primer must release */
    NEXUS_AudioDecoderPrimer_P_ReleaseStartResources(primer);

    return rc;
}

static void NEXUS_AudioDecoderPrimer_P_StopDecoder(NEXUS_AudioDecoderPrimerHandle primer)
{
    NEXUS_Error errCode;
    NEXUS_AudioDecoderHandle audioDecoder = primer->audioDecoder;

    /* Stop Audio decode without flushing RAVE data */
    errCode = NEXUS_AudioDecoder_P_Stop(audioDecoder, false);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }

    audioDecoder->running = false;
    audioDecoder->started = false;
    audioDecoder->suspended = false;
    audioDecoder->trickForceStopped = false; /* do we need to forcedly unmute on Stop, in a way it helps if in a PIP change mode decoder is moved from trickmode on one channel to normal mode on another channel, however it hurts if one stops decoder just in order to change a PID/ audio program */

    BKNI_Memset(&audioDecoder->programSettings, 0, sizeof(audioDecoder->programSettings));

    /* NEXUS_AudioDecoder_Stop would have released these, but we are using _P_Stop, so release here */
    NEXUS_OBJECT_RELEASE(audioDecoder, NEXUS_PidChannel, primer->startSettings.pidChannel);
    if (primer->startSettings.stcChannel) {
        NEXUS_OBJECT_RELEASE(audioDecoder, NEXUS_StcChannel, primer->startSettings.stcChannel);
    }
}

NEXUS_Error NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer( NEXUS_AudioDecoderPrimerHandle primer, NEXUS_AudioDecoderHandle audioDecoder )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);
    BDBG_MSG(("%p: StopDecodeAndStartPrimer %p", (void *)primer, (void*)audioDecoder));

    if (primer->active) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (primer->audioDecoder && primer->audioDecoder != audioDecoder) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (!primer->startSettings.pidChannel) {
        BDBG_ERR(("Must call NEXUS_AudioDecoderPrimer_Start first"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (!audioDecoder->started && !audioDecoder->suspended)
    {
        BDBG_ERR(("Decoder not started, so can't stop it !"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if ( audioDecoder->programSettings.input )
    {
        if ( NEXUS_AudioInput_P_SupportsFormatChanges( audioDecoder->programSettings.input) )
        {
            /* If this input supports dynamic format changes, disable the dynamic format change interrupt. */
            (void)NEXUS_AudioInput_P_SetFormatChangeInterrupt( audioDecoder->programSettings.input, NEXUS_AudioInputType_eDecoder, NULL, NULL, 0);
        }
    }

    /* transfer object ownership to primer */
    NEXUS_AudioDecoderPrimer_P_AcquireStartResources(primer);

    NEXUS_AudioDecoderPrimer_P_StopDecoder(primer);

    nexus_audiodecoderprimer_restore(primer);

    errCode = NEXUS_AudioDecoder_P_StartPrimer( primer, false );
    if (errCode)
    {
        BERR_TRACE(errCode);
        NEXUS_AudioDecoderPrimer_P_ReleaseStartResources(primer);
    }

    return errCode;
}

void NEXUS_AudioDecoder_GetPrimerSettings( NEXUS_AudioDecoderPrimerHandle primer, NEXUS_AudioDecoderPrimerSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);
    *pSettings = primer->settings;
}

NEXUS_Error NEXUS_AudioDecoder_SetPrimerSettings( NEXUS_AudioDecoderPrimerHandle primer, const NEXUS_AudioDecoderPrimerSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(primer, NEXUS_AudioDecoderPrimer);
    primer->settings = *pSettings;
    /* TODO: I've added the function for now, but audio primer may be simple enough to not need settings */
    return 0;
}
