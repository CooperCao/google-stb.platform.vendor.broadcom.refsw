/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bdbg.h"

BDBG_MODULE(memorymanagertest);

#include "bmem.h"
#include "bmem_debug.h"
#include "bmem_config.h"
#include "bmem_test.h"

static uint8_t  s_aBytes[1024 * 1024];

static BERR_Code BMEM_Test_CheckAlloc(
	BMEM_Handle   heap,
	unsigned int  uiHeapAlignment,
	void         *pAddr,
	size_t        ulAddrSize,
	unsigned int  uiAddrAlignment,
	unsigned int  uiAddrBoundary
	)
{
	BERR_Code  err = BERR_SUCCESS;
	uint32_t   ulAddr = (uint32_t)pAddr;
	uint32_t   ulConvertedOffset;
	void      *pConvertedAddr;

	/* fill allocated block with non-guardbanding value */
	BKNI_Memset(pAddr, 0xFF, ulAddrSize);

	/* doesn't match heap alignment? */
	if ((((UINT32_C(1) << uiHeapAlignment) - 1) & ulAddr) != 0)
	{
		/* failed to match heap alignment */
		err = BERR_TRACE(BERR_UNKNOWN);
		BDBG_ERR(("BMEM_Test_CheckAlloc: Failed heap alignment"));
		goto done;
	}

	/* doesn't match address alignment? */
	if ((((UINT32_C(1) << uiAddrAlignment) - 1) & ulAddr) != 0)
	{
		/* failed to match address alignment */
		err = BERR_TRACE(BERR_UNKNOWN);
		BDBG_ERR(("BMEM_Test_CheckAlloc: Failed address alignment"));
		goto done;
	}

	/* check boundary */

	/* convert address -> offset */
	err = BERR_TRACE(BMEM_ConvertAddressToOffset(heap, pAddr, 
		&ulConvertedOffset));

	if (err != BERR_SUCCESS)
	{
		/* unable to convert address -> offset */
		BDBG_ERR(("BMEM_Test_CheckAlloc: Failed address -> offset conversion"));
		goto done;
	}

	/* convert offset -> address */
	err = BERR_TRACE(BMEM_ConvertOffsetToAddress(heap, ulConvertedOffset,
		&pConvertedAddr));
	if (err != BERR_SUCCESS)
	{
		/* unable to convert address -> offset */
		BDBG_ERR(("BMEM_Test_CheckAlloc: Failed offset -> address conversion"));
		goto done;
	}

	/* insure that addresses are the same */
	if (pAddr != pConvertedAddr)
	{
		/* addresses did not match */
		err = BERR_TRACE(BERR_UNKNOWN);
		BDBG_ERR(("BMEM_Test_CheckAlloc: Failed address match"));
		goto done;
	}

	/* validate heap */
	err = BERR_TRACE(BMEM_ValidateHeap(heap));
	if (err != BERR_SUCCESS)
	{
		/* invalid heap */
		BDBG_ERR(("BMEM_Test_CheckAlloc: Invalid heap"));
		goto done;
	}

done:
	/* return status */
	return err;
}

void BMEM_Test_DisplayConfig(
	void )
{
	/* header */
	BKNI_Printf("BMEM compile-time configuration:\n\n");

	/* safety config */
	BKNI_Printf("BMEM_SAFETY_CONFIG      = ");
#if (BMEM_SAFETY_CONFIG == BMEM_CONFIG_FASTEST)
	BKNI_Printf("BMEM_CONFIG_FASTEST\n");
#elif (BMEM_SAFETY_CONFIG == BMEM_CONFIG_NORMAL)
	BKNI_Printf("BMEM_CONFIG_NORMAL\n");
#elif (BMEM_SAFETY_CONFIG == BMEM_CONFIG_TRACK)
	BKNI_Printf("BMEM_CONFIG_TRACK\n");
#elif (BMEM_SAFETY_CONFIG == BMEM_CONFIG_SAFE)
	BKNI_Printf("BMEM_CONFIG_SAFE\n");
#elif (BMEM_SAFETY_CONFIG == BMEM_CONFIG_SAFEST)
	BKNI_Printf("BMEM_CONFIG_SAFEST\n");
#else
	BKNI_Printf("***UNKNOWN***\n");
#endif

	/* bookkeeping config */
	BKNI_Printf("BMEM_BOOKKEEPING_CONFIG = ");
#if (BMEM_BOOKKEEPING_CONFIG == BMEM_BOOKKEEPING_LOCAL)
	BKNI_Printf("BMEM_BOOKKEEPING_LOCAL\n");
#elif (BMEM_BOOKKEEPING_CONFIG == BMEM_BOOKKEEPING_SYSTEM)
	BKNI_Printf("BMEM_BOOKKEEPING_SYSTEM\n");
#else
	BKNI_Printf("***UNKNOWN***\n");
#endif

	/* reentrant config */
	BKNI_Printf("BMEM_REENTRANT          = ");
#if (BMEM_REENTRANT_CONFIG == BMEM_REENTRANT)
	BKNI_Printf("BMEM_REENTRANT\n");
#elif (BMEM_REENTRANT_CONFIG == BMEM_NOT_REENTRANT)
	BKNI_Printf("BMEM_NOT_REENTRANT\n");
#else
	BKNI_Printf("***UNKNOWN***\n");
#endif

	/* test header */
	BKNI_Printf("------------------------------------------------\n");
}

BERR_Code BMEM_Test_DoTest1(
	void )
{
	BERR_Code     err = BERR_SUCCESS;
	BMEM_Handle   heap;
	void         *apvAlloced[1024 * 128];
	unsigned int  uiNumAlloc = 0;
	size_t        zSize;
	unsigned int  i;
	uint32_t      ulConvertOffset;
	uint32_t      ulErrCnt;

	/* set global debug level */
	BDBG_SetLevel(BDBG_eWrn);

#ifdef BMEM_TEST_DEBUG
	BDBG_SetModuleLevel("memorymanagertest", BDBG_eMsg);
	BDBG_SetModuleLevel("BMEM", BDBG_eMsg);
#else
	BDBG_SetModuleLevel("BMEM", BDBG_eWrn);
#endif	

	/* Create heap */
	BKNI_Printf("Allocate 1024x1024 bytes at offset 0x2000000 as the heap.\n");
	err = BERR_TRACE(BMEM_CreateHeap(&heap, (void *)s_aBytes,
		0x2000000, 1024*1024, 0));
	if (err != BERR_SUCCESS)
	{
		/* error creating heap */
		BDBG_ERR(("BMEM_Test_DoTest1: Failed heap create"));
		goto done;
	}

	do {
		/* allocate various sizes until failure */
		while (uiNumAlloc + 3 < 1024 * 128)
		{
			/* allocate tiny */
			apvAlloced[uiNumAlloc] = BMEM_Alloc(heap, 4);
			if (!apvAlloced[uiNumAlloc])
			{
				/* unable to allocate any more of this size */
				break;
			}

			/* check tiny */
			err = BERR_TRACE(BMEM_Test_CheckAlloc(heap, 0,
				apvAlloced[uiNumAlloc], 4, 0, 0));
			if (err != BERR_SUCCESS)
			{
				/* error */
				goto done;
			}
			BDBG_MSG(("---- tiny alloc(4-byte) #%d.", uiNumAlloc));
			uiNumAlloc++;

			/* allocate small */
			apvAlloced[uiNumAlloc] = BMEM_AllocAligned(heap, 64, 2, 0);
			if (!apvAlloced[uiNumAlloc])
			{
				/* unable to allocate any more of this size */
				break;
			}

			/* check small */
			err = BERR_TRACE(BMEM_Test_CheckAlloc(heap, 0,
				apvAlloced[uiNumAlloc], 64, 2, 0));
			if (err != BERR_SUCCESS)
			{
				/* error */
				goto done;
			}
			BDBG_MSG(("---- small alloc(64-byte) #%d.", uiNumAlloc));
			uiNumAlloc++;

			/* allocate medium */
			apvAlloced[uiNumAlloc] = BMEM_AllocAligned(heap, 1024, 4, 0);
			if (!apvAlloced[uiNumAlloc])
			{
				/* unable to allocate any more of this size */
				break;
			}

			/* check medium */
			err = BERR_TRACE(BMEM_Test_CheckAlloc(heap, 0,
				apvAlloced[uiNumAlloc], 1024, 4, 0));
			if (err != BERR_SUCCESS)
			{
				/* error */
				goto done;
			}
			BDBG_MSG(("---- medium alloc(1024-byte) #%d.", uiNumAlloc));
			uiNumAlloc++;

			/* allocate large */
			apvAlloced[uiNumAlloc] = BMEM_AllocAligned(heap, 16 * 1024, 5, 0);
			if (!apvAlloced[uiNumAlloc])
			{
				/* unable to allocate any more of this size */
				break;
			}

			/* check large */
			err = BERR_TRACE(BMEM_Test_CheckAlloc(heap, 0,
				apvAlloced[uiNumAlloc], 16 * 1024, 5, 0));
			if (err != BERR_SUCCESS)
			{
				/* error */
				goto done;
			}
			BDBG_MSG(("---- large alloc(16kbyte) #%d.", uiNumAlloc));
			uiNumAlloc++;

			/* allocate huge */
			apvAlloced[uiNumAlloc] = BMEM_AllocAligned(heap, 64 * 1024, 8, 0);
			if (!apvAlloced[uiNumAlloc])
			{
				/* unable to allocate any more of this size */
				break;
			}

			/* check huge */
			err = BERR_TRACE(BMEM_Test_CheckAlloc(heap, 0,
				apvAlloced[uiNumAlloc], 64 * 1024, 8, 0));
			if (err != BERR_SUCCESS)
			{
				/* error */
				goto done;
			}
			BDBG_MSG(("---- huge alloc(64kbyte) #%d.", uiNumAlloc));
			uiNumAlloc++;

			BMEM_ConvertAddressToOffset(heap, apvAlloced[uiNumAlloc - 4], &ulConvertOffset);
			BDBG_MSG(("==== free #%d at 0x%x.", uiNumAlloc-4, ulConvertOffset));
			BMEM_Free(heap, apvAlloced[uiNumAlloc - 4]);
			apvAlloced[uiNumAlloc - 4] = NULL;

			BMEM_ConvertAddressToOffset(heap, apvAlloced[uiNumAlloc - 2], &ulConvertOffset);
			BDBG_MSG(("==== free #%d at 0x%x.\n", uiNumAlloc-2, ulConvertOffset));
			BMEM_Free(heap, apvAlloced[uiNumAlloc - 2]);
			apvAlloced[uiNumAlloc - 2] = NULL;
		}

		/* allocate remaining slots */
		while (uiNumAlloc < 1024 * 128)
		{
			/* get largest available block */
			zSize = BMEM_GetLargestAvailableBlockSize(heap);

			/* no blocks to allocate? */
			if (zSize == 0)
			{
				/* no more blocks to allocate */
				BDBG_MSG(("No more blocks to allocate"));
				break;
			}

			/* allocate block */
			apvAlloced[uiNumAlloc] = BMEM_Alloc(heap, zSize);
			if (!apvAlloced[uiNumAlloc])
			{
				/* unable to allocate (should have) */
				err = BERR_TRACE(BERR_UNKNOWN);
				BDBG_ERR(("BMEM_Test_DoTest1: Failed largest block alloc"));
				goto done;
			}

			/* check allocation */
			err = BERR_TRACE(BMEM_Test_CheckAlloc(heap, 0,
				apvAlloced[uiNumAlloc], zSize, 0, 0));
			if (err != BERR_SUCCESS)
			{
				/* error */
				goto done;
			}
			BDBG_MSG(("---- remaining block alloc (%d-byte) #%d.", zSize, uiNumAlloc));
			uiNumAlloc++;
		}
		
		/* free random allocations */
		for (i=0; i<uiNumAlloc; ++i)
		{
			/* free random allocation */
			if((rand() % 2) && apvAlloced[i])
			{
				BMEM_ConvertAddressToOffset(heap, apvAlloced[i], &ulConvertOffset);
				BDBG_MSG(("==== free #%d at 0x%x.", i, ulConvertOffset));
				BMEM_Free(heap, apvAlloced[i]);
				apvAlloced[i] = NULL;
			}
		}

		BMEM_ValidateHeap(heap);
		BMEM_Dbg_DumpHeap(heap);
		BKNI_Printf("\nNumber of heap errors: %d.\n", BMEM_Dbg_GetErrorCount(heap));

		BKNI_Printf("Press any key to continue; press'q' to exit.\n");
	}while((getchar() != 'q') && (uiNumAlloc < 1024*128));

	/* free all allocations */
	for (i=0; i<uiNumAlloc; ++i)
	{
		/* free allocation */
		if(apvAlloced[i])
		{
			BMEM_ConvertAddressToOffset(heap, apvAlloced[i], &ulConvertOffset);
			BDBG_MSG(("==== free #%d at 0x%x.", i, ulConvertOffset));
			BMEM_Free(heap, apvAlloced[i]);
		}
	}
	/* dump heap map */
	BMEM_ValidateHeap(heap);
	BDBG_SetModuleLevel("BMEM", BDBG_eMsg);
	BMEM_Dbg_DumpHeap(heap);
	BDBG_SetModuleLevel("BMEM", BDBG_eWrn);
	ulErrCnt = BMEM_Dbg_GetErrorCount(heap);
	BKNI_Printf("Number of heap errors: %d.", ulErrCnt);
	/* Destroy heap */
	BMEM_DestroyHeap(heap);

done:
	/* display status */
	if ((err == BERR_SUCCESS) && (ulErrCnt == 0))
		BKNI_Printf("Test 1: SUCCESS");
	else
		BKNI_Printf("Test 1: FAILED (%ul)", err);

	/* return status */
	return err;
}

/* End of File */
