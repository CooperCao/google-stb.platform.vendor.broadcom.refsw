/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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
 * BMedia library, incremental index interface
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmedia_index.h"
#include "bkni.h"

BDBG_MODULE(bmedia_index);

BDBG_OBJECT_ID(bmedia_index_t);

struct bmedia_index {
	BDBG_OBJECT(bmedia_index_t)
	bmedia_player_pos duration;
	size_t entry_size;
	size_t nentries;
	size_t last_entry;
	bmedia_player_pos next;
	uint8_t buf[1]; /* veriable size index array */
};

bmedia_index_t
bmedia_index_create(bmedia_player_pos duration, size_t entry_size, size_t nentries)
{
	bmedia_index_t index;

	BDBG_ASSERT(nentries>0);
	BDBG_ASSERT(entry_size>0);

	index = BKNI_Malloc(sizeof(*index)+(entry_size*nentries));
	BDBG_MSG(("bmedia_index_create: %#lx duration:%u entry_size:%u nentries:%u", (unsigned long)index, (unsigned)duration, (unsigned)entry_size, (unsigned)nentries));
	if(index==NULL) {
		BDBG_ERR(("bmedia_indexer_create: can't allocated %u bytes", (unsigned)sizeof(*index)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(index, bmedia_index_t);

	index->duration = duration?duration:1; /* lower bound duration */

	index->entry_size = entry_size;
	index->nentries = nentries;
	index->last_entry = 0;
	index->next = 0;
	return index;
err_alloc:
	return NULL;
}

void
bmedia_index_destroy(bmedia_index_t index)
{
	BDBG_OBJECT_ASSERT(index, bmedia_index_t);
	BDBG_OBJECT_DESTROY(index, bmedia_index_t);
	BKNI_Free(index);
	return;
}

bmedia_player_pos
bmedia_index_next(bmedia_index_t index)
{
	BDBG_OBJECT_ASSERT(index, bmedia_index_t);
	return index->next;
}


void
bmedia_index_add(bmedia_index_t index, bmedia_player_pos time, const void *sample)
{
	BDBG_OBJECT_ASSERT(index, bmedia_index_t);
	BDBG_ASSERT(sample);
	BDBG_MSG(("bmedia_index_add: %#lx %u:%u %u", (unsigned long)index, (unsigned)time, (unsigned)index->duration, (unsigned)index->last_entry));
	if(time<index->next) {
		BDBG_WRN(("bmedia_index_add: %#lx unexpected sample %u:%u", (unsigned long)index, (unsigned)time, (unsigned)index->next));
		return;
	}
	do {
		if(index->last_entry >= index->nentries) {
			BDBG_WRN(("bmedia_index_add: %#lx too much entries in the cache %u:%u (%u:%u)", (unsigned long)index, (unsigned)index->last_entry, (unsigned)index->nentries, (unsigned)time, (unsigned)index->duration));
			index->next = BMEDIA_PLAYER_INVALID;
			break;
		}
		BKNI_Memcpy(index->buf+index->last_entry*index->entry_size, sample, index->entry_size);
		index->last_entry++;
		index->next = (bmedia_player_pos) (((uint64_t)index->last_entry * index->duration) / index->nentries);
		BDBG_MSG(("bmedia_index_add: %#lx next %u:%u",(unsigned long)index, (unsigned)index->last_entry, (unsigned)index->next));
	} while(time>=index->next);
	return;
}

bmedia_player_pos
bmedia_index_get_safe(bmedia_index_t index, bmedia_player_pos time, void *sample, size_t sample_size)
{
	unsigned pos;
	BDBG_OBJECT_ASSERT(index, bmedia_index_t);
	BDBG_ASSERT(sample);

    BDBG_ASSERT(sample_size==index->entry_size);
	if(time>index->duration) {
		BDBG_MSG(("bmedia_index_get: %#lx out of bounds %u>%u", (unsigned long)index, (unsigned)time, (unsigned)index->duration));
		return  BMEDIA_PLAYER_INVALID;
	}
	pos = (bmedia_player_pos) (((uint64_t)time* index->nentries) / index->duration);
	BDBG_MSG(("bmedia_index_get: %#lx pos %u:%u", (unsigned long)index, (unsigned)pos, (unsigned)time));
	BDBG_ASSERT(pos <= index->nentries);
	if(pos >= index->last_entry) {
		if(index->last_entry==0) {
			BDBG_MSG(("bmedia_index_get: %#lx no entries found", (unsigned long)index));
			return  BMEDIA_PLAYER_INVALID;
		}
		pos = index->last_entry-1;
	}
	BKNI_Memcpy(sample, index->buf+pos*index->entry_size, index->entry_size);
	return (bmedia_player_pos) (((uint64_t)pos * index->duration) / index->nentries);
}




