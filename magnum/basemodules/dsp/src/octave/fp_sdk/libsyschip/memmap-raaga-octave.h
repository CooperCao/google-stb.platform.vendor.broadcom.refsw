/******************************************************************************
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
 ******************************************************************************/

#ifndef _MEMMAP_RAAGA_OCTAVE_H_
#define _MEMMAP_RAAGA_OCTAVE_H_

#if ! (defined (__FP4015__) || defined (__COMPILE_HEADER__))
#  error "Included memmap-raaga-octave.h on unsupported machine architecture"
#endif

#include "fp_sdk_config.h"

#include "libsyschip/memmap-octave-v2-maestro-v1.h"

#include "memmap-utils.h"



#ifdef __FIREPATH__
#  if !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
extern __absolute Misc_Block taddr_Misc_Block;
#  endif

/*
 * The address of core 0's Misc block, given to the linker script to provide
 * taddr_Misc_Block.
 */
#define DEFAULT_MISC_BLOCK_ADDRESS  0x0fe34000

/*
 * The base of FP0's CCS area, cast to a volatile uint8_t* (so pointer
 * arithmetic is just the same as it would be on the address)
 */
#define FPMISC_CORE_BASE_0   ((volatile uint8_t *) & taddr_Misc_Block)

/*
 * Spacing between sets of register for one core and the same ones for the next
 * core
 */
#define FPMISC_CORE_SPACING  0x200

/*
 * The misc block structure for the N'th core.
 */
#define FPMISC_BLOCK_FOR_CORE(n)                        \
    ((Misc_Block*)(FPMISC_CORE_BASE_0 + (n) * FPMISC_CORE_SPACING))

/*
 * L1 cache parameters.
 */
#define L1_DCACHE_SIZE                  16384

/*
 * L2 cache parameters.
 */
#define L2_CACHE_LINE_SIZE_LOG2         9   /* 512 bytes/line */

/* Command queues allocation. Queues will be configured as follows:
 *  - queue 0: core 0, low prio
 *  - queue 1: core 0, medium prio
 *  - queue 2: core 0, high prio
 *  - queue 3: core 0, highest prio
 *  - queue 4: core 1, low prio
 *  - queue 5: core 1, medium prio
 *  - queue 6: core 1, high prio
 *  - queue 7: core 1, highest prio
 */
#define L2_CACHE_CMD_QUEUE_CORE_BASE_FOR_CORE(core) ((core) << 2)
#define L2_CACHE_CMD_QUEUE_CORE_BASE        L2_CACHE_CMD_QUEUE_CORE_BASE_FOR_CORE(FIREPATH_NUM)
#define L2_CACHE_CMD_QUEUE_LOW_OFFSET       0
#define L2_CACHE_CMD_QUEUE_MEDIUM_OFFSET    1
#define L2_CACHE_CMD_QUEUE_HIGH_OFFSET      2
#define L2_CACHE_CMD_QUEUE_HIGHEST_OFFSET   3
#define L2_CACHE_CMD_QUEUE_LOW              (L2_CACHE_CMD_QUEUE_CORE_BASE + L2_CACHE_CMD_QUEUE_LOW_OFFSET)
#define L2_CACHE_CMD_QUEUE_MEDIUM           (L2_CACHE_CMD_QUEUE_CORE_BASE + L2_CACHE_CMD_QUEUE_MEDIUM_OFFSET)
#define L2_CACHE_CMD_QUEUE_HIGH             (L2_CACHE_CMD_QUEUE_CORE_BASE + L2_CACHE_CMD_QUEUE_HIGH_OFFSET)
#define L2_CACHE_CMD_QUEUE_HIGHEST          (L2_CACHE_CMD_QUEUE_CORE_BASE + L2_CACHE_CMD_QUEUE_HIGHEST_OFFSET)

/* Default choices for queue and command ids, used by SDK-provided cache flush
 * commands. */
#define L2_CACHE_CMD_ID_SDK                 FIREPATH_NUM
#define L2_CACHE_CMD_QUEUE_SDK              L2_CACHE_CMD_QUEUE_HIGH

#if !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
/*
 * L2 cache initialisation routine.
 */
__init
extern void l2_init(void);
#endif

/* System element sizing values */
#define NUM_MUTEXES_PER_CORE    12


#if !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
/* Location and size of memory regions as set at link time.
 * Please use below defined macros in C code. */
extern char taddr_dram_ro, tsize_dram_ro;
extern char taddr_dram_rw, tsize_dram_rw;

/* The whole firmware image */
__absolute
extern char __begin_ro_image, __end_ro_image, __ro_image_size;
__absolute
extern char __begin_rw_image, __end_rw_image, __rw_image_size;

/* FPOS specific symbols. */
__absolute __weak
extern char __romfs_start, __romfs_length;
#endif


#if !defined(__LINKER_SCRIPT__)

/*
 * DRAM
 *
 * The whole 4 GiB space is split in three parts:
 * - Read-only DRAM, from 0 up to 64 MiB
 * - Low read-write DRAM, from 64 MiB up to before the simulated SMEM address
 * - High read-write DRAM, starting together with the I/O region  and
 *   continuing up to 4 GiB
 * See hwmap-raaga-octave.ldp for more details.
 */
#define DRAM_RO_START       SYMBOL_ADDR_TO_ADDR(taddr_dram_ro)
#define DRAM_RO_SIZE        SYMBOL_ADDR_TO_SIZE(tsize_dram_ro)
#define DRAM_RO_END         (DRAM_RO_START + DRAM_RO_SIZE)

#define DRAM_RW_START       SYMBOL_ADDR_TO_ADDR(taddr_dram_rw)
#define DRAM_RW_SIZE        SYMBOL_ADDR_TO_SIZE(tsize_dram_rw)
#define DRAM_RW_END         (DRAM_RW_START + DRAM_RW_SIZE)

/*
 * The whole firmware image, read-only (excluding any ROMFS area) and
 * read-write portions.
 */
#define IMAGE_RO_START         SYMBOL_ADDR_TO_ADDR(__begin_ro_image)
#define IMAGE_RO_END           SYMBOL_ADDR_TO_ADDR(__end_ro_image)
#define IMAGE_RO_SIZE          SYMBOL_ADDR_TO_ADDR(__ro_image_size)
#define IMAGE_RW_START         SYMBOL_ADDR_TO_ADDR(__begin_rw_image)
#define IMAGE_RW_END           SYMBOL_ADDR_TO_ADDR(__end_rw_image)
#define IMAGE_RW_SIZE          SYMBOL_ADDR_TO_ADDR(__rw_image_size)

/*
 * FPOS specific symbols.
 */
#define ROMFS_START         SYMBOL_ADDR_TO_ADDR(__romfs_start)
#define ROMFS_LENGTH        SYMBOL_ADDR_TO_ADDR(__romfs_length)

#endif /* __LINKER_SCRIPT__ */

/*
 * MUTEXES - Mutex registers
 */
#if !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
/* Hardware mutexes are accessed over a 32-bit bus. */
typedef volatile uint32_t hw_mutex_t;

#if !POWER_MANAGEMENT_SUPPORT
/* core_mutex.h integration.
 *
 * NUM_MUTEXES is the number available in the pool for core_mutex. The debug
 * agent uses 1 per core for its own purposes.
 */
#define NUM_MUTEXES ((NUM_MUTEXES_PER_CORE - 1) * NUM_CORES)

static inline hw_mutex_t *
get_mutex_addr (int idx)
{
    /* Grab mutexes from cores in a round robin fashion */
    int core = idx % NUM_CORES;
    volatile uint32_t *mutexes = &FPMISC_BLOCK_FOR_CORE(core)->corestate_sys_mtx0;
    return & mutexes [idx / NUM_CORES];
}
#else /* POWER_MANAGEMENT_SUPPORT */
/* Under power management, only provide mutexes from core 0. We don't support
 * mutex migration (yet), and other cores may be powered down.
 */
#define NUM_MUTEXES (NUM_MUTEXES_PER_CORE - 1)

static inline hw_mutex_t *
get_mutex_addr (int idx)
{
    volatile uint32_t *mutexes = &FPMISC_BLOCK_FOR_CORE(0)->corestate_sys_mtx0;
    return & mutexes [idx];
}
#endif /* !POWER_MANAGEMENT_SUPPORT */

#endif /* !ASMCPP && !__LINKER_SCRIPT__ */

/*
 * SDK services configuration
 */
/* Heartbeat word address. */
#ifdef ENABLE_HEARTBEAT
#  if !defined(ASMCPP) && !defined(__RALL2__) && !defined(__LINKER_SCRIPT__)
#    define HEARTBEAT_SHARED_ADDRESS_BASE   (&FPMISC_BLOCK_FOR_CORE(0)->corestate_sys_mbx4)
#  else
#    define HEARTBEAT_SHARED_ADDRESS_BASE   (taddr_Misc_Block + MISC_BLOCK_CORESTATE_SYS_MBX4)
#  endif
#  define HEARTBEAT_SHARED_ADDRESS_DELTA    FPMISC_CORE_SPACING
#endif

#if !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
/*
 * Generic interrupt controller block used in various places in Raaga.
 */
typedef struct
{
    volatile const uint32_t int_status;        /* 0x0            acc=RO       Interrupt Status Register */
    volatile       uint32_t int_set;           /* 0x4            acc=WO       Interrupt Set Register */
    volatile       uint32_t int_clear;         /* 0x8            acc=WO       Interrupt Clear Register */
    volatile const uint32_t mask_status;       /* 0xc            acc=RO       Interrupt Mask Status Register */
    volatile       uint32_t mask_set;          /* 0x10           acc=WO       Interrupt Mask Set Register */
    volatile       uint32_t mask_clear;        /* 0x14           acc=WO       Interrupt Mask Clear Register */
} Raaga_Interrupt_Control_Block;

#ifdef __FIREPATH__
__absolute __align(4) extern Raaga_Interrupt_Control_Block taddr_Mem_Done_Status_Int_RDB;
__absolute __align(4) extern Raaga_Interrupt_Control_Block taddr_Queue_Status_Int_RDB;
__absolute __align(4) extern Raaga_Interrupt_Control_Block taddr_Error_Int_RDB;
#endif
#endif  /* !defined(ASMCPP) && !defined(__LINKER_SCRIPT__) */

/**
 * RAAGA_DSP_MEM_DONE_STATUS_INT interrupt controller bits
 * @{
 */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_DMA_Q0_DONE   (1 << 0)  /**< DMA Q0 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_DMA_Q1_DONE   (1 << 1)  /**< DMA Q1 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_DMA_Q2_DONE   (1 << 2)  /**< DMA Q2 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_DMA_Q3_DONE   (1 << 3)  /**< DMA Q3 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_DMA_Q4_DONE   (1 << 4)  /**< DMA Q4 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_DMA_Q5_DONE   (1 << 5)  /**< DMA Q5 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_PCQ_Q0_DONE   (1 << 6)  /**< PCQ Q0 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_PCQ_Q1_DONE   (1 << 7)  /**< PCQ Q1 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_PCQ_Q2_DONE   (1 << 8)  /**< PCQ Q2 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_PCQ_Q3_DONE   (1 << 9)  /**< PCQ Q3 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_PCQ_Q4_DONE   (1 << 10) /**< PCQ Q4 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_PCQ_Q5_DONE   (1 << 11) /**< PCQ Q5 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_PCQ_Q6_DONE   (1 << 12) /**< PCQ Q6 done interrupt */
#define RAAGA_DSP_MEM_DONE_STATUS_INT_PCQ_Q7_DONE   (1 << 13) /**< PCQ Q7 done interrupt */
/** @} */

/**
 * RAAGA_DSP_QUEUE_STATUS_INT interrupt controller bits
 * @{
 */
#define RAAGA_DSP_QUEUE_STATUS_INT_L2C_QUEUE_STATUS (1 << 0)    /**< Queue status interrupt from L2C module */
#define RAAGA_DSP_QUEUE_STATUS_INT_PRQ_WATER_MARK   (1 << 1)    /**< PRQ Watermark reached interrupt from DMA module */
/** @} */

/**
 * RAAGA_DSP_ERROR_INT interrupt controller bits
 * @{
 */
#define RAAGA_DSP_ERROR_INT_RAAGA_AX_ERROR      (1 << 0)    /**< Error in Raaga_Ax module */
#define RAAGA_DSP_ERROR_INT_L2C_ERROR           (1 << 1)    /**< Error in L2C module */
#define RAAGA_DSP_ERROR_INT_GISB_ERROR          (1 << 2)    /**< GISB bridge error */
#define RAAGA_DSP_ERROR_INT_DMA_SUB_ERROR       (1 << 3)    /**< Error in DMA module */
#define RAAGA_DSP_ERROR_INT_UART_0_ERROR        (1 << 4)    /**< UART 0 error */
#define RAAGA_DSP_ERROR_INT_UART_1_ERROR        (1 << 5)    /**< UART 1 error */
/** @} */

#endif /* __FIREPATH__ */
#endif /* _MEMMAP_RAAGA_OCTAVE_H_ */
