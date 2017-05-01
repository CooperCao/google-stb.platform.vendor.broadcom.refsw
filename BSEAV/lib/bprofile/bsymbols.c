/***************************************************************************
 * Copyright (C) 2006-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 *
 * Module Description:
 *
 * Embeddable profiler library
 *  symbol module
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
			BDBG_MSG(("self %s(%#lx) table %s(%#x) offset %#x", self, (unsigned long)bsymbol_fixup, b_sym_table[i].name, b_sym_table[i].addr, bsymbol_map.offset));
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
