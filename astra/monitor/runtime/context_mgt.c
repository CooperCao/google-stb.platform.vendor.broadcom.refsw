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
 *****************************************************************************/

#include "config.h"
#include "monitor.h"
#include "boot.h"
#include "context_mgt.h"

void cm_init_context(
    cpu_context_t *pcpu_ctx,
    uintptr_t entry_pt,
    uintptr_t dev_tree,
    bool nsec,
    bool nsec_el2,
    bool aarch64)
{
    gp_regs_t *pgpregs_ctx;
    el3_state_t *pel3state_ctx;
    el1_sys_regs_t *psysregs_ctx;

    uint32_t sctlr_el1;
    uint32_t scr_el3;
    uint32_t spsr_el3;
    uint32_t sctlr_ee;

    pgpregs_ctx = get_gpregs_ctx(pcpu_ctx);
    pel3state_ctx = get_el3state_ctx(pcpu_ctx);
    psysregs_ctx = get_sysregs_ctx(pcpu_ctx);

    /* Setup sctlr_el1:
     * - EE bit is inherited from sctlr_el3
     * - M, C and I bits must be zero (as required by PSCI specification)
     * - RES1 bits are set differently for aarch64 and aarch32
     */
    sctlr_el1  = sctlr_ee = read_sctlr_el3() & SCTLR_EE_BIT;
    sctlr_el1 |= (aarch64) ? SCTLR_EL1_RES1 : SCTLR_AARCH32_EL1_RES1;

    /* For non-secure mode:
     * - CP15BEN is set to enable CP15 barrier instructions
     * - NTWI, NTWE are set to trap EL0 wfi/wfe to EL1
     */
    if (nsec)
        sctlr_el1 |= (SCTLR_CP15BEN_BIT | SCTLR_NTWI_BIT | SCTLR_NTWE_BIT);

    /* Populate sysregs registers in context */
    write_ctx_reg(psysregs_ctx, CTX_SCTLR_EL1, sctlr_el1);

    /* Construct scr_el3:
     * - SIF is set to disable instruction fetch from non-secure memory
     * - ST is set NOT to trap secure EL1 access to secure timer registers
     * - RW is set according to aarch64 and aarch32 of the image
     * - HCE is set to enable HVC instruction if non-secure mode uses EL2
     * - FIQ is set to take FIQ interrupts to EL3 in non-secure mode
     * - IRQ is NEVER set for IRQ interrupts are disabled in secure mode
     * - NS is set according to secure and non-secure mode
     */
    scr_el3  = SCR_RES1_BITS | SCR_SIF_BIT;
    scr_el3 |= (nsec) ? 0 : SCR_ST_BIT;
    scr_el3 |= (aarch64) ? SCR_RW_BIT : 0;
    scr_el3 |= (nsec && nsec_el2) ? SCR_HCE_BIT : 0;
    scr_el3 |= (nsec) ? SCR_FIQ_BIT : 0;
    scr_el3 |= (nsec) ? SCR_NS_BIT : 0;

    /* Setup spsr_el3:
     * - Enter into EL1 for aarch64, SVC for aarch32
     * - Disable all execptions
     */
    if (nsec && nsec_el2)
        spsr_el3 = (aarch64) ?
            SPSR_64(MODE_EL2, MODE_SP_ELX, ALL_EXCEPTIONS) :
            SPSR_32(MODE32_hyp, SPSR_T_ARM, (sctlr_ee) ? 1 : 0, AIF_EXCEPTIONS);
    else
        spsr_el3 = (aarch64) ?
            SPSR_64(MODE_EL1, MODE_SP_ELX, ALL_EXCEPTIONS) :
            SPSR_32(MODE32_svc, SPSR_T_ARM, (sctlr_ee) ? 1 : 0, AIF_EXCEPTIONS);

    /* Populate el3state registers in context */
    write_ctx_reg(pel3state_ctx, CTX_SCR_EL3,  scr_el3);
    write_ctx_reg(pel3state_ctx, CTX_SPSR_EL3, spsr_el3);
    write_ctx_reg(pel3state_ctx, CTX_ELR_EL3,  entry_pt);

    /* Populate gpregs registers in context */
    write_ctx_reg(
        pgpregs_ctx,
        (aarch64) ? CTX_GPREG_X0 : CTX_GPREG_X2,
        dev_tree);

    if (!nsec)
        write_ctx_reg(
            pgpregs_ctx,
            (aarch64) ? CTX_GPREG_X1 : CTX_GPREG_X3,
            uart_base());

}

void cm_switch_context(
    cpu_context_t *pcpu_ctx,
    bool nsec)
{
    cpu_context_t *next_ctx;

    /* Save additional registers into secure context */
    el1_sysregs_context_save(get_sysregs_ctx(pcpu_ctx));
    fpregs_context_save(get_fpregs_ctx(pcpu_ctx));

    /* Get non-secure context */
    next_ctx = cm_get_context(nsec);

    /* Restore additional registers from non-secure context */
    el1_sysregs_context_restore(get_sysregs_ctx(next_ctx));
    fpregs_context_restore(get_fpregs_ctx(next_ctx));

    /* Set non-secure context as next context*/
    cm_set_next_context(next_ctx);
}

void cm_setup_nsec_el2(
    cpu_context_t *pcpu_ctx,
    bool nsec_el2)
{
    /* Note: These are direct register writes, instead of writes into
     * contexts to be restored. Hence they need to be called on the
     * particular CPU core.
     */
    if (nsec_el2) {
        uint32_t sctlr_elx;

        /* Use SCTLR_EL1.EE value to initialise sctlr_el2 */
        sctlr_elx = read_ctx_reg(get_sysregs_ctx(pcpu_ctx), CTX_SCTLR_EL1);
        sctlr_elx &= ~SCTLR_EE_BIT;
        sctlr_elx |= SCTLR_EL2_RES1;
        write_sctlr_el2(sctlr_elx);
    }
    else if (read_id_aa64pfr0_el1() &
            (ID_AA64PFR0_ELX_MASK << ID_AA64PFR0_EL2_SHIFT)) {

        /* EL2 present but unused, need to disable safely */
        uint32_t scr_el3;
        uint32_t cptr_el2;

        /* HCR_EL2 = 0, except RW bit set to match SCR_EL3 */
        scr_el3 = read_ctx_reg(get_el3state_ctx(pcpu_ctx), CTX_SCR_EL3);
        write_hcr_el2((scr_el3 & SCR_RW_BIT) ? HCR_RW_BIT : 0);

        /* SCTLR_EL2 : can be ignored when bypassing */

        /* CPTR_EL2 : disable all traps TCPAC, TTA, TFP */
        cptr_el2 = read_cptr_el2();
        cptr_el2 &= ~(TCPAC_BIT | TTA_BIT | TFP_BIT);
        write_cptr_el2(cptr_el2);

        /* Enable EL1 access to timer */
        write_cnthctl_el2(EL1PCEN_BIT | EL1PCTEN_BIT);

        /* Reset CNTVOFF_EL2 */
        write_cntvoff_el2(0);

        /* Set VPIDR, VMPIDR to match MIDR, MPIDR */
        write_vpidr_el2(read_midr_el1());
        write_vmpidr_el2(read_mpidr_el1());

        /* Reset VTTBR_EL2.
         * Needed because cache maintenance operations depend on
         * the VMID even when non-secure EL1&0 stage 2 address
         * translation are disabled.
         */
        write_vttbr_el2(0);
    }
}
