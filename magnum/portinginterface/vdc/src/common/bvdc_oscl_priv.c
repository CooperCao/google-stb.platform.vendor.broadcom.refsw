/***************************************************************************
 *     Copyright (c) 2009-2012, Broadcom Corporation
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
#include "bvdc_oscl_priv.h"
#include "bmth_fix.h"
#include "bvdc_common_priv.h"

#if (BVDC_P_SUPPORT_OSCL)
#include "bchp_oscl_0.h"
#endif


BDBG_MODULE(BVDC_OSCL);

#if (BVDC_P_SUPPORT_OSCL)
void BVDC_P_OSCL_BuildRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldId )
{
	BVDC_P_Compositor_Info	 *pCurInfo;

	BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
	pCurInfo = &hCompositor->stCurInfo;

	/* Note: Currently we update each field of OSCL every vSync. This can be
	 * optimized so that common fields will be programmed only once, and only
	 * INIT_PHASE gets updated every vSync. */
	if (pCurInfo->bEnableOScl)
	{
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_SRC_HSIZE);
		*pList->pulCurrent++ = BCHP_FIELD_DATA(OSCL_0_SRC_HSIZE, HSIZE, pCurInfo->pFmtInfo->ulWidth);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_DST_VSIZE);
		*pList->pulCurrent++ = BCHP_FIELD_DATA(OSCL_0_DST_VSIZE, VSIZE, pCurInfo->pFmtInfo->ulHeight);

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_VERT_FIR_INIT_PHASE);
		*pList->pulCurrent++ = (BAVC_Polarity_eTopField == eFieldId) ? 0 : 0xffff8000;

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_VERT_FIR_SRC_STEP);
		*pList->pulCurrent++ = 0x00080000;

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_VERT_FIR_COEFF1_PHASE_00_01);
		*pList->pulCurrent++ = 0x08001000;

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_VERT_FIR_COEFF1_PHASE_02_03);
		*pList->pulCurrent++ = 0x08000800;

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_VERT_FIR_COEFF1_PHASE_04_05);
		*pList->pulCurrent++ = 0x08000800;

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_VERT_FIR_COEFF1_PHASE_06_07);
		*pList->pulCurrent++ = 0x08000800;

		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_CTRL);
		*pList->pulCurrent++ =
			BCHP_FIELD_ENUM(OSCL_0_CTRL, MODE, FILTER) |
			BCHP_FIELD_ENUM(OSCL_0_CTRL, ENABLE_CTRL, AUTO_DISABLE) |
			BCHP_FIELD_ENUM(OSCL_0_CTRL, BUFF_BYPASS, NORMAL)  |
			BCHP_FIELD_ENUM(OSCL_0_CTRL, SCALE_BYPASS, NORMAL) |
			BCHP_FIELD_ENUM(OSCL_0_CTRL, BUFF_ENABLE, ENABLE)  |
			BCHP_FIELD_ENUM(OSCL_0_CTRL, SCALE_ENABLE, ENABLE);
	}
	else
	{
		*pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
		*pList->pulCurrent++ = BRDC_REGISTER(BCHP_OSCL_0_CTRL);
		*pList->pulCurrent++ =
			BCHP_FIELD_ENUM(OSCL_0_CTRL, SCALE_BYPASS, BYPASS) |
			BCHP_FIELD_ENUM(OSCL_0_CTRL, BUFF_BYPASS, BYPASS);

	}
}
#endif
/* End of File */
