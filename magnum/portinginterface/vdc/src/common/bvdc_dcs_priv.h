/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *   Header file for DCS support
 *
 ***************************************************************************/

#ifndef BVDC_DCS_PRIV_H__
#define BVDC_DCS_PRIV_H__

#include "bvdc_dcs.h"
#include "bvdc_display_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BVDC_P_SMOOTH_DCS

/* These values refer to microcontroller programs in register array
 * IT_n_MICRO_INSTRUCTION */
#define BVDC_P_DCS_MC0_ON (0)
#define BVDC_P_DCS_MC0_OFF (0) /* NTSC ONLY */
#define BVDC_P_DCS_MC1_ON (20)
#define BVDC_P_DCS_MC1_OFF (20) /* NTSC ONLY */
#define BVDC_P_DCS_MC2 (90)
#define BVDC_P_DCS_MC3 (160)
#define BVDC_P_DCS_MC6 (170)
#define BVDC_P_DCS_SELFLOOP (89)
#define BVDC_P_DCS_SEGSTART BVDC_P_DCS_MC2
#define BVDC_P_DCS_SEGLEN (256 - BVDC_P_DCS_SEGSTART)
#define BVDC_P_DCS_CONFIRMER (252)

#define BVDC_P_DCS_ON_ADDRA_NTSC                                        \
(                                                                       \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_3_START_ADDR, BVDC_P_DCS_MC3   ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_2_START_ADDR, BVDC_P_DCS_MC2   ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_1_START_ADDR, BVDC_P_DCS_MC1_ON) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_0_START_ADDR, BVDC_P_DCS_MC0_ON)   \
)
#define BVDC_P_DCS_ON_ADDRA_PAL BVDC_P_DCS_ON_ADDRA_NTSC

#define BVDC_P_DCS_ON_ADDRB_NTSC                                          \
(                                                                         \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_6_START_ADDR, BVDC_P_DCS_MC6     ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_5_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_4_START_ADDR, BVDC_P_DCS_SELFLOOP)   \
)
#define BVDC_P_DCS_ON_ADDRB_PAL BVDC_P_DCS_ON_ADDRB_NTSC

#define BVDC_P_DCS_OFF_ADDRA_NTSC                                         \
(                                                                         \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_3_START_ADDR, BVDC_P_DCS_MC3     ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_2_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_1_START_ADDR, BVDC_P_DCS_MC1_OFF ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_0_START_ADDR, BVDC_P_DCS_MC0_OFF )   \
)
/* Programming note: there is no MC_0 DCS_OFF microcode for PAL. */
#define BVDC_P_DCS_OFF_ADDRA_PAL                                          \
(                                                                         \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_3_START_ADDR, BVDC_P_DCS_MC3     ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_2_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_1_START_ADDR, BVDC_P_DCS_MC1_ON  ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_0_START_ADDR, BVDC_P_DCS_MC0_ON  )   \
)

#define BVDC_P_DCS_OFF_ADDRB_NTSC                                         \
(                                                                         \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_6_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_5_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_4_START_ADDR, BVDC_P_DCS_SELFLOOP)   \
)
#define BVDC_P_DCS_OFF_ADDRB_PAL BVDC_P_DCS_OFF_ADDRB_NTSC

#define BVDC_P_DCS_ON_ADDRA_PROGRSV                                       \
(                                                                         \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_3_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_2_START_ADDR, BVDC_P_DCS_MC2     ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_1_START_ADDR, BVDC_P_DCS_MC1_ON  ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_0_START_ADDR, BVDC_P_DCS_MC0_ON  )   \
)

#define BVDC_P_DCS_ON_ADDRB_PROGRSV                                       \
(                                                                         \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_6_START_ADDR, BVDC_P_DCS_MC6     ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_5_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_4_START_ADDR, BVDC_P_DCS_SELFLOOP)   \
)

#define BVDC_P_DCS_OFF_ADDRA_PROGRSV                                      \
(                                                                         \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_3_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_2_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_1_START_ADDR, BVDC_P_DCS_MC1_ON  ) | \
    BCHP_FIELD_DATA(IT_0_ADDR_0_3,MC_0_START_ADDR, BVDC_P_DCS_MC0_ON  )   \
)

#define BVDC_P_DCS_OFF_ADDRB_PROGRSV                                      \
(                                                                         \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_6_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_5_START_ADDR, BVDC_P_DCS_SELFLOOP) | \
    BCHP_FIELD_DATA(IT_0_ADDR_4_6,MC_4_START_ADDR, BVDC_P_DCS_SELFLOOP)   \
)

#define BVDC_P_DCS_PSV_OFFSET \
    ((BCHP_VF_0_POS_SYNC_VALUES - BCHP_VF_0_FORMAT_ADDER)/4)
#define BVDC_P_DCS_PSE_OFFSET \
    ((BCHP_VF_0_POS_SYNC_AMPLITUDE_EXTN - BCHP_VF_0_FORMAT_ADDER)/4)
#define BVDC_P_DCS_NSE_OFFSET \
    ((BCHP_VF_0_NEG_SYNC_AMPLITUDE_EXTN - BCHP_VF_0_FORMAT_ADDER)/4)
#define BVDC_P_DCS_ST0_OFFSET \
    ((BCHP_VF_0_SYNC_TRANS_0 - BCHP_VF_0_FORMAT_ADDER)/4)
typedef struct
{
    uint32_t PosSyncValues;
    uint32_t PosSyncExtn;
    uint32_t NegSyncExtn;
    uint32_t SyncTrans0;
}
BVDC_P_DCS_VFvalues;

#if BVDC_P_NUM_SHARED_VF
const BVDC_P_DCS_VFvalues* BVDC_P_DCS_GetVFvalues_isr
(
    BFMT_VideoFmt eVideoFmt,
    bool bCompositePresent,
    BVDC_DCS_Mode eDcsMode
);
void BVDC_P_DCS_VF_Update_isr(
    const BVDC_P_DCS_VFvalues* pVfValues,
    uint32_t                   ulVfOffset,
    uint32_t**                 ppulRul);
#endif

typedef enum
{
    BVDC_P_DCS_State_eUndefined = 0,    /* No DCS for current video format */
    BVDC_P_DCS_State_eExecuteTop,       /* Executing top half              */
    BVDC_P_DCS_State_eExecuteBot,       /* Executing bottom half           */
    BVDC_P_DCS_State_eGoingTop,         /* Transitioning to top half       */
    BVDC_P_DCS_State_eGoingBot          /* Transitioning to bottom half    */

} BVDC_P_DCS_State;

struct BVDC_P_DCS_UpdateInfo
{
    BVDC_DCS_Mode    eDcsModeLoaded;
    BVDC_P_DCS_State eDcsState;
    bool             bDcsSetLock;
};

void BVDC_P_DCS_StateInit_isr (BVDC_Display_Handle pDisplay);

void BVDC_P_DCS_StateFault_isr (
    BVDC_Display_Handle pDisplay);

#if BVDC_P_NUM_SHARED_VF
void BVDC_P_DCS_StateUpdate_isr (
    BVDC_Display_Handle pDisplay,
    BAVC_Polarity       eFieldPolarity,
    BVDC_P_ListInfo*    pList
);
#endif

void BVDC_P_DCS_Check_isr (
    BVDC_Display_Handle pDisplay,
    BAVC_Polarity       eFieldPolarity,
    int                 index,
    BVDC_P_ListInfo*    pList
);

const uint32_t* BVDC_P_DCS_P_GetItTable_isr
(
    BFMT_VideoFmt eVideoFmt,
    bool bCompositePresent,
    BVDC_DCS_Mode eDcsMode
);
const uint32_t* BVDC_P_DCS_P_GetRamTableMv_isr
(
    BFMT_VideoFmt eVideoFmt,
    BVDC_DCS_Mode eDcsMode
);
uint32_t BVDC_P_DCS_P_GetItConfigMv_isr
(
    BFMT_VideoFmt eVideoFmt,
    bool          bDcsOn
);
void BVDC_P_DCS_IT_ON_OFF_isr(
    uint32_t      ulItOffset,
    BFMT_VideoFmt eVideoFmt,
    bool          bDcsOn,
    uint32_t** ppulRul);
void BVDC_P_DCS_IT_Final_ON_OFF_isr(
    uint32_t      ulItOffset,
    BFMT_VideoFmt eVideoFmt,
    bool          bDcsOn,
    uint32_t** ppulRul);
void BVDC_P_DCS_Lock_isr (
    uint32_t   ulItOffset,
    int        mode,
    uint32_t** ppulRul);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_DCS_PRIV_H__ */
/* End of file. */
