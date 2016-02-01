/****************************************************************************
*
*  Copyright (c) 2010-2012 Broadcom Corporation
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
*  Filename: bcmvenet.c
*
****************************************************************************
* Description: This is the network interface driver for the virtual Ethernet
*              interface between 7125 BNM and HOST.
*
* Updates    : 08-28-2009  wfeng.  Created.
*              12-20-2010  jcyr. Added support for second ethernet interface
*                          to ECM Unimac.
*
****************************************************************************/

// --------------------------------------------------------------------------
//      Driver includes, structures and defines
// --------------------------------------------------------------------------
#define CARDNAME    "bcmvenet"
#define VERSION     "1.21"
#define VER_STR     "v" VERSION " " __DATE__ " " __TIME__

#if defined(CONFIG_MODVERSIONS) && ! defined(MODVERSIONS)
#include <config/modversions.h>
#define MODVERSIONS
#endif

#include <linux/version.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/etherdevice.h>
#include <linux/proc_fs.h>
#include <asm/brcmstb/brcmstb.h>

#include "bcmvenet.h"

#define ENET_MAX_MTU_SIZE       1536
/* Body(1500) + EH_SIZE(14) + VLANTAG(4) + BRCMTAG(6) + FCS(4) = 1528.  1536 is multiple of 256 bytes */

//#define DUMP_TRACE
#if defined(DUMP_TRACE)
#define TRACE(x) printk x
#else
#define TRACE(x)
#endif

//********************** Local Types and variables *****************************************

static unsigned short largeflashpartition = 0;
module_param(largeflashpartition, ushort, S_IRUGO);

// SUB: SCB Bridge DMA Registers
typedef struct SUBDMARegisters {
    u32 dmaCtrl;
    u32 dmaStatus;
    u32 dmaBuffAlloc;
    u32 dmaBuffReturn;
    u32 dmaRxfifoThres;
    u32 dmaTxfifoThres;
    u32 dmaReserved0;
    u32 dmaRxLo;
    u32 dmaRxHi;
    u32 dmaTxLo;
    u32 dmaTxHi;
    u32 dmaTxBdCnt;
    u32 dmaTxMissedBdCnt;
} SUBDMARegisters;

// SUB: SCB BNM Control Registers
typedef struct SUBBNMCTLRegisters {
    u32 mbxTobnm0;
    u32 mbxTobnm1;
    u32 mbxTobnm2;
    u32 mbxTobnm3;
    u32 mbxFrmbnm0;
    u32 mbxFrmbnm1;
    u32 mbxFrmbnm2;
    u32 mbxFrmbnm3;
    u32 hostIrqMask;
    u32 hostIrqStatus;
    u32 hostBnmCtl;
    u32 hostBdStatus;
    u32 hostIrqMask2;
    u32 hostIrqStatus2;
} SUBBNMCTLRegisters;

typedef struct {
    struct list_head list;
    struct sk_buff skb;
} venet_sk_buff_list;

typedef struct {
    struct list_head head;
    spinlock_t qlock;
} venet_sk_buff_queue;

#define SUB_SCB_RT_TX_CTL 0xb4000150
#define SUB_SCB_RT_RX_CTL 0xb4000154
#define HIF_CPU_INTR1_INTR_W1_STATUS 0xb0441404
#define HIF_CPU_INTR1_INTR_W1_MASK_STATUS 0xb0441410
#define SUB_BNMCTL_HOSTIRQMASK 0xb4000020
#define SUB_BNMCTL_HOSTIRQSTATUS 0xb4000024

#define SGISB_BNM_DDR_DEST_BASE 0x14003010
#define SGISB_BNM_DDR_ORIG_BASE 0x14003014
#define SGISB_BNM_DDR_WMASK 0x14003018
#define SGISB_BNM_DATA_DEST_BASE 0x14003020
#define SGISB_BNM_DATA_ORIG_BASE 0x14003024
#define SGISB_BNM_DATA_WMASK 0x14003028
#define SGISB_BNM_LMB_ORIG_BASE 0x1400302c
#define SGISB_BNM_BOOT_DEST_BASE 0x14003030
#define SGISB_BNM_BOOT_ORIG_BASE 0x14003034
#define SGISB_BNM_BOOT_WMASK 0x14003038
#define SGISB_BNM_UBUS_BASE 0x14003040
#define SGISB_BNM_UBUS_MASK 0x14003044
#define SGISB_BNM_GISB_UBUS_BASE 0x14003050
#define SGISB_BNM_GISB_UBUS_END 0x14003054
#define SGISB_BNM_SCB_BUFFER_BASE 0x14003058
#define SGISB_BNM_SCB_BUFFER_MASK 0x1400305c
#define SGISB_BNM_MIPS_CODE_WIN_BASE 0x14003060
#define SGISB_BNM_MIPS_CODE_WIN_MASK 0x14003064
#define SGISB_BNM_MIPS_DATA_WIN_BASE 0x14003068
#define SGISB_BNM_MIPS_DATA_WIN_MASK 0x1400306c

#define SUB_UBUS_BUFFERBASE 0xb4880010
#define SUB_BNM_SCB_BUFFER_BASE 0xb4880998
#define SUB_BNM_SCB_BUFFER_MASK 0xb488099c
#define ECRAM_STO_RESERVED_DEFAULT (2 * 1024 * 1024)
#define ECRAM_STO_RESERVED_LARGE (4 * 1024 * 1024)
#define FPM_ALLOC_DEALLOC 0xb2010200

/*
BD[63:32]   = Buffer Address in SCB domain
BD[31:28]   = reserved/unused
BD[27:16]   = Buffer Length in bytes
BD[15:00]   = Buffer Status
*/
typedef struct BNM_BD {
    unsigned int ptr;
    unsigned int ctl;
} BNM_BD;

#define MBOX_MSG_MASK           0xe0000000
#define MBOX_SET_PROMISC        0x40000000
#define MBOX_RESET_DOCSIS       0x60000000
#define MBOX_DMA_READY 			0x80000000
#define MBOX_CONFIG_COMPLETE	0xc0000000
#define MBOX_REBOOTING          0xe0000000
#define MBOX_SHM_ADDR           0xa0000000
#define TIMEOUT_RESET           (HZ * 2)
#define WATCHDOG_INTERVAL       (HZ * 1)

#define BD_THRESH_IRQ (1 << 12)
#define BD_TIMEOUT_IRQ (1 << 11)
#define MASTER_IRQ_EN (1 << 31)
#define MBOX0_IRQ     (1 << 0)

#define VENET_RX_BUF_SIZE 2048
#define VENET_RX_BUF_NUM 256

#define VENET_CACHE_LINE_SIZE 32
#define FPM_TOKEN_MASK        0x7ffff000
#define FPM_TOKEN_VALID_MASK  0x80000000
#define FPM_TOKEN_INDEX_SHIFT 12
#define FPM_TOKEN_SIZE 0x800

#ifndef BRCM_L1_IRQ
#define BRCM_L1_IRQ(word, intr) \
    (BRCM_INTR_##word##_BASE + \
    BCHP_HIF_CPU_INTR1_INTR_##word##_STATUS_##intr##_CPU_INTR_SHIFT)
#endif

#ifdef BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_BNM_IRQ_CPU_INTR_SHIFT
#define BRCM_IRQ_BNM   	    BRCM_L1_IRQ(W1, BNM_IRQ)
#define BRCM_IRQ1_BNM  	    BRCM_L1_IRQ(W1, BNM_IRQ1)
#else
#define BRCM_IRQ_BNM   	    BRCM_L1_IRQ(W1, BNM)
#define BRCM_IRQ1_BNM  	    BRCM_IRQ_BNM
#endif
#ifndef BRCM_IRQ_UPG
#define BRCM_IRQ_UPG		BRCM_L1_IRQ(W0, UPG)
#endif

#define skb_dataref(x)   (&skb_shinfo(x)->dataref)

// Define the interrupt id as any non 0 since it is not used
#define INTERRUPT_PARAMETER ((void *)10)

// --------------------------------------------------------------------------
//      Driver parameter.
// --------------------------------------------------------------------------
static ushort eth_bridge = 0;
module_param(eth_bridge, ushort, S_IRUGO);

// --------------------------------------------------------------------------
//      External, indirect entry points.
// --------------------------------------------------------------------------
static int bcmvenet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
static int bcmvenet_bridge_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
// --------------------------------------------------------------------------
//      Called for "ifconfig ethX up" & "down"
// --------------------------------------------------------------------------
static int bcmvenet_open(struct net_device *dev);
static int bcmvenet_close(struct net_device *dev);
// --------------------------------------------------------------------------
//      Watchdog timeout
// --------------------------------------------------------------------------
static void bcmvenet_timeout(struct net_device *dev);
// --------------------------------------------------------------------------
//      Packet transmission.
// --------------------------------------------------------------------------
static int bcmvenet_host_xmit(struct sk_buff *skb, struct net_device *dev);
static int bcmvenet_bridge_xmit(struct sk_buff *skb, struct net_device *dev);
// --------------------------------------------------------------------------
//      Set address filtering mode
// --------------------------------------------------------------------------
static void bcmvenet_set_multicast_list(struct net_device *dev);
// --------------------------------------------------------------------------
//      Set the hardware MAC address.
// --------------------------------------------------------------------------
static int bcmvenet_set_mac_addr(struct net_device *dev, void *p);
// --------------------------------------------------------------------------
//      Interrupt routine, for all interrupts except ring buffer interrupts
// --------------------------------------------------------------------------
static irqreturn_t bcmvenet_isr(int irq, void *dev_id);
// --------------------------------------------------------------------------
//      Interrupt routine, for rx data interrupts only
// --------------------------------------------------------------------------
static irqreturn_t bcmvenet_rx_isr(int irq, void *dev_id);
// --------------------------------------------------------------------------
//      dev->poll() method
// --------------------------------------------------------------------------
static int bcmvenet_poll(struct napi_struct *napi, int budget);
// --------------------------------------------------------------------------
//      Process recived packet for descriptor based DMA
// --------------------------------------------------------------------------
static unsigned int bcmvenet_rx(unsigned int budget);
// --------------------------------------------------------------------------
//      Internal routines
// --------------------------------------------------------------------------
/* Allocate and initialize tx/rx buffer descriptor pools */
static int bcmvenet_init_dev(void);
static void bcmvenet_uninit_dev(void);
/* Initialize DMA control register */
static void init_subdma(void);
static void mbox_tasklet(unsigned long arg);
static void bcmvenet_rtimer_handler(unsigned long arg);
static void bcmvenet_bnm_watchdog_handler(unsigned long arg);

// --------------------------------------------------------------------------
//      Miscellaneous global variables
// --------------------------------------------------------------------------
struct napi_struct g_napi;
static unsigned int g_revid;
static unsigned int g_openDeviceCount = 0;
static volatile unsigned int *g_bnmctl = (unsigned int *)0xb4000028;
static struct net_device *g_devices[2]; /* ptr to net_device g_devices */
#define ECM_DEVICE_IX 0
#define ETH_DEVICE_IX 1
static spinlock_t g_dma_lock;				/* Serializing g_dma_lock */
static unsigned long g_shm_addr;             /* SUB register start address. */
static unsigned long g_mem_base;             /* SUB register start address. */
//static unsigned long g_base_addr;            /* SUB register start address. */
static unsigned long g_fpm_base;             /* SUB register start address. */
static unsigned long g_fpm_end;              /* SUB register start address. */
static unsigned long g_data_base;            /* SUB register start address. */
static volatile SUBBNMCTLRegisters *g_subbnmctl;/* SUB BNM control register block base address */
static volatile unsigned int *g_bdgctl;      /* SUB Bridge control register block base address */
static volatile SUBDMARegisters *g_dmactl;   /* SUB DMA register block base address */
static int g_irq;                            /* BNM HOST IRQ */
static int g_irq_rx;                         /* BNM HOST IRQ */
static int	g_irq_stat;                       /* Software copy of irq status, for botom half processing */
static struct tasklet_struct g_mtask;        /* Task to process mbox messages */
static int g_link;                           /* Link status */
static struct timer_list g_rtimer;           /* Timer to clean up */
static struct timer_list g_watchdogTimer;    /* Timer to poll BNM watchdog */

static venet_sk_buff_queue g_skb_list_free; /* Free SKB list */
static venet_sk_buff_queue g_skb_list_used; /* Used SKB list */
static bool g_isAxOrBxRevision;             /* SUB DMA Magic flag */

static atomic_t g_rx_cnt;                /* Packets successfully received */
static atomic_t g_rx_err_cnt;            /* Packets dropped because no skb is available */
static atomic_t g_tx_cnt;                /* Packets successfully transmitted */
static atomic_t g_tx_err_cnt;            /* Packets not transmitted because no BNM buffer is available */

static volatile unsigned int *g_watchdogdefcount = (unsigned int *)0xB4E000DC;

static unsigned int g_geometry;
static volatile int g_dma_ready_indication = 0;
static bool g_promiscuous = false;

static inline void set_txdma_bd(BNM_BD *bd) {
    unsigned long flags;
    spin_lock_irqsave(&g_dma_lock, flags);
    g_dmactl->dmaTxLo = bd->ctl;
    g_dmactl->dmaTxHi = bd->ptr;
    spin_unlock_irqrestore(&g_dma_lock, flags);
}

// --------------------------------------------------------------------------
//      Start of low level functions
// --------------------------------------------------------------------------
static inline void bcmvenet_buf_return(unsigned int addr) {

    BNM_BD bd;

    /* Flush and invalidate memory */
    dma_map_single(&g_devices[ECM_DEVICE_IX]->dev, (void *)((addr + g_mem_base) | 0x80000000), FPM_TOKEN_SIZE, DMA_BIDIRECTIONAL);
    if (!g_isAxOrBxRevision) {
        addr |= g_data_base;
        g_dmactl->dmaBuffReturn = addr;
        return;
    }

    /* Construct a BD */
    bd.ctl = (16 << 16);
    bd.ptr = addr;
    /* Write to TX FIFO */
    set_txdma_bd(&bd);
}

static inline void bcmvenet_skb_enqueue_used(struct sk_buff *skb) {

    unsigned long flags;
    venet_sk_buff_list *n = container_of(skb, venet_sk_buff_list, skb);

    spin_lock_irqsave(&g_skb_list_used.qlock, flags);
    list_add_tail(&n->list, &g_skb_list_used.head);
    spin_unlock_irqrestore(&g_skb_list_used.qlock, flags);
}

static inline void bcmvenet_skb_enqueue_free(struct sk_buff *skb) {

    unsigned long flags;
    venet_sk_buff_list *n = container_of(skb, venet_sk_buff_list, skb);

    spin_lock_irqsave(&g_skb_list_free.qlock, flags);
    list_add_tail(&n->list, &g_skb_list_free.head);
    spin_unlock_irqrestore(&g_skb_list_free.qlock, flags);
}

static inline struct sk_buff *bcmvenet_skb_dequeue_free(void) {

    unsigned long flags;
    venet_sk_buff_list *n;
    struct sk_buff *skb = NULL;

    spin_lock_irqsave(&g_skb_list_free.qlock, flags);
    if (!list_empty(&g_skb_list_free.head)) {
        n = list_first_entry(&g_skb_list_free.head, venet_sk_buff_list, list);
        list_del(g_skb_list_free.head.next);
        skb = &n->skb;
    }
    spin_unlock_irqrestore(&g_skb_list_free.qlock, flags);

    return skb;
}

static inline struct sk_buff *bcmvenet_skb_dequeue_used(void) {

    unsigned long flags;
    venet_sk_buff_list *n;
    struct sk_buff *skb = NULL;

    spin_lock_irqsave(&g_skb_list_used.qlock, flags);
    if (!list_empty(&g_skb_list_used.head)) {
        n = list_first_entry(&g_skb_list_used.head, venet_sk_buff_list, list);
        list_del(g_skb_list_used.head.next);
        skb = &n->skb;
    }
    spin_unlock_irqrestore(&g_skb_list_used.qlock, flags);

    return skb;
}

static inline void bcmvenet_reclaim_rx_skb(void) {

    unsigned long flags;
    struct list_head *pos, *next;
    struct sk_buff *skb;
    venet_sk_buff_queue *q = &g_skb_list_used;

    spin_lock_irqsave(&q->qlock, flags);
    list_for_each_safe(pos, next, &q->head) {
        unsigned int bd_lo;
        venet_sk_buff_list *skb_node;
        skb_node = list_entry(pos, venet_sk_buff_list, list);
        skb = &skb_node->skb;

        if ((skb_shared(skb) == 0) && atomic_read(skb_dataref(skb)) <= SKB_FCLONE_ORIG) {
            /* Remove it from used queue */
            list_del(pos);

            /* Free FPM buffer */
            bd_lo = (unsigned int)virt_to_phys(skb->head) - g_mem_base;
            //printk("Freeing %p : bd_lo = %08x len = %d\n", skb->head, bd_lo, skb->len);
            bcmvenet_buf_return(bd_lo);

            /* Append it to the free queue */
            bcmvenet_skb_enqueue_free(skb);
        }
    }
    spin_unlock_irqrestore(&q->qlock, flags);
}

static inline void bcmvenet_attach_skb(struct sk_buff *skb, unsigned char *buf) {

    struct skb_shared_info *si;

    /* Reset the structure */
    memset(skb, 0, offsetof(struct sk_buff, tail));
    skb->truesize = VENET_RX_BUF_SIZE + sizeof(struct sk_buff);
    /* Set header reference count to 2 */
    atomic_set(&skb->users, 2);
    skb->head = buf;
    skb->data = buf;
    skb_reset_tail_pointer(skb);
    skb->end =
        skb->tail + VENET_RX_BUF_SIZE -
        ((sizeof(struct skb_shared_info) + (VENET_CACHE_LINE_SIZE - 1)) & (~(VENET_CACHE_LINE_SIZE - 1)));
    skb->cloned = SKB_FCLONE_UNAVAILABLE;

    /* Configure shinfo */
    si = skb_shinfo(skb);
    /* Set data reference count to 1 */
    atomic_set(&si->dataref, SKB_FCLONE_ORIG);
    si->nr_frags = 0;
    si->gso_size = 0;
    si->gso_segs = 0;
    si->gso_type = 0;
    si->ip6_frag_id = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
    si->tx_flags.flags = 0;
#else
    si->tx_flags = 0;
#endif
    si->frag_list = NULL;
    memset(&si->hwtstamps, 0, sizeof(si->hwtstamps));
}

#define JTAG_OTP_GENERAL_CTRL_0 0xb0360100
#define JTAG_OTP_GENERAL_CTRL_1 0xb0360104
#define JTAG_OTP_GENERAL_STATUS_0 0xb0360110
#define JTAG_OTP_GENERAL_STATUS_1 0xb0360114

static unsigned int read_chip_geometry_otp_bit(void) {

    unsigned int geometry = 0;
    unsigned int timeout = 50;

    // Determine if this is a 60 or 65nm part
    *((volatile unsigned int *)JTAG_OTP_GENERAL_CTRL_0) = 0; // make sure jtag stopped
    // make sure otp init wait has completed and that the block is clocked
    if (*((volatile unsigned int *)JTAG_OTP_GENERAL_STATUS_1) & 0x80) {
        *((volatile unsigned int *)JTAG_OTP_GENERAL_CTRL_1) = (0x65 << 16) | 1; // set cpu mode to jtag, read address to 0x65
        *((volatile unsigned int *)JTAG_OTP_GENERAL_CTRL_0) = 0x00000001; // start read
        // wait for operation to complete
        while ((*((volatile unsigned int *)JTAG_OTP_GENERAL_STATUS_1) & 1) == 0) {
            if (--timeout == 0) {
                break;
            }
        }
        geometry = *((volatile unsigned int *)JTAG_OTP_GENERAL_STATUS_0); // get the otp data
        *((volatile unsigned int *)JTAG_OTP_GENERAL_CTRL_1) = 0; // restore cpu mode
        geometry = (geometry >> 5) & 1;
    }
    if (timeout == 0) {
        return 0;
    }
    return geometry;
}

static void reset_bnm_mips(bool soft) {

    unsigned int *bnmDestBase = (unsigned int *)0xb4880950;
    unsigned int *bnmDestMask = (unsigned int *)0xb4880958;
    unsigned int memSize = ~(*bnmDestMask | 0xe0000000) + 1;
    unsigned int memBase = (*bnmDestBase & 0xfffffff8) | 0xA0000000;

    g_subbnmctl->mbxTobnm3 = g_geometry;

    if (soft) {
        // We do the following instead of storing a full word because we don't
        // know whether we are running big or little endian and we want to
        // make sure the big endian ECM MIPS sees the right value.
        *((unsigned char *)(memBase + memSize - 4)) = 0x55;
        *((unsigned char *)(memBase + memSize - 3)) = 0xaa;
        *((unsigned char *)(memBase + memSize - 2)) = 0x55;
        *((unsigned char *)(memBase + memSize - 1)) = 0xaa;
    }

    *g_bnmctl &= ~3;  // Reset the BNM UBUS and MIPS
    *g_bnmctl |= 1;   // Release the UBUS
    *g_bnmctl |= 2;   // Release the MIPS
}

static void set_carrier(void) {
    if (g_link) {
        netif_carrier_on(g_devices[ECM_DEVICE_IX]);
        if (eth_bridge) {
            netif_carrier_on(g_devices[ETH_DEVICE_IX]);
        }
    } else {
        netif_carrier_off(g_devices[ECM_DEVICE_IX]);
        if (eth_bridge) {
            netif_carrier_off(g_devices[ETH_DEVICE_IX]);
        }
    }
}

static inline u32 swap32(u32 x) {
#if defined(__BIG_ENDIAN)
    return ((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | ((x >> 24) & 0xff);
#else
    return x;
#endif
}

/***********************************************************************/
/*   mbox_tasklet() -                                              */
/*                                                                     */
/*   Process MailBox receive interrupt                             */
/*                                                                     */
/***********************************************************************/
static void mbox_tasklet(unsigned long arg) {

    unsigned long MailBoxVal;

    /* Check interesting interrupts */
    if (g_irq_stat & MBOX0_IRQ) {
        /* Read the mbox message */
        MailBoxVal = g_subbnmctl->mbxFrmbnm0;
        switch (MailBoxVal & MBOX_MSG_MASK) {
        case MBOX_REBOOTING:
            printk(KERN_INFO "bcm0: MBOX_REBOOTING!\n");
            if (g_link) {
                g_link = 0;
                set_carrier();
                /* start the reset timer */
                g_rtimer.expires = jiffies + TIMEOUT_RESET;
                add_timer(&g_rtimer);
            }
            break;

        case MBOX_SHM_ADDR:
            printk(KERN_INFO "bcm0: MBOX_SHM_ADDR!\n");
            g_shm_addr = ((~MBOX_MSG_MASK) & MailBoxVal);
            TRACE(("g_shm_addr = %0x\n", (unsigned int)g_shm_addr));
            break;

        case MBOX_DMA_READY:
            printk(KERN_INFO "bcm0: MBOX_DMA_READY!\n");
            if (g_isAxOrBxRevision) {
                g_mem_base = ((~MBOX_MSG_MASK) & MailBoxVal);
            } else {
                g_mem_base = 0;
                g_fpm_base = ((~MBOX_MSG_MASK) & MailBoxVal);
                g_fpm_base += *(volatile unsigned int *)SUB_BNM_SCB_BUFFER_BASE;
                g_data_base = *(volatile unsigned int *)SUB_UBUS_BUFFERBASE;
                g_data_base &= (~0x1fffffff);
            }
            TRACE(("g_mem_base = %0x\n", (unsigned int)g_mem_base));
            TRACE(("&g_dmactl->dmaBuffReturn = %0x\n", (unsigned int) & (g_dmactl->dmaBuffReturn)));
            /* set the link up flag */
            g_link = 1;
            /* Enable data interrupt */
            if (!g_isAxOrBxRevision) {
                g_subbnmctl->hostIrqMask2 |= (BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
                // we shouldn't need to do this but it seems that any pending interrupts
                // will not trigger the ISR when the mask is enabled. So clear them now
                // and we'll catch the next one.
                g_subbnmctl->hostIrqStatus2 = (BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
            } else {
                g_subbnmctl->hostIrqMask |= (BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
            }
            set_carrier();
            MailBoxVal &= MBOX_MSG_MASK;
            if (eth_bridge) {
                MailBoxVal |= 1; // Tell the ECM that Ethernet is in passthrough
                MailBoxVal |= g_promiscuous ? 2 : 0; // Tell the ECM that Ethernet is in promiscuous mode
                // Pass the MAC address to the ECM
                g_subbnmctl->mbxTobnm1 = swap32(*((u32 *)g_devices[1]->dev_addr));
                g_subbnmctl->mbxTobnm2 = swap32(*((u32 *)&g_devices[1]->dev_addr[4]));
            }
            g_dma_ready_indication = 1;

            break;

        case MBOX_RESET_DOCSIS:
            /* start the reset timer */
            printk(KERN_INFO "bcm0: MBOX_RESET_DOCSIS! Reason %d\n", (int)(MailBoxVal & 15));
            if (g_link) {
                g_link = 0;
                set_carrier();
            }
            reset_bnm_mips(true);
            break;


        default:
            printk(KERN_ERR "bcm0: MBOXIST: Unknown mbox1 command (%08lx)!\n", MailBoxVal);
            break;
        }
        /* Acknowledge the message */
        g_subbnmctl->mbxTobnm0 = MailBoxVal;

        /* Enable mailbox receiving interrupts */
        g_subbnmctl->hostIrqMask |= MBOX0_IRQ;
    }

}

/* --------------------------------------------------------------------------
    Name: bcmvenet_rtimer_handler
 Purpose: clean up used BNM buffers
-------------------------------------------------------------------------- */
static void bcmvenet_rtimer_handler(unsigned long arg) {

    unsigned long flags;
    struct list_head *pos, *next;
    struct sk_buff *skb;
    venet_sk_buff_queue *q = &g_skb_list_used;
    unsigned int *ptr = (unsigned int *)(g_shm_addr | 0xA0000000);

    if ((unsigned int)ptr == 0xA0000000) {
        return;
    }

    spin_lock_irqsave(&q->qlock, flags);
    /* Reset the shared memory */
    memset(ptr, 0, 0x1000);

    list_for_each_safe(pos, next, &q->head) {
        venet_sk_buff_list *skb_node;
        skb_node = list_entry(pos, venet_sk_buff_list, list);
        skb = &skb_node->skb;

        if ((skb_shared(skb) == 0) && atomic_read(skb_dataref(skb)) <= SKB_FCLONE_ORIG) {
            /* Remove it from used queue */
            list_del(pos);

            TRACE(("%s : %p(free)\n", __FUNCTION__, skb->head));

            /* Append it to the free queue */
            bcmvenet_skb_enqueue_free(skb);
        } else {
            /* Write pointers to the shared memory */
            TRACE(("%s : %p(used)\n", __FUNCTION__, skb->head));
            *ptr = htonl((unsigned int)skb->head);
            ptr++;
        }
    }
    spin_unlock_irqrestore(&q->qlock, flags);
}

static void bcmvenet_bnm_watchdog_handler(unsigned long arg) {

    volatile unsigned int *watchdogcount = (unsigned int *)0xB4E000E0;

    if ((*g_watchdogdefcount != 0) && (*watchdogcount < (*g_watchdogdefcount >> 2))) {
        if (g_link) {
            g_link = 0;
            set_carrier();
        }
        printk(KERN_INFO "bcm0: BNM WATCHDOG RESET\n");
        reset_bnm_mips(true);
    }
    if (g_link) {
        napi_schedule(&g_napi);
    }

    g_watchdogTimer.expires = jiffies + WATCHDOG_INTERVAL;
    add_timer(&g_watchdogTimer);
}

/* --------------------------------------------------------------------------
    Name: bcmvenet_open
 Purpose: Open and Initialize the SUB DMA on the chip
-------------------------------------------------------------------------- */
static int bcmvenet_open(struct net_device *dev) {
    TRACE(("%s %d: bcmvenet_open\n", dev->name, dev->dev_id));

    if (g_openDeviceCount == 0) {
        TRACE(("enabling napi: bcmvenet_open\n"));
        napi_enable(&g_napi);
    }

    /* Start the network engine */
    TRACE(("starting q: bcmvenet_open\n"));
    netif_start_queue(dev);

    if (g_openDeviceCount == 0) {
        /* Enable mbox and SCB bridge interrupts */
        if (!g_isAxOrBxRevision) {
            g_subbnmctl->hostIrqMask |= (MASTER_IRQ_EN | MBOX0_IRQ);
            g_subbnmctl->hostIrqMask2 |= (BD_TIMEOUT_IRQ | BD_THRESH_IRQ | MASTER_IRQ_EN);
        } else {
            g_subbnmctl->hostIrqMask |= (BD_TIMEOUT_IRQ | BD_THRESH_IRQ | MASTER_IRQ_EN | MBOX0_IRQ);
        }
        /* Enable RX/TX DMA */
        g_subbnmctl->hostBnmCtl |= 0x3;
    }

    g_openDeviceCount++;

    return 0;
}


/* --------------------------------------------------------------------------
    Name: bcmvenet_close
    Purpose: Stop communicating with the outside world
    Note: Caused by 'ifconfig ethX down'
-------------------------------------------------------------------------- */
static int bcmvenet_close(struct net_device *dev) {
    TRACE(("%s: bcmvenet_close\n", dev->name));

    g_openDeviceCount--;

    /* Stop the network engine */
    if (g_openDeviceCount == 0) {
        napi_disable(&g_napi);
    }

    netif_stop_queue(dev);

    if (g_openDeviceCount == 0) {
        /* Disable mbox and SCB bridge interrupts */
        if (!g_isAxOrBxRevision) {
            g_subbnmctl->hostIrqMask &= ~(MASTER_IRQ_EN | MBOX0_IRQ);
            g_subbnmctl->hostIrqMask2 &= ~(BD_TIMEOUT_IRQ | BD_THRESH_IRQ | MASTER_IRQ_EN);
        } else {
            g_subbnmctl->hostIrqMask &= ~(BD_TIMEOUT_IRQ | BD_THRESH_IRQ | MASTER_IRQ_EN | MBOX0_IRQ);
        }
    }

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcmvenet_net_timeout
 Purpose:
-------------------------------------------------------------------------- */
static void bcmvenet_timeout(struct net_device *dev) {
    BUG_ON(dev == NULL);

    TRACE(("%s: bcmvenet_timeout\n", dev->name));

    dev->trans_start = jiffies;

    dev->stats.tx_errors++;
    netif_wake_queue(dev);
}

/* --------------------------------------------------------------------------
 Name: bcmvenet_set_multicast_list
 Purpose: Set the multicast mode, ie. promiscuous or multicast
-------------------------------------------------------------------------- */
static void bcmvenet_set_multicast_list(struct net_device *dev) {
    TRACE(("%s: bcmvenet_set_multicast_list: %08X\n", dev->name, dev->flags));
}

static void bcmvenet_set_bridge_multicast_list(struct net_device *dev) {
    TRACE(("%s: bcmvenet_set_multicast_list: %08X\n", dev->name, dev->flags));
    if (g_promiscuous ^ ((dev->flags & IFF_PROMISC) ? true : false)) {
        g_subbnmctl->mbxTobnm0 = MBOX_SET_PROMISC | ((dev->flags & IFF_PROMISC) ? 1 : 0);
        g_promiscuous = (dev->flags & IFF_PROMISC) ? true : false;
    }
}

/*
 * Set the hardware MAC address.
 */
static int bcmvenet_set_mac_addr(struct net_device *dev, void *p) {

    struct sockaddr *addr = p;

    if (netif_running(dev)) {
        return -EBUSY;
    }

    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

    return 0;
}

/* --------------------------------------------------------------------------
 Name: bcmvenet_xmit
 Purpose: Send ethernet traffic
-------------------------------------------------------------------------- */
static int bcmvenet_xmit(struct sk_buff *skb, struct net_device *dev, int tag) {

    unsigned char *pBuf;
    unsigned long flags;
    BNM_BD bd;
    unsigned int token;

    //TRACE(("%s %s\n", __FUNCTION__, dev->name));

    if (skb == NULL) {
        TRACE(("%s null skb\n", __FUNCTION__));
        return 0;
    }

    if (!g_link) {
        netif_stop_queue(dev);
        return 0;
    }

    /*
    * Obtain exclusive access to transmitter.  This is necessary because
    * we might have more than one stack transmitting at once.
    */
    spin_lock_irqsave(&g_dma_lock, flags);
    /* if no buffer is available */
    // or BNM MIPS and UBUS have been reset by the host
    if (!g_isAxOrBxRevision) {

        if (((g_dmactl->dmaStatus & 0x1F) == 0) || ((*g_bnmctl & 3) != 3)  ||
                (((token = *((unsigned int *)FPM_ALLOC_DEALLOC)) & 0x80000000) == 0)) {
            /* No buffer available */
tx_error:
            spin_unlock_irqrestore(&g_dma_lock, flags);
            atomic_inc_return(&g_tx_err_cnt);
            netif_stop_queue(dev);
            return 1;
        }

        bd.ptr = g_fpm_base;
        bd.ptr += ((token & FPM_TOKEN_MASK) >> FPM_TOKEN_INDEX_SHIFT) * FPM_TOKEN_SIZE;
        if (bd.ptr >= g_fpm_end) {
            if (((*g_bnmctl & 3) != 3) || (*g_watchdogdefcount == 0)) {
                /* No buffer available */
                goto tx_error;
            } else {
                printk("***bd_ptr = %08x - FORCE CRASH***\n", bd.ptr);
                *(unsigned int *)0 = 0;
            }
        }
    } else {
        /* Allocate a buffer */
        bd.ptr = (unsigned int)g_dmactl->dmaBuffAlloc;

        if (bd.ptr == 0) {
            /* No buffer available */
            goto tx_error;
        }
    }
    spin_unlock_irqrestore(&g_dma_lock, flags);

    pBuf = (unsigned char *)(bd.ptr | 0x80000000);

    /* Copy data into the buffer */
    memcpy(pBuf, skb->data, skb->len);

    /* Flush the cache */
    dma_map_single(&dev->dev, pBuf, FPM_TOKEN_SIZE, DMA_TO_DEVICE);

    /* Construct a BD */
    bd.ctl = ((unsigned int)skb->len << 16) | tag;
    bd.ptr -= g_mem_base;

    /* Write to TX FIFO */
    //TRACE(("%s writing %08x %08x to TX fifo\n", __FUNCTION__, bd.ctl, bd.ptr));
    set_txdma_bd(&bd);

    atomic_inc_return(&g_tx_cnt);

    /* Free the skb */
    dev_kfree_skb_any(skb);

    /* Update stats */
    dev->stats.tx_bytes += skb->len;
    dev->stats.tx_packets++;
    dev->trans_start = jiffies;

    return 0;
}

static int bcmvenet_host_xmit(struct sk_buff *skb, struct net_device *dev) {
    return bcmvenet_xmit(skb, dev, 0);
}

static int bcmvenet_bridge_xmit(struct sk_buff *skb, struct net_device *dev) {
    return bcmvenet_xmit(skb, dev, 0x4000);
}

/* NAPI polling method*/
static int bcmvenet_poll(struct napi_struct *napi, int budget) {

    unsigned int work_done;

    if (!g_link) {
        napi_complete(napi);
        return 0;
    }

    work_done = bcmvenet_rx(budget);
    if (work_done < budget) {
        napi_complete(napi);
        if ((g_isAxOrBxRevision) && (g_subbnmctl->hostBdStatus)) {
            napi_schedule(napi);
        } else {
            /* Enable data interrupt */
            if (!g_isAxOrBxRevision) {
                g_subbnmctl->hostIrqMask2 |= (BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
                // Make sure we haven't missed an interrupt
                if (g_subbnmctl->hostIrqStatus2 & (BD_THRESH_IRQ | BD_TIMEOUT_IRQ)) {
                    g_subbnmctl->hostIrqStatus2 = (BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
                    g_subbnmctl->hostIrqMask2 &= ~(BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
                    napi_schedule(napi);
                }
            } else {
                g_subbnmctl->hostIrqMask |= (BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
            }
        }
    }
    return work_done;
}

/*
 * bcmvenet_rx_isr: BNM rx data interrupt handler
 */
static irqreturn_t bcmvenet_rx_isr(int irq, void *not_used) {
    /* Disable the interrupt at L2 */
    g_subbnmctl->hostIrqMask2 &= ~(BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
    g_subbnmctl->hostIrqStatus2 = (BD_THRESH_IRQ | BD_TIMEOUT_IRQ);
    napi_schedule(&g_napi);
    return IRQ_HANDLED;
}

/*
 * bcmvenet_isr: BNM interrupt handler
 */
static irqreturn_t bcmvenet_isr(int irq, void *not_used) {

    u32 events;

    events = g_subbnmctl->hostIrqStatus;

    if (g_isAxOrBxRevision) {
        if (events & (BD_THRESH_IRQ | BD_TIMEOUT_IRQ)) {
            //TRACE(("%s:Disabling data interrupt\n", __FUNCTION__));
            /* Disable the interrupt at L2 */
            g_subbnmctl->hostIrqMask &= (~(BD_THRESH_IRQ | BD_TIMEOUT_IRQ));
            napi_schedule(&g_napi);
        }
    }

    if (events & MBOX0_IRQ) {
        /* Disable the interrupt at L2 */
        g_subbnmctl->hostIrqMask &= (~MBOX0_IRQ);
        g_irq_stat = events;
        tasklet_schedule(&g_mtask);
    }

    g_subbnmctl->hostIrqStatus = events;
    return IRQ_HANDLED;
}

/*
 *  bcmvenet_rx - descriptor based rx process.
 *  this could be called from bottom half, or from NAPI polling method.
 */
static unsigned int bcmvenet_rx(unsigned int budget) {

    struct net_device *dev;
    struct sk_buff *skb;
    int len;
    unsigned char *pBuf;
    unsigned int bd_lo, bd_hi;
    unsigned int num;
    unsigned int cnt = 0;

    num = g_subbnmctl->hostBdStatus;
    if (g_isAxOrBxRevision) {
        num++;
    }
    while ((cnt < budget) && (num != 0)) {
        /* Read from RX FIFO. */
        bd_lo = g_dmactl->dmaRxLo;

        /*
         * Reading dmaRxHi pops the entry from FIFO, so
         * next read will be from next entry, if one exists
         */
        bd_hi = g_dmactl->dmaRxHi;
        bd_lo &= 0x1fffffff;
        // Determine which device this buffer is for
        if ((bd_hi & 0x00004000) && (eth_bridge)) {
            dev = g_devices[ETH_DEVICE_IX];
        } else {
            dev = g_devices[ECM_DEVICE_IX];
        }

        pBuf = (unsigned char *)((bd_lo + g_mem_base) | 0x80000000);
        len = (int)((0x0fff0000 & bd_hi) >> 16);
        skb = bcmvenet_skb_dequeue_free();
        if (skb) { /* zero copy path */
            /* Invalidate the cache */
            dma_map_single(&dev->dev, pBuf, FPM_TOKEN_SIZE, DMA_FROM_DEVICE);

            /* Attach buffer to skb */
            bcmvenet_attach_skb(skb, pBuf);

            /* Queue the used skb */
            bcmvenet_skb_enqueue_used(skb);
        } else { /* data copy path */
            /* Get a skb buffer */
            skb = dev_alloc_skb(len + 2);
            if (skb == NULL) {
                atomic_inc_return(&g_rx_err_cnt);
                TRACE(("No skb buffer available!\n"));
                /* Return the buffer */
                bcmvenet_buf_return(bd_lo);
                break;
            }

            /* Align to 16 bit */
            skb_reserve(skb, 2);

            /* Invalidate the cache */
            dma_map_single(&dev->dev, pBuf, FPM_TOKEN_SIZE, DMA_FROM_DEVICE);

            /* Copy data */
            memcpy(skb->data, pBuf, len);

            /* Return the buffer */
            bcmvenet_buf_return(bd_lo);
        }

        /* Set packet length */
        if ((skb->tail + len) > skb->end) {
            /* something is wrong, discard this packet */
            /* Free FPM buffer */
            unsigned int bd;
            bd = (unsigned int)virt_to_phys(skb->head) - g_mem_base;
            //printk("Freeing %p : bd = %08x len = %d\n", skb->head, bd, skb->len);
            bcmvenet_buf_return(bd);

            /* Append it to the free queue */
            bcmvenet_skb_enqueue_free(skb);
        } else {
            skb_put(skb, len);

            /* Device, protocol and housekeeping */
            skb->dev = dev;
            skb->protocol = eth_type_trans(skb, dev);
            dev->stats.rx_packets++;
            dev->stats.rx_bytes += len;

            /* Give it to kernel */
            netif_receive_skb(skb);
        }
        atomic_inc_return(&g_rx_cnt);
        cnt++;
        if (g_isAxOrBxRevision) {
            if (bd_lo != g_dmactl->dmaRxLo) {
                num = g_subbnmctl->hostBdStatus;
                if (num == 0) {
                    num++;
                }
            } else {
                num--;
            }
        } else {
            num = g_subbnmctl->hostBdStatus;
        }
    }

    bcmvenet_reclaim_rx_skb();

    return cnt;
}

/*
 * init_edma: Initialize DMA control register
 */
static void init_subdma() {
    TRACE(("bcmvenet: init_subdma\n"));

    /* Configure thresholds */
    if ((g_revid & 0xffff) >= 0x20) {
        g_dmactl->dmaRxfifoThres = 0x40001C01;
    }
}

static int bcmvenet_proc_get_stat(char *page, char **start, off_t off, int cnt, int *eof, void *data) {

    int r = sprintf(page, "hostIrqStatus(%08x) = %08x, mbxFrmbnm0(%08x) = %08x\n",
                    (unsigned int)&g_subbnmctl->hostIrqStatus,
                    (unsigned int)g_subbnmctl->hostIrqStatus,
                    (unsigned int)&g_subbnmctl->mbxFrmbnm0,
                    (unsigned int)g_subbnmctl->mbxFrmbnm0);

    r += sprintf(page + r, "hostIrqMask(%08x) = %08x\n",
                 (unsigned int)&g_subbnmctl->hostIrqMask,
                 (unsigned int)g_subbnmctl->hostIrqMask);

    if (!g_isAxOrBxRevision) {
        r += sprintf(page + r, "hostIrqStatus2(%08x) = %08x\n",
                     (unsigned int)&g_subbnmctl->hostIrqStatus2,
                     (unsigned int)g_subbnmctl->hostIrqStatus);

        r += sprintf(page + r, "hostIrqMask2(%08x) = %08x\n",
                     (unsigned int)&g_subbnmctl->hostIrqMask2,
                     (unsigned int)g_subbnmctl->hostIrqMask2);
    }

    r += sprintf(page + r, "dmaRxfifoThres(%08x) = %08x\n",
                 (unsigned int)&g_dmactl->dmaRxfifoThres,
                 (unsigned int)g_dmactl->dmaRxfifoThres);

    r += sprintf(page + r, "Total queue length should be %d\n", VENET_RX_BUF_NUM);
    r += sprintf(page + r, "SUB MAGIC on : %d\n", g_isAxOrBxRevision);
    r += sprintf(page + r, "Memory_base : %08lx\n", g_mem_base);
    r += sprintf(page + r, "Host BD Status: %d\n", g_subbnmctl->hostBdStatus);
    r += sprintf(page + r, "TX BD count: %d\n", g_dmactl->dmaTxBdCnt);
    r += sprintf(page + r, "TX Missed BD count: %d\n", g_dmactl->dmaTxMissedBdCnt);
    r += sprintf(page + r, "TX DMA status: %d\n", g_dmactl->dmaStatus);
    r += sprintf(page + r, "RX count: %d\n", atomic_read(&g_rx_cnt));
    r += sprintf(page + r, "RX error count: %d\n", atomic_read(&g_rx_err_cnt));
    r += sprintf(page + r, "TX count: %d\n", atomic_read(&g_tx_cnt));
    r += sprintf(page + r, "TX error count: %d\n", atomic_read(&g_tx_err_cnt));
    r += sprintf(page + r, "FPM base = %08lx\n", g_fpm_base);
    r += sprintf(page + r, "FPM end = %08lx\n", g_fpm_end);
    r += sprintf(page + r, "DATA base = %08lx\n", g_data_base);
    *eof = 1;
    return r;
}

int bcmvenet_del_proc_files(struct net_device *dev) {
    remove_proc_entry("driver/bcmvenet/stat", NULL);
    remove_proc_entry("driver/bcmvenet", NULL);
    return 0;
}

int bcmvenet_add_proc_files(struct net_device *dev) {
    proc_mkdir("driver/bcmvenet", NULL);
    create_proc_read_entry("driver/bcmvenet/stat", 0, NULL, bcmvenet_proc_get_stat, 0);
    return 0;
}
/*
 * bcmvenet_init_dev: initialize device
 */
static int bcmvenet_init_dev() {

    venet_sk_buff_list *skb_node;
    int i;

    TRACE(("%s\n", __FUNCTION__));

    g_link = 0;

    /* register block locations */
    g_subbnmctl = (SUBBNMCTLRegisters *)(g_devices[ECM_DEVICE_IX]->base_addr);
    g_bdgctl = (unsigned int *)(g_devices[ECM_DEVICE_IX]->base_addr + 0x100);
    g_dmactl = (SUBDMARegisters *)(g_devices[ECM_DEVICE_IX]->base_addr + 0x300);
    TRACE(("%s: bnmctl=0x%08x dmactl=0x%08x\n", __FUNCTION__, (unsigned int)g_subbnmctl, (unsigned int)g_dmactl));
    TRACE(("hostIrqStatus = %08x\n", g_subbnmctl->hostIrqStatus));
    TRACE(("g_subbnmctl->mbxFrmbnm0 = %08x\n", g_subbnmctl->mbxFrmbnm0));

    // Detect the chip geometry and pass it to the ECM
    g_geometry = read_chip_geometry_otp_bit();
    g_geometry = g_geometry | (~g_geometry << 16);
    g_subbnmctl->mbxTobnm3 = g_geometry;

    /* Initialize the skb queues */
    INIT_LIST_HEAD(&g_skb_list_free.head);
    spin_lock_init(&g_skb_list_free.qlock);

    INIT_LIST_HEAD(&g_skb_list_used.head);
    spin_lock_init(&g_skb_list_used.qlock);

    /* Allocate memory for skb header */
    for (i = 0; i < VENET_RX_BUF_NUM; i++) {
        /* UNFINISHED, align skb header to cache line */
        skb_node = (venet_sk_buff_list *)kmalloc(sizeof(venet_sk_buff_list), GFP_KERNEL);
        if (skb_node == NULL) {
            printk("%s:%d failure allocating skb\n", __FUNCTION__, __LINE__);
            return -1;
        }

        /* Queue the skb header */
        bcmvenet_skb_enqueue_free(&skb_node->skb);
    }

    /* init SUB DMA registers */
    init_subdma();

    bcmvenet_add_proc_files(g_devices[ECM_DEVICE_IX]);

    /* init timer */
    init_timer(&g_rtimer);
    g_rtimer.data = 0;
    g_rtimer.function = bcmvenet_rtimer_handler;
    if ((g_revid & 0xff) >= 0x20) {
        init_timer(&g_watchdogTimer);
        g_watchdogTimer.data = 0;
        g_watchdogTimer.function = bcmvenet_bnm_watchdog_handler;
        /* start the watchdog timer */
        g_watchdogTimer.expires = jiffies + WATCHDOG_INTERVAL;
        add_timer(&g_watchdogTimer);
    }

    TRACE(("%s done! \n", __FUNCTION__));

    /* if we reach this point, we've init'ed successfully */
    return 0;
}

/* Uninitialize tx/rx buffer descriptor pools */
static void bcmvenet_uninit_dev() {
    bcmvenet_del_proc_files(g_devices[ECM_DEVICE_IX]);
    del_timer_sync(&g_rtimer);
    if ((g_revid & 0xff) >= 0x20) {
        del_timer_sync(&g_watchdogTimer);
    }
    TRACE(("%s\n", __FUNCTION__));
}

/*
* ethtool function - get driver info.
*/
static void bcmvenet_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info) {
    strncpy(info->driver, CARDNAME, sizeof(info->driver));
    strncpy(info->version, VER_STR, sizeof(info->version));
}

/*
 * standard ethtool support functions.
 */
static struct ethtool_ops bcmvenet_ethtool_ops = {
    .get_drvinfo		= bcmvenet_get_drvinfo,
    .get_link			= ethtool_op_get_link,
};

static int IsPowerOfTwo(unsigned int n) {

    unsigned int count = 0;

    while (n) {
        if (n & 1) {
            count++;
        }
        n >>= 1;
    }
    return count == 1;
}

static void ConfigureSCBBridge(unsigned int bootWindowAddress,
                               unsigned int bootWindowSize,
                               unsigned int docsisWindowAddress,
                               unsigned int docsisWindowSize) {
    static unsigned int DocsisInit[] = {
        0x00000010,
        0x000000db,
        0xabcdef00,
        0xd355aa2c,
        0x789a0058,
        0x00005080, // 5 SGISB_BNM_SCTL
        0x40000001, // 6 SGISB_BNMRST
        0x00000000, // 7 SGISB_BNM_DDR_DEST_BASE
        0x00000000, // 8 SGISB_BNM_DDR_ORIG_BASE
        0x00000000, // 9 SGISB_BNM_DDR_WMASK
        0x00000000, // 10 SGISB_BNM_DATA_DEST_BASE
        0x00000000, // 11 SGISB_BNM_DATA_ORIG_BASE
        0x00000000, // 12 SGISB_BNM_DATA_WMASK
        0x00000000, // 13 SGISB_BNM_LMB_ORIG_BASE
        0x00000000, // 14 SGISB_BNM_BOOT_DEST_BASE
        0x00000000, // 15 SGISB_BNM_BOOT_ORIG_BASE
        0x00000000, // 16 SGISB_BNM_BOOT_WMASK
        0x00000000, // 17 SGISB_BNM_UBUS_BASE
        0x1e000000, // 18 SGISB_BNM_UBUS_MASK
        0x12000000, // 19 SGISB_BNM_GISB_UBUS_BASE
        0x14ffffff, // 20 SGISB_BNM_GISB_UBUS_END
        0x00000000, // 21 SGISB_BNM_SCB_BUFFER_BASE
        0x00000000, // 22 SGISB_BNM_SCB_BUFFER_MASK
        0x12000000, // 23 SGISB_BNM_MIPS_CODE_WIN_BASE
        0x18000000, // 24 SGISB_BNM_MIPS_CODE_WIN_MASK
        0x00000000, // 25 SGISB_BNM_MIPS_DATA_WIN_BASE
        0x00000000  // 26 SGISB_BNM_MIPS_DATA_WIN_MASK
    };

    volatile unsigned int *scbRtTxCtl = (unsigned int *)0xb4000150;
    volatile unsigned int *scbRtRxCtl = (unsigned int *)0xb4000154;
    volatile unsigned int *cmd = (unsigned int *)0xb0327800;
    volatile unsigned int *iRdy = (unsigned int *)0xb032b008;
    volatile unsigned int *oLoad2 = (unsigned int *)0xb032B020;
    volatile unsigned int *iLoad2 = (unsigned int *)0xb032B028;

    // Setup boot window
    DocsisInit[14] = bootWindowAddress;
    DocsisInit[15] = 0x14800000;
    DocsisInit[16] = ~(bootWindowSize - 1);

    // Setup docsis window
    DocsisInit[7] = docsisWindowAddress;
    DocsisInit[9] = ~(docsisWindowSize - 1);

    // Setup SCB Window
    DocsisInit[21] = docsisWindowAddress;
    DocsisInit[22] = ~(docsisWindowSize - 1);

    // Setup Data Window
    DocsisInit[10] = docsisWindowAddress;
    DocsisInit[11] = 0xa0000000;
    DocsisInit[12] = ~(docsisWindowSize - 1);

    // wait for AEGIS ready
    while ((*iRdy & 2) == 0);
    // write to AEGIS
    memcpy((void *)(cmd + 0x60), (void *)DocsisInit, sizeof(DocsisInit));
    *iLoad2 = 1;
    while (((*oLoad2) & 1) == 0);
    *oLoad2 = 0;

    // Set SCB translation based on host side endianness
#if defined(__BIG_ENDIAN)
    *scbRtTxCtl = 0x10;
    *scbRtRxCtl = 0x10;
#else
    *scbRtTxCtl = 0x1f;
    *scbRtRxCtl = 0x1f;
#endif
}

#define SUN_TOP_CTRL_PIN_MUX_CTRL_4 0xb0404110
#define SUN_TOP_CTRL_PIN_MUX_CTRL_6 0xb0404118
#define SUN_TOP_CTRL_PIN_MUX_CTRL_7 0xb040411C
#define SUN_TOP_CTRL_PIN_MUX_CTRL_9 0xb0404124
#define SUN_TOP_CTRL_PIN_MUX_CTRL_10 0xb0404128
#define SUN_TOP_CTRL_PIN_MUX_CTRL_11 0xb040412C
#define GIO_DATA_LO 0xb0406704
#define GIO_IODIR_LO 0xb0406708

static void ConfigurePinMux(void) {

    volatile unsigned int *pinMux4 = (unsigned int *)SUN_TOP_CTRL_PIN_MUX_CTRL_4;
    volatile unsigned int *pinMux6 = (unsigned int *)SUN_TOP_CTRL_PIN_MUX_CTRL_6;
    volatile unsigned int *pinMux7 = (unsigned int *)SUN_TOP_CTRL_PIN_MUX_CTRL_7;
    volatile unsigned int *pinMux9 = (unsigned int *)SUN_TOP_CTRL_PIN_MUX_CTRL_9;
    volatile unsigned int *pinMux10 = (unsigned int *)SUN_TOP_CTRL_PIN_MUX_CTRL_10;
    volatile unsigned int *pinMux11 = (unsigned int *)SUN_TOP_CTRL_PIN_MUX_CTRL_11;
    volatile unsigned int *gpioDataLo = (unsigned int *)GIO_DATA_LO;
    volatile unsigned int *gpioIODirLo = (unsigned int *)GIO_IODIR_LO;

    // Setup the pin mux to enable UART pins and I2C pins and SPI stuff
    *pinMux4 = (*pinMux4 & 0xfff00fff) | 0x00011000;
    *pinMux6 = (*pinMux6 & 0x000000ff) | 0x11111100;
    *pinMux7 = (*pinMux7 & 0xfffff000) | 0x00000111;
    *pinMux9 = (*pinMux9 & 0x0fffffff) | 0x10000000;
    *pinMux10 = (*pinMux10 & 0x0ffffff0) | 0x20000001;
    *pinMux11 = (*pinMux11 & 0xfff00000) | 0x00011112;
    *gpioIODirLo &= ~1;
    *gpioDataLo |= 1;
}

static void CopyToSegment(void *segAddress, void *userAddress, unsigned int size, unsigned int segSize) {

    void *seg = (void *)((unsigned int)segAddress | 0xa0000000);

    memset(seg, 0, segSize);
    copy_from_user(seg, userAddress, size);
}

static int bcmvenet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd) {

    int result = -EINVAL;

    switch (cmd) {
    case SIOC_HOSTBOOT: {
        struct venet_ioctl_parm parms;
        u32 bootBase;
        u32 docsisBase;
        // Copy the host boot parameters from user space
        copy_from_user((void *)&parms, (void *)rq->ifr_data, sizeof(struct venet_ioctl_parm));
        result = -EINVAL;
        // Validate boot and docsis region sizes
        if (parms.bootCodeSize > parms.bootSegSize) {
            break;
        }
        if (parms.docsisCodeSize > (DOCSIS_MAX_SIZE - (2 * NONVOL_MAX_SIZE))) {
            break;
        }
        if (parms.dynNonVolSize > NONVOL_MAX_SIZE) {
            break;
        }
        if (parms.permNonVolSize > NONVOL_MAX_SIZE) {
            break;
        }
        if (!IsPowerOfTwo(parms.bootSegSize)) {
            break;
        }
        if (!IsPowerOfTwo(parms.docsisSegSize)) {
            break;
        }
        // We can't use anything over 256 meg due to EBI register overlap
        if (parms.physicalMemorySize > (256 * 1024 * 1024)) {
            parms.physicalMemorySize = 256 * 1024 * 1024;
        }
        docsisBase = parms.physicalMemorySize - parms.docsisSegSize;
        bootBase = docsisBase - parms.bootSegSize;
        // Make sure segment start addresses are multiple of segment sizes
        if ((bootBase & (parms.bootSegSize - 1)) != 0) {
            break;
        }
        if ((docsisBase & (parms.docsisSegSize - 1)) != 0) {
            break;
        }
        // Parameters look ok, initiate ECM reboot
        printk(KERN_INFO "bcm0: RESET_DOCSIS! Host application initiated\n");
        // kill all link activity while we are rebooting
        if (g_link) {
            g_link = 0;
            set_carrier();
        }
        *g_bnmctl |= 1; // Make sure UBUS is active
        *g_bnmctl &= ~2;  // Reset the MIPS
        // Copy the bootloader to boot segment
        CopyToSegment((void *)bootBase, (void *)parms.bootCodePtr, parms.bootCodeSize, parms.bootSegSize);
        // Copy docsis to docsis segment
        CopyToSegment((void *)(docsisBase + parms.docsisSegSize - DOCSIS_MAX_SIZE),
                      (void *)parms.docsisCodePtr, parms.docsisCodeSize, (DOCSIS_MAX_SIZE - (2 * NONVOL_MAX_SIZE)));
        // Copy nonvol to to of docsis segment
        CopyToSegment((void *)(docsisBase + parms.docsisSegSize - NONVOL_MAX_SIZE),
                      (void *)parms.dynNonVolPtr, parms.dynNonVolSize, NONVOL_MAX_SIZE);
        CopyToSegment((void *)(docsisBase + parms.docsisSegSize - (2 * NONVOL_MAX_SIZE)),
                      (void *)parms.permNonVolPtr, parms.permNonVolSize, NONVOL_MAX_SIZE);
        // Set up the SCB bridge
        ConfigureSCBBridge(bootBase, parms.bootSegSize, docsisBase, parms.docsisSegSize);
        ConfigurePinMux();
        reset_bnm_mips(false);
        result = 0;
    }
    break;
    case SIOC_GET_AND_RESET_DMA_READY_INDICATION: {
        result = g_dma_ready_indication;
        g_dma_ready_indication = 0;
    }
    break;
    }
    return result;
}

static int bcmvenet_bridge_ioctl(struct net_device *dev, struct ifreq *rq, int cmd) {
    return -EINVAL;
}

static const struct net_device_ops bcmvenet_netdev_ops = {
    .ndo_open = bcmvenet_open,
    .ndo_stop = bcmvenet_close,
    .ndo_start_xmit = bcmvenet_host_xmit,
    .ndo_tx_timeout = bcmvenet_timeout,
    .ndo_set_multicast_list = bcmvenet_set_multicast_list,
    .ndo_set_mac_address = bcmvenet_set_mac_addr,
    .ndo_do_ioctl = bcmvenet_ioctl,
};

static const struct net_device_ops bcmvenet_netdev_bridge_ops = {
    .ndo_open = bcmvenet_open,
    .ndo_stop = bcmvenet_close,
    .ndo_start_xmit = bcmvenet_bridge_xmit,
    .ndo_tx_timeout = bcmvenet_timeout,
    .ndo_set_multicast_list = bcmvenet_set_bridge_multicast_list,
    .ndo_set_mac_address = bcmvenet_set_mac_addr,
    .ndo_do_ioctl = bcmvenet_bridge_ioctl,
};

static inline void dummy_release(struct device *dev) {}

static struct bcmemac_platform_data venet_data;
static struct bcmemac_platform_data venet_bridge_data;

static struct resource venet_resources[] = {
    {
        .start		= BPHYSADDR(BCHP_SUB_BNMCTL_REG_START),
        .end		= BPHYSADDR(BCHP_SUB_DMA_REG_END) + 3,
        .flags		= IORESOURCE_MEM,
    },
    {
        .start		= BRCM_IRQ_BNM,
        .end		= BRCM_IRQ_BNM,
        .flags		= IORESOURCE_IRQ,
    },
    {
        .start		= BRCM_IRQ1_BNM,
        .end		= BRCM_IRQ1_BNM,
        .flags		= IORESOURCE_IRQ,
    }
};

static struct platform_device venet_plat_dev[2] = {
    {
        .name               = "bcmvenet",
        .id			        = 0,
        .num_resources		= ARRAY_SIZE(venet_resources),
        .resource		    = venet_resources,
        .dev = {
            .platform_data	= &venet_data,
            .release	    = dummy_release,
        }
    },
    {
        .name               = "bcmvbridge",
        .id			        = 1,
        .num_resources		= 0,
        .resource		    = 0,
        .dev = {
            .platform_data	= &venet_bridge_data,
            .release	    = dummy_release,
        }
    }
};

static struct resource *bcmvenet_platform_get_resource(struct platform_device *dev, unsigned int type,
        unsigned int num) {

    int i;
    struct resource *r = dev->resource;

    for (i = 0; i < dev->num_resources; i++) {
        if (type == (r->flags & (IORESOURCE_IO | IORESOURCE_MEM | IORESOURCE_IRQ | IORESOURCE_DMA))) {
            if (num == 0) {
                return r;
            } else {
                num--;
            }
        }
        r++;
    }
    return NULL;
}

static int bcmvenet_platform_get_irq(struct platform_device *dev, unsigned int num) {

    struct resource *r = bcmvenet_platform_get_resource(dev, IORESOURCE_IRQ, num);

    return r ? r->start : 0;
}

static int bcmvenet_platform_driver_register(struct platform_driver *drv, int ifs) {

    int err = 0;
    int d;

    /* Probe the devices */
    for (d = 0; d < ifs; d++) {
        /* Allocate a MAC address */
        brcm_alloc_macaddr(((struct bcmemac_platform_data *)(venet_plat_dev[d].dev.platform_data))->macaddr);
        // probe the device
        err = drv->probe(&venet_plat_dev[d]);
        if (err) {
            return err;
        }
    }
    return 0;
}

static void bcmvenet_platform_driver_unregister(struct platform_driver *drv, int ifs) {
    /* Remove the device */

    int d;

    /* remove the devices */
    for (d = 0; d < ifs; d++) {
        drv->remove(&venet_plat_dev[d]);
    }
}

static int bcmvenet_drv_probe(struct platform_device *pdev) {

    struct resource *mres = 0, *ires;
    void __iomem *base = 0;
    unsigned long res_size = 0;
    unsigned long MailBoxVal;
    static unsigned char devName[5] = { 'b', 'c', 'm', 0, 0 };

    /*
     * bcmemac and bcmgenet use same platform data structure.
     */
    struct bcmemac_platform_data *cfg = pdev->dev.platform_data;
    struct net_device *dev;

    TRACE(("%s %s %d\n", __FUNCTION__, pdev->name, pdev->id));

    devName[3] = '0' + pdev->id;
    dev = alloc_netdev(0, devName, ether_setup);
    if (dev == NULL) {
        printk(KERN_ERR "bcmgenet: can't allocate net device\n");
        return -ENOMEM;
    }
    dev->dev_id = pdev->id;
    memcpy(dev->dev_addr, cfg->macaddr, 6);
    dev->watchdog_timeo = 2 * HZ;
    SET_ETHTOOL_OPS(dev, &bcmvenet_ethtool_ops);
    netif_carrier_off(dev);
    g_devices[pdev->id] = dev;

    if (pdev->id == ECM_DEVICE_IX) {
        TRACE(("Getting IRQs %d %d\n", BRCM_IRQ_BNM, BRCM_IRQ1_BNM));
        mres = bcmvenet_platform_get_resource(pdev, IORESOURCE_MEM, 0);
        ires = bcmvenet_platform_get_resource(pdev, IORESOURCE_IRQ, 0);
        if (!mres || !ires) {
            printk(KERN_ERR "%s: can't get resources\n", __FUNCTION__);
            return (-EIO);
        }
        res_size = mres->end - mres->start + 1;
        base = ioremap(mres->start, mres->end - mres->start + 1);
        TRACE(("%s: base=0x%x\n", __FUNCTION__, (unsigned int)base));

        if (!base) {
            printk(KERN_ERR "%s: can't ioremap\n", __FUNCTION__);
            return (-EIO);
        }

        dev->base_addr = (unsigned long)base;
        dev->netdev_ops = &bcmvenet_netdev_ops;
        netif_napi_add(dev, &g_napi, bcmvenet_poll, 64);

        // Let boot setup info override default settings.
        netdev_boot_setup_check(dev);

        g_irq = bcmvenet_platform_get_irq(pdev, 0);
        g_irq_rx = bcmvenet_platform_get_irq(pdev, 1);

        spin_lock_init(&g_dma_lock);

        /* Init the tasklet */
        g_mtask.next = NULL;
        g_mtask.state = 0;
        atomic_set(&g_mtask.count, 0);
        g_mtask.func = mbox_tasklet;
        g_mtask.data = 0;

        /* Init GENET registers, Tx/Rx buffers */
        if (bcmvenet_init_dev() < 0) {
            printk(KERN_ERR "can't initialize device\n");
            return -ENOMEM;
        }

        if (request_irq(g_irq, bcmvenet_isr, IRQF_SHARED, dev->name, INTERRUPT_PARAMETER) < 0) {
            printk(KERN_ERR "can't request IRQ %d\n", g_irq);
            return -ENOMEM;
        }

        if (register_netdev(dev) != 0) {
            printk(KERN_ERR "can't register netdev\n");
            return -ENOMEM;
        }

        if ((g_revid & 0xffff) < 0x20) {
            g_isAxOrBxRevision = true;
        } else {
            g_isAxOrBxRevision = false;

            g_fpm_end = *(volatile unsigned int *)SUB_BNM_SCB_BUFFER_MASK;
            g_fpm_end |= 0xe0000000;
            g_fpm_end = ~g_fpm_end;
            g_fpm_end += 1;
            g_fpm_end += *(volatile unsigned int *)SUB_BNM_SCB_BUFFER_BASE;
            if (largeflashpartition) {
                g_fpm_end -= ECRAM_STO_RESERVED_LARGE;
            } else {
                g_fpm_end -= ECRAM_STO_RESERVED_DEFAULT;
            }

            if (request_irq(g_irq_rx, bcmvenet_rx_isr, IRQF_SHARED, dev->name, INTERRUPT_PARAMETER) < 0) {
                printk(KERN_ERR "can't request RX IRQ %d\n", g_irq_rx);
                return -ENOMEM;
            }
        }

        MailBoxVal = g_subbnmctl->mbxFrmbnm0;

        if ((MailBoxVal & MBOX_MSG_MASK) == MBOX_SHM_ADDR) {
            printk(KERN_INFO "bcm0: MBOX_SHM_ADDR at init!\n");
            g_shm_addr = ((~MBOX_MSG_MASK) & MailBoxVal);
            /* Acknowledge the message */
            g_subbnmctl->mbxTobnm0 = MailBoxVal;
            TRACE(("g_shm_addr = %0x\n", (unsigned int)g_shm_addr));
        }
    } else {
        dev->netdev_ops = &bcmvenet_netdev_bridge_ops;
        if (register_netdev(dev) != 0) {
            printk(KERN_ERR "can't register netdev\n");
            return -ENOMEM;
        }
    }
    TRACE(("%s done\n", __FUNCTION__));
    /* print the ChipID and module version info */
    printk(KERN_INFO "%s: Broadcom BCM%X(%X) Virtual Ethernet Driver " VER_STR "\n",
           devName, g_revid >> 16, (g_revid & 0xffff) + 0xa0);

    return 0;
}

static int bcmvenet_drv_remove(struct platform_device *pdev) {
    TRACE(("bcmvenet_drv_remove device id %d\n", pdev->id));

    unregister_netdev(g_devices[pdev->id]);
    if (pdev->id == ECM_DEVICE_IX) {
        free_irq(g_irq, INTERRUPT_PARAMETER);
        if (!g_isAxOrBxRevision) {
            free_irq(g_irq_rx, INTERRUPT_PARAMETER);
        }
        bcmvenet_uninit_dev();
    }
    free_netdev(g_devices[pdev->id]);
    return (0);
}

static struct platform_driver bcmvenet_plat_drv = {
    .probe =		bcmvenet_drv_probe,
    .remove =		bcmvenet_drv_remove,
    .driver = {
        .name =		"bcmvenet",
        .owner =	THIS_MODULE,
    },
};

static int bcmvenet_module_init(void) {
    TRACE(("bcmvenet_module_init\n"));

    // NOTE: The following workaround has nothing whatsoever to do with the proper functioning of this module.
    // It is included here since HOST side registers not accessible from the BNM side need to be initialized
    // in order for the BNM BPI sequence error workaround to operate correctly
    g_revid = *(volatile unsigned int *)((BCHP_SUN_TOP_CTRL_PROD_REVISION + BCHP_PHYSICAL_OFFSET) | 0xA0000000);

    // Only applies to Ax and Bx chip revisions
    if ((g_revid & 0xffff) < 0x20) {
        *((unsigned int *)0xb0404084) |= 0x00000100;
        *((unsigned int *)0xb040408c) = 0x00000606;
        printk(KERN_INFO CARDNAME ": BPI sequence error workaround enabled\n");
    }
    // END of workaround code.

    return bcmvenet_platform_driver_register(&bcmvenet_plat_drv, eth_bridge ? 2 : 1);
}

static void bcmvenet_module_cleanup(void) {

    struct sk_buff *skb = NULL;
    venet_sk_buff_list *n;

    TRACE(("bcmvenet_module_cleanup\n"));
    bcmvenet_platform_driver_unregister(&bcmvenet_plat_drv, eth_bridge ? 2 : 1);
    while ((skb = bcmvenet_skb_dequeue_free())) {
        n = container_of(skb, venet_sk_buff_list, skb);
        kfree(n);
    }
    while ((skb = bcmvenet_skb_dequeue_used())) {
        n = container_of(skb, venet_sk_buff_list, skb);
        kfree(n);
    }
}

module_init(bcmvenet_module_init);
module_exit(bcmvenet_module_cleanup);

MODULE_LICENSE("Proprietary");
