/****************************************************************************
 *                Copyright (c) 2013 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
 ****************************************************************************/

/**
 * @file DSP.c
 *
 * Implementation of the common DSP module API.
 */

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"
#include "fp_sdk_config.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stdbool.h>
#else
#  include "bstd_defs.h"
#endif

#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"



#if IS_HOST(DSP_LESS)
#  error "This module is not suitable for DSP-less builds"
#endif


/**
 * Copy a block of data into memory, dispatching the call to the address space-specific
 * function (#DSP_writeSystemData, #DSP_writeSharedData or #DSP_writeDspData).
 */
DSP_RET DSP_writeData(DSP *dsp, uint32_t dest, DSP_ADDRESS_SPACE dest_space, void *src, size_t n)
{
    switch(dest_space)
    {
    case DSP_ADDRSPACE_SYSTEM:
        return DSP_writeSystemData(dsp, dest, src, n);

    case DSP_ADDRSPACE_SHARED:
        return DSP_writeSharedData(dsp, dest, src, n);

    case DSP_ADDRSPACE_DSP:
        return DSP_writeDspData(dsp, dest, src, n);

    default:
        FATAL_ERROR("Error: wrong address space passed to function 'DSP_writeData'");
        return DSP_FAILURE;   /* we'll never reach here */
    }
}


/**
 * Copy a block of data from memory, dispatching the call to the address space-specific
 * function (#DSP_readSystemData, #DSP_readSharedData or #DSP_readDspData).
 */
DSP_RET DSP_readData(DSP *dsp, void *dest, uint32_t src, DSP_ADDRESS_SPACE src_space, size_t n)
{
    switch(src_space)
    {
    case DSP_ADDRSPACE_SYSTEM:
        return DSP_readSystemData(dsp, dest, src, n);

    case DSP_ADDRSPACE_SHARED:
        return DSP_readSharedData(dsp, dest, src, n);

    case DSP_ADDRSPACE_DSP:
        return DSP_readDspData(dsp, dest, src, n);

    default:
        FATAL_ERROR("Error: wrong address space passed to function 'DSP_readData'");
        return DSP_FAILURE;   /* we'll never reach here */
    }
}


void DSP_initParameters(DSP_PARAMETERS *params)
{
    /* You might be tempted to memset the structure to 0 and then set only the
     * fields that you think are not represented as zeros. Please don't do that:
     * all the fields are here explicitly assigned to have a at-a-glance view of
     * what each field is set to (without having to cross check the structure
     * declaration) and an easy way of checking no field has been missed. */

#if IS_HOST(BM)
    params->port = 0;
#endif

#if defined(CELIVERO) && IS_HOST(SILICON)
    params->flush_cache_before_read     = true;
    params->flush_cache_after_write     = true;
    params->mem_transfer_mem_overlap_tb = false;
    params->mem_use_shared_dram         = false;
    params->mem_transfers_size          = DSP_CELIVERO_DEFAULT_TRANSFERS_MEM_SIZE;
    params->mem_overlays_size           = 0;
    params->mem_vom_size                = 0;
#  if !defined(TARGET_BUFFER_MUX_SERVICES)
    params->mem_tb_targetprint_size     = DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_TARGETPRINT;
    params->mem_tb_statprof_size        = DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_STATPROF;
    params->mem_tb_instrumentation_size = DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_INSTRUMENTATION;
    params->mem_tb_coredump_size        = DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_COREDUMP;
#  else
    params->mem_tb_common_size          = DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_COMMON;
    params->mem_tb_coredump_size        = DSP_CELIVERO_DEFAULT_TB_MEM_SIZE_COREDUMP;
#  endif
#endif

#if defined(RAAGA) && IS_HOST(SILICON)
    params->hReg = NULL;
    params->hMem = NULL;
#endif

#if defined(DUNA) && IS_HOST(SILICON)
    params->read_block_size  = DSP_DUNA_DEFAULT_READ_BLOCK_SIZE;
    params->write_block_size = DSP_DUNA_DEFAULT_WRITE_BLOCK_SIZE;
    params->spi_frequency    = DSP_DUNA_DEFAULT_SPI_FREQUENCY;
    params->dongle_id        = NULL;
    params->write_verify     = false;
#endif

#if defined(MCPHY)
    params->dsp_subsystem                       = DSP_SUBSYS_0;
    params->check_dsp_enabled_before_write_imem = true;
    params->init_mcphy2imem_src_dma             = true;
    params->init_mcphy2imem_dst_dma             = true;
    params->init_mcphy2dmem_src_dma             = true;
    params->init_mcphy2dmem_dst_dma             = true;
    params->init_dmem2mcphy_src_dma             = true;
    params->init_dmem2mcphy_dst_dma             = true;
    params->mcphy2imem_src_dma_handle           = NULL;
    params->mcphy2imem_dst_dma_handle           = NULL;
    params->mcphy2dmem_src_dma_handle           = NULL;
    params->mcphy2dmem_dst_dma_handle           = NULL;
    params->dmem2mcphy_src_dma_handle           = NULL;
    params->dmem2mcphy_dst_dma_handle           = NULL;
#endif

#if IS_TARGET(Pike_haps)
    params->data_capim.device           = DSP_HAPS_INVALID_COORD;
    params->data_capim.bus              = DSP_HAPS_INVALID_COORD;
    params->data_capim.address          = DSP_HAPS_INVALID_COORD;
    params->control_capim.device        = DSP_HAPS_INVALID_COORD;
    params->control_capim.bus           = DSP_HAPS_INVALID_COORD;
    params->control_capim.address       = DSP_HAPS_INVALID_COORD;
    params->reset_bridge_on_init        = true;
    params->reset_bridge_on_error       = true;
    params->reset_bridge_on_transaction = false;
#endif
}
