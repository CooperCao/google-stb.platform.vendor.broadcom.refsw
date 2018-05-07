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
#include "dvfs.h"
#include "dvfs_priv.h"
#include "platform_dvfs.h"

dvfs_system_t dvfs_system;
dvfs_island_t dvfs_islands[MAX_NUM_ISLANDS];
dvfs_core_t dvfs_cores[MAX_NUM_CORES];
dvfs_cpu_t dvfs_cpus[MAX_NUM_CPUS];
bool dvfs_active;

static int cpu_pstate_update(void)
{
    dvfs_core_t *pcore = &dvfs_cores[0];
    uint32_t min_pstate = DVFS_PSTATE_INVALID;
    size_t i;

    /* Find the mininum of required pstates of all CPUs */
    for (i = 0; i < pcore->num_cpus; i++) {
        dvfs_cpu_t *pcpu = &dvfs_cpus[i];
        uint32_t cpu_load = 0; /* in KHz */
        size_t j;

        if (pcpu->state == DVFS_STATE_DOWN)
            continue;

        if (is_sec_enable())
            cpu_load += pcpu->sec_load;

        if (is_nsec_enable()) {
            /* Ignore CPU without request pstate */
            if (pcpu->nsec_pstate == DVFS_PSTATE_INVALID)
                continue;

            cpu_load += pcore->pstate_freqs[pcpu->nsec_pstate] * 1000;
        }

        /* Checking backwards, default to pstate 0 */
        for (j = pcore->num_pstates - 1; j > 0; j--) {
            if (cpu_load <= pcore->pstate_freqs[j] * 1000)
                break;
        }

        if (j < min_pstate)
            min_pstate = j;
    }

    if (min_pstate != DVFS_PSTATE_INVALID) {
        /* Decide next pstate
         *
         * - Increase frequency in one shot;
         * - Decrease freqeuncy
         *   o in one shot without secure CPUs;
         *   o in steps with secure CPUs.
         */
        uint32_t next_pstate =
            (min_pstate <= pcore->cur_pstate) ? min_pstate :
            (!pcore->sec_cpus) ? min_pstate : pcore->cur_pstate + 1;

        if (next_pstate != pcore->cur_pstate) {
            DBG_MSG("Changing CPU pstate from %d to %d",
                    pcore->cur_pstate, next_pstate);

            plat_cpu_pstate_set(next_pstate);
            pcore->cur_pstate = next_pstate;
        }
    }

    return MON_OK;
}

int dvfs_init(uint32_t num_cpus)
{
    dvfs_system_t *psystem;
    dvfs_island_t *pisland;
    dvfs_core_t *pcore;
    size_t i;

    /*
     * Construct topology tree
     *
     * - Initialized with only system -> CPU island (0) -> CPU core (0);
     * - Other islands and cores are added at runtime.
     */
    psystem = &dvfs_system;
    psystem->num_islands = 1;
    psystem->pislands[0] = &dvfs_islands[0];

    pisland = &dvfs_islands[0];
    pisland->num_cores = 1;
    pisland->pcores[0] = &dvfs_cores[0];

    pcore = &dvfs_cores[0];
    pcore->num_cpus = num_cpus;
    pcore->master_cpu = get_cpu_index();
    pcore->nsec_pstate = DVFS_PSTATE_INVALID;

    /*
     * Before DVFS is active, DVFS operates passively
     *
     * - CPU pstate update is done externally;
     * - DVFS is only allowed to read current CPU frequency values;
     * - DVFS still needs to interacts with the secure world, presenting
     *   a single pstate corresponding to current CPU frequency.
     */
    pcore->cur_pstate = 0;
    pcore->num_pstates = 1;
    plat_cpu_freq_get(&pcore->pstate_freqs[0]);

    for (i = 0; i < num_cpus; i++) {
        dvfs_cpu_t *pcpu = &dvfs_cpus[i];

        pcpu->state = DVFS_STATE_DOWN;
        pcpu->sec_load = 0;
        pcpu->nsec_pstate = pcore->nsec_pstate;
    }

    INFO_MSG("DVFS init done");
    return MON_OK;
}

int dvfs_activate(void)
{
    dvfs_core_t *pcore = &dvfs_cores[0];

    /* Init platform DVFS */
    plat_dvfs_init();

    /* Get current CPU pstate */
    plat_cpu_pstate_get(&pcore->cur_pstate);

    /* Get CPU pstate frequencies */
    pcore->num_pstates = MAX_NUM_PSTATES;
    plat_cpu_pstate_freqs(
        &pcore->num_pstates,
        pcore->pstate_freqs);

    /* Mark DVFS active */
    dvfs_active = true;

    INFO_MSG("DVFS late init done");
    return MON_OK;
}

int dvfs_cpu_up(uint32_t cpu_index)
{
    dvfs_system_t *psystem = &dvfs_system;
    dvfs_core_t *pcore = &dvfs_cores[0];
    dvfs_cpu_t *pcpu = &dvfs_cpus[cpu_index];
    cpu_data_t *pcpu_data;

    spin_lock(&psystem->lock);

    /* Hook up to CPU data */
    pcpu_data = get_cpu_data_by_index(cpu_index);
    pcpu_data->dvfs_cpu = pcpu;

    /* Choose lower numbered CPU as master */
    if (cpu_index < pcore->master_cpu)
        pcore->master_cpu = cpu_index;

    /* Increment secure CPU count if enabled */
    if (is_sec_enable_by_index(cpu_index))
        pcore->sec_cpus++;

    /* Update CPU state */
    pcpu->state = DVFS_STATE_UP;

    spin_unlock(&psystem->lock);
    return MON_OK;
}

int dvfs_cpu_down(uint32_t cpu_index)
{
    dvfs_system_t *psystem = &dvfs_system;
    dvfs_core_t *pcore = &dvfs_cores[0];
    dvfs_cpu_t *pcpu = &dvfs_cpus[cpu_index];
    size_t i;

    spin_lock(&psystem->lock);

    /* Update CPU state and reset CPU data */
    pcpu->state = DVFS_STATE_DOWN;
    pcpu->sec_load = 0;
    pcpu->nsec_pstate = pcore->nsec_pstate;

    /* Choose another CPU as master if master is down */
    if (cpu_index == pcore->master_cpu) {
        for (i = 0; i < pcore->num_cpus; i++) {
            dvfs_cpu_t *pcpu = &dvfs_cpus[i];

            if (pcpu->state != DVFS_STATE_DOWN)
                break;
        }
        pcore->master_cpu = i;
    }

    /* Descrement secure CPU count if enabled */
    if (is_sec_enable_by_index(cpu_index)) {
        pcore->sec_cpus--;

        /* Update CPU pstate without secure CPUs */
        if (!pcore->sec_cpus)
            cpu_pstate_update();
    }

    spin_unlock(&psystem->lock);
    return MON_OK;
}

int dvfs_set_sec_load(uint32_t load)
{
    dvfs_system_t *psystem = &dvfs_system;
    dvfs_core_t *pcore = &dvfs_cores[0];
    uint32_t cpu_index = get_cpu_index();
    dvfs_cpu_t *pcpu = &dvfs_cpus[cpu_index];

    spin_lock(&psystem->lock);

    /* Update CPU pstate on master CPU if DVFS is active */
    if (dvfs_active &&
        cpu_index == pcore->master_cpu)
        cpu_pstate_update();

    /* Remember secure CPU load in KHz */
    pcpu->sec_load = load;

    spin_unlock(&psystem->lock);
    return MON_OK;
}

int dvfs_get_sec_freq(uint32_t *pfreq)
{
    dvfs_system_t *psystem = &dvfs_system;
    dvfs_core_t *pcore = &dvfs_cores[0];

    spin_lock(&psystem->lock);

    /* Update current CPU frequency if DVFS is not active */
    if (!dvfs_active)
        plat_cpu_freq_get(&pcore->pstate_freqs[0]);

    /* Return (secure) CPU frequency in KHz */
    if (pfreq)
        *pfreq = pcore->pstate_freqs[pcore->cur_pstate] * 1000;

    spin_unlock(&psystem->lock);
    return MON_OK;
}

int dvfs_set_nsec_pstate_all(uint32_t pstate)
{
    dvfs_system_t *psystem = &dvfs_system;
    dvfs_core_t *pcore = &dvfs_cores[0];
    size_t i;

    spin_lock(&psystem->lock);

    /* Remember requested pstate of CPU core */
    pcore->nsec_pstate = pstate;

    /* Remember requested pstate of all CPUs */
    for (i = 0; i < pcore->num_cpus; i++) {
        dvfs_cpu_t *pcpu = &dvfs_cpus[i];

        /* Update even if CPU is down */
        pcpu->nsec_pstate = pstate;
    }

    /* Update CPU pstate without secure CPUs */
    if (!pcore->sec_cpus)
        cpu_pstate_update();

    spin_unlock(&psystem->lock);
    return MON_OK;
}

int dvfs_get_nsec_pstate_all(uint32_t *ppstate)
{
    dvfs_system_t *psystem = &dvfs_system;
    dvfs_core_t *pcore = &dvfs_cores[0];

    spin_lock(&psystem->lock);

    /* Return last requested pstate of CPU core */
    if (ppstate)
        *ppstate = pcore->nsec_pstate;

    spin_unlock(&psystem->lock);
    return MON_OK;
}

int dvfs_set_nsec_pstate(uint32_t pstate)
{
    dvfs_system_t *psystem = &dvfs_system;
    dvfs_core_t *pcore = &dvfs_cores[0];
    dvfs_cpu_t *pcpu = &dvfs_cpus[get_cpu_index()];

    spin_lock(&psystem->lock);

    /* Remember requested pstate */
    pcpu->nsec_pstate = pstate;

    /* Update CPU pstate without secure CPUs */
    if (!pcore->sec_cpus)
        cpu_pstate_update();

    spin_unlock(&psystem->lock);
    return MON_OK;
}

int dvfs_get_nsec_pstate(uint32_t *ppstate)
{
    dvfs_system_t *psystem = &dvfs_system;
    dvfs_cpu_t *pcpu = &dvfs_cpus[get_cpu_index()];

    spin_lock(&psystem->lock);

    /* Return last requested pstate */
    if (ppstate)
        *ppstate = pcpu->nsec_pstate;

    spin_unlock(&psystem->lock);
    return MON_OK;
}
