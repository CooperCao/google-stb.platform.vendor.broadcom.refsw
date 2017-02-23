/****************************************************************************
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

#include "fp_sdk_config.h"

#if !(FEATURE_IS(SW_HOST, RAAGA_MAGNUM) || FEATURE_IS(SW_HOST, RAAGA_ROCKFORD))
#  ifndef __cplusplus
#    include <stdbool.h>
#  endif
#  include <stdint.h>
#else
#  include "bstd.h"
#  include "bdsp_common_priv.h"
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
#elif defined(DUNA)
#  define DSP_BOOT_CORES    (DSP_CORES_0 | DSP_CORES_1)
#elif defined(RAAGA) && defined(__FP4015__)
#  define DSP_BOOT_CORES    (DSP_CORES_0)
#elif defined(YELLOWSTONE)
#  define DSP_BOOT_CORES    (DSP_CORES_0)
#else
#  error "Please define which cores must be brought out of reset by the host on this chip"
#endif


#if defined(DUNA) && IS_HOST(SILICON)
/* Known to work parameters */
#  define DSP_DUNA_DEFAULT_READ_BLOCK_SIZE      64
#  define DSP_DUNA_DEFAULT_WRITE_BLOCK_SIZE     64
#  define DSP_DUNA_DEFAULT_SPI_FREQUENCY        4000000
#endif
#if IS_TARGET(RaagaFP4015_haps)
#  define RAAGA_HAPS_MEMC_ARB_CLIENT_INFO_NUM_ENTRIES   2
#endif


#if defined(CELIVERO) || defined(CELTRIX)

/* Default memory allocation figures */
#  define DSP_CELIVERO_DEFAULT_TRANSFERS_MEM_SIZE               (1024 * 1024)
#  if !defined(TARGET_BUFFER_MUX_SERVICES)
#    define DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_TARGETPRINT        (512 * 1024)
#    define DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_STATPROF           (512 * 1024)
#    define DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_INSTRUMENTATION    (512 * 1024)
#    define DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_COREDUMP           (768 * 1024)
#  else
#    define DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_COMMON             (1024 * 1024)
#    define DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_COREDUMP           (768 * 1024)
#  endif

/* Memory management types definition */
typedef struct
{
    void *vDdr;                     /**< virtual address, NULL by default */
    uint32_t pDdr_raw;              /**< physical address in low-mem, NULL by default */
    uint32_t pDdr_cached;           /**< cached access to the physical address, NULL by default */
    uint32_t pDdr_uncached;         /**< uncached access to the physical address, NULL by default */
    uint32_t cDdr;                  /**< block size, 0 by default */
} DSP_CELIVERO_MEMORY_BLOCK;

typedef struct
{
    bool initialised;                       /**< false if not initialised, true after the call to CELIVERO_allocateMemory */
    DSP_CELIVERO_MEMORY_BLOCK overlays;     /**< memory assigned to the overlay system */
    DSP_CELIVERO_MEMORY_BLOCK vom;          /**< memory assigned to VOM code */
    DSP_CELIVERO_MEMORY_BLOCK mem_transfers;/**< memory assigned to the CELIVERO_read/write_*mem functions */
    DSP_CELIVERO_MEMORY_BLOCK services;     /**< memory assigned to services TBs */
#  if !defined(TARGET_BUFFER_MUX_SERVICES)
    unsigned tb_targetprint_size;           /**< size assigned to the Target Print TB */
    unsigned tb_statprof_size;              /**< size assigned to the Statistical Profiling TB */
    unsigned tb_instrumentation_size;       /**< size assigned to the Instrumentation TB */
#  else
    unsigned tb_common_size;                /**< size assigned to the Common TB */
#  endif
    unsigned tb_coredump_size;              /**< size assigned to the Core Dump TB */
} DSP_CELIVERO_MEMORY_LAYOUT;

#endif


#if defined(RAAGA) && defined(__FP4015_ONWARDS__)
#  if defined(RAAGA) && defined(__FP4015__)
#    define DSP_RAAGA_ATU_CONFIG_NUM_ENTRIES    16
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
    DSP_RAAGA_ATU_ENTRY entries[DSP_RAAGA_ATU_CONFIG_NUM_ENTRIES];
} DSP_RAAGA_ATU;

/** Support structure for DSP_RAAGA_ATU, optimised for searches. */
typedef struct
{
    /** list of enabled entries */
    const DSP_RAAGA_ATU_ENTRY *entries[DSP_RAAGA_ATU_CONFIG_NUM_ENTRIES];
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
    uint32_t          phys_base;    /*!< block physical base address */
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


typedef struct
{
#if IS_HOST(BM)
    /** The port on localhost where the SCP server (BM targets) is listening. */
    int port;
#endif

#if defined(CELIVERO) || defined(CELTRIX)
#  if IS_HOST(SILICON)
    /** call brcm_mmap_shared_flush before each brcm_read_physical invocation */
    bool     flush_cache_before_read     : 1;
    /** call brcm_mmap_shared_flush after each brcm_write_physical invocation */
    bool     flush_cache_after_write     : 1;
#  endif
    /** true if memory used for Target Buffers and data transfers (code loading) should overlap */
    bool     mem_transfer_mem_overlap_tb : 1;
    /** true if memory should be allocated from CPUH/CPUL shared area of memory (brcm_mmap_ipc_*) */
    bool     mem_use_shared_dram         : 1;
    /** memory assigned to the CELIVERO_read/write_*mem functions, used for example for code loading */
    unsigned mem_transfers_size;
    /** memory assigned to the overlay system */
    unsigned mem_overlays_size;
    /** memory assigned to VOM code */
    unsigned mem_vom_size;
#  if !defined(TARGET_BUFFER_MUX_SERVICES)
    /** size assigned to the Target Print TB shared buffer */
    unsigned mem_tb_targetprint_size;
    /** size assigned to the Statistical Profiling TB shared buffer */
    unsigned mem_tb_statprof_size;
    /** size assigned to the Instrumentation TB shared buffer */
    unsigned mem_tb_instrumentation_size;
#  else
    /** size assigned to the Common TB shared buffer */
    unsigned mem_tb_common_size;
#  endif
    /** size assigned to the Core Dump TB shared buffer */
    unsigned mem_tb_coredump_size;
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

#if defined(DUNA) && IS_HOST(SILICON)
    /** Memory will be read in blocks this big from the SPI interface */
    uint32_t read_block_size;
    /** Memory will be written in blocks this big from the SPI interface */
    uint32_t write_block_size;
    /** Frequency to run the SPI interface at */
    uint32_t spi_frequency;
    /** Name of the SPI dongle to use - NULL to use the first one */
    char *dongle_id;
    /** Whether to read back and verify each write */
    bool write_verify;
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
    uint32_t misc_block_addr_fp0;
    /** At which address the Misc Block of core 1 is reachable from the host */
    uint32_t misc_block_addr_fp1;
#endif
} DSP_PARAMETERS;


typedef struct
{
#if (defined(CELIVERO) || defined(CELTRIX) || defined(MCPHY) || \
     (defined(DUNA) && IS_HOST(SILICON)))
    /** Local copy of the user-provided DSP_PARAMETERS */
    DSP_PARAMETERS parameters;
#endif
#if defined(__FP2012__)
    /** On FP2012 the debug console is emulated by a software circular buffer.
     *  We have to keep track if the structure has been initialised. */
    bool debug_console_initialised;
#endif
#if defined(CELIVERO) || defined(CELTRIX)
    /** Layout of the allocated DRAM, in virtual and physical forms. */
    DSP_CELIVERO_MEMORY_LAYOUT memory_layout;
#endif
#if defined(DUNA) && IS_HOST(SILICON)
    /** Actually a "Tspi *", but we can't declare a C++ class
     *  pointer in a C context. */
    void *spi;
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
#if IS_TARGET(RaagaFP4015_si_magnum) || IS_TARGET(RaagaFP4015_bm) || IS_TARGET(RaagaFP4015_haps_bm)
    /** Cache of HW ATU entries. */
    DSP_RAAGA_ATU atu_cache;
    /** Index of local copy of ATU entries. */
    DSP_RAAGA_ATU_INDEX atu_index;
#endif
#if IS_TARGET(RaagaFP4015_barebone)
    /** At which address the Misc Block of core 0 is reachable from the host */
    uint32_t misc_block_addr_fp0;
    /** At which address the Misc Block of core 1 is reachable from the host */
    uint32_t misc_block_addr_fp1;
#endif
#if IS_TARGET(YellowstoneA0_si)
    /** Pointer to C++ object in HostManager */
    void *proxy;
#endif
#if IS_HOST(BM)
    /** Dummy entry to avoid "struct has no members" warnings */
    void *dummy;
#endif
} DSP;


typedef struct
{
#if defined(RAAGA) && !defined(__FP4015_ONWARDS__)
    uint32_t dsp_inth_host_status;
    uint32_t dsp_fw_inth_host_status;
    uint32_t dsp_mem_subsystem_memsub_error_status;
#endif
#if defined(__FP4014_ONWARDS__) || defined(__FPM1015__)
    uint32_t host_intc_host_irq;
#if defined(__FP4015_ONWARDS__) || defined(__FPM1015__)
    uint32_t obus_fault;
    uint32_t obus_fault_address;
#endif
#endif
} DSP_INTERRUPTS;


typedef enum
{
#if IS_TARGET(RaagaFP4015_si_magnum) || IS_TARGET(RaagaFP4015_bm) || IS_TARGET(RaagaFP4015_haps_bm)
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
