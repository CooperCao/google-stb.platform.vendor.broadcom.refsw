/***************************************************************************
 *     Copyright (c) 2007-2009, Broadcom Corporation
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
 * MPEG-2 PES Parser/Demux library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef __BMPEG2PES_PARSER_H__
#define __BMPEG2PES_PARSER_H__


#include "bmpeg2ts_parser.h"
#ifdef __cplusplus
extern "C"
{
#endif

/* must match  data_alignment_indicator from table 2-17 ISO/IEC 13818-1  */
#define BMPEG2PES_DATA_ALIGMENT (1<<18)

/* #define BMPEG2PES_PAYLOAD_END	(1<<15)  */
#define BMPEG2PES_PTS_VALID (1<<16)
#define BMPEG2PES_DISCONTINUITY  (1<<17)

#define BMPEG2PES_PTS_UNITS	45000

typedef struct bmpeg2pes_parser bmpeg2pes_parser;

typedef struct bmpeg2pes_atom_info {
	uint32_t flags; /* flags that are accumulated from the transport layer */
	uint32_t pts; /* pts from the header */
	unsigned data_offset; /* data offset into the pes header */
	uint8_t pes_id; /* stream ID, from the PES header */
} bmpeg2pes_atom_info;

typedef enum bmpeg2pes_stream_id_type {bmpeg2pes_stream_id_type_data, bmpeg2pes_stream_id_type_raw, bmpeg2pes_stream_id_type_invalid} bmpeg2pes_stream_id_type;
typedef enum bmpeg2pes_parser_state {bmpeg2pes_parser_state_sync, bmpeg2pes_parser_state_hdr, bmpeg2pes_parser_state_data} bmpeg2pes_parser_state;

struct bmpeg2pes_parser {
	bmpeg2pes_parser_state state;
	batom_accum_t pes_header;
	uint16_t data_len; /* PES packet header */
	uint16_t pkt_len; /* PES packet length */
	uint8_t id; /* stream ID */
	bool hold_enable; /* consumer could set this bit to hold consumption of es data */
	bmpeg2pes_atom_info info;
	void *packet_cnxt;
	void (*packet)(void *packet_cnxt, batom_accum_t src, batom_cursor *payload_start, size_t len, const bmpeg2pes_atom_info *info);
};

typedef struct bmpeg2pes_parser_stream bmpeg2pes_parser_stream;

struct bmpeg2pes_parser_stream {
	  uint16_t stream_id;
	  uint32_t flags;
	  unsigned npackets;
	  uint32_t pts;
	  bmpeg2ts_parser_action (*payload)(bmpeg2pes_parser_stream *stream, unsigned flags, batom_accum_t src, batom_cursor *cursor, size_t len);
};

#define BMPEG2PES_ID_ANY	0

int bmpeg2pes_parser_init(batom_factory_t factory, bmpeg2pes_parser *pes, uint8_t id);
void bmpeg2pes_parser_reset(bmpeg2pes_parser *pes);
void bmpeg2pes_parser_shutdown(bmpeg2pes_parser *pes);
bmpeg2ts_parser_action bmpeg2pes_parser_feed(bmpeg2pes_parser *pes, unsigned ts_flags, batom_accum_t src, batom_cursor *payload_start, size_t len);
void bmpeg2pes_parser_flush(bmpeg2pes_parser *pes);

bmpeg2pes_stream_id_type bmpeg2pes_decode_stream_id(unsigned stream_id);

typedef struct bmpeg2pes_demux  *bmpeg2pes_demux_t;

typedef struct bmpeg2pes_demux_config {
	void *packet_cnxt;
	void (*packet)(void *packet_cnxt, batom_accum_t src, batom_cursor *payload_start, size_t len, const bmpeg2pes_atom_info *info);
} bmpeg2pes_demux_config;

typedef struct bmpeg2pes_demux_status {
	unsigned nresyncs;
        uint64_t data_offset;
} bmpeg2pes_demux_status;

void bmpeg2pes_demux_default_config(bmpeg2pes_demux_config *config);
bmpeg2pes_demux_t bmpeg2pes_demux_create(batom_factory_t factory, const bmpeg2pes_demux_config *config);
void bmpeg2pes_demux_get_status(bmpeg2pes_demux_t demux, bmpeg2pes_demux_status *status);
void bmpeg2pes_demux_reset(bmpeg2pes_demux_t demux);
void bmpeg2pes_demux_destroy(bmpeg2pes_demux_t demux);
size_t bmpeg2pes_demux_feed(bmpeg2pes_demux_t pes, batom_pipe_t pipe);
void bmpeg2pes_demux_seek(bmpeg2pes_demux_t demux, uint64_t offset);

void bmpeg2pes_parser_stream_init(bmpeg2pes_parser_stream *stream, uint16_t stream_id);
int bmpeg2pes_parser_stream_feed(bmpeg2pes_parser_stream *stream, batom_accum_t src, batom_cursor *cursor);

#ifdef __cplusplus
}
#endif


#endif  /* __BMPEG2PES_PARSER_H__ */

