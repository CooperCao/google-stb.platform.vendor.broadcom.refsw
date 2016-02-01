/************************************************************************
 *             Copyright (c) 2013 Broadcom Corporation                  *
 *                                                                      *
 *  This material is the confidential trade secret and proprietary      *
 *  information of Broadcom Corporation. It may not be reproduced,      *
 *  used, sold or transferred to any third party without the prior      *
 *  written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                      *
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
 * Miscelaneous constants
 ************************************************************************/

/* Number of cores on target */
#define NUM_CORES 1

/*
  The bit that governs whether an address is treated as virtual when
  fetching an instruction
*/
#define VOM_ADDRESS_BIT 24

/************************************************************************
 * Interrupt configuration
 ************************************************************************/

/* Interrupt vector version */
#define IRQ_VECTOR_VERSION 2

#if IRQ_VECTOR_VERSION >= 3
/* Define the maximum pririty level */
#  define IRQ_MAX_PRIORITY 7
/* Interrupt nesting enabled */
#  if 1
#    define INTERRUPT_NESTING_ENABLED 1
#  endif
#endif

/************************************************************************
 * Instrumentation configuration
 ************************************************************************/

#if 0
#  define INSTR_ENABLE 1

/* Use shared event buffer */
#  if 0
#    define INSTR_SHARED_EVENT_BUFFER 1
#  endif

/* Core ID logging */
#  if 0
#    define INSTR_LOG_CORE_ID 1
#    define INSTR_N_CORE_ID_BITS 0
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
#  if 0
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
#  if 0
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
#    define INSTR_ENABLE_EVENT_LOGGING
#  endif

#  if defined (INSTR_HANDLE_THREADX_THREAD_EVENTS)  || \
      defined (INSTR_HANDLE_THREADX_CPULOAD_EVENTS) || \
      defined (INSTR_HANDLE_INTERRUPT_EVENTS)
    /* This define will be enabled if any of the handle events are enabled */
#    define INSTR_ENABLE_EVENT_HANDLING
#  endif


#endif

/************************************************************************
 * FP2011+ memory protection config
 ************************************************************************/

/* Disable fp2011+ memory protection */
#if ! 0
#  define MEMORY_PROTECTION_NOT_CONFIGURED 1
#endif

/************************************************************************
 * Parity checking config
 ************************************************************************/

/* Enable memory parity checking support  */
#if 0
#  define HAVE_PARITY 1
#endif

/************************************************************************
 * ICache manipulaition
 ************************************************************************/

#if 0
#  define CHIP_HAS_ICACHE 1
#endif

/************************************************************************
 * Data cache manipulaition
 ************************************************************************/

#if 0
#  define CHIP_HAS_DATA_CACHE 1
#endif

/************************************************************************
 * Bit Read Buffer available
 ************************************************************************/

/* Enable mBit Read Buffer support  */
#if 0
#  define HAVE_BRB 1
#endif

/************************************************************************
 * Soft breakpoint support available
 ************************************************************************/

/* Enable soft breakpoint support  */
#if 0
#  define CHIP_HAS_SOFTBPT 1
#endif

/************************************************************************
 * Characteristics derived from architecture versions
 ************************************************************************/

/* The core supports load/store instructions with long immediate operands. */
#if 0
#  define CORE_HAS_IMM32_LDST 1
#endif

/* The core has 8 MREGs as opposed to the earlier 4 */
#if 0
#  define CORE_HAS_8_MREGS 1
#endif

/* DIR instructions must be written on the Y side only */
#if 0
#  define CORE_DIRS_ON_Y_ONLY 1
#endif

/* Core uses modeless encodings instead of FNSC */
#if 0
#  define CORE_USES_MODELESS_ENCODING 1
#endif

/* The exception and interrupt vectors can be relocated at runtime */
#if 0
#  define CORE_HAS_RELOCATABLE_VECTORS 1
#endif

/* Core has an undefined instruction exception */
#if 0
#  define CORE_HAS_UNDEF_TRAP 1
#endif

/* Core has DIR instructions with register operand for directive number */
#if 0
#  define CORE_HAS_DIR_REG_REG 1
#endif

/************************************************************************
 * Define behaviour at init time.
 ************************************************************************/

/* At runtime init, read the entries from the __loadtable_zero and
   __loadtable_copy tables and appropriately zero/copy memory regions. */
#if 1
#  define RUNTIME_INIT_ZERO_COPY_REGIONS 1
#endif

/* At runtime init, fetch the values of argv, argc and envp. */
#if 1
#  define RUNTIME_INIT_FETCH_MAIN_ARGS 1
#endif

/************************************************************************
 * Define derived architecture version macros.
 ************************************************************************/

#if defined(__FP2008__) || defined(__FP2011__) || \
    defined(__FP2012__) || defined(__FP4014__)
#  define __FP2008_ONWARDS__
#endif

#if defined(__FP2011__) || defined(__FP2012__) || defined(__FP4014__)
#  define __FP2011_ONWARDS__
#endif

#if defined(__FP2012__) || defined(__FP4014__)
#  define __FP2012_ONWARDS__
#endif

#if defined(__FP4014__)
#  define __FP4014_ONWARDS__
#endif

/************************************************************************
 * Overlays support
 ************************************************************************/

#if 0
#  define ENABLE_OVERLAYS
#  define OVERLAYS_CODE_NUM_REGIONS              0
#  define OVERLAYS_CODE_MAX_OVERLAYS_PER_REGION  0
#  define OVERLAYS_DATA_NUM_REGIONS              0
#  define OVERLAYS_DATA_MAX_OVERLAYS_PER_REGION  0
#endif


/************************************************************************
 * Statistical profiling options
 ************************************************************************/

/* Enable modified interrupt vectors */
#if 1
#  define ENABLE_PROFILING
#endif

/************************************************************************
 * Target Buffer options
 ************************************************************************/
#define TARGET_BUFFER_VERSION 1
#if TARGET_BUFFER_VERSION >= 2 && 0
#  define TARGET_BUFFER_MUX_SERVICES
#endif


/************************************************************************
 * Heartbeat support
 ************************************************************************/
#if 0
#  define ENABLE_HEARTBEAT
#endif

/************************************************************************
 * Stitched executables support
 ************************************************************************/
#if 0
#  define ENABLE_STITCHING
#endif

/************************************************************************
 * Context save / restore
 ************************************************************************/
#if 1
#  define FIR_STATE_CONTEXT
#endif

#if 1
#  define MAC_CONTEXT
#endif

#if 1
#  define NON_VOLATILE_GREG_CONTEXT
#endif

#if 1
#  define REED_SOLOMON_CONTEXT
#endif

#if 1
#  define SIMD_BITFIELD_CONTEXT
#endif

#if 0
#  define TRELLIS_STATE_CONTEXT
#endif


#endif /* _FP_SDK_CONFIG_H_ */
