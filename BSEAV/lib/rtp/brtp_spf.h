/******************************************************************************
 * (c) 2007-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#ifndef _BRTP_SPF_H__
#define _BRTP_SPF_H__

#include "blst_squeue.h"
#include "bioatom.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct brtp_stats
{
    uint32_t packetsReceived;                   /* total IP/UDP/RTP packets received */
    uint64_t bytesReceived;                     /* total AV Payload bytes received in RTP Packets */
    uint32_t packetsDiscarded;                  /* total packets discarded: */
    uint32_t packetsOutOfSequence;              /* total packets whose RTP sequence doesn't match the expected next sequence */
    uint32_t packetsLost;                       /* total packets that were lost */
    uint32_t packetsLostBeforeErrorCorrection;  /* */
    uint32_t lossEvents;                        /* total number of times an expected packet was lost */
    uint32_t lossEventsBeforeErrorCorrection;   /* */
} brtp_stats;

#if (SPF_SUPPORT==n)
/* this is brtp_spf.h@@/main/4 */
/* RTP parser object */
typedef struct brtp *brtp_t;

/* RTP packet object */
typedef struct brtp_packet *brtp_packet_t;

/* configuration for the RTP parser */
typedef struct brtp_cfg {
    unsigned npackets; /* maximum number of allocated packets */
    unsigned wait_time; /* maximun time, that packet could stay in the reordering queue, ms */
    batom_factory_t factory;
    int (*copy_to_user)(void *dst, void *src, unsigned size);
} brtp_cfg;

typedef struct brtp_session_cfg {
    uint8_t pt_mask;
    uint8_t pt_value;
    uint32_t ssrc_mask;
    uint32_t ssrc_value;
} brtp_session_cfg;

typedef struct brtp_pkt_info {
    uint32_t ssrc;
    uint32_t timestamp;
    uint16_t sequence;
    uint16_t packet_size;
} brtp_pkt_info;

typedef enum brtp_enqueue {brtp_enqueue_queued, brtp_enqueue_invalid, brtp_enqueue_discard, brtp_enqueue_overflow} brtp_enqueue;

/*
Summary:
    Get the default config for the brtp module
*/
void brtp_default_cfg(brtp_cfg *cfg);

/*
Summary:
    Create the brtp module instance
*/
brtp_t brtp_create(const brtp_cfg *cfg);

/*
Summary:
    Destroy the brtp module instance
*/
void brtp_destroy(brtp_t rtp);

/*
Summary:
    Get the default brtp session config
*/
void brtp_default_session_cfg(brtp_t rtp, brtp_session_cfg *cfg);

/*
Summary:
    Start brtp processing with particular session settings, using the provided output pipe
*/
void brtp_start(brtp_t rtp, batom_pipe_t pipe, const brtp_session_cfg *cfg);

/*
Summary:
    Stop the brtp session
*/
void brtp_stop(brtp_t rtp);

/*
Summary:
    Feed data to the particular rtp session, using the pipe provided as the source of data
*/
brtp_enqueue brtp_feed(brtp_t rtp, batom_pipe_t pipe_in);

/*
Summary:
    Gets packet info needed for RTCP stats
*/
int brtp_get_pkt_info(brtp_t rtp, brtp_pkt_info *info, size_t max_entries);

/*
Summary:
    Gets packet info needed for RTCP stats
*/
int brtp_get_stats(brtp_t rtp, brtp_stats *stats);
#else
/* this is brtp_spf.h@@/main/3 */
/* RTP parser object */
typedef struct brtp_spf *brtp_spf_t;

/* configuration for the RTP parser */
typedef struct brtp_spf_cfg {
    unsigned npackets; /* maximum number of allocated packets */
    unsigned wait_time; /* maximun time, that packet could stay in the reordering queue, ms */
    batom_factory_t factory;
    int (*copy_to_user)(void *dst, void *src, unsigned size);
} brtp_spf_cfg;

typedef struct brtp_spf_session_cfg {
    uint8_t pt_mask;
    uint8_t pt_value;
    uint32_t ssrc_mask;
    uint32_t ssrc_value;
} brtp_spf_session_cfg;

typedef struct brtp_spf_pkt_info {
    uint32_t ssrc;
    uint32_t timestamp;
    uint16_t sequence;
    uint16_t packet_size;
} brtp_spf_pkt_info;

typedef enum brtp_spf_enqueue {brtp_spf_enqueue_queued, brtp_spf_enqueue_invalid, brtp_spf_enqueue_discard, brtp_spf_enqueue_overflow} brtp_spf_enqueue;

/*
Summary:
    Get the default config for the brtp_spf module
*/
void brtp_spf_default_cfg(brtp_spf_cfg *cfg);

/*
Summary:
    Create the brtp_spf module instance
*/
brtp_spf_t brtp_spf_create(const brtp_spf_cfg *cfg);

/*
Summary:
    Destroy the brtp_spf module instance
*/
void brtp_spf_destroy(brtp_spf_t rtp);

/*
Summary:
    Get the default brtp_spf session config
*/
void brtp_spf_default_session_cfg(brtp_spf_t rtp, brtp_spf_session_cfg *cfg);

/*
Summary:
    Start brtp_spf processing with particular session settings, using the provided output pipe
*/
void brtp_spf_start(brtp_spf_t rtp, batom_pipe_t pipe, const brtp_spf_session_cfg *cfg);

/*
Summary:
    Stop the brtp_spf session
*/
void brtp_spf_stop(brtp_spf_t rtp);

/*
Summary:
    Feed data to the particular rtp session, using the pipe provided as the source of data
*/
brtp_spf_enqueue brtp_spf_feed(brtp_spf_t rtp, batom_pipe_t pipe_in);

/*
Summary:
    Gets packet info needed for RTCP stats
*/
int brtp_spf_get_pkt_info(brtp_spf_t rtp, brtp_spf_pkt_info *info, size_t max_entries);

/*
Summary:
    Gets packet info needed for RTCP stats
*/
int brtp_get_stats(brtp_spf_t rtp, brtp_stats *stats);
#endif

#ifdef __cplusplus
}
#endif


#endif /* _BRTP_SPF_H__ */
