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
******************************************************************************/
/*******************************************************************
*               Do Not Edit Directly
* Auto-Generated from RTS environment:
*   at: Wed Jun 24 00:52:33 2015 GMT
*   by: robinc
*   for: Box 7445_6T
*         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 2 (32-bit DDR3@1067MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20150624005233_7445_6T[] = {
           0x00303002,  /*   0: XPT_WR_RS 1790.47619047619ns */
           0x8042f063,  /*   1: XPT_WR_XC RR 2633.97548161121ns */
           0x8054700e,  /*   2: XPT_WR_CDB RR 3133.33333333333ns */
           0x80d9806d,  /*   3: XPT_WR_ITB_MSG RR 8545.45454545454ns */
           0x80abf031,  /*   4: XPT_RD_RS RR 6372.8813559322ns */
           0x81d8004b,  /*   5: XPT_RD_XC_RMX_MSG RR 17488.3720930233ns */
           0x8054700d,  /*   6: XPT_RD_XC_RAVE RR 3133.33333333333ns */
           0x809b9069,  /*   7: XPT_RD_PB RR 6113.4328358209ns */
           0x80fd706e,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438075,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd003,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae906c,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617074,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05d,  /*  16: MOCA_MIPS RR 53000ns */
           0x807c1067,  /*  17: SATA RR 4876.19047619048ns */
           0x807c1068,  /*  18: SATA_1 RR 4876.19047619048ns */
           0x03e6e056,  /*  19: MCIF2_RD 37000ns */
           0x03e6e058,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05c,  /*  22: BSP RR 50000ns */
           0x80ad906a,  /*  23: SAGE RR 6820ns */
           0x86449077,  /*  24: FLASH_DMA RR 63000ns */
           0x81617073,  /*  25: HIF_PCIe RR 13880ns */
           0x86449079,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449078,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e057,  /*  29: MCIF_RD 37000ns */
           0x03e6e059,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db06f,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5072,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5071,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae106b,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db070,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x0050e00b,  /*  40: RAAGA 3000ns */
           0x001ae001,  /*  41: RAAGA_1 1000ns */
           0x0050e00c,  /*  42: RAAGA1 3000ns */
           0x001ae000,  /*  43: RAAGA1_1 1000ns */
           0x00bb4032,  /*  44: AUD_AIO 6940ns */
           0x3ffff07f,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff086,  /*  46: VICE_CME_CSC off */
           0x3ffff080,  /*  47: VICE_FME_CSC off */
           0x3ffff082,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff081,  /*  49: VICE_FME_Chroma_CMB off */
           0x818f704a,  /*  50: VICE_SG RR 14800ns */
           0x3ffff090,  /*  51: VICE_DBLK off */
           0x8292a053,  /*  52: VICE_CABAC0 RR 24400ns */
           0x84a1205b,  /*  53: VICE_CABAC1 RR 43900ns */
           0x3ffff085,  /*  54: VICE_ARCSS0 off */
           0x3ffff036,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff04f,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff037,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff050,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff07b,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff084,  /*  60: VICE1_CME_CSC off */
           0x3ffff07c,  /*  61: VICE1_FME_CSC off */
           0x3ffff07e,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff07d,  /*  63: VICE1_FME_Chroma_CMB off */
           0x818f7049,  /*  64: VICE1_SG RR 14800ns */
           0x3ffff08f,  /*  65: VICE1_DBLK off */
           0x8292a052,  /*  66: VICE1_CABAC0 RR 24400ns */
           0x84a1205a,  /*  67: VICE1_CABAC1 RR 43900ns */
           0x3ffff083,  /*  68: VICE1_ARCSS0 off */
           0x3ffff033,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff04c,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff034,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff04d,  /*  72: VICE1_VIP1_INST1 off */
           0x800fa089,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8007c087,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x80270060,  /*  75: HVD0_ILCPU RR 1451ns */
           0x8070a064,  /*  76: HVD0_OLCPU RR 4427ns */
           0x0071401c,  /*  77: HVD0_CAB 4200ns */
           0x00739021,  /*  78: HVD0_ILSI 4287ns */
           0x825d1076,  /*  79: HVD0_ILCPU_p2 RR 22416ns */
           0x00739022,  /*  80: HVD0_ILSI_p2 4287ns */
           0x3ffff08b,  /*  81: HVD1_DBLK_0 off */
           0x3ffff08c,  /*  82: HVD1_DBLK_1 off */
           0x80270061,  /*  83: HVD1_ILCPU RR 1451ns */
           0x8070a065,  /*  84: HVD1_OLCPU RR 4427ns */
           0x0071401d,  /*  85: HVD1_CAB 4200ns */
           0x0072d01f,  /*  86: HVD1_ILSI 4258ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff08d,  /*  88: HVD2_DBLK_0 off */
           0x3ffff08e,  /*  89: HVD2_DBLK_1 off */
           0x80270062,  /*  90: HVD2_ILCPU RR 1451ns */
           0x8070a066,  /*  91: HVD2_OLCPU RR 4427ns */
           0x0071401e,  /*  92: HVD2_CAB 4200ns */
           0x0072d020,  /*  93: HVD2_ILSI 4258ns */
           0x3ffff028,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff03d,  /*  95: BVN_MAD_QUANT off */
           0x3ffff044,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff024,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff039,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff040,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff025,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff03a,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff041,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff026,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff03b,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff042,  /* 105: BVN_MAD3_PIX_CAP off */
           0x00428004,  /* 106: BVN_MFD0 2469ns */
           0x00428005,  /* 107: BVN_MFD0_1 2469ns */
           0x0063d015,  /* 108: BVN_MFD1 3703ns */
           0x00428006,  /* 109: BVN_MFD1_1 2469ns */
           0x3ffff016,  /* 110: BVN_MFD2 off */
           0x3ffff007,  /* 111: BVN_MFD2_1 off */
           0x3ffff017,  /* 112: BVN_MFD3 off */
           0x3ffff008,  /* 113: BVN_MFD3_1 off */
           0x3ffff018,  /* 114: BVN_MFD4 off */
           0x3ffff009,  /* 115: BVN_MFD4_1 off */
           0x3ffff019,  /* 116: BVN_MFD5 off */
           0x3ffff00a,  /* 117: BVN_MFD5_1 off */
           0x3ffff045,  /* 118: BVN_VFD0 off */
           0x3ffff046,  /* 119: BVN_VFD1 off */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff029,  /* 122: BVN_VFD4 off */
           0x3ffff02a,  /* 123: BVN_VFD5 off */
           0x3ffff02b,  /* 124: BVN_VFD6 off */
           0x3ffff02c,  /* 125: BVN_VFD7 off */
           0x3ffff03f,  /* 126: BVN_CAP0 off */
           0x3ffff047,  /* 127: BVN_CAP1 off */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff02d,  /* 130: BVN_CAP4 off */
           0x3ffff02e,  /* 131: BVN_CAP5 off */
           0x3ffff02f,  /* 132: BVN_CAP6 off */
           0x3ffff030,  /* 133: BVN_CAP7 off */
           0x3ffff023,  /* 134: BVN_GFD0 off */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x0055900f,  /* 136: BVN_GFD2 3174ns */
           0x3ffff010,  /* 137: BVN_GFD3 off */
           0x3ffff011,  /* 138: BVN_GFD4 off */
           0x3ffff012,  /* 139: BVN_GFD5 off */
           0x3ffff013,  /* 140: BVN_GFD6 off */
           0x3ffff01b,  /* 141: BVN_MCVP0 off */
           0x3ffff01a,  /* 142: BVN_MCVP1 off */
           0x3ffff048,  /* 143: BVN_MCVP2 off */
           0x3ffff014,  /* 144: BVN_RDC off */
           0x03526054,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526055,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff038,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff051,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff035,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff04e,  /* 154: VICE1_VIP1_INST2 off */
           0x800fa08a,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8007c088,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff027,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff03c,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff043,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x8014205e,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000009b,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026105f,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000009c,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000093,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff094,  /* 217: HVD1_PFRI off */
           0x3ffff095,  /* 218: HVD2_PFRI off */
           0x3ffff092,  /* 219: VICE_PFRI off */
           0x3ffff091,  /* 220: VICE1_PFRI off */
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
           0xbfffe07a,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2703e   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20150624005233_7445_6T[] = {
           0x3ffff002,  /*   0: XPT_WR_RS off */
           0x3ffff063,  /*   1: XPT_WR_XC off */
           0x3ffff00e,  /*   2: XPT_WR_CDB off */
           0x3ffff06d,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff031,  /*   4: XPT_RD_RS off */
           0x3ffff04b,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00d,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff069,  /*   7: XPT_RD_PB off */
           0x80fd706e,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438075,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd003,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae906c,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617074,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05d,  /*  16: MOCA_MIPS RR 53000ns */
           0x807c1067,  /*  17: SATA RR 4876.19047619048ns */
           0x807c1068,  /*  18: SATA_1 RR 4876.19047619048ns */
           0x3ffff056,  /*  19: MCIF2_RD off */
           0x3ffff058,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05c,  /*  22: BSP RR 50000ns */
           0x80ad906a,  /*  23: SAGE RR 6820ns */
           0x86449077,  /*  24: FLASH_DMA RR 63000ns */
           0x81617073,  /*  25: HIF_PCIe RR 13880ns */
           0x86449079,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449078,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff057,  /*  29: MCIF_RD off */
           0x3ffff059,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db06f,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5072,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5071,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae106b,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db070,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff00b,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff00c,  /*  42: RAAGA1 off */
           0x3ffff000,  /*  43: RAAGA1_1 off */
           0x3ffff032,  /*  44: AUD_AIO off */
           0x3ffff07f,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff086,  /*  46: VICE_CME_CSC off */
           0x3ffff080,  /*  47: VICE_FME_CSC off */
           0x3ffff082,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff081,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff04a,  /*  50: VICE_SG off */
           0x3ffff090,  /*  51: VICE_DBLK off */
           0x3ffff053,  /*  52: VICE_CABAC0 off */
           0x3ffff05b,  /*  53: VICE_CABAC1 off */
           0x3ffff085,  /*  54: VICE_ARCSS0 off */
           0x3ffff036,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff04f,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff037,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff050,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff07b,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff084,  /*  60: VICE1_CME_CSC off */
           0x3ffff07c,  /*  61: VICE1_FME_CSC off */
           0x3ffff07e,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff07d,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff049,  /*  64: VICE1_SG off */
           0x3ffff08f,  /*  65: VICE1_DBLK off */
           0x3ffff052,  /*  66: VICE1_CABAC0 off */
           0x3ffff05a,  /*  67: VICE1_CABAC1 off */
           0x3ffff083,  /*  68: VICE1_ARCSS0 off */
           0x3ffff033,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff04c,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff034,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff04d,  /*  72: VICE1_VIP1_INST1 off */
           0x3ffff089,  /*  73: HVD0_DBLK_0 off */
           0x3ffff087,  /*  74: HVD0_DBLK_1 off */
           0x3ffff060,  /*  75: HVD0_ILCPU off */
           0x3ffff064,  /*  76: HVD0_OLCPU off */
           0x3ffff01c,  /*  77: HVD0_CAB off */
           0x3ffff021,  /*  78: HVD0_ILSI off */
           0x825d1076,  /*  79: HVD0_ILCPU_p2 RR 22416ns */
           0x3ffff022,  /*  80: HVD0_ILSI_p2 off */
           0x8007e08b,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8003e08c,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff061,  /*  83: HVD1_ILCPU off */
           0x3ffff065,  /*  84: HVD1_OLCPU off */
           0x3ffff01d,  /*  85: HVD1_CAB off */
           0x3ffff01f,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff08d,  /*  88: HVD2_DBLK_0 off */
           0x3ffff08e,  /*  89: HVD2_DBLK_1 off */
           0x3ffff062,  /*  90: HVD2_ILCPU off */
           0x3ffff066,  /*  91: HVD2_OLCPU off */
           0x3ffff01e,  /*  92: HVD2_CAB off */
           0x3ffff020,  /*  93: HVD2_ILSI off */
           0x008e1028,  /*  94: BVN_MAD_PIX_FD 5266.5ns */
           0x00c7d03d,  /*  95: BVN_MAD_QUANT 7407ns */
           0x011c4044,  /*  96: BVN_MAD_PIX_CAP 10534.5ns */
           0x008e1024,  /*  97: BVN_MAD1_PIX_FD 5266.5ns */
           0x00c7d039,  /*  98: BVN_MAD1_QUANT 7407ns */
           0x011c4040,  /*  99: BVN_MAD1_PIX_CAP 10534.5ns */
           0x008e1025,  /* 100: BVN_MAD2_PIX_FD 5266.5ns */
           0x00c7d03a,  /* 101: BVN_MAD2_QUANT 7407ns */
           0x011c4041,  /* 102: BVN_MAD2_PIX_CAP 10534.5ns */
           0x008e1026,  /* 103: BVN_MAD3_PIX_FD 5266.5ns */
           0x00c7d03b,  /* 104: BVN_MAD3_QUANT 7407ns */
           0x011c4042,  /* 105: BVN_MAD3_PIX_CAP 10534.5ns */
           0x3ffff004,  /* 106: BVN_MFD0 off */
           0x3ffff005,  /* 107: BVN_MFD0_1 off */
           0x3ffff015,  /* 108: BVN_MFD1 off */
           0x3ffff006,  /* 109: BVN_MFD1_1 off */
           0x0063d016,  /* 110: BVN_MFD2 3703ns */
           0x00428007,  /* 111: BVN_MFD2_1 2469ns */
           0x0063d017,  /* 112: BVN_MFD3 3703ns */
           0x00428008,  /* 113: BVN_MFD3_1 2469ns */
           0x3ffff018,  /* 114: BVN_MFD4 off */
           0x3ffff009,  /* 115: BVN_MFD4_1 off */
           0x3ffff019,  /* 116: BVN_MFD5 off */
           0x3ffff00a,  /* 117: BVN_MFD5_1 off */
           0x011fb045,  /* 118: BVN_VFD0 10660ns */
           0x011fb046,  /* 119: BVN_VFD1 10660ns */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x0095d029,  /* 122: BVN_VFD4 5555ns */
           0x0095d02a,  /* 123: BVN_VFD5 5555ns */
           0x0095d02b,  /* 124: BVN_VFD6 5555ns */
           0x0095d02c,  /* 125: BVN_VFD7 5555ns */
           0x0111f03f,  /* 126: BVN_CAP0 10152.584ns */
           0x011fb047,  /* 127: BVN_CAP1 10660ns */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x0095d02d,  /* 130: BVN_CAP4 5555ns */
           0x0095d02e,  /* 131: BVN_CAP5 5555ns */
           0x0095d02f,  /* 132: BVN_CAP6 5555ns */
           0x0095d030,  /* 133: BVN_CAP7 5555ns */
           0x0077d023,  /* 134: BVN_GFD0 4444ns */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x3ffff00f,  /* 136: BVN_GFD2 off */
           0x3ffff010,  /* 137: BVN_GFD3 off */
           0x3ffff011,  /* 138: BVN_GFD4 off */
           0x00559012,  /* 139: BVN_GFD5 3174ns */
           0x00559013,  /* 140: BVN_GFD6 3174ns */
           0x0070801b,  /* 141: BVN_MCVP0 4171.5ns */
           0x0070801a,  /* 142: BVN_MCVP1 4171.5ns */
           0x013c3048,  /* 143: BVN_MCVP2 11716.5ns */
           0x00571014,  /* 144: BVN_RDC 3230ns */
           0x3ffff054,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff055,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff038,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff051,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff035,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff04e,  /* 154: VICE1_VIP1_INST2 off */
           0x3ffff08a,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff088,  /* 156: HVD0_DBLK_p2_1 off */
           0x008e1027,  /* 157: BVN_MAD4_PIX_FD 5266.5ns */
           0x00c7d03c,  /* 158: BVN_MAD4_QUANT 7407ns */
           0x011c4043,  /* 159: BVN_MAD4_PIX_CAP 10534.5ns */
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
           0x8014205e,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000009b,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026105f,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000009c,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff093,  /* 216: HVD0_PFRI off */
           0x80000094,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff095,  /* 218: HVD2_PFRI off */
           0x3ffff092,  /* 219: VICE_PFRI off */
           0x3ffff091,  /* 220: VICE1_PFRI off */
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
           0xbfffe07a,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2703e   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc2_20150624005233_7445_6T[] = {
           0x3ffff002,  /*   0: XPT_WR_RS off */
           0x3ffff063,  /*   1: XPT_WR_XC off */
           0x3ffff00e,  /*   2: XPT_WR_CDB off */
           0x3ffff06d,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff031,  /*   4: XPT_RD_RS off */
           0x3ffff04b,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00d,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff069,  /*   7: XPT_RD_PB off */
           0x80fd706e,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438075,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd003,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae906c,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617074,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05d,  /*  16: MOCA_MIPS RR 53000ns */
           0x807c1067,  /*  17: SATA RR 4876.19047619048ns */
           0x807c1068,  /*  18: SATA_1 RR 4876.19047619048ns */
           0x3ffff056,  /*  19: MCIF2_RD off */
           0x3ffff058,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05c,  /*  22: BSP RR 50000ns */
           0x80ad906a,  /*  23: SAGE RR 6820ns */
           0x86449077,  /*  24: FLASH_DMA RR 63000ns */
           0x81617073,  /*  25: HIF_PCIe RR 13880ns */
           0x86449079,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449078,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff057,  /*  29: MCIF_RD off */
           0x3ffff059,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db06f,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5072,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5071,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae106b,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db070,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff00b,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff00c,  /*  42: RAAGA1 off */
           0x3ffff000,  /*  43: RAAGA1_1 off */
           0x3ffff032,  /*  44: AUD_AIO off */
           0x804c407f,  /*  45: VICE_CME_RMB_CMB RR 3000ns */
           0x82633086,  /*  46: VICE_CME_CSC RR 24000ns */
           0x808d3080,  /*  47: VICE_FME_CSC RR 5550ns */
           0x811a9082,  /*  48: VICE_FME_Luma_CMB RR 11100ns */
           0x811a9081,  /*  49: VICE_FME_Chroma_CMB RR 11100ns */
           0x3ffff04a,  /*  50: VICE_SG off */
           0x80000090,  /*  51: VICE_DBLK RR 0ns */
           0x3ffff053,  /*  52: VICE_CABAC0 off */
           0x3ffff05b,  /*  53: VICE_CABAC1 off */
           0x80684085,  /*  54: VICE_ARCSS0 RR 4100ns */
           0x80c7d036,  /*  55: VICE_VIP0_INST0 RR 7406ns */
           0x8257404f,  /*  56: VICE_VIP1_INST0 RR 22200ns */
           0x80c7d037,  /*  57: VICE_VIP0_INST1 RR 7406ns */
           0x82574050,  /*  58: VICE_VIP1_INST1 RR 22200ns */
           0x804c407b,  /*  59: VICE1_CME_RMB_CMB RR 3000ns */
           0x82633084,  /*  60: VICE1_CME_CSC RR 24000ns */
           0x808d307c,  /*  61: VICE1_FME_CSC RR 5550ns */
           0x811a907e,  /*  62: VICE1_FME_Luma_CMB RR 11100ns */
           0x811a907d,  /*  63: VICE1_FME_Chroma_CMB RR 11100ns */
           0x3ffff049,  /*  64: VICE1_SG off */
           0x8000008f,  /*  65: VICE1_DBLK RR 0ns */
           0x3ffff052,  /*  66: VICE1_CABAC0 off */
           0x3ffff05a,  /*  67: VICE1_CABAC1 off */
           0x80684083,  /*  68: VICE1_ARCSS0 RR 4100ns */
           0x80c7d033,  /*  69: VICE1_VIP0_INST0 RR 7406ns */
           0x8257404c,  /*  70: VICE1_VIP1_INST0 RR 22200ns */
           0x80c7d034,  /*  71: VICE1_VIP0_INST1 RR 7406ns */
           0x8257404d,  /*  72: VICE1_VIP1_INST1 RR 22200ns */
           0x3ffff089,  /*  73: HVD0_DBLK_0 off */
           0x3ffff087,  /*  74: HVD0_DBLK_1 off */
           0x3ffff060,  /*  75: HVD0_ILCPU off */
           0x3ffff064,  /*  76: HVD0_OLCPU off */
           0x3ffff01c,  /*  77: HVD0_CAB off */
           0x3ffff021,  /*  78: HVD0_ILSI off */
           0x825d1076,  /*  79: HVD0_ILCPU_p2 RR 22416ns */
           0x3ffff022,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff08b,  /*  81: HVD1_DBLK_0 off */
           0x3ffff08c,  /*  82: HVD1_DBLK_1 off */
           0x3ffff061,  /*  83: HVD1_ILCPU off */
           0x3ffff065,  /*  84: HVD1_OLCPU off */
           0x3ffff01d,  /*  85: HVD1_CAB off */
           0x3ffff01f,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x8007e08d,  /*  88: HVD2_DBLK_0 RR 0ns */
           0x8003e08e,  /*  89: HVD2_DBLK_1 RR 0ns */
           0x3ffff062,  /*  90: HVD2_ILCPU off */
           0x3ffff066,  /*  91: HVD2_OLCPU off */
           0x3ffff01e,  /*  92: HVD2_CAB off */
           0x3ffff020,  /*  93: HVD2_ILSI off */
           0x3ffff028,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff03d,  /*  95: BVN_MAD_QUANT off */
           0x3ffff044,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff024,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff039,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff040,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff025,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff03a,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff041,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff026,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff03b,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff042,  /* 105: BVN_MAD3_PIX_CAP off */
           0x3ffff004,  /* 106: BVN_MFD0 off */
           0x3ffff005,  /* 107: BVN_MFD0_1 off */
           0x3ffff015,  /* 108: BVN_MFD1 off */
           0x3ffff006,  /* 109: BVN_MFD1_1 off */
           0x3ffff016,  /* 110: BVN_MFD2 off */
           0x3ffff007,  /* 111: BVN_MFD2_1 off */
           0x3ffff017,  /* 112: BVN_MFD3 off */
           0x3ffff008,  /* 113: BVN_MFD3_1 off */
           0x0063d018,  /* 114: BVN_MFD4 3703ns */
           0x00428009,  /* 115: BVN_MFD4_1 2469ns */
           0x0063d019,  /* 116: BVN_MFD5 3703ns */
           0x0042800a,  /* 117: BVN_MFD5_1 2469ns */
           0x3ffff045,  /* 118: BVN_VFD0 off */
           0x3ffff046,  /* 119: BVN_VFD1 off */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff029,  /* 122: BVN_VFD4 off */
           0x3ffff02a,  /* 123: BVN_VFD5 off */
           0x3ffff02b,  /* 124: BVN_VFD6 off */
           0x3ffff02c,  /* 125: BVN_VFD7 off */
           0x3ffff03f,  /* 126: BVN_CAP0 off */
           0x3ffff047,  /* 127: BVN_CAP1 off */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff02d,  /* 130: BVN_CAP4 off */
           0x3ffff02e,  /* 131: BVN_CAP5 off */
           0x3ffff02f,  /* 132: BVN_CAP6 off */
           0x3ffff030,  /* 133: BVN_CAP7 off */
           0x3ffff023,  /* 134: BVN_GFD0 off */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x3ffff00f,  /* 136: BVN_GFD2 off */
           0x00559010,  /* 137: BVN_GFD3 3174ns */
           0x00559011,  /* 138: BVN_GFD4 3174ns */
           0x3ffff012,  /* 139: BVN_GFD5 off */
           0x3ffff013,  /* 140: BVN_GFD6 off */
           0x3ffff01b,  /* 141: BVN_MCVP0 off */
           0x3ffff01a,  /* 142: BVN_MCVP1 off */
           0x3ffff048,  /* 143: BVN_MCVP2 off */
           0x3ffff014,  /* 144: BVN_RDC off */
           0x3ffff054,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff055,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x80c7d038,  /* 151: VICE_VIP0_INST2 RR 7406ns */
           0x82574051,  /* 152: VICE_VIP1_INST2 RR 22200ns */
           0x80c7d035,  /* 153: VICE1_VIP0_INST2 RR 7406ns */
           0x8257404e,  /* 154: VICE1_VIP1_INST2 RR 22200ns */
           0x3ffff08a,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff088,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff027,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff03c,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff043,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x8014205e,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000009b,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026105f,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000009c,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff093,  /* 216: HVD0_PFRI off */
           0x3ffff094,  /* 217: HVD1_PFRI off */
           0x80000095,  /* 218: HVD2_PFRI RR 0ns */
           0x80000092,  /* 219: VICE_PFRI RR 0ns */
           0x80000091,  /* 220: VICE1_PFRI RR 0ns */
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
           0xbfffe07a,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2703e   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20150624005233_7445_6T[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80980803}, /* HVD0_PFRI (gHvd0) 259733.33 ns/20 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000057a}, /* d: 4; p: 1402.55 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00002340}, /* 9024 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00001526}, /* 60% * 9024 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80990803}, /* HVD1_PFRI (gHvd1) 265333.33 ns/20 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000598}, /* d: 4; p: 1432.7875 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x00002340}, /* 9024 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x00001526}, /* 60% * 9024 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_CONFIG,      0x809a0802}, /* HVD2_PFRI (gHvd2) 265333.33 ns/20 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_FILTER_CTRL, 0x40000598}, /* d: 4; p: 1432.7875 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_THRESH0,     0x000023f4}, /* 9204 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_THRESH1,     0x00001592}, /* 60% * 9204 */
  {BCHP_MEMC_GEN_2_PFRI_3_THROTTLE_CONFIG,      0x80970802}, /* VICE_PFRI (gVice) 222222.22 ns/80 */
  {BCHP_MEMC_GEN_2_PFRI_3_THROTTLE_FILTER_CTRL, 0x4000012c}, /* d: 4; p: 300 */
  {BCHP_MEMC_GEN_2_PFRI_3_THROTTLE_THRESH0,     0x000001a8}, /* 424 */
  {BCHP_MEMC_GEN_2_PFRI_3_THROTTLE_THRESH1,     0x000000fe}, /* 60% * 424 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_CONFIG,      0x80960803}, /* VICE1_PFRI (gVice1) 222222.22 ns/80 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_FILTER_CTRL, 0x4000012c}, /* d: 4; p: 300 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_THRESH1,     0x000000f9}  /* 60% * 416 */
};

static const uint32_t* const paulMemc_box7[] = { &aulMemc0_20150624005233_7445_6T[0], &aulMemc1_20150624005233_7445_6T[0], &aulMemc2_20150624005233_7445_6T[0]};

const BBOX_Rts stBoxRts_7445_6T_box7 = {
  "20150624005233_7445_6T_box7",
  7445,
  7,
  3,
  256,
  (const uint32_t**)&paulMemc_box7[0],
  20,
  stBoxRts_PfriClient_20150624005233_7445_6T
};
