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

#include "bstd.h"
#include "berr.h"
#include "budp.h"
#include "budp_bitread.h"

BDBG_MODULE(BUDP_Bitread);

/***************************************************************************
* Private data structures
***************************************************************************/

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/


/***************************************************************************
* Implementation of "BUDP_Bitread_" API functions
***************************************************************************/


/* Swap 4 byte unsigned integers */
#define SWAP_U_INT_4(tni4)                         \
 (((((tni4)>>24)&0xff  ) |   (((tni4)&0xff  )<<24) |   \
   (((tni4)>>8 )&0xff00) |   (((tni4)&0xff00)<<8 )))
/***************************************************************************
 *
 */
BERR_Code BUDP_Bitread_Init_isr (
    BUDP_Bitread_Context* pContext,
    bool                     bByteswap,
    void*                    userdata_start
)
{
    uint32_t temp;
    unsigned long address = (unsigned long)userdata_start;
    unsigned long     rem = address & (unsigned long)(0x00000003);

    BDBG_ENTER(BUDP_Bitread_Open);

    if(!pContext)
    {
        BDBG_ERR(("Invalid parameter"));
        BDBG_LEAVE(BUDP_Bitread_Open);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Take care of misalignment */
    address -= rem;

    /* Initialize the context */
    pContext->userdata_start  = (uint32_t*)address;  /* aligned on dword */
    pContext->userdata        = (uint32_t*)address;  /* aligned on dword */
    pContext->bByteswap       = bByteswap;           /* set the Endianess */
    temp = *pContext->userdata++;
    if(pContext->bByteswap)
        pContext->cache = SWAP_U_INT_4(temp);
    else
        pContext->cache = temp;

    /* make sure alignment did not cause too much to be read */
    pContext->bitsleft  = 8 * (4 - rem);
    pContext->cache <<= (8 * rem);

    BDBG_LEAVE(BUDP_Bitread_Open);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
uint32_t BUDP_Bitread_Read_isr (
    BUDP_Bitread_Context* pContext,
    unsigned int                nbits
)
{
    uint32_t result = 0;

    BDBG_ENTER(BUDP_Bitread_Read_isr);

    /* check parameters */
    BDBG_ASSERT (pContext != NULL);

    /* NOT OPTIMIZED!  SLOW! */
    while (nbits-- > 0)
    {
        if (pContext->bitsleft == 0)
        {
            uint32_t temp = *pContext->userdata++;
            if (pContext->bByteswap)
                pContext->cache = SWAP_U_INT_4(temp);
            else
                pContext->cache = temp;

            pContext->bitsleft = 32;
        }
        result = (result << 1) | ((pContext->cache >> 31) & 0x1);
        pContext->cache <<= 1;
        --pContext->bitsleft;
    }
    BDBG_LEAVE(BUDP_Bitread_Read_isr);
    return result;
}

/***************************************************************************
 *
 */
uint32_t BUDP_Bitread_Byte_isr (BUDP_Bitread_Context* pContext)

{
    uint32_t result = 0;

    BDBG_ENTER(BUDP_Bitread_Byte_isr);

    /* check parameters */
    BDBG_ASSERT (pContext != NULL);
    if (pContext->bitsleft < 8)
    {
        result = BUDP_Bitread_Read_isr (pContext, 8);

    }
    else
    {
        result = (pContext->cache >> 24 ) & 0xff;
        pContext->cache <<= 8;
        pContext->bitsleft -= 8;
    }

    BDBG_LEAVE(BUDP_Bitread_Byte_isr);
    return result;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
uint32_t BUDP_Bitread_GetByteOffset_isr(BUDP_Bitread_Context* pContext)

{
    uint32_t result;

    BDBG_ENTER(BUDP_Bitread_GetByteOffset_isr);

    /* check parameters */
    BDBG_ASSERT (pContext != NULL);

    result =
        sizeof(pContext->userdata[0]) *
            (pContext->userdata - pContext->userdata_start);
    result += (32 - pContext->bitsleft) / 8;

    BDBG_LEAVE(BUDP_Bitread_GetByteOffset_isr);
    return result;
}
#endif

/***************************************************************************
 *
 */
uint32_t BUDP_Bitread_next_start_code_isr(
    BUDP_Bitread_Context* pContext, size_t length, size_t* pBytesParsed)
{
    uint8_t saved[4];
    bool found = false;
    size_t bytesParsed = 0;

    /*
     * Go to first MPEG startcode
     */

    /* MPEG start codes are byte aligned, so make it so. */
    if (pContext->bitsleft & 0x7)
    {
        pContext->bitsleft &= ~(0x7);
        --length;
        ++bytesParsed;
    }

    /* Special case: not enough data */
    if (length < 4)
    {
        while (length-- > 0)
        {
            (void)BUDP_Bitread_Byte_isr (pContext);
            ++bytesParsed;
        }
        *pBytesParsed = bytesParsed;
        return 0x0;
    }

    /* Initialize */
    saved[0] = 0x0;
    saved[1] = BUDP_Bitread_Byte_isr (pContext);
    saved[2] = BUDP_Bitread_Byte_isr (pContext);
    saved[3] = BUDP_Bitread_Byte_isr (pContext);
    length -= 3;
    bytesParsed += 3;

    while (length > 0)
    {
        /* Read in another byte */
        saved[0] = saved[1];
        saved[1] = saved[2];
        saved[2] = saved[3];
        saved[3] = BUDP_Bitread_Byte_isr (pContext);
        --length;
        ++bytesParsed;

        if ((saved[0] == 0x00) &&
            (saved[1] == 0x00) &&
            (saved[2] == 0x01)    )
        {
            /* Found it! */
            found = true;
            break;
        }
    }

    *pBytesParsed = bytesParsed;
    if (found)
    {
        /* found the pattern before the end of stream */
        return
            ((uint32_t)saved[3] << 24) |
            ((uint32_t)saved[2] << 16) |
            ((uint32_t)saved[1] <<  8) |
            ((uint32_t)saved[0]      ) ;
    }
    else
    {
        /* Didn't find any start code */
        return 0x0;
    }

}

/* End of File */
