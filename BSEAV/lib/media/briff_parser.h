/***************************************************************************
 *     Copyright (c) 2009-2012, Broadcom Corporation
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
 * RIFF parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BRIFF_PARSER_H__
#define _BRIFF_PARSER_H__

#include "blst_slist.h"
#include "bmedia_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define B_RIFF_RIFF 	BMEDIA_FOURCC('R','I','F','F')
/* type corresponding to the 4 bytes RIFF value */
typedef uint32_t briff_atom;
typedef briff_atom briff_chunk_id;

typedef int64_t briff_off_t;
typedef struct briff_parser *briff_parser_t;

typedef struct briff_parser_status {
    bool data_discontinuity; /* set to true if some data from the stream has been dropped or lost since the last buffer was fed */
	briff_off_t offset; /* current offset in a stream */
	size_t acc_length; /* number of accumulated bytes */
	size_t obj_length; /* size of the object that parser expects */
	const char *state; /* state of the RIFF parser */
    bmedia_parsing_errors errors;
} briff_parser_status;

typedef enum briff_parser_action {
	briff_parser_action_none, /* no special action needed to perform */
	briff_parser_action_return /* RIFF parser shall return control as soon as possible */
} briff_parser_action;

/* forward declaration of the briff_object_handler structure */
typedef struct briff_parser_handler briff_parser_handler;

/* user defined object handler function */
typedef	briff_parser_action (*briff_parser_handler_cb)(
		briff_parser_handler *handler, /* pointer passed to briff_install_object_handler function */
		briff_chunk_id chunk_id, /* The chunk id of the object */
		unsigned object_size, /* offset into object */
        unsigned object_offset, /* offset into the object */
		batom_t object /* object payload */
		);

/* user defined object handler function */
typedef	briff_parser_action (*briff_parser_object_begin_cb)(
		void *cntx, /* user context */
		briff_chunk_id chunk_id, /* The chunk id of the object */
		unsigned size, 	/* object size */
		briff_off_t offset	/* offset in the stream */
		);

typedef	briff_parser_action (*briff_parser_object_end_cb)(
		void *cntx, /* user context */
		briff_chunk_id chunk_id, /* The chunk id of the object */
		briff_off_t offset	/* offset in the stream */
		);

/* application level configuration for the RIFF parser */
typedef struct briff_parser_cfg {
	void *user_cntx; /* user context */
	briff_parser_object_begin_cb object_begin; /* user handler that called at the beginning of the LIST type object */
	briff_parser_object_end_cb object_end;  /* user handler that called at the end of the LIST type object */
    briff_parser_action (*file_type)(void *context, briff_chunk_id file_type);
    void (*stream_error)(void *cntx); /* callback that was called when error detectect, this callback can't call back into the riff parser */
    bool le_endian; /* true if file is little-endian */
    uint32_t top_id;
    uint32_t list_id;
} briff_parser_cfg;

/* type for user defined object parser */
struct briff_parser_handler {
	BLST_S_ENTRY(briff_parser_handler) link; /* field that is used to link handlers together */
	briff_chunk_id	chunk_id;	/* chunk id of the object */
	briff_parser_handler_cb handler; /* handler to call when object with specfied chunk id is found */
};


briff_parser_t briff_parser_create(batom_factory_t factory, const briff_parser_cfg *cfg);
void briff_parser_reset(briff_parser_t riff);
void briff_parser_default_cfg(briff_parser_cfg *cfg);
void briff_parser_destroy(briff_parser_t riff);
size_t briff_parser_feed(briff_parser_t riff, batom_pipe_t pipe);
void briff_parser_reset(briff_parser_t riff);
int briff_parser_seek(briff_parser_t riff, briff_off_t off);
void briff_parser_get_status(briff_parser_t riff, briff_parser_status *status);

#define BRIFF_CHUNK_ID_BEGIN      ((briff_chunk_id)0)
#define BRIFF_CHUNK_ID_END        ((briff_chunk_id)(0xFFFFFFFFul))

/* this function is used to install user defined parser for the RIFF object,
 * Size of such object is limited to 64K.
 * User could register more that one hander for each object, in later case order which what parsers called is undetermined. 
 */
void briff_parser_install_handler(
		briff_parser_t riff,  /* riff parser context */
		briff_parser_handler *handler,  /* pointer to the user supplied handler, user responsible to keep that object intact untill call to the briff_remove_object_handler */
		briff_chunk_id chunk_id,  /* object id */
		briff_parser_handler_cb object_handler /* object handler */
		);

/* this function is used to remove handler */
void briff_parser_remove_handler(briff_parser_t riff, briff_parser_handler *handler);

#ifdef __cplusplus
}
#endif

#endif /* _BRIFF_PARSER_H__ */

