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
 ******************************************************************************/

#include "bstd.h"
#include "berr.h"
#include "bkni.h"
#include "bchp_priv.h"
#include "bchp_scpu_host_intr2.h"
#include "bchp_scpu_globalram.h"
#include "bchp_common.h"
#include "bsagelib_types.h"
#if SAGE_VERSION >= SAGE_VERSION_CALC(3,0)
#include "priv/bsagelib_shared_globalsram.h"
#endif
#include "priv/bsagelib_shared_types.h"
#include "bchp_bsp_glb_control.h"

#if !defined(BCHP_SCPU_GLOBALRAM_REG_START)
#error Cannot build with SAGE support without globalram registers
#endif

BDBG_MODULE(BCHP_SAGE);

#define SAGE_MAX_BOOT_TIME_US (10 * 1000 * 1000)
#define SAGE_STEP_BOOT_TIME_US (50 * 1000)

#define SAGE_RESET_REG BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eReset)
#define SAGE_SRR_START_REG BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eSRRStartOffset)
#define SAGE_CRR_START_REG BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eCRRStartOffset)
#define SAGE_BOOT_STATUS_REG BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBootStatus)

#define SAGE_RESET_WAIT_COUNT 1000
#define SAGE_RESET_WAIT_DELAY 10

bool BCHP_SAGE_HasEverStarted(BREG_Handle hReg)
{
    return (BREG_Read32(hReg, SAGE_CRR_START_REG) != 0x0);
}

uint32_t BCHP_SAGE_GetStatus(BREG_Handle hReg)
{
    uint32_t sage_status = BSAGElibBootStatus_eNotStarted;

    if((BREG_Read32(hReg, BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT)
            & BCHP_BSP_GLB_CONTROL_SCPU_SW_INIT_SCPU_SW_INIT_MASK) == 0)
    {
        uint32_t val;
        uint32_t totalBootTimeUs = 0;

        /* SAGE is started, SAGE Bootloader or SAGE Framework is up */
        do
        {
            val = BREG_Read32(hReg, SAGE_BOOT_STATUS_REG);
            BDBG_MSG(("%s %d SAGE_BOOT_STATUS_REG %d",BSTD_FUNCTION,__LINE__,val));
            if(val == BSAGElibBootStatus_eBlStarted
             ||val == BSAGElibBootStatus_eNotStarted
             ||val == BSAGElibBootStatus_eError)
            {
                /* Need to wait till SBL/SSF success or failure or timeout */
                BKNI_Sleep(SAGE_STEP_BOOT_TIME_US/1000); /* BKNI_Sleep() takes ms; convert from us to ms */
                totalBootTimeUs += SAGE_STEP_BOOT_TIME_US;
                if (totalBootTimeUs > SAGE_MAX_BOOT_TIME_US) {
                    BDBG_WRN(("SAGE takes too long to boot, it might be stuck. Keep going..."));
                    break;
                }
            }else if(val == BSAGElibBootStatus_eStarted)
            {
                BDBG_MSG(("SAGE SSF up sucessfuly"));
                break;
            }else if(val == BSAGElibBootStatus_ePolling)
            {
                BDBG_MSG(("SAGE SBL up, waiting to boot SSF"));
                break;
            }else
            {
                BDBG_WRN(("SAGE status 0x%x, May be stuck in an error. Keep going...",val));
                break;
            }
        } while (1);
        sage_status = val;
    }
    BDBG_MSG(("%s %d sage_status 0x%x",BSTD_FUNCTION,__LINE__,sage_status));
    return sage_status;
}

BERR_Code
BCHP_SAGE_Reset(
    BREG_Handle hReg)
{
    uint32_t val;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BCHP_SAGE_Reset);

    BDBG_ASSERT(hReg);

    if (BCHP_SAGE_GetStatus(hReg) != BSAGElibBootStatus_eStarted) {
        /* BSAGElibBootStatus_eNotStarted: SAGE already in reset, don't need to reset */
        /* BSAGElibBootStatus_ePolling:    SBL running, can not reset */
        /* BSAGElibBootStatus_eStarted:    SSF running, send command to reset */
        BDBG_MSG(("SAGE SSF is not started. Continue...."));
        goto end;
    }

    BDBG_WRN(("Waiting for SAGE to complete boot"));

    val = BREG_Read32(hReg, SAGE_RESET_REG);

    if(val == SAGE_RESETVAL_S2H_ERROR)
    {
        /* Error in a previous cleanup attempt */
        BDBG_ERR(("Previous SAGE clean failed! All TA's must be clean/closed."));
        rc=BERR_TRACE(BERR_LEAKED_RESOURCE);
        goto end;
    }

    if (val == SAGE_RESETVAL_S2H_READYTORESTART)
    {
        uint32_t cpt = 0;

        BDBG_MSG(("Request to reset SAGE..."));

        /* clear any pending watchdog interrupt */
        BREG_Write32(hReg, BCHP_SCPU_HOST_INTR2_CPU_CLEAR, 1 << BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_TIMER_SHIFT);

        /* request a reset */
        BREG_Write32(hReg, SAGE_RESET_REG, SAGE_RESETVAL_H2S_RESET);
        do {
            uint32_t wdval;
            cpt++;
            BKNI_Sleep(SAGE_RESET_WAIT_DELAY);
            wdval = BREG_Read32(hReg, BCHP_SCPU_HOST_INTR2_CPU_STATUS);
            if (wdval & (1 << BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_TIMER_SHIFT)) {
                BDBG_MSG(("SAGE has been reset successfully!"));
                break;
            }
            val=BREG_Read32(hReg, SAGE_RESET_REG);

            if(val == SAGE_RESETVAL_S2H_DOWN)
            {
                /* SAGE has indicated it's done, but BSP may not have powered off yet...
                 * wait for SAGE to be put in reset */
                if(BCHP_SAGE_GetStatus(hReg) == BSAGElibBootStatus_eStarted)
                {
                    continue;
                }
                BDBG_MSG(("SAGE shutdown successfully"));
                break;
            }
            if(val == SAGE_RESETVAL_S2H_READYTORESTART)
            {
                BDBG_MSG(("SAGE has cleaned succesfully!"));
                break;
            }
            if(val == SAGE_RESETVAL_S2H_ERROR)
            {
                BDBG_ERR(("SAGE CANNOT CLEAN... All TA's must be clean/closed."));
                rc=BERR_TRACE(BERR_LEAKED_RESOURCE);
                break;
            }
        } while (cpt < SAGE_RESET_WAIT_COUNT);

            if (cpt >= SAGE_RESET_WAIT_COUNT) {
                BDBG_WRN(("Cannot reset SAGE."));
            }
    }
    else {
        BDBG_MSG(("SAGE is not started (but was #0x%x). Continue....", val));
    }

end:
    BDBG_LEAVE(BCHP_SAGE_Reset);
    return rc;
}
