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
 * RTP AAC-LOAS parser library. 
 *   RFC 6416 module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BRTP_PARSER_AACLOAS_H__
#define _BRTP_PARSER_AACLOAS_H__

#include "brtp.h"
#include "brtp_parser.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum brtp_parser_aacloas_mode 
{
	brtp_parser_aacloas_aac_loas
} brtp_parser_aacloas_mode;

typedef struct brtp_parser_aacloas_stream_cfg {
	brtp_parser_aacloas_mode mode;
	unsigned sizelength;
	unsigned indexlength;
	unsigned headerlength;
	uint8_t config[8];
	size_t config_len;
	unsigned profile;
}brtp_parser_aacloas_stream_cfg;

typedef uint8_t brtp_parser_aacloas_cfg ;

void brtp_parser_aacloas_default_cfg(brtp_parser_aacloas_cfg *cfg);
brtp_parser_t brtp_parser_aacloas_create(const brtp_parser_aacloas_cfg *cfg);
void brtp_parser_aacloas_default_stream_cfg(brtp_parser_aacloas_mode mode, brtp_parser_aacloas_stream_cfg *cfg);


#ifdef __cplusplus
}
#endif

#endif /* _BRTP_PARSER_AACLOAS_H__ */

