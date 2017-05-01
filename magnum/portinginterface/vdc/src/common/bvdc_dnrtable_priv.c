/***************************************************************************
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
 ***************************************************************************/

#include "bvdc_dnr_priv.h"

BDBG_MODULE(BVDC_DNR);

#if BVDC_P_SUPPORT_DNR

#define BVDC_P_DCR_COUNT (sizeof(s_aDcrCfgTbl)/sizeof(BVDC_P_DcrCfgEntry))

#define BVDC_P_MAKE_DCR(f_0, f_1, f_2, f_3, f_clamp, rdmA, rdmB, rdmC, rdmD, r_clamp, order_A, order_B) \
{                                                                            \
    (f_0),                        /* DCR_0_DCR_FILT_LIMIT.FILT_0_LIMIT */    \
    (f_1),                        /* DCR_0_DCR_FILT_LIMIT.FILT_1_LIMIT */    \
    (f_2),                        /* DCR_0_DCR_FILT_LIMIT.FILT_2_LIMIT */    \
    (f_3),                        /* DCR_0_DCR_FILT_LIMIT.FILT_3_LIMIT */    \
    (f_clamp),                    /* DNR_0_DCR_DITH_OUT_CTRL.FILT_CLAMP */   \
    (rdmA),                       /* DNR_0_DCR_DITH_RANDOM_VALUE.RANDOM_A */ \
    (rdmB),                       /* DNR_0_DCR_DITH_RANDOM_VALUE.RANDOM_B */ \
    (rdmC),                       /* DNR_0_DCR_DITH_RANDOM_VALUE.RANDOM_C */ \
    (rdmD),                       /* DNR_0_DCR_DITH_RANDOM_VALUE.RANDOM_D */ \
    (r_clamp),                    /* DNR_0_DCR_DITH_OUT_CTRL.DITH_CLAMP */   \
    (order_A),                    /*DNR_0_DCR_DITH_ORDER_VALUE.ORDER_A */    \
    (order_B),                    /*DNR_0_DCR_DITH_ORDER_VALUE.ORDER_B */    \
}

/* Static lookup table to figure out configuruation of DCR
 * INDEX: none search. */
static const BVDC_P_DcrCfgEntry s_aDcrCfgTbl[] =
{
    /*              f_0, f_1, f_2, f_3, f_clamp, rdmA, rdmB, rdmC, rdmD, r_clamp, order_A, order_B */
    BVDC_P_MAKE_DCR(0x3, 0x2, 0x1, 0x1, 0x6,     0x2,  0x1,  0x1,  0x0,  0x7,     0x1,     0x0 ),
    BVDC_P_MAKE_DCR(0x4, 0x3, 0x2, 0x2, 0x8,     0x3,  0x2,  0x2,  0x1,  0xF,     0x2,     0x1 ),
    BVDC_P_MAKE_DCR(0x6, 0x5, 0x4, 0x4, 0xA,     0x4,  0x3,  0x3,  0x2,  0xF,     0x3,     0x1 ),
    BVDC_P_MAKE_DCR(0x8, 0x7, 0x6, 0x5, 0xC,     0x5,  0x4,  0x4,  0x3,  0xF,     0x4,     0x1 ),
    BVDC_P_MAKE_DCR(0xA, 0x9, 0x8, 0x7, 0xE,     0x6,  0x5,  0x5,  0x4,  0x1F,    0x6,     0x2 ),
};

#if 0
/* Static lookup table to figure out configuruation of MNR
 * INDEX: Qp value. */
static const BVDC_P_MnrCfgEntry s_aMnrCfgTbl[] =
{
    /* Spot, Merge, Rel, Limit */
    {  0,   0,    0,     0,   0},
};

/* Static lookup table to figure out configuruation of BNR
 * INDEX: Qp value. */
static const BVDC_P_BnrCfgEntry s_aBnrCfgTbl[] =
{
    /* SmallGrid, LrLimit, Rel, Limit */
    {  0,    0,    0,         2,       0,   0 }
};
#endif


/***************************************************************************
 * {private}
 *
 * BVDC_P_Dnr_GetDcrCfg_isr
 *
 * called by BVDC_P_Dnr_SetInfo_isr to look-up Dcr configurations from
 * Dcr level, at every vsync when RUL is built.
 *
 * This subroutine could be replaced by customer to customize Dcr
 * configuration.
 *
 * pFmtInfo represents source video format that might change dynamically
 * and get detected automatically by VDC. It could be NULL if MPEG input
 * source is used. pFmtInfo might be used for customer to fine tune the Dcr
 * values.
 *
 * pvUserInfo is a pointer to a user defined struct. It is passed to VDC as
 * a member of BVDC_Dnr_Settings from app with BVDC_Source_SetDnrConfiguration.
 * It could be used to pass any information that customer want to use to
 * decide the Dcr values.
 */
const BVDC_P_DcrCfgEntry* BVDC_P_Dnr_GetDcrCfg_isr
    ( int32_t                        iDcrLevel,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo )
{
    uint32_t index = 0;
    const BVDC_P_DcrCfgEntry *pDcrCfg;

    /* Mapping from ulDcrQp into index to s_aDcrCfgTbl[] */
    index = (iDcrLevel <= -60) ? 0 :
            (iDcrLevel <= 20)  ? 1 :
            (iDcrLevel <= 100) ? 2 :
            (iDcrLevel <= 180) ? 3 : 4;

    BDBG_MSG(("iDcrLevel = %d, Dcr table index %d", iDcrLevel, index));
    pDcrCfg = &s_aDcrCfgTbl[index];

    BSTD_UNUSED(pFmtInfo);
    BSTD_UNUSED(pvUserInfo);

    return pDcrCfg;
}


/***************************************************************************
 * {private}
 *
 * BVDC_P_Dnr_GetBnrCfg_isr
 *
 * called by BVDC_P_Dnr_SetInfo_isr to look-up Bnr configurations from
 * Qp value, at every vsync when RUL is built.
 *
 * This subroutine could be replaced by customer to customize Bnr
 * configuration.
 *
 * eSrcOrigPolarity is the source polarity which is used to calculate
 * internal Bnr Small Grid value.
 *
 * pFmtInfo represents source video format that might change dynamically
 * and get detected automatically by VDC. It could be NULL if MPEG input
 * source is used. pFmtInfo might be used for customer to fine tune the Bnr
 * values.
 *
 * pvUserInfo is a pointer to a user defined struct. It is passed to VDC as
 * a member of BVDC_Dnr_Settings from app with BVDC_Source_SetDnrConfiguration.
 * It could be used to pass any information that customer want to use to
 * decide the Bnr values.
 */
const BVDC_P_BnrCfgEntry* BVDC_P_Dnr_GetBnrCfg_isr
    ( uint32_t                       ulBnrQp,
      BAVC_Polarity                  eSrcOrigPolarity,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo )
{
    BSTD_UNUSED(ulBnrQp);
    BSTD_UNUSED(eSrcOrigPolarity);
    BSTD_UNUSED(pFmtInfo);
    BSTD_UNUSED(pvUserInfo);

    return NULL;
}


/***************************************************************************
 * {private}
 *
 * BVDC_P_Dnr_GetMnrCfg_isr
 *
 * called by BVDC_P_Dnr_SetInfo_isr to look-up Mnr configurations from
 * Qp value, at every vsync when RUL is built.
 *
 * This subroutine could be replaced by customer to customize Mnr
 * configuration.
 *
 * ulSrcHSize is the source width that used to calculate internal Mnr
 * Merge value.
 *
 * pFmtInfo represents source video format that might change dynamically
 * and get detected automatically by VDC. It could be NULL if MPEG input
 * source is used. pFmtInfo might be used for customer to fine tune the Mnr
 * values.
 *
 * pvUserInfo is a pointer to a user defined struct. It is passed to VDC as
 * a member of BVDC_Dnr_Settings from app with BVDC_Source_SetDnrConfiguration.
 * It could be used to pass any information that customer want to use to
 * decide the Mnr values.
 */
const BVDC_P_MnrCfgEntry* BVDC_P_Dnr_GetMnrCfg_isr
    ( uint32_t                       ulMnrQp,
      uint32_t                       ulSrcHSize,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo )
{
    BSTD_UNUSED(ulMnrQp);
    BSTD_UNUSED(ulSrcHSize);
    BSTD_UNUSED(pFmtInfo);
    BSTD_UNUSED(pvUserInfo);

    return NULL;
}

#else
const BVDC_P_DcrCfgEntry* BVDC_P_Dnr_GetDcrCfg_isr
    ( int32_t                        iDcrLevel,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo )
{
    BSTD_UNUSED(iDcrLevel);
    BSTD_UNUSED(pFmtInfo);
    BSTD_UNUSED(pvUserInfo);
    return NULL;
}

const BVDC_P_BnrCfgEntry* BVDC_P_Dnr_GetBnrCfg_isr
    ( uint32_t                       ulBnrQp,
      BAVC_Polarity                  eSrcOrigPolarity,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo )
{
    BSTD_UNUSED(ulBnrQp);
    BSTD_UNUSED(eSrcOrigPolarity);
    BSTD_UNUSED(pFmtInfo);
    BSTD_UNUSED(pvUserInfo);
    return NULL;
}

const BVDC_P_MnrCfgEntry* BVDC_P_Dnr_GetMnrCfg_isr
    ( uint32_t                       ulMnrQp,
      uint32_t                       ulSrcHSize,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo )
{
    BSTD_UNUSED(ulMnrQp);
    BSTD_UNUSED(ulSrcHSize);
    BSTD_UNUSED(pFmtInfo);
    BSTD_UNUSED(pvUserInfo);
    return NULL;
}
#endif /* BVDC_P_SUPPORT_DNR */

/* End of file. */
