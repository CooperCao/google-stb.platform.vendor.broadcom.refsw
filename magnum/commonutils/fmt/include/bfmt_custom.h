/******************************************************************************
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
 *****************************************************************************/

#ifndef BFMT_CUSTOM_H__
#define BFMT_CUSTOM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#include "bfmt.h"
#include "bkni.h"

typedef struct BFMT_P_RateInfo
{
    /* Use for searching a matching one! */
    uint64_t                    ulPixelClkRate; /*                  ----3548/3556----                */
    uint32_t                    ulMDiv;         /* LVDS_PHY_0_LVDS_PLL_CFG, PLL_M_DIV,               */
    uint32_t                    ulNDiv;         /* DVPO_RM_0_OFFSET, OFFSET,                         */
    uint32_t                    ulRDiv;         /* LVDS_PHY_0_LVDS_PLL_CFG, PLL_R_DIV,               */
    uint32_t                    ulSampleInc;    /* DVPO_RM_0_SAMPLE_INC, SAMPLE_INC,                 */
    uint32_t                    ulNumerator;    /* DVPO_RM_0_SAMPLE_INC, NUMERATOR,                  */
    uint32_t                    ulDenominator;  /* DVPO_RM_0_RATE_RATIO, DENOMINATOR,                */
    uint32_t                    ulVcoRange;     /* LVDS_PHY_0_LVDS_PLL_CFG, PLL_VCO_RANGE,           */
    uint32_t                    ulLinkDivCtrl;  /* LVDS_PHY_0_LVDS_PLL_CFG, LINKDIV_CTRL,            */
    uint32_t                    ulP2;           /* LVDS_PHY_0_LVDS_PLL_CFG, PLL_FEEDBACK_PRE_DIVIDER */
    const char                 *pchRate;
} BFMT_P_RateInfo;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BFMT_CUSTOM_H__ */

/* End of File */
