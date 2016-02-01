/***************************************************************************
 *	   Copyright (c) 2007-2013, Broadcom Corporation
 *	   All Rights Reserved
 *	   Confidential Property of Broadcom Corporation
 *
 *	THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *	AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *	EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Simple virtual memory module, it's used to convert between physical and virtual memory
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BOTF_MEM_H_
#define BOTF_MEM_H_

#ifdef __cplusplus
extern "C"
{
#endif

struct BOTF_Data;

typedef struct botf_mem {
    /* this is virtuall addrees corresponding to the physical address 0, it's used to convert from physicall to virtual (CPU) addresses */
    int32_t base; /* base0 is just a cheap way to convert from address to offset, offset = address - base0 and address = offset + base0 */
    uint32_t addr;/* used for debugging */
    const uint8_t *ptr;/* cached_ptr  - used for debugging */
    const uint8_t *uncached_ptr;/* used for debugging */
    size_t range; /* used for debugging */
    struct BOTF_Data *otf;
} botf_mem;

typedef const struct botf_mem *botf_mem_t;

void botf_mem_init(botf_mem *mem, uint32_t addr, void *ptr, size_t range, struct BOTF_Data *otf);
uint32_t botf_mem_paddr(botf_mem_t mem, const void *ptr);
void *botf_mem_vaddr(botf_mem_t mem, uint32_t addr);
void botf_mem_flush(botf_mem_t mem, const void *ptr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* BOTF_MEM_H_ */

