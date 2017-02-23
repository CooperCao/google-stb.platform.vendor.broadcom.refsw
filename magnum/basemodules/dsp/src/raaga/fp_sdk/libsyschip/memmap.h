/******************************************************************************
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
 ******************************************************************************/


#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#include "fp_sdk_config.h"
#include "memmap-utils.h"

#ifdef GENERIC
#  include "libsyschip/memmap-generic.h"
#endif

#ifdef TACANA
#  include "libsyschip/memmap-tacana.h"
#endif

#ifdef KOS
#  include "libsyschip/memmap-kos.h"
#endif

#if defined(YELLOWSTONE) && defined(__FP4014__)
#  include "libsyschip/memmap-yellowstone.h"
#endif

#if defined(YELLOWSTONE) && defined(__FP4015__)
#  include "libsyschip/memmap-yellowstone-b.h"
#endif

#ifdef RAAGA
#  include "libsyschip/memmap-raaga.h"
#endif

#ifdef CELIVERO
#  include "libsyschip/memmap-celivero.h"
#endif

#ifdef CELTRIX
#  include "libsyschip/memmap-celtrix.h"
#endif

#ifdef DUNA
#  include "libsyschip/memmap-duna.h"
#endif

#ifdef MCPHY
#  include "libsyschip/memmap-mcphy.h"
#endif

#ifdef WHITNEY
#  include "libsyschip/memmap-whitney.h"
#endif

#ifdef PIKE
#  include "libsyschip/memmap-pike.h"
#endif

#ifdef BSP
#  include "libsyschip/memmap-bsp.h"
#endif

#ifdef LEAP_PHY
#  include "libsyschip/memmap-leap-phy.h"
#endif

#ifdef PMC3
#  include "libsyschip/memmap-pmc3.h"
#endif

#if defined(__FP4014__) && !defined(YELLOWSTONE)
#  include "libsyschip/memmap-octave-v1.h"
#endif

#if defined(__FP4015_ONWARDS__) || defined(__FPM1015_ONWARDS__)
#  include "libsyschip/memmap-octave-v2-maestro-v1.h"
#endif

#if defined(TACANA) || defined(KOS) || defined(YELLOWSTONE)
#  include "libsyschip/descriptor.h"
#endif

/*
 Generic memory layout macros for chips with simple memories

 TEXT_START / TEXT_END delimit the main region where instructions live. This
 doesn't include instructions that get loaded after the kernel has started (on
 things like FP4015 Raaga)

 DATA_START / DATA_END delimits a contiguous region where data can be
 stored. This isn't meant to be exhaustive: on chips with DMEM it will point at
 that (and miss any SMEM data).
*/

#if (defined (__FPM1015_ONWARDS__) ||                           \
     (defined (__FP4014_ONWARDS__) && ! defined(YELLOWSTONE)))
# if !defined(ASMCPP) && !defined(__LINKER_SCRIPT__) && defined(__FIREPATH__)
extern void __text_start();
extern void __text_end();
extern int __data_start, __data_end;
# endif
# define TEXT_START_ADDR  SYMBOL_ADDR_TO_ADDR (__text_start)
# define TEXT_END_ADDR    SYMBOL_ADDR_TO_ADDR (__text_end)
# define DATA_START_ADDR  SYMBOL_ADDR_TO_ADDR (__data_start)
# define DATA_END_ADDR    SYMBOL_ADDR_TO_ADDR (__data_end)
#endif

/*
The following VOLATILE/NONVOLATILE macros simply set or unset the
non-volatile bit in the value supplied (bit 24) for kos when the
pcache is being used. For all other systems the macros just return
the value supplied.

The VOLATILE_ADDR/NONVOLATILE_ADDR macros assume the supplied value
is a pointer and performs the necessary casts prior to setting or
unsetting the non-volatile bit. Once the bit is set/unset the
value is cast to a void *. Again for systems other than kos
with the pcache these macros just return the value supplied.
 */
#if defined(KOS) || defined(YELLOWSTONE)
#  define TDMC_BLK    TdmcBlk
#elif defined(TACANA)
#  define TDMC_BLK    SHAREDREGS_blk2
#endif
#if defined(KOS)
#  define NONVOLATILE_BIT        (0x1000000)
#  define VOLATILE(val)          ((val) & ~NONVOLATILE_BIT)
#  define NONVOLATILE(val)       ((val) |  NONVOLATILE_BIT)
#  define VOLATILE_ADDR(addr)    ((void*)(((int)(void*)(addr)) & ~NONVOLATILE_BIT))
#  define NONVOLATILE_ADDR(addr) ((void*)(((int)(void*)(addr)) |  NONVOLATILE_BIT))
#else
#  define NONVOLATILE_BIT        (0x0)
#  define VOLATILE(val)          (val)
#  define NONVOLATILE(val)       (val)
#  define VOLATILE_ADDR(addr)    (addr)
#  define NONVOLATILE_ADDR(addr) (addr)
#endif

/* Also define some more meaningful names. */
#define CACHED_BIT      NONVOLATILE_BIT
#define CACHED          NONVOLATILE
#define CACHED_ADDR     NONVOLATILE_ADDR

/* FIREPATH_NUM handling for the C world */
#if (defined(__FP2012_ONWARDS__) || defined(__FPM1015_ONWARDS__)) && !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
#  if defined(FIREPATH_NUM) || defined(FIREPATH_NUM_UNCACHED)
#    error "FIREPATH_NUM and FIREPATH_NUM_UNCACHED must not be defined for FP2012+/Maestro chips, memmap.h will define them"
#  endif

#  if (NUM_CORES != 1)
#    ifdef __FIREPATH__
#      include <firepath-intrinsics.h>
#      include "libfp/dirs.h"
#    endif /* __FIREPATH__ */
#    define FIREPATH_NUM                FIREPATH_NUM_UNCACHED
#    if defined(__FPM1015_ONWARDS__)
#      define FIREPATH_NUM_UNCACHED     (__fp_dirr_w(DIR_CORE_ID))
#    else
#      define FIREPATH_NUM_UNCACHED     (__fp_dirr(DIR_CORE_ID))
#    endif
#  else
#    define FIREPATH_NUM           0
#    define FIREPATH_NUM_UNCACHED  0
#  endif
#endif


/* FIREPATH_NUM handling for the ASM world */
#ifdef ASMCPP
#  if defined(__FP2012_ONWARDS__) || defined(__FPM1015_ONWARDS__)
#    include "libfp/dirs.h"           /* for the DIR_CORE_ID constant */
#  endif

    /* This macro gets the uncached core id */
#if defined(__FP4014_ONWARDS__) || defined(__FPM1015_ONWARDS__)

#  if defined(__FP4014_ONWARDS__)
    .macro get_core_id  rdest:req
#    if (NUM_CORES != 1)
     nop             : dirr \rdest, #DIR_CORE_ID
#    else
     movl   \rdest, zr
#    endif /* NUM_CORES != 1 */
#  endif  /* __FP4014_ONWARDS__ */

#  if defined(__FPM1015_ONWARDS__)
    .macro get_core_id  rdest:req
#    if (NUM_CORES != 1)
     dirr   \rdest, #DIR_CORE_ID
#    else
     movw   \rdest, #0
#    endif /* NUM_CORES != 1 */
#  endif  /* __FPM1015_ONWARDS__ */

#else

    /* This macro gets the uncached core id */
    .macro get_core_id  rdest:req, p=p7
#if defined(__FP2012_ONWARDS__)

#  if (NUM_CORES != 1)
     sbpf \p\().0, 1f
     nop             : dirr \rdest, #DIR_CORE_ID
1:
#  else
     \p\().movl   \rdest, zr
#  endif /* NUM_CORES != 1 */

#elif defined(FIREPATH_NUM_ASM_ADDR) || (NUM_CORES == 1)

#  if defined(DUNA)
     \p\().ldiw  \rdest, [zr, #FIREPATH_NUM_ASM_ADDR]
     \p\().addwb \rdest, \rdest, #1
#  elif (NUM_CORES != 1)
    \p\().moviwb \rdest, #FIREPATH_NUM_ASM_ADDR
    \p\().ldl    \rdest, [\rdest]
#  else
    \p\().movl   \rdest, zr
#  endif

#else
#  error "Don't know how get core id in get_core_id macro"
#endif  /* __FP2012_ONWARDS__ / FIREPATH_NUM_ASM_ADDR */
#endif  /* __FP4014_ONWARDS__ / __FPM1015_ONWARDS__ */
    .endm
#endif /* ASMCPP */


/* Linker generated ELF HASH */
#if defined(__FIREPATH__) && !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
#  include "libfp/c_utils.h"

/* __fp_seg_hash is weak so that if the linker doesn't provide it, it will have
 * a correct value of 0. __fp_seg_hash_size is weak so that the compiler will
 * not optimise out comparisons like "&__fp_seg_hash_size == 0" (see the gcc
 * -fdelete-null-pointer-checks option). Other symbols are strong, so use them
 * only if you're sure the linker script contains the required snippet from
 * memmap-hash.ldp. */
__absolute __weak extern unsigned long __fp_seg_hash;
__absolute extern unsigned long __fp_seg_hash_addr;
__absolute extern unsigned long __fp_seg_hash_load_addr;
__absolute __weak extern unsigned long __fp_seg_hash_size;
#endif


#endif /* _MEMMAP_H_ */
