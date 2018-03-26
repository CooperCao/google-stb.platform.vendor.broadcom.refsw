/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ***************************************************************************/
#ifndef BRDC__BLOCKOUT_PRIV_H__
#define BRDC__BLOCKOUT_PRIV_H__

#include "blst_list.h"           /* Link list support */
#include "brdc.h"
#include "brdc_dbg.h"

/* For RDC block out RUL */
#define BRDC_P_RDC_BLOCKOUT_RUL_MAX_ENTRY    0x4000

/* HW7445-1476, use SW blockout before the fixes in */
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7366) || (BCHP_CHIP==74371) || \
    ((BCHP_CHIP==7439) && (BCHP_VER == BCHP_VER_A0))|| \
    ((BCHP_CHIP==7364) && (BCHP_VER < BCHP_VER_C0))|| \
    (BCHP_CHIP==7250)|| (BCHP_CHIP==7563)|| (BCHP_CHIP==7543)
#define BRDC_P_SUPPORT_HW_BLOCKOUT_WORKAROUND              (1)
#else
#define BRDC_P_SUPPORT_HW_BLOCKOUT_WORKAROUND              (0)
#endif

/* Support register udpate blockout by HW? */
#if (BCHP_RDC_br_0_start_addr && (!BRDC_P_SUPPORT_HW_BLOCKOUT_WORKAROUND))
#define BRDC_P_SUPPORT_HW_BLOCKOUT         (1)
#else
#define BRDC_P_SUPPORT_HW_BLOCKOUT         (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

BERR_Code BRDC_P_RdcBlockOutInit
    ( BRDC_Handle           hRdc );

BERR_Code BRDC_P_RdcBlockOutDestroy
    ( BRDC_Handle           hRdc );

#if !B_REFSW_MINIMAL
BERR_Code BRDC_P_ValidateBlockOutRegisters
    ( const BRDC_BlockOut *pstBlockOut,
      uint32_t             ulNumRegBlocks );
#endif

#if(!BRDC_P_SUPPORT_HW_BLOCKOUT)
bool BRDC_P_IsRdcBlockOutEnabled_isr
    ( BRDC_Handle           hRdc );

BERR_Code BRDC_P_ParseAndReplaceRul_isr
    ( BRDC_List_Handle hList);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BRDC__BLOCKOUT_PRIV_H__ */


/* end of file */
