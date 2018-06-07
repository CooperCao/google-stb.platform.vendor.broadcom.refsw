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

#include <stdbool.h>

#include "monitor.h"
#include "delay.h"
#include "avs.h"
#include "avs_priv.h"

static volatile uint32_t *pmbox;
static volatile uint32_t *phost_l2;
static volatile uint32_t *pavs_l2;

static bool avs_rev1;
static bool cmd_pending;

static size_t num_cores;
static size_t num_debugs;
static size_t num_sensors;

static size_t num_cpu_pstates;
static uint32_t cpu_pstate_freqs[MAX_AVS_PSTATES];

/* Saved pmap for warm boot */
static bool saved_pmap_valid __boot_params;
static avs_pmap_t saved_pmap __boot_params;

#ifdef DEBUG
static void avs_dump()
{
    uint32_t pstate;

    DBG_PRINT("\n");
    DBG_PRINT("AVS num of cores: %d\n", (int)num_cores);
    DBG_PRINT("AVS num of debugs: %d\n", (int)num_debugs);
    DBG_PRINT("AVS num of sensors: %d\n", (int)num_sensors);
    DBG_PRINT("AVS num of CPU pstates: %d\n", (int)num_cpu_pstates);

    /* Loop through all possible CPU pstates */
    for (pstate = 0; pstate < num_cpu_pstates; pstate++)
        DBG_PRINT("  pstate %d: %d MHz\n", pstate, cpu_pstate_freqs[pstate]);
}
#endif

#define AVS_STA_SET_TIMEOUT 500000 /* Timeout = 500,000us = 500ms */
#define AVS_CMD_CLR_TIMEOUT 50000  /* Timeout = 50,000us = 50ms */
#define AVS_WAIT            10     /* Wait = 10us */

static inline int wait_sta_set(void)
{
    int retry = AVS_STA_SET_TIMEOUT / AVS_WAIT;

    while (retry--) {
        if (pmbox[AVS_STA])
            return 0;
        udelay(AVS_WAIT);
    }
    return MON_ETIMEDOUT;
}

static inline int wait_cmd_clr(void)
{
    int retry = AVS_CMD_CLR_TIMEOUT / AVS_WAIT;

    while (retry--) {
        if (!pmbox[AVS_CMD])
            return 0;
        udelay(AVS_WAIT);
    }
    return MON_ETIMEDOUT;
}

static int send_cmd(
    uint32_t cmd,
    uint32_t *ppar, size_t par_size,
    uint32_t *pres, size_t res_size,
    bool wait)
{
    size_t par_words = par_size / 4;
    size_t res_words = res_size / 4;
    size_t i;
    int ret;

    /* Wait for AVS to set status word for pending command */
    if (cmd_pending) {
        cmd_pending = false;
        wait_sta_set();
    }

    /* Start new command sequence */
    pmbox[AVS_STA] = 0;

    /* Write command parameters */
    for (i = 0; i < par_words; i++)
        pmbox[AVS_PAR + i] = *ppar++;

    /* Write command word */
    pmbox[AVS_CMD] = cmd;

    /* Generate AVS SW interrupt */
    pavs_l2[L2_SET] = AVS_L2_SW_INTR_MASK;

    /* Wait for AVS to clear command word */
    ret = wait_cmd_clr();

    if (ret)
        return ret;

    /* Return if NOT to wait for results */
    if (!wait) {
        cmd_pending = true;
        return MON_OK;
    }

    /* Wait for AVS to set status word */
    ret = wait_sta_set();

    if (ret)
        return ret;

    if (pmbox[AVS_STA] != AVS_STA_SUCCESS)
        return (pmbox[AVS_STA] == AVS_STA_NO_MAP) ? MON_ENOENT : MON_EIO;

    /* Read command results if needed */
    if (pres) {
        for (i = 0; i < res_words; i++)
            *pres++ = pmbox[AVS_PAR + i];
    }

    return MON_OK;
}

static inline int get_pmap(
    avs_pmap_t *pres)
{
    return send_cmd(
        AVS_CMD_GET_PMAP,
        NULL, 0,
        (uint32_t *)pres, sizeof(*pres),
        true);
}

static inline int set_pmap(
    avs_pmap_t *ppar)
{
    return send_cmd(
        AVS_CMD_SET_PMAP,
        (uint32_t *)ppar, sizeof(*ppar),
        NULL, 0,
        false);
}

static inline int get_pstate(
    avs_get_pstate_par_t *ppar,
    avs_get_pstate_res_t *pres)
{
    return send_cmd(
        AVS_CMD_GET_PSTATE,
        (uint32_t *)ppar, sizeof(*ppar),
        (uint32_t *)pres, sizeof(*pres),
        true);
}

static inline int set_pstate(
    avs_set_pstate_par_t *ppar)
{
    return send_cmd(
        AVS_CMD_SET_PSTATE,
        (uint32_t *)ppar, sizeof(*ppar),
        NULL, 0,
        false);
}

static inline int read_sensor(
    avs_read_sensor_par_t *ppar,
    avs_read_sensor_res_t *pres)
{
    return send_cmd(
        AVS_CMD_READ_SENSOR,
        (uint32_t *)ppar, sizeof(*ppar),
        (uint32_t *)pres, sizeof(*pres),
        true);
}

static inline int read_debug(
    avs_read_debug_par_t *ppar,
    avs_read_debug_res_t *pres)
{
    return send_cmd(
        AVS_CMD_READ_DEBUG,
        (uint32_t *)ppar, sizeof(*ppar),
        (uint32_t *)pres, sizeof(*pres),
        true);
}

static inline int calc_freq(
    avs_calc_freq_par_t *ppar,
    avs_calc_freq_res_t *pres)
{
    return send_cmd(
        AVS_CMD_CALC_FREQ,
        (uint32_t *)ppar, sizeof(*ppar),
        (uint32_t *)pres, sizeof(*pres),
        true);
}

static inline int get_cpu_pstate(uint32_t *ppstate)
{
    avs_get_cpu_pstate_par_t par;
    avs_get_cpu_pstate_res_t res;
    int ret;

    par.mbz = 0;
    ret = send_cmd(
        AVS_CMD_GET_PSTATE,
        (uint32_t *)&par, sizeof(par),
        (uint32_t *)&res, sizeof(res),
        true);

    if (ret)
        return ret;

    *ppstate = res.pstate;
    return MON_OK;
}

static inline int set_cpu_pstate(uint32_t pstate, bool wait)
{
    avs_set_cpu_pstate_par_t par;

    par.pstate = pstate;
    return send_cmd(
        AVS_CMD_SET_PSTATE,
        (uint32_t *)&par, sizeof(par),
        NULL, 0,
        wait);
}

static inline int calc_cpu_freq(uint32_t pstate, uint32_t *pfreq)
{
    avs_calc_freq_par_t par;
    avs_calc_freq_res_t res;
    int ret;

    *pfreq = 0;

    par.pstate = pstate;
    ret = calc_freq(&par, &res);

    if (ret)
        return ret;

    *pfreq = res.freq;
    return MON_OK;
}

#define DEFINE_READ_DEBUG_FUNC(_name, _debug)       \
static inline int read_##_name(uint32_t *pvalue)    \
{                                                   \
    avs_read_debug_par_t par;                       \
    avs_read_debug_res_t res;                       \
    int ret;                                        \
                                                    \
    *pvalue = 0;                                    \
                                                    \
    par.debug = AVS_DEBUG_##_debug;                 \
    ret = read_debug(&par, &res);                   \
                                                    \
    if (ret)                                        \
        return ret;                                 \
                                                    \
    *pvalue = res.value;                            \
    return MON_OK;                                  \
}

DEFINE_READ_DEBUG_FUNC(avs_rev, AVS_REV)
DEFINE_READ_DEBUG_FUNC(avs_state, AVS_STATE)
DEFINE_READ_DEBUG_FUNC(avs_heartbeat, AVS_HEARTBEAT)
DEFINE_READ_DEBUG_FUNC(avs_magic, AVS_MAGIC)

#define DEFINE_READ_SENSOR_FUNC(_name, _sensor)     \
static inline int read_##_name(uint32_t *pvalue)    \
{                                                   \
    avs_read_sensor_par_t par;                      \
    avs_read_sensor_res_t res;                      \
    int ret;                                        \
                                                    \
    *pvalue = 0;                                    \
                                                    \
    par.sensor = AVS_SENSOR_##_sensor;              \
    ret = read_sensor(&par, &res);                  \
                                                    \
    if (ret)                                        \
        return ret;                                 \
                                                    \
    *pvalue = res.value;                            \
    return MON_OK;                                  \
}

DEFINE_READ_SENSOR_FUNC(temp, TEMP);
DEFINE_READ_SENSOR_FUNC(volt_curr, VOLT_CURR);

static int init_pmap(void)
{
    avs_pmap_t pmap;
    int ret;

    /* Get pmap */
    ret = get_pmap(&pmap);

    /* Restore pmap if no pmap */
    if (ret == MON_ENOENT) {
        if (saved_pmap_valid) {
            ret = set_pmap(&saved_pmap);

            if (!ret) {
                DBG_MSG("Saved pmap restored");
                return MON_OK;
            }
        }
    }

    if (ret)
        return ret;

    /* Save pmap */
    saved_pmap = pmap;
    saved_pmap_valid = true;
    return MON_OK;
}

static int init_cpu_pstates(void)
{
    avs_get_pstate_par_t par;
    avs_get_pstate_res_t res;
    uint32_t pstate;
    uint32_t freq;
    int ret;

    /* Get current CPU pstate with associated info */
    par.core = AVS_CORE_CPU;
    ret = get_pstate(&par, &res);

    if (ret) {
        ERR_MSG("Failed to get current CPU pstate (%d)", ret);
        return ret;
    }

    /* Retrieve number of cores and CPU pstates */
    num_cores       = res.num_cores;
    num_cpu_pstates = res.num_pstates;

    /* Loop through all CPU pstates */
    for (pstate = 0; pstate < num_cpu_pstates; pstate++) {

        /* Calculate CPU frequency */
        ret = calc_cpu_freq(pstate, &freq);

        if (ret) {
            ERR_MSG("Failed to calculate CPU frequency (%d)", ret);
            return ret;
        }

        /* Record CPU frequency */
        cpu_pstate_freqs[pstate] = freq;
    }

    return MON_OK;
}

static int init_sensors(void)
{
    avs_read_sensor_par_t par;
    avs_read_sensor_res_t res;
    int ret;

    /* Get current temperature */
    par.sensor = AVS_SENSOR_TEMP;
    ret = read_sensor(&par, &res);

    if (ret) {
        ERR_MSG("Failed to get current temperature (%d)", ret);
        return ret;
    }

    /* Retrieve number of sensors */
    num_sensors = res.num_sensors;
    return MON_OK;
}

int avs_init(
    uintptr_t mbox_base,
    uintptr_t host_l2_base,
    uintptr_t avs_l2_base)
{
    uint32_t avs_magic;
    uint32_t avs_rev;
    uint32_t avs_state;
    int ret;

    ASSERT(mbox_base);
    ASSERT(host_l2_base);
    ASSERT(avs_l2_base);

    pmbox    = (uint32_t *)mbox_base;
    phost_l2 = (uint32_t *)host_l2_base;
    pavs_l2  = (uint32_t *)avs_l2_base;

    /* Check AVS magic */
#if AVS_FW_COMPATIBLE
    avs_magic = pmbox[AVS_MAGIC];
#else
    ret = read_avs_magic(&avs_magic);

    if (ret) {
        ERR_MSG("Failed to read AVS magic (%d)", ret);
    }
#endif
    ASSERT(avs_magic == AVS_MAGIC_NUM);

    /* Get AVS info */
#if AVS_FW_COMPATIBLE
    avs_rev = pmbox[AVS_REV];

    /* AVS major rev < 0x2 is not supported */
    /* Note: rev number may be coded in ASCII (e.g. 0x31='1') */
    if (((avs_rev >> 24) & 0x0f) < 0x2) {
        ERR_MSG("AVS revision %x not supported", avs_rev);
        avs_rev1 = true;
        return MON_ENOTSUPP;
    }
#else
    read_avs_rev(&avs_rev);
#endif
    read_avs_state(&avs_state);
    INFO_MSG("AVS rev: 0x%x, state: %x", avs_rev, avs_state);

    /* Init pmap */
    ret = init_pmap();

    if (ret) {
        ERR_MSG("Failed to initialize pmap (%d)", ret);
        return ret;
    }

    /* Init CPU pstates info */
    ret = init_cpu_pstates();

    if (ret) {
        ERR_MSG("Failed to initialize CPU pstates info (%d)", ret);
        return ret;
    }

    /* Init sensors info */
    ret = init_sensors();

    if (ret) {
        ERR_MSG("Failed to initialize sensors info (%d)", ret);
        return ret;
    }

    /* Init debugs info */
    num_debugs = AVS_DEBUG_LAST + 1;

#ifdef DEBUG
    avs_dump();
#endif
    return MON_OK;
}

int avs_cpu_pstate_get(uint32_t *ppstate)
{
    int ret;

    if (!ppstate)
        return MON_EINVAL;

    ret = get_cpu_pstate(ppstate);

    if (ret) {
        ERR_MSG("Failed to get CPU pstate (%d)", ret);
        return ret;
    }

    return MON_OK;
}

int avs_cpu_pstate_set(uint32_t pstate)
{
    int ret;

    if (pstate >= num_cpu_pstates)
        return MON_EINVAL;

    ret = set_cpu_pstate(pstate, false);

    if (ret) {
        ERR_MSG("Failed to set CPU pstate %d (%d)", pstate, ret);
        return ret;
    }

    return MON_OK;
}

int avs_cpu_pstate_freq(
    uint32_t pstate,
    uint32_t *pfreq)
{
    if (!pfreq)
        return MON_EINVAL;

    if (pstate >= num_cpu_pstates)
        return MON_EINVAL;

    *pfreq = cpu_pstate_freqs[pstate];
    return MON_OK;
}

int avs_cpu_pstate_freqs(
    size_t *pcount,
    uint32_t *pfreqs)
{
    uint32_t pstate;

    if (!pfreqs)
        return MON_EINVAL;

    if (*pcount > num_cpu_pstates)
        *pcount = num_cpu_pstates;

    for (pstate = 0; pstate < *pcount; pstate++)
        pfreqs[pstate] = cpu_pstate_freqs[pstate];

    return MON_OK;
}

int avs_num_pstates(uint32_t core)
{
    avs_get_pstate_par_t par;
    avs_get_pstate_res_t res;
    int ret;

    if (avs_rev1)
        return MON_ENOTSUPP;

    if (core >= num_cores)
        return MON_EINVAL;

    if (core == AVS_CORE_CPU)
        return num_cpu_pstates;

    par.core = core;
    ret = get_pstate(&par, &res);

    if (ret) {
        ERR_MSG("Failed to get pstate for core %d (%d)", core, ret);
        return ret;
    }

    return res.num_pstates;
}

int avs_get_pstate(
    uint32_t core,
    uint32_t *ppstate)
{
    avs_get_pstate_par_t par;
    avs_get_pstate_res_t res;
    int ret;

    if (avs_rev1)
        return MON_ENOTSUPP;

    if (!ppstate ||
        core >= num_cores)
        return MON_EINVAL;

    par.core = core;
    ret = get_pstate(&par, &res);

    if (ret) {
        ERR_MSG("Failed to get pstate for core %d (%d)", core, ret);
        return ret;
    }

    *ppstate = res.pstate;
    return MON_OK;
}

int avs_set_pstate(
    uint32_t core,
    uint32_t pstate)
{
    avs_set_pstate_par_t par;
    int ret;

    if (avs_rev1)
        return MON_ENOTSUPP;

    if (core >= num_cores)
        return MON_EINVAL;

    par.core = core;
    par.pstate = pstate;
    ret = set_pstate(&par);

    if (ret) {
        ERR_MSG("Failed to set pstate for core %d to %d (%d)", core, pstate, ret);
        return ret;
    }

    return MON_OK;
}

int avs_num_debugs()
{
    return num_debugs;
}

int avs_debug_read(
    uint32_t debug,
    uint32_t *pvalue)
{
    avs_read_debug_par_t par;
    avs_read_debug_res_t res;
    int ret;

    if (avs_rev1)
        return MON_ENOTSUPP;

    if (!pvalue ||
        debug >= num_debugs)
        return MON_EINVAL;

    par.debug = debug;
    ret = read_debug(&par, &res);

    if (ret) {
        ERR_MSG("Failed to read debug %d (%d)", debug, ret);
        return ret;
    }

    *pvalue = res.value;
    return MON_OK;
}

int avs_num_sensors()
{
    return num_sensors;
}

int avs_sensor_read(
    uint32_t sensor,
    uint32_t *pvalue)
{
    avs_read_sensor_par_t par;
    avs_read_sensor_res_t res;
    int ret;

    if (avs_rev1)
        return MON_ENOTSUPP;

    if (!pvalue ||
        sensor >= num_sensors)
        return MON_EINVAL;

    par.sensor = sensor;
    ret = read_sensor(&par, &res);

    if (ret) {
        ERR_MSG("Failed to read sensor %d (%d)", sensor, ret);
        return ret;
    }

    *pvalue = res.value;
    return MON_OK;
}

int avs_cmd_send(
    uint32_t cmd,
    uint32_t *psta,
    uint32_t *ppar, size_t par_size,
    uint32_t *pres, size_t res_size)
{
    int ret;

    if (avs_rev1)
        return MON_ENOTSUPP;

    if (!psta || !ppar || !pres ||
        par_size > sizeof(uint32_t) * AVS_PMAX ||
        res_size > sizeof(uint32_t) * AVS_PMAX)
        return MON_EINVAL;

    ret = send_cmd(
        cmd,
        ppar, par_size,
        pres, res_size,
        (cmd != AVS_CMD_SET_PSTATE) ? true : false);

    if (ret) {
        *psta = pmbox[AVS_STA];
        return ret;
    }

    *psta = AVS_STA_SUCCESS;
    return MON_OK;
}

int avs_rev1_avs_rev(
    uintptr_t mbox_base,
    uint32_t *prev)
{
    if (prev)
        *prev = MMIO32(mbox_base + sizeof(uint32_t) * AVS_DEBUG_AVS_REV);

    return MON_OK;
}

int avs_rev1_cpu_freq(
    uintptr_t mbox_base,
    uint32_t *pfreq)
{
    if (pfreq)
        *pfreq = MMIO32(mbox_base + sizeof(uint32_t) * AVS_DEBUG_CPU_FREQ);

    return MON_OK;
}
