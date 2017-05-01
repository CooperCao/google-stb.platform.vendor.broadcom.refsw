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
 *******************************************************************************/
#include "bstd.h"
#include "biobits_api.h"

BDBG_MODULE(biobits);

void
bio_bitstream_init(bio_bitstream *bs, bio_cursor *cursor)
{
	bs->cursor = cursor;
	bs->cache = 0;
	bs->cache_pos = -1; /* invalid */
}

void
bio_bitstream_dump(bio_bitstream *bs)
{
	BSTD_UNUSED(bs);
	BDBG_WRN(("bio_bitstream: cache %#x cache_pos %d", bs->cache, bs->cache_pos));
	return;
}

bool
bio_bitstream_eof(bio_bitstream *bs)
{
	return bs->cache_pos == -1 && bio_cursor_eof(bs->cursor);
}

static int
bio_bitstream_refill(bio_bitstream *bs)
{
	bio_cursor *c = bs->cursor;

	BDBG_ASSERT(bs->cache_pos==-1);
#if B_IOVEC_FAST	
	if (c->left>=4) {
		bs->cache = (uint32_t)c->cursor[3] |
					((uint32_t)(c->cursor[2])<<8) |
					((uint32_t)(c->cursor[1])<<16) |
					((uint32_t)(c->cursor[0])<<24);
		bs->cache_pos=31;
		c->cursor += 4;
		c->left -= 4;
		return 0;
	} 
#endif
	{
		int next;
		uint32_t cache;
		
		next = bio_cursor_next(c);
		if (next==BIO_EOF) {
			return BIO_EOF;
		}
		cache = (uint32_t)next;
		next = bio_cursor_next(c);
		if (next==BIO_EOF) {
			bs->cache = cache;
			bs->cache_pos = 7; 
			return 0;
		}
		cache = (cache << 8) | (uint32_t)next;

		next = bio_cursor_next(c);
		if (next==BIO_EOF) {
			bs->cache = cache;
			bs->cache_pos = 15; 
			return 0;
		}
		cache = (cache << 8) | (uint32_t)next;

		next = bio_cursor_next(c);
		if (next==BIO_EOF) {
			bs->cache = cache;
			bs->cache_pos = 23; 
			return 0;
		}
		bs->cache = (cache << 8) | (uint32_t)next;
		bs->cache_pos = 31; 
	}
	return 0;
}

int
bio_bitstream_show(bio_bitstream *bs)
{
	int bit;

	if(bs->cache_pos==-1) {
		if (bio_bitstream_refill(bs)==BIO_EOF) {
			return BIO_EOF;
		}
	}
	bit = (bs->cache >> bs->cache_pos)&1;
	return bit;
}

int
bio_bitstream_bit(bio_bitstream *bs)
{
	int bit;
	
	bit = bio_bitstream_show(bs);
	if (bit!=BIO_EOF) {
		bs->cache_pos--;
	}
	return bit;
}

int
bio_bitstream_drop(bio_bitstream *bs)
{
	return bio_bitstream_bit(bs);
}

unsigned
bio_bitstream_bits(bio_bitstream *bs, unsigned nbits)
{
	uint32_t bits;
	int pos;

	BDBG_ASSERT(nbits<=31);
	BDBG_ASSERT(nbits>0);
	pos = bs->cache_pos - nbits;
	nbits--;
	if( bs->cache_pos>=(int)nbits) {
		bits = bs->cache >> (bs->cache_pos-nbits) & ((uint32_t)(-1)>>(31-nbits));
		bs->cache_pos = pos;
	} else {
		for(bits=0;;) {
			bits |= (unsigned) bio_bitstream_bit(bs);
			if (nbits==0) {
				break;
			}
			nbits--;
			bits <<=1;
		}
	}
	return bits;
}

void
bio_bitstream_drop_bits(bio_bitstream *bs, unsigned nbits)
{
	bio_bitstream_bits(bs, nbits);
	return;
}

