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

#ifndef _CPU_DATA_H_
#define _CPU_DATA_H_

#ifndef __ASSEMBLY__

#include <stdbool.h>
#include <arch.h>
#include <arch_helpers.h>
#include <context.h>

#define CPU_SEC_MASK            (0x01 << 0)
#define CPU_NSEC_MASK           (0x01 << 1)

typedef struct cpu_data {
    /* CPU contexts */
    cpu_context_t cpu_ctx[2];

    /* CPU flags */
    uint32_t cpu_flags;

    /* Pointer to PSCI CPU strcuture */
    struct psci_cpu *psci_cpu;

} __align_cache cpu_data_t;

/**************************************************************************
 * APIs for initialising and accessing per-cpu data
 *************************************************************************/

extern cpu_data_t cpu_data[];

/* Return the cpu_data structure by CPU index. */
static inline cpu_data_t *get_cpu_data_by_index(uint32_t cpu_index) {
    return &cpu_data[cpu_index];
}

/* Return secure enable status by CPU index. */
static inline bool is_sec_enable_by_index(uint32_t cpu_index) {
    cpu_data_t *pcpu_data = get_cpu_data_by_index(cpu_index);
    return pcpu_data->cpu_flags & CPU_SEC_MASK;
}

/* Return non-secure enable status by CPU index. */
static inline bool is_nsec_enable_by_index(uint32_t cpu_index) {
    cpu_data_t *pcpu_data = get_cpu_data_by_index(cpu_index);
    return pcpu_data->cpu_flags & CPU_NSEC_MASK;
}

/* Return the cpu_data structure for the current CPU. */
static inline cpu_data_t *get_cpu_data(void) {
    return (cpu_data_t *)read_tpidr_el3();
}

/* Return secure enable status for the current CPU. */
static inline bool is_sec_enable(void) {
    cpu_data_t *pcpu_data = get_cpu_data();
    return pcpu_data->cpu_flags & CPU_SEC_MASK;
}

/* Return non-secure enable status for the current CPU. */
static inline bool is_nsec_enable(void) {
    cpu_data_t *pcpu_data = get_cpu_data();
    return pcpu_data->cpu_flags & CPU_NSEC_MASK;
}

/* Return secure enable status given the cpu_data structure. */
static inline bool sec_enable(cpu_data_t *pcpu_data) {
    return pcpu_data->cpu_flags & CPU_SEC_MASK;
}

/* Return non-secure enable status given the cpu_data structure. */
static inline bool nsec_enable(cpu_data_t *pcpu_data) {
    return pcpu_data->cpu_flags & CPU_NSEC_MASK;
}

#endif /* __ASSEMBLY__ */

#endif /* _CPU_DATA_H_ */
