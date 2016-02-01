/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

/*= Module Overview *********************************************************
<verbatim>

Overview
BUDP_Bitread module eases reading through memory a few bits at a time.  It
only accesses the memory on 4-byte boundaries.  This restriction is necessary
to read the MPEG userdata that BMVD_USERDATA_Read() returns.

</verbatim>
***************************************************************************/

#ifndef BUDPBITREAD_H__
#define BUDPBITREAD_H__

#include "bstd.h"
#include "berr.h"
#include "budp.h"

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
	Module specific standard BERR codes
 *****************************************************************************/

/*****************************************************************************
 * Structures
 *****************************************************************************/

/*****************************************************************************
  Summary:
    Context for BUDP_Bitread object

  Description:
    The BUDP_Bitread_Context, once initielized, represents the state of the
	BUDP_Bitread object.

  See Also:
    BUDP_Bitread_Init_isr
 *****************************************************************************/
typedef struct
{
	uint32_t*    userdata_start;
	uint32_t*    userdata;
	uint32_t     cache;
	unsigned int bitsleft;
	bool         bByteswap;
}
BUDP_Bitread_Context;


/*****************************************************************************
 * Public API
 *****************************************************************************/


/*****************************************************************************
  Summary:
    Initialize a BUDP_Bitread context.

  Description:
    This function intializes a BUDP_Bitread context.  It does not
	allocate space for the context, though.

	This function reads from the address userdata (function argument).  If
	this address is not a multiple of four, then the function will read from
	the closest, lesser address that is a multiple of four.  Note that this can
	lead to memory access violations and crashes!  It is most reliable to
	simply feed this function userdata that starts at an address that is a
	multiple of four.

  Returns:
  	BERR_SUCCESS              - The context was successfully initialized.
	BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
							    possibly NULL.
	BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.

  See Also:
 *****************************************************************************/
BERR_Code BUDP_Bitread_Init_isr(
	BUDP_Bitread_Context*
	     pBitreadContext,	/*  [in] A pointer to a BUDP_Bitread_Context. */
	bool       bByteswap, 	/*  [in] Whether or not to do -endian
	                                 conversion.                             */
	void* userdata_start	/*  [in] The data to be read by future calls
	                                 into the object.  See above discussion
								     regarding the alignment of this value.  */
);


/*****************************************************************************
  Summary:
    Read bits from a BUDP_Bitread_Context.

  Description:
    This function reads bits from an initialized BUDP_Bitread_Context.
	Up to 32 bits of data may be read at a time.

  Returns:
	The specified number of bits from the input context.  The bits are right-
	justified.

  See Also:
    BUDP_Bitread_Byte_isr
 *****************************************************************************/
uint32_t BUDP_Bitread_Read_isr(
	BUDP_Bitread_Context*   pContext,	/* [in] A valid BUDP_Bitread
	                                              context.                   */
	unsigned int               nbits	/* [in] The number of bits to read.  */
);


/*****************************************************************************
  Summary:
    Read a byte (8 bits) from a BUDP_Bitread_Context.

  Description:
    This function reads 8 bits from an initialized BUDP_Bitread_Context.
	This function is faster than BUDP_Bitread_Read_isr().

  Returns:
	The byte from the input context.  Right-justified.

  See Also:
    BUDP_Bitread_Read_isr
 *****************************************************************************/
uint32_t BUDP_Bitread_Byte_isr(
    BUDP_Bitread_Context*   pContext	/* [in] A valid
									            BUDP_Bitread context. */
);


/*****************************************************************************
  Summary:
    Returns byte (not bit) position of an initialized BUDP_Bitread_Context.

  Description:
    This function discloses the next byte position to be read in a
	BUDP_Bitread_Context. Note that the BUDP_Bitread_Context maintains a
	bit position as part of its state. If this bit position is not an integer
	multiple of 8 bits, then conceptually, some information is lost.

  Returns:
	The byte offset of the object.

  See Also:
    BUDP_Bitread_Init_isr
 *****************************************************************************/
uint32_t BUDP_Bitread_GetByteOffset_isr(
    BUDP_Bitread_Context*   pContext	/* [in] A valid BUDP_Bitread
	                                            context.                     */
);


/*****************************************************************************
  Summary:
    Implements the MPEG next_start_code() function on a
	BUDP_Bitread_Context.

  Description:
  	This function reads bytes from a BUDP_Bitread_Context until an MPEG
	start code is found. Then, it "rewinds" the BUDP_Bitread_Context by four
	bytes. If no start code is found, the BUDP_Bitread_Context is left at
	the end of the search range, and a value of zero is returned.

	The caller provides a length (in bytes) to prevent an infinite search loop.

  Returns:
    The full 32-bit MPEG startcode.

  See Also:
    BUDP_Bitread_Init_isr
 *****************************************************************************/
uint32_t BUDP_Bitread_next_start_code_isr(
    BUDP_Bitread_Context* pContext, /*  [in] A valid BUDP_Bitread context. */
	 size_t                 length, /*  [in] How many bytes to search
	                                         forward.                      */
	size_t*           pBytesParsed  /* [out] How many bytes were read      */
);


#ifdef __cplusplus
}
#endif

#endif /* BUDPBITREAD_H__ */
