/***************************************************************************
 *     Copyright (c) 2006, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * RTP parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BRTP_H__
#define _BRTP_H__

#include "blst_squeue.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* RTP parser object */
typedef struct brtp *brtp_t;
/* RTP stream parser object */
typedef struct brtp_parser *brtp_parser_t;
/* RTP packet object */
typedef struct brtp_packet *brtp_packet_t;

/* configuration for the RTP parser */
typedef struct {
	unsigned npackets; /* maximum number of allocated packets */
	unsigned wait_time; /* maximun time, that packet could stay in the reordering queue, ms */
	void *pkt_source;
	void (*pkt_free)(void *pkt_src, void *pkt_cookie);
	void *noparser_cntx;
	void (*noparser)(void *noparser_cntx, const void *pkt);
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

void brtp_default_cfg(brtp_cfg *cfg);
brtp_t brtp_create(const brtp_cfg *cfg);
void brtp_destroy(brtp_t rtp);
void brtp_stop(brtp_t rtp);
void brtp_default_session_cfg(brtp_t rtp, brtp_session_cfg *cfg);
brtp_enqueue brtp_enqueue_pkt(brtp_t rtp, const void *pkt, size_t pkt_len, void *cookie);
void brtp_start(brtp_t rtp, brtp_parser_t parser, const brtp_session_cfg *cfg);
void brtp_stop(brtp_t rtp);
int brtp_get_pkt_info(brtp_t rtp, brtp_pkt_info *info, size_t max_entries);


#ifdef __cplusplus
}
#endif

#endif /* _BRTP_H__ */

