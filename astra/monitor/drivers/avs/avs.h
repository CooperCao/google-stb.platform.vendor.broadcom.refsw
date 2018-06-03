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

#ifndef _AVS_H_
#define _AVS_H_

#define MAX_AVS_PSTATES                 8
#define MAX_AVS_SENSORS                 8

int avs_init(
    uintptr_t mbox_base,
    uintptr_t host_l2_base,
    uintptr_t avs_l2_base);

/*
 * Get current CPU pstate
 *
 * - Input:
 *    *ppstate, current CPU pstate to be filled
 *
 * - Output:
 *    *ppstate, filled with current CPU pstate
 *
 * - Return:
 *     Monitor error code (negative)
 */
int avs_cpu_pstate_get(uint32_t *ppstate);

/*
 * Set CPU pstate
 *
 * - Input:
 *    pstate, CPU pstate to be set
 *
 * - Return:
 *     Monitor error code (negative)
 */
int avs_cpu_pstate_set(uint32_t pstate);

/*
 * Get CPU pstate frequency of a given pstate
 *
 * - Input:
 *    pstate, cpu_pstate of interest
 *    *pfreq, CPU pstate frequency to be filled
 *
 * - Output:
 *    *pfreq, filled with pstate frequency in MHz
 *
 * - Return:
 *     Monitor error code (negative)
 */
int avs_cpu_pstate_freq(
    uint32_t pstate,
    uint32_t *pfreq);

/*
 * Get all CPU pstate frequencies
 *
 * - Input:
 *     *pcount, size of the pfreqs array
 *     *pfreqs, array of CPU pstate frequencies to be filled
 *
 * - Output:
 *     *pcount, number of CPU pstates
 *     *pfreqs, filled with CPU pstate frequencies in MHz
 *
 * - Return:
 *     Monitor error code (negative)
 */
int avs_cpu_pstate_freqs(
    size_t *pcount,
    uint32_t *pfreqs);

/*
 * Get number of pstates of a given core
 *
 * - Input:
 *     core, core of interest
 *
 * - Return:
 *     Number of pstates of the given core, or
 *     Monitor error code (negative)
 */
int avs_num_pstates(uint32_t core);

/*
 * Get current pstate of a given core
 *
 * - Input:
 *    core, core of interest
 *    *ppstate, current pstate to be filled
 *
 * - Output:
 *    *ppstate, filled with current pstate
 *
 * - Return:
 *     Monitor error code (negative)
 */
int avs_get_pstate(
    uint32_t core,
    uint32_t *ppstate);

/*
 * Set pstate of a given core
 *
 * - Input:
 *    core, core of interest
 *    pstate, pstate to be set
 *
 * - Return:
 *     Monitor error code (negative)
 */
int avs_set_pstate(
    uint32_t core,
    uint32_t pstate);

/*
 * Get number of debug items
 *
 * - Return:
 *     Number of debug items, or
 *     Monitor error code (negative)
 */
int avs_num_debugs();

/*
 * Read debug value
 *
 * - Input:
 *    debug, debug item of interest
 *    *pvalue, debug value to be filled
 *
 * - Output:
 *    *pvalue, filled with debug value
 *
 * - Return:
 *     Monitor error code (negative)
 */
int avs_debug_read(
    uint32_t debug,
    uint32_t *pvalue);

/*
 * Get number of sensors
 *
 * - Return:
 *     Number of sensors, or
 *     Monitor error code (negative)
 */
int avs_num_sensors();

/*
 * Read sensor value
 *
 * - Input:
 *    sensor, sensor of interest
 *    *pvalue, sensor value to be filled
 *
 * - Output:
 *    *pvalue, filled with sensor value
 *
 * - Return:
 *     Monitor error code (negative)
 */
int avs_sensor_read(
    uint32_t sensor,
    uint32_t *pvalue);

/*
 * Raw AVS cmd send function
 *
 * - Monitor DVFS handling is bypassed if this function is called directly;
 * - It is only supposed to be used by SCMI for non-CPU related AVS commands;
 */
#define AVS_MAX_PARAMS 10

int avs_cmd_send(
    uint32_t cmd,
    uint32_t *psta,
    uint32_t *ppar, size_t par_size,
    uint32_t *pres, size_t res_size);

/*
 * The following AVS data can be read directly from AVS mailbox registers
 * via AVS API rev1, before AVS driver is initialized and AVS API switches
 * to rev2 mode:
 *
 *    cpu_freq
 *    avs_rev
 */
int avs_rev1_avs_rev(
    uintptr_t mbox_base,
    uint32_t *prev);

int avs_rev1_cpu_freq(
    uintptr_t mbox_base,
    uint32_t *pfreq);

#endif /* _AVS_H_ */
