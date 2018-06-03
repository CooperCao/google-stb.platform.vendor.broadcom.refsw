/************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ************************************************************************/

/*
  AUTOMATICALLY GENERATED FILE.
  DO NOT EDIT

  Generated for
    - machine architecture: fp2011
    - binutils release: dev-443
    - FPDB release: /projects/firepath/implementation/releases/fpdb/r1310/ (old format)
*/

#ifndef GENERATED_DREG_HEADER_H
#define GENERATED_DREG_HEADER_H

/*
  DREG list
*/

#define DIR_PROT1_ADDR_BOT                        70
#define DIR_PROT1_ADDR_TOP                        71
#define DIR_PROT2_ADDR_BOT                        72
#define DIR_PROT2_ADDR_TOP                        73
#define DIR_PROT3_ADDR_BOT                        74
#define DIR_PROT3_ADDR_TOP                        75
#define DIR_PROT4_ADDR_BOT                        76
#define DIR_PROT4_ADDR_TOP                        77
#define DIR_PROT_STATUS_X                         78
#define DIR_PROT_STATUS_Y                         79
#define DIR_WP1_ADDR_BOT                          90
#define DIR_WP1_ADDR_TOP                          91
#define DIR_WP2_ADDR_BOT                          92
#define DIR_WP2_ADDR_TOP                          93
#define DIR_WP3_ADDR_BOT                          94
#define DIR_WP3_ADDR_TOP                          95
#define DIR_WP4_ADDR_BOT                          96
#define DIR_WP4_ADDR_TOP                          97
#define DIR_WP5_ADDR_BOT                          98
#define DIR_WP5_ADDR_TOP                          99
#define DIR_WP6_ADDR_BOT                         100
#define DIR_WP6_ADDR_TOP                         101
#define DIR_WP7_ADDR_BOT                         102
#define DIR_WP7_ADDR_TOP                         103
#define DIR_WP8_ADDR_BOT                         104
#define DIR_WP8_ADDR_TOP                         105
#define DIR_WP_STATUS_X                          106
#define DIR_WP_STATUS_Y                          107
#define DIR_TOTC                                 128
#define DIR_USRC                                 129
#define DIR_TOTC_CMP1                            130
#define DIR_TOTC_CMP2                            131
#define DIR_USRC_CMP                             132
#define DIR_SI_ENABLE                            133
#define DIR_SI_LATCHED                           134
#define DIR_SI_CLEAR                             135
#define DIR_SI_SET                               136
#define DIR_SI_RAW                               137
#define DIR_INT_ENABLE                           138
#define DIR_INT_LATCHED                          139
#define DIR_INT_CLEAR                            140
#define DIR_INT_SET                              141
#define DIR_INT_RAW                              142
#define DIR_ICACHE_CONTROL                       144
#define DIR_INSTRUCTION_SET_ID                   150

/* incremements once per issued instruction which was not already in the ICache. */
#define DIR_ICACHE_MISS_INSN_CTR                 152

/* Increments once per cycle spent waiting for an instruction to arrive from the ICache. */
#define DIR_ICACHE_MISS_CYCLE_CTR                153

/* Increments once per issued instruction. */
#define DIR_ISSUED_INSN_CTR                      154
#define DIR_PROFILE_CYCLE_CTR                    155
#define DIR_PROFILE_START_ADDR                   156
#define DIR_PROFILE_STOP_ADDR                    157
#define DIR_SMEM_PARITY                          169
#define DIR_LOOP_COUNT                           170
#define DIR_LOOP_ADDR                            171
#define DIR_FNSC2_TABLE_PTR                      172
#define DIR_FNSC2_TABLE_DATA                     173
#define DIR_CONSOLE_READ                         177
#define DIR_CONSOLE_WRITE                        178
#define DIR_SOFT_BIST_CONTROL                    179
#define DIR_BPT1_ADDR                            164
#define DIR_BPT2_ADDR                            166


/*
 DREG validity bitmasks
*/

#define DIR_VALIDITY_MASK0 0x00000000
#define DIR_VALIDITY_MASK1 0x00000000
#define DIR_VALIDITY_MASK2 0xfc00ffc0
#define DIR_VALIDITY_MASK3 0x00000fff
#define DIR_VALIDITY_MASK4 0x3f417fff
#define DIR_VALIDITY_MASK5 0x000e3e50
#define DIR_VALIDITY_MASK6 0x00000000
#define DIR_VALIDITY_MASK7 0x00000000

#endif
