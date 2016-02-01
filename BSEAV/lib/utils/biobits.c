/***************************************************************************
 *     Copyright (c) 2006, Broadcom Corporation
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
#include "bstd.h"
#include "biobits.h"

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

