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
#ifndef NEXUS_PLAYPUMP_IMPL_H__
#define NEXUS_PLAYPUMP_IMPL_H__

#include "nexus_transport_module.h"

#ifndef B_HAS_MEDIA
#define B_HAS_MEDIA 1
#endif

#if B_HAS_MEDIA
#include "bmedia_filter.h"
#endif

#include "bpvrlib_feed.h"
#if BXPT_HAS_DIRECTV_SUPPORT
#include "bxpt_directv_playback.h"
#endif
#include "bfifo.h"
#if 0 && B_HAS_DRM
#include "drmdecryptor.h"
#endif
#if 0 && B_HAS_DRM_ND
#include "bdrmnd_decryptor.h"
#endif

/* setting this to 1 will save data that goes to the playpump */
#define B_SAVE_PLAYPUMP 0

/* We have a separate dbg macro for data flow. When you want to see it, uncomment
the following */
#define BDBG_MSG_FLOW(x) /* BDBG_MSG(x) */

#define B_THROTTLE_TIMEOUT  30

/* we use private1 id */
#define B_PES_ID    0xBD
#define B_PES_SIZE_MAX  (16384-6)

/* PVR atom mutlilpier of both transport packet size, filesyste block size  and filesystem block size */
#define B_PVR_ATOM_SIZE ((188/4)*4096)
#define B_PVR_N_ATOMS  12

/* SCD atom is enough to hold about 3 frames worth of data */
#define B_PVR_SCD_SIZE  (3*4*32)

/* This is the size of the buffer we'll allocate for playback operations */
#define B_PVR_PLAYBACK_BUFFER (B_PVR_ATOM_SIZE*8) /* = 1540096 or 1504K or 1.5MB */

/* Number of descriptors for playback,  this is should correspond to number of frames in the
   playback buffer, plus multiplier for 1 frame rewind.
   For Broadcom trick modes, we need one for every BTP and two for every frame,
   so that's why we increased it dramatically. */
#define NEXUS_NUM_PLAYBACK_DESC 100

/* all new chips have native support for MPEG1 System streams */
/* #define B_HAS_NATIVE_MPEG1  1 */

#if NEXUS_TRANSPORT_EXTENSION_CRC
#include "nexus_playpump_crc.h"
#endif

typedef NEXUS_PlaypumpScatterGatherDescriptor NEXUS_PlaypumpDesc;
struct bpvr_queue_item {
    NEXUS_PlaypumpDesc desc;
    size_t skip;
    unsigned ref_cnt;
    bool sg; /* app submitted scatter gather descriptor via NEXUS_PlaypumpMode_eScatterGather or NEXUS_Playpump_SubmitScatterGatherDescriptor() */

#if NEXUS_TRANSPORT_EXTENSION_CRC
    NEXUS_PlaypumpCrcWriteSettings crc;
#endif
};

/* current might have beed '#define'ed to something else, clear it */
#undef current

BFIFO_HEAD(bpvr_queue, struct bpvr_queue_item);

enum b_play_packetizer {b_play_packetizer_none=0, b_play_packetizer_media, b_play_packetizer_crypto};


typedef struct b_pid_map {
    unsigned base_pid;
    uint8_t map[32];
} b_pid_map;

void b_pid_map_init(b_pid_map *map, uint16_t base);
uint16_t b_pid_map_alloc(b_pid_map *map);
void b_pid_map_free(b_pid_map *map, uint16_t pid);

typedef struct NEXUS_P_PlaypumpPidChannel {
    BLST_S_ENTRY(NEXUS_P_PlaypumpPidChannel) link;
    NEXUS_P_HwPidChannel *pid_channel;
#if B_HAS_MEDIA
    bmedia_stream_t media_stream;
    unsigned stream_id; /* stream id */
    uint16_t media_pid; /* mapped pes id */
#endif
    NEXUS_PlaypumpOpenPidChannelSettings settings;
    struct {
        bool enable;
        bool active;
        BXPT_Playback_PacketizeConfig cfg;
        unsigned context;
    } packetizer;
    unsigned pid;
} NEXUS_P_PlaypumpPidChannel;

BDBG_OBJECT_ID_DECLARE(NEXUS_Playpump);

typedef struct b_pump_demux *b_pump_demux_t;
typedef struct b_pump_crypto *b_pump_crypto_t;

struct NEXUS_Playpump {
    NEXUS_OBJECT(NEXUS_Playpump);
    unsigned index;
    BFIFO_HEAD(NEXUS_Playpump_P_Fifo, uint8_t) fifo;
    struct bpvr_queue activeFifo;  /* FIFO of descriptors with data */
    struct bpvr_queue pendingFifo; /* FIFO of descriptors with data not submitted to hardware */
    NEXUS_TaskCallbackHandle dataCallback;
    NEXUS_TaskCallbackHandle errorCallback;
    NEXUS_IsrCallbackHandle ccErrorCallback;
    NEXUS_IsrCallbackHandle teiErrorCallback;

    BLST_S_HEAD(NEXUS_P_PlaypumpPidList, NEXUS_P_PlaypumpPidChannel) pid_list;

    NEXUS_PlaypumpSettings settings;
    NEXUS_PlaypumpOpenSettings openSettings;

    void *buf; /* cached address used through */
    BMMA_Block_Handle bufBlock;

    void *boundsHeapAddr;
    size_t boundsHeapSize;

    bool consumerStarted;
    bool paused;

    BXPT_Playback_Handle xpt_play;
#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    BXPT_Playback_PacingCounter pacingCounter;
#endif
    BPVRlib_Feed_Handle play_feed;
    BKNI_EventHandle descEvent;
    NEXUS_EventCallbackHandle playEventHandle;
    BINT_CallbackHandle pacingErrIntCallback;
    BINT_CallbackHandle ccErrorInt;
    BINT_CallbackHandle teiErrorInt;
    BINT_CallbackHandle outOfSyncErrorInt;
    NEXUS_HeapHandle heap;

    NEXUS_TimerHandle throttle_timer;

#if B_HAS_MEDIA
    b_pump_demux_t demux;
    bool use_media;
#endif
    b_pump_crypto_t crypto;

    struct {
        bool running;
        bool pacing; /* pacing was enabled and needed to be stopped */
        bool muxInput; /* used as mux input */
        void *last_addr; /* needed for flush and verification of user arguments in NEXUS_Playpump_WriteComplete */
        size_t last_size; /* needed for verification of user arguments in NEXUS_Playpump_WriteComplete */
        struct bpvr_queue_item *last_item; /* last item added into the playpump, but not send to hardware, it's used for merging data into the last chunk */
        enum b_play_packetizer packetizer; /* PES packetizer is active */
        NEXUS_PlaypumpDesc active; /* active transaction */
        NEXUS_PlaypumpDesc current;
        size_t segment_left;
        unsigned queued_in_hw; /* number of elements queued in the HW */
        unsigned pacingTsRangeError;
        uint64_t bytes_played;
        unsigned teiErrors;     /* count of TS packets with the Transport Error Indicator bit set. */
        unsigned syncErrors;   /* count of the number of transport packet sync errors */
    } state;

    b_pid_map packetizer_map;

    struct {
        NEXUS_AudioCodec codec;
        unsigned track;
    } vob_remap_state;

#if B_HAS_PLAYPUMP_IP
    struct b_playpump_ip_impl ip;
#endif
    struct bpvr_queue_item *item_mem;
};


void b_playpump_p_xpt_event(void *playback_);
void b_playpump_p_do_read_callback(NEXUS_PlaypumpHandle p);
void b_playpump_p_stop(NEXUS_PlaypumpHandle p);
void b_playpump_p_reset(NEXUS_PlaypumpHandle p);
void b_playpump_p_throttle_timer(void *play);
bool b_play_next(NEXUS_PlaypumpHandle p);
NEXUS_Error b_playpump_write_complete(NEXUS_PlaypumpHandle p, unsigned skip, unsigned amount_used, const NEXUS_PlaypumpWriteCompleteSettings *pSettings, const void *crcSettings );

BERR_Code b_playpump_p_add_request(NEXUS_PlaypumpHandle p, size_t skip, size_t amount_used, const NEXUS_PlaypumpScatterGatherDescriptor *pSgDesc, NEXUS_PlaypumpWriteCompleteSettings *settings, const void *pCrcSettings);

void NEXUS_P_Playpump_DescAvail_isr(void *p, int unused);


#if B_SAVE_PLAYPUMP
void b_playpump_p_data(const void *data, size_t len);
void b_playpump_p_data_flush(void);
#endif

void b_pump_crypto_enqueue(b_pump_crypto_t crypto, struct bpvr_queue_item *item);
bool b_pump_crypto_feed(b_pump_crypto_t crypto, struct bpvr_queue_item *item, const void *data, size_t len);
bool b_pump_crypto_is_empty(b_pump_crypto_t crypto);
NEXUS_Error b_pump_crypto_start(b_pump_crypto_t crypto);
NEXUS_Error b_pump_crypto_set_stream_type(b_pump_crypto_t crypto, NEXUS_TransportType transportType, bool *supported);
void b_pump_crypto_flush(b_pump_crypto_t crypto);
void b_pump_crypto_reclaim(b_pump_crypto_t crypto);
void b_pump_crypto_stop(b_pump_crypto_t crypto);
void b_pump_crypto_status(b_pump_crypto_t crypto);
void b_pump_crypto_destroy(b_pump_crypto_t crypto);
b_pump_crypto_t b_pump_crypto_create(NEXUS_PlaypumpHandle pump);

#if B_HAS_MEDIA
void b_pump_demux_reclaim(b_pump_demux_t demux);
void b_pump_demux_enqueue(b_pump_demux_t demux, struct bpvr_queue_item *item);
bool b_pump_demux_feed(b_pump_demux_t demux, struct bpvr_queue_item *item, const void *data, size_t len);
void b_pump_demux_flush(b_pump_demux_t demux);
void b_pump_demux_stop(b_pump_demux_t demux);
b_pump_demux_t b_pump_demux_create(NEXUS_PlaypumpHandle pump);
void b_pump_demux_destroy(b_pump_demux_t demux);
void b_pump_demux_status(b_pump_demux_t demux, NEXUS_PlaypumpStatus *pStatus);
NEXUS_Error b_pump_demux_set_stream_type(b_pump_demux_t demux, NEXUS_TransportType transportType, bool *supported);
NEXUS_Error b_pump_demux_start(b_pump_demux_t demux);
NEXUS_Error b_pump_demux_open_pid(b_pump_demux_t demux, NEXUS_P_PlaypumpPidChannel *pid, unsigned stream_id, const NEXUS_PlaypumpOpenPidChannelSettings *pSettings);
bool b_pump_demux_is_empty(b_pump_demux_t demux);
void b_pump_demux_close_pid(b_pump_demux_t demux, NEXUS_P_PlaypumpPidChannel *pid);
NEXUS_Error b_pump_demux_add_pid(b_pump_demux_t demux, NEXUS_P_PlaypumpPidChannel *pid);
void b_pump_demux_remove_pid(b_pump_demux_t demux, NEXUS_P_PlaypumpPidChannel *pid);
void b_pump_demux_descriptor(b_pump_demux_t demux, const NEXUS_PlaypumpSegment *desc);
void b_pump_demux_set_rate(b_pump_demux_t demux, int rate);
void b_pump_demux_advance(b_pump_demux_t demux);
bool b_pump_demux_is_congested(b_pump_demux_t);

#endif /* B_HAS_MEDIA */


#endif /* NEXUS_PLAYPUMP_IMPL_H__ */
