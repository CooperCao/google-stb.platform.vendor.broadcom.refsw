//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       3368_map.h
//  Author:         Mike Sieweke
//  Creation Date:  December 9, 2004
//
//****************************************************************************
//  Description:
//      This file defines addresses of major hardware components of 3368
//
//****************************************************************************

#ifndef __BCM3368_MAP_H
#define __BCM3368_MAP_H

#if defined(BCM7118)
#define SUN_TOP_CTRL_RESET_CTRL 0xb0404008
#define SUN_TOP_CTRL_SW_RESET 0xb0404014
#define SUN_TOP_CTRL_TEST_PORT_CTRL 0xB0404018
#define GIO_DATA_HI 0xB0400724 

#define BNM_BOOTLOADER_RAM_ADDRESS 0xa4000000
#define BNM_MEMORY_ADDRESS_BASE 0x40000000

/* for RTS stuff */
#define MEMC_0_CLIENT_INFO_10_BNM_MIPS 0xb010602c
#define MEMC_0_CLIENT_INFO_11_BNM_UBUS 0xb0106030
#define MEMC_0_CLIENT_INFO_12_BNM_DMA 0xb0106034
#define MEMC_0_REQ_MASK_LOW 0xb0106180
#define MEMC_0_REQ_MASK_HIGH 0xb0106184

/* BSP registers */
#define AEGIS_COMMAND_BUFFER 0xb0307980
#define AEGIS_RESPONSE_BUFFER 0xb0307c80
#define BSP_GLB_NONSECURE_GLB_IRDY 0xb030b008
#define BSP_GLB_NONSECURE_GLB_HOST_INTR_STATUS 0xb030b010
#define BSP_GLB_NONSECURE_GLB_OLOAD2 0xb030b020
#define BSP_GLB_NONSECURE_GLB_ILOAD2 0xb030b028

/* BNM registers */
#define INT_IRQMASK 0xfff8c00c
#define INT_IRQSTATUS 0xfff8c010
#define INT_IRQSTATUS1 0xfff8c03c
#define INT_IRQMASK1 0xfff8c038

#define NSGISB_MBOX_INT 0xfff8a028
#define MBOX_MBOX_INT 0xfff8c074
#define MBOX_MAILBOX_1 0xfff8c07c

#define MEM2MEM_DMA_RX0_INTERRUPT_MASK 0xfffb0908
#define MEM2MEM_DMA_RX0_INTERRUPT_STATUS 0xfffb0904
#define MEM2MEM_DMA_TX1_INTERRUPT_STATUS 0xfffb0934

#define MEM2MEM_CTRL_CONFIG0 0xfffb0c00
#define MEM2MEM_CTRL_CONFIG1 0xfffb0d00

#define SUB_R5F_IRQ_MASK_STATUS 0xfff8d144
#define SUB_PCI_IRQ_MASK_STATUS 0xfff8d140
#define SUB_IRQ_MASK_STATUS 0xfff8d008

#define NSGISB_HOSTIRQMASK 0xfff8a018
#define NSGISB_HOSTIRQSTATUS 0xfff8a01c

/* Host interrupt registers */
#define HIF_CPU_INTR1_INTR_W1_STATUS 0xb0001404
#define HIF_CPU_INTR1_INTR_W1_MASK_STATUS  0xb000140c
#define HIF_CPU_INTR1_INTR_W1_MASK_SET 0xb0001414
#define HIF_CPU_INTR1_INTR_W1_MASK_CLEAR  0xb000141c
/* HIF_CPU_INTR1 :: INTR_W1_MASK_STATUS :: XPT_PCR_CPU_INTR [25:25] */
#define BCHP_HIF_CPU_INTR1_INTR_W1_MASK_STATUS_DOCSIS_CPU_INTR_MASK 0x02000000
#define BCHP_HIF_CPU_INTR1_INTR_W1_MASK_STATUS_DOCSIS_CPU_INTR_SHIFT 25
#define BCM_LINUX_BNM_IRQ	(1+32+BCHP_HIF_CPU_INTR1_INTR_W1_MASK_STATUS_DOCSIS_CPU_INTR_SHIFT)

/* UBUS to SCB address translation window base */
#define PCIHOST_SDRAM_LOCAL_BASE	0x18000000

#define	PCI_DMA_ENABLE		DMA_ENABLE
#define PCI_DMA_NOTMY_DESC  0x00000004
#define PCI_DMA_PKT_DONE    0x00000002
#define PCI_DMA_DESC_DONE   0x00000001
#define PCI_MBOX1_RCV_INTSTATUS_BIT  (1<<1)

// Interrupt Controller
typedef struct IntControl
{
    uint32  RevID;                  // (00)
        #define CHIPID          0x711800a0
    uint32  blkEnables;             // (04)
        //#define EPHY_CLK_EN 	(1<<21)
        //#define USBSU_CLK_EN 	(1<<20)
        //#define EMAC2_CLK_EN 	(1<<19)
        #define EMAC_CLK_EN 	(1<<18)
        #define EMUSB_CLK_EN 	(1<<17)
        #define DISABLE_GLESS   (1<<16)
        #define SCB_BRIDGE_CLK_EN (1<<15)
        //#define ACP_B_CLK_EN    (1<<14)
        //#define DS2_TOP_CLK_EN  (1<<13)
        #define DS1_TOP_CLK_EN  (1<<12)
        #define DS_TOP_CLK_EN   (1<<11)
        //#define USBS_CLK_EN     (1<<10)
        #define SPI_CLK_EN      (1<< 9)
        //#define APM_CLK_EN      (1<< 8)
        #define OOB_CLK_EN   	(1<< 7)
        #define US_TOP_CLK_EN   (1<< 6)
        #define TC_CLK_EN       (1<< 5)
        #define DAVIC_CLK_EN    (1<< 4)
        #define MAC_CLK_EN      (1<< 3)
        //#define SDR_CLK_EN      (1<< 2) // Reserved

    // This is named pll_control for consistency with other chips, but
    // there is no PLL control register on the 3255.  The only defined
    // bit is for soft reset.
    uint32  pll_control;            // (08)
        #define SOFT_RESET      0x00000001

    uint32  IrqMask;                // (0c)
    uint32  IrqStatus;              // (10)
        #define  SCB_BRIDGE_IRQ (1<<31)
        #define  HMACCNTLIRQ    (1<<30)
        #define  HMACDATAIRQ    (1<<29)
        //#define  APM5DMA        (1<<28)
        //#define  APM4DMA        (1<<27)
        //#define  APM3DMA        (1<<26)
        //#define  APM2DMA        (1<<25)
        //#define  APM1DMA        (1<<24)
        //#define  APM0DMA        (1<<23)
        //#define  APMIRQ         (1<<22)
        #define  HOST_WD_IRQ    (1<<21)
        #define  DS2IRQ2        (1<<20)
        #define  DS2IRQ         (1<<19)
        #define  DS1IRQ2         (1<<18)
        #define  DS1IRQ         (1<<17)
        //#define  DSAIIRQ        (1<<19)
        //#define  EMAC2TXDMA     (1<<18)
        //#define  EMAC2RXDMA     (1<<17)
        #define  EMACTXDMA      (1<<16)
        #define  EMACRXDMA      (1<<15)
        #define  UBUS_MBOX1_IRQ (1<<14)
        #define  UBUS_MBOX0_IRQ (1<<13)
        #define  GISB_MBOX1_IRQ (1<<12)
        #define  GISB_MBOX0_IRQ (1<<11)
        #define  TCIRQ          (1<<10)
        #define  EPHYIRQ        (1<< 9)
        #define  ETHIRQ         (1<< 8)
        #define  OOBIRQ         (1<< 7)
        #define  DAVICIRQ       (1<< 6)
        #define  USIRQ          (1<< 5)
        //#define  reservedIRQ    (1<< 4)
        //#define  UART1IRQ       (1<< 3)
        #define  UART0IRQ       (1<< 2)
        #define  SPIIRQ         (1<< 1)
        #define  TIMRIRQ        (1<< 0)

    byte    unused3;                // (14)

    // Upper 4 bits RSVD, Lower 4 bits set disable edge sensitivity
    byte    ExtEdgeInSense;         // (15)

    // Upper 4 bits enable corresponding ExtIrq, Lower 4 bits clr IRQ
    byte    extIrqMskandDetClr;     // (16)

    // Upper 4 bits are current status of input, lower bits define edge
    // sensitivity.  If high, IRQ on rising edge. If low, IRQ on falling edge
    byte    extIrqStatEdgeConfig;   // (17)

    // Diagnostic and MBIST control
    uint32  DiagControl;            // (18)
    uint32  unused05;               // (1c)

    // Diagnostic mux and phase select control
    uint32  DiagSelControl;         // (20)

    // Read the state of diagnostic port bits 31:0
    uint32  DiagReadback;           // (24)

    // Read the state of diagnostic port bits 36:32
    uint32  DiagReadbackHi;         // (28)

    // Miscellaneous control
    uint32  DiagMiscControl;        // (2c)
        #define EPHY_RESET_N    (1<<8)
        #define DSPHY_BYP       (1<<1)

    // Device timeout enable
    uint32  unused4;        // (30)
//        #define APM_DEV_TIMEOUT_EN   (1<<8)
//        #define EMAC_DEV_TIMEOUT_EN  (1<<4)
//        #define MAC_DEV_TIMEOUT_EN   (1<<3)

    // Block level soft resets
    uint32  SoftResetB;             // (34)
        #define SOFT_RESET_TC   (1<<16)
        #define SOFT_RST_OOB    (1<<15)
        #define SOFT_RST_USPHY    (1<<14)
        #define SOFT_RST_DAVIC    (1<<13)
        #define SOFT_RST_DAVIC_IUDMA    (1<<12)
        #define SOFT_RST_DS1   (1<<11)
        #define SOFT_RST_DS0  (1<<10)
        #define SOFT_RST_EMAC1  (1<< 9)
        #define SOFT_RST_EMUSB_IUDMA  (1<< 8)
        #define SOFT_RST_HMAC   (1<< 7)
        #define SOFT_RST_EPHY   (1<< 6)
        #define SOFT_RST_FPM    (1<< 5)
        #define SOFT_RST_UBUS_ARB (1<<4)
        #define SOFT_RST_SCB_BRIDGE (1<< 3)
        #define SOFT_RST_EMAC   (1<< 2)
        #define SOFT_RST_SDRAM    (1<< 1)
        #define SOFT_RST_SPI    (1<< 0)

#if 0
#define SOFT_RST_BMU    (1<<15)
#define SOFT_RST_HVG    (1<<14)
#define SOFT_RST_PCM    (1<<13)
#define SOFT_RST_APM    (1<<12)
#define SOFT_RST_USBS   (1<<11)
#define SOFT_RST_EMAC2  (1<<10)
#define SOFT_RST_EMAC1  (1<< 9)
#define SOFT_RST_EMUSB_IUDMA  (1<< 8)
#define SOFT_RST_HMAC   (1<< 7)
#define SOFT_RST_EPHY   (1<< 6)
#define SOFT_RST_FPM    (1<< 5)
#define SOFT_RST_MPI    (1<< 3)
#define SOFT_RST_EMAC   (1<< 2)
#define SOFT_RST_ACP    (1<< 1)
#define SOFT_RST_SPI    (1<< 0)
#endif

    // Mask and status registers for thread processor 1
    uint32  IrqMask1;               // (38)
    uint32  IrqStatus1;             // (3c)

} IntControl;

#define INTC ((volatile IntControl * const) 0xfff8c000)

// SUB: SCB Bridge Control Registers
typedef struct SUBRegisters
{
    uint32  subCtrl;    
    uint32  subXferSize;
    uint32  subIrqStatus;        
    uint32  subErrMask;
    uint32  subErrStatus;    
    uint32  subDiagCtrl;
    uint32  subDiag;        
    uint32  subMipsXferSize; 
	uint8   pad0[0xfff8d050 - 0xfff8d020];
    uint32  subRtTxCtrl;        
    uint32  subRtRxCtrl;        
    uint32  subSlv3TxCtrl;        
    uint32  subSlv3RxCtrl;        
    uint32  subMipsTxCtrl; // 0xfff8d060        
    uint32  subMipsRxCtrl;        
	uint8   pad1[0xfff8d078 - 0xfff8d068];
    uint32  subScbReq2AckCtrl;        
    uint32  subScbReq2AckTimeout;        
    uint32  subTotalSlv3TxReq2AckTime;
    uint32  subMaxSlv3TxReq2AckTime;
    uint32  subTotalSlv3TxAck2DoneTime;
    uint32  subMaxSlv3TxAck2DoneTime;
    uint32  subTotalSlv3TxReqs;
    uint32  subTotalSlv3RxReq2AckTime;
    uint32  subMaxSlv3RxReq2AckTime;
    uint32  subTotalSlv3RxAck2DoneTime;
    uint32  subMaxSlv3RxAck2DoneTime;
    uint32  subTotalSlv3RxReqs;
	uint8   pad2[0xfff8d0c0 - 0xfff8d0a8];
    uint32  subTotalRtTxReq2AckTime;
    uint32  subMaxRtTxReq2AckTime;
    uint32  subTotalRtTxAck2DoneTime;
    uint32  subMaxRtTxAck2DoneTime;
    uint32  subTotalRtTxReqs;
    uint32  subTotalRtRxReq2AckTime;
    uint32  subMaxRtRxReq2AckTime;
    uint32  subTotalRtRxAck2DoneTime;
    uint32  subMaxRtRxAck2DoneTime;
    uint32  subTotalRtRxReqs;
	uint8   pad3[0xfff8d100 - 0xfff8d0e8];
    uint32  subTotalMipsTxReq2AckTime;
    uint32  subMaxMipsTxReq2AckTime;
    uint32  subTotalMipsTxAck2DoneTime;
    uint32  subMaxMipsTxAck2DoneTime;
    uint32  subTotalMipsTxReqs;
    uint32  subTotalMipsRxReq2AckTime;
    uint32  subMaxMipsRxReq2AckTime;
    uint32  subTotalMipsRxAck2DoneTime;
    uint32  subMaxMipsRxAck2DoneTime;
    uint32  subTotalMipsRxReqs;
	uint8   pad4[0xfff8d140 - 0xfff8d128];
    uint32  subPciIrqMaskStatus;
    uint32  subR5fIrqMaskStatus; // 0xfff8d144
    uint32  subMem64KCtrl;
} SUBRegisters;

#define SUBC_BASE 0xfff8d000
#define SUBC_SIZE (0xfff8d14C - 0xfff8d000)
#define SUBC ((volatile SUBRegisters * const) SUBC_BASE)

// NSGISB: Non-secure GISB Registers
typedef struct NSGISBRegisters
{
    uint32  mBox0;    
    uint32  mBox1;
    uint32  mipsCtrl;        
    uint32  bootCtrl;
    uint32  rstCtrl;    
    uint32  mipsSftRst;
    uint32  hostIrqMask;        
#define SCB_BRIDGE_IRQ_MASK 0x80000000
#define GISB_MBOX1_IRQ_MASK 0x00001000
#define GISB_MBOX0_IRQ_MASK 0x00000800
    uint32  hostIrqStatus;        
    uint32  pciIrqMask;
    uint32  pciIrqStatus;
	uint32  mboxInt;
} NSGISBRegisters;

#define NSGISBC_BASE 0xfff8a000
#define NSGISBC_SIZE (0xfff8a028 - 0xfff8a000)
#define NSGISBC ((volatile NSGISBRegisters * const) NSGISBC_BASE)

// MEM2MEM_CTRL: MEM2MEM Control Registers
typedef struct M2MCtrlRegisters
{
    uint32  m2mCtrlCfg;    
    uint32  m2mCtrlFifoDbg;
    uint8   pad[0x80 - 0x08];        
    uint32  m2mCtrlFifoMem[32];
} M2MCtrlRegisters;

// MEM2MEM_DMA_CTRL: DMA Control Registers
typedef struct M2MDmaCtrlRegisters
{
    uint32  m2mDmaCtrlCfg;    
    uint32  m2mDmaCtrlCh1FcLowTh;
    uint32  m2mDmaCtrlCh1FcHighTh;
    uint32  m2mDmaCtrlCh1FcBufAlloc;
    uint32  m2mDmaCtrlCh3FcLowTh;
    uint32  m2mDmaCtrlCh3FcHighTh;
    uint32  m2mDmaCtrlCh3FcBufAlloc;
    uint32  m2mDmaCtrlChReset;    
    uint32  m2mDmaCtrlChDbg;    
} M2MDmaCtrlRegisters;

// Each DMA channel in a particular core has this set of registers
// to configure and control the DMA channel behavior.
typedef struct IuDmaCfgRegisters
{
    uint32  Control;           // (00) assorted configuration
        #define DMA_ENABLE      0x01
        #define DMA_PKT_HALT    0x02
        #define DMA_BURST_HALT  0x04
    uint32  IntStat  ;        // (04) interrupts control and status
    uint32  IntMask;          // (08) interrupts mask
        #define DMA_BUFF_DONE   0x01
        #define DMA_DONE        0x02
        #define DMA_NO_DESC     0x04
    uint32  MaxBurst;         // (0c) max burst length permitted

} IuDmaCfgRegisters;
    
// Each DMA channel in a particular core has this set of registers
// to assign and track memory states and resources. Since this block
// is not contiguous with the above block in every core, they are 
// broken up into to distinct register blocks.
typedef struct IuDmaStateRamRegisters
{
    uint32  DescriptorBase;   // (100)
    uint32  StateRam2;        // (104)
    uint32  StateRam3;        // (108)
    uint32  StateRam4;        // (10c)

} IuDmaStateRamRegisters;

//This structure defines the highest level hierarchy
// MEM2MEM_BLOCK and associated DMA channels.
typedef struct M2MBlockRegisters
{

   //Register block for the #1 ethernet mac core. 	   
   struct M2MCtrlRegisters           M2MCtrl;        //0xfffb0000
	uint8  Pad1[((0x0800 - 0x0000) - (sizeof(M2MCtrlRegisters)))];

   //Register block for the master control of the emac/usb core dma engine.
   struct M2MDmaCtrlRegisters		 M2MDmaCtrl;   //0xfffb0800 
   uint8  Pad4[((0x0900 - 0x0800) - (sizeof(M2MDmaCtrlRegisters)))];
   
   //Control register blocks for each individual DMA channel.
   struct IuDmaCfgRegisters         IuDmaConfigRegisters[4];   //0xfffb0900 
   uint8  Pad5[((0x0a00 - 0x0900) - (sizeof(IuDmaCfgRegisters)*4))];
   // The m2m DMA channels have TX0 and RX0 connected by a FIFO, TX1 and RX0 connected by a FIFO
   //
   //        |-------TX0(1) <--> RX0(0)--------|
   //		SCB								  UBUS
   //		 |-------RX1(2) <--> TX1(3)--------|
   //
		#define SUB_DMA_SCB_UBUS2SCB 2 
        #define SUB_DMA_SCB_SCB2UBUS 1
        #define SUB_DMA_UBUS_SCB2UBUS 0
        #define SUB_DMA_UBUS_UBUS2SCB 3
                                                               
   //RAM state register blocks for each individual DMA channel.
   struct IuDmaStateRamRegisters    IuDmaStateRegisters[4];    //0xfffb0a00 

} M2MBlockRegisters;

#define M2MDMAC_BASE 0xfffb0000
#define M2MDMAC_SIZE (0xfffb0a40 - 0xfffb0000)
#define M2MDMAC ((volatile M2MBlockRegisters * const) M2MDMAC_BASE)

#else
typedef struct DMACONFIG
{
    uint32  Control;
		#define	PCI_DMA_LE			0x00000200
		#define	PCI_DMA_NOSWAP		0x00000100
		#define	PCI_DMA_HALT		0x00000008
		#define	PCI_DMA_PKTSTALL 	0x00000004
		#define	PCI_DMA_STALL		0x00000002
		#define	PCI_DMA_ENABLE		0x00000001
    uint32  IntStat;
    uint32  IntMask;
        #define PCI_DMA_NOTMY_DESC  0x00000004
        #define PCI_DMA_PKT_DONE    0x00000002
        #define PCI_DMA_DESC_DONE   0x00000001

    uint32  unused1;
    uint32  DescriptorAddress;
    uint32  Status1;
    uint32  Status2;
    uint32  unused2;
} LOCDmaConfig;

typedef struct PCIDMACONFIG
{
    uint32  Control;
    uint32  IntStat;
    uint32  IntMask;
    uint32  DescriptorAddress;
    uint32  Status1;
    uint32  Status2;
} PCIDmaConfig;

typedef struct MpiRegisters
{
    struct
    {
        uint32  Range;
        uint32  Remap;
        uint32  Config;
    } PciToSysBusAddressSpace[2];   // 00
    uint8   unused1[4];             // 18
    uint32  SysBusToPciIoControl;   // 1c
    struct
    {
        uint32  Range;
        uint32  Base;
        uint32  Remap;
    } SysBusToPciAddressSpace[3];   // 20
    uint32  PciModeSelect;          // 44
    uint32  PciIntStat;             // 48
    uint32  PciLocBusControl;       // 4c
    uint32  PciLocIntMaskStatus;    // 50
      #define PCI_LOCINTMASK_MASK         0x03ef0000
      #define PCI_EXTERNAL_INTMASK_BIT     (1<<23)
      #define PCI_MBOX1_SEND_INTENB_BIT    (1<<19)
      #define PCI_MBOX0_SEND_INTENB_BIT    (1<<18)
      #define PCI_MBOX1_RCV_INTENB_BIT     (1<<17)
      #define PCI_MBOX0_RCV_INTENB_BIT     (1<<16)
      #define PCI_EXTERNAL_INTSTATUS_BIT   (1<<7)
      #define PCI_MBOX1_SEND_INTSTATUS_BIT (1<<3)
      #define PCI_MBOX0_SEND_INTSTATUS_BIT (1<<2)
      #define PCI_MBOX1_RCV_INTSTATUS_BIT  (1<<1)
      #define PCI_MBOX0_RCV_INTSTATUS_BIT  (1<<0)
    uint8   unused2[0x70-0x54];     // 54
    uint32  MailBox0;               // 70
    uint32  MailBox1;               // 74
    uint32  L2PCfgCntrl;            // 78
    uint32  L2PCfgData;             // 7c
    LOCDmaConfig LocDmaConfig[2]; // 80, A0
        #define MPI_DMA_LOC_PCI2UBUS 0
        #define MPI_DMA_LOC_UBUS2PCI 1
    PCIDmaConfig PciDmaConfig[2];   // c0, d8
        #define MPI_DMA_PCI_UBUS2PCI 0
        #define MPI_DMA_PCI_PCI2UBUS 1
    uint32  VendorId;               // f0
    uint32  RevisionId;             // f4
} MpiRegisters;

#define MPIC ((volatile MpiRegisters * const) MPIC_BASE)
#endif

// DMA buffer descriptor
typedef struct DmaDesc
{
    uint16  length;     // RX - buffer size, TX - # bytes in buffer (bits 12:00 valid for length)
        #define DMA_USE_FREEPOOL (1<<15)  // Use Free Pool Manager to allocate (Rx) or deallocate (Tx) buffer

    uint16  status;     // buffer status
        #define DMA_OWN         (1<<15)  // cleared by DMA, set by SW
        #define DMA_EOP         (1<<14)  // last buffer in packet
        #define DMA_SOP         (1<<13)  // first buffer in packet
        #define DMA_WRAP        (1<<12)  // end of buffer ring
        #define	DMA_APPEND_CRC  (1<< 8)  // Tx append CRC
        // EMAC Descriptor Status definitions
        #define EMAC_UNDERRUN   (1<< 9)  // Tx underrun
        #define EMAC_MISS       (1<< 7)  // framed address recognition failed (promiscuous)
        #define EMAC_BRDCAST    (1<< 6)  // DA is Broadcast
        #define EMAC_MULT       (1<< 5)  // DA is multicast
        #define EMAC_LG         (1<< 4)  // frame length > RX_LENGTH register value
        #define EMAC_NO         (1<< 3)  // Non-Octet aligned
        #define EMAC_RXER       (1<< 2)  // RX_ERR on MII while RX_DV assereted
        #define EMAC_CRC_ERROR  (1<< 1)  // CRC error
        #define EMAC_OV         (1<< 0)  // Overflow

    uint32  address;    // address of data
} DmaDesc;
#define  DMA_DESC_LENGTH   0
#define  DMA_DESC_STATUS   2
#define  DMA_DESC_ADDR     4

#define DCACHE_LINE_SIZE 16         // line size for dcache

#endif
