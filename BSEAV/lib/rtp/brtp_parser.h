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
#ifndef _BRTP_PARSER_H__
#define _BRTP_PARSER_H__

#include "blst_squeue.h"
#include "btimestamp.h"
#include "brtp.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct brtp_parser_mux *brtp_parser_mux_t;

typedef struct brtp_parser_mux_stream_cfg {
	unsigned clock_rate; /* timestamp rate in kHz */
	unsigned pes_id;
	uint32_t clock_offset; /* stream timestamp that corresponds to the begining of this stream */
	void *sink_stream_cntx;
}brtp_parser_mux_stream_cfg;


typedef struct brtp_io_vec {
	BLST_SQ_ENTRY(brtp_io_vec) list;
	brtp_packet_t packet; /* parent packet or NULL */
	const void *data;
	uint32_t len;
} brtp_io_vec;

BLST_SQ_HEAD(brtp_io_block, brtp_io_vec);

typedef struct brtp_io_block brtp_io_block; 

struct brtp_parser_mux_stream {
	brtp_parser_mux_stream_cfg cfg;
	brtp_parser_mux_t mux;
	brtp_io_block acc;
	uint32_t last_timestamp;
	const void *header; /* stream specific header */
	size_t header_len; /* size of stream specific header data */
	struct btimestamp timestamp;
};

typedef struct brtp_parser_mux_stream *brtp_parser_mux_stream_t;
typedef enum  brtp_stream_type {
	brtp_stream_invalid,
	brtp_stream_video,
	brtp_stream_audio
} brtp_stream_type;

typedef struct brtp_parser_type {
	const char *name;
	brtp_stream_type type;
} brtp_parser_type;

/* Common methods for all RTP stream parsers */
struct brtp_parser {
	struct brtp_parser_mux_stream stream;
	BDBG_OBJECT(brtp_parser_t)
	void (*packet)(brtp_parser_t parser, brtp_packet_t pkt, const void *data, size_t len);
	void (*discontinuity)(brtp_parser_t parser);
	brtp_parser_mux_stream_t (*start)(brtp_parser_t parser, brtp_parser_mux_t mux, const brtp_parser_mux_stream_cfg *cfg, const void *parser_cfg, size_t parser_cfg_len);
	void (*stop)(brtp_parser_t parser);
	void (*destroy)(brtp_parser_t parser);
	const brtp_parser_type *type;
};


typedef enum {brtp_mux_mode_es, brtp_mux_mode_pes} brtp_mux_mode;

typedef struct brtp_parser_mux_cfg {
	brtp_mux_mode mode;
	void *data_sink_cntx;
	void (*data_sink)(void  *data_sink_cntx, brtp_io_block *block, brtp_parser_t parser, uint32_t pts);

	size_t n_iovec; /* number of I/O vectors */
	size_t n_ioblock; /* number of I/O blocks */
	size_t meta_len; /* size of allocated meta data */
	void *meta; /* pointer to buffer with meta data */
}brtp_parser_mux_cfg;

brtp_io_vec *brtp_parser_next(brtp_parser_t parser);
void brtp_parser_mux_default_cfg(brtp_parser_mux_cfg *cfg);
brtp_parser_mux_t brtp_parser_mux_create(const brtp_parser_mux_cfg *cfg);
void brtp_parser_mux_destroy(brtp_parser_mux_t mux);
void brtp_parser_mux_reset(brtp_parser_mux_t mux);
void brtp_parser_mux_attach(brtp_parser_mux_t mux, brtp_parser_t parser, const brtp_parser_mux_stream_cfg *cfg);
void brtp_parser_mux_stream_default_cfg(brtp_parser_mux_stream_cfg *cfg);
void brtp_parser_mux_recycle(brtp_parser_mux_t mux, brtp_io_block *io_block);


void brtp_parser_init(brtp_parser_t parser);
brtp_io_vec *brtp_parser(brtp_parser_t parser);
bool brtp_parser_begin(brtp_parser_t parser);
void brtp_parser_commit(brtp_parser_t parser);
void brtp_parser_abort(brtp_parser_t parser);
void brtp_parser_stop(brtp_parser_t parser);
bool brtp_parser_add_packet(brtp_parser_t parser, brtp_packet_t packet, const void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _BRTP_PARSER_H__ */

