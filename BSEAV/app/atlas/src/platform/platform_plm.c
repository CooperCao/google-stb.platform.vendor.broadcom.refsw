/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nexus_platform.h"
#include "nxclient.h"
#include "bstd.h"
#include "platform.h"
#include "platform_plm_priv.h"
#include "bdbg.h"

BDBG_MODULE(platform_plm);

struct lRangeAdjStore {
    unsigned   vid[MAX_MOSAICS];
} g_lRangeAdjStore = { { 0 } };

void platform_plm_get_vid_lra(
        unsigned inputIndex,
        unsigned rectIndex,
        bool *   enabled
        )
{
#if HAS_VID_NL_LUMA_RANGE_ADJ
    uint32_t reg;
    unsigned lRangeAdj;
    unsigned rectDelta;

#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
    rectDelta = rectIndex;
#else
    rectDelta = (rectIndex>>1);
#endif
    NEXUS_Platform_ReadRegister(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + 4*rectDelta + (inputIndex*VID_NLCONFIG_INDEX_DELTA), &reg);
#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
    lRangeAdj = BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ);
#else
    lRangeAdj = rectIndex&1 ? BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT1_SEL_LRANGE_ADJ) : BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_LRANGE_ADJ);
#endif
    *enabled = (lRangeAdj != LRANGE_ADJ_DISABLE);
#else /* if HAS_VID_NL_LUMA_RANGE_ADJ */
    BSTD_UNUSED(inputIndex);
    BSTD_UNUSED(rectIndex);
#endif /* if HAS_VID_NL_LUMA_RANGE_ADJ */
} /* platform_plm_get_vid_lra */

#include <stdio.h>
void platform_plm_set_vid_lra(
        unsigned inputIndex,
        unsigned rectIndex,
        bool     enabled
        )
{
#if HAS_VID_NL_LUMA_RANGE_ADJ
    uint32_t reg;
    unsigned lRangeAdj, curLRangeAdj;
    unsigned rectDelta;

#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
    rectDelta = rectIndex;
#else
    rectDelta = (rectIndex>>1);
#endif
    NEXUS_Platform_ReadRegister(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + 4*rectDelta + (inputIndex*VID_NLCONFIG_INDEX_DELTA), &reg);
#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
    curLRangeAdj = BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ);
#else
    curLRangeAdj = rectIndex&1 ? BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT1_SEL_LRANGE_ADJ) : BCHP_GET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_LRANGE_ADJ);
#endif
    if (enabled)
    {
        lRangeAdj = g_lRangeAdjStore.vid[rectIndex];
    }
    else
    {
        lRangeAdj = LRANGE_ADJ_DISABLE;
    }
    if (lRangeAdj != curLRangeAdj)
    {
#if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK
        BCHP_SET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ, lRangeAdj);
#else
        if (rectIndex&1)
        {
            BCHP_SET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT1_SEL_LRANGE_ADJ, lRangeAdj);
        }
        else
        {
            BCHP_SET_FIELD_DATA(reg, HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_LRANGE_ADJ, lRangeAdj);
        }
#endif /* if BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_MASK */
        NEXUS_Platform_WriteRegister(BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + 4*rectDelta + (inputIndex*VID_NLCONFIG_INDEX_DELTA), reg);
    }
    if (curLRangeAdj != LRANGE_ADJ_DISABLE)
    {
        g_lRangeAdjStore.vid[rectIndex] = curLRangeAdj;
    }
#else /* if HAS_VID_NL_LUMA_RANGE_ADJ */
    BSTD_UNUSED(inputIndex);
    BSTD_UNUSED(rectIndex);
#endif /* if HAS_VID_NL_LUMA_RANGE_ADJ */
} /* platform_plm_set_vid_lra */

void platform_plm_get_gfx_lra(
        unsigned inputIndex,
        unsigned rectIndex,
        bool *   enabled
        )
{
#if HAS_GFX_NL_LUMA_RANGE_ADJ
    uint32_t reg;
    unsigned lRangeAdj;

    NEXUS_Platform_ReadRegister(BCHP_GFD_0_NL_CSC_CTRL, &reg);
    lRangeAdj = BCHP_GET_FIELD_DATA(reg, GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN);
    *enabled  = (lRangeAdj == BCHP_GFD_0_NL_CSC_CTRL_LRANGE_ADJ_EN_ENABLE);
#endif /* if HAS_GFX_NL_LUMA_RANGE_ADJ */
    BSTD_UNUSED(inputIndex);
    BSTD_UNUSED(rectIndex);
}

void platform_plm_set_gfx_lra(
        unsigned inputIndex,
        unsigned rectIndex,
        bool     enabled
        )
{
#if HAS_GFX_NL_LUMA_RANGE_ADJ
    uint32_t reg;
    unsigned lRangeAdj, curLRangeAdj;

    NEXUS_Platform_ReadRegister(BCHP_GFD_0_NL_CSC_CTRL, &reg);
    curLRangeAdj = BCHP_GET_FIELD_DATA(reg, GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN);
    if (enabled)
    {
        lRangeAdj = 1;
    }
    else
    {
        lRangeAdj = 0;
    }
    if (lRangeAdj != curLRangeAdj)
    {
        BCHP_SET_FIELD_DATA(reg, GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN, lRangeAdj);
        NEXUS_Platform_WriteRegister(BCHP_GFD_0_NL_CSC_CTRL, reg);
    }
#else /* if HAS_GFX_NL_LUMA_RANGE_ADJ */
    BSTD_UNUSED(lra);
#endif /* if HAS_GFX_NL_LUMA_RANGE_ADJ */
    BSTD_UNUSED(inputIndex);
    BSTD_UNUSED(rectIndex);
} /* platform_plm_set_gfx_lra */