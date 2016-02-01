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
#ifndef BVLC_H__
#define BVLC_H__

#include "bstd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* for backward compatibility */
#define b_vlc_decode BVLC_Decode
#define b_vlc_skip BVLC_Skip

/***************************************************************************
BVLC is a Broadcom VLC (Variable Length Coding) exponential-golomb decode algorithm.
Also see batom_bitstream_exp_golomb() in BSEAV/lib/utils/bioatom.c for another algorithm.

Examples:
    unsigned char data[size];
    int next_index, next_bit;

    // Decode 3 contiguous vlc values
    val1 = BVLC_Decode(data, size, 0, 7, &next_index, &next_bit);
    val2 = BVLC_Decode(data, size, next_index, next_bit, &next_index, &next_bit);
    val3 = BVLC_Decode(data, size, next_index, next_bit, NULL, NULL);

    // Decode the 4th vlc value
    BVLC_Skip(data, size, 0, 7, &next_index, &next_bit);
    BVLC_Skip(data, size, next_index, next_bit, &next_index, &next_bit);
    BVLC_Skip(data, size, next_index, next_bit, &next_index, &next_bit);
    val = BVLC_Decode(data, size, next_index, next_bit, NULL, NULL);

    // Decode a vlc value which is an index of the subsequent vlc value to decode.
    val = BVLC_Decode(data, size, 0, 7, &next_index, &next_bit);
    while (val--)
        BVLC_Skip(data, size, next_index, next_bit, &next_index, &next_bit);
    val = BVLC_Decode(data, size, next_index, next_bit, NULL, NULL);


Examples:
    // Decode a vlc value and calculate the number of bits the processed
    // by the last BVLC_Decode or BVLC_Skip call.

    int current_index, current_bit, num_bits;

    // save next_index and next_bit before BVLC_Decode
    current_index = next_index;
    current_bit = next_bit;

    BVLC_Decode(data, size, next_index, next_bit, &next_index, &next_bit);

    num_bits = (next_index-current_index)*8 + (current_bit-next_bit);

    
***************************************************************************/

/***************************************************************************
Summary:
    Decodes a VLC value in a byte stream.

Description:

Input:
    data            - [size_is(size)] Array of bytes which contain a vlc
                      encoded value
    size            - Size of data.
    current_index   - The starting index the data array.
    current_bit     - The bit in data[current_index] where the vlc decode
                      should start.
    next_index      - [out] the index in data which should be used for an
                      adjacent vlc decode. Can be NULL.
    next_bit        - [out] the bit in data[index] which should be used for
                      an adjacent vlc decode. Can be NULL.

Returns:
    Returns a VLC decoded value if >= 0. 
    If -1 there was a VLC decode failure.

See Also:

***************************************************************************/

int BVLC_Decode( 
    const uint8_t *data,
    unsigned size,
    unsigned current_index,
    unsigned current_bit, 
    unsigned *next_index, 
    unsigned *next_bit
    );


/***************************************************************************
Summary:
    This works by short circuiting the VLC decode algorithm in order to skip
    over a VLC encoded value.

Description:

Input:
    data            - [size_is(size)] see BVLC_Decode
    size            - see BVLC_Decode
    current_index   - see BVLC_Decode
    current_bit     - see BVLC_Decode
    next_index      - [out] the index in data which should be used for an 
                      adjacent vlc decode. Required.
    next_bit        - [out] the bit in data[index] which should be used for 
                      an adjacent vlc decode. Required.

Returns:
    0 if successfully skipped. next_index and next_bit are updated.
    If -1 there was a VLC decode failure.

See Also:
    BVLC_Decode
***************************************************************************/
int BVLC_Skip( 
    const uint8_t *data,
    unsigned size,
    unsigned current_index,
    unsigned current_bit, 
    unsigned *next_index,
    unsigned *next_bit
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVLC_H__ */
/* End of File */

