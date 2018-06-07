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

#ifndef _DVFS_PRIV_H_
#define _DVFS_PRIV_H_

#include <stdint.h>
#include <stdbool.h>
#include <config.h>
#include <spinlock.h>

#define DVFS_PSTATE_INVALID             0xFFFFFFFF

/* DVFS CPU states */
typedef enum dvfs_state {
    DVFS_STATE_DOWN = 0,
    DVFS_STATE_UP,
    DVFS_STATE_MAX
} dvsf_state_t;

/* DVFS CPU control block */
typedef struct dvfs_cpu {
    uint32_t state;

    /* Loading of secure world (in KCycles) */
    uint32_t sec_load;

    /* Requested pstate of non-secure world */
    uint32_t nsec_pstate;
} dvfs_cpu_t;

/* DVFS core control block */
typedef struct dvfs_core {
    /* Core 0 (CPU core) only */
    size_t num_cpus;
    size_t sec_cpus;
    uint32_t master_cpu;

    /* Requested pstate of non-secure world */
    uint32_t nsec_pstate;

    /* Current pstate */
    uint32_t cur_pstate;

    /* Pstate frequency mapping */
    size_t num_pstates;
    uint32_t pstate_freqs[MAX_NUM_PSTATES];
} dvfs_core_t;

/* DVFS island control block */
typedef struct dvfs_island {
    size_t num_cores;
    struct dvfs_core *pcores[MAX_NUM_CORES];
} dvfs_island_t;

/* DVFS system control block */
typedef struct dvfs_system {
    size_t num_islands;
    struct dvfs_island *pislands[MAX_NUM_ISLANDS];

    /* DVFS system-wide lock */
    spinlock_t lock;
} dvfs_system_t;

/* DVFS global variables */
extern dvfs_system_t dvfs_system;
extern dvfs_island_t dvfs_islands[MAX_NUM_ISLANDS];
extern dvfs_core_t dvfs_cores[MAX_NUM_CORES];
extern dvfs_cpu_t dvfs_cpus[MAX_NUM_CPUS];

#endif /* _DVFS_PRIV_H_ */
