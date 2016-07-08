/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
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
    BVDC_P_Compositor_Info   *pCurInfo;

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
