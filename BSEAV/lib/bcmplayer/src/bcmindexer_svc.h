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
#ifndef BCMINDEXER_AVC_H
#define BCMINDEXER_AVC_H

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

#define BFIFO_STATUS(DBG, header,fifo)	DBG((header ":(%#lx:%#lx) write (%u/%u/%#lx), read (%u/%u/%#lx)", (unsigned long)(fifo)->bf_base, (unsigned long)(fifo)->bf_last, BFIFO_WRITE_PEEK(fifo), BFIFO_WRITE_LEFT(fifo), (unsigned long)BFIFO_WRITE(fifo), BFIFO_READ_PEEK(fifo), BFIFO_READ_LEFT(fifo), (unsigned long)BFIFO_READ(fifo)))


typedef struct BNAV_SVC_Entry_Common {
    uint64_t offset; /* 32 bits */
    size_t payload_size; /* 28 bits */
} BNAV_SVC_Entry_Common;


typedef enum BNAV_SVC_Entry_Type {
    BNAV_SVC_Entry_Type_eFrame,
    BNAV_SVC_Entry_Type_eExtensionFrame
} BNAV_SVC_Entry_Type;


typedef struct BNAV_SVC_Entry_Header {
    uint8_t version; /* 4 bits */
    uint16_t pid; /* 12 bits */
    BNAV_SVC_Entry_Type type; /* 4 bits */
} BNAV_SVC_Entry_Header;

typedef enum BNAV_SVC_Frame_Type {
    BNAV_SVC_Frame_Type_eI, /* has no references */
    BNAV_SVC_Frame_Type_eP, /* predicted from past frames */
    BNAV_SVC_Frame_Type_eB  /* predicted from past and further frames */
} BNAV_SVC_Frame_Type;

typedef struct BNAV_SVC_Entry_Frame {
    uint32_t timestamp; /* 32 bits */
    uint32_t pts;    /* 32 bits */
    uint32_t dts;    /* 32 bits */
    BNAV_SVC_Frame_Type frame_type;     /* whether frame uses references */ /* 2 bits */
} BNAV_SVC_Entry_Frame;



typedef struct BNAV_SVC_Frame_state {
    bool valid;
    bool pts_valid;
    uint64_t record_count;
    BNAV_SVC_Entry_Frame frame;
} BNAV_SVC_Frame_state;


typedef struct BNAV_SVC_Entry {
    BNAV_SVC_Entry_Header    header;
    BNAV_SVC_Entry_Common    common;
    union {
        BNAV_SVC_Entry_Frame frame;
    } data;
} BNAV_SVC_Entry;

typedef struct BNAV_Indexer_SVC_Pid {
    BNAV_Indexer_Handle parent;
    uint16_t pid;
    uint8_t start_code;
    uint8_t base_offset_frame_count;
    bool rai_valid; /* random_access_indicator */
    bool pts_valid;
    bool lastRecordCountValid;
    bool frame_completed;
    uint8_t adaptFieldControl;
    uint8_t payload_len;
    uint8_t payload[15];
    uint32_t pts;
    uint32_t dts;
    uint64_t pts_record_count;
    uint64_t lastRecordCount;
    uint64_t record_count;
    struct {
        uint64_t record_count;
        bool rai;
    } rai_state;
    BNAV_SVC_Frame_state frame_state;
    BNAV_SVC_Frame_state extension_frame_state;
    BFIFO_HEAD(BNAV_Indexer_SVC_EntryFifo, BNAV_Entry) entryFifo; /* fifo to reorder entrys based on the file offset */
    BNAV_Entry fifoEntries[32];
} BNAV_Indexer_SVC_Pid;

#define BNAV_INDEXER_SVC_MAX_PIDS    2
typedef struct BNAV_Indexer_SVC_Info {
    unsigned npids;
    BNAV_Indexer_SVC_Pid *fullPacketPid;
    BNAV_Indexer_SVC_Pid pids[BNAV_INDEXER_SVC_MAX_PIDS];
} BNAV_Indexer_SVC_Info;

long BNAV_Indexer_FeedAVC_SVC(BNAV_Indexer_Handle handle, const void *p_bfr, long numEntries);
long BNAV_Indexer_FeedAVC_MVC(BNAV_Indexer_Handle handle, const void *p_bfr, long numEntries);
void BNAV_Indexer_FeedSVC_Init(BNAV_Indexer_Handle handle);
int BNAV_Indexer_SVC_UnPackEntry(const BNAV_Entry *navEntry, BNAV_SVC_Entry *entry);
int BNAV_Indexer_FeedAVC_GetPosition(BNAV_Indexer_Handle handle, BNAV_Indexer_Position *position);

#endif /* BCMINDEXER_AVC_H */

