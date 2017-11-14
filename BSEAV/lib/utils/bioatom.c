/***************************************************************************
 *  Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 *******************************************************************************/
#include "bstd.h"
#include "balloc.h"
#include "blst_squeue.h"
#include "blst_list.h"
#include "bpool.h"
#include "barena.h"
#include "bioatom.h"
#include "bkni.h"

BDBG_MODULE(batom);

#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */
#define BDBG_MSG_FLOW(x) /* BDBG_MSG(x) */


#define B_ATOM_ALLOC_POOL	0xA0
#define B_ATOM_ALLOC_ARENA	0xA1
#define B_ATOM_ALLOC_ALLOC 	0xA2

#define B_ATOM_TYPE_ARRAY	0
#define B_ATOM_TYPE_CLONE	1
#define B_ATOM_TYPE_INVALID  0xBF

#define B_ATOM_TRUNCATE_SIZE(size) ((size) & 0x7FFFFFFFul)


struct batom {
	/* members are arranged in a way that they would be packed where appropriate */
	BLST_SQ_ENTRY(batom) pipe_link; /* link for the pipe */
	uint16_t ref_cnt; /* number of references to array */
	uint8_t atom_type; /* B_ATOM_TYPE_XXX */
	uint8_t alloc_type; /* kind of allocator used to create the atom */
	union {
		struct {
			uint16_t count; /* number of entries in the array */
			size_t length;  /* total length of data */
			/* variable length array 
			*  batom_vec vec[nvec];
			**/
			/* variable length array 
			*  batom *owner[nvec];
			*                        */
		} array;
		struct {
			batom_t parent; /* pointer to the parent atom */
			batom_t master; /* pointer to the last atom in the chain */
		} clone;
	} u;
	const batom_user  *ops;       
	batom_factory_t factory;
	/* variable length array 
	*     uint8_t udata[user_data];
	*/
};

BDBG_OBJECT_ID(batom_accum_t);

#define B_ATOM_USER_PTR(atom) (void *)((uint8_t *)(atom)+ sizeof(*(atom))+((atom)->atom_type==B_ATOM_TYPE_ARRAY?((unsigned)(atom)->u.array.count)*(sizeof(batom_vec)+sizeof(batom_t)):0))
#define B_ATOM_VEC_PTR_ARRAY(atom)  (batom_vec*)((uint8_t *)(atom)+sizeof(*(atom)))
/* #define B_ATOM_VEC_PTR(atom) ((atom)->atom_type==B_ATOM_TYPE_ARRAY?B_ATOM_VEC_PTR_ARRAY(atom):B_ATOM_VEC_PTR_ARRAY((atom)->u.clone.master)) */
#define B_ATOM_PTR_ARRAY(atom) (batom_t*)((uint8_t *)(atom)+sizeof(*(atom))+((unsigned)(atom)->u.array.count)*(sizeof(batom_vec)))
#define B_ATOM_MASTER(atom) ((atom)->atom_type==B_ATOM_TYPE_ARRAY?atom:(atom)->u.clone.master)

#define B_ATOM_INIT_VEC 2
#define B_ATOM_USER_SIZE (3*sizeof(unsigned))
#define B_ATOM_INIT_SIZE	(sizeof(struct batom)+B_ATOM_INIT_VEC*(sizeof(batom_vec)+sizeof(batom_t))+B_ATOM_USER_SIZE)


#define B_ATOM_ACC_INIT_VEC	16
struct batom_accum {
	batom_factory_t factory;
	uint16_t nvecs; /* number of allocated entries */   
	uint16_t count; /* number of entries in the array */
	size_t length;  /* total length of data accumulated */
	batom_vec *vec; /* pointer to the vector table */
	batom_t  *owner; /* pointer to the owner table */
	BDBG_OBJECT(batom_accum_t)
	/* inline vecs */
	/* inline owner */
};

BDBG_OBJECT_ID(batom_factory_t);

struct batom_factory {
	bpool_t pool;
	barena_t arena;
	balloc_iface_t alloc; 
	batom_factory_stats stats;
	BDBG_OBJECT(batom_factory_t)
};

BDBG_OBJECT_ID(batom_pipe_t);
struct batom_pipe {
	BLST_SQ_HEAD(b_atom_pipe_t, batom) atoms;
	batom_factory_t factory;
	unsigned push_cnt;
	unsigned pop_cnt;
	BDBG_OBJECT(batom_pipe_t)
};

batom_factory_t 
batom_factory_create(balloc_iface_t alloc, size_t natoms)
{
	batom_factory_t factory;

	factory = alloc->bmem_alloc(alloc, sizeof(*factory));
	BDBG_MSG_TRACE(("batom_factory_create: %#lx ATOM_INIT_SIZE %u", (unsigned long)factory, B_ATOM_INIT_SIZE));
	if (!factory) {
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(factory, batom_factory_t);
	factory->alloc = alloc;
	factory->pool = bpool_create(alloc, (3*natoms)/4, B_ATOM_INIT_SIZE ); /* 75% of total atoms shall be allocatable from the pool */
	if (!factory->pool) {
		goto err_pool;
	}
	factory->arena = barena_create(alloc, B_ATOM_INIT_SIZE * (natoms/4)); /* 25% of total atoms shall be allocated from the arena */
	if (!factory->arena) {
		goto err_arena;
	}
	BKNI_Memset(&factory->stats, 0 , sizeof(factory->stats));
	return factory;

err_arena:
	bpool_destroy(factory->pool);
err_pool:
	BDBG_OBJECT_DESTROY(factory, batom_factory_t);
	alloc->bmem_free(alloc, factory);
err_alloc:
	return NULL;
}

void
batom_factory_destroy(batom_factory_t factory)
{
	balloc_iface_t alloc;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	alloc = factory->alloc;
	barena_destroy(factory->arena);
	bpool_destroy(factory->pool);
	BDBG_OBJECT_DESTROY(factory, batom_factory_t);
	alloc->bmem_free(alloc, factory);
	return;

}

void
batom_factory_get_stats(batom_factory_t factory, batom_factory_stats *stats)
{
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	*stats = factory->stats;
	bpool_get_status(factory->pool, &stats->pool_status);
	return;
}

void
batom_factory_dump(batom_factory_t factory)
{
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	BDBG_WRN(("batom_factory_dump: atoms[live:%u allocated:%u freed:%u] alloc[pool:%u/%u/%u arena:%u/%u/%u alloc:%u/%u]", factory->stats.atom_live, factory->stats.atom_allocated, factory->stats.atom_freed, factory->stats.alloc_pool, factory->stats.free_pool, factory->stats.full_pool, factory->stats.alloc_arena, factory->stats.free_arena, factory->stats.full_arena, factory->stats.alloc_alloc, factory->stats.free_alloc));
	bpool_dump(factory->pool);
	barena_dump(factory->arena);
	return;
}

static const batom_user b_atom_default_ops = {
	NULL,
	0
};


static void
b_io_accum_init(batom_accum_t acc)
{
	acc->nvecs = B_ATOM_ACC_INIT_VEC;
	acc->vec = (batom_vec *)((uint8_t *)acc + sizeof(*acc));
	acc->owner = (batom_t*)((uint8_t *)acc->vec + B_ATOM_ACC_INIT_VEC*sizeof(batom_vec));
	return;
}

#if 0
static void
b_atom_lock(batom_t atom)
{
	atom->ref_cnt++;
	BDBG_ASSERT(atom->ref_cnt>0);
}
#else
#define b_atom_lock(atom) do {(atom)->ref_cnt++;BDBG_ASSERT((atom)->ref_cnt>0);}while(0)
#endif

void
batom_lock(batom_t atom)
{
	batom_factory_t factory;
	BDBG_MSG_TRACE(("batom_lock: >%#lx", (unsigned long)atom));
	BDBG_ASSERT(atom);

	factory = atom->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	b_atom_lock(atom);
	BDBG_MSG_TRACE(("batom_lock: <%#lx %u", (unsigned long)atom, atom->ref_cnt));
	return;
}

/* this function callled recursively */
void
batom_release(batom_t atom)
{
	batom_factory_t factory;
	BDBG_ASSERT(atom);
	BDBG_MSG_TRACE(("batom_release: >%#lx", (unsigned long)atom));

	factory = atom->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);

	BDBG_MSG_TRACE(("batom_release: %#lx %u", (unsigned long)atom, atom->ref_cnt));
	BDBG_ASSERT(atom->ref_cnt>0);
	atom->ref_cnt--;
	if (atom->ref_cnt>0) {
		BDBG_MSG_TRACE(("batom_release: <%#lx %u", (unsigned long)atom, atom->ref_cnt));
		return;
	}
	BDBG_MSG_TRACE(("batom_release: %#lx free", (unsigned long)atom));
	BDBG_ASSERT(atom->ops);
	if(atom->ops->user_free) {
		BDBG_MSG_TRACE(("batom_release: %#lx user free+", (unsigned long)atom));
		atom->ops->user_free(atom, B_ATOM_USER_PTR(atom));
		BDBG_MSG_TRACE(("batom_release: %#lx user free-", (unsigned long)atom));
		BDBG_ASSERT(atom->ref_cnt==0);/* free shall not increase reference counter */
	}

	switch(atom->atom_type) {
	case B_ATOM_TYPE_CLONE:
		batom_release(atom->u.clone.parent);
		break;
	default:
		BDBG_ASSERT(0);
        break;
	case B_ATOM_TYPE_ARRAY:
		{
		unsigned i;
		batom_t *owners=B_ATOM_PTR_ARRAY(atom);
#if BDBG_DEBUG_BUILD
			batom_vec *vecs = B_ATOM_VEC_PTR_ARRAY(atom);
			for(i=0;i<atom->u.array.count;i++) {
				vecs[i].base = NULL;
			}
#endif
			for(i=0;i<atom->u.array.count;i++) {
				BDBG_MSG_TRACE(("batom_release: %#lx %u:%lx", (unsigned long)atom, i, (unsigned long)owners[i]));
				BDBG_ASSERT(owners[i]);
				if (owners[i]!=atom) {
					batom_release(owners[i]);
				}
			}
		}
	}
	BDBG_ASSERT(atom->ref_cnt==0); /* recursive releases shall not lock this object either */
	BDBG_ASSERT(factory->stats.atom_live>0);
	factory->stats.atom_live--;
	factory->stats.atom_freed++;
	atom->atom_type = B_ATOM_TYPE_INVALID;
	atom->factory = NULL;
	BDBG_ASSERT(BLST_SQ_NEXT(atom, pipe_link) == NULL);
	switch(atom->alloc_type) {
	case B_ATOM_ALLOC_POOL:
		atom->alloc_type = 0;
		factory->stats.free_pool++;
		bpool_free(factory->pool, atom);
		break;
	case B_ATOM_ALLOC_ARENA:
		atom->alloc_type = 0;
		factory->stats.free_arena++;
		barena_free(factory->arena, atom);
		break;
	default:
		BDBG_ASSERT(0);
        break;
	case B_ATOM_ALLOC_ALLOC:
		atom->alloc_type = 0;
		factory->stats.free_alloc++;
		factory->alloc->bmem_free(factory->alloc, atom);
		break;
	}
	return;
}

batom_accum_t
batom_accum_create(batom_factory_t factory)
{
	batom_accum_t acc;

	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	acc = factory->alloc->bmem_alloc(factory->alloc, sizeof(*acc)+B_ATOM_ACC_INIT_VEC*(sizeof(batom_vec)+sizeof(batom_t)));
	if (!acc) {
		return NULL;
	}
	BDBG_OBJECT_INIT(acc, batom_accum_t);
	acc->factory = factory;
	b_io_accum_init(acc);
	acc->count = 0;
	acc->length = 0;
	return acc;
}

void
batom_accum_clear(batom_accum_t acc)
{
	unsigned count;
	batom_t *owner;
	BDBG_MSG_TRACE(("batom_accum_clear: >%#lx", (unsigned long)acc));
	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	count = acc->count;
	owner = acc->owner;
	while(count>0) {
		if(*owner) {
			batom_release(*owner);
		}
		count--;
		owner++;
	}
	acc->count = 0;
	acc->length = 0;
	BDBG_MSG_TRACE(("batom_accum_clear: <%#lx", (unsigned long)acc));
	return;
}

void
batom_accum_destroy(batom_accum_t acc)
{
	batom_factory_t factory;
	balloc_iface_t alloc;
	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	factory=acc->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	batom_accum_clear(acc);
	alloc = factory->alloc;
	if((uint8_t *)acc->vec != (uint8_t *)acc+sizeof(*acc)) {
		alloc->bmem_free(alloc, acc->vec);
	}
	BDBG_OBJECT_DESTROY(acc, batom_accum_t);
	alloc->bmem_free(alloc, acc);
	return;
}

const batom_vec *
batom_accum_get_vectors( batom_accum_t acc,  unsigned *nvecs )
{
	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
    BDBG_ASSERT(nvecs);
    *nvecs = acc->count;
    return acc->vec;
}


static bool
b_io_acc_grow(batom_accum_t acc)
{
	batom_factory_t factory;
	void *ptr;
	unsigned nvecs;
	unsigned old_nvecs;

	BDBG_MSG_TRACE(("b_io_acc_grow: >%#lx", (unsigned long)acc));

	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	factory = acc->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	old_nvecs = acc->nvecs;
	nvecs = old_nvecs*2;
	if (nvecs>BATOM_VEC_MAX_SIZE) {
		BDBG_WRN(("b_io_acc_grow: %#lx to many entries in the accumulator (%u)", (unsigned long)acc, (unsigned)nvecs));
		return false;
	}

	/* allocate new array */
	ptr = factory->alloc->bmem_alloc(factory->alloc, nvecs*(sizeof(batom_vec)+sizeof(batom_t)));
	if(!ptr) {
		BDBG_WRN(("b_io_acc_grow: %p can't allocate %u bytes", (void *)acc, (unsigned)(nvecs*(sizeof(batom_vec)+sizeof(batom_t)))));
		return false;
	}
	BKNI_Memcpy(ptr, acc->vec, old_nvecs*sizeof(batom_vec));
	BKNI_Memcpy( (uint8_t *)ptr + nvecs*sizeof(batom_vec), acc->owner, old_nvecs*sizeof(batom_t));
	if( (uint8_t *)acc->vec != (uint8_t *)acc+sizeof(*acc)) {
		/* free previously allocated block */
		factory->alloc->bmem_free(factory->alloc, acc->vec);
	}
	acc->nvecs= nvecs;
	acc->vec = ptr;
	acc->owner = (batom_t *)((uint8_t *)ptr + nvecs*sizeof(batom_vec));
	BDBG_MSG_TRACE(("b_io_acc_grow: <%#lx %u", (unsigned long)acc, acc->count));
	return true;
}

size_t 
batom_accum_len(batom_accum_t acc)
{
	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	BDBG_MSG_TRACE(("batom_accum_len: %#lx %u", (unsigned long)acc, acc->length));
	return acc->length;
}

void
batom_accum_add_vec(batom_accum_t acc, const batom_vec *vec)
{
	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	BDBG_ASSERT(vec);
	BDBG_MSG_TRACE(("batom_accum_add_vec: >%#lx %#lx %u:%u", (unsigned long)acc, (unsigned long)vec, acc->count, acc->nvecs));
	if (acc->count >= acc->nvecs) {
		if(!b_io_acc_grow(acc)) {
			return;
		}
	} 
	BDBG_ASSERT(acc->count<acc->nvecs);
	acc->vec[acc->count] = *vec;
	acc->owner[acc->count] = NULL;
	acc->length += vec->len;
	acc->count++;
	BDBG_MSG_TRACE(("batom_accum_add_vec: <%#lx %#lx[%#lx:%u] %u:%u", (unsigned long)acc, (unsigned long)vec, (unsigned long)vec->base, vec->len, acc->count, acc->nvecs));
	return;
}

void 
batom_accum_add_range(batom_accum_t acc, const void *base, size_t len)
{
	batom_vec vec;

	BDBG_OBJECT_ASSERT(acc, batom_accum_t);

    for(;;) {
        if(len<=BATOM_VEC_MAX_SIZE) {
	B_ATOM_VEC_INIT(&vec, base, len);
	batom_accum_add_vec(acc, &vec);
            break;
        } else {
            size_t vec_len =  BATOM_VEC_MAX_SIZE&(~(sizeof(int)-1)); /* don't make unaligned vectors */
            B_ATOM_VEC_INIT(&vec, base, vec_len);
            batom_accum_add_vec(acc, &vec);
            len -= vec_len;
            base = (uint8_t *)base + vec_len;
        }
    }
	return;
}

void 
batom_accum_add_atom(batom_accum_t acc, batom_t atom)
{
	unsigned count;
	const batom_vec *vecs;
	batom_vec *dst_vecs;
	batom_t *dst_owners;
	batom_t master;
	unsigned acc_count;
	unsigned acc_nvecs;
	size_t acc_len;

	BDBG_MSG_TRACE(("batom_accum_add_atom: >%#lx %#lx", (unsigned long)acc, (unsigned long)atom));
	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	BDBG_ASSERT(atom);
	BDBG_OBJECT_ASSERT(acc->factory, batom_factory_t);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	master = B_ATOM_MASTER(atom);
	BDBG_OBJECT_ASSERT(master->factory, batom_factory_t);
	vecs = B_ATOM_VEC_PTR_ARRAY(master);
	count = master->u.array.count;
#if 0
	if(count==1 && acc->count < acc->nvecs) {
		acc->owner[acc->count] = *owners;
		acc->vec[acc->count] = *vecs;
		acc->count++;
		BDBG_ASSERT(atom->ref_cnt<BATOM_VEC_MAX_SIZE);
		atom->ref_cnt++;
		return;
	}
#endif
	BDBG_ASSERT(acc->count<=acc->nvecs);
	/* temporary variables used to reduce number of memory loads */
	acc_count = acc->count;
	acc_nvecs = acc->nvecs;
	acc_len = acc->length;
	dst_owners = &acc->owner[acc_count];
	dst_vecs = &acc->vec[acc_count];
	for(acc_count=acc->count;count;) {
		/* code alligned in this fashion, to short-circuit most likely case (no grow) */
		if (acc_count < acc_nvecs) {
			b_atom_lock(atom);
			*dst_owners = atom;
			acc_len += vecs->len;
			*dst_vecs = *vecs;
			dst_owners++;
			vecs++;
			dst_vecs++;
			acc_count++;
			count--;
			continue;
		}
		acc->count = acc_count;
		acc->length = acc_len;
		if(!b_io_acc_grow(acc)) {
			break;
		}
		/* reload pointers they may changed */
		dst_owners = &acc->owner[acc_count];
		dst_vecs = &acc->vec[acc_count];
		acc_nvecs = acc->nvecs;
		BDBG_ASSERT(acc->count==acc_count);
		BDBG_ASSERT(acc->count<acc->nvecs);
	}
	acc->length = acc_len;
	acc->count = acc_count;
	BDBG_MSG_TRACE(("batom_accum_add_acc: <%#lx %u:%u", (unsigned long)acc, acc_len, acc_count));
	return;
}


void 
batom_accum_trim(batom_accum_t acc, batom_cursor *cursor)
{
	unsigned i;
	unsigned trim_pos;

	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	BDBG_ASSERT(cursor->vec == acc->vec);
	BDBG_ASSERT(cursor->count == acc->count);
	BDBG_ASSERT(cursor->pos <=  cursor->count);
	BDBG_ASSERT(acc->nvecs  >= acc->count);
	BDBG_MSG_TRACE(("batom_accum_trim<:%#lx %u %u  cursor: %#lx %u %d", (unsigned long)acc, acc->count, acc->length, (unsigned long)cursor, cursor->pos, cursor->left));

	if (cursor->left > 0) {
		trim_pos = cursor->pos-1;
	} else {
		trim_pos = cursor->pos;
	}
	if(trim_pos>0) {
		batom_vec *vec = acc->vec;
		batom_t  *owner = acc->owner;
		size_t count;

		BDBG_ASSERT(acc->count >= trim_pos);
		/* remove dead entries */
		for(count=0,i=0;i<trim_pos;i++) {
			if(*owner) {
				batom_release(*owner);
			}
			count += vec->len;
			owner++;
			vec++;
		}
		BDBG_ASSERT(acc->length >= count);
		acc->length -= count;
		vec = acc->vec;
		owner = acc->owner;
		count = acc->count;
		for(i=0;i+trim_pos<count;i++) {
			*vec = vec[trim_pos];
			*owner = owner[trim_pos];
			vec++;
			owner++;
		}
		acc->count = count - trim_pos;
	}
	if (cursor->left > 0) {
		/* adjust for the partial vector */
		BDBG_ASSERT(acc->vec[0].len >=cursor->left); 
		BDBG_ASSERT(acc->length >= (acc->vec[0].len - (unsigned)cursor->left)); 
		acc->length -= acc->vec[0].len - cursor->left;
		acc->vec[0].base = (void *)cursor->cursor;
		acc->vec[0].len = cursor->left;
	}
	/* adjust cursor */
	cursor->pos = 0;
	cursor->left = 0;
	cursor->cursor = 0;
    cursor->count = acc->count;
	BDBG_MSG_TRACE(("batom_accum_trim<: %#lx %u %u", (unsigned long)acc, acc->count, acc->length));
	return ;
}

static batom_t
b_atom_alloc(batom_factory_t factory, size_t atom_size)
{
	batom_t atom;

	if (atom_size<=B_ATOM_INIT_SIZE) {
		atom = bpool_alloc(factory->pool, B_ATOM_INIT_SIZE);
		if(atom) {
			factory->stats.alloc_pool++;
			atom->alloc_type = B_ATOM_ALLOC_POOL;
			goto done;
		}
		factory->stats.full_pool++;
	}
	atom = barena_alloc(factory->arena, atom_size);
	if (atom) {
		factory->stats.alloc_arena++;
		atom->alloc_type = B_ATOM_ALLOC_ARENA;
		goto done;
	} 
#if 0
	barena_dump(factory->arena);
	BDBG_ERR(("%u:%u:%u:%u %u:%u:%u", atom_size, B_ATOM_INIT_SIZE, factory->stats.full_pool, factory->stats.alloc_pool-factory->stats.free_pool, factory->stats.alloc_arena, factory->stats.alloc_arena-factory->stats.free_arena, factory->stats.full_arena));
	getchar();
#endif
	factory->stats.full_arena++;
	atom = factory->alloc->bmem_alloc(factory->alloc, atom_size);
	if (atom) {
		factory->stats.alloc_alloc++;
		atom->alloc_type = B_ATOM_ALLOC_ALLOC;
		goto done;
	}
	BDBG_ASSERT(0);
done:
	BDBG_MSG_TRACE(("b_atom_alloc: %#lx", (unsigned long)atom));
	return atom;
}


static void
b_atom_init(batom_factory_t factory, batom_t atom, unsigned count, unsigned type, const batom_user  *user, const void *udata)
{
	void * atom_user_ptr;

	atom->factory = factory;
	atom->ref_cnt = 1;
	atom->atom_type = type;
	atom->ops = user;
	BLST_SQ_NEXT(atom,pipe_link) = NULL;
	factory->stats.atom_live++;
	factory->stats.atom_allocated++;
	if(type==B_ATOM_TYPE_ARRAY) {
		atom->u.array.count = count;
	}
	atom_user_ptr = B_ATOM_USER_PTR(atom);
	if(user->user_size==sizeof (void *)) {
		BDBG_ASSERT(udata);
		*(void **) atom_user_ptr = *(void **)udata;
	} else if(user->user_size==2*sizeof (unsigned)) {
		BDBG_ASSERT(udata);
		((unsigned *)atom_user_ptr)[0] = ((unsigned *)udata)[0];
		((unsigned *)atom_user_ptr)[1] = ((unsigned *)udata)[1];
	} else if(user->user_size!=0) {
		BDBG_ASSERT(udata);
		BKNI_Memcpy(atom_user_ptr, udata, user->user_size);
	}
	return;
}

static batom_t
b_atom_create(batom_factory_t factory, unsigned count, unsigned type, const batom_user *user, const void *udata)
{
	batom_t atom;
	size_t atom_size;

	if (user==NULL) {
		user = &b_atom_default_ops;
	}
	atom_size = user->user_size + sizeof(*atom) + count*(sizeof(batom_vec)+sizeof(batom_t));
	atom = b_atom_alloc(factory, atom_size);
	if(atom) {
		b_atom_init(factory, atom, count, type, user, udata);
	}
	return atom;
}
		
batom_t
batom_from_vector(batom_factory_t factory, const batom_vec *vec, const batom_user  *user, const void *udata)
{
	batom_t atom;
	batom_vec *vecs;
	batom_t *owners;

	BDBG_ASSERT(vec);
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	atom = b_atom_create(factory, 1, B_ATOM_TYPE_ARRAY, user, udata);
	if (!atom) {
		goto done;
	}
	atom->u.array.length = vec->len;
	vecs = B_ATOM_VEC_PTR_ARRAY(atom);
	owners = B_ATOM_PTR_ARRAY(atom);
	*vecs = *vec;
	*owners = atom;
done:
	return atom;
}


static void
b_atom_copy_vecs(batom_vec *dst, const batom_vec *src, unsigned count)
{
	for(;count>0;count--) {
		*dst++ = *src++;
	}
	return;
}


static void
b_atom_copy_vecs_copy_owners(batom_vec *v_dst, const batom_vec *v_src, batom_t *o_dst, const batom_t *o_src, unsigned count, batom_t owner)
{
	for(;count>0;count--) {
		batom_t s_atom = *o_src++;
		if(s_atom==NULL) {
			s_atom = owner;
		}
		*o_dst++ = s_atom;
		*v_dst++ = *v_src++;
	}
	return;
}

static void
b_atom_copy_vecs_use_owners(batom_vec *v_dst, const batom_vec *v_src, batom_t *o_dst, const batom_t *o_src, unsigned count, batom_t owner)
{
	for(;count>0;o_src++,o_dst++,count--) {
		batom_t atom = *o_src;
		if(atom!=NULL) {
			b_atom_lock(atom);
			*o_dst = atom;
		} else {
			*o_dst = owner;
		}
		*v_dst++ = *v_src++;
	}
	return;
}


batom_t
batom_from_vectors(batom_factory_t factory, const batom_vec *vec, unsigned nvecs, const batom_user  *user, const void *udata)
{
	batom_t atom;
	batom_vec *vecs;
	batom_t *owners;
	unsigned i;

	BDBG_ASSERT(vec);
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	atom = b_atom_create(factory, nvecs, B_ATOM_TYPE_ARRAY, user, udata);
	if (!atom) {
		goto done;
	}
	vecs = B_ATOM_VEC_PTR_ARRAY(atom);
	owners = B_ATOM_PTR_ARRAY(atom);
	atom->u.array.length = 0;
	for(i=0;i<nvecs;i++) {
		atom->u.array.length += vec[i].len;
		owners[i] = atom;
	}
	b_atom_copy_vecs(vecs, vec, nvecs);
done:
	return atom;
}

/* limit size to BATOM_VEC_MAX_SIZE*48 =  3145680 bytes , to handle MP4/MKV streams this should be as large as largest frame in the stream */
#define BATOM_MAX_VECTORS	48

batom_t
batom_from_range(batom_factory_t factory, const void *base, size_t len, const batom_user  *user, const void *udata)
{
	batom_vec vec[BATOM_MAX_VECTORS];
	batom_t atom;
	BDBG_MSG_TRACE(("batom_from_range>: %p %p %u", (void *)factory, (void *)base, (unsigned)len));
	if(len<=BATOM_VEC_MAX_SIZE) {
		B_ATOM_VEC_INIT(&vec[0], base, len);
		atom = batom_from_vector(factory, &vec[0], user, udata);
	} else {
        unsigned count = 0;
        unsigned size = BATOM_VEC_MAX_SIZE&(~(sizeof(int)-1)); /* don't make unaligned vectors */
        unsigned max_count = 1+ (len / size);
        batom_vec *dynamic_vec;

        if(max_count<BATOM_MAX_VECTORS) {
            max_count = BATOM_MAX_VECTORS;
            dynamic_vec = vec;
        } else {
            dynamic_vec = BKNI_Malloc(max_count * sizeof(vec[0]));
            if(dynamic_vec==NULL) {
                (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                return NULL;
            }
        }

        for(count=0;len>0;count++) {
            if(count>=max_count) {
                BDBG_ERR(("batom_from_range: size of the %u range exceededs %u:(%u:%u)", (unsigned)(len+(max_count*size)), max_count*size, count, max_count));
                atom = NULL;
                goto done;
            }
            if(size>len) {
                size = len;
            }
            len -= size;
            B_ATOM_VEC_INIT(&dynamic_vec[count], base, size);
            base = (uint8_t *)base + size;
        }
        atom = batom_from_vectors(factory, dynamic_vec, count, user, udata);
done:
        if(dynamic_vec!=vec) {
            BKNI_Free(dynamic_vec);
        }
	}
	BDBG_MSG_TRACE(("batom_from_range<: %p -> %p", (void *)factory, (void *)atom));
	return atom;
}

batom_t 
batom_empty(batom_factory_t factory, const batom_user *user, const void *udata)
{
	batom_t atom;

	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	atom = b_atom_create(factory, 0, B_ATOM_TYPE_ARRAY, user, udata);
	if (!atom) {
		goto done;
	}
	atom->u.array.length = 0;
done:
	return atom;
}

batom_t 
batom_from_vec_and_atom(const batom_vec *vec, batom_t atom, const batom_user  *user, const void *udata)
{
	batom_factory_t factory;
	batom_t master;
	unsigned acc_count;

	BDBG_MSG_TRACE(("batom_from_vec_and_atom>: %#lx %#lx", (unsigned long)vec, (unsigned long)atom));
	BDBG_ASSERT(vec);
	BDBG_ASSERT(atom);
	factory = atom->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);
	master = B_ATOM_MASTER(atom);
	acc_count = master->u.array.count+1;
	atom = b_atom_create(factory, acc_count, B_ATOM_TYPE_ARRAY, user, udata);
	if (atom) {
		batom_vec *vecs = B_ATOM_VEC_PTR_ARRAY(atom);
		batom_t *owners = B_ATOM_PTR_ARRAY(atom);
		atom->u.array.length = master->u.array.length+vec->len;
		*vecs = *vec;
		*owners = atom;
		b_atom_copy_vecs_use_owners(vecs+1, B_ATOM_VEC_PTR_ARRAY(master), owners+1, B_ATOM_PTR_ARRAY(master), acc_count-1, atom);
	}
	BDBG_MSG_TRACE(("batom_from_vec_and_atom<: %#lx %#lx -> %#lx", (unsigned long)vec, (unsigned long)master, (unsigned long)atom));
	return atom;
}

batom_t 
batom_from_range_and_atom(const void *base, size_t len, batom_t atom, const batom_user  *user, const void *udata)
{
	batom_vec vec;

	BDBG_ASSERT(len<=BATOM_VEC_MAX_SIZE);

	B_ATOM_VEC_INIT(&vec, base, len);
	return batom_from_vec_and_atom(&vec, atom, user, udata);
}

batom_t
batom_from_accum(batom_accum_t acc, const batom_user  *user, const void *udata)
{
	batom_t atom;
	batom_factory_t factory;
	unsigned acc_count;

	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	factory = acc->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);

	acc_count = acc->count;
	atom = b_atom_create(factory, acc_count, B_ATOM_TYPE_ARRAY, user, udata);
	if (atom) {
		atom->u.array.length = acc->length;
		b_atom_copy_vecs_copy_owners(B_ATOM_VEC_PTR_ARRAY(atom), acc->vec, B_ATOM_PTR_ARRAY(atom), acc->owner, acc_count, atom);
		BDBG_MSG_TRACE(("batom_from_accum: %#lx -> %#lx (%u:%u)", (unsigned long)acc, (unsigned long)atom, atom->u.array.length, atom->u.array.count));
	}
	acc->count = 0;
	acc->length = 0;
	return atom;
}

batom_t
batom_accum_extract(batom_accum_t acc, const batom_cursor *first, const batom_cursor *last, const batom_user  *user, const void *udata)
{
	batom_t atom;
	batom_factory_t factory;
	unsigned nvecs;
	unsigned first_pos;

	BDBG_MSG_TRACE(("batom_accum_extract>: %#lx %#lx %#lx %#lx %#lx", (unsigned long)acc, (unsigned long)first, (unsigned long)last, (unsigned long)user, (unsigned long)udata));
	BDBG_OBJECT_ASSERT(acc, batom_accum_t);
	BDBG_ASSERT(first->vec == acc->vec);
	BDBG_ASSERT(first->count == acc->count);
	BDBG_ASSERT(first->pos <=  first->count);
	BDBG_ASSERT(last->vec == acc->vec);
	BDBG_ASSERT(last->count == acc->count);
	BDBG_ASSERT(last->pos <=  last->count);
	BDBG_ASSERT(acc->nvecs >= acc->count);
	BDBG_ASSERT(first->pos <= last->pos);
	BDBG_ASSERT(batom_cursor_pos(first) <= batom_cursor_pos(last));

	factory = acc->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);

	BDBG_MSG_TRACE(("batom_accum_extract: %#lx %u..%u %#lx[%u]", (unsigned long)acc, batom_cursor_pos(first), batom_cursor_pos(last), (unsigned long)user, user?user->user_size:0));
	if(first->left>0) {
		first_pos = first->pos - 1;
	} else {
		first_pos = first->pos;
	} 	
	BDBG_ASSERT(last->pos>=first_pos);
	nvecs = last->pos - first_pos;

	atom = b_atom_create(factory, nvecs, B_ATOM_TYPE_ARRAY, user, udata);
	if (atom) {
		atom->u.array.length = batom_cursor_pos(last) - batom_cursor_pos(first);
		if (nvecs>0) {
			batom_vec *vecs = B_ATOM_VEC_PTR_ARRAY(atom);
			/* copy vectors */

			b_atom_copy_vecs_use_owners(vecs, acc->vec+first_pos, B_ATOM_PTR_ARRAY(atom), acc->owner+first_pos, nvecs, atom);
			/* adjust first vector */
			if(first->left > 0) {
				BDBG_ASSERT(acc->vec[first_pos].len>= first->left);
				vecs[0].len = first->left;
				vecs[0].base = (uint8_t *)vecs[0].base + (acc->vec[first_pos].len - first->left);
			}
			/* adjust last vector */
			if(last->left > 0) {
				BDBG_ASSERT(vecs[nvecs-1].len >= last->left);
				vecs[nvecs-1].len -= last->left;
			}
			BDBG_MSG_TRACE(("batom_accum_extract: %#lx -> %#lx (%u:%u) %#lx:%u", (unsigned long)acc, (unsigned long)atom, atom->u.array.length, atom->u.array.count, (unsigned long)vecs[0].base, vecs[0].len));
		} /* else nvecs = 0, do nothing */
	}
	BDBG_MSG_TRACE(("batom_accum_extract<: %#lx %#lx(%u)", (unsigned long)acc, (unsigned long)atom, atom?atom->u.array.length:0));
	return atom;
}

bool
batom_accum_append(batom_accum_t dst, batom_accum_t src, const batom_cursor *first, const batom_cursor *last)
{
	batom_factory_t factory;
	unsigned nvecs;
	unsigned first_pos;
	unsigned dst_pos;

	BDBG_MSG_TRACE(("batom_accum_copy>: %#lx<-%lx %#lx %#lx %#lx", (unsigned long)dst, (unsigned long)src, (unsigned long)first, (unsigned long)last));
	BDBG_OBJECT_ASSERT(dst, batom_accum_t);
	BDBG_OBJECT_ASSERT(src, batom_accum_t);
	BDBG_ASSERT(first->vec == src->vec);
	BDBG_ASSERT(first->count == src->count);
	BDBG_ASSERT(first->pos <=  first->count);
	BDBG_ASSERT(last->vec == src->vec);
	BDBG_ASSERT(last->count == src->count);
	BDBG_ASSERT(last->pos <=  last->count);
	BDBG_ASSERT(src->nvecs >= src->count);
	BDBG_ASSERT(dst->nvecs >= dst->count);
	BDBG_ASSERT(first->pos <= last->pos);
	BDBG_ASSERT(batom_cursor_pos(first) <= batom_cursor_pos(last));

	factory = dst->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);

	BDBG_MSG_TRACE(("batom_accum_copy: %#lx->%lx %u..%u", (unsigned long)src, (unsigned long)dst, batom_cursor_pos(first), batom_cursor_pos(last)));
	if(first->left>0) {
		first_pos = first->pos - 1;
	} else {
		first_pos = first->pos;
	}
	BDBG_ASSERT(last->pos>=first_pos);
	nvecs = last->pos - first_pos;
	while(dst->count+nvecs >= dst->nvecs) {
		/* grow until it has enough space */
		dst_pos = dst->nvecs;
		if(!b_io_acc_grow(dst)) {
			return false;
		}
		BDBG_ASSERT(dst->nvecs>dst_pos); /* test that loop eventually terminates */
	} 
	dst->length += batom_cursor_pos(last) - batom_cursor_pos(first);
	if (nvecs>0) {
		batom_vec *vecs; /* pointer to the vector table */
		dst_pos = dst->count;
		dst->count += nvecs;
		vecs = &dst->vec[dst_pos];
		/* copy vectors */
		b_atom_copy_vecs_use_owners(vecs, src->vec+first_pos, dst->owner+dst_pos, src->owner+first_pos, nvecs, NULL);
		/* adjust first vector */
		if(first->left > 0) {
			BDBG_ASSERT(src->vec[first_pos].len>= first->left);
			vecs[0].len = first->left;
			vecs[0].base = (uint8_t *)vecs[0].base + (src->vec[first_pos].len - first->left);
		}
		/* adjust last vector */
		if(last->left > 0) {
			BDBG_ASSERT(vecs[nvecs-1].len >= last->left);
			vecs[nvecs-1].len -= last->left;
		}
		BDBG_MSG_TRACE(("batom_accum_copy: %#lx <- %#lx (%u(%u):%u) %#lx:%u", (unsigned long)dst, (unsigned long)src, dst->length, batom_cursor_pos(last) - batom_cursor_pos(first), nvecs, (unsigned long)vecs[0].base, vecs[0].len));
	} /* else nvecs = 0, do nothing */
	BDBG_MSG_TRACE(("batom_accum_copy<: %#lx %#lx(%u) %u:%u", (unsigned long)dst, (unsigned long)src, dst->length, dst->nvecs, dst->count, dst->nvecs));
	return true;
}

batom_t
batom_extract(batom_t src, const batom_cursor *first, const batom_cursor *last, const batom_user  *user, const void *udata)
{
	batom_t atom;
	batom_t master;
	batom_factory_t factory;
	unsigned nvecs;
	unsigned first_pos;

	BDBG_MSG_TRACE(("batom_extract>: %#lx %#lx %#lx %#lx %#lx", (unsigned long)src, (unsigned long)first, (unsigned long)last, (unsigned long)user, (unsigned long)udata));
	BDBG_ASSERT(src);

	master = B_ATOM_MASTER(src);
	BDBG_ASSERT(master);
	factory = src->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);

	BDBG_ASSERT(first->vec == B_ATOM_VEC_PTR_ARRAY(master));
	BDBG_ASSERT(first->count == master->u.array.count);
	BDBG_ASSERT(first->pos <=  first->count);
	BDBG_ASSERT(last->vec == B_ATOM_VEC_PTR_ARRAY(master));
	BDBG_ASSERT(last->count == master->u.array.count);
	BDBG_ASSERT(last->pos <=  last->count);
	BDBG_ASSERT(first->pos <= last->pos);
	BDBG_ASSERT(batom_cursor_pos(first) <= batom_cursor_pos(last));

	BDBG_MSG_TRACE(("batom_accum_extract: %#lx %u..%u %#lx[%u]", (unsigned long)src, batom_cursor_pos(first), batom_cursor_pos(last), (unsigned long)user, user?user->user_size:0));
	if(first->left>0) {
		first_pos = first->pos - 1;
	} else {
		first_pos = first->pos;
	} 	
	BDBG_ASSERT(last->pos>=first_pos);
	nvecs = last->pos - first_pos;

	atom = b_atom_create(factory, nvecs, B_ATOM_TYPE_ARRAY, user, udata);
	if (atom) {
		atom->u.array.length = batom_cursor_pos(last) - batom_cursor_pos(first);
		if (nvecs>0) {
			const batom_vec *svecs = B_ATOM_VEC_PTR_ARRAY(master);
			batom_vec *vecs = B_ATOM_VEC_PTR_ARRAY(atom);
			/* copy vectors */

			b_atom_copy_vecs_use_owners(vecs, svecs+first_pos, B_ATOM_PTR_ARRAY(atom), B_ATOM_PTR_ARRAY(master)+first_pos, nvecs, atom);
			/* adjust first vector */
			if(first->left > 0) {
				BDBG_ASSERT(svecs[first_pos].len>= first->left);
				vecs[0].len = first->left;
				vecs[0].base = (uint8_t *)vecs[0].base + (svecs[first_pos].len - first->left);
			}
			/* adjust last vector */
			if(last->left > 0) {
				BDBG_ASSERT(vecs[nvecs-1].len >= last->left);
				vecs[nvecs-1].len -= last->left;
			}
			BDBG_MSG_TRACE(("batom_accum_extract: %#lx -> %#lx (%u:%u) %#lx:%u", (unsigned long)src, (unsigned long)atom, atom->u.array.length, atom->u.array.count, (unsigned long)vecs[0].base, vecs[0].len));
		} /* else nvevcs = 0, do nothing */
	}
	BDBG_MSG_TRACE(("batom_accum_extract<: %#lx %#lx(%u)", (unsigned long)src, (unsigned long)atom, atom?atom->u.array.length:0));
	return atom;
}

size_t batom_cursor_extract (const batom_cursor *cursor, batom_vec *vecs, size_t vec_size, size_t *required_vec_size)
{
    unsigned first_pos;
    unsigned i;
    unsigned nvecs;
    size_t result = 0;

    if(cursor->left>0) {
        first_pos = cursor->pos - 1;
    } else {
        first_pos = cursor->pos;
    }
    nvecs = cursor->count - first_pos;
    *required_vec_size = nvecs;
    for(i=0;i+first_pos<cursor->count;i++) {
        const batom_vec *v = cursor->vec + (first_pos + i);

        result += v->len;
        if(i<vec_size) {
            BATOM_VEC_INIT(vecs+i, v->base, v->len);
        }
    }
    /* adjust first vector */
    if(cursor->left > 0) {
        vecs[0].len = cursor->left;
        vecs[0].base = (uint8_t *)vecs[0].base + (cursor->vec[first_pos].len - cursor->left);
    }
    return result;
}

batom_t 
batom_clone(batom_t atom, const batom_user  *user, const void *udata)
{
	batom_t clone;
	batom_factory_t factory;
	BDBG_ASSERT(atom);


	BDBG_MSG_TRACE(("batom_clone>: %#lx", (unsigned long)atom));
	factory = atom->factory;
	BDBG_OBJECT_ASSERT(factory, batom_factory_t);

	clone = b_atom_create(factory, 0, B_ATOM_TYPE_CLONE, user, udata);
	if(clone) {
		b_atom_lock(atom);
		clone->u.clone.parent = atom;
		clone->u.clone.master = B_ATOM_MASTER(atom);
	}
	BDBG_MSG_TRACE(("batom_clone<: %#lx -> %#lx", (unsigned long)atom, (unsigned long)clone));
	return clone;
}


size_t 
batom_len(batom_t atom)
{
	batom_t master;

	BDBG_MSG_TRACE(("batom_len: >%#lx", (unsigned long)atom));
	BDBG_ASSERT(atom);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	master = B_ATOM_MASTER(atom);
	BDBG_OBJECT_ASSERT(master->factory, batom_factory_t);
	BDBG_MSG_TRACE(("batom_len: <%#lx %u", (unsigned long)atom, master->u.array.length));
	return master->u.array.length;
}

const batom_vec *
batom_get_vectors(batom_t atom, unsigned *nvecs)
{
	batom_t master;

	BDBG_ASSERT(atom);
	BDBG_ASSERT(nvecs);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	master = B_ATOM_MASTER(atom);
	BDBG_ASSERT(master);
	*nvecs = master->u.array.count;
	BDBG_OBJECT_ASSERT(master->factory, batom_factory_t);
	return B_ATOM_VEC_PTR_ARRAY(master);
}

const batom_vec *
batom_get_vec(batom_t atom, unsigned vecno)
{
	batom_t master;

	BDBG_ASSERT(atom);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	master = B_ATOM_MASTER(atom);
	BDBG_ASSERT(master);
	BDBG_OBJECT_ASSERT(master->factory, batom_factory_t);
	if(vecno<master->u.array.count) {
		return B_ATOM_VEC_PTR_ARRAY(master)+vecno;
	} 
	return NULL;
}

unsigned 
batom_get_nvecs(batom_t atom)
{
	batom_t master;

	BDBG_ASSERT(atom);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	master = B_ATOM_MASTER(atom);
	BDBG_ASSERT(master);
	BDBG_OBJECT_ASSERT(master->factory, batom_factory_t);
	return master->u.array.count;
}

void *
batom_userdata(batom_t atom)
{
	BDBG_MSG_TRACE(("batom_userdata: >%#lx", (unsigned long)atom));
	BDBG_ASSERT(atom);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	if(atom->ops->user_size) {
		BDBG_MSG_TRACE(("batom_userdata: <%#lx %#lx %#lx", (unsigned long)atom, (unsigned long)B_ATOM_USER_PTR(atom), *(unsigned long*)B_ATOM_USER_PTR(atom)));
		return B_ATOM_USER_PTR(atom); 
	} else {
		BDBG_MSG_TRACE(("batom_userdata: <%#lx NULL", (unsigned long)atom));
		return NULL;
	}
}


static size_t 
b_atom_cursor_refill(batom_cursor *cursor)
{

	BDBG_ASSERT(cursor->left<=0);
	if (cursor->left==0) {
		unsigned pos;
		const batom_vec *vec;
		BDBG_ASSERT(cursor->vec);
		BDBG_ASSERT(cursor->pos <= cursor->count);
		for(pos=cursor->pos,vec=&cursor->vec[pos];pos < cursor->count;vec++) {
			pos++;
			cursor->pos = pos;
			if (vec->len>0) {
				cursor->left = vec->len;
				cursor->cursor = vec->base;
				BDBG_MSG_TRACE(("b_atom_cursor_refill: %#lx %u:%u %u %#lx", (unsigned long)cursor, cursor->pos, cursor->count, cursor->left, (unsigned long)cursor->cursor));
				return (size_t)cursor->left;
			}
		}
		/* (pos >= cursor->count)  */
		BDBG_MSG_TRACE(("b_atom_cursor_refill: %#lx %u %u EOF", (unsigned long)cursor, cursor->pos, cursor->count));
		/* reached EOF */
		cursor->left = BATOM_EOF;
	}
	return 0;
}

void 
batom_cursor_empty(batom_cursor *cursor)
{
	BDBG_ASSERT(cursor);
	/* empty array */
	cursor->left = -1;
	cursor->cursor = 0;

	cursor->count = 0;
	cursor->pos = 0;
	cursor->vec = NULL;
	return;
}


void 
batom_cursor_from_vec(batom_cursor *cursor, const batom_vec *vec, unsigned count)
{
	BDBG_ASSERT(cursor);
	/* empty array */
	cursor->left = 0;
	cursor->cursor = 0;

	cursor->count = count;
	cursor->pos = 0;
	cursor->vec = vec;
	b_atom_cursor_refill(cursor);
	return;
}

void
batom_cursor_from_atom(batom_cursor *cursor, batom_t atom)
{
	BDBG_ASSERT(cursor);
	BDBG_ASSERT(atom);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	atom = B_ATOM_MASTER(atom);
	BDBG_ASSERT(atom);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	batom_cursor_from_vec(cursor, B_ATOM_VEC_PTR_ARRAY(atom),atom->u.array.count);
	return;
}

void
batom_cursor_from_accum(batom_cursor *cursor, batom_accum_t acc)
{
	BDBG_ASSERT(cursor);
	BDBG_ASSERT(acc);
	BDBG_OBJECT_ASSERT(acc->factory, batom_factory_t);
	batom_cursor_from_vec(cursor, acc->vec, acc->count);
	return;
}

int 
batom_cursor_next(batom_cursor *cursor)
{
	int d;

	BDBG_ASSERT(cursor);
	while(cursor->left<=0) {
		if (b_atom_cursor_refill(cursor)==0) {
			return BATOM_EOF;
		}
		BDBG_ASSERT(cursor->left>0);
	}
	d = cursor->cursor[0];
	cursor->left--;
	cursor->cursor++;
	BDBG_MSG_FLOW(("batom_cursor_next: %#lx %d 0x%02x", (unsigned long)cursor, cursor->left, d));
	return d;
}

bool 
batom_cursor_eof(const batom_cursor *cursor)
{
	BDBG_ASSERT(cursor);
	BDBG_ASSERT(cursor->vec);
	BDBG_ASSERT(cursor->pos <= cursor->count);
	return cursor->left < 0;
}

size_t
batom_cursor_skip(batom_cursor *cursor, size_t count)
{
	size_t left;

	BDBG_MSG_TRACE(("batom_cursor_skip>: %#lx %d %u", (unsigned long)cursor, cursor->left, count));
	count = B_ATOM_TRUNCATE_SIZE(count);
	for(left=count;;) {
		if (cursor->left>=(int)left) {
			cursor->cursor+=left;
			cursor->left-=left;
			left = 0;
	 		break;
		} 
		if (cursor->left>=0) {
			left -= cursor->left;
			cursor->left = 0;
			if (b_atom_cursor_refill(cursor)==0) {
				break;
			}
		} else {
			break;
		}
	}
	BDBG_MSG_TRACE(("batom_cursor_skip<: %#lx %d %u %u", (unsigned long)cursor, cursor->left, count, count-left));
	return count-left;
}

size_t
batom_cursor_distance(const batom_cursor *from, const batom_cursor *to)
{
	batom_cursor save;
	size_t distance;

	BDBG_ASSERT(from);
	BDBG_ASSERT(to);
	BDBG_ASSERT(from->vec==to->vec);
	BDBG_ASSERT(from->pos<=to->pos);

	if(from->pos==to->pos && to->left!=BATOM_EOF) {
		BDBG_ASSERT(from->left!=BATOM_EOF);
		BDBG_ASSERT(from->left >= to->left);
		return from->left - to->left;
	}
	BDBG_ASSERT(from->left!=BATOM_EOF || (from->left==BATOM_EOF && to->left==BATOM_EOF && to->pos==from->pos));
	for(distance=0,save = *from; save.pos!=to->pos;) {
		distance += save.left;
		save.left = 0;
		b_atom_cursor_refill(&save);
	}
	BDBG_ASSERT(save.left >= to->left);
	distance += save.left - to->left;
	return distance;
}

size_t 
batom_cursor_copy(batom_cursor *cursor, void *dest, size_t count)
{
	size_t left;

	BDBG_ASSERT(cursor);
	BDBG_ASSERT((int)count>=0);
	BDBG_MSG_TRACE(("batom_cursor_copy>: %#lx %u", (unsigned long)cursor, count));

	for(left=count;;) {
		const uint8_t *src=cursor->cursor;
		int src_left = cursor->left;

		if(src_left>=(int)left) {
			cursor->cursor = src+left;
			cursor->left = src_left-left;
			BKNI_Memcpy(dest, src, left);
			left = 0;
			break;
		}
		if(src_left>=0) {
			cursor->cursor = src+src_left;
			cursor->left = 0;
			left -= src_left;
			BKNI_Memcpy(dest, src, src_left);
			dest = (uint8_t*)dest+src_left;
			if (b_atom_cursor_refill(cursor)==0) {
				break;
			}
		} else {
			break;
		}
	}
	return count-left;
}

int 
batom_cursor_compare(const batom_cursor *cursor, const void *buff, size_t count) 
{
	size_t left;
    batom_cursor data;

	BDBG_ASSERT(cursor);
	BDBG_ASSERT((int)count>=0);
	BDBG_MSG_TRACE(("batom_cursor_compare>: %#lx %u", (unsigned long)cursor, count));

    BATOM_CLONE(&data, cursor);
	for(left=count;;) {
		const uint8_t *cursor_data=data.cursor;
		int cursor_left = data.left;
        int rc;

		if(cursor_left>=(int)left) {
            /* last block */
            rc = BKNI_Memcmp(buff, cursor_data, left);
            return rc;
		} else if(cursor_left>=0) {
			data.cursor = cursor_data+cursor_left;
			data.left = 0;
			left -= cursor_left;
            rc = BKNI_Memcmp(buff, cursor_data, left);
            if(rc!=0) {
                /* no match */
                return rc;
            }
			buff = (uint8_t*)buff+cursor_left;
			if (b_atom_cursor_refill(&data)!=0) {
                /* some data left in the cursor */
                continue;
            }
        }
        break;
	}
    /* reached end of data */
    return -1;
}

void 
batom_cursor_save(const batom_cursor *cursor, batom_checkpoint *checkpoint)
{
	BDBG_MSG_TRACE(("batom_cursor_save: %#lx", (unsigned long)cursor));
	checkpoint->cp_left = cursor->left;
	checkpoint->cp_pos = cursor->pos;
	return;
}

void 
batom_cursor_rollback(batom_cursor *cursor, const batom_checkpoint *checkpoint)
{
	BDBG_MSG_TRACE(("batom_cursor_rollback: %#lx(%d:%u) %lx(%d:%u)", (unsigned long)cursor, cursor->left, cursor->pos, (unsigned long)checkpoint, checkpoint->cp_left, checkpoint->cp_pos));
	BDBG_ASSERT( cursor->pos >= checkpoint->cp_pos );
	cursor->left = checkpoint->cp_left;
	cursor->pos = checkpoint->cp_pos;
	if (cursor->left>0) {
		BDBG_ASSERT(cursor->vec);
		BDBG_ASSERT(cursor->pos <= cursor->count && cursor->pos>0);
		BDBG_ASSERT(cursor->left <= (int)cursor->vec[cursor->pos-1].len);
		cursor->cursor = cursor->vec[cursor->pos-1].base;
		cursor->cursor += cursor->vec[cursor->pos-1].len - cursor->left;
		BDBG_MSG_TRACE(("batom_cursor_rollback: %#lx(%d:%u:%#lx)", (unsigned long)cursor, cursor->left, cursor->pos, (unsigned long)cursor->cursor));
	} else {
		BDBG_MSG_TRACE(("batom_cursor_rollback: %#lx %u %u EOF", (unsigned long)cursor, cursor->pos, cursor->count));
		return;
	}
	return;
}

size_t 
batom_cursor_pos(const batom_cursor *cursor)
{
	size_t i;
	size_t size;

	/* count completed iovec's */
	for(size=0,i=0;i<cursor->pos;i++) {
		size += cursor->vec[i].len;
	}
	/* add current iovec */
	if (cursor->left>0) {
		size -= cursor->left;
	}
	BDBG_MSG_TRACE(("batom_cursor_pos: %#lx %u", (unsigned long)cursor, size));
	return size;
}

size_t 
batom_cursor_size(const batom_cursor *cursor)
{
	size_t size;
	unsigned i;
	for(size=0,i=0;i<cursor->count;i++) {
		size+=cursor->vec[i].len;
	}
	return size-batom_cursor_pos(cursor);
}

size_t
batom_cursor_reserve(const batom_cursor *cursor, size_t count)
{
	batom_cursor save;
	BDBG_ASSERT(cursor);
	BDBG_MSG_TRACE(("bio_cursor_reserve+: %#lx(%d,%u) %u", (unsigned long)cursor, cursor->left, cursor->pos, count));

	BDBG_ASSERT(count == B_ATOM_TRUNCATE_SIZE(count)); /* fast versuib BATOM_RESERVE doesn't mask msb, so make slow version to fail */
	if(cursor->left > (int)count) {
		return count; /* short circuit */
	}

    BATOM_CLONE(&save,cursor);

	return batom_cursor_skip(&save, count);
}

void 
batom_cursor_clone(batom_cursor *dst, const batom_cursor *src) 
{
	BDBG_ASSERT(dst);
	BDBG_ASSERT(src);
	/* do a plain copy */
	*dst = *src;
	return;
}

uint8_t
batom_cursor_byte(batom_cursor *c)
{
	uint8_t d;
	if (c->left>0) {
		d = c->cursor[0];
		c->left--;
		c->cursor++;
	}  else {
		if (b_atom_cursor_refill(c)==0) {
			return (uint8_t)BATOM_EOF;
		}
		BDBG_ASSERT(c->left>0);
		d = c->cursor[0];
		c->left--;
		c->cursor++;
	}
	return d;
}

uint32_t 
batom_cursor_vword_le(batom_cursor *c, unsigned v)
{
	uint32_t d;
	int c_left;
	const uint8_t *c_cursor;
	unsigned shift=0;

	c_left = c->left;
	c_cursor = c->cursor;
	for(d=0;v>0;) {
		if (c_left>0) {
			d |= ((uint32_t)c_cursor[0])<<shift;
			c_left--;
			c_cursor++;
			v--;
			shift +=8;
			continue;
		}  else {
			/* refill cursor */
			c->left = c_left;
			c->cursor = c_cursor;
			if (b_atom_cursor_refill(c)==0) {
				return BATOM_EOF;
			}
			c_left = c->left;
			c_cursor = c->cursor;
		}
	}
	c->left = c_left;
	c->cursor = c_cursor;
	return d;
}

uint32_t
batom_cursor_vword_be(batom_cursor *c, unsigned v)
{
	uint32_t d;
	int c_left;
	const uint8_t *c_cursor;

	c_left = c->left;
	c_cursor = c->cursor;
	for(d=0;v>0;) {
		if (c_left>0) {
			d = (d<<8) | ((uint32_t)c_cursor[0]);
			c_left--;
			c_cursor++;
			v--;
			continue;
		}  else {
			/* refill cursor */
			c->left = c_left;
			c->cursor = c_cursor;
			if (b_atom_cursor_refill(c)==0) {
				return BATOM_EOF;
			}
			c_left = c->left;
			c_cursor = c->cursor;
		}
	}
	c->left = c_left;
	c->cursor = c_cursor;
	return d;
}



uint16_t
batom_cursor_uint16_le(batom_cursor *c)
{
	uint16_t d;
	if (c->left>=2) {
		d =
						c->cursor[0] | 
			(((uint16_t)c->cursor[1])<<8);
		c->left -= 2;
		c->cursor += 2;
	} else {
		d = batom_cursor_vword_le(c, 2);
	}
	BDBG_MSG_TRACE(("batom_cursor_uint16_le: %#lx -> 0x%08x", (unsigned long)c, (unsigned)d));
	return d;
}

uint32_t
batom_cursor_uint32_le(batom_cursor *c)
{
	uint32_t d;
	if (c->left>=4) {
		d =
						c->cursor[0] |
			(((uint32_t)c->cursor[1])<<8) |
			(((uint32_t)c->cursor[2])<<16) |
			(((uint32_t)c->cursor[3])<<24);
		c->left -= 4;
		c->cursor += 4;
	} else {
		d = batom_cursor_vword_le(c, 4);
	}
	BDBG_MSG_TRACE(("batom_cursor_uint32_le: %#lx -> 0x%08x", (unsigned long)c, (unsigned)d));
	return d;
}

uint16_t
batom_cursor_uint16_be(batom_cursor *c)
{
	uint16_t d;
	if (c->left>=2) {
		d =
			(((uint16_t)c->cursor[0])<<8) |
						c->cursor[1];
		c->left -= 2;
		c->cursor += 2;
	} else {
		d = batom_cursor_vword_be(c, 2);
	}
	BDBG_MSG_TRACE(("batom_cursor_uint16_be: %#lx -> 0x%08x", (unsigned long)c, (unsigned)d));
	return d;
}

uint32_t
batom_cursor_uint32_be(batom_cursor *c)
{
	uint32_t d;
	if (c->left>=4) {
		d =
			(((uint32_t)c->cursor[0])<<24) |
			(((uint32_t)c->cursor[1])<<16) |
			(((uint32_t)c->cursor[2])<<8) |
						c->cursor[3];
		c->left -= 4;
		c->cursor += 4;
	} else {
		d = batom_cursor_vword_be(c, 4);
	}
	BDBG_MSG_TRACE(("batom_cursor_uint32_le: %#lx -> 0x%08x", (unsigned long)c, (unsigned)d));
	return d;
}

uint64_t 
batom_cursor_uint64_le(batom_cursor *c)
{
	uint64_t d;

	d = batom_cursor_uint32_le(c);
	d |= ((uint64_t)batom_cursor_uint32_le(c))<<32;
	BDBG_MSG_TRACE(("batom_cursor_uint64_le: %#lx -> 0x%16llx", (unsigned long)c, d));
	return d;
}

uint64_t 
batom_cursor_uint64_be(batom_cursor *c)
{
	uint64_t d;

	d = ((uint64_t)batom_cursor_uint32_be(c))<<32;
	d |= batom_cursor_uint32_be(c);
	BDBG_MSG_TRACE(("batom_cursor_uint64_be: %#lx -> 0x%16llx", (unsigned long)c, d));
	return d;
}

uint32_t 
batom_cursor_uint24_le(batom_cursor *c)
{
	uint32_t d;
	if (c->left>=3) {
		d =
						c->cursor[0] |
			(((uint32_t)c->cursor[1])<<8) |
			(((uint32_t)c->cursor[2])<<16);
		c->left -= 3;
		c->cursor += 3;
	} else {
		d = batom_cursor_vword_le(c, 3);
	}
	BDBG_MSG_TRACE(("batom_cursor_uint24_le: %#lx -> 0x%08x", (unsigned long)c, (unsigned)d));
	return d;
}

uint32_t 
batom_cursor_uint24_be(batom_cursor *c)
{
	uint32_t d;

	if (c->left>=3) {
		d =
			(((uint32_t)c->cursor[0])<<16) |
			(((uint32_t)c->cursor[1])<<8) |
						c->cursor[2];
		c->left -= 3;
		c->cursor += 3;
	} else {
		d = batom_cursor_vword_be(c, 3);
	}
	BDBG_MSG_TRACE(("batom_cursor_uint24_le: %#lx -> 0x%08x", (unsigned long)c, (unsigned)d));
	return d;
}

const void *
batom_cursor_continuous(const batom_cursor *c, size_t bytes)
{
	BDBG_ASSERT(bytes==B_ATOM_TRUNCATE_SIZE(bytes)); /* fast version BATOM_CONTINUOUS doesn't work properly if msb bytes set, so try to detect it here */
	if( ( c->left >= (int)bytes) /* there is enough bytes in the current segment */
		|| (c->left >= 0 && c->pos <= c->count && c->left + c->vec[c->pos].len >= (int)bytes && c->cursor+c->left == c->vec[c->pos].base)) /* next vector entry start at the end of current vector and it's they exceed requested size */ {
		return c->cursor;
	}
	return NULL;
}

void
batom_bitstream_init(batom_bitstream *bs, batom_cursor *cursor)
{
    bs->cursor = cursor;
    bs->cache = 0;
    bs->cache_pos = -1; /* invalid */
}

void
batom_bitstream_dump(const batom_bitstream *bs)
{
    BSTD_UNUSED(bs);
    BDBG_WRN(("batom_bitstream: cache %08x(%08x) cache_pos %d", bs->cache, (bs->cache << (31-bs->cache_pos)), bs->cache_pos));
    return;
}

bool
batom_bitstream_eof(batom_bitstream *bs)
{
    return bs->cache_pos == -1 && batom_cursor_eof(bs->cursor);
}

static int
b_atom_bitstream_refill(batom_bitstream *bs)
{
    batom_cursor *c = bs->cursor;

    BDBG_ASSERT(bs->cache_pos==-1);
    if (c->left>=4) {
        bs->cache = (uint32_t)c->cursor[3] |
            ((uint32_t)(c->cursor[2])<<8) |
            ((uint32_t)(c->cursor[1])<<16) |
            ((uint32_t)(c->cursor[0])<<24);
        bs->cache_pos=31;
        c->cursor += 4;
        c->left -= 4;
        return 0;
    } else
    {
        int next;
        uint32_t cache;

        next = batom_cursor_next(c);
        if (next==BATOM_EOF) {
            return next;
        }
        cache = (uint32_t)next;
        next = batom_cursor_next(c);
        if (next==BATOM_EOF) {
            bs->cache = cache;
            bs->cache_pos = 7;
            return 0;
        }
        cache = (cache << 8) | (uint32_t)next;

        next = batom_cursor_next(c);
        if (next==BATOM_EOF) {
            bs->cache = cache;
            bs->cache_pos = 15;
            return 0;
        }
        cache = (cache << 8) | (uint32_t)next;

        next = batom_cursor_next(c);
        if (next==BATOM_EOF) {
            bs->cache = cache;
            bs->cache_pos = 23;
            return 0;
        }
        bs->cache = (cache << 8) | (uint32_t)next;
        bs->cache_pos = 31;
        return 0;
    }
}

size_t
batom_bitstream_position(const batom_bitstream *bs)
{
    size_t cursor_pos = batom_cursor_pos(bs->cursor);
    return cursor_pos - (bs->cache_pos+1)/8;
}

size_t
batom_bitstream_bit_position(const batom_bitstream *bs)
{
    size_t cursor_pos = batom_cursor_pos(bs->cursor);
    return (cursor_pos*8) - (bs->cache_pos+1);
}

int
batom_bitstream_show(batom_bitstream *bs)
{
    int bit;

    if(bs->cache_pos==-1) {
        if (b_atom_bitstream_refill(bs)==BATOM_EOF) {
            return BATOM_EOF;
        }
    }
    bit = (bs->cache >> bs->cache_pos)&1;
    return bit;
}

int
batom_bitstream_bit(batom_bitstream *bs)
{
    int bit;

    bit = batom_bitstream_show(bs);
    bs->cache_pos = bit!=BATOM_EOF ? bs->cache_pos-1:bs->cache_pos;
    return bit;
}

int
batom_bitstream_drop(batom_bitstream *bs)
{
    return batom_bitstream_bit(bs);
}

unsigned
batom_bitstream_bits(batom_bitstream *bs, unsigned nbits)
{
    uint32_t bits;
    int pos;

    BDBG_ASSERT(nbits<=31);
    BDBG_ASSERT(nbits>0);
    pos = bs->cache_pos - nbits;
    nbits--;
    if( bs->cache_pos>=(int)nbits) {
        bits = (bs->cache >> (bs->cache_pos-nbits)) & ((uint32_t)(-1)>>(31-nbits));
        bs->cache_pos = pos;
        return bits;
    } else {
        for(bits=0;;) {
            bits |= (unsigned) batom_bitstream_bit(bs);
            if (nbits==0) {
                break;
            }
            nbits--;
            bits <<=1;
        }
        return bits;
    }
}

unsigned
batom_bitstream_exp_golomb(batom_bitstream *bs)
{
    unsigned prefix;
    unsigned data;
    for(prefix=0;;prefix++) {
        int bit = batom_bitstream_bit(bs);
        if(bit==1) {
            break;
        }
        if(bit==BATOM_EOF) {
            return 0;
        }
    }
    if(prefix==0) {
        return 0;
    }
    if(prefix>=8*sizeof(unsigned) - 1) {
        BDBG_WRN(("batom_bitstream_exp_golomb: not supported prefix len %u", prefix));
        return 0;
    }
    data = batom_bitstream_bits(bs, prefix);
    data = (1<<prefix) | data;
    data = data - 1;
    return data;
}

void
batom_bitstream_drop_bits(struct batom_bitstream *bs, unsigned nbits)
{
    do {
        unsigned to_drop = nbits;
        if(to_drop>31) {
            to_drop = 31;
        }
        nbits -= to_drop;
        batom_bitstream_bits(bs, to_drop);
        if(BATOM_IS_EOF(bs->cursor)) {
            break;
        }
    } while(nbits>0);

    return;
}

bool
batom_bitstream_reserve(const batom_bitstream *bs, unsigned nbits)
{
    batom_bitstream bs_peek = *bs;
    batom_cursor cursor_peek;

    BATOM_CLONE(&cursor_peek,bs->cursor);
    bs_peek.cursor = &cursor_peek;
    batom_bitstream_drop_bits(&bs_peek, nbits);
    return !batom_bitstream_eof(&bs_peek);
}

batom_pipe_t
batom_pipe_create(batom_factory_t factory)
{
	batom_pipe_t pipe;

	BDBG_OBJECT_ASSERT(factory, batom_factory_t);

	pipe = factory->alloc->bmem_alloc(factory->alloc, sizeof(*pipe));
	if (!pipe) {
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(pipe, batom_pipe_t);
	BLST_SQ_INIT(&pipe->atoms);
	pipe->push_cnt = 0;
	pipe->pop_cnt = 0;
	pipe->factory = factory;
err_alloc:
	return pipe;
}


void
batom_pipe_destroy(batom_pipe_t pipe)
{
	balloc_iface_t alloc;

	BDBG_OBJECT_ASSERT(pipe, batom_pipe_t);
	batom_pipe_flush(pipe);
	alloc = pipe->factory->alloc;
	BDBG_OBJECT_DESTROY(pipe, batom_pipe_t);
	alloc->bmem_free(alloc, pipe);
	return;
}

void
batom_pipe_push(batom_pipe_t pipe, batom_t atom)
{
	BDBG_MSG_TRACE(("batom_pipe_push: >%#lx %#lx", (unsigned long)pipe, (unsigned long)atom));
	BDBG_OBJECT_ASSERT(pipe, batom_pipe_t);
	BDBG_ASSERT(atom);
	BDBG_ASSERT(BLST_SQ_NEXT(atom, pipe_link)==NULL);
	BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
	BLST_SQ_INSERT_TAIL(&pipe->atoms, atom, pipe_link);
	pipe->push_cnt++;
	BDBG_MSG_TRACE(("batom_pipe_push: <%#lx:%u %#lx", (unsigned long)pipe, pipe->push_cnt-pipe->pop_cnt, (unsigned long)atom));
	return;
}

batom_t
batom_pipe_peek(batom_pipe_t pipe)
{
    BDBG_OBJECT_ASSERT(pipe, batom_pipe_t);
    BDBG_MSG_TRACE(("batom_pipe_peek: >%#lx:%u %#lx", (unsigned long)pipe, pipe->push_cnt-pipe->pop_cnt, BLST_SQ_FIRST(&pipe->atoms)));
    return BLST_SQ_FIRST(&pipe->atoms);
}

void
batom_pipe_drop(batom_pipe_t pipe)
{
	batom_t atom;
	atom = BLST_SQ_FIRST(&pipe->atoms);
	if(atom) {
		BLST_SQ_REMOVE_HEAD(&pipe->atoms, pipe_link);
		BLST_SQ_NEXT(atom, pipe_link) = NULL;
		BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
		pipe->pop_cnt++;
	} else {
		BDBG_ASSERT(0);
	}
	BDBG_MSG_TRACE(("batom_pipe_drop: <%#lx:%u", (unsigned long)pipe, pipe->push_cnt-pipe->pop_cnt));
	return;
}

batom_t
batom_pipe_pop(batom_pipe_t pipe)
{
	batom_t atom;
	BDBG_MSG_TRACE(("batom_pipe_pop: >%#lx", (unsigned long)pipe));
	BDBG_OBJECT_ASSERT(pipe, batom_pipe_t);
	atom = BLST_SQ_FIRST(&pipe->atoms);
	if(atom) {
		BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
		BLST_SQ_REMOVE_HEAD(&pipe->atoms, pipe_link);
		BLST_SQ_NEXT(atom, pipe_link) = NULL;
		pipe->pop_cnt++;
	}
	BDBG_MSG_TRACE(("batom_pipe_pop: <%#lx:%u %#lx", (unsigned long)pipe, pipe->push_cnt-pipe->pop_cnt, (unsigned long)atom));
	return atom;
}

void
batom_pipe_flush(batom_pipe_t pipe)
{
	batom_t atom;

	BDBG_OBJECT_ASSERT(pipe, batom_pipe_t);
	while( NULL!=(atom=BLST_SQ_FIRST(&pipe->atoms))) {
		BLST_SQ_REMOVE_HEAD(&pipe->atoms, pipe_link);
		BLST_SQ_NEXT(atom, pipe_link) = NULL;
		BDBG_OBJECT_ASSERT(atom->factory, batom_factory_t);
		pipe->pop_cnt++;
		batom_release(atom);
	}
	if(pipe->pop_cnt!=pipe->push_cnt) {
		BDBG_WRN(("batom_pipe_flush: runaway atoms: %#lx %u:%u", (unsigned long)pipe, pipe->push_cnt, pipe->pop_cnt));
	}
	return;
}

#define B_ATOM_DUMP_MAX	256
static void 
b_atom_cursor_dump(batom_cursor *c, const char *func, const char *name)
{
#if BDBG_DEBUG_BUILD
	uint8_t line[16];
	char split[16];
	unsigned i,j;
	unsigned off;
	unsigned addr;
	char str[80];

	BDBG_WRN(("%s: %s len: %u bytes vecs:%u left:%d pos:%d(%p:%u)", func, name, (unsigned)batom_cursor_size(c), c->count, c->left, c->pos-1, (void *)c->cursor, c->pos>0?(unsigned)((uint8_t *)c->cursor - (uint8_t *)c->vec[c->pos-1].base):0));
	for(i=0;i<c->count;i++) {
		BDBG_WRN(("%s: %s vec:%u:%#lx:%u", func, name,  i, (unsigned long)c->vec[i].base, (unsigned)c->vec[i].len));
	}
	for(addr=0;!BATOM_IS_EOF(c) && addr<B_ATOM_DUMP_MAX;addr+=sizeof(line)) {
		for(i=0;i<sizeof(line);i++) {
			uint16_t last_pos = c->pos;
			int b = batom_cursor_next(c);

			if(b==BATOM_EOF) {
				break;
			}
			line[i]=b;
			split[i]= (last_pos==c->pos)?' ':'|';
		}
        if(i==0) {
            break;
        }
		for(off=0,j=0;off<sizeof(str)&&j<i;j++) {
			off+=BKNI_Snprintf(str+off,sizeof(str)-off, "%c%02x", split[j], line[j]);
			if(j==7 && off<sizeof(str)) {
				off+=BKNI_Snprintf(str+off,sizeof(str)-off, " .");
			}
		}
        BDBG_WRN(("%s: %s 0x%04x:%s", func, name, addr, str));
	}
#else /* BDBG_DEBUG_BUILD */
	BSTD_UNUSED(c);
	BSTD_UNUSED(name);
	BSTD_UNUSED(func);
#endif
	return;
}

void
batom_accum_dump(batom_accum_t acc, const char *name)
{
    batom_cursor c;

    batom_cursor_from_accum(&c, acc);
    b_atom_cursor_dump(&c, "batom_accum_t", name);
    return;
}

void
batom_dump(batom_t atom, const char *name)
{
    batom_cursor c;
    BDBG_ASSERT(atom);

#if BDBG_DEBUG_BUILD
    BDBG_WRN(("batom_dump: %s:%p %s ref_cnt:%u len:%u vecs:%u", name, (void *)atom, atom->atom_type==B_ATOM_TYPE_CLONE?"clone":"", atom->ref_cnt, (unsigned)batom_len(atom), batom_get_nvecs(atom)));
#endif

    batom_cursor_from_atom(&c, atom);
    b_atom_cursor_dump(&c, "batom_t", name);
    return;
}

void
batom_cursor_dump(const batom_cursor *src, const char *name)
{
    batom_cursor c;

    BATOM_CLONE(&c, src);
    b_atom_cursor_dump(&c, "batom_cursor", name);
    return;
}

void batom_range_dump(const void *data, size_t length, const char *name)
{
    batom_vec vec;
    batom_cursor c;

    BATOM_VEC_INIT(&vec, data, length);
    batom_cursor_from_vec(&c, &vec, 1);
    b_atom_cursor_dump(&c, "range", name);
    return;
}

void
batom_vec_init(batom_vec *vec, const void *base, size_t len)
{
    B_ATOM_VEC_INIT(vec, base, len);
}

int
batom_cursor_memcmp(const batom_cursor *_cursor, const void *data, size_t n)
{
    batom_cursor cursor;
    const char *str=data;
    unsigned i;

    BATOM_CLONE(&cursor, _cursor);
    for(i=0;i<n;i++) {
        char ch = str[i];
        int data;

        BATOM_NEXT(data, &cursor);
        if(data==BATOM_EOF) {
            return 0 - ch;
        } if(data!=ch) {
            return data - ch;
        }
    }
    return 0;
}
