/****************************************************************************
 *                Copyright (c) 2014 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
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



/* ----------------
 * Phases constants
 * ---------------- */
/* Note: the best documentation for these sample points is looking at where they get generated */
#define HB_PHASE_NOT_STARTED                        0x00
#define     HB_SUBPHASE_NOT_STARTED                     0x00

#define HB_PHASE_ENTRY_SETUP                        0x10
#define     HB_SUBPHASE_ENTRY_SETUP_BEGINNING           0x00
#define     HB_SUBPHASE_BEFORE_STACK_INIT               0x10
#define     HB_SUBPHASE_BEFORE_ENABLE_SIRQS             0x20

#define HB_PHASE_RUNTIME_INIT                       0x30
#define     HB_SUBPHASE_RUNTIME_INIT_BEGINNING          0x00
#define     HB_SUBPHASE_BEFORE_LOADTABLES               0x10
#define     HB_SUBPHASE_BEFORE_SDK_INIT                 0x20
#define     HB_SUBPHASE_BEFORE_ALT_INIT                 0x30
#define     HB_SUBPHASE_BEFORE_INIT                     0x40
#define     HB_SUBPHASE_BEFORE_SWITCH_OVERLAYS          0x50

#define HB_PHASE_MAIN                               0x50
#define     HB_SUBPHASE_ENTERING_MAIN                   0x00

#define HB_PHASE_AFTER_MAIN                         0x70
#define     HB_SUBPHASE_RETURNED_FROM_MAIN              0x00

#define HB_PHASE_DBA_COREDUMP                       0xB0
#define     HB_SUBPHASE_DUMP_GLOBAL_HEADER              0x20
#define     HB_SUBPHASE_DUMP_REGION_HEADER_IMEM         0x31
#define     HB_SUBPHASE_DUMP_REGION_HEADER_DMEM         0x32
#define     HB_SUBPHASE_DUMP_REGION_HEADER_PMEM         0x33
#define     HB_SUBPHASE_DUMP_REGION_HEADER_METADATA     0x34
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


/* --------------------
 * Inter-language macro
 * -------------------- */
/**
 * @def HEARTBEAT_INT_UPDATE(phase, subphase, argument)
 * Updates the whole Heartbeat status word. Usable from both the assembler
 * and C world, invoking this macro will squash r0, r1 and r2 content.
 */
#ifdef ENABLE_HEARTBEAT
#  ifdef ASMCPP
#    define HEARTBEAT_INT_UPDATE(phase, subphase, argument)   heartbeat_int_update phase, subphase, argument
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
