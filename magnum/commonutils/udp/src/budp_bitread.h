/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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
         pBitreadContext,   /*  [in] A pointer to a BUDP_Bitread_Context. */
    bool       bByteswap,   /*  [in] Whether or not to do -endian
                                     conversion.                             */
    void* userdata_start    /*  [in] The data to be read by future calls
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
    BUDP_Bitread_Context*   pContext,   /* [in] A valid BUDP_Bitread
                                                  context.                   */
    unsigned int               nbits    /* [in] The number of bits to read.  */
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
    BUDP_Bitread_Context*   pContext    /* [in] A valid
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
    BUDP_Bitread_Context*   pContext    /* [in] A valid BUDP_Bitread
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
