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

#ifndef _C_UTILS_H_
#define _C_UTILS_H_

/*
  A note about __pid and <unistd.h>:

  The __pid macro stands for Position Independent Data and is important on
  Duna. There, it expands into a GCC attribute. On other platforms, it expands
  to the empty string.

  In unistd.h, the function prototypes have argument names as well as argument
  types. Unfortunately, one of them happens to be __pid. If you include
  libfp/c_utils.h and then unistd.h, these names clash. On non-Duna targets,
  this still isn't a problem: you end up declaring the function with no name for
  an argument. On Duna, however, you end up with something like

        void some_function (int __attribute__((pid)), int arg2);

  which is clearly not going to compile.

  To avoid this problem, just make sure that you include <unistd.h> before
  including libfp/c_utils.h.
*/

#ifndef ASMCPP

#include "fp_sdk_config.h"

/** Likely hint for conditionals */
#define _likely_(x)             __builtin_expect((long)(x),1)
/** Unlikely hint for conditionals */
#define _unlikely_(x)           __builtin_expect((long)(x),0)

#ifdef __FIREPATH__

#if defined(__FP2011_ONWARDS__) && defined(DUNA)
/** Annotate data as absolute */
#  define __absolute            __attribute__((absolute))
/** Annotate data as position independent */
#  define __pid                 __attribute__((pid))
/** Annotate data as GOT indirect (only effective on Duna) */
#  define __got                 __attribute__((got))
#elif defined(__FP4015_ONWARDS__) && defined(__PIC__)
/* On builds using SBA-relative addressing for PIC binaries, we just need the
 * absolute attribute to specify that a data symbol is _NOT_ SBA-relative.
 * Such variables must not be not be in a section marked PID and are shared
 * amongst all instances of a program. Typically this annotation will be used
 * on an extern linker-defined symbol, or it is used in conjunction with a
 * section attribute such as __shared to move it out of the default
 * SBA-relative data section. */
#  define __absolute            __attribute__((absolute))
#  define __pid
#  define __got
#else
#  define __absolute
#  define __pid
#  define __got
#endif

#define QUOTE_ARG(str) #str

#define __DOMAIN(n)             (1ULL << (n))
#define __DOMAIN_RANGE(u, l)    (((1ULL << ((u)+1ULL)) - 1ULL) & ~((1ULL << (l)) - 1ULL))

/* The temporal Init-Normal-Overlay domains */
/** Annotate code or data as init (These can be reclaimed after booting)
 *
 * The Init domain can call the Normal (default) and itself.
 */
#define __init                          __attribute__((domain("0:Temporal",  0, __DOMAIN(63) | __DOMAIN(0), ".initrt")))


#if defined(ENABLE_OVERLAYS) || defined(__COMPILE_HEADER__)

/** Annotate code as associated to code overlay region r overlay number n
 *
 * The code overlay domain can access the Normal domain, itself, code
 * overlays in other regions and any of the data overlay domains,
 * and is accessible from the normal domain.
 */
#define __code_overlay(r, n)            __attribute__((domain("0:Temporal", 32 + (r) * OVERLAYS_CODE_MAX_OVERLAYS_PER_REGION + (n), \
                                                       __DOMAIN(63) | __DOMAIN_RANGE(47, 40) |                                      \
                                                       (__DOMAIN_RANGE(39, 32) & ~__DOMAIN_RANGE(32 + ((r) + 1) * OVERLAYS_CODE_MAX_OVERLAYS_PER_REGION - 1, 32 + (r) * OVERLAYS_CODE_MAX_OVERLAYS_PER_REGION)) |   \
                                                       __DOMAIN(32 + (r) * OVERLAYS_CODE_MAX_OVERLAYS_PER_REGION + (n)),            \
                                                       ".overlay.t." QUOTE_ARG(r) "." QUOTE_ARG(n))))
#if OVERLAYS_CODE_NUM_REGIONS * OVERLAYS_CODE_MAX_OVERLAYS_PER_REGION > 8
#  error "The number of possible code overlays overflows the available code overlays domain bits"
#endif

/** Annotate data as associated to data overlay region r overlay number n
 *
 * The data overlay domain can access the Normal domain, itself, data
 * overlays in other regions and any of the code overlay domains,
 * and is accessible from the normal domain.
 * */
#define __data_overlay(r, n)            __attribute__((domain("0:Temporal", 40 + (r) * OVERLAYS_DATA_MAX_OVERLAYS_PER_REGION + (n), \
                                                       __DOMAIN(63) | __DOMAIN_RANGE(39, 32) |                                      \
                                                       (__DOMAIN_RANGE(47, 40) & ~__DOMAIN_RANGE(40 + ((r) + 1) * OVERLAYS_DATA_MAX_OVERLAYS_PER_REGION - 1, 40 + (r) * OVERLAYS_DATA_MAX_OVERLAYS_PER_REGION)) |   \
                                                       __DOMAIN(40 + (r) * OVERLAYS_DATA_MAX_OVERLAYS_PER_REGION + (n)),            \
                                                       ".overlay.d." QUOTE_ARG(r) "." QUOTE_ARG(n))))
#if OVERLAYS_DATA_NUM_REGIONS * OVERLAYS_DATA_MAX_OVERLAYS_PER_REGION > 8
#  error "The number of possible data overlays overflows the available data overlays domain bits"
#endif

/** Annotate code or data that cannot be accessed from overlay domains
 *
 * Anything marked with this is just like regular code/data, but overlay
 * code/data cannot reference it
 */
#define __not_overlay_safe              __attribute__((domain("0:Temporal", 62, __DOMAIN(63) | __DOMAIN_RANGE(62, 32), "")))

#endif  /* ENABLE_OVERLAYS */


/* The spatial Shared-Private(N) domains */
/** Annotate code or data as private (only accessible on a particular core
 *
 * The Private domains can call the Shared (default) domain, but not different
 * private domain.
 */
#define __private(n)                    __attribute__((domain("1:Spatial", n, __DOMAIN(63) | __DOMAIN(n), ".private." QUOTE_ARG(n))))

/** Annotate code as needed to be placed low in memory */
#define __low                           __attribute__((section_prefix(".low")))
/** Annotate data as protected */
#define __protected                     __attribute__((section_prefix(".protected")))
/** Data / bss that must not be copied / zeroed by __runtime_init */
#define __noinit                        __attribute__((section_prefix(".noinit")))

#if defined(SHARED_MEMORY_MULTICORE)
#  if defined(__PIC__)
/** Annotate data as shared, which must necessarily be absolute too */
#    define __shared                    __attribute__((section_prefix(".shared"))) __absolute
/** Annotate data as protected shared, which must necessarily be absolute too */
#    define __protected_shared          __attribute__((section_prefix(".protected.shared"))) __absolute
#  else
/** Shared data is not special on shared memory-multicores without PIC */
#    define __shared
/** Annotate data as protected - the shared attribute is not meaningful on shared memory-multicores without PIC*/
#    define __protected_shared          __protected
#  endif
#elif NUM_CORES > 1
/** Annotate data as shared */
#  define __shared                      __attribute__((section_prefix(".shared")))
/** Annotate data as protected shared */
#  define __protected_shared            __attribute__((section_prefix(".protected.shared")))
#else
/** Shared data is not special in single core SoCs */
#  define __shared
/** Annotate data as protected - the shared attribute is not meaningful in single core SoCs */
#  define __protected_shared            __protected
#endif

/*
  On a Kos chip with pcache, and on Yellowstone with its DCache, we
  have to distinguish between volatile and non-volatile
  access. Everywhere else, it doesn't make any difference. Rather than
  having to declare lots of volatile sections in every other chip's
  linker script, make __volatile a NOP on other targets.
*/
#if defined(KOS)
/** Annotate data as shared with volatile access (2012 P cache) */
# define __shared_volatile              __attribute__((section_prefix(".shared.volatile")))
/** Annotate data as protected shared with volatile access (2012 P cache) */
# define __protected_shared_volatile    __attribute__((section_prefix(".protected.shared.volatile")))
#elif defined(YELLOWSTONE)
/** Annotate data as shared with volatile access (*not* in FP4014 DataCache) */
# define __shared_volatile              __attribute__((section_prefix(".shared.volatile")))
/** Annotate data as protected shared with volatile access (no support for this in Yellowstone) */
# define __protected_shared_volatile    __protected_shared
#else
/** Annotate data as shared with volatile access (makes no difference when no pcache) */
# define __shared_volatile              __shared
/** Annotate data as protected shared with volatile access (makes no difference when no pcache) */
# define __protected_shared_volatile    __protected_shared
#endif

/** Force a 'default' symbol visibility, that is not 'hidden' */
#define __export                        __attribute__((export))

/** Annotate code as going into virtual space */
#define __vmem                          __attribute__((section_prefix(".vmem")))

/**
 * Annotate code as belonging to a named vmem code group
 *
 * Pass n between 0 and 7 to put the code in one of the 8 groups.
 */
#define __grouped_vmem(n)               __attribute__((section_prefix(".vmem" QUOTE_ARG(n))))

#if defined(CELIVERO) || defined(CELTRIX)
#  define __buf4k                       __attribute__((section_prefix(".buf4k")))
#endif

#endif  /* __FIREPATH__ */


/** For weak declarations */
#define  __weak                         __attribute__((weak))

/** Functions that should not be dictionary compressed */
#if defined(__FP4014_ONWARDS__) || defined(__FPM1015_ONWARDS__)
#  define __nocompress                  /* no-op on non-FP20xx machines */
#else
#  define __nocompress                  __attribute__((nocompress))
#endif

/** Compile function in debug mode */
#define __debug                         __attribute__((optimize("-O0")))

/** Force inlining */
#define __alwaysinline                  __attribute__((always_inline))

/** Disable inlining */
#define __noinline                      __attribute__((noinline))

/** Mark as not used */
#define __unused                        __attribute__((unused))

/** Avoid extra padding in structs and enums */
#define __packed                        __attribute__((packed))

/** Instruct the compiler to align a variable on an n-byte boundary */
#define __align(n)                      __attribute__((aligned(n)))


#if !defined _Static_assert && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L)
/* A fake _Static_assert implementation that we can use in pre-C11 mode.
 *
 * The error messages are nowhere near as good, but it will still explode at the
 * right time.
 */
#define _Static_assert(exp, msg) \
    typedef int _Static_assert_type [(exp) ? 1 : -1] __attribute__((unused))
#endif

#endif /* !defined(ASMCPP) */

#endif /* _C_UTILS_H_ */
