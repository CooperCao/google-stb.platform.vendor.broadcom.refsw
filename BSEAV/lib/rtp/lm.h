/******************************************************************************
 * (c) 2014 Broadcom Corporation
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

#ifndef _LM_H_
#define _LM_H_

#include "lm_lgpl.h"

#ifdef __cplusplus
extern "C"
{
#endif

lm_state_t lm_init(void);
void lm_shutdown(lm_state_t lm);
void lm_run(lm_state_t lm, char *stop);
typedef struct lm_timer *lm_timer_t;
lm_timer_t lm_timer_schedulle(lm_state_t lm, unsigned delay /* ms */, void (*timer)(void *cntx), void *cntx);
void lm_timer_cancel(lm_state_t lm, lm_timer_t timer);

lm_session_t lm_connect(lm_state_t lm, const char *url);

const lm_stream_desc *lm_get_stream_desc(lm_session_t session, blm_media media);
typedef struct lm_stream_connect_cfg {
	void *rtp;
	void *stream_cntx;
	brtp_enqueue (*rtp_data)(void *rtp, void *pkt, size_t len, void *cookie);
	void (*rtcp_sr)(void *stream_cntx, uint32_t ntp_msw, uint32_t ntp_lsw, uint32_t timestamp_offset);
	void (*rtcp_bye)(void *stream_cntx);
	bool use_tcp;
	unsigned buffer_size;
} lm_stream_connect_cfg;

typedef struct lm_session_status {
	unsigned end_time; /* end_time in seconds */
} lm_session_status;

void lm_stream_default_cfg(lm_stream_connect_cfg *cfg);
lm_stream_t lm_session_stream(lm_session_t session, blm_media media, const lm_stream_connect_cfg *cfg);
int lm_session_play(lm_session_t session);
void lm_session_close(lm_session_t session);
void lm_stream_update_pkt_info(lm_stream_t stream, brtp_pkt_info *info, size_t nentries);
void lm_pkt_free(void *source_cnxt, void *pkt);
void lm_session_get_status(lm_session_t session, lm_session_status *status);


#ifdef __cplusplus
}
#endif

#endif /* _LM_H_ */
