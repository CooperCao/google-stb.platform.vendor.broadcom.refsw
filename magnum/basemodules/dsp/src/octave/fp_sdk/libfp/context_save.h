/****************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

/**
 * @file
 * @ingroup libfp
 * @brief Routines to save and restore the processor state in a context structure.
 *
 * This header contains common declaration and then includes the right
 * sub-header depending on the machine architecture.
 */

#ifndef _CONTEXT_SAVE_H_
#define _CONTEXT_SAVE_H_

#include "fp_sdk_config.h"



/**
 * @name CS_FLAGS
 * Flags used as argument to (re)storeContext indicating which parts of context
 * should be saved / restored.
 *
 * @note When adding new ones don't forget to add to CS_FULL_SAVE.
 *
 * @{
 */

/**
 * On FP2000/Octave: the volatile registers r10-r31 also r56-r61. In case of
 * banked registers, the user r56-r61 are saved. Note that this excludes
 * predicate registers and r0-r9 because it is assumed that these are saved by
 * the caller.
 *
 * On Maestro: the volatile registers, r0-r4.
 */
#define CS_FLAG_VOLATILES_BIT               0
#define CS_FLAG_VOLATILES                   (1 << CS_FLAG_VOLATILES_BIT)
/** The non-volatile registers: r32-r55 on FP2000/Octave, r5-r15 on Maestro. */
#define CS_FLAG_NONVOLATILES_BIT            1
#define CS_FLAG_NONVOLATILES                (1 << CS_FLAG_NONVOLATILES_BIT)
#ifdef CORE_HAS_BANKED_REGS
/** The interrupt bank registers irq_r60 and irq_r61 */
#  define CS_FLAG_IRQ_REGS_BIT              2
#  define CS_FLAG_IRQ_REGS                  (1 << CS_FLAG_IRQ_REGS_BIT)
#endif
/** The zero overhead loop state */
#define CS_FLAG_LOOP_BIT                    3
#define CS_FLAG_LOOP                        (1 << CS_FLAG_LOOP_BIT)
/** The MAC registers and msr */
#define CS_FLAG_MREGS_BIT                   4
#define CS_FLAG_MREGS                       (1 << CS_FLAG_MREGS_BIT)
/** All the other custom / extended registers grouped together */
#define CS_FLAG_OTHERS_BIT                  5
#define CS_FLAG_OTHERS                      (1 << CS_FLAG_OTHERS_BIT)
/** The lowest unused bit */
#define CS_FLAG_UNUSED_BIT                  6
/** Bit mask indicating to save everything. */
#ifdef CORE_HAS_BANKED_REGS
#  define CS_FULL_SAVE                      (CS_FLAG_VOLATILES | CS_FLAG_NONVOLATILES | CS_FLAG_IRQ_REGS | CS_FLAG_LOOP | CS_FLAG_MREGS | CS_FLAG_OTHERS)
#else
#  define CS_FULL_SAVE                      (CS_FLAG_VOLATILES | CS_FLAG_NONVOLATILES | CS_FLAG_LOOP | CS_FLAG_MREGS | CS_FLAG_OTHERS)
#endif
/** Bit mask indicating to save general purpose registers (volatiles and non volatiles) and LOOP state */
#define CS_GREG_SAVE                        (CS_FLAG_VOLATILES | CS_FLAG_NONVOLATILES | CS_FLAG_LOOP)
/** @} */


#if defined(__FPM1015_ONWARDS__)
#  include "context_save_maestro.h"
#elif defined(__FP2006_ONWARDS__)
#  include "context_save_dsp.h"
#else
#  error "Unsupported architecture"
#endif


#endif  /* _CONTEXT_SAVE_H_ */
