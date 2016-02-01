/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BOARDMAP_H
#define BOARDMAP_H

/*****************************************************************************/
/*                    Include common chip definitions                        */
/*****************************************************************************/
#include "bcmmips.h"
#include "bchp_common.h"

/*****************************************************************************/
/*                    MIPS Physical Memory Map                               */
/*****************************************************************************/
#define CPU_PHYS_SDRAM_BASE	        0x00000000	/* SDRAM Base */
#define CPU_PHYS_ROM_BASE           0x1FC00000	/* ROM */
#define CPU_PHYS_FLASH_BASE         0x1C000000
#define CPU_PHYS_FPGA_BASE          0x1A000000
#define CPU_PHYS_1394_BASE          0x19000000
#define CPU_PHYS_POD_BASE			0x19800000

/*****************************************************************************/
/*                   CPU to PCI Bridge Memory Map                            */
/*****************************************************************************/

#define CPU2PCI_CPU_PHYS_MEM_WIN_BASE     0xd0000000

/* Allow CPU to access PCI memory addresses 0xd0000000 to 0xdfffffff */
#define CPU2PCI_PCI_PHYS_MEM_WIN0_BASE    0xd0000000 /* Not used in A0 */
#define CPU2PCI_PCI_PHYS_MEM_WIN1_BASE    0xd8000000 /* Not used in A0 */
#define CPU2PCI_PCI_PHYS_MEM_WIN2_BASE    0xe0000000
#define CPU2PCI_PCI_PHYS_MEM_WIN3_BASE    0xe8000000

/* Allow CPU to access PCI I/O addresses 0xe0000000 to 0xe05fffff */
#if (BRCM_ENDIAN_LITTLE == 1)
#define CPU2PCI_PCI_PHYS_IO_WIN0_BASE     0x00000000
#define CPU2PCI_PCI_PHYS_IO_WIN1_BASE     0x00200000
#define CPU2PCI_PCI_PHYS_IO_WIN2_BASE     0x00400000
#else
#define CPU2PCI_PCI_PHYS_IO_WIN0_BASE     0x00000002
#define CPU2PCI_PCI_PHYS_IO_WIN1_BASE     0x00200002
#define CPU2PCI_PCI_PHYS_IO_WIN2_BASE     0x00400002
#endif


/*****************************************************************************/
/*                      PCI Physical Memory Map                              */
/*****************************************************************************/

/* PCI physical memory map */
#define PCI_7401_PHYS_ISB_WIN_BASE    0x10000000
#if BCHP_CHIP==7335 || BCHP_CHIP==7405 || (BCHP_CHIP == 7400 && (defined(BCHP_REV_B0) || defined(BCHP_REV_C0)|| defined(BCHP_REV_D0)))
#define PCI_7401_PHYS_MEM_WIN0_BASE   0x00000001
#else
#define PCI_7401_PHYS_MEM_WIN0_BASE   0x00000000
#endif
#define PCI_7401_PHYS_MEM_WIN1_BASE   0x02000000
#define PCI_7401_PHYS_MEM_WIN2_BASE   0x04000000

#define PCI_1394_PHYS_MEM_WIN0_BASE   0xd0000000
#if BCM_BOARD==97456 || CFG_ECM==1
#define PCI_3255_PHYS_REG_WIN0_BASE   0xd1000000
#define PCI_3255_PHYS_MEM_WIN0_BASE   0xd8000000
#define PCI_3255_PHYS_MEM_WIN1_BASE   0xd9000000
#endif

#define PCI_DEVICE_ID_EXT       0x0d
#define PCI_DEVICE_ID_1394      0x0e
#define PCI_DEVICE_ID_MINI      0x04
#define PCI_DEVICE_ID_SATA      0 /* On 2ndary PCI bus */
#if BCM_BOARD==97456 || CFG_ECM==1
#define PCI_DEVICE_ID_3255		0x7 /* PCI AD23; PCI AD16=0, AD17=1... */
#endif

#define PCI_IDSEL_EXT           (0x10000 << PCI_DEVICE_ID_EXT)
#define PCI_IDSEL_1394          (0x10000 << PCI_DEVICE_ID_1394)
#define PCI_IDSEL_MINI          (0x10000 << PCI_DEVICE_ID_MINI)
#define PCI_IDSEL_SATA          (0x10000 << PCI_DEVICE_ID_SATA)

#define PCI_DEV_NUM_EXT         (PCI_DEVICE_ID_EXT  << 11)
#define PCI_DEV_NUM_1394        (PCI_DEVICE_ID_1394 << 11)
#define PCI_DEV_NUM_MINI        (PCI_DEVICE_ID_MINI << 11)
#define PCI_DEV_NUM_SATA        (PCI_DEVICE_ID_SATA << 11)
#if BCM_BOARD==97456 || CFG_ECM==1
#define PCI_DEV_NUM_3255        (PCI_DEVICE_ID_3255 << 11)
#endif

/* SATA device */
#define PCS0_OFS				0x200
#define PCS1_OFS				0x240
#define SCS0_OFS				0x280
#define SCS1_OFS				0x2c0
#define BM_OFS					0x300
#define MMIO_OFS				0xb0510000
#define PCI_SATA_PHYS_REG_BASE	(0xb0520000 + PCS0_OFS)

/*****************************************************************************/
/*                      MIPS Virtual Memory Map                              */
/*                                                                           */
/* Note that the addresses above are physical addresses and that programs    */
/* have to use converted addresses defined below:                            */
/*****************************************************************************/
#define DRAM_BASE_CACHE		BCM_PHYS_TO_K0(CPU_PHYS_SDRAM_BASE)   /* cached DRAM */
#define DRAM_BASE_NOCACHE	BCM_PHYS_TO_K1(CPU_PHYS_SDRAM_BASE)   /* uncached DRAM */
#define ROM_BASE_CACHE		BCM_PHYS_TO_K0(CPU_PHYS_ROM_BASE)
#define ROM_BASE_NOCACHE	BCM_PHYS_TO_K1(CPU_PHYS_ROM_BASE)
#define FLASH_BASE_NOCACHE  BCM_PHYS_TO_K1(CPU_PHYS_FLASH_BASE)
#define FPGA_BASE_NOCACHE   BCM_PHYS_TO_K1(CPU_PHYS_FPGA_BASE)
#define IEEE1394_BASE_NOCACHE   BCM_PHYS_TO_K1(CPU_PHYS_1394_BASE)

#define PCI_MEM_WIN_BASE    0xd0000000
#define PCI_MEM_WIN_SIZE    0x10000000
#define PCI_IO_WIN_BASE     0xf0000000
#define PCI_IO_WIN_SIZE     0x00600000



/*****************************************************************************/
/* Include chip specific .h files                                            */
/*****************************************************************************/



#endif
