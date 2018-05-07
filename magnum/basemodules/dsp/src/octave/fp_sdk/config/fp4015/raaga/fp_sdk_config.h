/************************************************************************
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
 ************************************************************************/

/************************************************************************
 *                                                                      *
 *                    #####   ######     ###    ######                  *
 *                   ##   ##    ##      ## ##   ##   ##                 *
 *                   ##         ##     ##   ##  ##   ##                 *
 *                    #####     ##     ##   ##  ######                  *
 *                        ##    ##     ##   ##  ##                      *
 *                   ##   ##    ##      ## ##   ##                      *
 *                    #####     ##       ###    ##                      *
 *                                                                      *
 *  This file is auto generted by SCons from config.h.in - DO NOT EDIT! *
 *                                                                      *
 ************************************************************************/

#ifndef _FP_SDK_CONFIG_H_
#define _FP_SDK_CONFIG_H_


/************************************************************************
 * Core details
 ************************************************************************/

/* Core generation - the earliest core generation which we support. */
#define CORE_GENERATION 0


/************************************************************************
 * Core characteristics derived from architecture versions
 ************************************************************************/

/* The core supports load/store instructions with long immediate operands. */
#if 1
#  define CORE_HAS_IMM32_LDST 1
#endif

/* The core has 8 MREGs as opposed to the earlier 4 */
#if 1
#  define CORE_HAS_8_MREGS 1
#endif

/* DIR instructions must be written on the Y side only */
#if 1
#  define CORE_DIRS_ON_Y_ONLY 1
#endif

/* Core uses modeless encodings instead of FNSC */
#if 1
#  define CORE_USES_MODELESS_ENCODING 1
#endif

/* The exception and interrupt vectors can be relocated at runtime */
#if 1
#  define CORE_HAS_RELOCATABLE_VECTORS 1
#endif

/* Core has an undefined instruction exception */
#if 1
#  define CORE_HAS_UNDEF_TRAP 1
#endif

/* Core has DIR instructions with register operand for directive number */
#if 1
#  define CORE_HAS_DIR_REG_REG 1
#endif

/* Core has banked general purpose registers */
#if 0
#  define CORE_HAS_BANKED_REGS 1
#endif

/* Core memory protection support */
#if 0
#  define CORE_HAS_MEM_PROT_FP2006 1
#endif
#if 0
#  define CORE_HAS_MEM_PROT_FP2011 1
#endif
#if 1
#  define CORE_HAS_MEM_PROT_FP4015 1
#endif

#if 1
#  define CORE_HAS_DREG_FEATURE_MPUD_STATUS 1
#endif
#if 0
#  define CORE_HAS_DREG_FEATURE_MPU_DTCM 1
#endif
#if 1
#  define CORE_HAS_DREG_FEATURE_MPU_ICA  1
#endif
#if 1
#  define CORE_HAS_DREG_FEATURE_MPU_DCA  1
#endif
#if 1
#  define CORE_HAS_DREG_FEATURE_MPU_IO   1
#endif

#define CORE_MPU_NUM_PROTI_REGIONS 4
#define CORE_MPU_NUM_PROTD_REGIONS 12

/* Instructions / architectural state availability */
#if 1
#  define CORE_HAS_MAC_INSTRUCTIONS 1
#endif
#if 1
#  define CORE_HAS_MAC_STATE 1
#endif
#if 1
#  define CORE_HAS_FIR_INSTRUCTIONS 1
#endif
#if 1
#  define CORE_HAS_FIR_STATE 1
#endif
#if 1
#  define CORE_HAS_REED_SOLOMON_INSTRUCTIONS 1
#endif
#if 0
#  define CORE_HAS_REED_SOLOMON_STATE 1
#endif
#if 0
#  define CORE_HAS_TRELLIS_INSTRUCTIONS 1
#endif
#if 0
#  define CORE_HAS_TRELLIS_STATE 1
#endif
#if 0
#  define CORE_HAS_SIMD_BITFIELD_INSTRUCTIONS 1
#endif
#if 0
#  define CORE_HAS_SIMD_BITFIELD_STATE 1
#endif

/* Core fetched addresses FNSC bitmask */
#define PC_FETCHING_COMPRESSED_MASK 0


/************************************************************************
 * SoC features and parameters
 ************************************************************************/

/* Number of cores */
#define NUM_CORES 2

/* Number of symmetrical subsystems */
#define NUM_SUBSYSTEMS 1

/* Are we a multicore machine with a shared view of the memory, which
 * presupposes that each core private data must live at different virtual
 * addresses? */
#if 1
#  define SHARED_MEMORY_MULTICORE 1
#endif

/* The core has a Bit Read Buffer */
#if 1
#  define CORE_HAS_BRB 1
#endif

/* The core has DREGs for driving I$ features */
#if 1
#  define CORE_HAS_DREG_FEATURE_ICA 1
#endif

/* Configurable reset vector */
#if 0
#  define CORE_HAS_CONFIGURABLE_RESET_VECTOR 1
#endif

/* Edge or level IRQE sources */
#define CHIP_IRQE_LEVEL_SOURCES_MASK     0x0000000000000000
#define CHIP_IRQE_LEVEL_SOURCES_MASK_TOP 0x00000000
#define CHIP_IRQE_LEVEL_SOURCES_MASK_BOT 0x00000000

/* Enable memory parity checking support  */
#if 0
#  define CHIP_HAS_PARITY 1
#endif

/* An L2 cache is attached to the core */
#if 1
#  define CHIP_HAS_L2_CACHE 1
#endif

/* An address translation unit is attached to the core */
#if 1
#  define CHIP_HAS_ATU 1
#endif

/* Host Port available */
#if 0
#  define CHIP_HAS_HOST_PORT 1
#endif

/* Whether the chip has a programmable Phase Locked Loop clock source
 * from which the core clock is derived */
#if 0
#  define CHIP_HAS_PLL 1
#endif

/* Soft breakpoint support */
#if 0
#  define CHIP_HAS_SOFTBPT 1
#endif

/* VOM support */
#if 0
#  define CHIP_HAS_VOM 1

#  define VOM_PAGE_SHIFT    0
#  define VOM_PAGE_SIZE     (1 << VOM_PAGE_SHIFT)
#  define VOM_PAGE_MASK     (VOM_PAGE_SIZE - 1)

#  if 0
#    define VOM_ADDRESS_BIT 0
#  endif

#  if 0
#    define VOM_FLAVOUR_FP2000  1
#    define VOM_VMEM_SIZE       0
#    define VOM_VMEM_PAGES      (VOM_VMEM_SIZE / VOM_PAGE_SIZE)
#  else
#    define VOM_FLAVOUR_FP2000  0
#    define VOM_PMEM_SIZE       0
#    define VOM_PMEM_PAGES      (VOM_PMEM_SIZE / VOM_PAGE_SIZE)
#  endif
#endif

/* The chip's cores have an icache that supports prefetching */
#if 1
#  define ICACHE_HAS_PREFETCH 1
#endif

/* The chip has some form of data cache */
#if 1
#  define CHIP_HAS_DATA_CACHE 1
#endif

/* Chip class */
#define CHIP_CLASS_UNKNOWN 1


/************************************************************************
 * SDK and SDK build tools versions
 ************************************************************************/

/* SDK version - raw and decoded */
#define SDK_VERSION         0xff014700
#define SDK_VERSION_DECODED "release 1.47"

/* Tools used for buildind the SDK */
#define FP_GCC_VERSION      "2.7.9.1"
#define FP_BINUTILS_VERSION "7.3"
#define RALL_VERSION        "2.5.4"


/************************************************************************
 * SDK features selection - interrupt handling
 ************************************************************************/

/* Are FATAL/DEBUG exceptions fully handled or should only stubs be provided? */
#if 1
#  define EXCEPTIONS_HANDLING 1
#endif

/*Is there an IPH handler installed - Disabled by default - EXCEPTIONS HANDLING is
 a pre-requsite */
#if defined (EXCEPTIONS_HANDLING)
#  if 0
#    define LOCAL_IPH_SUPPORT 1
#  endif
#endif

/* Interrupt vector version */
#define IRQ_VECTOR_VERSION 3

#if IRQ_VECTOR_VERSION >= 3
/* Define the maximum pririty level */
#  define IRQ_MAX_PRIORITY 7
/* Interrupt nesting enabled */
#  if 1
#    define INTERRUPT_NESTING_ENABLED 1
#  endif

/* Use the same stack for IRQ and main threads? */
#  if defined(IRQ_USES_APP_STACK)
#    if IRQ_USES_APP_STACK == 0
#      undef IRQ_USES_APP_STACK
#    endif
#  else
#    if 0
#      define IRQ_USES_APP_STACK 1
#    endif
#  endif
#endif

/* Do we expect interrupts to arrive during the execution of untrusted code? */
#if 1
#  define IRQ_INTERRUPTS_UNTRUSTED_CODE 1
#endif

/************************************************************************
 * SDK features selection - init-time behaviour
 ************************************************************************/

/* Multi-stage booting is supported, that is the software can decide to
 * "soft-reset" the core by jumping to the reset vector or to the entry address
 * of a different firmware image. */
#if 0
#  define MULTI_STAGE_BOOT 1
#endif

/* The reset vector will zero memories content between boundaries
 * defined by linker-script provided symbol values. */
#if 0
#  define RUNTIME_ZERO_MEMORIES 1
#endif
#define RUNTIME_ZERO_MEMORIES_NUM_AREAS 0

/* At runtime init, read the entries from the __loadtable_zero and
 * __loadtable_copy tables and appropriately zero/copy memory regions. */
#if 1
#  define RUNTIME_INIT_ZERO_COPY_REGIONS 1
#endif

/* At runtime init, fetch the values of argv, argc and envp. */
#if 1
#  define RUNTIME_INIT_FETCH_MAIN_ARGS 1
#endif

/* Is the __init_mpu_state hook (called from __init_core_state) mechanism
 * available? */
#if 0
#  define MEMORY_PROTECTION_INIT_HOOK 1
#endif

/* Support for the reset flags mechanism. */
#if 1
#  define RESET_FLAGS 1
#endif

/* Are reset flags read-only? */
#if 1
#  define RESET_FLAGS_RO 1
#endif

/************************************************************************
 * SDK features selection - compliance to C/C++ standards.
 ************************************************************************/

/* Support for atexit / on_exit functions. */
#if 1
#  define STANDARD_COMPLIANT_EXITPROCS 1
#endif

/* Support for calling _GLOBAL_REENT->__cleanup on exit. */
#if 1
#  define STANDARD_COMPLIANT_EXIT_LIBC_CLEANUP 1
#endif

/* Support for C/C++ static constructors and destructors. */
#if 1
#  define STANDARD_COMPLIANT_CTORS_DTORS 1
#endif


/************************************************************************
 * SDK features selection - memory protection behaviour.
 ************************************************************************/

/* On targets with an MPU, the first data/code region not used by the SDK */
#define MEMORY_PROTECTION_FIRST_USER_PROTI 1
#define MEMORY_PROTECTION_FIRST_USER_PROTD 8

/* Do we provide the "classic" protection style, where sensitive SDK and
 * read-only data/bss are placed in a protected region, while the rest
 * lives in an unprotected region? */
#if 0
#  define CLASSIC_MEMORY_PROTECTION 1
#endif
#define CLASSIC_MEMORY_PROTECTION_PROTD_REGION -16777216

/************************************************************************
 * SDK features selection - memory access restrictions
 ************************************************************************/

#if 0
#  define SDK_NO_MISALIGNED_DATA 1
#endif

/************************************************************************
 * SDK features selection - instrumentation configuration
 ************************************************************************/

#if 0
#  define INSTR_ENABLE 1

/* Use shared event buffer */
#  if 1
#    define INSTR_SHARED_EVENT_BUFFER 1
#  endif

/* Core ID logging */
#  if 1
#    define INSTR_LOG_CORE_ID 1
#    define INSTR_N_CORE_ID_BITS 1
#  endif


/* Enables the support for recording user events */
#  if 1
#    define INSTR_LOG_USER_EVENTS 1
#  endif
/* Enables the support for recording thread switch event handling-related events */
#  if 1
#    define INSTR_LOG_THREADX_EXEC_EVENTS 1
#  endif
/* Enables the support for recording cpu load event handling-related events */
#  if 1
#    define INSTR_LOG_THREADX_CPULOAD_EVENTS 1
#  endif
/* Enables the support for recording thread/scheduler related events events */
#  if 1
#    define INSTR_LOG_THREADX_THREAD_EVENTS 1
#  endif
/* Enables the support for recording bytepool events */
#  if 1
#    define INSTR_LOG_THREADX_BYTEPOOL_EVENTS 1
#  endif
/* Enables the support for recording blockpool events */
#  if 1
#    define INSTR_LOG_THREADX_BLOCKPOOL_EVENTS 1
#  endif
/* Enables the support for recording message queue events */
#  if 1
#    define INSTR_LOG_THREADX_MQUEUE_EVENTS 1
#  endif
/* Enables the support for recording eventflag events */
#  if 1
#    define INSTR_LOG_THREADX_EVENTFLAG_EVENTS 1
#  endif
/* Enables the support for recording semaphore events */
#  if 1
#    define INSTR_LOG_THREADX_SEMAPHORE_EVENTS 1
#  endif
/* Enables the support for recording mutex events */
#  if 1
#    define INSTR_LOG_THREADX_MUTEX_EVENTS 1
#  endif
/* Enables the support for recording timer events */
#  if 1
#    define INSTR_LOG_THREADX_TIMER_EVENTS 1
#  endif
/* Enables the support for recording interrupt events*/
#  if 1
#    define INSTR_LOG_INTERRUPT_EVENTS 1
#  endif
/* Enables the support for recording overlay events*/
#  if 1
#    define INSTR_LOG_OVERLAY_EVENTS 1
#  endif


/* Enables the support for handling thread start and events */
#  if 1
#    define INSTR_HANDLE_THREADX_THREAD_EVENTS 1
#  endif
/* Enables the support for handling cpuload computations */
#  if 1
#    define INSTR_HANDLE_THREADX_CPULOAD_EVENTS 1
#  endif
/* Enables the support for handling interrupt events */
#  if 1
#    define INSTR_HANDLE_INTERRUPT_EVENTS 1
#  endif

#  if defined (INSTR_LOG_USER_EVENTS)               || \
      defined (INSTR_LOG_THREADX_EXEC_EVENTS)       || \
      defined (INSTR_LOG_THREADX_CPULOAD_EVENTS)    || \
      defined (INSTR_LOG_THREADX_THREAD_EVENTS)     || \
      defined (INSTR_LOG_THREADX_BYTEPOOL_EVENTS)   || \
      defined (INSTR_LOG_THREADX_BLOCKPOOL_EVENTS)  || \
      defined (INSTR_LOG_THREADX_MQUEUE_EVENTS)     || \
      defined (INSTR_LOG_THREADX_EVENTFLAG_EVENTS)  || \
      defined (INSTR_LOG_THREADX_SEMAPHORE_EVENTS)  || \
      defined (INSTR_LOG_THREADX_MUTEX_EVENTS)      || \
      defined (INSTR_LOG_THREADX_TIMER_EVENTS)      || \
      defined (INSTR_LOG_INTERRUPT_EVENTS)
/* This define will be enabled if any of the record events are enabled */
#    define INSTR_ENABLE_EVENT_LOGGING 1
#  endif

#  if defined (INSTR_HANDLE_THREADX_THREAD_EVENTS)  || \
      defined (INSTR_HANDLE_THREADX_CPULOAD_EVENTS) || \
      defined (INSTR_HANDLE_INTERRUPT_EVENTS)
    /* This define will be enabled if any of the handle events are enabled */
#    define INSTR_ENABLE_EVENT_HANDLING 1
#  endif

#endif


/************************************************************************
 * SDK features selection - miscellaneous
 ************************************************************************/

/* Debug Agent version */
#define DBA_VERSION 2

/* Overlays support */
#if 0
#  define ENABLE_OVERLAYS
#  define OVERLAYS_CODE_NUM_REGIONS              0
#  define OVERLAYS_CODE_MAX_OVERLAYS_PER_REGION  0
#  define OVERLAYS_DATA_NUM_REGIONS              0
#  define OVERLAYS_DATA_MAX_OVERLAYS_PER_REGION  0
#endif

/* Enable statistical profiling. FIXME: what does this really do? */
#if 1
#  define ENABLE_PROFILING 1
#endif

/* Target Prints support */
#if 1
#  define TARGET_PRINT_SUPPORT 1
#endif

/* Should target prints be constructed inline. */
#if defined(TARGET_PRINT_INLINE)
#  if TARGET_PRINT_INLINE == 0
#    undef TARGET_PRINT_INLINE
#  endif
#else
#  if 0
#    define TARGET_PRINT_INLINE 1
#  endif
#endif

/* Target Buffer options */
#define TARGET_BUFFER_VERSION 2
#if TARGET_BUFFER_VERSION >= 2 && 1
#  define TARGET_BUFFER_MUX_SERVICES 1
#endif

/* A NULL pointer vector is provided */
#if 1
#  define NULL_POINTER_VECTOR 1
#endif

/* Heartbeat support */
#if 1
#  define ENABLE_HEARTBEAT 1
#endif

/* Stitched executables support */
#if 0
#  define ENABLE_STITCHING 1
#endif

/* Power management support */
#if 1
#  define POWER_MANAGEMENT_SUPPORT 1
#endif

/* Mutex migration support */
#if 0
#  define MUTEX_MIGRATION_SUPPORT 1
#endif

/* Preemption thresholding for ThreadX/FPOS */
#if 0
#  define TX_PREEMPTION_THRESHOLD 1
#endif

/************************************************************************
 * FPOS feature selection
 ************************************************************************/

/* FPOS - maximum size of the ROM filesystem image. */
#define TX_ROMFS_MAX_SIZE 67108864

/* FPOS support for immediate user interrupts */
#if defined(FPOS_UIRQ_SUPPORT)
#  if FPOS_UIRQ_SUPPORT == 0
#    undef FPOS_UIRQ_SUPPORT
#  endif
#else
#  if 1
#    define FPOS_UIRQ_SUPPORT 1
#  endif
#endif

/* FPOS support for writing to a target buffer (e.g. target prints, coredumps) */
#if defined(FPOS_TB_SUPPORT)
#  if FPOS_TB_SUPPORT == 0
#    undef FPOS_TB_SUPPORT
#  endif
#else
#  if 1
#    define FPOS_TB_SUPPORT 1
#  endif
#endif

/* FPOS support for core dumps */
#if defined(FPOS_COREDUMP_SUPPORT)
#  if FPOS_COREDUMP_SUPPORT == 0
#    undef FPOS_COREDUMP_SUPPORT
#  endif
#else
#  if 1
#    define FPOS_COREDUMP_SUPPORT 1
#  endif
#endif

/* FPOS support for overlays */
#if defined(FPOS_OVERLAY_SUPPORT)
#  if FPOS_OVERLAY_SUPPORT == 0
#    undef FPOS_OVERLAY_SUPPORT
#  endif
#else
#  if 0
#    define FPOS_OVERLAY_SUPPORT 1
#  endif
#endif

/* FPOS support for profiling */
#if defined(FPOS_PROFILING_SUPPORT)
#  if FPOS_PROFILING_SUPPORT == 0
#    undef FPOS_PROFILING_SUPPORT
#  endif
#else
#  if 0
#    define FPOS_PROFILING_SUPPORT 1
#  endif
#endif


/************************************************************************
 * SDK features selection - context switching options
 ************************************************************************/

#if 1
#  define FIR_CONTEXT 1
#endif

#if 1
#  define MAC_CONTEXT 1
#endif

#if 0
#  define REED_SOLOMON_CONTEXT 1
#endif

#if 0
#  define SIMD_BITFIELD_CONTEXT 1
#endif

#if 0
#  define TRELLIS_CONTEXT 1
#endif


/************************************************************************
 * Define derived architecture version macros.
 *
 * Below macros are based on the assumption that two lines of cores
 * exist, the Firepath DSP one:
 *   fp2006 -> fp2008 -> fp2011 -> fp2012 -> fp4014 -> fp4015 -> fp4017 -> ...
 * and the Firepath Maestro one:
 *   fpm1015 -> fpm1017 -> ...
 ************************************************************************/

#if __FP2006__ || __FP2008__ || \
    __FP2011__ || __FP2012__ || \
    __FP4014__ || __FP4015__ || \
    __FP4017__
#  define __FP2006_ONWARDS__    1
#endif

#if __FP2008__ || __FP2011__ || \
    __FP2012__ || __FP4014__ || \
    __FP4015__ || __FP4017__
#  define __FP2008_ONWARDS__    1
#endif

#if __FP2011__ || __FP2012__ || \
    __FP4014__ || __FP4015__ || \
    __FP4017__
#  define __FP2011_ONWARDS__    1
#endif

#if __FP2012__ || __FP4014__ || \
    __FP4015__ || __FP4017__
#  define __FP2012_ONWARDS__    1
#endif

#if __FP4014__ || __FP4015__ || \
    __FP4017__
#  define __FP4014_ONWARDS__    1
#endif

#if __FP4015__ || __FP4017__
#  define __FP4015_ONWARDS__    1
#endif

#if __FP4017__
#  define __FP4017_ONWARDS__    1
#endif

#if __FPM1015__ || __FPM1017__
#  define __FPM1015_ONWARDS__   1
#endif

#if __FPM1017__
#  define __FPM1017_ONWARDS__   1
#endif

/************************************************************************
 * If using the ROM shared library scheme, include any necessary header
 * files for the ROM build here.
************************************************************************/
#if 0
#  define SHLIB_ROM_SUPPORT 1
#  include "fp_sdk_rom_includes.h"
#endif

#endif /* _FP_SDK_CONFIG_H_ */
