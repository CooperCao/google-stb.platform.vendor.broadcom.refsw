/*
 * Broadcom driver for Linux User Mode SDIO
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

#include <typedefs.h>
#include <linuxver.h>
#include <pcicfg.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/ioctl.h>
#include <linux/time.h>
#include <linux/fs.h>

#include <asm/uaccess.h>

#include <sdioh.h>
#include <sdio.h>

#include "umsd.h"

MODULE_AUTHOR("Curt McDowell <csm@broadcom.com>");
MODULE_DESCRIPTION("User Mode SDIO");

/*
 * Minor number mapping:
 *	umsd0 - umsd3: Controller 0, Slot 1 - Slot 4
 *	umsd4 - umsd7: Controller 1, Slot 1 - Slot 4
 */

#define UMSD_CTLR_MAX			2
#define UMSD_CTLR_SLOT_MAX		4

#define UMSD_MAJOR			232
#define UMSD_MINOR_BASE			0
#define UMSD_MINOR_COUNT		(UMSD_CTLR_MAX * UMSD_CTLR_SLOT_MAX)

#define MINOR_CTLR(minor)		(((minor) - UMSD_MINOR_BASE) / UMSD_CTLR_SLOT_MAX)
#define MINOR_CTLR_SLOT(minor)		(((minor) - UMSD_MINOR_BASE) % UMSD_CTLR_SLOT_MAX)

#define MIN(a, b)			(((a) < (b)) ? (a) : (b))
#define ROUNDUP(x, y)			((((x) + ((y) - 1)) / (y)) * (y))
#define ISPOW2(x)			((x) != 0 && ((x) & ((x) - 1)) == 0)

#define DRV_MODULE_NAME			"umsd"
#define SDIO_REG_WINSZ			0x100
#define XFER_BUF_LEN			8192
#define TIMEOUT_CONTROL			7
#define DMA_SIZE_THRESHOLD		64		/* Use PIO below this */
#define COMMAND_TIMEOUT			(HZ * 2)
#define TRANSFER_TIMEOUT		(HZ * 5)
#define CMD_INHIBIT_TRIES		200
#define DATA_INHIBIT_TRIES		200
#define CMD_COMPLETE_TRIES		200

#define DMA_MODE_NONE			0
#define DMA_MODE_SDMA			1
#define DMA_MODE_ADMA1			2
#define DMA_MODE_ADMA2			3
#define DMA_MODE_ADMA2_64		4
#define DMA_MODE_AUTO			-1

#define SD_PAGE_BITS			12
#define SD_PAGE				(1 << SD_PAGE_BITS)

#define err(fmt, arg...) do {								\
	printk(KERN_ERR "umsd: %s: " fmt, __FUNCTION__ , ## arg);			\
} while (0)

#define msg(fmt, arg...) do {								\
	printk(KERN_INFO "umsd: %s: " fmt, __FUNCTION__ , ## arg);			\
} while (0)

#define dbg(fmt, arg...) do {								\
	if (umsd_debug) printk(KERN_INFO "umsd: %s: " fmt, __FUNCTION__ , ## arg);	\
} while (0)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20))
#define CALLBACK_ARGS			int irq, void *dev_id
#else
#define CALLBACK_ARGS			int irq, void *dev_id, struct pt_regs *ptregs
#endif

static int umsd_debug = 0;
module_param(umsd_debug, int, 0);

static int umsd_dma_mode_force = -1;	/* (Note: Broadcom card doesn't support SDMA) */
module_param(umsd_dma_mode_force, int, 0);

typedef struct slot_s {
	/* Driver attach state */
	int ctlr;			/* Backpointer to controller */
	struct semaphore sem;		/* Device lock for safe multi-thread access */
	volatile unsigned char *regs;	/* Mapped region for host controller registers for slot */
	spinlock_t intlock;		/* Spinlock required to access intr-shared vars (below) */
	wait_queue_head_t intr_wait;	/* Queue for waiting for interrupt */
	unsigned int caps;		/* Host controller capabilities field */
	unsigned int curr_caps;		/* Host controller power supply current capabilities */
	int use_count;			/* Number of times device opened (only 1 allowed now) */

	/* Device open state */
	struct task_struct *owner;	/* Task to receive devsig (that which opened device) */
	int intr_pend_xc;		/* Transfer Complete interrupt received flag */
	int intr_pend_err;		/* Error interrupt received flag */
	int intsig;			/* Signal to send on device interrupt (0 for none) */
	unsigned int power;		/* Slot is powered up */
	unsigned int clkbase;		/* Base clock in Hz */
	unsigned int clkdiv;		/* Configured clock divisor (0 for off) */
	unsigned int width;		/* Configured bus width in bits */
	unsigned int hispeed;		/* Controller is configured for high speed mode */
	unsigned short blksize[UMSD_FUNC_MAX];	/* Driver configured block sizes per function */
	unsigned int dma_mode;		/* Driver is configured to use diff DMAs */
	void *xfer_buf;			/* DMA-safe buffer */
	dma_addr_t xfer_buf_pa;		/* DMA-safe buffer physical address */

	/* ADMA2 DMA state */
	void *adma2_dscr_buf;		/* ADMA2 Descriptor Buffer virtual address */
	unsigned int adma2_dscr_phys;	/* ADMA2 Descriptor Buffer physical address */
	/* adjustments needed to make the dma align properly */
	void *adma2_dscr_start_buf;	/* Unaligned DMA buffer */
	dma_addr_t adma2_dscr_start_phys;
	unsigned int alloced_adma2_dscr_size;
} slot_t;

typedef struct umsd_s {
	struct semaphore sem;		/* Device lock for safe multi-thread access */
	struct pci_dev *pdev;		/* NULL if not allocated */
	unsigned int boardrev;		/* Board revision (PCI revid) */
	unsigned int slot_count;	/* Number of slots on controller */
	unsigned char version_hc;	/* Host controller version (SDIO spec number) */
	unsigned char version_ven;	/* Host controller vendor revision */
	slot_t slot[UMSD_CTLR_SLOT_MAX];
} umsd_t;

static umsd_t umsd[UMSD_CTLR_MAX];

#define RREG32(s, offset)	(*(volatile unsigned int *)((s)->regs + (offset)))
#define WREG32(s, offset, val)	(*(volatile unsigned int *)((s)->regs + (offset)) = (val))
#define RREG16(s, offset)	(*(volatile unsigned short *)((s)->regs + (offset)))
#define WREG16(s, offset, val)	(*(volatile unsigned short *)((s)->regs + (offset)) = (val))
#define RREG8(s, offset)	(*(volatile unsigned char *)((s)->regs + (offset)))
#define WREG8(s, offset, val)	(*(volatile unsigned char *)((s)->regs + (offset)) = (val))

#define PCI_VENDOR_ID_SI_IMAGE	0x1095		/* Silicon Image, used by Arasan SDIO host */
#define PCI_DEVICE_ID_ARASAN	0x670		/* Arasan */
#define	PCI_VENDOR_NAME_ARASAN	"Arasan"

#define PCI_VENDOR_ID_BRCM	0x14e4
#define PCI_DEVICE_ID_BRCM	0x43f2
#define	PCI_VENDOR_NAME_BRCM	"Broadcom"

static struct pci_device_id umsd_pci_table[] __devinitdata = {
	{
	vendor: PCI_VENDOR_ID_SI_IMAGE,
	device: PCI_DEVICE_ID_ARASAN,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: 0,
	class_mask: 0,
	driver_data: 0UL },
	{
	vendor: PCI_VENDOR_ID_BRCM,
	device: PCI_DEVICE_ID_BRCM,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: 0,
	class_mask: 0,
	driver_data: 0UL },
	{ 0 }
};

MODULE_DEVICE_TABLE(pci, umsd_pci_table);

/* Internal support routines */

static void
ksleep(int jiffies)
{
	wait_queue_head_t delay_wait;
	DECLARE_WAITQUEUE(wait, current);

	init_waitqueue_head(&delay_wait);
	add_wait_queue(&delay_wait, &wait);
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(jiffies < 1 ? 1 : jiffies);
	remove_wait_queue(&delay_wait, &wait);
	set_current_state(TASK_RUNNING);
}

static int
hc_reset(slot_t *s)
{
	unsigned int val;

	val = SFIELD(0, SW_RESET_ALL, 1);
	val = SFIELD(val, SW_RESET_CMD, 1);
	val = SFIELD(val, SW_RESET_DAT, 1);
	WREG8(s, SD_SoftwareReset, val);

	/* Give it 100mec */
	ksleep(HZ / 10);

	val = RREG8(s, SD_SoftwareReset);
	if (GFIELD(val, SW_RESET_ALL)) {
		err("Host Controller reset failed\n");
		return -EIO;
	}

	return 0;
}

static void
hc_dump(slot_t *s)
{
	err("SysAddr=%08x BlockSize=%04x BlockCount=%04x\n",
	    RREG32(s, 0x00), RREG16(s, 0x04), RREG16(s, 0x06));
	err("Arg0=%04x Arg1=%04x TransferMode=%04x Command=%04x\n",
	    RREG16(s, 0x08), RREG16(s, 0x0a), RREG16(s, 0x0c), RREG16(s, 0x0e));
	err("Response0-7(10-1e)=%04x %04x %04x %04x %04x %04x %04x %04x\n",
	    RREG16(s, 0x10), RREG16(s, 0x12), RREG16(s, 0x14), RREG16(s, 0x16),
	    RREG16(s, 0x18), RREG16(s, 0x1a), RREG16(s, 0x1c), RREG16(s, 0x1e));
	err("BufferDataPort0=%04x BufferDataPort1=%04x\n",
	    RREG16(s, 0x20), RREG16(s, 0x22));
	err("PresentState=%08x HostCntrl=%02x PwrCntrl=%02x\n",
	    RREG32(s, 0x24), RREG8(s, 0x28), RREG8(s, 0x29));
	err("BlockGapCntrl=%02x WakeupCntrl=%02x\n",
	    RREG8(s, 0x2a), RREG8(s, 0x2b));
	err("ClockCntrl=%04x TimeoutCntrl=%02x SoftwareReset=%02x\n",
	    RREG16(s, 0x2c), RREG8(s, 0x2e), RREG8(s, 0x2f));
	err("IntrStatus=%04x ErrorIntrStatus=%04x\n",
	    RREG16(s, 0x30), RREG16(s, 0x32));
	err("IntrStatusEnable=%04x ErrorIntrStatusEnable=%04x\n",
	    RREG16(s, 0x34), RREG16(s, 0x36));
	err("IntrSignalEnable=%04x ErrorIntrSignalEnable=%04x\n",
	    RREG16(s, 0x38), RREG16(s, 0x3a));
	err("CMD12ErrorStatus=%08x Capabilities=%08x\n",
	    RREG32(s, 0x3c), RREG32(s, 0x40));
	err("MaxCurCap=%04x ADMA_Sysaddr=%08x\n",
	    RREG16(s, 0x48), RREG32(s, 0x58));
	err("SlotInterruptStatus=%04x HostControllerVersion=%04x\n",
	    RREG16(s, 0xfc), RREG16(s, 0xfe));
}

static void
sd_clear_adma_dscr_buf(slot_t *s)
{
	memset((char *)s->adma2_dscr_buf, 0, SD_PAGE);
}

static void
set_dma_mode(slot_t *s)
{
	uint8 reg8, dma_sel_bits;

	/* Select DMA support mode set from the best to worst (none) */
	if (umsd_dma_mode_force >= 0)
		s->dma_mode = umsd_dma_mode_force;
	else if (GFIELD(s->caps, CAP_ADMA2))
		s->dma_mode = DMA_MODE_ADMA2;
	else if (GFIELD(s->caps, CAP_DMA))
		s->dma_mode = DMA_MODE_SDMA;
	else
		s->dma_mode = DMA_MODE_NONE;

	/* Configure mode */
	switch (s->dma_mode) {
	case DMA_MODE_ADMA2:
		dma_sel_bits = SDIOH_ADMA2_MODE;
		break;
	case DMA_MODE_SDMA:
		dma_sel_bits = SDIOH_SDMA_MODE;
		break;
	default:
		s->dma_mode = DMA_MODE_NONE;
		dma_sel_bits = SDIOH_SDMA_MODE;
		break;
	}

	reg8 = RREG8(s, SD_HostCntrl);
	reg8 = SFIELD(reg8, HOST_DMA_SEL, dma_sel_bits);
	WREG8(s, SD_HostCntrl, reg8);
}

static void
sd_map_dma(slot_t *s, struct pci_dev *pdev)
{
	void *va;

	if ((va = pci_alloc_consistent(pdev, SD_PAGE, &s->adma2_dscr_start_phys)) == NULL) {
		s->dma_mode = DMA_MODE_NONE;
		s->adma2_dscr_start_buf = 0;
		s->adma2_dscr_buf = (void *)0;
		s->adma2_dscr_phys = 0;
		s->alloced_adma2_dscr_size = 0;
		err("DMA_ALLOC failed for descriptor buffer. Disabling DMA support.\n");
	} else {
		s->adma2_dscr_start_buf = va;
		s->adma2_dscr_buf = (void *)ROUNDUP((unsigned long)va, SD_PAGE);
		s->adma2_dscr_phys = ROUNDUP((s->adma2_dscr_start_phys), SD_PAGE);
		s->alloced_adma2_dscr_size = SD_PAGE;
	}

	msg("Mapped ADMA2 Descriptor Buffer %d bytes @virt/phys: 0x%lx/0x%lx\n",
		s->alloced_adma2_dscr_size, (unsigned long)(s->adma2_dscr_buf),
		(unsigned long)(s->adma2_dscr_phys));
	sd_clear_adma_dscr_buf(s);
}

static void
sd_unmap_dma(slot_t *s, struct pci_dev *pdev)
{
	if (s->adma2_dscr_start_buf) {
		pci_free_consistent(pdev, s->alloced_adma2_dscr_size,
		                    s->adma2_dscr_start_buf, s->adma2_dscr_start_phys);

		s->adma2_dscr_start_buf = 0;
	}
}

static int
set_devsig(slot_t *s, unsigned int signo)
{
	dbg("signo=%d\n", signo);

	/*
	 * Set the signal number to send the controlling process when an interrupt
	 * arrives.  The interrupt will be masked at that point, and will remain masked
	 * until the process calls set_devsig again.
	 */
	if (signo >= _NSIG)
		return -EINVAL;

	s->intsig = signo;

	/* Enable the card interrupt status bit (the signal is already enabled) */
	WREG16(s, SD_IntrStatusEnable,
	       SFIELD(RREG16(s, SD_IntrStatusEnable), INTSTAT_CARD_INT, 1));

	return 0;
}

#define INTSTAT_ALL		0x81ff
#define ERRINT_ALL		0xf1ff

static int
set_power(slot_t *s, unsigned int enab)
{
	unsigned int val;

	dbg("enab=%u\n", enab);

	if ((enab && s->power) || (!enab && !s->power))
		return 0;

	if (enab) {
		unsigned int pwr;

		/* Clear pending status bits */
		WREG16(s, SD_IntrStatus, INTSTAT_ALL);

		/* Enable status bits for all but card interrupts */
		WREG16(s, SD_IntrStatusEnable, SFIELD(INTSTAT_ALL, INTSTAT_CARD_INT, 0));

		/* Assign interesting status bits to assert interrupt */
		val = 0;
		val = SFIELD(val, INTSTAT_XFER_COMPLETE, 1);
		val = SFIELD(val, INTSTAT_CARD_INT, 1);
		WREG16(s, SD_IntrSignalEnable, val);

		/* Clear pending error status bits */
		WREG16(s, SD_ErrorIntrStatus, ERRINT_ALL);

		/* Enable error status bits for all error interrupts */
		WREG16(s, SD_ErrorIntrStatusEnable, ERRINT_ALL);

		/* Assign all error status bits to assert interrupt */
		WREG16(s, SD_ErrorIntrSignalEnable, ERRINT_ALL);

		pwr = 0;
		if (GFIELD(s->caps, CAP_VOLT_1_8))
			pwr = SFIELD(pwr, PWR_VOLTS, 5);
		if (GFIELD(s->caps, CAP_VOLT_3_0))
			pwr = SFIELD(pwr, PWR_VOLTS, 6);
		if (GFIELD(s->caps, CAP_VOLT_3_3))
			pwr = SFIELD(pwr, PWR_VOLTS, 7);
		pwr = SFIELD(pwr, PWR_BUS_EN, 1);

		/* Turn on power */
		WREG8(s, SD_PwrCntrl, pwr);

		/* Allow 250ms for power to stabilize */
		ksleep(HZ / 4);

		s->power = 1;
	} else {
		WREG16(s, SD_IntrStatusEnable, 0);
		WREG16(s, SD_ErrorIntrStatusEnable, 0);

		WREG8(s, SD_PwrCntrl, 0);

		s->power = 0;
	}

	return 0;
}

static int
set_clkbase(slot_t *s, unsigned int hz)
{
	/* Setting clock base is only permitted if the capabilities don't specify it */

	if (GFIELD(s->caps, CAP_TO_CLKFREQ) != 0)
		return -EINVAL;

	s->clkbase = hz;

	return 0;
}

static int
set_clkdiv(slot_t *s, unsigned int div)
{
	unsigned int toval, todiv, tmp;

	dbg("div=%u\n", div);

	if (s->clkbase == 0 || div > UMSD_CLKDIV_MAX || (div > 0 && !ISPOW2(div)))
		return -EINVAL;

	/* Turn off SD Clock Enable */
	WREG16(s, SD_ClockCntrl, RREG16(s, SD_ClockCntrl) & ~4);

	/* Turn off Internal Clock Enable */
	WREG16(s, SD_ClockCntrl, RREG16(s, SD_ClockCntrl) & ~1);

	s->clkdiv = 0;

	if (div == 0)
		return 0;

	/* Set clock divisor */
	WREG16(s, SD_ClockCntrl, (RREG16(s, SD_ClockCntrl) & 0xff) | (div >> 1) << 8);

	/* Turn on Internal Clock Enable */
	WREG16(s, SD_ClockCntrl, RREG16(s, SD_ClockCntrl) | 1);

	/* Give some time (100ms) for clock to stabilize (could poll here to speed it up) */
	ksleep(HZ / 10);

	if (!(RREG16(s, SD_ClockCntrl) & 2)) {
		err("Clock did not stabilize\n");
		return -EIO;
	}

	/* Turn on SD Clock Enable */
	WREG16(s, SD_ClockCntrl, RREG16(s, SD_ClockCntrl) | 4);

	/*
	 * Set timeout control (adjust default value based on divisor).
	 * Base timeout counter value on 48MHz (2^20 @ 48MHz => 21845us)
	 * Could adjust by adding divisor (to maintain bit count) but really
	 * need something more elaborate to do that right.  Still allows xfer
	 * of about 1000 bytes at 400KHz, so constant is ok.
	 * Timeout control N produces 2^(13+N) counter.
	 */

	toval = TIMEOUT_CONTROL;
	todiv = div;

	while (toval && (todiv & 1) == 0) {
		toval -= 1;
		todiv >>= 1;
	}

	/* Disabling timeout interrupts during setting is advised by host spec */
	tmp = RREG16(s, SD_ErrorIntrStatusEnable);
	WREG16(s, SD_ErrorIntrStatusEnable, SFIELD(tmp, ERRINT_DATA_TIMEOUT, 0));
	WREG8(s, SD_TimeoutCntrl, toval);
	WREG16(s, SD_ErrorIntrStatusEnable, tmp);

	s->clkdiv = div;

	return 0;
}

static int
set_width(slot_t *s, unsigned int width)
{
	if (width != 1 && width != 4)
		return -EINVAL;

	WREG8(s, SD_HostCntrl,
	      SFIELD(RREG8(s, SD_HostCntrl), HOST_DATA_WIDTH, (width == 4)));

	s->width = width;

	return 0;
}

static int
set_hispeed(slot_t *s, unsigned int enab)
{
	if (enab && !GFIELD(s->caps, CAP_HIGHSPEED))
		return -EINVAL;

	WREG8(s, SD_HostCntrl,
	      SFIELD(RREG8(s, SD_HostCntrl), HOST_HI_SPEED_EN, enab ? 1 : 0));

	return 0;
}

static int
set_blksize(slot_t *s, unsigned short blksize[UMSD_FUNC_MAX])
{
	unsigned short maxblksize;
	int i;

	if (GFIELD(s->caps, CAP_MAXBLOCK) == 2)
		maxblksize = 2048;
	else if (GFIELD(s->caps, CAP_MAXBLOCK) == 1)
		maxblksize = 1024;
	else
		maxblksize = 512;

	dbg("maxblksize=%u\n", maxblksize);

	for (i = 0; i < UMSD_FUNC_MAX; i++)
		if (blksize[i] > maxblksize)
			return -EINVAL;

	dbg("(%u, %u, %u, %u...)\n", blksize[0], blksize[1], blksize[2], blksize[3]);

	memcpy(s->blksize, blksize, sizeof(s->blksize));

	return 0;
}

static int
cancel(slot_t *s, umsd_req_id_t id)
{
	return -EPERM;
}

/*
 * Compute the amount of transfer data (does not include status response)
 */
static int
request_len(slot_t *s, umsd_req_t *req, unsigned long *len)
{
	switch (req->cmd) {
	case SDIOH_CMD_0:	/* Set Card to Idle State - No Response */
	case SDIOH_CMD_3:	/* Ask card to send RCA - Response R6 */
	case SDIOH_CMD_5:	/* Send Operation condition - Response R4 */
	case SDIOH_CMD_7:	/* Select card - Response R1 */
	case SDIOH_CMD_15:	/* Set card to inactive state - Response None */
	case SDIOH_CMD_52:	/* IO R/W Direct (single byte) - Response R5 */
		*len = 0;
		break;
	case SDIOH_CMD_53:	/* IO R/W Extended (multiple bytes/blocks) */
		if (req->count < 1 || req->count > 512)
			return -EINVAL;
		if (req->flags & UMSD_REQ_BLKMODE) {
			if (req->func >= UMSD_FUNC_MAX || s->blksize[req->func] == 0)
				return -EINVAL;
			*len = req->count * s->blksize[req->func];
		} else
			*len = req->count;
		break;
	}

	if (*len > XFER_BUF_LEN)
		return -EMSGSIZE;

	return 0;
}

#define PIO_RETRY_LIMIT		1000

static int
write_port(slot_t *s, void *buf, unsigned int bytes)
{
	unsigned int i, *p = buf;
	int limit = PIO_RETRY_LIMIT;

	dbg("PIO write %u bytes from %p\n", bytes, buf);

	/* Transfer words, then from 0-3 left-over bytes */

	while (bytes >= 4) {
		while (!GFIELD(RREG32(s, SD_PresentState), PRES_WRITE_DATA_RDY)) {
			dbg("Data port full %d\n", limit);
			ksleep(1);
			if (--limit == 0)
				return -EIO;
		}
		WREG32(s, SD_BufferDataPort0, *p++);
		bytes -= 4;
	}

	for (i = 0; i < bytes; i++)
		WREG8(s, SD_BufferDataPort0 + i, ((unsigned char *)p)[i]);

	return 0;
}

static int
read_port(slot_t *s, void *buf, unsigned int bytes)
{
	unsigned int i, *p = buf;
	int limit = PIO_RETRY_LIMIT;

	dbg("PIO read %u bytes from %p\n", bytes, buf);

	/* Transfer words, then from 0-3 left-over bytes */

	while (bytes >= 4) {
		while (!GFIELD(RREG32(s, SD_PresentState), PRES_READ_DATA_RDY)) {
			dbg("Data port empty %d\n", limit);
			ksleep(1);
			if (--limit == 0)
				return -EIO;
		}
		*p++ = RREG32(s, SD_BufferDataPort0);
		bytes -= 4;
	}

	for (i = 0; i < bytes; i++)
		((unsigned char *)p)[i] = RREG8(s, SD_BufferDataPort0 + i);

	return 0;
}

/* return val 0:success, -1:err */
static int
command_inhibit_wait(slot_t *s)
{
	unsigned int retries = CMD_INHIBIT_TRIES;

	while (GFIELD(RREG32(s, SD_PresentState), PRES_CMD_INHIBIT) && --retries)
		;

	return (retries == 0) ? -1 : 0;
}

/* return val 0:success, -1:err */
static int
data_inhibit_wait(slot_t *s)
{
	unsigned int retries = DATA_INHIBIT_TRIES;

	while (GFIELD(RREG32(s, SD_PresentState), PRES_DAT_INHIBIT) && --retries)
		;

	return (retries == 0) ? -1 : 0;
}

/* return val 0:success, -1:err */
static int
spin_status_bits(slot_t *s)
{
	unsigned int ints;
	int retries = 0;

	do {
		ints = RREG16(s, SD_IntrStatus);

		if (GFIELD(ints, INTSTAT_CMD_COMPLETE) && !GFIELD(ints, INTSTAT_ERROR_INT)) {
		   ints = 0;
		   ints = SFIELD(ints, INTSTAT_CMD_COMPLETE, 1);
		   WREG16(s, SD_IntrStatus, ints);
		   break;
		}
	} while (++retries < CMD_COMPLETE_TRIES);

	if (retries >= CMD_COMPLETE_TRIES) {
		err("Timed out waiting for command complete!\n");
		return -1;
	} else
		return 0;
}

static int
request(slot_t *s, int is_write, umsd_req_t *req)
{
	unsigned int cmd_reg, arg_reg;
	unsigned int blocksize = 0;
	unsigned int blockcount = 0;
	unsigned int val;
	int use_dma = s->dma_mode;

	if (!s->power || s->clkdiv == 0) {
		err("No device power and/or clock\n");
		return -EINVAL;
	}

	/*
	 * request_len() will have already been used to verify the validity of req->func
	 * and req->count and that the total size fits in xfer_buf.
	 */

	if (command_inhibit_wait(s)) {
		err("Command Inhibit timeout!\n");
		return -EIO;
	}

	if (GFIELD(RREG32(s, SD_PresentState), PRES_DAT_INHIBIT)) {
		err("Data Inhibit active\n");
		return -EIO;
	}

	cmd_reg = 0;
	cmd_reg = SFIELD(cmd_reg, CMD_TYPE, CMD_TYPE_NORMAL);
	cmd_reg = SFIELD(cmd_reg, CMD_INDEX, req->cmd);

	arg_reg = req->arg;

	switch (req->cmd) {
	case SDIOH_CMD_0:	/* Set Card to Idle State - No Response */
		cmd_reg = SFIELD(cmd_reg, CMD_RESP_TYPE, RESP_TYPE_NONE);
		break;
	case SDIOH_CMD_3:	/* Ask card to send RCA - Response R6 */
		cmd_reg = SFIELD(cmd_reg, CMD_RESP_TYPE, RESP_TYPE_48);
		break;
	case SDIOH_CMD_5:	/* Send Operation condition - Response R4 */
		cmd_reg = SFIELD(cmd_reg, CMD_RESP_TYPE, RESP_TYPE_48);
		break;
	case SDIOH_CMD_7:	/* Select card - Response R1 */
		cmd_reg = SFIELD(cmd_reg, CMD_RESP_TYPE, RESP_TYPE_48);
		cmd_reg = SFIELD(cmd_reg, CMD_CRC_EN, 1);
		cmd_reg = SFIELD(cmd_reg, CMD_INDEX_EN, 1);
		break;
	case SDIOH_CMD_15:	/* Set card to inactive state - Response None */
		cmd_reg = SFIELD(cmd_reg, CMD_RESP_TYPE, RESP_TYPE_NONE);
		break;
	case SDIOH_CMD_52:	/* IO R/W Direct (single byte) - Response R5 */
		if ((req->addr & 0x1ffff) != req->addr) {
			err("CMD52 address too large (0x%x)\n", req->addr);
			return -EINVAL;
		}

		cmd_reg = SFIELD(cmd_reg, CMD_RESP_TYPE, RESP_TYPE_48);
		cmd_reg = SFIELD(cmd_reg, CMD_CRC_EN, 1);
		cmd_reg = SFIELD(cmd_reg, CMD_INDEX_EN, 1);

		arg_reg = 0;
		arg_reg = SFIELD(arg_reg, CMD52_RW_FLAG,
		                 is_write ? SD_IO_OP_WRITE : SD_IO_OP_READ);
		arg_reg = SFIELD(arg_reg, CMD52_FUNCTION,
		                 req->func);
		arg_reg = SFIELD(arg_reg, CMD52_RAW,
		                 (req->flags & UMSD_REQ_RAW) ? SD_IO_RW_RAW : SD_IO_RW_NORMAL);
		arg_reg = SFIELD(arg_reg, CMD52_REG_ADDR,
		                 req->addr);
		arg_reg = SFIELD(arg_reg, CMD52_DATA,
		                 (is_write ? req->arg & 0xff : 0));
		break;
	case SDIOH_CMD_53:	/* IO R/W Extended (multiple bytes/blocks) */
		if ((req->addr & 0x1ffff) != req->addr) {
			err("CMD53 address too large (0x%x)\n", req->addr);
			return -EINVAL;
		}

		cmd_reg = SFIELD(cmd_reg, CMD_RESP_TYPE, RESP_TYPE_48);
		cmd_reg = SFIELD(cmd_reg, CMD_CRC_EN, 1);
		cmd_reg = SFIELD(cmd_reg, CMD_INDEX_EN, 1);
		cmd_reg = SFIELD(cmd_reg, CMD_DATA_EN, 1);

		arg_reg = 0;
		arg_reg = SFIELD(arg_reg, CMD53_RW_FLAG,
		                 is_write ? SD_IO_OP_WRITE : SD_IO_OP_READ);
		arg_reg = SFIELD(arg_reg, CMD53_FUNCTION,
		                 req->func);
		arg_reg = SFIELD(arg_reg, CMD53_BLK_MODE,
		                 (req->flags & UMSD_REQ_BLKMODE) ?
		                 SD_IO_BLOCK_MODE : SD_IO_BYTE_MODE);
		arg_reg = SFIELD(arg_reg, CMD53_OP_CODE,
		                 (req->flags & UMSD_REQ_FIFO) ?
		                 SD_IO_FIXED_ADDRESS : SD_IO_INCREMENT_ADDRESS);
		arg_reg = SFIELD(arg_reg, CMD53_REG_ADDR,
		                 req->addr);
		arg_reg = SFIELD(arg_reg, CMD53_BYTE_BLK_CNT,
		                 req->count);

		if (req->flags & UMSD_REQ_BLKMODE) {
			blocksize = s->blksize[req->func];
			blockcount = req->count;
			dbg("blockmode: size=%u count=%u\n", blocksize, blockcount);
		} else {
			blocksize = req->count;
			blockcount = 1;		/* Older Arasans need it set to 1 */
			dbg("bytemode: size=%u count=%u\n", blocksize, blockcount);
		}

		if (blocksize * blockcount < DMA_SIZE_THRESHOLD)
			use_dma = DMA_MODE_NONE;

		switch (use_dma) {
		case DMA_MODE_SDMA:
			WREG32(s, SD_SysAddr, s->xfer_buf_pa);
			break;
		case DMA_MODE_ADMA1:
			break;
		case DMA_MODE_ADMA2: {
			adma2_dscr_32b_t *adma2_dscr_table =
			        (adma2_dscr_32b_t *)s->adma2_dscr_buf;
			/* we only support one descriptor buffer for now */
			adma2_dscr_table->phys_addr = s->xfer_buf_pa;
			adma2_dscr_table->len_attr =
			        (((blockcount * blocksize) << 16) |
			         ADMA2_ATTRIBUTE_VALID | ADMA2_ATTRIBUTE_END |
			         ADMA2_ATTRIBUTE_INT | ADMA2_ATTRIBUTE_ACT_TRAN);
			WREG32(s, SD_ADMA_SysAddr, s->adma2_dscr_phys);
			break;
		}
		default:	/* no dma */
			break;
		}

		val = 0;
		val = SFIELD(val, BLKSZ_BNDRY, 0);
		val = SFIELD(val, BLKSZ_BLKSZ, blocksize);
		WREG16(s, SD_BlockSize, val);

		WREG16(s, SD_BlockCount, blockcount);

		val = 0;
		val = SFIELD(val, XFER_DATA_DIRECTION, is_write ? 0 : 1);
		val = SFIELD(val, XFER_DMA_ENABLE, (use_dma != DMA_MODE_NONE));
		val = SFIELD(val, XFER_MULTI_BLOCK, (blockcount > 1));
		val = SFIELD(val, XFER_BLK_COUNT_EN, (blockcount > 1));
		/* val = SFIELD(val, XFER_CMD_12_EN, (blockcount > 1)); */

		if (data_inhibit_wait(s)) {
			err("CMD%d Data Inhibit timeout!\n", req->cmd);
			return -EIO;
		}

		WREG16(s, SD_TransferMode, val);

		/* msg("use_dma:%d xfer_mode:0x%x\n", use_dma, val); */
		break;
	default:
		err("Unsupported command %u\n", req->cmd);
		return -EINVAL;
	}

	s->intr_pend_xc = 0;
	s->intr_pend_err = 0;

	if (data_inhibit_wait(s)) {
		err("CMD%d Data Inhibit timeout!\n", req->cmd);
		return -EIO;
	}

	/* Issue the command */
	if (command_inhibit_wait(s)) {
		err("CMD%d Command Inhibit timeout!\n", req->cmd);
		return -EIO;
	}

	WREG32(s, SD_Arg0, arg_reg);
	WREG16(s, SD_Command, cmd_reg);
	if (spin_status_bits(s))
		return -EIO;

	if (s->intr_pend_err)
		return -EIO;

	/* Return one-word response in request structure */
	req->resp = RREG32(s, SD_Response0);
	dbg("Response=0x%x\n", req->resp);


	/* If not using DMA, transfer CMD53 data via PIO */
	if (!use_dma && req->cmd == SDIOH_CMD_53) {
		int rv;

		if (is_write)
			rv = write_port(s, s->xfer_buf, blocksize * blockcount);
		else
			rv = read_port(s, s->xfer_buf, blocksize * blockcount);

		if (rv != 0)
			return rv;
	}

	if (req->cmd == SDIOH_CMD_53) {
		sigset_t saved;
		sigset_t block;
		
		// Temporarly disable SIGUSR1 signal
		sigemptyset(&block);
		sigaddset(&block,SIGUSR1);
		sigprocmask(SIG_BLOCK, &block, &saved);
		
		dbg("Wait for Transfer Complete interrupt\n");
		wait_event_interruptible_timeout(s->intr_wait,
		                                 s->intr_pend_xc | s->intr_pend_err,
		                                 TRANSFER_TIMEOUT);

		if (signal_pending(current)) {
			err("Signal received while waiting for transfer complete\n");
			dbg("IntrStatus=0x%x ErrorIntrStatus=0x%x\n",
			    RREG16(s, SD_IntrStatus), RREG16(s, SD_ErrorIntrStatus));
		}

		sigprocmask(SIG_SETMASK, &saved, NULL);
		
		if (s->intr_pend_err)
			return -EIO;

		if (!s->intr_pend_xc) {
			err("Transfer timeout\n");
			return -EIO;
		}
	}


	return 0;
}

/* Device driver entry points */

static int
umsd_open(struct inode *inode, struct file *file)
{
	unsigned int minor, ctlr, slot;
	umsd_t *u;
	slot_t *s;
	int rv;

	minor = MINOR(inode->i_rdev);

	if (minor < UMSD_MINOR_BASE || minor >= UMSD_MINOR_BASE + UMSD_MINOR_COUNT)
		return -ENODEV;

	ctlr = MINOR_CTLR(minor);

	u = &umsd[ctlr];

	down(&u->sem);

	if (u->pdev == NULL) {
		up(&u->sem);
		return -ENODEV;
	}

	slot = MINOR_CTLR_SLOT(minor);

	s = &u->slot[slot];

	if (s->use_count > 0) {
		up(&u->sem);
		return -EBUSY;
	}

	if (!GFIELD(RREG32(s, SD_PresentState), PRES_CARD_PRESENT)) {
		err("No SDIO card found in Slot %d\n", slot + 1);
		up(&u->sem);
		return -ENODEV;
	}

	/* Reset host controller to get it in a known state */

	if ((rv = hc_reset(s)) < 0) {
		up(&u->sem);
		return rv;
	}

	s->owner = current;
	s->intsig = 0;
	s->power = 0;
	s->clkbase = GFIELD(s->caps, CAP_TO_CLKFREQ) * 1000000;
	s->clkdiv = 0;
	s->width = 1;
	s->hispeed = 0;

	memset(s->blksize, 0, sizeof(s->blksize));

	/* Allocate DMA-safe memory */
	if ((s->xfer_buf = pci_alloc_consistent(u->pdev, XFER_BUF_LEN, &s->xfer_buf_pa)) == NULL) {
		err("Could not allocate DMA consistent buffer\n");
		up(&u->sem);
		return -ENOMEM;
	}

	file->private_data = s;

	s->use_count++;

	OLD_MOD_INC_USE_COUNT;

	up(&u->sem);
	return 0;
}

static int
umsd_release(struct inode *inode, struct file *file)
{
	slot_t *s = file->private_data;
	umsd_t *u = &umsd[s->ctlr];

	down(&u->sem);

	/* Reset host controller to stop everything and ensure no interrupt is generated */
	(void)hc_reset(s);
	(void)set_clkdiv(s, 0);
	(void)set_power(s, FALSE);

	pci_free_consistent(u->pdev, XFER_BUF_LEN, s->xfer_buf, s->xfer_buf_pa);

	s->use_count--;

	OLD_MOD_DEC_USE_COUNT;

	up(&u->sem);

	return 0;
}

/*
 * read() not currently used
 */
static ssize_t
umsd_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	slot_t *s = file->private_data;

	down(&s->sem);

	dbg("count=%u\n", (unsigned int)count);

	up(&s->sem);

	return -EINVAL;
}

/*
 * write() not currently used
 */
static ssize_t
umsd_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	slot_t *s = file->private_data;

	down(&s->sem);

	dbg("count=%u\n", (unsigned int)count);

	up(&s->sem);

	return -EINVAL;
}

static int
umsd_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	unsigned long arg)
{
	slot_t *s = file->private_data;
	unsigned int val;
	int rv = 0;

	down(&s->sem);

	switch (cmd) {
	case UMSD_SDEVSIG:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			rv = set_devsig(s, val);
		break;
	case UMSD_GDEVSIG:
		val = s->intsig;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case UMSD_SPOWER:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			rv = set_power(s, val);
		break;
	case UMSD_GPOWER:
		val = s->power;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case UMSD_SCLKBASE:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			rv = set_clkbase(s, val);
		break;
	case UMSD_GCLKBASE:
		val = s->clkbase;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case UMSD_SCLKDIV:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			rv = set_clkdiv(s, val);
		break;
	case UMSD_GCLKDIV:
		val = s->clkdiv;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case UMSD_SWIDTH:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			rv = set_width(s, val);
		break;
	case UMSD_GWIDTH:
		val = s->width;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case UMSD_SHISPEED:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			rv = set_hispeed(s, val);
		break;
	case UMSD_GHISPEED:
		val = s->hispeed;
		if (copy_to_user((void *)arg, &val, sizeof(val)))
			rv = -EFAULT;
		break;
	case UMSD_SBLKSIZE: {
		unsigned short blksize[UMSD_FUNC_MAX];
		if (copy_from_user(&blksize, (void *)arg, sizeof(blksize)))
			rv = -EFAULT;
		else
			rv = set_blksize(s, blksize);
		break;
	}
	case UMSD_GBLKSIZE:
		if (copy_to_user((void *)arg, s->blksize, sizeof(s->blksize)))
			rv = -EFAULT;
		break;
	case UMSD_REQCANCEL:
		if (copy_from_user(&val, (void *)arg, sizeof(val)))
			rv = -EFAULT;
		else
			rv = cancel(s, val);
		break;
	case UMSD_REQREAD: {
		umsd_req_t req;
		unsigned long len;

		if (copy_from_user(&req, (void *)arg, sizeof(req)))
			rv = -EFAULT;
		else {
			if ((rv = request_len(s, &req, &len)) >= 0 &&
			    (rv = request(s, 0, &req)) >= 0) {
				if (copy_to_user(req.buf, s->xfer_buf, len) ||
				    copy_to_user((void *)arg, &req, sizeof(req)))
					rv = -EFAULT;
			}
		}
		break;
	}
	case UMSD_REQWRITE: {
		umsd_req_t req;
		unsigned long len;

		if (copy_from_user(&req, (void *)arg, sizeof(req)))
			rv = -EFAULT;
		else if ((rv = request_len(s, &req, &len)) >= 0) {
			if (copy_from_user(s->xfer_buf,
			                   req.buf,
			                   len))
				rv = -EFAULT;
			else if ((rv = request(s, 1, &req)) >= 0 &&
			         copy_to_user((void *)arg, &req, sizeof(req)))
				rv = -EFAULT;
		}
		break;
	}
	default:
		rv = -EINVAL;
		break;
	}

	up(&s->sem);

	return rv;
}

static void
errint_show(unsigned int errints)
{
	static char *errname[16] = {
		"CmdTimeout", "CmdCRC", "CmdEndBit", "CmdIndex",
		"DataTimeout", "DataCRC", "DataEndBit", "CurrentLimit",
		"AutoCmd12", "?", "??", "???",
		"Vendor0", "Vendor1", "Vendor2", "Vendor3"
	};

	int b;

	for (b = 0; b < 16; b++)
		if (errints & (1 << b))
			err("Error interrupt %d: %s\n", b, errname[b]);
}

/* Returns 1 if slot interrupt handled, 0 if there were none */
static int
slot_isr(slot_t *s)
{
	unsigned int ints, errints;
	int wake;


	ints = RREG16(s, SD_IntrStatus) & RREG16(s, SD_IntrSignalEnable);
	errints = RREG16(s, SD_ErrorIntrStatus) & RREG16(s, SD_ErrorIntrSignalEnable);

	if ((ints | errints) == 0)
		return 0;

	if (s->use_count < 1) {
		/* Oops, got an interrupt for a slot not in use */
		WREG8(s, SD_SoftwareReset, SFIELD(0, SW_RESET_ALL, 1));
		return 1;
	}

	wake = 0;

	if (GFIELD(ints, INTSTAT_CARD_INT)) {
		/* Mask the card interrupt enable */
		WREG16(s, SD_IntrStatusEnable,
		       SFIELD(RREG16(s, SD_IntrStatusEnable), INTSTAT_CARD_INT, 0));
		/* If so configured, deliver signal to controlling process */
		if (s->use_count > 0 && s->intsig != 0)
			send_sig(s->intsig, s->owner, 0);
	}


	if (GFIELD(ints, INTSTAT_XFER_COMPLETE)) {
		dbg("Transfer Complete interrupt\n");
		s->intr_pend_xc = 1;
		wake = 1;
	}

	if (errints) {
		errint_show(errints);
		hc_dump(s);
		s->intr_pend_err = 1;
		wake = 1;
	}

	/* Clear the processed interrupts */
	WREG16(s, SD_IntrStatus, ints);
	WREG16(s, SD_ErrorIntrStatus, errints);

	if (wake)
		wake_up_interruptible(&s->intr_wait);

	return 1;
}

/* Interrupt handler */
static irqreturn_t
umsd_isr(CALLBACK_ARGS)
{
	umsd_t *u = dev_id;
	int ours = 0, slot;
	unsigned int slotmask;

#ifdef PR81999_WAR
	slotmask = 3;
#else
	slotmask = RREG8(&u->slot[0], SD_SlotInterruptStatus);
#endif

	for (slot = 0; slot < u->slot_count; slot++)
		if (slotmask & (1 << slot))
			ours |= slot_isr(&u->slot[slot]);

	return IRQ_RETVAL(ours);
}

static struct file_operations umsd_fops = {
	owner:		THIS_MODULE,
	read:		umsd_read,
	write:		umsd_write,
	ioctl:		umsd_ioctl,
	open:		umsd_open,
	release:	umsd_release,
};

static int __devinit
umsd_pci_probe(struct pci_dev *pdev,
	const struct pci_device_id *ent)
{
	unsigned int val;
	uint8 boardrev;
	int rv;
	int ctlr;
	umsd_t *u;
	int first_bar, slot;

	pci_read_config_dword(pdev, PCI_REVISION_ID, &val);
	boardrev = val & 0xff;

	dbg("vendor=0x%02x device=0x%02x\n", pdev->vendor, pdev->device);

	msg("Found %s SDIO Host Controller: bus %d slot %d func %d rev %d irq %d\n",
	    (pdev->vendor == PCI_VENDOR_ID_SI_IMAGE) ? PCI_VENDOR_NAME_ARASAN :
	    (pdev->vendor == PCI_VENDOR_ID_BRCM) ? PCI_VENDOR_NAME_BRCM : "Unknown",
	    pdev->bus->number,
	    PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn),
	    boardrev, pdev->irq);

	if ((rv = pci_enable_device(pdev)) != 0) {
		err("Cannot enable PCI device; aborting\n");
		goto done;
	}

	if ((rv = pci_request_regions(pdev, DRV_MODULE_NAME)) != 0) {
		err("Cannot obtain PCI resources; aborting\n");
		goto done_disable_dev;
	}

	pci_set_master(pdev);

	/* Assign to next available minor device number (accessed from /dev/umsd<n>) */
	for (ctlr = 0; ctlr < UMSD_CTLR_MAX; ctlr++)
		if (umsd[ctlr].pdev == NULL)
			break;

	if (ctlr == UMSD_CTLR_MAX) {
		err("Too many SDIO controllers; aborting\n");
		rv = -ENOMEM;
		goto done_free_regions;
	}

	u = &umsd[ctlr];

	down(&u->sem);

	u->pdev = pdev;

	u->boardrev = boardrev;

	memset(&u->slot, 0, sizeof(u->slot));

	pci_read_config_dword(u->pdev, SD_SlotInfo, &val);

	u->slot_count = ((val >> 4) & 7) + 1;
	first_bar = val & 7;

	dbg("slot_count=%d first_bar=%d\n", u->slot_count, first_bar);

	if (u->slot_count < 1 || u->slot_count >= UMSD_CTLR_SLOT_MAX) {
		err("Unsupported number of slots on controller (%d)\n", u->slot_count);
		rv = -ENXIO;
		goto done_up;
	}

	for (slot = 0; slot < u->slot_count; slot++) {
		unsigned int bar;
		slot_t *s = &u->slot[slot];

		s->ctlr = ctlr;

		pci_read_config_dword(u->pdev, PCI_CFG_BAR0 + 4 * (first_bar + slot), &bar);

		init_MUTEX(&s->sem);

		if ((s->regs = ioremap_nocache(bar, SDIO_REG_WINSZ)) == NULL) {
			err("Cannot map controller registers; aborting\n");
			rv = -ENOMEM;
			goto done_unmap;
		}

		/* Host controller version can be read from any slot (use 0) */
		if (slot == 0) {
			val = RREG16(s, SD_HostControllerVersion);
			u->version_hc = val & 0xff;
			u->version_ven = val >> 8;

			msg("SDIO %d.0 Standard Host Controller Unit %d, Vendor Rev %d\n",
			    u->version_hc, ctlr + 1, u->version_ven);
		}

		spin_lock_init(&s->intlock);
		init_waitqueue_head(&s->intr_wait);

		s->caps = RREG32(s, SD_Capabilities);
		s->curr_caps = RREG32(s, SD_MaxCurCap);

		set_dma_mode(s);
		sd_map_dma(s, u->pdev);

		dbg("caps=0x%x curr_caps=0x%x\n", s->caps, s->curr_caps);

		if (GFIELD(s->caps, CAP_MAXBLOCK) != 3)
			dbg("max block size %d\n", 512 << GFIELD(s->caps, CAP_MAXBLOCK));
		if (GFIELD(s->caps, CAP_VOLT_3_3))
			dbg("supports 3.3V at %dmA\n", GFIELD(s->curr_caps, CAP_CURR_3_3) * 4);
		if (GFIELD(s->caps, CAP_VOLT_3_0))
			dbg("supports 3.0V at %dmA\n", GFIELD(s->curr_caps, CAP_CURR_3_0) * 4);
		if (GFIELD(s->caps, CAP_VOLT_3_3))
			dbg("supports 1.8V at %dmA\n", GFIELD(s->curr_caps, CAP_CURR_1_8) * 4);

		/* Reset host controller to ensure it isn't generating an interrupt */

		if ((rv = hc_reset(s)) < 0)
			goto done_unmap;

		msg("Configured Unit %d, Slot %d as umsd%d\n",
		    ctlr + 1, slot + 1, ctlr * UMSD_CTLR_SLOT_MAX + slot);
	}

	/* Map interrupt handler */

	if (request_irq(u->pdev->irq, umsd_isr, IRQF_SHARED, "umsd", u) < 0) {
		err("Request IRQ %d failed\n", u->pdev->irq);
		rv = -EIO;
		goto done_unmap;
	}

	pci_set_drvdata(pdev, u);

	up(&u->sem);
	return 0;

done_unmap:
	for (slot = 0; slot < u->slot_count; slot++) {
		slot_t *s = &u->slot[slot];
		if (s->regs != NULL)
			iounmap((void *)s->regs);
	}

done_up:
	up(&u->sem);

done_free_regions:
	pci_release_regions(pdev);

done_disable_dev:
	pci_disable_device(pdev);

done:
	return rv;
}

static void __devexit
umsd_pci_remove(struct pci_dev *pdev)
{
	umsd_t *u = pci_get_drvdata(pdev);
	int slot;


	dbg("entered\n");

	if (u == NULL || u->pdev != pdev) {
		err("Invalid PCI device %p\n", pdev);
		return;
	}

	/* Software clean-up */

	pci_set_drvdata(pdev, NULL);

	free_irq(u->pdev->irq, u);

	for (slot = 0; slot < u->slot_count; slot++) {
		slot_t *s = &u->slot[slot];

		if (s->use_count > 0)
			err("Removing driver for device that is still open!?\n");

		sd_unmap_dma(s, u->pdev);
		iounmap((void *)s->regs);
	}

	u->pdev = NULL;

	pci_release_regions(pdev);

	pci_disable_device(pdev);
}

static struct pci_driver umsd_driver = {
	.name		= DRV_MODULE_NAME,
	.id_table	= umsd_pci_table,
	.probe		= umsd_pci_probe,
	.remove		= __devexit_p(umsd_pci_remove),
};

static int __init
umsd_init(void)
{
	int rv, ctlr;

	if ((rv = register_chrdev(UMSD_MAJOR, "umsd", &umsd_fops)) < 0) {
		err("Cannot register chrdev umsd; aborting\n");
		return rv;
	}

	for (ctlr = 0; ctlr < UMSD_CTLR_MAX; ctlr++) {
		umsd_t *u = &umsd[ctlr];
		init_MUTEX(&u->sem);
		u->pdev = NULL;
	}

	return pci_module_init(&umsd_driver);
}

static void __exit
umsd_cleanup(void)
{
	pci_unregister_driver(&umsd_driver);

	unregister_chrdev(UMSD_MAJOR, "umsd");
}

module_init(umsd_init);
module_exit(umsd_cleanup);
