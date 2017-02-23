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
 *   at: Wed Dec  7 01:41:24 2016 GMT
 *   by: fmombers
 *   for: Box 7278_box2
 *         MemC 0 (32-bit LPDDR4@1600MHz) w/486MHz clock
 *         MemC 1 (32-bit LPDDR4@1600MHz) w/486MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/BCM7278A0.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/BCM7278A0MemoryTiming.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/l_MEMC_b2r8_timing_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/VideoDecoder_rT.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/BCM7278A0BvnLib.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/Vice_v3.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/Raaga.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/BCM7278A0Client.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/BCM7278A0ClientTime.cfg
 *     xls_input/box_Box_Mid_performance.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/BCM7278A0ClientMap.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/BCM7278A0Display.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/utils.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278A0/Boxes/BCM7278A0_boxdoc.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20161207014124_7278_box2[] = {
           0x80b14016,  /*   0: XPT_WR_TUNER_RS RR 5840.77669902913ns */
           0x8072100b,  /*   1: XPT_WR_CARD_RS RR 3760ns */
           0x8068f02c,  /*   2: XPT_WR_CDB RR 3459.45945945946ns */
           0x81116036,  /*   3: XPT_WR_ITB_MSG RR 9005.9880239521ns */
           0x8138c038,  /*   4: XPT_RD_RS RR 10301.3698630137ns */
           0x8072102d,  /*   5: XPT_RD_CARD_RS RR 3760ns */
           0x81065033,  /*   6: XPT_RD_PB RR 8641.35021097046ns */
           0x804e1027,  /*   7: XPT_WR_MEMDMA RR 2574.71264367816ns */
           0x80b2a030,  /*   8: XPT_RD_MEMDMA RR 5885.05747126437ns */
           0x3ffff0ff,  /*   9: UNASSIGNED off */
           0x8141303a,  /*  10: SYSPORT_WR RR 10580ns */
           0x81413039,  /*  11: SYSPORT_RD RR 10580ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8108f035,  /*  15: HIF_PCIe1 RR 8727.27272727272ns */
           0x3ffff0ff,  /*  16: UNASSIGNED off */
           0x80b2a031,  /*  17: SATA RR 5885.05747126437ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x85eea020,  /*  22: BSP RR 50000ns */
           0x80c34032,  /*  23: SAGE RR 6820ns */
           0x81ca603e,  /*  24: FLASH_DMA RR 16000ns */
           0x8108f034,  /*  25: HIF_PCIe RR 8727.27272727272ns */
           0x809b602f,  /*  26: SDIO_EMMC RR 5120ns */
           0x8136e037,  /*  27: SDIO_CARD RR 10240ns */
           0x3ffff0ff,  /*  28: UNASSIGNED off */
           0x047f101e,  /*  29: MCIF_RD_0 37900ns */
           0x047f101f,  /*  30: MCIF_WR_0 37900ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x3ffff0ff,  /*  33: UNASSIGNED off */
           0x3ffff0ff,  /*  34: UNASSIGNED off */
           0x83f72044,  /*  35: USB_X_WRITE_0 RR 35428.5714285714ns */
           0x83f72043,  /*  36: USB_X_READ_0 RR 35428.5714285714ns */
           0x8ade1045,  /*  37: USB_X_CTRL_0 RR 97090.3703703704ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x8080d02e,  /*  40: RAAGA RR 4500ns */
           0x3ffff0ff,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00d2a017,  /*  44: AUD_AIO 6940ns */
           0x3ffff0ff,  /*  45: VICE_CME0 off */
           0x3ffff0ff,  /*  46: VICE_CME1 off */
           0x3ffff0ff,  /*  47: VICE_FME off */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x3ffff0ff,  /*  49: VICE_IMD0 off */
           0x3ffff0ff,  /*  50: VICE_IMD1 off */
           0x3ffff0ff,  /*  51: VICE_DBLK off */
           0x3ffff0ff,  /*  52: VICE_CABAC0 off */
           0x3ffff0ff,  /*  53: VICE_CABAC1 off */
           0x3ffff0ff,  /*  54: VICE_ARCSS0 off */
           0x3ffff0ff,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff0ff,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff0ff,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff0ff,  /*  58: VICE_VIP1_INST1 off */
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
           0x800bd048,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x800bd049,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x8058e028,  /*  75: HVD0_ILCPU RR 2931ns */
           0x803a5024,  /*  76: HVD0_OLCPU RR 1925ns */
           0x80353022,  /*  77: HVD0_CAB RR 1756ns */
           0x007a200e,  /*  78: HVD0_ILSI 4026ns */
           0x8058e029,  /*  79: HVD0_ILCPU_p2 RR 2931ns */
           0x007a200f,  /*  80: HVD0_ILSI_p2 4026ns */
           0x3ffff04e,  /*  81: HVD1_DBLK_0 off */
           0x3ffff04f,  /*  82: HVD1_DBLK_1 off */
           0x8058e02a,  /*  83: HVD1_ILCPU RR 2931ns */
           0x803a5025,  /*  84: HVD1_OLCPU RR 1925ns */
           0x80353023,  /*  85: HVD1_CAB RR 1756ns */
           0x007a2010,  /*  86: HVD1_ILSI 4026ns */
           0x3ffff0ff,  /*  87: UNASSIGNED off */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x0069f009,  /*  94: MADR_RD 3510ns +HRT(0.5%) */
           0x00951014,  /*  95: MADR_QM 4938ns +HRT(0.5%) */
           0x00d41018,  /*  96: MADR_WR 7021ns +HRT(0.5%) */
           0x3ffff0ff,  /*  97: MADR1_RD off */
           0x3ffff0ff,  /*  98: MADR1_QM off */
           0x3ffff0ff,  /*  99: MADR1_WR off */
           0x3ffff0ff,  /* 100: MADR2_RD off */
           0x3ffff0ff,  /* 101: MADR2_QM off */
           0x3ffff0ff,  /* 102: MADR2_WR off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0095d012,  /* 106: BVNF_MFD0_0 4938ns */
           0x001dd000,  /* 107: BVNF_MFD0_1 987ns */
           0x3ffff013,  /* 108: BVNF_MFD1_0 off */
           0x3ffff001,  /* 109: BVNF_MFD1_1 off */
           0x3ffff0ff,  /* 110: BVNF_MFD2_0 off */
           0x3ffff0ff,  /* 111: BVNF_MFD2_1 off */
           0x3ffff0ff,  /* 112: BVNF_MFD3_0 off */
           0x3ffff0ff,  /* 113: BVNF_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff002,  /* 118: BVNF_VFD0 off */
           0x3ffff003,  /* 119: BVNF_VFD1 off */
           0x3ffff0ff,  /* 120: BVNF_VFD2 off */
           0x3ffff0ff,  /* 121: BVNF_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff004,  /* 126: BVNF_CAP0 off */
           0x3ffff005,  /* 127: BVNF_CAP1 off */
           0x3ffff0ff,  /* 128: BVNF_CAP2 off */
           0x3ffff0ff,  /* 129: BVNF_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0077d00d,  /* 134: BVNF_GFD0 3950ns */
           0x01e2701c,  /* 135: BVNF_GFD1 15888ns */
           0x3ffff0ff,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: BVNF_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x0054b007,  /* 141: MCVP0 2808ns +HRT(0.5%) */
           0x0054b006,  /* 142: MCVP1 2808ns +HRT(0.5%) */
           0x00ed2019,  /* 143: MCVP_QM 7850ns +HRT(0.5%) */
           0x005b0008,  /* 144: BVNF_RDC 3000ns */
           0x03c4e01d,  /* 145: VEC_VBI_ENC 31770ns */
           0x01a9201b,  /* 146: VEC_HDR0 14000ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x800bd04a,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x800bd04b,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0xbffff0ff,  /* 163: MM_M2MC1_0 RR */
           0xbffff0ff,  /* 164: MM_M2MC1_1 RR */
           0xbffff0ff,  /* 165: MM_M2MC1_2 RR */
           0x009cc015,  /* 166: CMP0 5166ns */
           0x800bd04c,  /* 167: HVD1_DBLK_p2_0 RR 0ns */
           0x800bd04d,  /* 168: HVD1_DBLK_p2_1 RR 0ns */
           0x8058e02b,  /* 169: HVD1_ILCPU_p2 RR 2931ns */
           0x007a2011,  /* 170: HVD1_ILSI_p2 4026ns */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x82d31042,  /* 172: ASP_WR_ARCSS_DDMA1 RR 23809.5238095238ns */
           0x82c24040,  /* 173: ASP_WR_ARCSS_DDMA2 RR 23255.8139534884ns */
           0x806fd00a,  /* 174: ASP_WR_EDPKT_AVIN RR 3686.99029700481ns */
           0x81afd03d,  /* 175: ASP_WR_EPKT_AVRTR RR 14222.2222222222ns */
           0x82d31041,  /* 176: ASP_RD_ARCSS_DDMA1 RR 23809.5238095238ns */
           0x82c2403f,  /* 177: ASP_RD_ARCSS_DDMA2 RR 23255.8139534884ns */
           0x8172203b,  /* 178: ASP_RD_MCPB_AVOUT RR 12190.4761904762ns */
           0xa3986047,  /* 179: ASP_RD_EDPKT_AVHDR RR 300000ns */
           0xbffff0ff,  /* 180: USB_BDC RR */
           0x8177901a,  /* 181: WIFI_SYSPORT_WR RR 12370ns */
           0x8177903c,  /* 182: WIFI_SYSPORT_RD RR 12370ns */
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
           0x3ffff0ff,  /* 199: BSP_CPU off */
           0x80202021,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1125ns */
           0x80000054,  /* 201: HOST_CPU_MCP_RD_LOW RR */
           0x80405026,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x80000055,  /* 203: HOST_CPU_MCP_WR_LOW RR */
           0xbffff0ff,  /* 204: V3D_MCP_R_HI RR */
           0xbffff0ff,  /* 205: V3D_MCP_R_LO RR */
           0xbffff0ff,  /* 206: V3D_MCP_W_HI RR */
           0xbffff0ff,  /* 207: V3D_MCP_W_LO RR */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x80000050,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff051,  /* 217: HVD1_PFRI off */
           0x3ffff0ff,  /* 218: VICE_PFRI off */
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
           0xbffff0ff,  /* 249: MEMC_DTU_SCRUBBER RR */
           0x8bdd6046,  /* 250: MEMC_ZQCS RR 100000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x0076700c   /* 255: REFRESH 3904ns */
         };
static const uint32_t aulMemc1_20161207014124_7278_box2[] = {
           0x3ffff016,  /*   0: XPT_WR_TUNER_RS off */
           0x3ffff00b,  /*   1: XPT_WR_CARD_RS off */
           0x3ffff02c,  /*   2: XPT_WR_CDB off */
           0x3ffff036,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff038,  /*   4: XPT_RD_RS off */
           0x3ffff02d,  /*   5: XPT_RD_CARD_RS off */
           0x3ffff033,  /*   6: XPT_RD_PB off */
           0x804e1027,  /*   7: XPT_WR_MEMDMA RR 2574.71264367816ns */
           0x80b2a030,  /*   8: XPT_RD_MEMDMA RR 5885.05747126437ns */
           0x3ffff0ff,  /*   9: UNASSIGNED off */
           0x8141303a,  /*  10: SYSPORT_WR RR 10580ns */
           0x81413039,  /*  11: SYSPORT_RD RR 10580ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8108f035,  /*  15: HIF_PCIe1 RR 8727.27272727272ns */
           0x3ffff0ff,  /*  16: UNASSIGNED off */
           0x80b2a031,  /*  17: SATA RR 5885.05747126437ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x85eea020,  /*  22: BSP RR 50000ns */
           0x80c34032,  /*  23: SAGE RR 6820ns */
           0x81ca603e,  /*  24: FLASH_DMA RR 16000ns */
           0x8108f034,  /*  25: HIF_PCIe RR 8727.27272727272ns */
           0x809b602f,  /*  26: SDIO_EMMC RR 5120ns */
           0x8136e037,  /*  27: SDIO_CARD RR 10240ns */
           0x3ffff0ff,  /*  28: UNASSIGNED off */
           0x3ffff01e,  /*  29: MCIF_RD_0 off */
           0x3ffff01f,  /*  30: MCIF_WR_0 off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x3ffff0ff,  /*  33: UNASSIGNED off */
           0x3ffff0ff,  /*  34: UNASSIGNED off */
           0x83f72044,  /*  35: USB_X_WRITE_0 RR 35428.5714285714ns */
           0x83f72043,  /*  36: USB_X_READ_0 RR 35428.5714285714ns */
           0x8ade1045,  /*  37: USB_X_CTRL_0 RR 97090.3703703704ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x3ffff02e,  /*  40: RAAGA off */
           0x3ffff0ff,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff017,  /*  44: AUD_AIO off */
           0x3ffff0ff,  /*  45: VICE_CME0 off */
           0x3ffff0ff,  /*  46: VICE_CME1 off */
           0x3ffff0ff,  /*  47: VICE_FME off */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x3ffff0ff,  /*  49: VICE_IMD0 off */
           0x3ffff0ff,  /*  50: VICE_IMD1 off */
           0x3ffff0ff,  /*  51: VICE_DBLK off */
           0x3ffff0ff,  /*  52: VICE_CABAC0 off */
           0x3ffff0ff,  /*  53: VICE_CABAC1 off */
           0x3ffff0ff,  /*  54: VICE_ARCSS0 off */
           0x3ffff0ff,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff0ff,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff0ff,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff0ff,  /*  58: VICE_VIP1_INST1 off */
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
           0x3ffff048,  /*  73: HVD0_DBLK_0 off */
           0x3ffff049,  /*  74: HVD0_DBLK_1 off */
           0x3ffff028,  /*  75: HVD0_ILCPU off */
           0x3ffff024,  /*  76: HVD0_OLCPU off */
           0x3ffff022,  /*  77: HVD0_CAB off */
           0x3ffff00e,  /*  78: HVD0_ILSI off */
           0x3ffff029,  /*  79: HVD0_ILCPU_p2 off */
           0x3ffff00f,  /*  80: HVD0_ILSI_p2 off */
           0x800bd04e,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x800bd04f,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff02a,  /*  83: HVD1_ILCPU off */
           0x3ffff025,  /*  84: HVD1_OLCPU off */
           0x3ffff023,  /*  85: HVD1_CAB off */
           0x3ffff010,  /*  86: HVD1_ILSI off */
           0x3ffff0ff,  /*  87: UNASSIGNED off */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff009,  /*  94: MADR_RD off */
           0x3ffff014,  /*  95: MADR_QM off */
           0x3ffff018,  /*  96: MADR_WR off */
           0x3ffff0ff,  /*  97: MADR1_RD off */
           0x3ffff0ff,  /*  98: MADR1_QM off */
           0x3ffff0ff,  /*  99: MADR1_WR off */
           0x3ffff0ff,  /* 100: MADR2_RD off */
           0x3ffff0ff,  /* 101: MADR2_QM off */
           0x3ffff0ff,  /* 102: MADR2_WR off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x3ffff012,  /* 106: BVNF_MFD0_0 off */
           0x3ffff000,  /* 107: BVNF_MFD0_1 off */
           0x0095d013,  /* 108: BVNF_MFD1_0 4938ns */
           0x001dd001,  /* 109: BVNF_MFD1_1 987ns */
           0x3ffff0ff,  /* 110: BVNF_MFD2_0 off */
           0x3ffff0ff,  /* 111: BVNF_MFD2_1 off */
           0x3ffff0ff,  /* 112: BVNF_MFD3_0 off */
           0x3ffff0ff,  /* 113: BVNF_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001f1002,  /* 118: BVNF_VFD0 1027ns */
           0x001f1003,  /* 119: BVNF_VFD1 1027ns */
           0x3ffff0ff,  /* 120: BVNF_VFD2 off */
           0x3ffff0ff,  /* 121: BVNF_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x001f5004,  /* 126: BVNF_CAP0 1041ns +HRT(0.5%) */
           0x001f5005,  /* 127: BVNF_CAP1 1041ns +HRT(0.5%) */
           0x3ffff0ff,  /* 128: BVNF_CAP2 off */
           0x3ffff0ff,  /* 129: BVNF_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff00d,  /* 134: BVNF_GFD0 off */
           0x3ffff01c,  /* 135: BVNF_GFD1 off */
           0x3ffff0ff,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: BVNF_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff007,  /* 141: MCVP0 off */
           0x3ffff006,  /* 142: MCVP1 off */
           0x3ffff019,  /* 143: MCVP_QM off */
           0x3ffff008,  /* 144: BVNF_RDC off */
           0x3ffff01d,  /* 145: VEC_VBI_ENC off */
           0x3ffff01b,  /* 146: VEC_HDR0 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x3ffff04a,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff04b,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0xbffff0ff,  /* 163: MM_M2MC1_0 RR */
           0xbffff0ff,  /* 164: MM_M2MC1_1 RR */
           0xbffff0ff,  /* 165: MM_M2MC1_2 RR */
           0x3ffff015,  /* 166: CMP0 off */
           0x800bd04c,  /* 167: HVD1_DBLK_p2_0 RR 0ns */
           0x800bd04d,  /* 168: HVD1_DBLK_p2_1 RR 0ns */
           0x3ffff02b,  /* 169: HVD1_ILCPU_p2 off */
           0x3ffff011,  /* 170: HVD1_ILSI_p2 off */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x3ffff042,  /* 172: ASP_WR_ARCSS_DDMA1 off */
           0x3ffff040,  /* 173: ASP_WR_ARCSS_DDMA2 off */
           0x3ffff00a,  /* 174: ASP_WR_EDPKT_AVIN off */
           0x3ffff03d,  /* 175: ASP_WR_EPKT_AVRTR off */
           0x3ffff041,  /* 176: ASP_RD_ARCSS_DDMA1 off */
           0x3ffff03f,  /* 177: ASP_RD_ARCSS_DDMA2 off */
           0x3ffff03b,  /* 178: ASP_RD_MCPB_AVOUT off */
           0x3ffff047,  /* 179: ASP_RD_EDPKT_AVHDR off */
           0xbffff0ff,  /* 180: USB_BDC RR */
           0x3ffff01a,  /* 181: WIFI_SYSPORT_WR off */
           0x3ffff03c,  /* 182: WIFI_SYSPORT_RD off */
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
           0x3ffff0ff,  /* 199: BSP_CPU off */
           0x80202021,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1125ns */
           0x80000054,  /* 201: HOST_CPU_MCP_RD_LOW RR */
           0x80405026,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x80000055,  /* 203: HOST_CPU_MCP_WR_LOW RR */
           0xbffff0ff,  /* 204: V3D_MCP_R_HI RR */
           0xbffff0ff,  /* 205: V3D_MCP_R_LO RR */
           0xbffff0ff,  /* 206: V3D_MCP_W_HI RR */
           0xbffff0ff,  /* 207: V3D_MCP_W_LO RR */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x3ffff050,  /* 216: HVD0_PFRI off */
           0x80000051,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff0ff,  /* 218: VICE_PFRI off */
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
           0xbffff0ff,  /* 249: MEMC_DTU_SCRUBBER RR */
           0x8bdd6046,  /* 250: MEMC_ZQCS RR 100000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x0076700c   /* 255: REFRESH 3904ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20161207014124_7278_box2[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80520e05}, /* HVD0_PFRI (gHVC0) 236520.00 ns/40 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x400002ce}, /* d: 4; p: 718.425 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00001620}, /* 5664 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00000d46}, /* 60% * 5664 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80530e05}, /* HVD1_PFRI (gHVC1) 236520.00 ns/40 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x400002ce}, /* d: 4; p: 718.425 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x00001620}, /* 5664 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x00000d46}  /* 60% * 5664 */
};

static const uint32_t* const paulMemc_box2[] = { &aulMemc0_20161207014124_7278_box2[0], &aulMemc1_20161207014124_7278_box2[0]};

const BBOX_Rts stBoxRts_7278_box2_box2 = {
  "20161207014124_7278_box2_box2",
  7278,
  2,
  2,
  256,
  (const uint32_t**)&paulMemc_box2[0],
  8,
  stBoxRts_PfriClient_20161207014124_7278_box2
};
