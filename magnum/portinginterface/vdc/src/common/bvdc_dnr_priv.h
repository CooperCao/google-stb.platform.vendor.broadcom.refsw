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
#ifndef BVDC_DNR_PRIV_H__
#define BVDC_DNR_PRIV_H__

#include "bvdc.h"
#include "bavc.h"
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"

#if BVDC_P_SUPPORT_DNR
#include "bchp_dnr_0.h"
#if (BVDC_P_SUPPORT_DNR == 2)
#include "bchp_dnr_1.h"
#endif
#if (BVDC_P_SUPPORT_DNR == 3)
#include "bchp_dnr_2.h"
#endif
#if (BVDC_P_SUPPORT_DNR == 4)
#include "bchp_dnr_3.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Private register cracking macros
 ***************************************************************************/
#if BVDC_P_SUPPORT_DNR
#define BVDC_P_DNR_H_MAX_RANGE        (200)
#define BVDC_P_DNR_MAX_HW_QP_STEPS    (100)


#define BVDC_P_Dnr_PostMuxValue(hDnr)     ((hDnr)->ulVnetMuxValue)

#define BVDC_P_DNR_GET_REG_IDX(reg) \
    ((BCHP##_##reg - BCHP_DNR_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_DNR_GET_REG_DATA(reg) \
    (hDnr->aulRegs[BVDC_P_DNR_GET_REG_IDX(reg)])
#define BVDC_P_DNR_SET_REG_DATA(reg, data) \
    (BVDC_P_DNR_GET_REG_DATA(reg) = (uint32_t)(data))

/* Get with index. */
#define BVDC_P_DNR_GET_REG_DATA_I(reg, idx) \
    (hDnr->aulRegs[BVDC_P_DNR_GET_REG_IDX(reg) + (idx)])

/* Get field */
#define BVDC_P_DNR_GET_FIELD_NAME(reg, field) \
    (BVDC_P_GET_FIELD(BVDC_P_DNR_GET_REG_DATA(reg), reg, field))

/* Compare field */
#define BVDC_P_DNR_COMPARE_FIELD_DATA(reg, field, data) \
    (BVDC_P_COMPARE_FIELD_DATA(BVDC_P_DNR_GET_REG_DATA(reg), reg, field, (data)))
#define BVDC_P_DNR_COMPARE_FIELD_NAME(reg, field, name) \
    (BVDC_P_COMPARE_FIELD_NAME(BVDC_P_DNR_GET_REG_DATA(reg), reg, field, name))

/* This macro does a write into RUL (write, addr, data). 3 dwords. */
#define BVDC_P_DNR_WRITE_TO_RUL(reg, addr_ptr) \
{ \
    *addr_ptr++ = BRDC_OP_IMM_TO_REG(); \
    *addr_ptr++ = BRDC_REGISTER(BCHP##_##reg + hDnr->ulRegOffset); \
    *addr_ptr++ = BVDC_P_DNR_GET_REG_DATA(reg); \
}

/* number of registers in one block. */
#define BVDC_P_DNR_REGS_COUNT    \
    BVDC_P_REGS_ENTRIES(DNR_0_REG_START, DNR_0_REG_END)
#else
#define BVDC_P_DNR_REGS_COUNT   1
#endif

/* Miscellaneous constants */
#define BVDC_P_Dnr_LAST UINT32_C(-1)

/* [lo,hi] */
#define BVDC_P_DNR_CLAMP(low, hi, x) (((int)(x) <= (int)(low)) ? (low) : (((int)(x) > (int)(hi)) ? (hi) : (x)))

#define BVDC_P_Dnr_SetVnet_isr(hDnr, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((hDnr)->SubRul), ulSrcMuxValue, eVnetPatchMode)

#define BVDC_P_Dnr_UnsetVnet_isr(hDnr) \
  BVDC_P_SubRul_UnsetVnet_isr(&((hDnr)->SubRul))

#define BVDC_P_Dnr_SetRulBuildWinId_isr(hDnr, eWinId) \
    BVDC_P_SubRul_SetRulBuildWinId_isr(&((hDnr)->SubRul), eWinId)

/***************************************************************************
 * DNR private data structures
 ***************************************************************************/
typedef struct
{
    uint32_t                        ulFilt0Limit;
    uint32_t                        ulFilt1Limit;
    uint32_t                        ulFilt2Limit;
    uint32_t                        ulFilt3Limit;
    uint32_t                        ulFiltClamp;
    uint32_t                        ulRandomA;
    uint32_t                        ulRandomB;
    uint32_t                        ulRandomC;
    uint32_t                        ulRandomD;
    uint32_t                        ulDithClamp;
    uint32_t                        ulOrderA;
    uint32_t                        ulOrderB;
} BVDC_P_DcrCfgEntry;

typedef struct
{
    uint32_t                        ulHBnr;      /* DNR_0.BNR_CTRL.HBNR_ENABLE */
    uint32_t                        ulVBnr;      /* DNR_0.BNR_CTRL.VBNR_ENABLE */
    uint32_t                        ulSmallGrid; /* DNR_0.BNR_CONFIG.HBNR_SMALL_GRID */
    uint32_t                        ulLrLimit;   /* DNR_0.BNR_CONFIG.VBNR_LR_LIMIT */
    uint32_t                        ulHRel;       /* DNR_0.HBNR_CONFIG.HBNR_REL */
    uint32_t                        ulHLimit;     /* DNR_0.HBNR_CONFIG.HBNR_LIMIT */
    uint32_t                        ulVRel;       /* DNR_0.VBNR_CONFIG.VBNR_REL */
    uint32_t                        ulVLimit;     /* DNR_0.VBNR_CONFIG.VBNR_LIMIT */
} BVDC_P_BnrCfgEntry;

typedef struct
{
    uint32_t                        ulMnr;       /* DNR_0.MNR_CTRL.MNR_ENABLE */
    uint32_t                        ulSpot;      /* DNR_0.MNR_CONFIG.MNR_SPOT */
    uint32_t                        ulMerge;     /* DNR_0.MNR_CONFIG.MNR_MERGE */
    uint32_t                        ulRel;       /* DNR_0.MNR_CONFIG.MNR_REL */
    uint32_t                        ulLimit;     /* DNR_0.MNR_CONFIG.MNR_LIMIT */
} BVDC_P_MnrCfgEntry;

typedef struct BVDC_P_DnrContext
{
    BDBG_OBJECT(BVDC_DNR)

    BVDC_Source_Handle             hSource;

    /* flag initial state, require reset; */
    bool                           bInitial;
    uint32_t                       ulResetRegAddr;
    uint32_t                       ulResetMask;
    uint32_t                       ulVnetResetAddr;
    uint32_t                       ulVnetResetMask;
    uint32_t                       ulVnetMuxAddr;
    uint32_t                       ulVnetMuxValue;


    /* flag for changes */
    uint32_t                       ulUpdateAll;

    /* adj_Qp */
    uint32_t                       ulMnrQp;
    uint32_t                       ulBnrQp;
    int32_t                        iDcrLevel;

    BVDC_P_BnrCfgEntry             stBnrCfg;
    BVDC_P_MnrCfgEntry             stMnrCfg;
    BVDC_P_DcrCfgEntry             stDcrCfg;

    uint32_t                       ulFilterLimit;
    BFMT_Orientation               eOrientation;

    /* private fields. */
    BVDC_P_DnrId                   eId;
    uint32_t                       ulRegOffset; /* in case of multiple instances */
    uint32_t                       aulRegs[BVDC_P_DNR_REGS_COUNT];

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext           SubRul;

    bool                           bDnrH;
    bool                           b10BitMode;
    BVDC_FilterMode                eDcrMode;
} BVDC_P_DnrContext;


/***************************************************************************
 * DNR private functions
 ***************************************************************************/
BERR_Code BVDC_P_Dnr_Create
    ( BVDC_P_Dnr_Handle            *phDnr,
      BVDC_P_DnrId                  eDnrId,
      BREG_Handle                   hRegister,
      BVDC_P_Resource_Handle        hResource );

void BVDC_P_Dnr_Destroy
    ( BVDC_P_Dnr_Handle             hDnr );

void BVDC_P_Dnr_Init_isr
    ( BVDC_P_Dnr_Handle             hDnr );

BERR_Code BVDC_P_Dnr_AcquireConnect_isr
    ( BVDC_P_Dnr_Handle             hDnr,
      BVDC_Source_Handle            hSource );

BERR_Code BVDC_P_Dnr_ReleaseConnect_isr
    ( BVDC_P_Dnr_Handle            *phDnr );

void BVDC_P_Dnr_BuildRul_isr
    ( const BVDC_P_Dnr_Handle       hDnr,
      BVDC_P_ListInfo              *pList,
      BVDC_P_State                  eVnetState,
      BVDC_P_PicComRulInfo         *pPicComRulInfo );

BERR_Code BVDC_P_Dnr_SetInfo_isr
    ( BVDC_P_Dnr_Handle             hDnr,
      const BVDC_P_PictureNodePtr   pPicture );

BERR_Code BVDC_P_Dnr_SetEnable_isr
    ( BVDC_P_Dnr_Handle             hDnr,
      bool                          bEnable );

const BVDC_P_DcrCfgEntry* BVDC_P_Dnr_GetDcrCfg_isr
    ( int32_t                        iDcrLevel,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo );

const BVDC_P_BnrCfgEntry* BVDC_P_Dnr_GetBnrCfg_isr
    ( uint32_t                       ulBnrQp,
      BAVC_Polarity                  eSrcOrigPolarity,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo );

const BVDC_P_MnrCfgEntry* BVDC_P_Dnr_GetMnrCfg_isr
    ( uint32_t                       ulMnrQp,
      uint32_t                       ulSrcHSize,
      const BFMT_VideoInfo          *pFmtInfo,
      void                          *pvUserInfo );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_DNR_PRIV_H__ */
/* End of file. */
