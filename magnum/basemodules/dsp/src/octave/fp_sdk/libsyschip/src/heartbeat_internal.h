/****************************************************************************
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
 ****************************************************************************/

#ifndef _HEARTBEAT_INTERNAL_H_
#define _HEARTBEAT_INTERNAL_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "fp_sdk_config.h"

/*#include "libfp/asm_macros.h"*/

#include "libsyschip/memmap.h"



/* ----------------
 * Phases constants
 * ---------------- */
/* Note: the best documentation for these sample points is looking at where they get generated */
#define HB_PHASE_NOT_STARTED                        0x00
#define     HB_SUBPHASE_NOT_STARTED                     0x00

#define HB_PHASE_ENTRY_SETUP                        0x10
#define     HB_SUBPHASE_ENTRY_SETUP_BEGINNING           0x00
#define     HB_SUBPHASE_BEFORE_MEMS_ZEROING             0x08
#define     HB_SUBPHASE_BEFORE_GOT_COPY                 0x0a
#define     HB_SUBPHASE_BEFORE_STACK_INIT               0x10
#define     HB_SUBPHASE_BEFORE_CTXT_INIT                0x18
#define     HB_SUBPHASE_BEFORE_ENABLE_SIRQS             0x20
#define     HB_SUBPHASE_BEFORE_ENABLE_EXCEPTIONS        HB_SUBPHASE_BEFORE_ENABLE_SIRQS

#define HB_PHASE_RUNTIME_INIT                       0x30
#define     HB_SUBPHASE_RUNTIME_INIT_BEGINNING          0x00
#define     HB_SUBPHASE_BEFORE_LOADTABLES               0x10
#define     HB_SUBPHASE_BEFORE_SDK_INIT                 0x20
#define     HB_SUBPHASE_BEFORE_ALT_INIT                 0x30
#define     HB_SUBPHASE_BEFORE_CONSTRUCTORS             0x38
#define     HB_SUBPHASE_BEFORE_INIT                     0x40
#define     HB_SUBPHASE_BEFORE_SWITCH_OVERLAYS          0x50

#define HB_PHASE_MAIN                               0x50
#define     HB_SUBPHASE_ENTERING_MAIN                   0x00

#define HB_PHASE_AFTER_MAIN                         0x70
#define     HB_SUBPHASE_RETURNED_FROM_MAIN              0x00

#define HB_PHASE_DBA_COREDUMP                       0xB0
#define     HB_SUBPHASE_DUMP_GLOBAL_HEADER              0x20
#define     HB_SUBPHASE_DUMP_REGION_HEADER_MEM          0x30
#define     HB_SUBPHASE_DUMP_REGION_HEADER_IMEM         0x31
#define     HB_SUBPHASE_DUMP_REGION_HEADER_ITCM         HB_SUBPHASE_DUMP_REGION_HEADER_IMEM
#define     HB_SUBPHASE_DUMP_REGION_HEADER_DMEM         0x32
#define     HB_SUBPHASE_DUMP_REGION_HEADER_DTCM         HB_SUBPHASE_DUMP_REGION_HEADER_DMEM
#define     HB_SUBPHASE_DUMP_REGION_HEADER_PMEM         0x33
#define     HB_SUBPHASE_DUMP_REGION_HEADER_METADATA     0x34
#define     HB_SUBPHASE_DUMP_REGION_MEM                 0x40
#define     HB_SUBPHASE_DUMP_REGION_IMEM                0x41
#define     HB_SUBPHASE_DUMP_REGION_DMEM                0x42
#define     HB_SUBPHASE_DUMP_REGION_PMEM                0x43
#define     HB_SUBPHASE_DUMP_REGION_METADATA            0x44
#define     HB_SUBPHASE_DUMP_FINISH                     0x80
#define     HB_SUBPHASE_DUMP_FLUSH_PRINTS               0xA0

#define HB_PHASE_THE_END                            0xFF
#define     HB_SUBPHASE_EXIT                            0x80
#define     HB_SUBPHASE_TEST_EXIT                       0x90
#define     HB_SUBPHASE_TEST_EXIT_WAIT_FOR_THE_END      0x94
#define     HB_SUBPHASE_LOOPING_AFTER_EXIT              0xD0
#define     HB_SUBPHASE_DBA_WAIT_FOR_THE_END            0xF0


/* -----------------------------
 * Heartbeat scenario assessment
 * ----------------------------- */
/* We can't rely simply on NUM_CORES to understand where and in how many places
 * the heartbeat word has to be stored, so we compute here what is the scenario
 * we are facing.
 * Also, we might get included when building host code, which needs the above
 * constants, so cope with this possibility. */
#if !defined(ENABLE_HEARTBEAT) || !defined(__FIREPATH__)
/* Do nothing - no scenario will be defined */
#elif (NUM_CORES * NUM_SUBSYSTEMS) == 1
#  define HEARTBEAT_SCENARIO_1_CORE
#elif (NUM_CORES * NUM_SUBSYSTEMS) == 2
#  define HEARTBEAT_SCENARIO_2_CORES
#elif (NUM_CORES == 1 && NUM_SUBSYSTEMS > 2) || (NUM_CORES > 2 && NUM_SUBSYSTEMS == 1)
#  define HEARTBEAT_SCENARIO_N_CORES
#else
#  error "Unsupported Heartbeat scenario"
#endif

#if defined(HEARTBEAT_SCENARIO_1_CORE) && !defined(HEARTBEAT_SHARED_ADDRESS)
#  error "Please define HEARTBEAT_SHARED_ADDRESS"
#elif (defined(HEARTBEAT_SCENARIO_2_CORES) || defined(HEARTBEAT_SCENARIO_N_CORES)) && \
      !(defined(HEARTBEAT_SHARED_ADDRESS_BASE) && defined(HEARTBEAT_SHARED_ADDRESS_DELTA))
#  error "Please define HEARTBEAT_SHARED_ADDRESS_BASE and HEARTBEAT_SHARED_ADDRESS_DELTA"
#endif


/* --------------------
 * Inter-language macro
 * -------------------- */
/**
 * @def HEARTBEAT_INT_UPDATE(phase, subphase, argument)
 * Updates the whole Heartbeat status word. Usable from both the assembler
 * and C world, invoking this macro will squash r0, r1, r2 and link (r58) content.
 */
#ifdef ENABLE_HEARTBEAT
#  if   __FP2012_ONWARDS__ && defined(ASMCPP)
#    define HEARTBEAT_INT_UPDATE(phase, subphase, argument)   heartbeat_int_update phase, subphase, argument

#  elif __FPM1015_ONWARDS__ && defined(ASMCPP)
#    error "Not implemented"

#  elif defined(ASMCPP) && __PIC__
#    error "Not implemented"

#  elif defined(__RALL2__) && __FP4014_ONWARDS__ && MNO_USE_MB
#    define HEARTBEAT_INT_UPDATE(phase, subphase, argument)   movb r0, phase : movb r1, subphase : movh r2, argument $ b heartbeat_int_update<! 3>, r58

#  elif defined(__RALL2__) && __FP4014_ONWARDS__ && !MNO_USE_MB
#    define HEARTBEAT_INT_UPDATE(phase, subphase, argument)   mb heartbeat_int_update<! 3> : movb r0, phase : movb r1, subphase : movh r2, argument

#  elif defined(__RALL2__) && __FP4014_ONWARDS__ && __PIC__
#    define HEARTBEAT_INT_UPDATE(phase, subphase, argument)   movb r0, phase : movb r1, subphase : movh r2, argument $ sbl [r3 TICK]<! 3>, r58 : m_addr_of_func(r3, heartbeat_int_update)

#  elif defined(__RALL2__) && __FPM1015_ONWARDS__
#    define HEARTBEAT_INT_UPDATE(phase, subphase, argument)   movw r0, phase $ movw r1, subphase $ movw r2, argument $ b heartbeat_int_update<! 3>, r15

#  elif defined(__RALL2__) && __FPM1015_ONWARDS__ && PIC
#    define HEARTBEAT_INT_UPDATE(phase, subphase, argument)   movw r0, phase $ movw r1, subphase $ movw r2, argument $ m_addr_of_func(r3, heartbeat_int_update) $ sbl [r3 TICK]<! 3>, r15

#  else
#    define HEARTBEAT_INT_UPDATE(phase, subphase, argument)   heartbeat_int_update(phase, subphase, argument)
#  endif
#else
#  define HEARTBEAT_INT_UPDATE(phase, subphase, argument)
#endif


/* --------------------------
 * Assembler-only definitions
 * -------------------------- */
#ifdef ASMCPP
;;*****************************************************************************
;;  Macro that expands arguments onto r0-r2 an calls heartbeat_int_update
;;*****************************************************************************
.macro heartbeat_int_update __phase:req, __subphase:req, __argument:req
    movb        r0, \__phase                        :   movb r1, \__subphase
    movh        r2, \__argument
    bl          heartbeat_int_update, r58
.endm
#endif



/* ------------------
 * C-only definitions
 * ------------------ */
#ifndef ASMCPP

#if defined(__FIREPATH__)
#  include <stdint.h>
#else
/* Let's assume we are being included from libdspcontrol */
#  include "libdspcontrol/CHIP.h"
#  if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#    include "bstd_defs.h"
#  else
#    include <stdint.h>
#  endif
#endif


/** Constructs a Heartbeat word from its components. */
#define HEARTBEAT_ASSEMBLE_WORD(phase, subphase, argument)   (((((uint8_t)  (phase))    << 24) & 0xFF000000) |  \
                                                              ((((uint8_t)  (subphase)) << 16) & 0x00FF0000) |  \
                                                              ((((uint16_t) (argument)) <<  0) & 0x0000FFFF))

/** @{ Extract components from and Heartbeat word. */
#define HEARTBEAT_GET_PHASE(heartbeat_word)     ((uint8_t)  (((heartbeat_word) & 0xFF000000) >> 24))
#define HEARTBEAT_GET_SUBPHASE(heartbeat_word)  ((uint8_t)  (((heartbeat_word) & 0x00FF0000) >> 16))
#define HEARTBEAT_GET_ARGUMENT(heartbeat_word)  ((uint16_t) (((heartbeat_word) & 0x0000FFFF) >>  0))
/** @} */



/**
 * Updates the whole Heartbeat status word.
 * C prototype of the assembler-implemented function.
 *
 * The provided <phase, subphase, argument> tuple will be immediately made
 * visible to the Host.
 */
void heartbeat_int_update(uint8_t phase, uint8_t subphase, uint16_t argument);
/* Note: gcc doesn't know that heartbeat_int_update clobbers only r0, r1 and
 * r2 and so, inefficiently, will probably try to save all volatiles before
 * invoking it. There exists a convoluted way to force gcc to do the right
 * thing, which involves inline static functions and inline assembler.
 * If somebody will ever want to implement this, please refer to the
 * "Using the GNU Compiler Collection" manual, §"Assembler Instructions
 * with C Expression Operands" (§6.41 in the version I have at hand). */

#endif


#endif /* _HEARTBEAT_INTERNAL_H_ */
