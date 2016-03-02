/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#include "splash_vdc_rulgen.h"
#include "nexus_platform.h"

/* Tuning from Makefile */
#define XSTRINGIZE(x) #x
#define STRINGIZE(x) XSTRINGIZE(x)
static const char* s_suffix_string = STRINGIZE(SPLASH_DATA_SUFFIX);


BDBG_MODULE(splash_vdc_generate);

static BERR_Code APP_BRDC_Slot_SetList_isr
(
	BRDC_Slot_Handle hSlot,
	BRDC_List_Handle hList,
	BRDC_Trigger     eTrigger
);
static BERR_Code APP_BRDC_Slot_ExecuteOnTrigger_isr
(
	BRDC_Slot_Handle hSlot,
	BRDC_Trigger     eRDCTrigger,
	bool             bRecurring
);
static BERR_Code APP_BRDC_Slot_Execute_isr
(
	BRDC_Slot_Handle hSlot,
	BRDC_Trigger     eTrigger
);

/* RUL storage */
#define MAX_NUM_TRIGGERS        12
#define MAX_RUL_COUNT           500
#define MAX_RUL_ENTRIES         2000

BRDC_List_Handle g_aRulList[MAX_RUL_COUNT];
BRDC_List_Handle g_aHandleList[MAX_RUL_COUNT];
BRDC_Slot_Handle g_aHandleSlot[MAX_RUL_COUNT];
int g_iListCount = 0;
bool g_bExecuteList[MAX_NUM_TRIGGERS];
bool g_bFirstBottom[MAX_NUM_TRIGGERS];

/* first lists to execute for primary t/b and secondary t/b */
int g_aTopLists[MAX_NUM_TRIGGERS] = { -1 };

int g_aListOrder[MAX_NUM_TRIGGERS][MAX_RUL_COUNT] = { { -1 } };
int g_aListOrderCount[MAX_NUM_TRIGGERS] = { 0 };

/* Mapping structure for the slots/triggers and the lists */
struct TriggerMap
{
	int TriggerHwNum; /* The hardware position of the trigger */
	int SlotNum;      /* The slot correposponding to the Slot */
} g_asTriggerMap[MAX_NUM_TRIGGERS];

extern ModeHandles g_stModeHandles;

/* register storage */
#define MAX_REG_ENTRIES 10000

uint32_t g_RegDataCount = 0;

struct {
	uint32_t    reg;
	uint32_t    val;
} g_RegData[MAX_REG_ENTRIES];

static void InitializeStorage(
	BRDC_Handle  hRdc
	)
{
	BERR_Code  eErr;
	int 	   i;

	/* allocate all temporary lists */
	for(i=0; i<MAX_RUL_COUNT; ++i)
	{
		/* allocate RUL */
		TestError( BRDC_List_Create(hRdc, MAX_RUL_ENTRIES,
			g_aRulList + i),
			"ERROR: BRDC_List_Create" );

		/* clear source handle */
		g_aHandleList[i] = NULL;
		g_aHandleSlot[i] = NULL;
	}

done:
    return;
}

static void UnInitializeStorage(void)
{
	BERR_Code  eErr;
	int        i;

	/* allocate all temporary lists */
	for(i=0; i<MAX_RUL_COUNT; ++i)
	{
		/* allocate RUL */
		TestError( BRDC_List_Destroy(g_aRulList[i]),
			"ERROR: BRDC_List_Destroy" );
	}

done:
	return;
}

/* searches a list and looks for a setting for a specific register. When found it will
   mask the existing value and then OR in the data value. This is mainly used to change
    register values that will not run in a script based system. */
static void ReplaceRegisterList(
	BRDC_List_Handle  hList,
	uint32_t          ulReg,
	uint32_t          ulMask,
	uint32_t          ulData
	)
{
	BRDC_DBG_ListEntry  eEntry;
	uint32_t            aulArgs[4];
	bool                bCheckCommand = false;
	uint32_t            ulCurrRegister = 0;
	uint32_t           *pulAddress;

	/* get address to list */
	pulAddress = BRDC_List_GetStartAddress_isr(hList);

	/* prepare to traverse this list */
	if (BRDC_DBG_SetList(hList) != BERR_SUCCESS)
	{
		/* error */
		BDBG_ERR(("ERROR parsing list %d", __LINE__));
		goto done;
	}

	/* get first entry in list */
	if(BRDC_DBG_GetListEntry(hList, &eEntry, aulArgs))
	{
		/* error */
		BDBG_ERR(("ERROR parsing list %d", __LINE__));
		goto done;
	}

	/* traverse until finished */
	while(eEntry != BRDC_DBG_ListEntry_eEnd)
	{
		/*printf("1--ReplaceRegisterList\n");*/
		/* command entry? */
		if(eEntry == BRDC_DBG_ListEntry_eCommand)
		{
			/*printf("2--ReplaceRegisterList\n");*/
			/* is this a write to register command? */
			bCheckCommand = false;
			switch(aulArgs[0])
			{
				case BRDC_OP_VAR_TO_REG_OPCODE:
				case BRDC_OP_REG_TO_REG_OPCODE:
				case BRDC_OP_REGS_TO_REGS_OPCODE:
				case BRDC_OP_REG_TO_REGS_OPCODE:
				case BRDC_OP_REGS_TO_REG_OPCODE:
					/* unhandled */
					BDBG_MSG(("Unhandled register write command!!!!"));
					break;

				case BRDC_OP_IMMS_TO_REG_OPCODE:
				case BRDC_OP_IMMS_TO_REGS_OPCODE:
					bCheckCommand = true;
					break;

				case BRDC_OP_IMM_TO_REG_OPCODE:
					bCheckCommand = true;
					break;

				default:
					break;
			}

		/* check this command contents? */
		} else if(bCheckCommand)
		{
			/* register? */
			if(eEntry == BRDC_DBG_ListEntry_eRegister)
			{
				/* store register */
				ulCurrRegister = aulArgs[0];

			} else if(eEntry == BRDC_DBG_ListEntry_eData)
			{
				/* data -- do we have the right register? */
				if(ulCurrRegister == ulReg)
				{
					/* update register with new value */
					*pulAddress &= ulMask;
					*pulAddress |= ulData;
					BDBG_MSG(("Replacing RUL register %08x old: %08x, new %08x",
						ulReg, aulArgs[0], *pulAddress));

				/* not a match */
				} else
				{
					/* assume that the next data goes with the next register */
					ulCurrRegister += 4;
				}
			}
		}

		/* get next entry in list */
		pulAddress++;
		if(BRDC_DBG_GetListEntry(hList, &eEntry, aulArgs))
		{
			/* error */
			BDBG_ERR(("ERROR parsing list %d", __LINE__));
			goto done;
		}
	}

done:
	return;
}

static void WriteSplashInfo(FILE *fp, ModeHandles *pMode)
{
	uint32_t        surfWidth;
	uint32_t        surfHeight;
	uint32_t        surfPitch;
	BPXL_Format     surfFormat;
	BFMT_VideoInfo  videoInfo;
	SplashDisplay   *pDisp;
	SplashSurface   *pSurf;
	int  ii;

	fprintf(fp,"#define BSPLASH_NUM_MEM\t\t\t %d\n\n", SPLASH_NUM_MEM);

	fprintf(fp,"#define BSPLASH_RULMEMIDX\t\t %d\t/* index to ahMem for vdc/rul usage */\n\n", pMode->iRulMemIdx);

	fprintf(fp,"#define BSPLASH_NUM_SURFACE\t\t %d\n", SPLASH_NUM_SURFACE);
	fprintf(fp,"static SplashSurfaceInfo  s_SplashSurfaceInfo[BSPLASH_NUM_SURFACE] =\n{\n");


	for (ii=0; ii<SPLASH_NUM_SURFACE; ii++)
	{
		pSurf = &pMode->surf[ii];

#if 0
		BSUR_Surface_GetDimensions(pSurf->hSurface, &surfWidth, &surfHeight);
		BSUR_Surface_GetAddress(pSurf->hSurface, &surfAddress, &surfPitch);
		BSUR_Surface_GetFormat(pSurf->hSurface, &surfFormat);
#else
		surfWidth = pSurf->surface.ulWidth;
		surfHeight = pSurf->surface.ulHeight;
		surfPitch = pSurf->surface.ulPitch;
		surfFormat = pSurf->surface.eFormat;
#endif
		fprintf(fp,"\t{   /* sur %d: */\n", ii);
		fprintf(fp,"\t\t%d,\t\t\t\t\t\t\t/* ihMemIdx */\n", pSurf->iMemIdx);
		fprintf(fp,"\t\t(BPXL_Format)0x%8.8x,\t/* %s */\n", surfFormat, BPXL_ConvertFmtToStr(surfFormat));
		fprintf(fp,"\t\t%d,\t\t\t\t\t\t/* width */\n", surfWidth);
		fprintf(fp,"\t\t%d,\t\t\t\t\t\t/* height */\n", surfHeight);
		fprintf(fp,"\t\t%d,\t\t\t\t\t\t/* pitch */\n", surfPitch);
		fprintf(fp,"\t\t\"%s\"\t\t\t\t/* bmp file in splashgen */\n", &pSurf->bmpFile[0]);
		if ((SPLASH_NUM_SURFACE-1) == ii)
			fprintf(fp,"\t}\n");
		else
			fprintf(fp,"\t},\n");
	}
	fprintf(fp,"};\n\n");

	fprintf(fp,"#define BSPLASH_NUM_DISPLAY\t\t %d\n", SPLASH_NUM_DISPLAY);
	fprintf(fp,"static SplashDisplayInfo  s_SplashDisplayInfo[BSPLASH_NUM_DISPLAY] =\n{\n");
	for (ii=0; ii<SPLASH_NUM_DISPLAY; ii++)
	{
		pDisp = &pMode->disp[ii];
		BFMT_GetVideoFormatInfo(pDisp->eDispFmt, &videoInfo);
		fprintf(fp,"\t{   /* disp %d: */\n", ii);
		fprintf(fp,"\t\t%d,\t\t\t\t\t/* iSurIdx */\n", pDisp->iSurfIdx);
		fprintf(fp,"\t\t(BFMT_VideoFmt)%d,\t/* %s */\n", pDisp->eDispFmt, videoInfo.pchFormatStr);
		fprintf(fp,"\t\t0x%8.8x,\t\t\t/* ulRdcScratchReg0 */\n", pDisp->ulGfdScratchReg0);
		fprintf(fp,"\t\t0x%8.8x \t\t\t/* ulRdcScratchReg1 */\n", pDisp->ulGfdScratchReg1);
		if ((SPLASH_NUM_DISPLAY-1) == ii)
			fprintf(fp,"\t}\n");
		else
			fprintf(fp,"\t},\n");
	}
	fprintf(fp,"};\n\n\n");
}


static void DumpLists(
	FILE* fp,
	ModeHandles* pMode
	)
{
	int i;
	uint32_t ulNumEntries;
	uint32_t *pulStartAddress;
	int iListIndex;
	int iIndex;
	int iTrigger;
	uint32_t ulCount;
	uint32_t totalListCount = 0;

	/* splash surface and format info */
	WriteSplashInfo(fp, pMode);

	/* dump lists in order of execution for a given trigger */
	for(iTrigger=0; iTrigger<MAX_NUM_TRIGGERS; iTrigger++)
	{
		totalListCount += g_aListOrderCount[iTrigger];
		if(g_aListOrderCount[iTrigger] == 0)
		{
			BDBG_MSG(("Skipping empty trigger %d", iTrigger));
			continue;
		}
		BDBG_MSG(("Writing trigger %d, cntr %d", iTrigger, g_aListOrderCount[iTrigger]));

		/* count header */
		fprintf(fp, "\nstatic uint32_t s_aListCount%d[] =\n{\n",
			iTrigger);

		ulCount = 0;

		/* if(iTrigger == 6)
			printf("totalListCount %d g_aListOrderCount[%d]%d\n",totalListCount,iTrigger,g_aListOrderCount[iTrigger]); */

		for(iIndex=0; iIndex < g_aListOrderCount[iTrigger]; iIndex++)
		{
			/* index of list */
			iListIndex = g_aListOrder[iTrigger][iIndex];

#ifdef BCHP_IT_0_VEC_TRIGGER_0
			/* replace list entries so resulting RULs will work without _isr assistance */
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_IT_0_VEC_TRIGGER_0,
				~BCHP_MASK(IT_0_VEC_TRIGGER_0,ENABLE),
				BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_0,ENABLE,1));

			/* todo for progressive formats this triiger is not required */
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_IT_0_VEC_TRIGGER_1,
				~BCHP_MASK(IT_0_VEC_TRIGGER_1,ENABLE),
				BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_1,ENABLE,1));
#elif defined BCHP_PRIM_IT_VEC_TRIGGER_0
			/* replace list entries so resulting RULs will work without _isr assistance */
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_PRIM_IT_VEC_TRIGGER_0,
				~BCHP_MASK(PRIM_IT_VEC_TRIGGER_0,ENABLE),
				BCHP_FIELD_DATA(PRIM_IT_VEC_TRIGGER_0,ENABLE,1));
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_PRIM_IT_VEC_TRIGGER_1,
				~BCHP_MASK(PRIM_IT_VEC_TRIGGER_1,ENABLE),
				BCHP_FIELD_DATA(PRIM_IT_VEC_TRIGGER_1,ENABLE,1));
#else
#error "Port required for splashgen, talk to VDC team"
#endif

#ifdef BCHP_IT_1_VEC_TRIGGER_0
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_IT_1_VEC_TRIGGER_0,
				~BCHP_MASK(IT_0_VEC_TRIGGER_0,ENABLE),
				BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_0,ENABLE,1));
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_IT_1_VEC_TRIGGER_1,
				~BCHP_MASK(IT_0_VEC_TRIGGER_1,ENABLE),
				BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_1,ENABLE,1));
#elif defined BCHP_SEC_IT_VEC_TRIGGER_0
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_SEC_IT_VEC_TRIGGER_0,
				~BCHP_MASK(SEC_IT_VEC_TRIGGER_0,ENABLE),
				BCHP_FIELD_DATA(SEC_IT_VEC_TRIGGER_0,ENABLE,1));
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_SEC_IT_VEC_TRIGGER_1,
				~BCHP_MASK(SEC_IT_VEC_TRIGGER_1,ENABLE),
				BCHP_FIELD_DATA(SEC_IT_VEC_TRIGGER_1,ENABLE,1));
#endif

#ifdef BCHP_IT_2_VEC_TRIGGER_0
			/* To check with VDC team about this */
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_IT_2_VEC_TRIGGER_0,
				~BCHP_MASK(IT_0_VEC_TRIGGER_0,ENABLE),
				BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_0,ENABLE,1));
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_IT_2_VEC_TRIGGER_1,
				~BCHP_MASK(IT_0_VEC_TRIGGER_1,ENABLE),
				BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_1,ENABLE,1));
#endif

#ifdef BCHP_DVI_DTG_0_DTG_TRIGGER_0
			/* To check with VDC team about this */
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_DVI_DTG_0_DTG_TRIGGER_0,
				~BCHP_MASK(DVI_DTG_0_DTG_TRIGGER_0,ENABLE),
				BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_0,ENABLE,1));
			ReplaceRegisterList(g_aRulList[iListIndex],BCHP_DVI_DTG_0_DTG_TRIGGER_1,
				~BCHP_MASK(DVI_DTG_0_DTG_TRIGGER_0,ENABLE),
				BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_0,ENABLE,1));
#endif

			/* get entries for list */
			BRDC_List_GetNumEntries_isr(g_aRulList[iListIndex], &ulNumEntries);

			/* print num entries */
			fprintf(fp, "\t%d,\n", ulCount);

			/* increment count */
			ulCount += ulNumEntries;
		}

		/* count end */
		fprintf(fp, "\t%d,\n};\n", ulCount);

		/* trigger header */
		fprintf(fp, "\nstatic uint32_t s_aList%d[] =\n{\n",
			iTrigger);

		for(iIndex=0; iIndex < g_aListOrderCount[iTrigger]; iIndex++)
		{
			/* index of list */
			iListIndex = g_aListOrder[iTrigger][iIndex];

			/* get entries for list */
			BRDC_List_GetNumEntries_isr(g_aRulList[iListIndex], &ulNumEntries);
			pulStartAddress = BRDC_List_GetStartAddress_isr(g_aRulList[iListIndex]);

			/* header */
			fprintf(fp, "\n\t/* LIST: %d (%d entries) */\n",
				iListIndex, (int)ulNumEntries);

			/* body */
			for(i=0; i<(int)ulNumEntries; i++)
			{
				fprintf(fp, "\t0x%08x,\n", *(pulStartAddress++));
			}
		}

		/* trigger close */
		fprintf(fp, "};\n");
	}

	if(totalListCount==0)
	{
		BDBG_ERR(("***************************************************************************"));
		BDBG_ERR(("Error ... Error ... Error ! " "\n" "RUL Dump is empty !!! "));
		BDBG_ERR(("Looks like your chip uses a new RDC trigger"));
		BDBG_ERR(("   add them into isTrigger() and GetArrayIndex()"));
		BDBG_ERR(("or you have not chosen the right version of RDC"));
		BDBG_ERR(("   please pickup the instrumented version from LATEST RDC commonutils"));
		BDBG_ERR(("   or reuqest VDC team to enable splash on RDC for your chip"));
		BDBG_ERR(("***************************************************************************"));
		return;
	}

	/* Generate the trigger map */
	fprintf(fp,
		"static SplashTriggerMap s_aTriggerMap[] =\n"
		"{\n"
	);
	for(iTrigger=0; iTrigger<MAX_NUM_TRIGGERS; iTrigger++)
	{
		char listcount[50],list[50];
		int lclTriggerHwNum, lclSlotnum;
		if(g_aListOrderCount[iTrigger])
		{
			sprintf(listcount,"s_aListCount%d",iTrigger);
			sprintf(list,"s_aList%d",iTrigger);
			lclTriggerHwNum = g_asTriggerMap[iTrigger].TriggerHwNum;
			lclSlotnum = g_asTriggerMap[iTrigger].SlotNum;
		}
		else
		{
			sprintf(listcount,"NULL");
			sprintf(list,"NULL");
			lclTriggerHwNum = -1;
			lclSlotnum = -1;
		}

		fprintf(fp," { %d, %d, %d, %s, %s },\n",
			lclTriggerHwNum,
			lclSlotnum,
			g_aListOrderCount[iTrigger],
			listcount,
			list);
	}
	fprintf(fp, "\n};\n");
}

#define ENT(_start_, _end_, _desc_) { _start_, _end_, _desc_ }

static struct
{
	uint32_t		start;
	uint32_t		end;
    char*  desc;
}
RegisterNames[]=
{
	/* new ENT add should use BCHP_*_REG_START and BCHP_*_REG_END in bchp_common.h,
	 * so you don't need to add any new include for this new ENT */
#if BCHP_FMISC_REG_START
	ENT(BCHP_FMISC_REG_START, BCHP_FMISC_REG_END, "FMISC"),
#endif
#if BCHP_MMISC_REG_START
	ENT(BCHP_MMISC_REG_START, BCHP_MMISC_REG_END, "MMISC"),
#endif
#if BCHP_BMISC_REG_START
	ENT(BCHP_BMISC_REG_START, BCHP_BMISC_REG_END, "BMISC"),
#endif
#if BCHP_DMISC_REG_START
	ENT(BCHP_DMISC_REG_START, BCHP_DMISC_REG_END, "DMISC"),
#endif
#if BCHP_BVNF_INTR2_0_REG_START
	ENT(BCHP_BVNF_INTR2_0_REG_START, BCHP_BVNF_INTR2_0_REG_END, "BVNF_INTR2_0"),
#endif
#if BCHP_BVNF_INTR2_1_REG_START
	ENT(BCHP_BVNF_INTR2_1_REG_START, BCHP_BVNF_INTR2_1_REG_END, "BVNF_INTR2_1"),
#endif
#if BCHP_BVNF_INTR2_2_REG_START
	ENT(BCHP_BVNF_INTR2_2_REG_START, BCHP_BVNF_INTR2_2_REG_END, "BVNF_INTR2_2"),
#endif
#if BCHP_BVNF_INTR2_3_REG_START
	ENT(BCHP_BVNF_INTR2_3_REG_START, BCHP_BVNF_INTR2_3_REG_END, "BVNF_INTR2_3"),
#endif
#if BCHP_BVNF_INTR2_4_REG_START
	ENT(BCHP_BVNF_INTR2_4_REG_START, BCHP_BVNF_INTR2_4_REG_END, "BVNF_INTR2_4"),
#endif
#if BCHP_BVNF_INTR2_5_REG_START
	ENT(BCHP_BVNF_INTR2_5_REG_START, BCHP_BVNF_INTR2_5_REG_END, "BVNF_INTR2_5"),
#endif
#if BCHP_BVNF_INTR2_6_REG_START
	ENT(BCHP_BVNF_INTR2_6_REG_START, BCHP_BVNF_INTR2_6_REG_END, "BVNF_INTR2_6"),
#endif
#if BCHP_BVNF_INTR2_7_REG_START
	ENT(BCHP_BVNF_INTR2_7_REG_START, BCHP_BVNF_INTR2_7_REG_END, "BVNF_INTR2_7"),
#endif
#if BCHP_BVNM_INTR2_0_REG_START
	ENT(BCHP_BVNM_INTR2_0_REG_START, BCHP_BVNM_INTR2_0_REG_END, "BVNM_INTR2_0"),
#endif
#if BCHP_BVNM_INTR2_1_REG_START
	ENT(BCHP_BVNM_INTR2_1_REG_START, BCHP_BVNM_INTR2_1_REG_END, "BVNM_INTR2_1"),
#endif
#if BCHP_BVNB_INTR2_REG_START
	ENT(BCHP_BVNB_INTR2_REG_START, BCHP_BVNB_INTR2_REG_END, "BVNB_INTR2"),
#endif
#if BCHP_CLKGEN_INTR2_REG_START
	ENT(BCHP_CLKGEN_INTR2_REG_START, BCHP_CLKGEN_INTR2_REG_END, "CLKGEN_INTR2"),
#endif
#if BCHP_VCXO_0_RM_REG_START
	ENT(BCHP_VCXO_0_RM_REG_START, BCHP_VCXO_0_RM_REG_END, "VCXO_0_RM"),
#endif
#if BCHP_VCXO_1_RM_REG_START
	ENT(BCHP_VCXO_1_RM_REG_START, BCHP_VCXO_1_RM_REG_END, "VCXO_1_RM"),
#endif
#if BCHP_VCXO_2_RM_REG_START
	ENT(BCHP_VCXO_2_RM_REG_START, BCHP_VCXO_2_RM_REG_END, "VCXO_2_RM"),
#endif
#if (BCHP_CLKGEN_REG_END && BCHP_CLKGEN_REG_START)
	ENT(BCHP_CLKGEN_REG_START, BCHP_CLKGEN_REG_END, "CLKGEN"),
#endif
#if (BCHP_CLKGEN_GR_REG_END && BCHP_CLKGEN_GR_REG_START)
	ENT(BCHP_CLKGEN_GR_REG_START, BCHP_CLKGEN_GR_REG_END, "CLKGEN_GR"),
#endif
#if (BCHP_GFD_0_REG_START)
	ENT(BCHP_GFD_0_REG_START, BCHP_GFD_0_REG_END, "GFD_0"),
#endif
#if (BCHP_GFD_1_REG_START)
	ENT(BCHP_GFD_1_REG_START, BCHP_GFD_1_REG_END, "GFD_1"),
#endif
#if (BCHP_GFD_2_REG_START)
	ENT(BCHP_GFD_2_REG_START, BCHP_GFD_2_REG_END, "GFD_2"),
#endif
#if (BCHP_GFD_3_REG_START)
	ENT(BCHP_GFD_3_REG_START, BCHP_GFD_3_REG_END, "GFD_3"),
#endif
#if (BCHP_GFD_4_REG_START)
	ENT(BCHP_GFD_4_REG_START, BCHP_GFD_4_REG_END, "GFD_4"),
#endif
#if (BCHP_GFD_5_REG_START)
	ENT(BCHP_GFD_5_REG_START, BCHP_GFD_5_REG_END, "GFD_5"),
#endif
#if (BCHP_GFD_6_REG_START)
	ENT(BCHP_GFD_6_REG_START, BCHP_GFD_6_REG_END, "GFD_6"),
#endif
#if (BCHP_CMP_0_REG_START)
	ENT(BCHP_CMP_0_REG_START, BCHP_CMP_0_REG_END, "CMP_0"),
#endif
#if (BCHP_CMP_1_REG_START)
	ENT(BCHP_CMP_1_REG_START, BCHP_CMP_1_REG_END, "CMP_1"),
#endif
#if (BCHP_CMP_2_REG_START)
	ENT(BCHP_CMP_2_REG_START, BCHP_CMP_2_REG_END, "CMP_2"),
#endif
#if (BCHP_CMP_3_REG_START)
	ENT(BCHP_CMP_3_REG_START, BCHP_CMP_3_REG_END, "CMP_3"),
#endif
#if (BCHP_CMP_4_REG_START)
	ENT(BCHP_CMP_4_REG_START, BCHP_CMP_4_REG_END, "CMP_4"),
#endif
#if (BCHP_CMP_5_REG_START)
	ENT(BCHP_CMP_5_REG_START, BCHP_CMP_5_REG_END, "CMP_5"),
#endif
#if (BCHP_CMP_6_REG_START)
	ENT(BCHP_CMP_6_REG_START, BCHP_CMP_6_REG_END, "CMP_6"),
#endif
#if BCHP_RAAGA_DSP_MISC_1_REG_START
	ENT(BCHP_RAAGA_DSP_MISC_1_REG_START, BCHP_RAAGA_DSP_MISC_1_REG_END, "RAAGA_DSP_MISC_1"),
#endif
#if BCHP_AIO_INTH_REG_START
	ENT(BCHP_AIO_INTH_REG_START, BCHP_AIO_INTH_REG_END, "AIO_INTH"),
#endif
#if BCHP_SID_L2_REG_START
	ENT(BCHP_SID_L2_REG_START, BCHP_SID_L2_REG_END, "SID_L2"),
#endif
#if BCHP_SCPU_HOST_INTR2_REG_START
	ENT(BCHP_SCPU_HOST_INTR2_REG_START, BCHP_SCPU_HOST_INTR2_REG_END, "SCPU_HOST_INTR2"),
#endif
#if BCHP_RAAGA_DSP_INTH_1_REG_START
	ENT(BCHP_RAAGA_DSP_INTH_1_REG_START, BCHP_RAAGA_DSP_INTH_1_REG_END, "RAAGA_DSP_INTH_1"),
#endif
#if BCHP_RAAGA_DSP_FW_INTH_1_REG_START
	ENT(BCHP_RAAGA_DSP_FW_INTH_1_REG_START, BCHP_RAAGA_DSP_FW_INTH_1_REG_END, "RAAGA_DSP_FW_INTH_1"),
#endif
#if (BCHP_SUN_TOP_CTRL_SW_RESET && BCHP_SUN_TOP_CTRL_SW_RESET)
	ENT(BCHP_SUN_TOP_CTRL_SW_RESET, BCHP_SUN_TOP_CTRL_SW_RESET, "SUN_TOP_CTRL_SW_RESET"),
#endif
#if (BCHP_SUN_TOP_CTRL_SW_INIT_0_SET && BCHP_SUN_TOP_CTRL_SW_INIT_0_SET)
	ENT(BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, "SUN_TOP_CTRL_SW_RESET"),
#endif
#if (BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR && BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR)
	ENT(BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, "SUN_TOP_CTRL_SW_INIT_0_CLEAR"),
#endif
#if (BCHP_SUN_TOP_CTRL_PIN_MUX_PAD_CTRL_12 && BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0)
	ENT(BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_PAD_CTRL_12, "SUN_TOP_CTRL_PIN_MUX"),
#endif
#if BCHP_SUN_TOP_CTRL_REG_START
	ENT(BCHP_SUN_TOP_CTRL_REG_START, BCHP_SUN_TOP_CTRL_REG_END, "SUN_TOP_CTRL ..."),
#endif
#if BCHP_AON_PIN_CTRL_REG_START
	ENT(BCHP_AON_PIN_CTRL_REG_START, BCHP_AON_PIN_CTRL_REG_END, "AON_PIN_MUX"),
#endif
#if (BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL && BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL)
	ENT(BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL,BCHP_M2MC_CLK_GATE_AND_SW_INIT_CONTROL, "M2MC_SW_INIT"),
#endif
#if BCHP_M2MC_REG_START
	ENT(BCHP_M2MC_REG_START, BCHP_M2MC_REG_END, "M2MC"),
#endif


#if (BCHP_DVP_DGEN_0_DVO_0_TEST_DATA_GEN_CFG_14 && BCHP_MISC_MISC_REVISION_ID)
	ENT(BCHP_MISC_MISC_REVISION_ID, BCHP_DVP_DGEN_0_DVO_0_TEST_DATA_GEN_CFG_14, "VEC"),
#endif
#if (BCHP_DVP_TVG_0_TVG_3D_CFG_3 && BCHP_MISC_MISC_REVISION_ID)
	ENT(BCHP_MISC_MISC_REVISION_ID, BCHP_DVP_TVG_0_TVG_3D_CFG_3, "VEC..."),
#endif
#if (BCHP_MISC_DAC_CRC_STATUS && BCHP_MISC_MISC_REVISION_ID)
	ENT(BCHP_MISC_MISC_REVISION_ID, BCHP_MISC_DAC_CRC_STATUS, "VEC"),
#endif
#if (BCHP_MEMC_GEN_0_ZQCS_PERIOD && BCHP_MEMC_GEN_0_CORE_REV_ID)
	ENT(BCHP_MEMC_GEN_0_CORE_REV_ID, BCHP_MEMC_GEN_0_ZQCS_PERIOD, "MEMC0"),
#endif
#if (BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_AON_STORAGE_IN_PHY_3 && BCHP_MEMC_GEN_0_CORE_REV_ID)
	ENT(BCHP_MEMC_GEN_0_CORE_REV_ID, BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_0_AON_STORAGE_IN_PHY_3, "MEMC0..."),
#endif
#if (BCHP_MEMC_GEN_1_ZQCS_PERIOD && BCHP_MEMC_GEN_1_CORE_REV_ID)
	ENT(BCHP_MEMC_GEN_1_CORE_REV_ID, BCHP_MEMC_GEN_1_ZQCS_PERIOD, "MEMC1"),
#endif
#if (BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_1_AON_STORAGE_IN_PHY_3 && BCHP_MEMC_GEN_1_CORE_REV_ID)
	ENT(BCHP_MEMC_GEN_1_CORE_REV_ID, BCHP_MEMC_DDR23_SHIM_ADDR_CNTL_1_AON_STORAGE_IN_PHY_3, "MEMC1"),
#endif
	ENT(BCHP_RDC_REG_START, BCHP_RDC_REG_START, "RDC"),
	ENT(BCHP_VNET_F_REG_START, BCHP_VNET_F_REG_START, "VNET_F"),
	ENT(BCHP_VNET_B_REG_START, BCHP_VNET_B_REG_START, "VNET_B"),
#if (BCHP_MEMC_GEN_2_ZQCS_PERIOD && BCHP_MEMC_GEN_2_CORE_REV_ID)
	ENT(BCHP_MEMC_GEN_2_CORE_REV_ID, BCHP_MEMC_GEN_2_ZQCS_PERIOD, "MEMC2"),
#endif
#if (BCHP_VFD_4_SCRATCH_REGISTER_1 && BCHP_MFD_0_REVISION_ID)
	ENT(BCHP_MFD_0_REVISION_ID, BCHP_VFD_4_SCRATCH_REGISTER_1, "MFD/VFD..."),
#endif
#if (BCHP_DVP_HR_TMR_WDCTRL && BCHP_DVP_HT_CORE_REV)
	ENT(BCHP_DVP_HT_CORE_REV, BCHP_DVP_HR_TMR_WDCTRL, "DVP HT and DVP HR..."),
#endif
#if (BCHP_AON_CTRL_ANA_XTAL_EXT_CML_CONTROL && BCHP_AON_CTRL_RESET_CTRL)
	ENT(BCHP_AON_CTRL_RESET_CTRL, BCHP_AON_CTRL_ANA_XTAL_EXT_CML_CONTROL, "AON..."),
#endif
#if (BCHP_AON_HDMI_RX_I2C_PASS_THROUGH_CONFIG2 && BCHP_AON_CTRL_RESET_CTRL)
	ENT(BCHP_AON_CTRL_RESET_CTRL, BCHP_AON_HDMI_RX_I2C_PASS_THROUGH_CONFIG2, "AON..."),
#endif
#if (BCHP_AON_HDMI_RX_I2C_PASS_THROUGH_CONFIG1 && BCHP_AON_CTRL_RESET_CTRL)
	ENT(BCHP_AON_CTRL_RESET_CTRL, BCHP_AON_HDMI_RX_I2C_PASS_THROUGH_CONFIG1, "AON..."),
#endif
#if (BCHP_HDMI_RAM_PACKET_RSVD_1 && BCHP_HDMI_BKSV0)
	ENT(BCHP_HDMI_BKSV0, BCHP_HDMI_RAM_PACKET_RSVD_1, "HDMI TX Control"),
#endif
#if (BCHP_SUN_GISB_ARB_ERR_CAP_MASTER && BCHP_SUN_GISB_ARB_REVISION)
	ENT(BCHP_SUN_GISB_ARB_REVISION, BCHP_SUN_GISB_ARB_ERR_CAP_MASTER, "SUN_GISB_ARB_..."),
#endif
#if (BCHP_SM_FAST_DAA_ROM_TM && BCHP_SUN_GISB_ARB_REVISION)
	ENT(BCHP_SUN_GISB_ARB_REVISION, BCHP_SM_FAST_DAA_ROM_TM, "SYS_CTRL..."),
#endif
#if (BCHP_SUN_L2_PCI_MASK_CLEAR && BCHP_SUN_L2_CPU_STATUS)
	ENT(BCHP_SUN_L2_CPU_STATUS, BCHP_SUN_L2_PCI_MASK_CLEAR, "SUN_L2_..."),
#endif
#if (BCHP_VICH_2_RESERVED_END && BCHP_HEVD_OL_CPU_REGS_0_HST2CPU_MBX)
	ENT(BCHP_HEVD_OL_CPU_REGS_0_HST2CPU_MBX, BCHP_VICH_2_RESERVED_END, "AVD..."),
#endif
#if (BCHP_V3D_DBG_ERRSTAT && BCHP_V3D_CTL_IDENT0)
	ENT(BCHP_V3D_CTL_IDENT0, BCHP_V3D_DBG_ERRSTAT, "V3D..."),
#endif
#if (BCHP_AVS_ROSC_THRESHOLD_2_INTERRUPT_STATUS_THRESHOLD2_FAULTY_SENSOR && BCHP_AVS_HW_MNTR_SW_CONTROLS)
	ENT(BCHP_AVS_HW_MNTR_SW_CONTROLS, BCHP_AVS_ROSC_THRESHOLD_2_INTERRUPT_STATUS_THRESHOLD2_FAULTY_SENSOR, "AVS..."),
#endif
#if (BCHP_UPG_UART_DMA_RX_REQ_SEL && BCHP_IRB_BLAST_NUMSEQ)
	ENT(BCHP_IRB_BLAST_NUMSEQ, BCHP_UPG_UART_DMA_RX_REQ_SEL, "UPG..."),
#endif
#if (BCHP_WKTMR_PRESCALER_VAL && BCHP_LDK_KEYROW32)
	ENT(BCHP_LDK_KEYROW32, BCHP_WKTMR_PRESCALER_VAL, "AON UPG..."),
#endif
#if (BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_CLEAR  && BCHP_RAAGA_DSP_MISC_REVISION)
	ENT(BCHP_RAAGA_DSP_MISC_REVISION, BCHP_RAAGA_DSP_MEM_SUBSYSTEM_MEMSUB_ERROR_CLEAR ,"RAAGA..."),
#endif
#if (BCHP_RAAGA_DSP_MEM_SUBSYSTEM_1_MEMSUB_ERROR_CLEAR  && BCHP_RAAGA_DSP_MISC_1_REVISION)
	ENT(BCHP_RAAGA_DSP_MISC_1_REVISION, BCHP_RAAGA_DSP_MEM_SUBSYSTEM_1_MEMSUB_ERROR_CLEAR ,"RAAGA_1..."),
#endif
#if (BCHP_VICE2_ARCSS_MISC_0_MISC_CTRL && BCHP_VICE2_CME_0_0_FW_CONTROL)
	ENT(BCHP_VICE2_CME_0_0_FW_CONTROL, BCHP_VICE2_ARCSS_MISC_0_MISC_CTRL, "VICE2_0..."),
#endif
#if (BCHP_VICE2_ARCSS_MISC_1_MISC_CTRL && BCHP_VICE2_CME_0_1_FW_CONTROL)
	ENT(BCHP_VICE2_CME_0_1_FW_CONTROL, BCHP_VICE2_ARCSS_MISC_1_MISC_CTRL, "VICE2_1..."),
#endif
#if (BCHP_AVD_CACHE_0_REG_PCACHE_END && BCHP_DECODE_RBNODE_REGS_0_RBNODE_REGS_END)
	ENT(BCHP_DECODE_RBNODE_REGS_0_RBNODE_REGS_END, BCHP_AVD_CACHE_0_REG_PCACHE_END, "AVD..."),
#endif
#if (BCHP_AVD_CACHE_1_REG_PCACHE_END && BCHP_DECODE_IP_SHIM_1_STC0_REG)
	ENT(BCHP_DECODE_IP_SHIM_1_STC0_REG, BCHP_AVD_CACHE_1_REG_PCACHE_END, "AVD SHIM and CACHE..."),
#endif
#if (BCHP_AVD_RGR_1_SW_INIT_1 && BCHP_AVD_INTR2_1_CPU_STATUS)
	ENT(BCHP_AVD_INTR2_1_CPU_STATUS, BCHP_AVD_RGR_1_SW_INIT_1, "AVD INTR2..."),
#endif
#if (BCHP_UHFR_GR_BRIDGE_SW_INIT_1 && BCHP_UHFR_RST)
	ENT(BCHP_UHFR_RST, BCHP_UHFR_GR_BRIDGE_SW_INIT_1, "UHFR..."),
#endif
#if (BCHP_VICE2_ARCSS_MISC_MISC_CTRL && BCHP_VICE2_CME_0_FW_CONTROL)
	ENT(BCHP_VICE2_CME_0_FW_CONTROL, BCHP_VICE2_ARCSS_MISC_MISC_CTRL, "VICE2..."),
#endif
#if (BCHP_HDMI_RAM_PACKET_RSVD_1 && BCHP_HDMI_CORE_REV)
	ENT(BCHP_HDMI_CORE_REV, BCHP_HDMI_RAM_PACKET_RSVD_1, "HDMI..."),
#endif
#if (BCHP_CLK_SCRATCH && BCHP_CLK_REVISION)
	ENT(BCHP_CLK_REVISION, BCHP_CLK_SCRATCH, "BCHP_CLK_..."),
#endif

	{0, 0, "??"}
};

static uint32_t IdentifyRegister(uint32_t reg, unsigned char* name, uint32_t namelen)
{
	uint32_t result = 9999;
	uint32_t i;
	uint32_t listlen = sizeof(RegisterNames)/sizeof(*RegisterNames);

	for (i = 0; i < listlen; i++)
	{
		if (RegisterNames[i].start <= reg && reg <= RegisterNames[i].end)
		{
			result = i;
			break;
		}
	}

	/* Point to the '??' */
	if (result == 9999)
		result = listlen - 1;

	strncpy((char *)name, RegisterNames[result].desc, namelen);

	return result;
}

static void DumpRegs(FILE* fp)
{
	uint32_t	i;
	uint32_t	lastRegisterNameIndex = 9999;
	uint32_t	registerNameIndex;
	unsigned char registerName[100];

	if(g_RegDataCount == 0)
	{
		BDBG_ERR(("\n" "***************************************************************************"));
		BDBG_ERR(("Error ... Error ... Error ! " "\n" "Register Dump is empty !!! " ));
		BDBG_ERR(("Looks like you have not chosen the instrumented version of breg_mem.c"));
		BDBG_ERR(("Please pickup the instrumented version from the SPLASH_Devel branch"));
		BDBG_ERR(("***************************************************************************"));
		return;
	}
	BDBG_MSG(("Dumping register settings: %d entries", g_RegDataCount));
	fprintf(fp,  "\nstatic uint32_t s_aulReg[] =\n{\n");

	for (i = 0; i < g_RegDataCount; i++)
	{
		registerNameIndex = IdentifyRegister(g_RegData[i].reg, registerName, sizeof(registerName));

		if (lastRegisterNameIndex == 9999 || lastRegisterNameIndex != registerNameIndex)
		{
			fprintf(fp, "\n");
			fprintf(fp, "\t0x%08x, 0x%08x, /* %s */\n", g_RegData[i].reg, g_RegData[i].val, registerName);
		}
		else
		{
			fprintf(fp, "\t0x%08x, 0x%08x,\n", g_RegData[i].reg, g_RegData[i].val);
		}

		lastRegisterNameIndex = registerNameIndex;
	}

	/* close */
	/* tail */
	fprintf(fp, "};\n");
}

static void DumpFinal(FILE* fp)
{
	fprintf (fp, "\nstatic SplashData s_aSplashData = {\n");
	fprintf (fp, "\tBSPLASH_NUM_MEM,\n");
	fprintf (fp, "\tBSPLASH_RULMEMIDX,\n");
	fprintf (fp, "\tBSPLASH_NUM_SURFACE,\n");
	fprintf (fp, "\t&s_SplashSurfaceInfo[0],\n");
	fprintf (fp, "\tBSPLASH_NUM_DISPLAY,\n");
	fprintf (fp, "\t&s_SplashDisplayInfo[0],\n");
	fprintf (fp, "\tsizeof(s_aTriggerMap)/sizeof(s_aTriggerMap[0]),\n");
	fprintf (fp, "\t&s_aTriggerMap[0],\n");
	fprintf (fp, "\tsizeof(s_aulReg)/(2*sizeof(s_aulReg[0])),\n");
	fprintf (fp, "\ts_aulReg\n");
	fprintf (fp, "};\n");
	fprintf (fp, "\n");
	fprintf (fp, "SplashData* GetSplashData%s(void)\n", s_suffix_string);
	fprintf (fp, "{\n");
	fprintf (fp, "\treturn &s_aSplashData;\n");
	fprintf (fp, "}\n");
}

static void DumpHeader(FILE* fp)
{
	fprintf(fp,"/******************************************************************************\n");
	fprintf(fp," *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.\n");
	fprintf(fp," *\n");
	fprintf(fp," * This program is the proprietary software of Broadcom and/or its licensors,\n");
	fprintf(fp," *  and may only be used, duplicated, modified or distributed pursuant to the terms and\n");
	fprintf(fp," *  conditions of a separate, written license agreement executed between you and Broadcom\n");
	fprintf(fp," *  (an \"Authorized License\").  Except as set forth in an Authorized License, Broadcom grants\n");
	fprintf(fp," *  no license (express or implied), right to use, or waiver of any kind with respect to the\n");
	fprintf(fp," *  Software, and Broadcom expressly reserves all rights in and to the Software and all\n");
	fprintf(fp," *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU\n");
	fprintf(fp," *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY\n");
	fprintf(fp," *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.\n");
	fprintf(fp," *\n");
	fprintf(fp," *  Except as expressly set forth in the Authorized License,\n");
	fprintf(fp," *\n");
	fprintf(fp," *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade\n");
	fprintf(fp," *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,\n");
	fprintf(fp," *  and to use this information only in connection with your use of Broadcom integrated circuit products.\n");
	fprintf(fp," *\n");
	fprintf(fp," *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED \"AS IS\"\n");
	fprintf(fp," *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR\n");
	fprintf(fp," *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO\n");
	fprintf(fp," *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES\n");
	fprintf(fp," *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,\n");
	fprintf(fp," *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION\n");
	fprintf(fp," *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF\n");
	fprintf(fp," *  USE OR PERFORMANCE OF THE SOFTWARE.\n");
	fprintf(fp," *\n");
	fprintf(fp," *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS\n");
	fprintf(fp," *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR\n");
	fprintf(fp," *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR\n");
	fprintf(fp," *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF\n");
	fprintf(fp," *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT\n");
	fprintf(fp," *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE\n");
	fprintf(fp," *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF\n");
	fprintf(fp," *  ANY LIMITED REMEDY.\n");
	fprintf(fp," ******************************************************************************/\n");
	return;
}

static int GetArrayIndex(BRDC_Trigger eTrigger)
{
	switch(eTrigger)
	{
	case BRDC_Trigger_eVec0Trig0:
		return 0;
	case BRDC_Trigger_eVec0Trig1:
		return 1;
	case BRDC_Trigger_eVec1Trig0:
		return 2;
	case BRDC_Trigger_eVec1Trig1:
		return 3;

	case BRDC_Trigger_eVdec0Trig0:
		return 4;
	case BRDC_Trigger_eVdec0Trig1:
		return 5;

	case BRDC_Trigger_eDtgTrig0:
		return 6;
	case BRDC_Trigger_eDtgTrig1:
		return 7;

	case BRDC_Trigger_eCmp_0Trig0:
		return 8;
	case BRDC_Trigger_eCmp_0Trig1:
		return 9;
	case BRDC_Trigger_eCmp_1Trig0:
		return 10;
	case BRDC_Trigger_eCmp_1Trig1:
		return 11;

	default:
		/* printf("Don't understand this trigger %d returning -1\n", eTrigger); */
		return -1;
	}
}

/* This determines if the trigger associated with the list qualifies the list to
   the stored RUL list. This assumes that the top field trigger was used to kick-start
   RDMA. */
static bool isTrigger(BRDC_Trigger eTrigger)
{
	switch(eTrigger)
		{
		case BRDC_Trigger_eVec0Trig1:
		case BRDC_Trigger_eVec1Trig1:
		case BRDC_Trigger_eVdec0Trig1:
		case BRDC_Trigger_eDtgTrig1:
		case BRDC_Trigger_eCmp_0Trig1:
		case BRDC_Trigger_eCmp_1Trig1:
			return(true);

		case BRDC_Trigger_eVec0Trig0:
		case BRDC_Trigger_eVec1Trig0:
		case BRDC_Trigger_eVdec0Trig0:
		case BRDC_Trigger_eDtgTrig0:
		case BRDC_Trigger_eCmp_0Trig0:
		case BRDC_Trigger_eCmp_1Trig0:
		default:
			return(false);
		}
}


extern BERR_Code  splash_vdc_setup
	( BCHP_Handle         hChp,
	  BREG_Handle         hReg,
	  BINT_Handle         hInt,
	  BTMR_Handle         hTmr,
	  BBOX_Handle         hBox,
	  ModeHandles        *pState );

int splash_generate_script
   ( BCHP_Handle hChp,
	 BREG_Handle hReg,
	 BINT_Handle hInt,
	 BREG_I2C_Handle hRegI2c,
	 BBOX_Handle     hBox )
{
	BFMT_VideoInfo   stVideoInfo;
	BTMR_Handle      hTmr = 0;
	BTMR_DefaultSettings stTimerSettings;
	BRDC_Settings stRDCSettings;
	int ii;
	FILE* fp;
	char filename[128];
	BERR_Code eErr = BERR_SUCCESS;

	/* error reporting */
	BDBG_SetLevel(BDBG_eErr);
	BDBG_SetModuleLevel("BRDC_DBG", BDBG_eErr);
	BDBG_SetModuleLevel("splash_setup_vdc", BDBG_eMsg);

	/* Init globals */
	for(ii=0;ii<MAX_NUM_TRIGGERS;ii++)
		g_bFirstBottom[ii] = true;

	for (ii=0; ii<SPLASH_NUM_DISPLAY; ii++)
	{
		SplashSurface *pSurf = g_stModeHandles.disp[ii].pSurf;

		if (pSurf->ulWidth == 0)
		{
			TestError( BFMT_GetVideoFormatInfo(g_stModeHandles.disp[ii].eDispFmt, &stVideoInfo),
				"ERROR:BFMT_GetVideoFormatInfo" );
			pSurf->ulWidth = stVideoInfo.ulWidth;
			pSurf->ulHeight = stVideoInfo.ulHeight;
		}
	}

	BTMR_GetDefaultSettings(&stTimerSettings);
	eErr = BTMR_Open(&hTmr, hChp, hReg, hInt, &stTimerSettings);
	if ( eErr )
	{
		BDBG_ERR(("Bad TMR return!"));
		goto done;
	}

	/* Configure anything platform-specific */
	TestError( ConfigPlatform(hReg), "ERROR: ConfigPlatform");

	for (ii=0; ii<MAX_NUM_TRIGGERS; ii++)
	{
		g_asTriggerMap[ii].SlotNum = -1;
		g_asTriggerMap[ii].TriggerHwNum = -1;
	}

	/* Init I2C Reg handle */
	g_stModeHandles.hRegI2c = hRegI2c;

	/* open Register DMA */
	stRDCSettings.pfnSlot_SetList_Intercept_isr = APP_BRDC_Slot_SetList_isr;
	stRDCSettings.pfnSlot_Execute_Intercept_isr = APP_BRDC_Slot_Execute_isr;
	stRDCSettings.pfnSlot_ExecuteOnTrigger_Intercept_isr =
		APP_BRDC_Slot_ExecuteOnTrigger_isr;
	TestError( BRDC_Open(&(g_stModeHandles.hRdc), hChp, hReg, g_stModeHandles.hRulMem, &stRDCSettings),
		"ERROR: BRDC_Open" );

	/* initialize static storage */
	InitializeStorage(g_stModeHandles.hRdc);

	/* setup VDC mode */
	BDBG_MSG(("Initializing display and surface"));
	TestError( splash_vdc_setup(
				hChp,
				hReg,
				hInt,
				hTmr,
				hBox,
				&g_stModeHandles),
			"ERROR: splash_vdc_setup" );

	/* ensure that all lists have been created */
	BDBG_WRN(("System is settling down. Please wait..."));
	for(ii=0;ii<5;ii++)
	{
		BKNI_Sleep(1000);
	}

	/* Write the dummy header file */
	sprintf (filename, "splash_vdc_rul%s.h", s_suffix_string);
	fp = fopen(filename, "w");
	if(!fp)
	{
		BDBG_ERR(("ERROR: cannot open file '%s' %d", filename, errno));
		/* TODO: choose a more specific error code. */
		eErr = BERR_UNKNOWN;
		goto done;
	}

	DumpHeader(fp);
	fprintf(fp, "#ifndef SPLASH_VERSION_2\n");
	fprintf(fp, "#define SPLASH_VERSION_2\n");
	fprintf(fp, "#endif\n");
	fprintf(fp, "SplashData* GetSplashData%s (void);\n", s_suffix_string);
	fclose(fp);

	/* Open a file for writing results */
	sprintf (filename, "splash_vdc_rul%s.c", s_suffix_string);
	fp = fopen(filename, "w");
	if(!fp)
	{
		BDBG_ERR(("ERROR: cannot open file '%s' %d", filename, errno));
		/* TODO: choose a more specific error code. */
		eErr = BERR_UNKNOWN;
		goto done;
	}
	BDBG_MSG(("Writing output to %s", filename));

	/* file header */
	DumpHeader(fp);
	fprintf(fp,"/***************************************************************************\n");
	fprintf(fp," File : splash_vdc_rul%s.h\n", s_suffix_string);
	fprintf(fp," Date  : %s\n", __DATE__ " " __TIME__);
	fprintf(fp," Description : Outputs for splash BVN configuration\n");
	fprintf(fp," ***************************************************************************/\n\n");
	fprintf(fp,  "#include \"splash_script_load.h\"\n\n");

	/* dump out all rul lists */
	DumpLists(fp, &g_stModeHandles);

	/* dump out all reg writes */
	DumpRegs(fp);

	/* Pull it together into one piece */
	DumpFinal(fp);

	/* Finished writing file */
	fclose(fp);

	printf("Press Enter to continue .....\n");
	getchar();
done:
	/* close VDC */
	#ifndef BVDC_FOR_BOOTUPDATER
	TestError( close_mode(&g_stModeHandles),  "ERROR: close_mode" );
	UnInitializeStorage();
	BRDC_Close(g_stModeHandles.hRdc);
	#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
	if (hTmr) BTMR_Close(hTmr);
	return (BERR_Code)eErr;
}

/* used in BREG_Write32 when macro BREG_CAPTURE is defined */
void APP_BREG_Write32
(
	BREG_Handle  hReg,
	uint32_t     ulReg,
	uint32_t     ulValue
)
{
	BSTD_UNUSED(hReg);

	/* do we want to dump out the register? */
	/* Ignore all writes for interrupts */
	/* Ignore all writes for XPT */
	/* Ignore all writes to register DMA */
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_ext_sys_sw_init_SHIFT
	if(!BSPLASH_REGDUMP_EXCLUDE(ulReg) &&
	   !(ulReg == BCHP_SUN_TOP_CTRL_SW_INIT_0_SET && ulValue == BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_ext_sys_sw_init_MASK) &&
	   !(ulReg ==  BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR && ulValue == BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_ext_sys_sw_init_MASK))
#elif defined(BCHP_SUN_TOP_CTRL_SW_RESET)
	if(!BSPLASH_REGDUMP_EXCLUDE(ulReg) &&
	   !(ulReg == BCHP_SUN_TOP_CTRL_SW_RESET && ulValue == BCHP_SUN_TOP_CTRL_SW_RESET_ext_sys_reset_1shot_MASK))
#else
#error "Port required for splashgen"
#endif
	{
		/*printf("reg count %d x%08x,	0x%08x \n", g_RegDataCount, ulReg, ulValue);*/
		/* dump register */
		if (g_RegDataCount < MAX_REG_ENTRIES)
		{
			g_RegData[g_RegDataCount].reg = ulReg;
			g_RegData[g_RegDataCount].val = ulValue;
			g_RegDataCount++;
		}

	}
}

/* called in BRDC_P_Slot_SetList_isr and BRDC_P_Slot_SetListDual_isr */
static BERR_Code APP_BRDC_Slot_SetList_isr
(
	BRDC_Slot_Handle hSlot,
	BRDC_List_Handle hList,
	BRDC_Trigger     eTrigger
)
{
	int i, j;
	bool bMatch = false;
	uint32_t ulNumEntries;
	uint32_t *pulStartAddress;
	uint32_t ulNumEntriesStored;
	uint32_t *pulStartAddressStored;
	BRDC_List_Handle  hNewList;
	int iTriggerIndex;

	/* get information on new list */
	BRDC_List_GetNumEntries_isr(hList, &ulNumEntries);
	pulStartAddress = BRDC_List_GetStartAddress_isr(hList);

	BDBG_MSG(("Slot_SetList_isr: hSlot[0x%x]; hList(0x%x) (%d) : %d", (uint32_t)hSlot, (uint32_t)hList, eTrigger, ulNumEntries));

	/* Search for the list and slot in existing list. */
	for(i=g_iListCount-1; i>=0; --i)
	{
		/* Find a matching slot/list entry. Note that the same list is used for both top and bottom field slots.
		   Frames only use the top field slot. */
		if ((g_aHandleList[i] == hList) &&
			(g_aHandleSlot[i] == hSlot))
		{
			/* match found */
			break;
		}
	}

	/* If slot/list entry is found, compare the entries with the stored slot/list entry.
	   If a match is found, don't store else store. If a match is found but the entries are
	   different clear the stored slot/list entry and overwrite with the new slot/list.
	 */
	if(i>=0)
	{
		BRDC_SlotId slotId;
		BRDC_Slot_GetId(hSlot, &slotId);
		BDBG_MSG(("Found slot %d for hList %d [%p] ; g_aRulList [%p]", slotId, i,
			(void*)hList, (void*)g_aRulList[i]));

		/* get information on stored list */
		BRDC_List_GetNumEntries_isr(g_aRulList[i], &ulNumEntriesStored);
		pulStartAddressStored = BRDC_List_GetStartAddress_isr(g_aRulList[i]);

		/* number of elements the same? */
		if(ulNumEntriesStored == ulNumEntries)
		{
			/* compare elements */
			bMatch = true;
			for(j=0; j<(int)ulNumEntries; j++)
			{
				/* element different? */
				if(*(pulStartAddress + j) != *(pulStartAddressStored + j))
				{
					if(true) /* !BRDC_IsScratchReg(g_stModeHandles.hRdc, *(pulStartAddress+j-1)) ) */
					{
						BDBG_MSG(("SlotList: Cleared %d [0x%8.8x] (element different: #%d of %d)", i, (uint32_t)hList, j+1, ulNumEntries));
						bMatch = false;
						break;
					}
					#if 0
					else
					{
						printf("************ Mismatch in Scratch pad area - ignoring (%d)!!\n", j);
					}
					#endif
				}
			}
		}
		else
		{
			BDBG_MSG(("SlotList: Cleared %d [0x%8.8x] (sizes different: %d vs. %d)", i, (uint32_t)hList, ulNumEntriesStored, ulNumEntries));
		}

		/* A match is found so skip storing it. */
		if(bMatch)
		{
			/* no need to update list */
			/* printf("SetList_isr: OLD LIST\n"); */
			BDBG_MSG(("Match for slot %d", slotId));
			goto done;
		}
		else /* Not a match so clear slot/list entry and store new slot/list. */
		{
			BDBG_MSG(("No match for slot %d, clearing list", slotId));
			/* clear out the handle since this is not active any longer */
			/* printf("Clearing...\n"); */
			g_aHandleList[i] = NULL;
		}
	}

	/* new list -- no more storage? */
	/* printf("SetList_isr: NEW LIST... (%x, %d)\n", (uint32_t)hList, ulNumEntries);*/
	if((g_iListCount == MAX_RUL_COUNT) ||
		(ulNumEntries > MAX_RUL_ENTRIES))
	{
		/* cannot store */
		BDBG_ERR(("ERROR: %d %d -No pre-allocation available for list.",g_iListCount,ulNumEntries));
	}

	/* Store new slot/list */
	g_aHandleList[g_iListCount] = hList;
	g_aHandleSlot[g_iListCount] = hSlot;
	hNewList = g_aRulList[g_iListCount];
	BRDC_List_SetNumEntries_isr(hNewList, ulNumEntries);
	pulStartAddressStored = BRDC_List_GetStartAddress_isr(hNewList);
	for(j=0; j<(int)ulNumEntries; j++)
	{
		/* copy */
		*(pulStartAddressStored + j) = *(pulStartAddress + j);
	}

	/* store as top list for that trigger */
	iTriggerIndex = GetArrayIndex(eTrigger);

	if (-1 == iTriggerIndex)
	{
		BDBG_MSG(("trigger index is -1"));
		return BERR_SUCCESS;
	}

	g_aTopLists[iTriggerIndex] = g_iListCount;

	/* Add the list. The trigger is used to determine if the list is
	   a candidate to be added. There are 2 qualifying triggers:
	   1. The trigger was used for kick-starting the RDMA - this is typically
	      associated with the top field
	   2. Since VEC/COMP triggers are pairs, the 2nd part of the trigger pair -
	      this typically associated with the bottom field.
	 */
	if(isTrigger(eTrigger) || g_bExecuteList[iTriggerIndex])
	{
		/* first bottom field? */
		if(g_bFirstBottom[iTriggerIndex] && isTrigger(eTrigger))
		{
			/* ignore first bottom field */
			g_bFirstBottom[iTriggerIndex] = false;

		/* not the first bottom */
		} else
		{
			/* add to list */
			g_aListOrder[iTriggerIndex][g_aListOrderCount[iTriggerIndex]++] =
				g_iListCount;
			BDBG_MSG(("SlotList: Added	 %d [0x%8.8x] TriggerIndex=%d", g_iListCount, (uint32_t)hList, iTriggerIndex));
		}
	}

	/* Update Trigger Map */
	{
		BRDC_SlotId slotId;
		const BRDC_TrigInfo *trigInfo = BRDC_Trigger_GetInfo(g_stModeHandles.hRdc, eTrigger);
		BRDC_Slot_GetId(hSlot, &slotId);
		BDBG_MSG(("Slot number %d, Trigger enum %d Trigger HW num %d TriggerIndex %d", slotId, eTrigger, trigInfo->ulTrigVal, iTriggerIndex));
		g_asTriggerMap[iTriggerIndex].SlotNum = slotId;
		g_asTriggerMap[iTriggerIndex].TriggerHwNum = trigInfo->ulTrigVal;
	}

	/* Added the new list element and update the slot/list entry count. */
	g_iListCount++;

done:
	return BERR_SUCCESS;
}

/* called in BRDC_Slot_ExecuteOnTrigger_isr */
static BERR_Code APP_BRDC_Slot_ExecuteOnTrigger_isr
(
	BRDC_Slot_Handle hSlot,
	BRDC_Trigger     eRDCTrigger,
	bool             bRecurring
)
{
	BRDC_List_Handle hList;
	uint32_t ulNumEntries;

	BSTD_UNUSED(bRecurring);

	/* get list that was executed without trigger
	   (only one should be this way) */
	BRDC_Slot_GetList_isr(hSlot, &hList);
	BRDC_List_GetNumEntries_isr(hList, &ulNumEntries);
	BDBG_MSG(("ExecuteOnTrigger_isr: hSlot[0x%x]; hList[0x%x] : (%d) : %d", (uint32_t)hSlot,  (uint32_t)hList, eRDCTrigger, ulNumEntries));

	switch(eRDCTrigger)
	{
	case BRDC_Trigger_eCap0Trig0:
		BDBG_MSG(("BRDC_Trigger_eCap0Trig0"));
		break;
	case BRDC_Trigger_eCap0Trig1:
		BDBG_MSG(("BRDC_Trigger_eCap0Trig1"));
		break;
	case BRDC_Trigger_eVec0Trig0:
		BDBG_MSG(("BRDC_Trigger_eVec0Trig0"));
		break;
	case BRDC_Trigger_eVec0Trig1:
		BDBG_MSG(("BRDC_Trigger_eVec0Trig1"));
		break;

	case BRDC_Trigger_eVec1Trig0:
		BDBG_MSG(("BRDC_Trigger_eVec1Trig0"));
		break;
	case BRDC_Trigger_eVec1Trig1:
		BDBG_MSG(("BRDC_Trigger_eVec1Trig1"));
		break;

	case BRDC_Trigger_eCmp_0Trig0:
		BDBG_MSG(("BRDC_Trigger_eCmp_0Trig0"));
		break;
	case BRDC_Trigger_eCmp_0Trig1:
		BDBG_MSG(("BRDC_Trigger_eCmp_0Trig1"));
		break;
	case BRDC_Trigger_eCmp_1Trig0:
		BDBG_MSG(("BRDC_Trigger_eCmp_1Trig0"));
		break;
	case BRDC_Trigger_eCmp_1Trig1:
		BDBG_MSG(("BRDC_Trigger_eCmp_1Trig1"));
		break;

	default:
		BDBG_MSG(("(%d)Trigger unknown" , eRDCTrigger));
		break;
	}

	return BERR_SUCCESS;
}
/* called in BRDC_Slot_Execute_isr */
static BERR_Code APP_BRDC_Slot_Execute_isr
(
	BRDC_Slot_Handle hSlot,
	BRDC_Trigger	 eTrigger
)
{
	BRDC_List_Handle hList;
	uint32_t ulNumEntries;
	int iTriggerIndex;

	/* printf("Execute_isr\n"); */

	/* get list that was executed without trigger
	   (only one should be this way) */
	BRDC_Slot_GetList_isr(hSlot, &hList);
	BRDC_List_GetNumEntries_isr(hList, &ulNumEntries);
	BDBG_MSG(("Execute_isr: t: hSlot[0x%x]; hList[0x%x] : (%d) : %d", (uint32_t)hSlot,  (uint32_t)hList, eTrigger, ulNumEntries));

	/* which trigger fired? */
	iTriggerIndex = GetArrayIndex(eTrigger);
	if(-1 == iTriggerIndex) return BERR_SUCCESS;

	/* add list that was executed to start of list */
	g_aListOrder[iTriggerIndex][0] = g_aTopLists[iTriggerIndex];
	g_aListOrderCount[iTriggerIndex]++;

	/* a list has been executed. At this point, keep all new lists */
	g_bExecuteList[iTriggerIndex] = true;

	return BERR_SUCCESS;
}

/* End of file */
