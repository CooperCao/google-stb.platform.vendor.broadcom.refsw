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
 ***************************************************************************/
#include "nexus_video_decoder_module.h"
#include "nexus_memory.h"
#include "priv/nexus_core.h"
#include "priv/nexus_stc_channel_priv.h"
#include "priv/nexus_pid_channel_priv.h"
#include "bvlc.h"

BDBG_MODULE(nexus_video_decoder_primer);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */
#define TIMER_INTERVAL 30 /* mSec */

struct NEXUS_VideoDecoderPrimer
{
    NEXUS_OBJECT(NEXUS_VideoDecoderPrimer);
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings startSettings;
    NEXUS_VideoDecoderPrimerCreateSettings createSettings;
    NEXUS_VideoDecoderPrimerSettings primerSettings;
    int div2ms; /* constant to convert PTS units to millisecond units. 45 for MPEG, 27000 for DSS */
    NEXUS_RaveHandle rave;
    NEXUS_TimerHandle timer;

    BAVC_XptContextMap cx_map;
    struct {
        BSTD_DeviceOffset base;
    } cdb, itb;
    bool started; /* public Start called. check 'timer' to see if priming. */
    int last_diff;
    bool skipFlushOnStart;
    BSTD_DeviceOffset sitb_read; /* shadow read register */

    /* read registers */
#define MAX_GOPS 100
    struct {
        BSTD_DeviceOffset cdb_read;
        BSTD_DeviceOffset itb_read;
        uint32_t pts;
        uint32_t pcr_offset;
    } gops[MAX_GOPS];
    /* gop ring buffer. never read outside of this.
    next_gop is the write ptr. It points to the gop being written. It cannot be used.
    consumed_gop is the read ptr.
    empty is (next_gop == consumed_gop). */
    unsigned next_gop;
    unsigned consumed_gop;


    /* last base_address and pcr_offset ITB's seen */
    BSTD_DeviceOffset cdb_base_entry; /* address in cdb pointed by last base_address ITB. if 0, none seen. */
    BSTD_DeviceOffset itb_base_entry;    /* address of base_address itb */
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

#define NEXT_GOP_INDEX(i) ((i+1==MAX_GOPS) ? 0 : (i+1))
static void flush_primer(NEXUS_VideoDecoderPrimerHandle primer);
static void reset_primer(NEXUS_VideoDecoderPrimerHandle primer);
static void NEXUS_VideoDecoderPrimer_P_AcquireStartResources(NEXUS_VideoDecoderPrimerHandle primer);
static void NEXUS_VideoDecoderPrimer_P_ReleaseStartResources(NEXUS_VideoDecoderPrimerHandle primer);

#if 0
static void NEXUS_VideoDecoder_P_PrintItb2(NEXUS_VideoDecoderPrimerHandle primer, BSTD_DeviceOffset from, BSTD_DeviceOffset to)
{
    struct itb_entry_t *from_itb;
    struct itb_entry_t *to_itb;
    unsigned count;

    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);

    from_itb = NEXUS_OffsetToCachedAddr(from);
    BDBG_ASSERT(from_itb);
    to_itb = NEXUS_OffsetToCachedAddr(to);
    BDBG_ASSERT(to_itb);
    BDBG_ASSERT(from_itb <= to_itb);
    NEXUS_FlushCache(from_itb, to_itb-from_itb);
    count = 0;
    while (from_itb < to_itb) {
        BDBG_WRN(("%p: ITB %p: %08x %08x %08x %08x", (void*)primer, (void*)from_itb, from_itb->word0, from_itb->word1, from_itb->word2, from_itb->word3));
        from_itb++;
        if (++count == 20) break;
    }
}

static void NEXUS_VideoDecoder_P_PrintItb(NEXUS_VideoDecoderPrimerHandle primer)
{
    BSTD_DeviceOffset itb_valid, itb_read;
    itb_valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
    itb_read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read);
    if (itb_valid < itb_read) {
        BSTD_DeviceOffset itb_wrap;
        itb_wrap = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Wrap);
        BDBG_WRN(("PrintItb " BDBG_UINT64_FMT "->" BDBG_UINT64_FMT ", " BDBG_UINT64_FMT "->" BDBG_UINT64_FMT,
            BDBG_UINT64_ARG(itb_read), BDBG_UINT64_ARG(itb_wrap), BDBG_UINT64_ARG(primer->itb.base), BDBG_UINT64_ARG(itb_valid)));
        NEXUS_VideoDecoder_P_PrintItb2(primer, itb_read, itb_wrap);
        NEXUS_VideoDecoder_P_PrintItb2(primer, primer->itb.base, itb_valid);
    }
    else {
        BDBG_WRN(("PrintItb " BDBG_UINT64_FMT "->" BDBG_UINT64_FMT,
            BDBG_UINT64_ARG(itb_read), BDBG_UINT64_ARG(itb_valid)));
        NEXUS_VideoDecoder_P_PrintItb2(primer, itb_read, itb_valid);
    }
}

static void NEXUS_VideoDecoder_P_DumpData(NEXUS_VideoDecoderPrimerHandle primer)
{
    BSTD_DeviceOffset itb_read, itb_valid;
    uint8_t *fromptr;
    unsigned i, j;

    for (i=0;i<2;i++) {
        if (i == 0) {
            itb_read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read);
            itb_valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
            BKNI_Printf("ITB read=" BDBG_UINT64_FMT " (depth=%d):\n", BDBG_UINT64_ARG(itb_read), (unsigned)(itb_valid - itb_read));
        }
        else {
            itb_read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Read);
            itb_valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Valid);
            BKNI_Printf("CDB read=" BDBG_UINT64_FMT " (depth=%d):\n", BDBG_UINT64_ARG(itb_read), (unsigned)(itb_valid - itb_read));
        }

        if (itb_valid < itb_read) {
            return;
        }

        fromptr = NEXUS_OffsetToCachedAddr(itb_read);
        BDBG_ASSERT(fromptr);
        NEXUS_FlushCache(fromptr, 64);
        for (j=0;j<64;j++) {
            uint8_t *ptr = &fromptr[j];
            ptr = ptr - (unsigned long)ptr%4 + (3-(unsigned long)ptr%4);
            BKNI_Printf("%02x ", *ptr); /* host endian/little endian, ITB */
            if ((j+1)%ITB_SIZE == 0) BKNI_Printf("\n");
        }
    }
}
#endif

static void NEXUS_VideoDecoder_P_SetReadPtr(NEXUS_VideoDecoderPrimerHandle primer)
{
    unsigned i;
    unsigned itbBackup;
    BSTD_DeviceOffset itb_read;

    /* AVD reads from the ITB in 128 byte blocks. So, we must back up to the previous 128 byte boundary and
    pad it with harmless ITB's */
    itb_read = primer->gops[primer->consumed_gop].itb_read;
#define ITB_BLOCK_READ_SIZE 128
    itbBackup = itb_read % ITB_BLOCK_READ_SIZE;
    if (itbBackup == 0 && itb_read != primer->itb.base) {
        itbBackup = ITB_BLOCK_READ_SIZE;
    }
    if (itb_read - itbBackup < primer->itb.base) {
        BDBG_WRN(("%p: unable to set pcr_offset: base " BDBG_UINT64_FMT ", read " BDBG_UINT64_FMT ", backup %d", (void*)primer,
            BDBG_UINT64_ARG(primer->itb.base), BDBG_UINT64_ARG(itb_read), itbBackup));
        itbBackup = 0;
    }
    if (itbBackup) {
        uint32_t *itb, *itbFlush;
        itb_read -= itbBackup;
        itb = itbFlush = NEXUS_OffsetToCachedAddr(itb_read);
        if (!itb) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return;
        }
        for (i=0;i<itbBackup/ITB_SIZE;i++) {
            itb[0] = 0x22800000; /* pcr offset, marked valid */
            itb[1] = primer->gops[primer->consumed_gop].pcr_offset;
            itb[2] = 0;
            itb[3] = 0;
            itb += 4;
        }
        NEXUS_FlushCache(itbFlush, itbBackup);
    }

    BDBG_MSG_TRACE(("%p: SetRave GOP %2d, PTS=%08x PCR=%08x, CDB " BDBG_UINT64_FMT " ITB " BDBG_UINT64_FMT ", add %d ITB leading pad", (void*)primer,
        primer->consumed_gop, primer->gops[primer->consumed_gop].pts, primer->gops[primer->consumed_gop].pcr_offset,
        BDBG_UINT64_ARG(primer->gops[primer->consumed_gop].cdb_read), BDBG_UINT64_ARG(itb_read), itbBackup/ITB_SIZE));

    BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Read, primer->gops[primer->consumed_gop].cdb_read);
    BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read, itb_read);
}

#define BDBG_MSG_SETRAVE(x) /* BDBG_MSG(x) */

static void clear_next_gop(NEXUS_VideoDecoderPrimerHandle primer)
{
    BKNI_Memset(&primer->gops[primer->next_gop], 0, sizeof(primer->gops[primer->next_gop]));
    if (primer->pcr_offset_set) {
        primer->gops[primer->next_gop].pcr_offset = primer->pcr_offset;
    }
}

#define BDBG_MSG_PROCESS(x) /* BDBG_MSG(x) */

static void NEXUS_VideoDecoder_P_PrimerSetRave(NEXUS_VideoDecoderPrimerHandle primer)
{
    uint32_t serialStc;
    int gop_index = -1;
    int min_diff = 0;
    unsigned i;

    if (primer->consumed_gop == primer->next_gop) return; /* empty fifo */

    LOCK_TRANSPORT();
    NEXUS_StcChannel_GetSerialStc_priv(primer->startSettings.stcChannel, &serialStc);
    UNLOCK_TRANSPORT();

    i = primer->consumed_gop;
    while (i != primer->next_gop) {
        uint32_t stc = serialStc + primer->gops[i].pcr_offset; /* Serial STC + offset = STC */
        int diff = primer->gops[i].pts + primer->primerSettings.ptsOffset - stc;

#if 0
        BDBG_MSG_TRACE(("%p: eval%d stc=%#x pts=%#x at " BDBG_UINT64_FMT "/" BDBG_UINT64_FMT,
            primer, i, stc, primer->gops[i].pts, BDBG_UINT64_ARG(primer->gops[i].cdb_read), BDBG_UINT64_ARG(primer->gops[i].itb_read)));
#endif
        if (diff <= 0) {
            if (gop_index == -1 || diff > min_diff) {
                gop_index = i;
                min_diff = diff;
            }
        }

        if (++i == MAX_GOPS) {
            i = 0;
        }
    }

    if (gop_index == -1) {
#if 0
        BDBG_MSG_TRACE(("%p: no action. Serial STC=%08x, closest diff %5d, %d %d", primer, serialStc, min_diff, primer->consumed_gop, primer->next_gop));
#endif
        return;
    }

    primer->consumed_gop = gop_index;
    NEXUS_VideoDecoder_P_SetReadPtr(primer);
    if (++primer->consumed_gop == MAX_GOPS) {
        primer->consumed_gop = 0;
    }
}

static void NEXUS_VideoDecoder_P_PrimerProcessItb(NEXUS_VideoDecoderPrimerHandle primer, BSTD_DeviceOffset itb_valid)
{
    NEXUS_Error rc;
    struct itb_entry_t * pitb;
    struct itb_entry_t * pitb_end;
    uint8_t type;

    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);

    pitb = NEXUS_OffsetToCachedAddr(primer->sitb_read);
    if (!pitb) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    pitb_end = NEXUS_OffsetToCachedAddr(itb_valid);
    if (!pitb_end) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    NEXUS_FlushCache(pitb, itb_valid - primer->sitb_read);

    while (pitb < pitb_end) {
        /* BDBG_MSG_TRACE(("%p: %08x %08x %08x %08x", primer, pitb->word0, pitb->word1, pitb->word2, pitb->word3)); */
        /* determine entry type */
        type = pitb->word0 >> 24;
        switch(type)
        {
        case 0x22: /* pcr offset */
            if (((pitb->word0 >> 23) & 0x1) == 0) {
                BDBG_WRN(("%p: received invalid pcr offset : %#x", (void *)primer, pitb->word1));
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
                        BDBG_WRN(("%p: reset primer on PTS discontinuity: %#x, %#x, diff %d", (void*)primer, pts, primer->last_pts, diff));
                        reset_primer(primer);
                    }
                }

                if (primer->pcr_offset_set && primer->cdb_base_entry && (!primer->last_pts || diff > 200)) {
                    primer->last_pts = primer->gops[primer->next_gop].pts = pts;
                    primer->gops[primer->next_gop].pcr_offset = primer->pcr_offset;
                    primer->gops[primer->next_gop].cdb_read = primer->cdb_base_entry;
                    primer->gops[primer->next_gop].itb_read = primer->itb_base_entry;
                    if (++primer->next_gop == MAX_GOPS) {
                        primer->next_gop = 0;
                    }
                    if (primer->next_gop == primer->consumed_gop) {
                        /* If you get overflow, there is a high chance of dropped audio data, resulting in no sound */
                        BDBG_ERR(("%p: primer overflow nxtGop=%d consGop=%d diff=%d" , (void *)primer,
                            primer->next_gop , primer->consumed_gop, diff));

                        /* BDBG_ASSERT(primer->next_gop != primer->consumed_gop); */
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
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                return;
            }

            if(0 != ( (CDB_OVERFLOW|ITB_OVERFLOW) & pitb->word2)){
                BDBG_ERR(("overflow(%p) pitb %p, overflow=%#x", (void *)primer, (void*)pitb, ( (CDB_OVERFLOW|ITB_OVERFLOW) & pitb->word2)));
                flush_primer(primer);
                return;
            }
            break;
        }
        pitb++;
    }

    /* need to ensure that we get a pcr offset if we have valid data we are processing */
    if (!primer->pcr_offset_set && (itb_valid - primer->sitb_read) > 0) {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_SetPcrOffsetContextAcquireMode_priv(primer->startSettings.stcChannel);
        UNLOCK_TRANSPORT();
    }

    primer->sitb_read = NEXUS_AddrToOffset(pitb);
    if (!primer->sitb_read) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
}

static void NEXUS_VideoDecoderPrimer_P_Prime(NEXUS_VideoDecoderPrimerHandle primer)
{
    BSTD_DeviceOffset itb_valid, itb_wrap;

    /* convert ITB_Valid and ITB_Wrap to standard ring buffer semantics in this function. instead of pointing to the last byte of the ITB it should point
    to the first byte of the next ITB. Note that RAVE has an exception when valid == base. */
    itb_valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
    if (itb_valid != primer->itb.base) itb_valid++;
    /* check for itb wrap around and handle it */
    if (itb_valid < primer->sitb_read) {
        /* for this to be true, the HW must have wrapped. */
        itb_wrap = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Wrap);
        if (!itb_wrap) {
            BDBG_WRN(("invalid wrap: " BDBG_UINT64_FMT " < " BDBG_UINT64_FMT ", but 0 wrap", BDBG_UINT64_ARG(itb_valid), BDBG_UINT64_ARG(primer->sitb_read)));
            return;
        }
        itb_wrap++;
        NEXUS_VideoDecoder_P_PrimerProcessItb(primer, itb_wrap);
        /* if we consumed up to wrap point then wrap */
        if(primer->sitb_read >= itb_wrap){
            primer->sitb_read = primer->itb.base;
            NEXUS_VideoDecoder_P_PrimerProcessItb(primer, itb_valid);
        }
    }
    else {
        NEXUS_VideoDecoder_P_PrimerProcessItb(primer, itb_valid);
    }

    NEXUS_VideoDecoder_P_PrimerSetRave(primer);
}

static void NEXUS_VideoDecoderPrimer_P_Timer(void *context)
{
    NEXUS_VideoDecoderPrimerHandle primer = context;
    primer->timer = NULL;
    NEXUS_VideoDecoderPrimer_P_Prime(primer);
    primer->timer = NEXUS_ScheduleTimer(TIMER_INTERVAL, NEXUS_VideoDecoderPrimer_P_Timer, primer);
}

void NEXUS_VideoDecoderPrimer_GetDefaultCreateSettings( NEXUS_VideoDecoderPrimerCreateSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->fifoSize = 8 * 1024 * 1024; /* high bit rate streams require a lot of CDB space for priming */
}

NEXUS_VideoDecoderPrimerHandle NEXUS_VideoDecoderPrimer_Open( NEXUS_VideoDecoderHandle videoDecoder )
{
    NEXUS_VideoDecoderPrimerCreateSettings createSettings;
    NEXUS_VideoDecoderPrimerHandle primer;

    NEXUS_VideoDecoderPrimer_GetDefaultCreateSettings(&createSettings);
    if (videoDecoder) {
        /* use public API to get settings */
        NEXUS_VideoDecoderOpenSettings openSettings;
        NEXUS_VideoDecoder_GetOpenSettings(videoDecoder, &openSettings);
        createSettings.fifoSize = openSettings.fifoSize;
    }
    primer = NEXUS_VideoDecoderPrimer_Create(&createSettings);
    if (primer && videoDecoder) {
        NEXUS_VideoDecoderSettings decoderSettings;
        NEXUS_VideoDecoder_GetSettings(videoDecoder, &decoderSettings);
        primer->primerSettings.ptsOffset = decoderSettings.ptsOffset;
    }
    return primer;
}

void NEXUS_VideoDecoderPrimer_Close( NEXUS_VideoDecoderPrimerHandle primer )
{
    NEXUS_VideoDecoderPrimer_Destroy(primer);
}

NEXUS_VideoDecoderPrimerHandle NEXUS_VideoDecoderPrimer_Create( const NEXUS_VideoDecoderPrimerCreateSettings *pSettings )
{
    NEXUS_VideoDecoderPrimerHandle primer;
    NEXUS_RaveOpenSettings raveOpenSettings;
    NEXUS_Error rc = 0;
    NEXUS_VideoDecoderOpenMosaicSettings decodeOpenSettings;
    NEXUS_VideoDecoderPrimerCreateSettings defaultCreateSettings;

    primer = BKNI_Malloc(sizeof(*primer));
    if (!primer) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_VideoDecoderPrimer, primer);

    if (!pSettings) {
        NEXUS_VideoDecoderPrimer_GetDefaultCreateSettings(&defaultCreateSettings);
        pSettings = &defaultCreateSettings;
    }
    primer->createSettings = *pSettings;
    NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&decodeOpenSettings);
    decodeOpenSettings.openSettings.fifoSize = pSettings->fifoSize;
    decodeOpenSettings.openSettings.itbFifoSize = 0;
    NEXUS_VideoDecoder_P_GetRaveSettings(0, &raveOpenSettings, &decodeOpenSettings);
    raveOpenSettings.config.UsePictureCounter = true;

    LOCK_TRANSPORT();
    primer->rave = NEXUS_Rave_Open_priv(&raveOpenSettings);
    if (!primer->rave) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
    }
    UNLOCK_TRANSPORT();
    if (rc) goto error; /* already traced above */
    return primer;

error:
    NEXUS_VideoDecoderPrimer_Close(primer);
    return NULL;
}

/*
 * TODO: mark Create as a constructor (Open already is) and this code can
 * be removed.
 */
static void NEXUS_VideoDecoderPrimer_P_Release( NEXUS_VideoDecoderPrimerHandle primer )
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoderPrimer, primer);
    /* because we have Destroy and Close, add explicit UNREGISTER here */
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoDecoderPrimer, primer, Destroy);
}

static void NEXUS_VideoDecoderPrimer_P_Finalizer( NEXUS_VideoDecoderPrimerHandle primer )
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoderPrimer, primer);

    if (primer->started) {
        NEXUS_VideoDecoderPrimer_Stop(primer);
    }
    if (primer->rave) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Close_priv(primer->rave);
        UNLOCK_TRANSPORT();
    }
    NEXUS_OBJECT_DESTROY(NEXUS_VideoDecoderPrimer, primer);
    BKNI_Free(primer);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_VideoDecoderPrimer, NEXUS_VideoDecoderPrimer_Destroy);

/* don't wipe out CDB; only wipe out primer state, but keep pcr_offset. */
static void reset_primer(NEXUS_VideoDecoderPrimerHandle primer)
{
    primer->next_gop = 0;
    primer->consumed_gop = 0;
    primer->last_pts = 0;
    clear_next_gop(primer);
}

/* wipe out CDB */
static void flush_primer(NEXUS_VideoDecoderPrimerHandle primer)
{
    BSTD_DeviceOffset valid;

    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);

    primer->cdb_base_entry = 0;
    primer->itb_base_entry = 0;
    BKNI_Memset(primer->gops, 0, sizeof(primer->gops));
    primer->next_gop = 0;
    primer->consumed_gop = 0;
    primer->last_pts = 0;

    /* empty the buffer */
    /* CDB_READ has exclusive semantics, but CDB_VALID is inclusive, so a conversion is needed. */
    valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Valid);
    BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Read, (valid == primer->cdb.base)?primer->cdb.base:valid+1);
    BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Wrap, 0);

    /* ITB_READ and ITB_VALID both have inclusive semantics, so we can copy directly to flush. */
    valid = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Valid);
    primer->sitb_read = valid+1;
    BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read, primer->sitb_read);
    BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Wrap, 0);
    reset_primer(primer);
}

static NEXUS_Error NEXUS_VideoDecoderPrimer_P_Start(NEXUS_VideoDecoderPrimerHandle primer);

NEXUS_Error NEXUS_VideoDecoderPrimer_Start( NEXUS_VideoDecoderPrimerHandle primer, const NEXUS_VideoDecoderStartSettings *pStartSettings )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);

    if (primer->started) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (!pStartSettings->stcChannel) {
        /* fcc algorithm requires stc channel */
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    primer->startSettings = *pStartSettings;

    NEXUS_VideoDecoderPrimer_P_AcquireStartResources(primer);

    rc = NEXUS_VideoDecoderPrimer_P_Start(primer);
    if (rc) {
        BERR_TRACE(rc);
        NEXUS_VideoDecoderPrimer_P_ReleaseStartResources(primer);
    }
    else {
        primer->started = true;
    }

    return rc;
}

static NEXUS_Error NEXUS_VideoDecoderPrimer_P_Start(NEXUS_VideoDecoderPrimerHandle primer)
{
    const NEXUS_VideoDecoderStartSettings *pStartSettings = &primer->startSettings;
    NEXUS_RaveSettings raveSettings;
    NEXUS_RaveStatus raveStatus;
    NEXUS_PidChannelStatus pidChannelStatus;
    NEXUS_Error rc;

    rc = NEXUS_PidChannel_GetStatus(pStartSettings->pidChannel, &pidChannelStatus);
    if (rc) return BERR_TRACE(rc);
    primer->div2ms = NEXUS_IS_DSS_MODE(pidChannelStatus.transportType) ? 27000 : 45;
    primer->last_diff = 0;

    LOCK_TRANSPORT();
    if (primer->skipFlushOnStart) {
        rc = 0;
    }
    else {
        struct NEXUS_VideoDecoderDevice *device = nexus_video_decoder_p_any_device(); /* primer is not tied to any HVD instance, so get one */
        NEXUS_Rave_GetDefaultSettings_priv(&raveSettings);
        raveSettings.pidChannel = pStartSettings->pidChannel;
        raveSettings.bandHold = false; /* should not be true when live, should not be true when IP */
        raveSettings.continuityCountEnabled = !pidChannelStatus.playback;
        raveSettings.includeRepeatedItbStartCodes = device && device->cap.bIncludeRepeatedItbStartCodes;
        rc = NEXUS_Rave_ConfigureVideo_priv(primer->rave, pStartSettings->codec, &raveSettings);
    }
    if (!rc) {
        rc = NEXUS_Rave_GetStatus_priv(primer->rave, &raveStatus);
    }
    if (!rc) {
        NEXUS_Rave_Disable_priv(primer->rave);
        if (!primer->skipFlushOnStart) {
            NEXUS_Rave_Flush_priv(primer->rave);
        }
    }

    NEXUS_StcChannel_EnablePidChannel_priv(pStartSettings->stcChannel, pStartSettings->pidChannel);

    UNLOCK_TRANSPORT();
    if (rc) { BERR_TRACE(rc); goto error; }

    primer->cx_map = raveStatus.xptContextMap;
    primer->cdb.base = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.CDB_Base);
    primer->itb.base = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Base);
    primer->sitb_read = BREG_ReadAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read);
    if (primer->skipFlushOnStart && primer->sitb_read % ITB_SIZE) {
        /* ITB_READ comes back from AVD non-ITB aligned. Touch it up. */
        primer->sitb_read -= (primer->sitb_read % ITB_SIZE);
        BREG_WriteAddr(g_pCoreHandles->reg, primer->cx_map.ITB_Read, primer->sitb_read);
    }
    primer->cdb_base_entry = 0;
    primer->itb_base_entry = 0;
    BKNI_Memset(primer->gops, 0xFF, sizeof(primer->gops));
    primer->next_gop = primer->consumed_gop = 0;
    primer->pcr_offset_set = false; /* we must get a pcr_offset because any gop counts */
    primer->last_pts = 0;
    clear_next_gop(primer);

    primer->skipFlushOnStart = false;

    BDBG_ASSERT(!primer->timer);
    primer->timer = NEXUS_ScheduleTimer(TIMER_INTERVAL, NEXUS_VideoDecoderPrimer_P_Timer, primer);
    if (!primer->timer) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto error;
    }
    NEXUS_PidChannel_ConsumerStarted(pStartSettings->pidChannel); /* needed to unpause playback & stuff like that */

    LOCK_TRANSPORT();
    NEXUS_Rave_Enable_priv(primer->rave);
    /* set the pcr offset context into acquire mode, so that we get at least one pcr offset entry in the ITB */
    NEXUS_StcChannel_SetPcrOffsetContextAcquireMode_priv(primer->startSettings.stcChannel);
    UNLOCK_TRANSPORT();

    return 0;

error:
    return rc;
}

static void NEXUS_VideoDecoderPrimer_P_AcquireStartResources(NEXUS_VideoDecoderPrimerHandle primer)
{
    NEXUS_OBJECT_ACQUIRE(primer, NEXUS_PidChannel, primer->startSettings.pidChannel);
    if (primer->startSettings.stcChannel)
    {
        NEXUS_OBJECT_ACQUIRE(primer, NEXUS_StcChannel, primer->startSettings.stcChannel);
    }
}

static void NEXUS_VideoDecoderPrimer_P_ReleaseStartResources(NEXUS_VideoDecoderPrimerHandle primer)
{
    NEXUS_OBJECT_RELEASE(primer, NEXUS_PidChannel, primer->startSettings.pidChannel);
    if (primer->startSettings.stcChannel)
    {
        NEXUS_OBJECT_RELEASE(primer, NEXUS_StcChannel, primer->startSettings.stcChannel);
    }
}

static void NEXUS_VideoDecoderPrimer_P_Stop( NEXUS_VideoDecoderPrimerHandle primer )
{
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    if (primer->timer) {
        NEXUS_CancelTimer(primer->timer);
        primer->timer = NULL;
    }
}

static void NEXUS_VideoDecoderPrimer_P_DisableRave( NEXUS_VideoDecoderPrimerHandle primer )
{
    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(primer->rave);
    NEXUS_Rave_RemovePidChannel_priv(primer->rave); /* this is required for StartPrimer with a different pid */
    NEXUS_Rave_Flush_priv(primer->rave);
    UNLOCK_TRANSPORT();
}

void NEXUS_VideoDecoderPrimer_Stop( NEXUS_VideoDecoderPrimerHandle primer )
{
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    BDBG_MSG(("%p: Stop", (void *)primer));

    if (!primer->started) {
        BDBG_ERR(("%p: Primer already stopped", (void *)primer));
        return;
    }

    if (primer->videoDecoder) {
        /* This will stop decoder and primer. skipFlushOnStop == false tells VideoDecoder to not
        only unlink from the primer, but also stop it through a recursive call. */
        BDBG_OBJECT_ASSERT(primer->videoDecoder, NEXUS_VideoDecoder);
        BDBG_ASSERT(!primer->videoDecoder->skipFlushOnStop);
        NEXUS_VideoDecoder_Stop(primer->videoDecoder);
        BDBG_ASSERT(!primer->videoDecoder);
        BDBG_ASSERT(!primer->started);
    }
    else {
        NEXUS_VideoDecoderPrimer_P_Stop(primer);

#if 0
        NEXUS_VideoDecoder_P_PrintItb(primer);
        NEXUS_VideoDecoder_P_DumpData(primer);
#endif

        /* Rave must be disabled in order for StartPrimer to work correctly later on */
        NEXUS_VideoDecoderPrimer_P_DisableRave(primer);
        LOCK_TRANSPORT();
        NEXUS_StcChannel_DisablePidChannel_priv(primer->startSettings.stcChannel, primer->startSettings.pidChannel);
        UNLOCK_TRANSPORT();
        NEXUS_VideoDecoderPrimer_P_ReleaseStartResources(primer);
        primer->started = false;
    }
}

void NEXUS_VideoDecoderPrimer_P_Link(NEXUS_VideoDecoderPrimerHandle primer, NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    BDBG_ASSERT(!primer->videoDecoder);
    BDBG_ASSERT(!videoDecoder->primer);
    primer->videoDecoder = videoDecoder;
    videoDecoder->primer = primer;
    videoDecoder->savedRave = videoDecoder->rave;
    videoDecoder->rave = primer->rave;
}

void NEXUS_VideoDecoderPrimer_P_Unlink(NEXUS_VideoDecoderPrimerHandle primer, NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    BDBG_ASSERT(primer->videoDecoder == videoDecoder);
    BDBG_ASSERT(videoDecoder->primer == primer);
    primer->videoDecoder = NULL;
    videoDecoder->primer = NULL;
    videoDecoder->rave = videoDecoder->savedRave;
    videoDecoder->savedRave = NULL;
}

static void NEXUS_VideoDecoderPrimer_P_Sync(NEXUS_VideoDecoderPrimerHandle primer)
{
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_VideoDecoderHandle videoDecoder = primer->videoDecoder;
    bool has_sync_channel = videoDecoder->sync.settings.startCallback_isr != NULL;

    if (primer->last_diff < 0 && primer->primerSettings.ptsStcDiffCorrectionEnabled && has_sync_channel) {
        videoDecoder->primerPtsOffset = -primer->last_diff;
        BDBG_MSG(("%p: applying pts offset %u", (void *)primer, videoDecoder->primerPtsOffset));
    }
    else {
        BDBG_MSG(("%p: don't apply pts offset %d user=%d has_sync_channel=%d", (void *)primer, primer->last_diff, primer->primerSettings.ptsStcDiffCorrectionEnabled, has_sync_channel));
        videoDecoder->primerPtsOffset = 0;
    }
    NEXUS_VideoDecoder_P_SetCustomSyncPtsOffset(videoDecoder);
#else
    BSTD_UNUSED(primer);
#endif
}

NEXUS_Error NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode( NEXUS_VideoDecoderPrimerHandle primer, NEXUS_VideoDecoderHandle videoDecoder )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    BDBG_MSG(("%p: StopPrimerAndStartDecode %p", (void*)primer, (void*)videoDecoder));

    if (!primer->started) {
        BDBG_ERR(("%p: cannot StopPrimerAndStartDecode on stopped primer", (void *)primer));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (videoDecoder->started) {
        BDBG_ERR(("VideoDecoder already started"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    NEXUS_VideoDecoderPrimer_P_Link(primer, videoDecoder);

    NEXUS_VideoDecoderPrimer_P_Stop(primer);

    /* last-second adjustment */
    NEXUS_VideoDecoderPrimer_P_Prime(primer);

    if (primer->consumed_gop == primer->next_gop) {
        BDBG_MSG(("%p: StopPrimerAndStartDecode without GOP", (void *)primer)); /* no previous RAP found */
        primer->last_diff = 0;
    }

    NEXUS_VideoDecoderPrimer_P_Sync(primer);

    rc = NEXUS_VideoDecoder_Start(videoDecoder, &primer->startSettings);
    if (rc!=NEXUS_SUCCESS) {
        BERR_TRACE(rc);
        NEXUS_VideoDecoderPrimer_P_Unlink(primer, videoDecoder);
        /* fall through to release resources for primer, as primer has stopped */
    }

    return rc;
}

static void NEXUS_VideoDecoderPrimer_P_StopDecoder(NEXUS_VideoDecoderPrimerHandle primer)
{
    NEXUS_VideoDecoderHandle videoDecoder = primer->videoDecoder;

    /* Stop video decode without flushing RAVE data */
    videoDecoder->skipFlushOnStop = true;
    /* Stop releases object resources */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    BDBG_ASSERT(!videoDecoder->skipFlushOnStop);
}

NEXUS_Error NEXUS_VideoDecoderPrimer_StopDecodeAndStartPrimer( NEXUS_VideoDecoderPrimerHandle primer, NEXUS_VideoDecoderHandle videoDecoder )
{
    NEXUS_Error rc;
    
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    BDBG_MSG(("%p: StopDecodeAndStartPrimer %p", (void *)primer, (void*)videoDecoder));

    if (!primer->started) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (primer->videoDecoder && videoDecoder != primer->videoDecoder) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    BDBG_ASSERT(videoDecoder->rave == primer->rave);
    
    if (!primer->startSettings.pidChannel) {
        BDBG_ERR(("Must call NEXUS_VideoDecoderPrimer_Start first"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (!videoDecoder->started)
    {
        BDBG_ERR(("Decoder not started, so can't stop it !"));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    NEXUS_VideoDecoderPrimer_P_StopDecoder(primer); /* calls unlink */

    BDBG_ASSERT(!primer->videoDecoder);

    primer->skipFlushOnStart = true;
    rc = NEXUS_VideoDecoderPrimer_P_Start(primer);
    if (rc) {
        BERR_TRACE(rc);
        primer->skipFlushOnStart = false;
    }
    BDBG_ASSERT(!primer->skipFlushOnStart);

    return rc;
}

void NEXUS_VideoDecoderPrimer_GetSettings(NEXUS_VideoDecoderPrimerHandle primer, NEXUS_VideoDecoderPrimerSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    *pSettings = primer->primerSettings;
}

NEXUS_Error NEXUS_VideoDecoderPrimer_SetSettings(NEXUS_VideoDecoderPrimerHandle primer, const NEXUS_VideoDecoderPrimerSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    primer->primerSettings = *pSettings;
    return 0;
}

NEXUS_Error NEXUS_VideoDecoderPrimer_GetStatus( NEXUS_VideoDecoderPrimerHandle primer, NEXUS_VideoDecoderStatus *pStatus )
{
    unsigned depth, size;
    BDBG_OBJECT_ASSERT(primer, NEXUS_VideoDecoderPrimer);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->started = primer->started;
    pStatus->pts = primer->last_pts;
    BKNI_EnterCriticalSection();
    NEXUS_Rave_GetCdbBufferInfo_isr(primer->rave, &depth, &size);
    BKNI_LeaveCriticalSection();
    pStatus->fifoDepth = depth;
    pStatus->fifoSize = size;
    pStatus->tsm = true;
    return 0;
}
