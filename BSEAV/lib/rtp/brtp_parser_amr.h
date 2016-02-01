/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * RTP AMR parser library. 
 *   RFC 6416 module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BRTP_PARSER_AMR_H__
#define _BRTP_PARSER_AMR_H__

#include "brtp.h"
#include "brtp_parser.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum brtp_parser_amr_mode 
{
	brtp_parser_amr_aac_loas
} brtp_parser_amr_mode;

typedef struct brtp_parser_amr_stream_cfg {
	brtp_parser_amr_mode mode;
	unsigned sizelength;
	unsigned indexlength;
	unsigned headerlength;
	uint8_t config[8];
	size_t config_len;
	unsigned profile;
}brtp_parser_amr_stream_cfg;

typedef uint8_t brtp_parser_amr_cfg ;

void brtp_parser_amr_default_cfg(brtp_parser_amr_cfg *cfg);
brtp_parser_t brtp_parser_amr_create(const brtp_parser_amr_cfg *cfg);
void brtp_parser_amr_default_stream_cfg(brtp_parser_amr_mode mode, brtp_parser_amr_stream_cfg *cfg);


#ifdef __cplusplus
}
#endif

#endif /* _BRTP_PARSER_AMR_H__ */

