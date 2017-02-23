/***************************************************************************
 *  Copyright (C) 2006-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Bit stream utility
 *******************************************************************************/
#ifndef _BIOBITS_H__
#define _BIOBITS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* low level bit manipulation */
/* 
Summary:
Returns bit 'b' from word 'w', 

Example:
	B_GET_BIT(0xDE,1)==0 
	B_GET_BIT(0xDE,7)!=0 
*/
#define B_GET_BIT(w,b)  ((w)&(1<<(b)))

/* 
Summary:
Returns bits 'e'..'b' from word 'w', 

Example:
	B_GET_BITS(0xDE,7,4)==0x0D
	B_GET_BITS(0xDE,3,0)=0x0E 
*/
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

/* 
Summary:
Sets bit 'b 'to value of 'v', 'name' is ignored (used as inline comment).
'v' is converted to bool (0/1) before setting bit, e.g. v with values of 1 or 1000 would generate the same result 

Example:
	B_SET_BIT("example 1",1,0)==1
	B_SET_BIT("example 2",1,2)==4
	B_SET_BIT("example 3",100,2)==4
*/

#define B_SET_BIT(name,v,b)  (((unsigned)((v)!=0)<<(b)))

/* 
Summary:
Sets bits 'e'..'b 'to value of 'v', 'name' is ignored (used as inline comment).
'v' is not truncated to 'e'-'b', e.g. if v has more than 'e'-'b'+1 bits result is undefined

*/
#define B_SET_BITS(name,v,e,b)  (((unsigned)(v))<<(b))

/* 
Summary:
Sets bit 'b'...'e' to value of 'v', name is ignored (used as inline comment).
'v' is not truncated, e.g. it's the user responsibilty that it would feet into the given number of bits

Example:
	B_SET_BITS("example 1",1,0,0)==1
	B_SET_BITS("example 2",1,1,0)==1
	B_SET_BITS("example 3",0x55,7,1)==0xAA
*/

#define B_SET_BITS(name,v,e,b)  (((unsigned)(v))<<(b))

#if 0
/* this type is used to define bitstrem */
typedef struct bio_bitstream {
	bio_cursor *cursor;
	uint32_t cache;
	int cache_pos;
} bio_bitstream;

void bio_bitstream_init(bio_bitstream *bs, bio_cursor *cursor);
void bio_bitstream_dump(bio_bitstream *bs);
bool bio_bitstream_eof(bio_bitstream *bs);
int bio_bitstream_show(bio_bitstream *bs);
int bio_bitstream_bit(bio_bitstream *bs);
int bio_bitstream_drop(bio_bitstream *bs);
unsigned bio_bitstream_bits(bio_bitstream *bs, unsigned nbits);
void bio_bitstream_drop_bits(bio_bitstream *bs, unsigned nbits);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BIOBITS_H__ */

