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
#include "bstd.h"
#include "bmth.h"
#include "bkni.h"
#include "btmr.h"
#ifndef LEAP_BASED_CODE
#include "bads.h"
#include "bads_priv.h"
#include "bads_mth.h"
#include "bchp_ds_topm.h"
#include "bchp_ds_tops.h"
#include "bads_global_clk.h"
#include "bads_3x7x_priv.h"
const uint32_t SPUR_TBL_u32[SPUR_TBL_SIZE] =
{
	0, 54000000, 675000000,  99900000, 432000000,
	810000000, 864000000, 486000000, 594000000,
	648000000, 540000000
};
#else
#include "bads_api.h"
#endif
#include "bads_acquire.h"
#include "bads_def.h"


#if (BCHP_FAMILY == 3128) ||  (BCHP_FAMILY == 7584) /* 4 Demods or fewer */
#include "bchp_ds_topm.h"
#include "bchp_ds_tops.h"
#include "bwfe_global_clk.h"
#if (BCHP_CHIP == 3128) || (BCHP_CHIP == 3148) /* 8 Demods */
#include "bchp_ds_b_tops.h"
#include "bchp_ds_b_topm.h"
#include "bchp_tm.h"
#endif
#endif
#if (BCHP_FAMILY==3461) || (BCHP_FAMILY==3462)
#include "btnr_global_clk.h"
#endif
/*registers needed for the functions in this file*/
#include "bchp_ds.h"
#include "bads_priv.h"

#ifndef LEAP_BASED_CODE
BDBG_MODULE(bads_utils);
#endif

#include "bads_coef.h"
#include "bads_utils.h"
/***********************************************************************************************
* BADS_Range_Check()	 This routine checks that parameters from the PI/BBS are in range
************************************************************************************************/
BERR_Code BADS_P_Range_Check(BADS_3x7x_ChannelHandle hChn)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;

/*Check the acquire params and scam params*/
/*error code is checked at end of acquire params chack and scan params check*/
#if (ADS_INTERNAL_ERROR_CHECKING > 0)

	/************************************************************************************************
	 * Check the BADS_Acquire_Params structure
	 ************************************************************************************************/
	switch(hChn->pChnAcqParam->BADS_Acquire_Params.Acquire_After_Tune_te)
	{
		case BADS_Acquire_Params_eDisable:
		case BADS_Acquire_Params_eEnable:
		BDBG_MSG(("hChn->pChnAcqParam->BADS_Acquire_Params.Acquire_After_Tune = %d", hChn->pChnAcqParam->BADS_Acquire_Params.Acquire_After_Tune_te));
		break;
		default :
		BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.Acquire_After_Tune OUT OF RANGE"));
		BDBG_ERR(("Value Received is %d", hChn->pChnAcqParam->BADS_Acquire_Params.Acquire_After_Tune_te));
		RetCode_u32 = BERR_INVALID_PARAMETER;
	}

	switch(hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te)
	{
		case BADS_Acquire_Params_AcqType_eAuto:
		case BADS_Acquire_Params_AcqType_eFast:
		case BADS_Acquire_Params_AcqType_eSlow:
		case BADS_Acquire_Params_AcqType_eScan:
		BDBG_MSG(("hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te= %d", hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te));
		break;
		default :
		BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te OUT OF RANGE"));
		BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te));
		RetCode_u32 = BERR_INVALID_PARAMETER;
	}

	switch(hChn->pChnAcqParam->BADS_Acquire_Params.Auto_te)
	{
		case BADS_Acquire_Params_eDisable:
		case BADS_Acquire_Params_eEnable:
		BDBG_MSG(("hChn->pChnAcqParam->BADS_Acquire_Params.Auto_te = %d", hChn->pChnAcqParam->BADS_Acquire_Params.Auto_te));
		break;
		default :
		BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.Auto_te OUT OF RANGE"));
		BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Acquire_Params.Auto_te));
		RetCode_u32 = BERR_INVALID_PARAMETER;
	}

	switch(hChn->pChnAcqParam->BADS_Acquire_Params.IS_te)
	{
		case BADS_Acquire_Params_eDisable:
		case BADS_Acquire_Params_eEnable:
		BDBG_MSG(("hChn->pChnAcqParam->BADS_Acquire_Params.IS_te = %d", hChn->pChnAcqParam->BADS_Acquire_Params.IS_te));
		break;
		default :
		BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.IS_te OUT OF RANGE"));
		BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Acquire_Params.IS_te));
		RetCode_u32 = BERR_INVALID_PARAMETER;
	}

	/*check only right combinations are sent 64/256/1024 for AnnexB, 16/32/64/128/256 for AnnexA*/
	switch(hChn->pChnAcqParam->BADS_Acquire_Params.Annex_te)
	{
		case BADS_Acquire_Params_Annex_eAnnexA :
			switch(hChn->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te)
			{
				case BADS_Acquire_Params_BPS_eQam16	 :
				case BADS_Acquire_Params_BPS_eQam32	 :
				case BADS_Acquire_Params_BPS_eQam64	 :
				case BADS_Acquire_Params_BPS_eQam128	 :
				case BADS_Acquire_Params_BPS_eQam256	 :
                /*SW3128-257: Add Q1024A directed support but not scan support*/
				case	BADS_Acquire_Params_BPS_eQam1024 :
				break;
				default: BDBG_ERR(("Unsupported QAM and FEC combination"));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}
		break;
		case BADS_Acquire_Params_Annex_eAnnexB :
			switch(hChn->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te)
			{
				case BADS_Acquire_Params_BPS_eQam64	:
				case BADS_Acquire_Params_BPS_eQam256	:
				case BADS_Acquire_Params_BPS_eQam1024	:
				break;
				default: BDBG_ERR(("Unsupported QAM and FEC combination"));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}
			break;
		default :
		BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.Annex_te OUT OF RANGE"));
		BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Acquire_Params.Annex_te));
		RetCode_u32 = BERR_INVALID_PARAMETER;
	}

	/*Check the ranges for the hChn->pChnAcqParam->BADS_Acquire_Params structure */
	if (hChn->pChnAcqParam->BADS_Acquire_Params.Carrier_Range_u32 <= MAX_CARRIER_RANGE/256)
	{
		BDBG_MSG(("hChn->pChnAcqParam->BADS_Acquire_Params.Carrier_Range_u32 = %d", hChn->pChnAcqParam->BADS_Acquire_Params.Carrier_Range_u32*256));
	}
	else
	{
		BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.Carrier_Range_u32 OUT OF RANGE"));
		BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Acquire_Params.Carrier_Range_u32));
		RetCode_u32 = BERR_INVALID_PARAMETER;
	}

	/*Only check AnnexA_Sym_Rate if AnnexA is selected*/
	if (hChn->pChnAcqParam->BADS_Acquire_Params.Annex_te == BADS_Acquire_Params_Annex_eAnnexA)
	{
		if ((hChn->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32 >= MIN_BAUD_RATE) &&
				(hChn->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32 <= MAX_BAUD_RATE))
		{
			BDBG_MSG(("hChn->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32 = %d", hChn->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32));
		}
		else
		{
			BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32 OUT OF RANGE"));
			BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32));
			RetCode_u32 = BERR_INVALID_PARAMETER;
		}
	}
	/*check if any part of acquire params check was in error*/
	if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/

	/************************************************************************************************
	 * Check the BADS_Scan_Params structure
	 ************************************************************************************************/
	if ((hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te == BADS_Acquire_Params_AcqType_eAuto) || (hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te == BADS_Acquire_Params_AcqType_eScan))
	{
		switch(hChn->pChnAcqParam->BADS_Scan_Params.AI_te)
		{
			case BADS_Scan_Params_eDisable:
			case BADS_Scan_Params_eEnable:
			BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.AI_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.AI_te));
			break;
			default :
			BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.AI_te OUT OF RANGE"));
			BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.AI_te));
			RetCode_u32 = BERR_INVALID_PARAMETER;
		}

		switch(hChn->pChnAcqParam->BADS_Scan_Params.QM_te)
		{
			case BADS_Scan_Params_eDisable:
			case BADS_Scan_Params_eEnable:
			BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.QM_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.QM_te));
			break;
			default :
			BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.QM_te OUT OF RANGE"));
			BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.QM_te));
			RetCode_u32 = BERR_INVALID_PARAMETER;
		}

		switch(hChn->pChnAcqParam->BADS_Scan_Params.CO_te)
		{
			case BADS_Scan_Params_eDisable:
			case BADS_Scan_Params_eEnable:
			BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.CO_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.CO_te));
			break;
			default :
			BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.CO_te OUT OF RANGE"));
			BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.CO_te));
			RetCode_u32 = BERR_INVALID_PARAMETER;
		}

		switch(hChn->pChnAcqParam->BADS_Scan_Params.TO_te)
		{
			case BADS_Scan_Params_eDisable:
			case BADS_Scan_Params_eEnable:
			BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.TO_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.TO_te));
			break;
			default :
			BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.TO_te OUT OF RANGE"));
			BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.TO_te));
			RetCode_u32 = BERR_INVALID_PARAMETER;
		}

		/*Check only if QM is enabled*/
		if (hChn->pChnAcqParam->BADS_Scan_Params.QM_te == BADS_Scan_Params_eEnable)
		{
			switch(hChn->pChnAcqParam->BADS_Scan_Params.B1024_te)
			{
				case BADS_Scan_Params_eDisable:
				case BADS_Scan_Params_eEnable:
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.B1024_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.B1024_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.B1024_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.B1024_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.B256_te)
			{
				case BADS_Scan_Params_eDisable:
				case BADS_Scan_Params_eEnable:
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.B256_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.B256_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.B256_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.B256_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.B64_te)
			{
				case BADS_Scan_Params_eDisable:
				case BADS_Scan_Params_eEnable:
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.B64_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.B64_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.B64_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.B64_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.A1024_te)
			{
				case BADS_Scan_Params_eDisable:
				/*case BADS_Scan_Params_eEnable:*/  /*SW3128-257: Add Q1024A directed support but not scan support*/
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.A1024_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.A1024_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.A1024_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.A1024_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.A512_te)
			{
				case BADS_Scan_Params_eDisable:
				/*case BADS_Scan_Params_eEnable:*/  /*SW3128-257: Add Q1024A directed support but not scan support*/
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.A512_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.A512_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.A512_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.A512_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.A256_te)
			{
				case BADS_Scan_Params_eDisable:
				case BADS_Scan_Params_eEnable:
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.A256_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.A256_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.A256_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.A256_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.A128_te)
			{
				case BADS_Scan_Params_eDisable:
				case BADS_Scan_Params_eEnable:
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.A128_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.A128_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.A128_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.A128_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.A64_te)
			{
				case BADS_Scan_Params_eDisable:
				case BADS_Scan_Params_eEnable:
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.A64_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.A64_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.A64_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.A64_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.A32_te)
			{
				case BADS_Scan_Params_eDisable:
				case BADS_Scan_Params_eEnable:
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.A32_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.A32_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.A32_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.A32_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}

			switch(hChn->pChnAcqParam->BADS_Scan_Params.A16_te)
			{
				case BADS_Scan_Params_eDisable:
				case BADS_Scan_Params_eEnable:
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.A16_te = %d", hChn->pChnAcqParam->BADS_Scan_Params.A16_te));
				break;
				default :
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.A16_te OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.A16_te));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}
		}

		/*Check only if CO is enabled*/
		if (hChn->pChnAcqParam->BADS_Scan_Params.CO_te == BADS_Scan_Params_eEnable)
		{
			if (hChn->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32 <= MAX_CARRIER_SCAN/256)
			{
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32 = %d", hChn->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32*256));
			}
			else
			{
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32 OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}
		}

		/*Check only if TO is enabled*/
		if (hChn->pChnAcqParam->BADS_Scan_Params.TO_te == BADS_Scan_Params_eEnable)
		{
			if ((hChn->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32 <= MAX_BAUD_SCAN) &&
					(hChn->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32 >= MIN_BAUD_SCAN))
			{
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32 = %d", hChn->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32));
				BDBG_MSG(("hChn->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32 = %d", hChn->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32));
			}
			else
			{
				BDBG_ERR(("hChn->pChnAcqParam->BADS_Scan_Params.xxxxxx_Baud_Search_u32 OUT OF RANGE"));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32));
				BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32));
				RetCode_u32 = BERR_INVALID_PARAMETER;
			}
		}
	}
	/*check if any part of scan params check was in error*/
	if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/
#endif

	/*In addition to range checking, assign the local param structure since it is used in LUT's locally*/
	switch(hChn->pChnAcqParam->BADS_Acquire_Params.Annex_te)
	{
		case BADS_Acquire_Params_Annex_eAnnexA :
			hChn->pChnAcqParam->BADS_Local_Params.Annex_te = BADS_Local_Params_Annex_eAnnexA;
			BDBG_MSG(("FEC is AnnexA"));
			break;
		case BADS_Acquire_Params_Annex_eAnnexB :
			hChn->pChnAcqParam->BADS_Local_Params.Annex_te = BADS_Local_Params_Annex_eAnnexB;
			BDBG_MSG(("FEC is AnnexB"));
			break;
		default :
		BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.Annex_te OUT OF RANGE"));
		BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Acquire_Params.Annex_te));
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
		RetCode_u32 = BERR_INVALID_PARAMETER;
		if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/
#endif
	}

	/*In addition to range checking, assign the local param structure since it is used in LUT's locally*/
	switch(hChn->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te)
	{
		case BADS_Acquire_Params_BPS_eQam16	:
			hChn->pChnAcqParam->BADS_Local_Params.QAM_te = BADS_Local_Params_QAM_eQam16;
			BDBG_MSG(("Qam_Mode is Qam16"));
			break;
		case BADS_Acquire_Params_BPS_eQam32	:
			hChn->pChnAcqParam->BADS_Local_Params.QAM_te = BADS_Local_Params_QAM_eQam32;
			BDBG_MSG(("Qam_Mode is Qam32"));
			break;
		case BADS_Acquire_Params_BPS_eQam64	:
			hChn->pChnAcqParam->BADS_Local_Params.QAM_te = BADS_Local_Params_QAM_eQam64;
			BDBG_MSG(("Qam_Mode is Qam64"));
			break;
		case BADS_Acquire_Params_BPS_eQam128	:
			hChn->pChnAcqParam->BADS_Local_Params.QAM_te = BADS_Local_Params_QAM_eQam128;
			BDBG_MSG(("Qam_Mode is Qam128"));
			break;
		case BADS_Acquire_Params_BPS_eQam256	:
			hChn->pChnAcqParam->BADS_Local_Params.QAM_te = BADS_Local_Params_QAM_eQam256;
			BDBG_MSG(("Qam_Mode is Qam256"));
			break;
		case BADS_Acquire_Params_BPS_eQam512	:
			hChn->pChnAcqParam->BADS_Local_Params.QAM_te = BADS_Local_Params_QAM_eQam512;
			BDBG_MSG(("Qam_Mode is Qam512"));
			break;
		case BADS_Acquire_Params_BPS_eQam1024:
			hChn->pChnAcqParam->BADS_Local_Params.QAM_te = BADS_Local_Params_QAM_eQam1024;
			BDBG_MSG(("Qam_Mode is Qam1024"));
			break;
		default :
		BDBG_ERR(("hChn->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te OUT OF RANGE"));
		BDBG_ERR(("Value Received is %d",hChn->pChnAcqParam->BADS_Acquire_Params.Qam_Mode_te));
		RetCode_u32 = BERR_INVALID_PARAMETER;
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
		RetCode_u32 = BERR_INVALID_PARAMETER;
		if (RetCode_u32 != BERR_SUCCESS) goto something_bad_happened; /*goto bottom of function to return error code*/
#endif
	}

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}

/********************************************************************************************
 *BADS_P_Set_CFL_Frequency() determines the value to program in the front mixer
 *Calculate the Carrier Loop Frequency Control Word a 16 bit 2's complement number
 *The NCO value depends on the F_HS_u32 and CFL_Frequency
 *CFLFOS = 2^14*CFL_Frequency/F_HS_u32: F_HS_u32 is F_HS
 *F_HS_u32 CAN NOT be 0: CFL_Frequency_i32 CAN NOT be negative full scale, -POWER_2_31
 *CALCULATION MUST FIT WITHIN [-POWER_2_15 to POWER_2_15_M1]
 ********************************************************************************************/
BERR_Code BADS_P_Set_CFL_Frequency(BADS_3x7x_ChannelHandle hChn, uint32_t F_HS_u32, int32_t CFL_Frequency_i32)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	bool IsNegative_b = false;
	uint32_t ReadReg_u32, Result_Hi_u32, Result_Lo_u32;
	int32_t TempValue_i32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (F_HS_u32 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Set_CFL_Frequency()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*Due to compiler problem get -2^31 by loading -(2^-31 - 1) and subtracting 1*/
	TempValue_i32 = -POWER2_31_M1;
	TempValue_i32 -= 1;
#if (ADS_INTERNAL_ERROR_CHECKING > 1)
	/*detect negative full scale*/
	if (CFL_Frequency_i32 == TempValue_i32)
	{
		BDBG_ERR(("ERROR2 in BADS_P_Set_CFL_Frequency()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*Detect if CFL_Frequency is negative*/
	if (CFL_Frequency_i32 < 0)
	{
		IsNegative_b = true;
		ReadReg_u32 = (uint32_t)(-1*CFL_Frequency_i32);
	}
	else
	{
		ReadReg_u32 = (uint32_t)CFL_Frequency_i32;
	}

	/* 2^14*CFL_Frequency_u32/F_HS_u32 */
	BMTH_HILO_32TO64_Mul(POWER2_14, ReadReg_u32, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, F_HS_u32, &Result_Hi_u32, &Result_Lo_u32);

#if (ADS_INTERNAL_ERROR_CHECKING > 2)
	/*Check the functions output range*/
	/*Check that result will fit into a 16 bit 2's complement number*/
	if ((Result_Hi_u32 != 0) || (Result_Lo_u32 > POWER2_15) || ((IsNegative_b == false) && (Result_Lo_u32 == POWER2_15)))
	{
		BDBG_ERR(("ERROR3 in BADS_P_Set_CFL_Frequency()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*If result should be negative, take twos complement of output, Mask to 16 bits, program FCW*/
	Result_Lo_u32 = (IsNegative_b == true) ? (Twos_Complement32(Result_Lo_u32) & 0x0000FFFF) : (Result_Lo_u32 & 0x0000FFFF);
	BREG_WriteField(hChn->hRegister, DS_CFLFOS, CFLOFFSET, Result_Lo_u32);

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}

/****************************************************************************************************
 *BADS_P_Set_TL_Frequency() determines the value to program in the VID
 *Calculate the Timing Loop Frequency Control Word a 24 bit unsigned number that can be no larger than 0x80000000
 *TLFOS = (4*BaudRate/F_1S_u32)*2^23  or TLFOS = (2^25*BaudRate/F_1S_u32): F_1S_u32 is F_1S
 *F_1S_u32 CAN NOT be 0
 *CALCULATION MUST FIT WITHIN [0 to POWER_2_23]
 ****************************************************************************************************/
BERR_Code BADS_P_Set_TL_Frequency(BADS_3x7x_ChannelHandle hChn, uint32_t F_1S_u32, uint32_t Symbol_Rate_u32)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	uint32_t Result_Hi_u32, Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (F_1S_u32 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Set_TL_Frequency()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	BMTH_HILO_32TO64_Mul(POWER2_25, Symbol_Rate_u32, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, F_1S_u32, &Result_Hi_u32, &Result_Lo_u32);

#if (ADS_INTERNAL_ERROR_CHECKING > 2)
	/*Check the functions output range*/
	/*Check that result will fit into a 24 bit number whose max value is POWER2_23*/
	if ((Result_Hi_u32 != 0) || (Result_Lo_u32 > POWER2_23))
	{
		BDBG_ERR(("ERROR3 in BADS_P_Set_TL_Frequency()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*Program the Timing Loop Frequency Control Word*/
	BREG_Write32(hChn->hRegister, BCHP_DS_TLFOS, Result_Lo_u32);

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}

/****************************************************************************************
 *BADS_P_Get_CFL_Frequency() determines the Frequency in the front mixer
 *IF = F_HS_u32*CFLFOS/2^14: F_HS_u32 is F_HS
 *F_HS_u32 CAN NOT be 0
 *CALCULATION MUST FIT WITHIN [-POWER2_31 to POWER_2_31_M1]
 ****************************************************************************************/
BERR_Code BADS_P_Get_CFL_Frequency(BADS_3x7x_ChannelHandle hChn, uint32_t F_HS_u32, int32_t *CFL_Frequency_pi32)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	bool  RegIsNegative_b = false;
	uint32_t ReadReg_u32;
	uint32_t Result_Hi_u32, Result_Lo_u32;
	uint32_t TempValue_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (POWER2_14 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Get_CFL_Frequency()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*Read register*/
	ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_CFLFOS, CFLOFFSET);

	/*This is a 16 bit 2's complement register, if negative, sign extend and twos complement*/
	if ((ReadReg_u32 & 0x00008000) != 0)
	{
		RegIsNegative_b = true;
		ReadReg_u32 = Twos_Complement32((ReadReg_u32 | 0xFFFF8000));
	}

	BMTH_HILO_32TO64_Mul(F_HS_u32, ReadReg_u32, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, POWER2_14, &Result_Hi_u32, &Result_Lo_u32);

	/*Due to compiler problem get 2^31 by loading (2^-31 - 1) and adding 1*/
	TempValue_u32 = POWER2_31_M1;
	TempValue_u32 += 1;
#if (ADS_INTERNAL_ERROR_CHECKING > 2)
	/*Check the functions output range*/
	/*Check that result will fit into a 32 bit 2's complement number*/
	if ((Result_Hi_u32 != 0) || ((RegIsNegative_b == true) && (Result_Lo_u32 > TempValue_u32)) || ((RegIsNegative_b == false) && (Result_Lo_u32 >= TempValue_u32)))
	{
		BDBG_ERR(("ERROR3 in BADS_P_Get_CFL_Frequency()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*If result should be negative, take twos complement of output, set return value*/
	Result_Lo_u32 = (RegIsNegative_b == true) ? Twos_Complement32(Result_Lo_u32) : Result_Lo_u32;
	*CFL_Frequency_pi32 = (int32_t)Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}

/****************************************************************************************
 *BADS_P_Get_TL_Frequency() determines the symbol rate in the VID
 *BaudRate = TLFOS*F_1S_u32/2^25: F_1S_u32 = F_1S
 *F_1S_u32 CAN NOT be 0
 *CALCULATION WILL FIT WITHIN [0 to POWER_2_32_M1]
 ****************************************************************************************/
BERR_Code BADS_P_Get_TL_Frequency(BADS_3x7x_ChannelHandle hChn, uint32_t F_1S_u32, uint32_t *TL_Frequency_pu32)
{

	BERR_Code RetCode_u32 = BERR_SUCCESS;
	uint32_t ReadReg_u32;
	uint32_t Result_Hi_u32, Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (POWER2_25 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Get_TL_Frequency()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*Get the timing loop NCO value, it is a 24 bit value with a max value of POWER2_23*/
	ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_TLFOS, TLOFFSET);

	/*clamp to POWER2_23*/
	if (ReadReg_u32 > 0x00800000)
	{
		ReadReg_u32 = 0x00800000;
		BDBG_ERR(("ERROR2 in BADS_P_Get_TL_Frequency()"));
	}

	BMTH_HILO_32TO64_Mul(ReadReg_u32, F_1S_u32, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, POWER2_25, &Result_Hi_u32, &Result_Lo_u32);

	/*set return value*/
	*TL_Frequency_pu32 = Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}

/***************************************************************************************
 *BADS_P_Get_CFL_Error() determines the Front Loop Frequency Error
 *IF = F_HS_u32*CFLI/2^34
 *POWER2_31 CAN NOT be 0
 *CALCULATION WILL FIT WITHIN [-POWER_2_31 to POWER_2_31_M1]
 ***************************************************************************************/
BERR_Code BADS_P_Get_CFL_Error(BADS_3x7x_ChannelHandle hChn, uint32_t F_HS_u32, int32_t *CFL_Error_pi32)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	bool  RegIsNegative_b = false;
	uint32_t ReadReg_u32;
	uint32_t Result_Hi_u32, Result_Lo_u32;
	uint32_t TempValue_u32;

	/*Due to compiler problem get 2^31 by loading (2^-31 - 1) and adding 1*/
	TempValue_u32 = POWER2_31_M1;
	TempValue_u32 += 1;
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (TempValue_u32 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Get_CFL_Error()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_CFLI, CFLVAL);

	/*This is a 32 bit 2's complement register, if negative, clamp negative full scale, take twos complement*/
	if ((ReadReg_u32 & 0x80000000) != 0)
	{
		RegIsNegative_b = true;
		ReadReg_u32 = (ReadReg_u32 == 0x80000000) ? Twos_Complement32(0x80000001) : Twos_Complement32(ReadReg_u32);
	}

	BMTH_HILO_32TO64_Mul(ReadReg_u32, F_HS_u32/8, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, TempValue_u32, &Result_Hi_u32, &Result_Lo_u32);

	/*If result should be negative, take twos complement of output*/
	Result_Lo_u32 = (RegIsNegative_b == true) ? Twos_Complement32(Result_Lo_u32) : Result_Lo_u32;

	*CFL_Error_pi32 = (int32_t)Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}

/****************************************************************************************
 *BADS_P_Get_VID_Error() determines the VID Timing Error
 *For QAM (with loop filter output gain set to 1.0) the offset in Hertz is:
 *       (1/8)*(TLI/2^31)*F1s in Hertz
 *For QAM (with loop filter output gain set to 2^-6) the offset in Hertz is:
 *       (1/512)*(TLI/2^31)*F1s in Hertz
 *POWER2_31 CAN NOT be 0
 *CALCULATION WILL FIT WITHIN [-POWER_2_31 to POWER_2_31_M1]
 ****************************************************************************************/
BERR_Code BADS_P_Get_VID_Error(BADS_3x7x_ChannelHandle hChn, uint32_t F_1S_u32, int32_t *VID_Error_pi32)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	bool  RegIsNegative_b = false;
	uint32_t ReadReg_u32, TL_Gain_u32;
	uint32_t Result_Hi_u32, Result_Lo_u32;
	uint32_t TempValue_u32;

	/*Due to compiler problem get 2^31 by loading (2^-31 - 1) and adding 1*/
	TempValue_u32 = POWER2_31_M1;
	TempValue_u32 += 1;
#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (TempValue_u32 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Get_VID_Error()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	TL_Gain_u32 = BREG_ReadField(hChn->hRegister, DS_TLC, TLLFOUTGAIN);
	ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_TLI, TLVAL);

	/*This is a 32 bit 2's complement register, if negative, clamp and twos complement*/
	if ((ReadReg_u32 & 0x80000000) != 0)
	{
		RegIsNegative_b = true;
		ReadReg_u32 = (ReadReg_u32 == 0x80000000) ? Twos_Complement32(0x80000001) : Twos_Complement32(ReadReg_u32);
	}

	/*loop filter output gain set to 1 or 2^-6*/
	F_1S_u32 = (TL_Gain_u32 == 0) ? F_1S_u32/8 : F_1S_u32/512;

	BMTH_HILO_32TO64_Mul(ReadReg_u32, F_1S_u32, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, TempValue_u32, &Result_Hi_u32, &Result_Lo_u32);

	/*If result should be negative, take twos complement of output*/
	Result_Lo_u32 = (RegIsNegative_b == true) ? Twos_Complement32(Result_Lo_u32) : Result_Lo_u32;

	*VID_Error_pi32 = (int32_t)Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}

/****************************************************************************************
 *BADS_P_Get_CPL_Error() determines the Phase Loop Frequency Error
 *phase error is Symbol_Rate_u32*CPLI/2^30
 *POWER2_30 CAN NOT be 0
 *CALCULATION WILL FIT WITHIN [-POWER_2_31 to POWER_2_31_M1]
 ****************************************************************************************/
BERR_Code BADS_P_Get_CPL_Error(BADS_3x7x_ChannelHandle hChn, uint32_t Symbol_Rate_u32, int32_t *CPL_Error_pi32)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	bool  RegIsNegative_b = false;
	uint32_t ReadReg_u32;
	uint32_t Result_Hi_u32, Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (POWER2_30 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Get_CPL_Error()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	ReadReg_u32 = BREG_ReadField(hChn->hRegister, DS_EQ_CPLI, CPLI);

	/*This is a 30 bit 2's complement register, if negative, sign extend and twos complement*/
	if ((ReadReg_u32 & 0x20000000) != 0)
	{
		RegIsNegative_b = true;
		ReadReg_u32 = Twos_Complement32((ReadReg_u32 | 0xE0000000));
	}

	BMTH_HILO_32TO64_Mul(ReadReg_u32, Symbol_Rate_u32, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, POWER2_30, &Result_Hi_u32, &Result_Lo_u32);

	/*If result should be negative, take twos complement of output*/
	Result_Lo_u32 = (RegIsNegative_b == true) ? Twos_Complement32(Result_Lo_u32) : Result_Lo_u32;

	*CPL_Error_pi32 = (int32_t)Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

	return RetCode_u32;
}


/****************************************************************************************
 *BADS_P_Get_TimingScan_Advanced_FFT() Perform the FFT timing scan search returning the baud rate
 *The function will return a 0 if the peak is too small or outside of a window defined by
 *hChn->pChnAcqParam->BADS_Internal_Params.Timing_Scan_Threshold_u32,
 *Upper window bound and Lower window bound
 *POWER2_11 CAN NOT be 0
 *CALCULATION WILL FIT WITHIN [0 to POWER_2_32_M1]
 ****************************************************************************************/
BERR_Code BADS_P_Get_TimingScan_Advanced_FFT(BADS_3x7x_ChannelHandle hChn, uint32_t Upper_Baud_Search_u32, bool ReturnBin_b, uint32_t *TimingScanResult_pu32)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	uint8_t	  NumFFTs_u8;
	uint16_t  Bin_u16;
	int16_t   Bin_Check1_i16 = 0, Bin_Check2_i16 = 0, Bin_Check3_i16 = 0; /*initialization is for compiler warning*/
	int16_t   DiffA_i16, DiffB_i16, DiffC_i16;
	uint32_t  Peak_u32;
	uint32_t  Result_Hi_u32, Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (POWER2_11 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Get_TimingScan_Advanced_FFT()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

#if (ADS_INTERNAL_ERROR_CHECKING > 1)
	/*detect out of range NUM_TIMING_FFTS*/
	if ((NUM_TIMING_FFTS < 1) || (NUM_TIMING_FFTS > 3))
	{
		BDBG_ERR(("ERROR2 in BADS_P_Get_TimingScan_Advanced_FFT()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*NUM_TIMING_FFTS must be 1, 2, or 3 which is how many FFTs to run*/
	/*If NUM_TIMING_FFTS = 1 then the first FFT peak value is used*/
	/*If NUM_TIMING_FFTS = 2 then the two FFT peak values must be within +-1 bin to be used*/
	/*If NUM_TIMING_FFTS = 3 then the two of three FFT peak values must be within +-1 bin to be used*/
	/*When loop will ends with a successful find the NUM_TIMING_FFTS = NUM_TIMING_FFTS + 1, otherwise NUM_TIMING_FFTS = NUM_TIMING_FFTS*/
	for (NumFFTs_u8=0;NumFFTs_u8<NUM_TIMING_FFTS;NumFFTs_u8++)
	{
		/*Registers for Fast Timing Mode*/
 #if (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_6) /*Register is now 13 bits in core 9.6 for window, not 12 like 9.5*/
		BREG_Write32(hChn->hRegister, BCHP_DS_FFT_PDETW, 0x012186F4);   /*Upper window bound at 2316, Lower window bound at 1780 */
 #else
		BREG_Write32(hChn->hRegister, BCHP_DS_FFT_PDETW, 0x0090C6F4);   /*Upper window bound at 2316, Lower window bound at 1780 */
 #endif
		BREG_Write32(hChn->hRegister, BCHP_DS_FFT_CTL, 0x00080C42);     /*default + Nyquist Prefilter Data: Stop: Channel Scan (CHSCN) Mode */

/**************************************************************************************************************************************
 *Since the FFT interrupt is broke, we will use the sample clock counter which is counting at 30 MHz
 *EqFFT counts for a 4K FFT size if [4096/(2*Fb) + 4096*2*13/270000000]*30000000 or FFTcounts = [2^11/Fb + 2^13*13/(2^7*3^3*5^7)]*(2^7*3*5^7)
 *Or FFTcounts = (2^18*3*5^7)/Fb + 2^13*13/3^2)  or FFTcounts = 2^14*3*5^7/(Fb/2^4) + 2^13*13/3^2) = 3840000000/(Fb/16) + 106496/9
 *Leaving a 10% extra count time gives FFTtime = 4224000000/(Fb/16) + 13017
  *Use result Result_Hi_u32 and Result_Lo_u32
 *************************************************************************************************************************************/
 #if (BCHP_FAMILY==7584)
	Result_Hi_u32 = 4224000000UL;
	Result_Lo_u32 = Upper_Baud_Search_u32/16;
	Result_Lo_u32 = Result_Hi_u32/Result_Lo_u32 + 13017;
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, START, 0);        /*stop*/
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQCLR2, BCHP_DS_IRQCLR2_FSCNT1_ICLR_MASK);
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMCLR2, BCHP_DS_IRQCLR2_FSCNT1_ICLR_MASK);
	BKNI_ResetEvent(hChn->hFftEvent);
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, START, 1);        /*start FFT*/
	BREG_Write32(hChn->hRegister, BCHP_DS_FSCNT1, Result_Lo_u32);  /*Start FS COunter*/
	/*Interrupt happens*/
	RetCode_u32 = BKNI_WaitForEvent(hChn->hFftEvent, TIMING_SCAN_FFT_TIMEOUT); /*set interrupt timeout*/
	if (RetCode_u32  != BERR_SUCCESS)
		{
		    BDBG_ERR(("CHSCN TIMEOUT EVENT or OS ERR waiting in BADS_P_Get_TimingScan_Advanced_FFT() 0x%08X", RetCode_u32));
			RetCode_u32 = BERR_TIMEOUT;
			goto something_bad_happened; /*goto bottom of function to leave early with error*/
		}
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMSET2,  BCHP_DS_IRQCLR2_FSCNT1_ICLR_MASK);
#else
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMCLR2, BCHP_DS_IRQMCLR2_FFT_IMCLR_MASK);
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMCLR2, BCHP_DS_IRQMCLR2_FFT_IMCLR_MASK);

	BKNI_ResetEvent(hChn->hFftEvent);
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, START, 1);        /*start*/
	RetCode_u32 = BKNI_WaitForEvent(hChn->hFftEvent, TIMING_SCAN_FFT_TIMEOUT); /*set interrupt timeout*/
 #if (ADS_INTERNAL_ERROR_CHECKING > 0)
		if (RetCode_u32  != BERR_SUCCESS)
		{
		    BDBG_ERR(("CHSCN TIMEOUT EVENT or OS ERR waiting in BADS_P_Get_TimingScan_Advanced_FFT() 0x%08X", RetCode_u32));
			RetCode_u32 = BERR_TIMEOUT;
			goto something_bad_happened; /*goto bottom of function to leave early with error*/
		}
 #endif
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMSET2,  BCHP_DS_IRQMCLR2_FFT_IMCLR_MASK);
#endif

		/*Get the FFT results*/
		Peak_u32 = BREG_ReadField(hChn->hRegister, DS_FFT_VAL, PEAK);
		Bin_u16 =  BREG_ReadField(hChn->hRegister, DS_FFT_BIN, INDEX);

		/*delay line to compare successive FFT_Bins with initialization to +-8192*/
		/*this disallows comparison between attempts 1:2, 1:3, and 2:3 when NumFFTs = 0, and 1:3 and 2:3 when NumFFTs = 1*/
		Bin_Check3_i16 = (NumFFTs_u8 == 0) ? -8192 : Bin_Check2_i16;
		Bin_Check2_i16 = (NumFFTs_u8 == 0) ?  8192 : Bin_Check1_i16;
		Bin_Check1_i16 = (int16_t)((Bin_u16 > 2048) ? (4096 - Bin_u16) : Bin_u16);

		/*Get absolute value of differences between attempts 1:2, 1:3 and 2:3*/
		DiffA_i16 = (Bin_Check1_i16 < Bin_Check2_i16) ? Bin_Check2_i16 - Bin_Check1_i16 : Bin_Check1_i16 - Bin_Check2_i16;
		DiffB_i16 = (Bin_Check1_i16 < Bin_Check3_i16) ? Bin_Check3_i16 - Bin_Check1_i16 : Bin_Check1_i16 - Bin_Check3_i16;
		DiffC_i16 = (Bin_Check2_i16 < Bin_Check3_i16) ? Bin_Check3_i16 - Bin_Check2_i16 : Bin_Check2_i16 - Bin_Check3_i16;

		/*Check if the absolute value of the bin differences are within 1 or if the NUM_TIMING_FFTS = 1 to leave loop successfully*/
		/*NumFFTs_u8 will become NUM_TIMING_FFTS+1*/
		if ((DiffA_i16 <=1) || (DiffB_i16 <=1) || (DiffC_i16 <=1) || (NUM_TIMING_FFTS == 1))
		{
			NumFFTs_u8 = NUM_TIMING_FFTS;
		}
	}

	/*disable fast timing*/
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, START, 0);        /*stop*/
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, MODE, 0);         /*FFT function is off*/

	/*Calculate the baud rate found*/
	BMTH_HILO_32TO64_Mul(Upper_Baud_Search_u32, (uint32_t)Bin_Check1_i16, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, POWER2_11, &Result_Hi_u32, &Result_Lo_u32);

	/*Disqualify bin if threshold not met or if there are not 2 bins within +-1 for NUM_TIMING_FFTS = 2 or 3*/
	if ((Peak_u32 < hChn->pChnAcqParam->BADS_Internal_Params.Timing_Scan_Threshold_u32) || (NumFFTs_u8 != (NUM_TIMING_FFTS+1)))
	{
		Result_Lo_u32 = 0;
	}

	/*set output value*/
	*TimingScanResult_pu32 = (ReturnBin_b == true) ? (uint32_t)Bin_u16 : Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

  return RetCode_u32;

}

/****************************************************************************************
 *BADS_P_Get_CarrierScan_Advanced_FFT() Perform the FFT carrier scan search returning the carrier offset
 *The function will return a 0 if the peak is too small or outside of a window defined by
 *hChn->pChnAcqParam->BADS_Internal_Params.BBS_Carrier_Scan_Threshold
 *The timing loop must be locked for this function to return good results
 *POWER2_14 CAN NOT be 0
 *CALCULATION WILL FIT WITHIN [-POWER_2_31 to POWER_2_31_M1]
 ****************************************************************************************/
BERR_Code BADS_P_Get_CarrierScan_Advanced_FFT(BADS_3x7x_ChannelHandle hChn, uint32_t Symbol_Rate, int32_t *CarrierScanResult_pi32)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;
	uint8_t	  NumFFTs_u8;
	uint16_t  Bin_u16;
	int16_t   Bin_Check1_i16 = 0, Bin_Check2_i16 = 0, Bin_Check3_i16 = 0; /*initialization is for compiler warning*/
	int16_t   DiffA_i16, DiffB_i16, DiffC_i16;
	uint32_t  Peak_u32;
	int32_t   Carrier_Offset_i32;
	uint32_t  Result_Hi_u32, Result_Lo_u32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
	/*Check the functions input variables*/
	/*Check that divisor will not be 0*/
	if (POWER2_14 == 0)
	{
		BDBG_ERR(("ERROR1 in BADS_P_Get_CarrierScan_Advanced_FFT()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

#if (ADS_INTERNAL_ERROR_CHECKING > 1)
	/*detect out of range NUM_CARRIER_FFTS*/
	if ((NUM_CARRIER_FFTS < 1) || (NUM_CARRIER_FFTS > 3))
	{
		BDBG_ERR(("ERROR2 in BADS_P_Get_CarrierScan_Advanced_FFT()"));
		RetCode_u32 = BERR_INVALID_PARAMETER;
		goto something_bad_happened; /*goto bottom of function to leave early with error*/
	}
#endif

	/*NUM_CARRIER_FFTS must be 1, 2, or 3 which is how many FFTs to run*/
	/*If NUM_CARRIER_FFTS = 1 then the first FFT peak value is used*/
	/*If NUM_CARRIER_FFTS = 2 then the two FFT peak values must be within +-1 bin to be used*/
	/*If NUM_CARRIER_FFTS = 3 then the two of three FFT peak values must be within +-1 bin to be used*/
	/*When loop will ends with a successful find the NUM_CARRIER_FFTS = NUM_CARRIER_FFTS + 1, otherwise NUM_CARRIER_FFTS = NUM_CARRIER_FFTS*/
	/*For this routine if the offset is not found, the last value is used*/
	for (NumFFTs_u8=0;NumFFTs_u8<NUM_CARRIER_FFTS;NumFFTs_u8++)
	{
		/*Registers for Fast Carrier Mode (timing must be locked for this to work)*/
 #if (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_6) /*Register is now 13 bits in core 9.6 for window, not 12 like 9.5*/
		BREG_Write32(hChn->hRegister, BCHP_DS_FFT_PDETW, 0x01FFE000);   /*Upper window bound at 4095, Lower window bound at 0 CHANGED FROM 1 12/3/12 CAB*/
 #else
		BREG_Write32(hChn->hRegister, BCHP_DS_FFT_PDETW, 0x00FFF000);   /*Upper window bound at 4095, Lower window bound at 0 CHANGED FROM 1 12/3/12 CAB*/
 #endif
		BREG_Write32(hChn->hRegister, BCHP_DS_FFT_CTL, 0x00080C11);     /*default + POSTEQ Data: Stop: Fast Carrier Acquisition (FCA) Mode */

/**************************************************************************************************************************************
 *Since the FFT interrupt is broke, we will use the sample clock counter which is counting at 30 MHz
 *EqFFT counts for a 4K FFT size if [4096/Fb + 4096*2*13/270000000]*30000000 or FFTcounts = [2^12/Fb + 2^13*13/(2^7*3^3*5^7)]*(2^7*3*5^7)
 *Or FFTcounts = (2^19*3*5^7)/Fb + 2^13*13/3^2)  or FFTcounts = 2^14*3*5^7/(Fb/2^5) + 2^13*13/3^2) = 3840000000/(Fb/32) + 106496/9
 *Leaving a 10% extra count time gives FFTtime = 4224000000/(Fb/32) + 13017
  *Use result Result_Hi_u32 and Result_Lo_u32
 *************************************************************************************************************************************/
 #if (BCHP_FAMILY==7584)
	Result_Hi_u32 = 4224000000UL;
	Result_Lo_u32 = Symbol_Rate/32;
	Result_Lo_u32 = Result_Hi_u32/Result_Lo_u32 + 13017;
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, START, 0);        /*stop*/
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQCLR2, BCHP_DS_IRQCLR2_FSCNT1_ICLR_MASK);
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMCLR2, BCHP_DS_IRQCLR2_FSCNT1_ICLR_MASK);
	BKNI_ResetEvent(hChn->hFftEvent);
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, START, 1);        /*start FFT*/
	BREG_Write32(hChn->hRegister, BCHP_DS_FSCNT1, Result_Lo_u32);  /*Start FS Counter*/

	/*Interrupt happens*/
	RetCode_u32 = BKNI_WaitForEvent(hChn->hFftEvent, CARRIER_SCAN_FFT_TIMEOUT); /*set interrupt timeout*/
	if (RetCode_u32  != BERR_SUCCESS)
		{
		    BDBG_ERR(("FCA TIMEOUT EVENT or OS ERR waiting in BADS_P_Get_CarrierScan_Advanced_FFT() 0x%08X", RetCode_u32));
			RetCode_u32 = BERR_TIMEOUT;
			goto something_bad_happened; /*goto bottom of function to leave early with error*/
		}
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMSET2,  BCHP_DS_IRQCLR2_FSCNT1_ICLR_MASK);
	BREG_Write32(hChn->hRegister, BCHP_DS_FSCNT1, 0);
#else
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMCLR2, BCHP_DS_IRQMCLR2_FFT_IMCLR_MASK);
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMCLR2, BCHP_DS_IRQMCLR2_FFT_IMCLR_MASK);

	BKNI_ResetEvent(hChn->hFftEvent);
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, START, 1);        /*start*/
	RetCode_u32 = BKNI_WaitForEvent(hChn->hFftEvent, CARRIER_SCAN_FFT_TIMEOUT); /*set interrupt timeout*/
 #if (ADS_INTERNAL_ERROR_CHECKING > 0)
		if (RetCode_u32  != BERR_SUCCESS)
		{
		    BDBG_ERR(("FCA TIMEOUT EVENT or OS ERR waiting in BADS_P_Get_CarrierScan_Advanced_FFT() 0x%08X", RetCode_u32));
			RetCode_u32 = BERR_TIMEOUT;
			goto something_bad_happened; /*goto bottom of function to leave early with error*/
		}
 #endif
	BREG_Write32(hChn->hRegister, BCHP_DS_IRQMSET2,  BCHP_DS_IRQMCLR2_FFT_IMCLR_MASK);
#endif




		/*Get the FFT results*/
		Peak_u32 = BREG_ReadField(hChn->hRegister, DS_FFT_VAL, PEAK);
		Bin_u16 =  BREG_ReadField(hChn->hRegister, DS_FFT_BIN, INDEX);

		/*delay line to compare successive FFT_Bins with initialization to +-8192*/
		/*this disallows comparison between attempts 1:2, 1:3, and 2:3 when NumFFTs = 0, and 1:3 and 2:3 when NumFFTs = 1*/
		Bin_Check3_i16 = (NumFFTs_u8 == 0) ? -8192 : Bin_Check2_i16;
		Bin_Check2_i16 = (NumFFTs_u8 == 0) ?  8192 : Bin_Check1_i16;
		Bin_Check1_i16 = (int16_t)((Bin_u16 > 2048) ? (4096 - Bin_u16) : Bin_u16);

		/*Get absolute value of differences between attempts 1:2, 1:3 and 2:3*/
		DiffA_i16 = (Bin_Check1_i16 < Bin_Check2_i16) ? Bin_Check2_i16 - Bin_Check1_i16 : Bin_Check1_i16 - Bin_Check2_i16;
		DiffB_i16 = (Bin_Check1_i16 < Bin_Check3_i16) ? Bin_Check3_i16 - Bin_Check1_i16 : Bin_Check1_i16 - Bin_Check3_i16;
		DiffC_i16 = (Bin_Check2_i16 < Bin_Check3_i16) ? Bin_Check3_i16 - Bin_Check2_i16 : Bin_Check2_i16 - Bin_Check3_i16;

		/*Check if the absolute value of the bin differences are within 1 or if the NUM_CARRIER_FFTS = 1 to leave loop successfully*/
		/*NumFFTs_u8 will become NUM_CARRIER_FFTS+1*/
		if ((DiffA_i16 <=1) || (DiffB_i16 <=1) || (DiffC_i16 <=1) || (NUM_TIMING_FFTS == 1))
		{
			NumFFTs_u8 = NUM_CARRIER_FFTS;
		}
	}

	/*disable fast carrier*/
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, START, 0);        /*stop*/
	BREG_WriteField(hChn->hRegister, DS_FFT_CTL, MODE, 0);         /*FFT function is off*/

	/*Calculate the carrier offset*/
	BMTH_HILO_32TO64_Mul(Symbol_Rate, (uint32_t)Bin_Check1_i16, &Result_Hi_u32, &Result_Lo_u32);
	BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, POWER2_14, &Result_Hi_u32, &Result_Lo_u32);

	/*if bin > 2047 the result is negative*/
	Carrier_Offset_i32 = (int32_t)Result_Lo_u32;
	Carrier_Offset_i32 = (Bin_u16 > 2048) ? -1*Carrier_Offset_i32 : Carrier_Offset_i32;

#if 0 /*Disable carrier scan disqualification*/
	/*Disqualify bin if threshold not met or if there are not 2 bins within +-1 for NUM_TIMING_FFTS = 2 or 3*/
	if ((Peak_u32 < hChn->pChnAcqParam->BADS_Internal_Params.Carrier_Scan_Threshold_u32) || (NumFFTs_u8 != (NUM_TIMING_FFTS+1)))
	{
		/*Due to compiler problem get -2^31 by loading -(2^-31 - 1) and subtracting 1*/
		Carrier_Offset_i32 = -POWER2_31_M1;
		Carrier_Offset_i32 -= Carrier_Offset_i32;
	}
#endif

	*CarrierScanResult_pi32 = Carrier_Offset_i32;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

  return RetCode_u32;
}

/********************************************************************************************
*Function to program the FEC
********************************************************************************************/
void BADS_P_ProgramFEC(BADS_3x7x_ChannelHandle hChn)
{
	if (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA)
	{
		BREG_Write32(hChn->hRegister, BCHP_DS_FECU, AnnexA_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECU);
		BREG_Write32(hChn->hRegister, BCHP_DS_FECM, AnnexA_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECM);
		BREG_Write32(hChn->hRegister, BCHP_DS_FECL, AnnexA_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECL);
		BREG_Write32(hChn->hRegister, BCHP_DS_FECOUT_NCON, AnnexA_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECOUT_NCON);
		BREG_Write32(hChn->hRegister, BCHP_DS_FECOUT_NCODL, AnnexA_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECOUT_NCODL);
		BREG_Write32(hChn->hRegister, BCHP_DS_BND, 0x08000000);
		BREG_Write32(hChn->hRegister, BCHP_DS_BND_THR, 0x00072B02);
	}
	else
	{
		BREG_Write32(hChn->hRegister, BCHP_DS_FECU, AnnexB_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECU);
		BREG_Write32(hChn->hRegister, BCHP_DS_FECM, AnnexB_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECM);
		BREG_Write32(hChn->hRegister, BCHP_DS_FECL, AnnexB_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECL);
		BREG_Write32(hChn->hRegister, BCHP_DS_FECOUT_NCON, AnnexB_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECOUT_NCON);
		BREG_Write32(hChn->hRegister, BCHP_DS_FECOUT_NCODL, AnnexB_FEC_TBL_struct[hChn->pChnAcqParam->BADS_Local_Params.QAM_te].DS_FECOUT_NCODL);
		BREG_Write32(hChn->hRegister, BCHP_DS_BND, 0x04000000);
		BREG_Write32(hChn->hRegister, BCHP_DS_BND_THR, 0x00000000);
	}
}

/********************************************************************************************
*BADS_P_AcquisitionPercentageTest
*This is a test routine to see the number of succesfull acquiaitions by acquiring,
*checking the lock status then telling the AP we are unlocked, Auto Acquire must be enabled
********************************************************************************************/
void BADS_P_AcquisitionPercentageTest(BADS_3x7x_ChannelHandle hChn)
{
	uint32_t ReadReg_u32;
	uint16_t UpperSpare_u16, LowerSpare_u16;

	/*Use upper 2 bytes for how many acquisitions to run, lower 2 bytes for number of successes*/
	ReadReg_u32 = BREG_Read32(hChn->hRegister, BCHP_DS_SPARE);
	UpperSpare_u16 = (ReadReg_u32>>16) & 0x0000FFFF;
	LowerSpare_u16 = ReadReg_u32 & 0x0000FFFF;

	if	(UpperSpare_u16 != 0)
	{
		/*Enable test: Auto Acquire needs to be enabled for this test*/
		hChn->pChnAcqParam->BADS_Local_Params.TestLockFlag_te = BADS_Local_Params_eEnable;
		if (hChn->pChnLockStatus->FLK_te == BADS_3x7x_ChnLockStatus_eLock)
		{
			LowerSpare_u16++;
		}
		UpperSpare_u16--;
	}
	else
	{
		/*Disable test*/
		hChn->pChnAcqParam->BADS_Local_Params.TestLockFlag_te = BADS_Local_Params_eDisable;
	}

	/*Write back to chip*/
	ReadReg_u32 = ((UpperSpare_u16<<16) & 0xFFFF0000) | (LowerSpare_u16 & 0x0000FFFF);
	BREG_Write32(hChn->hRegister, BCHP_DS_SPARE, ReadReg_u32);
}

/**********************************************************************************************************************
*Set_BADS_P_Set_CWC_Auto, this places the CWC cancellers according to the frequencies in the SPUR_TBL_u32[] in global_clk.h
*CALCULATION MUST FIT WITHIN [-POWER_2_23 to POWER_2_23_M1]
**********************************************************************************************************************/
BERR_Code BADS_P_Set_CWC_Auto(BADS_3x7x_ChannelHandle hChn, uint32_t Symbol_Rate_u32, int32_t CWC_Offset_Freq_i32, uint8_t *CWC_LengthResult_pu8)
{
	BERR_Code RetCode_u32 = BERR_SUCCESS;

	uint8_t spur_cnt_u8, CWC_Length_u8;
	int32_t Fin_Hz_i32[4];
	int32_t upper_spur_i32, lower_spur_i32, Wrap_i32;
	int32_t TempValue_i32;
	uint32_t Symbol_Rate_Div2_u32;
#if (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_6)
	uint8_t Index_u8;
	bool IsNegative_b = false;
	uint32_t Result_Hi_u32, Result_Lo_u32;
#endif

	/*Initial values*/
	CWC_Length_u8 = 0;
	Fin_Hz_i32[0] = 0;
	Fin_Hz_i32[1] = 0;
	Fin_Hz_i32[2] = 0;
	Fin_Hz_i32[3] = 0;
	Wrap_i32 = 0;
	Symbol_Rate_Div2_u32 = Symbol_Rate_u32/2;

	/*Due to compiler problem get -2^31 by loading -(2^-31 - 1) and subtracting 1*/
	TempValue_i32 = -POWER2_31_M1;
	TempValue_i32 -= 1;

#if (ADS_INTERNAL_ERROR_CHECKING > 1)
		/*Check function input range for overflow for CWC_Offset_Freq_i32 + (int32_t)(Symbol_Rate_u32/2) and CWC_Offset_Freq_i32 - (int32_t)(Symbol_Rate_u32/2)*/
		/*The divide by 2 clips the range for the (int32_t)Symbol_Rate_u32 while casting to an integer to [0, 2^31-1]*/
		if ((CWC_Offset_Freq_i32 > POWER2_31_M1 - (int32_t)Symbol_Rate_Div2_u32) || (CWC_Offset_Freq_i32 < (TempValue_i32  + (int32_t)Symbol_Rate_Div2_u32)))
		{
			BDBG_ERR(("ERROR: CWC INPUT OVERFLOW IN BADS_P_Set_CWC_Auto()"));
			RetCode_u32 = BERR_INVALID_PARAMETER;
			goto something_bad_happened; /*goto bottom of function to leave early with error*/
		}

#endif

	/*Define the boundary to be Baud Rate + 18% for excess BW*/
	upper_spur_i32 = (CWC_Offset_Freq_i32 + (int32_t)Symbol_Rate_Div2_u32 + (((int32_t)Symbol_Rate_Div2_u32)*18)/100);
	lower_spur_i32 = (CWC_Offset_Freq_i32 - (int32_t)Symbol_Rate_Div2_u32 - (((int32_t)Symbol_Rate_Div2_u32)*18)/100);

	/*Check for special case of spurs at -1.75 MHz for AnnexB, this is the Tak special*/
	if (hChn->pChnAcqParam->BADS_Internal_Params.Video_Carrier_Annex_B_te == BADS_Internal_Params_eEnable)
	{
		/*Only do if the symbol rate is within the Annex B bounds*/
		if (((Symbol_Rate_u32 >= MIN_Q256_Q1024_ANNEXB_SYMBOL_RATE) && (Symbol_Rate_u32 <= MAX_Q256_Q1024_ANNEXB_SYMBOL_RATE)) ||
		    ((Symbol_Rate_u32 >= MIN_Q64_ANNEXB_SYMBOL_RATE) && (Symbol_Rate_u32 <= MAX_Q64_ANNEXB_SYMBOL_RATE)))
		{
			Fin_Hz_i32[CWC_Length_u8] = -1750000;
			CWC_Length_u8++;
		}
	}

	/*Search for the CWC locations using the spur table in bwfe_global_clk.h or btnr_global_clk.h*/
	for (spur_cnt_u8=0;spur_cnt_u8<SPUR_TBL_SIZE;spur_cnt_u8++)
	{
		if (CWC_Length_u8 > 4)
		{
#if (ADS_INTERNAL_ERROR_CHECKING > 1)
			/*Check that there are not more than 4 tones to cancel since there are only 4 CWC's*/
			BDBG_ERR(("ERROR: CWC SPUR COUNT OUT OF RANGE IN BADS_P_Set_CWC_Auto()"));
			/*REMOVED FOR NOW SINCE THIS COULD TRIGGER IN THE FIELD SO CLAMP THE LENGTH FOR NOW*/
			/*THIS WILL RESULT IN A TABLE ENTRIES BEING IGNORED OF IT IS OVER 4 CWC's*/
			/*RetCode_u32 = BERR_INVALID_PARAMETER;*/
			/*goto something_bad_happened;*/ /*goto bottom of function to leave early with error*/
			CWC_Length_u8 = 4;
#endif
		}
		else
		{
			/*Check if DC spur is enabled in the table: spur frequency = 0*/
			if (SPUR_TBL_u32[spur_cnt_u8] == 0)
			{
				Fin_Hz_i32[CWC_Length_u8] = 0;
				CWC_Length_u8++;
			}
			else
			{
				/*Check the table for spur frequencies within +- buadrate/2 + excess BW*/
				/*Check for numerical overflow in calculation*/
				if (((int32_t)SPUR_TBL_u32[spur_cnt_u8] <= upper_spur_i32) && ((int32_t)SPUR_TBL_u32[spur_cnt_u8] >= lower_spur_i32) && (CWC_Length_u8 < 4))
				{
#if (ADS_INTERNAL_ERROR_CHECKING > 1)
					/*check that spurs are within [0 to POWER2_31_M1]*/
					/*check that [-POWER2_31 <= ((int32_t)SPUR_TBL_u32[spur_cnt_u8] - CWC_Offset_Freq_i32) < POWER2_31]*/
					if ((SPUR_TBL_u32[spur_cnt_u8] > POWER2_31_M1) || (CWC_Offset_Freq_i32 < (TempValue_i32 + (int32_t)SPUR_TBL_u32[spur_cnt_u8])))
					{
						BDBG_ERR(("ERROR: CWC CALCULATION OVERFLOW IN BADS_P_Set_CWC_Auto()"));
						RetCode_u32 = BERR_INVALID_PARAMETER;
						goto something_bad_happened; /*goto bottom of function to leave early with error*/
					}

#endif

					/*Check if we are in the upper bandedge, lower bandedge or neither bandedge*/
					if (((int32_t)SPUR_TBL_u32[spur_cnt_u8] - CWC_Offset_Freq_i32 ) > (int32_t)Symbol_Rate_Div2_u32)
					{
						Wrap_i32 = -2;
					}
					else if  (((int32_t)SPUR_TBL_u32[spur_cnt_u8] - CWC_Offset_Freq_i32 ) < -1*(int32_t)Symbol_Rate_Div2_u32)
					{
						Wrap_i32 = 2;
					}
					else
					{
						Wrap_i32 = 0;
					}

					/*Wrap_i32 is 0 if the spur is within +- BaudRate/2, it is -2 if spur is within +BaudRate/2 + 0.9*BaudRate and 2 if spur is within -BaudRate/2 - 0.9*BaudRate*/
					Fin_Hz_i32[CWC_Length_u8] = (int32_t)SPUR_TBL_u32[spur_cnt_u8] - CWC_Offset_Freq_i32 + Wrap_i32*(int32_t)Symbol_Rate_Div2_u32;
					CWC_Length_u8++;
				}
			}

			/*Check that the Fin_Hz_i32[] is within [-POWER2_23 to POWER2_23_M1] the 24 bit twos complement range in the chip*/
			if ((Fin_Hz_i32[CWC_Length_u8] > POWER2_23_M1) || (Fin_Hz_i32[CWC_Length_u8] < -POWER2_23))
			{
#if (ADS_INTERNAL_ERROR_CHECKING > 2)
				BDBG_ERR(("ERROR: CWC RESULT OUT OF RANGE IN BADS_P_Set_CWC_Auto()"));
				RetCode_u32 = BERR_INVALID_PARAMETER;
				goto something_bad_happened; /*goto bottom of function to leave early with error*/
#endif
			}
		}
	}

/*Some registers were removed in all cores 9.6 and above and the programming is different*/
#if (BADS_P_BCHP_DS_CORE_VER >= BADS_P_BCHP_DS_CORE_V_9_6)
	/*The equations for the CWC locations are now CWC_Tone_Location*POWER2_28/Baud_Rate*/
	/*The register is a twos complement 28 bit*/
	/*remember Symbol_Rate_Div2_u32 has already been divided by 2 so use (Fin_Hz_i32[x]*POWER2_27)/Symbol_Rate_Div2_u32*/

	for (Index_u8=0;Index_u8<3;Index_u8++)
	{
		/*Detect if Fin_Hz_i32[Index_u8] is negative*/
		IsNegative_b = false;
		if (Fin_Hz_i32[Index_u8] < 0)
		{
			IsNegative_b = true;
			Fin_Hz_i32[Index_u8] = -1*Fin_Hz_i32[Index_u8];
		}

		/*All values will be in range*/
		BMTH_HILO_32TO64_Mul(POWER2_27, (uint32_t)Fin_Hz_i32[Index_u8], &Result_Hi_u32, &Result_Lo_u32);
		BMTH_HILO_64TO64_Div32(Result_Hi_u32, Result_Lo_u32, Symbol_Rate_Div2_u32, &Result_Hi_u32, &Result_Lo_u32);

		/*If result should be negative, take twos complement of output, Mask to 28 bits and write value, the DS_EQ_CWC_FIN1 has other things in it*/
		Result_Lo_u32 = (IsNegative_b == true) ? (Twos_Complement32(Result_Lo_u32) & 0x0FFFFFFF) : (Result_Lo_u32 & 0x0FFFFFFF);

		switch (Index_u8)
		{
			case 0: BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS1, FOFFSET1, Result_Lo_u32); break;
			case 1: BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS2, FOFFSET2, Result_Lo_u32); break;
			case 2: BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS3, FOFFSET3, Result_Lo_u32); break;
			case 3: BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS4, FOFFSET4, Result_Lo_u32); break;
			default: BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS4, FOFFSET4,Result_Lo_u32);
		}
	}
#else
	BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FIN1, FIN1_HZ, (uint32_t)Fin_Hz_i32[0]);
	BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FIN2, FIN2_HZ, (uint32_t)Fin_Hz_i32[1]);
	BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FIN3, FIN3_HZ, (uint32_t)Fin_Hz_i32[2]);
	BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FIN4, FIN4_HZ, (uint32_t)Fin_Hz_i32[3]);
	BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS1, FOFFSET1, 0);
	BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS2, FOFFSET2, 0);
	BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS3, FOFFSET3, 0);
	BREG_WriteField(hChn->hRegister, DS_EQ_CWC_FOFS4, FOFFSET4, 0);
#endif

	*CWC_LengthResult_pu8 = CWC_Length_u8;

#if (ADS_INTERNAL_ERROR_CHECKING > 0)
/*goto label to return error code if something bad happened above*/
something_bad_happened:
#endif

  return RetCode_u32;

}
/************************************************************************************
*BADS_P_Get_AcquisitionScan_Settings()
*This function maps the acquisition and scan parameters to local copies
************************************************************************************/
void BADS_P_Get_AcquisitionScan_Settings(BADS_3x7x_ChannelHandle hChn)
{
	/*Acquire Type is Scan*/
	if (hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te == BADS_Acquire_Params_AcqType_eScan)
	{
		/*Set the acquisition Type*/
		hChn->pChnAcqParam->BADS_Local_Params.AcqType_te = BADS_Local_Params_AcqType_eScan;

		/*Check if the Timing Search is enabled in the Scan params*/
		if (hChn->pChnAcqParam->BADS_Scan_Params.TO_te == BADS_Scan_Params_eEnable)
		{
			hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32 = hChn->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32;
			hChn->pChnAcqParam->BADS_Local_Params.Lower_Baud_Search_u32 = hChn->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32;
		}
		else
		{
			/*Set the upper and lower baud search to AnnexA symbol rate or predefined AnnexB symbol rates*/
			/*The Baud Rate is fixed by the selected QAM Mode for Annex B*/
			if (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA)
			{
				hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32 = hChn->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32;
			}
			else
			{
				hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32 = (hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam64) ? Q64_ANNEXB_SYMBOL_RATE : Q256_Q1024_ANNEXB_SYMBOL_RATE;
			}
			hChn->pChnAcqParam->BADS_Local_Params.Lower_Baud_Search_u32 = hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32;
		}

		/*Check if the Carrier Search is enabled in the Scan params*/
		if (hChn->pChnAcqParam->BADS_Scan_Params.CO_te == BADS_Scan_Params_eEnable)
		{
			hChn->pChnAcqParam->BADS_Local_Params.Carrier_Search_u32 = 256*hChn->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32;
		}
		else
		{
			hChn->pChnAcqParam->BADS_Local_Params.Carrier_Search_u32 = 256*hChn->pChnAcqParam->BADS_Acquire_Params.Carrier_Range_u32;
		}

		/*Check if the QAM Search is enabled in the Scan params*/
		if (hChn->pChnAcqParam->BADS_Scan_Params.QM_te == BADS_Scan_Params_eEnable)
		{
			 /*SW3128-257: Add Q1024A directed support but not scan support*/
			hChn->pChnAcqParam->BADS_Local_Params.Q256A_b  =  (hChn->pChnAcqParam->BADS_Scan_Params.A256_te == BADS_Scan_Params_eEnable) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q256B_b  =  (hChn->pChnAcqParam->BADS_Scan_Params.B256_te == BADS_Scan_Params_eEnable) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q64A_b   =  (hChn->pChnAcqParam->BADS_Scan_Params.A64_te == BADS_Scan_Params_eEnable) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q64B_b   =  (hChn->pChnAcqParam->BADS_Scan_Params.B64_te == BADS_Scan_Params_eEnable) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q128A_b  =  (hChn->pChnAcqParam->BADS_Scan_Params.A128_te == BADS_Scan_Params_eEnable)  ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q32A_b   =  (hChn->pChnAcqParam->BADS_Scan_Params.A32_te == BADS_Scan_Params_eEnable)   ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q16A_b   =  (hChn->pChnAcqParam->BADS_Scan_Params.A16_te == BADS_Scan_Params_eEnable)   ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q1024A_b = false;
			hChn->pChnAcqParam->BADS_Local_Params.Q1024B_b =  (hChn->pChnAcqParam->BADS_Scan_Params.B1024_te == BADS_Scan_Params_eEnable) ? true : false;
		}
		else
		{
			 /*SW3128-257: Add Q1024A directed support but not scan support*/
			hChn->pChnAcqParam->BADS_Local_Params.Q256A_b  =  ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam256) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA)) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q256B_b  =  ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam256) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexB)) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q64A_b   =  ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam64) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA)) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q64B_b   =  ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam64) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexB)) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q128A_b =  (hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam128)  ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q32A_b  =  (hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam32)   ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q16A_b  =  (hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam16)   ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q1024A_b = ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam1024) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA)) ? true : false;
			hChn->pChnAcqParam->BADS_Local_Params.Q1024B_b = ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam1024) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexB)) ? true : false;
		}

		/*Check if the Spectrum Search is enabled in the Scan params, set initial spectrum polarity*/
		hChn->pChnAcqParam->BADS_Local_Params.IS_b =	(hChn->pChnAcqParam->BADS_Acquire_Params.IS_te == BADS_Acquire_Params_eEnable) ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Flip_Spectrum_b = (hChn->pChnAcqParam->BADS_Scan_Params.AI_te == BADS_Scan_Params_eEnable) ? true : false;
	}
	else
	{
		/*Set the acquisition Type*/
		/*set for fast acquire initially*/
		hChn->pChnAcqParam->BADS_Local_Params.AcqType_te = BADS_Local_Params_AcqType_eFastAcquire;

		/*Set Timing Search, Upper_Baud_Search and Lower_Baud_Search have the same value*/
		if (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA)
		{
			hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32 = hChn->pChnAcqParam->BADS_Acquire_Params.AnnexA_Sym_Rate_u32;
		}
		else
		{
			hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32 = (hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam64) ? Q64_ANNEXB_SYMBOL_RATE : Q256_Q1024_ANNEXB_SYMBOL_RATE;
		}
		hChn->pChnAcqParam->BADS_Local_Params.Lower_Baud_Search_u32 = hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32;

		/*Set the Carrier Search*/
		hChn->pChnAcqParam->BADS_Local_Params.Carrier_Search_u32 = 256*hChn->pChnAcqParam->BADS_Acquire_Params.Carrier_Range_u32;

		/*Set the QAM Mode*/
		/*SW3128-257: Add Q1024A directed support but not scan support*/
		hChn->pChnAcqParam->BADS_Local_Params.Q256A_b  =  ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam256) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA)) ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Q256B_b  =  ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam256) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexB)) ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Q64A_b   =  ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam64) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA)) ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Q64B_b   =  ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam64) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexB)) ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Q128A_b =  (hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam128)  ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Q32A_b  =  (hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam32)   ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Q16A_b  =  (hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam16)   ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Q1024A_b = ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam1024) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexA))? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Q1024B_b = ((hChn->pChnAcqParam->BADS_Local_Params.QAM_te == BADS_Local_Params_QAM_eQam1024) && (hChn->pChnAcqParam->BADS_Local_Params.Annex_te == BADS_Local_Params_Annex_eAnnexB))? true : false;

		/*set the spectrum polarity*/
		hChn->pChnAcqParam->BADS_Local_Params.IS_b =	(hChn->pChnAcqParam->BADS_Acquire_Params.IS_te == BADS_Acquire_Params_eEnable) ? true : false;
		hChn->pChnAcqParam->BADS_Local_Params.Flip_Spectrum_b =	(hChn->pChnAcqParam->BADS_Scan_Params.AI_te == BADS_Scan_Params_eEnable) ? true : false;

		/*check for transition to slow acquire*/
		if (((hChn->pChnLockStatus->ReAck_Count_u32 > NUM_FAST_ACQUIRES) && (hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te == BADS_Acquire_Params_AcqType_eAuto)) || (hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te == BADS_Acquire_Params_AcqType_eSlow))
		{
			/*set for slow acquire*/
			hChn->pChnAcqParam->BADS_Local_Params.AcqType_te = BADS_Local_Params_AcqType_eSlowAcquire;
		}

		/*check for transition to slow/scan acquire*/
		if ((hChn->pChnLockStatus->ReAck_Count_u32 > (NUM_FAST_ACQUIRES + NUM_SLOW_ACQUIRES)) && (hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te == BADS_Acquire_Params_AcqType_eAuto))
		{
			/*set for slow/scan acquire*/
			hChn->pChnAcqParam->BADS_Local_Params.AcqType_te = BADS_Local_Params_AcqType_eSlowAcquireScan;

			/*Check if the Timing Search is enabled in the Scan params and if so set to the larger value of acquire Baud Rate and scan Upper_Baud_Search*/
			if ((hChn->pChnAcqParam->BADS_Scan_Params.TO_te == BADS_Scan_Params_eEnable) && (hChn->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32 > hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32))
			{
				hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32 = hChn->pChnAcqParam->BADS_Scan_Params.Upper_Baud_Search_u32;
			}
			/*Check if the Timing Search is enabled in the Scan params and if so set to the smaller value of acquire Baud Rate and scan Lower_Baud_Search*/
			if ((hChn->pChnAcqParam->BADS_Scan_Params.TO_te == BADS_Scan_Params_eEnable) && (hChn->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32 < hChn->pChnAcqParam->BADS_Local_Params.Lower_Baud_Search_u32))
			{
				hChn->pChnAcqParam->BADS_Local_Params.Lower_Baud_Search_u32 = hChn->pChnAcqParam->BADS_Scan_Params.Lower_Baud_Search_u32;
			}
			/*Check if the Carrier Search is enabled in the Scan params and if so set to the larger value of acquire Carrier_Range and scan Carrier_Search*/
			if ((hChn->pChnAcqParam->BADS_Scan_Params.CO_te == BADS_Scan_Params_eEnable) && (256*hChn->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32 > hChn->pChnAcqParam->BADS_Local_Params.Carrier_Search_u32))
			{
				hChn->pChnAcqParam->BADS_Local_Params.Carrier_Search_u32 = 256*hChn->pChnAcqParam->BADS_Scan_Params.Carrier_Search_u32;
			}

			/*Check if the QAM Mode Search is enabled in the Scan params and if so set to the union of acquire and search QAM settings*/
			if (hChn->pChnAcqParam->BADS_Scan_Params.QM_te == BADS_Scan_Params_eEnable)
			{
				/*SW3128-257: Add Q1024A directed support but not scan support*/
				hChn->pChnAcqParam->BADS_Local_Params.Q256A_b  = hChn->pChnAcqParam->BADS_Local_Params.Q256A_b  || ((hChn->pChnAcqParam->BADS_Scan_Params.A256_te  == BADS_Scan_Params_eEnable) ? true : false);
				hChn->pChnAcqParam->BADS_Local_Params.Q256B_b  = hChn->pChnAcqParam->BADS_Local_Params.Q256B_b  || ((hChn->pChnAcqParam->BADS_Scan_Params.B256_te  == BADS_Scan_Params_eEnable) ? true : false);
				hChn->pChnAcqParam->BADS_Local_Params.Q64A_b   = hChn->pChnAcqParam->BADS_Local_Params.Q64A_b   || ((hChn->pChnAcqParam->BADS_Scan_Params.A64_te   == BADS_Scan_Params_eEnable) ? true : false);
				hChn->pChnAcqParam->BADS_Local_Params.Q64B_b   = hChn->pChnAcqParam->BADS_Local_Params.Q64B_b   || ((hChn->pChnAcqParam->BADS_Scan_Params.B64_te   == BADS_Scan_Params_eEnable) ? true : false);
				hChn->pChnAcqParam->BADS_Local_Params.Q128A_b  = hChn->pChnAcqParam->BADS_Local_Params.Q128A_b  || ((hChn->pChnAcqParam->BADS_Scan_Params.A128_te  == BADS_Scan_Params_eEnable) ? true : false);
				hChn->pChnAcqParam->BADS_Local_Params.Q32A_b   = hChn->pChnAcqParam->BADS_Local_Params.Q32A_b   || ((hChn->pChnAcqParam->BADS_Scan_Params.A32_te   == BADS_Scan_Params_eEnable) ? true : false);
				hChn->pChnAcqParam->BADS_Local_Params.Q16A_b   = hChn->pChnAcqParam->BADS_Local_Params.Q16A_b   || ((hChn->pChnAcqParam->BADS_Scan_Params.A16_te   == BADS_Scan_Params_eEnable) ? true : false);
				hChn->pChnAcqParam->BADS_Local_Params.Q1024A_b = hChn->pChnAcqParam->BADS_Local_Params.Q1024A_b;
				hChn->pChnAcqParam->BADS_Local_Params.Q1024B_b = hChn->pChnAcqParam->BADS_Local_Params.Q1024B_b || ((hChn->pChnAcqParam->BADS_Scan_Params.B1024_te == BADS_Scan_Params_eEnable) ? true : false);
			}

			/*Check if the Spectrum Search is enabled in the Scan params*/
			if (hChn->pChnAcqParam->BADS_Scan_Params.AI_te == BADS_Scan_Params_eEnable)
			{
				hChn->pChnAcqParam->BADS_Local_Params.Flip_Spectrum_b = true;
			}
		}
	}


	/*Debug print*/
	if (PRINT_DEBUG==1)
	{
		if (hChn->pChnLockStatus->ReAck_Count_u32 == 1)
		{
			BDBG_ERR((" *******************************************************************************************************************************************"));
		}
		if (hChn->pChnLockStatus->ReAck_Count_u32 < 9)
		{
			/*SW3128-257: Add Q1024A directed support but not scan support*/
			BDBG_ERR((" ReAck= %d AcqType= %d UBS= %d LBS= %d CS= %d SI= %d AI= %d Q256A= %d Q256B= %d Q64A= %d Q64B= %d Q128A= %d Q32A= %d Q16A= %d, Q1024A= %d, Q1024B= %d",
								hChn->pChnLockStatus->ReAck_Count_u32,
								hChn->pChnAcqParam->BADS_Local_Params.AcqType_te,
								hChn->pChnAcqParam->BADS_Local_Params.Upper_Baud_Search_u32,
								hChn->pChnAcqParam->BADS_Local_Params.Lower_Baud_Search_u32,
								hChn->pChnAcqParam->BADS_Local_Params.Carrier_Search_u32,
								hChn->pChnAcqParam->BADS_Local_Params.IS_b,
								hChn->pChnAcqParam->BADS_Local_Params.Flip_Spectrum_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q256A_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q256B_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q64A_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q64B_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q128A_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q32A_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q16A_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q1024A_b,
								hChn->pChnAcqParam->BADS_Local_Params.Q1024B_b
								));
		}
	}
}


/************************************************************************************
*BADS_P_Set_ScanStatus_Params()
*This function maps the scan status parameters
************************************************************************************/
void BADS_P_Set_ScanStatus_Params(BADS_3x7x_ChannelHandle hChn, uint8_t QamIndex_u8, uint8_t FECIndex_u8, int32_t CarrierOffset_i32, int32_t Carrier_Error_i32, uint32_t Symbol_Rate_u32)
{
	/*Assign the scan status parameters*/
	if (hChn->pChnLockStatus->FLK_te != BADS_3x7x_ChnLockStatus_eLock)
	{
		hChn->pChnScanStatus->FLK_te = BADS_3x7x_ChnLockStatus_eUnlock;
		hChn->pChnScanStatus->QLK_te = BADS_3x7x_ChnLockStatus_eUnlock;
        hChn->pChnScanStatus->Auto_te = 0;
        hChn->pChnScanStatus->AcqType_te = 0;
        hChn->pChnScanStatus->AcqStatus_te = 0;
		hChn->pChnScanStatus->Spectrum_te = 0;
		hChn->pChnScanStatus->BPS_te = 0;
		hChn->pChnScanStatus->AB_te = 0;
		hChn->pChnScanStatus->Interleaver_te = 0;
		hChn->pChnScanStatus->RF_Offset_i32 = 0;
		hChn->pChnScanStatus->Symbol_Rate_u32 = 0;
	}
	else
	{
        /* FEC and QAM Lock */
        hChn->pChnScanStatus->FLK_te = (hChn->pChnLockStatus->FLK_te == BADS_3x7x_ChnLockStatus_eLock) ?  BADS_3x7x_ChnScanStatus_eLock : BADS_3x7x_ChnScanStatus_eUnlock;
		hChn->pChnScanStatus->QLK_te = (hChn->pChnLockStatus->QLK_te == BADS_3x7x_ChnLockStatus_eLock) ?  BADS_3x7x_ChnScanStatus_eLock : BADS_3x7x_ChnScanStatus_eUnlock;

        /* Feedback of Acquire Parameters (Auto) */
        hChn->pChnScanStatus->Auto_te = (hChn->pChnAcqParam->BADS_Acquire_Params.Auto_te == BADS_Acquire_Params_eEnable) ? BADS_3x7x_ChnScanStatus_eEnable : BADS_3x7x_ChnScanStatus_eDisable;

        /* Feedback of Acquire Parameters (AcqType) */
        switch ( hChn->pChnAcqParam->BADS_Acquire_Params.AcqType_te )
        {
            case BADS_Acquire_Params_AcqType_eAuto : hChn->pChnScanStatus->AcqType_te =  BADS_3x7x_ChnScanStatus_AcqType_eAuto; break;
            case BADS_Acquire_Params_AcqType_eFast : hChn->pChnScanStatus->AcqType_te =  BADS_3x7x_ChnScanStatus_AcqType_eFast; break;
            case BADS_Acquire_Params_AcqType_eSlow : hChn->pChnScanStatus->AcqType_te =  BADS_3x7x_ChnScanStatus_AcqType_eSlow; break;
            case BADS_Acquire_Params_AcqType_eScan : hChn->pChnScanStatus->AcqType_te =  BADS_3x7x_ChnScanStatus_AcqType_eScan; break;
            default : BDBG_ERR(("ERROR!!! UNSUPPORTED OR UNDEFINED AcqType")); break;
        }

		switch (hChn->pChnAcqParam->BADS_Local_Params.AcqStatus_te)
		{
			case BADS_Local_Params_AcqStatus_eNoPower:    hChn->pChnScanStatus->AcqStatus_te = BADS_3x7x_ChnScanStatus_AcqStatus_eNoPower;    break;
			case BADS_Local_Params_AcqStatus_eNoTiming:   hChn->pChnScanStatus->AcqStatus_te = BADS_3x7x_ChnScanStatus_AcqStatus_eNoTiming;   break;
			case BADS_Local_Params_AcqStatus_eNoCarrier:  hChn->pChnScanStatus->AcqStatus_te = BADS_3x7x_ChnScanStatus_AcqStatus_eNoCarrier;  break;
			case BADS_Local_Params_AcqStatus_eNoFECLock:  hChn->pChnScanStatus->AcqStatus_te = BADS_3x7x_ChnScanStatus_AcqStatus_eNoFECLock;  break;
			case BADS_Local_Params_AcqStatus_eLockedFast: hChn->pChnScanStatus->AcqStatus_te = BADS_3x7x_ChnScanStatus_AcqStatus_eLockedFast; break;
			case BADS_Local_Params_AcqStatus_eLockedSlow: hChn->pChnScanStatus->AcqStatus_te = BADS_3x7x_ChnScanStatus_AcqStatus_eLockedSlow; break;
			case BADS_Local_Params_AcqStatus_eEarlyExit:  hChn->pChnScanStatus->AcqStatus_te = BADS_3x7x_ChnScanStatus_AcqStatus_eEarlyExit;  break;
			default : BDBG_ERR(("ERROR!!! UNSUPPORTED OR UNDEFINED AcqStatus")); break;
		}

		switch (hChn->pChnAcqParam->BADS_Local_Params.FECSpectrum_te)
		{
			case BADS_Local_Params_FECSpectrum_eNotInverted:           hChn->pChnScanStatus->Spectrum_te = BADS_3x7x_ChnScanStatus_Spectrum_eNormal;  break;
			case BADS_Local_Params_FECSpectrum_eInverted:              hChn->pChnScanStatus->Spectrum_te = BADS_3x7x_ChnScanStatus_Spectrum_eNormal;  break;
			case BADS_Local_Params_FECSpectrum_eNotInvertedAutoInvert: hChn->pChnScanStatus->Spectrum_te = BADS_3x7x_ChnScanStatus_Spectrum_eFlipped; break;
			case BADS_Local_Params_FECSpectrum_eInvertedAutoInvert:    hChn->pChnScanStatus->Spectrum_te = BADS_3x7x_ChnScanStatus_Spectrum_eFlipped; break;
			default : BDBG_ERR(("ERROR!!! UNSUPPORTED OR UNDEFINED FECSpectrum")); break;
		}

		switch (QamIndex_u8)
		{
			case 0: hChn->pChnScanStatus->BPS_te =	BADS_3x7x_ChnScanStatus_BPS_eQam256;  break;
			case 1: hChn->pChnScanStatus->BPS_te =	BADS_3x7x_ChnScanStatus_BPS_eQam64;   break;
			case 2: hChn->pChnScanStatus->BPS_te =	BADS_3x7x_ChnScanStatus_BPS_eQam128;  break;
			case 3: hChn->pChnScanStatus->BPS_te =	BADS_3x7x_ChnScanStatus_BPS_eQam32;   break;
			case 4: hChn->pChnScanStatus->BPS_te =	BADS_3x7x_ChnScanStatus_BPS_eQam16;   break;
			case 5: hChn->pChnScanStatus->BPS_te =	BADS_3x7x_ChnScanStatus_BPS_eQam1024; break;
			default : BDBG_ERR(("INVALID QamTry_b Value")); break;
		}

		hChn->pChnScanStatus->AB_te = (FECIndex_u8 == 0) ? BADS_3x7x_ChnScanStatus_Annex_eAnnexA : BADS_3x7x_ChnScanStatus_Annex_eAnnexB;

		if (FECIndex_u8 == 0)
		{
			hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI12_J17;
		}
		else
		{
			switch (BREG_ReadField(hChn->hRegister, DS_FEC, IDS))
			{
				case 0:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI128_J1; break;
				case 1:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI128_J1; break;
				case 2:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI128_J2; break;
				case 3:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI64_J2;  break;
				case 4:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI128_J3; break;
				case 5:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI32_J4;  break;
				case 6:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI128_J4; break;
				case 7:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI16_J8;  break;
				case 9:  hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI8_J16;  break;
				case 11: hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI4_J32;  break;
				case 13: hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI2_J64;  break;
				case 15: hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eI1_J128; break;
				default: hChn->pChnScanStatus->Interleaver_te = BADS_3x7x_ChnScanStatus_Interleaver_eUnsupported; break;
			}
		}

		if (hChn->pChnAcqParam->BADS_Internal_Params.Callback_Enable_te == BADS_Internal_Params_eEnable)
		{
			hChn->pChnScanStatus->RF_Offset_i32 = CarrierOffset_i32 + Carrier_Error_i32;
		}
		else
		{
			hChn->pChnScanStatus->RF_Offset_i32 = 0;
		}

		hChn->pChnScanStatus->Symbol_Rate_u32 = Symbol_Rate_u32;
	}
}

/********************************************************************************************
*BADS_P_ADS_SLEEP() This is used to allow early exit from acquisition by checking before each delay
********************************************************************************************/
bool BADS_P_ADS_SLEEP(BADS_3x7x_ChannelHandle hChn, unsigned int Delay)
{
    static const uint8_t uShift = 5; /* 32 msec increments */
    volatile uint32_t uRemainderDelay = 0;
    volatile uint32_t uIncrementalDelay;

    /* In rev B0 chip there will be more avail BKNI_Events to use.  For now, use loop-sleep delay method */
    /* Divide delay by 32 msec */
    uIncrementalDelay = (Delay >> uShift);
    uRemainderDelay = Delay - (uIncrementalDelay<<uShift);
    if ( uRemainderDelay > Delay )
    {
        uRemainderDelay = 0;
    }

    /* BDBG_MSG(("BADS_P_ADS_SLEEP: D: 0x%06X, I: 0x%06X, R: 0x%06X", Delay, uIncrementalDelay , uRemainderDelay)); */

    /* Delay loop */
    while ( (0 != uIncrementalDelay) || (0 != uRemainderDelay) )
    {
        /* Terminate on early exit */
        if ( hChn->pChnAcqParam->BADS_Local_Params.EarlyExit )
        {
#if FALSE /* Debug only */
            BDBG_MSG(("**** EARLY EXIT Ads Chn %d ****", hChn->chnNo));
            BDBG_MSG(("**** EARLY EXIT Ads Chn %d ****", hChn->chnNo));
            BDBG_MSG(("**** EARLY EXIT Ads Chn %d ****", hChn->chnNo));
#endif /* FALSE - Debug only */
            /* Stop while-loop*/
            break;
        }
        /* Delay using incremental delay count (Delay/32) */
        if ( uIncrementalDelay )
        {
            BKNI_Sleep((1<<uShift));
            --uIncrementalDelay;
            /* Continue while-loop*/
            continue;
        }
        /* Final/last Delay (Delay MOD uIncrementalDelay) */
        if ( 0 != uRemainderDelay )
        {
            BKNI_Sleep(uRemainderDelay);
            /* Stop while-loop*/
            break;
        }
    }

    return ( hChn->pChnAcqParam->BADS_Local_Params.EarlyExit );
}


