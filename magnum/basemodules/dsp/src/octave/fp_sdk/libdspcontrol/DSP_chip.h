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
 * @brief DSP module chip-specific functions and data structures.
 */

#ifndef _DSP_CHIP_H_
#define _DSP_CHIP_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"
#include "libsyschip/tbuf_chips.h"
#include "fp_sdk_config.h"

#if !(FEATURE_IS(SW_HOST, RAAGA_MAGNUM) || FEATURE_IS(SW_HOST, RAAGA_ROCKFORD))
#  ifndef __cplusplus
#    include <stdbool.h>
#  endif
#  include <stdint.h>
#else
#  include "bstd.h"
#  if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#    include "bdsp_common_priv.h"
#  endif
#endif
#if defined(MCPHY)
#  include "DMA_Drv.h"
#  if IS_HOST(BM)
#    include "FreeRTOS.h"
#    include "semphr.h"
#  endif
#endif
#if IS_TARGET(Pike_haps) || IS_TARGET(RaagaFP4015_haps)
#  include "umrbus.h"
#endif



#if !defined(_DSP_H_)
#  error "Don't include this header directly, include DSP.h instead"
#endif


#ifdef __cplusplus
extern "C" {
#endif


/* In a multicore SoC, which cores should be brought out of reset by the host? */
#if NUM_CORES == 1
#  define DSP_BOOT_CORES    (DSP_CORES_0)
#elif defined(RAAGA)
#  define DSP_BOOT_CORES    (DSP_CORES_0)
#elif defined(YELLOWSTONE) || defined(SHASTA)
#  define DSP_BOOT_CORES    (DSP_CORES_0)
#else
#  error "Please define which cores must be brought out of reset by the host on this chip"
#endif


#if IS_TARGET(RaagaFP4015_haps)
#  define RAAGA_HAPS_MEMC_ARB_CLIENT_INFO_NUM_ENTRIES   2
#endif


#if defined(RAAGA)
#  if defined(RAAGA) && defined(__FP4015__)
    /* Rev3000 silicon had 16 entries, and Rev3100 has 128. Just make space for
     * 128, and we'll waste space for the entries where A0 is involved, but
     * there's no point optimising for old silicon (or constructing a separate
     * build). */
#    define DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES    128
#  else
#    error "How many ATU entries in this SoC?"
#  endif

/** Local copy of an ATU entry. */
typedef struct
{
    DSP_ADDR virtual_start;     /**< region start virtual address */
    uint64_t physical_start;    /**< region start physical address */
    uint32_t length;            /**< region length */
    bool     enabled;           /**< address translation is enabled for this region */
} DSP_RAAGA_ATU_ENTRY;

/** Local copy of the ATU state. */
typedef struct
{
    /** all entries */
    DSP_RAAGA_ATU_ENTRY entries[DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES];
} DSP_RAAGA_ATU;

/** Support structure for DSP_RAAGA_ATU, optimised for searches. */
typedef struct
{
    /** list of enabled entries */
    const DSP_RAAGA_ATU_ENTRY *entries[DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES];
    /** number of valid entries in the array */
    unsigned entries_count;
} DSP_RAAGA_ATU_INDEX;
#endif


#if defined(RAAGA) && FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
/** One of the Raaga host memory regions */
typedef struct
{
    BMMA_Block_Handle block_handle; /*!< BMMA memory block where this region comes from */
    void             *virt_base;    /*!< block virtual base address */
    uint64_t          phys_base;    /*!< block physical base address */
    size_t            size;         /*!< block size */
} DSP_RAAGA_MEM_REGION;

#  define DSP_RAAGA_MEM_REGIONS_NUM_ENTRIES     4
#endif


#if IS_TARGET(Pike_haps) || IS_TARGET(RaagaFP4015_haps)
/** The (device, bus, address) coordinates describing an HAPS CAPIM */
typedef struct
{
    unsigned device;    /**< device no. */
    unsigned bus;       /**< bus no. */
    unsigned address;   /**< address */
} DSP_HAPS_COORDS;

#  define DSP_HAPS_INVALID_COORD    (~(0U))
#  define DSP_HAPS_INVALID_COORDS   { DSP_HAPS_INVALID_COORD, DSP_HAPS_INVALID_COORD, DSP_HAPS_INVALID_COORD }
#endif


#if IS_TARGET(RaagaFP4015_barebone)
#  define DSP_MISC_BLOCK_INVALID_ADDR   ((SHARED_ADDR) -1)
#endif


typedef struct DSP_PARAMETERS_STRUCT
{
#if IS_HOST(BM)
    char *hostname;
    /** The port on localhost where the SCP server (BM targets) is listening. */
    int port;
#endif

#if defined(RAAGA) && (FEATURE_IS(SW_HOST, RAAGA_MAGNUM) || FEATURE_IS(SW_HOST, RAAGA_ROCKFORD))
    /** Magnum BREG memory mapped registers handle */
    BREG_Handle hReg;
    /** Magnum BMMA heap handle */
    BMMA_Heap_Handle hMem;
#  if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
    /** BMMA buffers, user provided, that libdspcontrol is allowed to access */
    BDSP_P_FwBuffer sMmaBuffer[DSP_RAAGA_MEM_REGIONS_NUM_ENTRIES];
    /** Number of valid entries in sMmaBuffer */
    uint32_t ui32MmaBufferValidEntries;
#  endif
#endif

#if defined(MCPHY)
    /** which subsystem to control */
    DSP_SUBSYS dsp_subsystem;
    /** ban writes to IMEM if the DSP is enabled */
    bool check_dsp_enabled_before_write_imem    : 1;
    /** whether to initialise the mcphy2imem source side DMA; if false,
     * the DSP module assumes that the DMA is already configured with
     * base_address=0, block_size=2048, wr_status_or_skip_rd=no,
     * addr_alignment=4,double_command=true, status_en=true and a valid
     * handle is passed in mcphy2imem_src_dma_handle */
    bool init_mcphy2imem_src_dma                : 1;
    /** whether to initialise the mcphy2imem destination side DMA; if false,
     * the DSP module assumes that the DMA is already configured with
     * base_address=0, block_size=2048, wr_status_or_skip_rd=no,
     * addr_alignment=4,double_command=true, status_en=true and a valid
     * handle is passed in mcphy2imem_dst_dma_handle */
    bool init_mcphy2imem_dst_dma                : 1;
    /** whether to initialise the mcphy2dmem source side DMA; if false,
     * the DSP module assumes that the DMA is already configured with
     * base_address=0, block_size=2048, wr_status_or_skip_rd=no,
     * addr_alignment=4,double_command=true, status_en=true and a valid
     * handle is passed in mcphy2dmem_src_dma_handle */
    bool init_mcphy2dmem_src_dma                : 1;
    /** whether to initialise the mcphy2dmem destination side DMA; if false,
     * the DSP module assumes that the DMA is already configured with
     * base_address=0, block_size=2048, wr_status_or_skip_rd=no,
     * addr_alignment=4,double_command=true, status_en=true and a valid
     * handle is passed in mcphy2dmem_dst_dma_handle */
    bool init_mcphy2dmem_dst_dma                : 1;
    /** whether to initialise the dmem2mcphy source side DMA; if false,
     * the DSP module assumes that the DMA is already configured with
     * base_address=0, block_size=2048, wr_status_or_skip_rd=no,
     * addr_alignment=4,double_command=true, status_en=true and a valid
     * handle is passed in dmem2mcphy_src_dma_handle */
    bool init_dmem2mcphy_src_dma                : 1;
    /** whether to initialise the dmem2mcphy destination side DMA; if false,
     * the DSP module assumes that the DMA is already configured with
     * base_address=0, block_size=2048, wr_status_or_skip_rd=no,
     * addr_alignment=4,double_command=true, status_en=true and a valid
     * handle is passed in dmem2mcphy_dst_dma_handle */
    bool init_dmem2mcphy_dst_dma                : 1;
    /** mcphy2imem source side DMA driver handle, ignored if init_mcphy2imem_src_dma is true;
     *  set to NULL to disable transfers in this direction */
    DmaHandle_T *mcphy2imem_src_dma_handle;
    /** mcphy2imem destination side DMA driver handle, ignored if init_mcphy2imem_dst_dma is true;
     *  set to NULL to disable transfers in this direction */
    DmaHandle_T *mcphy2imem_dst_dma_handle;
    /** mcphy2dmem source side DMA driver handle, ignored if init_mcphy2dmem_src_dma is true;
     *  set to NULL to disable transfers in this direction */
    DmaHandle_T *mcphy2dmem_src_dma_handle;
    /** mcphy2dmem destination side DMA driver handle, ignored if init_mcphy2dmem_dst_dma is true;
     *  set to NULL to disable transfers in this direction */
    DmaHandle_T *mcphy2dmem_dst_dma_handle;
    /** dmem2mcphy source side DMA driver handle, ignored if init_dmem2mcphy_src_dma is true;
     *  set to NULL to disable transfers in this direction */
    DmaHandle_T *dmem2mcphy_src_dma_handle;
    /** dmem2mcphy destination side DMA driver handle, ignored if init_dmem2mcphy_dst_dma is true;
     *  set to NULL to disable transfers in this direction */
    DmaHandle_T *dmem2mcphy_dst_dma_handle;
#endif

#if IS_TARGET(Pike_haps) || IS_TARGET(RaagaFP4015_haps)
    /** CAPIM to drive the 'data' bus */
    DSP_HAPS_COORDS data_capim;
    /** CAPIM to drive the 'control' bus */
    DSP_HAPS_COORDS control_capim;
    /** Reset the UMRBUS bridge when calling DSP_init */
    bool reset_bridge_on_init;
    /** Reset the UMRBUS bridge if a transaction ends with an error */
    bool reset_bridge_on_error;
    /** Reset the UMRBUS bridge after the end of every transaction */
    bool reset_bridge_on_transaction;
#endif
#if IS_TARGET(RaagaFP4015_haps)
    /** Reset the design when calling DSP_init */
    bool reset_design_on_init;
    /** Init-time configuration for the MEMC_ARB_CLIENT_INFO_i registers */
    struct
    {
        bool     valid;               /*!< if this entry contains valid data */
        uint32_t pr_tag : 8;
        uint32_t bo_val : 18;
        uint32_t rr_en  : 1;
    } memc_arb_client_info[RAAGA_HAPS_MEMC_ARB_CLIENT_INFO_NUM_ENTRIES];
    /** Virtual address of the first byte of the secure region. */
    uint32_t secure_region_start;
    /** Virtual address of the first byte after the end of the secure region. */
    uint32_t secure_region_end;
#endif

#if IS_TARGET(RaagaFP4015_barebone)
    /** At which address the Misc Block of core 0 is reachable from the host */
    SHARED_ADDR misc_block_addr_fp0;
    /** At which address the Misc Block of core 1 is reachable from the host */
    SHARED_ADDR misc_block_addr_fp1;
#endif
} DSP_PARAMETERS;


typedef struct DSP_STRUCT
{
#if defined(MCPHY)
    /** Local copy of the user-provided DSP_PARAMETERS */
    DSP_PARAMETERS parameters;
#endif
#if defined(__FP2012__)
    /** On FP2012 the debug console is emulated by a software circular buffer.
     *  We have to keep track if the structure has been initialised. */
    bool debug_console_initialised;
#endif
#if defined(RAAGA) && (FEATURE_IS(SW_HOST, RAAGA_MAGNUM) || FEATURE_IS(SW_HOST, RAAGA_ROCKFORD))
    /** Magnum BREG memory mapped registers handle */
    BREG_Handle reg;
    /** Magnum BMMA heap handle */
    BMMA_Heap_Handle heap;
#  if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
    /** BMMA memory regions, user provided, that libdspcontrol is allowed to access */
    DSP_RAAGA_MEM_REGION mem_regions[DSP_RAAGA_MEM_REGIONS_NUM_ENTRIES];
    /** Number of valid entries in mem_regions */
    unsigned mem_regions_count;
#  endif
#endif
#if defined(MCPHY) && IS_HOST(BM)
    /** This mutex will be used to protect the SCP request/response socket I/O.
     *  FreeRTOS mutex API should't be called from interrupts, but we don't care
     *  as we are in a simulated world on RHEL, not on silicon. */
    xSemaphoreHandle scp_mutex;
#endif
#if IS_TARGET(Pike_haps) || IS_TARGET(RaagaFP4015_haps)
    UMR_HANDLE data_capim;
    UMR_HANDLE control_capim;
    bool reset_bridge_on_init;
    bool reset_bridge_on_error;
    bool reset_bridge_on_transaction;
#endif
#if IS_TARGET(RaagaFP4015_haps)
    bool reset_design_on_init;
    /** Init-time configuration for the MEMC_ARB_CLIENT_INFO_i registers. We have to keep
     * this around as we might initialise the MEMC more than once (e.g. in DSP_reset). */
    struct
    {
        bool     valid;
        uint32_t pr_tag : 8;
        uint32_t bo_val : 18;
        uint32_t rr_en  : 1;
    } memc_arb_client_info[RAAGA_HAPS_MEMC_ARB_CLIENT_INFO_NUM_ENTRIES];
    /** Virtual address of the first byte of the secure region. */
    uint32_t secure_region_start;
    /** Virtual address of the first byte after the end of the secure region. */
    uint32_t secure_region_end;
#endif
#if IS_TARGET(RaagaFP4015_si_magnum) || IS_TARGET(RaagaFP4015_bm) || IS_TARGET(RaagaFP4015_haps) || IS_TARGET(RaagaFP4015_haps_bm) || IS_TARGET(RaagaFP4015_si_magnum_permissive)
    /** Revision of ATU hardware */
    uint16_t atu_hw_revision;
    /** Cache of HW ATU entries. */
    DSP_RAAGA_ATU atu_cache;
    /** Index of local copy of ATU entries. */
    DSP_RAAGA_ATU_INDEX atu_index;
#endif
#if IS_TARGET(RaagaFP4015_barebone)
    /** At which address the Misc Block of core 0 is reachable from the host */
    SHARED_ADDR misc_block_addr_fp0;
    /** At which address the Misc Block of core 1 is reachable from the host */
    SHARED_ADDR misc_block_addr_fp1;
#endif
#if IS_TARGET(YellowstoneA0_si)
    /** Pointer to C++ object in HostManager */
    void *proxy;
#endif
#if IS_HOST(BM)
    /** Dummy entry to avoid "struct has no members" warnings */
    void *dummy;
#endif
#if defined (TB_SIZE_SET_BY_HOST)
    uint32_t tb_buf_len;
#endif
} DSP;


typedef struct
{
#if defined(__FP4014_ONWARDS__) || defined(__FPM1015__)
    uint32_t host_intc_host_irq;
#if defined(__FP4014_ONWARDS__) || defined(__FPM1015__)
    uint32_t obus_fault;
    uint32_t obus_fault_address;
#endif
#endif
} DSP_INTERRUPTS;


typedef enum
{
#if IS_TARGET(RaagaFP4015_si_magnum) || IS_TARGET(RaagaFP4015_bm) || IS_TARGET(RaagaFP4015_haps) || IS_TARGET(RaagaFP4015_haps_bm) || IS_TARGET(RaagaFP4015_si_magnum_permissive)
    /** Pseudo-option that, when set, triggers a refresh of the cached ATU status. */
    DSP_OPTION_REFRESH_ATU_INDEX,
#endif

    /* To ensure the enum has always at least one element */
    DSP_OPTION_DUMMY
} DSP_OPTION;


#ifdef __cplusplus
}
#endif


#endif  /* _DSP_CHIP_H_ */
