/***************************************************************************
 *     Copyright (c) 2002-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 * Revision History:
 *
 * $brcm_Log: $
 *
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
    unsigned addrspace_lim;
    unsigned vmalloc_lim;
    unsigned vmalloc;
    const unsigned _1mb = 1024 * 1024;
    const unsigned default_vmalloc = 248 * _1mb;
    const unsigned fudge = 16 * _1mb; /* some address space for ioremap */

#if defined(CONFIG_VMSPLIT_3G)
    addrspace_lim = 0x40000000u;
#elif defined(CONFIG_VMSPLIT_2G)
    addrspace_lim = 0x80000000u;
#elif defined(CONFIG_VMSPLIT_1G)
    addrspace_lim = 0xC0000000u;
#else
    /* invalid vmsplit */
    return 0;
#endif
    required_vmalloc += fudge;
    required_vmalloc += 8 * _1mb;  /* based on VMALLOC_OFFSET */
    if(highest_installed_dram < addrspace_lim) {
        vmalloc_lim = highest_installed_dram - (8  * _1mb);
    } else {
        vmalloc_lim = min((uint64_t)addrspace_lim, highest_installed_dram) - default_vmalloc;
    }
    required_vmalloc = max(default_vmalloc, required_vmalloc);
    if(lowest_bmem_addr > vmalloc_lim) {
        vmalloc = required_vmalloc;
    } else {
        if(addrspace_lim  < (lowest_bmem_addr + fudge)) {
            goto error;
        }
        vmalloc = addrspace_lim - (lowest_bmem_addr + fudge);
    }
    vmalloc = vmalloc_align(vmalloc, _1mb);
    vmalloc /= _1mb;
    return vmalloc;
error:
    return 0;
}
