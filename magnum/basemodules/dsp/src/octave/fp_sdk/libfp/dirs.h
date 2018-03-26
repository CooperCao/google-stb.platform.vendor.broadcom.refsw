/******************************************************************************
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
 ******************************************************************************/

/**
 * \file
 * \ingroup libfp
 * \brief Numbers for hardware directives
 *
 * This file lists numbers of hardware directives
 * for use with the dirr, dirw and dirs instructions. For
 * more detailed descriptions, priviledge requirements and
 * barrier / timing constraints see the architecture guide.
 *
 * For all architectures, these might apply:
 *  - ro == read only
 *  - wo == write only
 *  - rw == read / write
 * For fp2000 architectures, these might apply:
 *  - X-side == must be executed on X-side (left) of core
 *  - Y-side == must be executed on Y-side (right) of core
 */

#ifndef _DIRS_H_
#define _DIRS_H_

#include "fp_sdk_config.h"
#include "dreg-numbers.h"

#include "libfp/c_utils.h"


/**
 * \name Core Timers
 * @{
 */

/**
 * \brief TOTC counter.
 *
 * Y side, ro. Hardware counter giving total cycles since reset.
 */
#define DIR_TIME_TOTC         DIR_TOTC

/**
 * \brief TOTC comparison timer 1
 *
 * Y side, rw. Raise an interrupt when TOTC is this value.
 */
#define DIR_TIME_COMP1        DIR_TOTC_CMP1

/**
 * \brief TOTC comparison timer 2
 *
 * Y side, rw. Raise an interrupt when TOTC is this value.
 */
#define DIR_TIME_COMP2        DIR_TOTC_CMP2

/* DIR_USRC and the associated DIR_USRC_CMP comparator are not available on
 * Octave v1. DIR_USRC is back on Octave v2 and Maestro, but not the comparator.
 * See CRFIREPATH-987 for details. */
#if defined(DIR_USRC)
/**
 * \brief USRC counter.
 *
 * Y side, ro. Hardware counter giving total user bank cycles since reset.
 */
#  define DIR_TIME_USRC        DIR_USRC
#endif

#if defined(DIR_USRC_CMP)
/**
 * \brief USRC comparison timer 1
 *
 * Y side, rw. Raise an interrupt when USRC is this value.
 */
#  define DIR_TIME_COMP3       DIR_USRC_CMP
#endif

/**
 * @}
 * \name Super Interrupts
 * @{
 */

/*
  On Octave, super interrupts are called "SIRQ" in the FPDB. On earlier cores,
  they were called "SI". Similarly, ordinary interrupts (formerly called "INT")
  are called IRQE on Octave cores, for some reason.
*/
#ifdef DIR_SI_ENABLE
# define DIR_SI_PREFIX  DIR_SI_
# define DIR_INT_PREFIX DIR_INT_
#else
# define DIR_SI_PREFIX  DIR_SIRQ_
# define DIR_INT_PREFIX DIR_IRQE_
#endif
# define DIR_IRQ_CAT1(x,y) (x ## y)
# define DIR_IRQ_CAT(x,y)  DIR_IRQ_CAT1(x, y)

/**
 * \brief Enabled super interrupts.
 *
 * Y side, rw. Bitmap of enabled super interrupts.
 */
#define DIR_SUPINT_ENABLE     DIR_IRQ_CAT(DIR_SI_PREFIX, ENABLE)
/**
 * \brief Latched super interrupts.
 *
 * Y side, ro. Bitmap of latched super interrupts i.e. those sirqs
 * which would be raised if corresponding enable bit is on and
 * sirqs are enabled in PSR.
 */
#define DIR_SUPINT_LATCH      DIR_IRQ_CAT(DIR_SI_PREFIX, LATCHED)
/**
 * \brief Clear latched super interrupts.
 *
 * Y side, wo. Clear (zero) given bits in the super interrupt latch register.
 */
#define DIR_SUPINT_LATCH_CLR  DIR_IRQ_CAT(DIR_SI_PREFIX, CLEAR)
/**
 * \brief Set latched super interrupts.
 *
 * Y side, wo. Set (to one) given bits in the super interrupt latch register.
 * This will raise the given sirq if enabled.
 */
#define DIR_SUPINT_LATCH_SET  DIR_IRQ_CAT(DIR_SI_PREFIX, SET)
/**
 * \brief Raw super interrupts state.
 *
 * Y side, ro. Super interrupts lines which are currently high. Transient
 * highs might be read as zero.
 */
#define DIR_SUPINT_RAW        DIR_IRQ_CAT(DIR_SI_PREFIX, RAW)

/**
 * @}
 * \name Ordinary interrupts
 * @{
 */

/* fp2012/kos dreg-numbers.h already defines this, so
   conditionally define it here. */
#ifndef DIR_INT_ENABLE
/**
 * \brief Enabled interrupts.
 *
 * Y side, rw. Bitmap of enabled interrupts.
 */
#define DIR_INT_ENABLE        DIR_IRQ_CAT(DIR_INT_PREFIX, ENABLE)
#endif

/**
 * \brief Latched interrupts.
 *
 * Y side, ro. Bitmap of latched interrupts i.e. those irqs
 * which would be raised if corresponding enable bit is on and
 * irqs are enabled in PSR.
 */
#define DIR_INT_LATCH         DIR_IRQ_CAT(DIR_INT_PREFIX, LATCHED)
/**
 * \brief Clear latched interrupts.
 *
 * Y side, wo. Clear (zero) given bits in the interrupt latch register.
 */
#define DIR_INT_LATCH_CLR     DIR_IRQ_CAT(DIR_INT_PREFIX, CLEAR)
/**
 * \brief Set latched interrupts.
 *
 * Y side, wo. Set (to one) given bits in the interrupt latch register.
 * This will raise the given irq if enabled.
 */
#define DIR_INT_LATCH_SET     DIR_IRQ_CAT(DIR_INT_PREFIX, SET)

/* fp2012/kos dreg-numbers.h already defines this, so
   conditionally define it here. */
#ifndef DIR_INT_RAW
/**
 * \brief Raw interrupts state.
 *
 * Y side, ro. Interrupts lines which are currently high. Transient
 * highs might be read as zero.
 */
#define DIR_INT_RAW           DIR_IRQ_CAT(DIR_INT_PREFIX, RAW)
#endif

/**
 * @}
 * \name  Debugging Features
 * \brief Directives for the use of the debug agent.
 * Details are found in architecture guide.
 * @{
 */

/* console */

#define DIR_DEBUG_CONS_READ   DIR_CONSOLE_READ
#define DIR_DEBUG_CONS_WRITE  DIR_CONSOLE_WRITE

/* breakpoints */

#define DIR_BREAK1_LOW_ADR    DIR_BPT1_ADDR
#define DIR_BREAK2_LOW_ADR    DIR_BPT2_ADDR

/* watchpoints */

#ifndef __FP2011_ONWARDS__
#  define DIR_WATCH1_ADR      DIR_WPT1_ADDR
#endif
/* The names for later chips match the FPDB */

/**
 * @}
 * \name Miscellaneous
 * @{
 */

/** \brief icache control */
#define DIR_ICACHE_CTRL       DIR_ICACHE_CONTROL

 /* FP2008 + future */
#ifndef __FP2006__
#  define DIR_SEPARATE_LOOP_COUNT_STATE
#endif

/** @} */

/**
 * \name Memory protection
 * @{
 */
/* (The names for newer chips match the FPDB) */
#if CORE_HAS_MEM_PROT_FP2006
#   define DIR_DMEM_PROT       DIR_DMEM_WRITE_PROT

#   define DMEM_PROT_NONE       0
#   define DMEM_PROT_2K         1
#   define DMEM_PROT_4K         2
#   define DMEM_PROT_8K         3
#   define DMEM_PROT_16K        4
#   define DMEM_PROT_32K        5

#   define DIR_SMEM_PROT       DIR_SMEM_WRITE_PROT

#   define SMEM_PROT_NONE       0
#   define SMEM_PROT_128K       1
#   define SMEM_PROT_256K       2
#   define SMEM_PROT_384K       3
#endif /* memory protection */

/**
 * @}
 * \name Profiling counters
 * @{
 */
/* These don't exist on Octave, I don't think? */
#if defined(__FP2006__) || defined(__FP2008__) || \
    defined(__FP2011__) || defined(__FP2012__)
# define ICACHE_MISS_INSN_CTR    DIR_ICACHE_MISS_INSN_CTR
# define ICACHE_MISS_CYCLE_CTR   DIR_ICACHE_MISS_CYCLE_CTR
# define ISSUED_INSN_CTR         DIR_ISSUED_INSN_CTR
# define PROFILE_CYCLE_CTR       DIR_PROFILE_CYCLE_CTR
# define PROFILE_START_ADDR      DIR_PROFILE_START_ADDR
# define PROFILE_END_ADDR        DIR_PROFILE_STOP_ADDR
#endif
/** @} */


#if __FP2012__

/* FP2012 pcache */
#define DIR_PCACHE               DIR_PCACHE_CONTROL

/* Medium branch link register */
#define DIR_MB_LINK              DIR_MB_LINK_REG

#endif

/*
  Tell Doxygen not to document the opcode dictionary table, since we assume that
  the user won't need to fiddle with it...
 */
#ifndef DOXYGEN
#if __FP2008_ONWARDS__
#  ifdef  __FP2012_ONWARDS__
#    define DICT_TABLE_SIZE       192
#    define DIR_DICT_TABLE_PTR    DIR_DICTOP_TABLE_PTR
#    define DIR_DICT_TABLE_DATA   DIR_DICTOP_TABLE_DATA
#  else
/*
  DIR_DICT_TABLE_* exist on fp2008+ but aren't in the FPDB for some reason, so
  we hardcode their values here
*/
#    define DIR_DICT_TABLE_PTR    172
#    define DIR_DICT_TABLE_DATA   173

#    define DICT_TABLE_SIZE       272
#    define DICT_TABLE_MB_LINK    272
#  endif
#endif /* >= fp2008 */
#endif /*DOXYGEN*/


/* Allow this header to be included also from assembler and host code */
#if defined(__FIREPATH__) && !defined(ASMCPP)

#include <stdint.h>

#include <firepath-intrinsics.h>


/* The compiler provides __fp_dir*_{w,l} versions of dir operations intrinsics
 * only on Maestro. Provide here some macros for uniformity on non-Maestro
 * targets. */
#ifndef __FPM1015_ONWARDS__
#  define __fp_dirr_w(d)        ((uint32_t) __fp_dirr(d))
#  define __fp_dirr_l(d)        (__fp_dirr(d))
#  define __fp_dirw_w(v, d)     (__fp_dirw(v, d))
#  define __fp_dirw_l(v, d)     (__fp_dirw(v, d))
#  define __fp_dirs_w(v, d)     ((uint32_t) __fp_dirs(v, d))
#  define __fp_dirs_l(v, d)     (__fp_dirs(v, d))
#endif


/**
 * Fetch the value of the hardware TOTC timer.
 */
__alwaysinline
static inline uint64_t get_totc(void)
{
    return __fp_dirr_l(DIR_TIME_TOTC);
}


#if __FP4014_ONWARDS__ || __FPM1015_ONWARDS__
/**
 * Set the value of the hardware TOTC timer.
 */
__alwaysinline
static inline void set_totc(uint64_t new_value)
{
    __fp_dirw_l(new_value, DIR_TIME_TOTC);
    // DIRW_NOTE: a BARRIERFE is required as this affects the frontend.
    __fp_barrierfe();
}
#endif  /* Maestro and Octave only */


/**
 * Get the TOTC comparison timer 1 value.
 */
__alwaysinline
static inline uint64_t get_totc_comp1(void)
{
    return __fp_dirr_l(DIR_TIME_COMP1);
}

/**
 * Set the TOTC comparison timer 1 to value.
 */
__alwaysinline
static inline void set_totc_comp1(uint64_t interrupt_time)
{
    __fp_dirw_l(interrupt_time, DIR_TIME_COMP1);
#if __FP4014_ONWARDS__ || __FPM1015_ONWARDS__
    // DIRW_NOTE: a BARRIERFE is required to force the update.
    __fp_barrierfe();
#endif
}

/**
 * Get the TOTC comparison timer 2 value.
 */
__alwaysinline
static inline uint64_t get_totc_comp2(void)
{
    return __fp_dirr_l(DIR_TIME_COMP2);
}

/**
 * Set the TOTC comparison timer 2 to value.
 */
__alwaysinline
static inline void set_totc_comp2(uint64_t interrupt_time)
{
    __fp_dirw_l(interrupt_time, DIR_TIME_COMP2);
#if __FP4014_ONWARDS__ || __FPM1015_ONWARDS__
    // DIRW_NOTE: a BARRIERFE is required to force the update.
    __fp_barrierfe();
#endif
}

#if defined(DIR_TIME_USRC)
/**
 * Fetch the value of the hardware USRC timer.
 */
__alwaysinline
static inline uint64_t get_usrc(void)
{
    return __fp_dirr_l(DIR_TIME_USRC);
}
#endif

#if defined(DIR_TIME_COMP3)
/**
 * Get the USRC comparison timer value.
 */
__alwaysinline
static inline uint64_t get_usrc_comp1(void)
{
    return __fp_dirr_l(DIR_TIME_COMP3);
}

/**
 * Set the USRC comparison timer to value.
 */
__alwaysinline
static inline void set_usrc_comp1(uint64_t interrupt_time)
{
    __fp_dirw_l(interrupt_time, DIR_TIME_COMP3);
#if __FP4014_ONWARDS__ || __FPM1015_ONWARDS__
    // DIRW_NOTE: a BARRIERFE is required to force the update.
    __fp_barrierfe();
#endif
}
#endif // DIR_USRC_CMP

#endif /* defined(__FIREPATH__) && !defined(ASMCPP) */

#endif /* _DIRS_H_ */
