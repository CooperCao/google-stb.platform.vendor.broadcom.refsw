/***************************************************************************
 * Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/
#include "bstd.h"
#include "bint.h"
#include "bint_dump.h"
#include "bint_stats.h"
#include "bkni.h"


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

static void BINT_P_Stats_DumpLabel
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
