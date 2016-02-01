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
 * OGG parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BOGG_PARSER_H__
#define _BOGG_PARSER_H__

#include "bogg_util.h"
#include "blst_slist.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef int64_t bogg_off_t;
typedef struct bogg_parser *bogg_parser_t;

typedef struct bogg_parser_status {
    bool data_discontinuity; /* set to true if some data from the stream has been dropped or lost since the last buffer was fed */
    bogg_off_t offset; /* current offset in a stream */
    size_t acc_length; /* number of accumulated bytes */
    size_t obj_length; /* size of the object that parser expects */
    const char *state; /* state of the OGG parser */
    bmedia_parsing_errors errors;
} bogg_parser_status;

typedef enum bogg_parser_action {
    bogg_parser_action_none, /* no special action needed to perform */
    bogg_parser_action_return /* OGG parser shall return control as soon as possible */
} bogg_parser_action;

/* forward declaration of the bogg_object_handler structure */
typedef struct bogg_parser_handler bogg_parser_handler;

/* user defined object handler function */
typedef    bogg_parser_action (*bogg_parser_handler_cb)(
        bogg_parser_handler *handler, /* pointer passed ot bogg_install_object_handler function */
        batom_t object /* object payload */
        );



/* application level configuration for the OGG parser */
typedef struct bogg_parser_cfg {
    balloc_iface_t alloc;
    void *user_cntx; /* user context */
    void (*stream_error)(void *cntx); /* callback that was called when error detectect, this callback can't call back into the ogg parser */
} bogg_parser_cfg;


/* type for user defined object parser */
struct bogg_parser_handler {
    BLST_S_ENTRY(bogg_parser_handler) link; /* field that is used to link handlers together */
    uint32_t id;    /* stream serial number */
    bogg_parser_handler_cb handler; /* handler to call when object with specfied ID is found */
};


bogg_parser_t bogg_parser_create(batom_factory_t factory, const bogg_parser_cfg *cfg);
void bogg_parser_reset(bogg_parser_t ogg);
void bogg_parser_default_cfg(bogg_parser_cfg *cfg);
void bogg_parser_destroy(bogg_parser_t ogg);
size_t bogg_parser_feed(bogg_parser_t ogg, batom_pipe_t pipe);
void bogg_parser_reset(bogg_parser_t ogg);
int bogg_parser_seek(bogg_parser_t ogg, bogg_off_t off);
void bogg_parser_get_status(bogg_parser_t ogg, bogg_parser_status *status);
void bogg_parser_flush(bogg_parser_t ogg);


/* this function is used to install user defined parser for the OGG object,
 * Size of such object is limited to 64K.
 * User could register more that one hander for each object, in later case order which what parsers called is undetermined. 
 */
void bogg_parser_install_handler(
        bogg_parser_t ogg,  /* ogg parser context */
        bogg_parser_handler *handler,  /* pointer to the user supplied handler, user responsible to keep that object intact untill call to the bogg_remove_object_handler */
        uint32_t bitstream_serial_number,    /* stream serial number */
        bogg_parser_handler_cb object_handler /* object handler */
        );

/* this function is used to remove handler */
void bogg_parser_remove_handler(bogg_parser_t ogg, bogg_parser_handler *handler);

void bogg_parser_get_stream_cfg(bogg_parser_t parser, bmedia_pes_stream_cfg *cfg);
#ifdef __cplusplus
}
#endif

#endif /* _BOGG_PARSER_H__ */

