/****************************************************************************
*
*  Copyright (c) 2005-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or
*  its licensors, and may only be used, duplicated, modified or distributed
*  pursuant to the terms and conditions of a separate, written license
*  agreement executed between you and Broadcom (an "Authorized License").
*  Except as set forth in an Authorized License, Broadcom grants no license
*  (express or implied), right to use, or waiver of any kind with respect to
*  the Software, and Broadcom expressly reserves all rights in and to the
*  Software and all intellectual property rights therein.  IF YOU HAVE NO
*  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
*  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
*  SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************
*
*  Filename: bcm7xxxPciEnet.c
*
****************************************************************************
*  Description:	This is Linux network driver for the BCM
* 				BroadBand Modems that interface with 7xxx via PCI or SCB
*               For example, 3368/7038, 3255/7401 and 7118.
*
*  Updates: 10/10/2005  wfeng.  Created.
*           09/18/2006  wfeng.  Added 7118 support.
            02/10/2013  hshi.   Added ethernet passthrough support
****************************************************************************/
#include <linux/version.h>
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#include <linux/config.h>
#endif
#include <linux/pci.h>

#if defined(BCM7118)
#define CARDNAME    "bcmScbEnet"
  #define PASS_THROUGH 0
#else
#define CARDNAME    "bcmPciEnet"
  #define PASS_THROUGH 1
  #define PACKET_COPY
#endif
#define VERSION     "1.31"
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__

#define PAUSE_LO_THRESHOLD 5
#define PAUSE_HI_THRESHOLD 10


#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
#if defined(CONFIG_MODVERSIONS) && ! defined(MODVERSIONS)
   #include <linux/modversions.h>
   #define MODVERSIONS
#endif
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
#include <linux/slab.h>
#endif
#include <linux/string.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/delay.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <asm/mipsregs.h>

#include <linux/reboot.h>
#include <linux/autoconf.h>
#include <linux/mtd/mtd.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#define MTD_READ(mtd, args...) (*(mtd->read))(mtd, args)
#endif

#if defined(CONFIG_MIPS_BCM97456B0) || defined(CONFIG_MIPS_BCM97456D0) || defined(CONFIG_MIPS_BCM97459B0)
/* 64MB Flash */
#define FLASH_MACADDR_OFFSET 0x03FFF824
#else
/* 32MB Flash */
#define FLASH_MACADDR_OFFSET 0x01FFF824
#endif

#include <linux/wait.h>
#include <linux/stddef.h>

#ifdef USE_PROC
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#endif

#include "if_net.h"
#include "bcm7xxxPciEnet.h"

#define skb_dataref(x)   (&skb_shinfo(x)->dataref)

// --------------------------------------------------------------------------
//		Compile time local build options
// --------------------------------------------------------------------------
#define A0PATCH 1
//#define BLOCKOUT_COUNTER 0
#define TIMEOUT_10MS        (HZ/100)
#define TIMEOUT_1S        (HZ)
//#define GPIO_DEBUG
//#define CES2006_DEMO
// --------------------------------------------------------------------------
//		GPIO DEBUG MACROS
// --------------------------------------------------------------------------
#ifdef GPIO_DEBUG
  //0 set to output and set outputs to hi
  #define GPIO_DEBUG_INIT	    andREG32( GPIO_DIR_HI, ~(GPIO38+GPIO46+GPIO47) ); \
								orREG32( GPIO_DATA_HI, (GPIO38+GPIO46+GPIO47) )

  #define GPIO_SET_LO( pin )	andREG32( GPIO_DATA_HI, ~pin )
  #define GPIO_SET_HI( pin )	orREG32( GPIO_DATA_HI, pin )
#else
  #define GPIO_DEBUG_INIT
  #define GPIO_SET_LO( pin )
  #define GPIO_SET_HI( pin )
#endif

#if (PASS_THROUGH)
#define SKB_ALIGNMENT		32
 // --------------------------------------------------------------------------
//      Driver parameter.
// --------------------------------------------------------------------------
ushort tagged_vdevs = 1; 
module_param(tagged_vdevs, ushort, S_IRUGO);

static int tagging_disable = 0;

static struct _vdev_names vdev_names[] = {
    { VDEV0_IFNUM, "eth0", {0x00, 0xde, 0xad, 0xbe, 0xef, 0x00} },
    { VDEV1_IFNUM, "bcm0", {0x00, 0xde, 0xad, 0xbe, 0xef, 0x01} }
};

static unsigned char *tempBuffer;  
#endif
// --------------------------------------------------------------------------
//      External, indirect entry points.
// --------------------------------------------------------------------------
extern void bcm_inv_rac_all(void);
extern unsigned long getPhysFlashBase(void);

static int bcmpcieth_enet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
static int bcmIsEnetUp(uint32 uPhyAddr);

// --------------------------------------------------------------------------
//      Called for "ifconfig ethX up" & "down"
// --------------------------------------------------------------------------
static int bcmpcieth_net_open(struct net_device * dev);
static int bcmpcieth_net_close(struct net_device * dev);
// --------------------------------------------------------------------------
//      Watchdog timeout
// --------------------------------------------------------------------------
static void bcmpcieth_net_timeout(struct net_device * dev);
// --------------------------------------------------------------------------
//      Packet transmission.
// --------------------------------------------------------------------------
static int bcmpcieth_net_xmit(struct sk_buff * skb, struct net_device * dev);
static int bcmpcieth_locked_net_xmit(struct sk_buff * skb, struct net_device * dev);

// --------------------------------------------------------------------------
//      Allow proc filesystem to query driver statistics
// --------------------------------------------------------------------------
static struct net_device_stats * bcmpcieth_net_query(struct net_device * dev);
// --------------------------------------------------------------------------
//      Set address filtering mode
// --------------------------------------------------------------------------
static void bcm_set_multicast_list(struct net_device * dev);
// --------------------------------------------------------------------------
//      Set the hardware MAC address.
// --------------------------------------------------------------------------
static int bcm_set_mac_addr(struct net_device *dev, void *p);

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//      Interrupt routines
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static irqreturn_t bcmpcieth_net_isr(int irq, void *);
#else
static irqreturn_t bcmpcieth_net_isr(int irq, void *, struct pt_regs *regs);
#endif
// --------------------------------------------------------------------------
//  Bottom half service routine. Process all received packets.
// --------------------------------------------------------------------------
static void bcmpcieth_rx(void *);

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//      Internal routines
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

/* Remove registered netdevice */
static void bcmpcieth_init_cleanup(struct net_device *dev);
/* Allocate and initialize tx/rx buffer descriptor pools */
static int bcmpcieth_init_dev(BcmEnet_devctrl *pDevCtrl);
/* Assign the Rx descriptor ring */
static int assign_rx_buffers(BcmEnet_devctrl *pDevCtrl);
/* Initialize driver's pools of receive buffers and tranmit headers */
static int init_buffers(BcmEnet_devctrl *pDevCtrl);

/* Reclaim transmit frames which have been sent out */
static void mb_timer_handler(unsigned long arg);
static void refill_timer_handler(unsigned long arg);

/* Add a Tx control block to the pool */
static void txCb_enq(BcmEnet_devctrl *pDevCtrl, Enet_Tx_CB *txCbPtr);
/* Remove a Tx control block from the pool*/
static Enet_Tx_CB *txCb_deq(BcmEnet_devctrl *pDevCtrl);

#ifdef DUMP_DATA
/* Display hex base data */
static void dumpHexData(unsigned char *head, int len);

/* dumpMem32 dump out the number of 32 bit hex data  */
static void dumpMem32(uint32 * pMemAddr, int iNumWords);
#endif

#ifdef CES2006_DEMO
int initcode( void );
void uninitcode( void );
#endif

static void pci_mbox_tasklet ( unsigned long not_used );

// --------------------------------------------------------------------------
//      Local variables
// --------------------------------------------------------------------------
#define DS_PCI_DMA_IRQ_MASK	(PCI_DMA_PKT_DONE | PCI_DMA_DESC_DONE | PCI_DMA_NOTMY_DESC)
#define US_PCI_DMA_IRQ_MASK (PCI_DMA_PKT_DONE | PCI_DMA_DESC_DONE )

static unsigned short csumoe = 0;
module_param(csumoe, ushort, S_IRUGO);

/* This will prevent from running multiple instances of this driver */
/* I guess it's okay with 3368 or 3255 */
static BcmEnet_devctrl *pDevContext = NULL;
#if (PASS_THROUGH)
static struct BcmVirtEnet_devctrl *pVirtDevCtrl = NULL;
#endif
static  int loopCount = 0;  // This should be local to the rx routine, but we need it here so that
					    // dump_emac() can display it.  It is not really a global var.
static unsigned long savedmailbox1;
static unsigned long MboxIntStatus;
static BcmEnet_DocsisNonvol localDocsisNonvol;
DECLARE_TASKLET( PCIMBOX_TASK, pci_mbox_tasklet, 0 );
#if defined(BCM7118)
static uint32 mcode = 0;
static uint8 ready_cnt = 0;
#if defined(LED_SUPPORT)
static uint8 cm_ready = 0;
#endif
#endif

#if !defined(BCM7118)
/* 7401 C0 workaround*/
static int putwindow_buf[2048];
#else
static uint32 imagePointer;
static int usize = 0;
static int ubase = 0;
static unsigned int bnmBootAddress;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
module_param(usize, int, 0);
module_param(ubase, int, 0);
#else
MODULE_PARM(usize, "i");
MODULE_PARM(ubase, "i");
#endif
#endif

#ifdef DUMP_DATA
/*
 * dumpHexData dump out the hex base binary data
 */
static void dumpHexData(unsigned char *head, int len)
{
    int i;
    unsigned char *curPtr = head;

    for (i = 0; i < len; ++i)
    {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X ", *curPtr++);
    }
    printk("\n");

} //dumpHexData

/*
 * dumpMem32 dump out the number of 32 bit hex data
 */
static void dumpMem32(uint32 * pMemAddr, int iNumWords)
{
    int i = 0;
    static char buffer[80];

    sprintf(buffer, "%08X: ", (UINT)pMemAddr);
    printk(buffer);
    while (iNumWords) {
        sprintf(buffer, "%08X ", (UINT)*pMemAddr++);
        printk(buffer);
        iNumWords--;
        i++;
        if ((i % 4) == 0 && iNumWords) {
            sprintf(buffer, "\n%08X: ", (UINT)pMemAddr);
            printk(buffer);
        }
    }
    printk("\n");
}

static void dumpRTSSetting(int i)
{
	unsigned long *rts = ((unsigned long *)0xb0106004) + i;
	printk("Client %02d, REG = %08x, BO_VAL = %06d, PR_EN = %d, PR_TAG = %d\n", i, *rts, (*rts & 0x001fff00) >> 8,
		(*rts & 0x00000040) >> 6, *rts & 0x0000003f);
}

static void dumpRTSSettings(void)
{
	int i;
	for (i = 0; i < 64; i++)
	{
		dumpRTSSetting(i);
	}
}

#endif

/* --------------------------------------------------------------------------
    Name: bcmpcieth_net_open
 Purpose: Open and Initialize the EMAC on the chip
-------------------------------------------------------------------------- */
static int bcmpcieth_net_open(struct net_device * dev)
{
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;	
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#else
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif
    unsigned long flags;

    ASSERT(pDevCtrl != NULL);

	TRACE(("%s: bcmpcieth_net_open, &RxDMA=%x, rxDma.Control=%lx\n",
    dev->name, (unsigned int)&pDevCtrl->rxDma, pDevCtrl->rxDma->Control));

    //MOD_INC_USE_COUNT;
#if (PASS_THROUGH)
     if (pDevCtrl->open_count++) {
        return 0;
    }
    tempBuffer = (unsigned char *) kmalloc(ENET_MAX_MTU_SIZE +4, GFP_ATOMIC);
#endif

    spin_lock_irqsave(&pDevCtrl->lock, flags);

    atomic_inc(&pDevContext->devInUsed);
    pDevCtrl->rxDma->IntMask = DMA_DONE|DMA_NO_DESC;

	/* Enable mailbox receiving interrupts */
#if defined(BCM7118)
    pDevCtrl->nsGisbRegs->hostIrqMask |= SCB_BRIDGE_IRQ_MASK; // bit 31 : SCB_BRIDGE_IRQ
    pDevCtrl->nsGisbRegs->hostIrqMask |= GISB_MBOX1_IRQ_MASK;
#else
    pDevCtrl->mpiRegs->PciIntStat = PCI_MBOX1_RCV_INTENB_BIT;
#endif

    /* Are we previously closed by bcmpcieth_net_close, which set InUsed to -1 */
    if (0 == atomic_read(&pDevContext->devInUsed)) {
       atomic_inc(&pDevContext->devInUsed);
	   TRACE(("%s: bcmpcieth_net_open: enable rx interrupt!\n", dev->name));
       BcmHalInterruptEnable(pDevCtrl->rxIrq);
    }

    pDevCtrl->reset_timer.expires = jiffies + TIMEOUT_1S;
    add_timer(&pDevCtrl->reset_timer);

#if defined(BCM7118)
    pDevCtrl->reboot_timer.expires = jiffies + (TIMEOUT_1S * 8);
    add_timer(&pDevCtrl->reboot_timer);
#endif

    /* enable DMA from the PCI side */
	pDevCtrl->rxDma->Control = PCI_DMA_ENABLE;

	/* Start the network engine */
    netif_start_queue(dev);
 
    spin_unlock_irqrestore(&pDevCtrl->lock, flags);

#if defined(BCM7118)
	TRACE(("%s: bcmpcieth_net_open, rxDMA=%lx, rxDma->Control=%lx rxDma->DescriptorBase=%lx\n",
    dev->name, (uint32)pDevCtrl->rxDma, pDevCtrl->rxDma->Control, pDevCtrl->rxDmaState->DescriptorBase));
#else
	TRACE(("%s: bcmpcieth_net_open, rxDMA=%x, rxDma->Control=%x rxDma->DescriptorAddress=%x\n",
    dev->name, pDevCtrl->rxDma, pDevCtrl->rxDma->Control, pDevCtrl->rxDma->DescriptorAddress));
#endif
#ifdef DUMP_DATA
	printk("RTS Settings\n");
	dumpRTSSettings();
#endif
    return 0;
}
#if (PASS_THROUGH)
static inline void handleAlignment(struct BcmEnet_devctrl *pDevCtrl,
                                   struct sk_buff *skb) 
    {
    /*
     * We request to allocate 2048 + 32 bytes of buffers, and the
     * dev_alloc_skb() added 16B for NET_SKB_PAD, so we totally
     * requested 2048+32+16 bytes buffer, the size was aligned to
     * SMP_CACHE_BYTES, which is 64B.(is it?), so we finnally ended
     * up got 2112 bytes of buffer! Among which, the first 16B is
     * reserved for NET_SKB_PAD, to make the skb->data aligned 32B
     * boundary, we should have enough space to fullfill the 2KB
     * buffer after alignment!
     */

    unsigned long boundary32, curData, resHeader;

    curData = (unsigned long) skb->data;
    boundary32 = (curData + (SKB_ALIGNMENT - 1)) & ~(SKB_ALIGNMENT - 1);
    resHeader = boundary32 - curData ;
    /* 4 bytes for skb pointer */
    if (resHeader < 4) {
        boundary32 += 32;
    }

    resHeader = boundary32 - curData - 4;
    /* We'd have minimum 16B reserved by default. */
    skb_reserve(skb, resHeader);

    *(unsigned int *)skb->data = (unsigned int)skb;
    skb_reserve(skb, 4);
    /*
     * Make sure it is on 32B boundary, should never happen if our
     * calculation was correct.
     */
    if ((unsigned long) skb->data & (SKB_ALIGNMENT - 1)) 
        {
        printk(KERN_WARNING "skb buffer is NOT aligned on %d boundary!\n",
               SKB_ALIGNMENT);
        }
    }

struct net_device *bcmgenet_match_tag(struct BcmEnet_devctrl *pDevCtrl, struct sk_buff *skb, unsigned int len, unsigned int *vdev_num) 
    {
    unsigned int f_len, dev_num;

    f_len = skb->data[len - 2];
    f_len <<= 8;
    f_len |= skb->data[len - 1];
    skb_trim(skb, f_len);
    *vdev_num = ((len - f_len) / 2) - 1;
	dev_num = *vdev_num;
	if ((dev_num == NUM_TAGS) || (dev_num == (NUM_TAGS + 1))) 
	    {
		dev_num = 1;
	    }
    if ((dev_num >= 1) && (dev_num < NUM_TAGS)) 
        {
            return pDevCtrl->p_virt_dev[dev_num - 1];	
        } else if (dev_num == 0) 
        {
            return pDevCtrl->dev;	
        }
    return 0;
    }
#endif
/* --------------------------------------------------------------------------
    Name: bcmpcieth_net_close
 Purpose: Stop communicating with the outside world
    Note: Caused by 'ifconfig ethX down'
-------------------------------------------------------------------------- */
static int bcmpcieth_net_close(struct net_device * dev)
{
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#else
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif
    Enet_Tx_CB *txCBPtr;
    unsigned long flags;

    ASSERT(pDevCtrl != NULL);

    TRACE(("%s: bcmpcieth_net_close\n", dev->name));
#if (PASS_THROUGH)
     if (--pDevCtrl->open_count) {
        return 0;
    }
    kfree(tempBuffer);
#endif
    netif_stop_queue(dev);

    //MOD_DEC_USE_COUNT;
    spin_lock_irqsave(&pDevCtrl->lock, flags);

    atomic_dec(&pDevContext->devInUsed);

    pDevCtrl->rxDma->Control &= ~DMA_ENABLE;

    if (0 == atomic_read(&pDevContext->devInUsed)) {
       atomic_set(&pDevContext->devInUsed, -1); /* Mark interrupt disable */
    	BcmHalInterruptDisable(pDevCtrl->rxIrq);
    }

    del_timer_sync(&pDevCtrl->reset_timer);
    del_timer_sync(&pDevCtrl->refill_timer);
#if defined(BCM7118)
    del_timer_sync(&pDevCtrl->reboot_timer);
#endif

    /* free the skb in the txCbPtrHead */
    while (pDevCtrl->txCbPtrHead)  {
        pDevCtrl->txFreeBds += pDevCtrl->txCbPtrHead->nrBds;

        dev_kfree_skb (pDevCtrl->txCbPtrHead->skb);

        txCBPtr = pDevCtrl->txCbPtrHead;

        /* Advance the current reclaim pointer */
        pDevCtrl->txCbPtrHead = pDevCtrl->txCbPtrHead->next;

        /* Finally, return the transmit header to the free list */
        txCb_enq(pDevCtrl, txCBPtr);
    }
    /* Adjust the tail if the list is empty */
    if(pDevCtrl->txCbPtrHead == NULL)
        pDevCtrl->txCbPtrTail = NULL;

    spin_unlock_irqrestore(&pDevCtrl->lock, flags);

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcmpcieth_net_timeout
 Purpose:
-------------------------------------------------------------------------- */
static void bcmpcieth_net_timeout(struct net_device * dev)
{
    BcmEnet_devctrl *pDevCtrl;
    unsigned long flags;

	ASSERT(dev != NULL);
#if (PASS_THROUGH)
	 pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	pDevCtrl = netdev_priv(dev);
#else
   	pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif

    printk(KERN_INFO CARDNAME ": %s: bcmpcieth_net_timeout\n", dev->name);

	if (pDevCtrl->PciHostDmaReady)
	{
		/* try to reclaim buffers */
		spin_lock_irqsave(&pDevCtrl->lock, flags);
		bcmpcieth_net_xmit(NULL, pDevCtrl->dev);
		spin_unlock_irqrestore(&pDevCtrl->lock, flags);

		dev->trans_start = jiffies;
		netif_wake_queue(dev);

#if defined(BCM7118)
		pDevCtrl->txDma->Control |= DMA_ENABLE;
#else
		pDevCtrl->txDma->Control |= (PCI_DMA_LE | PCI_DMA_ENABLE);
#endif
		pDevCtrl->rxDma->Control |= DMA_ENABLE;
	}
}

/* --------------------------------------------------------------------------
    Name: bcm_set_multicast_list
 Purpose: Set the multicast mode, ie. promiscuous or multicast
-------------------------------------------------------------------------- */
static void bcm_set_multicast_list(struct net_device * dev)
{
    ASSERT(dev != NULL);

    TRACE(("%s: bcm_set_multicast_list: %08X\n", dev->name, dev->flags));
}

/* --------------------------------------------------------------------------
    Name: bcmpcieth_net_query
 Purpose: Return the current statistics. This may be called with the card
          open or closed.
-------------------------------------------------------------------------- */
static struct net_device_stats *
bcmpcieth_net_query(struct net_device * dev)
{
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;		
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#else
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif

    ASSERT(pDevCtrl != NULL);

    return &(pDevCtrl->stats);
}

#if defined(BCM7118)
static void reboot_timer_handler(unsigned long arg)
{
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)arg;

    if(!pDevCtrl->PciHostDmaReady)
    {
        printk(KERN_INFO CARDNAME ": BNM Reset Failed - Rebooting\n");
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
		*((volatile unsigned long*)SUN_TOP_CTRL_SW_RESET) |= 0x80000000;
#else
		machine_restart(NULL);
#endif
    }
}
#endif

#if defined(BCM7118)
static void mb_timer_handler(unsigned long arg)
{
#if defined(LED_SUPPORT)
	uint32 temp;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)arg;

	if(cm_ready == 0)
	{
		temp = *(uint32 *)GIO_DATA_HI;

		/* GPIO_45 */
		*(uint32 *)GIO_DATA_HI = (temp & 0xffffdfff) | (0x00002000 & (~(temp & 0x00002000)));

		/* rearm the timer */
		pDevCtrl->reset_timer.expires = jiffies + TIMEOUT_1S;
		add_timer(&pDevCtrl->reset_timer);
	}
#endif
}
#else
static void recover_3255_pci(BcmEnet_devctrl *pDevCtrl)
{
    // At this point we are toast. Need to reboot host.
    // This hack restores the PCI controller to its state before reset
    unsigned long flags;
    int i;

    // get exclusive access
    spin_lock_irqsave(&pDevCtrl->lock, flags);

    // Throw away any queued mailbox messages
    pDevCtrl->mailbox1 = 0;

    // Restore the config space
    pci_write_config_word(pDevCtrl->pdev, PCI_COMMAND,
        PCI_COMMAND_SERR | PCI_COMMAND_PARITY | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
    pci_write_config_dword(pDevCtrl->pdev, PCI_BASE_ADDRESS_0, (uint32)pci_resource_start(pDevContext->pdev, 0) | 8);
    pci_write_config_dword(pDevCtrl->pdev, PCI_BASE_ADDRESS_3, (uint32)pci_resource_start(pDevContext->pdev, 3));
    pci_write_config_dword(pDevCtrl->pdev, PCI_BASE_ADDRESS_4, (uint32)pci_resource_start(pDevContext->pdev, 4));
    pci_write_config_dword(pDevCtrl->pdev, PCI_INTERRUPT_LINE, 0x121);

    // Capture any buffers received up to this point
    bcmpcieth_rx(pDevCtrl);
    // Reclaim any transmitted buffers
    bcmpcieth_net_xmit(NULL, pDevCtrl->dev);

    /* initialize rx ring pointer variables. */
    pDevCtrl->rxBdAssignPtr = pDevCtrl->rxBdReadPtr =
        pDevCtrl->rxFirstBdPtr = pDevCtrl->rxBds;
    pDevCtrl->rxLastBdPtr = pDevCtrl->rxFirstBdPtr + pDevCtrl->nrRxBds - 1;

    // Reset the flags and length in the RX descriptor ring
    for (i = 0; i < pDevCtrl->nrRxBds; i++)
    {
        (pDevCtrl->rxFirstBdPtr + i)->length = htons(pDevCtrl->rxBufLen);
        (pDevCtrl->rxFirstBdPtr + i)->status = htons(DMA_OWN);
    }
    pDevCtrl->rxLastBdPtr->status |= htons(DMA_WRAP);

    /* init transmit buffer descriptor variables */
    pDevCtrl->txNextBdPtr = pDevCtrl->txFirstBdPtr = pDevCtrl->txBds;
    pDevCtrl->txLastBdPtr = pDevCtrl->txFirstBdPtr + pDevCtrl->nrTxBds - 1;

    /* clear the transmit buffer descriptors */
    for (i = 0; i < pDevCtrl->nrTxBds; i++)
    {
        (pDevCtrl->txFirstBdPtr + i)->length = 0;
        (pDevCtrl->txFirstBdPtr + i)->status = 0;
        (pDevCtrl->txFirstBdPtr + i)->address = 0;
    }
    pDevCtrl->txLastBdPtr->status |= htons(DMA_WRAP);
    pDevCtrl->txFreeBds = pDevCtrl->nrTxBds;

    /* Enable mailbox receiving interrupts */
    pDevCtrl->mpiRegs->PciIntStat = PCI_MBOX1_RCV_INTENB_BIT;

    // release exclusive access
    spin_unlock_irqrestore(&pDevCtrl->lock, flags);

}

static void mb_timer_handler(unsigned long arg)
{
    uint16 cmd;
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)arg;
#endif
    pci_read_config_word(pDevCtrl->pdev, PCI_COMMAND, &cmd);

    if (cmd == 0) {
        printk(KERN_INFO CARDNAME ": 3255 has reset. Reinitializing 3255 PCI\n");
        recover_3255_pci(pDevCtrl);
        pDevCtrl->PciResetDetected = TRUE;
    }

    pDevCtrl->reset_timer.expires = jiffies + TIMEOUT_1S;
	add_timer(&pDevCtrl->reset_timer);
}
#endif

static void refill_timer_handler(unsigned long arg)
{
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)arg;
#endif

	if (atomic_read(&pDevCtrl->rxDmaRefill) == 1)
	{
		assign_rx_buffers(pDevCtrl);
		if (atomic_read(&pDevCtrl->rxDmaRefill) == 1)
		{
			pDevCtrl->refill_timer.expires = jiffies + TIMEOUT_10MS;
			add_timer(&pDevCtrl->refill_timer);
		}
		else
		{
			pDevCtrl->rxDma->Control |= DMA_ENABLE;
		}
	}
}

/*
 * txCb_enq: add a Tx control block to the pool
 */
static void txCb_enq(BcmEnet_devctrl *pDevCtrl, Enet_Tx_CB *txCbPtr)
{
    txCbPtr->next = NULL;

    if (pDevCtrl->txCbQTail) {
        pDevCtrl->txCbQTail->next = txCbPtr;
        pDevCtrl->txCbQTail = txCbPtr;
    }
    else {
        pDevCtrl->txCbQHead = pDevCtrl->txCbQTail = txCbPtr;
    }
}

/*
 * txCb_deq: remove a Tx control block from the pool
 */
static Enet_Tx_CB *txCb_deq(BcmEnet_devctrl *pDevCtrl)
{
    Enet_Tx_CB *txCbPtr;

    if ((txCbPtr = pDevCtrl->txCbQHead)) {
        pDevCtrl->txCbQHead = txCbPtr->next;
        txCbPtr->next = NULL;

        if (pDevCtrl->txCbQHead == NULL)
            pDevCtrl->txCbQTail = NULL;
    }
    else {
        txCbPtr = NULL;
    }

    return txCbPtr;
}

/* --------------------------------------------------------------------------
    Name: bcmpcieth_net_xmit
 Purpose: Send ethernet traffic
-------------------------------------------------------------------------- */
static int bcmpcieth_locked_net_xmit(struct sk_buff * skb, struct net_device * dev)
{
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#else
   	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif
    int rc = 0;
    unsigned long flags;
#if !defined(BCM7118)
    unsigned short cmd;
#endif

    // Make sure the 3255 hasn't spontaneously reset. There's still a race
    // opportunity here, but minimize it.
#if defined(BCM7118)
    if ( pDevCtrl->PciHostDmaReady == TRUE )
#else
    if ((pDevCtrl->PciHostDmaReady == TRUE) &&
        (pci_read_config_word(pDevCtrl->pdev, PCI_COMMAND, &cmd) == 0) &&
        (cmd != 0))
#endif
	{
        spin_lock_irqsave(&pDevCtrl->lock, flags);

        rc = bcmpcieth_net_xmit(skb, dev);
        spin_unlock_irqrestore(&pDevCtrl->lock, flags);
    }
    else {
        netif_stop_queue(dev);
        rc = 1;
    }
    return rc;
}

/****************************************************************************
-----------------------------------------------------------------------------
***** Broadcom proprietary scatter/gather packet transmit interface *****
-----------------------------------------------------------------------------
*****************************************************************************/
struct net_device *bcmpcieth_get_device(void)
{
	if(pDevContext)
	{
		return pDevContext->dev;
	}
	else
	{
		return NULL;
	}
}
/* --------------------------------------------------------------------------
    Name: bcmpcieth_get_free_txdesc
 Purpose: Get Current Available TX desc count
-------------------------------------------------------------------------- */
int bcmpcieth_get_free_txdesc( struct net_device *net ){
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(net);
#else
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)net->priv;
#endif
#endif
	return pDevCtrl->txFreeBds;
}

/* --------------------------------------------------------------------------
    Name: bcmpcieth_xmit_check
    Purpose: Reclaims TX descriptors
-------------------------------------------------------------------------- */
int bcmpcieth_xmit_check(struct net_device * dev)
{
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#else
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif
    unsigned long flags,ret;

	if( pDevCtrl->PciHostDmaReady == FALSE )
	{
		return 1;
	}

	ASSERT(pDevCtrl != NULL);

    spin_lock_irqsave(&pDevCtrl->lock, flags);
    ret = bcmpcieth_net_xmit(NULL, pDevCtrl->dev);
    spin_unlock_irqrestore(&pDevCtrl->lock, flags);

	return ret;
}
/* --------------------------------------------------------------------------
    Name: bcmpcieth_xmit_fragment
 Purpose: Send ethernet traffic Buffer DESC and submit to UDMA
-------------------------------------------------------------------------- */
int bcmpcieth_xmit_fragment( int ch, unsigned char *buf, int buf_len,
						   unsigned long tx_flags , struct net_device *dev)
{
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#else
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif
    Enet_Tx_CB *txCBPtr;
    volatile DmaDesc *firstBdPtr;
	struct pci_dev *pdev = pDevCtrl->pdev;

    ASSERT(pDevCtrl != NULL);

    txCBPtr = txCb_deq(pDevCtrl);

    /*
     * Obtain a transmit header from the free list.  If there are none
     * available, we can't send the packet. Discard the packet.
     */
    if (txCBPtr == NULL) {
        return 1;
    }

    txCBPtr->nrBds = 1;
    txCBPtr->skb = NULL;
#if defined(DMA_SG_TEST)
	/* for test purpose only */
	if(DMA_EOP & tx_flags)
	{
		txCBPtr->skb = (struct sk_buff *)ch;
	}
#endif

    /* If we could not queue this packet, free it */
    if (pDevCtrl->txFreeBds < txCBPtr->nrBds) {
        TRACE(("%s: bcmpcieth_net_xmit low on txFreeBds\n", dev->name));
        return 1;
    }

	/*--------first fragment------*/
    firstBdPtr = pDevCtrl->txNextBdPtr;
    /* store off the last BD address used to enqueue the packet */
    txCBPtr->lastBdAddr = pDevCtrl->txNextBdPtr;

    /* assign skb data to TX Dma descriptor */
    /*
     * Add the buffer to the ring.
     * Set addr and length of DMA BD to be transmitted.
     */
	txCBPtr->dma_addr = pci_map_single(pdev, (void *)buf, (size_t)buf_len, PCI_DMA_TODEVICE);

	pDevCtrl->txNextBdPtr->address = htonl((uint32)txCBPtr->dma_addr | NEW_PCI_7038_PHYS_MEM_WIN0_BASE);
    pDevCtrl->txNextBdPtr->length = htons(buf_len);
    /*
     * Set status of DMA BD to be transmitted and
     * advance BD pointer to next in the chain.
     */
    if (pDevCtrl->txNextBdPtr == pDevCtrl->txLastBdPtr) {
        pDevCtrl->txNextBdPtr->status = htons(DMA_WRAP);
        pDevCtrl->txNextBdPtr = pDevCtrl->txFirstBdPtr;
    }
    else {
        pDevCtrl->txNextBdPtr->status = 0;
        pDevCtrl->txNextBdPtr++;
    }

    /*
     * Turn on the "OWN" bit in the first buffer descriptor
     * This tells the switch that it can transmit this frame.
     */
    firstBdPtr->status |= htons(DMA_OWN | tx_flags);


    /* Enable DMA for this channel */
#if defined(BCM7118)
    pDevCtrl->txDma->Control |= DMA_ENABLE;
#else
    pDevCtrl->txDma->Control |= (PCI_DMA_LE | PCI_DMA_ENABLE);
#endif

    /* Decrement total BD count */
    pDevCtrl->txFreeBds -= txCBPtr->nrBds;

	/*
     * Packet was enqueued correctly.
     * Advance to the next Enet_Tx_CB needing to be assigned to a BD
     */
    txCBPtr->next = NULL;
    if(pDevCtrl->txCbPtrHead == NULL) {
        pDevCtrl->txCbPtrHead = txCBPtr;
        pDevCtrl->txCbPtrTail = txCBPtr;
    }
    else{
        pDevCtrl->txCbPtrTail->next = txCBPtr;
        pDevCtrl->txCbPtrTail = txCBPtr;
    }

    /* update stats */
    pDevCtrl->stats.tx_bytes += buf_len;
	if(DMA_EOP & tx_flags)
	{
		pDevCtrl->stats.tx_packets++;
	}

    dev->trans_start = jiffies;

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcmpcieth_xmit_multibuf
 Purpose: Send ethernet traffic in multi buffers (hdr, buf, tail)
-------------------------------------------------------------------------- */
int bcmpcieth_xmit_multibuf( int ch, unsigned char *hdr, int hdr_len, unsigned char *buf, int buf_len, unsigned char *tail, int tail_len, struct net_device *dev)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#endif
	unsigned int cycles_per_jiffy = (594000000/2 + HZ / 2) / HZ; /* TODO: define CPU_CLOCK_RATE somewhere */
#else
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
	unsigned int cycles_per_jiffy = (CPU_CLOCK_RATE/2 + HZ / 2) / HZ;
#endif
    unsigned long flags;
	unsigned int start, end;

	spin_lock_irqsave(&pDevCtrl->lock, flags);
													
	/* read the clock counts */
	start = read_c0_count();

	/* Header + Optional payload in two parts */
	if((hdr_len> 0) && (buf_len > 0) && (tail_len > 0) && (hdr) && (buf) && (tail)){
		while(bcmpcieth_get_free_txdesc(dev) < 3)
		{
			/* read the clock counts */
			end = read_c0_count();
			if((unsigned int)(end - start) > cycles_per_jiffy)
			{
			    spin_unlock_irqrestore(&pDevCtrl->lock, flags);
#if defined(DMA_SG_TEST)
		        netif_stop_queue(dev);
#endif
				return 1;
			}
			bcmpcieth_xmit_check(dev);
		}

		/* Send Header */
		bcmpcieth_xmit_fragment( ch, hdr, hdr_len, DMA_SOP, dev);
		/* Send First Fragment */
		bcmpcieth_xmit_fragment( ch, buf, buf_len, 0, dev);
										/* Send 2nd Fragment */
		bcmpcieth_xmit_fragment( ch, tail, tail_len, DMA_EOP, dev);
	}
	/* Header + Optional payload */
	else if((hdr_len> 0) && (buf_len > 0) && (hdr) && (buf)){
		while(bcmpcieth_get_free_txdesc(dev) < 2)
		{
			/* read the clock counts */
			end = read_c0_count();
			if((unsigned int)(end - start) > cycles_per_jiffy)
			{
			    spin_unlock_irqrestore(&pDevCtrl->lock, flags);
#if defined(DMA_SG_TEST)
		        netif_stop_queue(dev);
#endif
				return 1;
			}
			bcmpcieth_xmit_check(dev);
		}

		/* Send Header */
		bcmpcieth_xmit_fragment( ch, hdr, hdr_len, DMA_SOP, dev);
		/* Send First Fragment */
		bcmpcieth_xmit_fragment( ch, buf, buf_len, DMA_EOP, dev);
	}
	/* Header Only (includes payload) */
	else if((hdr_len> 0) && (hdr)){
		while(bcmpcieth_get_free_txdesc(dev) < 1)
		{
			/* read the clock counts */
			end = read_c0_count();
			if((unsigned int)(end - start) > cycles_per_jiffy)
			{
			    spin_unlock_irqrestore(&pDevCtrl->lock, flags);
#if defined(DMA_SG_TEST)
		        netif_stop_queue(dev);
#endif
				return 1;
			}
			bcmpcieth_xmit_check(dev);
		}

		bcmpcieth_xmit_fragment( ch, hdr, hdr_len, DMA_SOP | DMA_EOP, dev);
	}

    spin_unlock_irqrestore(&pDevCtrl->lock, flags);
	return 0;
}

static int bcmpcieth_net_xmit(struct sk_buff * skb, struct net_device * dev)
{
#if (PASS_THROUGH)
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;	
    struct BcmVirtEnet_devctrl *pVirtDevCtrl = (struct BcmVirtEnet_devctrl *)dev->priv;		
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#else
   	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif

	struct pci_dev *pdev = pDevCtrl->pdev;
    Enet_Tx_CB *txCBPtr;
    Enet_Tx_CB *txedCBPtr;
    volatile DmaDesc *firstBdPtr;

    ASSERT(pDevCtrl != NULL);

	/* validate data pointer and packet length */
	if( skb && (((void *)skb->data == NULL) || ((size_t)skb->len == 0)) )
	{
		return 1;
	}
#if (PASS_THROUGH)
     if (tagging_disable && pDevCtrl->has_virt_dev && (pVirtDevCtrl->if_num > 0)) {
        dev_kfree_skb_any(skb);
        return 1;
    }
#endif
    /*
     * Obtain exclusive access to transmitter.  This is necessary because
     * we might have more than one stack transmitting at once.
     */

    /* Reclaim transmitted buffers */
    while (pDevCtrl->txCbPtrHead)  {
        if (ntohs(pDevCtrl->txCbPtrHead->lastBdAddr->status) & DMA_OWN) {
            break;
        }

        pDevCtrl->txFreeBds += pDevCtrl->txCbPtrHead->nrBds;

		if(pDevCtrl->txCbPtrHead->skb)
		{
			dev_kfree_skb (pDevCtrl->txCbPtrHead->skb);
		}

        txedCBPtr = pDevCtrl->txCbPtrHead;

        /* Advance the current reclaim pointer */
        pDevCtrl->txCbPtrHead = pDevCtrl->txCbPtrHead->next;

        /*
         * Finally, return the transmit header to the free list.
         */
            txCb_enq(pDevCtrl, txedCBPtr);
        }

    /* Adjust the tail if the list is empty */
    if(pDevCtrl->txCbPtrHead == NULL)
        pDevCtrl->txCbPtrTail = NULL;

    if (skb == NULL) {
        // TRACE(("%s: bcmpcieth_net_xmit NULL skb\n", dev->name));
		if(pDevCtrl->txCbQHead && (pDevCtrl->txFreeBds>0))
		{
			return 0;
		}
        return 1;
    }

	/* discard packets if PCI DMA is not ready */
	if( pDevCtrl->PciHostDmaReady == FALSE )
	{
		TRACE(("bcmPciEnetSend: PciHost not ready! Discarding....\n"));
        netif_stop_queue(dev);
        return 1;
	}

    TRACE(("bcmpcieth_net_xmit, txCfg=%08lx, txIntStat=%08lx\n", pDevCtrl->txDma->Control, pDevCtrl->txDma->IntStat));
    txCBPtr = txCb_deq(pDevCtrl);

    /*
     * Obtain a transmit header from the free list.  If there are none
     * available, we can't send the packet. Discard the packet.
     */
    if (txCBPtr == NULL) {
        netif_stop_queue(dev);
        return 1;
    }

    txCBPtr->nrBds = 1;
    txCBPtr->skb = skb;

    /* If we could not queue this packet, free it */
    if (pDevCtrl->txFreeBds < txCBPtr->nrBds) {
        TRACE(("%s: bcmpcieth_net_xmit low on txFreeBds\n", dev->name));
        txCb_enq(pDevCtrl, txCBPtr);
        netif_stop_queue(dev);
        return 1;
    }

    firstBdPtr = pDevCtrl->txNextBdPtr;
    /* store off the last BD address used to enqueue the packet */
    txCBPtr->lastBdAddr = pDevCtrl->txNextBdPtr;
#if (PASS_THROUGH)
#ifdef PACKET_COPY
    memcpy(&tempBuffer[0], (void *)skb->data, skb->len);
    tempBuffer[skb->len] = (unsigned char)(skb->len >> 8);
    tempBuffer[skb->len+1] = (unsigned char)(skb->len);		
    tempBuffer[skb->len+2] = (unsigned char)(TAG_MAGIC);
    tempBuffer[skb->len+3] = (unsigned char)pVirtDevCtrl->if_num;
#endif
#endif      
    /* assign skb data to TX Dma descriptor */
    /*
     * Add the buffer to the ring.
     * Set addr and length of DMA BD to be transmitted.
     */
#if (PASS_THROUGH)
#ifdef PACKET_COPY
  	txCBPtr->dma_addr = pci_map_single(pdev, (void *)&tempBuffer[0], (size_t)skb->len+4, PCI_DMA_TODEVICE);
#else
  	txCBPtr->dma_addr = pci_map_single(pdev, (void *)skb->data, (size_t)skb->len, PCI_DMA_TODEVICE);
#endif	
	pDevCtrl->txNextBdPtr->address = htonl((uint32)txCBPtr->dma_addr | NEW_PCI_7038_PHYS_MEM_WIN0_BASE);
#ifdef PACKET_COPY
    pDevCtrl->txNextBdPtr->length = htons(skb->len +4 );
#else
    pDevCtrl->txNextBdPtr->length = htons(skb->len );
#endif
#else
	txCBPtr->dma_addr = pci_map_single(pdev, (void *)skb->data, (size_t)skb->len, PCI_DMA_TODEVICE);
	pDevCtrl->txNextBdPtr->address = htonl((uint32)txCBPtr->dma_addr | NEW_PCI_7038_PHYS_MEM_WIN0_BASE);
    pDevCtrl->txNextBdPtr->length = htons(skb->len);
#endif

#if !defined(BCM7118)
#if A0PATCH
	// A0 patch: Apply patch here for PCI-DMA engine does not handle DMA size of
	// [(56N + 4) + 1,2,3 ]
	if( skb->len & 0x3 )
	{
		unsigned int patchbyte = skb->len % 56;
		// Now look for length 5,6,or 7
		if( (patchbyte > 4) && (patchbyte < 8) )
			pDevCtrl->txNextBdPtr->length = htons(skb->len + (8 - patchbyte));
	}
	else if (skb->len == 56)
	{
		/* Bug 58314 and Broadcom case 146100 */
		/* Packets length 56 are not transfered reliably -
		so increase size to something which does work*/
		/* Tests with ping show that only packets length 56 are broken */
		pDevCtrl->txNextBdPtr->length = htons(skb->len + 1);
	}
#endif
#endif
    /*
     * Set status of DMA BD to be transmitted and
     * advance BD pointer to next in the chain.
     */
    if (pDevCtrl->txNextBdPtr == pDevCtrl->txLastBdPtr) {
        pDevCtrl->txNextBdPtr->status = htons(DMA_WRAP);
        pDevCtrl->txNextBdPtr = pDevCtrl->txFirstBdPtr;
    }
    else {
        pDevCtrl->txNextBdPtr->status = 0;
        pDevCtrl->txNextBdPtr++;
    }

    /*
     * Turn on the "OWN" bit in the first buffer descriptor
     * This tells the switch that it can transmit this frame.
    */
#if (PASS_THROUGH)
    /* If we have no virt devs, then no tag to add, so set EOP */
    if ((!pDevCtrl->has_virt_dev) || (tagging_disable)) {
        firstBdPtr->status |= htons(DMA_OWN | DMA_SOP | DMA_EOP );
    }
	else {
#ifdef PACKET_COPY
        firstBdPtr->status |= htons(DMA_OWN | DMA_SOP | DMA_EOP);
#else
        firstBdPtr->status |= htons(DMA_OWN | DMA_SOP);
#endif
	}
#else
     firstBdPtr->status |= htons(DMA_OWN | DMA_SOP | DMA_EOP);
#endif
    /*
	 * Enable DMA for this channel, set PCI_DMA_LE since data has
	 * byte-swapped by the 7038 PCI bridge. Effectively data appears
	 * on PCI as BE.
	 */
#if defined(BCM7118)
    pDevCtrl->txDma->Control |= DMA_ENABLE;
#else
    pDevCtrl->txDma->Control |= (PCI_DMA_LE | PCI_DMA_ENABLE);
#endif

    /* Decrement total BD count */
    pDevCtrl->txFreeBds -= txCBPtr->nrBds;

    if ( (pDevCtrl->txFreeBds == 0) || (pDevCtrl->txCbQHead == NULL) ) {
        TRACE(("%s: bcmpcieth_net_xmit no transmit queue space -- stopping queues\n", dev->name));
        netif_stop_queue(dev);
    }
    /*
     * Packet was enqueued correctly.
     * Advance to the next Enet_Tx_CB needing to be assigned to a BD
     */
    txCBPtr->next = NULL;
    if(pDevCtrl->txCbPtrHead == NULL) {
        pDevCtrl->txCbPtrHead = txCBPtr;
        pDevCtrl->txCbPtrTail = txCBPtr;
    }
    else {
        pDevCtrl->txCbPtrTail->next = txCBPtr;
        pDevCtrl->txCbPtrTail = txCBPtr;
    }

    /* update stats */
    pDevCtrl->stats.tx_bytes += skb->len;
    pDevCtrl->stats.tx_packets++;

#if (PASS_THROUGH)
    /* transmit tag */
#ifdef PACKET_COPY
       if(0){		  
#else
    if (!tagging_disable && pDevCtrl->has_virt_dev) {
#endif
        struct sk_buff *tag_skb;
        struct _stb_eth_tag *stb_tag;
        unsigned int tx_len = ((int)((skb->len < ETH_ZLEN) ?  ETH_ZLEN : skb->len));
        //unsigned int tx_len = (int)skb->len;

        tag_skb = netdev_alloc_skb(pDevCtrl->dev, pDevCtrl->rxBufLen + SKB_ALIGNMENT);
        if (!tag_skb) {
            printk(KERN_ERR " failed to allocate skb for rx\n");
        }
        handleAlignment(pDevCtrl, tag_skb);
	 
        while (pDevCtrl->txCbPtrHead)  {
            if (ntohs(pDevCtrl->txCbPtrHead->lastBdAddr->status) & DMA_OWN) {
            break;
        }

        pDevCtrl->txFreeBds += pDevCtrl->txCbPtrHead->nrBds;

		if(pDevCtrl->txCbPtrHead->skb)
		{
			dev_kfree_skb (pDevCtrl->txCbPtrHead->skb);
		}

        txedCBPtr = pDevCtrl->txCbPtrHead;

        /* Advance the current reclaim pointer */
        pDevCtrl->txCbPtrHead = pDevCtrl->txCbPtrHead->next;

        /*
         * Finally, return the transmit header to the free list.
         */
            txCb_enq(pDevCtrl, txedCBPtr);
        }

        /* Obtain transmit control block */
        // txCBPtr = bcmgenet_get_txcb(dev, &write_ptr, index);
        txCBPtr = txCb_deq(pDevCtrl);

        if (txCBPtr == NULL) {
           netif_stop_queue(dev);
           return 1;
        }

        txCBPtr->nrBds = 1;
        txCBPtr->skb = tag_skb;

        if (pDevCtrl->txFreeBds < txCBPtr->nrBds) {
            TRACE(("%s: bcmpcieth_net_xmit low on txFreeBds\n", dev->name));
            txCb_enq(pDevCtrl, txCBPtr);
            netif_stop_queue(dev);
            return 1;
        }

        firstBdPtr = pDevCtrl->txNextBdPtr;
        /* store off the last BD address used to enqueue the packet */
        txCBPtr->lastBdAddr = pDevCtrl->txNextBdPtr;

        // memcpy(tag_skb->data, &pVirtDevCtrl->virt_dev_tag, sizeof(unsigned int));
        stb_tag = (struct _stb_eth_tag *)(tag_skb->data);
        stb_tag->len_h = (unsigned char)(tx_len >> 8);
        stb_tag->len_l = (unsigned char)(tx_len);
        stb_tag->magic = (unsigned char)(TAG_MAGIC);
        stb_tag->if_num = (unsigned char)pVirtDevCtrl->if_num;

        tag_skb->len = sizeof(struct _stb_eth_tag);

        txCBPtr->dma_addr = pci_map_single(pdev, (void *)tag_skb->data, tag_skb->len, PCI_DMA_TODEVICE);
        if (!txCBPtr->dma_addr) {
        TRACE(("%s: Tx DMA map failed\n", dev->name));

       //     spin_unlock_irqrestore(&pDevCtrl->lock, flags);
            return 0;
        }

        pDevCtrl->txNextBdPtr->address = htonl((uint32)txCBPtr->dma_addr | NEW_PCI_7038_PHYS_MEM_WIN0_BASE);
        pDevCtrl->txNextBdPtr->length = htons(tag_skb->len);

        /* Set status of DMA BD to be transmitted and
        * advance BD pointer to next in the chain.
        */
        if (pDevCtrl->txNextBdPtr == pDevCtrl->txLastBdPtr) {
            pDevCtrl->txNextBdPtr->status = htons(DMA_WRAP);
            pDevCtrl->txNextBdPtr = pDevCtrl->txFirstBdPtr;
        }
        else {
            pDevCtrl->txNextBdPtr->status = 0;
            pDevCtrl->txNextBdPtr++;
        }

        firstBdPtr->status |= htons(DMA_OWN | DMA_EOP );

#if defined(BCM7118)
        pDevCtrl->txDma->Control |= DMA_ENABLE;
#else
        pDevCtrl->txDma->Control |= (PCI_DMA_LE | PCI_DMA_ENABLE);
#endif

        /* Decrement total BD count */
        pDevCtrl->txFreeBds -= 1;

        if ( (pDevCtrl->txFreeBds == 0) || (pDevCtrl->txCbQHead == NULL) ) {
         //   TRACE(("%s: bcmpcieth_net_xmit no transmit queue space -- stopping queues\n", dev->name));
            netif_stop_queue(dev);
        }
         /* Packet was enqueued correctly.
        * Advance to the next Enet_Tx_CB needing to be assigned to a BD
        */
        txCBPtr->next = NULL;
        if(pDevCtrl->txCbPtrHead == NULL) {
            pDevCtrl->txCbPtrHead = txCBPtr;
            pDevCtrl->txCbPtrTail = txCBPtr;
        }
        else {
            pDevCtrl->txCbPtrTail->next = txCBPtr;
            pDevCtrl->txCbPtrTail = txCBPtr;
        }
    }
#endif
    dev->trans_start = jiffies;

    return 0;
}
/*
 * bcmpcieth_net_isr: Acknowledge interrupts and check if any packets have
 *                  arrived
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static irqreturn_t bcmpcieth_net_isr(int irq, void * dev_id)
#else
static irqreturn_t bcmpcieth_net_isr(int irq, void * dev_id, struct pt_regs * regs)
#endif
{
#if (PASS_THROUGH)
	BcmEnet_devctrl *pDevCtrl =	pDevContext;
#else
    BcmEnet_devctrl *pDevCtrl = dev_id;
#endif
    uint32 rxEvents, mbEvents;
	irqreturn_t ret = IRQ_NONE;
#if defined(BCM7118)
#if defined(BCM7118A0)
	uint32 bnm_rx_intstat;
	uint32 bnm_tx_intstat;
#endif
#endif

    /* get Rx interrupt events */
    rxEvents = pDevCtrl->rxDma->IntStat;
#if defined(BCM7118)
	mbEvents = pDevCtrl->nsGisbRegs->hostIrqStatus;
#else
	mbEvents = pDevCtrl->mpiRegs->PciIntStat;
#endif

    TRACE(("bcmpcieth_net_isr: intstat=%08lx\n",rxEvents));
    if (rxEvents & DMA_BUFF_DONE)
    {
        pDevCtrl->rxDma->IntStat = DMA_BUFF_DONE;  /* clr interrupt */
    }

    if (rxEvents & DMA_DONE)
    {
        pDevCtrl->rxDma->IntStat = DMA_DONE;  // clr interrupt
        // no BH and handle every thing inside interrupt routine
        bcmpcieth_rx(pDevCtrl);

		ret = IRQ_HANDLED;
    }
    else {
        if (rxEvents & DMA_NO_DESC) {
			pDevCtrl->gNumNoDescErrors++;
        	pDevCtrl->rxDma->IntStat = DMA_NO_DESC; // Clear the interrupt
			{
				int bdfilled;

				bdfilled = assign_rx_buffers(pDevCtrl);
				if (bdfilled == 0) {
					/* For now, until we figure out why we can't reclaim the buffers */
					/*printk(KERN_CRIT "Running out of buffers, all busy\n");*/
					pDevCtrl->refill_timer.expires = jiffies + TIMEOUT_10MS;
					add_timer(&pDevCtrl->refill_timer);
				}
		        else {
			        /* Re-enable DMA if there are buffers available */
				    pDevCtrl->rxDma->Control |= DMA_ENABLE;
		        }
			}
			ret = IRQ_HANDLED;
        }
    }
#if defined(BCM7118)
	if(rxEvents != 0)
	{
		// clear the rx interrupt
		pDevCtrl->subRegs->subR5fIrqMaskStatus |= 0x2;
	}
#if defined(BCM7118A0)
	if( (pDevCtrl->subRegs->subR5fIrqMaskStatus & 0x8) == 0x8)
	{
		// get BNM RX int status
		bnm_rx_intstat = *(uint32 *)MEM2MEM_DMA_RX0_INTERRUPT_STATUS;
		// send a mailbox interrupt
		if((bnm_rx_intstat & 0x2) == 0x2)
		{
			pDevCtrl->nsGisbRegs->mBox1 = mcode++;
		}

		// clear the BNM RX INT
		*(uint32 *)MEM2MEM_DMA_RX0_INTERRUPT_STATUS = bnm_rx_intstat;
		pDevCtrl->subRegs->subR5fIrqMaskStatus |= 0x8;
	}

	if( (pDevCtrl->subRegs->subR5fIrqMaskStatus & 0x1) == 0x1)
	{
		// get BNM TX int status
		bnm_tx_intstat = *(uint32 *)MEM2MEM_DMA_TX1_INTERRUPT_STATUS;

		// send a mailbox interrupt
		if((bnm_tx_intstat & 0x2) == 0x2)
		{
			pDevCtrl->nsGisbRegs->mBox0 = mcode++;
		}

		// clear the BNM TX INT
		*(uint32 *)MEM2MEM_DMA_TX1_INTERRUPT_STATUS = bnm_tx_intstat;
		pDevCtrl->subRegs->subR5fIrqMaskStatus |= 0x1;
	}
#endif
#endif

	// Only interested in the mbox interrupts that are enabled
#if defined(BCM7118)
	if ( pDevCtrl->nsGisbRegs->hostIrqMask & (mbEvents & GISB_MBOX1_IRQ_MASK) )
#else
	if ( (mbEvents>>16) & mbEvents )
#endif
	{
#if defined(BCM7118)
		// Disable interrupt from this source, IST will enable it
		pDevCtrl->nsGisbRegs->hostIrqMask &= (~GISB_MBOX1_IRQ_MASK);
	  	pDevCtrl->nsGisbRegs->mboxInt |= 0x2;
#else
		//write 1 to clear pending interrupts
		pDevCtrl->mpiRegs->PciIntStat = mbEvents;

		// Disable interrupt from this source, IST will enable it
		pDevCtrl->mpiRegs->PciIntStat = 0;
#endif
		MboxIntStatus = mbEvents;
   		tasklet_schedule( &PCIMBOX_TASK );
		ret = IRQ_HANDLED;
	}

	return ret;
}

/*
 *  bcmpcieth_rx: Process all received packets.
 */

#define MAX_RX                  0x0fffffff //to disable it

static void bcmpcieth_rx(void *ptr)
{
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)ptr;
    struct sk_buff *skb;
    struct net_device *dev;
    unsigned long dmaFlag;
    int len;
	volatile DmaDesc *desc;
    Enet_Tx_CB *cb;
#if (PASS_THROUGH)
    unsigned char upDown;
#endif
#if defined(CONFIG_MIPS_BCM7420AX) || defined(CONFIG_BCM7420B0) || defined(CONFIG_BCM7420C0)
	struct pci_dev *pdev = pDevCtrl->pdev;
#endif

	GPIO_SET_LO(GPIO46);

    dmaFlag = ntohs(pDevCtrl->rxBdReadPtr->status & 0xffff);

    loopCount = 0;

    /* Loop here for each received packet */
    while(!(dmaFlag & DMA_OWN) && (dmaFlag & DMA_EOP))
    {
        if (++loopCount > MAX_RX) {
	   	    break;
	    }
		desc = pDevCtrl->rxBdReadPtr;

	    // Stop when we hit a buffer with no data, or a BD w/ no buffer.
        // This implies that we caught up with DMA, or that we haven't been
        // able to fill the buffers.
        if ( (desc->address == (uint32) NULL))
        {
            break;
        }
		cb = &pDevCtrl->rxCbs[desc - pDevCtrl->rxBds];

		/* get the buffer address */

#if defined(CONFIG_MIPS_BCM7420AX) || defined(CONFIG_BCM7420B0) || defined(CONFIG_BCM7420C0)
	    /* TRACE(("bcmpcieth_rx: pBuf=%08lx\n", (uint32)pBuf)); */
        pci_unmap_single(pdev, (dma_addr_t)((ntohl(desc->address)) &
					   ~NEW_PCI_7038_PHYS_MEM_WIN0_BASE),
                       (size_t)cb->dma_len,
			           PCI_DMA_FROMDEVICE);
#else
        /* TRACE(("bcmpcieth_rx: pBuf=%08lx\n", (uint32)pBuf)); */

        /*
         * THT: Invalidate the RAC cache again, since someone may have read near the vicinity
         * of the buffer.  This is necessary because the RAC cache is much larger than the CPU cache
         */
	    bcm_inv_rac_all();
#endif
		/* get the buffer length */
        len = ntohs(desc->length);
        /* Null the BD field to prevent reuse */
        desc->status = 0; //clear status.
        desc->address = 0;

        IncRxBDptr(pDevCtrl->rxBdReadPtr, pDevCtrl);
		// Recover the SKB pointer saved during assignment.
        skb = cb->skb;

		/* refresh the dmaFlag for the loop */
        dmaFlag = ntohs(pDevCtrl->rxBdReadPtr->status);

        /* adjust the skb, there is no CRC */
        skb_trim(skb, len);

#if (PASS_THROUGH)

        if (pDevCtrl->has_virt_dev && !tagging_disable) 
        {
			unsigned int dev_num;

			dev = bcmgenet_match_tag(pDevCtrl, skb, len, &dev_num);

            if (!dev) 
            {
                dev_kfree_skb_any(cb->skb);
                cb->skb = NULL;
#endif
                dev = pDevCtrl->dev;
#if (PASS_THROUGH)
                continue;
            } 
            else 
            {

				if (dev_num == NUM_TAGS) 
				{
					upDown = *(skb->data);
					if (upDown) 
					{
						if (!netif_carrier_ok(dev) && netif_carrier_ok(pDevCtrl->dev)) 
						{
							dev->flags |= IFF_RUNNING;
							netif_carrier_on(dev);
						}
					} 
					else 
					{
						if (netif_carrier_ok(dev) && netif_carrier_ok(pDevCtrl->dev)) 
						{
							dev->flags &= ~IFF_RUNNING;
							netif_carrier_off(dev);
						}
					}
					dev_kfree_skb_any(skb);
					cb->skb = NULL;
					dev = pDevCtrl->dev;
					continue;
				}
#endif

#ifdef DUMP_DATA
        printk("bcmpcieth_rx : len = %d", len);
        dumpHexData(skb, len);
#endif
#if (PASS_THROUGH)
            skb->dev = dev;
            skb->protocol = eth_type_trans(skb, dev);
            }
        }
        else
        {
            dev = pDevCtrl->dev;
#endif
            /* Finish setting up the received SKB and send it to the kernel */
            skb->dev = dev;
            /* Set the protocol type */
            skb->protocol = eth_type_trans(skb, dev);
#if (PASS_THROUGH)
		}
#endif
		/* 3255 has already performed checksum */
		if(csumoe == 1)
		{
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		}

		/* Allocate a new SKB for the ring */
        if (atomic_read(&pDevCtrl->rxDmaRefill) == 0)
		{
            assign_rx_buffers(pDevCtrl);
		    pDevCtrl->rxDma->Control |= DMA_ENABLE;
		}

        pDevCtrl->stats.rx_packets++;
        pDevCtrl->stats.rx_bytes += len;

		GPIO_SET_LO(GPIO38);
		/* Notify kernel */
#ifdef CES2006_DEMO
		if ((pBuf[36] == 0x13) && (pBuf[37] == 0x8a))
			atomic_dec(&skb->users);
		else
#endif
		netif_rx(skb);
		GPIO_SET_HI(GPIO38);

        dev->last_rx = jiffies;

        continue;
    }

    loopCount = -1; // tell dump_emac that we are outside RX
	GPIO_SET_HI(GPIO46);
}


/*
 * Set the hardware MAC address.
 */
static int bcm_set_mac_addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr=p;

    if(netif_running(dev))
        return -EBUSY;

    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

    return 0;
}

/*******************************************************************************
*
* skb_headerinit
*
* Reinitializes the socket buffer.  Lifted from skbuff.c
*
* RETURNS: None.
*/

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static inline void skb_headerinit(void *p, struct kmem_cache *cache, unsigned long flags)
#else
static inline void skb_headerinit(void *p, kmem_cache_t *cache, unsigned long flags)
#endif
{
    struct sk_buff *skb = p;

    skb->next = NULL;
    skb->prev = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)
	skb->list = NULL;
	skb->stamp.tv_sec=0;    /* No idea about time */
	skb->security = 0;  /* By default packets are insecure */
#endif
    skb->sk = NULL;
    skb->dev = NULL;
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
    skb->dst = NULL;
#endif
    /* memset(skb->cb, 0, sizeof(skb->cb)); */
    skb->pkt_type = PACKET_HOST;    /* Default type */
    skb->ip_summed = 0;
    skb->priority = 0;
    skb->destructor = NULL;

#ifdef CONFIG_NETFILTER
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14)

    skb->nfmark = 0;
#else
    skb->nfmark = skb->nfcache = 0;
#endif
#endif
    skb->nfct = NULL;
#ifdef CONFIG_NETFILTER_DEBUG
    skb->nf_debug = 0;
#endif
#endif
#ifdef CONFIG_NET_SCHED
    skb->tc_index = 0;
#endif
}

/*
 * assign_rx_buffers: Beginning with the first receive buffer descriptor
 *                  used to receive a packet since this function last
 *                  assigned a buffer, reassign buffers to receive
 *                  buffer descriptors, and mark them as EMPTY; available
 *                  to be used again.
 *
 */

int gSkbTrueSize;

static int assign_rx_buffers(BcmEnet_devctrl *pDevCtrl)
{
	struct pci_dev *pdev = pDevCtrl->pdev;
    struct sk_buff *skb;
    uint16  bdsfilled=0;
    int devInUsed;
    int i;
    volatile unsigned long boundary256, curData, resHeader;

    /*
     * check assign_rx_buffers is in process?
     * This function may be called from various context, like timer
     * or bcm6352_rx
     */
    if(test_and_set_bit(0, &pDevCtrl->rxbuf_assign_busy)) {
        return bdsfilled;
    }

    /* loop here for each buffer needing assign */
    for (;;)
    {
		volatile DmaDesc *p = pDevCtrl->rxBdAssignPtr;
		Enet_Tx_CB *cb = &pDevCtrl->rxCbs[p - pDevCtrl->rxBds];

		cb->dma_len = pDevCtrl->rxBufLen;
        /* exit if no receive buffer descriptors are in "unused" state */
        if (pDevCtrl->rxBdAssignPtr->address != 0)
        {
            break;
        }

        skb = pDevCtrl->skb_pool[pDevCtrl->nextskb++];

        if (pDevCtrl->nextskb == MAX_RX_BUFS)
            pDevCtrl->nextskb = 0;

        /* check if skb is free */
        if (skb_shared(skb) ||
            atomic_read(skb_dataref(skb)) > 1) {
            /* find next free skb */
            for (i = 0; i < MAX_RX_BUFS; i++) {
                skb = pDevCtrl->skb_pool[pDevCtrl->nextskb++];
                if (pDevCtrl->nextskb == MAX_RX_BUFS)
                    pDevCtrl->nextskb = 0;
                if ((skb_shared(skb) == 0) &&
                    atomic_read(skb_dataref(skb)) <= 1) {
                    /* found a free skb */
                    break;
                }
            }
            if (i == MAX_RX_BUFS) {
                /* no free skb available now */
                /* rxDma is empty, set flag and let timer function to refill later */
                atomic_set(&pDevCtrl->rxDmaRefill, 1);
                break;
            }
        }

        atomic_set(&pDevCtrl->rxDmaRefill, 0);
        skb_headerinit(skb, NULL, 0);  /* clean state */


        if (gSkbTrueSize != skb->truesize) {
            printk("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
            printk("$   gTrueSize=%d, skb->truesize=%d\n", gSkbTrueSize, skb->truesize);
            printk("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
        }

        /* init the skb, in case other app. modified the skb pointer */
        skb->data = skb->tail = skb->head;
        skb->end = skb->data + (skb->truesize - sizeof(struct sk_buff));
        skb->len = 0;
        skb->data_len = 0;
        skb->cloned = 0;

	    // We will reserve 20 bytes, and wants that skb->data to be on
	    // a 256B boundary.
	    curData = (unsigned long) skb->data;
	    boundary256 = (curData+16+BCM_SKB_PAD+BCM_SKB_DMA_ALIGN-1) & 0xffffff00;
	    resHeader = boundary256 - curData;

	    // If we don't have room for 20B, go to next boundary.  Sanity check
	    if (resHeader < (16+BCM_SKB_PAD)) {
		    boundary256 += BCM_SKB_DMA_ALIGN;
	    }
	    resHeader = boundary256 - curData - 4;
	    // This advances skb->data 4B shy of the boundary,
	    // and also includes the 16B headroom commented out below,
	    // by virtue of our sanity check above.
	    skb_reserve(skb, resHeader);

        /* reserve space ditto __dev_alloc_skb */
        *(unsigned int *)skb->data = (unsigned int)skb;
        skb_reserve(skb, 4);

	    // Make sure it is on 256B boundary, should never happen if our
	    // calculation was correct.
	    if ((unsigned long) skb->data & 0xff) {
            printk("$$$$$$$$$$$$$$$$$$$ skb buffer is NOT aligned on 256B boundary\n");
	    }

        skb_put(skb, pDevCtrl->rxBufLen);

        /*
         * Set the user count to two so when the upper layer frees the
         * socket buffer, only the user count is decremented.
         */
        atomic_inc(&skb->users);

        /* kept count of any BD's we refill */
        bdsfilled++;

		/* assign packet, prepare descriptor, and advance pointer */
		cb->skb = skb;
        cb->dma_addr = pci_map_single(pdev, (void *)skb->data, (size_t)cb->dma_len, PCI_DMA_FROMDEVICE);
        pDevCtrl->rxBdAssignPtr->address = htonl((uint32)cb->dma_addr | NEW_PCI_7038_PHYS_MEM_WIN0_BASE);
        pDevCtrl->rxBdAssignPtr->length  = htons(pDevCtrl->rxBufLen);
        pDevCtrl->rxBdAssignPtr->status = 0;

        /* turn on the newly assigned BD for DMA to use */
        if (pDevCtrl->rxBdAssignPtr == pDevCtrl->rxLastBdPtr) {
            pDevCtrl->rxBdAssignPtr->status |= htons(DMA_OWN | DMA_WRAP);
            pDevCtrl->rxBdAssignPtr = pDevCtrl->rxFirstBdPtr;
        }
        else {
            pDevCtrl->rxBdAssignPtr->status |= htons(DMA_OWN);
            pDevCtrl->rxBdAssignPtr++;
        }
    }

    /*
     * restart DMA in case the DMA ran out of descriptors
     */
    devInUsed = atomic_read(&pDevCtrl->devInUsed);

    clear_bit(0, &pDevCtrl->rxbuf_assign_busy);

    return bdsfilled;
}

/*
 *  init_buffers: initialize driver's pools of receive buffers
 *  and tranmit headers
 */
static int init_buffers(BcmEnet_devctrl *pDevCtrl)
{
    struct sk_buff *skb;
    int bdfilled;
    int i;

    TRACE(("bcmpcieth: init_buffers\n"));

    /* set initial state of all BD pointers to top of BD ring */
    pDevCtrl->txCbPtrHead = pDevCtrl->txCbPtrTail = NULL;

    /* allocate recieve buffer pool */
    for (i = 0; i < MAX_RX_BUFS; i++) {
        /* allocate a new SKB for the ring */
        /* BCM_SKB_PAD bytes for skb pointer, BCM_SKB_DMA_ALIGN bytes for alignment */
        skb = dev_alloc_skb(pDevCtrl->rxBufLen + BCM_SKB_PAD + BCM_SKB_DMA_ALIGN);

        if (i == 0)
            gSkbTrueSize = skb->truesize;

        if (skb == NULL)
        {
            printk(KERN_NOTICE CARDNAME ": Low memory.\n");
            break;
        }
        /* save skb pointer */
        pDevCtrl->skb_pool[i] = skb;
    }

    if (i < MAX_RX_BUFS) {
        /* release allocated receive buffer memory */
        for (i = 0; i < MAX_RX_BUFS; i++) {
            if (pDevCtrl->skb_pool[i] != NULL) {
                dev_kfree_skb (pDevCtrl->skb_pool[i]);
                pDevCtrl->skb_pool[i] = NULL;
            }
        }
        return -ENOMEM;
    }
    /* init the next free skb index */
    pDevCtrl->nextskb = 0;
    atomic_set(&pDevCtrl->rxDmaRefill, 0);
    clear_bit(0, &pDevCtrl->rxbuf_assign_busy);

    /* assign packet buffers to all available Dma descriptors */
    bdfilled = assign_rx_buffers(pDevCtrl);
    if (bdfilled > 0) {
        TRACE(("init_buffers: %d descriptors initialized\n", bdfilled));
    }

	return 0;
}

/*
 * bcmpcieth_init_dev: initialize PCI DMA,
 * allocate Tx/Rx buffer descriptors pool, Tx control block pool.
 */
static int bcmpcieth_init_dev(BcmEnet_devctrl *pDevCtrl)
{
    int i;
    int nrCbs;
    void *p;
    Enet_Tx_CB *txCbPtr;
	volatile DmaDesc *gPciHostRxBdBase;
	volatile DmaDesc *gPciHostTxBdBase;
	volatile DmaDesc *pRxBdMem;
	volatile DmaDesc *pTxBdMem;
#if !defined(BCM7118)
	uint32	temp;
#endif
	/* reset the dma ready flag */
    pDevCtrl->PciHostDmaReady = FALSE;
    pDevCtrl->PciResetDetected = FALSE;

    /* start off with empty mailbox */
    pDevCtrl->mailbox1 = 0;

	/* set some constants */
	pDevCtrl->nrTxBds = NR_PCIHOST_TX_BDS;
    pDevCtrl->nrRxBds = NR_PCIHOST_RX_BDS;
    pDevCtrl->rxBufLen = ENET_MAX_MTU_SIZE;

#if !defined(BCM7118)
	/* configure byte translation on local PCI window */
	temp = REG32(PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_PCI_CFG_PCI_SDRAM_ENDIAN_CTRL));
	temp &= 0xfffffffc;
	temp |= 0x00000002;
	wREG32(PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_PCI_CFG_PCI_SDRAM_ENDIAN_CTRL), temp);

#if defined(BLOCKOUT_COUNTER)
	/* Ethernet is client 45 (32'h0010_60b8), PCI is client 42(32'h0010_60ac) */
	/* default setting for client 45 is 32'h0003e572 */
	/* default setting for client 42 is 32'h0003b471 */
	printk("MEMC_0_CLIENT_INFO_42 = %08x\n", *(unsigned int *)0xA01060AC);
	printk("MEMC_0_CLIENT_INFO_45 = %08x\n", *(unsigned int *)0xA01060B8);
	/* bits 8-20 are block out value */
	/* 1111,1111,1110,0000,0000,0000,1111,1111 = 0xFFE000FF */
	*(unsigned int *)0xA01060B8 = ( (*(unsigned int *)0xA01060B8) & 0xFFE000FF);
	printk("MEMC_0_CLIENT_INFO_42 = %08x\n", *(unsigned int *)0xA01060AC);
	printk("MEMC_0_CLIENT_INFO_45 = %08x\n", *(unsigned int *)0xA01060B8);

#endif
	/* set up the memory window */
	wREG32(PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_PCI_CFG_MEMORY_BASE_W0), NEW_PCI_7038_PHYS_MEM_WIN0_BASE);

#endif

    /* init rx/tx dma channels */
#if defined(BCM7118)
	pDevCtrl->txDma = &pDevCtrl->m2mRegs->IuDmaConfigRegisters[SUB_DMA_SCB_SCB2UBUS];
	pDevCtrl->txDmaState = &pDevCtrl->m2mRegs->IuDmaStateRegisters[SUB_DMA_SCB_SCB2UBUS];

	//Clear IUDMA state ram locations.
    pDevCtrl->txDmaState->DescriptorBase = 0;
    pDevCtrl->txDmaState->StateRam2 = 0;
    pDevCtrl->txDmaState->StateRam3 = 0;
    pDevCtrl->txDmaState->StateRam4 = 0;

    pDevCtrl->rxDma = &pDevCtrl->m2mRegs->IuDmaConfigRegisters[SUB_DMA_SCB_UBUS2SCB];
	pDevCtrl->rxDmaState = &pDevCtrl->m2mRegs->IuDmaStateRegisters[SUB_DMA_SCB_UBUS2SCB];

	//Clear IUDMA state ram locations.
    pDevCtrl->rxDmaState->DescriptorBase = 0;
    pDevCtrl->rxDmaState->StateRam2 = 0;
    pDevCtrl->rxDmaState->StateRam3 = 0;
    pDevCtrl->rxDmaState->StateRam4 = 0;

	TRACE(("bcmpcieth_init_dev: SUB_R5F_IRQ_MASK_STATUS=%08lx\n",
    	*((uint32 *)SUB_R5F_IRQ_MASK_STATUS)));

#if defined(BCM7118A0)
	// enable interrupts for both RX0 (BNM), TX1(BNM) and RX1 (HOST)
	*(uint32*)SUB_R5F_IRQ_MASK_STATUS |= 0x00000b00;
#else
	// enable interrupt for RX1 (HOST)
	*(uint32*)SUB_R5F_IRQ_MASK_STATUS |= 0x00000200;
	// enable interrupts for both RX0 (BNM), TX1(BNM)
	*(uint32*)SUB_IRQ_MASK_STATUS |= 0x00000900;
#endif

	TRACE(("bcmpcieth_init_dev: SUB_R5F_IRQ_MASK_STATUS=%08lx\n",
    	*((uint32 *)SUB_R5F_IRQ_MASK_STATUS)));
	TRACE(("bcmpcieth_init_dev: SUB_IRQ_MASK_STATUS=%08lx\n",
    	*((uint32 *)SUB_IRQ_MASK_STATUS)));
	TRACE(("bcmpcieth_init_dev: SUB_IRQ_MASK_STATUS=%08lx\n",
    	*((uint32 *)SUB_IRQ_MASK_STATUS)));
	TRACE(("bcmpcieth_init_dev: HIF_CPU_INTR1_INTR_W1_MASK_STATUS=%08lx\n",
    	*((uint32 *)HIF_CPU_INTR1_INTR_W1_MASK_STATUS)));

	// enable HIF interrupts
	*(uint32*)HIF_CPU_INTR1_INTR_W1_MASK_CLEAR |= 0x02000000;

	TRACE(("bcmpcieth_init_dev: HIF_CPU_INTR1_INTR_W1_MASK_STATUS=%08lx\n",
    	*((uint32 *)HIF_CPU_INTR1_INTR_W1_MASK_STATUS)));
	TRACE(("bcmpcieth_init_dev: NSGISB_HOSTIRQMASK=%08lx\n",
    	*((uint32 *)NSGISB_HOSTIRQMASK)));

	// set MEM2MEM_CTRL
	*(uint32 *)0xfffb0c00 = 0x1;
	*(uint32 *)0xfffb0d00 = 0x1;

	// set MEM2MEM_DMA_CTRL_CTRL_CONFIG
	// don't need to do this since it defaults to 0x1.
//	*((uint32 *)0xfffb0800) |= 0x1;

	TRACE(("bcmpcieth_init_dev: MEM2MEM_DMA_CTRL_CTRL_CONFIG=%08lx\n",
    	*((uint32 *)0xfffb0800)));
	TRACE(("bcmpcieth_init_dev: MEM2MEM_CTRL_CONFIG0=%08lx\n",
    	*((uint32 *)0xfffb0c00)));
	TRACE(("bcmpcieth_init_dev: MEM2MEM_CTRL_CONFIG1=%08lx\n",
    	*((uint32 *)0xfffb0d00)));

	TRACE(("bcmpcieth_init_dev: m2mRegs=%08lx, IuDmaConfigRegisters=%08lx\n",
    	(uint32)pDevCtrl->m2mRegs, (uint32)pDevCtrl->m2mRegs->IuDmaConfigRegisters));
	TRACE(("bcmpcieth_init_dev: subRegs=%08lx, subR5fIrqMaskStatus=%08lx\n",
    	(uint32)pDevCtrl->subRegs, (uint32)&pDevCtrl->subRegs->subR5fIrqMaskStatus));
	TRACE(("bcmpcieth_init_dev: nsGisbRegs=%08lx\n", (uint32)pDevCtrl->nsGisbRegs));
	TRACE(("bcmpcieth_init_dev: rxDmaState=%08lx, txDmaState=%08lx\n",
    	(uint32)pDevCtrl->rxDmaState, (uint32)pDevCtrl->txDmaState));

#else
	pDevCtrl->txDma = &pDevCtrl->mpiRegs->PciDmaConfig[MPI_DMA_PCI_PCI2UBUS];
    pDevCtrl->rxDma = &pDevCtrl->mpiRegs->PciDmaConfig[MPI_DMA_PCI_UBUS2PCI];
#endif

	TRACE(("bcmpcieth_init_dev: rxDMA=%08lx, txDma=%08lx\n",
    	(uint32)pDevCtrl->rxDma, (uint32)pDevCtrl->txDma));

	/* allocate memory for RX buffer descriptors */
 	pRxBdMem = (volatile DmaDesc *) pci_alloc_consistent(pDevCtrl->pdev, (NR_PCIHOST_RX_BDS*sizeof(DmaDesc))+DCACHE_LINE_SIZE,
						       (dma_addr_t*) &pDevCtrl->rxdpa);

    if(!pRxBdMem)
	{
		printk(KERN_ERR CARDNAME ": Can not allocate memory for %d RX BD's!\n", NR_PCIHOST_RX_BDS);
        return -ENOMEM;
	}
	gPciHostRxBdBase = (volatile DmaDesc *)BCM_ALIGN((uint32)pRxBdMem, DCACHE_LINE_SIZE);
    pDevCtrl->rxdpa = BCM_ALIGN((uint32)pDevCtrl->rxdpa, DCACHE_LINE_SIZE);
	pDevCtrl->PciRxSetupPtr = (uint32) pDevCtrl->rxdpa | NEW_PCI_7038_PHYS_MEM_WIN0_BASE;

#if defined(BCM7118)
	pDevCtrl->rxDmaState->DescriptorBase = pDevCtrl->PciRxSetupPtr;
#else
	pDevCtrl->rxDma->DescriptorAddress = pDevCtrl->PciRxSetupPtr;
#endif
	pDevCtrl->rxDma->IntMask = DS_PCI_DMA_IRQ_MASK;
#if defined(DRV_DEBUG)
#if defined(BCM7118)
	printk("PCIHost Rx DescriptorBase(%08lx)=%08lx\n", (uint32)&pDevCtrl->rxDmaState->DescriptorBase, pDevCtrl->rxDmaState->DescriptorBase );
#else
	printk("PCIHost Rx DescriptorAddress(%08lx)=%08lx\n", (uint32)&pDevCtrl->rxDma->DescriptorAddress, pDevCtrl->rxDma->DescriptorAddress );
#endif
	printk("PCIHost Rx Control(%08lx)=%08lx\n", (uint32)&pDevCtrl->rxDma->Control, pDevCtrl->rxDma->Control );
#endif

	/* allocate memory for TX buffer descriptors */
 	pTxBdMem = (volatile DmaDesc *) pci_alloc_consistent(pDevCtrl->pdev, (NR_PCIHOST_TX_BDS*sizeof(DmaDesc))+DCACHE_LINE_SIZE,
						       (dma_addr_t*) &pDevCtrl->txdpa);

    if(!pTxBdMem)
	{
		printk(KERN_ERR CARDNAME ": Can not allocate memory for TX BD's!\n");
		kfree((void *)pRxBdMem);
        return -ENOMEM;
	}


	gPciHostTxBdBase = (volatile DmaDesc *)BCM_ALIGN((uint32)pTxBdMem, DCACHE_LINE_SIZE);
    memset((void *)gPciHostTxBdBase, 0, sizeof(DmaDesc) * NR_PCIHOST_TX_BDS);
    pDevCtrl->txdpa = BCM_ALIGN((uint32)pDevCtrl->txdpa, DCACHE_LINE_SIZE);
	pDevCtrl->PciTxSetupPtr = (uint32) pDevCtrl->txdpa | NEW_PCI_7038_PHYS_MEM_WIN0_BASE;



#if defined(BCM7118)
	pDevCtrl->txDmaState->DescriptorBase = pDevCtrl->PciTxSetupPtr;
#else
	pDevCtrl->txDma->DescriptorAddress = pDevCtrl->PciTxSetupPtr;
#endif

#if defined(DRV_DEBUG)
#if defined(BCM7118)
	printk("PCIHost TX DescriptorBase(%08lx)=%08lx\n", (uint32)&pDevCtrl->txDma->DescriptorBase,
		   pDevCtrl->txDma->DescriptorBase );
#else
	printk("PCIHost TX DescriptorAddress(%08lx)=%08lx\n", (uint32)&pDevCtrl->txDma->DescriptorAddress,
		   pDevCtrl->txDma->DescriptorAddress );
#endif
	printk("PCIHost TX Control(%08lx)=%08lx\n", (uint32)&pDevCtrl->txDma->Control,
		   pDevCtrl->txDma->Control );
#endif

	/* set the BD base */
	pDevCtrl->txBds = (DmaDesc *) gPciHostTxBdBase;
	pDevCtrl->rxBds = (DmaDesc *) gPciHostRxBdBase;

    /* alloc space for the tx control block pool */
    nrCbs = pDevCtrl->nrTxBds;
    if (!(p = kmalloc(nrCbs*sizeof(Enet_Tx_CB), GFP_KERNEL))) {
		kfree((void *)pRxBdMem);
		kfree((void *)pTxBdMem);
        return -ENOMEM;
    }
    memset(p, 0, nrCbs*sizeof(Enet_Tx_CB));
    pDevCtrl->txCbs = (Enet_Tx_CB *)p;/* tx control block pool */

	if (!(p = kmalloc(pDevCtrl->nrRxBds * sizeof(Enet_Tx_CB), GFP_KERNEL))) {
		kfree(pDevCtrl->rxCbs);
		return -ENOMEM;
	}
	memset(p, 0, pDevCtrl->nrRxBds * sizeof(Enet_Tx_CB));
	pDevCtrl->rxCbs = (void *)p;

    /* initialize rx ring pointer variables. */
    pDevCtrl->rxBdAssignPtr = pDevCtrl->rxBdReadPtr =
                pDevCtrl->rxFirstBdPtr = pDevCtrl->rxBds;
    pDevCtrl->rxLastBdPtr = pDevCtrl->rxFirstBdPtr + pDevCtrl->nrRxBds - 1;

    /* init the receive buffer descriptor ring */
    for (i = 0; i < pDevCtrl->nrRxBds; i++)
    {
        (pDevCtrl->rxFirstBdPtr + i)->length = pDevCtrl->rxBufLen;
        (pDevCtrl->rxFirstBdPtr + i)->status = 0;
        (pDevCtrl->rxFirstBdPtr + i)->address = 0;

    }
    pDevCtrl->rxLastBdPtr->status |= htons(DMA_WRAP);

    /* init transmit buffer descriptor variables */
    pDevCtrl->txNextBdPtr = pDevCtrl->txFirstBdPtr = pDevCtrl->txBds;
    pDevCtrl->txLastBdPtr = pDevCtrl->txFirstBdPtr + pDevCtrl->nrTxBds - 1;

    /* clear the transmit buffer descriptors */
    for (i = 0; i < pDevCtrl->nrTxBds; i++)
    {
        (pDevCtrl->txFirstBdPtr + i)->length = 0;
        (pDevCtrl->txFirstBdPtr + i)->status = 0;
        (pDevCtrl->txFirstBdPtr + i)->address = 0;
    }
    pDevCtrl->txLastBdPtr->status |= htons(DMA_WRAP);
    pDevCtrl->txFreeBds = pDevCtrl->nrTxBds;

    /* initialize the receive buffers and transmit headers */
    if (init_buffers(pDevCtrl)) {
        kfree((void *)pDevCtrl->txCbs);
		kfree((void *)pDevCtrl->rxCbs);
		kfree((void *)pRxBdMem);
		kfree((void *)pTxBdMem);
        return -ENOMEM;
    }

    for (i = 0; i < nrCbs; i++)
    {
        txCbPtr = pDevCtrl->txCbs + i;
        txCb_enq(pDevCtrl, txCbPtr);
    }

	// Init remote flash structure
	localDocsisNonvol.pDynmNonvol = (uint32)kmalloc( DYN_NONVOLMAXSIZE, GFP_KERNEL );
	localDocsisNonvol.DynmLen = 0;
	localDocsisNonvol.pPermNonvol = (uint32)kmalloc( PERM_NONVOLMAXSIZE, GFP_KERNEL );
	localDocsisNonvol.PermLen = 0;

	/*bcmpcieth_dump(pDevCtrl);*/

	GPIO_DEBUG_INIT;

	/* if we reach this point, we've init'ed successfully */
    return 0;
}

#ifdef USE_PROC

#define PROC_DUMP_TXBD_1of6	0
#define PROC_DUMP_TXBD_2of6	1
#define PROC_DUMP_TXBD_3of6	2
#define PROC_DUMP_TXBD_4of6	3
#define PROC_DUMP_TXBD_5of6	4
#define PROC_DUMP_TXBD_6of6	5

#define PROC_DUMP_RXBD_1of6	6
#define PROC_DUMP_RXBD_2of6	7
#define PROC_DUMP_RXBD_3of6	8
#define PROC_DUMP_RXBD_4of6	9
#define PROC_DUMP_RXBD_5of6	10
#define PROC_DUMP_RXBD_6of6	11

#define PROC_DUMP_DMADESC_STATUS	12

#define PROC_DUMP_SKB_1of6		13
#define PROC_DUMP_SKB_2of6		14
#define PROC_DUMP_SKB_3of6		15
#define PROC_DUMP_SKB_4of6		16
#define PROC_DUMP_SKB_5of6		17
#define PROC_DUMP_SKB_6of6		18

#define PROC_DUMP_EMAC_REGISTERS	19

/*
 * bcmpcieth_net_dump - display EMAC information
 */
int bcmpcieth_net_dump(BcmEnet_devctrl *pDevCtrl, char *buf, int reqinfo)
{
    int  i;
    int len = 0;

	switch (reqinfo) {

/*----------------------------- TXBD --------------------------------*/
    case PROC_DUMP_TXBD_1of6: /* tx DMA BD descriptor ring 1 of 6 */
        len += sprintf(&buf[len], "\ntx buffer descriptor ring status.\n");
        len += sprintf(&buf[len], "BD\tlocation\tlength\tstatus addr\n");
        for (i = 0; i < pDevCtrl->nrTxBds/6; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(unsigned int)&pDevCtrl->txBds[i],
                   ntohs(pDevCtrl->txBds[i].length),
                   ntohs(pDevCtrl->txBds[i].status),
                   ntohl(pDevCtrl->txBds[i].address));
        }
        break;

    case PROC_DUMP_TXBD_2of6: /* tx DMA BD descriptor ring, 2 of 6 */
        for (i = pDevCtrl->nrTxBds/6; i < pDevCtrl->nrTxBds/3; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(unsigned int)&pDevCtrl->txBds[i],
                   ntohs(pDevCtrl->txBds[i].length),
                   ntohs(pDevCtrl->txBds[i].status),
                   ntohl(pDevCtrl->txBds[i].address));
        }
        break;

   case PROC_DUMP_TXBD_3of6: /* tx DMA BD descriptor ring, 3 of 6 */
        for (i = pDevCtrl->nrTxBds/3; i < pDevCtrl->nrTxBds/2; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(unsigned int)&pDevCtrl->txBds[i],
                   ntohs(pDevCtrl->txBds[i].length),
                   ntohs(pDevCtrl->txBds[i].status),
                   ntohl(pDevCtrl->txBds[i].address));
        }
        break;

   case PROC_DUMP_TXBD_4of6: /* tx DMA BD descriptor ring, 4 of 6 */
        for (i = pDevCtrl->nrTxBds/2; i < 2*pDevCtrl->nrTxBds/3; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(unsigned int)&pDevCtrl->txBds[i],
                   ntohs(pDevCtrl->txBds[i].length),
                   ntohs(pDevCtrl->txBds[i].status),
                   ntohl(pDevCtrl->txBds[i].address));
        }
        break;

  case PROC_DUMP_TXBD_5of6: /* tx DMA BD descriptor ring, 5 of 6 */
        for (i = 2*pDevCtrl->nrTxBds/3; i < 5*pDevCtrl->nrTxBds/6; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(unsigned int)&pDevCtrl->txBds[i],
                   ntohs(pDevCtrl->txBds[i].length),
                   ntohs(pDevCtrl->txBds[i].status),
                   ntohl(pDevCtrl->txBds[i].address));
        }
        break;

 case PROC_DUMP_TXBD_6of6: /* tx DMA BD descriptor ring, 5 of 6 */
        for (i = 5*pDevCtrl->nrTxBds/6; i < pDevCtrl->nrTxBds; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(unsigned int)&pDevCtrl->txBds[i],
                   ntohs(pDevCtrl->txBds[i].length),
                   ntohs(pDevCtrl->txBds[i].status),
                   ntohl(pDevCtrl->txBds[i].address));
        }
        break;

/*----------------------------- RXBD --------------------------------*/
    case PROC_DUMP_RXBD_1of6: /* rx DMA BD descriptor ring, 1 of 6 */
        len += sprintf(&buf[len], "\nrx buffer descriptor ring status.\n");
        len += sprintf(&buf[len], "BD\tlocation\tlength\tstatus\n");
        for (i = 0; i < pDevCtrl->nrRxBds/6; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(unsigned int)&pDevCtrl->rxBds[i],
                   ntohs(pDevCtrl->rxBds[i].length),
                   ntohs(pDevCtrl->rxBds[i].status&0xffff),
				   ntohl(pDevCtrl->rxBds[i].address));
        }
        break;

   case PROC_DUMP_RXBD_2of6: /* rx DMA BD descriptor ring, 2 of 6 */
        for (i = pDevCtrl->nrRxBds/6; i < 2*pDevCtrl->nrRxBds/6; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(int)&pDevCtrl->rxBds[i],
                   ntohs(pDevCtrl->rxBds[i].length),
                   ntohs(pDevCtrl->rxBds[i].status&0xffff),
				   ntohl(pDevCtrl->rxBds[i].address));
        }
		break;

   case PROC_DUMP_RXBD_3of6: /* rx DMA BD descriptor ring, 3 of 6 */
        for (i = 2*pDevCtrl->nrRxBds/6; i < 3*pDevCtrl->nrRxBds/6; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(int)&pDevCtrl->rxBds[i],
                   ntohs(pDevCtrl->rxBds[i].length),
                   ntohs(pDevCtrl->rxBds[i].status&0xffff),
				   ntohl(pDevCtrl->rxBds[i].address));
        }
		break;

  case PROC_DUMP_RXBD_4of6: /* rx DMA BD descriptor ring, 4 of 6 */
        for (i = 3*pDevCtrl->nrRxBds/6; i < 4*pDevCtrl->nrRxBds/6; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(int)&pDevCtrl->rxBds[i],
                   ntohs(pDevCtrl->rxBds[i].length),
                   ntohs(pDevCtrl->rxBds[i].status&0xffff),
				   ntohl(pDevCtrl->rxBds[i].address));
        }
		break;

  case PROC_DUMP_RXBD_5of6: /* rx DMA BD descriptor ring, 5 of 6 */
        for (i = 4*pDevCtrl->nrRxBds/6; i < 5*pDevCtrl->nrRxBds/6; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(int)&pDevCtrl->rxBds[i],
                   ntohs(pDevCtrl->rxBds[i].length),
                   ntohs(pDevCtrl->rxBds[i].status&0xffff),
				   ntohl(pDevCtrl->rxBds[i].address));
        }
		break;

  case PROC_DUMP_RXBD_6of6: /* rx DMA BD descriptor ring, 5 of 6 */
        for (i = 5*pDevCtrl->nrRxBds/6; i < 6*pDevCtrl->nrRxBds/6; i++)
        {
            len += sprintf(&buf[len], "%03d\t%08x\t%04d\t%04x %08x\n",
                   i,(int)&pDevCtrl->rxBds[i],
                   ntohs(pDevCtrl->rxBds[i].length),
                   ntohs(pDevCtrl->rxBds[i].status&0xffff),
				   ntohl(pDevCtrl->rxBds[i].address));
        }
		break;

    case PROC_DUMP_DMADESC_STATUS:  /* DMA descriptors pointer and status */
        len += sprintf(&buf[len], "\nrx pointers:\n");
        len += sprintf(&buf[len], "DmaDesc *rxBds:\t\t%08x\n",(unsigned int)pDevCtrl->rxBds);
        len += sprintf(&buf[len], "DmaDesc *rxBdAssignPtr:\t%08x\n",(unsigned int)pDevCtrl->rxBdAssignPtr);
        len += sprintf(&buf[len], "DmaDesc *rxBdReadPtr:\t%08x\n",(unsigned int)pDevCtrl->rxBdReadPtr);
        len += sprintf(&buf[len], "DmaDesc *rxLastBdPtr:\t%08x\n",(unsigned int)pDevCtrl->rxLastBdPtr);
        len += sprintf(&buf[len], "DmaDesc *rxFirstBdPtr:\t%08x\n",(unsigned int)pDevCtrl->rxFirstBdPtr);

        len += sprintf(&buf[len], "\ntx pointers:\n");
        len += sprintf(&buf[len], "DmaDesc *txBds:\t\t%08x\n",(unsigned int)pDevCtrl->txBds);
        len += sprintf(&buf[len], "DmaDesc *txLastBdPtr:\t%08x\n",(unsigned int)pDevCtrl->txLastBdPtr);
        len += sprintf(&buf[len], "DmaDesc *txFirstBdPtr:\t%08x\n",(unsigned int)pDevCtrl->txFirstBdPtr);
        len += sprintf(&buf[len], "DmaDesc *txNextBdPtr:\t%08x\n",(unsigned int)pDevCtrl->txNextBdPtr);
        len += sprintf(&buf[len], "Enet_Tx_CB *txCbPtrHead:%08x\n",(unsigned int)pDevCtrl->txCbPtrHead);
        if (pDevCtrl->txCbPtrHead)
            len += sprintf(&buf[len], "txCbPtrHead->lastBdAddr:\t%08x\n",(unsigned int)pDevCtrl->txCbPtrHead->lastBdAddr);
        len += sprintf(&buf[len], "EnetTxCB *txCbPtrTail:\t%08x\n",(unsigned int)pDevCtrl->txCbPtrTail);
        if (pDevCtrl->txCbPtrTail)
            len += sprintf(&buf[len], "txCbPtrTail->lastBdAddr:\t%08x\n",(unsigned int)pDevCtrl->txCbPtrTail->lastBdAddr);
        len += sprintf(&buf[len], "txFreeBds (TxBDs %d):\t%d\n", NR_TX_BDS, (unsigned int) pDevCtrl->txFreeBds);

        len += sprintf(&buf[len], "\ntx DMA Channel Config(%08x)\t\t%08x\n", (unsigned int) pDevCtrl->txDma, (unsigned int) pDevCtrl->txDma->Control);
        len += sprintf(&buf[len], "tx DMA Intr Control/Status\t%08x\n", (unsigned int) pDevCtrl->txDma->IntStat);
        len += sprintf(&buf[len], "tx DMA Intr Mask\t\t%08x\n", (unsigned int) pDevCtrl->txDma->IntMask);
#if defined(BCM7118)
        len += sprintf(&buf[len], "tx DMA MaxBurst\t\t%08x\n", (unsigned int) pDevCtrl->txDma->MaxBurst);
        len += sprintf(&buf[len], "tx DMA DescriptorBase\t\t%08x\n", (unsigned int) pDevCtrl->txDmaState->DescriptorBase);
        len += sprintf(&buf[len], "tx DMA StateRam2\t\t%08x\n", (unsigned int) pDevCtrl->txDmaState->StateRam2);
        len += sprintf(&buf[len], "tx DMA StateRam3\t\t%08x\n", (unsigned int) pDevCtrl->txDmaState->StateRam3);
        len += sprintf(&buf[len], "tx DMA StateRam4\t\t%08x\n", (unsigned int) pDevCtrl->txDmaState->StateRam4);
#endif
        len += sprintf(&buf[len], "\nrx DMA Channel Config(%08x)\t\t%08x\n", (unsigned int) pDevCtrl->rxDma, (unsigned int) pDevCtrl->rxDma->Control);
        len += sprintf(&buf[len], "rx DMA Intr Control/Status\t%08x\n", (unsigned int) pDevCtrl->rxDma->IntStat);
        len += sprintf(&buf[len], "rx DMA Intr Mask\t\t%08x\n", (unsigned int) pDevCtrl->rxDma->IntMask);
#if defined(BCM7118)
        len += sprintf(&buf[len], "rx DMA MaxBurst\t\t%08x\n", (unsigned int) pDevCtrl->rxDma->MaxBurst);
        len += sprintf(&buf[len], "rx DMA DescriptorBase\t\t%08x\n", (unsigned int) pDevCtrl->rxDmaState->DescriptorBase);
        len += sprintf(&buf[len], "rx DMA StateRam2\t\t%08x\n", (unsigned int) pDevCtrl->rxDmaState->StateRam2);
        len += sprintf(&buf[len], "rx DMA StateRam3\t\t%08x\n", (unsigned int) pDevCtrl->rxDmaState->StateRam3);
        len += sprintf(&buf[len], "rx DMA StateRam4\t\t%08x\n", (unsigned int) pDevCtrl->rxDmaState->StateRam4);

        len += sprintf(&buf[len], "*(uint32 *)0xfffb0c00\t\t%08x\n", (unsigned int) *(unsigned int *)0xfffb0c00);
        len += sprintf(&buf[len], "*(uint32 *)0xfffb0d00\t\t%08x\n", (unsigned int) *(unsigned int *)0xfffb0d00);
        len += sprintf(&buf[len], "*(uint32 *)0xfffb0d04\t\t%08x\n", (unsigned int) *(unsigned int *)0xfffb0d04);

        len += sprintf(&buf[len], "*(uint32 *)HIF_CPU_INTR1_INTR_W1_STATUS\t\t%08x\n", (unsigned int) *(unsigned int *)HIF_CPU_INTR1_INTR_W1_STATUS);
        len += sprintf(&buf[len], "*(uint32 *)INT_IRQSTATUS\t\t%08x\n", (unsigned int) *(unsigned int *)INT_IRQSTATUS);
        len += sprintf(&buf[len], "*(uint32 *)INT_IRQMASK\t\t%08x\n", (unsigned int) *(unsigned int *)INT_IRQMASK);
        len += sprintf(&buf[len], "*(uint32 *)SUB_IRQ_MASK_STATUS\t\t%08x\n", (unsigned int) *(unsigned int *)SUB_IRQ_MASK_STATUS);
        len += sprintf(&buf[len], "*(uint32 *)SUB_R5F_IRQ_MASK_STATUS\t\t%08x\n", (unsigned int) *(unsigned int *)SUB_R5F_IRQ_MASK_STATUS);
        len += sprintf(&buf[len], "*(uint32 *)SUB_PCI_IRQ_MASK_STATUS\t\t%08x\n", (unsigned int) *(unsigned int *)SUB_PCI_IRQ_MASK_STATUS);
        len += sprintf(&buf[len], "*(uint32 *)HIF_CPU_INTR1_INTR_W1_MASK_STATUS\t\t%08x\n", (unsigned int) *(unsigned int *)HIF_CPU_INTR1_INTR_W1_MASK_STATUS);
        len += sprintf(&buf[len], "*(uint32 *)NSGISB_HOSTIRQMASK\t\t%08x\n", (unsigned int) *(unsigned int *)NSGISB_HOSTIRQMASK);
        len += sprintf(&buf[len], "*(uint32 *)NSGISB_HOSTIRQSTATUS\t\t%08x\n", (unsigned int) *(unsigned int *)NSGISB_HOSTIRQSTATUS);
        len += sprintf(&buf[len], "*(uint32 *)MBOX_MAILBOX_1\t\t%08x\n", (unsigned int) *(unsigned int *)MBOX_MAILBOX_1);
        len += sprintf(&buf[len], "mcode\t\t%08lx\n", mcode);
		len += sprintf(&buf[len], "*(uint32 *)SUN_TOP_CTRL_TEST_PORT_CTRL = %08x\n", (unsigned int)*(unsigned int *)SUN_TOP_CTRL_TEST_PORT_CTRL);
#endif
		break;

    case PROC_DUMP_EMAC_REGISTERS: /* EMAC registers */
    default:
        break;
    }

    return len;
}

static int dev_proc_engine(loff_t pos, char *buf)
{
    BcmEnet_devctrl *pDevCtrl;
    int len = 0;

    if (pDevContext)
        pDevCtrl = pDevContext;
    else
        return len;

    if (pos >= PROC_DUMP_TXBD_1of6 && pos <= PROC_DUMP_EMAC_REGISTERS) {
        len = bcmpcieth_net_dump(pDevCtrl, buf, pos);
    }
    return len;
}

/*
 *  read proc interface
 */
static ssize_t eth_proc_read(struct file *file, char *buf, size_t count,
        loff_t *pos)
{
    char *page;
    int len = 0, x, left;

    page = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!page)
        return -ENOMEM;
    left = PAGE_SIZE - 256;
    if (count < left)
        left = count;

    for (;;) {
        x = dev_proc_engine(*pos, &page[len]);
        if (x == 0)
            break;
        if ((x + len) > left)
            x = -EINVAL;
        if (x < 0) {
            break;
        }
        len += x;
        left -= x;
        (*pos)++;
        if (left < 256)
            break;
    }
    if (len > 0 && copy_to_user(buf, page, len))
        len = -EFAULT;
    kfree(page);
    return len;
}

/*
 * /proc/driver/eth_rtinfo
 */
static struct file_operations eth_proc_operations = {
        read: eth_proc_read, /* read_proc */
};


#define BUFFER_LEN PAGE_SIZE

void bcmpcieth_dump(BcmEnet_devctrl *pDevCtrl)
{
    // We may have to do dynamic allocation here, since this is run from
    // somebody else's stack
    char buffer[BUFFER_LEN];
    int len, i;
    int bufcnt;
    struct sk_buff *skb;

    if (pDevCtrl == NULL) {
        return; // EMAC not initialized
    }

	/* First qtr of TX ring */
	printk("\ntx buffer descriptor ring status.\n");
    printk("BD\tlocation\tlength\tstatus addr\n");
	printk("%d TX buffers\n", pDevCtrl->nrTxBds);
    for (i = 0; i < pDevCtrl->nrTxBds; i++)
    {
        len += printk("%03d\t%08x\t%04d\t%04x %08x\n",
                    i,(unsigned int)&pDevCtrl->txBds[i],
                    ntohs(pDevCtrl->txBds[i].length),
                    ntohs(pDevCtrl->txBds[i].status),
		            ntohl(pDevCtrl->txBds[i].address));
    }


	printk("\nrx buffer descriptor ring status.\n");
    printk("BD\tlocation\tlength\tstatus\n");
    for (i = 0; i < pDevCtrl->nrRxBds; i++)
    {
        len += printk("%03d\t%08x\t%04d\t%04x %08x\n",
                    i,(int)&pDevCtrl->rxBds[i],
                    ntohs(pDevCtrl->rxBds[i].length),
                    ntohs(pDevCtrl->rxBds[i].status),
			        ntohl(pDevCtrl->rxBds[i].address));
    }


	/* DMA descriptor pointers and status */
	printk("\n\n=========== DMA descriptrs pointers and status ===========\n");
	len = bcmpcieth_net_dump(pDevContext, buffer, PROC_DUMP_DMADESC_STATUS);
	if (len >= BUFFER_LEN)
        printk("************ dump_emac: ERROR: Buffer too small, PROC_DUMP_DMADESC_STATUS need %d\n", len);
	buffer[len] = '\0';
	printk("%s\n\n", buffer);

	printk("\n\n=========== SKB buffer usage ===========\n");
	bufcnt = 0;
    printk("\nskb\taddress\t\tuser\tdataref\n");
    for (i = 0; i < MAX_RX_BUFS; i++) {
        skb = pDevCtrl->skb_pool[i];
        printk( "%d\t%08x\t%d\t%d\n",
                    i, (int)skb, atomic_read(&skb->users), atomic_read(skb_dataref(skb)));
        if ((atomic_read(&skb->users) == 2) && (atomic_read(skb_dataref(skb)) == 1))
            bufcnt++;

	}
    printk("\nnumber RX SKBs available %d\n", bufcnt);


	/* EMAC Registers */
	printk("\n\n=========== EMAC Registers  ===========\n");
	len = bcmpcieth_net_dump(pDevContext, buffer, PROC_DUMP_EMAC_REGISTERS);
	if (len >= BUFFER_LEN) printk("************ dump_emac: ERROR: Buffer too small, PROC_DUMP_EMAC_REGISTERS need %d\n", len);
	buffer[len] = '\0';
	printk("%s\n\n", buffer);

	printk("Other Debug info: Loop Count=%d, #rx_errors=%lu, #resets=%d, #overflow=%d,#NO_DESC=%d\n",
		loopCount,  pDevCtrl->stats.rx_errors, pDevCtrl->gNumResetsErrors, pDevCtrl->gNumOverflowErrors, pDevCtrl->gNumNoDescErrors);
}

#endif


static void dump_pcieth(void)
{
#ifdef USE_PROC
	if (pDevContext) {
		bcmpcieth_dump(pDevContext);
	}
#endif
}
EXPORT_SYMBOL(dump_pcieth);

static void bcmpcieth_dev_setup_dummy(struct net_device *dev)
{
	pDevContext = (struct BcmEnet_devctrl *) netdev_priv(dev);
	/* initialize the context memory */
	memset(pDevContext, 0, sizeof(BcmEnet_devctrl));
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static const struct net_device_ops bcmpcieth_netdev_ops = {
	.ndo_open = bcmpcieth_net_open,
	.ndo_stop = bcmpcieth_net_close,
	.ndo_start_xmit = bcmpcieth_locked_net_xmit,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_multicast_list = bcm_set_multicast_list,
	.ndo_set_mac_address = bcm_set_mac_addr,
	.ndo_do_ioctl = bcmpcieth_enet_ioctl,
	.ndo_tx_timeout = bcmpcieth_net_timeout,
	.ndo_get_stats = bcmpcieth_net_query
};
#endif

static void bcmpcieth_dev_setup(struct net_device *dev)
{
	int ret;
	ulong regpa;
    unsigned char macAddr[ETH_ALEN];
	unsigned int i;
#if (PASS_THROUGH)
#else
 	pDevContext = (struct BcmEnet_devctrl *) netdev_priv(dev);
#endif
#if defined(BCM7118)
	/* read chip id and revision */
    pDevContext->chipId  = *((unsigned long *)0xb0404000) >> 16;
    pDevContext->chipRev = (*((unsigned long *)0xb0404000) & 0x0000FFFF) + 0xa0;

	/* map register address space */
	regpa = M2MDMAC_BASE;
	pDevContext->m2mRegs = (M2MBlockRegisters *)regpa;
	if ((pDevContext->m2mRegs = (M2MBlockRegisters *) ioremap_nocache(regpa, M2MDMAC_SIZE)) == NULL) {
		TRACE(("bcmpcieth_dev_setup: M2M ioremap failed\n"));
		free_netdev(dev);
		pDevContext = NULL;
        return;
	}

	regpa = NSGISBC_BASE;
	pDevContext->nsGisbRegs = (NSGISBRegisters *)regpa;
	if ((pDevContext->nsGisbRegs = (NSGISBRegisters *) ioremap_nocache(regpa, NSGISBC_SIZE)) == NULL) {
		TRACE(("bcmpcieth_dev_setup: NSGISB ioremap failed\n"));
		free_netdev(dev);
		pDevContext = NULL;
        return;
	}

	regpa = SUBC_BASE;
	pDevContext->subRegs = (SUBRegisters *)regpa;
	if ((pDevContext->subRegs = (SUBRegisters *) ioremap_nocache(regpa, SUBC_SIZE)) == NULL) {
		TRACE(("bcmpcieth_dev_setup: SUB ioremap failed\n"));
		free_netdev(dev);
		pDevContext = NULL;
        return;
	}
    /* BULK_WIN_SIZE = Max kmalloc will allow under Linux */
    imagePointer = (uint32) kmalloc(BULK_WIN_SIZE, GFP_KERNEL);
#else
	/* MPI registers are mapped by BAR0 */
	regpa = pci_resource_start(pDevContext->pdev, 0);
	TRACE(("bcmpcieth_net_probe: 0x%lx, %d\n", regpa, pDevContext->pdev->irq));

	// requesting resources
	if ((pDevContext->mpiRegs = (MpiRegisters *) ioremap_nocache(regpa, 256)) == NULL) {
		TRACE(("bcmpcieth_dev_setup: ioremap failed\n"));
		free_netdev(dev);
		pDevContext = NULL;
        return;
	}

	ioremap_nocache(pci_resource_start(pDevContext->pdev, 0), pci_resource_len(pDevContext->pdev, 0));
    pDevContext->mbar3_mapped =
		(int)ioremap_nocache(pci_resource_start(pDevContext->pdev, 3), pci_resource_len(pDevContext->pdev, 3));
    pDevContext->mbar4_mapped =
		(int)ioremap_nocache(pci_resource_start(pDevContext->pdev, 4), pci_resource_len(pDevContext->pdev, 4));

    /* figure out which chip we're running on */
    pDevContext->chipId  = (uint16)(pDevContext->mpiRegs->VendorId >> 16);
    pDevContext->chipRev = (uint16)pDevContext->mpiRegs->RevisionId;

	switch (pDevContext->chipId) {
        case 0x3348:
        case 0x3255:
            break;
        default:
            printk(KERN_DEBUG CARDNAME": not found\n");
			free_netdev(dev);
			pDevContext = NULL;
            return;
    }

	mdelay(10);
#endif
    /* print the ChipID and module version info */
	printk(KERN_INFO CARDNAME ": Broadcom BCM%X%X Ethernet Network Device " VER_STR "\n",
                    pDevContext->chipId, pDevContext->chipRev);

	/* initialize the device */
 	if ((ret = bcmpcieth_init_dev(pDevContext))) {
 		free_netdev(dev);
 		pDevContext = NULL;
        return;
    }

    /* register the interrupt service handler */
	/* At this point dev is not initialized yet, so use dummy name */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	request_irq(pDevContext->rxIrq, bcmpcieth_net_isr, /*IRQF_DISABLED|*/IRQF_SHARED, dev->name, pDevContext);
#else
	request_irq(pDevContext->rxIrq, bcmpcieth_net_isr, SA_INTERRUPT|SA_SHIRQ, dev->name, pDevContext);
#endif

    spin_lock_init(&pDevContext->lock);
    atomic_set(&pDevContext->devInUsed, 0);

	/*
     * Setup the timer
     */
#if defined(BCM7118)
    init_timer(&pDevContext->reboot_timer);
    pDevContext->reboot_timer.data = (unsigned long)pDevContext;
    pDevContext->reboot_timer.function = reboot_timer_handler;
#endif

    init_timer(&pDevContext->reset_timer);
    pDevContext->reset_timer.data = (unsigned long)pDevContext;
    pDevContext->reset_timer.function = mb_timer_handler;

    init_timer(&pDevContext->refill_timer);
    pDevContext->refill_timer.data = (unsigned long)pDevContext;
    pDevContext->refill_timer.function = refill_timer_handler;

#if !defined(CONFIG_MIPS_BCM7420) && !defined(CONFIG_BCM7420)
#ifdef DUMP_DATA
    printk(KERN_INFO CARDNAME ": CP0 BRCMCONFIG: %08X\n", read_c0_diag(/*$22*/));
    printk(KERN_INFO CARDNAME ": CP0 MIPSCONFIG: %08X\n", read_c0_config(/*$16*/));
    printk(KERN_INFO CARDNAME ": CP0 MIPSSTATUS: %08X\n", read_c0_status(/*$12*/));
#endif
#endif /* !defined(CONFIG_MIPS_BCM7420) && !defined(CONFIG_BCM7420) */

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
    SET_MODULE_OWNER(dev);
#endif

    /* back pointer to our device */
    pDevContext->dev = dev;

    /* Read MAC address */
	{
		uint8 flash_eaddr[6];
#if defined(CONFIG_MIPS_BCM7420) || defined(CONFIG_BCM7420)
		flash_eaddr[0] = 0x00;
		flash_eaddr[1] = 0xc0;
		flash_eaddr[2] = 0xa8;
		flash_eaddr[3] = 0x74;
		flash_eaddr[4] = 0x3b;
		flash_eaddr[5] = 0x52;
		printk("%s(7420): Default MAC address %02X:%02X:%02X:%02X:%02X:%02X used\n",
		       dev->name,
		       flash_eaddr[0],flash_eaddr[1],flash_eaddr[2],
		       flash_eaddr[3],flash_eaddr[4],flash_eaddr[5]);

#else /* defined(CONFIG_MIPS_BCM7420) || defined(CONFIG_BCM7420) */
#if defined( CONFIG_MTD_BRCMNAND )
		extern int gNumHwAddrs;
		extern unsigned char* gHwAddrs[];

		if (gNumHwAddrs >= 1)
		{
			for (i=0; i < 6; i++)
			{
				flash_eaddr[i] = (uint8) gHwAddrs[0][i];
			}
		}
		else
		{
			printk(KERN_ERR "%s: No MAC addresses defined, use hardcoded one\n", __FUNCTION__);
			flash_eaddr[0] = 0x00;
			flash_eaddr[1] = 0xc0;
			flash_eaddr[2] = 0xa8;
			flash_eaddr[3] = 0x74;
			flash_eaddr[4] = 0x3b;
			flash_eaddr[5] = 0x52;
		}
		printk("%s: MAC address %02X:%02X:%02X:%02X:%02X:%02X fetched from bootloader\n",
			dev->name,
			flash_eaddr[0],flash_eaddr[1],flash_eaddr[2],
			flash_eaddr[3],flash_eaddr[4],flash_eaddr[5]
			);

#else
		void *virtAddr;
		uint16 word;

		virtAddr = (void*) KSEG1ADDR((getPhysFlashBase() + FLASH_MACADDR_OFFSET));

        /* It is a common problem that the flash and/or Chip Select are
		 * not initialized properly, so leave this printk on
		 */
		word=0;
		word=readw(virtAddr);
		flash_eaddr[0]=(uint8) (word & 0x00FF);
		flash_eaddr[1]=(uint8) ((word & 0xFF00) >> 8);
		word=readw(virtAddr+2);
		flash_eaddr[2]=(uint8) (word & 0x00FF);
		flash_eaddr[3]=(uint8) ((word & 0xFF00) >> 8);
		word=readw(virtAddr+4);
		flash_eaddr[4]=(uint8) (word & 0x00FF);
		flash_eaddr[5]=(uint8) ((word & 0xFF00) >> 8);
		// get PCI ETH MAC address by incrementing it by 1
		flash_eaddr[5] = (uint8) (flash_eaddr[5] + 1);

		/* Use hard coded value if Flash not properly initialized */
		if ((*flash_eaddr & 0xff) == 0xff)
		{
#ifdef USE_HARDCODED_MACADDR
			flash_eaddr[0] = 0x00;
			flash_eaddr[1] = 0xc0;
			flash_eaddr[2] = 0xa8;
			flash_eaddr[3] = 0x74;
			flash_eaddr[4] = 0x3b;
			flash_eaddr[5] = 0x52;
			printk("%s: Default MAC address %02X:%02X:%02X:%02X:%02X:%02X used\n",
				dev->name,
				flash_eaddr[0],flash_eaddr[1],flash_eaddr[2],
				flash_eaddr[3],flash_eaddr[4],flash_eaddr[5]);
#else
			printk("%s: Cannot read EMAC address from flash.  Please run macprog from CFE\n", CARDNAME);
#endif
		}
#endif
#endif /* defined(CONFIG_MIPS_BCM7420) || defined(CONFIG_BCM7420) */
		memcpy(macAddr, flash_eaddr, 6);
	}

    /* fill in the MAC address */
    for (i = 0; i < 6; i++) {
        dev->dev_addr[i] = macAddr[i];
    }

    /* print the Ethenet address */
    printk("%s MAC Address: ", CARDNAME);
    for (i = 0; i < 5; i++) {
        printk("%2.2X:", dev->dev_addr[i]);
    }
    printk("%2.2X\n", dev->dev_addr[i]);

    dev->irq                = pDevContext->rxIrq;
    dev->base_addr          = regpa;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	dev->netdev_ops         = &bcmpcieth_netdev_ops;
	dev->watchdog_timeo     = 2 * HZ;
#else
	dev->open				= bcmpcieth_net_open;		
	dev->stop				= bcmpcieth_net_close;
	dev->hard_start_xmit	= bcmpcieth_locked_net_xmit;
	dev->tx_timeout 		= bcmpcieth_net_timeout;
	dev->watchdog_timeo 	= 2*HZ;
	dev->get_stats			= bcmpcieth_net_query;
	dev->set_mac_address	= bcm_set_mac_addr;
	/* no multicast support */
	dev->set_multicast_list = bcm_set_multicast_list;
	dev->do_ioctl			= &bcmpcieth_enet_ioctl;
#endif

	ether_setup(dev);

	// Let boot setup info overwrite default settings.
	netdev_boot_setup_check(dev);

	TRACE(("bcmemac_net_probe: dev 0x%x\n", (unsigned int)dev));

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,18)
	SET_MODULE_OWNER(dev);
#endif

}
/*
 *      bcmpcieth_net_probe: - Probe Ethernet switch and allocate device
 */
#ifdef MODULE
static
#endif
int __init bcmpcieth_net_probe(void)
{
    static int probed = 0;
#if (PASS_THROUGH)
    struct net_device *ecmdev = NULL;
	int i;
#endif
    struct net_device *dev = NULL;
	int ret;

#ifdef USE_PROC
	struct proc_dir_entry *p;
#endif
#if defined(BCM7118)
	int i;
	unsigned long res;

	if (ubase && usize)
	{
		for (i = 0, res = 0; i < 32; i++)
			if ((1 << i) & usize)
				res++;
		if (res != 1)
		{
			printk(KERN_ERR CARDNAME ": -usize must be power of 2\n");
			return -ENODEV;
		}
		if (ubase % usize)
		{
			printk(KERN_ERR CARDNAME ": -ubase must be multiple of -usize\n");
			return -ENODEV;
		}
		printk(KERN_INFO CARDNAME ": Using unified base %08x, unified size %08x\n", ubase, usize);
	}
#else
	struct pci_dev *pdev;
    uint16 cmd;

	/* try to find the device on PCI bus */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	pdev = pci_get_device(0x14e4, 0x3255, NULL);
#else
	pdev = pci_find_device(0x14e4, 0x3255, NULL);
#endif
	if(pdev == NULL)
	{
		printk(KERN_ERR CARDNAME ": bcmpcieth_net_probe: can not find 3255 on PCI!\n");
		return -ENODEV;
	}


	/* enable the device before going any further */
	if (pci_enable_device(pdev))
		return -EIO;

	/* set it to be master and enable memory access */
    pci_read_config_word(pdev, PCI_COMMAND, &cmd);
    cmd |= PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
    pci_write_config_word(pdev, PCI_COMMAND, cmd);
#endif

    TRACE(("bcmpcieth_net_probe\n"));

    if (probed == 0) 
    {
#ifdef USE_PROC				 
        /*
         * create a /proc entry for display driver runtime information
         */
        if ((p = create_proc_entry("driver/pcieth_rtinfo", 0, NULL)) == NULL)
            return -ENOMEM;
        p->proc_fops = &eth_proc_operations;	   
#endif

#if (PASS_THROUGH)
        /* Support for tagging and multiple virtual Eth interfaces */
        if ((tagged_vdevs)) 
        { // && (cfg->phy_type != BRCM_PHY_TYPE_MOCA))
            dev = alloc_netdev(sizeof(struct BcmVirtEnet_devctrl), "eth%d", ether_setup);
	      //  dev = alloc_netdev(sizeof(struct BcmEnet_devctrl), "eth%d", bcmpcieth_dev_setup_dummy);

	        if (dev == NULL) 
	        {
		     	printk(KERN_ERR CARDNAME": Unable to allocate "
			    	"net_device structure!\n");
		     	return -ENODEV;
	        }

            pDevContext = kmalloc(sizeof(struct BcmEnet_devctrl), GFP_KERNEL);
            memset(pDevContext, 0, sizeof(struct BcmEnet_devctrl));

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
            pVirtDevCtrl = netdev_priv(dev);
#else
            pVirtDevCtrl = (struct BcmVirtEnet_devctrl *)dev->priv;	
#endif 

            pVirtDevCtrl->pDevCtrl = pDevContext;
            pDevContext->has_virt_dev = 1;
            pVirtDevCtrl->if_num = vdev_names[0].tag;
        }
        /* Normal case */
        else 
        {
#endif
	        dev = alloc_netdev(sizeof(struct BcmEnet_devctrl), "eth%d", bcmpcieth_dev_setup_dummy);

		    TRACE(("bcmpcieth_net_probe: after alloc_netdev. dev = %08lx\n", (uint32)dev));
		    // if bcmpcieth_dev_setup fails for some reason, pDevContext is NULL
	        if (dev == NULL) 
	        {
		     	printk(KERN_ERR CARDNAME": Unable to allocate "
			    	"net_device structure!\n");
		     	return -ENODEV;
	        }

            pDevContext = (struct BcmEnet_devctrl *)netdev_priv(dev);
#if (PASS_THROUGH)
            pDevContext->has_virt_dev = 0;
        }
#endif

#if defined(BCM7118)
		/* need to initialize the irq here */
		pDevContext->rxIrq = BCM_LINUX_BNM_IRQ;
		/* pointer to the PCI device structure */
		pDevContext->pdev = NULL;
		/* remember the interrupt */
		dev->irq = BCM_LINUX_BNM_IRQ;
#else
		/* need to initialize the irq here */
		pDevContext->rxIrq = pdev->irq;
		/* pointer to the PCI device structure */
		pDevContext->pdev = pdev;
		/* remember the interrupt */
		dev->irq = pdev->irq;
#endif
		/* do the initialization */
   		bcmpcieth_dev_setup(dev);

		if (pDevContext == NULL) 
		{
			printk(KERN_ERR CARDNAME": Unable to initialize device!\n");
			return -ENODEV;
		}
    }

    if ((ret = register_netdev(dev)) != 0) 
    {
        free_netdev(dev);
        return ret;
	}
#if (PASS_THROUGH)
    else
    {
        if (tagged_vdevs) 
        {
            struct BcmVirtEnet_devctrl *childDev;

            pDevContext->p_virt_dev = kmalloc(sizeof(struct net_device *) * (NUM_TAGS - 1), GFP_KERNEL);
            //TODO:  CHECK AND HANDLE KMALLOC FAILURE HERE
            // Create virtual devices for num_tags-1 (the main device uses the first tag)
            for (i = 1; i < NUM_TAGS; i++) 
            {
                printk(KERN_INFO "bcmgenet: Adding pseudo ECM bridge device %s\n", vdev_names[i].ifname);
                ecmdev = alloc_netdev(sizeof(struct BcmVirtEnet_devctrl), vdev_names[i].ifname, ether_setup);
                if (ecmdev == NULL) 
                {
                    printk(KERN_ERR "bcmgenet: can't allocate ecm pseudo device\n");
					return -ENODEV;
                }

				childDev = netdev_priv(ecmdev);
                childDev->pDevCtrl = pDevContext;
                childDev->if_num = vdev_names[i].tag;
                pDevContext->p_virt_dev[i - 1] = ecmdev;

                memcpy(ecmdev->dev_addr, vdev_names[i].macaddr, 6);
   
                ecmdev->irq                = pDevContext->rxIrq;
                ecmdev->base_addr          = dev->base_addr;
                #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
                ecmdev->netdev_ops         = &bcmpcieth_netdev_ops;
                ecmdev->watchdog_timeo     = 2 * HZ;
                #else
                ecmdev->open				= bcmpcieth_net_open;
                ecmdev->stop				= bcmpcieth_net_close;
                ecmdev->hard_start_xmit	= bcmpcieth_locked_net_xmit;
                ecmdev->tx_timeout 		= bcmpcieth_net_timeout;
                ecmdev->watchdog_timeo 	= 2*HZ;
                ecmdev->get_stats			= bcmpcieth_net_query;			
                ecmdev->set_mac_address	= bcm_set_mac_addr;
                /* no multicast support */
                ecmdev->set_multicast_list = bcm_set_multicast_list;
                ecmdev->do_ioctl			= &bcmpcieth_enet_ioctl;
                #endif
				ether_setup(ecmdev);
                    printk(KERN_ERR "Register virtual devices\n");

               // netif_carrier_off(ecmdev);
                if (register_netdev(ecmdev)) 
                {
                    printk(KERN_ERR "bcmgenet: can't register ecm pseudo device\n");
                } 
                else 
                {
                    printk(KERN_INFO "bcmgenet: registered device %s\n", ecmdev->name);
                }
            }
	    } // tagged devices
    }
#endif
#ifdef CES2006_DEMO
	initcode();
#endif

	TRACE(("<-- bcmpcieth_net_probe\n"));
    return 0;
}

/* get ethernet port's status; return nonzero for Link up, 0 for Link down */
static int bcmIsEnetUp(uint32 uPhyAddr)
{
    return 1;
}

/*
 * Generic cleanup handling data allocated during init. Used when the
 * module is unloaded or if an error occurs during initialization
 */
static void bcmpcieth_init_cleanup(struct net_device *dev)
{
	TRACE(("%s: bcmpcieth_init_cleanup\n", dev->name));
    unregister_netdev(dev);
#ifdef USE_PROC
    remove_proc_entry("driver/eth_rtinfo", NULL);
#endif
}

static void __exit bcmpcieth_module_cleanup(void)
{
    Enet_Tx_CB *txCBPtr;
    int i;
    BcmEnet_devctrl *pDevCtrl = pDevContext;
	struct net_device *dev = pDevCtrl->dev;
    //void* k0p = NULL;

    TRACE(("bcmpcieth_module_cleanup\n"));

    if (pDevCtrl) {
        /* disable DMA */
        pDevCtrl->txDma->Control = 0;
        pDevCtrl->rxDma->Control = 0;

        /* free the irq */
            if (pDevCtrl->rxIrq)
        {
            BcmHalInterruptDisable(pDevCtrl->rxIrq);
            free_irq(pDevCtrl->rxIrq, pDevCtrl);
        }

        /* free the skb in the txCbPtrHead */
        while (pDevCtrl->txCbPtrHead)  {
            pDevCtrl->txFreeBds += pDevCtrl->txCbPtrHead->nrBds;

            dev_kfree_skb (pDevCtrl->txCbPtrHead->skb);

            txCBPtr = pDevCtrl->txCbPtrHead;

            /* Advance the current reclaim pointer */
            pDevCtrl->txCbPtrHead = pDevCtrl->txCbPtrHead->next;

            /* Finally, return the transmit header to the free list */
            txCb_enq(pDevCtrl, txCBPtr);
        }

                bcmpcieth_init_cleanup(pDevCtrl->dev);
        /* release allocated receive buffer memory */
        for (i = 0; i < MAX_RX_BUFS; i++) {
            if (pDevCtrl->skb_pool[i] != NULL) {
                dev_kfree_skb (pDevCtrl->skb_pool[i]);
                pDevCtrl->skb_pool[i] = NULL;
            }
        }
        /* free the transmit buffer descriptor */
        if (pDevCtrl->txBds) {
	     	pDevCtrl->txBds = NULL;
        }

        /* free the receive buffer descriptor */
        if (pDevCtrl->rxBds) {
		    pDevCtrl->rxBds = NULL;
	    }
        /* free the transmit control block pool */
        if (pDevCtrl->txCbs) {
        	kfree(pDevCtrl->txCbs);
		    pDevCtrl->txCbs = NULL;
	    }
        if (pDevCtrl->rxCbs) {
        	kfree(pDevCtrl->rxCbs);
		    pDevCtrl->rxCbs = NULL;
	    }

        free_netdev(dev);
        pDevContext = NULL;

	// DeInit remote flash structure
	if( localDocsisNonvol.pDynmNonvol )
		kfree( (void *)localDocsisNonvol.pDynmNonvol );
	localDocsisNonvol.pDynmNonvol = 0;
	localDocsisNonvol.DynmLen = 0;
	if( localDocsisNonvol.pPermNonvol )
		kfree( (void *)localDocsisNonvol.pPermNonvol );
	localDocsisNonvol.pPermNonvol = 0;
	localDocsisNonvol.PermLen = 0;
#if defined(BCM7118)
    kfree((void*) imagePointer);
#endif
#ifdef CES2006_DEMO
	uninitcode();
#endif

    }
}

static int bcmpcieth_enet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
#if (PASS_THROUGH)
	 BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *) pDevContext;
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);
#else
	BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)dev->priv;
#endif
#endif
    int status;
	BcmEnet_ioctlparms ioctlparms;
    uint32 offset;
    NETFILTER filter;

    /* we can add sub-command in ifr_data if we need to in the future */
    switch (cmd)
    {
    case SIOCGLINKSTATE:
        status = bcmIsEnetUp(pDevCtrl->phyAddr);
        if (copy_to_user(rq->ifr_data, (void*)&status, sizeof(int))) {
            return -EFAULT;
        }
        break;

    case SIOCSCLEARMIBCNTR:
        ASSERT(pDevCtrl != NULL);
        memset(&pDevCtrl->stats, 0, sizeof(struct net_device_stats));
        break;

    case SIOCSNETFILTER:
        ASSERT(pDevCtrl != NULL);
        copy_from_user((void*)&filter, rq->ifr_data, sizeof(NETFILTER));
        printk(KERN_INFO CARDNAME " bcmpcieth_enet_ioctl: filter.ip = %08x, filter.port = %08x\n", filter.ip_addr, filter.port);
        break;

    case SIOCGETMBOX:
        offset = 0;
		if (pDevCtrl->mailbox1)
		{
			offset = pDevCtrl->mailbox1;
			pDevCtrl->mailbox1 = 0;
		}

        copy_to_user(rq->ifr_data, (void *)&offset, sizeof(offset));
        break;

#if !defined(BCM7118)
    case SIOCGETWINDOW:
        copy_from_user((void *)&ioctlparms, (void*)rq->ifr_data, sizeof(BcmEnet_ioctlparms));
#if 1 // 7401C0 Workaround
	{
	int i, data[2];
	volatile int *p = (volatile int *) (pDevCtrl->mbar3_mapped + ioctlparms.from);
	for (i=0;i<ioctlparms.length>>2; i++)
		data[i] = *p++;
        copy_to_user((void *)ioctlparms.to, data, ioctlparms.length);
	}
#else
        copy_to_user((void *)ioctlparms.to, (void *)(pDevCtrl->mbar3_mapped + ioctlparms.from), ioctlparms.length);
#endif
        break;

    case SIOCPUTWINDOW:
        copy_from_user((void *)&ioctlparms, (void*)rq->ifr_data, sizeof(BcmEnet_ioctlparms));
#if 1 // 7401C0 Workaround
	{
	int i;
	volatile int *p= (volatile int *)(pDevCtrl->mbar3_mapped + ioctlparms.to);
        copy_from_user(putwindow_buf, (void *)ioctlparms.from, ioctlparms.length);
        for (i=0; i< ioctlparms.length>>2;i++)
		*p++ = putwindow_buf[i];
	}
#else
        copy_from_user((void *)(pDevCtrl->mbar3_mapped + ioctlparms.to), (void *)ioctlparms.from, ioctlparms.length);
#endif
        break;

    case SIOCBULKDMA:
    {
        int alignedByte;

        copy_from_user((void *)&ioctlparms, (void *)rq->ifr_data, sizeof(BcmEnet_ioctlparms));
        alignedByte = (ioctlparms.length + 3) & 0xfffffffc;
        copy_from_user((void *)(pDevCtrl->mbar4_mapped + ioctlparms.to), (void *)ioctlparms.from, alignedByte);
        pDevCtrl->mpiRegs->MailBox1 = savedmailbox1;
        savedmailbox1 = 0;
        break;
    }
#else	  //7118
    case SIOCBULKDMA:
    {
        int filesize;
        void *pcopyfrom;

        if(copy_from_user((void *)&ioctlparms, (void *)rq->ifr_data, sizeof(BcmEnet_ioctlparms)))
        {
			printk(KERN_INFO CARDNAME ": SIOCBULKDMA: Not all buffer copied\n");
            break;
		}

        // work arround older bootloader code: handle this extra cmd
        if( (savedmailbox1 & 0x0000ffff) != 0x00000001 )
        {
            /* work arround bootloader code */
			printk(KERN_INFO CARDNAME ": Extra mbox cmd! Please update your bcmscbeth.ko driver!\n");
            pDevCtrl->nsGisbRegs->mBox1 = savedmailbox1;
            pDevCtrl->mailbox1 = savedmailbox1 = 0;
            break;
        }

        filesize = (int) ioctlparms.length;
        pcopyfrom = (void *)ioctlparms.from;
        /* Optimize image transfer in one ioctl call, spin here untill full image is copied */
        while( filesize || savedmailbox1 )
        {
            if( savedmailbox1 )
            {
                //printk("-->Process mbox cmd %08lx. (filesize=%d)\n", savedmailbox1, filesize);
                if (filesize > BULK_WIN_SIZE)
                    copy_from_user((void *) (K0_TO_K1(imagePointer)), pcopyfrom, BULK_WIN_SIZE);
                else
                    copy_from_user((void *) (K0_TO_K1(imagePointer)), pcopyfrom, filesize);

                /* inform the eCM to come get it */
                pDevCtrl->nsGisbRegs->mBox1 = savedmailbox1;
                pDevCtrl->mailbox1 = savedmailbox1 = 0;

                /* update pointer and length to go */
                if( filesize > BULK_WIN_SIZE )
                {
                    filesize -= BULK_WIN_SIZE;
                    pcopyfrom += BULK_WIN_SIZE;
                }
                else
                {
                    printk("Done!\n");
                    break;  /* exit while loop */
                }
            }
        }
        break;
    }

#endif
#if !defined(BCM7118)
    case SIOCPREPAREFORRESET:
        pDevCtrl->PciHostDmaReady = FALSE;
        break;
#endif
#if (PASS_THROUGH)
     case SIOCTAGGINGON:
        printk(KERN_INFO "Turning tagging on!\n");
        tagging_disable = 0;
        break;

    case SIOCTAGGINGOFF:
        printk(KERN_INFO "Turning tagging off!\n");
        tagging_disable = 1;
        break;
#endif
    default:
		break;
    }
    return 0;
}
#if defined(BCM7118)
static int secure_BNM_remap (uint32 mask1, uint32 base_in1, uint32 base_out1, uint32 mask2, uint32 base_in2, uint32 base_out2)
{
	volatile unsigned long *aegis_cmd_buf = (volatile unsigned long *)AEGIS_COMMAND_BUFFER;
	volatile unsigned long *aegis_res_buf = (volatile unsigned long *)AEGIS_RESPONSE_BUFFER;

	//printk("===>secure_BNM_boot (%08x, %08x, %08x, %08x, %08x, %08x)\n", mask1, base_in1, base_out1,
	//	mask2, base_in2, base_out2);
	while ( (*((volatile unsigned long *)BSP_GLB_NONSECURE_GLB_IRDY) & 0x02) == 0 );
	aegis_cmd_buf[0] = 0x10;		// Version Number
	aegis_cmd_buf[1] = 0x11;		// Owner ID
	aegis_cmd_buf[2] = 0x00;		// Cont mode
	aegis_cmd_buf[3] = 0x2c;		// BNM boot
	aegis_cmd_buf[4] = 0x18;			// Parameter length
	aegis_cmd_buf[5] = mask1;	// mask1
	aegis_cmd_buf[6] = base_in1;  // base in1
	aegis_cmd_buf[7] = base_out1;  // base out1
	aegis_cmd_buf[8] = mask2;	// mask2
	aegis_cmd_buf[9] = base_in2;  // base in2
	aegis_cmd_buf[10] = base_out2;  // base out2


	/* set ILOAD2. */
	*((volatile unsigned long *)BSP_GLB_NONSECURE_GLB_ILOAD2) = 0x01;

	/* poll for OLOAD2 */
	while ( (*((volatile unsigned long *)BSP_GLB_NONSECURE_GLB_OLOAD2) & 0x01) == 0 )
		;

	/* clear interrupts. */
	/* BSP_GLB_NONSECURE_GLB_HOST_INTR_STATUS = 0 */
	*(volatile unsigned long*) (0xb030b010) = 0x00;
 	/* BSP_GLB_NONSECURE_GLB_OLOAD2 = 0 */
	*(volatile unsigned long*) (0xb030b020) = 0x00;

	/* read out results. */
	if (aegis_res_buf[5]) {
		printk(KERN_ERR CARDNAME ": Aegis command error!!!\n");
		return 1;
	}
	return 0;
}

void bcm7118_m2mdma_host_config(void)
{
	TRACE(("checking RTS settings (before): \n"));
	TRACE(("BNM_MIPS : %08lx\n", *(unsigned long *)MEMC_0_CLIENT_INFO_10_BNM_MIPS));
	TRACE(("BNM_UBUS : %08lx\n", *(unsigned long *)MEMC_0_CLIENT_INFO_11_BNM_UBUS));
	TRACE(("BNM_DMA  : %08lx\n", *(unsigned long *)MEMC_0_CLIENT_INFO_12_BNM_DMA));

	/* turn on round robin */
	*(unsigned long *)MEMC_0_CLIENT_INFO_10_BNM_MIPS |= 0x00000040;
	*(unsigned long *)MEMC_0_CLIENT_INFO_11_BNM_UBUS |= 0x00000040;
	*(unsigned long *)MEMC_0_CLIENT_INFO_12_BNM_DMA |= 0x00000040;

	TRACE(("checking RTS settings (after): \n"));
	TRACE(("BNM_MIPS : %08lx\n", *(unsigned long *)MEMC_0_CLIENT_INFO_10_BNM_MIPS));
	TRACE(("BNM_UBUS : %08lx\n", *(unsigned long *)MEMC_0_CLIENT_INFO_11_BNM_UBUS));
	TRACE(("BNM_DMA  : %08lx\n", *(unsigned long *)MEMC_0_CLIENT_INFO_12_BNM_DMA));

	TRACE(("checking request mask settings : \n"));
	TRACE(("MEMC_0_REQ_MASK_LOW : %08lx\n", *(unsigned long *)MEMC_0_REQ_MASK_LOW));
	TRACE(("MEMC_0_REQ_MASK_HIGH : %08lx\n", *(unsigned long *)MEMC_0_REQ_MASK_HIGH));
}

static int LoadAndMapBootRam(void)
{
	struct mtd_info *mtd;
	size_t retlen;

	printk(KERN_INFO CARDNAME ": Copying code from ecmboot MTD device to 0x%08x\n", bnmBootAddress);
	mtd	= get_mtd_device(NULL, 1);
	if (mtd == NULL)
	{
		printk(KERN_ERR CARDNAME ": Could not open ecmboot MTD device\n");
		return -ENODEV;
	}
	if (MTD_READ(mtd, 0, 0x2000 * 4, &retlen, (unsigned char *)bnmBootAddress) < 0)
	{
		put_mtd_device(mtd);
		printk(KERN_ERR CARDNAME ": Could not read ecmboot MTD device\n");
		return -ENODEV;
	}

	put_mtd_device(mtd);

	printk(KERN_INFO CARDNAME ": Remapping memory at 0x%08x to BNM boot address\n", bnmBootAddress);
	if (secure_BNM_remap (0xffc00000, 0x1fc00000, K1_TO_PHYS(bnmBootAddress),
		0xF0000000, NEW_PCI_7038_PHYS_MEM_WIN0_BASE, 0x00000000))
		return -ENODEV;

	return 0;
}

int bcmsubeth_bnm_boot(int reset)
{
	if (reset)
	{
		printk(KERN_INFO CARDNAME ": Asserting BNM reset\n");
		// setting master_reset_en
		*((volatile unsigned long*)SUN_TOP_CTRL_RESET_CTRL) |= 0x08;
		// Assert software reset for the BNM subsystem block
		*((volatile unsigned long*)SUN_TOP_CTRL_SW_RESET) |= 0x08000000;
	}
	if (LoadAndMapBootRam() == -ENODEV)
	{
		printk(KERN_ERR CARDNAME "Could not load and map BNM bootloader memory\n");
		return -ENODEV;
	}

	/* configure SUB endianess control */
#if defined(__MIPSEB__)
	*((volatile unsigned long*)0xfff8d050) = 0x00;
	*((volatile unsigned long*)0xfff8d054) = 0x00;
	*((volatile unsigned long*)0xfff8d058) = 0x00;
	*((volatile unsigned long*)0xfff8d05c) = 0x00;
	*((volatile unsigned long*)0xfff8d060) = 0x00;
	*((volatile unsigned long*)0xfff8d064) = 0x00;
#else
	*((volatile unsigned long*)0xfff8d050) = 0x01;
	*((volatile unsigned long*)0xfff8d054) = 0x01;
	*((volatile unsigned long*)0xfff8d058) = 0x01;
	*((volatile unsigned long*)0xfff8d05c) = 0x01;
	*((volatile unsigned long*)0xfff8d060) = 0x01;
	*((volatile unsigned long*)0xfff8d064) = 0x01;
#endif

	bcm7118_m2mdma_host_config();

	if (reset)
	{
		/* bring BNM mips out of reset. */
		TRACE(("old values %08lx\n", *((volatile unsigned long *)SUN_TOP_CTRL_SW_RESET)));
		printk(KERN_INFO CARDNAME ": Deasserting BNM reset\n");
		*((volatile unsigned long *)SUN_TOP_CTRL_SW_RESET) &= 0xe7ffffff;
	}

	return 0;
}
#endif

int __init bcmpcieth_module_init(void)
{
    int status;

#if defined(BCM7118)
	volatile unsigned long *aegis_cmd_buf;
#endif

	TRACE(("bcmpcieth_module_init\n"));
#if defined(BCM7118)
	aegis_cmd_buf = (volatile unsigned long *)AEGIS_COMMAND_BUFFER;

#if !defined(BCM7118A0)
	if( ((*((unsigned long *)0xb0404000)) & 0xffff) == 0)
	{
		printk(KERN_ERR CARDNAME "BCM7118A0 detected but not supported!\n");
		return -ENODEV;
	}
#endif

	bnmBootAddress = BNM_BOOTLOADER_RAM_ADDRESS;
	if (ubase)
	{
		bnmBootAddress = ubase | 0xa0000000;
	}
    if ((*(unsigned long *)SUN_TOP_CTRL_SW_RESET) & 0x08000000)
	{
        /* If CFE has not done it, bring BNM mips out of reset. */
        if (bcmsubeth_bnm_boot(1) < 0)
        {
            printk(KERN_ERR CARDNAME "Failed to boot 3255\n");
            return -ENODEV;
        }
	}
	else if (usize == 0)
	{
		if (LoadAndMapBootRam() == -ENODEV)
		{
			printk(KERN_ERR CARDNAME "Could not load and map BNM bootloader memory\n");
			return -ENODEV;
		}
	}
#endif

	status = bcmpcieth_net_probe();
	return status;
}

#if defined(BCM7118)
int bcmemac_get_free_txdesc( struct net_device *net )
{
	return bcmpcieth_get_free_txdesc(net);
}

struct net_device *bcmemac_get_device(void)
{
	return bcmpcieth_get_device();
}

int bcmemac_xmit_multibuf( int ch, unsigned char *hdr, int hdr_len, unsigned char *buf, int buf_len, unsigned char *tail, int tail_len, struct net_device *dev)
{
	return bcmpcieth_xmit_multibuf( ch, hdr, hdr_len, buf, buf_len, tail, tail_len, dev);
}

int bcmemac_xmit_check(struct net_device * dev)
{
	return bcmpcieth_xmit_check(dev);
}

EXPORT_SYMBOL(bcmemac_get_free_txdesc);
EXPORT_SYMBOL(bcmemac_get_device);
EXPORT_SYMBOL(bcmemac_xmit_multibuf);
EXPORT_SYMBOL(bcmemac_xmit_check);
#else
EXPORT_SYMBOL(bcmpcieth_get_free_txdesc);
EXPORT_SYMBOL(bcmpcieth_get_device);
EXPORT_SYMBOL(bcmpcieth_xmit_multibuf);
EXPORT_SYMBOL(bcmpcieth_xmit_check);
#endif

module_init(bcmpcieth_module_init);
module_exit(bcmpcieth_module_cleanup);

MODULE_LICENSE("Proprietary");

#ifdef CES2006_DEMO
struct proc_dir_entry *docsis_dir_entry;
struct proc_dir_entry *docsis_bytes_entry;

static int print_docsis_bytes(char *buf, char **start, off_t off,
	int bufsize, int *eof, void *yourhandle)
{
	int len;

	GPIO_SET_LO(GPIO47);
	len = sprintf(buf, "%ld\n", pDevContext->stats.rx_bytes);
	GPIO_SET_HI(GPIO47);

	return len;
}

int initcode( void )
{
	docsis_dir_entry = proc_mkdir("docsis", &proc_root);
	if (!docsis_dir_entry)
		return -1;

	docsis_bytes_entry = create_proc_entry("bytes", 0500 /*mode*/, docsis_dir_entry);
	if (!docsis_bytes_entry)
		return -1;

	docsis_bytes_entry->read_proc = print_docsis_bytes;
	docsis_bytes_entry->data = (void*)0; /* if needed */

	return 0;
}

void uninitcode( void )
{
	remove_proc_entry("bytes", docsis_bytes_entry);
	remove_proc_entry("docsis", docsis_dir_entry);
}
#endif

/***********************************************************************/
/*   pci_mbox_tasklet() -                                              */
/*                                                                     */
/*   Process PCI MailBox receive interrupt                             */
/*                                                                     */
/***********************************************************************/
static void pci_mbox_tasklet ( unsigned long not_used )
{
	BcmEnet_devctrl *pDevCtrl = pDevContext;
	unsigned long MailBoxVal;

#if defined(BCM7118)
#if defined(LED_SUPPORT)
	uint32 temp;
#endif
	if(MboxIntStatus & GISB_MBOX1_IRQ_MASK)
	{
#if defined(LED_SUPPORT)
		temp = *(uint32 *)GIO_DATA_HI;
		*(uint32 *)GIO_DATA_HI = temp & 0xffffbfff; /* GPIO_46 */
#endif
		MailBoxVal = pDevCtrl->nsGisbRegs->mBox1;
#else
	if(MboxIntStatus & PCI_MBOX1_RCV_INTSTATUS_BIT)
	{
		MailBoxVal = pDevCtrl->mpiRegs->MailBox1;
#endif
		switch(MailBoxVal & 0xff000000)
		{
        case MBOX_BOOT_BULKDMA:
		    printk(KERN_INFO CARDNAME " MBOXIST: MBOX_BOOT_BULKDMA!\n");
#if !defined(BCM7118)
			if ((pDevCtrl->PciResetDetected) || (pDevCtrl->PciHostDmaReady == FALSE)) {
				// Reinitialize protected registers if 3255 is running in secure mode.
				// You may wonder why this is here? It is needed in the cases wher the 3255
				// is reset in secure mode and the following registered are protected.
				pDevCtrl->txDma->DescriptorAddress = pDevCtrl->PciTxSetupPtr;
				pDevCtrl->rxDma->DescriptorAddress = pDevCtrl->PciRxSetupPtr;
				pDevCtrl->rxDma->IntMask = DS_PCI_DMA_IRQ_MASK;
				pDevCtrl->rxDma->Control = PCI_DMA_ENABLE;
				pDevCtrl->PciResetDetected = FALSE;
			}

			/* send an acknowledgement to the PCI Client */
			pDevCtrl->mpiRegs->MailBox1 = MBOX_BOOT_BULKDMA;
#else
			pDevCtrl->nsGisbRegs->mBox1	= MBOX_BOOT_BULKDMA | ((K1_TO_PHYS(imagePointer))>>16 & 0x0000FFFF);

			*(uint32 *)0xfffb0c00 = 0x1;
			*(uint32 *)0xfffb0d00 = 0x1;

			/* enable DMA from the PCI side */
			pDevCtrl->rxDma->Control = PCI_DMA_ENABLE;
			pDevCtrl->rxDma->IntMask = DMA_DONE|DMA_NO_DESC;

			/* Disable the reboot timer	 */
			pDevCtrl->PciHostDmaReady = TRUE;
#endif
			break;

		case MBOX_BULKDMA_P_NVOL:
		case MBOX_BULKDMA_D_NVOL:
		case MBOX_BULKDMA_IMG_HDR:
		case MBOX_BULKDMA_IMG:
        case MBOX_BULKDMA_IMG_FULL:
			printk(KERN_INFO CARDNAME " MBOXIST: %08lx\n", MailBoxVal);
			pDevCtrl->mailbox1 = MailBoxVal;
			savedmailbox1 = pDevCtrl->mailbox1;
			break;

        case MBOX_REBOOTING:
            printk(KERN_INFO CARDNAME " MBOXIST: MBOX_REBOOTING!\n");
            pDevCtrl->PciHostDmaReady = FALSE;
#if defined(BCM7118)
			// if running on 7118, we need to actually reset the BNM processor
			bcmsubeth_bnm_boot(usize != 0);
#endif
            break;


		case MBOX_DMA_READY:
			printk(KERN_INFO CARDNAME ": MBOXIST: MBOX_DMA_READY!\n");
#if defined(BCM7118)
			/* send an acknowledgement to the PCI Client */
			pDevCtrl->nsGisbRegs->mBox1 = MBOX_DMA_READY;
			ready_cnt++;
			if(ready_cnt == 2)
			{
#if defined(FFT_SUPPORT)
				*(uint32 *)SUN_TOP_CTRL_TEST_PORT_CTRL = 0x9; /* ds0 */
				printk("*(uint32 *)SUN_TOP_CTRL_TEST_PORT_CTRL = %08lx\n", *(uint32 *)SUN_TOP_CTRL_TEST_PORT_CTRL);
#endif
#if defined(LED_SUPPORT)
				/* start flashing LED */
				pDevCtrl->reset_timer.expires = jiffies + TIMEOUT_1S;
				add_timer(&pDevCtrl->reset_timer);
#endif
			}
#else
            if ((pDevCtrl->PciResetDetected) || (pDevCtrl->PciHostDmaReady == FALSE)) {
                // Reinitialize protected registers if 3255 is running in secure mode.
                // You may wonder why this is here? It is need in the cases wher the 3255
                // is reset in secure mode and the following registered are protected.
                pDevCtrl->txDma->DescriptorAddress = pDevCtrl->PciTxSetupPtr;
                pDevCtrl->rxDma->DescriptorAddress = pDevCtrl->PciRxSetupPtr;
                pDevCtrl->rxDma->IntMask = DS_PCI_DMA_IRQ_MASK;
                pDevCtrl->rxDma->Control = PCI_DMA_ENABLE;
                pDevCtrl->PciResetDetected = FALSE;
            }
#if (PASS_THROUGH)
            /* Inform eCM regarding tagging */
            if (!tagging_disable) 
            {
                pDevCtrl->mpiRegs->MailBox0 |= 1; // Tell the ECM that Ethernet is in passthrough
            }
#endif

			/* send an acknowledgement to the PCI Client */
			pDevCtrl->mpiRegs->MailBox1 = MBOX_DMA_READY;
#endif
			/* set the DMA-ready flag */
			pDevCtrl->PciHostDmaReady = TRUE;
 

			break;
#if defined(BCM7118)
#if defined(LED_SUPPORT)
        case MBOX_CM_READY:
            /* cable modem is ready */
			cm_ready = 1;
			temp = *(uint32 *)GIO_DATA_HI;
			*(uint32 *)GIO_DATA_HI = temp & 0xffffdfff; /* GPIO_45 */
            break;
		case MBOX_CM_NOT_READY:
			/* cable modem is ready */
			cm_ready = 0;
			/* start flashing LED */
			pDevCtrl->reset_timer.expires = jiffies + TIMEOUT_1S;
			add_timer(&pDevCtrl->reset_timer);
			break;
#endif
		case MBOX_7118_REMAP_MEM:
			secure_BNM_remap (((usize - 1) ^ 0xffffffff), 0, ubase,
				0xF0000000, NEW_PCI_7038_PHYS_MEM_WIN0_BASE, 0x00000000);
			pDevCtrl->nsGisbRegs->mBox1 = MBOX_7118_REMAP_MEM;
			break;
		case MBOX_7118_MEM_SIZE:
			pDevCtrl->nsGisbRegs->mBox1 = MBOX_7118_MEM_SIZE | (usize >> 16);
			break;
#else
        case MBOX_WAKUP_CALL:
            printk(KERN_INFO CARDNAME " MBOXIST: MBOX_WAKEUP_CALL!\n");
            /* PCI Client non-vol driver needs to know if we are ready */
            /* send an acknowledgement to the PCI Client */
			pDevCtrl->mpiRegs->MailBox1 = (MBOX_WAKUP_CALL | MBOX_RESPONSE_CODE);
            break;

        case RESERVED_FOR_REBOOT:
            /* send by 3255 internal secure bootrom */
            /* Wake up hostboot to push in secure bootloader via IOCTL */
            if (pDevCtrl->mailbox1)
                printk(KERN_ERR CARDNAME ": MBOXIST: Mailbox1 overun!\n");
            pDevCtrl->mailbox1 = pDevCtrl->mpiRegs->MailBox1;
            break;
#endif
		default:
#if defined(BCM7118)
			pDevCtrl->nsGisbRegs->mBox1 = MailBoxVal;
#else
			pDevCtrl->mpiRegs->MailBox1 = MailBoxVal;
#endif

		   printk(KERN_ERR CARDNAME ": MBOXIST: Unknown mbox1 command (%08lx)!\n", MailBoxVal);
		   break;
		}

#if defined(LED_SUPPORT)
		temp = *(uint32 *)GIO_DATA_HI;
		*(uint32 *)GIO_DATA_HI = temp | 0x00004000; /* GPIO_46 */
#endif
	}

	/* Enable mailbox receiving interrupts */
#if defined(BCM7118)
    pDevCtrl->nsGisbRegs->hostIrqMask |= GISB_MBOX1_IRQ_MASK;
#else
    pDevCtrl->mpiRegs->PciIntStat |= PCI_MBOX1_RCV_INTENB_BIT;
#endif
}
