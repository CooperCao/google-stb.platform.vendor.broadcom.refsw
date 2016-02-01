/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "berr.h"
#include "bkni.h"
#include "bchp_scpu_host_intr2.h"
#include "bchp_scpu_globalram.h"

BDBG_MODULE(BCHP_SAGE);

#define SAGE_BOOT_STATUS_OFFSET 0x120
#define SAGE_BOOT_STATUS_REG (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + SAGE_BOOT_STATUS_OFFSET)

#define SAGE_BOOT_ERROR          (0xFFFF)
#define SAGE_BOOT_NOT_STARTED    (0x00FF)
#define SAGE_BOOT_BL_SUCCESS     (0x0001)
#define SAGE_BOOT_SUCCESS        (0x0000)

#define SAGE_MAX_BOOT_TIME_US (10 * 1000 * 1000)
#define SAGE_STEP_BOOT_TIME_US (50 * 1000)

#define SAGE_RESET_OFFSET 0x1a4
#define SAGE_SRR_START_OFFSET 0x18
#define SAGE_RESET_REG (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + SAGE_RESET_OFFSET)
#define SAGE_SRR_START_REG (BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + SAGE_SRR_START_OFFSET)

#define SAGE_RESETVAL_RESET 0x4E404E40
#define SAGE_RESETVAL_READYTORESTART 0x0112E00E
#define SAGE_RESETVAL_MAINSTART 0x0211DCB0
#define SAGE_RESETVAL_DOWN  0x1FF1FEED

BERR_Code
BCHP_SAGE_Reset(
    BREG_Handle hReg)
{
    uint32_t val;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BCHP_SAGE_Reset);

    BDBG_ASSERT(hReg);

    val = BREG_Read32(hReg, SAGE_SRR_START_REG);
    if (val == 0x0) {
        BDBG_MSG(("SAGE is not started. Continue...."));
        goto end;
    }

    BDBG_WRN(("Waiting for SAGE to complete boot"));

    {
        uint32_t totalBootTimeUs = 0;
        do {
            val = BREG_Read32(hReg, SAGE_BOOT_STATUS_REG);
            if ((val == SAGE_BOOT_BL_SUCCESS) ||
                (val == SAGE_BOOT_NOT_STARTED) ||
                (val == SAGE_BOOT_ERROR)) {
                /* Need to wait till OS/App success or failure or timeout */
                BKNI_Sleep(SAGE_STEP_BOOT_TIME_US/1000); /* BKNI_Sleep() takes ms; convert from us to ms */
                totalBootTimeUs += SAGE_STEP_BOOT_TIME_US;
                if (totalBootTimeUs > SAGE_MAX_BOOT_TIME_US) {
                    BDBG_WRN(("SAGE takes too long to boot, it might be stuck. Keep going..."));
                    break;
                }
            }
            else {
                if (val != SAGE_BOOT_SUCCESS) {
                    BDBG_WRN(("SAGE May be stuck in an error. Keep going..."));
                }
                break;
            }
        /* exit the loop, move to restart check logic */
        } while (1);
    }

    val = BREG_Read32(hReg, SAGE_RESET_REG);

    if (val == SAGE_RESETVAL_READYTORESTART)
    {
        uint32_t cpt = 0;

        BDBG_MSG(("Request to reset SAGE..."));

        /* clear any pending watchdog interrupt */
        BREG_Write32(hReg, BCHP_SCPU_HOST_INTR2_CPU_CLEAR, 1 << BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_TIMER_SHIFT);

        /* request a reset */
        BREG_Write32(hReg, SAGE_RESET_REG, SAGE_RESETVAL_RESET);
        do {
            uint32_t wdval;
            cpt++;
            BKNI_Sleep(1);
            wdval = BREG_Read32(hReg, BCHP_SCPU_HOST_INTR2_CPU_STATUS);
            if (wdval & (1 << BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_TIMER_SHIFT)) {
                BDBG_MSG(("SAGE has been reset successfully!"));
                break;
            }
        } while (cpt < 500);

		    if (cpt >= 500) {
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
