/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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

#include "bvbi_cap.h"
#include "bvbi_util_priv.h"
#include "bvbi_priv.h"
#include "bkni.h"

BDBG_MODULE(BVBI);

/*
 * Handle processing
 */

/* The LineBuilder object implements a double buffer. This struct defines
 properties common to both buffers. */
typedef struct P_Bank
{
	uint8_t* rawData;       /* The raw data being stored/buffered.          */
	size_t accumLength;     /* How many bytes have been accumulated.        */
	int sequenceNumber;     /* A trigger that causes accumulation to reset.
	                           If zero, indicates there is no valid data.   */
    int lineNumber;         /* Another trigger that causes accumulation
	                           to reset.                                    */

} P_Bank;

typedef struct BVBI_P_LineBuilder_Handle
{
	/* Black magic */
	uint32_t                       ulBlackMagic;

	/* Needed to allocate memory */
	BMEM_Handle hMem;

	/* Usable line length in bytes, set at creation time. */
	size_t lineCount;

	/* Physical line length in bytes, set at creation time. */
	size_t lineSize;

	/* Which bank is being accumulated */
	int accumBank;

	/* Raw storage, double buffered. */
	P_Bank bank[2];

} BVBI_P_LineBuilder_Handle;

/* Convenience function to translate from public handle to private context */
#define BVBI_P_GET_LINEBUILDER_CONTEXT(handle, context) \
	BVBI_P_GENERIC_GET_CONTEXT(handle, context, BVBI_P_LineBuilder_Handle)


#if (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) /** { **/

/***************************************************************************
 *
 */
BERR_Code BVBI_LineBuilder_Open (
	BVBI_LineBuilder_Handle* pHandle,
	BMEM_Handle hMem, size_t lineCount, size_t lineSize)
{

	int iBank;
	size_t iEntry;
	BVBI_P_LineBuilder_Handle* context;

	BDBG_ENTER(BVBI_LineBuilder_Open);

	/* A few sanity checks */
	if(!pHandle)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	if ((lineSize == 0) ||
	    (lineCount == 0) ||
		(lineSize < lineCount))
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Alloc the main context. */
	context = (BVBI_P_LineBuilder_Handle*)
		(BKNI_Malloc(sizeof(BVBI_P_LineBuilder_Handle)));

	if(!context)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Store attributes provided by user */
	context->hMem      =      hMem;
	context->lineCount = lineCount;
	context->lineSize  =  lineSize;

	/* Two allocations to go. Avoid memory leaks after failure. */
	context->bank[0].rawData =
		(uint8_t*)(BMEM_AllocAligned (
			context->hMem,
			lineSize,
			4,
			0));
	if (!(context->bank[0].rawData))
	{
		BKNI_Free (context);
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
	context->bank[1].rawData =
		(uint8_t*)(BMEM_AllocAligned (
			context->hMem,
			lineSize,
			4,
			0));
	if (!(context->bank[1].rawData))
	{
		BMEM_Free (context->hMem, context->bank[0].rawData);
		BKNI_Free (context);
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Initialize state */
	context->accumBank = 0;
	for (iBank = 0 ; iBank < 2 ; ++iBank)
	{
		void* cached_ptr;
		uint8_t* rawData;
		BERR_Code eErr;
		P_Bank* bank = &(context->bank[iBank]);
		bank->accumLength    = 0;
		bank->sequenceNumber = 0;
		bank->lineNumber     = 0;
		if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached (
			context->hMem, bank->rawData, &cached_ptr))) !=
				BERR_SUCCESS)
		{
			BMEM_Free (context->hMem, context->bank[1].rawData);
			BMEM_Free (context->hMem, context->bank[0].rawData);
			BKNI_Free (context);
			return eErr;
		}
		rawData = (uint8_t*)cached_ptr;
		for (iEntry = lineCount ; iEntry < lineSize ; ++iEntry)
			rawData[iEntry] = 0;
		BMEM_FlushCache (context->hMem, cached_ptr, lineSize);
	}

	/* Set magic number to the size of the struct */
	context->ulBlackMagic = sizeof(BVBI_P_LineBuilder_Handle);
	
	/* All done. now return the new context to user. */
	*pHandle = (BVBI_LineBuilder_Handle)context;

	BDBG_LEAVE(BVBI_LineBuilder_Open);
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
void BVBI_LineBuilder_Close (BVBI_LineBuilder_Handle handle)
{
	BVBI_P_LineBuilder_Handle* context;
	int iBank;

	BDBG_ENTER (BVBI_LineBuilder_Close);

	/* check parameters */
	BVBI_P_GET_LINEBUILDER_CONTEXT(handle, context);
	BDBG_ASSERT (context != NULL);

	/* Wipe out bank storage */
	for (iBank = 0 ; iBank < 2 ; ++iBank)
		BMEM_Free (context->hMem, context->bank[iBank].rawData);

	/* The handle is invalid */
	context->ulBlackMagic = 0;

	/* Release context in system memory */
	BKNI_Free((void*)context);

	BDBG_LEAVE (BVBI_LineBuilder_Close);
}


/***************************************************************************
 *
 */
BERR_Code BVBI_LineBuilder_Put_isr (
	BVBI_LineBuilder_Handle handle, uint8_t* sectionData, size_t sectionSize,
	size_t sectionOffset, int sequenceNumber, int lineNumber)
{
	BVBI_P_LineBuilder_Handle* context;
	P_Bank* bank;
	void* cached_ptr;
	uint8_t* rawData;
	BERR_Code eErr;

	BDBG_ENTER (BVBI_LineBuilder_Put_isr);

	/* check parameters */
	BVBI_P_GET_LINEBUILDER_CONTEXT(handle, context);
	if (!context)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Will work on the accumulation bank, mostly. */
	bank = &context->bank[context->accumBank];

	/* Debug code: if non-accumulation bank is complete, freeze everything!
	if (context->bank[1-context->accumBank].sequenceNumber)
		return BERR_SUCCESS;
	*/

	/* Special case: forced reset. */
	if (sequenceNumber == 0)
	{
		bank->sequenceNumber = 0;
		bank->lineNumber     = 0;
		bank->accumLength    = 0;
		bank = &context->bank[1 - context->accumBank];
		bank->sequenceNumber = 0;
		bank->lineNumber     = 0;
		bank->accumLength    = 0;
		BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
		return BERR_SUCCESS;
	}

	/* Access cached memory only */
	if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached_isr (
		context->hMem, bank->rawData, &cached_ptr))) !=
			BERR_SUCCESS)
	{
		BDBG_ERR(("Cache memory failure"));
		BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
		return eErr;
	}
	rawData = (uint8_t*)cached_ptr;

	/* Can the new data be added? */
	if ((sequenceNumber == bank->sequenceNumber) &&
	    (lineNumber     == bank->lineNumber    ) &&
		(sectionOffset  == bank->accumLength   )   )
	{
		/* Slide the new data in at the end */
		if (bank->accumLength + sectionSize > context->lineCount)
		{
			BDBG_ERR(("Invalid parameter"));
			BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}
		BKNI_Memcpy (
			rawData + sectionOffset, sectionData, sectionSize);
		bank->accumLength += sectionSize;
	}
	else
	{
		/* Replace data completely, it does not match the new data */
		if (sectionOffset == 0)
		{
			if (sectionSize > context->lineCount)
			{
				BDBG_ERR(("Invalid parameter"));
				BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}
			BKNI_Memcpy (rawData, sectionData, sectionSize);
			bank->accumLength    = sectionSize;
			bank->sequenceNumber = sequenceNumber;
			bank->lineNumber     = lineNumber;
		}
		else
		{
			bank->accumLength    = 0;
			bank->sequenceNumber = 0;
			bank->lineNumber     = 0;
		}
	}

	/* If the accumulation bank is full, start using it for output. */
	if (bank->accumLength == context->lineCount)
	{
		context->accumBank = 1 - context->accumBank;
		bank = &context->bank[context->accumBank];
		bank->sequenceNumber = 0;
		bank->lineNumber     = 0;
		bank->accumLength    = 0;
	}

	BDBG_LEAVE (BVBI_LineBuilder_Put_isr);
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
BERR_Code BVBI_LineBuilder_Get_isr (
	BVBI_LineBuilder_Handle handle, uint8_t** pLineData, int* pSequenceNumber,
	int* pLineNumber)
{
	BVBI_P_LineBuilder_Handle* context;
	P_Bank* bank;
	BERR_Code eErr;
	void* cached_ptr;

	BDBG_ENTER (BVBI_LineBuilder_Get_isr);

	/* check parameters */
	BVBI_P_GET_LINEBUILDER_CONTEXT(handle, context);
	if (!context)
	{
		BDBG_ERR(("Invalid parameter"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Will work on the non-accumulation bank */
	bank = &context->bank[1 - context->accumBank];

	/* Flush cached DRAM */
	if ((eErr = BERR_TRACE (BMEM_ConvertAddressToCached_isr (
		context->hMem, bank->rawData, &cached_ptr))) !=
			BERR_SUCCESS)
	{
		BDBG_ERR(("Cache memory failure"));
		BDBG_LEAVE (BVBI_LineBuilder_Get_isr);
		return eErr;
	}
	BMEM_FlushCache_isr (context->hMem, cached_ptr, bank->accumLength);

	/* Point to data, or lack thereof. */
	*pLineData = bank->rawData;
	*pSequenceNumber = bank->sequenceNumber;
	*pLineNumber = bank->lineNumber;

	BDBG_LEAVE (BVBI_LineBuilder_Get_isr);
	return BERR_SUCCESS;
}

#endif /** } (BVBI_NUM_SCTEE > 0) || (BVBI_NUM_SCTEE_656 > 0) **/
