/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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
#include "bstd.h"
#include "bdbg.h"
#include "bvdc_display_priv.h"

BDBG_MODULE(BVDC_DISP);

#if !BVDC_P_ORTHOGONAL_VEC
void BVDC_P_Vec_BuildCCBRul_isr
	( BVDC_P_DisplayContext           *pDisplay,
	  BVDC_P_ListInfo                 *pList)
{
#if BVDC_P_VEC_SUPPORT_DVI_COLOR_CNVT
	/* TODO: This build RUL should be off the current context */
	BVDC_P_DisplayInfo  *pNewInfo = &pDisplay->stNewInfo;

	BDBG_ENTER(BVDC_P_Vec_BuildCCBRul_isr);
	BDBG_MSG(("disp[%d] - Build BVDC_P_Vec_BuildCCBRul_isr", pDisplay->eId));

	if(pNewInfo->bCCEnable)
	{
		if(!pNewInfo->bUserCCTable)
		{
			const BVDC_P_FormatCCBTbl *pCCBTbl;
			pCCBTbl = BVDC_P_Display_GetCCBTable(pNewInfo->ulGammaTableId, pNewInfo->ulColorTempId);
			if(pCCBTbl == NULL)
			{
				return;
			}

			/* Copy pCCBTbl to RUL */
			BDBG_MSG(("disp[%d] - Load Internal table", pDisplay->eId));
			*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CCB_TABLE_SIZE);
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_CCB_CCB_ELEMENT_i_ARRAY_BASE);
			BKNI_Memcpy((void*)pList->pulCurrent, (void*)&pCCBTbl->pulCCBTbl[0], BVDC_P_CCB_TABLE_SIZE * sizeof(uint32_t));
			pList->pulCurrent += BVDC_P_CCB_TABLE_SIZE;
		}
		else
		{
			/* Copy user table to RUL */
			BDBG_MSG(("disp[%d] - Load External table", pDisplay->eId));
			*pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CCB_TABLE_SIZE);
			*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_CCB_CCB_ELEMENT_i_ARRAY_BASE);
			BKNI_Memcpy((void*)pList->pulCurrent, (void*)&pNewInfo->aulUserCCTable[0], BVDC_P_CCB_TABLE_SIZE * sizeof(uint32_t));
			pList->pulCurrent += BVDC_P_CCB_TABLE_SIZE;
		}
	}

	*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
	*pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_CCB_CCB_CTRL);
	*pList->pulCurrent++ =
		BCHP_FIELD_ENUM(DVI_CCB_CCB_CTRL, BYPASS,                    AUTO) |
		BCHP_FIELD_ENUM(DVI_CCB_CCB_CTRL, CH2_WR_ENABLE,               ON) |
		BCHP_FIELD_ENUM(DVI_CCB_CCB_CTRL, CH1_WR_ENABLE,               ON) |
		BCHP_FIELD_ENUM(DVI_CCB_CCB_CTRL, CH0_WR_ENABLE,               ON) |
		BCHP_FIELD_DATA(DVI_CCB_CCB_CTRL, CH2_ENABLE, pNewInfo->bCCEnable) |
		BCHP_FIELD_DATA(DVI_CCB_CCB_CTRL, CH1_ENABLE, pNewInfo->bCCEnable) |
		BCHP_FIELD_DATA(DVI_CCB_CCB_CTRL, CH0_ENABLE, pNewInfo->bCCEnable);

	BDBG_LEAVE(BVDC_P_Vec_BuildCCBRul_isr);
#else
	BSTD_UNUSED(pDisplay);
	BSTD_UNUSED(pList);
#endif

	return;
}
#endif

/* End of File */
