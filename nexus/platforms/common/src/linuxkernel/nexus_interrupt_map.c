/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* API Description:
*
***************************************************************************/
#include "nexus_platform_module.h"
#include "nexus_interrupt_map.h"
#include "nexus_platform_priv.h"

/*
Hydra Linux interrupt numbering scheme:

The linux kernel has a virtual numbering scheme. We'll refer to it as "linuxirq".
The Hydra has two level one interrupt registers: W0 and W1. They are both 32 bits, but
not all bits are using.
The following code is a mapping between the virtual linuxirq and the physical W0 and W1
registers. The NEXUS_Platform_P_IrqMap structure uses the physical bit number as an index, treating
W0 and W1 as a 64 bit register.
Therefore NEXUS_Platform_P_IrqMap[physical_bit].linux_irq will obtain the virtual linuxirq.

WARNING: The virtual numbering scheme skips over the unused bits. Unfortunately,
these bits have and will become used in future versions of the chip, therefore the
following mapping is likely to break in the future. It's easy to fix it if you understand
the mapping.

You can see that W0 bits 29, 30, and 31 cannot be used with the current scheme. They
are currently unused. They may be used in the future.

linuxirq 0 is unused.
linuxirq 1 corresponds to W0 bit 0.
linuxirq 2 corresponds to W0 bit 1.
...
For 7038Bx:
  linuxirq 29 corresponds to W0 bit 28.
  linuxirq 30 corresponds to W1 bit 0.
  linuxirq 31 corresponds to W1 bit 1.
For 7038Cx:
  linuxirq 29 corresponds to W0 bit 28.
  linuxirq 30 corresponds to W0 bit 29.
  linuxirq 31 corresponds to W1 bit 0.
  linuxirq 32 corresponds to W1 bit 1.
For 3560Ax:
  linuxirq 29 corresponds to W0 bit 28.
  linuxirq 30 corresponds to W0 bit 29.
  linuxirq 31 corresponds to W0 bit 30.
  linuxirq 32 corresponds to W0 bit 31.
  linuxirq 33 corresponds to W1 bit 0.
  linuxirq 34 corresponds to W1 bit 1.
*/
#if ((BCHP_CHIP==7346) || (BCHP_CHIP==73465))
struct NEXUS_Platform_P_IrqMap NEXUS_Platform_P_IrqMap[32*NEXUS_NUM_L1_REGISTERS] = {
    {1,  false, true, "AIO"}, /* 0*/
    {2,  false, true, "BSP"},
    {3,  false, true, "BVNB"},
    {4,  false, true, "BVNF0"},
    {5,  false, true, "BVNF1"},
    {6,  false, true, "BVNF5"},
    {7,  false, true, "BVNM"},
    {8,  false, true, "CLKGEN"},
    {9, false, true, "EXT_IRQ0"},
    {10, false, true, "EXT_IRQ1"},
    {11, false, true, "EXT_IRQ2"},
    {12, false, true, "EXT_IRQ3"},
    {13, false, true, "EXT_IRQ4"},
    {0,  false, true, "EXT_IRQ5"},
    {0, false, true, "EXT_IRQ6"},
    {0,  false, true, "EXT_IRQ7"},
    {0, false, true, "EXT_IRQ8"},
    {18,  false, true, "EXT_IRQ9"},
    {0, false, true, "EXT_IRQ10"},
    {0,  false, true, "EXT_IRQ11"},
    {0, false, true, "EXT_IRQ12"},
    {0,  false, true, "EXT_IRQ13"},
    {0, false, true, "EXT_IRQ14"}, /*25*/
    {24,  false, true, "FTM"},   /*24*/
    {0,  false, true, "GENET0_A"},
    {0,  false, true, "GENET0_B"},
    {0,  false, true, "GENET1_A"},
    {0,  false, true, "GENET1_B"},
    {29, false, true, "GFX"},
    {30, false, true, "HDMI_TX"},
    {0,  false, true, "HIF"},
    {0, false, true, "HIF_SPI"},  /* 32*/
    {33, false, true, "IPI0"},
    {34, false, true, "IPI1"},
    {35, false, true, "MEMC0"},
    {0,  false, true, "MOCA"},
    {37, false, true, "NMI"},
    {38, false, true, "RAAGA"},
    {39, false, true, "RAAGA_FW"},
    {0,  false, true, "SATA0"},
    {0,  false, true, "SATA1"},
    {42,  false, true, "SDS0_AFEC"},
    {43,  false, true, "SDS0_RCVR0"},
    {44,  false, true, "SDS0_RCVR1"},
    {45,  false, true, "SDS0_TFEC"},
    {46,  false, true, "SDS1_AFEC"},
    {47,  false, true, "SDS1_RCVR0"},
    {48,  false, true, "SDS1_RCVR1"},
    {49,  false, true, "SDS1_TFEC"},
    {50, false, true, "SM"},
    {51, false, true, "SVD0"},
    {52, false, true, "SYS"},
    {53, false, true, "SYS_AON"},
    {0, false, true, "SYS_PM"},
    {55, false, true, "UHF"},
    {56, false, true, "UPG_AUX"},
    {57, false, true, "UPG_AUX_AON"},
    {58, false, true, "UPG_BSC"},
    {59, false, true, "UPG_BSC_AON"},
    {60, false, true, "UPG_MAIN"},
    {61, false, true, "UPG_MAIN_AON"},
    {62, false, true, "UPG_SC"},
    {63, false, true, "UPG_SPI"},
    {64, false, true, "UPG_TMR"},
    {0,  false, true, "UPG_UART0"},
    {0,  false, true, "UPG_UART1"},
    {0,  false, true, "UPG_UART2"},
    {0,  false, true, "USB0"}, /*68*/
    {0,  false, true, "USB0_EHCI0"},
    {0,  false, true, "USB0_EHCI1"},
    {0,  false, true, "USB0_OHCI0"},
    {0,  false, true, "USB0_OHCI1"},
    {0,  false, true, "USB1"},
    {0,  false, true, "USB1_EHCI0"},
    {0,  false, true, "USB1_EHCI1"},
    {0,  false, true, "USB1_OHCI0"},
    {0,  false, true, "USB1_OHCI1"}, /*77*/
    {78, false, true, "VEC"},
    {79,  false, true, "XPT_FE"},
    {80,  false, true, "XPT_MSG"},
    {81,  false, true, "XPT_MSG_STAT"},
    {82,  false, true, "XPT_OVFL"},
    {83,  false, true, "XPT_PCR"},
    {84,  false, true, "XPT_RAV"},
    {85,  false, true, "XPT_STATUS"}, /*85*/
    {0,   false, false, NULL}
};
#elif (BCHP_CHIP==7344)
struct NEXUS_Platform_P_IrqMap NEXUS_Platform_P_IrqMap[32*NEXUS_NUM_L1_REGISTERS] = {
    {1,  false, true, "AIO"}, /* 0*/
    {2,  false, true, "BSP"},
    {3,  false, true, "BVNB"},
    {4,  false, true, "BVNF0"},
    {5,  false, true, "BVNF1"},
    {6,  false, true, "BVNF5"},
    {7,  false, true, "BVNM"},
    {8,  false, true, "CLKGEN"},
    {9, false, true, "EXT_IRQ0"},
    {10, false, true, "EXT_IRQ1"},
    {11, false, true, "EXT_IRQ2"},
    {12, false, true, "EXT_IRQ3"},
    {13, false, true, "EXT_IRQ4"},
    {0,  false, true, "EXT_IRQ5"},
    {0, false, true, "EXT_IRQ6"},
    {0,  false, true, "EXT_IRQ7"},
    {0, false, true, "EXT_IRQ8"},
    {0,  false, true, "EXT_IRQ9"},
    {19, false, true, "EXT_IRQ10"}, /* 18 */
    {0,  false, true, "EXT_IRQ11"},
    {0, false, true, "EXT_IRQ12"},
    {0,  false, true, "EXT_IRQ13"},
    {0, false, true, "EXT_IRQ14"},
    {0, false, true, "EXT_IRQ15"},
    {0, false, true, "EXT_IRQ16"}, /*25*/
    {0, false, true, "EXT_IRQ17"}, /*25*/
    {0, false, true, "EXT_IRQ18"}, /*25*/
    {0, false, true, "EXT_IRQ19"}, /*25*/
    {0, false, true, "EXT_IRQ20"}, /*25*/
    {30,  false, true, "FTM"},   /*30*/
    {0,  false, true, "GENET0_A"},
    {0,  false, true, "GENET0_B"},
    {0,  false, true, "GENET1_A"},
    {0,  false, true, "GENET1_B"},
    {35, false, true, "GFX"}, /*35*/
    {36, false, true, "HDMI_TX"},
    {0,  false, true, "HIF"},
    {0, false, true, "HIF_SPI"},  /* 38*/
    {39, false, true, "IPI0"},
    {40, false, true, "IPI1"},
    {41, false, true, "MEMC0"},
    {0,  false, true, "MOCA"},
    {43, false, true, "NMI"},
    {44, false, true, "RAAGA"},
    {45, false, true, "RAAGA_FW"},
    {46,  false, true, "SDS0_AFEC"},
    {47,  false, true, "SDS0_RCVR0"},
    {48,  false, true, "SDS0_RCVR1"},
    {49,  false, true, "SDS0_TFEC"},
    {50, false, true, "SM"},
    {51, false, true, "SVD0"},
    {52, false, true, "SYS"},
    {53, false, true, "SYS_AON"},
    {0, false, true, "SYS_PM"},
    {55, false, true, "UHF"},
    {56, false, true, "UPG_AUX"},
    {57, false, true, "UPG_AUX_AON"},
    {58, false, true, "UPG_BSC"},
    {59, false, true, "UPG_BSC_AON"},
    {60, false, true, "UPG_MAIN"},
    {61, false, true, "UPG_MAIN_AON"},
    {62, false, true, "UPG_SC"},
    {63, false, true, "UPG_SPI"},
    {64, false, true, "UPG_TMR"},
    {0,  false, true, "UPG_UART0"},
    {0,  false, true, "UPG_UART1"},
    {0,  false, true, "UPG_UART2"},
    {0,  false, true, "USB0"}, /*68*/
    {0,  false, true, "USB0_EHCI0"},
    {0,  false, true, "USB0_EHCI1"},
    {0,  false, true, "USB0_OHCI0"},
    {0,  false, true, "USB0_OHCI1"},
    {0,  false, true, "USB1"},
    {0,  false, true, "USB1_EHCI0"},
    {0,  false, true, "USB1_EHCI1"},
    {0,  false, true, "USB1_OHCI0"},
    {0,  false, true, "USB1_OHCI1"}, /*77*/
    {78, false, true, "VEC"},
    {79,  false, true, "XPT_FE"},
    {80,  false, true, "XPT_MSG"},
    {81,  false, true, "XPT_MSG_STAT"},
    {82,  false, true, "XPT_OVFL"},
    {83,  false, true, "XPT_PCR"},
    {84,  false, true, "XPT_RAV"},
    {85,  false, true, "XPT_STATUS"}, /*85*/
    {0,   false, false, NULL}
};
#elif BCHP_CHIP == 7358
struct NEXUS_Platform_P_IrqMap NEXUS_Platform_P_IrqMap[32*NEXUS_NUM_L1_REGISTERS] = {
    {1,  false, true, "AIO"},
    {2,  false, true, "BSP"},
    {3,  false, true, "BVNB"},
    {4,  false, true, "BVNF0"},
    {5,  false, true, "BVNF1"},
    {6,  false, true, "BVNF5"},
    {7,  false, true, "BVNM"},
    {8,  false, true, "CLKGEN"},
    {9,  false, false,"EXT_IRQ0"},
    {10, false, false,"EXT_IRQ1"},
    {11, false, false,"EXT_IRQ2"},
    {12, false, false,"EXT_IRQ3"},
    {13, false, false,"EXT_IRQ4"},
    {14, false, false,"EXT_IRQ5"},
    {15, false, false,"EXT_IRQ6"},
    {16, false, false,"EXT_IRQ7"},
    {17, false, false,"EXT_IRQ8"},
    {18, false, false,"EXT_IRQ9"},
    {19, false, false,"EXT_IRQ10" },
    {20, false, false,"EXT_IRQ11"},
    {21, false, false,"EXT_IRQ12"},
    {22, false, false,"EXT_IRQ13"},
    {23, false, false,"EXT_IRQ14"},
    {24, false, true, "FTM"},
    {25, false, false,"GENET_0_A"},
    {26, false, false,"GENET_0_B"},
    {27, false, false,"SPARE0"},
    {28, false, false,"SPARE1"},
    {29, false, true, "M2MC"},
    {30, false, true, "HDMI_TX"},
    {31, false, false,"HIF"},
    {32, false, false,"HIF_SPI"},
    {33, false, true, "IPI0"},
    {34, false, true, "IPI1"},
    {35, false, true, "MEMC0"},
    {36, false, false,"SPARE2"},
    {37, false, true, "NMI_PIN"},
    {38, false, true, "RAAGA"},
    {39, false, true, "RAAGA_FW"},
    {40, false, true, "SDS0_AFEC"},
    {41, false, true, "SDS0_RCVR_0"},
    {42, false, true, "SDS0_RCVR_1"},
    {43, false, true, "SDS0_TFEC"},
    {44, false, false,"SPARE3"},
    {45, false, false,"SPARE4"},
    {46, false, false,"SPARE5"},
    {47, false, false,"SPARE6"},
    {48, false, true, "SOFT_MODEM"},
    {49, false, true, "SYS"},
    {50, false, true, "SYS_AON"},
    {0, false, true, "SYS_PM"},
    {52, false, false,"SPARE7"},
    {53, false, false,"SPARE8"},
    {54, false, true, "UPG_AUX_AON"},
    {55, false, true, "UPG_BSC"},
    {56, false, true, "UPG_BSC_AON"},
    {57, false, true, "UPG_MAIN"},
    {58, false, true, "UPG_MAIN_AON"},
    {59, false, true, "UPG_SC"},
    {60, false, true, "UPG_SPI"},
    {61, false, true, "UPG_TMR"},
    {62, false, false,"UPG_UART0"},
    {63, false, false,"UPG_UART1"},
    {64, false, false,"UPG_UART2"},
    {65, false, false,"USB0_BRIDGE"},
    {66, false, false,"USB0_EHCI_0"},
    {67, false, false,"USB0_OHCI_0"},
    {68, false, false,"SPARE9"},
    {69, false, false,"SPARE10"},
    {70, false, false,"SPARE11"},
    {71, false, false,"SPARE12"},
    {72, false, false,"SPARE13"},
    {73, false, false,"SPARE14"},
    {74, false, false,"SPARE15"},
    {75, false, true, "VEC"},
    {76, false, true, "XPT_FE"},
    {77, false, true, "XPT_MSG"},
    {78, false, true, "XPT_MSG_STAT"},
    {79, false, true, "XPT_OVFL"},
    {80, false, true, "XPT_PCR"},
    {81, false, true, "XPT_RAV"},
    {82, false, true, "XPT_STATUS"},
    {83, false, false,"SDIO0"},
    {84, false, false,"SPARE16"},
    {85, false, true, "AVD0"},
    {86, false, false,"SPARE17"},
    {87, false, false,"SPARE18"},
    {88, false, false,"RFM"},
    {89, false, false,"THD_A"},
    {90, false, false,"THD_B"},
    {91, false, false,"UFE_CPU"},
    {92, false, false,"DS_0"},
    {93, false, false,"SPARE19"},
    {94, false, false,"SPARE20"},
    {95, false, false,"MCIF"},
    {96, false, true, "UPG_AUX"},
    {0,   false, false, NULL}
}; /* End 7358 */
#elif BCHP_CHIP == 7552
struct NEXUS_Platform_P_IrqMap NEXUS_Platform_P_IrqMap[] = {
    {1,  false, true, "AIO"},
    {2,  false, true, "BSP"},
    {3,  false, true, "BVNB"},
    {4,  false, true, "BVNF0"},
    {5,  false, true, "BVNF1"},
    {6,  false, true, "BVNF5"},
    {7,  false, true, "BVNM"},
    {8,  false, true, "CLKGEN"},
    {9,  false, false,"EXT_IRQ0"},
    {10, false, true,"EXT_IRQ1"},
    {11, false, false,"EXT_IRQ2"},
    {12, false, false,"EXT_IRQ3"},
    {13, false, false,"EXT_IRQ4"},
    {14, false, false,"EXT_IRQ5"},
    {15, false, false,"EXT_IRQ6"},
    {16, false, false,"EXT_IRQ7"},
    {17, false, false,"EXT_IRQ8"},
    {18, false, false,"EXT_IRQ9"},
    {19, false, false,"EXT_IRQ10" },
    {20, false, false,"EXT_IRQ11"},
    {21, false, false,"EXT_IRQ12"},
    {22, false, false,"EXT_IRQ13"},
    {23, false, false,"EXT_IRQ14"},
    {24, false, false,"FTM"},
    {25, false, false,"GENET_0_A"},
    {26, false, false,"GENET_0_B"},
    {27, false, false,"SPARE0"},
    {28, false, false,"SPARE1"},
    {29, false, true, "M2MC"},
    {30, false, true, "HDMI_TX"},
    {31, false, false,"HIF"},
    {32, false, false,"HIF_SPI"},
    {33, false, true, "IPI0"},
    {34, false, true, "IPI1"},
    {35, false, true, "MEMC0"},
    {36, false, false,"SPARE2"},
    {37, false, true, "NMI_PIN"},
    {38, false, true, "RAAGA"},
    {39, false, true, "RAAGA_FW"},
    {40, false, false,"SDS0_AFEC"},
    {41, false, false,"SDS0_RCVR_0"},
    {42, false, false,"SDS0_RCVR_1"},
    {43, false, false,"SDS0_TFEC"},
    {44, false, false,"SPARE3"},
    {45, false, false,"SPARE4"},
    {46, false, false,"SPARE5"},
    {47, false, false,"SPARE6"},
    {48, false, true, "SOFT_MODEM"},
    {49, false, true, "SYS"},
    {50, false, true, "SYS_AON"},
    {0, false, true, "SYS_PM"},
    {52, false, false,"SPARE7"},
    {53, false, false,"SPARE8"},
    {54, false, true, "UPG_AUX_AON"},
    {55, false, true, "UPG_BSC"},
    {56, false, true, "UPG_BSC_AON"},
    {57, false, true, "UPG_MAIN"},
    {58, false, true, "UPG_MAIN_AON"},
    {59, false, true, "UPG_SC"},
    {60, false, true, "UPG_SPI"},
    {61, false, true, "UPG_TMR"},
    {62, false, false,"UPG_UART0"},
    {63, false, false,"UPG_UART1"},
    {64, false, false,"UPG_UART2"},
    {65, false, false,"USB0_BRIDGE"},
    {66, false, false,"USB0_EHCI_0"},
    {67, false, false,"USB0_OHCI_0"},
    {68, false, false,"SPARE9"},
    {69, false, false,"SPARE10"},
    {70, false, false,"SPARE11"},
    {71, false, false,"SPARE12"},
    {72, false, false,"SPARE13"},
    {73, false, false,"SPARE14"},
    {74, false, false,"SPARE15"},
    {75, false, true, "VEC"},
    {76, false, true, "XPT_FE"},
    {77, false, true, "XPT_MSG"},
    {78, false, true, "XPT_MSG_STAT"},
    {79, false, true, "XPT_OVFL"},
    {80, false, true, "XPT_PCR"},
    {81, false, true, "XPT_RAV"},
    {82, false, true, "XPT_STATUS"},
    {83, false, false,"SDIO0"},
    {84, false, false,"SPARE16"},
    {85, false, true, "AVD0"},
    {86, false, false,"SPARE17"},
    {87, false, false,"SPARE18"},
    {88, false, true, "RFM"},
    {89, false, true, "THD_A"},
    {90, false, true, "THD_B"},
    {91, false, true, "UFE_CPU"},
    {92, false, true, "DS_0"},
    {93, false, false,"SPARE19"},
    {94, false, false,"SPARE20"},
    {95, false, false,"MCIF"},
    {96, false, true, "UPG_AUX"},
    {0,   false, false, NULL}
}; /* End 7552 */
#elif (BCHP_CHIP==7231)
struct NEXUS_Platform_P_IrqMap NEXUS_Platform_P_IrqMap[] = {
    {9,  false, true, "EXT_IRQ0"},
    {10, false, true, "EXT_IRQ1"},
    {11, false, true, "EXT_IRQ2"},
    {12, false, true, "EXT_IRQ3"},
    {13, false, true, "EXT_IRQ4"},
    {0,  false, false, NULL}
};
#elif (BCHP_CHIP==7429) || (BCHP_CHIP==74295)
struct NEXUS_Platform_P_IrqMap NEXUS_Platform_P_IrqMap[32*NEXUS_NUM_L1_REGISTERS] = {
    {10,  false, true, "EXT_IRQ0"},
    {11,  false, true, "EXT_IRQ1"},
    {12,  false, true, "EXT_IRQ2"},
    {13,  false, true, "EXT_IRQ3"},
    {14,  false, true, "EXT_IRQ4"},
    {15,  false, true, "EXT_IRQ5"},
    {0,   false, false, NULL}
};
#else
/* in the default case, nexus will use BINT_P_IntMap for all L1 validation */
struct NEXUS_Platform_P_IrqMap NEXUS_Platform_P_IrqMap[] = {
    {0, false, false, NULL}
};
#endif
