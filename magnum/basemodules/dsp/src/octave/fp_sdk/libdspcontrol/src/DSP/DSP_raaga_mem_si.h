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

#ifndef _DSP_RAAGA_MEM_SI_H_
#define _DSP_RAAGA_MEM_SI_H_

#include "libdspcontrol/CHIP.h"
#include "fp_sdk_config.h"
#include "libdspcontrol/DSP.h"



/**
 * Configure memory management related entries in the DSP instance from the
 * provided parameters. Notably, it caches the memory regions provided by
 * the user for future usage by DSP_raagaFindExistingRegionByPhysical.
 *
 * @param dsp[out]       the DSP instance
 * @param parameters[in] user provided parameters
 */
DSP_RET DSP_raagaMemInit(DSP *dsp, DSP_PARAMETERS *parameters);


/**
 * Find, among the ones provided to DSP_raagaMemInit, the DSP_RAAGA_MEM_REGION
 * that contains all or a left subinterval of [phys_addr, phys_addr+length[.
 * The returned DSP_RAAGA_MEM_REGION will have phys_base equal to the input
 * phys_addr, virt_base the associated virtual address and size the maximum
 * [phys_addr, phys_addr+size[ (or, equivalently, [virt_addr, virt_addr+size[)
 * range accessible in the found region.
 *
 * @param dsp[in]        the DSP instance
 * @param found[out]     memory range associated with the provided (phys_addr, length) pair
 * @param phys_addr[in]  physical address to look for
 * @param length[in]     original extension of the range to look for
 * @return DSP_FAILURE if no associated memory region could be found, DSP_SUCCESS otherwise
 */
DSP_RET DSP_raagaFindExistingRegionByPhysical(DSP *dsp, DSP_RAAGA_MEM_REGION *found, uint64_t phys_addr, size_t length);


/**
 * Allocate a region of memory in DRAM.
 *
 * @param dsp[in]        the DSP instance
 * @param region[out]    details of the allocated memory
 * @param size[in]       requested region size
 * @param alignment[in]  requested region alignment
 * @return DSP_FAILURE if the allocation failed, DSP_SUCCESS otherwise
 */
DSP_RET DSP_raagaMemAlloc(DSP *dsp, DSP_RAAGA_MEM_REGION *region, size_t size, unsigned alignment);

/**
 * Free a region of memory previously allocated with DSP_raagaMemAlloc.
 *
 * @param dsp[in]     the DSP instance
 * @param region[in]  the region to free
 * @return DSP_SUCCESS
 */
DSP_RET DSP_raagaMemFree(DSP *dsp, DSP_RAAGA_MEM_REGION *region);


#endif  /* _DSP_RAAGA_MEM_SI_H_ */
