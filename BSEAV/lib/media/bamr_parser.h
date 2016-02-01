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
 * AMR parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/

#ifndef _BAMR_PARSER_H__
#define _BAMR_PARSER_H__

#include "bamr_util.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef int64_t bamr_off_t;
typedef struct bamr_parser *bamr_parser_t;


typedef struct bamr_parser_status {
    bool data_discontinuity; /* set to true if some data from the stream has been dropped or lost since the last buffer was fed */
    bamr_off_t offset; /* current offset in a stream */
    size_t acc_length; /* number of accumulated bytes */
    size_t obj_length; /* size of the object that parser expects */
    const char *state; /* state of the AMR parser */
    bmedia_parsing_errors errors;
} bamr_parser_status;

typedef enum bamr_parser_action {
    bamr_parser_action_none, /* no special action needed to perform */
    bamr_parser_action_return /* AMR parser shall return control as soon as possible */
} bamr_parser_action;

/* forward declaration of the bamr_object_handler structure */
typedef struct bamr_parser_handler bamr_parser_handler;

/* application level configuration for the AMR parser */
typedef struct bamr_parser_cfg {
    balloc_iface_t alloc;
    void *user_cntx; /* user context */
    void (*stream_error)(void *cntx); /* callback that was called when error detectect, this callback can't call back into the amr parser */
    bamr_parser_action (*frame)(void *user_cntx, batom_t frame);
} bamr_parser_cfg;



bamr_parser_t bamr_parser_create(batom_factory_t factory, const bamr_parser_cfg *cfg);
void bamr_parser_reset(bamr_parser_t amr);
void bamr_parser_default_cfg(bamr_parser_cfg *cfg);
void bamr_parser_destroy(bamr_parser_t amr);
size_t bamr_parser_feed(bamr_parser_t amr, batom_pipe_t pipe);
int bamr_parser_seek(bamr_parser_t amr, bamr_off_t off);
void bamr_parser_get_status(bamr_parser_t amr, bamr_parser_status *status);
void bamr_parser_flush(bamr_parser_t amr);
void bamr_parser_get_stream_cfg(bamr_parser_t parser, bmedia_pes_stream_cfg *cfg);


#ifdef __cplusplus
}
#endif

#endif /* _BAMR_PARSER_H__ */

