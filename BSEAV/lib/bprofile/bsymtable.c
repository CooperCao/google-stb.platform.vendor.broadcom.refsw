/***************************************************************************
 *     Copyright (c) 2006-2008, Broadcom Corporation
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
 *
 * Embeddeble profiler library
 * 		Symbol lookup module
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bsymtable.h"
BDBG_MODULE(bsymtable);

static B_BIN_SEARCH(b_symbol_lookup, bsymbol_entry, addr)

const char *
bsymbol_lookup(const bsymbol_table *map, unsigned long addr_, unsigned *offset)
{
	unsigned long addr = addr_ - map->offset;
	int pos = b_symbol_lookup(map->table, map->nentries, addr);
	BDBG_MSG(("pos=%d, addr=%#x(%#x) entries=%u", pos, addr, addr_, map->nentries));
	if (pos>=0) {
		*offset = 0;
		return map->table[pos].name;
	} 
	pos = -(pos+1);
	pos --;
	if (pos < 0 || pos>=(int)map->nentries) {
		*offset = (unsigned)-1;
		return "";
	}
	/* BKNI_Printf("%d: %#lx %#lx %#lx\n", pos, addr, map->table[pos].addr, map->table[pos+1].addr); */
	*offset = addr - map->table[pos].addr;
	return map->table[pos].name;
}

