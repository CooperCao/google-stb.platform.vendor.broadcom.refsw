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
*   at: Wed Jun 24 00:54:26 2015 GMT
*   by: robinc
*   for: Box 7445_3T
*         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 2 (32-bit DDR3@1067MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20150624005426_7445_3T[] = {
           0x004e900b,  /*   0: XPT_WR_RS 2914.72868217054ns */
           0x80709053,  /*   1: XPT_WR_XC RR 4423.52941176471ns */
           0x8089802a,  /*   2: XPT_WR_CDB RR 5098.30508474576ns */
           0x81c94064,  /*   3: XPT_WR_ITB_MSG RR 17958.2089552239ns */
           0x80d59038,  /*   4: XPT_RD_RS RR 7915.78947368421ns */
           0x83f71044,  /*   5: XPT_RD_XC_RMX_MSG RR 37600ns */
           0x80898029,  /*   6: XPT_RD_XC_RAVE RR 5098.30508474576ns */
           0x815ba05e,  /*   7: XPT_RD_PB RR 13653.3333333333ns */
           0x80e4105a,  /*   8: XPT_WR_MEMDMA RR 8960ns */
           0x82098066,  /*   9: XPT_RD_MEMDMA RR 20480ns */
           0x803fd007,  /*  10: SYSPORT_WR RR 2370ns */
           0x8150105d,  /*  11: SYSPORT_RD RR 13200ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617062,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e046,  /*  16: MOCA_MIPS RR 53000ns */
           0x80d08056,  /*  17: SATA RR 8192ns */
           0x80d08057,  /*  18: SATA_1 RR 8192ns */
           0x03e6e040,  /*  19: MCIF2_RD 37000ns */
           0x03e6e042,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e045,  /*  22: BSP RR 50000ns */
           0x80ad9054,  /*  23: SAGE RR 6820ns */
           0x86449067,  /*  24: FLASH_DMA RR 63000ns */
           0x81617061,  /*  25: HIF_PCIe RR 13880ns */
           0x86449069,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449068,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e041,  /*  29: MCIF_RD 37000ns */
           0x03e6e043,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db05b,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5060,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c505f,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1055,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db05c,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x0050e012,  /*  40: RAAGA 3000ns */
           0x001ae001,  /*  41: RAAGA_1 1000ns */
           0x00a1e02b,  /*  42: RAAGA1 6000ns */
           0x001ae000,  /*  43: RAAGA1_1 1000ns */
           0x00bb402c,  /*  44: AUD_AIO 6940ns */
           0x8032d04d,  /*  45: VICE_CME_RMB_CMB RR 2000ns */
           0x81977063,  /*  46: VICE_CME_CSC RR 16000ns */
           0x805d6050,  /*  47: VICE_FME_CSC RR 3670ns */
           0x805d6052,  /*  48: VICE_FME_Luma_CMB RR 3670ns */
           0x805d6051,  /*  49: VICE_FME_Chroma_CMB RR 3670ns */
           0x80dca039,  /*  50: VICE_SG RR 8176.66666666667ns */
           0x8000007e,  /*  51: VICE_DBLK RR 0ns */
           0x81edf03b,  /*  52: VICE_CABAC0 RR 18300ns */
           0x8378203e,  /*  53: VICE_CABAC1 RR 32900ns */
           0x804d904f,  /*  54: VICE_ARCSS0 RR 3050ns */
           0x8063d01d,  /*  55: VICE_VIP0_INST0 RR 3703ns */
           0x80c7a033,  /*  56: VICE_VIP1_INST0 RR 7400ns */
           0x8063d01e,  /*  57: VICE_VIP0_INST1 RR 3703ns */
           0x80c7a034,  /*  58: VICE_VIP1_INST1 RR 7400ns */
           0x3ffff06b,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff070,  /*  60: VICE1_CME_CSC off */
           0x3ffff06c,  /*  61: VICE1_FME_CSC off */
           0x3ffff06e,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff06d,  /*  63: VICE1_FME_Chroma_CMB off */
           0x81b8d03a,  /*  64: VICE1_SG RR 16333.3333333333ns */
           0x3ffff07d,  /*  65: VICE1_DBLK off */
           0x83dc103f,  /*  66: VICE1_CABAC0 RR 36600ns */
           0x86f07047,  /*  67: VICE1_CABAC1 RR 65800ns */
           0x3ffff06f,  /*  68: VICE1_ARCSS0 off */
           0x3ffff01a,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff030,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff01b,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff031,  /*  72: VICE1_VIP1_INST1 off */
           0x800bb073,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x8005c071,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x801c2049,  /*  75: HVD0_ILCPU RR 1048ns */
           0x804a704e,  /*  76: HVD0_OLCPU RR 2927ns */
           0x00596014,  /*  77: HVD0_CAB 3317ns */
           0x00710020,  /*  78: HVD0_ILSI 4191ns */
           0x81ea3065,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x00710021,  /*  80: HVD0_ILSI_p2 4191ns */
           0x3ffff079,  /*  81: HVD1_DBLK_0 off */
           0x3ffff07a,  /*  82: HVD1_DBLK_1 off */
           0x8027004b,  /*  83: HVD1_ILCPU RR 1451ns */
           0x80d9b058,  /*  84: HVD1_OLCPU RR 8553ns */
           0x007de024,  /*  85: HVD1_CAB 4667ns */
           0x0071a022,  /*  86: HVD1_ILSI 4214ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff07b,  /*  88: HVD2_DBLK_0 off */
           0x3ffff07c,  /*  89: HVD2_DBLK_1 off */
           0x8027004c,  /*  90: HVD2_ILCPU RR 1451ns */
           0x80d9b059,  /*  91: HVD2_OLCPU RR 8553ns */
           0x007de025,  /*  92: HVD2_CAB 4667ns */
           0x0071a023,  /*  93: HVD2_ILSI 4214ns */
           0x3ffff0ff,  /*  94: UNASSIGNED off */
           0x3ffff0ff,  /*  95: UNASSIGNED off */
           0x3ffff0ff,  /*  96: UNASSIGNED off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff015,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff026,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff02d,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff016,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff027,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff02e,  /* 105: BVN_MAD3_PIX_CAP off */
           0x00c7d036,  /* 106: BVN_MFD0_Ch 7407ns */
           0x0038f006,  /* 107: BVN_MFD0_Ch_1 2115ns */
           0x3ffff018,  /* 108: BVN_MFD1 off */
           0x3ffff009,  /* 109: BVN_MFD1_1 off */
           0x3ffff019,  /* 110: BVN_MFD2 off */
           0x3ffff00a,  /* 111: BVN_MFD2_1 off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff0ff,  /* 118: UNASSIGNED off */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff00c,  /* 123: BVN_VFD5 off */
           0x3ffff00d,  /* 124: BVN_VFD6 off */
           0x3ffff00e,  /* 125: BVN_VFD7 off */
           0x3ffff0ff,  /* 126: UNASSIGNED off */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff00f,  /* 131: BVN_CAP5 off */
           0x3ffff010,  /* 132: BVN_CAP6 off */
           0x3ffff011,  /* 133: BVN_CAP7 off */
           0x3ffff0ff,  /* 134: UNASSIGNED off */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff003,  /* 138: BVN_GFD4 off */
           0x3ffff004,  /* 139: BVN_GFD5 off */
           0x3ffff005,  /* 140: BVN_GFD6 off */
           0x3ffff0ff,  /* 141: UNASSIGNED off */
           0x3ffff0ff,  /* 142: UNASSIGNED off */
           0x3ffff0ff,  /* 143: UNASSIGNED off */
           0x3ffff013,  /* 144: BVN_RDC off */
           0x0352603c,  /* 145: VEC_VBI_ENC0 31500ns */
           0x0352603d,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x8063d01f,  /* 151: VICE_VIP0_INST2 RR 3703ns */
           0x80c7a035,  /* 152: VICE_VIP1_INST2 RR 7400ns */
           0x3ffff01c,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff032,  /* 154: VICE1_VIP1_INST2 off */
           0x800bb074,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x8005c072,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
           0x3ffff017,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff028,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff02f,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x80142048,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000008b,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104a,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008c,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000081,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x3ffff083,  /* 217: HVD1_PFRI off */
           0x3ffff084,  /* 218: HVD2_PFRI off */
           0x8000007f,  /* 219: VICE_PFRI RR 0ns */
           0x3ffff082,  /* 220: VICE1_PFRI off */
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
           0xbfffe06a,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27037   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20150624005426_7445_3T[] = {
           0x3ffff00b,  /*   0: XPT_WR_RS off */
           0x3ffff053,  /*   1: XPT_WR_XC off */
           0x3ffff02a,  /*   2: XPT_WR_CDB off */
           0x3ffff064,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff038,  /*   4: XPT_RD_RS off */
           0x3ffff044,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff029,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff05e,  /*   7: XPT_RD_PB off */
           0x80e4105a,  /*   8: XPT_WR_MEMDMA RR 8960ns */
           0x82098066,  /*   9: XPT_RD_MEMDMA RR 20480ns */
           0x803fd007,  /*  10: SYSPORT_WR RR 2370ns */
           0x8150105d,  /*  11: SYSPORT_RD RR 13200ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617062,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e046,  /*  16: MOCA_MIPS RR 53000ns */
           0x80d08056,  /*  17: SATA RR 8192ns */
           0x80d08057,  /*  18: SATA_1 RR 8192ns */
           0x3ffff040,  /*  19: MCIF2_RD off */
           0x3ffff042,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e045,  /*  22: BSP RR 50000ns */
           0x80ad9054,  /*  23: SAGE RR 6820ns */
           0x86449067,  /*  24: FLASH_DMA RR 63000ns */
           0x81617061,  /*  25: HIF_PCIe RR 13880ns */
           0x86449069,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449068,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff041,  /*  29: MCIF_RD off */
           0x3ffff043,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db05b,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5060,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c505f,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1055,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db05c,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff012,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff02b,  /*  42: RAAGA1 off */
           0x3ffff000,  /*  43: RAAGA1_1 off */
           0x3ffff02c,  /*  44: AUD_AIO off */
           0x3ffff04d,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff063,  /*  46: VICE_CME_CSC off */
           0x3ffff050,  /*  47: VICE_FME_CSC off */
           0x3ffff052,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff051,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff039,  /*  50: VICE_SG off */
           0x3ffff07e,  /*  51: VICE_DBLK off */
           0x3ffff03b,  /*  52: VICE_CABAC0 off */
           0x3ffff03e,  /*  53: VICE_CABAC1 off */
           0x3ffff04f,  /*  54: VICE_ARCSS0 off */
           0x3ffff01d,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff033,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff01e,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff034,  /*  58: VICE_VIP1_INST1 off */
           0x8065c06b,  /*  59: VICE1_CME_RMB_CMB RR 4000ns */
           0x832ef070,  /*  60: VICE1_CME_CSC RR 32000ns */
           0x80bb106c,  /*  61: VICE1_FME_CSC RR 7350ns */
           0x80bb106e,  /*  62: VICE1_FME_Luma_CMB RR 7350ns */
           0x80bb106d,  /*  63: VICE1_FME_Chroma_CMB RR 7350ns */
           0x3ffff03a,  /*  64: VICE1_SG off */
           0x8000007d,  /*  65: VICE1_DBLK RR 0ns */
           0x3ffff03f,  /*  66: VICE1_CABAC0 off */
           0x3ffff047,  /*  67: VICE1_CABAC1 off */
           0x80a2e06f,  /*  68: VICE1_ARCSS0 RR 6400ns */
           0x8063d01a,  /*  69: VICE1_VIP0_INST0 RR 3703ns */
           0x80c7a030,  /*  70: VICE1_VIP1_INST0 RR 7400ns */
           0x8063d01b,  /*  71: VICE1_VIP0_INST1 RR 3703ns */
           0x80c7a031,  /*  72: VICE1_VIP1_INST1 RR 7400ns */
           0x3ffff075,  /*  73: HVD0_DBLK_0 off */
           0x3ffff076,  /*  74: HVD0_DBLK_1 off */
           0x3ffff049,  /*  75: HVD0_ILCPU off */
           0x3ffff04e,  /*  76: HVD0_OLCPU off */
           0x3ffff014,  /*  77: HVD0_CAB off */
           0x3ffff020,  /*  78: HVD0_ILSI off */
           0x81ea3065,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff021,  /*  80: HVD0_ILSI_p2 off */
           0x80103079,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8008007a,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff04b,  /*  83: HVD1_ILCPU off */
           0x3ffff058,  /*  84: HVD1_OLCPU off */
           0x3ffff024,  /*  85: HVD1_CAB off */
           0x3ffff022,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x8010307b,  /*  88: HVD2_DBLK_0 RR 0ns */
           0x8008007c,  /*  89: HVD2_DBLK_1 RR 0ns */
           0x3ffff04c,  /*  90: HVD2_ILCPU off */
           0x3ffff059,  /*  91: HVD2_OLCPU off */
           0x3ffff025,  /*  92: HVD2_CAB off */
           0x3ffff023,  /*  93: HVD2_ILSI off */
           0x3ffff0ff,  /*  94: UNASSIGNED off */
           0x3ffff0ff,  /*  95: UNASSIGNED off */
           0x3ffff0ff,  /*  96: UNASSIGNED off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x005ea015,  /* 100: BVN_MAD2_PIX_FD 3511ns */
           0x00853026,  /* 101: BVN_MAD2_QUANT 4938ns */
           0x00bd702d,  /* 102: BVN_MAD2_PIX_CAP 7023ns */
           0x005ea016,  /* 103: BVN_MAD3_PIX_FD 3511ns */
           0x00853027,  /* 104: BVN_MAD3_QUANT 4938ns */
           0x00bd702e,  /* 105: BVN_MAD3_PIX_CAP 7023ns */
           0x3ffff008,  /* 106: BVN_MFD0 off */
           0x3ffff002,  /* 107: BVN_MFD0_1 off */
           0x0063d018,  /* 108: BVN_MFD1 3703ns */
           0x00428009,  /* 109: BVN_MFD1_1 2469ns */
           0x0063d019,  /* 110: BVN_MFD2 3703ns */
           0x0042800a,  /* 111: BVN_MFD2_1 2469ns */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff0ff,  /* 118: UNASSIGNED off */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x004fc00c,  /* 123: BVN_VFD5 2960ns */
           0x004fc00d,  /* 124: BVN_VFD6 2960ns */
           0x004fc00e,  /* 125: BVN_VFD7 2960ns */
           0x3ffff0ff,  /* 126: UNASSIGNED off */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x004fc00f,  /* 131: BVN_CAP5 2960ns */
           0x004fc010,  /* 132: BVN_CAP6 2960ns */
           0x004fc011,  /* 133: BVN_CAP7 2960ns */
           0x3ffff0ff,  /* 134: UNASSIGNED off */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x0027d003,  /* 138: BVN_GFD4 1481ns */
           0x3ffff004,  /* 139: BVN_GFD5 off */
           0x3ffff005,  /* 140: BVN_GFD6 off */
           0x3ffff0ff,  /* 141: UNASSIGNED off */
           0x3ffff0ff,  /* 142: UNASSIGNED off */
           0x3ffff0ff,  /* 143: UNASSIGNED off */
           0x3ffff013,  /* 144: BVN_RDC off */
           0x3ffff03c,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff03d,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff01f,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff035,  /* 152: VICE_VIP1_INST2 off */
           0x8063d01c,  /* 153: VICE1_VIP0_INST2 RR 3703ns */
           0x80c7a032,  /* 154: VICE1_VIP1_INST2 RR 7400ns */
           0x3ffff077,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff078,  /* 156: HVD0_DBLK_p2_1 off */
           0x005ea017,  /* 157: BVN_MAD4_PIX_FD 3511ns */
           0x00853028,  /* 158: BVN_MAD4_QUANT 4938ns */
           0x00bd702f,  /* 159: BVN_MAD4_PIX_CAP 7023ns */
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
           0x80142048,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000008b,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104a,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008c,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff080,  /* 216: HVD0_PFRI off */
           0x80000083,  /* 217: HVD1_PFRI RR 0ns */
           0x80000084,  /* 218: HVD2_PFRI RR 0ns */
           0x3ffff07f,  /* 219: VICE_PFRI off */
           0x80000082,  /* 220: VICE1_PFRI RR 0ns */
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
           0xbfffe06a,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27037   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc2_20150624005426_7445_3T[] = {
           0x3ffff00b,  /*   0: XPT_WR_RS off */
           0x3ffff053,  /*   1: XPT_WR_XC off */
           0x3ffff02a,  /*   2: XPT_WR_CDB off */
           0x3ffff064,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff038,  /*   4: XPT_RD_RS off */
           0x3ffff044,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff029,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff05e,  /*   7: XPT_RD_PB off */
           0x80e4105a,  /*   8: XPT_WR_MEMDMA RR 8960ns */
           0x82098066,  /*   9: XPT_RD_MEMDMA RR 20480ns */
           0x803fd007,  /*  10: SYSPORT_WR RR 2370ns */
           0x8150105d,  /*  11: SYSPORT_RD RR 13200ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617062,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e046,  /*  16: MOCA_MIPS RR 53000ns */
           0x80d08056,  /*  17: SATA RR 8192ns */
           0x80d08057,  /*  18: SATA_1 RR 8192ns */
           0x3ffff040,  /*  19: MCIF2_RD off */
           0x3ffff042,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e045,  /*  22: BSP RR 50000ns */
           0x80ad9054,  /*  23: SAGE RR 6820ns */
           0x86449067,  /*  24: FLASH_DMA RR 63000ns */
           0x81617061,  /*  25: HIF_PCIe RR 13880ns */
           0x86449069,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449068,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff041,  /*  29: MCIF_RD off */
           0x3ffff043,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db05b,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5060,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c505f,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1055,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db05c,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff012,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff02b,  /*  42: RAAGA1 off */
           0x3ffff000,  /*  43: RAAGA1_1 off */
           0x3ffff02c,  /*  44: AUD_AIO off */
           0x3ffff04d,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff063,  /*  46: VICE_CME_CSC off */
           0x3ffff050,  /*  47: VICE_FME_CSC off */
           0x3ffff052,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff051,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff039,  /*  50: VICE_SG off */
           0x3ffff07e,  /*  51: VICE_DBLK off */
           0x3ffff03b,  /*  52: VICE_CABAC0 off */
           0x3ffff03e,  /*  53: VICE_CABAC1 off */
           0x3ffff04f,  /*  54: VICE_ARCSS0 off */
           0x3ffff01d,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff033,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff01e,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff034,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff06b,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff070,  /*  60: VICE1_CME_CSC off */
           0x3ffff06c,  /*  61: VICE1_FME_CSC off */
           0x3ffff06e,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff06d,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff03a,  /*  64: VICE1_SG off */
           0x3ffff07d,  /*  65: VICE1_DBLK off */
           0x3ffff03f,  /*  66: VICE1_CABAC0 off */
           0x3ffff047,  /*  67: VICE1_CABAC1 off */
           0x3ffff06f,  /*  68: VICE1_ARCSS0 off */
           0x3ffff01a,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff030,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff01b,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff031,  /*  72: VICE1_VIP1_INST1 off */
           0x800bb075,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8005c076,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff049,  /*  75: HVD0_ILCPU off */
           0x3ffff04e,  /*  76: HVD0_OLCPU off */
           0x3ffff014,  /*  77: HVD0_CAB off */
           0x3ffff020,  /*  78: HVD0_ILSI off */
           0x81ea3065,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff021,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff079,  /*  81: HVD1_DBLK_0 off */
           0x3ffff07a,  /*  82: HVD1_DBLK_1 off */
           0x3ffff04b,  /*  83: HVD1_ILCPU off */
           0x3ffff058,  /*  84: HVD1_OLCPU off */
           0x3ffff024,  /*  85: HVD1_CAB off */
           0x3ffff022,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff07b,  /*  88: HVD2_DBLK_0 off */
           0x3ffff07c,  /*  89: HVD2_DBLK_1 off */
           0x3ffff04c,  /*  90: HVD2_ILCPU off */
           0x3ffff059,  /*  91: HVD2_OLCPU off */
           0x3ffff025,  /*  92: HVD2_CAB off */
           0x3ffff023,  /*  93: HVD2_ILSI off */
           0x3ffff0ff,  /*  94: UNASSIGNED off */
           0x3ffff0ff,  /*  95: UNASSIGNED off */
           0x3ffff0ff,  /*  96: UNASSIGNED off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff015,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff026,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff02d,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff016,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff027,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff02e,  /* 105: BVN_MAD3_PIX_CAP off */
           0x00428008,  /* 106: BVN_MFD0 2469ns */
           0x001c6002,  /* 107: BVN_MFD0_1 1057.5ns */
           0x3ffff018,  /* 108: BVN_MFD1 off */
           0x3ffff009,  /* 109: BVN_MFD1_1 off */
           0x3ffff019,  /* 110: BVN_MFD2 off */
           0x3ffff00a,  /* 111: BVN_MFD2_1 off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff0ff,  /* 118: UNASSIGNED off */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff00c,  /* 123: BVN_VFD5 off */
           0x3ffff00d,  /* 124: BVN_VFD6 off */
           0x3ffff00e,  /* 125: BVN_VFD7 off */
           0x3ffff0ff,  /* 126: UNASSIGNED off */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff00f,  /* 131: BVN_CAP5 off */
           0x3ffff010,  /* 132: BVN_CAP6 off */
           0x3ffff011,  /* 133: BVN_CAP7 off */
           0x3ffff0ff,  /* 134: UNASSIGNED off */
           0x3ffff0ff,  /* 135: UNASSIGNED off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff003,  /* 138: BVN_GFD4 off */
           0x0027d004,  /* 139: BVN_GFD5 1481ns */
           0x0027d005,  /* 140: BVN_GFD6 1481ns */
           0x3ffff0ff,  /* 141: UNASSIGNED off */
           0x3ffff0ff,  /* 142: UNASSIGNED off */
           0x3ffff0ff,  /* 143: UNASSIGNED off */
           0x00571013,  /* 144: BVN_RDC 3230ns */
           0x3ffff03c,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff03d,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff01f,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff035,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff01c,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff032,  /* 154: VICE1_VIP1_INST2 off */
           0x800bb077,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8005c078,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff017,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff028,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff02f,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x80142048,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000008b,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104a,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008c,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000080,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff083,  /* 217: HVD1_PFRI off */
           0x3ffff084,  /* 218: HVD2_PFRI off */
           0x3ffff07f,  /* 219: VICE_PFRI off */
           0x3ffff082,  /* 220: VICE1_PFRI off */
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
           0xbfffe06a,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d27037   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20150624005426_7445_3T[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80870802}, /* HVD0_PFRI_Ch (gHvdC0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00000d40}, /* 3392 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x000007f3}, /* 60% * 3392 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_CONFIG,      0x80850803}, /* VICE_PFRI (gVice) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80890802}, /* HVD1_PFRI (gHvd1) 532224.00 ns/20 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000b3a}, /* d: 4; p: 2874 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x000023f4}, /* 9204 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x00001592}, /* 60% * 9204 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_CONFIG,      0x808a0802}, /* HVD2_PFRI (gHvd2) 532224.00 ns/20 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_FILTER_CTRL, 0x40000b3a}, /* d: 4; p: 2874 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH0,     0x000023f4}, /* 9204 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH1,     0x00001592}, /* 60% * 9204 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_CONFIG,      0x80880803}, /* VICE1_PFRI (gVice1) 441176.47 ns/120 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_FILTER_CTRL, 0x4000018d}, /* d: 4; p: 397.058333333333 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_CONFIG,      0x80860803}, /* HVD0_PFRI (gHvd0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH0,     0x00001642}, /* 5698 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH1,     0x00000d5a}  /* 60% * 5698 */
};

static const uint32_t* const paulMemc_box8[] = { &aulMemc0_20150624005426_7445_3T[0], &aulMemc1_20150624005426_7445_3T[0], &aulMemc2_20150624005426_7445_3T[0]};

const BBOX_Rts stBoxRts_7445_3T_box8 = {
  "20150624005426_7445_3T_box8",
  7445,
  8,
  3,
  256,
  (const uint32_t**)&paulMemc_box8[0],
  24,
  stBoxRts_PfriClient_20150624005426_7445_3T
};
