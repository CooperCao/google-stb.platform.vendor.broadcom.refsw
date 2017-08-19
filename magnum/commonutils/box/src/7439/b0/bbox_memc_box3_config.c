/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *   at: Mon Jul 10 21:38:50 2017 GMT
 *   by: rc905201
 *   for: Box 7252S_4K_gfxpip
 *         MemC 0 (32-bit DDR3@933MHz) w/432MHz clock
 *         MemC 1 (32-bit DDR3@933MHz) w/432MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439B0_Robin/BCM7439B0_box3.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20170710213850_7252S_4K_gfxpip[] = {
           0x001e6001,  /*   0: XPT_WR_RS 1130ns */
           0x803a3033,  /*   1: XPT_WR_XC RR 2290ns */
           0x8049000b,  /*   2: XPT_WR_CDB RR 2710ns */
           0x80b9103c,  /*   3: XPT_WR_ITB_MSG RR 7270ns */
           0x80780014,  /*   4: XPT_RD_RS RR 4450ns */
           0x819e5020,  /*   5: XPT_RD_XC_RMX_MSG RR 15350ns */
           0x8049000a,  /*   6: XPT_RD_XC_RAVE RR 2710ns */
           0x80a2e039,  /*   7: XPT_RD_PB RR 6400ns */
           0x809d8037,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e6044,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd008,  /*  10: GENET0_WR RR 2370ns */
           0x8102603d,  /*  11: GENET0_RD RR 10150ns */
           0x8089d017,  /*  12: GENET1_WR RR 5110ns */
           0x8102603e,  /*  13: GENET1_RD RR 10150ns */
           0x803fd009,  /*  14: GENET2_WR RR 2370ns */
           0x8102603f,  /*  15: GENET2_RD RR 10150ns */
           0x8596e02e,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662035,  /*  17: SATA RR 4015ns */
           0x80662036,  /*  18: SATA_1 RR 4015ns */
           0x03e6e029,  /*  19: MCIF2_RD 37000ns */
           0x03e6e02b,  /*  20: MCIF2_WR 37000ns */
           0x81617046,  /*  21: HIF_PCIe1 RR 13880ns */
           0x8545e02d,  /*  22: BSP RR 50000ns */
           0x80ad903a,  /*  23: SAGE RR 6820ns */
           0x86449048,  /*  24: FLASH_DMA RR 63000ns */
           0x81617045,  /*  25: HIF_PCIe RR 13880ns */
           0x8644904a,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449049,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e02a,  /*  29: MCIF_RD 37000ns */
           0x03e6e02c,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db040,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5043,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5042,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae103b,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x801ae02f,  /*  40: RAAGA RR 1000ns */
           0x001ae000,  /*  41: RAAGA_1 1000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00bb4019,  /*  44: AUD_AIO 6940ns */
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
           0x80000050,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x80000051,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x80270031,  /*  75: HVD0_ILCPU RR 1451ns */
           0x809ee038,  /*  76: HVD0_OLCPU RR 6242ns */
           0x002d0005,  /*  77: HVD0_CAB 1671.5ns */
           0x0071a012,  /*  78: HVD0_ILSI 4214ns */
           0x81fea047,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x00726013,  /*  80: HVD0_ILSI_p2 4242ns */
           0x80000054,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x80000055,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x803b3034,  /*  83: HVD1_ILCPU RR 2198ns */
           0x81444041,  /*  84: HVD1_OLCPU RR 12734ns */
           0x00916018,  /*  85: HVD1_CAB 5390ns */
           0x01a6f021,  /*  86: HVD1_ILSI 15669ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x01c45022,  /*  94: BVN_MAD_PIX_FD 16758ns */
           0x027cb023,  /*  95: BVN_MAD_QUANT 23586ns */
           0x0388d028,  /*  96: BVN_MAD_PIX_CAP 33517ns */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x010a801c,  /* 106: BVN_MFD0_Ch 9876ns */
           0x004fd00e,  /* 107: BVN_MFD0_Ch_1 2961ns */
           0x00853016,  /* 108: BVN_MFD1 4938ns */
           0x0058c011,  /* 109: BVN_MFD1_1 3292ns */
           0x3ffff0ff,  /* 110: UNASSIGNED off */
           0x3ffff0ff,  /* 111: UNASSIGNED off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff002,  /* 118: BVN_VFD0 off */
           0x003f8007,  /* 119: BVN_VFD1 2358.20895522388ns */
           0x0359a026,  /* 120: BVN_VFD2 31770ns */
           0x0359a027,  /* 121: BVN_VFD3 31770ns */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff003,  /* 126: BVN_CAP0 off */
           0x007f3015,  /* 127: BVN_CAP1 4716.41791044776ns */
           0x0174701e,  /* 128: BVN_CAP2 13800ns */
           0x0174701f,  /* 129: BVN_CAP3 13800ns */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff006,  /* 134: BVN_GFD0 off */
           0x0167c01d,  /* 135: BVN_GFD1 13330ns */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff00d,  /* 141: BVN_MCVP0 off */
           0x3ffff00c,  /* 142: BVN_MCVP1 off */
           0x3ffff01b,  /* 143: BVN_MCVP2 off */
           0x0057100f,  /* 144: BVN_RDC 3230ns */
           0x03526024,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526025,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80000052,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x80000053,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
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
           0x801e4030,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x8000005c,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393032,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x8000005d,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000056,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x80000058,  /* 217: HVD1_PFRI RR 0ns */
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
           0xbfffe04b,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2701a   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20170710213850_7252S_4K_gfxpip[] = {
           0x3ffff001,  /*   0: XPT_WR_RS off */
           0x3ffff033,  /*   1: XPT_WR_XC off */
           0x3ffff00b,  /*   2: XPT_WR_CDB off */
           0x3ffff03c,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff014,  /*   4: XPT_RD_RS off */
           0x3ffff020,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff039,  /*   7: XPT_RD_PB off */
           0x809d8037,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e6044,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd008,  /*  10: GENET0_WR RR 2370ns */
           0x8102603d,  /*  11: GENET0_RD RR 10150ns */
           0x8089d017,  /*  12: GENET1_WR RR 5110ns */
           0x8102603e,  /*  13: GENET1_RD RR 10150ns */
           0x803fd009,  /*  14: GENET2_WR RR 2370ns */
           0x8102603f,  /*  15: GENET2_RD RR 10150ns */
           0x8596e02e,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662035,  /*  17: SATA RR 4015ns */
           0x80662036,  /*  18: SATA_1 RR 4015ns */
           0x3ffff029,  /*  19: MCIF2_RD off */
           0x3ffff02b,  /*  20: MCIF2_WR off */
           0x81617046,  /*  21: HIF_PCIe1 RR 13880ns */
           0x8545e02d,  /*  22: BSP RR 50000ns */
           0x80ad903a,  /*  23: SAGE RR 6820ns */
           0x86449048,  /*  24: FLASH_DMA RR 63000ns */
           0x81617045,  /*  25: HIF_PCIe RR 13880ns */
           0x8644904a,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449049,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff02a,  /*  29: MCIF_RD off */
           0x3ffff02c,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db040,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5043,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5042,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae103b,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x3ffff02f,  /*  40: RAAGA off */
           0x3ffff000,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff019,  /*  44: AUD_AIO off */
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
           0x8000004c,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8000004d,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff031,  /*  75: HVD0_ILCPU off */
           0x3ffff038,  /*  76: HVD0_OLCPU off */
           0x3ffff005,  /*  77: HVD0_CAB off */
           0x3ffff012,  /*  78: HVD0_ILSI off */
           0x81fea047,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x3ffff013,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff054,  /*  81: HVD1_DBLK_0 off */
           0x3ffff055,  /*  82: HVD1_DBLK_1 off */
           0x3ffff034,  /*  83: HVD1_ILCPU off */
           0x3ffff041,  /*  84: HVD1_OLCPU off */
           0x3ffff018,  /*  85: HVD1_CAB off */
           0x3ffff021,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff022,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff023,  /*  95: BVN_MAD_QUANT off */
           0x3ffff028,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0058c010,  /* 106: BVN_MFD0 3292ns */
           0x0027d004,  /* 107: BVN_MFD0_1 1480.5ns */
           0x3ffff016,  /* 108: BVN_MFD1 off */
           0x3ffff011,  /* 109: BVN_MFD1_1 off */
           0x3ffff0ff,  /* 110: UNASSIGNED off */
           0x3ffff0ff,  /* 111: UNASSIGNED off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001fb002,  /* 118: BVN_VFD0 1179.10447761194ns */
           0x3ffff007,  /* 119: BVN_VFD1 off */
           0x3ffff026,  /* 120: BVN_VFD2 off */
           0x3ffff027,  /* 121: BVN_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x001fb003,  /* 126: BVN_CAP0 1179.10447761194ns */
           0x3ffff015,  /* 127: BVN_CAP1 off */
           0x3ffff01e,  /* 128: BVN_CAP2 off */
           0x3ffff01f,  /* 129: BVN_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0031d006,  /* 134: BVN_GFD0 1851ns */
           0x3ffff01d,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b500d,  /* 141: BVN_MCVP0 2794ns */
           0x004b500c,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d01b,  /* 143: BVN_MCVP2 7850ns */
           0x3ffff00f,  /* 144: BVN_RDC off */
           0x3ffff024,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff025,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x8000004e,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8000004f,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
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
           0x801e4030,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x8000005c,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393032,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x8000005d,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000057,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff058,  /* 217: HVD1_PFRI off */
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
           0xbfffe04b,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2701a   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20170710213850_7252S_4K_gfxpip[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80590702}, /* HVD0_PFRI_Ch (gHvdC0) 493440.00 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000029a}, /* d: 4; p: 666.14375 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x000007c8}, /* 1992 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x000004ab}, /* 60% * 1992 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x805b0702}, /* HVD1_PFRI (gHvd1) 916320.00 ns/60 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000671}, /* d: 4; p: 1649.375 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x000008bd}, /* 2237 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x0000053e}, /* 60% * 2237 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x805a0702}, /* HVD0_PFRI (gHvd0) 493440.00 ns/80 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000029a}, /* d: 4; p: 666.14375 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000f56}, /* 3926 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000933}  /* 60% * 3926 */
};

static const uint32_t* const paulMemc_box3[] = { &aulMemc0_20170710213850_7252S_4K_gfxpip[0], &aulMemc1_20170710213850_7252S_4K_gfxpip[0]};

const BBOX_Rts stBoxRts_7252S_4K_gfxpip_box3 = {
  "20170710213850_7252S_4K_gfxpip_box3",
  7439,
  3,
  2,
  256,
  (const uint32_t**)&paulMemc_box3[0],
  12,
  stBoxRts_PfriClient_20170710213850_7252S_4K_gfxpip
};
