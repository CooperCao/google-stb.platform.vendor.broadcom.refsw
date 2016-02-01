/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "psip_priv.h"
#include "psip_mss.h"
BDBG_MODULE(psip_mss);

extern const uint8_t PSIP_decode_tree_01[];
extern const uint8_t PSIP_decode_tree_02[];

typedef struct
{
	uint32_t	ISO_639_language_code;
	uint32_t	number_segments;
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
