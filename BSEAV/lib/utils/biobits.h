/***************************************************************************
 *     Copyright (c) 2006-2007, Broadcom Corporation
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
 * Bit stream utility
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BIOBITS_H__
#define _BIOBITS_H__

#include "biovec.h"

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

#ifdef __cplusplus
}
#endif

#endif /* _BIOBITS_H__ */

