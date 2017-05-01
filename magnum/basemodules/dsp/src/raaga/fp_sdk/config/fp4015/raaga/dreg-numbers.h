/************************************************************************
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
 ************************************************************************/

/*
  AUTOMATICALLY GENERATED FILE.
  DO NOT EDIT

  Generated for
    - machine architecture: fp4015
    - binutils release: dev-443
    - FPDB release: /projects/firepath/implementation/releases/fpdb/r1577/octave (new format)
*/

#ifndef GENERATED_DREG_HEADER_H
#define GENERATED_DREG_HEADER_H

/*
  DREG list
*/

#define DIR_SHADOW_REG_0                           0
#define DIR_SHADOW_REG_1                           1
#define DIR_SHADOW_REG_2                           2
#define DIR_SHADOW_REG_3                           3
#define DIR_SHADOW_REG_4                           4
#define DIR_SHADOW_REG_5                           5
#define DIR_SHADOW_REG_6                           6
#define DIR_SHADOW_REG_7                           7
#define DIR_SHADOW_REG_8                           8
#define DIR_SHADOW_REG_9                           9
#define DIR_SHADOW_REG_10                         10
#define DIR_SHADOW_REG_11                         11
#define DIR_SHADOW_REG_12                         12
#define DIR_SHADOW_REG_13                         13
#define DIR_SHADOW_REG_14                         14
#define DIR_SHADOW_REG_15                         15
#define DIR_SHADOW_REG_16                         16
#define DIR_SHADOW_REG_SVC_RET                    16
#define DIR_SHADOW_REG_17                         17
#define DIR_SHADOW_REG_SVC_PSR                    17
#define DIR_SHADOW_REG_18                         18
#define DIR_SHADOW_REG_IRQ_RET                    18
#define DIR_SHADOW_REG_19                         19
#define DIR_SHADOW_REG_IRQ_PSR                    19
#define DIR_SHADOW_REG_20                         20
#define DIR_SHADOW_REG_SIRQ_RET                   20
#define DIR_SHADOW_REG_21                         21
#define DIR_SHADOW_REG_SIRQ_PSR                   21
#define DIR_SHADOW_REG_22                         22
#define DIR_SHADOW_REG_CB_RET                     22
#define DIR_SHADOW_REG_23                         23
#define DIR_SHADOW_REG_CB_PSR                     23
#define DIR_SHADOW_REG_24                         24
#define DIR_SHADOW_REG_VOM_RET                    24
#define DIR_SHADOW_REG_25                         25
#define DIR_SHADOW_REG_VOM_PSR                    25
#define DIR_SHADOW_REG_26                         26
#define DIR_SHADOW_REG_RECOV_RET                  26
#define DIR_SHADOW_REG_27                         27
#define DIR_SHADOW_REG_RECOV_PSR                  27
#define DIR_SHADOW_REG_28                         28
#define DIR_SHADOW_REG_DEBUG_RET                  28
#define DIR_SHADOW_REG_29                         29
#define DIR_SHADOW_REG_DEBUG_PSR                  29
#define DIR_SHADOW_REG_30                         30
#define DIR_SHADOW_REG_FATAL_RET                  30
#define DIR_SHADOW_REG_31                         31
#define DIR_SHADOW_REG_FATAL_PSR                  31
#define DIR_IRQC_RAW                              48
#define DIR_IRQC_LATCHED                          49
#define DIR_IRQC_ENABLE                           50
#define DIR_IRQC_CLEAR                            51
#define DIR_IRQC_SET                              52
#define DIR_LOOP_COUNT                            82
#define DIR_LOOP_MATCH                            83
#define DIR_LOOP_START                            84
#define DIR_SVC_VECTOR                            86
#define DIR_IRQ_VECTOR                            87
#define DIR_SIRQ_VECTOR                           88
#define DIR_VOM_VECTOR                            89
#define DIR_RECOV_VECTOR                          90
#define DIR_DEBUG_VECTOR                          91
#define DIR_FATAL_VECTOR                          92
#define DIR_RESET_VECTOR                         204
#define DIR_IRQE_RAW                              53
#define DIR_IRQE_LATCHED                          54
#define DIR_IRQE_ENABLE                           55
#define DIR_IRQE_CLEAR                            56
#define DIR_IRQE_SET                              57
#define DIR_SIRQ_RAW                              58
#define DIR_SIRQ_LATCHED                          59
#define DIR_SIRQ_ENABLE                           60
#define DIR_SIRQ_CLEAR                            61
#define DIR_SIRQ_SET                              62
#define DIR_RECOV_RAW                             63
#define DIR_RECOV_RAW                             63
#define DIR_RECOV_LATCHED                         64
#define DIR_RECOV_ENABLE                          65
#define DIR_RECOV_ENABLE                          65
#define DIR_RECOV_CLEAR                           66
#define DIR_RECOV_SET                             67
#define DIR_DEBUG_RAW                             68
#define DIR_DEBUG_LATCHED                         69
#define DIR_DEBUG_ENABLE                          70
#define DIR_DEBUG_CLEAR                           71
#define DIR_DEBUG_SET                             72
#define DIR_FATAL_RAW                             73
#define DIR_FATAL_RAW                             73
#define DIR_FATAL_LATCHED                         74
#define DIR_FATAL_ENABLE                          75
#define DIR_FATAL_ENABLE                          75
#define DIR_FATAL_CLEAR                           76
#define DIR_FATAL_SET                             77

/* total cycles since reset */
#define DIR_TOTC                                 192
#define DIR_TOTC_CMP1                            193
#define DIR_TOTC_CMP2                            194

/* scaled timer as per CRFIREPATH-871 */
#define DIR_APPC                                 196

/* scale factors for APPC timer */
#define DIR_APPC_CFG                             197

/* APPC comparator */
#define DIR_APPC_CMP1                            198

/* APPC comparator */
#define DIR_APPC_CMP2                            199

/* APPC delta */
#define DIR_APPC_D                               200

/* count unprivileged core cycles */
#define DIR_USRC                                 195
#define DIR_INSTRUCTION_SET_ID                   201
#define DIR_INSTRUCTION_SET_ID                   201
#define DIR_UARCH_CMD                             93
#define DIR_UARCH_CFG                             94
#define DIR_CORE_ID                              202
#define DIR_GP_CFG0                              218
#define DIR_DEBUG_CONTROL                         85
#define DIR_PROTD_STATUS_PC                      156
#define DIR_PROTD_STATUS_ADDR                    157
#define DIR_PROTD_STATUS_FAULT                   158
#define DIR_PROTD_STATUS_CLEAR                   159
#define DIR_BUSFAULT_STATUS_PC                   186
#define DIR_BUSFAULT_STATUS_ADDR                 187
#define DIR_BUSFAULT_STATUS_CLEAR                188
#define DIR_BPT0                                  32
#define DIR_BPT1                                  33
#define DIR_BPT2                                  34
#define DIR_BPT3                                  35
#define DIR_BPT4                                  36
#define DIR_BPT5                                  37
#define DIR_BPT6                                  38
#define DIR_BPT7                                  39
#define DIR_WP0_ADDR_BOT                         174
#define DIR_WP0_ADDR_TOP                         175
#define DIR_WP1_ADDR_BOT                         176
#define DIR_WP1_ADDR_TOP                         177
#define DIR_WP_CONTROL                           182
#define DIR_WP_STATUS_PC                         183
#define DIR_WP_STATUS_ADDR                       184
#define DIR_WP_STATUS_CLEAR                      185
#define DIR_ICACHE_CFG                            78
#define DIR_ICACHE_PARITY                         79
#define DIR_ICACHE_INVALIDATE_ADDR                80
#define DIR_ICACHE_INVALIDATE_WAY                 81
#define DIR_PPU_CONTROL                           95
#define DIR_PPU_STATUS                            96
#define DIR_PPU_COMMAND                           97
#define DIR_IPH_CFG                               98

/* ppu current counter 0 value */
#define DIR_PPU_COUNTER_0                        205

/* ppu current counter 1 value */
#define DIR_PPU_COUNTER_1                        206

/* ppu current counter 2 value */
#define DIR_PPU_COUNTER_2                        207

/* ppu current counter 3 value */
#define DIR_PPU_COUNTER_3                        208

/* ppu current counter 4 value */
#define DIR_PPU_COUNTER_4                        209

/* ppu current counter 5 value */
#define DIR_PPU_COUNTER_5                        210

/* ppu current counter 6 value */
#define DIR_PPU_COUNTER_6                        211

/* ppu current counter 7 value */
#define DIR_PPU_COUNTER_7                        212
#define DIR_PPU_AUTO_START                       213
#define DIR_PPU_AUTO_STOP                        214
#define DIR_PPU_SELECT                           215
#define DIR_IPH_PRNG                             216
#define DIR_IPH_SELECT                           217
#define DIR_PROTI_BOT_0                           99
#define DIR_PROTI_TOP_0                          100
#define DIR_PROTI_BOT_1                          101
#define DIR_PROTI_TOP_1                          102
#define DIR_PROTI_BOT_2                          103
#define DIR_PROTI_TOP_2                          104
#define DIR_PROTI_BOT_3                          105
#define DIR_PROTI_TOP_3                          106
#define DIR_PROTI_SBA_0                          107
#define DIR_PROTI_SBA_1                          108
#define DIR_PROTI_SBA_2                          109
#define DIR_PROTI_SBA_3                          110
#define DIR_PROTI_STATUS_FAULT                   111
#define DIR_PROTI_STATUS_CLEAR                   112
#define DIR_PROTD_BOT_0                          128
#define DIR_PROTD_TOP_0                          129
#define DIR_PROTD_BOT_1                          130
#define DIR_PROTD_TOP_1                          131
#define DIR_PROTD_BOT_2                          132
#define DIR_PROTD_TOP_2                          133
#define DIR_PROTD_BOT_3                          134
#define DIR_PROTD_TOP_3                          135
#define DIR_PROTD_BOT_4                          136
#define DIR_PROTD_TOP_4                          137
#define DIR_PROTD_BOT_5                          138
#define DIR_PROTD_TOP_5                          139
#define DIR_PROTD_BOT_6                          140
#define DIR_PROTD_TOP_6                          141
#define DIR_PROTD_BOT_7                          142
#define DIR_PROTD_TOP_7                          143
#define DIR_PROTD_BOT_8                          144
#define DIR_PROTD_TOP_8                          145
#define DIR_PROTD_BOT_9                          146
#define DIR_PROTD_TOP_9                          147
#define DIR_PROTD_BOT_10                         148
#define DIR_PROTD_TOP_10                         149
#define DIR_PROTD_BOT_11                         150
#define DIR_PROTD_TOP_11                         151
#define DIR_PROTDTCM_U                           152
#define DIR_PROTDTCM_P                           153
#define DIR_PROTIO_U                             154
#define DIR_PROTIO_P                             155


/*
  DREG bits list
*/

/* ADDR::ADDRESS: 64/32 bit address */
#define ADDR_ADDRESS_MASK                    0xffffffff
#define ADDR_ADDRESS_BIT0                    0
#define ADDR_ADDRESS_WIDTH                   32

/* LSU_PC_CMD::FAULT: Write 1 to this bit to clear the fault */
#define LSU_PC_CMD_FAULT_MASK                0x1
#define LSU_PC_CMD_FAULT_BIT                 0

/* PROT_CFG::ST_FAULT_ENABLE_0: Enable faulting on stores for region 0 */
#define PROT_CFG_ST_FAULT_ENABLE_0_MASK      0x1
#define PROT_CFG_ST_FAULT_ENABLE_0_BIT       0

/* PROT_CFG::LD_FAULT_ENABLE_0: Enable faulting on loads  for region 0 */
#define PROT_CFG_LD_FAULT_ENABLE_0_MASK      0x2
#define PROT_CFG_LD_FAULT_ENABLE_0_BIT       1

/* PROT_CFG::ST_FAULT_ENABLE_1: Enable faulting on stores for region 1 */
#define PROT_CFG_ST_FAULT_ENABLE_1_MASK      0x4
#define PROT_CFG_ST_FAULT_ENABLE_1_BIT       2

/* PROT_CFG::LD_FAULT_ENABLE_1: Enable faulting on loads  for region 1 */
#define PROT_CFG_LD_FAULT_ENABLE_1_MASK      0x8
#define PROT_CFG_LD_FAULT_ENABLE_1_BIT       3

/* PROT_CFG::ST_FAULT_ENABLE_2: Enable faulting on stores for region 2 */
#define PROT_CFG_ST_FAULT_ENABLE_2_MASK      0x10
#define PROT_CFG_ST_FAULT_ENABLE_2_BIT       4

/* PROT_CFG::LD_FAULT_ENABLE_2: Enable faulting on loads  for region 2 */
#define PROT_CFG_LD_FAULT_ENABLE_2_MASK      0x20
#define PROT_CFG_LD_FAULT_ENABLE_2_BIT       5

/* PROT_CFG::ST_FAULT_ENABLE_3: Enable faulting on stores for region 3 */
#define PROT_CFG_ST_FAULT_ENABLE_3_MASK      0x40
#define PROT_CFG_ST_FAULT_ENABLE_3_BIT       6

/* PROT_CFG::LD_FAULT_ENABLE_3: Enable faulting on loads  for region 3 */
#define PROT_CFG_LD_FAULT_ENABLE_3_MASK      0x80
#define PROT_CFG_LD_FAULT_ENABLE_3_BIT       7

/* WP_CFG::ST_FAULT_ENABLE_0: Enable faulting on stores for watchpoint 0 */
#define WP_CFG_ST_FAULT_ENABLE_0_MASK        0x1
#define WP_CFG_ST_FAULT_ENABLE_0_BIT         0

/* WP_CFG::LD_FAULT_ENABLE_0: Enable faulting on loads  for watchpoint 0 */
#define WP_CFG_LD_FAULT_ENABLE_0_MASK        0x2
#define WP_CFG_LD_FAULT_ENABLE_0_BIT         1

/* WP_CFG::ST_FAULT_ENABLE_1: Enable faulting on stores for watchpoint 1 */
#define WP_CFG_ST_FAULT_ENABLE_1_MASK        0x4
#define WP_CFG_ST_FAULT_ENABLE_1_BIT         2

/* WP_CFG::LD_FAULT_ENABLE_1: Enable faulting on loads  for watchpoint 1 */
#define WP_CFG_LD_FAULT_ENABLE_1_MASK        0x8
#define WP_CFG_LD_FAULT_ENABLE_1_BIT         3

/* WP_CFG::ST_FAULT_ENABLE_2: Enable faulting on stores for watchpoint 2 */
#define WP_CFG_ST_FAULT_ENABLE_2_MASK        0x10
#define WP_CFG_ST_FAULT_ENABLE_2_BIT         4

/* WP_CFG::LD_FAULT_ENABLE_2: Enable faulting on loads  for watchpoint 2 */
#define WP_CFG_LD_FAULT_ENABLE_2_MASK        0x20
#define WP_CFG_LD_FAULT_ENABLE_2_BIT         5

/* WP_CFG::ST_FAULT_ENABLE_3: Enable faulting on stores for watchpoint 3 */
#define WP_CFG_ST_FAULT_ENABLE_3_MASK        0x40
#define WP_CFG_ST_FAULT_ENABLE_3_BIT         6

/* WP_CFG::LD_FAULT_ENABLE_3: Enable faulting on loads  for watchpoint 3 */
#define WP_CFG_LD_FAULT_ENABLE_3_MASK        0x80
#define WP_CFG_LD_FAULT_ENABLE_3_BIT         7

/* PROTD_BOT::PROT_USER_READ: Enable user-mode loads for range */
#define PROTD_BOT_PROT_USER_READ_MASK        0x1
#define PROTD_BOT_PROT_USER_READ_BIT         0

/* PROTD_BOT::PROT_USER_WRITE: Enable user-mode stores for range */
#define PROTD_BOT_PROT_USER_WRITE_MASK       0x2
#define PROTD_BOT_PROT_USER_WRITE_BIT        1

/* PROTD_BOT::PROT_PRIV_READ: Enable priv-mode loads for range */
#define PROTD_BOT_PROT_PRIV_READ_MASK        0x4
#define PROTD_BOT_PROT_PRIV_READ_BIT         2

/* PROTD_BOT::PROT_PRIV_WRITE: Enable priv-mode stores for range */
#define PROTD_BOT_PROT_PRIV_WRITE_MASK       0x8
#define PROTD_BOT_PROT_PRIV_WRITE_BIT        3

/* PROTD_BOT::ADDRESS: Base address of protection range */
#define PROTD_BOT_ADDRESS_MASK               0xffffffe0
#define PROTD_BOT_ADDRESS_BIT0               5
#define PROTD_BOT_ADDRESS_WIDTH              27

/* PROTD_TOP::ADDRESS: Top address of protection range */
#define PROTD_TOP_ADDRESS_MASK               0xffffffe0
#define PROTD_TOP_ADDRESS_BIT0               5
#define PROTD_TOP_ADDRESS_WIDTH              27

/* PROTI_BOT::PROT_USER_EXEC: Enable user-mode execution for range */
#define PROTI_BOT_PROT_USER_EXEC_MASK        0x1
#define PROTI_BOT_PROT_USER_EXEC_BIT         0

/* PROTI_BOT::PROT_PRIV_EXEC: Enable priv-mode execution for range */
#define PROTI_BOT_PROT_PRIV_EXEC_MASK        0x2
#define PROTI_BOT_PROT_PRIV_EXEC_BIT         1

/* PROTI_BOT::ADDRESS: Base address of execution range */
#define PROTI_BOT_ADDRESS_MASK               0xffffffe0
#define PROTI_BOT_ADDRESS_BIT0               5
#define PROTI_BOT_ADDRESS_WIDTH              27

/* PROTI_TOP::ADDRESS: Top address of execution range */
#define PROTI_TOP_ADDRESS_MASK               0xffffffe0
#define PROTI_TOP_ADDRESS_BIT0               5
#define PROTI_TOP_ADDRESS_WIDTH              27

/* PROTISTAT::FAULT_I: Instruction fetch protection fault: 0 for no fault, 1 for fault */
#define PROTISTAT_FAULT_I_MASK               0x1
#define PROTISTAT_FAULT_I_BIT                0

/* PROTISTAT::FAULT_OOR_I: Instruction out-of-range protection fault: 0 for no fault, 1 for fault */
#define PROTISTAT_FAULT_OOR_I_MASK           0x2
#define PROTISTAT_FAULT_OOR_I_BIT            1

/* PROTICMD::FAULT: Instruction fetch protection fault: write 1 to clear PROTI_STATUS_FAULT */
#define PROTICMD_FAULT_MASK                  0x1
#define PROTICMD_FAULT_BIT                   0

/* PROTDSTAT::FAULT_D: Data protection fault: 0 for no fault, 1 for fault */
#define PROTDSTAT_FAULT_D_MASK               0x1
#define PROTDSTAT_FAULT_D_BIT                0

/* PROTDSTAT::FAULT_DTCM: DTCM protection fault: 0 for no fault, 1 for fault */
#define PROTDSTAT_FAULT_DTCM_MASK            0x2
#define PROTDSTAT_FAULT_DTCM_BIT             1

/* PROTDSTAT::FAULT_IO: IO protection fault: 0 for no fault, 1 for fault */
#define PROTDSTAT_FAULT_IO_MASK              0x4
#define PROTDSTAT_FAULT_IO_BIT               2

/* PROTDSTAT::FAULT_OOR: Out-of-range protection fault: 0 for no fault, 1 for fault */
#define PROTDSTAT_FAULT_OOR_MASK             0x8
#define PROTDSTAT_FAULT_OOR_BIT              3

/* PROTDCMD::FAULT: Data protection fault: write 1 to clear PROTD_STATUS_FAULT/PC/ADDR */
#define PROTDCMD_FAULT_MASK                  0x1
#define PROTDCMD_FAULT_BIT                   0

/* PROTMAP::PROT_READ_0: Enable loads for range 0 */
#define PROTMAP_PROT_READ_0_MASK             0x1
#define PROTMAP_PROT_READ_0_BIT              0

/* PROTMAP::PROT_WRITE_0: Enable stores for range 0 */
#define PROTMAP_PROT_WRITE_0_MASK            0x2
#define PROTMAP_PROT_WRITE_0_BIT             1

/* PROTMAP::PROT_READ_1: Enable loads for range 1 */
#define PROTMAP_PROT_READ_1_MASK             0x4
#define PROTMAP_PROT_READ_1_BIT              2

/* PROTMAP::PROT_WRITE_1: Enable stores for range 1 */
#define PROTMAP_PROT_WRITE_1_MASK            0x8
#define PROTMAP_PROT_WRITE_1_BIT             3

/* PROTMAP::PROT_READ_2: Enable loads for range 2 */
#define PROTMAP_PROT_READ_2_MASK             0x10
#define PROTMAP_PROT_READ_2_BIT              4

/* PROTMAP::PROT_WRITE_2: Enable stores for range 2 */
#define PROTMAP_PROT_WRITE_2_MASK            0x20
#define PROTMAP_PROT_WRITE_2_BIT             5

/* PROTMAP::PROT_READ_3: Enable loads for range 3 */
#define PROTMAP_PROT_READ_3_MASK             0x40
#define PROTMAP_PROT_READ_3_BIT              6

/* PROTMAP::PROT_WRITE_3: Enable stores for range 3 */
#define PROTMAP_PROT_WRITE_3_MASK            0x80
#define PROTMAP_PROT_WRITE_3_BIT             7

/* PROTMAP::PROT_READ_4: Enable loads for range 4 */
#define PROTMAP_PROT_READ_4_MASK             0x100
#define PROTMAP_PROT_READ_4_BIT              8

/* PROTMAP::PROT_WRITE_4: Enable stores for range 4 */
#define PROTMAP_PROT_WRITE_4_MASK            0x200
#define PROTMAP_PROT_WRITE_4_BIT             9

/* PROTMAP::PROT_READ_5: Enable loads for range 5 */
#define PROTMAP_PROT_READ_5_MASK             0x400
#define PROTMAP_PROT_READ_5_BIT              10

/* PROTMAP::PROT_WRITE_5: Enable stores for range 5 */
#define PROTMAP_PROT_WRITE_5_MASK            0x800
#define PROTMAP_PROT_WRITE_5_BIT             11

/* PROTMAP::PROT_READ_6: Enable loads for range 6 */
#define PROTMAP_PROT_READ_6_MASK             0x1000
#define PROTMAP_PROT_READ_6_BIT              12

/* PROTMAP::PROT_WRITE_6: Enable stores for range 6 */
#define PROTMAP_PROT_WRITE_6_MASK            0x2000
#define PROTMAP_PROT_WRITE_6_BIT             13

/* PROTMAP::PROT_READ_7: Enable loads for range 7 */
#define PROTMAP_PROT_READ_7_MASK             0x4000
#define PROTMAP_PROT_READ_7_BIT              14

/* PROTMAP::PROT_WRITE_7: Enable stores for range 7 */
#define PROTMAP_PROT_WRITE_7_MASK            0x8000
#define PROTMAP_PROT_WRITE_7_BIT             15

/* PROTMAP::PROT_READ_8: Enable loads for range 8 */
#define PROTMAP_PROT_READ_8_MASK             0x10000
#define PROTMAP_PROT_READ_8_BIT              16

/* PROTMAP::PROT_WRITE_8: Enable stores for range 8 */
#define PROTMAP_PROT_WRITE_8_MASK            0x20000
#define PROTMAP_PROT_WRITE_8_BIT             17

/* PROTMAP::PROT_READ_9: Enable loads for range 9 */
#define PROTMAP_PROT_READ_9_MASK             0x40000
#define PROTMAP_PROT_READ_9_BIT              18

/* PROTMAP::PROT_WRITE_9: Enable stores for range 9 */
#define PROTMAP_PROT_WRITE_9_MASK            0x80000
#define PROTMAP_PROT_WRITE_9_BIT             19

/* PROTMAP::PROT_READ_10: Enable loads for range 10 */
#define PROTMAP_PROT_READ_10_MASK            0x100000
#define PROTMAP_PROT_READ_10_BIT             20

/* PROTMAP::PROT_WRITE_10: Enable stores for range 10 */
#define PROTMAP_PROT_WRITE_10_MASK           0x200000
#define PROTMAP_PROT_WRITE_10_BIT            21

/* PROTMAP::PROT_READ_11: Enable loads for range 11 */
#define PROTMAP_PROT_READ_11_MASK            0x400000
#define PROTMAP_PROT_READ_11_BIT             22

/* PROTMAP::PROT_WRITE_11: Enable stores for range 11 */
#define PROTMAP_PROT_WRITE_11_MASK           0x800000
#define PROTMAP_PROT_WRITE_11_BIT            23

/* PROTMAP::PROT_READ_12: Enable loads for range 12 */
#define PROTMAP_PROT_READ_12_MASK            0x1000000
#define PROTMAP_PROT_READ_12_BIT             24

/* PROTMAP::PROT_WRITE_12: Enable stores for range 12 */
#define PROTMAP_PROT_WRITE_12_MASK           0x2000000
#define PROTMAP_PROT_WRITE_12_BIT            25

/* PROTMAP::PROT_READ_13: Enable loads for range 13 */
#define PROTMAP_PROT_READ_13_MASK            0x4000000
#define PROTMAP_PROT_READ_13_BIT             26

/* PROTMAP::PROT_WRITE_13: Enable stores for range 13 */
#define PROTMAP_PROT_WRITE_13_MASK           0x8000000
#define PROTMAP_PROT_WRITE_13_BIT            27

/* PROTMAP::PROT_READ_14: Enable loads for range 14 */
#define PROTMAP_PROT_READ_14_MASK            0x10000000
#define PROTMAP_PROT_READ_14_BIT             28

/* PROTMAP::PROT_WRITE_14: Enable stores for range 14 */
#define PROTMAP_PROT_WRITE_14_MASK           0x20000000
#define PROTMAP_PROT_WRITE_14_BIT            29

/* PROTMAP::PROT_READ_15: Enable loads for range 15 */
#define PROTMAP_PROT_READ_15_MASK            0x40000000
#define PROTMAP_PROT_READ_15_BIT             30

/* PROTMAP::PROT_WRITE_15: Enable stores for range 15 */
#define PROTMAP_PROT_WRITE_15_MASK           0x80000000
#define PROTMAP_PROT_WRITE_15_BIT            31

/* PROTMAP::PROT_READ_16: Enable loads for range 16 */
#define PROTMAP_PROT_READ_16_MASK            0x100000000
#define PROTMAP_PROT_READ_16_BIT             32

/* PROTMAP::PROT_WRITE_16: Enable stores for range 16 */
#define PROTMAP_PROT_WRITE_16_MASK           0x200000000
#define PROTMAP_PROT_WRITE_16_BIT            33

/* PROTMAP::PROT_READ_17: Enable loads for range 17 */
#define PROTMAP_PROT_READ_17_MASK            0x400000000
#define PROTMAP_PROT_READ_17_BIT             34

/* PROTMAP::PROT_WRITE_17: Enable stores for range 17 */
#define PROTMAP_PROT_WRITE_17_MASK           0x800000000
#define PROTMAP_PROT_WRITE_17_BIT            35

/* PROTMAP::PROT_READ_18: Enable loads for range 18 */
#define PROTMAP_PROT_READ_18_MASK            0x1000000000
#define PROTMAP_PROT_READ_18_BIT             36

/* PROTMAP::PROT_WRITE_18: Enable stores for range 18 */
#define PROTMAP_PROT_WRITE_18_MASK           0x2000000000
#define PROTMAP_PROT_WRITE_18_BIT            37

/* PROTMAP::PROT_READ_19: Enable loads for range 19 */
#define PROTMAP_PROT_READ_19_MASK            0x4000000000
#define PROTMAP_PROT_READ_19_BIT             38

/* PROTMAP::PROT_WRITE_19: Enable stores for range 19 */
#define PROTMAP_PROT_WRITE_19_MASK           0x8000000000
#define PROTMAP_PROT_WRITE_19_BIT            39

/* PROTMAP::PROT_READ_20: Enable loads for range 20 */
#define PROTMAP_PROT_READ_20_MASK            0x10000000000
#define PROTMAP_PROT_READ_20_BIT             40

/* PROTMAP::PROT_WRITE_20: Enable stores for range 20 */
#define PROTMAP_PROT_WRITE_20_MASK           0x20000000000
#define PROTMAP_PROT_WRITE_20_BIT            41

/* PROTMAP::PROT_READ_21: Enable loads for range 21 */
#define PROTMAP_PROT_READ_21_MASK            0x40000000000
#define PROTMAP_PROT_READ_21_BIT             42

/* PROTMAP::PROT_WRITE_21: Enable stores for range 21 */
#define PROTMAP_PROT_WRITE_21_MASK           0x80000000000
#define PROTMAP_PROT_WRITE_21_BIT            43

/* PROTMAP::PROT_READ_22: Enable loads for range 22 */
#define PROTMAP_PROT_READ_22_MASK            0x100000000000
#define PROTMAP_PROT_READ_22_BIT             44

/* PROTMAP::PROT_WRITE_22: Enable stores for range 22 */
#define PROTMAP_PROT_WRITE_22_MASK           0x200000000000
#define PROTMAP_PROT_WRITE_22_BIT            45

/* PROTMAP::PROT_READ_23: Enable loads for range 23 */
#define PROTMAP_PROT_READ_23_MASK            0x400000000000
#define PROTMAP_PROT_READ_23_BIT             46

/* PROTMAP::PROT_WRITE_23: Enable stores for range 23 */
#define PROTMAP_PROT_WRITE_23_MASK           0x800000000000
#define PROTMAP_PROT_WRITE_23_BIT            47

/* PROTMAP::PROT_READ_24: Enable loads for range 24 */
#define PROTMAP_PROT_READ_24_MASK            0x1000000000000
#define PROTMAP_PROT_READ_24_BIT             48

/* PROTMAP::PROT_WRITE_24: Enable stores for range 24 */
#define PROTMAP_PROT_WRITE_24_MASK           0x2000000000000
#define PROTMAP_PROT_WRITE_24_BIT            49

/* PROTMAP::PROT_READ_25: Enable loads for range 25 */
#define PROTMAP_PROT_READ_25_MASK            0x4000000000000
#define PROTMAP_PROT_READ_25_BIT             50

/* PROTMAP::PROT_WRITE_25: Enable stores for range 25 */
#define PROTMAP_PROT_WRITE_25_MASK           0x8000000000000
#define PROTMAP_PROT_WRITE_25_BIT            51

/* PROTMAP::PROT_READ_26: Enable loads for range 26 */
#define PROTMAP_PROT_READ_26_MASK            0x10000000000000
#define PROTMAP_PROT_READ_26_BIT             52

/* PROTMAP::PROT_WRITE_26: Enable stores for range 26 */
#define PROTMAP_PROT_WRITE_26_MASK           0x20000000000000
#define PROTMAP_PROT_WRITE_26_BIT            53

/* PROTMAP::PROT_READ_27: Enable loads for range 27 */
#define PROTMAP_PROT_READ_27_MASK            0x40000000000000
#define PROTMAP_PROT_READ_27_BIT             54

/* PROTMAP::PROT_WRITE_27: Enable stores for range 27 */
#define PROTMAP_PROT_WRITE_27_MASK           0x80000000000000
#define PROTMAP_PROT_WRITE_27_BIT            55

/* PROTMAP::PROT_READ_28: Enable loads for range 28 */
#define PROTMAP_PROT_READ_28_MASK            0x100000000000000
#define PROTMAP_PROT_READ_28_BIT             56

/* PROTMAP::PROT_WRITE_28: Enable stores for range 28 */
#define PROTMAP_PROT_WRITE_28_MASK           0x200000000000000
#define PROTMAP_PROT_WRITE_28_BIT            57

/* PROTMAP::PROT_READ_29: Enable loads for range 29 */
#define PROTMAP_PROT_READ_29_MASK            0x400000000000000
#define PROTMAP_PROT_READ_29_BIT             58

/* PROTMAP::PROT_WRITE_29: Enable stores for range 29 */
#define PROTMAP_PROT_WRITE_29_MASK           0x800000000000000
#define PROTMAP_PROT_WRITE_29_BIT            59

/* PROTMAP::PROT_READ_30: Enable loads for range 30 */
#define PROTMAP_PROT_READ_30_MASK            0x1000000000000000
#define PROTMAP_PROT_READ_30_BIT             60

/* PROTMAP::PROT_WRITE_30: Enable stores for range 30 */
#define PROTMAP_PROT_WRITE_30_MASK           0x2000000000000000
#define PROTMAP_PROT_WRITE_30_BIT            61

/* PROTMAP::PROT_READ_31: Enable loads for range 31 */
#define PROTMAP_PROT_READ_31_MASK            0x4000000000000000
#define PROTMAP_PROT_READ_31_BIT             62

/* PROTMAP::PROT_WRITE_31: Enable stores for range 31 */
#define PROTMAP_PROT_WRITE_31_MASK           0x8000000000000000
#define PROTMAP_PROT_WRITE_31_BIT            63

/* ICACHE_CFG::LOCK: Lock icache way 0-3 */
#define ICACHE_CFG_LOCK_MASK                 0xf0000
#define ICACHE_CFG_LOCK_BIT0                 16
#define ICACHE_CFG_LOCK_WIDTH                4

/* ICACHE_CFG::PREFETCH: Enables prefetching of the line following a cache miss */
#define ICACHE_CFG_PREFETCH_MASK             0x1
#define ICACHE_CFG_PREFETCH_BIT              0

/* ICACHE_PARITY::ENABLE: Enables parity checking */
#define ICACHE_PARITY_ENABLE_MASK            0x1
#define ICACHE_PARITY_ENABLE_BIT             0

/* ICACHE_INVALIDATE_ADDR::PC: Invalidate the set that contains the given address in the instruction cache */
#define ICACHE_INVALIDATE_ADDR_PC_MASK       0xffffffe0
#define ICACHE_INVALIDATE_ADDR_PC_BIT0       5
#define ICACHE_INVALIDATE_ADDR_PC_WIDTH      27

/* ICACHE_INVALIDATE_WAY::CMD: Invalidate icache way 0-3 - invalidate the specified set if 0 */
#define ICACHE_INVALIDATE_WAY_CMD_MASK       0xf
#define ICACHE_INVALIDATE_WAY_CMD_BIT0       0
#define ICACHE_INVALIDATE_WAY_CMD_WIDTH      4

/* APPC_CFG::APPC_M: Numerator of APPC scale factor (Octave only) */
#define APPC_CFG_APPC_M_MASK                 0xfff
#define APPC_CFG_APPC_M_BIT0                 0
#define APPC_CFG_APPC_M_WIDTH                12

/* APPC_CFG::APPC_N: Denominator of APPC scale factor (Octave only) */
#define APPC_CFG_APPC_N_MASK                 0xfff0000
#define APPC_CFG_APPC_N_BIT0                 16
#define APPC_CFG_APPC_N_WIDTH                12

/* APPC_DELTA::APPC_D: APPC delta (Octave only) */
#define APPC_DELTA_APPC_D_MASK               0x1fff
#define APPC_DELTA_APPC_D_BIT0               0
#define APPC_DELTA_APPC_D_WIDTH              13

/* INSTRUCTION_SET_ID::FEATURE: Feature flags */
#define INSTRUCTION_SET_ID_FEATURE_MASK      0xff00
#define INSTRUCTION_SET_ID_FEATURE_BIT0      8
#define INSTRUCTION_SET_ID_FEATURE_WIDTH     8

/* INSTRUCTION_SET_ID::VERSION: ISA version */
#define INSTRUCTION_SET_ID_VERSION_MASK      0xff
#define INSTRUCTION_SET_ID_VERSION_BIT0      0
#define INSTRUCTION_SET_ID_VERSION_WIDTH     8

/* INSTRUCTION_SET_ID::GENERATION: ISA generation */
#define INSTRUCTION_SET_ID_GENERATION_MASK   0xff0000
#define INSTRUCTION_SET_ID_GENERATION_BIT0   16
#define INSTRUCTION_SET_ID_GENERATION_WIDTH  8

/* CORE_ID::ID: Core ID, decimal value between 0 and 31, from pinstraps */
#define CORE_ID_ID_MASK                      0x1f
#define CORE_ID_ID_BIT0                      0
#define CORE_ID_ID_WIDTH                     5

/* IRQC::TOTC_CMP1: Core timer comparison event 1 */
#define IRQC_TOTC_CMP1_MASK                  0x1
#define IRQC_TOTC_CMP1_BIT                   0

/* IRQC::TOTC_CMP2: Core timer comparison event 2 */
#define IRQC_TOTC_CMP2_MASK                  0x2
#define IRQC_TOTC_CMP2_BIT                   1

/* IRQC::APPC_CMP1: Application timer comparison event 1 */
#define IRQC_APPC_CMP1_MASK                  0x4
#define IRQC_APPC_CMP1_BIT                   2

/* IRQC::APPC_CMP2: Application timer comparison event 2 */
#define IRQC_APPC_CMP2_MASK                  0x8
#define IRQC_APPC_CMP2_BIT                   3

/* IRQC::EXT_SW: External software interrupts */
#define IRQC_EXT_SW_MASK                     0xff0
#define IRQC_EXT_SW_BIT0                     4
#define IRQC_EXT_SW_WIDTH                    8

/* IRQC::INT_SW: Internal software interrupts */
#define IRQC_INT_SW_MASK                     0xff000
#define IRQC_INT_SW_BIT0                     12
#define IRQC_INT_SW_WIDTH                    8

/* IRQE::BITS: External interrupts */
#define IRQE_BITS_MASK                       0xffffffffffffffff
#define IRQE_BITS_BIT0                       0
#define IRQE_BITS_WIDTH                      64

/* LOOP_MATCH::MATCH: Loop match */
#define LOOP_MATCH_MATCH_MASK                0xfffffffc
#define LOOP_MATCH_MATCH_BIT0                2
#define LOOP_MATCH_MATCH_WIDTH               30

/* LOOP_START::START: Loop start */
#define LOOP_START_START_MASK                0xfffffffc
#define LOOP_START_START_BIT0                2
#define LOOP_START_START_WIDTH               30

/* LOOP_CTR::CTR: Loop count */
#define LOOP_CTR_CTR_MASK                    0xffffffff
#define LOOP_CTR_CTR_BIT0                    0
#define LOOP_CTR_CTR_WIDTH                   32

/* FATAL::MISALIGNED_PC: Misaligned pc */
#define FATAL_MISALIGNED_PC_MASK             0x1
#define FATAL_MISALIGNED_PC_BIT              0

/* FATAL::INVALID_INSN: Invalid instruction detected */
#define FATAL_INVALID_INSN_MASK              0x2
#define FATAL_INVALID_INSN_BIT               1

/* FATAL::PRIVILEGE: Privilege fault (PUTPSR/DIRR/DIRS) */
#define FATAL_PRIVILEGE_MASK                 0x4
#define FATAL_PRIVILEGE_BIT                  2

/* FATAL::ASYNC_PRIVILEGE: Asynchronous privilege fault (DIR{R,W,S} DC{FLUSH,PURGE}CB) */
#define FATAL_ASYNC_PRIVILEGE_MASK           0x8
#define FATAL_ASYNC_PRIVILEGE_BIT            3

/* FATAL::PROTECTION_VIOLATION: Protection fault */
#define FATAL_PROTECTION_VIOLATION_MASK      0x10
#define FATAL_PROTECTION_VIOLATION_BIT       4

/* FATAL::OBUS_ACCESS_INVALID: Formerly known as bus error */
#define FATAL_OBUS_ACCESS_INVALID_MASK       0x20
#define FATAL_OBUS_ACCESS_INVALID_BIT        5

/* FATAL::INVALID_DIR: Invalid dreg number in (DIRR/DIRS) */
#define FATAL_INVALID_DIR_MASK               0x40
#define FATAL_INVALID_DIR_BIT                6

/* FATAL::MISALIGNED_CALLBACK: Misaligned address provided to DC{FLUSH,PURGE}CB (NOT IMPLEMENTED IN OCTv1) */
#define FATAL_MISALIGNED_CALLBACK_MASK       0x80
#define FATAL_MISALIGNED_CALLBACK_BIT        7

/* FATAL::EXT_SW: From SW writable subsystem register */
#define FATAL_EXT_SW_MASK                    0x300
#define FATAL_EXT_SW_BIT0                    8
#define FATAL_EXT_SW_WIDTH                   2

/* FATAL::EXT_HW: From core/subsystem pins */
#define FATAL_EXT_HW_MASK                    0xc00
#define FATAL_EXT_HW_BIT0                    10
#define FATAL_EXT_HW_WIDTH                   2

/* SIRQ::BITS: External super-interrupts */
#define SIRQ_BITS_MASK                       0xff
#define SIRQ_BITS_BIT0                       0
#define SIRQ_BITS_WIDTH                      8

/* SIRQ::EXT_SW: External software super-interrupts */
#define SIRQ_EXT_SW_MASK                     0x300
#define SIRQ_EXT_SW_BIT0                     8
#define SIRQ_EXT_SW_WIDTH                    2

/* RECOV::PARITY: I-mem parity interupt */
#define RECOV_PARITY_MASK                    0x1
#define RECOV_PARITY_BIT                     0

/* RECOV::WATCHDOG: Watchdog interrupt */
#define RECOV_WATCHDOG_MASK                  0x2
#define RECOV_WATCHDOG_BIT                   1

/* RECOV::BRB: BRB fifo threshold interrupt */
#define RECOV_BRB_MASK                       0x4
#define RECOV_BRB_BIT                        2

/* DEBUG::BREAKPOINT_0: Breakpoint 0 hit */
#define DEBUG_BREAKPOINT_0_MASK              0x1
#define DEBUG_BREAKPOINT_0_BIT               0

/* DEBUG::BREAKPOINT_1: Breakpoint 1 hit */
#define DEBUG_BREAKPOINT_1_MASK              0x2
#define DEBUG_BREAKPOINT_1_BIT               1

/* DEBUG::BREAKPOINT_2: Breakpoint 2 hit */
#define DEBUG_BREAKPOINT_2_MASK              0x4
#define DEBUG_BREAKPOINT_2_BIT               2

/* DEBUG::BREAKPOINT_3: Breakpoint 3 hit */
#define DEBUG_BREAKPOINT_3_MASK              0x8
#define DEBUG_BREAKPOINT_3_BIT               3

/* DEBUG::BREAKPOINT_4: Breakpoint 4 hit */
#define DEBUG_BREAKPOINT_4_MASK              0x10
#define DEBUG_BREAKPOINT_4_BIT               4

/* DEBUG::BREAKPOINT_5: Breakpoint 5 hit */
#define DEBUG_BREAKPOINT_5_MASK              0x20
#define DEBUG_BREAKPOINT_5_BIT               5

/* DEBUG::BREAKPOINT_6: Breakpoint 6 hit */
#define DEBUG_BREAKPOINT_6_MASK              0x40
#define DEBUG_BREAKPOINT_6_BIT               6

/* DEBUG::BREAKPOINT_7: Breakpoint 7 hit */
#define DEBUG_BREAKPOINT_7_MASK              0x80
#define DEBUG_BREAKPOINT_7_BIT               7

/* DEBUG::BREAKPOINT_8: Breakpoint 8 hit (FP4014 only) */
#define DEBUG_BREAKPOINT_8_MASK              0x100
#define DEBUG_BREAKPOINT_8_BIT               8

/* DEBUG::BREAKPOINT_9: Breakpoint 9 hit (FP4014 only) */
#define DEBUG_BREAKPOINT_9_MASK              0x200
#define DEBUG_BREAKPOINT_9_BIT               9

/* DEBUG::BREAKPOINT_10: Breakpoint 10 hit (FP4014 only) */
#define DEBUG_BREAKPOINT_10_MASK             0x400
#define DEBUG_BREAKPOINT_10_BIT              10

/* DEBUG::BREAKPOINT_11: Breakpoint 11 hit (FP4014 only) */
#define DEBUG_BREAKPOINT_11_MASK             0x800
#define DEBUG_BREAKPOINT_11_BIT              11

/* DEBUG::BREAKPOINT_12: Breakpoint 12 hit (FP4014 only) */
#define DEBUG_BREAKPOINT_12_MASK             0x1000
#define DEBUG_BREAKPOINT_12_BIT              12

/* DEBUG::BREAKPOINT_13: Breakpoint 13 hit (FP4014 only) */
#define DEBUG_BREAKPOINT_13_MASK             0x2000
#define DEBUG_BREAKPOINT_13_BIT              13

/* DEBUG::BREAKPOINT_14: Breakpoint 14 hit (FP4014 only) */
#define DEBUG_BREAKPOINT_14_MASK             0x4000
#define DEBUG_BREAKPOINT_14_BIT              14

/* DEBUG::BREAKPOINT_15: Breakpoint 15 hit (FP4014 only) */
#define DEBUG_BREAKPOINT_15_MASK             0x8000
#define DEBUG_BREAKPOINT_15_BIT              15

/* DEBUG::SINGLE_STEP: Single step */
#define DEBUG_SINGLE_STEP_MASK               0x10000
#define DEBUG_SINGLE_STEP_BIT                16

/* DEBUG::WATCHPOINT_0: Watchpoint 0 hit */
#define DEBUG_WATCHPOINT_0_MASK              0x20000
#define DEBUG_WATCHPOINT_0_BIT               17

/* DEBUG::WATCHPOINT_1: Watchpoint 1 hit */
#define DEBUG_WATCHPOINT_1_MASK              0x40000
#define DEBUG_WATCHPOINT_1_BIT               18

/* DEBUG::WATCHPOINT_2: Watchpoint 2 hit (FP4014 only) */
#define DEBUG_WATCHPOINT_2_MASK              0x80000
#define DEBUG_WATCHPOINT_2_BIT               19

/* DEBUG::WATCHPOINT_3: Watchpoint 3 hit (FP4014 only) */
#define DEBUG_WATCHPOINT_3_MASK              0x100000
#define DEBUG_WATCHPOINT_3_BIT               20

/* DEBUG::PPU: PPU auto stop reached */
#define DEBUG_PPU_MASK                       0x2000000
#define DEBUG_PPU_BIT                        25

/* DEBUG::IPH: IPH sample ready */
#define DEBUG_IPH_MASK                       0x4000000
#define DEBUG_IPH_BIT                        26

/* DEBUG::EXT_SW: From SW writable subsystem registers */
#define DEBUG_EXT_SW_MASK                    0x18000000
#define DEBUG_EXT_SW_BIT0                    27
#define DEBUG_EXT_SW_WIDTH                   2

/* DEBUG::EXT_HW: From SW writable subsystem registers */
#define DEBUG_EXT_HW_MASK                    0x60000000
#define DEBUG_EXT_HW_BIT0                    29
#define DEBUG_EXT_HW_WIDTH                   2

/* SOFT_BIST_CONTROL::PASS: Control pins */
#define SOFT_BIST_CONTROL_PASS_MASK          0x1
#define SOFT_BIST_CONTROL_PASS_BIT           0

/* SOFT_BIST_CONTROL::DONE: Control pins */
#define SOFT_BIST_CONTROL_DONE_MASK          0x2
#define SOFT_BIST_CONTROL_DONE_BIT           1

/* DEBUG_CONTROL::IRQ_DISABLE: Interrupt disable */
#define DEBUG_CONTROL_IRQ_DISABLE_MASK       0x40
#define DEBUG_CONTROL_IRQ_DISABLE_BIT        6

/* DEBUG_CONTROL::SIRQ_DISABLE: Super interrupt disable */
#define DEBUG_CONTROL_SIRQ_DISABLE_MASK      0x80
#define DEBUG_CONTROL_SIRQ_DISABLE_BIT       7

/* PSR::IRQ_DISABLE: Interrupt disable */
#define PSR_IRQ_DISABLE_MASK                 0x2
#define PSR_IRQ_DISABLE_BIT                  1

/* PSR::SIRQ_DISABLE: Super interrupt disable */
#define PSR_SIRQ_DISABLE_MASK                0x4
#define PSR_SIRQ_DISABLE_BIT                 2

/* PSR::CB_DISABLE: Callback disable */
#define PSR_CB_DISABLE_MASK                  0x8
#define PSR_CB_DISABLE_BIT                   3

/* PSR::RECOV_DISABLE: Recov disable */
#define PSR_RECOV_DISABLE_MASK               0x20
#define PSR_RECOV_DISABLE_BIT                5

/* PSR::DEBUG_DISABLE: Debug disable */
#define PSR_DEBUG_DISABLE_MASK               0x40
#define PSR_DEBUG_DISABLE_BIT                6

/* PSR::FATAL_DISABLE: Fatal disable */
#define PSR_FATAL_DISABLE_MASK               0x80
#define PSR_FATAL_DISABLE_BIT                7

/* PSR::PRIVILEGED: Privilege */
#define PSR_PRIVILEGED_MASK                  0x100
#define PSR_PRIVILEGED_BIT                   8

/* PSR::SINGLE_STEP: Enable single step */
#define PSR_SINGLE_STEP_MASK                 0x200
#define PSR_SINGLE_STEP_BIT                  9

/* PSR::IGNOREDREGS: Disregard implicit state except PC and PSR */
#define PSR_IGNOREDREGS_MASK                 0x400
#define PSR_IGNOREDREGS_BIT                  10

/* MSR::WIDTH_M7: Width of m7 - 2'b10 = word, 2'b01 = half, 2'b00 = byte */
#define MSR_WIDTH_M7_MASK                    0xc00000000000
#define MSR_WIDTH_M7_BIT0                    46
#define MSR_WIDTH_M7_WIDTH                   2

/* MSR::WIDTH_M6: Width of m6 - 2'b10 = word, 2'b01 = half, 2'b00 = byte */
#define MSR_WIDTH_M6_MASK                    0x300000000000
#define MSR_WIDTH_M6_BIT0                    44
#define MSR_WIDTH_M6_WIDTH                   2

/* MSR::WIDTH_M5: Width of m5 - 2'b10 = word, 2'b01 = half, 2'b00 = byte */
#define MSR_WIDTH_M5_MASK                    0xc0000000000
#define MSR_WIDTH_M5_BIT0                    42
#define MSR_WIDTH_M5_WIDTH                   2

/* MSR::WIDTH_M4: Width of m4 - 2'b10 = word, 2'b01 = half, 2'b00 = byte */
#define MSR_WIDTH_M4_MASK                    0x30000000000
#define MSR_WIDTH_M4_BIT0                    40
#define MSR_WIDTH_M4_WIDTH                   2

/* MSR::WIDTH_M3: Width of m3 - 2'b10 = word, 2'b01 = half, 2'b00 = byte */
#define MSR_WIDTH_M3_MASK                    0xc000000000
#define MSR_WIDTH_M3_BIT0                    38
#define MSR_WIDTH_M3_WIDTH                   2

/* MSR::WIDTH_M2: Width of m2 - 2'b10 = word, 2'b01 = half, 2'b00 = byte */
#define MSR_WIDTH_M2_MASK                    0x3000000000
#define MSR_WIDTH_M2_BIT0                    36
#define MSR_WIDTH_M2_WIDTH                   2

/* MSR::WIDTH_M1: Width of m1 - 2'b10 = word, 2'b01 = half, 2'b00 = byte */
#define MSR_WIDTH_M1_MASK                    0xc00000000
#define MSR_WIDTH_M1_BIT0                    34
#define MSR_WIDTH_M1_WIDTH                   2

/* MSR::WIDTH_M0: Width of m0 - 2'b10 = word, 2'b01 = half, 2'b00 = byte */
#define MSR_WIDTH_M0_MASK                    0x300000000
#define MSR_WIDTH_M0_BIT0                    32
#define MSR_WIDTH_M0_WIDTH                   2

/* MSR::TRELLIS: Trellis mode bit - OLD - 0; NEW - 1 */
#define MSR_TRELLIS_MASK                     0x800000
#define MSR_TRELLIS_BIT                      23

/* MSR::ZLRS: When set, set result of shift by more than width to 0 on MSHIFT */
#define MSR_ZLRS_MASK                        0x400000
#define MSR_ZLRS_BIT                         22

/* MSR::TRUNC_EARLY: Enable early truncation */
#define MSR_TRUNC_EARLY_MASK                 0x200000
#define MSR_TRUNC_EARLY_BIT                  21

/* MSR::SAT_EARLY: Enable early saturation */
#define MSR_SAT_EARLY_MASK                   0x100000
#define MSR_SAT_EARLY_BIT                    20

/* MSR::MMV_SAT: MMV saturation control - 2'b10 = unsigned saturation [SATU], 2'b01 = signed saturation [SATS], 2'b00 = no saturation [MODULO] */
#define MSR_MMV_SAT_MASK                     0xc0000
#define MSR_MMV_SAT_BIT0                     18
#define MSR_MMV_SAT_WIDTH                    2

/* MSR::MMV_ROUND: MMV rounding control - 2'b10 = round to nearest or positive [RNP], 2'b01 = round to nearest or even [RNE], 2'b00 = no rounding [TRUNC] */
#define MSR_MMV_ROUND_MASK                   0x30000
#define MSR_MMV_ROUND_BIT0                   16
#define MSR_MMV_ROUND_WIDTH                  2

/* MSR::BSR_CONVERT: When set, BFLYH performs extra operations usedin in converting between 2N real and N complex points */
#define MSR_BSR_CONVERT_MASK                 0x1000
#define MSR_BSR_CONVERT_BIT                  12

/* MSR::BSR_NEGATE: When set, CMxxx negates imaginary part of 2nd input operand - in BFLYx, negate imaginary part of twiddle factor (3rd operand) */
#define MSR_BSR_NEGATE_MASK                  0x800
#define MSR_BSR_NEGATE_BIT                   11

/* MSR::BSR_REPLICATE: When set, BFLYH replicates the first twiddle factor over both lanes */
#define MSR_BSR_REPLICATE_MASK               0x400
#define MSR_BSR_REPLICATE_BIT                10

/* MSR::BSR_DIV2: When set, BLYx divides butterfly results by 2 */
#define MSR_BSR_DIV2_MASK                    0x200
#define MSR_BSR_DIV2_BIT                     9

/* MSR::BSR_INTERLEAVE: When set, BFLYH interleaves butterfly results */
#define MSR_BSR_INTERLEAVE_MASK              0x100
#define MSR_BSR_INTERLEAVE_BIT               8

/* BREAKPOINT::ADDRESS: Breakpoint match word address */
#define BREAKPOINT_ADDRESS_MASK              0xfffffffc
#define BREAKPOINT_ADDRESS_BIT0              2
#define BREAKPOINT_ADDRESS_WIDTH             30

/* BREAKPOINT::ENABLE: Breakpoint enable */
#define BREAKPOINT_ENABLE_MASK               0x1
#define BREAKPOINT_ENABLE_BIT                0

/* UARCH_CMD::BTAC_INVALIDATE: Invalidate the branch target address cache */
#define UARCH_CMD_BTAC_INVALIDATE_MASK       0x2
#define UARCH_CMD_BTAC_INVALIDATE_BIT        1

/* UARCH_CMD::SPEC_ISS_DISABLE: Control speculative issue  2'b00 = revert to default behaviour, 2'b01 = add4, 2'b10 = add8, 2'b11 = disable all speculative issue */
#define UARCH_CMD_SPEC_ISS_DISABLE_MASK      0x300
#define UARCH_CMD_SPEC_ISS_DISABLE_BIT0      8
#define UARCH_CMD_SPEC_ISS_DISABLE_WIDTH     2

/* UARCH_CFG::BRANCH_PREDICTION: Branch prediction enable */
#define UARCH_CFG_BRANCH_PREDICTION_MASK     0x1
#define UARCH_CFG_BRANCH_PREDICTION_BIT      0

/* GP_CFG::CONFIG_BITS: Value of GP configuration parameter */
#define GP_CFG_CONFIG_BITS_MASK              0xffffffffffffffff
#define GP_CFG_CONFIG_BITS_BIT0              0
#define GP_CFG_CONFIG_BITS_WIDTH             64

/* PPU_AUTO_START::PC: PC of start instruction */
#define PPU_AUTO_START_PC_MASK               0xfffffffffffffffc
#define PPU_AUTO_START_PC_BIT0               2
#define PPU_AUTO_START_PC_WIDTH              62

/* PPU_AUTO_START::ENABLE: Enable starting on start instruction */
#define PPU_AUTO_START_ENABLE_MASK           0x1
#define PPU_AUTO_START_ENABLE_BIT            0

/* PPU_AUTO_STOP::PC: PC of stop instruction */
#define PPU_AUTO_STOP_PC_MASK                0xfffffffffffffffc
#define PPU_AUTO_STOP_PC_BIT0                2
#define PPU_AUTO_STOP_PC_WIDTH               62

/* PPU_AUTO_STOP::EXCEPTION: Raise exception when the stop instruction triggers */
#define PPU_AUTO_STOP_EXCEPTION_MASK         0x2
#define PPU_AUTO_STOP_EXCEPTION_BIT          1

/* PPU_AUTO_STOP::ENABLE: Enable stoppin on stop instruction */
#define PPU_AUTO_STOP_ENABLE_MASK            0x1
#define PPU_AUTO_STOP_ENABLE_BIT             0

/* PPU_AUTO_START_V2::PC: PC of start instruction */
#define PPU_AUTO_START_V2_PC_MASK            0xfffffffc
#define PPU_AUTO_START_V2_PC_BIT0            2
#define PPU_AUTO_START_V2_PC_WIDTH           30

/* PPU_AUTO_START_V2::ENABLE: Enable starting on start instruction */
#define PPU_AUTO_START_V2_ENABLE_MASK        0x1
#define PPU_AUTO_START_V2_ENABLE_BIT         0

/* PPU_AUTO_STOP_V2::PC: PC of stop instruction */
#define PPU_AUTO_STOP_V2_PC_MASK             0xfffffffc
#define PPU_AUTO_STOP_V2_PC_BIT0             2
#define PPU_AUTO_STOP_V2_PC_WIDTH            30

/* PPU_AUTO_STOP_V2::EXCEPTION: Raise exception when the stop instruction triggers */
#define PPU_AUTO_STOP_V2_EXCEPTION_MASK      0x2
#define PPU_AUTO_STOP_V2_EXCEPTION_BIT       1

/* PPU_AUTO_STOP_V2::ENABLE: Enable stoppin on stop instruction */
#define PPU_AUTO_STOP_V2_ENABLE_MASK         0x1
#define PPU_AUTO_STOP_V2_ENABLE_BIT          0

/* PPU_SELECT::COUNTER_0: Selector for counter 0 */
#define PPU_SELECT_COUNTER_0_MASK            0xff
#define PPU_SELECT_COUNTER_0_BIT0            0
#define PPU_SELECT_COUNTER_0_WIDTH           8

/* PPU_SELECT::COUNTER_1: Selector for counter 1 */
#define PPU_SELECT_COUNTER_1_MASK            0xff00
#define PPU_SELECT_COUNTER_1_BIT0            8
#define PPU_SELECT_COUNTER_1_WIDTH           8

/* PPU_SELECT::COUNTER_2: Selector for counter 2 */
#define PPU_SELECT_COUNTER_2_MASK            0xff0000
#define PPU_SELECT_COUNTER_2_BIT0            16
#define PPU_SELECT_COUNTER_2_WIDTH           8

/* PPU_SELECT::COUNTER_3: Selector for counter 3 */
#define PPU_SELECT_COUNTER_3_MASK            0xff000000
#define PPU_SELECT_COUNTER_3_BIT0            24
#define PPU_SELECT_COUNTER_3_WIDTH           8

/* PPU_SELECT::COUNTER_4: Selector for counter 4 */
#define PPU_SELECT_COUNTER_4_MASK            0xff00000000
#define PPU_SELECT_COUNTER_4_BIT0            32
#define PPU_SELECT_COUNTER_4_WIDTH           8

/* PPU_SELECT::COUNTER_5: Selector for counter 5 */
#define PPU_SELECT_COUNTER_5_MASK            0xff0000000000
#define PPU_SELECT_COUNTER_5_BIT0            40
#define PPU_SELECT_COUNTER_5_WIDTH           8

/* PPU_SELECT::COUNTER_6: Selector for counter 6 */
#define PPU_SELECT_COUNTER_6_MASK            0xff000000000000
#define PPU_SELECT_COUNTER_6_BIT0            48
#define PPU_SELECT_COUNTER_6_WIDTH           8

/* PPU_SELECT::COUNTER_7: Selector for counter 7 */
#define PPU_SELECT_COUNTER_7_MASK            0xff00000000000000
#define PPU_SELECT_COUNTER_7_BIT0            56
#define PPU_SELECT_COUNTER_7_WIDTH           8

/* PPU_COUNTER::CTR: 32 bit unsigned Counter value */
#define PPU_COUNTER_CTR_MASK                 0xffffffff
#define PPU_COUNTER_CTR_BIT0                 0
#define PPU_COUNTER_CTR_WIDTH                32

/* PPU_CONTROL::ENABLE: Enable(1) Disable(0) PPU */
#define PPU_CONTROL_ENABLE_MASK              0x1
#define PPU_CONTROL_ENABLE_BIT               0

/* PPU_STATUS::COUNTERS_RUNNING: Counters running */
#define PPU_STATUS_COUNTERS_RUNNING_MASK     0x1
#define PPU_STATUS_COUNTERS_RUNNING_BIT      0

/* PPU_STATUS::STOP_TRIGGERED: Stop triggered flag (sticky flag, must be cleared from software) */
#define PPU_STATUS_STOP_TRIGGERED_MASK       0x4
#define PPU_STATUS_STOP_TRIGGERED_BIT        2

/* PPU_STATUS::SPEC_OVERLAP: A speculative auto start was ignored while a speculative stop was waiting to be resolved */
#define PPU_STATUS_SPEC_OVERLAP_MASK         0x8
#define PPU_STATUS_SPEC_OVERLAP_BIT          3

/* PPU_COMMAND::START_COUNTERS: Start counters (write 1 to start counters) */
#define PPU_COMMAND_START_COUNTERS_MASK      0x1
#define PPU_COMMAND_START_COUNTERS_BIT       0

/* PPU_COMMAND::STOP_COUNTERS: Stop counters (write 1 to stop counters) */
#define PPU_COMMAND_STOP_COUNTERS_MASK       0x2
#define PPU_COMMAND_STOP_COUNTERS_BIT        1

/* PPU_COMMAND::CLEAR_STOP: Clear stop triggered flag (write 1 to clear flag) */
#define PPU_COMMAND_CLEAR_STOP_MASK          0x4
#define PPU_COMMAND_CLEAR_STOP_BIT           2

/* PPU_COMMAND::CLEAR_SPEC_OVERLAP: Clear spec overlap flag (write 1 to clear flag) */
#define PPU_COMMAND_CLEAR_SPEC_OVERLAP_MASK  0x8
#define PPU_COMMAND_CLEAR_SPEC_OVERLAP_BIT   3

/* IPH_CFG::ENABLE: Enable PRNG for instruction selection */
#define IPH_CFG_ENABLE_MASK                  0x1
#define IPH_CFG_ENABLE_BIT                   0

/* IPH_PRNG::XOR_STATE: PRNG xor state */
#define IPH_PRNG_XOR_STATE_MASK              0xffffffff
#define IPH_PRNG_XOR_STATE_BIT0              0
#define IPH_PRNG_XOR_STATE_WIDTH             32

/* IPH_PRNG::WEYL_STATE: PRNG weyl state */
#define IPH_PRNG_WEYL_STATE_MASK             0xffffff00000000
#define IPH_PRNG_WEYL_STATE_BIT0             32
#define IPH_PRNG_WEYL_STATE_WIDTH            24

/* IPH_SELECT::MASK: Mask parameter */
#define IPH_SELECT_MASK_MASK                 0xffffff00000000
#define IPH_SELECT_MASK_BIT0                 32
#define IPH_SELECT_MASK_WIDTH                24

/* IPH_SELECT::SPAN: Span parameter */
#define IPH_SELECT_SPAN_MASK                 0xffffff
#define IPH_SELECT_SPAN_BIT0                 0
#define IPH_SELECT_SPAN_WIDTH                24

/* SHADOW_REG::BITS: None */
#define SHADOW_REG_BITS_MASK                 0xffffffffffffffff
#define SHADOW_REG_BITS_BIT0                 0
#define SHADOW_REG_BITS_WIDTH                64

/* SHADOW_PC_REG::PC: Cut down shadow register for storing a PC only */
#define SHADOW_PC_REG_PC_MASK                0xfffffffc
#define SHADOW_PC_REG_PC_BIT0                2
#define SHADOW_PC_REG_PC_WIDTH               30

/* RANDOM_STALL::ENABLE: Enable random stalling */
#define RANDOM_STALL_ENABLE_MASK             0x1
#define RANDOM_STALL_ENABLE_BIT              0

/* RANDOM_STALL::DERATE: Proportion of random stalls to inject */
#define RANDOM_STALL_DERATE_MASK             0x6
#define RANDOM_STALL_DERATE_BIT0             1
#define RANDOM_STALL_DERATE_WIDTH            2

/* RANDOM_STALL::LFSR: Seed for random stalling */
#define RANDOM_STALL_LFSR_MASK               0x1ffffffff00
#define RANDOM_STALL_LFSR_BIT0               8
#define RANDOM_STALL_LFSR_WIDTH              33

/* BPRED_CTRL::RANDOM: Branch predictor contol: 0 = default, 1 = random, 2 = always untaken, 3 = always taken */
#define BPRED_CTRL_RANDOM_MASK               0x3
#define BPRED_CTRL_RANDOM_BIT0               0
#define BPRED_CTRL_RANDOM_WIDTH              2

/* BPRED_CTRL::LFSR: Seed for random branch prediction */
#define BPRED_CTRL_LFSR_MASK                 0x1ffffffff00
#define BPRED_CTRL_LFSR_BIT0                 8
#define BPRED_CTRL_LFSR_WIDTH                33



/*
 DREG validity bitmasks
*/

#define DIR_VALIDITY_MASK0 0xffffffff
#define DIR_VALIDITY_MASK1 0xffff00ff
#define DIR_VALIDITY_MASK2 0xffffffff
#define DIR_VALIDITY_MASK3 0x0001ffff
#define DIR_VALIDITY_MASK4 0xffffffff
#define DIR_VALIDITY_MASK5 0x1fc3c000
#define DIR_VALIDITY_MASK6 0x07fff7ff
#define DIR_VALIDITY_MASK7 0x00000000

#endif
