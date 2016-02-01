/***************************************************************************
 *     Copyright (c) 2004-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bvdc_compositor_priv.h"
#include "bvdc_pep_priv.h"

BDBG_MODULE(BVDC_PEP);

#if (BVDC_P_SUPPORT_PEP)
/***************************************************************************
 * {private}
 * This section is for the contrast stretch algorithm
 */
#define BVDC_P_PEP_LUMA_BITS            8

/* fixed to integer */
#define BVDC_P_PEP_FIXTOI(x) \
	((int32_t)((x) >> BVDC_P_PEP_FIX_FRACTIONAL_SHIFT))

#define BVDC_P_PEP_FIX_MUL_CONST(x, k) ((int32_t)(x * k))
#define BVDC_P_PEP_FIX_DIV_CONST(x, k) ((int32_t)(x / k))

#define BVDC_P_PEP_FIX_FRACTIONAL_HALFSHIFT  (BVDC_P_PEP_FIX_FRACTIONAL_SHIFT / 2)
#define BVDC_P_PEP_7BIT_FRAC_SHIFT           7
#define BVDC_P_PEP_COLOR_SAT_BIN_OFFSET      2
#define BVDC_P_PEP_COLOR_SAT_MAX             127
#define BVDC_P_PEP_COLOR_SAT_MIN             -128
#define BVDC_P_PEP_LAB_COUNTS_PER_BIN        (((BVDC_P_PEP_MAX_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)) + 1) / BVDC_P_LAB_TABLE_SIZE)
#define BVDC_P_PEP_NUMBER_OF_BLACK_LAB_BINS  ((BVDC_P_PEP_BLACK_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS))/ BVDC_P_PEP_LAB_COUNTS_PER_BIN)

#define BVDC_P_PEP_PWM_MAX_APL               130
#define BVDC_P_PEP_PWM_MIN_APL               70
#define BVDC_P_PEP_PWM_MIN_PERCENT           40

/* This macro convert float to fix point format.  Float is in 100th. */
/* i.e: to convert 0.1 to fixed, x = 100.  To convert 1.125 to fixed */
/* x = 1125 */
#define BVDC_P_PEP_FTOFIX(x)   ((x << BVDC_P_PEP_FIX_FRACTIONAL_SHIFT) / 1000)

#define BVDC_P_PEP_4TH_ROOT_OF_4_FIX        0x0016A09E
#define BVDC_P_PEP_8TH_ROOT_OF_4_FIX        0x001306FE
#define BVDC_P_PEP_12TH_ROOT_OF_4_FIX       0x0011F59A
#define BVDC_P_PEP_16TH_ROOT_OF_4_FIX       0x001172B8

#define BVDC_P_PEP_FIX_MUL(x, y) \
	((int32_t)(((x) >> BVDC_P_PEP_FIX_FRACTIONAL_HALFSHIFT) * ((y) >> BVDC_P_PEP_FIX_FRACTIONAL_HALFSHIFT)))
#define BVDC_P_PEP_FIX_DIV(x, y) \
	(int32_t)((((x) << BVDC_P_PEP_FIX_FRACTIONAL_HALFSHIFT) / (y)) << BVDC_P_PEP_FIX_FRACTIONAL_HALFSHIFT)
#define BVDC_P_PEP_FRAC_TO_7BITFRAC(x) \
	((int32_t)((x) >> (BVDC_P_PEP_FIX_FRACTIONAL_SHIFT - BVDC_P_PEP_7BIT_FRAC_SHIFT)))

#define BVDC_P_DC_TABLE_COLS                 13

/* Shift values before filtering to allow closer convergence of filter result to input value */
#define BVDC_P_FILT_SHIFT                    7

static const int32_t s_alIRE[BVDC_P_DC_TABLE_COLS - 1] = {
	/* IRE0, IRE?, IRE10, IRE20, IRE30, IRE40, IRE50, IRE60, IRE70, IRE80, IRE90, IRE100 */
	   64,   100,  152,   239,   327,   414,   502,   590,   677,   765,   852,   940
};

static const uint32_t s_alDCTable1[BVDC_DC_TABLE_ROWS * BVDC_P_DC_TABLE_COLS] = {
	/* Used for settop-box */
	 64, 64, 100, 190, 382, 550, 658, 735, 799, 826, 874, 916, 940,
	 75, 64, 100, 190, 382, 550, 658, 735, 799, 826, 874, 916, 940,
	124, 64,  99, 170, 310, 442, 560, 650, 722, 790, 840, 890, 940,
	198, 64,  97, 160, 260, 406, 511, 601, 680, 753, 820, 880, 940,
	275, 64,  94, 150, 235, 360, 460, 560, 650, 735, 815, 880, 940,
	349, 64,  90, 140, 215, 328, 430, 531, 629, 725, 800, 870, 940,
	425, 64,  85, 130, 200, 305, 418, 515, 624, 718, 795, 870, 940,
	497, 64,  80, 120, 200, 292, 409, 508, 619, 711, 790, 866, 940,
	572, 64,  80, 120, 200, 290, 395, 491, 602, 700, 783, 863, 940,
	652, 64,  80, 120, 200, 290, 388, 481, 581, 684, 773, 860, 940,
	723, 64,  80, 120, 200, 288, 384, 473, 569, 676, 768, 856, 940,
	799, 64,  80, 120, 200, 282, 378, 469, 563, 661, 743, 840, 940,
	875, 64,  80, 120, 200, 277, 375, 465, 558, 659, 742, 835, 940,
	924, 64,  80, 120, 200, 275, 372, 463, 555, 650, 742, 834, 940,
	940, 64,  80, 120, 200, 275, 372, 463, 555, 650, 742, 834, 940
};

/*
 * This function implement the dynamic contrast as if it was for 10 bit 1024
 * entry LAB table, then interpolate the luma value by sub sample it down to
 * 128 entries.  This algorithm to work with the new LAB table in the VIVID
 * block
 */
static void BVDC_P_Pep_DynamicContrastInterpolate_isr
	( const BVDC_ContrastStretch  *pCS,
	  BVDC_P_PepContext           *pPep,
	  uint32_t                    *pulLabTable )
{
	uint32_t        id;
	int32_t         ulInputAPL, ulTotal = 0;
	int32_t         lLoBits, lHiBits, lLoFrac, lHiFrac;
	int             iGain;
	uint32_t        lPwmSetting;
	int32_t        *plLowerAPL, *plUpperAPL;
	int32_t         lTableInter, lIREInter;
	int32_t         lCurrentLevel;
	int32_t         lLowerVal, lUpperVal, lLimitVal;
	uint32_t        ulDcTableCol;
	const uint32_t *plTablePointerA;
	const int32_t  *plLowerIRE, *plUpperIRE;
	int32_t        *plLowLowDCT, *plUppLowDCT, *plLowUppDCT, *plUppUppDCT, *plTablePointer;
	int32_t         lStraightLineRatio;
	int32_t         lLoBitsMask, lHiBitsShift;
	int32_t         ulInputBin;

#if PEP_DYN_CONTRAST_DBG
	int32_t         lHiRatio, lTableRatio;
	int32_t         sSat;
	bool            bDbg = false;
	uint32_t        ulDbgCnt;
#endif

	BDBG_ENTER(BVDC_P_Pep_DynamicContrastInterpolate_isr);

	/*----------------------------------*/
	/* Read histogram and fill in bins  */
	/* Average bins across 3 fields     */
	/* and find total of all bins       */
	for(id = 0; id < pPep->ulHistSize; id++)
	{
		/* average out the bins of the previous 3 frames */
		pPep->aulBin[id] = (pPep->aulLastLastBin[id] + pPep->aulLastBin[id] + pPep->stHistoData.aulHistogram[id]) / 3;
		ulTotal += pPep->aulBin[id];
	}
	ulInputAPL = 0;
	for(id = 0; id < pPep->ulHistSize; id++)
	{
		ulInputBin = (pPep->aulBin[id] * (2 * id + 1));
		ulInputAPL += ulInputBin;
	}
	ulInputAPL = ulInputAPL * ((((BVDC_P_PEP_MAX_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)) + 1) / 2) / pPep->ulHistSize);

	/* Exit if no pixels counted, or LabTable not defined */
	if(ulTotal == 0 || pulLabTable == NULL)
	{
		pPep->bLabTableValid = false;
		BDBG_MSG(("Histogram is not ready"));
		BDBG_LEAVE(BVDC_P_Pep_DynamicContrastInterpolate_isr);
		return;
	}
	else
	{
		pPep->bLabTableValid = true;
	}

	/* Convert gains to internal fixed point format used by dynamic contrast */
	if(pCS->ulShift > BVDC_P_PEP_FIX_FRACTIONAL_SHIFT)
	{
		iGain = pCS->iGain >> (pCS->ulShift - BVDC_P_PEP_FIX_FRACTIONAL_SHIFT);
	}
	else if(pCS->ulShift < BVDC_P_PEP_FIX_FRACTIONAL_SHIFT)
	{
		iGain = pCS->iGain << (BVDC_P_PEP_FIX_FRACTIONAL_SHIFT - pCS->ulShift);
	}
	else
	{
		iGain = pCS->iGain;
	}

#if PEP_DYN_CONTRAST_DBG
	ulDbgCnt = BREG_Read32(pPep->hReg, BCHP_PEP_CMP_0_V0_SCRATCH_REGISTER);
	if((ulDbgCnt & 0x3f) == 0)
	{
		bDbg = true;
	}

	if(bDbg)
	{
		for(id = 0; id < pPep->ulHistSize; id++)
		{
			BDBG_MSG(("pPep->Bin[%d] = %d", id, pPep->aulBin[id]));
		}
		BDBG_MSG(("Total = %d   = 0x%x", ulTotal, ulTotal));
		BDBG_MSG(("pCS->iGain   = 0x%x", pCS->iGain));
		BDBG_MSG(("ulShift      = %d", pCS->ulShift));
		BDBG_MSG(("iGain        = 0x%x", iGain));
		BDBG_MSG(("bBypassSat   = %s", pCS->bBypassSat ? "true" : "false"));
	}
#endif

	if(iGain != 0)
	{
		lStraightLineRatio = (BVDC_P_PEP_FTOFIX(1000) - iGain);
		if(lStraightLineRatio < 0)
		{
			lStraightLineRatio = 0;
		}
		if(lStraightLineRatio > BVDC_P_PEP_FTOFIX(1000))
		{
			lStraightLineRatio = BVDC_P_PEP_FTOFIX(1000);
		}

		if(pCS->aulDcTable1[0] == 0)
		{
			/* If user has not provided DC structure, then use default table */
			plTablePointerA = &s_alDCTable1[0];
			plLowerIRE = &s_alIRE[0];
			ulDcTableCol = BVDC_P_DC_TABLE_COLS;
		}
		else
		{
			plTablePointerA = &pCS->aulDcTable1[0];
			plLowerIRE = &pCS->alIreTable[0];
			ulDcTableCol = BVDC_DC_TABLE_COLS;
		}

		ulInputAPL = ulInputAPL / ulTotal;
		if(ulInputAPL < (BVDC_P_PEP_BLACK_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)))
			ulInputAPL = (BVDC_P_PEP_BLACK_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS));
		if(ulInputAPL > (BVDC_P_PEP_WHITE_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)))
			ulInputAPL = (BVDC_P_PEP_WHITE_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS));
		plTablePointer = &pPep->alDCTableTemp[0];

		/* Do not support 2 tables for Settop Box products */
		/* Copy DCTable1 to the temporary buffer if table interpolation is not used */
		for(id = 0; id < (BVDC_DC_TABLE_ROWS * ulDcTableCol); id++)
		{
			*(plTablePointer + id) = *(plTablePointerA + id) >> (10 - BVDC_P_PEP_LUMA_BITS);
		}

#if PEP_DYN_CONTRAST_DBG
		lHiRatio = 0;
		lTableRatio = 0;
		if(bDbg)
		{
			bool bInterpolateTablesFlag = pCS->bInterpolateTables;
			bInterpolateTablesFlag = false;
			BDBG_MSG(("Input APL       = %9d", ulInputAPL));
			BDBG_MSG(("Filt APL        = %9d", (pPep->ulFiltAPL >> BVDC_P_FILT_SHIFT)));
			BDBG_MSG(("iGain           = %9d", iGain));
			BDBG_MSG(("Interpolate     = %s ", bInterpolateTablesFlag ? "true" : "false"));
			if(bInterpolateTablesFlag)
			{
				BDBG_MSG(("LevelStats 5%%  = %9d", pPep->stHistoData.aulLevelStats[pCS->ulDcLoThresh]));
				BDBG_MSG(("LevelStats 95%% = %9d", pPep->stHistoData.aulLevelStats[pCS->ulDcHiThresh]));
				BDBG_MSG(("Hi Ratio        = %9d", lHiRatio));
				BDBG_MSG(("Table Ratio     = %9d", lTableRatio));
			}
			BDBG_MSG(("StrLine Ratio   = %9d", lStraightLineRatio));
/*			BDBG_MSG(("Table Pointer   = %8d", (int)plTablePointer)); */
		}
#endif
		/* Find the two rows in DCTables array to use */
		/* plLowerAPL points to the row with the lower APL, plUpperAPL points to the row with the higher APL */
		for(id = 0; id < (BVDC_DC_TABLE_ROWS - 1); id++)
		{
			plLowerAPL = (plTablePointer + (ulDcTableCol *  id   ) + 0);
			plUpperAPL = (plTablePointer + (ulDcTableCol * (id+1)) + 0);
			/* Settop Box bypasses the calculations of ulFiltAPL, so need to */
			/* copy ulInputAPL to ulFiltAPL here */
			pPep->ulFiltAPL = (ulInputAPL << BVDC_P_FILT_SHIFT);
			if((int)(pPep->ulFiltAPL >> BVDC_P_FILT_SHIFT) <= *plUpperAPL)
			{
				/* Found correct range */
				break;
			}
		}

		/* Calculate interpolation factor between points along each line of DC Table */
		/* if DC tables from user contain invalid data which cause plUpperAPL and */
		/* plLowerAPL to be the same, default behavior to divide by 1 instead */
		lTableInter = BVDC_P_PEP_FIX_DIV(((pPep->ulFiltAPL >> BVDC_P_FILT_SHIFT) - *plLowerAPL), BVDC_P_MAX(1,(*plUpperAPL - *plLowerAPL)));
		/* pLowLow points to the first point on the LowerAPL line; pUppLow points to the second point on the LowerAPL line */
		plLowLowDCT = plLowerAPL  + 1;
		plUppLowDCT = plLowLowDCT + 1;
		plUpperIRE  = plLowerIRE  + 1;
		/* pLowUpp points to the first point on the UpperAPL line; pUppUpp points to the second point on the UpperAPL line */
		plLowUppDCT = plUpperAPL  + 1;
		plUppUppDCT = plLowUppDCT + 1;

#if PEP_DYN_CONTRAST_DBG
		if(bDbg)
		{
			BDBG_MSG(("LowerAPL = %4d, UpperAPL = %4d, lTableInter = %10d", *plLowerAPL, *plUpperAPL, lTableInter));
		}

#endif
		/* Fill in BVDC_P_PEP_LAB_GEN_SIZE-entry LAB table */
		for(id = 0; id < BVDC_P_PEP_LAB_GEN_SIZE; id++)
		{
			lCurrentLevel = ((id + 1) * (((BVDC_P_PEP_MAX_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)) + 1) / BVDC_P_PEP_LAB_GEN_SIZE));
			if(lCurrentLevel < (BVDC_P_PEP_BLACK_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)))
			{
				/* Straight line outside visible range.  Allow for lowest values != 64 */
				lLowerVal = BVDC_P_PEP_ITOFIX(*plLowLowDCT);
				lUpperVal = BVDC_P_PEP_ITOFIX(*plLowUppDCT);
				lLimitVal = BVDC_P_PEP_FIX_MUL((BVDC_P_PEP_ITOFIX(1) - lTableInter), lLowerVal) + BVDC_P_PEP_FIX_MUL(lTableInter, lUpperVal);
				pPep->lFixHist_out[id] = BVDC_P_PEP_FIX_MUL((lLimitVal / (BVDC_P_PEP_BLACK_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS))), BVDC_P_PEP_ITOFIX(lCurrentLevel));
			}
			else if(lCurrentLevel > (BVDC_P_PEP_WHITE_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)))
			{
				/* Straight line outside visible range.  Allow for highest values != 940 */
				lLowerVal = BVDC_P_PEP_ITOFIX(*plUppLowDCT);
				lUpperVal = BVDC_P_PEP_ITOFIX(*plUppUppDCT);
				lLimitVal = BVDC_P_PEP_FIX_MUL((BVDC_P_PEP_ITOFIX(1) - lTableInter), lLowerVal) + BVDC_P_PEP_FIX_MUL((lTableInter), lUpperVal);
				/* Reduced slope above visible range */
				pPep->lFixHist_out[id] = lLimitVal + (BVDC_P_PEP_ITOFIX(lCurrentLevel - (BVDC_P_PEP_WHITE_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS))) >> 1);
			}
			else
			{
				/* Interpolate for visible range */
				if(lCurrentLevel > (*plUpperIRE >> (10 - BVDC_P_PEP_LUMA_BITS)))
				{
					/* Move to next set of table entries */
					plLowerIRE++;
					plUpperIRE++;
					plLowLowDCT++;
					plUppLowDCT++;
					plLowUppDCT++;
					plUppUppDCT++;
				}
				/* if IRE table from user contains invalid data which cause plUpperIRE and */
				/* plLowerIRE to be the same, default behavior to divide by 1 instead */
				lIREInter = BVDC_P_PEP_FIX_DIV((lCurrentLevel - (*plLowerIRE >> (10 - BVDC_P_PEP_LUMA_BITS))),
					BVDC_P_MAX(1, ((*plUpperIRE - *plLowerIRE) >> (10 - BVDC_P_PEP_LUMA_BITS))));
				/* First interpolate along each line of the DCTable */
				lLowerVal = ((BVDC_P_PEP_ITOFIX(1)-lIREInter) * (*plLowLowDCT)) + ((lIREInter) * (*plUppLowDCT));
				lUpperVal = ((BVDC_P_PEP_ITOFIX(1)-lIREInter) * (*plLowUppDCT)) + ((lIREInter) * (*plUppUppDCT));
				/* Then interpolate between the two lines of the DCTable */
				pPep->lFixHist_out[id] = BVDC_P_PEP_FIX_MUL((BVDC_P_PEP_ITOFIX(1) - lTableInter), lLowerVal) + BVDC_P_PEP_FIX_MUL((lTableInter), lUpperVal);

#if PEP_DYN_CONTRAST_DBG
				if(bDbg)
				{
					if((id > 4) && (id < 12))
					{
						BDBG_MSG(("id = %4d, lCurrentLevel = %4d, LowerIRE = %4d, UpperIRE = %4d, lIREInter = %10d", id, lCurrentLevel, *plLowerIRE, *plUpperIRE, lIREInter));
						BDBG_MSG(("     LowLow = %4d, LowUpp = %4d, UppLow = %4d, UppUpp = %4d", *plLowLowDCT, *plUppLowDCT, *plLowUppDCT, *plUppUppDCT));
						BDBG_MSG(("     lLowerVal = %4d, lUpperVal = %4d, Lab = %10d", lLowerVal, lUpperVal, pPep->lFixHist_out[id]));
					}
				}
#endif
			}

			/* Blend with straight line, depending on the User-controlled iGain setting */
			pPep->lFixHist_out[id] = BVDC_P_PEP_FIX_MUL((BVDC_P_PEP_ITOFIX(1) - lStraightLineRatio), pPep->lFixHist_out[id]) + BVDC_P_PEP_FIX_MUL((lStraightLineRatio), BVDC_P_PEP_ITOFIX((id + 1) * (((BVDC_P_PEP_MAX_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)) + 1) / BVDC_P_PEP_LAB_GEN_SIZE)));
		}
	}
	else
	{
		for(id = 0; id < BVDC_P_PEP_LAB_GEN_SIZE; id++)
		{
			/* Fill in straight line when DC is off */
			pPep->lFixHist_out[id] = BVDC_P_PEP_ITOFIX((id+1)*(((BVDC_P_PEP_MAX_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)) + 1) / BVDC_P_PEP_LAB_GEN_SIZE));
		}
	}

	/* Expand LAB_GEN_SIZE-entry histogram to LAB_TABLE output table */
#if(1 == ((BVDC_P_LAB_TABLE_SIZE / BVDC_P_PEP_LAB_GEN_SIZE) - 1))
		lLoBitsMask  = 1; /* LAB = 128 */
		lHiBitsShift = 1;
#elif(3 == ((BVDC_P_LAB_TABLE_SIZE / BVDC_P_PEP_LAB_GEN_SIZE) - 1))
		lLoBitsMask  = 3; /* LAB = 256 */
		lHiBitsShift = 2;
#elif(7 == ((BVDC_P_LAB_TABLE_SIZE / BVDC_P_PEP_LAB_GEN_SIZE) - 1))
		lLoBitsMask  = 7; /* LAB = 512 - not in any devices today */
		lHiBitsShift = 3;
#elif(15 == ((BVDC_P_LAB_TABLE_SIZE / BVDC_P_PEP_LAB_GEN_SIZE) - 1))
		lLoBitsMask  = 15; /* LAB = 1024 */
		lHiBitsShift = 4;
#else
	{
		/* This is an error condition! */
		/* LAB should be 1024 or 256 or 128 entries */
		/* BVDC_P_PEP_LAB_GEN_SIZE should be 64 entries */
		pPep->bLabTableValid = false;
		BDBG_MSG(("LAB table size and LAB_GEN_SIZE size are not compatible"));
		BDBG_LEAVE(BVDC_P_Pep_DynamicContrastInterpolate_isr);
		return;
	}
#endif

#if PEP_DYN_CONTRAST_DBG
	if(bDbg)
	{
		BDBG_MSG(("LAB_TABLE_SIZE = %4d, LAB_GEN_SIZE = %4d", BVDC_P_LAB_TABLE_SIZE , BVDC_P_PEP_LAB_GEN_SIZE));
		BDBG_MSG(("LoBitsMask = %4d, HiBitsShift = %4d", lLoBitsMask, lHiBitsShift));
	}
#endif

	for(id = 0; id < BVDC_P_LAB_TABLE_SIZE; id++)
	{
		lLoBits = id & lLoBitsMask;
		lHiBits = id >> lHiBitsShift;
		lLoFrac = BVDC_P_PEP_ITOFIX(lLoBits) >> lHiBitsShift;
		lHiFrac = BVDC_P_PEP_ITOFIX((lLoBitsMask + 1) - lLoBits) >> lHiBitsShift;
		if(lHiBits == 0)
		{
			/* First few bins need special processing, low bin is 0 */
			pPep->lFixEstLuma[id] = BVDC_P_PEP_FIX_MUL(lHiFrac, 0) + BVDC_P_PEP_FIX_MUL(lLoFrac, pPep->lFixHist_out[0]);
		}
		else
		{
			/* Rest of bins require averaging between previous and next */
			pPep->lFixEstLuma[id] = BVDC_P_PEP_FIX_MUL(lHiFrac, pPep->lFixHist_out[lHiBits-1]) + BVDC_P_PEP_FIX_MUL(lLoFrac, pPep->lFixHist_out[lHiBits]);
		}

		/* Limit output to legal range */
		if(pPep->lFixEstLuma[id] > BVDC_P_PEP_ITOFIX((BVDC_P_PEP_MAX_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS))))
			pPep->lFixEstLuma[id] = BVDC_P_PEP_ITOFIX((BVDC_P_PEP_MAX_LUMA_VALUE >> (10 - BVDC_P_PEP_LUMA_BITS)));
		if(pPep->lFixEstLuma[id] < 0)
			pPep->lFixEstLuma[id] = 0;
	}

	/* Adjust color Saturation */
	/* Build LAB table */
	id = (BVDC_P_LAB_TABLE_SIZE - 1);
	do
	{
		*(pulLabTable + id) = BVDC_P_PEP_FIXTOI(pPep->lFixEstLuma[id]);
		/* Might need to add code to adjust psSaturation here...*/
		/* For settop box, this adjustment should not be needed */

#if PEP_DYN_CONTRAST_DBG
		sSat = 0;
		if(bDbg)
		{
			BDBG_MSG(("id=%d: LUMA=%d SAT=%d", id,
				BVDC_P_PEP_FIXTOI(pPep->lFixEstLuma[id]), sSat & 255));
		}
#endif
		id--;
	}while (id != 0);

#if PEP_DYN_CONTRAST_DBG
	if(bDbg)
	{
		BDBG_MSG(("PWM Shift   = %d", pPep->stCallbackData.ulShift));
	}
#endif

	/* Calculate backlight dimming */
	if(pCS->pfCallback)
	{
		if(iGain != pPep->lLastGain)
		{
			/* Update immediately if iGain has changed */
			pPep->ulAvgAPL = (ulInputAPL << BVDC_P_FILT_SHIFT);
		}
		else if(ulInputAPL > (int32_t)((pPep->ulAvgAPL >> BVDC_P_FILT_SHIFT) + 100))
		{
			/* If there is a large increase in APL then move average quickly */
			pPep->ulAvgAPL = ((pPep->ulAvgAPL * 50) + ((ulInputAPL << BVDC_P_FILT_SHIFT) * 50)) / 100;
		}
		else
		{
			/* In normal circumstances, adjust APL slowly */
			pPep->ulAvgAPL = ((pPep->ulAvgAPL * 90) + ((ulInputAPL << BVDC_P_FILT_SHIFT) * 10)) / 100;
		}

		/* Save the current iGain value */
		pPep->lLastGain = iGain;

		if((iGain == BVDC_P_PEP_FTOFIX(0)) || ((pPep->ulAvgAPL >> BVDC_P_FILT_SHIFT) > pCS->ulPwmMaxApl))
		{
			/* PWM is full on when DC is disabled, i.e. iGain == 0), or for APL>BVDC_P_PEP_PWM_MAX_APL */
			lPwmSetting = (1 << pPep->stCallbackData.ulShift);
		}
		else
		{
			/* lPwmSetting is minimum of 40% at APL=BVDC_P_PEP_PWM_MIN_APL, and maximum of 100% at APL=BVDC_P_PEP_PWM_MAX_APL */
			if((pPep->ulAvgAPL >> BVDC_P_FILT_SHIFT) >= pCS->ulPwmMinApl)
			{
				lPwmSetting = pCS->ulPwmMinPercent + ((((int32_t)(pPep->ulAvgAPL >> BVDC_P_FILT_SHIFT) - pCS->ulPwmMinApl) * (100 - pCS->ulPwmMinPercent)) / (pCS->ulPwmMaxApl - pCS->ulPwmMinApl));
			}
			else
			{
				lPwmSetting = pCS->ulPwmMinPercent;
			}

			/* Translate into fractional value */
			lPwmSetting = (lPwmSetting << pPep->stCallbackData.ulShift) / 100;
		}

#if PEP_DYN_CONTRAST_DBG
		if(bDbg)
		{
			BDBG_MSG(("PWM Setting    = %8d", (lPwmSetting * 100)>>pPep->stCallbackData.ulShift));
		}
#endif
		pPep->stCallbackData.iScalingFactor = lPwmSetting;

		/* Callback application with the above data */
		pCS->pfCallback(pCS->pvParm1, pCS->iParm2, (void*)&pPep->stCallbackData);
	}

#if PEP_DYN_CONTRAST_DBG
	BREG_Write32(pPep->hReg, BCHP_PEP_CMP_0_V0_SCRATCH_REGISTER, --ulDbgCnt);
	bDbg = false;
#endif

	BDBG_LEAVE(BVDC_P_Pep_DynamicContrastInterpolate_isr);
	return;
}


/***************************************************************************
Summary:
	This is the dynamic contrast stretch algorithm.

Description:
	This function calculates the LAB table from the histogram data using
	the dynamic contrast stretch algorithm.  It is also calculating the
	saturation to adjust the compositor color matrix accordingly.

Input:
	pCs              - pointer to the contrast stretch structure where
	                   all the user settings for this algorithm are stored.
	pPep             - pointer to the PEP context, where the histogram
	                   data and other information needed by the algorithm
	                   are stored.

Input & Output:
	psSaturation     - saturation value
	psBrightness     - brightness value
	psContrast       - contrast value

Output:
	pulLabTable      - the LAB table

Returns:
	None.

**************************************************************************/
void BVDC_P_Pep_DynamicContrast_isr
	( const BVDC_ContrastStretch  *pCS,
	  BVDC_P_PepContext           *pPep,
	  uint32_t                    *pulLabTable )
{
	BDBG_ENTER(BVDC_P_Pep_DynamicContrast_isr);

	BVDC_P_Pep_DynamicContrastInterpolate_isr(pCS, pPep, pulLabTable);

	BDBG_LEAVE(BVDC_P_Pep_DynamicContrast_isr);
}

#endif /* (BVDC_P_SUPPORT_PEP) */

/* End of File */
