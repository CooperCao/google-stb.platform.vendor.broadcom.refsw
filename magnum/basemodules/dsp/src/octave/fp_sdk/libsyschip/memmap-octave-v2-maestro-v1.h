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

/**
 * Common definitions for Octave V2 and Maestro V1 subsystems.
 */

#ifndef _MEMMAP_OCTAVE_V2_MAESTRO_V1_H_
#define _MEMMAP_OCTAVE_V2_MAESTRO_V1_H_

#include "fp_sdk_config.h"
#include "dreg-numbers.h"

#if !(defined(__FP4015_ONWARDS__) || defined(__FPM1015_ONWARDS__)) && !defined(__COMPILE_HEADER__)
#  error "Inconsistent build: including memmap-octave-v2-maestro-v1.h on an unsupported machine"
#endif



/* L1 instruction and data caches parameters. L2 cache(s) details are to be added by
 * SoC-specific memory map headers, if an L2 is available at all. */
#define L1_CACHE_LINE_SIZE_LOG2             5
#define L1_CACHE_LINE_SIZE                  (1 << (L1_CACHE_LINE_SIZE_LOG2))

/* L1/L2 caches related useful macros. */
#define CEIL_TO_L1_CACHE_LINE(addr)         CEIL_TO_MULTIPLE_OF_POW2(addr, L1_CACHE_LINE_SIZE)
#define CEIL_TO_L2_CACHE_LINE(addr)         CEIL_TO_MULTIPLE_OF_POW2(addr, L2_CACHE_LINE_SIZE)
#define FLOOR_TO_L1_CACHE_LINE(addr)        FLOOR_TO_MULTIPLE_OF_POW2(addr, L1_CACHE_LINE_SIZE)
#define FLOOR_TO_L2_CACHE_LINE(addr)        FLOOR_TO_MULTIPLE_OF_POW2(addr, L2_CACHE_LINE_SIZE)



#if defined(__FIREPATH__) && !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
#  include <libfp/c_utils.h>
#  include <stdint.h>

/**
 * Octave V2 and Maestro V1 Misc Block
 * Core Control and Status registers, Flags, and Revision                                     -  0x000 - 0x043
 * Mailbox and Mutex registers                                                                -  0x080 - 0x0ef
 * External software exception request and Host interrupt controller and OBUSFAULT registers  -  0x100 - 0x15b
 * PC trace and Profiling registers                                                           -  0x180 - 0x1cf
 */
typedef struct __packed
{
#ifndef DOXYGEN
    volatile uint32_t corectrl_core_enable;                   /* 0x000 */
    volatile uint32_t corectrl_auto_reset_control;            /* 0x004 */
    volatile uint32_t corectrl_core_idle;                     /* 0x008 */
    volatile uint32_t corectrl_core_reset_cause;              /* 0x00c */
    volatile uint32_t corectrl_memory_power_down_control_0;   /* 0x010 */
    volatile uint32_t corectrl_memory_power_down_status_0;    /* 0x014 */
    volatile uint32_t corectrl_memory_power_down_control_1;   /* 0x018 */
    volatile uint32_t corectrl_memory_power_down_status_1;    /* 0x01c */
    volatile uint32_t corectrl_sys_flg0_status;               /* 0x020 */
    volatile uint32_t corectrl_sys_flg0_set;                  /* 0x024 */
    volatile uint32_t corectrl_sys_flg0_clear;                /* 0x028 */
    uint8_t gap10[0x030 - 0x02c];
    volatile uint32_t corectrl_usr_flg0_status;               /* 0x030 */
    volatile uint32_t corectrl_usr_flg0_set;                  /* 0x034 */
    volatile uint32_t corectrl_usr_flg0_clear;                /* 0x038 */
    uint8_t gap11[0x040 - 0x03c];
    volatile uint32_t corectrl_subsystem_revision;            /* 0x040 */
    volatile uint32_t corectrl_subsystem_reset_vector;        /* 0x044 */
    uint8_t gap1[0x080 - 0x048];

    volatile uint32_t corestate_sys_mbx0;                     /* 0x080 */
    volatile uint32_t corestate_sys_mbx1;                     /* 0x084 */
    volatile uint32_t corestate_sys_mbx2;                     /* 0x088 */
    volatile uint32_t corestate_sys_mbx3;                     /* 0x08c */
    volatile uint32_t corestate_sys_mbx4;                     /* 0x090 */
    volatile uint32_t corestate_sys_mbx5;                     /* 0x094 */
    volatile uint32_t corestate_sys_mbx6;                     /* 0x098 */
    volatile uint32_t corestate_sys_mbx7;                     /* 0x09c */
    volatile uint32_t corestate_usr_mbx0;                     /* 0x0a0 */
    volatile uint32_t corestate_usr_mbx1;                     /* 0x0a4 */
    volatile uint32_t corestate_usr_mbx2;                     /* 0x0a8 */
    volatile uint32_t corestate_usr_mbx3;                     /* 0x0ac */
    volatile uint32_t corestate_usr_mbx4;                     /* 0x0b0 */
    volatile uint32_t corestate_usr_mbx5;                     /* 0x0b4 */
    volatile uint32_t corestate_usr_mbx6;                     /* 0x0b8 */
    volatile uint32_t corestate_usr_mbx7;                     /* 0x0bc */
    volatile uint32_t corestate_sys_mtx0;                     /* 0x0c0 */
    volatile uint32_t corestate_sys_mtx1;                     /* 0x0c4 */
    volatile uint32_t corestate_sys_mtx2;                     /* 0x0c8 */
    volatile uint32_t corestate_sys_mtx3;                     /* 0x0cc */
    volatile uint32_t corestate_usr_mtx0;                     /* 0x0d0 */
    volatile uint32_t corestate_usr_mtx1;                     /* 0x0d4 */
    volatile uint32_t corestate_usr_mtx2;                     /* 0x0d8 */
    volatile uint32_t corestate_usr_mtx3;                     /* 0x0dc */
    volatile uint32_t corestate_usr_mtx4;                     /* 0x0e0 */
    volatile uint32_t corestate_usr_mtx5;                     /* 0x0e4 */
    volatile uint32_t corestate_usr_mtx6;                     /* 0x0e8 */
    volatile uint32_t corestate_usr_mtx7;                     /* 0x0ec */
    uint8_t gap2[0x100 - 0x0f0];

    volatile uint32_t interrupt_irq_status;                   /* 0x100 */
    volatile uint32_t interrupt_irq_set;                      /* 0x104 */
    volatile uint32_t interrupt_irq_clear;                    /* 0x108 */
    uint8_t gap30[0x110 - 0x10c];
    volatile uint32_t interrupt_srq_status;                   /* 0x110 */
    volatile uint32_t interrupt_srq_set;                      /* 0x114 */
    volatile uint32_t interrupt_srq_clear;                    /* 0x118 */
    uint8_t gap31[0x120 - 0x11c];
    volatile uint32_t interrupt_drq_status;                   /* 0x120 */
    volatile uint32_t interrupt_drq_set;                      /* 0x124 */
    volatile uint32_t interrupt_drq_clear;                    /* 0x128 */
    uint8_t gap32[0x130 - 0x12c];
    volatile uint32_t interrupt_frq_status;                   /* 0x130 */
    volatile uint32_t interrupt_frq_set;                      /* 0x134 */
    volatile uint32_t interrupt_frq_clear;                    /* 0x138 */
    uint8_t gap33[0x140 - 0x13c];
    volatile uint32_t interrupt_host_irq_latched;             /* 0x140 */
    volatile uint32_t interrupt_host_irq_set;                 /* 0x144 */
    volatile uint32_t interrupt_host_irq_clear;               /* 0x148 */
    volatile uint32_t interrupt_host_irq_enable;              /* 0x14c */
    volatile uint32_t interrupt_obusfault_status;             /* 0x150 */
    volatile uint32_t interrupt_obusfault_clear;              /* 0x154 */
    volatile uint32_t interrupt_obusfault_address;            /* 0x158 */
    uint8_t gap3[0x180 - 0x15c];

    volatile uint32_t profile_mutex;                          /* 0x180 */
    volatile uint32_t profile_last_conf_pc_lo;                /* 0x184 */
    volatile uint32_t profile_last_conf_pc_hi;                /* 0x188 */
    volatile uint32_t profile_last_pc_lo;                     /* 0x18c */
    volatile uint32_t profile_last_pc_hi;                     /* 0x190 */
    volatile uint32_t profile_bra_target_pc_0_lo;             /* 0x194 */
    volatile uint32_t profile_bra_target_pc_0_hi;             /* 0x198 */
    volatile uint32_t profile_bra_target_pc_1_lo;             /* 0x19c */
    volatile uint32_t profile_bra_target_pc_1_hi;             /* 0x1a0 */
    volatile uint32_t profile_bra_target_pc_2_lo;             /* 0x1a4 */
    volatile uint32_t profile_bra_target_pc_2_hi;             /* 0x1a8 */
    volatile uint32_t profile_bra_target_pc_3_lo;             /* 0x1ac */
    volatile uint32_t profile_bra_target_pc_3_hi;             /* 0x1b0 */
    uint8_t gap40[0x1c0 - 0x1b4];
    volatile uint32_t profile_prof_sample_w0;                 /* 0x1c0 */
    volatile uint32_t profile_prof_sample_w1;                 /* 0x1c4 */
    volatile uint32_t profile_prof_sample_w2;                 /* 0x1c8 */
    volatile uint32_t profile_prof_sample_w3;                 /* 0x1cc */
#endif  /* DOXYGEN */
} Misc_Block;


#if NUM_CORES == 1 && NUM_SUBSYSTEMS == 1
__absolute __align(4)
extern Misc_Block taddr_Misc_Block;
#endif

#endif  /* defined(__FIREPATH__) && !defined(ASMCPP) && !defined(__LINKER_SCRIPT__) */


/**
 * Misc Block offsets.
 * @{
 */
#define MISC_BLOCK_CORECTRL_CORE_ENABLE                  0x000
#define MISC_BLOCK_CORECTRL_AUTO_RESET_CONTROL           0x004
#define MISC_BLOCK_CORECTRL_CORE_IDLE                    0x008
#define MISC_BLOCK_CORECTRL_CORE_RESET_CAUSE             0x00c
#define MISC_BLOCK_CORECTRL_MEMORY_POWER_DOWN_CONTROL_0  0x010
#define MISC_BLOCK_CORECTRL_MEMORY_POWER_DOWN_STATUS_0   0x014
#define MISC_BLOCK_CORECTRL_MEMORY_POWER_DOWN_CONTROL_1  0x018
#define MISC_BLOCK_CORECTRL_MEMORY_POWER_DOWN_STATUS_1   0x01c
#define MISC_BLOCK_CORECTRL_SYS_FLG0_STATUS              0x020
#define MISC_BLOCK_CORECTRL_SYS_FLG0_SET                 0x024
#define MISC_BLOCK_CORECTRL_SYS_FLG0_CLEAR               0x028
#define MISC_BLOCK_CORECTRL_USR_FLG0_STATUS              0x030
#define MISC_BLOCK_CORECTRL_USR_FLG0_SET                 0x034
#define MISC_BLOCK_CORECTRL_USR_FLG0_CLEAR               0x038
#define MISC_BLOCK_CORECTRL_SUBSYSTEM_REVISION           0x040
#define MISC_BLOCK_CORECTRL_RESET_VECTOR                 0x044

#define MISC_BLOCK_CORESTATE_SYS_MBX0                    0x080
#define MISC_BLOCK_CORESTATE_SYS_MBX1                    0x084
#define MISC_BLOCK_CORESTATE_SYS_MBX2                    0x088
#define MISC_BLOCK_CORESTATE_SYS_MBX3                    0x08c
#define MISC_BLOCK_CORESTATE_SYS_MBX4                    0x090
#define MISC_BLOCK_CORESTATE_SYS_MBX5                    0x094
#define MISC_BLOCK_CORESTATE_SYS_MBX6                    0x098
#define MISC_BLOCK_CORESTATE_SYS_MBX7                    0x09c
#define MISC_BLOCK_CORESTATE_USR_MBX0                    0x0a0
#define MISC_BLOCK_CORESTATE_USR_MBX1                    0x0a4
#define MISC_BLOCK_CORESTATE_USR_MBX2                    0x0a8
#define MISC_BLOCK_CORESTATE_USR_MBX3                    0x0ac
#define MISC_BLOCK_CORESTATE_USR_MBX4                    0x0b0
#define MISC_BLOCK_CORESTATE_USR_MBX5                    0x0b4
#define MISC_BLOCK_CORESTATE_USR_MBX6                    0x0b8
#define MISC_BLOCK_CORESTATE_USR_MBX7                    0x0bc
#define MISC_BLOCK_CORESTATE_SYS_MTX0                    0x0c0
#define MISC_BLOCK_CORESTATE_SYS_MTX1                    0x0c4
#define MISC_BLOCK_CORESTATE_SYS_MTX2                    0x0c8
#define MISC_BLOCK_CORESTATE_SYS_MTX3                    0x0cc
#define MISC_BLOCK_CORESTATE_USR_MTX0                    0x0d0
#define MISC_BLOCK_CORESTATE_USR_MTX1                    0x0d4
#define MISC_BLOCK_CORESTATE_USR_MTX2                    0x0d8
#define MISC_BLOCK_CORESTATE_USR_MTX3                    0x0dc
#define MISC_BLOCK_CORESTATE_USR_MTX4                    0x0e0
#define MISC_BLOCK_CORESTATE_USR_MTX5                    0x0e4
#define MISC_BLOCK_CORESTATE_USR_MTX6                    0x0e8
#define MISC_BLOCK_CORESTATE_USR_MTX7                    0x0ec

#define MISC_BLOCK_INTERRUPT_IRQ_STATUS                  0x100
#define MISC_BLOCK_INTERRUPT_IRQ_SET                     0x104
#define MISC_BLOCK_INTERRUPT_IRQ_CLEAR                   0x108
#define MISC_BLOCK_INTERRUPT_SRQ_STATUS                  0x110
#define MISC_BLOCK_INTERRUPT_SRQ_SET                     0x114
#define MISC_BLOCK_INTERRUPT_SRQ_CLEAR                   0x118
#define MISC_BLOCK_INTERRUPT_DRQ_STATUS                  0x120
#define MISC_BLOCK_INTERRUPT_DRQ_SET                     0x124
#define MISC_BLOCK_INTERRUPT_DRQ_CLEAR                   0x128
#define MISC_BLOCK_INTERRUPT_FRQ_STATUS                  0x130
#define MISC_BLOCK_INTERRUPT_FRQ_SET                     0x134
#define MISC_BLOCK_INTERRUPT_FRQ_CLEAR                   0x138
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_LATCHED            0x140
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SET                0x144
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_CLEAR              0x148
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_ENABLE             0x14c
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_STATUS            0x150
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_CLEAR             0x154
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_ADDRESS           0x158

#define MISC_BLOCK_PROFILE_MUTEX                         0x180
#define MISC_BLOCK_PROFILE_LAST_CONF_PC_LO               0x184
#define MISC_BLOCK_PROFILE_LAST_CONF_PC_HI               0x188
#define MISC_BLOCK_PROFILE_LAST_PC_LO                    0x18c
#define MISC_BLOCK_PROFILE_LAST_PC_HI                    0x190
#define MISC_BLOCK_PROFILE_BRA_TARGET_PC_0_LO            0x194
#define MISC_BLOCK_PROFILE_BRA_TARGET_PC_0_HI            0x198
#define MISC_BLOCK_PROFILE_BRA_TARGET_PC_1_LO            0x19c
#define MISC_BLOCK_PROFILE_BRA_TARGET_PC_1_HI            0x1a0
#define MISC_BLOCK_PROFILE_BRA_TARGET_PC_2_LO            0x1a4
#define MISC_BLOCK_PROFILE_BRA_TARGET_PC_2_HI            0x1a8
#define MISC_BLOCK_PROFILE_BRA_TARGET_PC_3_LO            0x1ac
#define MISC_BLOCK_PROFILE_BRA_TARGET_PC_3_HI            0x1b0
#define MISC_BLOCK_PROFILE_PROF_SAMPLE_W0                0x1c0
#define MISC_BLOCK_PROFILE_PROF_SAMPLE_W1                0x1c4
#define MISC_BLOCK_PROFILE_PROF_SAMPLE_W2                0x1c8
#define MISC_BLOCK_PROFILE_PROF_SAMPLE_W3                0x1cc
/** @} */

#define MISC_BLOCK_SIZE                                  0x200


/**
 * Misc Block MISC_BLOCK_INTERRUPT_HOST_IRQ bits.
 * @{
 */
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_CORE_RESET         (1 << 0)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_PROFILER           (1 << 1)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SYS_SW_INT_0       (1 << 8)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SYS_SW_INT_1       (1 << 9)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SYS_SW_INT_2       (1 << 10)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SYS_SW_INT_3       (1 << 11)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SYS_SW_INT_4       (1 << 12)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SYS_SW_INT_5       (1 << 13)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SYS_SW_INT_6       (1 << 14)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_SYS_SW_INT_7       (1 << 15)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_USR_SW_INT_0       (1 << 16)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_USR_SW_INT_1       (1 << 17)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_USR_SW_INT_2       (1 << 18)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_USR_SW_INT_3       (1 << 19)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_USR_SW_INT_4       (1 << 20)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_USR_SW_INT_5       (1 << 21)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_USR_SW_INT_6       (1 << 22)
#define MISC_BLOCK_INTERRUPT_HOST_IRQ_USR_SW_INT_7       (1 << 23)
/** @} */


/**
 * Misc Block MISC_BLOCK_INTERRUPT_OBUSFAULT bits.
 * @{
 */
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_FAULT_PENDING     (1 << 0)
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_CMD_MASK          0x00000030
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_CMD_BITS          2
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_CMD_SHIFT         4
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_BYTEEN_MASK       0x0000ff00
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_BYTEEN_BITS       8
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_BYTEEN_SHIFT      8
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_ID_MASK           0x0fff0000
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_ID_BITS           12
#define MISC_BLOCK_INTERRUPT_OBUSFAULT_ID_SHIFT          16

#define MISC_BLOCK_INTERRUPT_OBUSFAULT_CLEAR_FAULT       (1 << 0)
/** @} */


#define MISC_BLOCK_GET_FIELD(var, field)    (((var) & MISC_BLOCK_##field##_MASK) >> MISC_BLOCK_##field##_SHIFT)


#if defined(__LINKER_SCRIPT__)
/*
 * Useful memory protection macros independent from the "classic" protection support.
 */
#  include "dreg-numbers.h"
#  include "libfp/src/c_utils_internal.h"

/**
 * Linker script macro for calculating protection ranges top value.
 *
 * From CRFIREPATH-1023:
 * The ranges have a bottom and top address value. The address values have an alignment restriction
 * of 32 bytes (to match the L1 cache line size and also to allow the bottom 5 bits of the address
 * register to be used for the permissions). The top and bottom values are both inclusive.
 * I.e. setting them equal means a range of 32 bytes is specified. Setting them to bottom all zeros,
 * top all ones means the whole address space is specified. Setting the top to a value below the
 * bottom means the range will never be hit.
 * @{
 */
ASSERT(PROTI_TOP_ADDRESS_MASK == PROTD_TOP_ADDRESS_MASK, "Error: PROTI_TOP_ADDRESS_MASK != PROTD_TOP_ADDRESS_MASK")
#  define ADDR_TO_PROT_TOP(addr)            ((addr) == 0 ? 0 : ((addr) - 1) & PROTI_TOP_ADDRESS_MASK)
/** @} */

#  if CORE_HAS_DREG_FEATURE_MPU_ICA || CORE_HAS_DREG_FEATURE_MPU_DCA
/**
 * Constants for a "closed" (inaccessible) PROTD/PROTI region.
 * @{
 */
#    define PROT_ALWAYS_CLOSED_BOT      (0xffffffff & PROTD_TOP_ADDRESS_MASK)
#    define PROT_ALWAYS_CLOSED_TOP      0
/** @} */
#  endif /* CORE_HAS_DREG_FEATURE_MPU_DCA */

#  if CORE_HAS_DREG_FEATURE_MPU_ICA || CORE_HAS_DREG_FEATURE_MPU_DCA
/**
 * Linker scripts macros for marking DTCM protection regions boundaries.
 * They require the taddr_dtcm_prot and tsize_dtcm_prot symbols to be defined to work properly.
 * @{
 */
#    define CEIL_TO_DTCM_PROT_RANGE(size)           ((size < 1024) ? 1024 : CEIL_TO_POWER_OF_2(size))
#    define DTCM_PROT_REGION_SIZE                   (tsize_dtcm_prot / 32)
#    define FLOOR_TO_DTCM_PROT_REGION(addr)         FLOOR_TO_MULTIPLE_OF_POW2(addr, DTCM_PROT_REGION_SIZE)
#    define CEIL_TO_DTCM_PROT_REGION(addr)          CEIL_TO_MULTIPLE_OF_POW2(addr, DTCM_PROT_REGION_SIZE)
#    define ADDR_TO_DTCM_PROT_REGION(addr)          (((addr) - taddr_dtcm_prot) / DTCM_PROT_REGION_SIZE)
#    define ADDR_TO_DTCM_PROT_REGION_BIT(addr)      (1 << ADDR_TO_DTCM_PROT_REGION(addr))
#    define DTCM_PROT_REGION_RW_MASK(first, last)   ((((0xffffffffffffffff << (62 - (last) * 2)) & 0xffffffffffffffff) \
                                                     >> (62 - ((last) - (first)) * 2)) << ((first) * 2))
#    define DTCM_PROT_REGION_R_MASK(first, last)    (DTCM_PROT_REGION_RW_MASK(first, last) & 0x5555555555555555)
#    define DTCM_PROT_REGION_W_MASK(first, last)    (DTCM_PROT_REGION_RW_MASK(first, last) & 0xaaaaaaaaaaaaaaaa)
#    define ASSERT_DTCM_PROT_REGION_ALIGNED(what)   ASSERT(IS_MULTIPLE_OF_POW2((what), DTCM_PROT_REGION_SIZE), \
                                                           STRINGIFY(what is not aligned to DTCM region size))
/** @} */
#  endif
#endif  /* defined(__LINKER_SCRIPT__) */


/*
 * "Classic" memory protection support.
 */
#if CLASSIC_MEMORY_PROTECTION

/*
 * DTCM-based support.
 */
#  if CORE_HAS_DREG_FEATURE_MPU_DTCM
#    if !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
/**
 * DTCM protection boundaries and masks symbols.
 * @{
 */
extern char __begin_protected_dtcm, __end_protected_dtcm;
extern char __begin_unprotected_dtcm, __end_unprotected_dtcm;
extern char __system_scenario_dtcm_mask_bot, __system_scenario_dtcm_mask_top;
extern char __user_scenario_dtcm_mask_bot, __user_scenario_dtcm_mask_top;
/** @} */
#    endif
#  endif /* CORE_HAS_DREG_FEATURE_MPU_DTCM */

/*
 * DCMEM-based support.
 */
#  if CORE_HAS_DREG_FEATURE_MPU_DCA
/**
 * One PROTD region is reserved to cover the SDK "protected" data/bss.
 * @{
 */
#    define CLASSIC_PROT_PROTD_BOT    (DIR_PROTD_BOT_0 + 2 * CLASSIC_MEMORY_PROTECTION_PROTD_REGION)
#    define CLASSIC_PROT_PROTD_TOP    (DIR_PROTD_BOT_0 + 2 * CLASSIC_MEMORY_PROTECTION_PROTD_REGION)
/** @} */

/* The SDK configuration mechanism set this to a negative number on purpose if something is wrong */
#    if CLASSIC_MEMORY_PROTECTION_PROTD_REGION < 0 && !defined(__COMPILE_HEADER__)
       /* Disable this check on DSL chips for the moment, as they use a different scheme */
#      if !CHIP_CLASS_DSL
#        error "CLASSIC_MEMORY_PROTECTION_PROTD_REGION < 0, please check the SoC configuration file"
#      endif
#    endif
#  endif /* CORE_HAS_DREG_FEATURE_MPU_DCA */

#else  /* CLASSIC_MEMORY_PROTECTION */

#  if CORE_HAS_DREG_FEATURE_MPU_DTCM
#    if !defined(ASMCPP) && !defined(__LINKER_SCRIPT__)
/**
 * DTCM loose protection boundaries and masks symbols - just wrapping (with
 * region size granularity) existing data / bss.
 * @{
 */
extern char __begin_allowed_dtcm, __end_allowed_dtcm;
extern char __allowed_dtcm_mask_bot, __allowed_dtcm_mask_top;
/** @} */
#    endif
#  endif

#endif /* CLASSIC_MEMORY_PROTECTION */


/* Define the Heartbeat word address for the single-core/single-subsystem case. */
#if defined(ENABLE_HEARTBEAT) && NUM_CORES == 1 && NUM_SUBSYSTEMS == 1
#  if !(defined(ASMCPP) || defined(__RALL2__))
#    define HEARTBEAT_SHARED_ADDRESS    (&taddr_Misc_Block.corestate_sys_mbx4)
#  else
#    define HEARTBEAT_SHARED_ADDRESS    (taddr_Misc_Block + MISC_BLOCK_CORESTATE_SYS_MBX4)
#  endif
#endif


#endif /* _MEMMAP_OCTAVE_V2_MAESTRO_V1_H_ */
