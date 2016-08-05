/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
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
 *   at: Tue Jul 19 23:46:53 2016 GMT
 *   by: robinc
 *   for: Box 7251_4kstb50
 *         MemC 0 (32-bit DDR4@1186.812MHz) w/432MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439B0_Robin/BCM7439B0_4kp50_192pip_adj.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20160719234653_7251_4kstb50[] = {
           0x007d2016,  /*   0: XPT_WR_RS 4640ns */
           0x814a3054,  /*   1: XPT_WR_XC RR 12970ns */
           0x8182802d,  /*   2: XPT_WR_CDB RR 14320ns */
           0x84a4e05f,  /*   3: XPT_WR_ITB_MSG RR 46680ns */
           0x82163030,  /*   4: XPT_RD_RS RR 19790ns */
           0x9a6ff043,  /*   5: XPT_RD_XC_RMX_MSG RR 250670ns */
           0x8182802c,  /*   6: XPT_RD_XC_RAVE RR 14320ns */
           0x82b7805d,  /*   7: XPT_RD_PB RR 27310ns */
           0x828bf05c,  /*   8: XPT_WR_MEMDMA RR 25600ns */
           0x85d25060,  /*   9: XPT_RD_MEMDMA RR 58514ns */
           0x803fd005,  /*  10: GENET0_WR RR 2370ns */
           0x81026050,  /*  11: GENET0_RD RR 10150ns */
           0x803fd006,  /*  12: GENET1_WR RR 2370ns */
           0x81026051,  /*  13: GENET1_RD RR 10150ns */
           0x803fd007,  /*  14: GENET2_WR RR 2370ns */
           0x81026052,  /*  15: GENET2_RD RR 10150ns */
           0x8596e041,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662049,  /*  17: SATA RR 4015ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x03e6e03c,  /*  19: MCIF2_RD 37000ns */
           0x03e6e03e,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e040,  /*  22: BSP RR 50000ns */
           0x80ad904d,  /*  23: SAGE RR 6820ns */
           0x86449061,  /*  24: FLASH_DMA RR 63000ns */
           0x81617057,  /*  25: HIF_PCIe RR 13880ns */
           0x86449063,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449062,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e03d,  /*  29: MCIF_RD 37000ns */
           0x03e6e03f,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db053,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5056,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5055,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae104e,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x00a1e022,  /*  40: RAAGA 6000ns */
           0x3fffe044,  /*  41: RAAGA_1 1000000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x0236e031,  /*  44: AUD_AIO 21000ns */
           0x8072804a,  /*  45: VICE_CME_RMB_CMB RR 4500ns */
           0x8394e05e,  /*  46: VICE_CME_CSC RR 36000ns */
           0x80d4104f,  /*  47: VICE_FME_CSC RR 8330ns */
           0x81a6b05a,  /*  48: VICE_FME_Luma_CMB RR 16600ns */
           0x81a6b059,  /*  49: VICE_FME_Chroma_CMB RR 16600ns */
           0x82574032,  /*  50: VICE_SG RR 22200ns */
           0x80165069,  /*  51: VICE_DBLK RR 10ns */
           0x83dc103b,  /*  52: VICE_CABAC0 RR 36600ns */
           0x86f07042,  /*  53: VICE_CABAC1 RR 65800ns */
           0x80a2e04c,  /*  54: VICE_ARCSS0 RR 6400ns */
           0x80c7d025,  /*  55: VICE_VIP0_INST0 RR 7406ns */
           0x82574033,  /*  56: VICE_VIP1_INST0 RR 22200ns */
           0x80c7d026,  /*  57: VICE_VIP0_INST1 RR 7406ns */
           0x82574034,  /*  58: VICE_VIP1_INST1 RR 22200ns */
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
           0x80028065,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x80028066,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x80270046,  /*  75: HVD0_ILCPU RR 1451ns */
           0x809ee04b,  /*  76: HVD0_OLCPU RR 6242ns */
           0x002d0003,  /*  77: HVD0_CAB 1671.5ns */
           0x0071a014,  /*  78: HVD0_ILSI 4214ns */
           0x81fea05b,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x00726015,  /*  80: HVD0_ILSI_p2 4242ns */
           0x8018e06a,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8018e06b,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x803b3048,  /*  83: HVD1_ILCPU RR 2198ns */
           0x8188e058,  /*  84: HVD1_OLCPU RR 15429ns */
           0x00973020,  /*  85: HVD1_CAB 5606ns */
           0x01b7f02e,  /*  86: HVD1_ILSI 16299ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x01c4502f,  /*  94: BVN_MAD_PIX_FD 16758ns */
           0x027cb035,  /*  95: BVN_MAD_QUANT 23586ns */
           0x0388d03a,  /*  96: BVN_MAD_PIX_CAP 33517ns */
           0x005e2012,  /*  97: BVN_MAD1_PIX_FD 3493ns */
           0x00848017,  /*  98: BVN_MAD1_QUANT 4914ns */
           0x00bc7023,  /*  99: BVN_MAD1_PIX_CAP 6986ns */
           0x005e2013,  /* 100: BVN_MAD2_PIX_FD 3493ns */
           0x00848018,  /* 101: BVN_MAD2_QUANT 4914ns */
           0x00bc7024,  /* 102: BVN_MAD2_PIX_CAP 6986ns */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0047b008,  /* 106: BVN_MFD0 2661.6170212766ns */
           0x0019b000,  /* 107: BVN_MFD0_1 957.6ns */
           0x00853019,  /* 108: BVN_MFD1 4938ns */
           0x0058c00f,  /* 109: BVN_MFD1_1 3292ns */
           0x0085301a,  /* 110: BVN_MFD2 4938ns */
           0x0058c010,  /* 111: BVN_MFD2_1 3292ns */
           0x0085301b,  /* 112: BVN_MFD3 4938ns */
           0x0058c011,  /* 113: BVN_MFD3_1 3292ns */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x00261002,  /* 118: BVN_VFD0 1414.92537313433ns */
           0x004c400b,  /* 119: BVN_VFD1 2829.85074626866ns */
           0x0359a038,  /* 120: BVN_VFD2 31770ns */
           0x0359a039,  /* 121: BVN_VFD3 31770ns */
           0x0095101c,  /* 122: BVN_VFD4 5527.3631840796ns */
           0x0095101d,  /* 123: BVN_VFD5 5527.3631840796ns */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x00245001,  /* 126: BVN_CAP0 1351.06888361045ns */
           0x0098a021,  /* 127: BVN_CAP1 5659.70149253731ns */
           0x0174702a,  /* 128: BVN_CAP2 13800ns */
           0x0174702b,  /* 129: BVN_CAP3 13800ns */
           0x0095101e,  /* 130: BVN_CAP4 5527.3631840796ns */
           0x0095101f,  /* 131: BVN_CAP5 5527.3631840796ns */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x003bd004,  /* 134: BVN_GFD0 2221.2ns */
           0x0167c029,  /* 135: BVN_GFD1 13330ns */
           0x0055900c,  /* 136: BVN_GFD2 3174ns */
           0x0055900d,  /* 137: BVN_GFD3 3174ns */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b500a,  /* 141: BVN_MCVP0 2794ns */
           0x004b5009,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d028,  /* 143: BVN_MCVP2 7850ns */
           0x0057100e,  /* 144: BVN_RDC 3230ns */
           0x03526036,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526037,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80028067,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x80028068,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
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
           0x80000072,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393047,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000073,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000006d,  /* 216: HVD0_PFRI RR 0ns */
           0x8000006e,  /* 217: HVD1_PFRI RR 0ns */
           0x8000206c,  /* 218: VICE_PFRI RR 10ns */
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
           0xbfffe064,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27027   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20160719234653_7251_4kstb50[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80700905}, /* HVD0_PFRI (gHvd0) 493440.00 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000029a}, /* d: 4; p: 666.14375 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x000019d1}, /* 6609 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00000f7d}, /* 60% * 6609 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x80710905}, /* HVD1_PFRI (gHvd1) 11381727.27 ns/60 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x40005007}, /* d: 4; p: 20487.1083333333 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x00000b0d}, /* 2829 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x000006a1}, /* 60% * 2829 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x806f0905}, /* VICE_PFRI (gVice) 333333.33 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x400001c1}, /* d: 4; p: 449.996875 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x000001c2}, /* 450 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x0000010e}  /* 60% * 450 */
};

static const uint32_t* const paulMemc_box26[] = { &aulMemc0_20160719234653_7251_4kstb50[0]};

const BBOX_Rts stBoxRts_7251_4kstb50_box26 = {
  "20160719234653_7251_4kstb50_box26",
  7439,
  26,
  1,
  256,
  (const uint32_t**)&paulMemc_box26[0],
  12,
  stBoxRts_PfriClient_20160719234653_7251_4kstb50
};
