/***************************************************************************
 *     Copyright (c) 2011 Broadcom Corporation
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
 * FLAC parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BFLAC_PARSER_H__
#define _BFLAC_PARSER_H__

#include "bflac_util.h"
#include "blst_slist.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef int64_t bflac_off_t;
typedef struct bflac_parser *bflac_parser_t;

#define BFLAC_BLOCK_STREAMINFO  0
#define BFLAC_BLOCK_PADDING     1
#define BFLAC_BLOCK_APPLICATION 2
#define BFLAC_BLOCK_SEEKTABLE   3
#define BFLAC_BLOCK_VORBIS_COMMENT  4
#define BFLAC_BLOCK_CUESHEET    5
#define BFLAC_BLOCK_PICTURE     6



typedef struct bflac_parser_status {
    bool data_discontinuity; /* set to true if some data from the stream has been dropped or lost since the last buffer was fed */
    bflac_off_t offset; /* current offset in a stream */
    size_t acc_length; /* number of accumulated bytes */
    size_t obj_length; /* size of the object that parser expects */
    const char *state; /* state of the FLAC parser */
    bmedia_parsing_errors errors;
} bflac_parser_status;

typedef enum bflac_parser_action {
    bflac_parser_action_none, /* no special action needed to perform */
    bflac_parser_action_return /* FLAC parser shall return control as soon as possible */
} bflac_parser_action;

/* forward declaration of the bflac_object_handler structure */
typedef struct bflac_parser_handler bflac_parser_handler;

/* user defined object handler function */
typedef    bflac_parser_action (*bflac_parser_handler_cb)(
        bflac_parser_handler *handler, /* pointer passed ot bflac_install_object_handler function */
        batom_t object /* object payload */
        );

/* application level configuration for the FLAC parser */
typedef struct bflac_parser_cfg {
    void *user_cntx; /* user context */
    void (*stream_error)(void *cntx); /* callback that was called when error detectect, this callback can't call back into the flac parser */
    bflac_parser_action (*frame)(void *user_cntx, batom_t frame, const bflac_frame_header *header);
} bflac_parser_cfg;


/* type for user defined object parser */
struct bflac_parser_handler {
    BLST_S_ENTRY(bflac_parser_handler) link; /* field that is used to link handlers together */
    unsigned block_type;    /* type of block */
    bflac_parser_handler_cb handler; /* handler to call when object with specfied ID is found */
};


bflac_parser_t bflac_parser_create(batom_factory_t factory, const bflac_parser_cfg *cfg);
void bflac_parser_default_cfg(bflac_parser_cfg *cfg);
void bflac_parser_destroy(bflac_parser_t flac);
size_t bflac_parser_feed(bflac_parser_t flac, batom_pipe_t pipe);
void bflac_parser_reset(bflac_parser_t flac);
int bflac_parser_seek(bflac_parser_t flac, bflac_off_t off);
void bflac_parser_get_status(bflac_parser_t flac, bflac_parser_status *status);
void bflac_parser_flush(bflac_parser_t flac);


/* this function is used to install user defined parser for the FLAC object,
 * Size of such object is limited to 64K.
 * User could register more that one hander for each object, in later case order which what parsers called is undetermined. 
 */
void bflac_parser_install_handler(
        bflac_parser_t flac,  /* flac parser context */
        bflac_parser_handler *handler,  /* pointer to the user supplied handler, user responsible to keep that object intact untill call to the bflac_remove_object_handler */
        uint32_t block_type,    /* stream serial number */
        bflac_parser_handler_cb object_handler /* object handler */
        );

/* this function is used to remove handler */
void bflac_parser_remove_handler(bflac_parser_t flac, bflac_parser_handler *handler);

#ifdef __cplusplus
}
#endif

#endif /* _BFLAC_PARSER_H__ */

