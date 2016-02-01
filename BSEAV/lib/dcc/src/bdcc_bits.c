/***************************************************************************
 *     (c)2002-2008 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 *   This module contains functions useful for extracting bits
 *   from a byte stream.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


/*************************************************************************
 * 
 * MODULE DETAILS:
 *
 * The functions in this module operate on bits in a byte buffer (array).
 * Each function is given a pointer to the buffer and a byte offset and a
 * bit offset indicating the current position in the buffer.  The byte/bit
 * offsets are either ints or int pointers, depending on whether or not
 * the current position is to be updated.
 * 
 * It is assumed that the first bit to be operated on is in the msb position,
 * that is, 0x80.
 *
 *************************************************************************/


#include "bdcc_bits.h"

 
/*************************************************************************
 *
 * FUNCTION:			BDCC_BITS_P_GetNextBit
 *
 * Inputs:				
 *						pBuf				- stream buffer
 *						pbyteoff			- current byte offset into pBuf
 *						pbitoff				- bit offset into current byte
 *
 * Outputs:				
 *						pbyteoff			- current byte offset into pBuf
 *						pbitoff				- bit offset into current byte
 *
 * Returns:				the requested bit, 0 or 1
 *
 * Description:
 *
 * This function returns the next bit and increments the position.
 *
 *************************************************************************/
unsigned int 
BDCC_BITS_P_GetNextBit(unsigned char * pBuf, int * pbyteoff, int * pbitoff)
{
	unsigned char mask ;
	unsigned char maskedbyte ;

	/*
	 * find the bit in question
	 */
	mask = (0x80 >> *pbitoff) ;
	maskedbyte = pBuf[*pbyteoff] & mask ;

	/*
	 * update the position
	 */
	if ( ++(*pbitoff) == 8 )
	{
		*pbitoff = 0 ;
		(*pbyteoff)++ ;
	}

	/*
	 * return either a 1 or 0
	 */
	return( maskedbyte ? 1 : 0 ) ;
}

/*************************************************************************
 *
 * FUNCTION:			BDCC_BITS_P_GetNextBits
 *
 * Inputs:				
 *						pBuf				- stream buffer
 *						pbyteoff			- current byte offset into pBuf
 *						pbitoff				- bit offset into current byte
 *						numbits				- number of bits to get (max 32)
 *
 * Outputs:				
 *						pbyteoff			- current byte offset into pBuf
 *						pbitoff				- bit offset into current byte
 *
 * Returns:				the requested bits, lsb justified
 *
 * Description:
 *
 * This function returns the requested number of bits and
 * updates the position.
 *
 *************************************************************************/
unsigned int 
BDCC_BITS_P_GetNextBits(unsigned char * pBuf, int * pbyteoff, int * pbitoff, int numbits)
{
	unsigned int retbits = 0 ;
	unsigned int newbit ;

	while ( numbits-- )
	{
		newbit = BDCC_BITS_P_GetNextBit(pBuf, pbyteoff, pbitoff) ;
		retbits = (retbits << 1) | newbit ;
	}

	return(retbits) ;
}

/*************************************************************************
 *
 * FUNCTION:			BDCC_BITS_P_GetNextBits_rev
 *
 * Inputs:				
 *						pBuf				- stream buffer
 *						pbyteoff			- current byte offset into pBuf
 *						pbitoff				- bit offset into current byte
 *						numbits				- number of bits to get (max 32)
 *
 * Outputs:				
 *						pbyteoff			- current byte offset into pBuf
 *						pbitoff				- bit offset into current byte
 *
 * Returns:				the requested bits, bit reversed, lsb justified
 *
 * Description:
 *
 * This function returns the requested number of bits in reverse
 * order and updates the position.
 *
 *************************************************************************/
unsigned int 
BDCC_BITS_P_GetNextBitsRev(unsigned char * pBuf, int * pbyteoff, int * pbitoff, int numbits)
{
	unsigned int retbits = 0 ;
	unsigned int newbit ;
	unsigned int bitmask = 1 ;

	while ( numbits-- )
	{
		newbit = BDCC_BITS_P_GetNextBit(pBuf, pbyteoff, pbitoff) ;
		if ( newbit )
			retbits |= bitmask ;
		bitmask <<= 1 ;
	}

	return(retbits) ;
}

/*************************************************************************
 *
 * FUNCTION:			BDCC_BITS_P_Nextbits
 *
 * Inputs:				
 *						pBuf				- stream buffer
 *						byteoff				- current byte offset into pBuf
 *						bitoff				- bit offset into current byte
 *						numbits				- number of bits to get (max 32)
 *
 * Outputs:				
 *						<none>
 *
 * Returns:				the requested bits, lsb justified
 *
 * Description:
 *
 * This function returns the requested number of bits but does
 * not update the position.
 *
 *************************************************************************/
unsigned int 
BDCC_BITS_P_Nextbits(unsigned char * pBuf, int byteoff, int bitoff, int numbits)
{
	return( BDCC_BITS_P_GetNextBits(pBuf, &byteoff, &bitoff, numbits) ) ;
}

/*************************************************************************
 *
 * FUNCTION:			BDCC_BITS_P_NextStartCode
 *
 * Inputs:				
 *						pBuf				- stream buffer
 *						pbyteoff			- current byte offset into pBuf
 *						pbitoff				- bit offset into current byte
 *						numbytes			- max bytes to scan forward
 *
 * Outputs:				
 *						pbyteoff			- current byte offset into pBuf
 *						pbitoff				- bit offset into current byte
 *
 * Returns:				0 for success
 *
 * Description:
 *
 * This function updates the position to the next start code.
 *
 *************************************************************************/
int BDCC_BITS_P_NextStartCode(unsigned char * pBuf, int * pbyteoff, int * pbitoff, int numbytes)
{
	/*
	 * first, get byte aligned
	 */
	while ( *pbitoff )
	{
		BDCC_BITS_P_GetNextBit(pBuf, pbyteoff, pbitoff) ;
	}

	/* 
	 * search for the StartCode pattern, 0x000001
	 */
	while ( BDCC_BITS_P_Nextbits(pBuf, *pbyteoff, *pbitoff, 24) != 0x000001 )
	{
		/* consume one byte */
		BDCC_BITS_P_GetNextBits(pBuf, pbyteoff, pbitoff, 8) ;
		if ( *pbyteoff >= numbytes )
		{
			/* gone too far */
			return(-1) ;
		}
	}
	return(0) ;
} /* BDCC_BITS_P_NextStartCode */

