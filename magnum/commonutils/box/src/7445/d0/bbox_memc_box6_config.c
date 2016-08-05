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
******************************************************************************/
/*******************************************************************
*               Do Not Edit Directly
* Auto-Generated from RTS environment:
*   at: Mon Oct 12 21:57:44 2015 GMT
*   by: robinc
*   for: Box 7252_1u2t
*         MemC 0 (32-bit DDR3@1066MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1066MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20151012215744_7252_1u2t[] = {
           0x001e6002,  /*   0: XPT_WR_RS 1130ns */
           0x803a303e,  /*   1: XPT_WR_XC RR 2290ns */
           0x8049000a,  /*   2: XPT_WR_CDB RR 2710ns */
           0x80b91046,  /*   3: XPT_WR_ITB_MSG RR 7270ns */
           0x8078001a,  /*   4: XPT_RD_RS RR 4450ns */
           0x819e5028,  /*   5: XPT_RD_XC_RMX_MSG RR 15350ns */
           0x80490009,  /*   6: XPT_RD_XC_RAVE RR 2710ns */
           0x80a2e043,  /*   7: XPT_RD_PB RR 6400ns */
           0x809d8042,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e604d,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd005,  /*  10: SYSPORT_WR RR 2370ns */
           0x81026048,  /*  11: SYSPORT_RD RR 10150ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161704f,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e039,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066203f,  /*  17: SATA RR 4015ns */
           0x80662040,  /*  18: SATA_1 RR 4015ns */
           0x03e6e034,  /*  19: MCIF2_RD 37000ns */
           0x03e6e036,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e038,  /*  22: BSP RR 50000ns */
           0x80ad9044,  /*  23: SAGE RR 6820ns */
           0x86449051,  /*  24: FLASH_DMA RR 63000ns */
           0x8161704e,  /*  25: HIF_PCIe RR 13880ns */
           0x86449053,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449052,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e035,  /*  29: MCIF_RD 37000ns */
           0x03e6e037,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db049,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c504c,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c504b,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1045,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db04a,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x0050e010,  /*  40: RAAGA 3000ns */
           0x001ae001,  /*  41: RAAGA_1 1000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00bb401e,  /*  44: AUD_AIO 6940ns */
           0x3ffff055,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff05a,  /*  46: VICE_CME_CSC off */
           0x3ffff056,  /*  47: VICE_FME_CSC off */
           0x3ffff058,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff057,  /*  49: VICE_FME_Chroma_CMB off */
           0x812b9025,  /*  50: VICE_SG RR 11100ns */
           0x3ffff05f,  /*  51: VICE_DBLK off */
           0x81edf02b,  /*  52: VICE_CABAC0 RR 18300ns */
           0x83782033,  /*  53: VICE_CABAC1 RR 32900ns */
           0x3ffff059,  /*  54: VICE_ARCSS0 off */
           0x3ffff01f,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff02c,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff020,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff02d,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff0ff,  /*  59: UNASSIGNED off */
           0x3ffff0ff,  /*  60: UNASSIGNED off */
           0x3ffff0ff,  /*  61: UNASSIGNED off */
           0x3ffff0ff,  /*  62: UNASSIGNED off */
           0x3ffff0ff,  /*  63: UNASSIGNED off */
           0x3ffff0ff,  /*  64: UNASSIGNED off */
           0x3ffff0ff,  /*  65: UNASSIGNED off */
           0x3ffff0ff,  /*  66: UNASSIGNED off */
           0x3ffff0ff,  /*  67: UNASSIGNED off */
           0x3ffff0ff,  /*  68: UNASSIGNED off */
           0x3ffff0ff,  /*  69: UNASSIGNED off */
           0x3ffff0ff,  /*  70: UNASSIGNED off */
           0x3ffff0ff,  /*  71: UNASSIGNED off */
           0x3ffff0ff,  /*  72: UNASSIGNED off */
           0x3ffff05b,  /*  73: HVD0_DBLK_0 off */
           0x3ffff05c,  /*  74: HVD0_DBLK_1 off */
           0x8027003b,  /*  75: HVD0_ILCPU RR 1451ns */
           0x80d9b047,  /*  76: HVD0_OLCPU RR 8553ns */
           0x007de01b,  /*  77: HVD0_CAB 4667ns */
           0x0071a017,  /*  78: HVD0_ILSI 4214ns */
           0x82c90050,  /*  79: HVD0_ILCPU_p2 RR 26413ns */
           0x0073f019,  /*  80: HVD0_ILSI_p2 4299ns */
           0x80000060,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x80000061,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x8027003c,  /*  83: HVD1_ILCPU RR 1451ns */
           0x8070a041,  /*  84: HVD1_OLCPU RR 4427ns */
           0x00714016,  /*  85: HVD1_CAB 4200ns */
           0x0072d018,  /*  86: HVD1_ILSI 4258ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff0ff,  /*  94: UNASSIGNED off */
           0x3ffff0ff,  /*  95: UNASSIGNED off */
           0x3ffff0ff,  /*  96: UNASSIGNED off */
           0x3ffff023,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff02e,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff029,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff024,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff02f,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff02a,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x3ffff006,  /* 106: BVN_MFD0 off */
           0x3ffff000,  /* 107: BVN_MFD0_1 off */
           0x3ffff0ff,  /* 108: UNASSIGNED off */
           0x3ffff0ff,  /* 109: UNASSIGNED off */
           0x0063d014,  /* 110: BVN_MFD2 3703ns */
           0x00428007,  /* 111: BVN_MFD2_1 2469ns */
           0x0063d015,  /* 112: BVN_MFD3 3703ns */
           0x00428008,  /* 113: BVN_MFD3_1 2469ns */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff00f,  /* 118: BVN_VFD0 off */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff032,  /* 120: BVN_VFD2 off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff01c,  /* 122: BVN_VFD4 off */
           0x3ffff01d,  /* 123: BVN_VFD5 off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff003,  /* 126: BVN_CAP0 off */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff027,  /* 128: BVN_CAP2 off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff00b,  /* 130: BVN_CAP4 off */
           0x3ffff00c,  /* 131: BVN_CAP5 off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0031d004,  /* 134: BVN_GFD0 1851ns */
           0x3ffff026,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff011,  /* 137: BVN_GFD3 off */
           0x3ffff012,  /* 138: BVN_GFD4 off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff00e,  /* 141: BVN_MCVP0 off */
           0x3ffff00d,  /* 142: BVN_MCVP1 off */
           0x3ffff022,  /* 143: BVN_MCVP2 off */
           0x00571013,  /* 144: BVN_RDC 3230ns */
           0x03526030,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526031,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x3ffff05d,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff05e,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0x3ffff0ff,  /* 163: UNASSIGNED off */
           0x3ffff0ff,  /* 164: UNASSIGNED off */
           0x3ffff0ff,  /* 165: UNASSIGNED off */
           0x3ffff0ff,  /* 166: UNASSIGNED off */
           0x3ffff0ff,  /* 167: UNASSIGNED off */
           0x3ffff0ff,  /* 168: UNASSIGNED off */
           0x3ffff0ff,  /* 169: UNASSIGNED off */
           0x3ffff0ff,  /* 170: UNASSIGNED off */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x3ffff0ff,  /* 172: UNASSIGNED off */
           0x3ffff0ff,  /* 173: UNASSIGNED off */
           0x3ffff0ff,  /* 174: UNASSIGNED off */
           0x3ffff0ff,  /* 175: UNASSIGNED off */
           0x3ffff0ff,  /* 176: UNASSIGNED off */
           0x3ffff0ff,  /* 177: UNASSIGNED off */
           0x3ffff0ff,  /* 178: UNASSIGNED off */
           0x3ffff0ff,  /* 179: UNASSIGNED off */
           0x3ffff0ff,  /* 180: UNASSIGNED off */
           0x3ffff0ff,  /* 181: UNASSIGNED off */
           0x3ffff0ff,  /* 182: UNASSIGNED off */
           0x3ffff0ff,  /* 183: UNASSIGNED off */
           0x3ffff0ff,  /* 184: UNASSIGNED off */
           0x3ffff0ff,  /* 185: UNASSIGNED off */
           0x3ffff0ff,  /* 186: UNASSIGNED off */
           0x3ffff0ff,  /* 187: UNASSIGNED off */
           0x3ffff0ff,  /* 188: UNASSIGNED off */
           0x3ffff0ff,  /* 189: UNASSIGNED off */
           0x3ffff0ff,  /* 190: UNASSIGNED off */
           0x3ffff0ff,  /* 191: UNASSIGNED off */
           0x3ffff0ff,  /* 192: UNASSIGNED off */
           0x3ffff0ff,  /* 193: UNASSIGNED off */
           0x3ffff0ff,  /* 194: UNASSIGNED off */
           0x3ffff0ff,  /* 195: UNASSIGNED off */
           0x3ffff0ff,  /* 196: UNASSIGNED off */
           0x3ffff0ff,  /* 197: UNASSIGNED off */
           0x3ffff0ff,  /* 198: UNASSIGNED off */
           0x3ffff0ff,  /* 199: UNASSIGNED off */
           0x801e403a,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000068,  /* 201: CPU_MCP_RD_LOW RR */
           0x8039303d,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000069,  /* 203: CPU_MCP_WR_LOW RR */
           0xbffff0ff,  /* 204: V3D_MCP_RD_HIGH RR */
           0xbffff0ff,  /* 205: V3D_MCP_RD_LOW RR */
           0xbffff0ff,  /* 206: V3D_MCP_WR_HIGH RR */
           0xbffff0ff,  /* 207: V3D_MCP_WR_LOW RR */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x3ffff062,  /* 216: HVD0_PFRI off */
           0x80000064,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff0ff,  /* 218: UNASSIGNED off */
           0x3ffff063,  /* 219: VICE_PFRI off */
           0x3ffff0ff,  /* 220: UNASSIGNED off */
           0x3ffff0ff,  /* 221: UNASSIGNED off */
           0x3ffff0ff,  /* 222: UNASSIGNED off */
           0x3ffff0ff,  /* 223: UNASSIGNED off */
           0x3ffff0ff,  /* 224: UNASSIGNED off */
           0x3ffff0ff,  /* 225: UNASSIGNED off */
           0x3ffff0ff,  /* 226: UNASSIGNED off */
           0x3ffff0ff,  /* 227: UNASSIGNED off */
           0x3ffff0ff,  /* 228: UNASSIGNED off */
           0x3ffff0ff,  /* 229: UNASSIGNED off */
           0x3ffff0ff,  /* 230: UNASSIGNED off */
           0x3ffff0ff,  /* 231: UNASSIGNED off */
           0x3ffff0ff,  /* 232: UNASSIGNED off */
           0x3ffff0ff,  /* 233: UNASSIGNED off */
           0x3ffff0ff,  /* 234: UNASSIGNED off */
           0x3ffff0ff,  /* 235: UNASSIGNED off */
           0x3ffff0ff,  /* 236: UNASSIGNED off */
           0x3ffff0ff,  /* 237: UNASSIGNED off */
           0x3ffff0ff,  /* 238: UNASSIGNED off */
           0x3ffff0ff,  /* 239: UNASSIGNED off */
           0x3ffff0ff,  /* 240: UNASSIGNED off */
           0x3ffff0ff,  /* 241: UNASSIGNED off */
           0x3ffff0ff,  /* 242: UNASSIGNED off */
           0x3ffff0ff,  /* 243: UNASSIGNED off */
           0x3ffff0ff,  /* 244: UNASSIGNED off */
           0x3ffff0ff,  /* 245: UNASSIGNED off */
           0x3ffff0ff,  /* 246: UNASSIGNED off */
           0x3ffff0ff,  /* 247: UNASSIGNED off */
           0xbffff0ff,  /* 248: MEMC_TRACELOG RR */
           0x3ffff0ff,  /* 249: UNASSIGNED off */
           0xbfffe054,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27021   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20151012215744_7252_1u2t[] = {
           0x3ffff002,  /*   0: XPT_WR_RS off */
           0x3ffff03e,  /*   1: XPT_WR_XC off */
           0x3ffff00a,  /*   2: XPT_WR_CDB off */
           0x3ffff046,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff01a,  /*   4: XPT_RD_RS off */
           0x3ffff028,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff009,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff043,  /*   7: XPT_RD_PB off */
           0x809d8042,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e604d,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd005,  /*  10: SYSPORT_WR RR 2370ns */
           0x81026048,  /*  11: SYSPORT_RD RR 10150ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161704f,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e039,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066203f,  /*  17: SATA RR 4015ns */
           0x80662040,  /*  18: SATA_1 RR 4015ns */
           0x3ffff034,  /*  19: MCIF2_RD off */
           0x3ffff036,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e038,  /*  22: BSP RR 50000ns */
           0x80ad9044,  /*  23: SAGE RR 6820ns */
           0x86449051,  /*  24: FLASH_DMA RR 63000ns */
           0x8161704e,  /*  25: HIF_PCIe RR 13880ns */
           0x86449053,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449052,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff035,  /*  29: MCIF_RD off */
           0x3ffff037,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db049,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c504c,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c504b,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1045,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db04a,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff010,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff01e,  /*  44: AUD_AIO off */
           0x80393055,  /*  45: VICE_CME_RMB_CMB RR 2250ns */
           0x81ca605a,  /*  46: VICE_CME_CSC RR 18000ns */
           0x8069d056,  /*  47: VICE_FME_CSC RR 4160ns */
           0x80d41058,  /*  48: VICE_FME_Luma_CMB RR 8330ns */
           0x80d41057,  /*  49: VICE_FME_Chroma_CMB RR 8330ns */
           0x3ffff025,  /*  50: VICE_SG off */
           0x8000205f,  /*  51: VICE_DBLK RR 10ns */
           0x3ffff02b,  /*  52: VICE_CABAC0 off */
           0x3ffff033,  /*  53: VICE_CABAC1 off */
           0x804d9059,  /*  54: VICE_ARCSS0 RR 3050ns */
           0x80c7d01f,  /*  55: VICE_VIP0_INST0 RR 7406ns */
           0x8257402c,  /*  56: VICE_VIP1_INST0 RR 22200ns */
           0x80c7d020,  /*  57: VICE_VIP0_INST1 RR 7406ns */
           0x8257402d,  /*  58: VICE_VIP1_INST1 RR 22200ns */
           0x3ffff0ff,  /*  59: UNASSIGNED off */
           0x3ffff0ff,  /*  60: UNASSIGNED off */
           0x3ffff0ff,  /*  61: UNASSIGNED off */
           0x3ffff0ff,  /*  62: UNASSIGNED off */
           0x3ffff0ff,  /*  63: UNASSIGNED off */
           0x3ffff0ff,  /*  64: UNASSIGNED off */
           0x3ffff0ff,  /*  65: UNASSIGNED off */
           0x3ffff0ff,  /*  66: UNASSIGNED off */
           0x3ffff0ff,  /*  67: UNASSIGNED off */
           0x3ffff0ff,  /*  68: UNASSIGNED off */
           0x3ffff0ff,  /*  69: UNASSIGNED off */
           0x3ffff0ff,  /*  70: UNASSIGNED off */
           0x3ffff0ff,  /*  71: UNASSIGNED off */
           0x3ffff0ff,  /*  72: UNASSIGNED off */
           0x8000005b,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8000005c,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff03b,  /*  75: HVD0_ILCPU off */
           0x3ffff047,  /*  76: HVD0_OLCPU off */
           0x3ffff01b,  /*  77: HVD0_CAB off */
           0x3ffff017,  /*  78: HVD0_ILSI off */
           0x82c90050,  /*  79: HVD0_ILCPU_p2 RR 26413ns */
           0x3ffff019,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff060,  /*  81: HVD1_DBLK_0 off */
           0x3ffff061,  /*  82: HVD1_DBLK_1 off */
           0x3ffff03c,  /*  83: HVD1_ILCPU off */
           0x3ffff041,  /*  84: HVD1_OLCPU off */
           0x3ffff016,  /*  85: HVD1_CAB off */
           0x3ffff018,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff0ff,  /*  94: UNASSIGNED off */
           0x3ffff0ff,  /*  95: UNASSIGNED off */
           0x3ffff0ff,  /*  96: UNASSIGNED off */
           0x00f24023,  /*  97: BVN_MAD1_PIX_FD 8978.507ns */
           0x025de02e,  /*  98: BVN_MAD1_QUANT 22446.741ns */
           0x01e4b029,  /*  99: BVN_MAD1_PIX_CAP 17957.014ns */
           0x00f24024,  /* 100: BVN_MAD2_PIX_FD 8978.507ns */
           0x025de02f,  /* 101: BVN_MAD2_QUANT 22446.741ns */
           0x01e4b02a,  /* 102: BVN_MAD2_PIX_CAP 17957.014ns */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x00428006,  /* 106: BVN_MFD0 2469ns */
           0x0012e000,  /* 107: BVN_MFD0_1 705ns */
           0x3ffff0ff,  /* 108: UNASSIGNED off */
           0x3ffff0ff,  /* 109: UNASSIGNED off */
           0x3ffff014,  /* 110: BVN_MFD2 off */
           0x3ffff007,  /* 111: BVN_MFD2_1 off */
           0x3ffff015,  /* 112: BVN_MFD3 off */
           0x3ffff008,  /* 113: BVN_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x004f600f,  /* 118: BVN_VFD0 2945.2736318408ns */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x0359a032,  /* 120: BVN_VFD2 31770ns */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x0095101c,  /* 122: BVN_VFD4 5527.3631840796ns */
           0x0095101d,  /* 123: BVN_VFD5 5527.3631840796ns */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x0027a003,  /* 126: BVN_CAP0 1472.6368159204ns */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x01747027,  /* 128: BVN_CAP2 13800ns */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x004a700b,  /* 130: BVN_CAP4 2763.6815920398ns */
           0x004a700c,  /* 131: BVN_CAP5 2763.6815920398ns */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff004,  /* 134: BVN_GFD0 off */
           0x0167c026,  /* 135: BVN_GFD1 13330ns */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x00559011,  /* 137: BVN_GFD3 3174ns */
           0x00559012,  /* 138: BVN_GFD4 3174ns */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b500e,  /* 141: BVN_MCVP0 2794ns */
           0x004b500d,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d022,  /* 143: BVN_MCVP2 7850ns */
           0x3ffff013,  /* 144: BVN_RDC off */
           0x3ffff030,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff031,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x8000005d,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8000005e,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0x3ffff0ff,  /* 163: UNASSIGNED off */
           0x3ffff0ff,  /* 164: UNASSIGNED off */
           0x3ffff0ff,  /* 165: UNASSIGNED off */
           0x3ffff0ff,  /* 166: UNASSIGNED off */
           0x3ffff0ff,  /* 167: UNASSIGNED off */
           0x3ffff0ff,  /* 168: UNASSIGNED off */
           0x3ffff0ff,  /* 169: UNASSIGNED off */
           0x3ffff0ff,  /* 170: UNASSIGNED off */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x3ffff0ff,  /* 172: UNASSIGNED off */
           0x3ffff0ff,  /* 173: UNASSIGNED off */
           0x3ffff0ff,  /* 174: UNASSIGNED off */
           0x3ffff0ff,  /* 175: UNASSIGNED off */
           0x3ffff0ff,  /* 176: UNASSIGNED off */
           0x3ffff0ff,  /* 177: UNASSIGNED off */
           0x3ffff0ff,  /* 178: UNASSIGNED off */
           0x3ffff0ff,  /* 179: UNASSIGNED off */
           0x3ffff0ff,  /* 180: UNASSIGNED off */
           0x3ffff0ff,  /* 181: UNASSIGNED off */
           0x3ffff0ff,  /* 182: UNASSIGNED off */
           0x3ffff0ff,  /* 183: UNASSIGNED off */
           0x3ffff0ff,  /* 184: UNASSIGNED off */
           0x3ffff0ff,  /* 185: UNASSIGNED off */
           0x3ffff0ff,  /* 186: UNASSIGNED off */
           0x3ffff0ff,  /* 187: UNASSIGNED off */
           0x3ffff0ff,  /* 188: UNASSIGNED off */
           0x3ffff0ff,  /* 189: UNASSIGNED off */
           0x3ffff0ff,  /* 190: UNASSIGNED off */
           0x3ffff0ff,  /* 191: UNASSIGNED off */
           0x3ffff0ff,  /* 192: UNASSIGNED off */
           0x3ffff0ff,  /* 193: UNASSIGNED off */
           0x3ffff0ff,  /* 194: UNASSIGNED off */
           0x3ffff0ff,  /* 195: UNASSIGNED off */
           0x3ffff0ff,  /* 196: UNASSIGNED off */
           0x3ffff0ff,  /* 197: UNASSIGNED off */
           0x3ffff0ff,  /* 198: UNASSIGNED off */
           0x3ffff0ff,  /* 199: UNASSIGNED off */
           0x801e403a,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000068,  /* 201: CPU_MCP_RD_LOW RR */
           0x8039303d,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000069,  /* 203: CPU_MCP_WR_LOW RR */
           0xbffff0ff,  /* 204: V3D_MCP_RD_HIGH RR */
           0xbffff0ff,  /* 205: V3D_MCP_RD_LOW RR */
           0xbffff0ff,  /* 206: V3D_MCP_WR_HIGH RR */
           0xbffff0ff,  /* 207: V3D_MCP_WR_LOW RR */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x80000062,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff064,  /* 217: HVD1_PFRI off */
           0x3ffff0ff,  /* 218: UNASSIGNED off */
           0x80002063,  /* 219: VICE_PFRI RR 10ns */
           0x3ffff0ff,  /* 220: UNASSIGNED off */
           0x3ffff0ff,  /* 221: UNASSIGNED off */
           0x3ffff0ff,  /* 222: UNASSIGNED off */
           0x3ffff0ff,  /* 223: UNASSIGNED off */
           0x3ffff0ff,  /* 224: UNASSIGNED off */
           0x3ffff0ff,  /* 225: UNASSIGNED off */
           0x3ffff0ff,  /* 226: UNASSIGNED off */
           0x3ffff0ff,  /* 227: UNASSIGNED off */
           0x3ffff0ff,  /* 228: UNASSIGNED off */
           0x3ffff0ff,  /* 229: UNASSIGNED off */
           0x3ffff0ff,  /* 230: UNASSIGNED off */
           0x3ffff0ff,  /* 231: UNASSIGNED off */
           0x3ffff0ff,  /* 232: UNASSIGNED off */
           0x3ffff0ff,  /* 233: UNASSIGNED off */
           0x3ffff0ff,  /* 234: UNASSIGNED off */
           0x3ffff0ff,  /* 235: UNASSIGNED off */
           0x3ffff0ff,  /* 236: UNASSIGNED off */
           0x3ffff0ff,  /* 237: UNASSIGNED off */
           0x3ffff0ff,  /* 238: UNASSIGNED off */
           0x3ffff0ff,  /* 239: UNASSIGNED off */
           0x3ffff0ff,  /* 240: UNASSIGNED off */
           0x3ffff0ff,  /* 241: UNASSIGNED off */
           0x3ffff0ff,  /* 242: UNASSIGNED off */
           0x3ffff0ff,  /* 243: UNASSIGNED off */
           0x3ffff0ff,  /* 244: UNASSIGNED off */
           0x3ffff0ff,  /* 245: UNASSIGNED off */
           0x3ffff0ff,  /* 246: UNASSIGNED off */
           0x3ffff0ff,  /* 247: UNASSIGNED off */
           0xbffff0ff,  /* 248: MEMC_TRACELOG RR */
           0x3ffff0ff,  /* 249: UNASSIGNED off */
           0xbfffe054,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27021   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20151012215744_7252_1u2t[] = {
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x80670803}, /* HVD1_PFRI (gHvd1) 1061333.33 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000598}, /* d: 4; p: 1432.796875 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x00002340}, /* 9024 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x00001526}, /* 60% * 9024 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80650803}, /* HVD0_PFRI (gHvd0) 471600.00 ns/20 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x400009f2}, /* d: 4; p: 2546.6375 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00002340}, /* 9024 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00001526}, /* 60% * 9024 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_CONFIG,      0x80660802}, /* VICE_PFRI (gVice) 888888.89 ns/320 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_FILTER_CTRL, 0x4000012c}, /* d: 4; p: 300 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_THRESH0,     0x000001a8}, /* 424 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_THRESH1,     0x000000fe}  /* 60% * 424 */
};

static const uint32_t* const paulMemc_box6[] = { &aulMemc0_20151012215744_7252_1u2t[0], &aulMemc1_20151012215744_7252_1u2t[0]};

const BBOX_Rts stBoxRts_7252_1u2t_box6 = {
  "20151012215744_7252_1u2t_box6",
  7445,
  6,
  2,
  256,
  (const uint32_t**)&paulMemc_box6[0],
  12,
  stBoxRts_PfriClient_20151012215744_7252_1u2t
};
