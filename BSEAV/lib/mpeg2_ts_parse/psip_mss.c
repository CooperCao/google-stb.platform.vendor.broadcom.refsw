/******************************************************************************
 * (c) 2003-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bstd.h"
#include "psip_priv.h"
#include "psip_mss.h"
BDBG_MODULE(psip_mss);

extern const uint8_t PSIP_decode_tree_01[];
extern const uint8_t PSIP_decode_tree_02[];

typedef struct
{
    uint32_t    ISO_639_language_code;
    uint32_t    number_segments;
} PSIP_MSS_header;

typedef enum
{
    no_compression,
    huffman_program_title,
    huffman_program_description
} PSIP_MSS_compression_type;

uint8_t PSIP_MSS_getNumStrings( PSIP_MSS_string mss )
{
    return mss[0];
}

void PSIP_MSS_P_decompressString( const uint8_t *p_decodeTree, const uint8_t *p_compressedString, uint8_t numberBytes, int *p_stringSize, char *p_string )
{
    int maxSize = *p_stringSize;
    uint32_t treeRootOffset = 0;
    uint32_t childOffset = 0;
    int curBitNum = 0;
    uint8_t priorSymbol = 0;
    bool newCharacter = true;
    uint8_t curByteValue = p_compressedString[0];

    *p_stringSize = 0;

    while( curBitNum < (numberBytes*8) )
    {
        if( !(curBitNum % 8) )
        {
            curByteValue = p_compressedString[curBitNum/8];
        }

        /* Bytes following esc code (0x27) or characters greater than 127 are never compressed */
        while( priorSymbol > 127 || priorSymbol == 27 )
        {
            /* Most likely we do not have all 8 bits available from our bitstream,
             * so read them from the next byte and or them into our symbol. */
            int bitsMissing;
            priorSymbol = curByteValue;
            curByteValue = p_compressedString[(curBitNum/8)+1];
            /* Determine how many bits we have */
            bitsMissing = curBitNum%8;
            if( bitsMissing )
            {
                priorSymbol |= curByteValue>>(8-bitsMissing);
                curByteValue <<= bitsMissing;
            }

            p_string[*p_stringSize] = priorSymbol;
            *p_stringSize += 1;
            curBitNum += 8;

            newCharacter = true;
        }

        if( newCharacter )
        {
            CHECK( priorSymbol < 128 );

            treeRootOffset = TS_READ_16( &p_decodeTree[priorSymbol*2] );
            childOffset = 0;
            newCharacter = false;
        }

        /* Does the MSB bit point to the right or left node? (0=left, 1=right)*/
        if( curByteValue & 0x80 )
        {
            /* right node byte is found one byte ahead of left node */
            childOffset+=1;
        }

        /* Is this a leaf node? */
        if( p_decodeTree[treeRootOffset+childOffset] & 0x80 )
        {
            priorSymbol = (uint8_t)(p_decodeTree[treeRootOffset+childOffset] & 0x7F);

            if( priorSymbol < 127 && priorSymbol != 27 )
            {
                p_string[*p_stringSize] = priorSymbol;
                *p_stringSize += 1;
                newCharacter = true;

                if( priorSymbol == 0 || *p_stringSize >= maxSize )
                {
                    break;
                }
            }
        }
        else
        {
            childOffset = p_decodeTree[treeRootOffset+childOffset] * 2;
        }

        curByteValue <<= 1;
        curBitNum++;
    }
}

void PSIP_MSS_P_decodeStringSegment( const uint8_t *segBuf, int *p_stringSize, char *p_string )
{
    const uint8_t *p_decodeTree = PSIP_decode_tree_02;
    int i;

    /* This routine currently only supports standard ASCII (Latin) character set */
    if( segBuf[1] != 0 )
    {
        *p_stringSize = 0;
        return;
    }

    switch( segBuf[0] )
    {
    case no_compression:
        if( *p_stringSize > segBuf[2] )
        {
            *p_stringSize = segBuf[2];
        }
        for( i = 0; i < *p_stringSize; i++ )
        {
            p_string[i] = segBuf[3+i];
        }
        break;
    case huffman_program_title:
        p_decodeTree = PSIP_decode_tree_01;
        /* Fallthrough */
    case huffman_program_description:
        PSIP_MSS_P_decompressString( p_decodeTree, &segBuf[3], segBuf[2], p_stringSize, p_string );
        break;
    default:
        CHECK( false );
        return;
    }
}


static int PSIP_MSS_P_getStringOffset( PSIP_MSS_string mss, int stringNum )
{
    int i, j, numSegs;
    int byteOffset = 1;

    for( i = 0; i < stringNum; i++ )
    {
        numSegs = mss[byteOffset+3];
        byteOffset += 4;
        for( j=0; j<numSegs; j++ )
        {
            byteOffset += mss[byteOffset+2] + 3;
        }
    }

    return byteOffset;
}

BERR_Code PSIP_MSS_getString( PSIP_MSS_string mss, int stringNum, int *p_stringSize, char *p_string )
{
    int j, numSegs;
    int byteOffset;
    int stringOffset = 0;
    int maxStringOffset = *p_stringSize - 1;
    int size;

    if( stringNum >= mss[0] )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_MSS_P_getStringOffset( mss, stringNum );
    numSegs = mss[byteOffset+3];
    byteOffset += 4;

    for( j=0; j<numSegs; j++ )
    {
        size = maxStringOffset - stringOffset;
        PSIP_MSS_P_decodeStringSegment( &mss[byteOffset], &size, &p_string[stringOffset] );
        stringOffset += size;
        byteOffset += mss[byteOffset+2] + 3;
    }

    p_string[stringOffset] = 0;
    *p_stringSize = stringOffset;

    return BERR_SUCCESS;
}

BERR_Code PSIP_MSS_getCode( PSIP_MSS_string mss, int stringNum, char **ppLanguageCode )
{
    int byteOffset;

    if( stringNum >= mss[0] )
    {
        return BERR_INVALID_PARAMETER;
    }

    byteOffset = PSIP_MSS_P_getStringOffset( mss, stringNum );
    *ppLanguageCode = (char *)&mss[byteOffset];
    return BERR_SUCCESS;
}

/* End of File */
