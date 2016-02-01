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
#ifndef NEXUS_PLAYBACK_IMPL_H__
#define NEXUS_PLAYBACK_IMPL_H__

#include "nexus_playback.h"
#include "blst_slist.h"
#include "bfile_io.h"
#include "nexus_file_pvr.h"
#include "bmedia_player.h"
#include "bmedia_util.h"


/* #define B_PLAYBACK_CAPTURE    1 */

#if B_PLAYBACK_CAPTURE
#include <stdio.h>
void b_play_capture_open(NEXUS_PlaybackHandle p);
void b_play_capture_close(NEXUS_PlaybackHandle p);
void b_play_capture_write(NEXUS_PlaybackHandle p, const void *buf, size_t len);
#else
#define b_play_capture_write(p, buf, len)
#define b_play_capture_open(p)
#define b_play_capture_close(p)
#endif
#if NEXUS_PLAYBACK_BLOCKAUTH
#include "nexus_playback_blockauth_impl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * all requests in playback are called synchronously with data flow, so for example
 * we could jump to any position immediatly, without waiting for frame to be decoded
 * while in slowmotion mode. However not desctructive operations, such as pause,
 * frameadvance, play, should be performed asynchronously. In order to allow that
 * we should store command and handle it when it's possible. With such scheme some
 * commands, like frameadvace, with cumulative effect require us to accumulate commands.
 * This queue is cleared by any desctructive command, such as jump */

enum b_playback_state {
    /* because we have another state bplayback_state, start this one from 100, so we could at least separate between those two */
    eTransition=100, /* set when we are about to transition our state, usually it's not seen by neither event handlers */
    eWaitingPlayback,  /* set when we are waiting for playback buffer */
    eWaitingIo,  /* set when  I/O is in progress and we are waiting completeion */
    eWaitingRecord, /* set when we are waiting for record to notify us about data written to the file */
    eCancelIo, /* state when API request data pump to cancel I/O */
    eIoCanceled, /* acknowledgement set, if I/O was canceled */
    eAborted,
    eStopping,
    eTimer, /* waiting for callback from b_timer api */
    eStopped
};

/* this type is used to set where control activation is coming from */
enum b_control_origin {
    eControlFrame, /* control is called from the frame feed routine */
    eControlData, /* control is called from the internal data feeding loop */
    eControlDataIO /* control is called from the I/O request handler */
};


typedef struct NEXUS_Playback_P_PidChannel {
    BLST_S_ENTRY(NEXUS_Playback_P_PidChannel) link;
    NEXUS_PlaybackHandle playback;
    NEXUS_PidChannelHandle  pidChn;
    uint16_t pid;
    NEXUS_PlaybackPidChannelSettings cfg;
} NEXUS_Playback_P_PidChannel;

BDBG_OBJECT_ID_DECLARE(NEXUS_Playback);

typedef enum b_trick_state {
    b_trick_state_normal, /* Host sends unmodified stream to decoder */
    b_trick_state_host_trick_mode, /* Host manipulates the stream before sending to decoder for vsync decode */
    b_trick_state_brcm_trick_mode, /* Host manipulates the stream, including embedded BTP's, before sending to decoder for vsync decode */
    b_trick_state_host_paced,      /* Host manipulates the stream and the PTS's before sending to decoder for TSM decode */
    b_trick_state_display_queue_trick_mode /* Host sends beginning of GOP for decode and post-decode reordering */
} b_trick_state;

typedef struct b_trick_settings {
    b_trick_state state;
    int decode_rate;   /* based on NEXUS_NORMAL_PLAY_SPEED */
    NEXUS_VideoDecoderDecodeMode decode_mode;   /* Special mode the decoder. Often used to skip content. */
    NEXUS_VideoDecoderReorderingMode reordering_mode; /* decoder receives out-of-order fragments of the stream */
    bool fragmented; /* decoder receives fragments of the stream */
    bool forward;   /* if false, reverse the display of fields */
    bool stc_trick;
    bool audio_only_pause;
    bool set_decoders; /* if true, Playback can set decoder trick state. if false, don't set it. */
    bool avoid_flush;
    unsigned force_source_frame_rate;
    unsigned maxFrameRepeat;
    bool simulated_tsm;
} b_trick_settings;

struct NEXUS_Playback {
    BDBG_OBJECT(NEXUS_Playback)
    NEXUS_FilePlayHandle file;
    bmedia_player_t media_player; /* media player is used to play various container formats */
    BKNI_EventHandle ack_event; /* feedback to public api context that processing has been stopped */
    BKNI_EventHandle playpump_event; /* event from the playpump */
    NEXUS_EventCallbackHandle playpump_event_callback; /* callback called in responce of playpump_event */
    NEXUS_ThreadHandle playpump_thread; /* thread that is used to handle callbacks from playpump */
    BKNI_EventHandle playpump_thread_event; /* event that is used to wakeup playpump thread */
    BLST_S_HEAD(NEXUS_Playback_P_PidChannels, NEXUS_Playback_P_PidChannel) pid_list;
    NEXUS_TaskCallbackHandle errorCallback, endOfStreamCallback, beginningOfStreamCallback, parsingErrorCallback;
    NEXUS_PlaybackIndexFileIo index_file_mode; /* control what context is used for index file I/O */
    NEXUS_CallbackHandler dataCallbackHandler;
    NEXUS_CallbackHandler videoDecoderFirstPts;
    NEXUS_CallbackHandler videoDecoderFirstPtsPassed;
    bool thread_terminate; /* set to true when playpump_thread should exit */
    unsigned recordProgressCnt; /* number of current NEXUS_Record_AddPlayback connections. if zero, don't go into eWaitingRecord. */

    NEXUS_PlaybackSettings params;
    NEXUS_TransportType actualTransportType; /* format of data sent to the playpump, might be different from params.playpumpSettings.transportType if player convers stream format */
    void *buf_limit; /* The last start point if can use in the playback buffer with O_DIRECT */
#if NEXUS_PLAYBACK_BLOCKAUTH
    struct NEXUS_PlaybackBlockAuthorizationSettings blockauthSettings;
#endif

    /* the following structure is "state" which is zeroed out on bplayback_start. */
    struct {
#if NEXUS_PLAYBACK_BLOCKAUTH
        struct NEXUS_PlaybackBlockAuthorizationState blockauth;
#endif
        bool drain_mode; /* set to true if we are in a drain mode, waiting for the decoder FIFO to go empty */
        NEXUS_PlaybackState mode; /* external state */
        bool loopedDuringPause; /* only fire beginningOfStreamAction or endOfStreamAction once when paused */

        enum b_playback_state state; /* current state of the data pump */
        int direction;
        int index_entrysize; /* the sizeof each index entry. depends on BNAV_Version. */

        void *buf; /* buffer which points to the playback buffer */
        size_t read_size; /* size of the last I/O transaction, this is size passed into read */

        /* The following handle detection of end-of-decode without losing data in the fifos */
        uint32_t fifoMarker; /* some marker which shows activity when decoding. currently using video PTS */
        NEXUS_Time fifoLast; /* last time when fifoMarker has changed */
        int fifoMarkerCounter; /* used for frame advance only */

        void (*data_source) (NEXUS_PlaybackHandle p);  /* function called when data may become avaliable in the playback buffer */
        NEXUS_PlaybackTrickModeSettings trickmode_params; /* parameters used for trick mode (fastforward or rewind) */
        bool reset; /* it's set to false, when control enters into the API function,
            code might want set it to true what would
            force decoder and playback will be flushed before continuing normal execution */
        bool start_paused;  /* playback in started, but in paused state, wait for bplayback_play function to resume playback */
        ssize_t io_size; /* this is used to revive canceled I/O, it keeps number of bytes what I/O has returned */
        NEXUS_TimerHandle timer; /* currently active timer  or NULL */
        struct {
            /* this state is used to manager virtual I/O, so all offsets in the
            file are 4K aligned */
            size_t file_behind; /* number of bytes which we had to seek backwards in the file
                                    in order to read the file in 4K pages */
            size_t junk_data; /* amount of data being skipped in the playback buffer. This
                                    is equal to file_behind + whatever is needed to page-align
                                    the playback buffer memory */
            size_t next_data; /* number of bytes which were read in addition into the playback buffer,
                                    which will become the next file_behind value unless we
                                    have to seek to a new location. */
            off_t last_file_offset; /* allow playback to use SEEK_SET to recover from a bad read. if playback was to use SEEK_CUR,
                                       it could lose alignment due to file fifo trim. */
        } io;
        struct {
            unsigned cur; /* current index into the packet buffer */
            unsigned size; /* last valid byte in the packet buffer */
            const uint8_t *buf;
        } packet; /* structure to handle injected packets */
        struct {
            unsigned cur; /* current byte what we have read */
            unsigned size; /* number of bytes what we have to read */
            off_t offset; /* offset in a file for the frame payload */
            bool valid; /* set to true if entry is valid */
        } frame;
        bool encrypted; /* true for encrypted playback on RAVE platforms */
        bool decoder_flushed; /* used to indicate that decoder/playback buffers were flushed, it's function to remove back-to-back flushes and indicate that media player shall be restarted  */
        struct {
            bool segmented;
            uint8_t *last_segment_hdr; /* last address where the sequnece header could start */
            bmedia_player_entry entry;
            bmedia_time_scale time_scale;
            NEXUS_TimerHandle pts_timer; /* periodic 10sec timer go get current PTS and update index_offset */
            batom_factory_t factory;
            bfile_buffer_t buffer;
            void *buf; /* memory allocated for the buffer */
            batom_cursor cursor;
            struct { /* status for b_lock'ed async I/O */
                void (*read_cont)(void *cont, ssize_t size);
                void *cntx;
            } async_read;
            unsigned nest_count;
            NEXUS_TimerHandle nest_timer;
            uint8_t probe_buf[4096];
        } media;
        enum {NEXUS_Playback_P_FrameAdvance_Invalid, NEXUS_Playback_P_FrameAdvance_Forward, NEXUS_Playback_P_FrameAdvance_Reverse} frame_advance; /* this state variable keeps track of compatible frameadvance mode  (e.g. play is compatible with frameforward and 1xrewind compatible with framereverse, and 2xfastforward not compatible with either */

        bool inAccurateSeek;
        bool validPts;

        bool seekPositionValid; /* this set during seek, and cleared on about any transition */
        NEXUS_PlaybackPosition seekPosition; /* position during seek, it's used when playback in the paused state, so status just returns position user seeked to, it very well might be position that doesn't really exists in the file */

        bmedia_player_pos abortPosition; /* last playback position before abort */
        unsigned index_error_cnt;
        unsigned data_error_cnt;
        NEXUS_PlaybackMpeg2TsIndexType index_type;
        bool navTrailingMode; /* set to true if sending last bytes of MPEG-2 TS file, only used with NAV file */

        bool noDecoderWarning; /* print warning if playback started with no decoder */

        struct {
            const NEXUS_Playback_P_PidChannel *pid;
            NEXUS_VideoDecoderStatus lastStatus; /* use during eWaitingSimpleDecoder */
        } simpleDecoderSuspend;
        struct {
            bool primary, secondary; /* set true if trick mode was settable. avoids extra GetStatus calls. */
        } audioTrick;
    } state;
#if B_PLAYBACK_CAPTURE
    struct {
        bool has_data;
        unsigned no;
        FILE *fout;
    } capture;
#endif
    struct {
        b_trick_settings settings;
        NEXUS_TimerHandle rap_monitor_timer;
    } trick;
    NEXUS_ThreadSettings playpump_thread_settings;
};

#define B_IO_BLOCK_SIZE BIO_BLOCK_SIZE
#define B_IO_BLOCK_LIMIT (4*B_IO_BLOCK_SIZE)
#define B_IO_ALIGN_TRUNC(n) ((n)&(~(B_IO_BLOCK_SIZE-1)))
#define B_IO_ALIGN_ROUND(n) (((n)+B_IO_BLOCK_SIZE-1)&(~(B_IO_BLOCK_SIZE-1)))
#define B_IO_ALIGN_REST(n)  (((unsigned long)(n)) & (B_IO_BLOCK_SIZE-1))

#define B_MEDIA_ATOM_MAGIC  (-2)
#define B_MEDIA_NEST_MAGIC  (-3)

/* related to frame time, but doesn't need to be exact */
#define B_FRAME_DISPLAY_TIME    30

#define B_ONE(s,val,str,next) (((s)==(val)?(str):(next)))
#define B_LAST(s,val,str) (((s)==(val)?(str):("#Invalid#")))
#define B_BOOL_STR(v,tr,fl) ((v)?(tr):(fl))

#define B_TRICK_STATE_STR(s) B_ONE(s, b_trick_state_normal,"normal",B_ONE(s, b_trick_state_host_paced,"host_paced", B_ONE(s, b_trick_state_host_trick_mode,"host",B_ONE(s, b_trick_state_display_queue_trick_mode,"dqt",B_LAST(s, b_trick_state_brcm_trick_mode, "BRCM")))))
#define B_FORWARD_STR(s) B_BOOL_STR(s,"forward","reverse")
#define B_DECODE_MODE_STR(s) B_ONE(s, NEXUS_VideoDecoderDecodeMode_eAll, "all",B_ONE(s, NEXUS_VideoDecoderDecodeMode_eI, "I", B_LAST(s, NEXUS_VideoDecoderDecodeMode_eIP, "IP")))

void b_play_check_buffer(NEXUS_PlaybackHandle p);
void b_play_read_callback(void *context);

/* index play */
void b_play_next_frame(NEXUS_PlaybackHandle p);
void b_play_send_packet(NEXUS_PlaybackHandle p);
void b_play_frame_data(void *playback_, ssize_t size);
NEXUS_Error b_play_start_drain(NEXUS_PlaybackHandle p);
void b_play_send_frame(NEXUS_PlaybackHandle p);
void b_play_timer(void *);

bool b_play_control(NEXUS_PlaybackHandle playback, enum b_control_origin origin);
void b_play_update_location(NEXUS_PlaybackHandle p);
/* this  function would returnh either position based on PTS (if decoders are connected) or position based on last read location, if there is no connected decoders */
bmedia_player_pos b_play_correct_position(NEXUS_PlaybackHandle p, const bmedia_player_status *player_status);
bool b_play_has_connected_decoders(NEXUS_PlaybackHandle p);
void b_play_handle_read_error(NEXUS_PlaybackHandle p, ssize_t size);

NEXUS_Error b_play_media_handle_loop_condition(NEXUS_PlaybackHandle p, bool is_beginning, NEXUS_PlaybackLoopMode mode, const bmedia_player_bounds *bounds);
void b_play_media_time_test(void);
void b_play_media_send_meta(NEXUS_PlaybackHandle p);
NEXUS_Error b_play_start_media(NEXUS_PlaybackHandle playback, NEXUS_FilePlayHandle file, const NEXUS_PlaypumpStatus *playpump_status,const NEXUS_PlaybackStartSettings *params);
void b_play_stop_media(NEXUS_PlaybackHandle playback);
void b_play_suspend_timer(void *playback);

void b_play_flush(NEXUS_PlaybackHandle playback);
void b_play_trick_get(NEXUS_PlaybackHandle p, b_trick_settings *cfg);
NEXUS_Error b_play_trick_set(NEXUS_PlaybackHandle p, const b_trick_settings *cfg);
NEXUS_Error b_play_trick_set_pid(NEXUS_PlaybackHandle playback, const NEXUS_Playback_P_PidChannel *pid, const b_trick_settings *settings);
NEXUS_Error b_play_trick_frameadvance(NEXUS_PlaybackHandle p);
void b_play_trick_init(NEXUS_PlaybackHandle p);
void b_play_trick_shutdown(NEXUS_PlaybackHandle p);
void b_play_update_media_player_config(NEXUS_PlaybackHandle p, bmedia_player_decoder_config *config);

void bplay_get_decode_mark(NEXUS_PlaybackHandle playback, uint32_t *pFifoMarker, unsigned *pCdbDepth, unsigned *pWaitTime);

NEXUS_Error bplay_p_play(NEXUS_PlaybackHandle p, bool *restart, bool flush);
NEXUS_Error bplay_p_play_from(NEXUS_PlaybackHandle p, bmedia_player_pos position);

NEXUS_Error bplay_p_pause(NEXUS_PlaybackHandle p);
void bplay_p_clear_buffer(NEXUS_PlaybackHandle p);

void NEXUS_Playback_P_AbortPlayback(NEXUS_PlaybackHandle p);

/* nexus_playback_decoder functions - abstracts decoder api's */
NEXUS_Error NEXUS_P_Playback_VideoDecoder_GetStatus(const NEXUS_Playback_P_PidChannel *pid, NEXUS_VideoDecoderStatus *pStatus);
void        NEXUS_P_Playback_VideoDecoder_GetTrickState(const NEXUS_Playback_P_PidChannel *pid, NEXUS_VideoDecoderTrickState *pState);
NEXUS_Error NEXUS_P_Playback_VideoDecoder_SetTrickState(const NEXUS_Playback_P_PidChannel *pid, const NEXUS_VideoDecoderTrickState *pState);
void        NEXUS_P_Playback_VideoDecoder_GetPlaybackSettings(const NEXUS_Playback_P_PidChannel *pid, NEXUS_VideoDecoderPlaybackSettings *pSettings);
NEXUS_Error NEXUS_P_Playback_VideoDecoder_SetPlaybackSettings(const NEXUS_Playback_P_PidChannel *pid, const NEXUS_VideoDecoderPlaybackSettings *pSettings);
void        NEXUS_P_Playback_VideoDecoder_Flush(const NEXUS_Playback_P_PidChannel *pid);
NEXUS_Error NEXUS_P_Playback_AudioDecoder_GetStatus(const NEXUS_Playback_P_PidChannel *pid, NEXUS_AudioDecoderStatus *pStatus, NEXUS_AudioDecoderStatus *pSecondaryStatus);
NEXUS_Error NEXUS_Playback_P_AudioDecoder_Advance(const NEXUS_Playback_P_PidChannel *pid, uint32_t video_pts);
void        NEXUS_P_Playback_AudioDecoder_Flush(const NEXUS_Playback_P_PidChannel *pid);
void        NEXUS_P_Playback_AudioDecoder_GetTrickState(const NEXUS_Playback_P_PidChannel *pid, NEXUS_AudioDecoderTrickState *pState, NEXUS_AudioDecoderTrickState *pSecondaryState);
NEXUS_Error NEXUS_P_Playback_AudioDecoder_SetTrickState(const NEXUS_Playback_P_PidChannel *pid, const NEXUS_AudioDecoderTrickState *pState);
NEXUS_Error b_play_getpts(NEXUS_PlaybackHandle p, uint32_t *pts);
bool b_play_decoders_in_tsm_mode(NEXUS_PlaybackHandle playback);
const NEXUS_Playback_P_PidChannel *b_play_get_video_decoder(NEXUS_PlaybackHandle playback);
bool NEXUS_Playback_P_CheckSimpleDecoderSuspended(const NEXUS_Playback_P_PidChannel *pid);

/* callbacks from decoders */
void NEXUS_Playback_P_VideoDecoderFirstPts(void *context);
void NEXUS_Playback_P_VideoDecoderFirstPtsPassed(void *context);

#define b_imin(x,y) ((x)<(y) ? (x):(y))

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_PLAYBACK_IMPL_H__ */

