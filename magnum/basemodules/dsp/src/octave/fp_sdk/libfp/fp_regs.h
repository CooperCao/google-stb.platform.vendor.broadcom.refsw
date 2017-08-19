/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

/**
 * \file
 * \ingroup libfp
 *
 * File listing the number of different kinds of registers available on core.
 */

#ifndef _FP_REGS_H_
#define _FP_REGS_H_

/**
 * Number of general purpose registers, not including hardwired ones
 * (e.g. zr, or). On FP20xx machines, this refers only to registers in the
 * user bank.
 */
/*FIXME: The USER_REG_NUM/PRED_REG_NUM macros used in the context_save_xx
 * structures is shared between the DSP and host code (for on-chip debug).
 * The host code doesn't know about the __MAESTRO__ macro
 * To accomodate the host, we have to use the SDK defined __FPM1015__
 * macro. To add to the mess, this header file couldn't include
 * fp-sdk-config.h, because of our legacy to build libadi even for targets
 * which doesn't require them !!!
 * This implies, the header/source file which includes this header
 * file needs to include fp-sdk-config.h to get correct behaviour !!!
 * */
#if !defined(__FPM1015_ONWARDS__)
#  define USER_REG_NUM                           (62)
#else
#  define USER_REG_NUM                           (16)
#endif


/**
 * Number of predicate registers, not including hardwired ones (e.g. p7).
 */
/*FIXME: look at the FIXME above*/
#if !defined(__FPM1015_ONWARDS__)
#  define PRED_REG_NUM                           (7)
#else
#  define PRED_REG_NUM                           (2)
#endif


/* Octave and Maestro architectures don't have banked registers */
#if !defined(__FPM1015_ONWARDS__) && !defined(__FP4014_ONWARDS__)

/**
 * Number of general purpose register in irq bank (irq_r60 and irq_r61).
 */
#  define IRQ_REG_NUM                            (2)

/**
 * Number of general purpose register in svc bank (svc_r60 and svc_r61).
 */
#  define SVC_REG_NUM                            (2)

/**
 * Number of general purpose register in sirq bank (sirq_r56 - sirq_r61).
 */
#  define SIRQ_REG_NUM                           (6)

#endif


/* No MAC registers on Maestro for now */
#if !defined(__FPM1015__)

/**
 * Number of MAC registers.
 */
#  if !defined(CORE_HAS_8_MREGS)
#    define M_REG_NUM                            (4)
#  else
#    define M_REG_NUM                            (8)
#  endif

#endif


#endif  /* _FP_REGS_H_ */
