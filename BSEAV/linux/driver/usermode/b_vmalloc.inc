/***************************************************************************
 * Copyright (C) 2002-2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
 ***************************************************************************/

static unsigned vmalloc_align(unsigned v, unsigned alignment)
{
    unsigned r;
    r = v + (alignment - 1);
    r -= r%alignment;
    return r;
}

static unsigned vmalloc_size(
    unsigned required_vmalloc, /* vmalloc space required by Nexus to map physical memory  and registers */
    uint64_t highest_installed_dram, /* (highest address of address-contiguous installed DRAM) + 1 */
    unsigned lowest_bmem_addr /* lowest address of all bmem regions */
   )
{
    unsigned vmalloc=0;
    const unsigned _1mb = 1024 * 1024;
#if !defined(CONFIG_64BIT)
    unsigned addrspace_lim;
    unsigned vmalloc_lim;
    const unsigned default_vmalloc = 248 * _1mb;
    const unsigned vmalloc_offset = 8 * _1mb; /* based on VMALLOC_OFFSET */
    const unsigned fudge = 192 * _1mb; /* some address space for core Linux use */

    addrspace_lim = PAGE_OFFSET;
    if(addrspace_lim==0) {
        /* invalid vmsplit */
        goto done;
    }
    addrspace_lim = ~0UL - addrspace_lim;
    required_vmalloc += fudge;
    required_vmalloc += vmalloc_offset;
    if(highest_installed_dram < addrspace_lim) {
        vmalloc_lim = highest_installed_dram - vmalloc_offset;
    } else {
        vmalloc_lim = min((uint64_t)addrspace_lim, highest_installed_dram) - default_vmalloc;
    }
    required_vmalloc = max(default_vmalloc, required_vmalloc);
    if(lowest_bmem_addr > vmalloc_lim) {
        vmalloc = required_vmalloc;
    } else {
        if(addrspace_lim  < (lowest_bmem_addr + vmalloc_offset)) {
            goto done;
        }
        vmalloc = addrspace_lim - (lowest_bmem_addr + vmalloc_offset);
    }
done:
#endif /* CONFIG_64BIT */
    vmalloc = vmalloc_align(vmalloc, _1mb);
    vmalloc /= _1mb;
    return vmalloc;
}
