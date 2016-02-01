/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

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
#define BRPC_CallProc(a, b, c, d, e, f, g) 0xdead
#endif

BDBG_MODULE(bmxt_common);

#define VIRTUAL_HANDLE_REG_OFFSET 0x80000000 /* hard-coded for now */
uint32_t BMXT_RegAddrVirtual(BMXT_Handle handle, uint32_t addr)
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
