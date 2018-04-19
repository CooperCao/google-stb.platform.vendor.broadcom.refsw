/***************************************************************************
 *  Copyright (C) 2007-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * Start Code Parser module for the on the fly PVR
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
	unsigned scv_len,		   /* length of the SCV table (bytes) */
	unsigned *scv_processed  /* number of processed bytes in the SCV table */
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
#define B_SCV_TYPE_BASE_40BIT 0x28
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



