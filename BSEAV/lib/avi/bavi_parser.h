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
 * AVI parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BAVI_PARSER_H__
#define _BAVI_PARSER_H__

#include "bavi_util.h"
#include "blst_slist.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef int64_t bavi_off_t;
typedef struct bavi_parser *bavi_parser_t;

typedef struct bavi_parser_status {
    bool data_discontinuity; /* set to true if some data from the stream has been dropped or lost since the last buffer was fed */
	bavi_off_t offset; /* current offset in a stream */
	size_t acc_length; /* number of accumulated bytes */
	size_t obj_length; /* size of the object that parser expects */
	const char *state; /* state of the AVI parser */
    bmedia_parsing_errors errors;
} bavi_parser_status;

typedef enum bavi_parser_action {
	bavi_parser_action_none, /* no special action needed to perform */
	bavi_parser_action_return /* AVI parser shall return control as soon as possible */
} bavi_parser_action;

/* forward declaration of the bavi_object_handler structure */
typedef struct bavi_parser_handler bavi_parser_handler;

/* user defined object handler function */
typedef	bavi_parser_action (*bavi_parser_handler_cb)(
		bavi_parser_handler *handler, /* pointer passed ot bavi_install_object_handler function */
		bavi_fourcc fourcc, /* The FOURCC of the object */
		batom_t object /* object payload */
		);

/* user defined object handler function */
typedef	bavi_parser_action (*bavi_parser_object_begin_cb)(
		void *cntx, /* user context */
		bavi_fourcc fourcc, /* The FOURCC of the object */
		bavi_dword size, 	/* object size */
		bavi_off_t offset	/* offset in the stream */
		);

typedef	bavi_parser_action (*bavi_parser_object_end_cb)(
		void *cntx, /* user context */
		bavi_fourcc fourcc, /* The FOURCC of the object */
		bavi_off_t offset	/* offset in the stream */
		);

/* application level configuration for the AVI parser */
typedef struct bavi_parser_cfg {
	void *user_cntx; /* user context */
	bavi_parser_object_begin_cb object_begin; /* user handler that called at the beginning of the LIST type object */
	bavi_parser_object_end_cb object_end;  /* user handler that called at the end of the LIST type object */
    void (*stream_error)(void *cntx); /* callback that was called when error detectect, this callback can't call back into the avi parser */
} bavi_parser_cfg;

typedef struct bavi_parser_payload_sink {
	void *sink_cntx; /* context passed into the application callbacks */
	void (*payload_flush)(void *sink_cntx); /* function called when there is a discontinuity in the AVI stream */
} bavi_parser_payload_sink;

/* type for user defined object parser */
struct bavi_parser_handler {
	BLST_S_ENTRY(bavi_parser_handler) link; /* field that is used to link handlers together */
	bavi_fourcc	fourcc;	/* FOURCC code of the object */
	bavi_parser_handler_cb handler; /* handler to call when object with specfied FOURCC is found */
};


bavi_parser_t bavi_parser_create(batom_factory_t factory, const bavi_parser_cfg *cfg);
void bavi_parser_reset(bavi_parser_t avi);
void bavi_parser_default_cfg(bavi_parser_cfg *cfg);
void bavi_parser_destroy(bavi_parser_t avi);
size_t bavi_parser_feed(bavi_parser_t avi, batom_pipe_t pipe);
void bavi_parser_reset(bavi_parser_t avi);
int bavi_parser_seek(bavi_parser_t avi, bavi_off_t off);
void bavi_parser_get_status(bavi_parser_t avi, bavi_parser_status *status);

#define BAVI_FOURCC_BEGIN      (bavi_fourcc)0
#define BAVI_FOURCC_END                (bavi_fourcc)(0xFFFFFFFFul)

/* this function is used to install user defined parser for the AVI object,
 * Size of such object is limited to 64K.
 * User could register more that one hander for each object, in later case order which what parsers called is undetermined. 
 */
void bavi_parser_install_handler(
		bavi_parser_t avi,  /* avi parser context */
		bavi_parser_handler *handler,  /* pointer to the user supplied handler, user responsible to keep that object intact untill call to the bavi_remove_object_handler */
		bavi_fourcc fourcc,  /* object id */
		bavi_parser_handler_cb object_handler /* object handler */
		);

/* this function is used to remove handler */
void bavi_parser_remove_handler(bavi_parser_t avi, bavi_parser_handler *handler);

/* this function shall be used to initialized basf_parser_payload_sink type */
void bavi_parser_default_payload_sink(bavi_parser_payload_sink *sink);

/* this function is used to install consumer for the ASF payload */
void bavi_parser_set_payload_sink(bavi_parser_t avi, const bavi_parser_payload_sink *sink);

#ifdef __cplusplus
}
#endif

#endif /* _BAVI_PARSER_H__ */

