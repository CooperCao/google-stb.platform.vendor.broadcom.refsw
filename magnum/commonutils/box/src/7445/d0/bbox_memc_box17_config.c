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
 *   at: Mon Jun  6 21:45:55 2016 GMT
 *   by: robinc
 *   for: Box 7445_4T
 *         MemC 0 (32-bit DDR4@1185MHz) w/432MHz clock
 *         MemC 1 (32-bit DDR4@1185MHz) w/432MHz clock
 *         MemC 2 (32-bit DDR4@1185MHz) w/432MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0.cfg
 *     xls_input/box_directv_4T.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0MemoryTiming.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b1.3r8_timing_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b1r8_timingf_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/VideoDecoder_rR.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/Vice_v2p1c.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0BvnLib.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0Client.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0ClientMap.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0ClientTime.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0Display.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20160606214555_7445_4T[] = {
           0x0018f000,  /*   0: XPT_WR_RS 930ns */
           0x80271065,  /*   1: XPT_WR_XC RR 1540ns */
           0x802f600d,  /*   2: XPT_WR_CDB RR 1760ns */
           0x8073806b,  /*   3: XPT_WR_ITB_MSG RR 4540ns */
           0x8062b028,  /*   4: XPT_RD_RS RR 3660ns */
           0x815ad053,  /*   5: XPT_RD_XC_RMX_MSG RR 12850ns */
           0x802f600c,  /*   6: XPT_RD_XC_RAVE RR 1760ns */
           0x805bd069,  /*   7: XPT_RD_PB RR 3610ns */
           0x80255062,  /*   8: XPT_WR_MEMDMA RR 1470ns */
           0x80557068,  /*   9: XPT_RD_MEMDMA RR 3360ns */
           0x803fd010,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9070,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617077,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05f,  /*  16: MOCA_MIPS RR 53000ns */
           0x807c106c,  /*  17: SATA RR 4876.19047619048ns */
           0x807c106d,  /*  18: SATA_1 RR 4876.19047619048ns */
           0x03e6e05a,  /*  19: MCIF2_RD 37000ns */
           0x03e6e05c,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05e,  /*  22: BSP RR 50000ns */
           0x80ad906e,  /*  23: SAGE RR 6820ns */
           0x86449079,  /*  24: FLASH_DMA RR 63000ns */
           0x81617076,  /*  25: HIF_PCIe RR 13880ns */
           0x8644907b,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644907a,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e05b,  /*  29: MCIF_RD 37000ns */
           0x03e6e05d,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db072,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5075,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5074,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae106f,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db073,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x001ae001,  /*  40: RAAGA 1000ns */
           0x001ae003,  /*  41: RAAGA_1 1000ns */
           0x0028600b,  /*  42: RAAGA1 1500ns */
           0x001ae002,  /*  43: RAAGA1_1 1000ns */
           0x00bb4043,  /*  44: AUD_AIO 6940ns */
           0x3ffff081,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff088,  /*  46: VICE_CME_CSC off */
           0x3ffff082,  /*  47: VICE_FME_CSC off */
           0x3ffff084,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff083,  /*  49: VICE_FME_Chroma_CMB off */
           0x80dca052,  /*  50: VICE_SG RR 8176.66666666667ns */
           0x3ffff096,  /*  51: VICE_DBLK off */
           0x81edf055,  /*  52: VICE_CABAC0 RR 18300ns */
           0x83782059,  /*  53: VICE_CABAC1 RR 32900ns */
           0x3ffff087,  /*  54: VICE_ARCSS0 off */
           0x3ffff031,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff04c,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff032,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff04d,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff07d,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff086,  /*  60: VICE1_CME_CSC off */
           0x3ffff07e,  /*  61: VICE1_FME_CSC off */
           0x3ffff080,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff07f,  /*  63: VICE1_FME_Chroma_CMB off */
           0x80dca051,  /*  64: VICE1_SG RR 8176.66666666667ns */
           0x3ffff095,  /*  65: VICE1_DBLK off */
           0x81edf054,  /*  66: VICE1_CABAC0 RR 18300ns */
           0x83782058,  /*  67: VICE1_CABAC1 RR 32900ns */
           0x3ffff085,  /*  68: VICE1_ARCSS0 off */
           0x3ffff02e,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff049,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff02f,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff04a,  /*  72: VICE1_VIP1_INST1 off */
           0x80028089,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x8002808a,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x801c2061,  /*  75: HVD0_ILCPU RR 1048ns */
           0x804a7067,  /*  76: HVD0_OLCPU RR 2927ns */
           0x004be019,  /*  77: HVD0_CAB 2815ns */
           0x00710036,  /*  78: HVD0_ILSI 4191ns */
           0x81ea3078,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x00710037,  /*  80: HVD0_ILSI_p2 4191ns */
           0x3ffff091,  /*  81: HVD1_DBLK_0 off */
           0x3ffff092,  /*  82: HVD1_DBLK_1 off */
           0x80270064,  /*  83: HVD1_ILCPU RR 1451ns */
           0x8070a06a,  /*  84: HVD1_OLCPU RR 4427ns */
           0x006a8035,  /*  85: HVD1_CAB 3951ns */
           0x0073d038,  /*  86: HVD1_ILSI 4296ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff093,  /*  88: HVD2_DBLK_0 off */
           0x3ffff094,  /*  89: HVD2_DBLK_1 off */
           0x803b3066,  /*  90: HVD2_ILCPU RR 2198ns */
           0x80e33071,  /*  91: HVD2_OLCPU RR 8925ns */
           0x00916042,  /*  92: HVD2_CAB 5390ns */
           0x00743039,  /*  93: HVD2_ILSI 4308ns */
           0x005ea027,  /*  94: BVN_MAD_PIX_FD 3511ns */
           0x00853041,  /*  95: BVN_MAD_QUANT 4938ns */
           0x00bd7048,  /*  96: BVN_MAD_PIX_CAP 7023ns */
           0x005ea023,  /*  97: BVN_MAD1_PIX_FD 3511ns */
           0x0085303d,  /*  98: BVN_MAD1_QUANT 4938ns */
           0x00bd7044,  /*  99: BVN_MAD1_PIX_CAP 7023ns */
           0x005ea024,  /* 100: BVN_MAD2_PIX_FD 3511ns */
           0x0085303e,  /* 101: BVN_MAD2_QUANT 4938ns */
           0x00bd7045,  /* 102: BVN_MAD2_PIX_CAP 7023ns */
           0x005ea025,  /* 103: BVN_MAD3_PIX_FD 3511ns */
           0x0085303f,  /* 104: BVN_MAD3_QUANT 4938ns */
           0x00bd7046,  /* 105: BVN_MAD3_PIX_CAP 7023ns */
           0x00c7d04f,  /* 106: BVN_MFD0_Ch 7407ns */
           0x0038f00f,  /* 107: BVN_MFD0_Ch_1 2115ns */
           0x3ffff029,  /* 108: BVN_MFD1 off */
           0x3ffff012,  /* 109: BVN_MFD1_1 off */
           0x3ffff02a,  /* 110: BVN_MFD2 off */
           0x3ffff013,  /* 111: BVN_MFD2_1 off */
           0x3ffff02b,  /* 112: BVN_MFD3 off */
           0x3ffff014,  /* 113: BVN_MFD3_1 off */
           0x3ffff02c,  /* 114: BVN_MFD4 off */
           0x3ffff015,  /* 115: BVN_MFD4_1 off */
           0x3ffff02d,  /* 116: BVN_MFD5 off */
           0x3ffff016,  /* 117: BVN_MFD5_1 off */
           0x3ffff03a,  /* 118: BVN_VFD0 off */
           0x3ffff03b,  /* 119: BVN_VFD1 off */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff01a,  /* 122: BVN_VFD4 off */
           0x3ffff01b,  /* 123: BVN_VFD5 off */
           0x3ffff01c,  /* 124: BVN_VFD6 off */
           0x3ffff01d,  /* 125: BVN_VFD7 off */
           0x3ffff005,  /* 126: BVN_CAP0 off */
           0x3ffff03c,  /* 127: BVN_CAP1 off */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff01e,  /* 130: BVN_CAP4 off */
           0x3ffff01f,  /* 131: BVN_CAP5 off */
           0x3ffff020,  /* 132: BVN_CAP6 off */
           0x3ffff021,  /* 133: BVN_CAP7 off */
           0x3ffff00e,  /* 134: BVN_GFD0 off */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x3ffff006,  /* 136: BVN_GFD2 off */
           0x3ffff007,  /* 137: BVN_GFD3 off */
           0x3ffff008,  /* 138: BVN_GFD4 off */
           0x3ffff009,  /* 139: BVN_GFD5 off */
           0x3ffff00a,  /* 140: BVN_GFD6 off */
           0x004b5018,  /* 141: BVN_MCVP0 2794ns */
           0x004b5017,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d050,  /* 143: BVN_MCVP2 7850ns */
           0x3ffff022,  /* 144: BVN_RDC off */
           0x03526056,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526057,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff033,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff04e,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff030,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff04b,  /* 154: VICE1_VIP1_INST2 off */
           0x8002808b,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x8002808c,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
           0x005ea026,  /* 157: BVN_MAD4_PIX_FD 3511ns */
           0x00853040,  /* 158: BVN_MAD4_QUANT 4938ns */
           0x00bd7047,  /* 159: BVN_MAD4_PIX_CAP 7023ns */
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
           0x80142060,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x800000a3,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261063,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x800000a4,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000009a,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x3ffff09b,  /* 217: HVD1_PFRI off */
           0x3ffff09c,  /* 218: HVD2_PFRI off */
           0x3ffff098,  /* 219: VICE_PFRI off */
           0x3ffff097,  /* 220: VICE1_PFRI off */
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
           0xbfffe07c,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00692034   /* 255: REFRESH 3900ns */
         };
static const uint32_t aulMemc1_20160606214555_7445_4T[] = {
           0x3ffff000,  /*   0: XPT_WR_RS off */
           0x3ffff065,  /*   1: XPT_WR_XC off */
           0x3ffff00d,  /*   2: XPT_WR_CDB off */
           0x3ffff06b,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff028,  /*   4: XPT_RD_RS off */
           0x3ffff053,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00c,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff069,  /*   7: XPT_RD_PB off */
           0x80255062,  /*   8: XPT_WR_MEMDMA RR 1470ns */
           0x80557068,  /*   9: XPT_RD_MEMDMA RR 3360ns */
           0x803fd010,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9070,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617077,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05f,  /*  16: MOCA_MIPS RR 53000ns */
           0x807c106c,  /*  17: SATA RR 4876.19047619048ns */
           0x807c106d,  /*  18: SATA_1 RR 4876.19047619048ns */
           0x3ffff05a,  /*  19: MCIF2_RD off */
           0x3ffff05c,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05e,  /*  22: BSP RR 50000ns */
           0x80ad906e,  /*  23: SAGE RR 6820ns */
           0x86449079,  /*  24: FLASH_DMA RR 63000ns */
           0x81617076,  /*  25: HIF_PCIe RR 13880ns */
           0x8644907b,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644907a,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff05b,  /*  29: MCIF_RD off */
           0x3ffff05d,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db072,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5075,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5074,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae106f,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db073,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff001,  /*  40: RAAGA off */
           0x3ffff003,  /*  41: RAAGA_1 off */
           0x3ffff00b,  /*  42: RAAGA1 off */
           0x3ffff002,  /*  43: RAAGA1_1 off */
           0x3ffff043,  /*  44: AUD_AIO off */
           0x8032d081,  /*  45: VICE_CME_RMB_CMB RR 2000ns */
           0x81977088,  /*  46: VICE_CME_CSC RR 16000ns */
           0x805d6082,  /*  47: VICE_FME_CSC RR 3670ns */
           0x805d6084,  /*  48: VICE_FME_Luma_CMB RR 3670ns */
           0x805d6083,  /*  49: VICE_FME_Chroma_CMB RR 3670ns */
           0x3ffff052,  /*  50: VICE_SG off */
           0x8009c096,  /*  51: VICE_DBLK RR 10ns */
           0x3ffff055,  /*  52: VICE_CABAC0 off */
           0x3ffff059,  /*  53: VICE_CABAC1 off */
           0x804d9087,  /*  54: VICE_ARCSS0 RR 3050ns */
           0x8063d031,  /*  55: VICE_VIP0_INST0 RR 3703ns */
           0x80c7a04c,  /*  56: VICE_VIP1_INST0 RR 7400ns */
           0x8063d032,  /*  57: VICE_VIP0_INST1 RR 3703ns */
           0x80c7a04d,  /*  58: VICE_VIP1_INST1 RR 7400ns */
           0x3ffff07d,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff086,  /*  60: VICE1_CME_CSC off */
           0x3ffff07e,  /*  61: VICE1_FME_CSC off */
           0x3ffff080,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff07f,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff051,  /*  64: VICE1_SG off */
           0x3ffff095,  /*  65: VICE1_DBLK off */
           0x3ffff054,  /*  66: VICE1_CABAC0 off */
           0x3ffff058,  /*  67: VICE1_CABAC1 off */
           0x3ffff085,  /*  68: VICE1_ARCSS0 off */
           0x3ffff02e,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff049,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff02f,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff04a,  /*  72: VICE1_VIP1_INST1 off */
           0x8002808d,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8002808e,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff061,  /*  75: HVD0_ILCPU off */
           0x3ffff067,  /*  76: HVD0_OLCPU off */
           0x3ffff019,  /*  77: HVD0_CAB off */
           0x3ffff036,  /*  78: HVD0_ILSI off */
           0x81ea3078,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff037,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff091,  /*  81: HVD1_DBLK_0 off */
           0x3ffff092,  /*  82: HVD1_DBLK_1 off */
           0x3ffff064,  /*  83: HVD1_ILCPU off */
           0x3ffff06a,  /*  84: HVD1_OLCPU off */
           0x3ffff035,  /*  85: HVD1_CAB off */
           0x3ffff038,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff093,  /*  88: HVD2_DBLK_0 off */
           0x3ffff094,  /*  89: HVD2_DBLK_1 off */
           0x3ffff066,  /*  90: HVD2_ILCPU off */
           0x3ffff071,  /*  91: HVD2_OLCPU off */
           0x3ffff042,  /*  92: HVD2_CAB off */
           0x3ffff039,  /*  93: HVD2_ILSI off */
           0x3ffff027,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff041,  /*  95: BVN_MAD_QUANT off */
           0x3ffff048,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff023,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff03d,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff044,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff024,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff03e,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff045,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff025,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff03f,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff046,  /* 105: BVN_MAD3_PIX_CAP off */
           0x00428011,  /* 106: BVN_MFD0 2469ns */
           0x001c6004,  /* 107: BVN_MFD0_1 1057.5ns */
           0x3ffff029,  /* 108: BVN_MFD1 off */
           0x3ffff012,  /* 109: BVN_MFD1_1 off */
           0x3ffff02a,  /* 110: BVN_MFD2 off */
           0x3ffff013,  /* 111: BVN_MFD2_1 off */
           0x3ffff02b,  /* 112: BVN_MFD3 off */
           0x3ffff014,  /* 113: BVN_MFD3_1 off */
           0x3ffff02c,  /* 114: BVN_MFD4 off */
           0x3ffff015,  /* 115: BVN_MFD4_1 off */
           0x3ffff02d,  /* 116: BVN_MFD5 off */
           0x3ffff016,  /* 117: BVN_MFD5_1 off */
           0x3ffff03a,  /* 118: BVN_VFD0 off */
           0x3ffff03b,  /* 119: BVN_VFD1 off */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff01a,  /* 122: BVN_VFD4 off */
           0x3ffff01b,  /* 123: BVN_VFD5 off */
           0x3ffff01c,  /* 124: BVN_VFD6 off */
           0x3ffff01d,  /* 125: BVN_VFD7 off */
           0x3ffff005,  /* 126: BVN_CAP0 off */
           0x3ffff03c,  /* 127: BVN_CAP1 off */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff01e,  /* 130: BVN_CAP4 off */
           0x3ffff01f,  /* 131: BVN_CAP5 off */
           0x3ffff020,  /* 132: BVN_CAP6 off */
           0x3ffff021,  /* 133: BVN_CAP7 off */
           0x3ffff00e,  /* 134: BVN_GFD0 off */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x3ffff006,  /* 136: BVN_GFD2 off */
           0x3ffff007,  /* 137: BVN_GFD3 off */
           0x3ffff008,  /* 138: BVN_GFD4 off */
           0x3ffff009,  /* 139: BVN_GFD5 off */
           0x3ffff00a,  /* 140: BVN_GFD6 off */
           0x3ffff018,  /* 141: BVN_MCVP0 off */
           0x3ffff017,  /* 142: BVN_MCVP1 off */
           0x3ffff050,  /* 143: BVN_MCVP2 off */
           0x00571022,  /* 144: BVN_RDC 3230ns */
           0x3ffff056,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff057,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x8063d033,  /* 151: VICE_VIP0_INST2 RR 3703ns */
           0x80c7a04e,  /* 152: VICE_VIP1_INST2 RR 7400ns */
           0x3ffff030,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff04b,  /* 154: VICE1_VIP1_INST2 off */
           0x8002808f,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x80028090,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff026,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff040,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff047,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x80142060,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x800000a3,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261063,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x800000a4,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000099,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff09b,  /* 217: HVD1_PFRI off */
           0x3ffff09c,  /* 218: HVD2_PFRI off */
           0x80000098,  /* 219: VICE_PFRI RR 0ns */
           0x3ffff097,  /* 220: VICE1_PFRI off */
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
           0xbfffe07c,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00692034   /* 255: REFRESH 3900ns */
         };
static const uint32_t aulMemc2_20160606214555_7445_4T[] = {
           0x3ffff000,  /*   0: XPT_WR_RS off */
           0x3ffff065,  /*   1: XPT_WR_XC off */
           0x3ffff00d,  /*   2: XPT_WR_CDB off */
           0x3ffff06b,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff028,  /*   4: XPT_RD_RS off */
           0x3ffff053,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00c,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff069,  /*   7: XPT_RD_PB off */
           0x80255062,  /*   8: XPT_WR_MEMDMA RR 1470ns */
           0x80557068,  /*   9: XPT_RD_MEMDMA RR 3360ns */
           0x803fd010,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9070,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617077,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05f,  /*  16: MOCA_MIPS RR 53000ns */
           0x807c106c,  /*  17: SATA RR 4876.19047619048ns */
           0x807c106d,  /*  18: SATA_1 RR 4876.19047619048ns */
           0x3ffff05a,  /*  19: MCIF2_RD off */
           0x3ffff05c,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05e,  /*  22: BSP RR 50000ns */
           0x80ad906e,  /*  23: SAGE RR 6820ns */
           0x86449079,  /*  24: FLASH_DMA RR 63000ns */
           0x81617076,  /*  25: HIF_PCIe RR 13880ns */
           0x8644907b,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644907a,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff05b,  /*  29: MCIF_RD off */
           0x3ffff05d,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db072,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5075,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5074,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae106f,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db073,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff001,  /*  40: RAAGA off */
           0x3ffff003,  /*  41: RAAGA_1 off */
           0x3ffff00b,  /*  42: RAAGA1 off */
           0x3ffff002,  /*  43: RAAGA1_1 off */
           0x3ffff043,  /*  44: AUD_AIO off */
           0x3ffff081,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff088,  /*  46: VICE_CME_CSC off */
           0x3ffff082,  /*  47: VICE_FME_CSC off */
           0x3ffff084,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff083,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff052,  /*  50: VICE_SG off */
           0x3ffff096,  /*  51: VICE_DBLK off */
           0x3ffff055,  /*  52: VICE_CABAC0 off */
           0x3ffff059,  /*  53: VICE_CABAC1 off */
           0x3ffff087,  /*  54: VICE_ARCSS0 off */
           0x3ffff031,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff04c,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff032,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff04d,  /*  58: VICE_VIP1_INST1 off */
           0x8032d07d,  /*  59: VICE1_CME_RMB_CMB RR 2000ns */
           0x81977086,  /*  60: VICE1_CME_CSC RR 16000ns */
           0x805d607e,  /*  61: VICE1_FME_CSC RR 3670ns */
           0x805d6080,  /*  62: VICE1_FME_Luma_CMB RR 3670ns */
           0x805d607f,  /*  63: VICE1_FME_Chroma_CMB RR 3670ns */
           0x3ffff051,  /*  64: VICE1_SG off */
           0x8009c095,  /*  65: VICE1_DBLK RR 10ns */
           0x3ffff054,  /*  66: VICE1_CABAC0 off */
           0x3ffff058,  /*  67: VICE1_CABAC1 off */
           0x804d9085,  /*  68: VICE1_ARCSS0 RR 3050ns */
           0x8063d02e,  /*  69: VICE1_VIP0_INST0 RR 3703ns */
           0x80c7a049,  /*  70: VICE1_VIP1_INST0 RR 7400ns */
           0x8063d02f,  /*  71: VICE1_VIP0_INST1 RR 3703ns */
           0x80c7a04a,  /*  72: VICE1_VIP1_INST1 RR 7400ns */
           0x3ffff08d,  /*  73: HVD0_DBLK_0 off */
           0x3ffff08e,  /*  74: HVD0_DBLK_1 off */
           0x3ffff061,  /*  75: HVD0_ILCPU off */
           0x3ffff067,  /*  76: HVD0_OLCPU off */
           0x3ffff019,  /*  77: HVD0_CAB off */
           0x3ffff036,  /*  78: HVD0_ILSI off */
           0x81ea3078,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff037,  /*  80: HVD0_ILSI_p2 off */
           0x800b1091,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x800b1092,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff064,  /*  83: HVD1_ILCPU off */
           0x3ffff06a,  /*  84: HVD1_OLCPU off */
           0x3ffff035,  /*  85: HVD1_CAB off */
           0x3ffff038,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x80147093,  /*  88: HVD2_DBLK_0 RR 0ns */
           0x80147094,  /*  89: HVD2_DBLK_1 RR 0ns */
           0x3ffff066,  /*  90: HVD2_ILCPU off */
           0x3ffff071,  /*  91: HVD2_OLCPU off */
           0x3ffff042,  /*  92: HVD2_CAB off */
           0x3ffff039,  /*  93: HVD2_ILSI off */
           0x3ffff027,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff041,  /*  95: BVN_MAD_QUANT off */
           0x3ffff048,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff023,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff03d,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff044,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff024,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff03e,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff045,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff025,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff03f,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff046,  /* 105: BVN_MAD3_PIX_CAP off */
           0x3ffff011,  /* 106: BVN_MFD0 off */
           0x3ffff004,  /* 107: BVN_MFD0_1 off */
           0x0063d029,  /* 108: BVN_MFD1 3703ns */
           0x00428012,  /* 109: BVN_MFD1_1 2469ns */
           0x0063d02a,  /* 110: BVN_MFD2 3703ns */
           0x00428013,  /* 111: BVN_MFD2_1 2469ns */
           0x0063d02b,  /* 112: BVN_MFD3 3703ns */
           0x00428014,  /* 113: BVN_MFD3_1 2469ns */
           0x0063d02c,  /* 114: BVN_MFD4 3703ns */
           0x00428015,  /* 115: BVN_MFD4_1 2469ns */
           0x0063d02d,  /* 116: BVN_MFD5 3703ns */
           0x00428016,  /* 117: BVN_MFD5_1 2469ns */
           0x007fd03a,  /* 118: BVN_VFD0 4740ns */
           0x007fd03b,  /* 119: BVN_VFD1 4740ns */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x004fc01a,  /* 122: BVN_VFD4 2960ns */
           0x004fc01b,  /* 123: BVN_VFD5 2960ns */
           0x004fc01c,  /* 124: BVN_VFD6 2960ns */
           0x004fc01d,  /* 125: BVN_VFD7 2960ns */
           0x001e5005,  /* 126: BVN_CAP0 1128.594ns */
           0x007fd03c,  /* 127: BVN_CAP1 4740ns */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x004fc01e,  /* 130: BVN_CAP4 2960ns */
           0x004fc01f,  /* 131: BVN_CAP5 2960ns */
           0x004fc020,  /* 132: BVN_CAP6 2960ns */
           0x004fc021,  /* 133: BVN_CAP7 2960ns */
           0x0031d00e,  /* 134: BVN_GFD0 1851ns */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x0027d006,  /* 136: BVN_GFD2 1481ns */
           0x0027d007,  /* 137: BVN_GFD3 1481ns */
           0x0027d008,  /* 138: BVN_GFD4 1481ns */
           0x0027d009,  /* 139: BVN_GFD5 1481ns */
           0x0027d00a,  /* 140: BVN_GFD6 1481ns */
           0x3ffff018,  /* 141: BVN_MCVP0 off */
           0x3ffff017,  /* 142: BVN_MCVP1 off */
           0x3ffff050,  /* 143: BVN_MCVP2 off */
           0x3ffff022,  /* 144: BVN_RDC off */
           0x3ffff056,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff057,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff033,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff04e,  /* 152: VICE_VIP1_INST2 off */
           0x8063d030,  /* 153: VICE1_VIP0_INST2 RR 3703ns */
           0x80c7a04b,  /* 154: VICE1_VIP1_INST2 RR 7400ns */
           0x3ffff08f,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff090,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff026,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff040,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff047,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x80142060,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x800000a3,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261063,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x800000a4,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff099,  /* 216: HVD0_PFRI off */
           0x8000009b,  /* 217: HVD1_PFRI RR 0ns */
           0x8000009c,  /* 218: HVD2_PFRI RR 0ns */
           0x3ffff098,  /* 219: VICE_PFRI off */
           0x80000097,  /* 220: VICE1_PFRI RR 0ns */
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
           0xbfffe07c,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00692034   /* 255: REFRESH 3900ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20160606214555_7445_4T[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80a00905}, /* HVD0_PFRI_Ch (gHvdC0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00000882}, /* 2178 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x0000051a}, /* 60% * 2178 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x809f0905}, /* HVD0_PFRI (gHvd0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00001124}, /* 4388 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000a48}, /* 60% * 4388 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_CONFIG,      0x809e0905}, /* VICE_PFRI (gVice) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_THRESH0,     0x000001c2}, /* 450 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_THRESH1,     0x0000010e}, /* 60% * 450 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_CONFIG,      0x80a10905}, /* HVD1_PFRI (gHvd1) 530666.67 ns/20 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000b31}, /* d: 4; p: 2865.5875 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_THRESH0,     0x000019d1}, /* 6609 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_THRESH1,     0x00000f7d}, /* 60% * 6609 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_CONFIG,      0x80a20905}, /* HVD2_PFRI (gHvd2) 967920.00 ns/20 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_FILTER_CTRL, 0x4000146a}, /* d: 4; p: 5226.7625 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_THRESH0,     0x000019d1}, /* 6609 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_THRESH1,     0x00000f7d}, /* 60% * 6609 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_CONFIG,      0x809d0905}, /* VICE1_PFRI (gVice1) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_THRESH0,     0x000001c2}, /* 450 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_THRESH1,     0x0000010e}  /* 60% * 450 */
};

static const uint32_t* const paulMemc_box17[] = { &aulMemc0_20160606214555_7445_4T[0], &aulMemc1_20160606214555_7445_4T[0], &aulMemc2_20160606214555_7445_4T[0]};

const BBOX_Rts stBoxRts_7445_4T_box17 = {
  "20160606214555_7445_4T_box17",
  7445,
  17,
  3,
  256,
  (const uint32_t**)&paulMemc_box17[0],
  24,
  stBoxRts_PfriClient_20160606214555_7445_4T
};
