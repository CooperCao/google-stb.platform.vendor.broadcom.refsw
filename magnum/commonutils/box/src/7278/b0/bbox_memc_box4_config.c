/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
 *                            Do Not Edit Directly
 * Auto-Generated from RTS environment:
 *   at: Fri Jul 20 19:24:27 2018 GMT
 *   by: fm945799
 *   for: Box 7278B0_box4_box_mid_performance_sm
 *         MemC 0 (32-bit LPDDR4@1600MHz) w/486MHz clock
 *         MemC 1 (32-bit LPDDR4@1600MHz) w/486MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/BCM7278B0.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/BCM7278B0MemoryTiming.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b3.1r8_timing_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b3r8_timing_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/VideoDecoder_rT_vp9.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/BCM7278B0BvnLib.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/Vice_v3.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/Raaga.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/BCM7278B0Client.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/BCM7278B0ClientTime.cfg
 *     xls_input/box_mid_performance_sm.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/BCM7278B0ClientMap.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/BCM7278B0Display.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/utils.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7278B0/v2/BCM7278B0_boxdoc.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20180720192427_7278B0_box4_box_mid_performance_sm[] = {
           0x80edc017,  /*   0: XPT_WR_TUNER_RS RR 7833.33333333333ns */
           0x80721009,  /*   1: XPT_WR_CARD_RS RR 3760ns */
           0x804bb02b,  /*   2: XPT_WR_CDB RR 2497.56097560976ns */
           0x80e6e03b,  /*   3: XPT_WR_ITB_MSG RR 7605.56257901391ns */
           0x80c4c038,  /*   4: XPT_RD_RS RR 6482.75862068965ns */
           0x8072102e,  /*   5: XPT_RD_CARD_RS RR 3760ns */
           0x81b46042,  /*   6: XPT_RD_PB RR 14371.9298245614ns */
           0x8058602c,  /*   7: XPT_WR_MEMDMA RR 2913.82113821138ns */
           0x80ca2039,  /*   8: XPT_RD_MEMDMA RR 6660.16260162602ns */
           0x3ffff0ff,  /*   9: UNASSIGNED off */
           0x81413041,  /*  10: SYSPORT_WR RR 10580ns */
           0x81413040,  /*  11: SYSPORT_RD RR 10580ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x80b09035,  /*  15: HIF_PCIe1 RR 5818.18181818182ns */
           0x3ffff0ff,  /*  16: UNASSIGNED off */
           0x80ca203a,  /*  17: SATA RR 6660.16260162602ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x85eea023,  /*  22: BSP RR 50000ns */
           0x80c34037,  /*  23: SAGE RR 6820ns */
           0x81ca6043,  /*  24: FLASH_DMA RR 16000ns */
           0x80b09034,  /*  25: HIF_PCIe RR 5818.18181818182ns */
           0x809b6033,  /*  26: SDIO_EMMC RR 5120ns */
           0x8136e03f,  /*  27: SDIO_CARD RR 10240ns */
           0x3ffff0ff,  /*  28: UNASSIGNED off */
           0x047f1021,  /*  29: MCIF_RD_0 37900ns */
           0x047f1022,  /*  30: MCIF_WR_0 37900ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x3ffff0ff,  /*  33: UNASSIGNED off */
           0x3ffff0ff,  /*  34: UNASSIGNED off */
           0x80ffc03e,  /*  35: USB_X_WRITE_0 RR 8928.57142857143ns */
           0x80ffc03c,  /*  36: USB_X_READ_0 RR 8928.57142857143ns */
           0x80ffc03d,  /*  37: USB_X_CTRL_0 RR 8928.57142857143ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x80280026,  /*  40: RAAGA RR 1400ns */
           0x3ffff0ff,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00d2a014,  /*  44: AUD_AIO 6940ns */
           0x3ffff04d,  /*  45: VICE_CME0 off */
           0x3ffff050,  /*  46: VICE_CME1 off */
           0x3ffff051,  /*  47: VICE_FME off */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x3ffff04e,  /*  49: VICE_IMD0 off */
           0x3ffff04f,  /*  50: VICE_IMD1 off */
           0x3ffff05a,  /*  51: VICE_DBLK off */
           0xbfffe024,  /*  52: VICE_CABAC0 RR 800000ns */
           0x81175019,  /*  53: VICE_CABAC1 RR 9200ns */
           0x8067c02d,  /*  54: VICE_ARCSS0 RR 3625ns */
           0x3ffff016,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff01a,  /*  56: VICE_VIP1_INST0 off */
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
           0x800bd052,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x800bd053,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x8072f02f,  /*  75: HVD0_ILCPU RR 3790ns */
           0x803c5029,  /*  76: HVD0_OLCPU RR 1991ns */
           0x80316027,  /*  77: HVD0_CAB RR 1630ns */
           0x807b100d,  /*  78: HVD0_ILSI RR 4057ns */
           0x8072f030,  /*  79: HVD0_ILCPU_p2 RR 3790ns */
           0x807b100e,  /*  80: HVD0_ILSI_p2 RR 4057ns */
           0x3ffff056,  /*  81: HVD1_DBLK_0 off */
           0x3ffff057,  /*  82: HVD1_DBLK_1 off */
           0x8072f031,  /*  83: HVD1_ILCPU RR 3790ns */
           0x80b84036,  /*  84: HVD1_OLCPU RR 6071ns */
           0x80316028,  /*  85: HVD1_CAB RR 1630ns */
           0x807f300f,  /*  86: HVD1_ILSI RR 4192ns */
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
           0x3ffff0ff,  /*  97: MADR1_RD off */
           0x3ffff0ff,  /*  98: MADR1_QM off */
           0x3ffff0ff,  /*  99: MADR1_WR off */
           0x0069f007,  /* 100: MADR2_RD 3510ns +HRT(0.5%) */
           0x00951013,  /* 101: MADR2_QM 4938ns +HRT(0.5%) */
           0x00d41015,  /* 102: MADR2_WR 7021ns +HRT(0.5%) */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0095d011,  /* 106: BVNF_MFD0_0 4938ns */
           0x001dd000,  /* 107: BVNF_MFD0_1 987ns */
           0x3ffff012,  /* 108: BVNF_MFD1_0 off */
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
           0x3ffff01c,  /* 119: BVNF_VFD1 off */
           0x3ffff0ff,  /* 120: BVNF_VFD2 off */
           0x3ffff0ff,  /* 121: BVNF_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff003,  /* 126: BVNF_CAP0 off */
           0x3ffff01f,  /* 127: BVNF_CAP1 off */
           0x3ffff0ff,  /* 128: BVNF_CAP2 off */
           0x3ffff0ff,  /* 129: BVNF_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0077d00b,  /* 134: BVNF_GFD0 3950ns */
           0x3ffff01d,  /* 135: BVNF_GFD1 off */
           0x3ffff0ff,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: BVNF_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x0054b005,  /* 141: MCVP0 2808ns +HRT(0.5%) */
           0x0054b004,  /* 142: MCVP1 2808ns +HRT(0.5%) */
           0x00ed2018,  /* 143: MCVP_QM 7850ns +HRT(0.5%) */
           0x005b0006,  /* 144: BVNF_RDC 3000ns */
           0x03c4e020,  /* 145: VEC_VBI_ENC 31770ns */
           0x01a9201b,  /* 146: VEC_HDR0 14000ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x800bd054,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x800bd055,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0xbffff0ff,  /* 163: MM_M2MC1_0 RR */
           0xbffff0ff,  /* 164: MM_M2MC1_1 RR */
           0xbffff0ff,  /* 165: MM_M2MC1_2 RR */
           0x0078900c,  /* 166: CMP0 3974ns */
           0x3ffff058,  /* 167: HVD1_DBLK_p2_0 off */
           0x3ffff059,  /* 168: HVD1_DBLK_p2_1 off */
           0x8072f032,  /* 169: HVD1_ILCPU_p2 RR 3790ns */
           0x807f3010,  /* 170: HVD1_ILSI_p2 RR 4192ns */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x82d31046,  /* 172: ASP_WR_ARCSS_DDMA1 RR 23809.5238095238ns */
           0x8334d049,  /* 173: ASP_WR_ARCSS_DDMA2 RR 27027.027027027ns */
           0x806fd008,  /* 174: ASP_WR_EDPKT_AVIN RR 3686.99029700481ns */
           0x838b104a,  /* 175: ASP_WR_EPKT_AVRTR RR 29866.6666666667ns */
           0x82d31045,  /* 176: ASP_RD_ARCSS_DDMA1 RR 23809.5238095238ns */
           0x8334d048,  /* 177: ASP_RD_ARCSS_DDMA2 RR 27027.027027027ns */
           0x83097047,  /* 178: ASP_RD_MCPB_AVOUT RR 25600ns */
           0xa398604c,  /* 179: ASP_RD_EDPKT_AVHDR RR 300000ns */
           0xbffff0ff,  /* 180: USB_BDC RR */
           0x8222a01e,  /* 181: WIFI_SYSPORT_WR RR 18000ns */
           0x8222a044,  /* 182: WIFI_SYSPORT_RD RR 18000ns */
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
           0x80202025,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1125ns */
           0x8000005e,  /* 201: HOST_CPU_MCP_RD_LOW RR 0ns */
           0x8040502a,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x8000005f,  /* 203: HOST_CPU_MCP_WR_LOW RR 0ns */
           0x80000060,  /* 204: V3D_MCP_R_HI RR 0ns */
           0x80000062,  /* 205: V3D_MCP_R_LO RR 0ns */
           0x80000061,  /* 206: V3D_MCP_W_HI RR 0ns */
           0x80000063,  /* 207: V3D_MCP_W_LO RR 0ns */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x8000005c,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff05d,  /* 217: HVD1_PFRI off */
           0x3ffff05b,  /* 218: VICE_PFRI off */
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
           0x8bdd604b,  /* 250: MEMC_ZQCS RR 100000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x0076700a   /* 255: REFRESH 3904ns */
         };
static const uint32_t aulMemc1_20180720192427_7278B0_box4_box_mid_performance_sm[] = {
           0x3ffff017,  /*   0: XPT_WR_TUNER_RS off */
           0x3ffff009,  /*   1: XPT_WR_CARD_RS off */
           0x3ffff02b,  /*   2: XPT_WR_CDB off */
           0x3ffff03b,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff038,  /*   4: XPT_RD_RS off */
           0x3ffff02e,  /*   5: XPT_RD_CARD_RS off */
           0x3ffff042,  /*   6: XPT_RD_PB off */
           0x8058602c,  /*   7: XPT_WR_MEMDMA RR 2913.82113821138ns */
           0x80ca2039,  /*   8: XPT_RD_MEMDMA RR 6660.16260162602ns */
           0x3ffff0ff,  /*   9: UNASSIGNED off */
           0x81413041,  /*  10: SYSPORT_WR RR 10580ns */
           0x81413040,  /*  11: SYSPORT_RD RR 10580ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x80b09035,  /*  15: HIF_PCIe1 RR 5818.18181818182ns */
           0x3ffff0ff,  /*  16: UNASSIGNED off */
           0x80ca203a,  /*  17: SATA RR 6660.16260162602ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x85eea023,  /*  22: BSP RR 50000ns */
           0x80c34037,  /*  23: SAGE RR 6820ns */
           0x81ca6043,  /*  24: FLASH_DMA RR 16000ns */
           0x80b09034,  /*  25: HIF_PCIe RR 5818.18181818182ns */
           0x809b6033,  /*  26: SDIO_EMMC RR 5120ns */
           0x8136e03f,  /*  27: SDIO_CARD RR 10240ns */
           0x3ffff0ff,  /*  28: UNASSIGNED off */
           0x3ffff021,  /*  29: MCIF_RD_0 off */
           0x3ffff022,  /*  30: MCIF_WR_0 off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x3ffff0ff,  /*  33: UNASSIGNED off */
           0x3ffff0ff,  /*  34: UNASSIGNED off */
           0x80ffc03e,  /*  35: USB_X_WRITE_0 RR 8928.57142857143ns */
           0x80ffc03c,  /*  36: USB_X_READ_0 RR 8928.57142857143ns */
           0x80ffc03d,  /*  37: USB_X_CTRL_0 RR 8928.57142857143ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x3ffff026,  /*  40: RAAGA off */
           0x3ffff0ff,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff014,  /*  44: AUD_AIO off */
           0x8064204d,  /*  45: VICE_CME0 RR 3500ns */
           0x8316c050,  /*  46: VICE_CME1 RR 27600ns */
           0x8112b051,  /*  47: VICE_FME RR 9590ns */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x80d2704e,  /*  49: VICE_IMD0 RR 7350ns */
           0x80d2704f,  /*  50: VICE_IMD1 RR 7350ns */
           0x800b005a,  /*  51: VICE_DBLK RR 0ns */
           0x3ffff024,  /*  52: VICE_CABAC0 off */
           0x3ffff019,  /*  53: VICE_CABAC1 off */
           0x3ffff02d,  /*  54: VICE_ARCSS0 off */
           0x80e14016,  /*  55: VICE_VIP0_INST0 RR 7420ns */
           0x812c401a,  /*  56: VICE_VIP1_INST0 RR 9890ns */
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
           0x3ffff052,  /*  73: HVD0_DBLK_0 off */
           0x3ffff053,  /*  74: HVD0_DBLK_1 off */
           0x3ffff02f,  /*  75: HVD0_ILCPU off */
           0x3ffff029,  /*  76: HVD0_OLCPU off */
           0x3ffff027,  /*  77: HVD0_CAB off */
           0x3ffff00d,  /*  78: HVD0_ILSI off */
           0x3ffff030,  /*  79: HVD0_ILCPU_p2 off */
           0x3ffff00e,  /*  80: HVD0_ILSI_p2 off */
           0x800bd056,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x800bd057,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff031,  /*  83: HVD1_ILCPU off */
           0x3ffff036,  /*  84: HVD1_OLCPU off */
           0x3ffff028,  /*  85: HVD1_CAB off */
           0x3ffff00f,  /*  86: HVD1_ILSI off */
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
           0x3ffff0ff,  /*  97: MADR1_RD off */
           0x3ffff0ff,  /*  98: MADR1_QM off */
           0x3ffff0ff,  /*  99: MADR1_WR off */
           0x3ffff007,  /* 100: MADR2_RD off */
           0x3ffff013,  /* 101: MADR2_QM off */
           0x3ffff015,  /* 102: MADR2_WR off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x3ffff011,  /* 106: BVNF_MFD0_0 off */
           0x3ffff000,  /* 107: BVNF_MFD0_1 off */
           0x0095d012,  /* 108: BVNF_MFD1_0 4938ns */
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
           0x01c4f01c,  /* 119: BVNF_VFD1 14917ns */
           0x3ffff0ff,  /* 120: BVNF_VFD2 off */
           0x3ffff0ff,  /* 121: BVNF_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x0021f003,  /* 126: BVNF_CAP0 1128.57142857143ns +HRT(0.5%) */
           0x0253b01f,  /* 127: BVNF_CAP1 19714.2857142857ns +HRT(0.5%) */
           0x3ffff0ff,  /* 128: BVNF_CAP2 off */
           0x3ffff0ff,  /* 129: BVNF_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff00b,  /* 134: BVNF_GFD0 off */
           0x01e2701d,  /* 135: BVNF_GFD1 15888ns */
           0x3ffff0ff,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: BVNF_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff005,  /* 141: MCVP0 off */
           0x3ffff004,  /* 142: MCVP1 off */
           0x3ffff018,  /* 143: MCVP_QM off */
           0x3ffff006,  /* 144: BVNF_RDC off */
           0x3ffff020,  /* 145: VEC_VBI_ENC off */
           0x3ffff01b,  /* 146: VEC_HDR0 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x3ffff054,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff055,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0xbffff0ff,  /* 160: M2MC1_0 RR */
           0xbffff0ff,  /* 161: M2MC1_1 RR */
           0xbffff0ff,  /* 162: M2MC1_2 RR */
           0xbffff0ff,  /* 163: MM_M2MC1_0 RR */
           0xbffff0ff,  /* 164: MM_M2MC1_1 RR */
           0xbffff0ff,  /* 165: MM_M2MC1_2 RR */
           0x3ffff00c,  /* 166: CMP0 off */
           0x800bd058,  /* 167: HVD1_DBLK_p2_0 RR 0ns */
           0x800bd059,  /* 168: HVD1_DBLK_p2_1 RR 0ns */
           0x3ffff032,  /* 169: HVD1_ILCPU_p2 off */
           0x3ffff010,  /* 170: HVD1_ILSI_p2 off */
           0x3ffff0ff,  /* 171: UNASSIGNED off */
           0x3ffff046,  /* 172: ASP_WR_ARCSS_DDMA1 off */
           0x3ffff049,  /* 173: ASP_WR_ARCSS_DDMA2 off */
           0x3ffff008,  /* 174: ASP_WR_EDPKT_AVIN off */
           0x3ffff04a,  /* 175: ASP_WR_EPKT_AVRTR off */
           0x3ffff045,  /* 176: ASP_RD_ARCSS_DDMA1 off */
           0x3ffff048,  /* 177: ASP_RD_ARCSS_DDMA2 off */
           0x3ffff047,  /* 178: ASP_RD_MCPB_AVOUT off */
           0x3ffff04c,  /* 179: ASP_RD_EDPKT_AVHDR off */
           0xbffff0ff,  /* 180: USB_BDC RR */
           0x3ffff01e,  /* 181: WIFI_SYSPORT_WR off */
           0x3ffff044,  /* 182: WIFI_SYSPORT_RD off */
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
           0x80202025,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1125ns */
           0x8000005e,  /* 201: HOST_CPU_MCP_RD_LOW RR 0ns */
           0x8040502a,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x8000005f,  /* 203: HOST_CPU_MCP_WR_LOW RR 0ns */
           0x80000060,  /* 204: V3D_MCP_R_HI RR 0ns */
           0x80000062,  /* 205: V3D_MCP_R_LO RR 0ns */
           0x80000061,  /* 206: V3D_MCP_W_HI RR 0ns */
           0x80000063,  /* 207: V3D_MCP_W_LO RR 0ns */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x3ffff05c,  /* 216: HVD0_PFRI off */
           0x8000005d,  /* 217: HVD1_PFRI RR 0ns */
           0x8000005b,  /* 218: VICE_PFRI RR 0ns */
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
           0x8bdd604b,  /* 250: MEMC_ZQCS RR 100000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x0076700a   /* 255: REFRESH 3904ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20180720192427_7278B0_box4_box_mid_performance_sm[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80650d05}, /* HVD0_PFRI (gHVC0) 236400.00 ns/40 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x400002ce}, /* d: 4; p: 718.0625 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00001620}, /* 5664 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00000d46}, /* 60% * 5664 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80660d04}, /* HVD1_PFRI (gHVC1) 236400.00 ns/40 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x400002ce}, /* d: 4; p: 718.0625 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x00001691}, /* 5777 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x00000d8a}, /* 60% * 5777 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_CONFIG,      0x80640d05}, /* VICE_PFRI (gVICE) 220588.24 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_FILTER_CTRL, 0x400001be}, /* d: 4; p: 446.6875 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH0,     0x00000208}, /* 520 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH1,     0x00000138}  /* 60% * 520 */
};

static const uint32_t* const paulMemc_box4[] = { &aulMemc0_20180720192427_7278B0_box4_box_mid_performance_sm[0], &aulMemc1_20180720192427_7278B0_box4_box_mid_performance_sm[0]};

const BBOX_Rts stBoxRts_7278B0_box4_box_mid_performance_sm_box4 = {
  "20180720192427_7278B0_box4_box_mid_performance_sm_box4",
  7278,
  4,
  2,
  256,
  (const uint32_t**)&paulMemc_box4[0],
  12,
  stBoxRts_PfriClient_20180720192427_7278B0_box4_box_mid_performance_sm
};
