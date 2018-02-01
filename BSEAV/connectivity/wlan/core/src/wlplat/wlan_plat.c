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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/kallsyms.h>

struct device *gp_stbsoc_dev;


int (*wl_resume_normalmode)(void);

struct stbsoc_context {
    void __iomem *regs;
    int irq;
    int wowl_irq;
    struct platform_device *pdev;
};

static irqreturn_t wowl_isr(int irq, void *dev)
{
    struct stbsoc_context *priv = dev;
    char *sym_name = "wl_resume_normalmode";
    unsigned long sym_addr = kallsyms_lookup_name(sym_name);
    char filename[256];

    strncpy(filename, (char *)sym_addr, 255);
    pm_wakeup_event(&priv->pdev->dev, 0);
    printk("#####wowl_isr######\n\n\n\n\n");
	wl_resume_normalmode = (void*)sym_addr;
	if(wl_resume_normalmode)
		(*wl_resume_normalmode)();
	else
		printk("SYM not available \n\n");
    return 0;
}

static int stbsoc_wlan_remove(struct platform_device *pdev)
{
    int ret;
    struct stbsoc_context *priv;

    priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);

    priv->pdev = pdev;
    priv->wowl_irq = platform_get_irq(pdev, 1);

    disable_irq_wake(priv->wowl_irq);

    ret = device_set_wakeup_enable(&pdev->dev, 0);
    device_set_wakeup_capable(&pdev->dev, 0);
    return 0;
}

static int stbsoc_wlan_probe(struct platform_device *pdev)
{
    struct stbsoc_context *priv;
    struct resource *r;
    int ret;

    priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    priv->regs = devm_ioremap_resource(&pdev->dev, r);

    if (IS_ERR(priv->regs))
        return PTR_ERR(priv->regs);
    priv->pdev = pdev;
    priv->irq = platform_get_irq(pdev, 0);
    priv->wowl_irq = platform_get_irq(pdev, 1);

    if (priv->irq < 0 || priv->wowl_irq < 0)
        return -EINVAL;

    ret = devm_request_irq(&pdev->dev, priv->wowl_irq, wowl_isr, IRQF_NO_SUSPEND, "wlan_wol", priv);

    device_set_wakeup_capable(&pdev->dev, 1);

	gp_stbsoc_dev = &pdev->dev;

    ret = device_set_wakeup_enable(&pdev->dev, 1);
    enable_irq_wake(priv->wowl_irq);
    return 0;
}


static const struct of_device_id stbsoc_wlan_of_match[] = {
    { .compatible = "brcm,bcm7271-wlan" },
    { /* sentinel */ },
};

static struct platform_driver stbsoc_wlan_driver = {
    .probe	= stbsoc_wlan_probe,
    .remove = stbsoc_wlan_remove,
    .driver = {
        .name = "bcm7271-wlan",
        .of_match_table = stbsoc_wlan_of_match,
    },
};

static int __init stbsoc_wlan_init(void)
{
    return platform_driver_register(&stbsoc_wlan_driver);
}
module_init(stbsoc_wlan_init);

static void __exit stbsoc_wlan_exit(void)
{
    platform_driver_unregister(&stbsoc_wlan_driver);
}
module_exit(stbsoc_wlan_exit);


#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ethtool.h>
#include <linux/completion.h>
#include <linux/random.h>


#include "osl_decl.h"
#include "osl.h"
#include "linux_osl_priv.h"
#include "linux_osl.h"

/*================================================================*/
/* Linux Kernel: OSL attach/detach: start */
#define OS_ABS 1
#ifdef OS_ABS
void *osl_getdev(osl_t *osh);

void *
osl_getdev(osl_t *osh)
{
	if (osh == NULL)
		return NULL;
	return((void*)osh->pdev);
}
EXPORT_SYMBOL(osl_getdev);

osl_t *
osl_attach(void *pdev, uint bustype, bool pkttag)
{
	void **osl_cmn = NULL;
	osl_t *osh;

//HACKHACK
	/*printk("\n================%s %s\n", __FILE__, __TIME__);*/

	if (!(osh = kmalloc(sizeof(osl_t), GFP_ATOMIC)))
		return osh;


	memset(osh, '\0', sizeof(osl_t));

	if (osl_cmn == NULL) {
		if (!(osh->cmn = kmalloc(sizeof(osl_cmn_t), GFP_ATOMIC))) {
			kfree(osh);
			return NULL;
		}

		memset(osh->cmn, '\0', sizeof(osl_cmn_t));
		if (osl_cmn) {
			*osl_cmn = osh->cmn;
		}
		atomic_set(&osh->cmn->malloced, 0);
		osh->cmn->dbgmem_list = NULL;
		spin_lock_init(&(osh->cmn->dbgmem_lock));

#ifdef BCMDBG_PKT
		spin_lock_init(&(osh->cmn->pktlist_lock));
#endif
		spin_lock_init(&(osh->cmn->pktalloc_lock));

	} else {
		osh->cmn = *osl_cmn;
	}
	atomic_add(1, &osh->cmn->refcount);

	if (pdev==NULL)
		pdev = gp_stbsoc_dev;
	osh->failed = 0;
	osh->pdev = pdev;
	osh->pub.pkttag = pkttag;
	osh->bustype = bustype;
	osh->magic = OS_HANDLE_MAGIC;
	osh->pub.mmbus = true;

	return osh;
}
EXPORT_SYMBOL(osl_attach);

void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;

#ifdef BCMDBG_MEM
	if (MEMORY_LEFTOVER(osh)) {
		static char dumpbuf[DUMPBUFSZ];
		struct bcmstrbuf b;

		printf("%s: MEMORY LEAK %d bytes\n", __FUNCTION__, MALLOCED(osh));
		bcm_binit(&b, dumpbuf, DUMPBUFSZ);
		MALLOC_DUMP(osh, &b);
		printf("%s", b.origbuf);
	}
#endif

//	bcm_object_trace_deinit();

	ASSERT(osh->magic == OS_HANDLE_MAGIC);
	atomic_sub(1, &osh->cmn->refcount);
	if (atomic_read(&osh->cmn->refcount) == 0) {
			kfree(osh->cmn);
	}
	kfree(osh);
}
EXPORT_SYMBOL(osl_detach);
/* Linux Kernel: OSL attach/detach: end */
/*================================================================*/
#endif /* OS_ABS */


#define OS_ABS_FILE 1
#ifdef OS_ABS_FILE
/*================================================================*/
/* Linux Kernel: File Operations: start */
#include <linux/fs.h>

#ifdef __cplusplus
extern "C" {
#endif
void * osl_os_open_image(char * filename);
int osl_os_get_image_block(char * buf, int len, void * image);
void osl_os_close_image(void * image);
int osl_os_image_size(void *image);
#ifdef __cplusplus
}
#endif

void *
osl_os_open_image(char *filename)
{
	struct file *fp;

	fp = filp_open(filename, O_RDONLY, 0);
	/*
	 * 2.6.11 (FC4) supports filp_open() but later revs don't?
	 * Alternative:
	 * fp = open_namei(AT_FDCWD, filename, O_RD, 0);
	 * ???
	 */
	 if (IS_ERR(fp))
		 fp = NULL;

	 return fp;
}
EXPORT_SYMBOL(osl_os_open_image);

int
osl_os_get_image_block(char *buf, int len, void *image)
{
	struct file *fp = (struct file *)image;
	int rdlen;

	if (!image)
		return 0;

	rdlen = kernel_read(fp, fp->f_pos, buf, len);
	if (rdlen > 0)
		fp->f_pos += rdlen;

	return rdlen;
}
EXPORT_SYMBOL(osl_os_get_image_block);

void
osl_os_close_image(void *image)
{
	if (image)
		filp_close((struct file *)image, NULL);
}
EXPORT_SYMBOL(osl_os_close_image);

int
osl_os_image_size(void *image)
{
	int len = 0, curroffset;

	if (image) {
		/* store the current offset */
		curroffset = generic_file_llseek(image, 0, 1);
		/* goto end of file to get length */
		len = generic_file_llseek(image, 0, 2);
		/* restore back the offset */
		generic_file_llseek(image, curroffset, 0);
	}
	return len;
}
EXPORT_SYMBOL(osl_os_image_size);
/* Linux Kernel: File Operations: end */
/*================================================================*/
#endif /* OS_ABS_FILE */


#define OS_ABS_DMA 1
#ifdef OS_ABS_DMA
#include <asm/cacheflush.h>

#ifndef ISALIGNED
#define	ISALIGNED(a, x)		(((uintptr)(a) & ((x) - 1)) == 0)
#endif /* ISALIGNED */

/*================================================================*/
/* Linux Kernel: DMA Operations: start */

inline void BCMFASTPATH
osl_cache_flush(void *va, uint size)
{
	if (size > 0) {
		dma_sync_single_for_device(OSH_NULL, virt_to_phys(va), size, DMA_TX);
		//dma_sync_single_for_device(OSH_NULL, virt_to_dma(OSH_NULL, va), size,
		//	DMA_TO_DEVICE);
	}
}

inline void BCMFASTPATH
osl_cache_inv(void *va, uint size)
{
	dma_sync_single_for_cpu(OSH_NULL, virt_to_phys(va), size, DMA_RX);
	//dma_sync_single_for_cpu(OSH_NULL, virt_to_dma(OSH_NULL, va), size, DMA_FROM_DEVICE);
}

inline void BCMFASTPATH
osl_prefetch(const void *ptr)
{
}

uint
osl_dma_consistent_align(void)
{
	return (PAGE_SIZE);
}

void *
osl_virt_to_phys(void *va)
{
	return (void *)(uintptr)virt_to_phys(va);
}


void*
osl_dma_alloc_consistent(osl_t *osh, uint size, uint16 align_bits, uint *alloced, dmaaddr_t *pap)
{
	void *va;
	dma_addr_t pap_lin;
	uint16 align = (1 << align_bits);
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (!ISALIGNED(DMA_CONSISTENT_ALIGN, align))
		size += align;
	*alloced = size;

 #if 0
	va = kmalloc(size, GFP_ATOMIC | __GFP_ZERO);
	if (va)
		*pap = (ulong)__virt_to_phys((ulong)va);
#else
	va = dma_alloc_coherent(osh->pdev, size, &pap_lin, GFP_ATOMIC | __GFP_ZERO);
	*pap = (dmaaddr_t)pap_lin;
#endif

	return va;
}
EXPORT_SYMBOL(osl_dma_alloc_consistent);

void
osl_dma_free_consistent(osl_t *osh, void *va, uint size, dmaaddr_t pa)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

#if 0
	kfree(va);
#else
	dma_free_coherent(osh->pdev, size, va, pa);
#endif
	
}
EXPORT_SYMBOL(osl_dma_free_consistent);


void BCMFASTPATH
osl_dma_flush(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	if (direction == DMA_TX) { /* to device */
		osl_cache_flush(va, size);
	}
	else {
		printk("ERROR %s[%d]\n", __FUNCTION__, __LINE__);
	}
	return;
}
EXPORT_SYMBOL(osl_dma_flush);

dmaaddr_t BCMFASTPATH
osl_dma_map(osl_t *osh, void *va, uint size, int direction, void *p, hnddma_seg_map_t *dmah)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (size==0) {
		printf("va=%p size=%d\n", va, size);
		return virt_to_phys(va); /* just return the pa, size should not be zero */
	}
	
#if 0
//#if (__LINUX_ARM_ARCH__ == 8)
	/* need to flush or invalidate the cache here */
	if (direction == DMA_TX) { /* to device */
		osl_cache_flush(va, size);
	} else /*dir == DMA_RX*/ { /* from device */
		osl_cache_inv(va, size);
	} /* bidirectional not valid value for "dir" in this function */

	return virt_to_phys(va);
#else /* (__LINUX_ARM_ARCH__ == 8) */
	return dma_map_single(osh->pdev, va, size, direction);
#endif /* (__LINUX_ARM_ARCH__ == 8) */
}
EXPORT_SYMBOL(osl_dma_map);

void BCMFASTPATH
osl_dma_unmap(osl_t *osh, dmaaddr_t pa, uint size, int direction)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

#if 0
//#if (__LINUX_ARM_ARCH__ == 8)
	if (dir == DMA_TX) { /* to device */
		dma_sync_single_for_device(OSH_NULL, pa, size, DMA_TX);
	} else /*dir == DMA_RX*/ { /* from device */
		dma_sync_single_for_cpu(OSH_NULL, pa, size, DMA_RX);
	} /* bidirectional not valid value for "dir" in this function*/

#else /* (__LINUX_ARM_ARCH__ == 8) */
	dma_unmap_single(osh->pdev, (uintptr)pa, size, direction);
#endif /* (__LINUX_ARM_ARCH__ == 8) */
}
EXPORT_SYMBOL(osl_dma_unmap);



/* Linux Kernel: DMA Operations: end */
/*================================================================*/
#endif /* OS_ABS_DMA */




#define OS_ABS_MEM 1
#ifdef OS_ABS_MEM
/*================================================================*/
/* Linux Kernel: Memory Operations: start */

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
osl_check_memleak(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	if (atomic_read(&osh->cmn->refcount) == 1)
		return (atomic_read(&osh->cmn->malloced));
	else
		return 0;
}
EXPORT_SYMBOL(osl_check_memleak);


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


/* Linux Kernel: Memory Operations: end */
/*================================================================*/
#endif /* OS_ABS_MEM */
























EXPORT_SYMBOL(gp_stbsoc_dev);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Broadcom stbsoc WLAN SHIM");
MODULE_AUTHOR("Broadcom");
