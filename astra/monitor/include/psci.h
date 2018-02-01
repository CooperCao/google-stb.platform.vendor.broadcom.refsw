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

#ifndef _PSCI_H_
#define _PSCI_H_

#include <arch_helpers.h>
#include "std_psci.h"

/*
 * PSCI error codes
 *
 * Use standard PSCI error codes for common errors.
 * Additional PSCI error code may be defined here as well.
 */
#define PSCI_SUCCESS                    STD_SVC_PSCI_SUCCESS
#define PSCI_NOT_SUPPORTED              STD_SVC_PSCI_NOT_SUPPORTED
#define PSCI_INVALID_PARAMMETERS        STD_SVC_PSCI_INVALID_PARAMETERS
#define PSCI_DENIED                     STD_SVC_PSCI_DENIED
#define PSCI_ALREADY_ON                 STB_SVC_PSCI_ALREADY_ON
#define PSCI_ON_PENDING                 STB_SVC_PSCI_ON_PENDING
#define PSCI_INTERNAL_FAILURE           STB_SVC_PSCI_INTERNAL_FAILURE
#define PSCI_NOT_PRESENT                STB_SVC_PSCI_NOT_PRESENT
#define PSCI_DISABLED                   STB_SVC_PSCI_DISABLED
#define PSCI_INVALID_ADDRESS            STB_SVC_PSCI_INVALID_ADDRESS

/*
 * PSCI functions
 */
int psci_init(
    uint32_t num_clusters,
    uint32_t num_cpus,
    uint32_t *cluster_num_cpus);

int psci_cpu_up(void);

int psci_cpu_off(void);

int psci_cpu_on(
    uint64_t target_mpidr,
    uintptr_t entry_point,
    uint64_t context_id);

int psci_cluster_off(void);

int psci_system_off(void);

int psci_system_reset(void);

/*
 * System utility functions
 */

/* Return the CPU index by mpidr */
int get_cpu_index_by_mpidr(uint64_t mpidr);

/* Return the CPU index for the current CPU */
int get_cpu_index();

/* Return non-secure entry point and context ID for the current CPU */
uintptr_t get_nsec_entry_point();
uint64_t get_nsec_context_id();

#endif /* _PSCI_H_ */
