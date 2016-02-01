/***************************************************************************
 *     Copyright (c) 2006, Broadcom Corporation
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
 * 		symbol module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bsymtable.h"

BDBG_MODULE(bsymtable);



static const bsymbol_entry b_sym_table[] = {
#define B_SYM(addr,name) {addr,#name},
#include "bsymbols.inc"
	{0,""}
};

static bsymbol_table bsymbol_map = {
	b_sym_table,
	(sizeof(b_sym_table)/sizeof(*b_sym_table))-1,
	0
};


static int 
b_sym_strcmp(const char *s1, const char *s2)
{
	for(;;) {
		int ch1 = *s1++;
		int diff = ch1 - *s2++;
		if (diff || /* strings are different */
			ch1 == 0 /* end of string reached */
			) {
			return diff; /* return difference */
		} 
	}
}

const bsymbol_table *
bsymbol_fixup(void)
{
	unsigned i;
	const unsigned size = (sizeof(b_sym_table)/sizeof(*b_sym_table))-1;
	static const char self[] = "bsymbol_fixup";

	BDBG_MSG(("size = %u", size));
	BDBG_ASSERT(size>1); /* shall have more than one entry in the symbol table */
	for(i=0;i<size;i++) {
		if(b_sym_strcmp(self, b_sym_table[i].name)==0) {
			bsymbol_map.offset = (unsigned long)bsymbol_fixup - b_sym_table[i].addr;
			BDBG_MSG(("self %s(%#lx) table %s(%#x) offset %#lx", self, (unsigned long)bsymbol_fixup, b_sym_table[i].name, b_sym_table[i].addr, bsymbol_map.offset));
#if 0
			if (offset>0) {
				/* move all entries to discovered offset */
				for(i=0;i<size;i++) {
					b_sym_table[i].addr += offset;
				}
				BDBG_ASSERT(b_sym_table[0].addr < b_sym_table[size-1].addr);
			}
#endif
			return &bsymbol_map;
		}
	}
	/* no self entry was found */
	BDBG_ASSERT(0);
	return &bsymbol_map;
}
