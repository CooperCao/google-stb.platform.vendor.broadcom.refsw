/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
* Description: This file implements the RTSP Wrapper to the Live Media RTSP library.
*
***************************************************************************/
#if defined ( LINUX ) || defined ( __vxworks )

#include "b_playback_ip_lib.h"
//#include "b_playback_ip_lm_helper.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include "bstd.h"
#include "bkni.h"
#include "blst_squeue.h"
#include "brtp.h"
#include "brtp_packet.h"
#include "brtp_parser_h263.h"
#include "brtp_parser_h264.h"
#include "brtp_parser_mpeg4part2.h"
#include "brtp_parser_mpeg4.h"
#include "brtp_parser_aacloas.h"
#include "brtp_parser_amr.h"
#include "bbase64.h"
#include "btimestamp.h"
#include "brtp_util.h"
#include "brtp_play.h"
#include "lm.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"

BDBG_MODULE( b_playback_ip_rtsp_es );

#define B_RTP_N_CHANNELS 2
struct brtp_channel {
    brtp_parser_mux_t          mux;
    brtp_parser_mux_cfg        mux_cfg;
    brtp_parser_h264_cfg       h264_cfg;
    brtp_t                     rtp;
    brtp_parser_t              parser;
    brtp_parser_mux_stream_cfg stream_cfg;
    brtp_parser_mux_stream_t   stream;
    lm_stream_t                lm_stream;
    brtp_ntp                   sr_ntp;
    uint32_t                   sr_timestamp;
    uint32_t                   rtp_timestamp;
    struct {
        bool rtcp_sr : 1;
        bool rtp_ts : 1;
    } sync;
    uint8_t rtp_format;
    uint8_t cfg[256] __attribute__ (( aligned ));
    uint8_t cfg_len;
};

static void rtsp_cmd_completion(
    void                *appCtx,
    B_PlaybackIpEventIds eventId
    )
{
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)appCtx;

    BDBG_ASSERT( playback_ip );
    BKNI_SetEvent( playback_ip->liveMediaSyncEvent );
    BDBG_MSG(( "%s: sent LiveMediaSyncEvent for eventId %d\n", __FUNCTION__, eventId ));
}

void
print_av_pipeline_buffering_status( B_PlaybackIpHandle playback_ip );
static void bFeedRtpPayloadToPlaypump(
    void          *cntx,
    brtp_io_block *block,
    brtp_parser_t  parser,
    uint32_t       pts
    )
{
    brtp_io_vec       *vec;
    B_PlaybackIpHandle playback_ip = cntx;

#ifdef ENABLE_RECOR
    char         recordFileName[32];
    static int   fileNameSuffix = 0;
    static FILE *fclear         = NULL;
#endif
    size_t   buffer_size;
    void    *buffer = NULL;
    unsigned copied;

    BSTD_UNUSED( parser );
    BSTD_UNUSED( pts );
    BSTD_UNUSED(parser);

#ifdef ENABLE_RECOR
    if (playback_ip->enableRecording && !fclear)
    {
        memset( recordFileName, 0, sizeof( recordFileName ));
        snprintf( recordFileName, sizeof( recordFileName )-1, "./videos/rtp_rec%d.ts", fileNameSuffix++ );
        fclear = fopen( recordFileName, "w+b" );
        BDBG_WRN(( "file name %s", recordFileName ));
    }

    for (vec = BLST_SQ_FIRST( block ); vec; vec = BLST_SQ_NEXT( vec, list )) {
        BDBG_WRN(( "%s: ES: %d %#lx", __FUNCTION__, vec->len, vec->data ));
        /* write data to file */
        if (playback_ip->enableRecording && fclear)
        {
            BDBG_WRN(( "writing %d byte", vec->len ));
            fwrite( vec->data, 1, vec->len, fclear );
            fflush( fclear );
        }
    }
#endif /* ifdef ENABLE_RECOR */

    for (buffer_size = 0, copied = 0, vec = BLST_SQ_FIRST( block ); vec; vec = BLST_SQ_NEXT( vec, list )) {
        unsigned consumed;
        unsigned to_copy;
        unsigned left;
        for (consumed = 0; consumed<vec->len; ) {
            if (buffer_size==copied)
            {
                if (copied>0)
                {
                    if (NEXUS_Playpump_WriteComplete( playback_ip->nexusHandles.playpump, 0, buffer_size ))
                    {
                        BDBG_ERR(( "Returned error from NEXUS_Playpump_ReadComplete()!" ));
                        return;
                    }
                }
                for (;; ) {
                    if (NEXUS_Playpump_GetBuffer( playback_ip->nexusHandles.playpump, &buffer, &buffer_size ))
                    {
                        BDBG_ERR(( "Returned error from NEXUS_Playpump_GetBuffer()!" ));
                        return;
                    }
                    if (buffer_size>0)
                    {
                        break;
                    }
                    BDBG_MSG(( "playpump buffer size is %zu, sleeping and retrying ", buffer_size ));
                    //print_av_pipeline_buffering_status(playback_ip);
                    BKNI_Sleep( 60 );
                }
                copied = 0;
            }
            BDBG_ASSERT( buffer );
            BDBG_ASSERT( vec->len>=consumed );
            BDBG_ASSERT( buffer_size>=copied );
            to_copy = vec->len - consumed;
            left    = buffer_size-copied;
            if (to_copy>left)
            {
                to_copy = left;
            }
            BKNI_Memcpy((uint8_t *)buffer + copied, (uint8_t *)vec->data + consumed, to_copy );
            copied   += to_copy;
            consumed += to_copy;
            BDBG_MSG(( "copied %d, consume %d, to_copy %d, vec->len %d, left %d",
                       copied, consumed, to_copy, vec->len, left ));
        }
    }
    if (copied)
    {
        BDBG_MSG(( "calling read complete for %d bytes", copied ));
        if (NEXUS_Playpump_ReadComplete( playback_ip->nexusHandles.playpump, 0, copied ))
        {
            BDBG_ERR(( "Returned error from NEXUS_Playpump_ReadComplete()!" ));
            return;
        }
    }
    //print_av_pipeline_buffering_status(playback_ip);
    BDBG_MSG(( "%s: done", __FUNCTION__ ));
    brtp_parser_mux_recycle( playback_ip->channels[0].mux, block );
    return;
} /* bFeedRtpPayloadToPlaypump */

void brtpClose(
    B_PlaybackIpHandle playback_ip
    )
{
    BKNI_Free( playback_ip->channels[blm_media_video].mux_cfg.meta );
    brtp_parser_mux_destroy( playback_ip->channels[blm_media_video].mux );
    BKNI_Free( playback_ip->channels );
}

int brtpOpen(
    B_PlaybackIpHandle playback_ip
    )
{
    unsigned            chn_no;
    brtp_channel_t      channels;
    brtp_parser_mux_t   mux;
    brtp_parser_mux_cfg mux_cfg;

    channels = BKNI_Malloc( sizeof( struct brtp_channel )*B_RTP_N_CHANNELS );
    BDBG_ASSERT( channels );
    playback_ip->channels = channels;
    BKNI_Memset( channels, 0, sizeof( struct brtp_channel )*B_RTP_N_CHANNELS );

    for (chn_no = 0; chn_no<B_RTP_N_CHANNELS; chn_no++) {
        brtp_parser_mux_stream_default_cfg( &channels[chn_no].stream_cfg );
        channels[chn_no].mux          = NULL;
        channels[chn_no].rtp          = NULL;
        channels[chn_no].parser       = NULL;
        channels[chn_no].stream       = NULL;
        channels[chn_no].lm_stream    = NULL;
        channels[chn_no].cfg_len      = 0;
        channels[chn_no].sync.rtp_ts  = false;
        channels[chn_no].sync.rtcp_sr = false;
    }
    brtp_parser_mux_default_cfg( &mux_cfg );
    channels[blm_media_video].stream_cfg.pes_id = 0xE0;
    channels[blm_media_audio].stream_cfg.pes_id = 0xC0;

    mux_cfg.meta = BKNI_Malloc( mux_cfg.meta_len );
    BDBG_ASSERT( mux_cfg.meta );
    mux_cfg.data_sink      = bFeedRtpPayloadToPlaypump;
    mux_cfg.data_sink_cntx = playback_ip;
    mux_cfg.mode           = brtp_mux_mode_pes;
    mux = brtp_parser_mux_create( &mux_cfg );
    BDBG_ASSERT( mux );
    channels[blm_media_video].mux     = mux;
    channels[blm_media_audio].mux     = mux;
    channels[blm_media_video].mux_cfg = mux_cfg;
    BDBG_MSG(( "v pes id %x", channels[blm_media_video].stream_cfg.pes_id ));
    BDBG_MSG(( "a pes id %x", channels[blm_media_audio].stream_cfg.pes_id ));

    return( 0 );
} /* brtpOpen */

#define LIVEMEDIA_LIB_API_WAIT_TIME 2000
/*
   Function does following:
   -sdf
   -sdf
*/
B_PlaybackIpError B_PlaybackIp_RtspSessionOpen(
    B_PlaybackIpHandle               playback_ip,
    B_PlaybackIpSessionOpenSettings *openSettings,
    B_PlaybackIpSessionOpenStatus   *openStatus /* [out] */
    )
{
    B_PlaybackIpSocketState *socketState;
    B_PlaybackIpError        errorCode = B_ERROR_PROTO;

    if (!playback_ip || !openSettings || !openStatus)
    {
        BDBG_ERR(( "%s: invalid params, playback_ip %p, openSettings %p, openStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)openSettings, (void *)openStatus ));
        return( B_ERROR_INVALID_PARAMETER );
    }

    /* if SessionOpen is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
    {
        return( B_ERROR_IN_PROGRESS );
    }

    /* if SessionOpen is completed, return results to app */
    if (playback_ip->apiCompleted)
    {
        BDBG_WRN(( "%s: previously started session setup operation completed, playback_ip %p\n", __FUNCTION__, (void *)playback_ip ));
        goto done;
    }

    // Set this as early as possible, as it may be changed by calling of sessionStop.
    playback_ip->lmStop = 0;

    /* Neither SessionOpen is in progress nor it is completed, verify input params and then start work */
    if (openSettings->u.rtsp.additionalHeaders && !strstr( openSettings->u.rtsp.additionalHeaders, "\r\n" ))
    {
        BDBG_ERR(( "%s: additional RTSP header is NOT properly terminated (missing \\r\\n), header is %s\n", __FUNCTION__, openSettings->u.rtsp.additionalHeaders ));
        return( B_ERROR_INVALID_PARAMETER );
    }

    /* input parameters are good, so update the api progress state */
    playback_ip->apiInProgress = true;
    playback_ip->apiCompleted  = false;
    socketState                = &openStatus->socketState;

    if (!openSettings->nonBlockingMode)
    {
        /* App wants to do a blocking call, so we will need to define our internal callback to wait for LiveMedia Session Completion */
        /* All RTSP related Live Media calls are non-blocking */
        playback_ip->openSettings.eventCallback = rtsp_cmd_completion;
        playback_ip->openSettings.appCtx        = playback_ip;
    }

    if (brtpOpen( playback_ip ) < 0)
    {
        goto error;
    }

    NEXUS_SetEnv( "nexus_watchdog", NULL );

    /* TODO: During Close Add :     NEXUS_SetEnv("nexus_watchdog","y"); */
done:
    /* RTSP command completed, verify response */
    BDBG_MSG(( "%s: successfully received the RTSP Response", __FUNCTION__ ));
    playback_ip->playback_state = B_PlaybackIpState_eOpened;
    errorCode = B_ERROR_SUCCESS;
    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted  = false;
    return( errorCode );

error:
    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted  = false;
    return( errorCode );
} /* B_PlaybackIp_RtspSessionOpen */

static void b_pkt_free(
    void *source_cntx,
    void *pkt
    )
{
    BSTD_UNUSED( source_cntx );
    BDBG_MSG(( "%s: freeing pkt %p", __FUNCTION__, (void *)pkt ));
    BKNI_Free( pkt );
    return;
}

static void bRtpStoptRtpParser(
    struct brtp_channel *chn
    )
{
    if (chn->parser == NULL)
        return;
    BDBG_WRN(( "%s: ch %p parser %p, rtp %p", __FUNCTION__, (void *)chn, (void *)chn->parser, (void *)chn->rtp ));

    chn->parser->stop( chn->parser );
    brtp_stop( chn->rtp );
    if (chn->h264_cfg.meta) {
        BKNI_Free(chn->h264_cfg.meta);
        chn->h264_cfg.meta = NULL;
    }
    brtp_destroy( chn->rtp );
    chn->parser->destroy( chn->parser );
    return;
}

static void bRtpStartRtpParser(
    struct brtp_channel *chn,
    uint32_t             timestamp_offset
    )
{
    brtp_session_cfg session_cfg;

    BDBG_ASSERT( chn->mux );
    BDBG_ASSERT( chn->rtp );
    BDBG_ASSERT( chn->parser );
    BDBG_ASSERT( chn->stream==NULL );

    BDBG_WRN(( "%s: starting stream %#lx[0x%02x] parser %p offset %u", __FUNCTION__, (unsigned long)chn, chn->stream_cfg.pes_id, (void *)chn->parser, timestamp_offset ));

    brtp_default_session_cfg( chn->rtp, &session_cfg );
    session_cfg.pt_mask  = 0xFF;
    session_cfg.pt_value = chn->rtp_format;
    brtp_start( chn->rtp, chn->parser, &session_cfg );

    chn->stream_cfg.clock_offset = timestamp_offset;

    chn->stream = chn->parser->start( chn->parser, chn->mux, &chn->stream_cfg, &chn->cfg, chn->cfg_len );
    BDBG_ASSERT( chn->stream );
    return;
} /* bRtpStartRtpParser */

/* invoked as a callback from brtp_enqueue_pkt */
static void bRtpTimestampPkt(
    void       *cntx,
    const void *rtp_pkt
    )
{
    struct brtp_channel *chn = cntx;
    uint32_t             timestamp_offset;

    BDBG_MSG(( "%s: ....", __FUNCTION__ ));
    if (chn->sync.rtp_ts)
    {
        return;
    }

    chn->sync.rtp_ts   = true;
    chn->rtp_timestamp = B_RTP_TIMESTAMP( rtp_pkt );
    if (chn->stream)
    {
        return;
    }
    if (chn->parser->type->type == brtp_stream_video)
    {
        timestamp_offset = chn->rtp_timestamp - (uint32_t)( chn->stream_cfg.clock_rate*B_FIRST_TIMESTAMP_DELAY );
        BDBG_WRN(( "autosync %s video timestamp_offset=%u", chn->parser->type->name, timestamp_offset ));
        BDBG_WRN(("video rtp_timestamp %u, clock rate %d, rate*delay %d", chn->rtp_timestamp, chn->stream_cfg.clock_rate, (uint32_t)(chn->stream_cfg.clock_rate*B_FIRST_TIMESTAMP_DELAY)));
        bRtpStartRtpParser( chn, timestamp_offset );
    }

#if 0
    /* commenting this as I am starting audio parser thru rtcp_sr processing function */
        timestamp_offset = chn->rtp_timestamp - (uint32_t)( chn->stream_cfg.clock_rate*B_FIRST_TIMESTAMP_DELAY );
        BDBG_WRN(("autosync %s audio timestamp_offset=%u", chn->parser->type->name, timestamp_offset));
        BDBG_WRN(("video rtp_timestamp %u, clock rate %d, rate*delay %d", chn->rtp_timestamp, chn->stream_cfg.clock_rate, (uint32_t)(chn->stream_cfg.clock_rate*B_FIRST_TIMESTAMP_DELAY)));
        bRtpStartRtpParser( chn, timestamp_offset );
    }
#endif

    return;
} /* bRtpTimestampPkt */

static brtp_enqueue bRtpFeedPktToRtpParser(
    void  *rtp,
    void  *pkt,
    size_t len,
    void  *cookie
    )
{
    struct brtp_channel *chn = rtp;
    brtp_enqueue         status;
    brtp_pkt_info        pkt_info[8];
    int                  ninfo;

    status = brtp_enqueue_pkt( chn->rtp, pkt, len, cookie );
    if (chn->lm_stream)
    {
        ninfo = brtp_get_pkt_info( chn->rtp, pkt_info, sizeof( pkt_info )/sizeof( *pkt_info ));
        if (ninfo>0)
        {
            lm_stream_update_pkt_info( chn->lm_stream, pkt_info, ninfo );
        }
    }
    return( status );
} /* bRtpFeedPktToRtpParser */

static void
bRtpSyncOne(void *cntx, uint32_t ntp_msw, uint32_t ntp_lsw, uint32_t timestamp_offset)
{
    struct brtp_channel *chn = cntx;
    struct brtp_channel *video=NULL;
    struct brtp_channel *audio=NULL;
    int ntp_delta;
    int ts_delta;
    uint32_t rtp_ts;

    BDBG_ASSERT(chn->mux);
    BDBG_ASSERT(chn->rtp);
    BDBG_ASSERT(chn->parser);
#if 0
    BDBG_ASSERT(chn->test);
    if (chn->test->timed_pkt || chn->test->timed_rtp) {
        struct b_pkt_rtcp_sr sr;
        sr.ntp_msw = ntp_msw;
        sr.ntp_lsw = ntp_lsw;
        sr.timestamp = timestamp_offset;
        sr.clock_rate = chn->stream_cfg.clock_rate;
        b_timed_pkt_write_header(chn->test, b_pkt_type_rtcp_sr, sizeof(sr));
        fwrite(&sr, sizeof(sr), 1, chn->test->fout);
        fflush(chn->test->fout);
    }
#endif
    brtp_ntp_init(&chn->sr_ntp, ntp_msw, ntp_lsw);
    chn->sr_timestamp = timestamp_offset;
    chn->sync.rtcp_sr = true;
    audio = &chn[blm_media_audio];
    video=&chn[blm_media_video];
    if (video->stream==NULL) {
        BDBG_WRN(("%#lx don't start %s video until rtp timestamp is known", (unsigned long)chn, chn->parser->type->name));
        return;
    }
    if(audio->stream) {
        return; /* already started */
    }
    BDBG_ASSERT(audio);
    BDBG_ASSERT(video);
    if (!video->sync.rtcp_sr) {
        //BDBG_WRN(("%#lx don't start %s video until av is synchronized", (unsigned long)chn, video->parser->type->name));
        return;
    }
    if (!audio->sync.rtcp_sr) {
        //BDBG_WRN(("%#lx don't start %s audio until av is synchronized", (unsigned long)chn, audio->parser->type->name));
        return;
    }
    /* this is an audio stream and it's time to calculate delay between audio and video */
    ntp_delta = brtp_ntp_diff(&video->sr_ntp, &audio->sr_ntp);
    ts_delta = (brtp_delta_time(video->rtp_timestamp, video->sr_timestamp)*125)/((int)video->stream_cfg.clock_rate/16);
    rtp_ts = ((ntp_delta+ts_delta)*((int)audio->stream_cfg.clock_rate/100))/10; /* calculate rtp_ts that shall bring audio back to 0 */
    timestamp_offset = audio->sr_timestamp + rtp_ts  - (uint32_t)(audio->stream_cfg.clock_rate*B_FIRST_TIMESTAMP_DELAY) ;
    BDBG_WRN(("%s: delta:(%d) ntp:%d ts:%d, rtp_ts=%u(%u) timestamp_offset=%u (audio ts:%d)", __FUNCTION__, ntp_delta+ts_delta, ntp_delta, ts_delta, rtp_ts, audio->sr_timestamp, timestamp_offset, (brtp_delta_time(audio->rtp_timestamp, audio->sr_timestamp)*125)/((int)audio->stream_cfg.clock_rate/16)));
    bRtpStartRtpParser(audio, timestamp_offset);
    return;
}

static int bRtpEsSessionSetup(
    lm_session_t         session,
    brtp_parser_mux_t    mux,
    blm_media            media,
    struct brtp_channel *channels
    )
{
    const lm_stream_desc      *desc;
    struct brtp_channel       *chn = &channels[media];
    brtp_parser_h264_cfg       h264_cfg;
    brtp_parser_h263_cfg       h263_cfg;
    brtp_parser_mpeg4part2_cfg m4p2_cfg;
    brtp_parser_mpeg4_cfg      mpeg4_cfg;
    brtp_parser_aacloas_cfg    aacloas_cfg;
    brtp_parser_amr_cfg        amr_cfg;
    brtp_cfg                   cfg;
    lm_stream_connect_cfg      connect_cfg;

    desc = lm_get_stream_desc( session, media );

    // It is OK one type of media (video or audio) does not exist
    // Do not assert.
    if ( !desc ) {
        BDBG_ERR(("%s: lm_get_stream_desc failed for media (%d)", __FUNCTION__, media));
        return -1;
    }

    brtp_default_cfg( &cfg );
    switch (desc->stream_type)
    {
        case blm_stream_h264:
        {
            BDBG_ASSERT( media==blm_media_video );
            brtp_parser_h264_default_cfg( &h264_cfg );
            h264_cfg.meta = BKNI_Malloc( h264_cfg.meta_len );
            chn->parser   = brtp_parser_h264_create( &h264_cfg );
            break;
        }
        case blm_stream_h263:
        {
            BDBG_ASSERT( media==blm_media_video );
            brtp_parser_h263_default_cfg( &h263_cfg );
            h263_cfg.meta = BKNI_Malloc( h263_cfg.meta_len );
            chn->parser   = brtp_parser_h263_create( &h263_cfg );
            break;
        }
        case blm_stream_mpeg4part2:
        {
            BDBG_ASSERT( media==blm_media_video );
            brtp_parser_mpeg4part2_default_cfg( &m4p2_cfg );
            m4p2_cfg.meta_len = desc->cfg_len;
            m4p2_cfg.meta     = BKNI_Malloc( m4p2_cfg.meta_len );
            BKNI_Memcpy( m4p2_cfg.meta, desc->cfg, desc->cfg_len );
            chn->parser = brtp_parser_mpeg4part2_create( &m4p2_cfg );
            break;
        }
        case blm_stream_aac_hbr:
        case blm_stream_g711:
        {
            // G711 is implemented in brtp_parser_mpeg4 module
            BDBG_ASSERT( media==blm_media_audio );
            brtp_parser_mpeg4_default_cfg( &mpeg4_cfg );
            chn->parser = brtp_parser_mpeg4_create( &mpeg4_cfg );
            break;
        }
        case blm_stream_aac_loas:
        {
            BDBG_ASSERT( media==blm_media_audio );
            brtp_parser_aacloas_default_cfg( &aacloas_cfg );
            chn->parser = brtp_parser_aacloas_create( &aacloas_cfg );
            break;
        }
        case blm_stream_amr:
        {
            BDBG_ASSERT( media==blm_media_audio );
            brtp_parser_amr_default_cfg( &amr_cfg );
            chn->parser = brtp_parser_amr_create( &amr_cfg );
            break;
        }

        default:
            BDBG_ERR(( "%s: unknown stream type %d", __FUNCTION__, desc->stream_type ));
            return( -1 );
    } /* switch */
    BDBG_ASSERT( chn->parser );
    cfg.noparser_cntx = chn;
    cfg.noparser      = bRtpTimestampPkt;
    cfg.pkt_free      = b_pkt_free;
    cfg.wait_time     = 500; /* TODO: in msec, needs to do more tuning of this based on max bitrate */
    chn->mux          = mux;
    chn->rtp          = brtp_create( &cfg );
    if (desc->stream_type == blm_stream_h264)
        chn->h264_cfg     = h264_cfg;

    chn->cfg_len               = desc->cfg_len;
    chn->stream_cfg.clock_rate = desc->clock_rate;
    chn->rtp_format            = desc->rtp_format;
    BKNI_Memcpy( chn->cfg, desc->cfg, desc->cfg_len );
    BDBG_ASSERT( chn->rtp );
    lm_stream_default_cfg( &connect_cfg );
    connect_cfg.rtp      = chn;
    connect_cfg.rtp_data = bRtpFeedPktToRtpParser;
    connect_cfg.rtcp_sr = bRtpSyncOne;
    connect_cfg.stream_cntx = chn;
    chn->lm_stream          = lm_session_stream( session, media, &connect_cfg );
    BDBG_ASSERT( chn->lm_stream );
    return( 0 );
} /* bRtpEsSessionSetup */

static int bRtpSessionSetup(
    B_PlaybackIpHandle playback_ip
    )
{
    brtp_parser_mux_t    mux;
    struct brtp_channel *channels;
    lm_state_t           lm;
    lm_session_t         session;
    char                 url[256];

    channels = playback_ip->channels;
    mux      = channels[blm_media_video].mux;
    /* TODO: dynamically allocate url */
    snprintf( url, sizeof( url )-1,
        "rtsp://"
        "%s:%d%s",
        playback_ip->openSettings.socketOpenSettings.ipAddr,
        playback_ip->openSettings.socketOpenSettings.port,
        playback_ip->openSettings.socketOpenSettings.url );

    lm      = lm_init();
    session = lm_connect( lm, url );
    if ( !session )
    {
        BDBG_MSG(("%s: Failed to setup the RTSP Session w/ server for url %s", __FUNCTION__, url));
        goto error;
    }
    /* RTSP describe is complete at this point and we have the media info */

    bRtpEsSessionSetup( session, mux, blm_media_video, channels );
    bRtpEsSessionSetup( session, mux, blm_media_audio, channels );

    playback_ip->lm        = lm;
    playback_ip->lmSession = session;
    BDBG_WRN(( "lm %p, lmSession %p setup done", (void *)playback_ip->lm, (void *)playback_ip->lmSession ));
    return 0;
error:
    if (lm) lm_shutdown(lm);
    brtpClose( playback_ip );
    return -1;
} /* bRtpSessionSetup */

/*
   Function does following:
   -sdf
   -sdf
*/
B_PlaybackIpError B_PlaybackIp_RtspSessionSetup(
    B_PlaybackIpHandle                playback_ip,
    B_PlaybackIpSessionSetupSettings *setupSettings,
    B_PlaybackIpSessionSetupStatus   *setupStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;
    const lm_stream_desc *audiodesc, *videodesc;

    if (!playback_ip || !setupSettings || !setupStatus)
    {
        BDBG_ERR(( "%s: invalid params, playback_ip %p, setupSettings %p, setupStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)setupSettings, (void *)setupStatus ));
        return( B_ERROR_INVALID_PARAMETER );
    }

    if (setupSettings->u.rtsp.additionalHeaders && !strstr( setupSettings->u.rtsp.additionalHeaders, "\r\n" ))
    {
        BDBG_ERR(( "%s: additional RTSP header is NOT properly terminated (missing \\r\\n), header is: %s\n", __FUNCTION__, setupSettings->u.rtsp.additionalHeaders ));
        errorCode = B_ERROR_INVALID_PARAMETER;
        goto error;
    }

    /* if SessionSetup is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
    {
        return( B_ERROR_IN_PROGRESS );
    }

    /* if SessionSetup is completed, return results to app */
    if (playback_ip->apiCompleted)
    {
        BDBG_WRN(( "%s: previously started session setup operation completed, playback_ip %p\n", __FUNCTION__, (void *)playback_ip ));
        /* Note: since this api was run in a separate thread, we defer thread cleanup until the Ip_Start */
        /* as this call to read up the session status may be invoked in the context of this thread via the callback */
        goto done;
    }

    /* Neither SessionSetup is in progress nor it is completed, so start setup */
    playback_ip->apiInProgress = true;
    playback_ip->apiCompleted  = false;
    memset( &playback_ip->setupStatus, 0, sizeof( playback_ip->setupStatus ));

    if (bRtpSessionSetup( playback_ip ) < 0)
    {
        BDBG_WRN(( "%s: Failed to setup RTSP session for playback_ip %p\n", __FUNCTION__, (void *)playback_ip ));
        return (B_ERROR_PROTO);
    }
done:
    errorCode           = B_ERROR_SUCCESS;
    setupStatus->u.rtsp = playback_ip->setupStatus.u.rtsp;

    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted  = false;

    videodesc = lm_get_stream_desc(playback_ip->lmSession, blm_media_video);
    audiodesc = lm_get_stream_desc(playback_ip->lmSession, blm_media_audio);
    // Do not assert. It is OK to have only audio or video.
    if ( !videodesc && !audiodesc ) {
        BDBG_ERR(("%s: neither video nor audio exists.", __FUNCTION__));
        goto error;
    }

    if ( videodesc )
    {
        switch(videodesc->stream_type)
        {
            case blm_stream_h264:
            {
                setupStatus->u.rtsp.psi.videoCodec =  NEXUS_VideoCodec_eH264;
                break;
            }
            case blm_stream_h263:
            {
                setupStatus->u.rtsp.psi.videoCodec =  NEXUS_VideoCodec_eH263;
                break;
            }
            case blm_stream_mpeg4part2:
            {
                setupStatus->u.rtsp.psi.videoCodec =  NEXUS_VideoCodec_eMpeg4Part2;
                break;
            }
            default:
            {
                setupStatus->u.rtsp.psi.videoCodec =  NEXUS_VideoCodec_eH264;
                BDBG_ERR(( "%s: unknown stream type %d", __FUNCTION__, videodesc->stream_type ));
            }
        } /* switch */

        setupStatus->u.rtsp.psi.videoPid    = 0xe0;
        setupStatus->u.rtsp.psi.pcrPid      = 0xe0;
        setupStatus->u.rtsp.psi.mpegType    = NEXUS_TransportType_eMpeg2Pes;
        setupStatus->u.rtsp.psi.psiValid    = true;
        setupStatus->u.rtsp.psi.duration = videodesc->duration*1000; /* in msec */
        setupStatus->u.rtsp.psi.videoWidth = videodesc->videoWidth;
        setupStatus->u.rtsp.psi.videoHeight = videodesc->videoHeight;
        BDBG_WRN(("%s: duration %d, w %d, h %d", __FUNCTION__, videodesc->duration, videodesc->videoWidth, videodesc->videoHeight));
    }

    if ( audiodesc )
    {
        switch(audiodesc->stream_type)
        {
            case blm_stream_g711:
                setupStatus->u.rtsp.psi.audioCodec     =  NEXUS_AudioCodec_eG711;
                BDBG_WRN(("%s: blm_stream_g711 use codec:%d", __FUNCTION__, setupStatus->u.rtsp.psi.audioCodec ));
                setupStatus->u.rtsp.psi.mpegType = NEXUS_TransportType_eMpeg2Pes;
                setupStatus->u.rtsp.psi.audioBitsPerSample = 8;
                setupStatus->u.rtsp.psi.audioSampleRate = 8000;
                setupStatus->u.rtsp.psi.audioNumChannels = 1;
                break;
            case blm_stream_aac_hbr:
            {
                setupStatus->u.rtsp.psi.audioCodec =  NEXUS_AudioCodec_eAac;
                break;
            }
            case blm_stream_aac_loas:
            {
                setupStatus->u.rtsp.psi.audioCodec = NEXUS_AudioCodec_eAacPlusLoas;
                break;
            }
            case blm_stream_amr:
            {
                setupStatus->u.rtsp.psi.audioCodec = NEXUS_AudioCodec_eAmr;
                break;
            }
            default:
            {
                setupStatus->u.rtsp.psi.audioCodec =  NEXUS_AudioCodec_eAac;
                BDBG_ERR(( "%s: unknown stream type %d", __FUNCTION__, audiodesc->stream_type ));
            }
        } /* switch */
        setupStatus->u.rtsp.psi.audioPid = 0xc0;
    }
    return( errorCode );

error:
    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted  = false;
    return( errorCode );
} /* B_PlaybackIp_RtspSessionSetup */

void B_PlaybackIp_RtpEsProcessing(
    void *data
    )
{
    int                rc          = 0;
    B_PlaybackIpHandle playback_ip = (B_PlaybackIpHandle)data;

    BDBG_WRN(( "%s: lm %p, lmSession %p, lmStop %d", __FUNCTION__, (void *)playback_ip->lm, (void *)playback_ip->lmSession, playback_ip->lmStop ));

    // lmStop might already be set to 1 before execution gets here if user keeps pressing start and stop. Check it before proceeding.
    if (playback_ip->lmStop != 0)
    {
        BDBG_WRN(( "%s: lmStop (%d) not 0. Do not start lm_session_play", __FUNCTION__, playback_ip->lmStop ));
        goto done;
    }

    rc = lm_session_play( playback_ip->lmSession );

    // Proceed only when it is not being stopped.
    if (( playback_ip->lmStop != 0 ) || ( rc != 0 ))
    {
        BDBG_WRN(( "%s: lmStop (%d) not 0 or rc (%d) not 0. Do not start lm_run", __FUNCTION__, playback_ip->lmStop, rc ));
        goto done;
    }

    lm_run( playback_ip->lm, &playback_ip->lmStop ); /* doesn't return */

    BDBG_WRN(( "%s: lm_run finished", __FUNCTION__ ));
    if (playback_ip->playback_state != B_PlaybackIpState_eStopping)
    {
        BDBG_WRN(( "%s: stopping rtsp es processing even though playback_ip state (%d) is not stopping", __FUNCTION__, playback_ip->playback_state ));
    }

    BKNI_Sleep( 200 );
    bRtpStoptRtpParser( &playback_ip->channels[blm_media_video] ); // RtpParser started after lm_run
    bRtpStoptRtpParser( &playback_ip->channels[blm_media_audio] ); // RtpParser started after lm_run

done:
    if (playback_ip->lmSession) {lm_session_close( playback_ip->lmSession ); }
    if (playback_ip->lm) {lm_shutdown( playback_ip->lm ); }
    brtpClose( playback_ip );
    BKNI_SetEvent( playback_ip->playback_halt_event );
    BDBG_MSG(( "%s: finished", __FUNCTION__ ));
} /* B_PlaybackIp_RtpEsProcessing */

void B_PlaybackIp_RtspSessionStop(
    B_PlaybackIpHandle playback_ip
    )
{
    playback_ip->lmStop = 1;
}

B_PlaybackIpError B_PlaybackIp_RtspSessionStart(
    B_PlaybackIpHandle                playback_ip,
    B_PlaybackIpSessionStartSettings *startSettings,
    B_PlaybackIpSessionStartStatus   *startStatus /* [out] */
    )
{
    B_PlaybackIpError errorCode = B_ERROR_PROTO;

#if 1
    B_ThreadSettings settingsThread;
    char            *threadName;
#endif

    if (!playback_ip || !startSettings || !startStatus)
    {
        BDBG_ERR(( "%s: invalid params, playback_ip %p, startSettings %p, startStatus %p\n", __FUNCTION__, (void *)playback_ip, (void *)startSettings, (void *)startStatus ));
        return( B_ERROR_INVALID_PARAMETER );
    }

    /* if SessionStart is in progress, return INCOMPLETE */
    if (playback_ip->apiInProgress)
    {
        return( B_ERROR_IN_PROGRESS );
    }

    /* if SessionStart is completed, return results to app */
    if (playback_ip->apiCompleted)
    {
        BDBG_WRN(( "%s: previously started session start operation completed, playback_ip %p\n", __FUNCTION__, (void *)playback_ip ));
        /* Note: since this api was run in a separate thread, we defer thread cleanup until the Ip_Start */
        /* as this call to read up the session status may be invoked in the context of this thread via the callback */
        goto done;
    }

    BDBG_MSG(( "%s: RTSP Media Transport Protocol: %s, position start %d, end %d, keepAliveInterval %d",
               __FUNCTION__, startSettings->u.rtsp.mediaTransportProtocol == B_PlaybackIpProtocol_eUdp ? "UDP" : "RTP",
               (int)startSettings->u.rtsp.start,
               (int)startSettings->u.rtsp.end,
               startSettings->u.rtsp.keepAliveInterval
               ));
    /* Neither SessionStart is in progress nor it is completed, so start session */
    playback_ip->apiInProgress = true;
    playback_ip->apiCompleted  = false;
    memset( &playback_ip->startStatus, 0, sizeof( playback_ip->startStatus ));

    if (B_PlaybackIp_UtilsWaitForPlaypumpDecoderSetup( playback_ip ))
    {
        goto error;
    }
#if 0
    B_PlaybackIp_RtpEsProcessing( playback_ip );
#else
    threadName = "PlaybackIpRtpEsThread";
    /* create thread to process incoming IP packets & feed them to PB hw */
    B_Thread_GetDefaultSettings( &settingsThread );
    playback_ip->playbackIpThread = B_Thread_Create( threadName, (B_ThreadFunc)B_PlaybackIp_RtpEsProcessing, (void *)playback_ip, &settingsThread );
    if (NULL == playback_ip->playbackIpThread)
    {
        BDBG_ERR(( "%s: Failed to create the %s thread for RTP media transport protocol\n", __FUNCTION__, threadName ));
        goto error;
    }
#endif /* if 0 */
done:
    startStatus->u.rtsp = playback_ip->startStatus.u.rtsp;

    errorCode = B_ERROR_SUCCESS;
    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted  = false;
    return( errorCode );

error:
    playback_ip->apiInProgress = false;
    playback_ip->apiCompleted  = false;
    return( errorCode );
} /* B_PlaybackIp_RtspSessionStart */

#endif /* LINUX || VxWorks */
