/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
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
 * Module Description: Converts startcode index to bcmplayer index
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ************************************************************/
#include "bvlc.h"

/* Magnum debug interface */
#include "bstd.h"
BDBG_MODULE(bvlc);

/**
Advance both bit and index by one through the byte array
**/
#define ADVANCE(bit, index, size) \
	do {if (bit) {(bit)--;} else {(bit)=7;if(++(index)>=size) {goto err_eof;}} } while (0)

/* set next_index and next_bit so any function call then would return EOF */
static int BVLC_MarkEof(unsigned size, unsigned *next_index, unsigned *next_bit)
{
    *next_bit = 0;
    if(size>0) {
        *next_index = size-1;
    } else {
        *next_index = 0;
    }
    return -1;
}

static int BVLC_MeasurePrefix(const uint8_t *data,unsigned size,unsigned index,unsigned current_bit, 
	unsigned *next_index,unsigned *next_bit,unsigned *prefix_length)
{
	int bit = current_bit;
	int done = 0;

    *next_bit = current_bit; /* always initialize result */
	if (bit > 7) {
		BDBG_ERR(("Invalid bit. Must be 0..7."));
		return -1;
	}
	
	*prefix_length = 0;
	
	/* count 0's to determine prefix_length */
	for (;;) {
		while (bit >= 0 && !done) {
			if (data[index] & (0x1 << bit))
				done = 1;
			else {
				(*prefix_length)++;
				bit--;
			}
		}
		if (done)  {
			break;
        }
        index++;
        if(index<size) {
		    bit = 7;
        } else {
            bit = 0;
            if(size>0) {
                index = size-1;
            } else {
                index = 0;
            }
            goto err_eof;
        }
	}
	/* make sure we have enough data */
	if (*prefix_length > (size * 8)/2-1) {
		BDBG_ERR(("not enough data sent for this vlc value"));
        goto err_eof;
	}
	
	*next_index = index;
	*next_bit = bit;
	return 0;
err_eof:
    return BVLC_MarkEof(size, next_index, next_bit);
}

int BVLC_Skip(const uint8_t *data, unsigned size, unsigned current_index, unsigned current_bit, 
	unsigned *next_index,unsigned *next_bit)
{
	unsigned prefix_length;
	
	if (BVLC_MeasurePrefix(data, size, current_index, current_bit, next_index, next_bit, &prefix_length)) {
        goto err_eof;
    }

#if 1
	/* Optimization: use integer math to advance by prefix_length + 1 */
	prefix_length++; /* take into account the middle 1 */
	if (prefix_length <= *next_bit)
		*next_bit -= prefix_length;
	else {
        unsigned index;
        prefix_length -= *next_bit + 1;
        index = *next_index +  1 + prefix_length/8;
        if(index<size) {
            *next_index = index;
            *next_bit = 7 - (prefix_length % 8);
        } else {
            goto err_eof;
        }
	}
#else
	/* advance one bit at a time */
	/* skip the middle 1 */
	ADVANCE(*next_bit, *next_index, size);
	
	/* skip the suffix - measure_prefix has already guaranteed we have enough data */
	while (prefix_length--)
		ADVANCE(*next_bit, *next_index, size);
#endif
		
	return 0;
err_eof:
    return BVLC_MarkEof(size, next_index, next_bit);
}

int BVLC_Decode(const uint8_t *data,unsigned size,unsigned current_index,unsigned current_bit, 
	unsigned *next_index,unsigned *next_bit)
{
	unsigned index;
	int bit;
	unsigned suffix_length, prefix_length;
	unsigned value;
	uint64_t suffix;
		
	BDBG_MSG(("vlc: %02x%02x %d, bit %d", data[current_index], (size>current_index+1)?data[current_index+1]:0, size, current_bit));

	/* find the first 1, which determines prefix_length */
	if (BVLC_MeasurePrefix(data, size, current_index, current_bit, &index, (unsigned*)&bit, &prefix_length)) {
        goto err_eof;
    }
	
	if (prefix_length > 64) {
		BDBG_ERR(("vlc value too large for this algorithm"));
		/* If you hit this, rework the algorithm. I'm assuming I can load the
		entire suffix into a primitive datatype. */
        goto err_eof;
	}
	
	/* advance past middle 1 */
	ADVANCE(bit, index, size);
	
	/* get suffix based on prefix_length */
	
	/* Optimization: no prefix, no suffix, value is 0 */
	if (!prefix_length) {
		value = 0;
		BDBG_MSG(("  prefix len 0, done"));
	}
	else {
		/* Optimization: if the suffix is completely contained in this byte, we 
		don't have to operate bitwise */
		if (prefix_length <= 8 && (int)prefix_length <= bit+1) {
			static const unsigned char mask[] = {0x0,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
			suffix = (data[index] >> (bit-(prefix_length-1))) & mask[prefix_length];
			BDBG_MSG(("  prefix len %d, suffix (in byte) val %x", prefix_length, (int)suffix));
			bit -= prefix_length;
			if (bit == -1) {
				bit = 7;
				index++;
				if(index>=size) { goto err_eof; }
			}
		}
		else {
			/* step through it bit by bit. this is the most general way and will
			work with any value. */
			suffix = 0;
			BDBG_MSG(("  prefix len %d, index %d bit %d", prefix_length, index, bit));
			for (suffix_length = 0; suffix_length < prefix_length; suffix_length++) {
				suffix <<= 1;
				if (data[index] & (0x1 << bit))
					suffix |= 1;
				ADVANCE(bit, index, size);
			}
			BDBG_MSG(("  prefix len %d, suffix val %x", prefix_length, (int)suffix));
		}
	
		/* now do the final calculation: 2^prefix_length - 1 + suffix */
		value = 1;
		while (prefix_length--) value *= 2;
		value = value - 1 + suffix;
	}

	/* move to the next bit so that subsequent calls to b_vld_decode are easy
	to make for adjacent values. */
	if (next_index) *next_index = index;
	if (next_bit) *next_bit = bit;
		
	BDBG_MSG(("  result = %d (%d, %d)", value, index, bit));
	return value;
err_eof:
    return BVLC_MarkEof(size, next_index, next_bit);
}

