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

#ifndef _AVS_PRIV_H_
#define _AVS_PRIV_H_

#define AVS_FW_COMPATIBLE 1

/* Word offsets of L2 interrupt registers */
#define L2_STA                          0
#define L2_SET                          1
#define L2_CLR                          2
#define L2_MASK_STA                     3
#define L2_MASK_SET                     4
#define L2_MASK_CLR                     5

/* AVS to host L2 interrupts */
#define HOST_L2_SW_INTR_SHIFT           26
#define HOST_L2_SW_INTR_MASK            (0x1 << HOST_L2_SW_INTR_SHIFT)

/* Host to AVS L2 interrupts */
#define AVS_L2_SW_INTR_SHIFT            31
#define AVS_L2_SW_INTR_MASK             (0x1 << AVS_L2_SW_INTR_SHIFT)

/* AVS magic number */
#define AVS_MAGIC_NUM                   0xA11600D1

/* Word offsets of AVS mailbox words */
#define AVS_CMD                         0
#define AVS_STA                         1
#define AVS_PAR                         6
#define AVS_P0                          (AVS_PAR + 0)
#define AVS_P1                          (AVS_PAR + 1)
#define AVS_P2                          (AVS_PAR + 2)
#define AVS_P3                          (AVS_PAR + 3)
#define AVS_P4                          (AVS_PAR + 4)
#define AVS_P5                          (AVS_PAR + 5)
#define AVS_P6                          (AVS_PAR + 6)
#define AVS_P7                          (AVS_PAR + 7)
#define AVS_P8                          (AVS_PAR + 8)
#define AVS_P9                          (AVS_PAR + 9)
#define AVS_PMAX                        10 /* max number of P-words */

#if AVS_FW_COMPATIBLE
#define AVS_REV                         10
#define AVS_HEARTBEAT                   12
#define AVS_MAGIC                       13
#endif

/* AVS commands */
#define AVS_CMD_GET_PMAP                0x30
#define AVS_CMD_SET_PMAP                0x31
#define AVS_CMD_GET_PSTATE              0x40
#define AVS_CMD_SET_PSTATE              0x41
#define AVS_CMD_READ_SENSOR             0x50
#define AVS_CMD_READ_DEBUG              0x51
#define AVS_CMD_CALC_FREQ               0x52

/* AVS status */
#define AVS_STA_IN_PROG                 0x00
#define AVS_STA_SUCCESS                 0xF0
#define AVS_STA_FAILURE                 0xFF
#define AVS_STA_INVALID                 0xF1
#define AVS_STA_NO_SUPPORT              0xF2
#define AVS_STA_NO_MAP                  0xF3
#define AVS_STA_MAP_SET                 0xF4

/* AVS cores */
#define AVS_CORE_CPU                    0

/* AVS sensors */
#define AVS_SENSOR_TEMP                 0
#define AVS_SENSOR_VOLT_CURR            1

/* AVS debug values */
#define AVS_DEBUG_AVS_REV               10
#define AVS_DEBUG_AVS_STATE             11
#define AVS_DEBUG_AVS_HEARTBEAT         12
#define AVS_DEBUG_AVS_MAGIC             13

#define AVS_DEBUG_CPU_FREQ              20
#define AVS_DEBUG_LAST                  AVS_DEBUG_CPU_FREQ

/*
 *  AVS commands and results
 */

/* struct avs_pmap is used in
 * - GET_P_MAP as results
 * - SET_P_MAP as parameters
 */
typedef struct avs_pmap {
    /* AVS_P0 */
    uint32_t avs_mode           : 8;
    uint32_t reserved_w0        : 24;

    /* AVS_P1 */
    uint32_t ndiv_int           : 8;
    uint32_t pdiv               : 8;
    uint32_t mdiv_p0            : 8;
    uint32_t reserved_w1        : 8;

    /* AVS_P2 */
    uint32_t mdiv_p1            : 8;
    uint32_t mdiv_p2            : 8;
    uint32_t mdiv_p3            : 8;
    uint32_t mdiv_p4            : 8;

    /* AVS_P3 */
    uint32_t init_pstate        : 8;
    uint32_t reserved_w3        : 24;

} avs_pmap_t;

typedef struct avs_get_pstate_par {
    /* AVS_P0 */
    uint32_t core;

} avs_get_pstate_par_t;

typedef struct avs_get_pstate_res {
    /* AVS_P0 */
    uint32_t pstate;

    /* AVS_P1 */
    uint32_t core               : 8;
    uint32_t num_pstates        : 8;
    uint32_t num_cores          : 8;
    uint32_t reserved           : 8;

} avs_get_pstate_res_t;

typedef struct avs_set_pstate_par {
    /* AVS_P0 */
    uint32_t pstate             : 8;
    uint32_t core               : 8;
    uint32_t num_clkregs        : 8;
    uint32_t reserved           : 8;

    /* AVS_P1/P2/P3 */
    uint32_t mdiv_addr;
    uint32_t mdiv_data;
    uint32_t mdiv_mask;

    /* AVS_P4/P5/P6 */
    uint32_t ndiv_addr;
    uint32_t ndiv_data;
    uint32_t ndiv_mask;

    /* AVS_P7/P8/P9 */
    uint32_t mux_addr;
    uint32_t mux_data;
    uint32_t mux_mask;

} avs_set_pstate_par_t;

typedef struct avs_read_sensor_par {
    /* AVS_P0 */
    uint32_t sensor;

} avs_read_sensor_par_t;

typedef struct avs_read_sensor_res {
    /* AVS_P0 */
    uint32_t sensor             : 8;
    uint32_t num_sensors        : 8;
    uint32_t reserved           : 16;

    /* AVS_P1 */
    union {
        uint32_t value;
        uint32_t temp;
        struct {
            uint32_t curr       : 16;
            uint32_t volt       : 16;
        };
    };

} avs_read_sensor_res_t;

typedef struct avs_read_debug_par {
    /* AVS_P0 */
    uint32_t debug;

} avs_read_debug_par_t;

typedef struct avs_read_debug_res {
    /* AVS_P0 */
    uint32_t debug;

    /* AVS_P1 */
    uint32_t value;

} avs_read_debug_res_t;

typedef struct avs_calc_freq_par {
    /* AVS_P0 */
    uint32_t pstate;

} avs_calc_freq_par_t;

typedef struct avs_calc_freq_res {
    /* AVS_P0 */
    uint32_t pstate;

    /* AVS_P1 */
    uint32_t freq;

} avs_calc_freq_res_t;

/*
 * The following commands and results structs
 * have been shortened for efficiency.
 */
typedef struct avs_get_cpu_pstate_par {
    /* AVS_P0 */
    uint32_t mbz; /* must be zero */

} avs_get_cpu_pstate_par_t;

typedef struct avs_get_cpu_pstate_res {
    /* AVS_P0 */
    uint32_t pstate;

} avs_get_cpu_pstate_res_t;

typedef struct avs_set_cpu_pstate_par {
    /* AVS_P0 */
    uint32_t pstate;

} avs_set_cpu_pstate_par_t;

#endif /* _AVS_PRIV_H_ */
