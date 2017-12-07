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
 *   at: Wed Aug  9 23:42:54 2017 GMT
 *   by: rc905201
 *   for: Box 7445_4kstb1t
 *         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
 *         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
 *         MemC 2 (32-bit DDR3@1067MHz) w/432MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445E0_BW.cfg
 *     xls_input/box_BW.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0MemoryTiming.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b1.3r8_timing_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b1r8_timingh_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/VideoDecoder_rR.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/Vice_v2p1c.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0BvnLib.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0Client.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0ClientMap_BW.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0ClientTime.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445E0_Robin/BCM7445D0Display_BW.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20170809234254_7445_4kstb1t[] = {
           0x001e7007,  /*   0: XPT_WR_RS 1134.2383107089ns */
           0x803b7051,  /*   1: XPT_WR_XC RR 2339.03576982893ns */
           0x804b1012,  /*   2: XPT_WR_CDB RR 2785.18518518519ns */
           0x80bc9062,  /*   3: XPT_WR_ITB_MSG RR 7408.86699507389ns */
           0x80780027,  /*   4: XPT_RD_RS RR 4449.70414201183ns */
           0x819e3039,  /*   5: XPT_RD_XC_RMX_MSG RR 15346.9387755102ns */
           0x804b1011,  /*   6: XPT_RD_XC_RAVE RR 2785.18518518519ns */
           0x80aae059,  /*   7: XPT_RD_PB RR 6714.75409836066ns */
           0x80fd7065,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x8243806e,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd00d,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae905c,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161706b,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e049,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662055,  /*  17: SATA RR 4015.6862745098ns */
           0x80662056,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x03e6e044,  /*  19: MCIF2_RD 37000ns */
           0x03e6e046,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e048,  /*  22: BSP RR 50000ns */
           0x80ad905a,  /*  23: SAGE RR 6820ns */
           0x86449070,  /*  24: FLASH_DMA RR 63000ns */
           0x8161706a,  /*  25: HIF_PCIe RR 13880ns */
           0x86449072,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449071,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e045,  /*  29: MCIF_RD 37000ns */
           0x03e6e047,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db066,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5069,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5068,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae105b,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db067,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x001ae000,  /*  40: RAAGA 1000ns */
           0x001ae003,  /*  41: RAAGA_1 1000ns */
           0x001ae001,  /*  42: RAAGA1 1000ns */
           0x001ae002,  /*  43: RAAGA1_1 1000ns */
           0x00bb402c,  /*  44: AUD_AIO 6940ns */
           0x8032d050,  /*  45: VICE_CME_RMB_CMB RR 2000ns */
           0x8197706c,  /*  46: VICE_CME_CSC RR 16000ns */
           0x805d6053,  /*  47: VICE_FME_CSC RR 3670ns */
           0x80bb1061,  /*  48: VICE_FME_Luma_CMB RR 7350ns */
           0x80bb1060,  /*  49: VICE_FME_Chroma_CMB RR 7350ns */
           0x80dca037,  /*  50: VICE_SG RR 8176.66666666667ns */
           0x8009c074,  /*  51: VICE_DBLK RR 10ns */
           0x8231703f,  /*  52: VICE_CABAC0 RR 20800ns */
           0x83e17043,  /*  53: VICE_CABAC1 RR 36800ns */
           0x80a2e058,  /*  54: VICE_ARCSS0 RR 6400ns */
           0x8063d01d,  /*  55: VICE_VIP0_INST0 RR 3703ns */
           0x80c7a032,  /*  56: VICE_VIP1_INST0 RR 7400ns */
           0x8063d01e,  /*  57: VICE_VIP0_INST1 RR 3703ns */
           0x80c7a033,  /*  58: VICE_VIP1_INST1 RR 7400ns */
           0x8065c054,  /*  59: VICE1_CME_RMB_CMB RR 4000ns */
           0x832ef06f,  /*  60: VICE1_CME_CSC RR 32000ns */
           0x80bb105d,  /*  61: VICE1_FME_CSC RR 7350ns */
           0x80bb105f,  /*  62: VICE1_FME_Luma_CMB RR 7350ns */
           0x80bb105e,  /*  63: VICE1_FME_Chroma_CMB RR 7350ns */
           0x81b8d03e,  /*  64: VICE1_SG RR 16333.3333333333ns */
           0x8013b077,  /*  65: VICE1_DBLK RR 10ns */
           0x83dc1042,  /*  66: VICE1_CABAC0 RR 36600ns */
           0x86f0704a,  /*  67: VICE1_CABAC1 RR 65800ns */
           0x80a2e057,  /*  68: VICE1_ARCSS0 RR 6400ns */
           0x8063d01a,  /*  69: VICE1_VIP0_INST0 RR 3703ns */
           0x80c7a02f,  /*  70: VICE1_VIP1_INST0 RR 7400ns */
           0x8063d01b,  /*  71: VICE1_VIP0_INST1 RR 3703ns */
           0x80c7a030,  /*  72: VICE1_VIP1_INST1 RR 7400ns */
           0x3ffff078,  /*  73: HVD0_DBLK_0 off */
           0x3ffff079,  /*  74: HVD0_DBLK_1 off */
           0x801c204c,  /*  75: HVD0_ILCPU RR 1048ns */
           0x804a7052,  /*  76: HVD0_OLCPU RR 2927ns */
           0x0025e00a,  /*  77: HVD0_CAB 1407.5ns */
           0x00710021,  /*  78: HVD0_ILSI 4191ns */
           0x81ea306d,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x00710022,  /*  80: HVD0_ILSI_p2 4191ns */
           0x3ffff080,  /*  81: HVD1_DBLK_0 off */
           0x3ffff081,  /*  82: HVD1_DBLK_1 off */
           0x8027004e,  /*  83: HVD1_ILCPU RR 1451ns */
           0x80d9b063,  /*  84: HVD1_OLCPU RR 8553ns */
           0x00744025,  /*  85: HVD1_CAB 4311ns */
           0x0071a023,  /*  86: HVD1_ILSI 4214ns */
           0xbffff0ff,  /*  87: SID RR */
           0x8009f075,  /*  88: HVD2_DBLK_0 RR 0ns */
           0x8009f076,  /*  89: HVD2_DBLK_1 RR 0ns */
           0x8027004f,  /*  90: HVD2_ILCPU RR 1451ns */
           0x80d9b064,  /*  91: HVD2_OLCPU RR 8553ns */
           0x00744026,  /*  92: HVD2_CAB 4311ns */
           0x0071a024,  /*  93: HVD2_ILSI 4214ns */
           0x3ffff019,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff029,  /*  95: BVN_MAD_QUANT off */
           0x3ffff02e,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x3ffff00e,  /* 106: BVN_MFD0 off */
           0x3ffff004,  /* 107: BVN_MFD0_1 off */
           0x3ffff02a,  /* 108: BVN_MFD1 off */
           0x3ffff016,  /* 109: BVN_MFD1_1 off */
           0x0085302b,  /* 110: BVN_MFD2 4938ns */
           0x0058c017,  /* 111: BVN_MFD2_1 3292ns */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff008,  /* 118: BVN_VFD0 off */
           0x3ffff009,  /* 119: BVN_VFD1 off */
           0x3ffff03a,  /* 120: BVN_VFD2 off */
           0x3ffff03b,  /* 121: BVN_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff013,  /* 125: BVN_VFD7 off */
           0x3ffff005,  /* 126: BVN_CAP0 off */
           0x3ffff006,  /* 127: BVN_CAP1 off */
           0x3ffff03c,  /* 128: BVN_CAP2 off */
           0x3ffff03d,  /* 129: BVN_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff014,  /* 133: BVN_CAP7 off */
           0x3ffff00b,  /* 134: BVN_GFD0 off */
           0x3ffff038,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff010,  /* 141: BVN_MCVP0 off */
           0x3ffff00f,  /* 142: BVN_MCVP1 off */
           0x3ffff036,  /* 143: BVN_MCVP2 off */
           0x3ffff015,  /* 144: BVN_RDC off */
           0x03526040,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526041,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x8063d01f,  /* 151: VICE_VIP0_INST2 RR 3703ns */
           0x80c7a034,  /* 152: VICE_VIP1_INST2 RR 7400ns */
           0x8063d01c,  /* 153: VICE1_VIP0_INST2 RR 3703ns */
           0x80c7a031,  /* 154: VICE1_VIP1_INST2 RR 7400ns */
           0x3ffff07c,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff07d,  /* 156: HVD0_DBLK_p2_1 off */
           0x005e3018,  /* 157: BVN_MAD4_PIX_FD 3493.445ns */
           0x00848028,  /* 158: BVN_MAD4_QUANT 4913.31ns */
           0x00bc802d,  /* 159: BVN_MAD4_PIX_CAP 6987.885ns */
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
           0x8014204b,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000008e,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104d,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008f,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff083,  /* 216: HVD0_PFRI off */
           0x3ffff086,  /* 217: HVD1_PFRI off */
           0x80000087,  /* 218: HVD2_PFRI RR 0ns */
           0x80000082,  /* 219: VICE_PFRI RR 0ns */
           0x80000085,  /* 220: VICE1_PFRI RR 0ns */
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
           0xbfffe073,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00692020   /* 255: REFRESH 3900ns */
         };
static const uint32_t aulMemc1_20170809234254_7445_4kstb1t[] = {
           0x3ffff007,  /*   0: XPT_WR_RS off */
           0x3ffff051,  /*   1: XPT_WR_XC off */
           0x3ffff012,  /*   2: XPT_WR_CDB off */
           0x3ffff062,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff027,  /*   4: XPT_RD_RS off */
           0x3ffff039,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff011,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff059,  /*   7: XPT_RD_PB off */
           0x80fd7065,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x8243806e,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd00d,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae905c,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161706b,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e049,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662055,  /*  17: SATA RR 4015.6862745098ns */
           0x80662056,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x3ffff044,  /*  19: MCIF2_RD off */
           0x3ffff046,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e048,  /*  22: BSP RR 50000ns */
           0x80ad905a,  /*  23: SAGE RR 6820ns */
           0x86449070,  /*  24: FLASH_DMA RR 63000ns */
           0x8161706a,  /*  25: HIF_PCIe RR 13880ns */
           0x86449072,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449071,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff045,  /*  29: MCIF_RD off */
           0x3ffff047,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db066,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5069,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5068,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae105b,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db067,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff000,  /*  40: RAAGA off */
           0x3ffff003,  /*  41: RAAGA_1 off */
           0x3ffff001,  /*  42: RAAGA1 off */
           0x3ffff002,  /*  43: RAAGA1_1 off */
           0x3ffff02c,  /*  44: AUD_AIO off */
           0x3ffff050,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff06c,  /*  46: VICE_CME_CSC off */
           0x3ffff053,  /*  47: VICE_FME_CSC off */
           0x3ffff061,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff060,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff037,  /*  50: VICE_SG off */
           0x3ffff074,  /*  51: VICE_DBLK off */
           0x3ffff03f,  /*  52: VICE_CABAC0 off */
           0x3ffff043,  /*  53: VICE_CABAC1 off */
           0x3ffff058,  /*  54: VICE_ARCSS0 off */
           0x3ffff01d,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff032,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff01e,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff033,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff054,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff06f,  /*  60: VICE1_CME_CSC off */
           0x3ffff05d,  /*  61: VICE1_FME_CSC off */
           0x3ffff05f,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff05e,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff03e,  /*  64: VICE1_SG off */
           0x3ffff077,  /*  65: VICE1_DBLK off */
           0x3ffff042,  /*  66: VICE1_CABAC0 off */
           0x3ffff04a,  /*  67: VICE1_CABAC1 off */
           0x3ffff057,  /*  68: VICE1_ARCSS0 off */
           0x3ffff01a,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff02f,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff01b,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff030,  /*  72: VICE1_VIP1_INST1 off */
           0x8002807a,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x8002807b,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x3ffff04c,  /*  75: HVD0_ILCPU off */
           0x3ffff052,  /*  76: HVD0_OLCPU off */
           0x3ffff00a,  /*  77: HVD0_CAB off */
           0x3ffff021,  /*  78: HVD0_ILSI off */
           0x81ea306d,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff022,  /*  80: HVD0_ILSI_p2 off */
           0x8009f080,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8009f081,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff04e,  /*  83: HVD1_ILCPU off */
           0x3ffff063,  /*  84: HVD1_OLCPU off */
           0x3ffff025,  /*  85: HVD1_CAB off */
           0x3ffff023,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff075,  /*  88: HVD2_DBLK_0 off */
           0x3ffff076,  /*  89: HVD2_DBLK_1 off */
           0x3ffff04f,  /*  90: HVD2_ILCPU off */
           0x3ffff064,  /*  91: HVD2_OLCPU off */
           0x3ffff026,  /*  92: HVD2_CAB off */
           0x3ffff024,  /*  93: HVD2_ILSI off */
           0x005e3019,  /*  94: BVN_MAD_PIX_FD 3493.445ns */
           0x00848029,  /*  95: BVN_MAD_QUANT 4913.31ns */
           0x00bc802e,  /*  96: BVN_MAD_PIX_CAP 6987.885ns */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x00c7d035,  /* 106: BVN_MFD0_Ch 7407ns */
           0x0038f00c,  /* 107: BVN_MFD0_Ch_1 2115ns */
           0x0085302a,  /* 108: BVN_MFD1 4938ns */
           0x0058c016,  /* 109: BVN_MFD1_1 3292ns */
           0x3ffff02b,  /* 110: BVN_MFD2 off */
           0x3ffff017,  /* 111: BVN_MFD2_1 off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001fd008,  /* 118: BVN_VFD0 1185ns */
           0x3ffff009,  /* 119: BVN_VFD1 off */
           0x3ffff03a,  /* 120: BVN_VFD2 off */
           0x3ffff03b,  /* 121: BVN_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x004fc013,  /* 125: BVN_VFD7 2960ns */
           0x001e5005,  /* 126: BVN_CAP0 1128.594ns */
           0x3ffff006,  /* 127: BVN_CAP1 off */
           0x3ffff03c,  /* 128: BVN_CAP2 off */
           0x3ffff03d,  /* 129: BVN_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x004fc014,  /* 133: BVN_CAP7 2960ns */
           0x3ffff00b,  /* 134: BVN_GFD0 off */
           0x3ffff038,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004ae010,  /* 141: BVN_MCVP0 2780.03ns */
           0x004ae00f,  /* 142: BVN_MCVP1 2780.03ns */
           0x00d2c036,  /* 143: BVN_MCVP2 7810.75ns */
           0x3ffff015,  /* 144: BVN_RDC off */
           0x3ffff040,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff041,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff01f,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff034,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff01c,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff031,  /* 154: VICE1_VIP1_INST2 off */
           0x8002807e,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x8002807f,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
           0x3ffff018,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff028,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff02d,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x8014204b,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000008e,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104d,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008f,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000084,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x80000086,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff087,  /* 218: HVD2_PFRI off */
           0x3ffff082,  /* 219: VICE_PFRI off */
           0x3ffff085,  /* 220: VICE1_PFRI off */
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
           0xbfffe073,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00692020   /* 255: REFRESH 3900ns */
         };
static const uint32_t aulMemc2_20170809234254_7445_4kstb1t[] = {
           0x3ffff007,  /*   0: XPT_WR_RS off */
           0x3ffff051,  /*   1: XPT_WR_XC off */
           0x3ffff012,  /*   2: XPT_WR_CDB off */
           0x3ffff062,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff027,  /*   4: XPT_RD_RS off */
           0x3ffff039,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff011,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff059,  /*   7: XPT_RD_PB off */
           0x80fd7065,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x8243806e,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd00d,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae905c,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161706b,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e049,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662055,  /*  17: SATA RR 4015.6862745098ns */
           0x80662056,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x3ffff044,  /*  19: MCIF2_RD off */
           0x3ffff046,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e048,  /*  22: BSP RR 50000ns */
           0x80ad905a,  /*  23: SAGE RR 6820ns */
           0x86449070,  /*  24: FLASH_DMA RR 63000ns */
           0x8161706a,  /*  25: HIF_PCIe RR 13880ns */
           0x86449072,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449071,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff045,  /*  29: MCIF_RD off */
           0x3ffff047,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db066,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5069,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5068,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae105b,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db067,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff000,  /*  40: RAAGA off */
           0x3ffff003,  /*  41: RAAGA_1 off */
           0x3ffff001,  /*  42: RAAGA1 off */
           0x3ffff002,  /*  43: RAAGA1_1 off */
           0x3ffff02c,  /*  44: AUD_AIO off */
           0x3ffff050,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff06c,  /*  46: VICE_CME_CSC off */
           0x3ffff053,  /*  47: VICE_FME_CSC off */
           0x3ffff061,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff060,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff037,  /*  50: VICE_SG off */
           0x3ffff074,  /*  51: VICE_DBLK off */
           0x3ffff03f,  /*  52: VICE_CABAC0 off */
           0x3ffff043,  /*  53: VICE_CABAC1 off */
           0x3ffff058,  /*  54: VICE_ARCSS0 off */
           0x3ffff01d,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff032,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff01e,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff033,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff054,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff06f,  /*  60: VICE1_CME_CSC off */
           0x3ffff05d,  /*  61: VICE1_FME_CSC off */
           0x3ffff05f,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff05e,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff03e,  /*  64: VICE1_SG off */
           0x3ffff077,  /*  65: VICE1_DBLK off */
           0x3ffff042,  /*  66: VICE1_CABAC0 off */
           0x3ffff04a,  /*  67: VICE1_CABAC1 off */
           0x3ffff057,  /*  68: VICE1_ARCSS0 off */
           0x3ffff01a,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff02f,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff01b,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff030,  /*  72: VICE1_VIP1_INST1 off */
           0x80028078,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x80028079,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff04c,  /*  75: HVD0_ILCPU off */
           0x3ffff052,  /*  76: HVD0_OLCPU off */
           0x3ffff00a,  /*  77: HVD0_CAB off */
           0x3ffff021,  /*  78: HVD0_ILSI off */
           0x81ea306d,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff022,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff080,  /*  81: HVD1_DBLK_0 off */
           0x3ffff081,  /*  82: HVD1_DBLK_1 off */
           0x3ffff04e,  /*  83: HVD1_ILCPU off */
           0x3ffff063,  /*  84: HVD1_OLCPU off */
           0x3ffff025,  /*  85: HVD1_CAB off */
           0x3ffff023,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff075,  /*  88: HVD2_DBLK_0 off */
           0x3ffff076,  /*  89: HVD2_DBLK_1 off */
           0x3ffff04f,  /*  90: HVD2_ILCPU off */
           0x3ffff064,  /*  91: HVD2_OLCPU off */
           0x3ffff026,  /*  92: HVD2_CAB off */
           0x3ffff024,  /*  93: HVD2_ILSI off */
           0x3ffff019,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff029,  /*  95: BVN_MAD_QUANT off */
           0x3ffff02e,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0042800e,  /* 106: BVN_MFD0 2469ns */
           0x001c6004,  /* 107: BVN_MFD0_1 1057.5ns */
           0x3ffff02a,  /* 108: BVN_MFD1 off */
           0x3ffff016,  /* 109: BVN_MFD1_1 off */
           0x3ffff02b,  /* 110: BVN_MFD2 off */
           0x3ffff017,  /* 111: BVN_MFD2_1 off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff008,  /* 118: BVN_VFD0 off */
           0x001fd009,  /* 119: BVN_VFD1 1185ns */
           0x01aa703a,  /* 120: BVN_VFD2 15800ns */
           0x01aa703b,  /* 121: BVN_VFD3 15800ns */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff013,  /* 125: BVN_VFD7 off */
           0x3ffff005,  /* 126: BVN_CAP0 off */
           0x001e5006,  /* 127: BVN_CAP1 1128.594ns */
           0x01aa703c,  /* 128: BVN_CAP2 15800ns */
           0x01aa703d,  /* 129: BVN_CAP3 15800ns */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff014,  /* 133: BVN_CAP7 off */
           0x0031d00b,  /* 134: BVN_GFD0 1851ns */
           0x0167c038,  /* 135: BVN_GFD1 13330ns */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff010,  /* 141: BVN_MCVP0 off */
           0x3ffff00f,  /* 142: BVN_MCVP1 off */
           0x3ffff036,  /* 143: BVN_MCVP2 off */
           0x00571015,  /* 144: BVN_RDC 3230ns */
           0x3ffff040,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff041,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff01f,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff034,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff01c,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff031,  /* 154: VICE1_VIP1_INST2 off */
           0x8002807c,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8002807d,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff018,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff028,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff02d,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x8014204b,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000008e,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104d,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008f,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000083,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff086,  /* 217: HVD1_PFRI off */
           0x3ffff087,  /* 218: HVD2_PFRI off */
           0x3ffff082,  /* 219: VICE_PFRI off */
           0x3ffff085,  /* 220: VICE1_PFRI off */
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
           0xbfffe073,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00692020   /* 255: REFRESH 3900ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20170809234254_7445_4kstb1t[] = {
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x808d0802}, /* HVD2_PFRI (gHvd2) 483840.00 ns/60 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x40000366}, /* d: 4; p: 870.908333333333 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x00000660}, /* 1632 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x000003d3}, /* 60% * 1632 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_CONFIG,      0x80880803}, /* VICE_PFRI (gVice) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_0_PFRI_4_THROTTLE_CONFIG,      0x808b0802}, /* VICE1_PFRI (gVice1) 441176.47 ns/120 */
  {BCHP_MEMC_GEN_0_PFRI_4_THROTTLE_FILTER_CTRL, 0x4000018d}, /* d: 4; p: 397.058333333333 */
  {BCHP_MEMC_GEN_0_PFRI_4_THROTTLE_THRESH0,     0x000001a8}, /* 424 */
  {BCHP_MEMC_GEN_0_PFRI_4_THROTTLE_THRESH1,     0x000000fe}, /* 60% * 424 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x808a0803}, /* HVD0_PFRI_Ch (gHvdC0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x000007e0}, /* 2016 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x000004b9}, /* 60% * 2016 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x808c0802}, /* HVD1_PFRI (gHvd1) 483840.00 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000366}, /* d: 4; p: 870.908333333333 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x00000660}, /* 1632 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x000003d3}, /* 60% * 1632 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_CONFIG,      0x80890803}, /* HVD0_PFRI (gHvd0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH0,     0x00000f90}, /* 3984 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH1,     0x00000956}  /* 60% * 3984 */
};

static const uint32_t* const paulMemc_box15[] = { &aulMemc0_20170809234254_7445_4kstb1t[0], &aulMemc1_20170809234254_7445_4kstb1t[0], &aulMemc2_20170809234254_7445_4kstb1t[0]};

const BBOX_Rts stBoxRts_7445_4kstb1t_box15 = {
  "20170809234254_7445_4kstb1t_box15",
  7445,
  15,
  3,
  256,
  (const uint32_t**)&paulMemc_box15[0],
  24,
  stBoxRts_PfriClient_20170809234254_7445_4kstb1t
};
