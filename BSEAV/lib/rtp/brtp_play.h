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
#ifndef _BRTP_PLAY_H__
#define _BRTP_PLAY_H__

#include "brtp_parser.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define B_FIRST_TIMESTAMP_DELAY 0 /* delay of first timestamp, in seconds */

struct brtp_play_stream_config {
	void *stream_cntx;
	void (*data)(void *stream_cntx, brtp_io_block *block, uint32_t pts);
	void (*sync)(void *stream_cntx, const brtp_parser_mux_stream_cfg *stream_cfg, const brtp_ntp *ntp, uint32_t rtcp_timestamp);
};

typedef struct brtp_play_config {
	struct brtp_play_stream_config video;
	struct brtp_play_stream_config audio;
	void *play_cntx;
	void (*timer)(void *play_cntx);
	void (*end_of_stream)(void *play_cntx);
	bool use_tcp;
}brtp_play_config;

typedef struct brtp_source *brtp_source_t;
typedef struct brtp_play *brtp_play_t;

brtp_play_t brtp_play_create(void);
void brtp_play_default_cfg(brtp_play_t play, brtp_play_config *config);
brtp_source_t brtp_play_connect(brtp_play_t play, const char *url);
int brtp_play_start(brtp_play_t play, brtp_source_t source, const brtp_play_config *config);
int brtp_play_stop(brtp_play_t play);
void brtp_play_destroy(brtp_play_t play);


#ifdef __cplusplus
}
#endif

#endif /* _BRTP_PLAY_H__ */

