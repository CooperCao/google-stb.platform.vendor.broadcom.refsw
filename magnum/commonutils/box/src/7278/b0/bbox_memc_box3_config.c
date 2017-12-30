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
 *   at: Tue Nov 28 21:31:58 2017 GMT
 *   by: fm945799
 *   for: Box 7278B0_box3_box_mid_performance_sm_supportalt
 *         MemC 0 (32-bit LPDDR4@1600MHz) w/486MHz clock
 *         MemC 1 (32-bit LPDDR4@1600MHz) w/486MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/BCM7278B0.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/BCM7278B0MemoryTiming.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/l_MEMC_b2r8_timing_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/VideoDecoder_rT.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/BCM7278B0BvnLib.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/Vice_v3.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/Raaga.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/BCM7278B0Client.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/BCM7278B0ClientTime.cfg
 *     xls_input/box_mid_performance_sm.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/BCM7278B0ClientMap.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/BCM7278B0Display.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/utils.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/BCM7278B0_boxdoc.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20171128213158_7278B0_box3_box_mid_performance_sm_supportalt[] = {
           0x80edc021,  /*   0: XPT_WR_TUNER_RS RR 7833.33333333333ns */
           0x80721013,  /*   1: XPT_WR_CARD_RS RR 3760ns */
           0x804bb036,  /*   2: XPT_WR_CDB RR 2497.56097560976ns */
           0x80e6e046,  /*   3: XPT_WR_ITB_MSG RR 7605.56257901391ns */
           0x80c4c043,  /*   4: XPT_RD_RS RR 6482.75862068965ns */
           0x8072103c,  /*   5: XPT_RD_CARD_RS RR 3760ns */
           0x81b4604d,  /*   6: XPT_RD_PB RR 14371.9298245614ns */
           0x80586038,  /*   7: XPT_WR_MEMDMA RR 2913.82113821138ns */
           0x80ca2044,  /*   8: XPT_RD_MEMDMA RR 6660.16260162602ns */
           0x3ffff0ff,  /*   9: UNASSIGNED off */
           0x8141304c,  /*  10: SYSPORT_WR RR 10580ns */
           0x8141304b,  /*  11: SYSPORT_RD RR 10580ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x80b09041,  /*  15: HIF_PCIe1 RR 5818.18181818182ns */
           0x3ffff0ff,  /*  16: UNASSIGNED off */
           0x80ca2045,  /*  17: SATA RR 6660.16260162602ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x85eea02e,  /*  22: BSP RR 50000ns */
           0x80c34042,  /*  23: SAGE RR 6820ns */
           0x81ca604e,  /*  24: FLASH_DMA RR 16000ns */
           0x80b09040,  /*  25: HIF_PCIe RR 5818.18181818182ns */
           0x809b603f,  /*  26: SDIO_EMMC RR 5120ns */
           0x8136e04a,  /*  27: SDIO_CARD RR 10240ns */
           0x3ffff0ff,  /*  28: UNASSIGNED off */
           0x047f102c,  /*  29: MCIF_RD_0 37900ns */
           0x047f102d,  /*  30: MCIF_WR_0 37900ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x3ffff0ff,  /*  33: UNASSIGNED off */
           0x3ffff0ff,  /*  34: UNASSIGNED off */
           0x80ffc049,  /*  35: USB_X_WRITE_0 RR 8928.57142857143ns */
           0x80ffc047,  /*  36: USB_X_READ_0 RR 8928.57142857143ns */
           0x80ffc048,  /*  37: USB_X_CTRL_0 RR 8928.57142857143ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x8013f030,  /*  40: RAAGA RR 700ns */
           0x3ffff0ff,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00d2a01c,  /*  44: AUD_AIO 6940ns */
           0x3ffff058,  /*  45: VICE_CME0 off */
           0x3ffff05b,  /*  46: VICE_CME1 off */
           0x3ffff05c,  /*  47: VICE_FME off */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x3ffff059,  /*  49: VICE_IMD0 off */
           0x3ffff05a,  /*  50: VICE_IMD1 off */
           0x3ffff065,  /*  51: VICE_DBLK off */
           0xbfffe02f,  /*  52: VICE_CABAC0 RR 800000ns */
           0x81175023,  /*  53: VICE_CABAC1 RR 9200ns */
           0x8067c03b,  /*  54: VICE_ARCSS0 RR 3625ns */
           0x3ffff01f,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff024,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff020,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff025,  /*  58: VICE_VIP1_INST1 off */
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
           0x800ae05d,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x800ae05e,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x8058e039,  /*  75: HVD0_ILCPU RR 2931ns */
           0x803a5033,  /*  76: HVD0_OLCPU RR 1925ns */
           0x802ed032,  /*  77: HVD0_CAB RR 1546.4ns */
           0x8068900c,  /*  78: HVD0_ILSI RR 3448ns */
           0x8058e03a,  /*  79: HVD0_ILCPU_p2 RR 2931ns */
           0x8068900d,  /*  80: HVD0_ILSI_p2 RR 3448ns */
           0x3ffff061,  /*  81: HVD1_DBLK_0 off */
           0x3ffff062,  /*  82: HVD1_DBLK_1 off */
           0x8086103d,  /*  83: HVD1_ILCPU RR 4419ns */
           0x80579037,  /*  84: HVD1_OLCPU RR 2888ns */
           0x803e5034,  /*  85: HVD1_CAB RR 2057.6ns */
           0x8068a00e,  /*  86: HVD1_ILSI RR 3448.8ns */
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
           0x3ffff010,  /*  97: MADR1_RD off */
           0x3ffff01a,  /*  98: MADR1_QM off */
           0x3ffff01d,  /*  99: MADR1_WR off */
           0x3ffff011,  /* 100: MADR2_RD off */
           0x3ffff01b,  /* 101: MADR2_QM off */
           0x3ffff01e,  /* 102: MADR2_WR off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0095d017,  /* 106: BVNF_MFD0_0 4938ns */
           0x001dd000,  /* 107: BVNF_MFD0_1 987ns */
           0x3ffff018,  /* 108: BVNF_MFD1_0 off */
           0x3ffff00a,  /* 109: BVNF_MFD1_1 off */
           0x3ffff019,  /* 110: BVNF_MFD2_0 off */
           0x3ffff00b,  /* 111: BVNF_MFD2_1 off */
           0x3ffff0ff,  /* 112: BVNF_MFD3_0 off */
           0x3ffff0ff,  /* 113: BVNF_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff001,  /* 118: BVNF_VFD0 off */
           0x3ffff02a,  /* 119: BVNF_VFD1 off */
           0x3ffff005,  /* 120: BVNF_VFD2 off */
           0x3ffff006,  /* 121: BVNF_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff002,  /* 126: BVNF_CAP0 off */
           0x3ffff029,  /* 127: BVNF_CAP1 off */
           0x3ffff007,  /* 128: BVNF_CAP2 off */
           0x3ffff008,  /* 129: BVNF_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff015,  /* 134: BVNF_GFD0 off */
           0x01e27027,  /* 135: BVNF_GFD1 15888ns */
           0x3ffff0ff,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: BVNF_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff004,  /* 141: MCVP0 off */
           0x3ffff003,  /* 142: MCVP1 off */
           0x3ffff022,  /* 143: MCVP_QM off */
           0x005b0009,  /* 144: BVNF_RDC 3000ns */
           0x03c4e02b,  /* 145: VEC_VBI_ENC 31770ns */
           0x01a92026,  /* 146: VEC_HDR0 14000ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x800ae05f,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x800ae060,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0xbffff0ff,  /* 163: MM_M2MC1_0 RR */
           0xbffff0ff,  /* 164: MM_M2MC1_1 RR */
           0xbffff0ff,  /* 165: MM_M2MC1_2 RR */
           0x00789016,  /* 166: CMP0 3974ns */
           0x3ffff063,  /* 167: HVD1_DBLK_p2_0 off */
           0x3ffff064,  /* 168: HVD1_DBLK_p2_1 off */
           0x8086103e,  /* 169: HVD1_ILCPU_p2 RR 4419ns */
           0x8068a00f,  /* 170: HVD1_ILSI_p2 RR 3448.8ns */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x82d31051,  /* 172: ASP_WR_ARCSS_DDMA1 RR 23809.5238095238ns */
           0x8334d054,  /* 173: ASP_WR_ARCSS_DDMA2 RR 27027.027027027ns */
           0x806fd012,  /* 174: ASP_WR_EDPKT_AVIN RR 3686.99029700481ns */
           0x838b1055,  /* 175: ASP_WR_EPKT_AVRTR RR 29866.6666666667ns */
           0x82d31050,  /* 176: ASP_RD_ARCSS_DDMA1 RR 23809.5238095238ns */
           0x8334d053,  /* 177: ASP_RD_ARCSS_DDMA2 RR 27027.027027027ns */
           0x83097052,  /* 178: ASP_RD_MCPB_AVOUT RR 25600ns */
           0xa3986057,  /* 179: ASP_RD_EDPKT_AVHDR RR 300000ns */
           0xbffff0ff,  /* 180: USB_BDC RR */
           0x8222a028,  /* 181: WIFI_SYSPORT_WR RR 18000ns */
           0x8222a04f,  /* 182: WIFI_SYSPORT_RD RR 18000ns */
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
           0x80202031,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1125ns */
           0x80000069,  /* 201: HOST_CPU_MCP_RD_LOW RR 0ns */
           0x80405035,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x8000006a,  /* 203: HOST_CPU_MCP_WR_LOW RR 0ns */
           0x8000006b,  /* 204: V3D_MCP_R_HI RR 0ns */
           0x8000006d,  /* 205: V3D_MCP_R_LO RR 0ns */
           0x8000006c,  /* 206: V3D_MCP_W_HI RR 0ns */
           0x8000006e,  /* 207: V3D_MCP_W_LO RR 0ns */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x80000066,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff068,  /* 217: HVD1_PFRI off */
           0x3ffff067,  /* 218: VICE_PFRI off */
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
           0x8bdd6056,  /* 250: MEMC_ZQCS RR 100000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00767014   /* 255: REFRESH 3904ns */
         };
static const uint32_t aulMemc1_20171128213158_7278B0_box3_box_mid_performance_sm_supportalt[] = {
           0x3ffff021,  /*   0: XPT_WR_TUNER_RS off */
           0x3ffff013,  /*   1: XPT_WR_CARD_RS off */
           0x3ffff036,  /*   2: XPT_WR_CDB off */
           0x3ffff046,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff043,  /*   4: XPT_RD_RS off */
           0x3ffff03c,  /*   5: XPT_RD_CARD_RS off */
           0x3ffff04d,  /*   6: XPT_RD_PB off */
           0x80586038,  /*   7: XPT_WR_MEMDMA RR 2913.82113821138ns */
           0x80ca2044,  /*   8: XPT_RD_MEMDMA RR 6660.16260162602ns */
           0x3ffff0ff,  /*   9: UNASSIGNED off */
           0x8141304c,  /*  10: SYSPORT_WR RR 10580ns */
           0x8141304b,  /*  11: SYSPORT_RD RR 10580ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x80b09041,  /*  15: HIF_PCIe1 RR 5818.18181818182ns */
           0x3ffff0ff,  /*  16: UNASSIGNED off */
           0x80ca2045,  /*  17: SATA RR 6660.16260162602ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x85eea02e,  /*  22: BSP RR 50000ns */
           0x80c34042,  /*  23: SAGE RR 6820ns */
           0x81ca604e,  /*  24: FLASH_DMA RR 16000ns */
           0x80b09040,  /*  25: HIF_PCIe RR 5818.18181818182ns */
           0x809b603f,  /*  26: SDIO_EMMC RR 5120ns */
           0x8136e04a,  /*  27: SDIO_CARD RR 10240ns */
           0x3ffff0ff,  /*  28: UNASSIGNED off */
           0x3ffff02c,  /*  29: MCIF_RD_0 off */
           0x3ffff02d,  /*  30: MCIF_WR_0 off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x3ffff0ff,  /*  33: UNASSIGNED off */
           0x3ffff0ff,  /*  34: UNASSIGNED off */
           0x80ffc049,  /*  35: USB_X_WRITE_0 RR 8928.57142857143ns */
           0x80ffc047,  /*  36: USB_X_READ_0 RR 8928.57142857143ns */
           0x80ffc048,  /*  37: USB_X_CTRL_0 RR 8928.57142857143ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x3ffff030,  /*  40: RAAGA off */
           0x3ffff0ff,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff01c,  /*  44: AUD_AIO off */
           0x80642058,  /*  45: VICE_CME0 RR 3500ns */
           0x8316c05b,  /*  46: VICE_CME1 RR 27600ns */
           0x8112b05c,  /*  47: VICE_FME RR 9590ns */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x80d27059,  /*  49: VICE_IMD0 RR 7350ns */
           0x80d2705a,  /*  50: VICE_IMD1 RR 7350ns */
           0x800b0065,  /*  51: VICE_DBLK RR 0ns */
           0x3ffff02f,  /*  52: VICE_CABAC0 off */
           0x3ffff023,  /*  53: VICE_CABAC1 off */
           0x3ffff03b,  /*  54: VICE_ARCSS0 off */
           0x80e1401f,  /*  55: VICE_VIP0_INST0 RR 7420ns */
           0x812c4024,  /*  56: VICE_VIP1_INST0 RR 9890ns */
           0x80e14020,  /*  57: VICE_VIP0_INST1 RR 7420ns */
           0x812c4025,  /*  58: VICE_VIP1_INST1 RR 9890ns */
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
           0x3ffff05d,  /*  73: HVD0_DBLK_0 off */
           0x3ffff05e,  /*  74: HVD0_DBLK_1 off */
           0x3ffff039,  /*  75: HVD0_ILCPU off */
           0x3ffff033,  /*  76: HVD0_OLCPU off */
           0x3ffff032,  /*  77: HVD0_CAB off */
           0x3ffff00c,  /*  78: HVD0_ILSI off */
           0x3ffff03a,  /*  79: HVD0_ILCPU_p2 off */
           0x3ffff00d,  /*  80: HVD0_ILSI_p2 off */
           0x80130061,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x80130062,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff03d,  /*  83: HVD1_ILCPU off */
           0x3ffff037,  /*  84: HVD1_OLCPU off */
           0x3ffff034,  /*  85: HVD1_CAB off */
           0x3ffff00e,  /*  86: HVD1_ILSI off */
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
           0x0069f010,  /*  97: MADR1_RD 3510ns +HRT(0.5%) */
           0x0095101a,  /*  98: MADR1_QM 4938ns +HRT(0.5%) */
           0x00d4101d,  /*  99: MADR1_WR 7021ns +HRT(0.5%) */
           0x0069f011,  /* 100: MADR2_RD 3510ns +HRT(0.5%) */
           0x0095101b,  /* 101: MADR2_QM 4938ns +HRT(0.5%) */
           0x00d4101e,  /* 102: MADR2_WR 7021ns +HRT(0.5%) */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x3ffff017,  /* 106: BVNF_MFD0_0 off */
           0x3ffff000,  /* 107: BVNF_MFD0_1 off */
           0x0095d018,  /* 108: BVNF_MFD1_0 4938ns */
           0x0063d00a,  /* 109: BVNF_MFD1_1 3292ns */
           0x0095d019,  /* 110: BVNF_MFD2_0 4938ns */
           0x0063d00b,  /* 111: BVNF_MFD2_1 3292ns */
           0x3ffff0ff,  /* 112: BVNF_MFD3_0 off */
           0x3ffff0ff,  /* 113: BVNF_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001f1001,  /* 118: BVNF_VFD0 1027ns */
           0x038a102a,  /* 119: BVNF_VFD1 29835ns */
           0x0059d005,  /* 120: BVNF_VFD2 2962ns */
           0x0059d006,  /* 121: BVNF_VFD3 2962ns */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x001f5002,  /* 126: BVNF_CAP0 1041ns +HRT(0.5%) */
           0x0253b029,  /* 127: BVNF_CAP1 19714.2857142857ns +HRT(0.5%) */
           0x00596007,  /* 128: BVNF_CAP2 2962ns +HRT(0.5%) */
           0x00596008,  /* 129: BVNF_CAP3 2962ns +HRT(0.5%) */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0077d015,  /* 134: BVNF_GFD0 3950ns */
           0x3ffff027,  /* 135: BVNF_GFD1 off */
           0x3ffff0ff,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: BVNF_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x0054b004,  /* 141: MCVP0 2808ns +HRT(0.5%) */
           0x0054b003,  /* 142: MCVP1 2808ns +HRT(0.5%) */
           0x00ed2022,  /* 143: MCVP_QM 7850ns +HRT(0.5%) */
           0x3ffff009,  /* 144: BVNF_RDC off */
           0x3ffff02b,  /* 145: VEC_VBI_ENC off */
           0x3ffff026,  /* 146: VEC_HDR0 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x3ffff05f,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff060,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0xbffff0ff,  /* 163: MM_M2MC1_0 RR */
           0xbffff0ff,  /* 164: MM_M2MC1_1 RR */
           0xbffff0ff,  /* 165: MM_M2MC1_2 RR */
           0x3ffff016,  /* 166: CMP0 off */
           0x80130063,  /* 167: HVD1_DBLK_p2_0 RR 0ns */
           0x80130064,  /* 168: HVD1_DBLK_p2_1 RR 0ns */
           0x3ffff03e,  /* 169: HVD1_ILCPU_p2 off */
           0x3ffff00f,  /* 170: HVD1_ILSI_p2 off */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x3ffff051,  /* 172: ASP_WR_ARCSS_DDMA1 off */
           0x3ffff054,  /* 173: ASP_WR_ARCSS_DDMA2 off */
           0x3ffff012,  /* 174: ASP_WR_EDPKT_AVIN off */
           0x3ffff055,  /* 175: ASP_WR_EPKT_AVRTR off */
           0x3ffff050,  /* 176: ASP_RD_ARCSS_DDMA1 off */
           0x3ffff053,  /* 177: ASP_RD_ARCSS_DDMA2 off */
           0x3ffff052,  /* 178: ASP_RD_MCPB_AVOUT off */
           0x3ffff057,  /* 179: ASP_RD_EDPKT_AVHDR off */
           0xbffff0ff,  /* 180: USB_BDC RR */
           0x3ffff028,  /* 181: WIFI_SYSPORT_WR off */
           0x3ffff04f,  /* 182: WIFI_SYSPORT_RD off */
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
           0x80202031,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1125ns */
           0x80000069,  /* 201: HOST_CPU_MCP_RD_LOW RR 0ns */
           0x80405035,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x8000006a,  /* 203: HOST_CPU_MCP_WR_LOW RR 0ns */
           0x8000006b,  /* 204: V3D_MCP_R_HI RR 0ns */
           0x8000006d,  /* 205: V3D_MCP_R_LO RR 0ns */
           0x8000006c,  /* 206: V3D_MCP_W_HI RR 0ns */
           0x8000006e,  /* 207: V3D_MCP_W_LO RR 0ns */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x3ffff066,  /* 216: HVD0_PFRI off */
           0x80000068,  /* 217: HVD1_PFRI RR 0ns */
           0x80000067,  /* 218: VICE_PFRI RR 0ns */
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
           0x8bdd6056,  /* 250: MEMC_ZQCS RR 100000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00767014   /* 255: REFRESH 3904ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20171128213158_7278B0_box3_box_mid_performance_sm_supportalt[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x806f0d05}, /* HVD0_PFRI (gHVC0) 218040.00 ns/40 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000296}, /* d: 4; p: 662.29375 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00001620}, /* 5664 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00000d46}, /* 60% * 5664 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80710d04}, /* HVD1_PFRI (gHVC1) 378240.00 ns/40 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x4000047c}, /* d: 4; p: 1148.9 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x00001691}, /* 5777 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x00000d8a}, /* 60% * 5777 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_CONFIG,      0x80700d05}, /* VICE_PFRI (gVICE) 220588.24 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_FILTER_CTRL, 0x400001be}, /* d: 4; p: 446.6875 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH0,     0x00000240}, /* 576 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH1,     0x00000159}  /* 60% * 576 */
};

static const uint32_t* const paulMemc_box3[] = { &aulMemc0_20171128213158_7278B0_box3_box_mid_performance_sm_supportalt[0], &aulMemc1_20171128213158_7278B0_box3_box_mid_performance_sm_supportalt[0]};

const BBOX_Rts stBoxRts_7278B0_box3_box_mid_performance_sm_supportalt_box3 = {
  "20171128213158_7278B0_box3_box_mid_performance_sm_supportalt_box3",
  7278,
  3,
  2,
  256,
  (const uint32_t**)&paulMemc_box3[0],
  12,
  stBoxRts_PfriClient_20171128213158_7278B0_box3_box_mid_performance_sm_supportalt
};
