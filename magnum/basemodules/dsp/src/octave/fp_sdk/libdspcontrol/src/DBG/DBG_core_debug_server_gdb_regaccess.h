/****************************************************************************
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
 ****************************************************************************/
#ifndef _DBG_CORE_DEBUG_SERVER_GDB_REGACCESS_H_
#define _DBG_CORE_DEBUG_SERVER_GDB_REGACCESS_H_

#include <stdint.h>
#include <stdbool.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DBG.h"

#include "fp_sdk_config.h"

#include "DBG_core_debug_server.h"

#if (FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
#include "libthreadxp/registers.h"
#endif

#define FIREPATH_REGS_HAS_CREGS (1)

#ifdef FIREPATH_REGS_H
//If we have access to firepath-regs.h, include the file
    #include "firepath-regs.h"
#else   /*FIREPATH_REGS_H*/
#if FIREPATH_REGS_HAS_CREGS
    #include "custom-reg-num.h"
    #ifdef __cplusplus
        #define num_cregs   Sim::NUM_CREGS
    #else
        #define num_cregs   NUM_CREGS
    #endif
#endif  /*FIREPATH_REGS_HAS_CREGS*/

#define FP_REGS_CREG_INDEX(x) FP_REGS_CREG_START + (x)

/* else - replicate the enumeration */
typedef enum firepath_regs {
  /* General registers 0-55*/
  FP_REG_R0 = 0, FP_REG_R1, FP_REG_R2, FP_REG_R3,
  FP_REG_R4, FP_REG_R5, FP_REG_R6, FP_REG_R7,
  FP_REG_R8, FP_REG_R9, FP_REG_R10, FP_REG_R11,
  FP_REG_R12, FP_REG_R13, FP_REG_R14, FP_REG_R15,
  FP_REG_R16, FP_REG_R17, FP_REG_R18, FP_REG_R19,
  FP_REG_R20, FP_REG_R21, FP_REG_R22, FP_REG_R23,
  FP_REG_R24, FP_REG_R25, FP_REG_R26, FP_REG_R27,
  FP_REG_R28, FP_REG_R29, FP_REG_R30, FP_REG_R31,
  FP_REG_R32, FP_REG_R33, FP_REG_R34, FP_REG_R35,
  FP_REG_R36, FP_REG_R37, FP_REG_R38, FP_REG_R39,
  FP_REG_R40, FP_REG_R41, FP_REG_R42, FP_REG_R43,
  FP_REG_R44, FP_REG_R45, FP_REG_R46, FP_REG_R47,
  FP_REG_R48, FP_REG_R49, FP_REG_R50, FP_REG_R51,
  FP_REG_R52, FP_REG_R53, FP_REG_R54, FP_REG_R55,

  /* user bank 56-61*/
  FP_REG_R56, FP_REG_R57, FP_REG_R58,
  FP_REG_R59, FP_REG_R60, FP_REG_R61,

  /* hardwired registers 62-63*/
  FP_REG_R62, FP_REG_OR = FP_REG_R62,
  FP_REG_R63, FP_REG_ZR = FP_REG_R63,

  /* svc bank 64-65*/
  FP_REG_SU_R60, FP_REG_SU_R61,

  /* irq mode 66-67*/
  FP_REG_IN_R60, FP_REG_IN_R61,

  /* sirq mode 68-73*/
  FP_REG_SI_R56, FP_REG_SI_R57, FP_REG_SI_R58,
  FP_REG_SI_R59, FP_REG_SI_R60, FP_REG_SI_R61,

  /* special regs 74-79*/
  FP_REG_PSR,
  FP_REG_MSR0, FP_REG_MSR1, FP_REG_MSR2, FP_REG_MSR3,
  FP_REG_PC,  /* It's critical that this is the last GREG! */

  /* MAC registers 80-87*/
  FP_REG_M0, FP_REG_M1, FP_REG_M2, FP_REG_M3,
  FP_REG_M4, FP_REG_M5, FP_REG_M6, FP_REG_M7,

  /* Predicate registers 88-95*/
  FP_REG_P0, FP_REG_P1, FP_REG_P2, FP_REG_P3,
  FP_REG_P4, FP_REG_P5, FP_REG_P6, FP_REG_P7,

#if FIREPATH_REGS_HAS_CREGS
  /* custom registers */
  FP_REGS_CREG_START,
  FP_REGS_NUM_CREGS = num_cregs,    /* from custom-regs.h */
  FP_REGS_CREG_LIMIT = FP_REGS_CREG_START + FP_REGS_NUM_CREGS,
#endif

  /* housekeeping */
  FP_REGS_GREG_START = FP_REG_R0,
  FP_REGS_NUM_GREGS  = (FP_REG_PC + 1),
  FP_REGS_GREG_LIMIT = (FP_REG_R0 + FP_REGS_NUM_GREGS),

  FP_REGS_MREG_START = FP_REG_M0,
  FP_REGS_NUM_MREGS  = 8,
  FP_REGS_MREG_LIMIT = (FP_REGS_MREG_START + FP_REGS_NUM_MREGS),

  FP_REGS_PREG_START = FP_REG_P0,
  FP_REGS_NUM_PREGS  = 8,
  FP_REGS_PREG_LIMIT = (FP_REG_P0 + FP_REGS_NUM_PREGS),

  /* aliases - for GDB mainly */
  FP_REG_MSR = FP_REG_MSR0,     /* alias for single-MSR architectures */
  FP_REG_A1 = FP_REG_R0,        /* first argument register */
  FP_REG_A8 = FP_REG_R7,        /* last argument register */
  FP_REG_LAST_ARG = FP_REG_R7,
  FP_REG_RET = FP_REG_R8,       /* return register */
  FP_REG_RET2 = FP_REG_R9,      /* (other) return register */
  FP_REG_FB = FP_REG_R55,               /* frame base (sometimes) */
  FP_REG_SP = FP_REG_R57,       /* stack pointer */
  FP_REG_LR = FP_REG_R58,       /* link register */

  /*aliases for maestro*/
  FPM_REG_SP = FP_REG_R14,
  FPM_REG_LR = FP_REG_R15,
  FPM_REG_PSR = FP_REG_R16,
  FPM_REG_PC = FP_REG_R17,      /*This is the last General register for Maestro*/
  FPM_REGS_GREG_START = FP_REG_R0,
  FPM_REGS_NUM_GREGS = (FPM_REG_PC + 1),
  FPM_REGS_GREG_LIMIT = (FPM_REGS_GREG_START + FPM_REGS_NUM_GREGS),

  /*Maestro Predicate register number*/
  FPM_REG_P0 = FP_REG_R18,
  FPM_REG_P1 = FP_REG_R19,
  FPM_REGS_PREG_START = FPM_REG_P0,
  FPM_REGS_NUM_PREGS = 2,
  FPM_REGS_PREG_LIMIT = (FPM_REGS_PREG_START + FPM_REGS_NUM_PREGS),

  FPM_REG_A1        = FP_REG_R0, /*first argument register for maestro*/
  FPM_REG_LAST_ARG  = FP_REG_R3, /*last argument register for maestro*/
  FPM_REG_RET       = FP_REG_R0, /*return register for maestro*/
  FPM_REG_RET2      = FP_REG_R1, /*(other) return register for maestro*/

  FPM_REGS_LIMIT = FPM_REGS_PREG_LIMIT,

#if defined (__FPM1015_ONWARDS__)
      FP_REGS_PC    = FPM_REG_PC,
      FP_REGS_PSR   = FPM_REG_PSR,
      FP_REGS_LIMIT = FPM_REGS_LIMIT,
#else
      FP_REGS_PC    = FP_REG_PC,
      FP_REGS_PSR   = FP_REG_PSR,
#  if FIREPATH_REGS_HAS_CREGS
      FP_REGS_LIMIT = FP_REGS_CREG_LIMIT,
#  else
      FP_REGS_LIMIT = FP_REG_P7,
#  endif
#endif
} firepath_regs_t;

#endif /*FIREPATH_REGS_H*/


/*
 * Context information for registers
 * */
typedef struct
{
    bool        b_valid;
    uint32_t    u32_cs_reg_size;
    uint32_t    u32_cs_reg_offset;
} DBG_core_cs_reg_info_t;

/*
 * Register number translation structure
 * */
typedef struct
{
    bool            b_valid;                    /* Indicate if the register range is valid */
    firepath_regs_t fp_reg_lo;                  /* Lower firepath register range */
    firepath_regs_t fp_reg_hi;                  /* Higher firepath register range */
    uint32_t        u32_reg_size;               /* size in bytes of the register */
    uint32_t        u32_dbp_cs_base_offset;     /* offset inside the debug agent context */
                                                /* save structure - to be used in conjuction with */
                                                /* b_valid */
} DBG_core_reg_info_t;

extern void
DBG_core_get_dba_reg_info(uint32_t u32_reg_num, DBG_core_cs_reg_info_t *p_cs_reg_info);

#if (FEATURE_IS(DBG_CORE, DEBUG_SERVER_MP))
TX_REGISTER_ID
DBG_core_gdb_to_threadxp_regnum(firepath_regs_t gdb_regnum);
#endif

#endif
