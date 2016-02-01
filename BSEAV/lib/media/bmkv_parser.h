/***************************************************************************
 *     Copyright (c) 2007-2008, Broadcom Corporation
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
 * MKV container parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMKV_PARSER_H__
#define _BMKV_PARSER_H__

#include "blst_slist.h"
#include "bmkv_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct bmkv_parser *bmkv_parser_t;

typedef enum bmkv_parser_action {
	bmkv_parser_action_none, /* no special action needed to perform */
	bmkv_parser_action_return, /* MKV parser shall return control as soon as possible */
	bmkv_parser_action_discard /* this object shall be discarded */
} bmkv_parser_action;

/* forward declaration of the bmkv_parser_handler structure */
typedef struct bmkv_parser_handler bmkv_parser_handler;

/* user defined element handler function */
typedef	bmkv_parser_action (*bmkv_parser_handler_cb)(
		bmkv_parser_handler *handler, /* pointer passed ot bmkv_parser_install_handler function */
		bmkv_id id, /* The ID of the element */
		batom_t element /* element payload */
		);

/* user defined element handler function */
typedef	bmkv_parser_action (*bmkv_parser_element_begin_cb)(
		void *cntx, /* user context */
		bmkv_id id, /* The element ID */
		bmkv_size size, 	/* element size */
		uint64_t offset	/* offset in the stream */
		);

typedef	bmkv_parser_action (*bmkv_parser_element_end_cb)(
		void *cntx, /* user context */
		bmkv_id id, /* The element ID */
		uint64_t offset	/* offset in the stream */
		);

/* application level configuration for the MKV parser */
typedef struct bmkv_parser_cfg {
	void *user_cntx; /* user context */
	bmkv_parser_element_begin_cb element_begin; /* user handler that called at the beginning of element*/
	bmkv_parser_element_end_cb element_end;  /* user handler that called at the end of element */
} bmkv_parser_cfg;

/* type for user defined element parser */
struct bmkv_parser_handler {
	BLST_S_ENTRY(bmkv_parser_handler) link; /* field that is used to link handlers together */
	bmkv_id id;	/* ID of the element */
	bmkv_parser_handler_cb handler; /* handler to call when element with specfied type is found */
};

typedef struct bmkv_parser_status {
	uint64_t offset; /* current offset in the stream */
	size_t acc_length; /* number of accumulated bytes */
	size_t element_length; /* size of the element that parser expects */
	const char *state; /* state of the MKV parser */
} bmkv_parser_status;


void bmkv_parser_install_handler(bmkv_parser_t mkv, bmkv_parser_handler *handler, uint32_t type, bmkv_parser_handler_cb element_handler);
void bmkv_parser_remove_handler(bmkv_parser_t mkv, bmkv_parser_handler *handler);
void bmkv_parser_get_status(bmkv_parser_t mkv, bmkv_parser_status *status);
void bmkv_parser_reset(bmkv_parser_t mkv);
int bmkv_parser_seek(bmkv_parser_t mkv, uint64_t off);
bmkv_parser_t bmkv_parser_create(batom_factory_t factory, const bmkv_parser_cfg *cfg);
void bmkv_parser_default_cfg(bmkv_parser_cfg *cfg);
void bmkv_parser_destroy(bmkv_parser_t mkv);
size_t bmkv_parser_feed(bmkv_parser_t mkv, batom_pipe_t pipe);




#ifdef __cplusplus
}
#endif


#endif /* _BMKV_PARSER_H__ */

