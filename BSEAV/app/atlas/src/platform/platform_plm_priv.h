/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef PLATFORM_PLM_PRIV_H__
#define PLATFORM_PLM_PRIV_H__  1

#include "bstd.h"
#include "bchp_common.h"
#define SLOPE_MAN_I_BITS            0
#define SLOPE_MAN_F_BITS            15
#define SLOPE_EXP_I_BITS            4
#define SLOPE_EXP_F_BITS            0
#define LR_COORD_I_BITS             1
#define LR_COORD_F_BITS             15

/* TODO: from bmth_fix.h, fix makefile so can include */
#define BMTH_P_FIX_MAX_BITS         32
#define BMTH_P_FIX_SIGNED_MAX_BITS  (BMTH_P_FIX_MAX_BITS - 1)
#define BMTH_P_FIX_SIGNED_MASK(intbits, fractbits) \
    (0xFFFFFFFF >> (BMTH_P_FIX_SIGNED_MAX_BITS - (intbits + fractbits)))
#define BMTH_P_FIX_SIGN_BIT(intbits, fractbits) \
    (1 << (intbits + fractbits))
#define BMTH_FIX_SIGNED_ITOFIX(x, intbits, fractbits) \
    (((x) << fractbits) & BMTH_P_FIX_SIGNED_MASK(intbits, fractbits))
#define BMTH_FIX_SIGNED_FIXTOI(x, intbits, fractbits)                     \
    (((x) & BMTH_P_FIX_SIGN_BIT(intbits, fractbits)) ?                    \
     ((x) >> (fractbits) | ~BMTH_P_FIX_SIGNED_MASK(intbits, fractbits)) : \
     ((x) >> (fractbits)))
#define BMTH_FIX_SIGNED_FTOFIX(x, intbits, fractbits) \
    ((int32_t)(((x) * (1 << fractbits)) + ((x > 0) ? 0.5f : -0.5f)) & BMTH_P_FIX_SIGNED_MASK(intbits, fractbits))
#define BMTH_FIX_SIGNED_FIXTOF(x, intbits, fractbits)          \
    ((int32_t)((BMTH_P_FIX_SIGN_BIT(intbits, fractbits) & x) ? \
               -((~BMTH_P_FIX_SIGN_BIT(intbits, fractbits) & ~x) + 1) : x) / (float)(1 << fractbits))

#ifdef BCHP_HDR_CMP_0_REG_START
#include "bchp_hdr_cmp_0.h"

/* no mosaic HDR support so disable feature */
#ifndef BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_BASE
#define HAS_VID_NL_LUMA_RANGE_ADJ  0

/* HDR 1.0 */
#elif defined (BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE)
#define VID_SLOPE_RECT_DELTA       (BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE)
#define VID_SLOPE_INDEX_DELTA      (BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE)
#define VID_XY_RECT_DELTA          (BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_ARRAY_BASE)
#define VID_XY_INDEX_DELTA         (BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_ARRAY_BASE)
#define VID_NLCONFIG_INDEX_DELTA   (BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE)
#define LRANGE_ADJ_DISABLE         BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_DISABLE
#define HAS_VID_NL_LUMA_RANGE_ADJ  1

/* HDR 2.0 */
#elif defined (BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_ARRAY_BASE)
#define VID_SLOPE_RECT_DELTA       (BCHP_HDR_CMP_0_V0_R1_LR_SLOPEi_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_ARRAY_BASE)
#define VID_SLOPE_INDEX_DELTA      (BCHP_HDR_CMP_0_V1_R0_LR_SLOPEi_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_ARRAY_BASE)
#define VID_XY_RECT_DELTA          (BCHP_HDR_CMP_0_V0_R1_LR_XY_0i_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_LR_XY_0i_ARRAY_BASE)
#define VID_XY_INDEX_DELTA         (BCHP_HDR_CMP_0_V1_R0_LR_XY_0i_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_LR_XY_0i_ARRAY_BASE)
#define VID_NLCONFIG_INDEX_DELTA   (BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE)
#define LRANGE_ADJ_DISABLE         BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LRANGE_ADJ_DISABLE
#define HAS_VID_NL_LUMA_RANGE_ADJ  1

/* UNKNOWN HDR chip */
#else /* if defined (BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE) */
#define HAS_VID_NL_LUMA_RANGE_ADJ  0
#endif /* if defined (BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE) */

/* NO HDR_CMP registers */
#else /* ifdef BCHP_HDR_CMP_0_REG_START */
#define HAS_VID_NL_LUMA_RANGE_ADJ  0
#endif /* ifdef BCHP_HDR_CMP_0_REG_START */

#ifdef BCHP_GFD_0_REG_START
#include "bchp_gfd_0.h"
#ifdef BCHP_GFD_0_NL_LR_SLOPEi_ARRAY_BASE
#define HAS_GFX_NL_LUMA_RANGE_ADJ  1
#else
#define HAS_GFX_NL_LUMA_RANGE_ADJ  0
#endif
#else /* ifdef BCHP_GFD_0_REG_START */
#define HAS_GFX_NL_LUMA_RANGE_ADJ  0
#endif /* ifdef BCHP_GFD_0_REG_START */

#endif /* PLATFORM_PLM_PRIV_H__ */