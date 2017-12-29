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
#include "cpu_data.h"
#include "psci.h"
#include "psci_priv.h"
#include "platform_power.h"

psci_system_t psci_system;
psci_cluster_t psci_clusters[MAX_NUM_CLUSTERS];
psci_cpu_t psci_cpus[MAX_NUM_CPUS];

extern void mon_secondary_entry_point(void);
extern ptrdiff_t mon_load_link_offset;

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
     * Init PSCI control blocks
     */
    for (i = 0; i < num_clusters; i++) {
        pcluster = &psci_clusters[i];
        pcluster->valid = true;
        pcluster->index = i;
    }

    for (i = 0; i < num_cpus; i++) {
        pcpu = &psci_cpus[i];
        pcpu->valid = true;
        pcpu->index = i;
    }

    /*
     * Construct PSCI topology tree.
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

int psci_cpu_up(void)
{
    psci_system_t *psystem;
    psci_cluster_t *pcluster;
    psci_cpu_t *pcpu;
    cpu_data_t *pcpu_data;

    /* Update PSCI states */
    uint64_t curr_cpu = read_mpidr();

    psystem = &psci_system;
    psystem->state = PSCI_STATE_RUNNING;

    pcluster = &psci_clusters[MPIDR_AFFLVL1_VAL(curr_cpu)];
    pcluster->state = PSCI_STATE_RUNNING;

    pcpu = pcluster->pcpus[MPIDR_AFFLVL0_VAL(curr_cpu)];
    pcpu->state = PSCI_STATE_RUNNING;

    /* Hook up to CPU data */
    pcpu_data = get_cpu_data();
    pcpu_data->psci_cpu = pcpu;

    return PSCI_SUCCESS;
}

int psci_cpu_off(void)
{
    psci_cpu_t *pcpu = psci_get_cpu();

    /* Call plaform power function */
    plat_cpu_power_down(pcpu->index);

    return PSCI_SUCCESS;
}

int psci_cpu_on(
    uint64_t target_mpidr,
    uintptr_t entry_point,
    uint64_t context_id)
{
    psci_cpu_t *pcpu = psci_get_cpu_by_mpidr(target_mpidr);

    if (!pcpu)
        return PSCI_NOT_PRESENT;

    if (pcpu->state)
        return PSCI_ALREADY_ON;

    /* Save non-secure entry point and context ID */
    pcpu->nsec_entry_point = entry_point;
    pcpu->nsec_context_id  = context_id;

    /* Call plaform power function */
    /* Must use load address for secondary boot entry point */
    if (plat_cpu_power_up(
            pcpu->index,
            (uintptr_t)mon_secondary_entry_point + mon_load_link_offset))
        return PSCI_INTERNAL_FAILURE;

    return PSCI_SUCCESS;
}

/*
 * System utility functions
 */
int get_cpu_index_by_mpidr(uint64_t mpidr)
{
    psci_cpu_t *pcpu = psci_get_cpu_by_mpidr(mpidr);
    return (pcpu) ? (int)pcpu->index : MON_NOENT;
}

int get_cpu_index()
{
    return get_cpu_index_by_mpidr(read_mpidr());
}

uintptr_t get_nsec_entry_point()
{
    cpu_data_t *pcpu_data = get_cpu_data();
    psci_cpu_t *pcpu = pcpu_data->psci_cpu;

    return pcpu->nsec_entry_point;
}

uint64_t get_nsec_context_id()
{
    cpu_data_t *pcpu_data = get_cpu_data();
    psci_cpu_t *pcpu = pcpu_data->psci_cpu;

    return pcpu->nsec_context_id;
}
