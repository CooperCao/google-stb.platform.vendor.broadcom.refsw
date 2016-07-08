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
/******************************************************************************
 *
 *                            Do Not Edit Directly
 * Auto-Generated from RTS environment:
 *   at: Fri Mar 11 01:42:11 2016 GMT
 *   by: robinc
 *   for: Box 7252s_headless_dualxcode_pip
 *         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
 *         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439B0_Robin/BCM7439B0_dual_headless_amazon.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20160311014211_7252s_headless_dualxcode_pip[] = {
           0x001e6002,  /*   0: XPT_WR_RS 1130ns */
           0x803a304a,  /*   1: XPT_WR_XC RR 2290ns */
           0x8049000d,  /*   2: XPT_WR_CDB RR 2710ns */
           0x80b91057,  /*   3: XPT_WR_ITB_MSG RR 7270ns */
           0x80780021,  /*   4: XPT_RD_RS RR 4450ns */
           0x819e5038,  /*   5: XPT_RD_XC_RMX_MSG RR 15350ns */
           0x8049000c,  /*   6: XPT_RD_XC_RAVE RR 2710ns */
           0x80a2e052,  /*   7: XPT_RD_PB RR 6400ns */
           0x809d8051,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e605e,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd009,  /*  10: GENET0_WR RR 2370ns */
           0x81026058,  /*  11: GENET0_RD RR 10150ns */
           0x803fd00a,  /*  12: GENET1_WR RR 2370ns */
           0x81026059,  /*  13: GENET1_RD RR 10150ns */
           0x803fd00b,  /*  14: GENET2_WR RR 2370ns */
           0x8102605a,  /*  15: GENET2_RD RR 10150ns */
           0x8596e044,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066204f,  /*  17: SATA RR 4015ns */
           0x80662050,  /*  18: SATA_1 RR 4015ns */
           0x03e6e03f,  /*  19: MCIF2_RD 37000ns */
           0x03e6e041,  /*  20: MCIF2_WR 37000ns */
           0x81617060,  /*  21: HIF_PCIe1 RR 13880ns */
           0x8545e043,  /*  22: BSP RR 50000ns */
           0x80ad9053,  /*  23: SAGE RR 6820ns */
           0x86449063,  /*  24: FLASH_DMA RR 63000ns */
           0x8161705f,  /*  25: HIF_PCIe RR 13880ns */
           0x86449065,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449064,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e040,  /*  29: MCIF_RD 37000ns */
           0x03e6e042,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db05b,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c505d,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c505c,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1054,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x0050e014,  /*  40: RAAGA 3000ns */
           0x001ae001,  /*  41: RAAGA_1 1000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00bb402c,  /*  44: AUD_AIO 6940ns */
           0x8032d048,  /*  45: VICE_CME_RMB_CMB RR 2000ns */
           0x81977061,  /*  46: VICE_CME_CSC RR 16000ns */
           0x805d604c,  /*  47: VICE_FME_CSC RR 3670ns */
           0x805d604e,  /*  48: VICE_FME_Luma_CMB RR 3670ns */
           0x805d604d,  /*  49: VICE_FME_Chroma_CMB RR 3670ns */
           0x80dca034,  /*  50: VICE_SG RR 8176.66666666667ns */
           0x80002067,  /*  51: VICE_DBLK RR 10ns */
           0x81edf039,  /*  52: VICE_CABAC0 RR 18300ns */
           0x8378203e,  /*  53: VICE_CABAC1 RR 32900ns */
           0x804d904b,  /*  54: VICE_ARCSS0 RR 3050ns */
           0x8063d01d,  /*  55: VICE_VIP0_INST0 RR 3703ns */
           0x80c7a030,  /*  56: VICE_VIP1_INST0 RR 7400ns */
           0x8063d01e,  /*  57: VICE_VIP0_INST1 RR 3703ns */
           0x80c7a031,  /*  58: VICE_VIP1_INST1 RR 7400ns */
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
           0x8000006a,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8000006b,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x80270046,  /*  75: HVD0_ILCPU RR 1451ns */
           0x80af7055,  /*  76: HVD0_OLCPU RR 6893ns */
           0x007dd022,  /*  77: HVD0_CAB 4666ns */
           0x0073901f,  /*  78: HVD0_ILSI 4287ns */
           0x82048062,  /*  79: HVD0_ILCPU_p2 RR 19135ns */
           0x00739020,  /*  80: HVD0_ILSI_p2 4287ns */
           0x3ffff068,  /*  81: HVD1_DBLK_0 off */
           0x3ffff069,  /*  82: HVD1_DBLK_1 off */
           0x80270047,  /*  83: HVD1_ILCPU RR 1451ns */
           0x80af7056,  /*  84: HVD1_OLCPU RR 6893ns */
           0x007dd023,  /*  85: HVD1_CAB 4666ns */
           0x00b6e02b,  /*  86: HVD1_ILSI 6779ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff01c,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff027,  /*  95: BVN_MAD_QUANT off */
           0x3ffff02f,  /*  96: BVN_MAD_PIX_CAP off */
           0x805e201a,  /*  97: BVN_MAD1_PIX_FD RR 3493ns */
           0x80848025,  /*  98: BVN_MAD1_QUANT RR 4914ns */
           0x80bc702d,  /*  99: BVN_MAD1_PIX_CAP RR 6986ns */
           0x805e201b,  /* 100: BVN_MAD2_PIX_FD RR 3493ns */
           0x80848026,  /* 101: BVN_MAD2_QUANT RR 4914ns */
           0x80bc702e,  /* 102: BVN_MAD2_PIX_CAP RR 6986ns */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x8058c016,  /* 106: BVN_MFD0 RR 3292ns */
           0x801a8000,  /* 107: BVN_MFD0_1 RR 987ns */
           0x80853028,  /* 108: BVN_MFD1 RR 4938ns */
           0x8058c017,  /* 109: BVN_MFD1_1 RR 3292ns */
           0x3ffff029,  /* 110: BVN_MFD2 off */
           0x3ffff018,  /* 111: BVN_MFD2_1 off */
           0x3ffff02a,  /* 112: BVN_MFD3 off */
           0x3ffff019,  /* 113: BVN_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff003,  /* 118: BVN_VFD0 off */
           0x3ffff008,  /* 119: BVN_VFD1 off */
           0x3ffff03c,  /* 120: BVN_VFD2 off */
           0x0359a03d,  /* 121: BVN_VFD3 31770ns */
           0x3ffff010,  /* 122: BVN_VFD4 off */
           0x3ffff011,  /* 123: BVN_VFD5 off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff004,  /* 126: BVN_CAP0 off */
           0x3ffff024,  /* 127: BVN_CAP1 off */
           0x3ffff036,  /* 128: BVN_CAP2 off */
           0x01747037,  /* 129: BVN_CAP3 13800ns */
           0x3ffff012,  /* 130: BVN_CAP4 off */
           0x3ffff013,  /* 131: BVN_CAP5 off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff007,  /* 134: BVN_GFD0 off */
           0x3ffff035,  /* 135: BVN_GFD1 off */
           0x3ffff005,  /* 136: BVN_GFD2 off */
           0x3ffff006,  /* 137: BVN_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff00f,  /* 141: BVN_MCVP0 off */
           0x3ffff00e,  /* 142: BVN_MCVP1 off */
           0x3ffff033,  /* 143: BVN_MCVP2 off */
           0x00571015,  /* 144: BVN_RDC 3230ns */
           0x0352603a,  /* 145: VEC_VBI_ENC0 31500ns */
           0x0352603b,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x8000006c,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8000006d,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
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
           0x801e4045,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000074,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393049,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000075,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000006f,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff070,  /* 217: HVD1_PFRI off */
           0x8000206e,  /* 218: VICE_PFRI RR 10ns */
           0x3ffff0ff,  /* 219: UNASSIGNED off */
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
           0xbfffe066,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27032   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20160311014211_7252s_headless_dualxcode_pip[] = {
           0x3ffff002,  /*   0: XPT_WR_RS off */
           0x3ffff04a,  /*   1: XPT_WR_XC off */
           0x3ffff00d,  /*   2: XPT_WR_CDB off */
           0x3ffff057,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff021,  /*   4: XPT_RD_RS off */
           0x3ffff038,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00c,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff052,  /*   7: XPT_RD_PB off */
           0x809d8051,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e605e,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd009,  /*  10: GENET0_WR RR 2370ns */
           0x81026058,  /*  11: GENET0_RD RR 10150ns */
           0x803fd00a,  /*  12: GENET1_WR RR 2370ns */
           0x81026059,  /*  13: GENET1_RD RR 10150ns */
           0x803fd00b,  /*  14: GENET2_WR RR 2370ns */
           0x8102605a,  /*  15: GENET2_RD RR 10150ns */
           0x8596e044,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066204f,  /*  17: SATA RR 4015ns */
           0x80662050,  /*  18: SATA_1 RR 4015ns */
           0x3ffff03f,  /*  19: MCIF2_RD off */
           0x3ffff041,  /*  20: MCIF2_WR off */
           0x81617060,  /*  21: HIF_PCIe1 RR 13880ns */
           0x8545e043,  /*  22: BSP RR 50000ns */
           0x80ad9053,  /*  23: SAGE RR 6820ns */
           0x86449063,  /*  24: FLASH_DMA RR 63000ns */
           0x8161705f,  /*  25: HIF_PCIe RR 13880ns */
           0x86449065,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449064,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff040,  /*  29: MCIF_RD off */
           0x3ffff042,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db05b,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c505d,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c505c,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1054,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x3ffff014,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff02c,  /*  44: AUD_AIO off */
           0x3ffff048,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff061,  /*  46: VICE_CME_CSC off */
           0x3ffff04c,  /*  47: VICE_FME_CSC off */
           0x3ffff04e,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff04d,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff034,  /*  50: VICE_SG off */
           0x3ffff067,  /*  51: VICE_DBLK off */
           0x3ffff039,  /*  52: VICE_CABAC0 off */
           0x3ffff03e,  /*  53: VICE_CABAC1 off */
           0x3ffff04b,  /*  54: VICE_ARCSS0 off */
           0x3ffff01d,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff030,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff01e,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff031,  /*  58: VICE_VIP1_INST1 off */
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
           0x3ffff06a,  /*  73: HVD0_DBLK_0 off */
           0x3ffff06b,  /*  74: HVD0_DBLK_1 off */
           0x3ffff046,  /*  75: HVD0_ILCPU off */
           0x3ffff055,  /*  76: HVD0_OLCPU off */
           0x3ffff022,  /*  77: HVD0_CAB off */
           0x3ffff01f,  /*  78: HVD0_ILSI off */
           0x82048062,  /*  79: HVD0_ILCPU_p2 RR 19135ns */
           0x3ffff020,  /*  80: HVD0_ILSI_p2 off */
           0x80000068,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x80000069,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff047,  /*  83: HVD1_ILCPU off */
           0x3ffff056,  /*  84: HVD1_OLCPU off */
           0x3ffff023,  /*  85: HVD1_CAB off */
           0x3ffff02b,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x005e201c,  /*  94: BVN_MAD_PIX_FD 3493ns */
           0x00848027,  /*  95: BVN_MAD_QUANT 4914ns */
           0x00bc702f,  /*  96: BVN_MAD_PIX_CAP 6986ns */
           0x3ffff01a,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff025,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff02d,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff01b,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff026,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff02e,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x3ffff016,  /* 106: BVN_MFD0 off */
           0x3ffff000,  /* 107: BVN_MFD0_1 off */
           0x3ffff028,  /* 108: BVN_MFD1 off */
           0x3ffff017,  /* 109: BVN_MFD1_1 off */
           0x80853029,  /* 110: BVN_MFD2 RR 4938ns */
           0x8058c018,  /* 111: BVN_MFD2_1 RR 3292ns */
           0x8085302a,  /* 112: BVN_MFD3 RR 4938ns */
           0x8058c019,  /* 113: BVN_MFD3_1 RR 3292ns */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001fb003,  /* 118: BVN_VFD0 1179.10447761194ns */
           0x003f8008,  /* 119: BVN_VFD1 2358.20895522388ns */
           0x0359a03c,  /* 120: BVN_VFD2 31770ns */
           0x3ffff03d,  /* 121: BVN_VFD3 off */
           0x804f6010,  /* 122: BVN_VFD4 RR 2945.2736318408ns */
           0x804f6011,  /* 123: BVN_VFD5 RR 2945.2736318408ns */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x001fb004,  /* 126: BVN_CAP0 1179.10447761194ns */
           0x007f3024,  /* 127: BVN_CAP1 4716.41791044776ns */
           0x01747036,  /* 128: BVN_CAP2 13800ns */
           0x3ffff037,  /* 129: BVN_CAP3 off */
           0x804f6012,  /* 130: BVN_CAP4 RR 2945.2736318408ns */
           0x804f6013,  /* 131: BVN_CAP5 RR 2945.2736318408ns */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0031d007,  /* 134: BVN_GFD0 1851ns */
           0x0167c035,  /* 135: BVN_GFD1 13330ns */
           0x0027d005,  /* 136: BVN_GFD2 1481ns */
           0x0027d006,  /* 137: BVN_GFD3 1481ns */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b500f,  /* 141: BVN_MCVP0 2794ns */
           0x004b500e,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d033,  /* 143: BVN_MCVP2 7850ns */
           0x3ffff015,  /* 144: BVN_RDC off */
           0x3ffff03a,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff03b,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x3ffff06c,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff06d,  /* 156: HVD0_DBLK_p2_1 off */
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
           0x801e4045,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000074,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393049,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000075,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff06f,  /* 216: HVD0_PFRI off */
           0x80000070,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff06e,  /* 218: VICE_PFRI off */
           0x3ffff0ff,  /* 219: UNASSIGNED off */
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
           0xbfffe066,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27032   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20160311014211_7252s_headless_dualxcode_pip[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80720802}, /* HVD0_PFRI (gHvd0) 497333.33 ns/60 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000037f}, /* d: 4; p: 895.2 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00000905}, /* 2309 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00000569}, /* 60% * 2309 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x80710803}, /* VICE_PFRI (gVice) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80730803}, /* HVD1_PFRI (gHvd1) 497333.33 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x4000037f}, /* d: 4; p: 895.2 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x000008d8}, /* 2264 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x0000054e}  /* 60% * 2264 */
};

static const uint32_t* const paulMemc_box24[] = { &aulMemc0_20160311014211_7252s_headless_dualxcode_pip[0], &aulMemc1_20160311014211_7252s_headless_dualxcode_pip[0]};

const BBOX_Rts stBoxRts_7252s_headless_dualxcode_pip_box24 = {
  "20160311014211_7252s_headless_dualxcode_pip_box24",
  7439,
  24,
  2,
  256,
  (const uint32_t**)&paulMemc_box24[0],
  12,
  stBoxRts_PfriClient_20160311014211_7252s_headless_dualxcode_pip
};
