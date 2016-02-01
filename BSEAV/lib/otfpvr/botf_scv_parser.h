/***************************************************************************
 *	   Copyright (c) 2007-2009, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Start Code Parser module for the on the fly PVR
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BOTF_SCV_PARSER_H_
#define BOTF_SCV_PARSER_H_

#include "botf_gop_manager.h"
#include "botf_mem.h"

#define B_SCV_LEN	16
#define ITB_ENDSCD_IDX    0x10
struct b_scv_data{
    uint32_t data[B_SCV_LEN/4];
};
#define B_SCV_COPY(dst,src) (*(struct b_scv_data *)(dst) = *(struct b_scv_data *)(src))

#ifdef __cplusplus
extern "C"
{
#endif

/* this function is used to create SCV parser*/
botf_scv_parser
botf_scv_parser_create(
	bpvr_gop_manager manager, /* instance of the GOP manager */
    const BOTF_ParserPtrs *IPParserPtrs
);

void 
botf_scv_parser_destroy(
	botf_scv_parser parser /* instance of the scv parser */
);

void botf_scv_parser_reset(botf_scv_parser parser);

/* this function is used to feed data into the SCV parser */
bool
botf_scv_parser_feed(
	botf_scv_parser parser, /* instance of the SCV parser */
   	const void *scv_ptr,   /* pointer into the SCV table */
	size_t scv_len,		   /* length of the SCV table (bytes) */
	size_t *scv_processed  /* number of processed bytes in the SCV table */
);

void botf_scv_parser_getlastscode(botf_scv_parser parser, bpvr_start_code *scode);
void botf_scv_parser_getdatabytes(botf_scv_parser parser, uint8_t offset, uint8_t count, uint8_t *buf);

#ifdef UNIFIED_ITB_SUPPORT
#define B_SCV_TYPE_BASE 0x20
#define B_SCV_TYPE_PTS 0x21
#define B_SCV_TYPE_PCR_OFFSET 0x22
#define B_SCV_TYPE_SCODE 0x00
#define B_SCV_TYPE_BTP  0x23
#define B_SCV_TYPE_PCR  0x26
#else
#define B_SCV_TYPE_BASE 1
#define B_SCV_TYPE_PTS 2
#define B_SCV_TYPE_PCR_OFFSET 3
#define B_SCV_TYPE_SCODE 4
#define B_SCV_TYPE_BTP  5
#define B_SCV_TYPE_PCR  0xe
#endif

#define B_SCV_TYPE_SVPSCODE 0xf
#define B_SCV_TYPE_TERM 0x70

#define B_SCV_OFFSET_LAST 0xFF
#define B_SCV_PTS_FLAG_DTS	0x80

#define B_SCV_BTP_PICTAGCMD 13

#define B_SCV_ISSPLIT(word0) (((word0) & 0x40000) != 0)
#define B_SCV_SPLITINDX(word0) ((((word0) & 0x20000) != 0))
#define B_SCV_TYPE(word0)  (((word0) >> 24)&0xff)
#define B_SCV_SCODE_OFF(word1) ((word1) & 0xff)
#define B_SCV_SCODE(word1) (((word1)>>8) & 0xff)



#ifdef __cplusplus
}
#endif

#endif /* BOTF_SCV_PARSER_H_ */



