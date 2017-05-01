/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2017 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * API Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************
 *
 *                            Do Not Edit Directly
 * Auto-Generated from RTS environment:
 *   at: Fri Feb 24 01:11:34 2017 GMT
 *   by: robinc
 *   for: Box 7439_4K_1080p_pal_3display
 *         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
 *         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439B0_Robin/BCM7439B0_4kdual1080i_3display_box30.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20170224011134_7439_4K_1080p_pal_3display[] = {
           0x001e6002,  /*   0: XPT_WR_RS 1130ns */
           0x803a3039,  /*   1: XPT_WR_XC RR 2290ns */
           0x8049000e,  /*   2: XPT_WR_CDB RR 2710ns */
           0x80b91043,  /*   3: XPT_WR_ITB_MSG RR 7270ns */
           0x8078001d,  /*   4: XPT_RD_RS RR 4450ns */
           0x819e502b,  /*   5: XPT_RD_XC_RMX_MSG RR 15350ns */
           0x8049000d,  /*   6: XPT_RD_XC_RAVE RR 2710ns */
           0x80a2e040,  /*   7: XPT_RD_PB RR 6400ns */
           0x809d803e,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e604a,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd008,  /*  10: GENET0_WR RR 2370ns */
           0x81026044,  /*  11: GENET0_RD RR 10150ns */
           0x803fd009,  /*  12: GENET1_WR RR 2370ns */
           0x81026045,  /*  13: GENET1_RD RR 10150ns */
           0x803fd00a,  /*  14: GENET2_WR RR 2370ns */
           0x81026046,  /*  15: GENET2_RD RR 10150ns */
           0x8596e034,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066203b,  /*  17: SATA RR 4015ns */
           0x8066203c,  /*  18: SATA_1 RR 4015ns */
           0x03e6e02f,  /*  19: MCIF2_RD 37000ns */
           0x03e6e031,  /*  20: MCIF2_WR 37000ns */
           0x8161704c,  /*  21: HIF_PCIe1 RR 13880ns */
           0x8545e033,  /*  22: BSP RR 50000ns */
           0x80ad9041,  /*  23: SAGE RR 6820ns */
           0x8644904e,  /*  24: FLASH_DMA RR 63000ns */
           0x8161704b,  /*  25: HIF_PCIe RR 13880ns */
           0x86449050,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644904f,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e030,  /*  29: MCIF_RD 37000ns */
           0x03e6e032,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db047,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5049,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5048,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1042,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x804c403a,  /*  40: RAAGA RR 3000ns */
           0x001ae000,  /*  41: RAAGA_1 1000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00bb4023,  /*  44: AUD_AIO 6940ns */
           0x3ffff0ff,  /*  45: UNASSIGNED off */
           0x3ffff0ff,  /*  46: UNASSIGNED off */
           0x3ffff0ff,  /*  47: UNASSIGNED off */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x3ffff0ff,  /*  49: UNASSIGNED off */
           0x3ffff0ff,  /*  50: UNASSIGNED off */
           0x3ffff0ff,  /*  51: UNASSIGNED off */
           0x3ffff0ff,  /*  52: UNASSIGNED off */
           0x3ffff0ff,  /*  53: UNASSIGNED off */
           0x3ffff0ff,  /*  54: UNASSIGNED off */
           0x3ffff0ff,  /*  55: UNASSIGNED off */
           0x3ffff0ff,  /*  56: UNASSIGNED off */
           0x3ffff0ff,  /*  57: UNASSIGNED off */
           0x3ffff0ff,  /*  58: UNASSIGNED off */
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
           0x80000056,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x80000057,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x80270037,  /*  75: HVD0_ILCPU RR 1451ns */
           0x809ee03f,  /*  76: HVD0_OLCPU RR 6242ns */
           0x002bc006,  /*  77: HVD0_CAB 1626ns */
           0x0071a01b,  /*  78: HVD0_ILSI 4214ns */
           0x81fea04d,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x0072601c,  /*  80: HVD0_ILSI_p2 4242ns */
           0x8000005a,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8000005b,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x801c2035,  /*  83: HVD1_ILCPU RR 1048ns */
           0x8082303d,  /*  84: HVD1_OLCPU RR 5117ns */
           0x00596018,  /*  85: HVD1_CAB 3317ns */
           0x00b72022,  /*  86: HVD1_ILSI 6789ns */
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
           0x005e2019,  /*  97: BVN_MAD1_PIX_FD 3493ns */
           0x0084801e,  /*  98: BVN_MAD1_QUANT 4914ns */
           0x00bc7024,  /*  99: BVN_MAD1_PIX_CAP 6986ns */
           0x005e201a,  /* 100: BVN_MAD2_PIX_FD 3493ns */
           0x0084801f,  /* 101: BVN_MAD2_QUANT 4914ns */
           0x00bc7025,  /* 102: BVN_MAD2_PIX_CAP 6986ns */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x010a8028,  /* 106: BVN_MFD0_Ch 9876ns */
           0x004fd013,  /* 107: BVN_MFD0_Ch_1 2961ns */
           0x00853020,  /* 108: BVN_MFD1 4938ns */
           0x0058c016,  /* 109: BVN_MFD1_1 3292ns */
           0x00853021,  /* 110: BVN_MFD2 4938ns */
           0x0058c017,  /* 111: BVN_MFD2_1 3292ns */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff003,  /* 118: BVN_VFD0 off */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff02e,  /* 120: BVN_VFD2 off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x0045800b,  /* 122: BVN_VFD4 2580ns */
           0x0045800c,  /* 123: BVN_VFD5 2580ns */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff001,  /* 126: BVN_CAP0 off */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff02a,  /* 128: BVN_CAP2 off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x004f6011,  /* 130: BVN_CAP4 2945.2736318408ns */
           0x004f6012,  /* 131: BVN_CAP5 2945.2736318408ns */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff007,  /* 134: BVN_GFD0 off */
           0x3ffff005,  /* 135: BVN_GFD1 off */
           0x0167c029,  /* 136: BVN_GFD2 13330ns */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff010,  /* 141: BVN_MCVP0 off */
           0x3ffff00f,  /* 142: BVN_MCVP1 off */
           0x3ffff027,  /* 143: BVN_MCVP2 off */
           0x00571014,  /* 144: BVN_RDC 3230ns */
           0x0352602c,  /* 145: VEC_VBI_ENC0 31500ns */
           0x0352602d,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80000058,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x80000059,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
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
           0x801e4036,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000062,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393038,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000063,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000005d,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x8000005c,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff0ff,  /* 218: UNASSIGNED off */
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
           0xbfffe051,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27026   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20170224011134_7439_4K_1080p_pal_3display[] = {
           0x3ffff002,  /*   0: XPT_WR_RS off */
           0x3ffff039,  /*   1: XPT_WR_XC off */
           0x3ffff00e,  /*   2: XPT_WR_CDB off */
           0x3ffff043,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff01d,  /*   4: XPT_RD_RS off */
           0x3ffff02b,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00d,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff040,  /*   7: XPT_RD_PB off */
           0x809d803e,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e604a,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd008,  /*  10: GENET0_WR RR 2370ns */
           0x81026044,  /*  11: GENET0_RD RR 10150ns */
           0x803fd009,  /*  12: GENET1_WR RR 2370ns */
           0x81026045,  /*  13: GENET1_RD RR 10150ns */
           0x803fd00a,  /*  14: GENET2_WR RR 2370ns */
           0x81026046,  /*  15: GENET2_RD RR 10150ns */
           0x8596e034,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066203b,  /*  17: SATA RR 4015ns */
           0x8066203c,  /*  18: SATA_1 RR 4015ns */
           0x3ffff02f,  /*  19: MCIF2_RD off */
           0x3ffff031,  /*  20: MCIF2_WR off */
           0x8161704c,  /*  21: HIF_PCIe1 RR 13880ns */
           0x8545e033,  /*  22: BSP RR 50000ns */
           0x80ad9041,  /*  23: SAGE RR 6820ns */
           0x8644904e,  /*  24: FLASH_DMA RR 63000ns */
           0x8161704b,  /*  25: HIF_PCIe RR 13880ns */
           0x86449050,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644904f,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff030,  /*  29: MCIF_RD off */
           0x3ffff032,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db047,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5049,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5048,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1042,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x3ffff03a,  /*  40: RAAGA off */
           0x3ffff000,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff023,  /*  44: AUD_AIO off */
           0x3ffff0ff,  /*  45: UNASSIGNED off */
           0x3ffff0ff,  /*  46: UNASSIGNED off */
           0x3ffff0ff,  /*  47: UNASSIGNED off */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x3ffff0ff,  /*  49: UNASSIGNED off */
           0x3ffff0ff,  /*  50: UNASSIGNED off */
           0x3ffff0ff,  /*  51: UNASSIGNED off */
           0x3ffff0ff,  /*  52: UNASSIGNED off */
           0x3ffff0ff,  /*  53: UNASSIGNED off */
           0x3ffff0ff,  /*  54: UNASSIGNED off */
           0x3ffff0ff,  /*  55: UNASSIGNED off */
           0x3ffff0ff,  /*  56: UNASSIGNED off */
           0x3ffff0ff,  /*  57: UNASSIGNED off */
           0x3ffff0ff,  /*  58: UNASSIGNED off */
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
           0x80000052,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x80000053,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff037,  /*  75: HVD0_ILCPU off */
           0x3ffff03f,  /*  76: HVD0_OLCPU off */
           0x3ffff006,  /*  77: HVD0_CAB off */
           0x3ffff01b,  /*  78: HVD0_ILSI off */
           0x81fea04d,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x3ffff01c,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff05a,  /*  81: HVD1_DBLK_0 off */
           0x3ffff05b,  /*  82: HVD1_DBLK_1 off */
           0x3ffff035,  /*  83: HVD1_ILCPU off */
           0x3ffff03d,  /*  84: HVD1_OLCPU off */
           0x3ffff018,  /*  85: HVD1_CAB off */
           0x3ffff022,  /*  86: HVD1_ILSI off */
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
           0x3ffff019,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff01e,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff024,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff01a,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff01f,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff025,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0058c015,  /* 106: BVN_MFD0 3292ns */
           0x0027d004,  /* 107: BVN_MFD0_1 1480.5ns */
           0x3ffff020,  /* 108: BVN_MFD1 off */
           0x3ffff016,  /* 109: BVN_MFD1_1 off */
           0x3ffff021,  /* 110: BVN_MFD2 off */
           0x3ffff017,  /* 111: BVN_MFD2_1 off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001fb003,  /* 118: BVN_VFD0 1179.10447761194ns */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x0359a02e,  /* 120: BVN_VFD2 31770ns */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff00b,  /* 122: BVN_VFD4 off */
           0x3ffff00c,  /* 123: BVN_VFD5 off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x001e4001,  /* 126: BVN_CAP0 1125.89073634204ns */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x0174702a,  /* 128: BVN_CAP2 13800ns */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff011,  /* 130: BVN_CAP4 off */
           0x3ffff012,  /* 131: BVN_CAP5 off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0031d007,  /* 134: BVN_GFD0 1851ns */
           0x0027d005,  /* 135: BVN_GFD1 1481ns */
           0x3ffff029,  /* 136: BVN_GFD2 off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b5010,  /* 141: BVN_MCVP0 2794ns */
           0x004b500f,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d027,  /* 143: BVN_MCVP2 7850ns */
           0x3ffff014,  /* 144: BVN_RDC off */
           0x3ffff02c,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff02d,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80000054,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x80000055,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
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
           0x801e4036,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000062,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393038,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000063,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000005e,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff05c,  /* 217: HVD1_PFRI off */
           0x3ffff0ff,  /* 218: UNASSIGNED off */
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
           0xbfffe051,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27026   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20170224011134_7439_4K_1080p_pal_3display[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80600802}, /* HVD0_PFRI_Ch (gHvdC0) 493440.00 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000029a}, /* d: 4; p: 666.14375 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00000808}, /* 2056 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x000004d1}, /* 60% * 2056 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x805f0803}, /* HVD1_PFRI (gHvd1) 439111.11 ns/60 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000316}, /* d: 4; p: 790.4 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x000008d8}, /* 2264 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x0000054e}, /* 60% * 2264 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80610803}, /* HVD0_PFRI (gHvd0) 493440.00 ns/80 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000029a}, /* d: 4; p: 666.14375 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000f90}, /* 3984 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000956}  /* 60% * 3984 */
};

static const uint32_t* const paulMemc_box30[] = { &aulMemc0_20170224011134_7439_4K_1080p_pal_3display[0], &aulMemc1_20170224011134_7439_4K_1080p_pal_3display[0]};

const BBOX_Rts stBoxRts_7439_4K_1080p_pal_3display_box30 = {
  "20170224011134_7439_4K_1080p_pal_3display_box30",
  7439,
  30,
  2,
  256,
  (const uint32_t**)&paulMemc_box30[0],
  12,
  stBoxRts_PfriClient_20170224011134_7439_4K_1080p_pal_3display
};
