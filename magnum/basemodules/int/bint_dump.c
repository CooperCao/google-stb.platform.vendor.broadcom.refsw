/***************************************************************************
 *	   Copyright (c) 2003-2006, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
#include "bint.h"
#include "bint_stats.h"
#include "bkni.h"
#include "bchp.h"


BDBG_MODULE(intstats);

BERR_Code BINT_Stats_Dump
	( BINT_Handle          intHandle )
{
	BINT_CallbackHandle cbHandle;
	BINT_Stats_CallbackStats *pCbStats = NULL;
	BINT_Id intId = 0;
	int iCallbackNum = 1;
	int i = 0;
	bool bCallbackEnabled = false;
	bool bLastDumpStats = false;
	bool bFirstLine = true;

	if (intHandle == NULL)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	cbHandle = BINT_GetCallbackFirst( intHandle );

	while (cbHandle)
	{
		int iDumpBins = 0;
		bool bDumpStats = false;

		BINT_Stats_Get(cbHandle, &pCbStats);

		BINT_GetInterruptId(cbHandle, &intId);
		BINT_GetCallbackStatus(cbHandle, &bCallbackEnabled);

		if (bCallbackEnabled ||
			!pCbStats->bDefaultBins ||
			(pCbStats->ulCbHitCount != 0))
		{
			bDumpStats = true;
		}

		if (bDumpStats || bLastDumpStats || bFirstLine)
		{
			BKNI_Printf("-------------------------------------------------------------------------------\n");
			bFirstLine = false;
		}

		BKNI_Printf(" Callback %-2d %s -- IntID: 0x%08x  L2Reg: 0x%08x  L2Shift: %u\n",
			iCallbackNum++,
			(bCallbackEnabled) ? "(on) ": "(off)",
			intId,
			BCHP_INT_ID_GET_REG(intId),
			BCHP_INT_ID_GET_SHIFT(intId));

		if (bDumpStats)
		{
			BKNI_Printf("                      Min: %-6u Max: %-6u  Avg: %-6u  CbHitCount : %-6u\n", 
				(pCbStats->ulTimeMin == UINT32_MAX) ? 0: pCbStats->ulTimeMin,
				pCbStats->ulTimeMax,
				pCbStats->ulTimeAvg,
				pCbStats->ulCbHitCount);
		}

		/* print bins */
		for (i = 0; i < (int)pCbStats->ulActiveBins; i++)
		{
			if (!pCbStats->bDefaultBins ||
				(pCbStats->aBinInfo[i].ulBinHitCount != 0))
			{
				iDumpBins++;

				BKNI_Printf("  %sBin %-2d -- Range Min: %-6u  Range Max: %-6u  BinHitCount: %-6u\n",
					pCbStats->bDefaultBins ? "(Default) ": "(User)    ",
					i + 1,
					pCbStats->aBinInfo[i].ulBinRangeMin,
					pCbStats->aBinInfo[i].ulBinRangeMax,
					pCbStats->aBinInfo[i].ulBinHitCount);
			}
		}

		bLastDumpStats = bDumpStats;
		cbHandle = BINT_GetCallbackNext( cbHandle );
	}

	BKNI_Printf("\n");

	return BERR_SUCCESS;
}

void BINT_P_Stats_DumpLabel
	( BINT_Stats_CallbackStats *pCbStats )
{
	int i = 0;

	BKNI_Printf("\n");
	BKNI_Printf("Callback #,Status,IntId,L2Reg,L2Shift,");
	BKNI_Printf("Min,Max,Avg,CbHitCount,");

	for (i = 0; i < (int)pCbStats->ulActiveBins; i++)
	{
		BKNI_Printf("%d-%d us,",
			pCbStats->aBinInfo[i].ulBinRangeMin,
			pCbStats->aBinInfo[i].ulBinRangeMax);
	}

	BKNI_Printf("\n");
}

void BINT_P_Stats_DumpCbData
	( BINT_CallbackHandle cbHandle, int iCallbackNum )
{
	BINT_Stats_CallbackStats *pCbStats = NULL;
	BINT_Id intId = 0;
	bool bCallbackEnabled = false;
	int i = 0;

	BINT_Stats_Get(cbHandle, &pCbStats);
	BINT_GetInterruptId(cbHandle, &intId);
	BINT_GetCallbackStatus(cbHandle, &bCallbackEnabled);

	BKNI_Printf("Callback %d,%s,0x%08x,0x%08x,%u,",
		iCallbackNum,
		(bCallbackEnabled) ? "(on) ": "(off)",
		intId,
		BCHP_INT_ID_GET_REG(intId),
		BCHP_INT_ID_GET_SHIFT(intId));

	BKNI_Printf("%u,%u,%u,%u,", 
		(pCbStats->ulTimeMin == UINT32_MAX) ? 0: pCbStats->ulTimeMin,
		pCbStats->ulTimeMax,
		pCbStats->ulTimeAvg,
		pCbStats->ulCbHitCount);

	/* print bins */
	for (i = 0; i < (int)pCbStats->ulActiveBins; i++)
	{
		if (pCbStats->aBinInfo[i].ulBinHitCount != 0)
		{
			BKNI_Printf("%u",  pCbStats->aBinInfo[i].ulBinHitCount);
		}

		BKNI_Printf(",");
	}

	BKNI_Printf("\n");
}

BERR_Code BINT_Stats_DumpData
	( BINT_Handle          intHandle )
{
	BINT_CallbackHandle cbHandle;
	BINT_Stats_CallbackStats *pCbStats = NULL;
	int iCallbackNum = 1;
	bool bFirstDump = true;

	if (intHandle == NULL)
	{
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* print stats using default bins */
	cbHandle = BINT_GetCallbackFirst( intHandle );

	while (cbHandle)
	{
		BINT_Stats_Get(cbHandle, &pCbStats);

		if (pCbStats->bDefaultBins)
		{
			/* print out label */
			if (bFirstDump)
			{
				BINT_P_Stats_DumpLabel(pCbStats);
				bFirstDump = false;
			}

			BINT_P_Stats_DumpCbData(cbHandle, iCallbackNum);
		}

		iCallbackNum++;
		cbHandle = BINT_GetCallbackNext( cbHandle );
	}

	/* print stats using user bins */
	iCallbackNum = 1;
	cbHandle = BINT_GetCallbackFirst( intHandle );

	while (cbHandle)
	{
		BINT_Stats_Get(cbHandle, &pCbStats);

		if (!pCbStats->bDefaultBins)
		{
			BINT_P_Stats_DumpLabel(pCbStats);
			BINT_P_Stats_DumpCbData(cbHandle, iCallbackNum);
		}

		iCallbackNum++;
		cbHandle = BINT_GetCallbackNext( cbHandle );
	}

	return BERR_SUCCESS;
}
