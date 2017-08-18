/*
 * Broadcom PCI-SPI Host Controller Driver for Linux User Mode access
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <linuxver.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/ioctl.h>
#include <linux/time.h>
#include <linux/fs.h>

#include <asm/uaccess.h>

#include <typedefs.h>
#include <bcmpcispi.h>

#include "h2spi.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20))
#define IRQ_SHARED	IRQF_SHARED
#define CALLBACK_ARGS	int irq, void* dev_id
#else
#define IRQ_SHARED	SA_SHIRQ
#define CALLBACK_ARGS	int irq, void *dev_id, struct pt_regs *ptregs
#endif

#define	PCI_VENDOR_ID_BROADCOM		0x14e4
#define PCI_DEVICE_ID_SDIOH_FPGA_ID	0x43f5

#define H2SPI_MAJOR			230
#define H2SPI_MINOR_BASE		0

#define H2SPI_MAXDEV			4

#define PCI_CLOCK			66660000

MODULE_AUTHOR("Curt McDowell <csm@broadcom.com>");
MODULE_DESCRIPTION("Howard Harte's SPI controller");

#define DRV_MODULE_NAME			"h2spi"

#define PFX				"h2spi: "

#define SWAP16S(x)			(((x) >> 8) | ((x) << 8))
#define SWAP16L(x)			((((x) & 0xff00ff00) >> 8) | (((x) & 0x00ff00ff) << 8))
#define SWAP32(x)			((((x) & 0xff000000) >> 24) | \
					 (((x) & 0x00ff0000) >>  8) | \
					 (((x) & 0x0000ff00) <<  8) | \
					 (((x) & 0x000000ff) << 24))

#define CLK_DIV_MIN			1
#define CLK_DIV_MAX			2048

#define CLK_INTERNAL			0	/* Index into clockbase[] */
#define CLK_EXTERNAL			1

#define HARDWARE_SWAP(hp)		((hp)->boardrev >= 8)
#define WORDLEN_VALID(hp, len)		((len == 4) || \
					 ((hp)->boardrev >= 8 && ((len) == 1 || (len) == 2)))
#define BIG_PCI_CORE(hp)		((hp)->boardrev < 10)

static int h2spi_debug = 0;
module_param(h2spi_debug, int, 0);

static uint32 h2spi_clock = PCI_CLOCK / 4;	/* Default 16.67MHz */
module_param(h2spi_clock, int, 0);

static uint32 h2spi_cpol = 0;
module_param(h2spi_cpol, int, 0);

static uint32 h2spi_cpha = 0;
module_param(h2spi_cpha, int, 0);

static uint32 h2spi_spol = 0;
module_param(h2spi_spol, int, 0);	/* Default active low */

static uint32 h2spi_pwrctl = 0;
module_param(h2spi_pwrctl, int, 0);	/* Controls slot pwr on device close() */

typedef struct h2spi_s {
	struct pci_dev *pdev;		/* NULL if not allocated */
	struct semaphore sem;		/* Device lock for safe multi-thread access */
	int use_count;			/* Number of times device opened (only 1 now allowed) */
	struct task_struct *owner;	/* Task to receive devsig (that which opened device) */
	uint32 boardrev;		/* H2SPI board revision from PCI revid */
	int swswap;			/* Swap data in S/W (used if swapping is turned on */
					/*   but H/W doesn't support it) */
	int wordlen;			/* Word length (1, 2 or 4) */
	int spol;			/* Chip select polarity (0 = active low) */
	int clockbase[2];		/* Internal and external (0 if none) clock frequency */
	int clock;			/* Actual clock from programmed source/divisor */
	spih_pciregs_t *pci_regs;	/* Big PCI core only */
	spih_regs_t *spi_regs;
	char msg[H2SPI_MAXLEN];		/* Message buffer, big enough for any boardrev */
	int msgmax;			/* Maximum message length for this boardrev */
	int msglen;			/* Actual message length, -1 when not set */

	spinlock_t intlock;		/* Spinlock required to access intr-shared vars (below) */
	int intsig;			/* Signal to send on device interrupt (0 for none) */
	uint32 intmask;			/* Cached value of spih_int_mask register */
} h2spi_t;

static h2spi_t h2spi[H2SPI_MAXDEV];

static struct pci_device_id h2spi_pci_table[] __devinitdata = {
	{ PCI_VENDOR_ID_BROADCOM, PCI_DEVICE_ID_SDIOH_FPGA_ID,
	PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, h2spi_pci_table);

static void
hexdump(const char *pfx, const uchar *msg, int msglen)
{
	int i, col;
	char buf[80];

	col = 0;

	for (i = 0; i < msglen; i++, col++) {
		if (col % 16 == 0)
			strcpy(buf, pfx);
		sprintf(buf + strlen(buf), "%02x", msg[i]);
		if ((col + 1) % 16 == 0)
			printk("%s\n", buf);
		else
			sprintf(buf + strlen(buf), " ");
	}

	if (col % 16 != 0)
		printk("%s\n", buf);
}

static int
div_to_espr(uint32 *espr, int div)
{
	switch (div) {
	case 1:		*espr = 0x0; break;
	case 2:		*espr = 0x1; break;
	case 4:		*espr = 0x2; break;
	case 8:		*espr = 0x5; break;	/* Note: not plain log2 */
	case 16:	*espr = 0x3; break;
	case 32:	*espr = 0x4; break;
	case 64:	*espr = 0x6; break;
	case 128:	*espr = 0x7; break;
	case 256:	*espr = 0x8; break;
	case 512:	*espr = 0x9; break;
	case 1024:	*espr = 0xa; break;
	case 2048:	*espr = 0xb; break;
	default:	return -1;
	}

	return 0;
}

static int
ksleep(int jiffies)
{
	wait_queue_head_t delay_wait;
	DECLARE_WAITQUEUE(wait, current);
	int pending;

	init_waitqueue_head(&delay_wait);
	add_wait_queue(&delay_wait, &wait);
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(jiffies);
	pending = signal_pending(current);
	remove_wait_queue(&delay_wait, &wait);
	set_current_state(TASK_RUNNING);

	return pending;
}

/*
 * Set the clock source (internal vs external) and divisor to get
 * as close as possible to a requested frequency.
 */

static int
setclock(h2spi_t *hp, int freq, int dont_go_over)
{
	int source, div, error;
	int best_source, best_div, best_error;
	int f;
	uint32 espr, t;

	best_source = -1;
	best_div = -1;
	best_error = 999999999;

	for (source = CLK_INTERNAL; source <= CLK_EXTERNAL; source++) {
		if (hp->clockbase[source] == 0)
			continue;

		for (div = CLK_DIV_MIN; div <= CLK_DIV_MAX; div *= 2) {
			f = hp->clockbase[source] / div;

			if (dont_go_over && f > freq)
				continue;

			error = (f > freq) ? (f - freq) : (freq - f);

			if (error < best_error) {
				best_source = source;
				best_div = div;
				best_error = error;
			}
		}
	}

	if (best_source < 0 || best_div < 0)
		return -1;

	if (hp->boardrev >= 5) {
		writel((best_source == 0) ? 0 : SPIH_EXT_CLK, &hp->spi_regs->spih_pll_ctrl);
		ksleep(HZ / 10);
		if (!(readl(&hp->spi_regs->spih_pll_status) & SPIH_PLL_LOCKED))
			return -1;
	}

	if (div_to_espr(&espr, best_div) < 0)
		return -1;

	t = readl(&hp->spi_regs->spih_ctrl);
	t &= ~3;
	t |= espr & 3;
	writel(t, &hp->spi_regs->spih_ctrl);

	t = readl(&hp->spi_regs->spih_ext);
	t &= ~3;
	t |= (espr >> 2) & 3;
	writel(t, &hp->spi_regs->spih_ext);

	hp->clock = hp->clockbase[best_source] / best_div;

	return 0;
}

static void
setbswap(h2spi_t *hp, int bswap)
{
	if (!HARDWARE_SWAP(hp))
		hp->swswap = bswap ? 1 : 0;
	else {
		uint32 t;

		t = readl(&hp->spi_regs->spih_ext);
		if (bswap)
			t |= 32;
		else
			t &= ~32;

		writel(t, &hp->spi_regs->spih_ext);
	}
}

static int
getbswap(h2spi_t *hp)
{
	int rv;

	if (!HARDWARE_SWAP(hp))
		rv = hp->swswap;
	else {
		uint32 t;

		t = readl(&hp->spi_regs->spih_ext);

		rv = ((t & 32) != 0);
	}

	return rv;
}

static void
setintenable(h2spi_t *hp, int enable)
{
	if (enable) {
		/*
		 * Clear interrupt in FPGA.  If a device interrupt is pending,
		 * the FPGA interrupt will be reasserted immediately.
		 */
		writel(SPIH_DEV_INTR, &hp->spi_regs->spih_int_status);

		/* Unmask device interrupt */
		hp->intmask |= SPIH_DEV_INTR;
		writel(hp->intmask, &hp->spi_regs->spih_int_mask);

		/* Flush */
		(void)readl(&hp->spi_regs->spih_int_mask);
	} else {
		/* Mask device interrupt */
		hp->intmask &= ~SPIH_DEV_INTR;
		writel(hp->intmask, &hp->spi_regs->spih_int_mask);
	}
}

static int
h2spi_open(struct inode *inode, struct file *file)
{
	int minor = MINOR(inode->i_rdev);
	h2spi_t *hp;
	uint32 t;

	if (h2spi_debug)
		printk(KERN_INFO PFX "%s: entered (minor=%d)\n", __FUNCTION__, minor);

	if (minor < H2SPI_MINOR_BASE || minor >= H2SPI_MINOR_BASE + H2SPI_MAXDEV)
		return -ENODEV;

	hp = &h2spi[minor - H2SPI_MINOR_BASE];

	down(&hp->sem);

	if (hp->pdev == NULL) {
		up(&hp->sem);
		return -ENODEV;
	}

	if (hp->use_count != 0) {
		up(&hp->sem);
		return -EBUSY;
	}

	hp->msglen = -1;
	hp->msgmax = (hp->boardrev >= 12) ? 4096 : 2048;
	hp->intsig = 0;
	hp->intmask = 0;
	hp->spol = h2spi_spol;

	hp->owner = current;

	/* Enable SPI Controller, CPOL=0, CPHA=0 */
	t = 0x000000d1;
	if (h2spi_cpol)
		t |= 0x8;
	if (h2spi_cpha)
		t |= 0x4;
	writel(t, &hp->spi_regs->spih_ctrl);

	/* Initialize spih_ext */
	writel(0x00000000, &hp->spi_regs->spih_ext);

	/* Set GPIO CS# de-asserted */
	if (hp->spol)
		writel(0x00000000, &hp->spi_regs->spih_gpio_data);	/* active high */
	else
		writel(0x00000001, &hp->spi_regs->spih_gpio_data);	/* active low */

	/*
	 * Enable GPIO[5:0] as outputs.
	 *   set GPIO[0] to output for CS#
	 *   set GPIO[1] to output for power control (low = on)
	 *   set GPIO[2] to input for card detect (low = present)
	 */
	if (h2spi_pwrctl) {
		writel(0x00000000, &hp->spi_regs->spih_gpio_ctrl);
		/* Wait */
		ksleep(HZ / 10);
	}

	writel(0x00000003, &hp->spi_regs->spih_gpio_ctrl);

	/* Wait 0.1 seconds for power to stabilize to the SDIO Card */
	ksleep(HZ / 10);

	/* Check card detect */
	if (hp->boardrev >= 4 &&
	    (readl(&hp->spi_regs->spih_gpio_data) & SPIH_CARD_DETECT) != 0) {
		printk(KERN_ERR PFX "No card detected in SD slot\n");
		up(&hp->sem);
		return -ENXIO;
	}

	/* Interrupts are level-sensitive */
	writel(0x80000000, &hp->spi_regs->spih_int_edge);

	/* Interrupts are active low */
	writel(0x40000000, &hp->spi_regs->spih_int_pol);

	/* Interrupts are initially masked */
	writel(0x00000000, &hp->spi_regs->spih_int_mask);

	/* Write display roughly to "GOOD" */
	writel(0x0000900d, &hp->spi_regs->spih_hex_disp);

	/* On second thought, show the current consumption on the display :-) */
	writel(0x00000001, &hp->spi_regs->spih_disp_sel);

	/* Internal clock frequency is fixed */
	hp->clockbase[CLK_INTERNAL] = PCI_CLOCK / 2;

	/* See if there is an external clock, and if so, determine its frequency */
	hp->clockbase[CLK_EXTERNAL] = 0;
	if (hp->boardrev >= 5 &&
	    readl(&hp->spi_regs->spih_clk_count) != readl(&hp->spi_regs->spih_clk_count)) {
		writel(SPIH_EXT_CLK, &hp->spi_regs->spih_pll_ctrl);
		ksleep(HZ / 10);
		if (readl(&hp->spi_regs->spih_pll_status) & SPIH_PLL_LOCKED)
			hp->clockbase[CLK_EXTERNAL] =
			        readl(&hp->spi_regs->spih_xtal_freq) * 10000;
	}

	/* Set clock */
	if (setclock(hp, h2spi_clock, 0) < 0)
		return -EINVAL;

	if (BIG_PCI_CORE(hp)) {
	    /* Enable interrupts through PCI Core */
	    writel(0x00000001, &hp->pci_regs->ICR);
	}

	file->private_data = hp;

	hp->use_count++;

	OLD_MOD_INC_USE_COUNT;

	up(&hp->sem);

	return 0;
}

static void
h2spi_hw_idle(h2spi_t *hp)
{
	/* Hardware clean-up */

	if (BIG_PCI_CORE(hp))
	    writel(0x00000000, &hp->pci_regs->ICR);

	writel(0x00000010, &hp->spi_regs->spih_ctrl);
	writel(h2spi_pwrctl ? 0x00000002 : 0x00000000, &hp->spi_regs->spih_gpio_ctrl);
	writel(0x00000000, &hp->spi_regs->spih_int_mask);
	writel(0x00000000, &hp->spi_regs->spih_int_edge);
	writel(0x00000000, &hp->spi_regs->spih_int_pol);
	writel(0x00000000, &hp->spi_regs->spih_hex_disp);
	writel(0x00000000, &hp->spi_regs->spih_disp_sel);
}

static int
h2spi_release(struct inode *inode, struct file *file)
{
	h2spi_t *hp = file->private_data;

	down(&hp->sem);

	h2spi_hw_idle(hp);

	hp->use_count--;

	OLD_MOD_DEC_USE_COUNT;

	up(&hp->sem);

	return 0;
}

/*
 * Read returns data that came back from previous SPI operation.  The
 * read length must be the same as the write length was.
 */
static ssize_t
h2spi_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	h2spi_t *hp = file->private_data;

	if (h2spi_debug)
		printk(KERN_INFO PFX "%s: entered (count=%d)\n", __FUNCTION__, (unsigned int)count);

	down(&hp->sem);

	if (hp->msglen < 0) {
		up(&hp->sem);
		return -ENOMSG;
	}

	if (count != hp->msglen) {
		up(&hp->sem);
		return -EINVAL;
	}

	if (copy_to_user(buf, hp->msg, count)) {
		up(&hp->sem);
		return -EFAULT;
	}

	hp->msglen = -1;	/* Transaction completed */

	up(&hp->sem);

	return count;
}

static ssize_t
h2spi_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	h2spi_t *hp = file->private_data;
	uint16 s;
	uint32 t;
	int words, i;

	if (h2spi_debug)
		printk(KERN_INFO PFX "%s: entered (count=%d)\n", __FUNCTION__, (unsigned)count);

	down(&hp->sem);

	/* This controller only handles multiples of 2 or 4 bytes */
	if (count < 0 || count > hp->msgmax || (count % hp->wordlen) != 0) {
		up(&hp->sem);
		return -EINVAL;
	}

	if (copy_from_user(hp->msg, buf, count)) {
		up(&hp->sem);
		return -EFAULT;
	}

	words = count / hp->wordlen;

	/* Assert GPIO CS# */
	t = readl(&hp->spi_regs->spih_gpio_data);
	if (hp->spol)
		t |= 0x00000001;	/* Asserted (active high) */
	else
		t &= ~0x00000001;	/* Asserted (active low) */
	writel(t, &hp->spi_regs->spih_gpio_data);

	/* Write output data to FIFO */
	switch (hp->wordlen) {
	case 1:
		for (i = 0; i < words; i++)
			writeb((uint8)hp->msg[i], &hp->spi_regs->spih_data);
		break;
	case 2:
		if (hp->swswap) {
			for (i = 0; i < words; i++) {
				s = ((uint16 *)hp->msg)[i];
				s = SWAP16S(s);
				writew(s, &hp->spi_regs->spih_data);
			}
		} else {
			for (i = 0; i < words; i++) {
				s = ((uint16 *)hp->msg)[i];
				writew(s, &hp->spi_regs->spih_data);
			}
		}
		break;
	case 4:
		if (hp->swswap) {
			for (i = 0; i < words; i++) {
				t = ((uint32 *)hp->msg)[i];
				t = SWAP32(t);
				writel(t, &hp->spi_regs->spih_data);
			}
		} else {
			for (i = 0; i < words; i++) {
				t = ((uint32 *)hp->msg)[i];
				writel(t, &hp->spi_regs->spih_data);
			}
		}
		break;
	}

	if (h2spi_debug)
		hexdump("OUT: ", (uchar *)hp->msg, count);

	/* Wait for write fifo to empty */
	do
	        t = readl(&hp->spi_regs->spih_stat);
	while ((t & 0x4) == 0);

	/* Read input data from FIFO */
	switch (hp->wordlen) {
	case 1:
		for (i = 0; i < words; i++)
			hp->msg[i] = (char)readb(&hp->spi_regs->spih_data);
		break;
	case 2:
		if (hp->swswap) {
			for (i = 0; i < words; i++) {
				s = readw(&hp->spi_regs->spih_data);
				s = SWAP16S(s);
				((uint16 *)hp->msg)[i] = s;
			}
		} else {
			for (i = 0; i < words; i++) {
				s = readw(&hp->spi_regs->spih_data);
				((uint16 *)hp->msg)[i] = s;
			}
		}
		break;
	case 4:
		if (hp->swswap) {
			for (i = 0; i < words; i++) {
				t = readl(&hp->spi_regs->spih_data);
				t = SWAP32(t);
				((uint32 *)hp->msg)[i] = t;
			}
		} else {
			for (i = 0; i < words; i++) {
				t = readl(&hp->spi_regs->spih_data);
				((uint32 *)hp->msg)[i] = t;
			}
		}
		break;
	}

	if (h2spi_debug)
		hexdump(" IN: ", (uchar *)hp->msg, count);

	/* De-assert GPIO CS# */
	t = readl(&hp->spi_regs->spih_gpio_data);
	if (hp->spol)
		t &= ~0x00000001;	/* De-asserted (active high) */
	else
		t |= 0x00000001;	/* De-asserted (active low) */
	writel(t, &hp->spi_regs->spih_gpio_data);

	/* Remember how much was written (require read to be same size) */
	hp->msglen = count;

	up(&hp->sem);

	return count;
}

static int
h2spi_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
            unsigned long arg)
{
	h2spi_t *hp = file->private_data;
	uint32 val, t;
	int rv = 0;

	if (h2spi_debug)
		printk(KERN_INFO PFX "%s: entered (cmd=0x%x)\n", __FUNCTION__, cmd);

	down(&hp->sem);

	switch (cmd) {
	case H2SPI_GBOARDREV:
		val = hp->boardrev;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case H2SPI_SCPOL:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else {
			t = readl(&hp->spi_regs->spih_ctrl);
			if (val)
				t |= 0x8;
			else
				t &= ~0x8;
			writel(t, &hp->spi_regs->spih_ctrl);
		}
		break;
	case H2SPI_GCPOL:
		t = readl(&hp->spi_regs->spih_ctrl);
		val = (t & 0x8) ? 1 : 0;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case H2SPI_SCPHA:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else {
			t = readl(&hp->spi_regs->spih_ctrl);
			if (val)
				t |= 0x4;
			else
				t &= ~0x4;
			writel(t, &hp->spi_regs->spih_ctrl);
		}
		break;
	case H2SPI_GCPHA:
		t = readl(&hp->spi_regs->spih_ctrl);
		val = (t & 0x4) ? 1 : 0;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case H2SPI_SSPOL:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			hp->spol = (val != 0);
		break;
	case H2SPI_GSPOL:
		val = hp->spol;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case H2SPI_SCLOCK:	/* Set clock as close as possible to requested value */
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else if (setclock(hp, val, 0))
			rv = -EINVAL;
		break;
	case H2SPI_SCLOCKMAX:	/* Set clock as close as possible, but without going over */
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else if (setclock(hp, val, 1))
			rv = -EINVAL;
		break;
	case H2SPI_GCLOCK:
		val = hp->clock;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case H2SPI_SDEVSIG:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else if (val >= _NSIG)
			rv = -EINVAL;
		else {
			ulong flags;
			/*
			 * Enable the interrupt.  When an interrupt is received, it
			 * will be turned into a signal sent to the owning process,
			 * and it will be masked until this ioctl enables it again.
			 */
			spin_lock_irqsave(&hp->intlock, flags);
			hp->intsig = val;
			setintenable(hp, hp->intsig != 0);
			spin_unlock_irqrestore(&hp->intlock, flags);
		}
		break;
	case H2SPI_GDEVSIG:
		val = hp->intsig;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case H2SPI_SBSWAP:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			setbswap(hp, val);
		break;
	case H2SPI_GBSWAP:
		val = getbswap(hp);
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case H2SPI_SWORDLEN:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else if (!WORDLEN_VALID(hp, val))
			rv = -EINVAL;
		else
			hp->wordlen = val;
		break;
	case H2SPI_GWORDLEN:
		val = hp->wordlen;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case H2SPI_GVOLTAGE:
		/* Not yet implemented */
		rv = -ENXIO;
		break;
	case H2SPI_GCURRENT:
		/* Not yet implemented */
		rv = -ENXIO;
		break;
	case H2SPI_SDISPMODE:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		writel(val, &hp->spi_regs->spih_disp_sel);
		break;
	case H2SPI_SDISPVAL:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		writel(val, &hp->spi_regs->spih_hex_disp);
		break;
	case H2SPI_SPWRCTL:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			h2spi_pwrctl = val;
		break;
	default:
		rv = -EINVAL;
	}

	up(&hp->sem);

	return rv;
}

/* Interrupt handler */
irqreturn_t
h2spi_isr(CALLBACK_ARGS)
{
	h2spi_t *hp = dev_id;
	uint32 raw_int, cur_int;
	int ours;

	raw_int = readl(&hp->spi_regs->spih_int_status);
	cur_int = raw_int & hp->intmask;

	if (cur_int & SPIH_DEV_INTR) {
		/* Mask the interrupt */
		hp->intmask &= ~SPIH_DEV_INTR;
		writel(hp->intmask, &hp->spi_regs->spih_int_mask);

		/* If configured, deliver signal to controlling process */
		if (hp->use_count > 0 && hp->intsig != 0)
			send_sig(hp->intsig, hp->owner, 0);

		ours = 1;
	} else if (cur_int & SPIH_CTLR_INTR) {
		/* Clear the interrupt in the SPI_STAT register */
		writel(0x00000080, &hp->spi_regs->spih_stat);

		/* Ack the interrupt in the interrupt controller */
		writel(SPIH_CTLR_INTR, &hp->spi_regs->spih_int_status);

		/* Flush */
		(void)readl(&hp->spi_regs->spih_int_status);

		ours = 1;
	} else {
		/* Not an error: can share interrupts... */
		ours = 0;
	}

	return IRQ_RETVAL(ours);
}

static struct file_operations h2spi_fops = {
	owner:		THIS_MODULE,
	read:		h2spi_read,
	write:		h2spi_write,
	ioctl:		h2spi_ioctl,
	open:		h2spi_open,
	release:	h2spi_release,
};


static int __devinit h2spi_pci_probe(struct pci_dev *pdev,
                                     const struct pci_device_id *ent)
{
	unsigned long reg_base, reg_len;
	int err, dev_index;
	h2spi_t *hp;
	uint32 t;
	int devno;

	if (h2spi_debug)
		printk(KERN_INFO PFX "%s: entered\n", __FUNCTION__);

	if ((err = pci_enable_device(pdev)) != 0) {
		printk(KERN_ERR PFX "Cannot enable PCI device; aborting\n");
		return err;
	}

	if ((err = pci_request_regions(pdev, DRV_MODULE_NAME)) != 0) {
		printk(KERN_ERR PFX "Cannot obtain PCI resources; aborting\n");
		goto fail_disable_pdev;
	}

	pci_set_master(pdev);

	for (dev_index = 0; dev_index < H2SPI_MAXDEV; dev_index++)
		if (h2spi[dev_index].pdev == NULL)
			break;

	if (dev_index == H2SPI_MAXDEV) {
		printk(KERN_ERR PFX "Too many devices; aborting\n");
		err = -ENOMEM;
		goto fail_free_regions;
	}

	hp = &h2spi[dev_index];

	down(&hp->sem);

	hp->pdev = pdev;
	hp->use_count = 0;
	spin_lock_init(&hp->intlock);

	pci_read_config_dword(pdev, PCI_REVISION_ID, &t);

	hp->boardrev = t & 0xff;
	hp->swswap = 0;
	hp->wordlen = 4;

	printk(KERN_INFO PFX "Enabling device h2spi%d (rev %d)\n",
	       dev_index, hp->boardrev);

	devno = 0;

	/*
	 * The big PCI core is both a master and target.
	 * It has its own registers in BAR0 and SPI registers in BAR1.
	 *
	 * The small PCI core is target-only and happens to do PIO much faster.
	 * It has SPI registers in BAR0.
	 */
	if (BIG_PCI_CORE(hp)) {
	    reg_base = pci_resource_start(pdev, devno);
	    reg_len = pci_resource_len(pdev, devno);

	    if ((hp->pci_regs = ioremap_nocache(reg_base, reg_len)) == NULL) {
		printk(KERN_ERR PFX "Cannot map PCI core registers; aborting\n");
		err = -ENOMEM;
		goto fail_free_dev;
	    }

	    devno++;
	} else
	    hp->pci_regs = NULL;

	reg_base = pci_resource_start(pdev, devno);
	reg_len = pci_resource_len(pdev, devno);

	if ((hp->spi_regs = ioremap_nocache(reg_base, reg_len)) == NULL) {
		printk(KERN_ERR PFX "Cannot map SPI registers; aborting\n");
		err = -ENOMEM;
		goto fail_pci_unmap;
	}

	pci_set_drvdata(pdev, hp);

	if (request_irq(pdev->irq, h2spi_isr, IRQ_SHARED, "h2spi", hp) < 0) {
		printk(KERN_ERR PFX "Cannot request interrupt; aborting\n");
		err = -ENOMEM;
		goto fail_spi_unmap;
	}

	h2spi_hw_idle(hp);

	up(&hp->sem);

	return 0;

fail_spi_unmap:
	pci_set_drvdata(pdev, NULL);
	iounmap((void *)hp->spi_regs);

fail_pci_unmap:
	if (hp->pci_regs != NULL)
	    iounmap((void *)hp->pci_regs);

fail_free_dev:
	hp->pdev = NULL;
	up(&hp->sem);

fail_free_regions:
	pci_release_regions(pdev);

fail_disable_pdev:
	pci_disable_device(pdev);

	return err;
}

static void __devexit h2spi_pci_remove(struct pci_dev *pdev)
{
	h2spi_t *hp = pci_get_drvdata(pdev);

	if (h2spi_debug)
		printk(KERN_INFO PFX "%s: entered\n", __FUNCTION__);

	if (hp == NULL || hp->pdev != pdev) {
		printk(KERN_ERR PFX "Invalid PCI device %p\n", pdev);
		return;
	}

	/* Software clean-up */

	down(&hp->sem);

	free_irq(pdev->irq, hp);

	iounmap((void *)hp->spi_regs);
	if (hp->pci_regs != NULL)
	    iounmap((void *)hp->pci_regs);

	hp->pdev = NULL;

	up(&hp->sem);

	pci_release_regions(pdev);
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);
}

static struct pci_driver h2spi_driver = {
	.name		= DRV_MODULE_NAME,
	.id_table	= h2spi_pci_table,
	.probe		= h2spi_pci_probe,
	.remove		= __devexit_p(h2spi_pci_remove),
};

static int __init h2spi_init(void)
{
	int err, i;

	if ((err = register_chrdev(H2SPI_MAJOR, "h2spi", &h2spi_fops)) < 0) {
		printk(KERN_ERR PFX "Can't register chrdev h2spi; aborting\n");
		return err;
	}

	for (i = 0; i < H2SPI_MAXDEV; i++) {
		init_MUTEX(&h2spi[i].sem);
		h2spi[i].pdev = NULL;
	}

	return pci_module_init(&h2spi_driver);
}

static void __exit h2spi_cleanup(void)
{
	pci_unregister_driver(&h2spi_driver);

	unregister_chrdev(H2SPI_MAJOR, "h2spi");
}

module_init(h2spi_init);
module_exit(h2spi_cleanup);
