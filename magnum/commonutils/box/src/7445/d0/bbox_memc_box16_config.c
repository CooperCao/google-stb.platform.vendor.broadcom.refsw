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
 ******************************************************************************
 *
 *                            Do Not Edit Directly
 * Auto-Generated from RTS environment:
 *   at: Wed Mar 23 22:37:16 2016 GMT
 *   by: robinc
 *   for: Box 7445D0_4kstb2t
 *         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
 *         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
 *         MemC 2 (32-bit DDR3@1067MHz) w/432MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445D0_Robin/BCM7445D0_zip1018.cfg
 *     xls_input/box_estar_zip1018.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445D0_Robin/BCM7445D0MemoryTiming.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b1.3r8_timing_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b1r8_timingf_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/VideoDecoder_rQ.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/Vice_v2p1c.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445D0_Robin/BCM7445D0BvnLib.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445D0_Robin/BCM7445D0Client.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445D0_Robin/BCM7445D0ClientMap.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445D0_Robin/BCM7445D0ClientTime_zip1018.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7445D0_Robin/BCM7445D0Display.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20160323223716_7445D0_4kstb2t[] = {
           0x001e7005,  /*   0: XPT_WR_RS 1134.2383107089ns */
           0x803b7056,  /*   1: XPT_WR_XC RR 2339.03576982893ns */
           0x804b1012,  /*   2: XPT_WR_CDB RR 2785.18518518519ns */
           0x80bc9065,  /*   3: XPT_WR_ITB_MSG RR 7408.86699507389ns */
           0x80780029,  /*   4: XPT_RD_RS RR 4449.70414201183ns */
           0x819e3040,  /*   5: XPT_RD_XC_RMX_MSG RR 15346.9387755102ns */
           0x804b1011,  /*   6: XPT_RD_XC_RAVE RR 2785.18518518519ns */
           0x80aae05e,  /*   7: XPT_RD_PB RR 6714.75409836066ns */
           0x80fd7067,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x8243806f,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd00a,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9061,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161706d,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e04e,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066205a,  /*  17: SATA RR 4015.6862745098ns */
           0x8066205b,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x03e6e049,  /*  19: MCIF2_RD 37000ns */
           0x03e6e04b,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e04d,  /*  22: BSP RR 50000ns */
           0x80ad905f,  /*  23: SAGE RR 6820ns */
           0x86449071,  /*  24: FLASH_DMA RR 63000ns */
           0x8161706c,  /*  25: HIF_PCIe RR 13880ns */
           0x86449073,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449072,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e04a,  /*  29: MCIF_RD 37000ns */
           0x03e6e04c,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db068,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c506b,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c506a,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1060,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db069,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x8050e058,  /*  40: RAAGA RR 3000ns */
           0x001ae001,  /*  41: RAAGA_1 1000ns */
           0x00a1e02f,  /*  42: RAAGA1 6000ns */
           0x001ae000,  /*  43: RAAGA1_1 1000ns */
           0x00bb4030,  /*  44: AUD_AIO 6940ns */
           0x8065c059,  /*  45: VICE_CME_RMB_CMB RR 4000ns */
           0x832ef070,  /*  46: VICE_CME_CSC RR 32000ns */
           0x80bb1062,  /*  47: VICE_FME_CSC RR 7350ns */
           0x80bb1064,  /*  48: VICE_FME_Luma_CMB RR 7350ns */
           0x80bb1063,  /*  49: VICE_FME_Chroma_CMB RR 7350ns */
           0x81b8d042,  /*  50: VICE_SG RR 16333.3333333333ns */
           0x80000088,  /*  51: VICE_DBLK RR 0ns */
           0x83dc1048,  /*  52: VICE_CABAC0 RR 36600ns */
           0x86f07050,  /*  53: VICE_CABAC1 RR 65800ns */
           0x80a2e05d,  /*  54: VICE_ARCSS0 RR 6400ns */
           0x8063d022,  /*  55: VICE_VIP0_INST0 RR 3703ns */
           0x80c7a037,  /*  56: VICE_VIP1_INST0 RR 7400ns */
           0x8063d023,  /*  57: VICE_VIP0_INST1 RR 3703ns */
           0x80c7a038,  /*  58: VICE_VIP1_INST1 RR 7400ns */
           0x3ffff075,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff07a,  /*  60: VICE1_CME_CSC off */
           0x3ffff076,  /*  61: VICE1_FME_CSC off */
           0x3ffff078,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff077,  /*  63: VICE1_FME_Chroma_CMB off */
           0x81b8d041,  /*  64: VICE1_SG RR 16333.3333333333ns */
           0x3ffff087,  /*  65: VICE1_DBLK off */
           0x83dc1047,  /*  66: VICE1_CABAC0 RR 36600ns */
           0x86f0704f,  /*  67: VICE1_CABAC1 RR 65800ns */
           0x3ffff079,  /*  68: VICE1_ARCSS0 off */
           0x3ffff01f,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff034,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff020,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff035,  /*  72: VICE1_VIP1_INST1 off */
           0x3ffff07d,  /*  73: HVD0_DBLK_0 off */
           0x3ffff07e,  /*  74: HVD0_DBLK_1 off */
           0x801c2052,  /*  75: HVD0_ILCPU RR 1048ns */
           0x804a7057,  /*  76: HVD0_OLCPU RR 2927ns */
           0x00596018,  /*  77: HVD0_CAB 3317ns */
           0x00710025,  /*  78: HVD0_ILSI 4191ns */
           0x81ea306e,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x00710026,  /*  80: HVD0_ILSI_p2 4191ns */
           0x800ff07c,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8007e07b,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x80270054,  /*  83: HVD1_ILCPU RR 1451ns */
           0x8070a05c,  /*  84: HVD1_OLCPU RR 4427ns */
           0x007dd02a,  /*  85: HVD1_CAB 4666ns */
           0x0073d027,  /*  86: HVD1_ILSI 4296ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff085,  /*  88: HVD2_DBLK_0 off */
           0x3ffff086,  /*  89: HVD2_DBLK_1 off */
           0x803b3055,  /*  90: HVD2_ILCPU RR 2198ns */
           0x80e33066,  /*  91: HVD2_OLCPU RR 8925ns */
           0x0091602e,  /*  92: HVD2_CAB 5390ns */
           0x00743028,  /*  93: HVD2_ILSI 4308ns */
           0x005ea01b,  /*  94: BVN_MAD_PIX_FD 3511ns */
           0x0085302d,  /*  95: BVN_MAD_QUANT 4938ns */
           0x00bd7033,  /*  96: BVN_MAD_PIX_CAP 7023ns */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff019,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff02b,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff031,  /* 105: BVN_MAD3_PIX_CAP off */
           0x3ffff00b,  /* 106: BVN_MFD0 off */
           0x3ffff002,  /* 107: BVN_MFD0_1 off */
           0x0063d01c,  /* 108: BVN_MFD1 3703ns */
           0x0042800c,  /* 109: BVN_MFD1_1 2469ns */
           0x0063d01d,  /* 110: BVN_MFD2 3703ns */
           0x0042800d,  /* 111: BVN_MFD2_1 2469ns */
           0x3ffff01e,  /* 112: BVN_MFD3 off */
           0x3ffff00e,  /* 113: BVN_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001fd006,  /* 118: BVN_VFD0 1185ns */
           0x3ffff007,  /* 119: BVN_VFD1 off */
           0x3ffff045,  /* 120: BVN_VFD2 off */
           0x3ffff046,  /* 121: BVN_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff013,  /* 124: BVN_VFD6 off */
           0x3ffff014,  /* 125: BVN_VFD7 off */
           0x001e5003,  /* 126: BVN_CAP0 1128.594ns */
           0x3ffff004,  /* 127: BVN_CAP1 off */
           0x3ffff03e,  /* 128: BVN_CAP2 off */
           0x3ffff03f,  /* 129: BVN_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff015,  /* 132: BVN_CAP6 off */
           0x3ffff016,  /* 133: BVN_CAP7 off */
           0x3ffff008,  /* 134: BVN_GFD0 off */
           0x3ffff03d,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff010,  /* 141: BVN_MCVP0 off */
           0x3ffff00f,  /* 142: BVN_MCVP1 off */
           0x3ffff03c,  /* 143: BVN_MCVP2 off */
           0x3ffff017,  /* 144: BVN_RDC off */
           0x03526043,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526044,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x8063d024,  /* 151: VICE_VIP0_INST2 RR 3703ns */
           0x80c7a039,  /* 152: VICE_VIP1_INST2 RR 7400ns */
           0x3ffff021,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff036,  /* 154: VICE1_VIP1_INST2 off */
           0x3ffff081,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff082,  /* 156: HVD0_DBLK_p2_1 off */
           0x005ea01a,  /* 157: BVN_MAD4_PIX_FD 3511ns */
           0x0085302c,  /* 158: BVN_MAD4_QUANT 4938ns */
           0x00bd7032,  /* 159: BVN_MAD4_PIX_CAP 7023ns */
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
           0x80142051,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x80000095,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261053,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x80000096,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff089,  /* 216: HVD0_PFRI off */
           0x8000008d,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff08e,  /* 218: HVD2_PFRI off */
           0x8000008c,  /* 219: VICE_PFRI RR 0ns */
           0x3ffff08b,  /* 220: VICE1_PFRI off */
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
           0xbfffe074,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2703b   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20160323223716_7445D0_4kstb2t[] = {
           0x3ffff005,  /*   0: XPT_WR_RS off */
           0x3ffff056,  /*   1: XPT_WR_XC off */
           0x3ffff012,  /*   2: XPT_WR_CDB off */
           0x3ffff065,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff029,  /*   4: XPT_RD_RS off */
           0x3ffff040,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff011,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff05e,  /*   7: XPT_RD_PB off */
           0x80fd7067,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x8243806f,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd00a,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9061,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161706d,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e04e,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066205a,  /*  17: SATA RR 4015.6862745098ns */
           0x8066205b,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x3ffff049,  /*  19: MCIF2_RD off */
           0x3ffff04b,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e04d,  /*  22: BSP RR 50000ns */
           0x80ad905f,  /*  23: SAGE RR 6820ns */
           0x86449071,  /*  24: FLASH_DMA RR 63000ns */
           0x8161706c,  /*  25: HIF_PCIe RR 13880ns */
           0x86449073,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449072,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff04a,  /*  29: MCIF_RD off */
           0x3ffff04c,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db068,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c506b,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c506a,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1060,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db069,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff058,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff02f,  /*  42: RAAGA1 off */
           0x3ffff000,  /*  43: RAAGA1_1 off */
           0x3ffff030,  /*  44: AUD_AIO off */
           0x3ffff059,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff070,  /*  46: VICE_CME_CSC off */
           0x3ffff062,  /*  47: VICE_FME_CSC off */
           0x3ffff064,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff063,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff042,  /*  50: VICE_SG off */
           0x3ffff088,  /*  51: VICE_DBLK off */
           0x3ffff048,  /*  52: VICE_CABAC0 off */
           0x3ffff050,  /*  53: VICE_CABAC1 off */
           0x3ffff05d,  /*  54: VICE_ARCSS0 off */
           0x3ffff022,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff037,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff023,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff038,  /*  58: VICE_VIP1_INST1 off */
           0x8065c075,  /*  59: VICE1_CME_RMB_CMB RR 4000ns */
           0x832ef07a,  /*  60: VICE1_CME_CSC RR 32000ns */
           0x80bb1076,  /*  61: VICE1_FME_CSC RR 7350ns */
           0x80bb1078,  /*  62: VICE1_FME_Luma_CMB RR 7350ns */
           0x80bb1077,  /*  63: VICE1_FME_Chroma_CMB RR 7350ns */
           0x3ffff041,  /*  64: VICE1_SG off */
           0x80000087,  /*  65: VICE1_DBLK RR 0ns */
           0x3ffff047,  /*  66: VICE1_CABAC0 off */
           0x3ffff04f,  /*  67: VICE1_CABAC1 off */
           0x80a2e079,  /*  68: VICE1_ARCSS0 RR 6400ns */
           0x8063d01f,  /*  69: VICE1_VIP0_INST0 RR 3703ns */
           0x80c7a034,  /*  70: VICE1_VIP1_INST0 RR 7400ns */
           0x8063d020,  /*  71: VICE1_VIP0_INST1 RR 3703ns */
           0x80c7a035,  /*  72: VICE1_VIP1_INST1 RR 7400ns */
           0x800bb07f,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x8005c080,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x3ffff052,  /*  75: HVD0_ILCPU off */
           0x3ffff057,  /*  76: HVD0_OLCPU off */
           0x3ffff018,  /*  77: HVD0_CAB off */
           0x3ffff025,  /*  78: HVD0_ILSI off */
           0x81ea306e,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff026,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff07c,  /*  81: HVD1_DBLK_0 off */
           0x3ffff07b,  /*  82: HVD1_DBLK_1 off */
           0x3ffff054,  /*  83: HVD1_ILCPU off */
           0x3ffff05c,  /*  84: HVD1_OLCPU off */
           0x3ffff02a,  /*  85: HVD1_CAB off */
           0x3ffff027,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x80208085,  /*  88: HVD2_DBLK_0 RR 0ns */
           0x80103086,  /*  89: HVD2_DBLK_1 RR 0ns */
           0x3ffff055,  /*  90: HVD2_ILCPU off */
           0x3ffff066,  /*  91: HVD2_OLCPU off */
           0x3ffff02e,  /*  92: HVD2_CAB off */
           0x3ffff028,  /*  93: HVD2_ILSI off */
           0x3ffff01b,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff02d,  /*  95: BVN_MAD_QUANT off */
           0x3ffff033,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x005ea019,  /* 103: BVN_MAD3_PIX_FD 3511ns */
           0x0085302b,  /* 104: BVN_MAD3_QUANT 4938ns */
           0x00bd7031,  /* 105: BVN_MAD3_PIX_CAP 7023ns */
           0x00c7d03a,  /* 106: BVN_MFD0_Ch 7407ns */
           0x0038f009,  /* 107: BVN_MFD0_Ch_1 2115ns */
           0x3ffff01c,  /* 108: BVN_MFD1 off */
           0x3ffff00c,  /* 109: BVN_MFD1_1 off */
           0x3ffff01d,  /* 110: BVN_MFD2 off */
           0x3ffff00d,  /* 111: BVN_MFD2_1 off */
           0x0063d01e,  /* 112: BVN_MFD3 3703ns */
           0x0042800e,  /* 113: BVN_MFD3_1 2469ns */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff006,  /* 118: BVN_VFD0 off */
           0x3ffff007,  /* 119: BVN_VFD1 off */
           0x3ffff045,  /* 120: BVN_VFD2 off */
           0x3ffff046,  /* 121: BVN_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x004fc013,  /* 124: BVN_VFD6 2960ns */
           0x004fc014,  /* 125: BVN_VFD7 2960ns */
           0x3ffff003,  /* 126: BVN_CAP0 off */
           0x3ffff004,  /* 127: BVN_CAP1 off */
           0x3ffff03e,  /* 128: BVN_CAP2 off */
           0x3ffff03f,  /* 129: BVN_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x004fc015,  /* 132: BVN_CAP6 2960ns */
           0x004fc016,  /* 133: BVN_CAP7 2960ns */
           0x3ffff008,  /* 134: BVN_GFD0 off */
           0x3ffff03d,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004af010,  /* 141: BVN_MCVP0 2781ns */
           0x004af00f,  /* 142: BVN_MCVP1 2781ns */
           0x00d2c03c,  /* 143: BVN_MCVP2 7811ns */
           0x3ffff017,  /* 144: BVN_RDC off */
           0x3ffff043,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff044,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff024,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff039,  /* 152: VICE_VIP1_INST2 off */
           0x8063d021,  /* 153: VICE1_VIP0_INST2 RR 3703ns */
           0x80c7a036,  /* 154: VICE1_VIP1_INST2 RR 7400ns */
           0x800bb083,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x8005c084,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
           0x3ffff01a,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff02c,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff032,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x80142051,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x80000095,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261053,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x80000096,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000008a,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x3ffff08d,  /* 217: HVD1_PFRI off */
           0x8000008e,  /* 218: HVD2_PFRI RR 0ns */
           0x3ffff08c,  /* 219: VICE_PFRI off */
           0x8000008b,  /* 220: VICE1_PFRI RR 0ns */
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
           0xbfffe074,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2703b   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc2_20160323223716_7445D0_4kstb2t[] = {
           0x3ffff005,  /*   0: XPT_WR_RS off */
           0x3ffff056,  /*   1: XPT_WR_XC off */
           0x3ffff012,  /*   2: XPT_WR_CDB off */
           0x3ffff065,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff029,  /*   4: XPT_RD_RS off */
           0x3ffff040,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff011,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff05e,  /*   7: XPT_RD_PB off */
           0x80fd7067,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x8243806f,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd00a,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9061,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161706d,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e04e,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066205a,  /*  17: SATA RR 4015.6862745098ns */
           0x8066205b,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x3ffff049,  /*  19: MCIF2_RD off */
           0x3ffff04b,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e04d,  /*  22: BSP RR 50000ns */
           0x80ad905f,  /*  23: SAGE RR 6820ns */
           0x86449071,  /*  24: FLASH_DMA RR 63000ns */
           0x8161706c,  /*  25: HIF_PCIe RR 13880ns */
           0x86449073,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449072,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff04a,  /*  29: MCIF_RD off */
           0x3ffff04c,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db068,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c506b,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c506a,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1060,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db069,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff058,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff02f,  /*  42: RAAGA1 off */
           0x3ffff000,  /*  43: RAAGA1_1 off */
           0x3ffff030,  /*  44: AUD_AIO off */
           0x3ffff059,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff070,  /*  46: VICE_CME_CSC off */
           0x3ffff062,  /*  47: VICE_FME_CSC off */
           0x3ffff064,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff063,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff042,  /*  50: VICE_SG off */
           0x3ffff088,  /*  51: VICE_DBLK off */
           0x3ffff048,  /*  52: VICE_CABAC0 off */
           0x3ffff050,  /*  53: VICE_CABAC1 off */
           0x3ffff05d,  /*  54: VICE_ARCSS0 off */
           0x3ffff022,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff037,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff023,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff038,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff075,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff07a,  /*  60: VICE1_CME_CSC off */
           0x3ffff076,  /*  61: VICE1_FME_CSC off */
           0x3ffff078,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff077,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff041,  /*  64: VICE1_SG off */
           0x3ffff087,  /*  65: VICE1_DBLK off */
           0x3ffff047,  /*  66: VICE1_CABAC0 off */
           0x3ffff04f,  /*  67: VICE1_CABAC1 off */
           0x3ffff079,  /*  68: VICE1_ARCSS0 off */
           0x3ffff01f,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff034,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff020,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff035,  /*  72: VICE1_VIP1_INST1 off */
           0x800bb07d,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8005c07e,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff052,  /*  75: HVD0_ILCPU off */
           0x3ffff057,  /*  76: HVD0_OLCPU off */
           0x3ffff018,  /*  77: HVD0_CAB off */
           0x3ffff025,  /*  78: HVD0_ILSI off */
           0x81ea306e,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff026,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff07c,  /*  81: HVD1_DBLK_0 off */
           0x3ffff07b,  /*  82: HVD1_DBLK_1 off */
           0x3ffff054,  /*  83: HVD1_ILCPU off */
           0x3ffff05c,  /*  84: HVD1_OLCPU off */
           0x3ffff02a,  /*  85: HVD1_CAB off */
           0x3ffff027,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff085,  /*  88: HVD2_DBLK_0 off */
           0x3ffff086,  /*  89: HVD2_DBLK_1 off */
           0x3ffff055,  /*  90: HVD2_ILCPU off */
           0x3ffff066,  /*  91: HVD2_OLCPU off */
           0x3ffff02e,  /*  92: HVD2_CAB off */
           0x3ffff028,  /*  93: HVD2_ILSI off */
           0x3ffff01b,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff02d,  /*  95: BVN_MAD_QUANT off */
           0x3ffff033,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff019,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff02b,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff031,  /* 105: BVN_MAD3_PIX_CAP off */
           0x0042800b,  /* 106: BVN_MFD0 2469ns */
           0x001c6002,  /* 107: BVN_MFD0_1 1057.5ns */
           0x3ffff01c,  /* 108: BVN_MFD1 off */
           0x3ffff00c,  /* 109: BVN_MFD1_1 off */
           0x3ffff01d,  /* 110: BVN_MFD2 off */
           0x3ffff00d,  /* 111: BVN_MFD2_1 off */
           0x3ffff01e,  /* 112: BVN_MFD3 off */
           0x3ffff00e,  /* 113: BVN_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff006,  /* 118: BVN_VFD0 off */
           0x001fd007,  /* 119: BVN_VFD1 1185ns */
           0x0359a045,  /* 120: BVN_VFD2 31770ns */
           0x0359a046,  /* 121: BVN_VFD3 31770ns */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff013,  /* 124: BVN_VFD6 off */
           0x3ffff014,  /* 125: BVN_VFD7 off */
           0x3ffff003,  /* 126: BVN_CAP0 off */
           0x001e5004,  /* 127: BVN_CAP1 1128.594ns */
           0x0174703e,  /* 128: BVN_CAP2 13800ns */
           0x0174703f,  /* 129: BVN_CAP3 13800ns */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff015,  /* 132: BVN_CAP6 off */
           0x3ffff016,  /* 133: BVN_CAP7 off */
           0x0031d008,  /* 134: BVN_GFD0 1851ns */
           0x0167c03d,  /* 135: BVN_GFD1 13330ns */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff010,  /* 141: BVN_MCVP0 off */
           0x3ffff00f,  /* 142: BVN_MCVP1 off */
           0x3ffff03c,  /* 143: BVN_MCVP2 off */
           0x00571017,  /* 144: BVN_RDC 3230ns */
           0x3ffff043,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff044,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff024,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff039,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff021,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff036,  /* 154: VICE1_VIP1_INST2 off */
           0x800bb081,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8005c082,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff01a,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff02c,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff032,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x80142051,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x80000095,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261053,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x80000096,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000089,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff08d,  /* 217: HVD1_PFRI off */
           0x3ffff08e,  /* 218: HVD2_PFRI off */
           0x3ffff08c,  /* 219: VICE_PFRI off */
           0x3ffff08b,  /* 220: VICE1_PFRI off */
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
           0xbfffe074,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2703b   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20160323223716_7445D0_4kstb2t[] = {
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x80930802}, /* HVD1_PFRI (gHvd1) 530666.67 ns/20 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000b31}, /* d: 4; p: 2865.5875 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x000023f4}, /* 9204 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x00001592}, /* 60% * 9204 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_CONFIG,      0x80920803}, /* VICE_PFRI (gVice) 441176.47 ns/120 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_FILTER_CTRL, 0x4000018d}, /* d: 4; p: 397.058333333333 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80900803}, /* HVD0_PFRI_Ch (gHvdC0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000cfe}, /* 3326 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x000007cb}, /* 60% * 3326 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_CONFIG,      0x80940802}, /* HVD2_PFRI (gHvd2) 967920.00 ns/20 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_FILTER_CTRL, 0x4000146a}, /* d: 4; p: 5226.7625 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH0,     0x000023f4}, /* 9204 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH1,     0x00001592}, /* 60% * 9204 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_CONFIG,      0x80910802}, /* VICE1_PFRI (gVice1) 441176.47 ns/120 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_FILTER_CTRL, 0x4000018d}, /* d: 4; p: 397.058333333333 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH0,     0x000001a8}, /* 424 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH1,     0x000000fe}, /* 60% * 424 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_CONFIG,      0x808f0803}, /* HVD0_PFRI (gHvd0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH0,     0x00001642}, /* 5698 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH1,     0x00000d5a}  /* 60% * 5698 */
};

static const uint32_t* const paulMemc_box16[] = { &aulMemc0_20160323223716_7445D0_4kstb2t[0], &aulMemc1_20160323223716_7445D0_4kstb2t[0], &aulMemc2_20160323223716_7445D0_4kstb2t[0]};

const BBOX_Rts stBoxRts_7445D0_4kstb2t_box16 = {
  "20160323223716_7445D0_4kstb2t_box16",
  7445,
  16,
  3,
  256,
  (const uint32_t**)&paulMemc_box16[0],
  24,
  stBoxRts_PfriClient_20160323223716_7445D0_4kstb2t
};
