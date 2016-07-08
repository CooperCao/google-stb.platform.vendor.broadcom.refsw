/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description: Converts startcode index to bcmplayer index
 *
 ****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvlc.h"
#include "bcmindexer.h"
#include "bcmindexerpriv.h"
#include "mpeg2types.h"

BDBG_MODULE(bcmindexer_svc);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x)  */
#define BNAV_INDEXER_SVC_MINOR_VERSION  1


typedef struct b_vlc_state {
    unsigned index;
    unsigned bit;
} b_vlc_state;

static void BNAV_Indexer_SVC_VlcStart(b_vlc_state *vlc)
{
    vlc->index = 0;
    vlc->bit = 7;
    return;
}

static void BNAV_Indexer_SVC_VlcSkip(b_vlc_state *vlc, unsigned bytes)
{
    vlc->index += bytes;
    vlc->bit = 7;
    return;
}

static int BNAV_Indexer_SVC_VlcDecode(BNAV_Indexer_SVC_Pid *pid, b_vlc_state *vlc)
{
    return b_vlc_decode(pid->payload, pid->payload_len, vlc->index, vlc->bit, &vlc->index, &vlc->bit);
}

/* low level bit manipulation */
/* 
Summary:
Returns bit 'b' from word 'w', 

Example:
    B_GET_BIT(0xDE,1)==0 
    B_GET_BIT(0xDE,7)!=0 
*/
#define B_GET_BIT(w,b)  ((w)&(1<<(b)))

/* 
Summary:
Returns bits 'e'..'b' from word 'w', 

Example:
    B_GET_BITS(0xDE,7,4)==0x0D
    B_GET_BITS(0xDE,3,0)=0x0E 
*/
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

/* 
Summary:
Sets bit 'b 'to value of 'v', 'name' is ignored (used as inline comment).
'v' is converted to bool (0/1) before setting bit, e.g. v with values of 1 or 1000 would generate the same result 

Example:
    B_SET_BIT("example 1",1,0)==1
    B_SET_BIT("example 2",1,2)==4
    B_SET_BIT("example 3",100,2)==4
*/

#define B_SET_BIT(name,v,b)  (((unsigned)((v)!=0)<<(b)))

/* 
Summary:
Sets bits 'e'..'b 'to value of 'v', 'name' is ignored (used as inline comment).
'v' is not truncated to 'e'-'b', e.g. if v has more than 'e'-'b'+1 bits result is undefined

*/
#define B_SET_BITS(name,v,e,b)  (((unsigned)(v))<<(b))

/* 
Summary:
Sets bit 'b'...'e' to value of 'v', name is ignored (used as inline comment).
'v' is not truncated, e.g. it's the user responsibilty that it would feet into the given number of bits

Example:
    B_SET_BITS("example 1",1,0,0)==1
    B_SET_BITS("example 2",1,1,0)==1
    B_SET_BITS("example 3",0x55,7,1)==0xAA
*/

#define B_SET_BITS(name,v,e,b)  (((unsigned)(v))<<(b))



static void BNAV_Indexer_SVC_PackEntry(const BNAV_SVC_Entry *entry, BNAV_Entry *navEntry)
{
    int32_t pts_dts_diff;

    BKNI_Memset(navEntry, 0, sizeof(*navEntry));
    BNAV_set_version(navEntry, entry->header.version); /* word1 bits 31..24 */
    BNAV_set_frameSize(navEntry, entry->common.payload_size); /* word5 bits 28 .. 0 */
    BNAV_set_frameOffset(navEntry, entry->common.offset); /* word3 */
    navEntry->words[0] = 
        B_SET_BITS("pid", entry->header.pid, 12, 0) |
        B_SET_BITS("type", entry->header.type, 16, 13) |
        B_SET_BITS("minor", BNAV_INDEXER_SVC_MINOR_VERSION,  19, 17);

    switch(entry->header.type) {
    case BNAV_SVC_Entry_Type_eExtensionFrame:
    case BNAV_SVC_Entry_Type_eFrame:
        BNAV_set_framePts( navEntry, entry->data.frame.pts); /* word4 */
        BNAV_set_timestamp( navEntry, entry->data.frame.timestamp); /* word6 */
        navEntry->words[0] |= 
            B_SET_BITS("frame_type", entry->data.frame.frame_type, 31, 30);
        pts_dts_diff = entry->data.frame.pts - entry->data.frame.dts;
        if(pts_dts_diff > 0x7FFF) {
            pts_dts_diff = 0x7FFF;
        } else if (pts_dts_diff < -0x8000) {
            pts_dts_diff = -0x8000;
        }
        navEntry->words[1] |= B_SET_BITS("pts-dts[15..0]", (((uint32_t)pts_dts_diff)&0xFFFF), 15, 0);
        break;
    default:
        BDBG_ASSERT(0);
    }
    return;
}


static unsigned BNAV_Indexer_P_GetEntryType(const BNAV_Entry *navEntry)
{
    return B_GET_BITS(navEntry->words[0], 16, 13);
}

int BNAV_Indexer_SVC_UnPackEntry(const BNAV_Entry *navEntry, BNAV_SVC_Entry *entry)
{
    unsigned minor;
    int32_t pts_dts_diff;

    BKNI_Memset(entry, 0, sizeof(*entry));
    entry->header.version = BNAV_get_version(navEntry);
    entry->common.payload_size = BNAV_get_frameSize(navEntry);
    entry->common.offset = BNAV_get_frameOffset(navEntry);
    entry->header.pid = B_GET_BITS(navEntry->words[0], 12, 0);
    entry->header.type = BNAV_Indexer_P_GetEntryType(navEntry);

    minor = B_GET_BITS(navEntry->words[0], 19, 17);
    if(minor != BNAV_INDEXER_SVC_MINOR_VERSION) {
        BDBG_WRN(("unknown minor version %u(%u)", minor, BNAV_INDEXER_SVC_MINOR_VERSION));
        return -1;
    }
    switch(entry->header.type) {
    case BNAV_SVC_Entry_Type_eExtensionFrame:
    case BNAV_SVC_Entry_Type_eFrame:
        entry->data.frame.pts = BNAV_get_framePts(navEntry);
        entry->data.frame.timestamp = BNAV_get_timestamp(navEntry);
        entry->data.frame.frame_type = B_GET_BITS(navEntry->words[0], 31, 30);
        pts_dts_diff = (int16_t)(B_GET_BITS(navEntry->words[1], 15, 0));
        entry->data.frame.dts = entry->data.frame.pts - pts_dts_diff; 
        break;
    default:
        BDBG_WRN(("unknown entry type %u", entry->header.type));
        return -1;
    }
    return 0;
}    
   

static void *BNAV_Indexer_SVC_InitEntry(const BNAV_Indexer_SVC_Pid *pid, BNAV_SVC_Entry *entry, BNAV_SVC_Entry_Type type, uint64_t start)
{
    BKNI_Memset(entry, 0, sizeof(*entry));
    entry->header.pid = pid->pid;
    entry->header.type = type;
    entry->header.version = BNAV_Version_AVC_Extended;
    entry->common.offset = start;
    entry->common.payload_size = pid->record_count - start;
    return &entry->data;
}

static BERR_Code BNAV_Indexer_SVC_P_WriteEntry(BNAV_Indexer_SVC_Pid *pid, const BNAV_Entry *navEntry)
{
    BNAV_Indexer_Handle indexer = pid->parent;
    int numBytes;

    numBytes = (indexer->settings.writeCallback)(navEntry, 1, sizeof(*navEntry), indexer->settings.filePointer);
    if(numBytes!=sizeof(*navEntry)) { return BERR_TRACE(BERR_UNKNOWN);}
    indexer->totalEntriesWritten++;
    return BERR_SUCCESS;
}


static BERR_Code BNAV_Indexer_SVC_EnqueueEntry(BNAV_Indexer_SVC_Pid *pid, const BNAV_SVC_Entry *entry)
{
    unsigned i;
    BNAV_Entry *minNavEntry;
    BNAV_Entry *navEntry;
    BNAV_Indexer_SVC_Pid *minPid;

    BDBG_MSG(("%#x:new entry %u at %lu:%u", pid->pid, entry->header.type, (unsigned long)entry->common.offset, (unsigned)entry->common.payload_size));
    if(BFIFO_WRITE_PEEK(&pid->entryFifo)==0) {
        navEntry = BFIFO_READ(&pid->entryFifo);
        BFIFO_READ_COMMIT(&pid->entryFifo, 1);
        BDBG_MSG(("%#x:reordering fifo full saving entry %u at %lu:%u", pid->pid, entry->header.type, (unsigned long)entry->common.offset, (unsigned)entry->common.payload_size));
        BNAV_Indexer_SVC_P_WriteEntry(pid, navEntry);
    }
    navEntry = BFIFO_WRITE(&pid->entryFifo);
    BFIFO_WRITE_COMMIT(&pid->entryFifo,1);
    BNAV_Indexer_SVC_PackEntry(entry, navEntry);
    for(;;) {
        BNAV_Indexer_Handle indexer = pid->parent;
        for(minPid=NULL,minNavEntry=NULL,i=0; i<indexer->svc.npids;i++) {
            BNAV_Indexer_SVC_Pid *nextPid = &indexer->svc.pids[i];

            if(BFIFO_READ_PEEK(&nextPid->entryFifo)==0) {
                minNavEntry = NULL;
                break;
            }
            navEntry = BFIFO_READ(&nextPid->entryFifo);
            if(minNavEntry==NULL || BNAV_get_frameOffset(minNavEntry)>BNAV_get_frameOffset(navEntry)) {
                minPid = nextPid;
                minNavEntry = navEntry;
            }
        }
        if(minNavEntry==NULL) {
            break;
        }
        BFIFO_READ_COMMIT(&minPid->entryFifo, 1);
        BDBG_MSG(("%#x:saving reordered entry %u at %lu:%u", minPid->pid, (unsigned)BNAV_Indexer_P_GetEntryType(minNavEntry), (unsigned long)BNAV_get_frameOffset(minNavEntry), (unsigned)BNAV_get_frameSize(minNavEntry)));
        BNAV_Indexer_SVC_P_WriteEntry(minPid, minNavEntry);
    }

    return BERR_SUCCESS;
}

static void BNAV_Indexer_SVC_CompleteFrame(BNAV_Indexer_SVC_Pid *pid, BNAV_SVC_Frame_state *frame, BNAV_SVC_Entry *entry)
{
    if(frame->valid && frame->pts_valid) {
        BNAV_Entry navEntry;
        BNAV_SVC_Entry_Frame *svc_frame;
        BNAV_Indexer_Handle handle = pid->parent;
        BNAV_SVC_Entry_Type type = frame == &pid->extension_frame_state ? BNAV_SVC_Entry_Type_eExtensionFrame : BNAV_SVC_Entry_Type_eFrame;

        frame->valid = false;
        frame->pts_valid = false;

        BDBG_MSG(("%#x:found frame %u at %u:%u", pid->pid, (unsigned)frame->frame.frame_type, (unsigned)frame->record_count, (unsigned)(pid->record_count - frame->record_count)));
        /* simulate inputs to the BNAV_Indexer_StampEntry */
        BKNI_Memset(&navEntry, 0, sizeof(navEntry));
        BNAV_set_frameType(&navEntry, pid->frame_state.frame.frame_type == BNAV_SVC_Frame_Type_eI? eSCTypeIFrame : eSCTypeBFrame); 
        handle->next_pts = pid->pts;
        BNAV_Indexer_StampEntry(handle, &navEntry);
        svc_frame = BNAV_Indexer_SVC_InitEntry(pid, entry, type, frame->record_count);
        *svc_frame = frame->frame;
        svc_frame->timestamp = BNAV_get_timestamp(&navEntry);
        pid->frame_state.frame.timestamp = svc_frame->timestamp;
        pid->base_offset_frame_count ++;
        BNAV_Indexer_SVC_EnqueueEntry(pid, entry);
        if(type == BNAV_SVC_Entry_Type_eFrame) {
            pid->frame_completed = false;
        }
    }
    return;
}


static void BNAV_Indexer_SVC_P_StartFrame(BNAV_Indexer_SVC_Pid *pid, BNAV_SVC_Frame_state *frame)
{
    if(pid->pts_valid) {
        frame->frame.pts = pid->pts;
        frame->frame.dts = pid->dts;
        frame->pts_valid = pid->pts_valid;
        frame->record_count = pid->pts_record_count;
        frame->frame.frame_type = BNAV_SVC_Frame_Type_eI;
        frame->valid = true;
        pid->pts_valid = false;
    } else {
        BDBG_WRN(("%s frame without dedicated PTS", frame == &pid->extension_frame_state ? "extension":"base"));
    }
    return;
}

static void BNAV_Indexer_SVC_P_FrameSlice(BNAV_Indexer_SVC_Pid *pid, BNAV_SVC_Frame_state *frame, b_vlc_state *vlc, BNAV_SVC_Entry *entry)
{
    unsigned first_mb_in_slice, slice_type, pic_parameter_set_id;

    /* vlc decode the payload */
    first_mb_in_slice = BNAV_Indexer_SVC_VlcDecode(pid, vlc);
    slice_type = BNAV_Indexer_SVC_VlcDecode(pid, vlc);
    pic_parameter_set_id = BNAV_Indexer_SVC_VlcDecode(pid, vlc);
    BSTD_UNUSED(pic_parameter_set_id);
    BDBG_MSG_TRACE(("pid:%#x first_mb_in_slice:%u slice_type:%u", pid->pid, first_mb_in_slice, slice_type));
    if (first_mb_in_slice == 0) {
        /* we have the beginning of a new frame. first, complete the previous frame. */
        if(frame->valid) {
            BDBG_WRN(("multiple %s frames", frame == &pid->extension_frame_state ? "extension":"base"));
            BNAV_Indexer_SVC_CompleteFrame(pid, frame, entry);
        }
        BNAV_Indexer_SVC_P_StartFrame(pid, frame);
    }
    switch (slice_type) {
    case 2:
    case 7:
        /* we've already defaulted to I frame */
        break;
    case 0:
    case 5:
        /* if we ever get one P slice, then it's either a P or B frame, cannot be I */
        if(frame->frame.frame_type == BNAV_SVC_Frame_Type_eI) {
            frame->frame.frame_type = BNAV_SVC_Frame_Type_eP;
        }
        break;
    case 1:
    case 6:
        /* if we ever get one B slice, it's a B frame */
        frame->frame.frame_type = BNAV_SVC_Frame_Type_eB;
        break;
    default:
        BDBG_WRN(("unsupported slice_type %d", slice_type));
        break;
    }
    return;
}

static BERR_Code BNAV_Indexer_SVC_ProcessPayload(BNAV_Indexer_SVC_Pid *pid, BNAV_SVC_Entry *entry)
{
    unsigned nal_ref_idc;
    unsigned nal_unit_type;
    unsigned sc = pid->start_code;
    b_vlc_state vlc;

    if (B_GET_BIT(sc, 7)) {
       /* if forbidden_zero_bit is set, this is not an AVC start code */
        goto done;
    }


    nal_ref_idc = B_GET_BITS(sc, 6, 2);
    nal_unit_type = B_GET_BITS(sc, 4, 0);
    BSTD_UNUSED(nal_ref_idc);


    BNAV_Indexer_SVC_VlcStart(&vlc);

    switch (nal_unit_type) {
    case 1: /* non-IDR slice */
    case 5: /* IDR slice */
        BNAV_Indexer_SVC_P_FrameSlice(pid, &pid->frame_state, &vlc, entry);
        break;
    case 20: /* Coded slice extension */
        BNAV_Indexer_SVC_VlcSkip(&vlc, 3);
        BNAV_Indexer_SVC_P_FrameSlice(pid, &pid->extension_frame_state, &vlc, entry);
        break;
    default:
        break;
    }

done:
    pid->payload_len = 0;
    return BERR_SUCCESS;
}

static void BNAV_Indexer_SVC_P_UpdateRecordCount(BNAV_Indexer_Handle handle, BNAV_Indexer_SVC_Pid *pid, const BSCT_SixWord_Entry *itb)
{
    pid->lastRecordCount = BNAV_Indexer_getOffset(handle, CONVERT_TO_SCT4(itb));
    pid->lastRecordCountValid = true;
    pid->record_count = pid->lastRecordCount;
    return ;
}

static void BNAV_Indexer_SVC_P_SetPts(BNAV_Indexer_SVC_Pid *pid, uint32_t pts, BNAV_SVC_Entry *entry)
{
    BDBG_MSG_TRACE(("pid %#x, PTS=%#x", pid->pid, pts));
    pid->pts_valid = true;
    pid->pts = pts;
    pid->dts = pts;
    pid->pts_record_count = pid->record_count;
    BNAV_Indexer_SVC_CompleteFrame(pid, &pid->frame_state, entry);
    BNAV_Indexer_SVC_CompleteFrame(pid, &pid->extension_frame_state, entry);
    return;
}

static BERR_Code BNAV_Indexer_SVC_OneItb(BNAV_Indexer_Handle handle, BNAV_Indexer_SVC_Pid *pid, unsigned entry_type, const BSCT_SixWord_Entry *itb)
{
    uint8_t start_code;
    BERR_Code rc;
    BNAV_SVC_Entry entry;

    switch(entry_type) {
    case 0x00:
    case 0x10: /* 16 byte start code extraction , first 9 bytes */
        start_code = B_GET_BITS(itb->startCodeBytes, 31, 24);
        if(start_code==SC_PTS) {
            BNAV_Indexer_SVC_P_SetPts(pid, itb->recordByteCountHi /* pts */, &entry);
        } else {
            if(pid->payload_len!=0) {
                rc = BNAV_Indexer_SVC_ProcessPayload(pid, &entry);
                if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
            }
            pid->start_code = start_code;
            BNAV_Indexer_SVC_P_UpdateRecordCount(handle, pid, itb);
            pid->payload[0] = B_GET_BITS(itb->startCodeBytes, 23, 16);
            pid->payload[1] = B_GET_BITS(itb->startCodeBytes, 15, 8);
            pid->payload[2] = B_GET_BITS(itb->recordByteCountHi, 23, 16);
            pid->payload[3] = B_GET_BITS(itb->recordByteCountHi, 15, 8);
            pid->payload[4] = B_GET_BITS(itb->recordByteCountHi, 8,  0);
            pid->payload[5] = B_GET_BITS(itb->flags, 23, 16);
            pid->payload[6] = B_GET_BITS(itb->flags, 15, 8);
            pid->payload[7] = B_GET_BITS(itb->flags, 8, 0);
            pid->payload_len = 8;
        }
        break;
    case 0x01: /* TPIT */
        BNAV_Indexer_SVC_P_UpdateRecordCount(handle, pid, itb);
        if(B_GET_BIT(itb->startCodeBytes, 22)) {
            BDBG_MSG_TRACE(("SCT PAYLOAD_TO_FOLLOW"));
            handle->fullPacketCount = 4;
            handle->fullPacket[0] = 0;
            handle->fullPacket[1] = 0;
            handle->fullPacket[2] = 0;
            handle->fullPacket[3] = 0;
            handle->adaptFieldControl = B_GET_BITS(itb->word0, 5, 4);
            handle->svc.fullPacketPid = pid;
        }
        if (itb->startCodeBytes & 0x20) {
           pid->rai_valid = true;
           BNAV_Indexer_SVC_P_UpdateRecordCount(handle, pid, itb);
           pid->rai_state.record_count = pid->record_count;
        }
        break;
    case 0x07:
        if(pid->payload_len!=8) {
            BDBG_WRN(("out of sequence entry %u", entry_type));
            handle->parsingErrors++;
            break;
        }
        pid->payload[8] = B_GET_BITS(itb->flags, 31, 24);
        pid->payload[9] = B_GET_BITS(itb->flags, 23, 16);
        pid->payload[10] = B_GET_BITS(itb->flags, 15, 8);
        pid->payload[11] = B_GET_BITS(itb->flags, 8, 0);
        pid->payload_len = 12;
        rc = BNAV_Indexer_SVC_ProcessPayload(pid, &entry);
        if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
        break;
    case 0x11: /* 16 byte start code extraction , extra 7 bytes */
        if(pid->payload_len!=8) {
            BDBG_WRN(("out of sequence entry %u", entry_type));
            handle->parsingErrors++;
            break;
        }
        pid->payload[8] = B_GET_BITS(itb->startCodeBytes, 31, 24);
        pid->payload[9] = B_GET_BITS(itb->startCodeBytes, 23, 16);
        pid->payload[10] = B_GET_BITS(itb->startCodeBytes, 15, 8);
        pid->payload[11] = B_GET_BITS(itb->startCodeBytes, 8, 0);
        pid->payload[12] = B_GET_BITS(itb->recordByteCount, 31, 24);
        pid->payload[13] = B_GET_BITS(itb->recordByteCount, 23, 16);
        pid->payload[14] = B_GET_BITS(itb->recordByteCount, 15, 8);
        pid->payload_len = 15;
        rc = BNAV_Indexer_SVC_ProcessPayload(pid, &entry);
        if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
        break;
    default:
        BDBG_WRN(("unsupported itb type %#x", entry_type));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}

static int
BNAV_Indexer_P_SVC_ProcessFullPacket(BNAV_Indexer_Handle handle, BNAV_Indexer_SVC_Pid *pid)
{
    unsigned offset = 4;
    bool error=false;
    uint8_t *pkt = handle->fullPacket + 4;
    BNAV_SVC_Entry entry;
    uint32_t scode;
    long rc;

    BDBG_MSG_TRACE(("BNAV_Indexer_P_SVC_ProcessFullPacket"));
    BDBG_MSG_TRACE(("adaptation_field_control:%u", handle->adaptFieldControl));
    switch (handle->adaptFieldControl) {
    case 0x01: 
        break;
    case 0x03: 
        {
            unsigned adaptation_field_length = pkt[0];
            BDBG_MSG_TRACE(("adaptation_field_length:%u", pkt[0]));
            adaptation_field_length ++;
            offset += adaptation_field_length;
            pkt += adaptation_field_length;
            if(offset>=BCMINDEXER_TS_PACKET_SIZE-8) {
                goto done;
            }
        }
        break;
    default: 
        goto done;
    }

    if(pkt[0]==0x00 && pkt[1]==0x00 && pkt[2]==0x01 && (pkt[3]&0xF0) == 0xE0) { /* PES video header */
        if (B_GET_BITS(pkt[7], 7, 6) & 2) {
            uint32_t pts = 
             /* pts_32_30 */((uint32_t)B_GET_BITS(pkt[9], 3, 1)<<29) |
             /* pts_29_22 */((uint32_t)pkt[10]<<21) |
             /* pts_21_15 */((uint32_t)B_GET_BITS(pkt[11], 7, 1)<<14) |
             /* pts_14_7 */((uint32_t)pkt[12]<<6) |
             /* pts_6_0 */B_GET_BITS(pkt[13], 7, 2);
            BNAV_Indexer_SVC_P_SetPts(pid, pts, &entry);
        }
        /* skip through PES start code */
        offset += 4;
        pkt += 4;
    }
    /* Find start codes and build SCT packets.
    The loop test is "<=" so that we can process any remainder. */
    for (scode=0xFF;offset<=BCMINDEXER_TS_PACKET_SIZE-13;offset++) { /* we need to capture 13 bytes of payload, so we should stop 13 bytes prior to the end of packet */
        scode = (scode<<8) | *pkt;
        pkt++;
        if((scode>>8) != 0x01) {
            continue;
        }
        BDBG_MSG_TRACE(("found scode at %u: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",  offset-3, scode&0xFF, pkt[0], pkt[1], pkt[2], pkt[3], pkt[4], pkt[5], pkt[6], pkt[7], pkt[8], pkt[9], pkt[10], pkt[11], pkt[12]));
        pid->start_code = (scode&0xFF);
        scode = 0xFF;
        pid->payload_len = 12;
        /* copy payload */
        BKNI_Memcpy(pid->payload, pkt, 12);
        rc = BNAV_Indexer_SVC_ProcessPayload(pid, &entry);
        if(rc!=BERR_SUCCESS) {
            rc=BERR_TRACE(rc);
            error = true;
            handle->parsingErrors++;
        }
    }
done:
    return error?-1:0;
}

long BNAV_Indexer_FeedAVC_SVC(BNAV_Indexer_Handle handle, const void *p_bfr, long numEntries)
{  
    const BSCT_SixWord_Entry *p_curBfr=p_bfr;
    unsigned i;
    BERR_Code rc;


    if (handle->settings.sctVersion != BSCT_Version6wordEntry) {
        BDBG_ERR(("Must use 6 word SCT's for AVC SVC/MVC content"));
        return -1;
    }

    for(i=0; i<(unsigned)numEntries; i++, p_curBfr++) {
        unsigned j;
        uint16_t pid;
        unsigned entry_type;
        BNAV_Indexer_SVC_Pid *svc_pid=NULL;


        BDBG_MSG(("SCT %08x %08x %08x %08x %08x %08x", p_curBfr->word0, p_curBfr->word1, p_curBfr->startCodeBytes, p_curBfr->recordByteCount, p_curBfr->recordByteCountHi, p_curBfr->flags));
        entry_type = B_GET_BITS(p_curBfr->word0, 31, 24);
        if(entry_type==0x80) {
            if(handle->svc.fullPacketPid) {
                if(BNAV_Indexer_P_AddFullPacketPayload(handle, p_curBfr)!=0) {
                    handle->svc.fullPacketPid = NULL;
                    continue;
                }
                if(handle->fullPacketCount == BCMINDEXER_TS_PACKET_SIZE) {
                    handle->fullPacketCount = 0;
                    rc = BNAV_Indexer_P_SVC_ProcessFullPacket(handle, handle->svc.fullPacketPid);
                    if(rc!=0) {continue;}
                }
            } else {
                BDBG_WRN(("itb type %#x out of sequence", entry_type));
                handle->parsingErrors++;
            }
            continue;
        }
        pid = B_GET_BITS(p_curBfr->word0, 20, 8);
        for(j=0;j<handle->svc.npids;j++) {
            if(handle->svc.pids[j].pid==pid) {
                svc_pid = &handle->svc.pids[j];
                break;
            }
        }
        if(svc_pid==NULL) {
            if(handle->svc.npids>=BNAV_INDEXER_SVC_MAX_PIDS) { 
                BDBG_WRN(("extra pid %#x", pid));
                continue;
            }
            /* allocate new pid */
            svc_pid = &handle->svc.pids[handle->svc.npids];
            BDBG_MSG(("allocating pid %#lx:%#x at %u", (unsigned long)svc_pid, pid, handle->svc.npids));
            handle->svc.npids++;
            BKNI_Memset(svc_pid, 0, sizeof(*svc_pid));
            svc_pid->parent = handle;
            svc_pid->pid = pid;
            svc_pid->base_offset_frame_count = 0;
            svc_pid->frame_state.valid = false;
            svc_pid->frame_state.pts_valid = false;
            svc_pid->extension_frame_state.valid = false;
            svc_pid->extension_frame_state.pts_valid = false;
            svc_pid->pts_valid = false;
            svc_pid->rai_valid = false;
            svc_pid->frame_completed = false;
            svc_pid->lastRecordCountValid = false;
            svc_pid->pts = 0;
            svc_pid->dts = 0;
            svc_pid->pts_record_count = 0;
            svc_pid->lastRecordCount = 0;
            svc_pid->record_count = 0;        
            svc_pid->payload_len = 0;
            BFIFO_INIT(&svc_pid->entryFifo, svc_pid->fifoEntries, sizeof(svc_pid->fifoEntries)/sizeof(*svc_pid->fifoEntries));
        }
        rc = BNAV_Indexer_SVC_OneItb(handle, svc_pid, entry_type, p_curBfr);
        if(rc!=BERR_SUCCESS) {
            rc=BERR_TRACE(rc);
            handle->parsingErrors++;
            continue;
        }
    }

    return -1;
}

long BNAV_Indexer_FeedAVC_MVC(BNAV_Indexer_Handle handle, const void *p_bfr, long numEntries)
{
    BSTD_UNUSED(handle); 
    BSTD_UNUSED(p_bfr); 
    BSTD_UNUSED(numEntries); 
    return -1;
}

void BNAV_Indexer_FeedSVC_Init(BNAV_Indexer_Handle handle)
{
    BKNI_Memset(&handle->svc, 0, sizeof(handle->svc));

    handle->svc.npids = 0;
    handle->svc.fullPacketPid = NULL;
    return;
}

int BNAV_Indexer_FeedAVC_GetPosition(BNAV_Indexer_Handle handle, BNAV_Indexer_Position *position)
{
    BNAV_Indexer_SVC_Pid *pid=NULL;

    if(handle->svc.npids==0) {
        return -1;
    }
    pid = &handle->svc.pids[0];
    if(!pid->frame_completed) {
        return -1;
    }
    position->index = handle->totalEntriesWritten;
    position->pts = pid->frame_state.frame.pts;
    position->vchipState = 0;
    position->timestamp = pid->frame_state.frame.timestamp;
    position->offsetHi = (uint32_t) (pid->record_count>>32);
    position->offsetLo = (uint32_t) pid->record_count;
    return 0;
}

