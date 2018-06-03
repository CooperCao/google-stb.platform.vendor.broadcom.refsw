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

/**
 * \file
 * \ingroup libfp
 * \brief Routines to save and restore the processor state in a context structure.
 */

#ifndef _CONTEXT_SAVE_DSP_H_
#define _CONTEXT_SAVE_DSP_H_

/* Guard to disallow direct inclusion */
#if !defined(_CONTEXT_SAVE_H_) && !defined(__COMPILE_HEADER__)
#  error "Don't include this header directly, please use context_save.h"
#endif

#include "fp_sdk_config.h"

#include "libfp/dirs.h"
#include "libfp/fp_regs.h"



#if !defined(ASMCPP)

#include <stdint.h>

/** Store current registers in context. Not suitable for calling from C code. */
void storeContext( void );

/** Restore registers from context. Not suitable for calling from C code. */
void restoreContext( void );


#ifdef MAC_CONTEXT

/** Number of uint64_t used to store MREGs. */
# define M_REGS_SIZE_IN_UINT64S            (2*M_REG_NUM + 2)
/** Number of uint64_t used to store MAC status register. */
# define MSR_SIZE_IN_UINT64S               (1)

/** MREGs are stored interleaved in parts. */
#  if defined (__FP2006__) || defined (__FP2008__)
typedef struct CS_mRegPart
{
    /** Interleaved parts of all MREGs */
    uint64_t u64MReg[M_REG_NUM];
} CS_mRegPart;
#  else
/** fp2011 onwards, MREGs are stored in an implementation defined way */
typedef struct CS_mReg
{
    /** MREG parts */
    uint64_t u64MReg[3];
} CS_mReg;
#  endif

/**
 * For fp200[3-8]:
 *   Stored MREG layout.
 *   2.5 8-byte chuncks per MREG.
 *   B: Bottom, MT: Middle if H, top if W, T: Top if H
 *   (see section 8.1.3 of the fp2006 architecture guide).
 *
 * For fp2011+:
 *   MRegs are stored in the natural layout as produced by getmaca and getmacb.
 */
typedef struct CS_mRegInfo
{
#  if defined (__FP2006__) || defined (__FP2008__)
    /**
     * Bottom parts of all MREGs.
     * mReg0B,  mReg1B,  mReg2B,  mReg3B
     */
    CS_mRegPart mRegB;
    /**
     * Middle or top parts (depends on mode).
     * mReg0MT, mReg1MT, mReg2MT, mReg3MT
     */
    CS_mRegPart mRegMorT;

    /**
     * Top parts of m0,m2 if H mode.
     * mReg0T interleaved with mReg2T, (Bytes 0,2,4,6 - mReg0T, Bytes 1,3,5,7 - mReg2T)
     */
    uint64_t u64MReg02T;
    /**
     * Top parts of m1,m3 if H mode.
     * mReg1T interleaved with mReg3T, (Bytes 0,2,4,6 - mReg1T, Bytes 1,3,5,7 - mReg3T)
     */
    uint64_t u64MReg13T;
#  else
    /**
     * Top parts (available in all modes)
     * mReg0T, mReg1T, mReg2T, mReg3T
     */
    CS_mReg mRegs[M_REG_NUM];
#  endif
} CS_mRegInfo;

/** Stored MREGs. */
typedef struct CS_mRegContext
{
    /** The MREG data. */
    CS_mRegInfo mRegInfo;
#  ifdef __FP4014_ONWARDS__
    /** The MSRs */
    uint64_t    u64Msr[MSR_SIZE_IN_UINT64S * 4];
#  else
    /** The MSR */
    uint64_t    u64Msr[MSR_SIZE_IN_UINT64S];
#  endif
} CS_mRegContext;

#endif  // MAC_CONTEXT


#ifdef SIMD_BITFIELD_CONTEXT

#  ifdef __FP4014_ONWARDS__
typedef struct CS_bitFieldState
{
    uint64_t bcxm;
    uint64_t bxr[4];
    uint64_t bcr[2];
} CS_bitFieldState;
#  else
/** Stored bit extractor state */
typedef struct CS_simdBitFieldExtractor
{
    /** @{ */
    uint64_t u64BxReservoir[4];
    uint64_t u64BxCount;
    uint64_t u64BxTrellis;
    /** @} */
} CS_simdBitFieldExtractor;

/** Store bit combiner state */
typedef struct CS_simdBitFieldCombiner
{
    /** @{ */
    uint64_t u64BcReservoir[2];
    uint64_t u64BcCount;
    /** @} */
} CS_simdBitFieldCombiner;

/** Store bit-field manipulation state */
typedef struct CS_bitFieldState
{
    /** @{ */
    CS_simdBitFieldExtractor simdBitFieldExtractor;
    CS_simdBitFieldCombiner  simdBitFieldCombiner;
    /** @} */
} CS_bitFieldState;
#  endif /* !FP4014 onwards */

#endif  // SIMD_BITFIELD_CONTEXT


#ifdef REED_SOLOMON_CONTEXT

/** Reed solomon decoder state. */
typedef struct CS_reedSolomonState
{
    /** @{ */
    uint64_t u64RsAccX[2];
    uint64_t u64RsAccY[2];
    /** @} */
} CS_reedSolomonState;

#endif  // REED_SOLOMON_CONTEXT


#ifdef TRELLIS_STATE_CONTEXT

/** Trellis decoder state. */
typedef struct CS_trellisState
{
    /** Pipe state. */
    uint64_t u64TrellisDecodePipeState[2];
    /** Other state. */
    uint64_t u64TrellisState;
} CS_trellisState;

#endif  // TRELLIS_STATE_CONTEXT


#ifdef FIR_STATE_CONTEXT

/** FIR state */
typedef struct CS_firState
{
    /** @{ */
#ifdef __FP4014_ONWARDS__
    uint64_t state[4];
#else
    uint64_t u64x;
    uint64_t u64y;
#endif
    /** @} */
} CS_firState;

#endif  // FIR_STATE_CONTEXT


/* FIXME: this structure might get split/reorganised in the future to fix the
 * issue described in TLFIREPATH-2483 (excessive stack usage in nested interrupts). */
/** Layout of the full register context. */
typedef struct CS_registerContext
{
    /** General purpose registers, volatiles and non volatiles
     * (user bank only, in case of banked registers) */
    uint64_t u64UserRegs[USER_REG_NUM];

#ifdef CORE_HAS_BANKED_REGS
    /** irq_r60/61, but sometimes svc_r60/61! */
    uint64_t u64IrqRegs[IRQ_REG_NUM];
#else
    /** Program Counter */
    uint64_t pc;
    /** Program Status Register */
    uint64_t psr;
#endif

#ifdef MAC_CONTEXT
    /** MAC regs and MSR. */
    CS_mRegContext mReg;
#endif
#ifdef __FP4014_ONWARDS__
    uint64_t u64DirLoopAddr;
    uint64_t u64DirLoopCount;
    uint64_t u64DirLoopStart;
#elif defined(DIR_SEPARATE_LOOP_COUNT_STATE)
    /** Loop state address */
    uint64_t u64DirLoopAddr;
    /** Loop state count */
    uint64_t u64DirLoopCount;
#else
    /** Loop state */
    uint64_t u64DirLoopState;
#endif

    /** @{ */
#ifdef SIMD_BITFIELD_CONTEXT
    CS_bitFieldState bitFieldState;
#endif
#ifdef REED_SOLOMON_CONTEXT
    CS_reedSolomonState reedSolomonState;
#endif
#ifdef TRELLIS_STATE_CONTEXT
    CS_trellisState trellisState;
#endif
#ifdef FIR_STATE_CONTEXT
    CS_firState firState;
#endif
    /** @} */
} CS_registerContext;

/** Storage space for full volatile context. */
typedef struct CS_FullVolatileContext
{
    /** MAC regs and MSR. */
#ifdef MAC_CONTEXT
    CS_mRegContext mReg;
#endif
    /** @{ */
#ifdef SIMD_BITFIELD_CONTEXT
    CS_bitFieldState bitFieldState;
#endif
#ifdef REED_SOLOMON_CONTEXT
    CS_reedSolomonState reedSolomonState;
#endif
#ifdef TRELLIS_STATE_CONTEXT
    CS_trellisState trellisState;
#endif
#ifdef FIR_STATE_CONTEXT
    CS_firState firState;
#endif
    /** @} */
} CS_FullVolatileContext;

#endif /* ASMCPP */


/** \cond boring */

/* Convenience Defines for Structure Offsets. For use in assembly files. */
#define CS_userRegs                             (CS_registerContext_u64UserRegs)
#define CS_irqRegs                              (CS_registerContext_u64IrqRegs)

#define CS_mRegInfo                             (CS_registerContext_mReg + CS_mRegContext_mRegInfo)

#define CS_dirLoopState                         (CS_registerContext_u64DirLoopState)
#define CS_dirLoopAddr                          (CS_registerContext_u64DirLoopAddr)
#define CS_dirLoopCount                         (CS_registerContext_u64DirLoopCount)
#define CS_dirLoopStart                         (CS_registerContext_u64DirLoopStart)

#ifdef __FP4014_ONWARDS__
#  define CS_bitFieldState_bcxm                 (CS_registerContext_bitFieldState + CS_bitFieldState_bcxm)
#  define CS_bitFieldState_bxr                  (CS_registerContext_bitFieldState + CS_bitFieldState_bxr)
#  define CS_bitFieldState_bcr                  (CS_registerContext_bitFieldState + CS_bitFieldState_bcr)
#else
#  define CS_bitFieldExtract_reservoir          (CS_registerContext_bitFieldState + CS_bitFieldState_simdBitFieldExtractor + CS_simdBitFieldExtractor_u64BxReservoir)
#  define CS_bitFieldExtract_trellis            (CS_registerContext_bitFieldState + CS_bitFieldState_simdBitFieldExtractor + CS_simdBitFieldExtractor_u64BxTrellis)
#  define CS_bitFieldExtract_count              (CS_registerContext_bitFieldState + CS_bitFieldState_simdBitFieldExtractor + CS_simdBitFieldExtractor_u64BxCount)
#  define CS_bitFieldCombine_reservoir          (CS_registerContext_bitFieldState + CS_bitFieldState_simdBitFieldCombiner  + CS_simdBitFieldCombiner_u64BcReservoir)
#  define CS_bitFieldCombine_count              (CS_registerContext_bitFieldState + CS_bitFieldState_simdBitFieldCombiner  + CS_simdBitFieldCombiner_u64BcCount)
#endif

#define CS_reedSolomon_x                        (CS_registerContext_reedSolomonState + CS_reedSolomonState_u64RsAccX)
#define CS_reedSolomon_y                        (CS_registerContext_reedSolomonState + CS_reedSolomonState_u64RsAccY)

#define CS_trellisDecode_pipeState              (CS_registerContext_trellisState + CS_trellisState_u64TrellisDecodePipeState)
#define CS_trellisDecode_state                  (CS_registerContext_trellisState + CS_trellisState_u64TrellisState)

#define CS_rakeState_scramblerAX                (CS_registerContext_rakeState + CS_rakeState_u64ScramblerA_X)
#define CS_rakeState_scramblerBX                (CS_registerContext_rakeState + CS_rakeState_u64ScramblerB_X)
#define CS_rakeState_dataCCX                    (CS_registerContext_rakeState + CS_rakeState_u64DataCC_X)
#define CS_rakeState_scramblerAY                (CS_registerContext_rakeState + CS_rakeState_u64ScramblerA_Y)
#define CS_rakeState_scramblerBY                (CS_registerContext_rakeState + CS_rakeState_u64ScramblerB_Y)
#define CS_rakeState_dataCCY                    (CS_registerContext_rakeState + CS_rakeState_u64DataCC_Y)
#define CS_rakeState_rkaccCX                    (CS_registerContext_rakeState + CS_rakeState_u64RkaccC_X)
#define CS_rakeState_rkaccDX                    (CS_registerContext_rakeState + CS_rakeState_u64RkaccD_X)
#define CS_rakeState_rkaccCY                    (CS_registerContext_rakeState + CS_rakeState_u64RkaccC_Y)
#define CS_rakeState_rkaccDY                    (CS_registerContext_rakeState + CS_rakeState_u64RkaccD_Y)

#define CS_fir                                  (CS_registerContext_firState)
#define CS_firState_x                           (CS_registerContext_firState + CS_firState_u64x)
#define CS_firState_y                           (CS_registerContext_firState + CS_firState_u64x)

/** \endcond */

#endif /* _CONTEXT_SAVE_DSP_H_ */
