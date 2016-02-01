/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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
 * MP4 container parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMP4_PARSER_H__
#define _BMP4_PARSER_H__

#include "blst_slist.h"
#include "bmp4_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bmp4_parser *bmp4_parser_t;

typedef enum bmp4_parser_action {
	bmp4_parser_action_none, /* no special action needed to perform */
	bmp4_parser_action_return, /* MP4 parser shall return control as soon as possible */
	bmp4_parser_action_discard /* this object shall be discarded */
} bmp4_parser_action;

/* forward declaration of the bmp4_parser_handler structure */
typedef struct bmp4_parser_handler bmp4_parser_handler;

/* user defined box handler function */
typedef	bmp4_parser_action (*bmp4_parser_handler_cb)(
		bmp4_parser_handler *handler, /* pointer passed ot bmp4_parser_install_handler function */
		uint32_t type, /* The TYPE of the box */
		batom_t box /* box payload */
		);

/* user defined box handler function */
typedef	bmp4_parser_action (*bmp4_parser_box_begin_cb)(
		void *cntx, /* user context */
		uint32_t type, /* The box type */
		uint64_t size, 	/* box size */
		uint64_t offset,	/* offset in the stream */
        size_t  header_size  /* size of the objects header */
		);

typedef	bmp4_parser_action (*bmp4_parser_box_end_cb)(
		void *cntx, /* user context */
		uint32_t type, /* The box type */
		uint64_t offset	/* offset in the stream */
		);

/* application level configuration for the MP4 parser */
typedef struct bmp4_parser_cfg {
	void *user_cntx; /* user context */
	bmp4_parser_box_begin_cb box_begin; /* user handler that called at the beginning of box*/
	bmp4_parser_box_end_cb box_end;  /* user handler that called at the end of box */
} bmp4_parser_cfg;

/* type for user defined box parser */
struct bmp4_parser_handler {
	BLST_S_ENTRY(bmp4_parser_handler) link; /* field that is used to link handlers together */
	uint32_t type;	/* type of the box */
	bmp4_parser_handler_cb handler; /* handler to call when box with specfied type is found */
};

typedef struct bmp4_parser_status {
	uint64_t offset; /* current offset in the stream */
	size_t acc_length; /* number of accumulated bytes */
	size_t box_length; /* size of the box that parser expects */
	const char *state; /* state of the MP4 parser */
} bmp4_parser_status;

void bmp4_parser_install_handler(bmp4_parser_t mp4, bmp4_parser_handler *handler, uint32_t type, bmp4_parser_handler_cb box_handler);
void bmp4_parser_remove_handler(bmp4_parser_t mp4, bmp4_parser_handler *handler);
void bmp4_parser_get_status(bmp4_parser_t mp4, bmp4_parser_status *status);
void bmp4_parser_reset(bmp4_parser_t mp4);
int bmp4_parser_seek(bmp4_parser_t mp4, uint64_t off);
bmp4_parser_t bmp4_parser_create(batom_factory_t factory, const bmp4_parser_cfg *cfg);
void bmp4_parser_default_cfg(bmp4_parser_cfg *cfg);
void bmp4_parser_destroy(bmp4_parser_t mp4);
size_t bmp4_parser_feed(bmp4_parser_t mp4, batom_pipe_t pipe);


#ifdef __cplusplus
}
#endif


#endif /* _BMP4_PARSER_H__ */

