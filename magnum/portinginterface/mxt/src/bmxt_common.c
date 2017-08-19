/******************************************************************************
 *  Copyright (C) 2016-2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "bstd.h"
#include "bchp.h"
#include "bkni.h"
#include "bmxt_priv.h"
#include "bmxt.h"

#include "bhab.h"
#include "brpc.h" /* for BRPC_CallProc() */
#include "brpc_docsis.h" /* for BRPC_Param_ECM_ReadXptBlock / WriteXptBlock */

#define BMXT_RPC_READXPT_CMD  68 /* BRPC_ProcId_ECM_ReadXptBlock */
#define BMXT_RPC_WRITEXPT_CMD 69 /* BRPC_ProcId_ECM_WriteXptBlock */

#if BMXT_NO_RPC
#define BRPC_CallProc(a, b, c, d, e, f, g) 0xdead;(void)c
#endif

BDBG_MODULE(bmxt_common);

#define VIRTUAL_HANDLE_REG_OFFSET 0x80000000 /* hard-coded for now */
static uint32_t BMXT_RegAddrVirtual(BMXT_Handle handle, uint32_t addr)
{
    uint32_t regbaseSlave = handle->platform.regbase;
    uint32_t regbaseMaster = handle->settings.hVirtual->platform.regbase;

    addr -= regbaseSlave + regbaseMaster; /* virtual handle must use master device's regbase (i.e. regoffset is relative from DEMOD_XPT_FE, not absolute) */
    addr += VIRTUAL_HANDLE_REG_OFFSET;
    return addr;
}

uint32_t BMXT_RegRead32_common(BMXT_Handle handle, uint32_t addr)
{
    BERR_Code rc;

    /* if virtual handle, then use master device's chip to determine reg R/W method */
    if (handle->settings.hVirtual) {
        addr = BMXT_RegAddrVirtual(handle, addr);
        handle = handle->settings.hVirtual;
    }

    if (handle->platform.type==BMXT_P_PlatformType_eHab) {
        BHAB_Handle hab = handle->hHab;
        uint32_t val;
        rc = BHAB_ReadRegister(hab, addr, &val);
        if (rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
            return 0;
        }
        return val;
    }
    else if (handle->platform.type==BMXT_P_PlatformType_eRpc) {
        BRPC_Param_ECM_ReadXptBlock block;
        BERR_Code rcR = BERR_SUCCESS;
        BDBG_ASSERT(handle->platform.regbase==0);
        block.startRegisterAddress = addr; /* addresses are offset-based (i.e. 0-based) and inclusive */
        block.endRegisterAddress = addr;
        rc = BRPC_CallProc(handle->hRpc, BMXT_RPC_READXPT_CMD,
            (const uint32_t *)&block, sizeof(block)/sizeof(uint32_t),
            (uint32_t *)&block, sizeof(block)/sizeof(uint32_t), &rcR);
        /* coverity[dead_error_condition] */
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); return 0; }
        if (rcR!=BERR_SUCCESS) { rc = BERR_TRACE(rcR); return 0; } /* propagate the failure */
        return block.returnRegisterValues[0];
    }
    else {
        BREG_Handle hReg = handle->hReg;
        BSTD_UNUSED(rc);
        return BREG_Read32(hReg, addr);
    }

    return 0;
}

void BMXT_RegWrite32_common(BMXT_Handle handle, uint32_t addr, uint32_t data)
{
    BERR_Code rc;

    /* if virtual handle, then use master device's chip to determine reg R/W method */
    if (handle->settings.hVirtual) {
        addr = BMXT_RegAddrVirtual(handle, addr);
        handle = handle->settings.hVirtual;
    }

    if (handle->platform.type==BMXT_P_PlatformType_eHab) {
        BHAB_Handle hab = handle->hHab;
        rc = BHAB_WriteRegister(hab, addr, &data);
        if (rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
        }
    }
    else if (handle->platform.type==BMXT_P_PlatformType_eRpc) {
        BRPC_Param_ECM_WriteXptBlock block;
        BERR_Code rcR = BERR_SUCCESS;
        BDBG_ASSERT(handle->platform.regbase==0);
        block.startRegisterAddress = addr; /* addresses are offset-based (i.e. 0-based) and inclusive */
        block.endRegisterAddress = addr;
        block.writeRegisterValues[0] = data;
        rc = BRPC_CallProc(handle->hRpc, BMXT_RPC_WRITEXPT_CMD,
            (const uint32_t *)&block, sizeof(block)/sizeof(uint32_t),
            NULL, 0, &rcR);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
        if (rcR!=BERR_SUCCESS) { rc = BERR_TRACE(rcR); } /* propagate the failure */
    }
    else {
        BREG_Handle hReg = handle->hReg;
        BSTD_UNUSED(rc);
        BREG_Write32(hReg, addr, data);
    }

    return;
}
