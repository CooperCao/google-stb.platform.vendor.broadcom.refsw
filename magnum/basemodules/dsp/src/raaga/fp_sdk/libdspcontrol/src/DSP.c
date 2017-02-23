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
 * @file DSP.c
 *
 * Implementation of the common DSP module API.
 *
 * Functions in this file are normally included in barebone flavours of
 * libdspcontrol as 'default' implementations, so be careful when adding
 * new code here.
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
#  include "bstd.h"
#  include "bkni.h"
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


void DSP_initParameters(DSP_PARAMETERS *params __attribute__((unused)))
{
    /* You might be tempted to memset the structure to 0 and then set only the
     * fields that you think are not represented as zeros. Please don't do that:
     * all the fields are here explicitly assigned to have a at-a-glance view of
     * what each field is set to (without having to cross check the structure
     * declaration) and an easy way of checking no field has been missed. */

#if IS_HOST(BM)
    params->port = 0;
#endif

#if defined(CELIVERO) || defined(CELTRIX)
#  if IS_HOST(SILICON)
    params->flush_cache_before_read     = true;
    params->flush_cache_after_write     = true;
#  endif
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

#if defined(RAAGA) && (FEATURE_IS(SW_HOST, RAAGA_MAGNUM) || FEATURE_IS(SW_HOST, RAAGA_ROCKFORD))
    params->hReg = NULL;
#  if FEATURE_IS(SW_HOST, RAAGA_ROCKFORD)
    params->hMem = NULL;
#  elif FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
    BKNI_Memset(params->sMmaBuffer, 0, sizeof(params->sMmaBuffer));
    params->ui32MmaBufferValidEntries = 0;
#  endif
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

#if IS_TARGET(Pike_haps) || IS_TARGET(RaagaFP4015_haps)
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

#if IS_TARGET(RaagaFP4015_haps)
    params->reset_design_on_init = true;
    for(unsigned i = 0; i < RAAGA_HAPS_MEMC_ARB_CLIENT_INFO_NUM_ENTRIES; i++)
        params->memc_arb_client_info[i].valid = false;
    params->secure_region_start = 0;
    params->secure_region_end = 0;
#endif

#if IS_TARGET(RaagaFP4015_barebone)
    /* Initialise so that any access will fail unless the user provides proper
     * value (the default NULL has the risk of failing silently). */
    params->misc_block_addr_fp0 = params->misc_block_addr_fp1 = 0xffffffff;
#endif
}
