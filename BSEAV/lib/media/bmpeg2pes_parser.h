/***************************************************************************
 *  Copyright (C) 2007-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 * MPEG-2 PES Parser/Demux library
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
#define BMPEG2PES_DTS_VALID (1<<18)

#define BMPEG2PES_PTS_UNITS	45000

typedef struct bmpeg2pes_parser bmpeg2pes_parser;

typedef struct bmpeg2pes_atom_info {
	uint32_t flags; /* flags that are accumulated from the transport layer */
	uint32_t pts; /* PTS from the header */
	uint32_t dts; /* DTS from the header */
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

