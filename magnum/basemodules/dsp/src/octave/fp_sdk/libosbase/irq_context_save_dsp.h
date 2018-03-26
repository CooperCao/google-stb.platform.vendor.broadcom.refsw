/************************************************************************
 *                  Copyright 2000 Element 14 Inc                       *
 *                                                                      *
 *  This material is the confidential trade secret and proprietary      *
 *  information of Element 14 Inc. It may not be reproduced, used, sold *
 *  or transferred to any third party without the prior written consent *
 *  of Element 14 Inc. All rights reserved.                             *
 *                                                                      *
 ************************************************************************/

/**
 * \file
 * \ingroup libosbase
 *
 * Definitions of context structures used during interrupt.
 *
 * During interrupt (or context switch) FirePath core state is stored so that
 * the interrupted code can be resumed without disturbing it. There are two
 * types of state to consider (as defined in EABI):
 *
 *  - Volatile: state not preserved across function calls i.e. r0-r31 + all
 *              special registers.
 *
 *  - Non-volatile: state which is preserved across function calls. Just r32-r61ish.
 *
 * These definitions are significant because, with care, only volatile regs need
 * be stored on interrupt: non-volatile ones ought to be preserved automatically
 * by C compiler (and careful assembly programmers). Conversely only non-volatile
 * state need be stored by ThreadX for a solicited context switch (caused by an API call
 * such as tx_semaphore_get) because the compiler already assumes that all volatile
 * registers are clobbered.
 *
 * Accordingly this file declares two structures: TX_fullContext and
 * TX_solicitedContext. The former is used for interrupts and the latter by
 * ThreadX system return (tx_tsr.fp). They both contain a type field at the same
 * offset so that they can be differentiated by the ThreadX scheduler loop
 * (tx_ts.fp).
 *
 * If ThreadX is not present only TX_fullContext is used (for storing state during
 * interrupt).
 */

/* This was the original tx_ctxt_toliman.h, renamed to allow disambiguation with
 * the Maestro version of this same file. */

#ifndef _IRQ_CONTEXT_SAVE_DSP_H_
#define _IRQ_CONTEXT_SAVE_DSP_H_

/* Guard to disallow direct inclusion */
#if !defined(_IRQ_CONTEXT_SAVE_H_) && !defined(__COMPILE_HEADER__)
#  error "Don't include this header directly, please use irq_context_save.h"
#endif


#include "fp_sdk_config.h"

#include "libfp/context_save.h"


#if !defined(ASMCPP)

#include <stdint.h>


/**
 * State which is stored when entering an interrupt. Name is a bit of a misnomer
 * since it is used whether ThreadX is present or not.
 *
 * FIXME: some of the following struct fields are not used anymore on Octave/Maestro
 * and/or IRQ manager v3 (e.g. u32sp, u32lr), should better live in the
 * CS_registerContext structure (e.g. u64PredReg, u64Valid) or would be better
 * moved at a different position in the structure (e.g. u64TxType). Anyway, there
 * are too many dependencies on the memory layout of this structure and on the
 * presence of some fields in legacy code, so any restructuring will have to be
 * carefully planned. Postponed by now.
 */
typedef struct TX_fullContext
{
#ifdef THREADX_LITE
    /** Interrupts we are servicing */
    uint64_t u64IntLatched;
    /** unused. was irq_r61 on interrupt */
    uint64_t u64Spare;
#else
#  if !__FP4014_ONWARDS__  /* !THREADX_LITE, FP2000 machines */
    /* Store (duplicates of) previous sp and irq_r61 here to aid with debugging
     * (it keeps the stack pointer chain in tact: see PR16160) */
    /** Stack pointer on interrupt */
    uint32_t u32sp;
    /** Lower word of irq_r61 on interrupt */
    uint32_t u32lr;
    /* For the sole purpose of having u64TxType at the right offset */
    uint64_t dummy;
#  else                             /* !THREADX_LITE, Octave machines */
    /* For the sole purpose of having u64TxType at the right offset */
    uint64_t dummy[2];
#  endif
#endif
    /** Whether this is an interrupt or solicited context, note that this field
     * must be at same offset as in TX_solicitedContext. One for interrupt
     * context.  */
    uint64_t u64TxType;
    /** Flag field indicating which fields of the context which are actually valid
     * as bit mask using flags from context_save.h */
    uint64_t u64Valid;
    /** Store predicate registers. */
    uint64_t u64PredReg;
#if CORE_HAS_DREG_FEATURE_ICA
    /** Stored ICACHE locking state (ICACHE configuration on Octave). */
    uint64_t u64ICacheLockBits;
#endif
    /** Store register context. */
    CS_registerContext commonRegisterContext;
} TX_fullContext;

#endif  /* !ASMCPP */

/** \cond boring */
#define TX_msr             (TX_fullContext_commonRegisterContext + CS_registerContext_mReg + CS_mRegContext_u64Msr)
#ifndef __FP4014_ONWARDS__
#  define TX_irqRegs       (TX_fullContext_commonRegisterContext + CS_registerContext_u64IrqRegs)
#  define TX_u32sp         (TX_fullContext_u32sp)
#  define TX_u32lr         (TX_fullContext_u32lr)
#else
#  define TX_pc            (TX_fullContext_commonRegisterContext + CS_registerContext_pc)
#  define TX_psr           (TX_fullContext_commonRegisterContext + CS_registerContext_psr)
#endif
#define TX_userRegs        (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs)
#define TX_r0r1            (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs + 8*0)
#define TX_r2r3            (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs + 8*2)
#define TX_r4r5            (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs + 8*4)
#define TX_r6r7            (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs + 8*6)
#define TX_r8r9            (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs + 8*8)
#define TX_r10r11          (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs + 8*10)
#define TX_r52r53          (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs + 8*52)
#define TX_r57             (TX_fullContext_commonRegisterContext + CS_registerContext_u64UserRegs + 8*57)
#define TX_LOOP            (TX_fullContext_commonRegisterContext + CS_registerContext_u64DirLoopState)
#define TX_LOOP_ADDR       (TX_fullContext_commonRegisterContext + CS_registerContext_u64DirLoopAddr)
#define TX_LOOP_COUNT      (TX_fullContext_commonRegisterContext + CS_registerContext_u64DirLoopCount)
#define TX_LOOP_START      (TX_fullContext_commonRegisterContext + CS_registerContext_u64DirLoop)

#if defined(__FP2006_ONWARDS__) && !__FP4014_ONWARDS__
#  define TX_BitfieldExtractCount   (TX_fullContext_commonRegisterContext + CS_bitFieldExtract_count)
#  define TX_BitfieldCombineCount   (TX_fullContext_commonRegisterContext + CS_bitFieldCombine_count)
#elif __FP4014_ONWARDS__
#  define TX_Bitfield_bcxm          (TX_fullContext_commonRegisterContext + CS_bitFieldState_bcxm)
#  define TX_Bitfield_bxr           (TX_fullContext_commonRegisterContext + CS_bitFieldState_bxr)
#  define TX_Bitfield_bcr           (TX_fullContext_commonRegisterContext + CS_bitFieldState_bcr)
#endif


#define TX_txType          (TX_fullContext_u64TxType)
#define TX_valid           (TX_fullContext_u64Valid)
#define TX_predReg         (TX_fullContext_u64PredReg)
#define TX_iCacheLockBits  (TX_fullContext_u64ICacheLockBits)

#define TX_ctxtSize        (TX_fullContext_SIZE)
/** \endcond */


#if !defined(ASMCPP)

/** The number of general purpose registers which need to be preserved across
 *  function calls.
 *  Pre-Octave (EABI <= 3): we must save r32-r56. r57 (sp) should also be
 *  preserved but is stored elsewhere; same for r58.
 *  Octave (EABI == 4): we must save r32-r56, r59-r61. r57 (sp) should also
 *  be preserved but is stored elsewhere, as well as r58. */
#ifndef __FP4014_ONWARDS__
#  define NON_VOLATILE_USER_REGS    25
#else
#  define NON_VOLATILE_USER_REGS    28
#endif

/**
 * Context stored when entering ThreadX following an API call. Since much state
 * is 'volatile' (not preserved across function calls) this is much smaller
 * than the TX_fullContext stored on interrupt. The first three fields however
 * are shared with that structure.
 *
 * Only used with ThreadX.
 *
 * Note that the u64TxType field must be at the same offset as in TX_fullContext.
 */
typedef struct TX_solicitedContext
{
    /** Program status register */
    uint64_t u64Psr;
    /** Link register */
    uint64_t u64Link;
    /** Field to identify context type. Zero for solicited context. */
    uint64_t u64TxType;
#if CORE_HAS_DREG_FEATURE_ICA
    /** Stored icache locking bits. */
    uint64_t u64ICacheLockBits;
#endif
    /** Stored registers. */
    uint64_t u64NonVolatiles[NON_VOLATILE_USER_REGS];
} TX_solicitedContext;

#endif  /* !ASMCPP */

/** \cond boring */
#define TXSOL_TxType       (TX_solicitedContext_u64TxType)
#define TXSOL_Psr          (TX_solicitedContext_u64Psr)
#define TXSOL_Link         (TX_solicitedContext_u64Link)
#define TXSOL_iCacheLockBits  (TX_solicitedContext_u64ICacheLockBits)
#define TXSOL_NonVols      (TX_solicitedContext_u64NonVolatiles)
#define TXSOL_size         (TX_solicitedContext_SIZE)
/** \endcond */

#endif /* _IRQ_CONTEXT_SAVE_DSP_H_ */
