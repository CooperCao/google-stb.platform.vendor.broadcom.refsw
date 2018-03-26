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

#include <arch.h>
#include <arch_helpers.h>

#include "config.h"
#include "monitor.h"
#include "boot.h"
#include "cache.h"
#include "cpu_data.h"
#include "psci.h"
#include "psci_priv.h"
#include "platform_power.h"

psci_system_t psci_system;
psci_cluster_t psci_clusters[MAX_NUM_CLUSTERS];
psci_cpu_t psci_cpus[MAX_NUM_CPUS];

extern void mon_secondary_entry_point(void);
extern ptrdiff_t mon_load_link_offset;

/***************************
 * Local utility functions *
 ***************************/

static bool last_on_cluster_in_system(psci_cluster_t *pcluster)
{
    psci_system_t *psystem = &psci_system;
    size_t i;

    for (i = 0; i < psystem->num_clusters; i++) {
        psci_cluster_t *pscluster = psystem->pclusters[i];
        if (pscluster != pcluster &&
            pscluster->state != PSCI_STATE_OFF)
            return false;
    }
    return true;
}

static bool last_on_cpu_in_cluster(psci_cpu_t *pcpu)
{
    psci_cluster_t *pcluster = pcpu->pcluster;
    size_t i;

    for (i = 0; i < pcluster->num_cpus; i++) {
        psci_cpu_t *pccpu = pcluster->pcpus[i];
        if (pccpu != pcpu &&
            pccpu->state != PSCI_STATE_OFF &&
            pccpu->state != PSCI_STATE_OFF_PENDING)
            return false;
    }
    return true;
}

static int cpu_power_up(psci_cpu_t *pcpu)
{
    DBG_MSG("Power up CPU %d from CPU %d",
            pcpu->index, psci_get_cpu()->index);

    /* Call plaform power up */
    /* Must use load address for secondary boot entry point */
    if (plat_cpu_power_up(pcpu->index,
            link_to_load((uintptr_t)mon_secondary_entry_point)))
        return PSCI_INTERNAL_FAILURE;

    return PSCI_SUCCESS;
}

static int cpu_power_down(psci_cpu_t *pcpu)
{
    DBG_MSG("Power down CPU %d from CPU %d",
            pcpu->index, psci_get_cpu()->index);

    /* Call plaform power down */
    if (plat_cpu_power_down(pcpu->index))
        return PSCI_INTERNAL_FAILURE;

    return PSCI_SUCCESS;
}

static int cpu_detach()
{
    uint32_t sctlr, cpuectlr;

    DBG_MSG("Detach CPU %d", psci_get_cpu()->index);

    /* Clear any dangling exclusive locks */
    clrex();

    /* ARM recipe for "Individual core shutdown mode":
     * http://infocenter.arm.com/help/ index.jsp? \
     * topic=/com.arm.doc.ddi0500d/CACJFAJC.html
     */

    /* 1. Disable data cache */
    sctlr  = read_sctlr_el3();
    sctlr &= ~SCTLR_C_BIT;
    write_sctlr_el3(sctlr);

    /* 2. Clear and invalidate L1 data cache */
    dcsw_op_all(DCCISW);

    /* 3. Disable data coherency with other cores in the cluster */
    cpuectlr  = read_cpuectlr();
    cpuectlr &= ~CPUECTLR_SMP_BIT;
    write_cpuectlr(cpuectlr);

    /* 4. Execute a ISB instruction */
    /* 5. Execute a DSB SY instruction */
    isb();
    dsb();

    /* 6. Execute a WFI instruction */
    while (1) wfi();

    /* !!! Should NOT have reached here !!! */
    return PSCI_INTERNAL_FAILURE;
}

static int cpu_standby()
{
    uint32_t isr;

    DBG_MSG("Standby CPU %d", psci_get_cpu()->index);

    /* Return if interrupt(s) pending */
    isr = read_isr_el1();
    if (isr)
        return PSCI_SUCCESS;

    /* Wait for interrupt */
    wfi();

    /* Wake up and continue */
    return PSCI_SUCCESS;
}

static void cpus_power_down()
{
    psci_system_t *psystem = &psci_system;
    psci_cpu_t *pcpu = psci_get_cpu();
    size_t i, j;

    for (i = 0; i < psystem->num_clusters; i++) {
        psci_cluster_t *pscluster = psystem->pclusters[i];

        for (j = 0; j < pscluster->num_cpus; j++) {
            psci_cpu_t *pccpu = pscluster->pcpus[j];

            if (pccpu != pcpu &&
                pccpu->state == PSCI_STATE_OFF_PENDING) {
                /* Power down CPU */
                cpu_power_down(pccpu);

                /* Update CPU state */
                pccpu->state = PSCI_STATE_OFF;
            }
        }
    }
}

/************************
 * System API functions *
 ************************/

int psci_init(
    uint32_t num_clusters,
    uint32_t num_cpus,
    uint32_t *cluster_num_cpus)
{
    psci_system_t *psystem;
    psci_cluster_t *pcluster;
    psci_cpu_t *pcpu;
    size_t i, j;

    /*
     * Init control blocks
     */
    for (i = 0; i < num_clusters; i++) {
        pcluster = &psci_clusters[i];
        pcluster->index = i;
    }

    for (i = 0; i < num_cpus; i++) {
        pcpu = &psci_cpus[i];
        pcpu->index = i;
    }

    /*
     * Construct topology tree.
     */
    psystem = &psci_system;
    pcluster = &psci_clusters[0];

    psystem->num_clusters = num_clusters;
    for (i = 0; i < num_clusters; i++) {
        psystem->pclusters[i] = pcluster;
        pcluster->psystem = psystem;
        pcluster++;
    }

    if (cluster_num_cpus) {
        /* Assume CPUs are linearly ordered */
        pcluster = &psci_clusters[0];
        pcpu = &psci_cpus[0];

        for (i = 0; i < num_clusters; i++) {
            pcluster->num_cpus = cluster_num_cpus[i];
            for (j = 0; j < cluster_num_cpus[i]; j++) {
                pcluster->pcpus[j] = pcpu;
                pcpu->pcluster = pcluster;
                pcpu++;
            }
            pcluster++;
        }
    }
    else {
        /* Assume each cluster has the same number of CPUs. */
        size_t num_cpus_per_cluster = num_cpus / num_clusters;
        ASSERT(num_cpus == num_clusters * num_cpus_per_cluster);

        pcluster = &psci_clusters[0];
        pcpu = &psci_cpus[0];

        for (i = 0; i < num_clusters; i++) {
            pcluster->num_cpus = num_cpus_per_cluster;
            for (j = 0; j < num_cpus_per_cluster; j++) {
                pcluster->pcpus[j] = pcpu;
                pcpu->pcluster = pcluster;
                pcpu++;
            }
            pcluster++;
        }
    }

    INFO_MSG("PSCI init done");
    return PSCI_SUCCESS;
}

int psci_cpu_on(
    uint64_t target_mpidr,
    uintptr_t entry_point,
    uint64_t context_id)
{
    psci_system_t *psystem = &psci_system;
    psci_cluster_t *pcluster = psci_get_cluster_by_mpidr(target_mpidr);
    psci_cpu_t *pcpu = psci_get_cpu_by_mpidr(target_mpidr);
    int ret;

    if (!pcpu)
        return PSCI_INVALID_PARAMETERS;

    SYS_MSG("CPU %d is powering up...", pcpu->index);

    spin_lock(&psystem->lock);

    if (pcpu->state == PSCI_STATE_ON) {
        ret = PSCI_ALREADY_ON;
        goto exit;
    }

    if (pcpu->state == PSCI_STATE_ON_PENDING) {
        ret = PSCI_ON_PENDING;
        goto exit;
    }

    /* Save non-secure entry point and context ID */
    pcpu->nsec_entry_point = entry_point;
    pcpu->nsec_context_id  = context_id;

    /* Call plaform power up */
    ret = cpu_power_up(pcpu);
    if (ret) goto exit;

    /* Update CPU state */
    pcpu->state = PSCI_STATE_ON_PENDING;

    /* Update cluster state if necessary */
    if (pcluster->state == PSCI_STATE_OFF) {
        pcluster->state = PSCI_STATE_ON_PENDING;

        /* Update system state if necessary */
        /* System state should be in ON state */
        UNUSED(psystem);
    }

    ret = PSCI_SUCCESS;

exit:
    spin_unlock(&psystem->lock);
    return ret;
}

int psci_cpu_up(void)
{
    uint64_t mpidr = read_mpidr();
    psci_system_t *psystem = &psci_system;
    psci_cluster_t *pcluster = &psci_clusters[MPIDR_AFFLVL1_VAL(mpidr)];
    psci_cpu_t *pcpu = pcluster->pcpus[MPIDR_AFFLVL0_VAL(mpidr)];
    cpu_data_t *pcpu_data;

    SYS_MSG("CPU %d is up", pcpu->index);

    spin_lock(&psystem->lock);

    /* Hook up to CPU data */
    pcpu_data = get_cpu_data();
    pcpu_data->psci_cpu = pcpu;

    /* Update all PSCI states */
    pcpu->state = PSCI_STATE_ON;
    pcluster->state = PSCI_STATE_ON;
    psystem->state = PSCI_STATE_ON;

    spin_unlock(&psystem->lock);
    return PSCI_SUCCESS;
}

int psci_cpu_off(void)
{
    uint64_t mpidr = read_mpidr();
    psci_system_t *psystem = &psci_system;
    psci_cluster_t *pcluster = &psci_clusters[MPIDR_AFFLVL1_VAL(mpidr)];
    psci_cpu_t *pcpu = pcluster->pcpus[MPIDR_AFFLVL0_VAL(mpidr)];

    SYS_MSG("CPU %d is powering down...", pcpu->index);

    spin_lock(&psystem->lock);

    /* Update CPU state */
    pcpu->state = PSCI_STATE_OFF_PENDING;

    /* Update cluster state if necessary */
    if (last_on_cpu_in_cluster(pcpu)) {
        pcluster->state = PSCI_STATE_OFF;

        /* Update system state if necessary */
        if (last_on_cluster_in_system(pcluster)) {
            psystem->state = PSCI_STATE_OFF;

            /* Power down CPU directly, no one else can do it */
            pcpu->state = PSCI_STATE_OFF;
        }
    }

    /* Unlock before going down */
    spin_unlock(&psystem->lock);

    if (pcpu->state == PSCI_STATE_OFF) {
        /* Power down CPU directly */
        cpu_power_down(pcpu);
    }
    else {
        /* Detach first, pending power down */
        cpu_detach();
    }

    /* !!! Should NOT have reached here !!! */
    return PSCI_INTERNAL_FAILURE;
}

int psci_cpu_suspend(
    uint32_t power_state,
    uintptr_t entry_point,
    uint64_t context_id)
{
    uint64_t mpidr = read_mpidr();
    psci_system_t *psystem = &psci_system;
    psci_cluster_t *pcluster = &psci_clusters[MPIDR_AFFLVL1_VAL(mpidr)];
    psci_cpu_t *pcpu = pcluster->pcpus[MPIDR_AFFLVL0_VAL(mpidr)];
    int pstate_level = PSCI_PSTATE_LEVEL(power_state);
    int pstate_type = PSCI_PSTATE_TYPE(power_state);
    int pstate_id = PSCI_PSTATE_ID(power_state);

    SYS_MSG("CPU %d is being suspended: level=%d type =%d state=%d...",
            pcpu->index, pstate_level, pstate_type, pstate_id);

    /* Cluster or system level standby is not supported yet */
    if (pstate_type == PSCI_PSTATE_TYPE_STANDBY &&
        pstate_level != PSCI_PSTATE_LEVEL_CORE)
	return PSCI_INVALID_PARAMETERS;

    spin_lock(&psystem->lock);

    if (pstate_type == PSCI_PSTATE_TYPE_STANDBY) {
        /* Update CPU state */
        pcpu->state = PSCI_STATE_STANDBY;

        /* Unlock before going to standby */
        spin_unlock(&psystem->lock);

	/* Go into standby */
	cpu_standby();

        /* Relock after coming from standby */
        spin_lock(&psystem->lock);

        /* Restore CPU state */
        pcpu->state = PSCI_STATE_RUN;
    }
    else {
	/* Save non-secure entry point and context ID */
	pcpu->nsec_entry_point = entry_point;
	pcpu->nsec_context_id  = context_id;

        if (pstate_level == PSCI_PSTATE_LEVEL_CORE) {
            /* CPU_OFF disguised as CPU_SUSPEND */
            spin_unlock(&psystem->lock);
            return psci_cpu_off();
        }
        else if (pstate_level == PSCI_PSTATE_LEVEL_SYSTEM) {
            /* SYSTEM_OFF disguised as CPU_SUSPEND */
            spin_unlock(&psystem->lock);
            return psci_system_off();
        }
    }

    spin_unlock(&psystem->lock);
    return PSCI_SUCCESS;
}

int psci_affinity_info(
    uint64_t target_mpidr,
    uint32_t target_level)
{
    psci_system_t *psystem = &psci_system;
    psci_cluster_t *pcluster = psci_get_cluster_by_mpidr(target_mpidr);
    psci_cpu_t *pcpu = psci_get_cpu_by_mpidr(target_mpidr);
    psci_state_t target_state;
    int ret;

    if ((target_level == PSCI_AFFINITY_LEVEL_CORE && !pcpu) ||
        (target_level == PSCI_AFFINITY_LEVEL_CLUSTER && !pcluster) ||
        (target_level >  PSCI_AFFINITY_LEVEL_SYSTEM))
        return PSCI_INVALID_PARAMETERS;

    spin_lock(&psystem->lock);

    /* Power down CPUs in OFF_PENDING state */
    cpus_power_down();

    switch (target_level) {
    case PSCI_AFFINITY_LEVEL_CORE:
    default:
        target_state = pcpu->state;
        break;
    case PSCI_AFFINITY_LEVEL_CLUSTER:
        target_state = pcluster->state;
        break;
    case PSCI_AFFINITY_LEVEL_SYSTEM:
        target_state = psystem->state;
        break;
    }

    switch (target_state) {
    case PSCI_STATE_ON_PENDING:
        ret = PSCI_AFFINITY_ON_PENDING;
        break;
    case PSCI_STATE_OFF:
    case PSCI_STATE_OFF_PENDING:
        ret = PSCI_AFFINITY_OFF;
        break;
    default:
        ret = PSCI_AFFINITY_ON;
        break;
    }

    spin_unlock(&psystem->lock);

    DBG_MSG("Affinity info: mpidr=0x%llx, level=%d, state=%d",
            (unsigned long long)target_mpidr, target_level, ret);

    return ret;
}

int psci_system_off(void)
{
    uint64_t mpidr = read_mpidr();
    psci_system_t *psystem = &psci_system;
    psci_cluster_t *pcluster = &psci_clusters[MPIDR_AFFLVL1_VAL(mpidr)];
    psci_cpu_t *pcpu = pcluster->pcpus[MPIDR_AFFLVL0_VAL(mpidr)];
    size_t i, j;

    SYS_MSG("System is powering down...");

    spin_lock(&psystem->lock);

    for (i = 0; i < psystem->num_clusters; i++) {
        psci_cluster_t *pscluster = psystem->pclusters[i];

        for (j = 0; j < pscluster->num_cpus; j++) {
            psci_cpu_t *pccpu = pscluster->pcpus[j];

            if (pccpu == pcpu) {
                /* Power down current CPU at the end */
                /* Update CPU state */
                pccpu->state = PSCI_STATE_OFF;
            }
            else if (pccpu->state != PSCI_STATE_OFF) {
                /* Power down CPU */
                cpu_power_down(pccpu);

                /* Update CPU state */
                pccpu->state = PSCI_STATE_OFF;
            }
        }

        /* Update cluster state */
        pscluster->state = PSCI_STATE_OFF;
    }

    /* Update system state */
    psystem->state = PSCI_STATE_OFF;

    /* Unlock before going down */
    spin_unlock(&psystem->lock);

    /* Power down current CPU */
    cpu_power_down(pcpu);

    /* !!! Should NOT have reached here !!! */
    return PSCI_INTERNAL_FAILURE;
}

int psci_system_reset(void)
{
    SYS_MSG("System is resetting...");

    /* Call plaform system reset */
    plat_system_reset();

    /* !!! Should NOT have reached here !!! */
    return PSCI_INTERNAL_FAILURE;
}

/****************************
 * System utility functions *
 ****************************/

int get_cpu_index_by_mpidr(uint64_t mpidr)
{
    psci_cpu_t *pcpu = psci_get_cpu_by_mpidr(mpidr);
    return (pcpu) ? (int)pcpu->index : MON_NOENT;
}

int get_cpu_index(void)
{
    return get_cpu_index_by_mpidr(read_mpidr());
}

uintptr_t get_nsec_entry_point(void)
{
    cpu_data_t *pcpu_data = get_cpu_data();
    psci_cpu_t *pcpu = pcpu_data->psci_cpu;

    return pcpu->nsec_entry_point;
}

uint64_t get_nsec_context_id(void)
{
    cpu_data_t *pcpu_data = get_cpu_data();
    psci_cpu_t *pcpu = pcpu_data->psci_cpu;

    return pcpu->nsec_context_id;
}
