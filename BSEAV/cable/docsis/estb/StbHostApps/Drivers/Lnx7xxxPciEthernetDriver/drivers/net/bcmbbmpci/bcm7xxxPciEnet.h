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
*  Filename: bcm7xxxPciEnet.h 
*
****************************************************************************
*  Description:	Definitions for BCM7xxx PCI/SCB driver registers and
                structures.
*
*
****************************************************************************/
#ifndef _BCM7XXXPCIENET_H_
#define _BCM7XXXPCIENET_H_

#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/kernel.h>	/* For barrier() */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
#include "bcmtypes.h"
#include <asm/brcmstb/brcmstb.h>
#else
#include <asm/brcmstb/common/bcmtypes.h>
#include <asm/brcmstb/common/brcmstb.h>
#endif
#include <linux/spinlock.h>

#define	K0_TO_K1(x)	((unsigned)(x)|0xA0000000)	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)	((unsigned)(x)&0x9FFFFFFF)	/* kseg1 to kseg0 */

#define	K0_TO_PHYS(x)	((unsigned)(x)&0x1FFFFFFF)	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	((unsigned)(x)&0x1FFFFFFF)	/* kseg1 to physical */
#ifdef PHYS_TO_K0
#undef PHYS_TO_K0
#define	PHYS_TO_K0(x)	((unsigned)(x)|0x80000000)	/* physical to kseg0 */
#endif
#define	PHYS_TO_K1(x)	((unsigned)(x)|0xA0000000)	/* physical to kseg1 */

#define REG8(addr)                  (*(volatile UCHAR  * const)(addr))
#define REG32(addr)                (*(volatile ULONG  * const)(addr))
#define REG32_w(addr)              (*(volatile ULONG  * const)(addr))
#define wREG32(addr, val)		   REG32_w(addr) = val
#define wREG32(addr, val)		   REG32_w(addr) = val
#define orREG32(addr, val)			   REG32(addr) |= (val)
#define andREG32(addr, val)			   REG32(addr) &= (val)

// **
#include "pcidma_3368.h"
// **

// ********************* Post-defineds ***************************************
#define BCM_ALIGN(addr, boundary) ((addr + boundary - 1) & ~(boundary - 1))

#define BCM_SKB_PAD 4
#define BCM_SKB_DMA_ALIGN 256
/*---------------------------------------------------------------------*/
/* specify number of BDs and buffers to use                            */
/*---------------------------------------------------------------------*/
#define NR_TX_BDS               192
#if defined(BCM7118)
#define NR_RX_BDS               192
#else
#ifdef CES2006_DEMO
#define NR_RX_BDS               1024	
#else
#define NR_RX_BDS               192
#endif
#endif
#define ENET_MAX_MTU_SIZE       1536    //1618    /* Body(1500) + EH_SIZE(14) + FCS(4) */
#define DMA_MAX_BURST_LENGTH    0x40      /* in 32 bit words = 256 bytes  THT per David F, to allow 256B burst, was 16; */
#define MAX_RX_BUFS             (NR_RX_BDS * 4)
#define NR_PCIHOST_RX_BDS	NR_RX_BDS
#define NR_PCIHOST_TX_BDS	NR_TX_BDS

#define CACHE_TO_NONCACHE(x)	KSEG1ADDR(x)

#define ERROR(x)        printk x
#ifndef ASSERT
#define ASSERT(x)       if (x); else ERROR(("assert: "__FILE__" line %d\n", __LINE__));
#endif

#if defined(DUMP_TRACE)
#define TRACE(x)        printk x
#else
#define TRACE(x)
#endif

#pragma pack(1)
typedef struct Enet_Tx_CB {
    struct sk_buff      *skb;
    uint32              nrBds;          /* # of bds req'd (incl 1 for this header) */
    volatile DmaDesc    *lastBdAddr;
	dma_addr_t dma_addr;
	int dma_len;
    struct Enet_Tx_CB   *next;          /* ptr to next header in free list */
} Enet_Tx_CB;

typedef struct Enet_Rx_Buff {
    unsigned char       dAddr[ETH_ALEN];/* destination hardware address */
    unsigned char       sAddr[ETH_ALEN];/* source hardware address */
    uint16              type;           /* Ethernet type/length */
    char                buffer[1500];   /* data */
    uint32              crc;            /* normal checksum (FCS1) */
    uint16              pad1;           /* rsvd padding */
    uint32              pad2[4];        /* Must be mult of EMAC_DMA_MAX_BURST_LENGTH */
} Enet_Rx_Buff;
#pragma pack()
/*
 * device context
 */
typedef struct BcmEnet_devctrl {
    struct net_device *dev;             /* ptr to net_device */
	struct pci_dev *pdev;				/* ptr to pci_dev */
	spinlock_t      lock;               /* Serializing lock */
    struct timer_list reset_timer;      /* used by to poll for reset 3255 */
    struct timer_list refill_timer;      /* used to refill RX DMA */
#if defined(BCM7118)
    struct timer_list reboot_timer;      /* used to detect 3255 reset failure */
#endif
    atomic_t        devInUsed;          /* device in used */

    struct net_device_stats stats;      /* statistics used by the kernel */

    struct sk_buff  *skb_pool[MAX_RX_BUFS]; /* rx buffer pool */
    int             nextskb;            /* next free skb in skb_pool */
    atomic_t        rxDmaRefill;        /* rxDmaRefill == 1 needs refill rxDma */
    volatile unsigned long             rxbuf_assign_busy;  /* assign_rx_buffers busy */

    /* transmit variables */
#if defined(BCM7118)
	volatile SUBRegisters      *subRegs;
	volatile NSGISBRegisters   *nsGisbRegs;
	volatile M2MBlockRegisters *m2mRegs;
    volatile IuDmaCfgRegisters *txDma;      /* Location of transmit DMA register set */
	volatile IuDmaStateRamRegisters *txDmaState;             /* Location of receive DMA state register set */
#else
    volatile MpiRegisters *mpiRegs;
    volatile PCIDmaConfig *txDma;      /* Location of transmit DMA register set */
#endif

    Enet_Tx_CB      *txCbPtrHead;       /* points to EnetTxCB struct head */
    Enet_Tx_CB      *txCbPtrTail;       /* points to EnetTxCB struct tail */

    Enet_Tx_CB      *txCbQHead;         /* points to EnetTxCB struct queue head */
    Enet_Tx_CB      *txCbQTail;         /* points to EnetTxCB struct queue tail */
    Enet_Tx_CB      *txCbs;             /* memory locaation of tx control block pool */

    volatile DmaDesc *txBds;            /* Memory location of tx Dma BD ring */
    volatile DmaDesc *txLastBdPtr;      /* ptr to last allocated Tx BD */
    volatile DmaDesc *txFirstBdPtr;     /* ptr to first allocated Tx BD */
    volatile DmaDesc *txNextBdPtr;      /* ptr to next Tx BD to transmit with */

    int             nrTxBds;            /* number of transmit bds */
    int             txFreeBds;          /* # of free transmit bds */

    /* receive variables */
#if defined(BCM7118)
    volatile  IuDmaCfgRegisters *rxDma;     /* Location of receive DMA register set */
	volatile  IuDmaStateRamRegisters *rxDmaState;             /* Location of receive DMA state register set */
#else
    volatile  PCIDmaConfig *rxDma;     /* Location of receive DMA register set */
#endif
    volatile DmaDesc *rxBds;            /* Memory location of rx bd ring */
    volatile DmaDesc *rxBdAssignPtr;    /* ptr to next rx bd to become full */
    volatile DmaDesc *rxBdReadPtr;      /* ptr to next rx bd to be processed */
    volatile DmaDesc *rxLastBdPtr;      /* ptr to last allocated rx bd */
    volatile DmaDesc *rxFirstBdPtr;     /* ptr to first allocated rx bd */
	Enet_Tx_CB *rxCbs;

    int             nrRxBds;            /* number of receive bds */
    int             rxBufLen;           /* size of rx buffers for DMA */

    uint16          chipId;             /* chip's id */
    uint16          chipRev;            /* step */
    int             rxIrq;              /* rx dma irq */
    int             phyAddr;            /* 1 - external MII, 0 - internal MII (default after reset) */
#ifdef USE_BH
    struct tq_struct task;              /* task queue */
#endif
    int             linkState;
	uint32			txdpa;		/* physical address of descriptor ring */
	uint32			rxdpa;		/* physical address of descriptor ring */
	BOOL			PciHostDmaReady;
    BOOL            PciResetDetected;

	/* These should be part of pDevCtrl, as they are not reset */
	int gNumResetsErrors;
	int gNumNoDescErrors;
	int gNumOverflowErrors;
	int gNumberOfOverflows;
	int gNoDescCount;

    /* Boot ioctl */
    volatile uint32  mailbox1;

    uint32 mbar3_mapped;
    uint32 mbar4_mapped;

    uint32 PciTxSetupPtr;
    uint32 PciRxSetupPtr;

	uint32 has_virt_dev;
	uint32 open_count;

	struct net_device **p_virt_dev;
} BcmEnet_devctrl;

typedef struct BcmEnet_bootparms {
    uint32          offset;
    uint32          length;
} BcmEnet_bootparms;

typedef struct BcmEnet_ioctlparms {
    uint32          from;
    uint32          to;
    uint32          length;
} BcmEnet_ioctlparms;

struct _vdev_names
{
    unsigned int tag;
    unsigned char *ifname;
    unsigned char macaddr[6];
};

struct _stb_eth_tag
{
    unsigned char len_h;
    unsigned char len_l;
    unsigned char magic;
    unsigned char if_num;
};

struct BcmVirtEnet_devctrl
{
    struct BcmEnet_devctrl *pDevCtrl;
    unsigned char  if_num;
} BcmVirtEnet_devctrl;

extern ushort tagged_vdevs;
 /*
 *  Th following defines list the tags to add to the different
 *  virtual ethernet interfaces for directing packets.
 *  A virtual eth interface is created for each tag.
 */
#define VDEV0_IFNUM 0x0
#define VDEV1_IFNUM 0x1
#define NUM_TAGS    0x02
#define TAG_MAGIC   0xAA
// BD macros
#define IncTxBDptr(x, s) if (x == ((BcmEnet_devctrl *)s)->txLastbdPtr) \
                             x = ((BcmEnet_devctrl *)s)->txFirstbdPtr; \
                      else x++

#define IncRxBDptr(x, s) if (x == ((BcmEnet_devctrl *)s)->rxLastBdPtr) \
                             x = ((BcmEnet_devctrl *)s)->rxFirstBdPtr; \
                      else x++

#define BcmHalInterruptDisable(irq) disable_irq(irq)
#define BcmHalInterruptEnable(irq)  enable_irq(irq)

#if !defined(CONFIG_MIPS_BCM7420) && !defined(CONFIG_BCM7420)

#define BCHP_PCI_CFG_PCI_SDRAM_ENDIAN_CTRL       0x0000026c /* PCI CFG PCI SDRAM Endian Control */
#define BCHP_PCI_CFG_STATUS_COMMAND              0x00000204 /* PCI CFG Status Command */
#define BCHP_PCI_CFG_GISB_BASE_W                 0x00000210 /* PCI CFG GISB Base Window */
#define BCHP_PCI_CFG_MEMORY_BASE_W0              0x00000214 /* PCI CFG Memory Base Window0 */
#define BCHP_PCI_CFG_MEMORY_BASE_W1              0x00000218 /* PCI CFG Memory Base Window1 */
#define BCHP_PCI_CFG_MEMORY_BASE_W2              0x0000021c /* PCI CFG Memory Base Window2 */
#define BCHP_GIO_REG_START                                 0x00400700

#endif /* !defined(CONFIG_MIPS_BCM7420) && !defined(CONFIG_BCM7420) */

#if defined(BCM7118)
#define NEW_PCI_7038_PHYS_MEM_WIN0_BASE   0x10000000
#else
#define NEW_PCI_7038_PHYS_MEM_WIN0_BASE   0x20000000
#endif


#define GPIO_BASE		PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_GIO_REG_START)
#define GPIO_DATA_HI	(GPIO_BASE+0x24)
#define GPIO_DIR_HI		(GPIO_BASE+0x28)
#define GPIO38			0x00000040
#define GPIO39			0x00000080
#define GPIO40			0x00000100
#define GPIO41			0x00000200
#define GPIO46			0x00004000
#define GPIO47			0x00008000
#define LOCAL static
#define BCMEND_STATUS int

#if !defined(BCM7118)
#define DMA_BUFF_DONE   PCI_DMA_DESC_DONE
#define DMA_DONE        PCI_DMA_PKT_DONE
#define DMA_NO_DESC     PCI_DMA_NOTMY_DESC
#define DMA_ENABLE      PCI_DMA_ENABLE
#endif

//********************** PCI Mailbox CMD IDs ************************************
// response code command bit
#define MBOX_RESPONSE_CODE     	0x80000000

#define RESERVED_FOR_REBOOT 	0x01000000
#define MBOX_CM_READY			0x02000000
#define MBOX_CM_NOT_READY		0x04000000
#define MBOX_WAKUP_CALL			0x08000000
#define MBOX_DMA_READY 			0x10000000
#define MBOX_CONFIG_COMPLETE	0x11000000
#define MBOX_REBOOTING          0x40000000
#define MBOX_DEBUG_PRINT_DESC	0x55000000

// PCI Mailbox CMD IDs for Bulk DMA boot
// NOTE:  SOME OF THESE CMDS MAY OVERRIDE THE DEFINITIONS LISTED ABOVE AS THE
//        BULK DMA BOOT MODE IS EXCLUSIVE FROM OTHER BOOT MODES.

// Tells the host-side that we're going to use the Bulk DMA method of booting
#define MBOX_BOOT_BULKDMA	0xFF000000
// Commands that tell the host-side which file to send.  Here's how the info
// is encoded in these commands:
// Bits 0,1:  File to move over, with the following mapping:
//            00 - Permanent NonVol
//            01 - Dynamic NonVol
//            02 - Program image header
//            03 - Program image
// Bits 31-2: Offset in the PCI Window where the file should be copied to.
#define MBOX_BULKDMA_P_NVOL     0xF0000000
#define MBOX_BULKDMA_D_NVOL     0xF1000000
#define MBOX_BULKDMA_IMG_HDR    0xF2000000
#define MBOX_BULKDMA_IMG        0xF3000000
#define MBOX_BULKDMA_IMG_FULL   0xF4000000
#if defined(BCM7118)
#define BULK_WIN_SIZE       	(128 * 1024)
#define MBOX_7118_REMAP_MEM		0xFA000000
#define MBOX_7118_MEM_SIZE		0xFB000000
#endif

//********************** Remote Flash Disk File ************************************
// This has to match the values defined by the docsis code
#define PERM_NONVOLMAXSIZE	(1024 * 128)
#define DYN_NONVOLMAXSIZE	(1024 * 128)

typedef struct BcmEnet_DocsisNonvolStruct {
    uint32          pDynmNonvol;
    uint32          DynmLen;
    uint32          pPermNonvol;
    uint32          PermLen;
} BcmEnet_DocsisNonvol;


//********************** NonVol parameters ************************************
// The two sections that can be read/written.  Typically, Permanent settings
// are only written during manufacturing, but Dynamic settings can be written
// at any time during normal operation of the system.
typedef enum
{
    kNonVolDriverDynamicSection = 0,
    kNonVolDriverPermanentSection = 1,
	kNonVolDriverSections = 2
} NonVolDriverSection;

// Result codes returned by various functions.
typedef enum
{
    kNonVolDriverFailure = 0,
    kNonVolDriverSuccess = 1
} NonVolDriverResult;

#define DEV_CTRL_FROM_DEV(dev) ((tagged_vdevs == 1) ? ((struct BcmVirtEnet_devctrl *)netdev_priv(dev))->pDevCtrl : ((struct BcmEnet_devctrl *)netdev_priv(dev)))

#endif /* _BCM7XXXPCIENET_H_ */
