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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/kallsyms.h>

#include <epivers.h>

#define DUMPBUFSZ 1024

struct device *gp_stbsoc_dev;

struct stbsoc_context {
    void __iomem *regs;
    int irq;
    int wowl_irq;
    struct platform_device *pdev;
};

int (*wl_resume_normalmode)(void);

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

#if 0 //#ifdef BCMDBG_PKT
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

#if 0 //#ifdef BCMDBG_MEM
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

	printf("Broadcom BCM7271 Wireless Platform Module\n"
		"TagInfo:%s\n",
		EPI_VERSION_TAG);

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

EXPORT_SYMBOL(gp_stbsoc_dev);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Broadcom stbsoc WLAN DEVICE SHIM");
MODULE_AUTHOR("Broadcom");
