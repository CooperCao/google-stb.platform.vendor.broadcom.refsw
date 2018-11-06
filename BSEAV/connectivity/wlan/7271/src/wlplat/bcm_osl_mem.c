/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

#include <typedefs.h>
#include <bcmendian.h>
#include <linuxver.h>
#include <bcmdefs.h>
#include <osl.h>
#include <osl_decl.h>
#include <linux_osl_priv.h>

void *
osl_malloc(osl_t *osh, uint size)
{
	void *addr;
	gfp_t flags;

	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

	flags = CAN_SLEEP() ? GFP_KERNEL: GFP_ATOMIC;
	if ((addr = kmalloc(size, flags)) == NULL) {
		if (osh)
			osh->failed++;
		return (NULL);
	}
	if (osh && osh->cmn)
		atomic_add(size, &osh->cmn->malloced);

	return (addr);
}
EXPORT_SYMBOL(osl_malloc);

void *
osl_mallocz(osl_t *osh, uint size)
{
	void *ptr;

	ptr = osl_malloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
EXPORT_SYMBOL(osl_mallocz);

void
osl_mfree(osl_t *osh, void *addr, uint size)
{
	if (osh && osh->cmn) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

		ASSERT(size <= osl_malloced(osh));

		atomic_sub(size, &osh->cmn->malloced);
	}
	kfree(addr);
}
EXPORT_SYMBOL(osl_mfree);

void *
osl_vmalloc(osl_t *osh, uint size)
{
	void *addr;

	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);
	if ((addr = vmalloc(size)) == NULL) {
		if (osh)
			osh->failed++;
		return (NULL);
	}
	if (osh && osh->cmn)
		atomic_add(size, &osh->cmn->malloced);

	return (addr);
}
EXPORT_SYMBOL(osl_vmalloc);

void *
osl_vmallocz(osl_t *osh, uint size)
{
	void *ptr;

	ptr = osl_vmalloc(osh, size);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
EXPORT_SYMBOL(osl_vmallocz);

void
osl_vmfree(osl_t *osh, void *addr, uint size)
{
	if (osh && osh->cmn) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

		ASSERT(size <= osl_malloced(osh));

		atomic_sub(size, &osh->cmn->malloced);
	}
	vfree(addr);
}
EXPORT_SYMBOL(osl_vmfree);

uint
osl_malloced(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (atomic_read(&osh->cmn->malloced));
}
EXPORT_SYMBOL(osl_malloced);

uint
osl_malloc_failed(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->failed);
}
EXPORT_SYMBOL(osl_malloc_failed);

uint
osl_check_memleak(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	if (atomic_read(&osh->cmn->refcount) == 1)
		return (atomic_read(&osh->cmn->malloced));
	else
		return 0;
}
EXPORT_SYMBOL(osl_check_memleak);

#define MEMLIST_LOCK(osh, flags)	spin_lock_irqsave(&(osh)->cmn->dbgmem_lock, flags)
#define MEMLIST_UNLOCK(osh, flags)	spin_unlock_irqrestore(&(osh)->cmn->dbgmem_lock, flags)
void *
osl_debug_malloc(osl_t *osh, uint size, int line, const char* file)
{
	bcm_mem_link_t *p;
	const char* basename;
	unsigned long flags = 0;
	if (!size) {
		printk("%s: allocating zero sized mem at %s line %d\n", __FUNCTION__, file, line);
		ASSERT(0);
	}

	if ((p = (bcm_mem_link_t*)osl_malloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL) {
		return (NULL);
	}

	if (osh) {
		MEMLIST_LOCK(osh, flags);
	}

	p->size = size;
	p->line = line;
	p->osh = (void *)osh;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	strncpy(p->file, basename, BCM_MEM_FILENAME_LEN);
	p->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	/* link this block */
	if (osh) {
		p->prev = NULL;
		p->next = osh->cmn->dbgmem_list;
		if (p->next)
			p->next->prev = p;
		osh->cmn->dbgmem_list = p;
		MEMLIST_UNLOCK(osh, flags);
	}

	return p + 1;
}
EXPORT_SYMBOL(osl_debug_malloc);

void *
osl_debug_mallocz(osl_t *osh, uint size, int line, const char* file)
{
	void *ptr;

	ptr = osl_debug_malloc(osh, size, line, file);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
EXPORT_SYMBOL(osl_debug_mallocz);

void
osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, const char* file)
{
	bcm_mem_link_t *p = (bcm_mem_link_t *)((int8*)addr - sizeof(bcm_mem_link_t));
	unsigned long flags = 0;

	ASSERT(osh == NULL || osh->magic == OS_HANDLE_MAGIC);

	if (p->size == 0) {
		printk("osl_debug_mfree: double free on addr %p size %d at line %d file %s\n",
			addr, size, line, file);
		ASSERT(p->size);
		return;
	}

	if (p->size != size) {
		printk("%s: dealloca size does not match alloc size\n", __FUNCTION__);
		printk("Dealloc addr %p size %d at line %d file %s\n", addr, size, line, file);
		printk("Alloc size %d line %d file %s\n", p->size, p->line, p->file);
		ASSERT(p->size == size);
		return;
	}

	if (osh && ((osl_t*)p->osh)->cmn != osh->cmn) {
		printk("osl_debug_mfree: alloc osh %p does not match dealloc osh %p\n",
			((osl_t*)p->osh)->cmn, osh->cmn);
		printk("Dealloc addr %p size %d at line %d file %s\n", addr, size, line, file);
		printk("Alloc size %d line %d file %s\n", p->size, p->line, p->file);
		ASSERT(((osl_t*)p->osh)->cmn == osh->cmn);
		return;
	}

	/* unlink this block */
	if (osh && osh->cmn) {
		MEMLIST_LOCK(osh, flags);
		if (p->prev)
			p->prev->next = p->next;
		if (p->next)
			p->next->prev = p->prev;
		if (osh->cmn->dbgmem_list == p)
			osh->cmn->dbgmem_list = p->next;
		p->next = p->prev = NULL;
	}
	p->size = 0;

	if (osh && osh->cmn) {
		MEMLIST_UNLOCK(osh, flags);
	}
	osl_mfree(osh, p, size + sizeof(bcm_mem_link_t));
}
EXPORT_SYMBOL(osl_debug_mfree);

void *
osl_debug_vmalloc(osl_t *osh, uint size, int line, const char* file)
{
	bcm_mem_link_t *p;
	const char* basename;
	unsigned long flags = 0;
	if (!size) {
		printk("%s: allocating zero sized mem at %s line %d\n", __FUNCTION__, file, line);
		ASSERT(0);
	}

	if ((p = (bcm_mem_link_t*)osl_vmalloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL) {
		return (NULL);
	}

	if (osh) {
		MEMLIST_LOCK(osh, flags);
	}

	p->size = size;
	p->line = line;
	p->osh = (void *)osh;

	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;

	strncpy(p->file, basename, BCM_MEM_FILENAME_LEN);
	p->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	/* link this block */
	if (osh) {
		p->prev = NULL;
		p->next = osh->cmn->dbgvmem_list;
		if (p->next)
			p->next->prev = p;
		osh->cmn->dbgvmem_list = p;
		MEMLIST_UNLOCK(osh, flags);
	}

	return p + 1;
}
EXPORT_SYMBOL(osl_debug_vmalloc);

void *
osl_debug_vmallocz(osl_t *osh, uint size, int line, const char* file)
{
	void *ptr;

	ptr = osl_debug_vmalloc(osh, size, line, file);

	if (ptr != NULL) {
		bzero(ptr, size);
	}

	return ptr;
}
EXPORT_SYMBOL(osl_debug_vmallocz);

void
osl_debug_vmfree(osl_t *osh, void *addr, uint size, int line, const char* file)
{
	bcm_mem_link_t *p = (bcm_mem_link_t *)((int8*)addr - sizeof(bcm_mem_link_t));
	unsigned long flags = 0;

	ASSERT(osh == NULL || osh->magic == OS_HANDLE_MAGIC);

	if (p->size == 0) {
		printk("osl_debug_mfree: double free on addr %p size %d at line %d file %s\n",
			addr, size, line, file);
		ASSERT(p->size);
		return;
	}

	if (p->size != size) {
		printk("%s: dealloca size does not match alloc size\n", __FUNCTION__);
		printk("Dealloc addr %p size %d at line %d file %s\n", addr, size, line, file);
		printk("Alloc size %d line %d file %s\n", p->size, p->line, p->file);
		ASSERT(p->size == size);
		return;
	}

	if (osh && ((osl_t*)p->osh)->cmn != osh->cmn) {
		printk("osl_debug_mfree: alloc osh %p does not match dealloc osh %p\n",
			((osl_t*)p->osh)->cmn, osh->cmn);
		printk("Dealloc addr %p size %d at line %d file %s\n", addr, size, line, file);
		printk("Alloc size %d line %d file %s\n", p->size, p->line, p->file);
		ASSERT(((osl_t*)p->osh)->cmn == osh->cmn);
		return;
	}

	/* unlink this block */
	if (osh && osh->cmn) {
		MEMLIST_LOCK(osh, flags);
		if (p->prev)
			p->prev->next = p->next;
		if (p->next)
			p->next->prev = p->prev;
		if (osh->cmn->dbgvmem_list == p)
			osh->cmn->dbgvmem_list = p->next;
		p->next = p->prev = NULL;
	}
	p->size = 0;

	if (osh && osh->cmn) {
		MEMLIST_UNLOCK(osh, flags);
	}
	osl_vmfree(osh, p, size + sizeof(bcm_mem_link_t));
}
EXPORT_SYMBOL(osl_debug_vmfree);
