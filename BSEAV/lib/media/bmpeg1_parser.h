/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
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
 * MPEG1 System stream parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BMPEG1_PARSER_H__
#define _BMPEG1_PARSER_H__

#include "bmedia_pes.h"
#include "blst_slist.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bmpeg1_parser *bmpeg1_parser_t;

typedef enum bmpeg1_parser_action {
	bmpeg1_parser_action_none, /* no special action needed to perform */
	bmpeg1_parser_action_return /* MPEG1 parser shall return control as soon as possible */
} bmpeg1_parser_action;


typedef struct bmpeg1_parser_handler bmpeg1_parser_handler;
typedef	bmpeg1_parser_action (*bmpeg1_parser_handler_cb)(
		bmpeg1_parser_handler *handler, /* pointer passed ot bmpeg1_install_handler function */
		unsigned stream_id, /* The stream id of the packet */
		batom_t packet/* packet payload */
		);

/* type for the user defined packet parser */
struct bmpeg1_parser_handler {
	BLST_S_ENTRY(bmpeg1_parser_handler) link; /* field that is used to link handlers together */
	uint16_t stream_id;
	uint16_t stream_id_mask;
	bmpeg1_parser_handler_cb handler; /* handler to call when packet with specfied ID is found */
};

typedef struct bmpeg1_parser_cfg {
	unsigned unused;
} bmpeg1_parser_cfg;


#define BMPEG1_RESULT_EOS			(-1)
#define BMPEG1_RESULT_HOLD		(-2)
#define BMPEG1_RESULT_SYNC_ERROR 	(-3)

typedef struct bmpeg1_parser_stream bmpeg1_parser_stream;

struct bmpeg1_parser_stream {
	  uint16_t stream_id;
	  uint32_t flags;	  
	  uint32_t pts;
          bmpeg1_parser_action (*payload)(bmpeg1_parser_stream *stream, unsigned flags, batom_accum_t src, batom_cursor *cursor, size_t len);
};

size_t bmpeg1_parser_feed(bmpeg1_parser_t parser, batom_pipe_t pipe);
void bmpeg1_parser_default_cfg(bmpeg1_parser_cfg *cfg);
bmpeg1_parser_t bmpeg1_parser_create(batom_factory_t factory, const bmpeg1_parser_cfg *cfg);
void bmpeg1_parser_destroy(bmpeg1_parser_t parser);
void bmpeg1_parser_reset(bmpeg1_parser_t parser);
void bmpeg1_parser_install_handler(bmpeg1_parser_t parser, bmpeg1_parser_handler *handler, uint16_t stream_id, uint16_t stream_id_mask, bmpeg1_parser_handler_cb packet_handler);
void bmpeg1_parser_remove_handler(bmpeg1_parser_t parser, bmpeg1_parser_handler *handler);
bool b_mpeg1_test_stream_id(unsigned stream_id);

void bmpeg1_parser_stream_init(bmpeg1_parser_stream *stream, uint16_t stream_id);
int bmpeg1_parser_stream_feed(bmpeg1_parser_stream *stream, batom_accum_t src, batom_cursor *cursor);

#ifdef __cplusplus
}
#endif

#endif /* _BMPEG1_PARSER_H__ */

