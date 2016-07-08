/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

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

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stddef.h>
#  include <stdint.h>
#else
#  include "bstd.h"
#endif



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * A DSP core address.
 */
typedef uint32_t DSP_ADDR;


/**
 * Enumeration of the address spaces supported by the DSP module.
 */
typedef enum
{
    DSP_ADDRSPACE_SYSTEM,   /**< 'system' address space */
    DSP_ADDRSPACE_SHARED,   /**< 'shared' address space */
    DSP_ADDRSPACE_DSP       /**< 'dsp' address space */
} DSP_ADDRESS_SPACE;


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
    DSP_DEBUG_CONSOLE_NO_SPACE        = 13
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
    DSP_ADDRESS_SPACE addressSpace;     /**< the address space startAddress refers to */
    uint32_t startAddress;              /**< where data can be accessed in addressSpace address space */
    uint32_t length;                    /**< memory area length in bytes */
    DSP_ADDR dspAddress;                /**< at which address the DSP can access this memory, */
                                        /**< DSP_MEMORY_AREA_NO_DSP_ACCESS if this memory area is not accessible from the DSP
                                             If type is any of DSP_MEMORY_TYPE_FP_* this is guaranteed to be a valid address. */
} DSP_MEMORY_AREA;


#define DSP_MEMORY_AREA_NO_DSP_ACCESS   ((DSP_ADDR) 0xFFFFFFFF)  /**< @see DSP_MEMORY_AREA */


/**
 * Collection of visible memory areas in the system.
 */
typedef struct
{
    unsigned               areasCount;  /**< number of elements in the #areas array */
    const DSP_MEMORY_AREA *areas;       /**< array of #DSP_MEMORY_AREA structures */
} DSP_MEMORY_LAYOUT;


/**
 * Structure describing a memory area in DRAM.
 */
typedef struct
{
    void *pAddress;          /**< Host memory physical address, NULL if not available */
    void *vAddress;          /**< Host memory virtual address, NULL if not available */
    uint32_t dspAddress;     /**< at which address the DSP can access this memory, in a direct or indirect way */
    uint32_t length;         /**< size of the memory block */
} DSP_DRAM_MEMORY_AREA;


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
    DSP_CORE_11 = 11
} DSP_CORE;


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

/* Do this include here as it might need above definitions */
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
 * Return the system memory layout, that is a comprehensive list
 * of memory areas accessible from the Host.
 *
 * @param[in] dsp  the DSP instance
 * @return         a read-only instance of the system memory layout
 */
const DSP_MEMORY_LAYOUT *DSP_getMemoryLayout(DSP *dsp);


/**
 * Return a pointer to a DSP_DRAM_MEMORY_AREA describing the overlay memory that
 * got allocated in DSP_init. If it's not big enough for \p size bytes, raise a
 * fatal error. As a result, this function never returns on failure.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] size  requested size
 * @return          a memory area descriptor
 */
const DSP_DRAM_MEMORY_AREA *DSP_getOverlayDRAM(DSP *dsp, size_t size);


/**
 * Return a pointer to a DSP_DRAM_MEMORY_AREA describing the VOM memory that got
 * allocated in DSP_init. If it's not big enough for \p size bytes, raise a
 * fatal error. As a result, this function never returns on failure.
 *
 * @param[in] dsp  the DSP instance
 * @param[in] size requested size
 * @return         a memory area descriptor
 */
const DSP_DRAM_MEMORY_AREA *DSP_getVomDRAM(DSP *dsp, size_t size);


/**
 * Translate one address from the address space visible from the DSP to the one
 * offered by the interface used by the Host to communicate with the system.
 *
 * @param[in] dsp  the DSP instance
 * @param[in] addr the address to be translated (DSP address space)
 * @return         the translated address (system address space)
 */
uint32_t DSP_addrDsp2System(DSP *dsp, DSP_ADDR addr);


/**
 * Translate one address from the address space offered by the interface used
 * by the Host to communicate with the system to the one visible from the DSP.
 *
 * @param[in] dsp   the DSP instance
 * @param[in] addr  the address to be translated (system address space)
 * @return          the translated address (DSP address space)
 */
DSP_ADDR DSP_addrSystem2Dsp(DSP *dsp, uint32_t addr);


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
DSP_RET DSP_writeSystemData(DSP *dsp, uint32_t dest, const void *src, size_t n);


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
DSP_RET DSP_readSystemData(DSP *dsp, void *dest, uint32_t src, size_t n);


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
DSP_RET DSP_writeDspData(DSP *dsp, DSP_ADDR dest, const void *src, size_t n);


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
DSP_RET DSP_readDspData(DSP *dsp, void *dest, DSP_ADDR src, size_t n);


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
DSP_RET DSP_writeSharedData(DSP *dsp, uint32_t dest, void *src, size_t n);


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
DSP_RET DSP_readSharedData(DSP *dsp, void *dest, uint32_t src, size_t n);


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
void DSP_writeSharedRegister(DSP *dsp, uint32_t reg_addr, uint32_t value);


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
uint32_t DSP_readSharedRegister(DSP *dsp, uint32_t reg_addr);


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
 * @param[in] dest_space  address space of the data destination address
 * @param[in] src         where to copy data from, Host address space
 * @param[in] n           number of bytes to copy
 * @return                one of the #DSP_RET return values
 */
DSP_RET DSP_writeData(DSP *dsp,
                      uint32_t dest,
                      DSP_ADDRESS_SPACE dest_space,
                      void *src,
                      size_t n);


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
DSP_RET DSP_readData(DSP *dsp,
                     void *dest,
                     uint32_t src,
                     DSP_ADDRESS_SPACE src_space,
                     size_t n);


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
DSP_RET DSP_writeDataAsDsp(DSP *dsp, DSP_ADDR dest, void *src, size_t n);


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
DSP_RET DSP_readDataAsDsp(DSP *dsp, void *dest, DSP_ADDR src, size_t n);


/**
 * Reads interrupt bits from the DSP.
 *
 * @param[in]  dsp         the DSP instance
 * @param[out] interrupts  a structure that will be filled with currently active interrupts information
 */
void DSP_pollInterrupts(DSP *dsp, DSP_INTERRUPTS *interrupts);


/**
 * Clear interrupt bits from the DSP.
 *
 * @param[in] dsp         the DSP instance
 * @param[in] interrupts  a structure with informations about which interrupts to reset
 */
void DSP_clearInterrupts(DSP *dsp, DSP_INTERRUPTS *interrupts);


/**
 * Clear all interrupt bits from the DSP.
 *
 * @param[in] dsp  the DSP instance
 */
void DSP_clearAllInterrupts(DSP *dsp);


/**
 * Reads and decodes the Heartbeat word.
 * Note: some chip don't support the Heartbeat service, in which case its
 *       functionality will be roughly emulated.
 *
 * @param[in]  dsp       the DSP instance
 * @param[out] heartbeat the heartbeat structure to be filled in
 */
void DSP_pollHeartbeat(DSP *dsp, DSP_HEARTBEAT *heartbeat);


/**
 * Resets the Heartbeat word. Normally used at reset time before enabling the DSP.
 * Note: some chip don't support the Heartbeat service, in which case its
 *       functionality will be roughly emulated.
 *
 * @param dsp  the DSP instance
 */
void DSP_clearHeartbeat(DSP *dsp);


#ifndef __FP4014_ONWARDS__
/* FIXME: this should really be disabled also for FP2012. Let's keep this for
 *        now until the DBG module sorts out a different communication
 *        abstraction for this machine. */

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
