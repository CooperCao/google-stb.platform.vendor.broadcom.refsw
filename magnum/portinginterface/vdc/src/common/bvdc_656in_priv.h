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
#ifndef BVDC_656IN_PRIV_H__
#define BVDC_656IN_PRIV_H__

#include "bvdc.h"
#include "bvdc_common_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Defines
 ***************************************************************************/
/* 7038x-ish, 3560x-ish, 7401Ax, 7118, 7440 */
#define BVDC_P_656IN_NEW_VER_0                   (0)
/* 7401Bx:
 * Use EXT_656_TOP_0_ext_656_count */
#define BVDC_P_656IN_NEW_VER_1                   (1)
/* 3563, 7405:
 * Use EXT_656_TOP_0_ext_656_count.
 * No  EXT_656_TOP_0_ext_656_reset */
#define BVDC_P_656IN_NEW_VER_2                   (2)
/* 7400-ish:
 * Use EXT_656_TOP_0_ext_656_short_count & EXT_656_TOP_0_ext_656_long_count */
#define BVDC_P_656IN_NEW_VER_3                   (3)

#if (BVDC_P_SUPPORT_NEW_656_IN_VER == BVDC_P_656IN_NEW_VER_3)
#define BVDC_P_NUM_656IN_SUPPORT                 (2)
#else
#define BVDC_P_NUM_656IN_SUPPORT                 (1)
#endif

/* Trigger offset from the picture height. */
#define BVDC_P_656IN_TRIGGER_OFFSET              (6)


/***************************************************************************
 * Private macros
 ***************************************************************************/

/***************************************************************************
 * Private enums
 ***************************************************************************/
typedef enum BVDC_P_656Id
{
    BVDC_P_656Id_e656In0 = 0,
    BVDC_P_656Id_e656In1
} BVDC_P_656Id;

/***************************************************************************
 * 656 Context
 ***************************************************************************/
typedef struct BVDC_P_656InContext
{
    BDBG_OBJECT(BVDC_656)

    BVDC_Source_Handle             hSource;
    BVDC_P_656Id                   eId;
    uint32_t                       ulOffset;

    bool                           bVideoDetected;

    /* 656 output slow start countdown to avoid prematurely output garbage */
    uint32_t                       ulDelayStart;

    /* 656 frame rate code */
    BAVC_FrameRateCode             eFrameRateCode;

} BVDC_P_656InContext;


/***************************************************************************
 * Private function prototypes
 ***************************************************************************/
BERR_Code BVDC_P_656In_Create
    ( BVDC_P_656In_Handle             *ph656In,
      BVDC_P_656Id                     e656Id,
      BVDC_Source_Handle               hSource );

void BVDC_P_656In_Destroy
    ( BVDC_P_656In_Handle              h656In );

void BVDC_P_656In_Init
    ( BVDC_P_656In_Handle              h656In );

void BVDC_P_656In_UpdateStatus_isr
    ( BVDC_P_656In_Handle              h656In );

void BVDC_P_656In_GetStatus_isr
    ( const BVDC_P_656In_Handle        h656In,
      bool                            *pbVideoDetected );

void BVDC_P_656In_Bringup_isr
    ( BVDC_P_656In_Handle              h656In );

void BVDC_P_656In_BuildRul_isr
    ( const BVDC_P_656In_Handle        h656In,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldId );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_656IN_PRIV_H__ */
/* End of file. */
