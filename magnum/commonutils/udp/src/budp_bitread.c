/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
