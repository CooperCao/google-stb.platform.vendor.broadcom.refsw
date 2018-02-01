/****************************************************************************
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
 ****************************************************************************/

/**
 * @file
 * @ingroup libdspcontrol
 * @brief Functions for accessing DSP subsystems.
 *
 * This module contains functions to access DSP subsystems from the Host.\n
 *
 * From the library point of view, each system is composed of certain number of
 * DSP subsystems (each containing one or more FP cores) and possibly other
 * peripherals; a Host processor (where libdspcontrol-based processes run) can
 * be part of the system or live outside the system. The Host is capable of
 * communicating to the system and/or the DSPs, through direct or indirect means.
 * The number and type of shared entities (e.g. memory areas) and capabilities
 * vary from system to system.\n
 * To generalise different systems behaviours, in the DSP module functions two
 * types of address spaces are defined:
 * - the address space offered by the interface used by the Host to communicate
 *   with the whole system/chip. It is usually a "global" address space that
 *   includes also the DSP address space, possibly remapped.
 * - the address space visible from the DSP, usually a subset (with different
 *   mappings) of the global address space.
 * .
 * Be aware that different memory-access functions operate on different address
 * spaces; where possible, #DSP_addrDsp2System and #DSP_addrSystem2Dsp are
 * implemented to help in translating between address spaces. Take into account
 * that each system offers different levels of access to different areas of the
 * memory space.\n
 *
 * Usage note: before calling any of the DSP module functions, please initialise
 * a DSP structure calling #DSP_init; upon finishing working with the DSP instance,
 * call #DSP_finish to release resources.
 */

#ifndef _DSP_H_
#define _DSP_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"
#include "fp_sdk_config.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stddef.h>
#  include <stdint.h>
#else
#  include "bstd.h"
#endif



#if IS_HOST(DSP_LESS)
#  error "This header is not supported on DSP-less builds"
#endif


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @{
 * System address types.
 */
#if IS_HOST(SILICON)
#  if IS_TARGET(Pike_haps) || IS_TARGET(RaagaFP4015_haps)
typedef uint32_t SYSTEM_ADDR;
typedef uint32_t SYSTEM_ADDR_SIZE;
#    define SYSTEM_ADDR_CAST(x)     ((SYSTEM_ADDR)(x))
#  else
typedef uint8_t *SYSTEM_ADDR;
typedef size_t   SYSTEM_ADDR_SIZE;
#    define SYSTEM_ADDR_CAST(x)     ((SYSTEM_ADDR)(uintptr_t)(x))
#  endif
#elif IS_HOST(BM)
typedef uint32_t SYSTEM_ADDR;
typedef uint32_t SYSTEM_ADDR_SIZE;
#  define SYSTEM_ADDR_CAST(x)       ((SYSTEM_ADDR)(x))
#endif
/** @} */


/**
 * @{
 * Shared address types.
 */
#if IS_HOST(SILICON)
#  if IS_TARGET(Pike_haps) || IS_TARGET(RaagaFP4015_haps)
typedef uint32_t SHARED_ADDR;
typedef uint32_t SHARED_ADDR_SIZE;
#    define SHARED_ADDR_CAST(x)     ((SHARED_ADDR)(x))
#  elif IS_TARGET(RaagaFP4015_si_magnum) || IS_TARGET(RaagaFP4015_si_magnum_permissive)
typedef uint32_t SHARED_ADDR;
typedef uint32_t SHARED_ADDR_SIZE;
#    define SHARED_ADDR_CAST(x)     ((SHARED_ADDR)(x))
#  else
typedef uint8_t *SHARED_ADDR;
typedef size_t   SHARED_ADDR_SIZE;
#    define SHARED_ADDR_CAST(x)     ((SHARED_ADDR)(uintptr_t)(x))
#  endif
#elif IS_HOST(BM)
typedef uint32_t SHARED_ADDR;
typedef uint32_t SHARED_ADDR_SIZE;
#  define SHARED_ADDR_CAST(x)       ((SHARED_ADDR)(x))
#endif
/** @} */


/**
 * @{
 * DSP core address types.
 */
typedef uint32_t DSP_ADDR;
typedef uint32_t DSP_ADDR_SIZE;
#define DSP_ADDR_CAST(x)            ((DSP_ADDR)(x))
/** @} */


/**
 * Enumeration of the address spaces supported by the DSP module.
 */
typedef enum
{
    ADDR_SPACE_SYSTEM,  /**< 'system' address space */
    ADDR_SPACE_SHARED,  /**< 'shared' address space */
    ADDR_SPACE_DSP      /**< 'dsp' address space */
} ADDR_SPACE;


/**
 * @{
 * Any of the possible address types.
 */
typedef struct
{
    union __attribute__((packed)) ADDR_UNION
    {
        SYSTEM_ADDR system;     /**< 'system' address space */
        SHARED_ADDR shared;     /**< 'shared' address space */
        DSP_ADDR    dsp;        /**< 'dsp' address space */
    } addr;
    ADDR_SPACE addr_space;      /**< which of the possible addr entries is valid */
} ADDR;

typedef union __attribute__((packed)) ADDR_SIZE
{
    SYSTEM_ADDR_SIZE system;     /**< 'system' address space */
    SHARED_ADDR_SIZE shared;     /**< 'shared' address space */
    DSP_ADDR_SIZE    dsp;        /**< 'dsp' address space */
} ADDR_SIZE;
/** @} */


#if FEATURE_IS(DSP_SYS_ADDR_DIRECT_ACCESS, SUPPORTED)

/**
 * A mapped SYSTEM_ADDR for direct memory access.
 */
typedef struct
{
    void   *addr;       /**< start address of the mapped region */
    size_t  length;     /**< length in bytes of the mapped region */
} MAPPED_ADDR;

#endif


/**
 * Enumeration of the possible return values of DSP functions.
 */
typedef enum
{
    DSP_SUCCESS                       = 0,
    DSP_FAILURE                       = 1,
    DSP_NOT_SUPPORTED                 = 2,
    DSP_WRONG_SUBSYSTEM               = 3,
    DSP_WRONG_CORE                    = 4,
    DSP_ALREADY_ENABLED_CORE          = 5,
    DSP_DEAD_CORE                     = 6,
    DSP_BAD_ADDRESS_RANGE             = 7,
    DSP_BAD_ADDRESS_ALIGNMENT         = 8,
    DSP_BAD_SIZE                      = 9,
    DSP_BAD_ADDR_OR_SIZE              = 10, /**< A superset of DSP_BAD_ADDRESS_ALIGNMENT or DSP_BAD_SIZE */
    DSP_DEBUG_CONSOLE_NOT_INITIALISED = 11,
    DSP_DEBUG_CONSOLE_NO_DATA         = 12,
    DSP_DEBUG_CONSOLE_NO_SPACE        = 13,
    DSP_UNKNOW_OPTION                 = 14
} DSP_RET;


/** Returns true if ret_value indicates a success. */
#define DSP_SUCCEEDED(ret_value)    ((ret_value) == DSP_SUCCESS)
/** Returns true if ret_value indicates a failure. */
#define DSP_FAILED(ret_value)       ((ret_value) != DSP_SUCCESS)


/**
 * Enumeration of the types of memory areas visible from the Host.
 * Laid out as a bit set to ease filtering.
 */
typedef enum
{
    DSP_MEMORY_TYPE_FP_ROM      = 0x0001,   /**< Firepath ROM  */
    DSP_MEMORY_TYPE_FP_SMEM     = 0x0002,   /**< Firepath SMEM */
    DSP_MEMORY_TYPE_FP_DMEM     = 0x0004,   /**< Firepath DMEM */
    DSP_MEMORY_TYPE_SHARED      = 0x0008    /**< Shared memory, usually registers / a mailbox */
} DSP_MEMORY_TYPE;


/**
 * Flags describing a memory area capabilities.
 */
typedef enum
{
    DSP_MEMORY_CAN_READ             = 0x01,     /**< this memory area can be read (e.g. with #DSP_readData) */
    DSP_MEMORY_CAN_READ_INDIRECT    = 0x02,     /**< an indirect way of accessing memory is used (e.g. DMA) */
    DSP_MEMORY_CAN_READ_MAPPED      = 0x04,     /**< this memory area is mirrored / mapped to be accessed */
    DSP_MEMORY_CAN_READ_SLOW        = 0x08,     /**< expect slow performances when reading */
    DSP_MEMORY_CAN_WRITE            = 0x10,     /**< this memory area can be written (e.g. with #DSP_writeData) */
    DSP_MEMORY_CAN_WRITE_INDIRECT   = 0x20,     /**< an indirect way of accessing memory is used (e.g. DMA) */
    DSP_MEMORY_CAN_WRITE_MAPPED     = 0x40,     /**< this memory area is mirrored / mapped to be accessed */
    DSP_MEMORY_CAN_WRITE_SLOW       = 0x80      /**< expect slow performances when writing */
} DSP_MEMORY_FLAGS;


/**
 * Flags describing a memory area access constraints.
 */
typedef struct
{
    uint8_t read_alignment;     /**< read accesses must be this number of bytes aligned */
    uint8_t read_size;          /**< read accesses sizes must be multiple of this number of bytes */
    uint8_t write_alignment;    /**< write accesses must be this number of bytes aligned */
    uint8_t write_size;         /**< write accesses sizes must be multiple of this number of bytes */
} DSP_MEMORY_ACCESS_CONSTRAINTS;


/**
 * Structure containing information about a memory area and how to access it.
 */
typedef struct
{
    DSP_MEMORY_TYPE type;               /**< what broad class does this memory fall into? */
    DSP_MEMORY_FLAGS flags;             /**< library capabilities over this memory */
    DSP_MEMORY_ACCESS_CONSTRAINTS constraints;  /**< size/alignment access constraints */
    ADDR startAddress;                  /**< where data can be accessed in addressSpace address space */
    ADDR_SIZE length;                   /**< memory area length in bytes */
    DSP_ADDR dspAddress;                /**< at which address the DSP can access this memory, */
                                        /**< DSP_MEMORY_AREA_NO_DSP_ACCESS if this memory area is not accessible from the DSP
                                             If type is any of DSP_MEMORY_TYPE_FP_* this is guaranteed to be a valid address. */
} DSP_MEMORY_AREA;


#define DSP_MEMORY_AREA_NO_DSP_ACCESS   DSP_ADDR_CAST(0xFFFFFFFF)   /**< @see DSP_MEMORY_AREA */


/**
 * Collection of visible memory areas in the system.
 */
typedef struct
{
    unsigned               areasCount;  /**< number of elements in the #areas array */
    const DSP_MEMORY_AREA *areas;       /**< array of #DSP_MEMORY_AREA structures */
} DSP_MEMORY_LAYOUT;


/**
 * Structure containing a decoded Heartbeat word.
 */
typedef struct
{
    uint8_t  phase;     /**< 'phase' part of the Heartbeat word */
    uint8_t  subphase;  /**< 'subphase' part of the Heartbeat word */
    uint16_t argument;  /**< 'argument' part of the Heartbeat word */
} DSP_HEARTBEAT;


/**
 * Bitmap in which each bit corresponds to a FP core in the DSP subsystem;
 * LSB is core 0.
 */
typedef uint16_t DSP_CORES_BITMAP;


/**
 * DSP cores masks for the #DSP_CORES_BITMAP type.
 */
typedef enum
{
    DSP_CORES_NONE = 0,
    DSP_CORES_ALL  = (1 << NUM_CORES) - 1,
    DSP_CORES_0    = (1 << 0),
    DSP_CORES_1    = (1 << 1),
    DSP_CORES_2    = (1 << 2),
    DSP_CORES_3    = (1 << 3),
    DSP_CORES_4    = (1 << 4),
    DSP_CORES_5    = (1 << 5),
    DSP_CORES_6    = (1 << 6),
    DSP_CORES_7    = (1 << 7),
    DSP_CORES_8    = (1 << 8),
    DSP_CORES_9    = (1 << 9),
    DSP_CORES_10   = (1 << 10),
    DSP_CORES_11   = (1 << 11),
    DSP_CORES_12   = (1 << 12),
    DSP_CORES_13   = (1 << 13),
    DSP_CORES_14   = (1 << 14),
    DSP_CORES_15   = (1 << 15)
} DSP_CORES_MASK;


/**
 * DSP cores enumeration.
 */
typedef enum
{
    DSP_CORE_0  = 0,
    DSP_CORE_1  = 1,
    DSP_CORE_2  = 2,
    DSP_CORE_3  = 3,
    DSP_CORE_4  = 4,
    DSP_CORE_5  = 5,
    DSP_CORE_6  = 6,
    DSP_CORE_7  = 7,
    DSP_CORE_8  = 8,
    DSP_CORE_9  = 9,
    DSP_CORE_10 = 10,
    DSP_CORE_11 = 11,
    DSP_CORE_12 = 12,
    DSP_CORE_13 = 13,
    DSP_CORE_14 = 14,
    DSP_CORE_15 = 15
} DSP_CORE;


#if NUM_CORES > 16
#  error "Please update DSP_CORES_BITMAP, DSP_CORES_MASK, DSP_CORE to support the number of cores on this SoC"
#endif


/**
 * DSP subsystems enumeration.
 */
typedef enum
{
    DSP_SUBSYS_0  = 0,
    DSP_SUBSYS_1  = 1
} DSP_SUBSYS;


#ifdef __cplusplus
}
#endif

/* Do include this here as it might need the above data types / constants definitions */
#include "libdspcontrol/DSP_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Initialiser for a structure of type #DSP_PARAMETERS.
 * The provided structure will be initialised with sane values, it's anyway
 * advisable to always check and tune all settings based on specific needs.
 *
 * @param[out] params  the #DSP_PARAMETERS structure to initialise
 */
void DSP_initParameters(DSP_PARAMETERS *params);


/**
 * DSP_init represents the "constructor" for the DSP object.
 * Calling this function doesn't affect the state of the DSP itself because a connection could be made
 * to a running DSP. It does instead claim any resources needed on the Host (e.g. opening hardware devices,
 * allocating physical memory).
 *
 * @param[out] dsp        the DSP instance to initialise
 * @param[in] parameters  platform-specific parameters used to initialise and customise the DSP object
 *                        behaviour; for more information, see the \ref DSP_PARAMETERS structure
 *                        documentation for the specific chip. The pointer structure should remain valid
 *                        until the return from DSP_init, not for the whole life of the DSP object.
 * @return                one of the #DSP_RET return values
 */
DSP_RET DSP_init(DSP *dsp, DSP_PARAMETERS *parameters);


/**
 * DSP_finish represents the "destructor" for the DSP object.
 * It should release any resources so that future DSP instances can run successfully.
 * It should not modify the DSP status itself (see #DSP_init).
 *
 * @param[in] dsp  the DSP instance to terminate
 */
void DSP_finish(DSP *dsp);


/**
 * Apply a reset signal to the DSP subsystem.
 *
 * In an ideal situation, on return from this call no core in the subsystem
 * should be executing instructions. Since reset is not supported on all
 * platforms, the return value should be checked to make sure the DSP are
 * really in reset. To be on the safe side, call #DSP_enabledStatus afterward
 * to check for enabled cores.
 *
 * @param[in] dsp  the DSP instance
 * @return         one of the #DSP_RET return values
 */
DSP_RET DSP_reset(DSP *dsp);


/**
 * Cause the DSP to start executing instructions on the specified cores.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] core  which core to enable
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_enable(DSP *dsp, DSP_CORE core);


/**
 * Return a bitmap describing whether each core is enabled or disabled.
 *
 * @param[in] dsp     the DSP instance
 * @param[out] cores  enabled cores bitmap
 * @return            one of the #DSP_RET return values
 */
DSP_RET DSP_enabledStatus(DSP *dsp, DSP_CORES_BITMAP *cores);


/**
 * Translate one address from the address space visible from the DSP to the one
 * offered by the interface used by the Host to communicate with the system.
 *
 * @param[in]  dsp      the DSP instance
 * @param[in]  dsp_addr the address to be translated (DSP address space)
 * @param[out] sys_addr the translated address (system address space)
 * @return              one of the #DSP_RET return values
 */
DSP_RET DSP_addrDsp2System(DSP *dsp, DSP_ADDR dsp_addr, SYSTEM_ADDR *sys_addr);


/**
 * Translate one address from the address space offered by the interface used
 * by the Host to communicate with the system to the one visible from the DSP.
 *
 * @param[in]  dsp      the DSP instance
 * @param[in]  sys_addr the address to be translated (system address space)
 * @param[out] dsp_addr the translated address (DSP address space)
 * @return              one of the #DSP_RET return values
 */
DSP_RET DSP_addrSystem2Dsp(DSP *dsp, SYSTEM_ADDR sys_addr, DSP_ADDR *dsp_addr);


/**
 * Copy a block of data into the system memory.
 * The visible address space is the one offered by the interface used by the Host
 * to communicate with the system/DSP.
 *
 * @see DSP_readSystemData.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] dest  data destination address, in system address space
 * @param[in] src   where to copy data from, Host address space
 * @param[in] n     number of bytes to copy
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_writeSystemData(DSP *dsp, SYSTEM_ADDR dest, const void *src, SYSTEM_ADDR_SIZE n);


/**
 * Copy a block of data from system memory.
 * The visible address space is the one offered by the interface used by the Host
 * to communicate with the system/DSP.
 *
 * @see DSP_writeSystemData.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] dest  data destination address, Host address space
 * @param[in] src   where to copy data from, in system address space
 * @param[in] n     number of bytes to copy
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_readSystemData(DSP *dsp, void *dest, SYSTEM_ADDR src, SYSTEM_ADDR_SIZE n);


/**
 * Copy a block of data into the DSP.
 * The destination address is interpreted as the one visible from the DSP.
 *
 * @see DSP_readDspData.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] dest  data destination address, in DSP address space
 * @param[in] src   where to copy data from, Host address space
 * @param[in] n     number of bytes to copy
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_writeDspData(DSP *dsp, DSP_ADDR dest, const void *src, DSP_ADDR_SIZE n);


/**
 * Copy a block of data from DSP memory.
 * The source address is interpreted as the one visible from the DSP.
 *
 * @see DSP_writeDspData.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] dest  data destination address, Host address space
 * @param[in] src   where to copy data from, in DSP address space
 * @param[in] n     number of bytes to copy
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_readDspData(DSP *dsp, void *dest, DSP_ADDR src, DSP_ADDR_SIZE n);


/**
 * Copy a block of data into the shared area of memory visible both from the Host and the DSP.
 * The destination address is interpreted as one offered by the interface
 * used by the Host to communicate with the system/DSP.
 *
 * @see DSP_readSharedData.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] dest  data destination address, in system address space
 * @param[in] src   where to copy data from, Host address space
 * @param[in] n     number of bytes to copy
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_writeSharedData(DSP *dsp, SHARED_ADDR dest, const void *src, SHARED_ADDR_SIZE n);


/**
 * Copy a block of data from the shared area of memory visible both from the Host and the DSP.
 * The source address is interpreted as one offered by the interface
 * used by the Host to communicate with the system/DSP.
 *
 * @see DSP_writeSharedData.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] dest  data destination address, Host address space
 * @param[in] src   where to copy data from, in system address space
 * @param[in] n     number of bytes to copy
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_readSharedData(DSP *dsp, void *dest, SHARED_ADDR src, SHARED_ADDR_SIZE n);


/**
 * Write a hardware register (a 32 bit word) in shared memory.
 * Potentially less safe that DSP_writeSharedData, as in case of
 * errors there's no safe way to notify the caller.
 *
 * @see DSP_writeSharedData
 *
 * @param[in] dsp       the DSP instance
 * @param[in] reg_addr  register address, in system address space
 * @param[in] value     the data to write
 */
void DSP_writeSharedRegister(DSP *dsp, SHARED_ADDR reg_addr, uint32_t value);


/**
 * Read a hardware register (a 32 bit word) in shared memory.
 * Potentially less safe that DSP_readSharedData, as in case of
 * errors there's no safe way to notify the caller.
 *
 * @see DSP_writeSharedData
 *
 * @param[in] dsp       the DSP instance
 * @param[in] reg_addr  register address, in system address space
 * @return              the read data
 */
uint32_t DSP_readSharedRegister(DSP *dsp, SHARED_ADDR reg_addr);


/**
 * Copy a block of data into memory, dispatching the call to the address space-specific
 * function (#DSP_writeSystemData, #DSP_writeSharedData or #DSP_writeDspData).
 *
 * @see DSP_writeSystemData
 * @see DSP_writeSharedData
 * @see DSP_writeDspData
 *
 * @param[in] dsp         the DSP instance
 * @param[in] dest        data destination address
 * @param[in] src         where to copy data from, Host address space
 * @param[in] n           number of bytes to copy
 * @return                one of the #DSP_RET return values
 */
DSP_RET DSP_writeData(DSP *dsp, ADDR dest, const void *src, ADDR_SIZE n);


/**
 * Copy a block of data from memory, dispatching the call to the address space-specific
 * function (#DSP_readSystemData, #DSP_readSharedData or #DSP_readDspData).
 *
 * @see DSP_readSystemData
 * @see DSP_readSharedData
 * @see DSP_readDspData
 *
 * @param[in] dsp        the DSP instance
 * @param[in] dest       data destination address, Host address space
 * @param[in] src        where to copy data from, in system address space
 * @param[in] src_space  address space of the data destination address
 * @param[in] n          number of bytes to copy
 * @return               one of the #DSP_RET return values
 */
DSP_RET DSP_readData(DSP *dsp, void *dest, ADDR src, ADDR_SIZE n);


/**
 * Copy a block of data into the system as if the transfer was originated from the DSP.
 * The data can end up into the DSP itself (so this would be fully equivalent to have called
 * DSP_writeDspData) or in any system component directly visible from the DSP.
 * The destination address is interpreted as the one visible from the DSP.
 *
 * @see DSP_readDataAsDsp.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] dest  data destination address, in DSP address space
 * @param[in] src   where to copy data from, Host address space
 * @param[in] n     number of bytes to copy
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_writeDataAsDsp(DSP *dsp, DSP_ADDR dest, const void *src, DSP_ADDR_SIZE n);


/**
 * Copy a block of data from the system as if the transfer was originated from the DSP.
 * The data can be read from the DSP itself (so this would be fully equivalent to have called
 * DSP_readDspData) or from any system component directly visible from the DSP.
 * The source address is interpreted as the one visible from the DSP.
 *
 * @see DSP_writeDataAsDsp.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] dest  data destination address, Host address space
 * @param[in] src   where to copy data from, in DSP address space
 * @param[in] n     number of bytes to copy
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_readDataAsDsp(DSP *dsp, void *dest, DSP_ADDR src, DSP_ADDR_SIZE n);


#if FEATURE_IS(DSP_SYS_ADDR_DIRECT_ACCESS, SUPPORTED)

/**
 * Map a SYSTEM_ADDR in local address space for direct access.
 *
 * @param[in]  dsp         the DSP instance
 * @param[in]  addr        SYSTEM_ADDR to map
 * @param[in]  length      length of the region starting at #addr to map
 * @param[out] mapped_addr the result of the mapping
 * @return                 one of the #DSP_RET return values
 */
__attribute__((nonnull))
DSP_RET DSP_mapSystemAddress(DSP *dsp, SYSTEM_ADDR addr, SYSTEM_ADDR_SIZE length, MAPPED_ADDR *mapped_addr);


/**
 * Unmap an address previously mapped with #DSP_mapSystemAddress.
 *
 * @param[in] dsp          the DSP instance
 * @param[in] mapped_addr  the region to unmap
 * @return
 */
__attribute__((nonnull))
DSP_RET DSP_unmapSystemAddress(DSP *dsp, MAPPED_ADDR *mapped_addr);

#endif /* FEATURE_IS(DSP_SYS_ADDR_DIRECT_ACCESS, SUPPORTED) */


/**
 * Reads interrupt bits from the specified DSP core.
 *
 * @param[in]  dsp         the DSP instance
 * @param[in]  core        which core to read interrupts for
 * @param[out] interrupts  a structure that will be filled with currently active interrupts information
 */
void DSP_pollInterrupts(DSP *dsp, DSP_CORE core, DSP_INTERRUPTS *interrupts);


/**
 * Clear interrupt bits on the specified DSP core.
 *
 * @param[in] dsp         the DSP instance
 * @param[in] core        which core to clear interrupts for
 * @param[in] interrupts  a structure with informations about which interrupts to reset
 */
void DSP_clearInterrupts(DSP *dsp, DSP_CORE core, DSP_INTERRUPTS *interrupts);


/**
 * Clear all interrupt bits from the specified DSP core.
 *
 * @param[in] core which core to clear all interrupts for
 * @param[in] dsp  the DSP instance
 */
void DSP_clearAllInterrupts(DSP *dsp, DSP_CORE core);


/**
 * Reads and decodes the Heartbeat word for the specified core.
 * Note: some chip don't support the Heartbeat service, in which case its
 *       functionality will be roughly emulated.
 *
 * @param[in]  dsp       the DSP instance
 * @param[in]  core      which core to read the Heartbeat word for
 * @param[out] heartbeat the heartbeat structure to be filled in; output value is undefined if the function doesn't succeed
 * @return               one of the #DSP_RET return values
 */
DSP_RET DSP_pollHeartbeat(DSP *dsp, DSP_CORE core, DSP_HEARTBEAT *heartbeat);


/**
 * Resets the Heartbeat word. Normally used at reset time before enabling the DSP.
 * Note: some chip don't support the Heartbeat service, in which case its
 *       functionality will be roughly emulated.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] core  which core to clear the Heartbeat word for
 * @return          one of the #DSP_RET return values
 */
DSP_RET DSP_clearHeartbeat(DSP *dsp, DSP_CORE core);


/**
 * Abstract way to set options regarding the DSP object.
 *
 * @param[in] dsp    the DSP instance
 * @param[in] option which option to set
 * @param[in] value  the new value for the option; pointed type is option-dependent
 * @return           DSP_SUCCESS if the options was set successfully, DSP_UNKNOW_OPTION otherwise
 */
DSP_RET DSP_setOption(DSP *dsp, DSP_OPTION option, void *value);


/**
 * Abstract way to get options regarding the DSP object.
 *
 * @param[in]  dsp    the DSP instance
 * @param[in]  option which option to get
 * @param[out] value  out parameter to store the value associated to the requested option; pointed type is option-dependent
 * @return            DSP_SUCCESS if the options was retrieved successfully, DSP_UNKNOW_OPTION otherwise
 */
DSP_RET DSP_getOption(DSP *dsp, DSP_OPTION option, void *value);


#ifndef __FP4014_ONWARDS__
/**
 * Query if data is available in the debug console read buffer
 * for the specified core.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] core  the core for which the debug console read buffer status is requested
 * @return          true if the console is available, false otherwise.
 */
bool DSP_debugConsoleDataAvailable(DSP *dsp, DSP_CORE core);


/**
 * Query if space is available in the debug console write buffer
 * for the specified core.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] core  the core for which the debug console write buffer status is requested
 * @return          true if space is available, false otherwise.
 */
bool DSP_debugConsoleSpace(DSP *dsp, DSP_CORE core);


/**
 * Read a byte from the debug console read buffer for the specified core.
 *
 * @param[in]  dsp   the DSP instance
 * @param[in]  core  the core for which the debug console read is requested
 * @param[out] dst   destination for the read byte
 * @return           DSP_SUCCESS, DSP_DEBUG_CONSOLE_NO_DATA or DSP_DEBUG_CONSOLE_NOT_INITIALISED
 */
DSP_RET DSP_debugConsoleRead(DSP *dsp, DSP_CORE core, uint8_t *dst);


/**
 * Write a byte to the debug console write buffer for the specified core.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] core  the core for which the debug console write is requested
 * @param[in] src   the data to be written
 * @return          DSP_SUCCESS, DSP_DEBUG_CONSOLE_NO_SPACE or DSP_DEBUG_CONSOLE_NOT_INITIALISED
 */
DSP_RET DSP_debugConsoleWrite(DSP *dsp, DSP_CORE core, uint8_t src);

#endif  /* __FP4014_ONWARDS__ */


#ifdef __cplusplus
}
#endif


#endif  /* _DSP_H_ */
