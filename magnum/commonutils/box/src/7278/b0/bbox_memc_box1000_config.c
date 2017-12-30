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
 *
 ******************************************************************************
 *
 *                            Do Not Edit Directly
 * Auto-Generated from RTS environment:
 *   at: Fri Dec 16 02:02:52 2016 GMT
 *   by: fmombers
 *   for: Box 7278_box1000
 *         MemC 0 (32-bit LPDDR4@1600MHz) w/486MHz clock
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

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20161216020252_7278_box1000[] = {
           0x80b14016,  /*   0: XPT_WR_TUNER_RS RR 5840.77669902913ns */
           0x8072100e,  /*   1: XPT_WR_CARD_RS RR 3760ns */
           0x8068f029,  /*   2: XPT_WR_CDB RR 3459.45945945946ns */
           0x81116036,  /*   3: XPT_WR_ITB_MSG RR 9005.9880239521ns */
           0x8138c039,  /*   4: XPT_RD_RS RR 10301.3698630137ns */
           0x8072102a,  /*   5: XPT_RD_CARD_RS RR 3760ns */
           0x81065033,  /*   6: XPT_RD_PB RR 8641.35021097046ns */
           0x804e1026,  /*   7: XPT_WR_MEMDMA RR 2574.71264367816ns */
           0x80b2a02e,  /*   8: XPT_RD_MEMDMA RR 5885.05747126437ns */
           0x3ffff0ff,  /*   9: UNASSIGNED off */
           0x8141303b,  /*  10: SYSPORT_WR RR 10580ns */
           0x8141303a,  /*  11: SYSPORT_RD RR 10580ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8108f035,  /*  15: HIF_PCIe1 RR 8727.27272727272ns */
           0x3ffff0ff,  /*  16: UNASSIGNED off */
           0x80b2a02f,  /*  17: SATA RR 5885.05747126437ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x85eea01f,  /*  22: BSP RR 50000ns */
           0x80c34030,  /*  23: SAGE RR 6820ns */
           0x81ca603f,  /*  24: FLASH_DMA RR 16000ns */
           0x8108f034,  /*  25: HIF_PCIe RR 8727.27272727272ns */
           0x809b602d,  /*  26: SDIO_EMMC RR 5120ns */
           0x8136e038,  /*  27: SDIO_CARD RR 10240ns */
           0x3ffff0ff,  /*  28: UNASSIGNED off */
           0x047f101d,  /*  29: MCIF_RD_0 37900ns */
           0x047f101e,  /*  30: MCIF_WR_0 37900ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x3ffff0ff,  /*  33: UNASSIGNED off */
           0x3ffff0ff,  /*  34: UNASSIGNED off */
           0x83f72046,  /*  35: USB_X_WRITE_0 RR 35428.5714285714ns */
           0x83f72045,  /*  36: USB_X_READ_0 RR 35428.5714285714ns */
           0x8ade1047,  /*  37: USB_X_CTRL_0 RR 97090.3703703704ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x8013f021,  /*  40: RAAGA RR 700ns */
           0x3ffff0ff,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00d2a017,  /*  44: AUD_AIO 6940ns */
           0x80325023,  /*  45: VICE_CME0 RR 1760ns */
           0x831e4044,  /*  46: VICE_CME1 RR 27860ns */
           0x8112b037,  /*  47: VICE_FME RR 9590ns */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x80d27031,  /*  49: VICE_IMD0 RR 7350ns */
           0x80d27032,  /*  50: VICE_IMD1 RR 7350ns */
           0x800b004a,  /*  51: VICE_DBLK RR 0ns */
           0xbfffe020,  /*  52: VICE_CABAC0 RR 800000ns */
           0x8117501a,  /*  53: VICE_CABAC1 RR 9200ns */
           0x8067c028,  /*  54: VICE_ARCSS0 RR 3625ns */
           0x8070900a,  /*  55: VICE_VIP0_INST0 RR 3710ns */
           0x8070900c,  /*  56: VICE_VIP1_INST0 RR 3710ns */
           0x8070900b,  /*  57: VICE_VIP0_INST1 RR 3710ns */
           0x8070900d,  /*  58: VICE_VIP1_INST1 RR 3710ns */
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
           0x8017b04b,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8017b04c,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x8086102b,  /*  75: HVD0_ILCPU RR 4419ns */
           0x80579027,  /*  76: HVD0_OLCPU RR 2888ns */
           0x804df025,  /*  77: HVD0_CAB RR 2572ns */
           0x007cb010,  /*  78: HVD0_ILSI 4111ns */
           0x8086102c,  /*  79: HVD0_ILCPU_p2 RR 4419ns */
           0x007cb011,  /*  80: HVD0_ILSI_p2 4111ns */
           0x3ffff0ff,  /*  81: HVD1_DBLK_0 off */
           0x3ffff0ff,  /*  82: HVD1_DBLK_1 off */
           0x3ffff0ff,  /*  83: HVD1_ILCPU off */
           0x3ffff0ff,  /*  84: HVD1_OLCPU off */
           0x3ffff0ff,  /*  85: HVD1_CAB off */
           0x3ffff0ff,  /*  86: HVD1_ILSI off */
           0x3ffff0ff,  /*  87: UNASSIGNED off */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff0ff,  /*  94: MADR_RD off */
           0x3ffff0ff,  /*  95: MADR_QM off */
           0x3ffff0ff,  /*  96: MADR_WR off */
           0x0069f007,  /*  97: MADR1_RD 3510ns +HRT(0.5%) */
           0x00951014,  /*  98: MADR1_QM 4938ns +HRT(0.5%) */
           0x00d41018,  /*  99: MADR1_WR 7021ns +HRT(0.5%) */
           0x0069f008,  /* 100: MADR2_RD 3510ns +HRT(0.5%) */
           0x00951015,  /* 101: MADR2_QM 4938ns +HRT(0.5%) */
           0x00d41019,  /* 102: MADR2_WR 7021ns +HRT(0.5%) */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0095d012,  /* 106: BVNF_MFD0_0 4938ns */
           0x0031d002,  /* 107: BVNF_MFD0_1 1646ns */
           0x0095d013,  /* 108: BVNF_MFD1_0 4938ns */
           0x0031d003,  /* 109: BVNF_MFD1_1 1646ns */
           0x3ffff0ff,  /* 110: BVNF_MFD2_0 off */
           0x3ffff0ff,  /* 111: BVNF_MFD2_1 off */
           0x3ffff0ff,  /* 112: BVNF_MFD3_0 off */
           0x3ffff0ff,  /* 113: BVNF_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff0ff,  /* 118: BVNF_VFD0 off */
           0x3ffff0ff,  /* 119: BVNF_VFD1 off */
           0x0059d004,  /* 120: BVNF_VFD2 2962ns */
           0x0059d005,  /* 121: BVNF_VFD3 2962ns */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff0ff,  /* 126: BVNF_CAP0 off */
           0x3ffff0ff,  /* 127: BVNF_CAP1 off */
           0x002ca000,  /* 128: BVNF_CAP2 1481ns +HRT(0.5%) */
           0x002ca001,  /* 129: BVNF_CAP3 1481ns +HRT(0.5%) */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff0ff,  /* 134: BVNF_GFD0 off */
           0x3ffff0ff,  /* 135: BVNF_GFD1 off */
           0x3ffff0ff,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: BVNF_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff0ff,  /* 141: MCVP0 off */
           0x3ffff0ff,  /* 142: MCVP1 off */
           0x3ffff0ff,  /* 143: MCVP_QM off */
           0x005b0006,  /* 144: BVNF_RDC 3000ns */
           0x03c4e01c,  /* 145: VEC_VBI_ENC 31770ns */
           0x3ffff0ff,  /* 146: VEC_HDR0 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x8017b04d,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8017b04e,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0xbffff0ff,  /* 163: MM_M2MC1_0 RR */
           0xbffff0ff,  /* 164: MM_M2MC1_1 RR */
           0xbffff0ff,  /* 165: MM_M2MC1_2 RR */
           0x3ffff0ff,  /* 166: CMP0 off */
           0x3ffff0ff,  /* 167: HVD1_DBLK_p2_0 off */
           0x3ffff0ff,  /* 168: HVD1_DBLK_p2_1 off */
           0x3ffff0ff,  /* 169: HVD1_ILCPU_p2 off */
           0x3ffff0ff,  /* 170: HVD1_ILSI_p2 off */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x82d31043,  /* 172: ASP_WR_ARCSS_DDMA1 RR 23809.5238095238ns */
           0x82c24041,  /* 173: ASP_WR_ARCSS_DDMA2 RR 23255.8139534884ns */
           0x806fd009,  /* 174: ASP_WR_EDPKT_AVIN RR 3686.99029700481ns */
           0x81afd03e,  /* 175: ASP_WR_EPKT_AVRTR RR 14222.2222222222ns */
           0x82d31042,  /* 176: ASP_RD_ARCSS_DDMA1 RR 23809.5238095238ns */
           0x82c24040,  /* 177: ASP_RD_ARCSS_DDMA2 RR 23255.8139534884ns */
           0x8172203c,  /* 178: ASP_RD_MCPB_AVOUT RR 12190.4761904762ns */
           0xa3986049,  /* 179: ASP_RD_EDPKT_AVHDR RR 300000ns */
           0xbffff0ff,  /* 180: USB_BDC RR */
           0x8177901b,  /* 181: WIFI_SYSPORT_WR RR 12370ns */
           0x8177903d,  /* 182: WIFI_SYSPORT_RD RR 12370ns */
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
           0x80202022,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1125ns */
           0x80000053,  /* 201: HOST_CPU_MCP_RD_LOW RR */
           0x80405024,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x80000054,  /* 203: HOST_CPU_MCP_WR_LOW RR */
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
           0x3ffff0ff,  /* 217: HVD1_PFRI off */
           0x8000004f,  /* 218: VICE_PFRI RR 0ns */
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
           0x8bdd6048,  /* 250: MEMC_ZQCS RR 100000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x0076700f   /* 255: REFRESH 3904ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20161216020252_7278_box1000[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80520e04}, /* HVD0_PFRI (gHVC0) 470520.00 ns/40 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000595}, /* d: 4; p: 1429.2 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00001691}, /* 5777 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00000d8a}, /* 60% * 5777 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x80510e05}, /* VICE_PFRI (gVICE) 220588.24 ns/60 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x400001be}, /* d: 4; p: 446.6875 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x00000240}, /* 576 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x00000159}  /* 60% * 576 */
};

static const uint32_t* const paulMemc_box1000[] = { &aulMemc0_20161216020252_7278_box1000[0]};

const BBOX_Rts stBoxRts_7278_box1000_box1000 = {
  "20161216020252_7278_box1000_box1000",
  7278,
  1000,
  1,
  256,
  (const uint32_t**)&paulMemc_box1000[0],
  8,
  stBoxRts_PfriClient_20161216020252_7278_box1000
};
