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

#ifndef _PSCI_PRIV_H_
#define _PSCI_PRIV_H_

#include <stdint.h>
#include <stdbool.h>
#include <config.h>
#include <spinlock.h>

/* Power states */
/* This a superset of states that covers:
 * - states as defined in PSCI AFFINITY_INFO
 * - states as defined in PSCI NODE_HW_STATE
 * - two-step CPU off procedure (detach and power-down)
 */
typedef enum psci_state {
    PSCI_STATE_OFF = 0,
    PSCI_STATE_OFF_PENDING, /* CPU specific, considered OFF in PSCI */
    PSCI_STATE_ON_PENDING,
    PSCI_STATE_ON,

    /* ON states */
    PSCI_STATE_RUN  = PSCI_STATE_ON,
    PSCI_STATE_STANDBY,
    PSCI_STATE_RETENTION,

    PSCI_STATE_MAX
} psci_state_t;

/* PSCI CPU control block */
typedef struct psci_cpu {
    uint32_t index;
    uint32_t state;
    bool detached;

    uintptr_t nsec_entry_point;
    uint64_t nsec_context_id;

    struct psci_cluster *pcluster;
} psci_cpu_t;

/* PSCI cluster control block */
typedef struct psci_cluster {
    uint32_t index;
    uint32_t state;

    struct psci_system *psystem;

    uint32_t num_cpus;
    struct psci_cpu *pcpus[MAX_NUM_CPUS];
} psci_cluster_t;

/* PSCI system control block */
typedef struct psci_system {
    uint32_t state;
    spinlock_t lock;

    uint32_t num_clusters;
    struct psci_cluster *pclusters[MAX_NUM_CLUSTERS];
} psci_system_t;

/* PSCI global variables */
extern psci_system_t psci_system;
extern psci_cluster_t psci_clusters[MAX_NUM_CLUSTERS];
extern psci_cpu_t psci_cpus[MAX_NUM_CPUS];

/* Return the cluster control block by mpidr */
static inline psci_cluster_t *psci_get_cluster_by_mpidr(uint64_t mpidr)
{
    psci_system_t *psystem = &psci_system;
    size_t icluster = MPIDR_AFFLVL1_VAL(mpidr);

    return (icluster < psystem->num_clusters) ?
        psystem->pclusters[icluster] : NULL;
}

/* Return the CPU control block by mpidr */
static inline psci_cpu_t *psci_get_cpu_by_mpidr(uint64_t mpidr)
{
    psci_cluster_t *pcluster = psci_get_cluster_by_mpidr(mpidr);
    size_t icpu = MPIDR_AFFLVL0_VAL(mpidr);

    return (pcluster && icpu < pcluster->num_cpus) ?
        pcluster->pcpus[icpu] : NULL;
}

/* Return the cluster control block for the current CPU */
static inline psci_cluster_t *psci_get_cluster()
{
    return psci_get_cluster_by_mpidr(read_mpidr());
}

/* Return the CPU control block for the current CPU */
static inline psci_cpu_t *psci_get_cpu()
{
    return psci_get_cpu_by_mpidr(read_mpidr());
}

#endif /* _PSCI_PRIV_H_ */
