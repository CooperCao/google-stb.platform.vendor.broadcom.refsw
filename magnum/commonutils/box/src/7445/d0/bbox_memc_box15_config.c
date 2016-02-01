/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/
/*******************************************************************
*               Do Not Edit Directly
* Auto-Generated from RTS environment:
*   at: Wed Nov 25 19:03:23 2015 GMT
*   by: robinc
*   for: Box 7445_1u4t
*         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 2 (32-bit DDR3@1067MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20151125190323_7445_1u4t[] = {
           0x001e7004,  /*   0: XPT_WR_RS 1134.2383107089ns */
           0x803a1052,  /*   1: XPT_WR_XC RR 2285.71428571429ns */
           0x8049000b,  /*   2: XPT_WR_CDB RR 2709.90990990991ns */
           0x80b9205b,  /*   3: XPT_WR_ITB_MSG RR 7274.48609431681ns */
           0x80780024,  /*   4: XPT_RD_RS RR 4449.70414201183ns */
           0x819e303e,  /*   5: XPT_RD_XC_RMX_MSG RR 15346.9387755102ns */
           0x8049000a,  /*   6: XPT_RD_XC_RAVE RR 2709.90990990991ns */
           0x80a2e056,  /*   7: XPT_RD_PB RR 6400ns */
           0x80fd7061,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438069,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd009,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae905a,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617067,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e04b,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662054,  /*  17: SATA RR 4015.6862745098ns */
           0x80662055,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x03e6e046,  /*  19: MCIF2_RD 37000ns */
           0x03e6e048,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e04a,  /*  22: BSP RR 50000ns */
           0x80ad9058,  /*  23: SAGE RR 6820ns */
           0x8644906b,  /*  24: FLASH_DMA RR 63000ns */
           0x81617066,  /*  25: HIF_PCIe RR 13880ns */
           0x8644906d,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644906c,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e047,  /*  29: MCIF_RD 37000ns */
           0x03e6e049,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db062,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5065,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5064,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1059,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db063,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x0050e00e,  /*  40: RAAGA 3000ns */
           0x001ae002,  /*  41: RAAGA_1 1000ns */
           0x00a1e02e,  /*  42: RAAGA1 6000ns */
           0x001ae001,  /*  43: RAAGA1_1 1000ns */
           0x00bb402f,  /*  44: AUD_AIO 6940ns */
           0x8032d051,  /*  45: VICE_CME_RMB_CMB RR 2000ns */
           0x81977068,  /*  46: VICE_CME_CSC RR 16000ns */
           0x805d6053,  /*  47: VICE_FME_CSC RR 3670ns */
           0x80bb105d,  /*  48: VICE_FME_Luma_CMB RR 7350ns */
           0x80bb105c,  /*  49: VICE_FME_Chroma_CMB RR 7350ns */
           0x80dca03b,  /*  50: VICE_SG RR 8176.66666666667ns */
           0x8000007e,  /*  51: VICE_DBLK RR 0ns */
           0x82317040,  /*  52: VICE_CABAC0 RR 20800ns */
           0x83e17045,  /*  53: VICE_CABAC1 RR 36800ns */
           0x80a2e057,  /*  54: VICE_ARCSS0 RR 6400ns */
           0x8063d01c,  /*  55: VICE_VIP0_INST0 RR 3703ns */
           0x80c7a036,  /*  56: VICE_VIP1_INST0 RR 7400ns */
           0x8063d01d,  /*  57: VICE_VIP0_INST1 RR 3703ns */
           0x80c7a037,  /*  58: VICE_VIP1_INST1 RR 7400ns */
           0x3ffff06f,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff074,  /*  60: VICE1_CME_CSC off */
           0x3ffff070,  /*  61: VICE1_FME_CSC off */
           0x3ffff072,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff071,  /*  63: VICE1_FME_Chroma_CMB off */
           0x80dca03a,  /*  64: VICE1_SG RR 8176.66666666667ns */
           0x3ffff07d,  /*  65: VICE1_DBLK off */
           0x8231703f,  /*  66: VICE1_CABAC0 RR 20800ns */
           0x83e17044,  /*  67: VICE1_CABAC1 RR 36800ns */
           0x3ffff073,  /*  68: VICE1_ARCSS0 off */
           0x3ffff019,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff033,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff01a,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff034,  /*  72: VICE1_VIP1_INST1 off */
           0x3ffff075,  /*  73: HVD0_DBLK_0 off */
           0x3ffff076,  /*  74: HVD0_DBLK_1 off */
           0x8027004e,  /*  75: HVD0_ILCPU RR 1451ns */
           0x80d9b05e,  /*  76: HVD0_OLCPU RR 8553ns */
           0x007de025,  /*  77: HVD0_CAB 4667ns */
           0x0071a020,  /*  78: HVD0_ILSI 4214ns */
           0x82c9006a,  /*  79: HVD0_ILCPU_p2 RR 26413ns */
           0x0073f023,  /*  80: HVD0_ILSI_p2 4299ns */
           0x3ffff079,  /*  81: HVD1_DBLK_0 off */
           0x3ffff07a,  /*  82: HVD1_DBLK_1 off */
           0x8027004f,  /*  83: HVD1_ILCPU RR 1451ns */
           0x80d9b05f,  /*  84: HVD1_OLCPU RR 8553ns */
           0x007de026,  /*  85: HVD1_CAB 4667ns */
           0x0071a021,  /*  86: HVD1_ILSI 4214ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff07b,  /*  88: HVD2_DBLK_0 off */
           0x3ffff07c,  /*  89: HVD2_DBLK_1 off */
           0x80270050,  /*  90: HVD2_ILCPU RR 1451ns */
           0x80d9b060,  /*  91: HVD2_OLCPU RR 8553ns */
           0x007de027,  /*  92: HVD2_CAB 4667ns */
           0x0071a022,  /*  93: HVD2_ILSI 4214ns */
           0x3ffff018,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff02d,  /*  95: BVN_MAD_QUANT off */
           0x3ffff032,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff016,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff02b,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff030,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x3ffff010,  /* 106: BVN_MFD0 off */
           0x3ffff000,  /* 107: BVN_MFD0_1 off */
           0x3ffff028,  /* 108: BVN_MFD1 off */
           0x3ffff011,  /* 109: BVN_MFD1_1 off */
           0x3ffff029,  /* 110: BVN_MFD2 off */
           0x3ffff012,  /* 111: BVN_MFD2_1 off */
           0x3ffff02a,  /* 112: BVN_MFD3 off */
           0x3ffff013,  /* 113: BVN_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff005,  /* 118: BVN_VFD0 off */
           0x3ffff006,  /* 119: BVN_VFD1 off */
           0x3ffff043,  /* 120: BVN_VFD2 off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff00c,  /* 125: BVN_VFD7 off */
           0x3ffff003,  /* 126: BVN_CAP0 off */
           0x3ffff007,  /* 127: BVN_CAP1 off */
           0x3ffff03d,  /* 128: BVN_CAP2 off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff00d,  /* 133: BVN_CAP7 off */
           0x3ffff008,  /* 134: BVN_GFD0 off */
           0x3ffff03c,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff015,  /* 141: BVN_MCVP0 off */
           0x3ffff014,  /* 142: BVN_MCVP1 off */
           0x3ffff039,  /* 143: BVN_MCVP2 off */
           0x3ffff00f,  /* 144: BVN_RDC off */
           0x03526041,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526042,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x8063d01e,  /* 151: VICE_VIP0_INST2 RR 3703ns */
           0x80c7a038,  /* 152: VICE_VIP1_INST2 RR 7400ns */
           0x3ffff01b,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff035,  /* 154: VICE1_VIP1_INST2 off */
           0x3ffff077,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff078,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff017,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff02c,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff031,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x8014204c,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x80000089,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104d,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008a,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff081,  /* 216: HVD0_PFRI off */
           0x3ffff082,  /* 217: HVD1_PFRI off */
           0x3ffff083,  /* 218: HVD2_PFRI off */
           0x80000080,  /* 219: VICE_PFRI RR 0ns */
           0x3ffff07f,  /* 220: VICE1_PFRI off */
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
           0xbfffe06e,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x0069201f   /* 255: REFRESH 3900ns */
         };
static const uint32_t aulMemc1_20151125190323_7445_1u4t[] = {
           0x3ffff004,  /*   0: XPT_WR_RS off */
           0x3ffff052,  /*   1: XPT_WR_XC off */
           0x3ffff00b,  /*   2: XPT_WR_CDB off */
           0x3ffff05b,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff024,  /*   4: XPT_RD_RS off */
           0x3ffff03e,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff056,  /*   7: XPT_RD_PB off */
           0x80fd7061,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438069,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd009,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae905a,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617067,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e04b,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662054,  /*  17: SATA RR 4015.6862745098ns */
           0x80662055,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x3ffff046,  /*  19: MCIF2_RD off */
           0x3ffff048,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e04a,  /*  22: BSP RR 50000ns */
           0x80ad9058,  /*  23: SAGE RR 6820ns */
           0x8644906b,  /*  24: FLASH_DMA RR 63000ns */
           0x81617066,  /*  25: HIF_PCIe RR 13880ns */
           0x8644906d,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644906c,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff047,  /*  29: MCIF_RD off */
           0x3ffff049,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db062,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5065,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5064,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1059,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db063,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff00e,  /*  40: RAAGA off */
           0x3ffff002,  /*  41: RAAGA_1 off */
           0x3ffff02e,  /*  42: RAAGA1 off */
           0x3ffff001,  /*  43: RAAGA1_1 off */
           0x3ffff02f,  /*  44: AUD_AIO off */
           0x3ffff051,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff068,  /*  46: VICE_CME_CSC off */
           0x3ffff053,  /*  47: VICE_FME_CSC off */
           0x3ffff05d,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff05c,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff03b,  /*  50: VICE_SG off */
           0x3ffff07e,  /*  51: VICE_DBLK off */
           0x3ffff040,  /*  52: VICE_CABAC0 off */
           0x3ffff045,  /*  53: VICE_CABAC1 off */
           0x3ffff057,  /*  54: VICE_ARCSS0 off */
           0x3ffff01c,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff036,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff01d,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff037,  /*  58: VICE_VIP1_INST1 off */
           0x8032d06f,  /*  59: VICE1_CME_RMB_CMB RR 2000ns */
           0x81977074,  /*  60: VICE1_CME_CSC RR 16000ns */
           0x805d6070,  /*  61: VICE1_FME_CSC RR 3670ns */
           0x80bb1072,  /*  62: VICE1_FME_Luma_CMB RR 7350ns */
           0x80bb1071,  /*  63: VICE1_FME_Chroma_CMB RR 7350ns */
           0x3ffff03a,  /*  64: VICE1_SG off */
           0x8000007d,  /*  65: VICE1_DBLK RR 0ns */
           0x3ffff03f,  /*  66: VICE1_CABAC0 off */
           0x3ffff044,  /*  67: VICE1_CABAC1 off */
           0x80a2e073,  /*  68: VICE1_ARCSS0 RR 6400ns */
           0x8063d019,  /*  69: VICE1_VIP0_INST0 RR 3703ns */
           0x80c7a033,  /*  70: VICE1_VIP1_INST0 RR 7400ns */
           0x8063d01a,  /*  71: VICE1_VIP0_INST1 RR 3703ns */
           0x80c7a034,  /*  72: VICE1_VIP1_INST1 RR 7400ns */
           0x80140075,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8009f076,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff04e,  /*  75: HVD0_ILCPU off */
           0x3ffff05e,  /*  76: HVD0_OLCPU off */
           0x3ffff025,  /*  77: HVD0_CAB off */
           0x3ffff020,  /*  78: HVD0_ILSI off */
           0x82c9006a,  /*  79: HVD0_ILCPU_p2 RR 26413ns */
           0x3ffff023,  /*  80: HVD0_ILSI_p2 off */
           0x80140079,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8009f07a,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff04f,  /*  83: HVD1_ILCPU off */
           0x3ffff05f,  /*  84: HVD1_OLCPU off */
           0x3ffff026,  /*  85: HVD1_CAB off */
           0x3ffff021,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x8014007b,  /*  88: HVD2_DBLK_0 RR 0ns */
           0x8009f07c,  /*  89: HVD2_DBLK_1 RR 0ns */
           0x3ffff050,  /*  90: HVD2_ILCPU off */
           0x3ffff060,  /*  91: HVD2_OLCPU off */
           0x3ffff027,  /*  92: HVD2_CAB off */
           0x3ffff022,  /*  93: HVD2_ILSI off */
           0x005ea018,  /*  94: BVN_MAD_PIX_FD 3511ns */
           0x0085302d,  /*  95: BVN_MAD_QUANT 4938ns */
           0x00bd7032,  /*  96: BVN_MAD_PIX_CAP 7023ns */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x005ea016,  /* 100: BVN_MAD2_PIX_FD 3511ns */
           0x0085302b,  /* 101: BVN_MAD2_QUANT 4938ns */
           0x00bd7030,  /* 102: BVN_MAD2_PIX_CAP 7023ns */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0058c010,  /* 106: BVN_MFD0 3292ns */
           0x001a8000,  /* 107: BVN_MFD0_1 987ns */
           0x00853028,  /* 108: BVN_MFD1 4938ns */
           0x0058c011,  /* 109: BVN_MFD1_1 3292ns */
           0x00853029,  /* 110: BVN_MFD2 4938ns */
           0x0058c012,  /* 111: BVN_MFD2_1 3292ns */
           0x0085302a,  /* 112: BVN_MFD3 4938ns */
           0x0058c013,  /* 113: BVN_MFD3_1 3292ns */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff005,  /* 118: BVN_VFD0 off */
           0x3ffff006,  /* 119: BVN_VFD1 off */
           0x3ffff043,  /* 120: BVN_VFD2 off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff00c,  /* 125: BVN_VFD7 off */
           0x3ffff003,  /* 126: BVN_CAP0 off */
           0x3ffff007,  /* 127: BVN_CAP1 off */
           0x3ffff03d,  /* 128: BVN_CAP2 off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff00d,  /* 133: BVN_CAP7 off */
           0x3ffff008,  /* 134: BVN_GFD0 off */
           0x3ffff03c,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x005c1015,  /* 141: BVN_MCVP0 3415ns */
           0x005c1014,  /* 142: BVN_MCVP1 3415ns */
           0x00cf2039,  /* 143: BVN_MCVP2 7676ns */
           0x3ffff00f,  /* 144: BVN_RDC off */
           0x3ffff041,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff042,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff01e,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff038,  /* 152: VICE_VIP1_INST2 off */
           0x8063d01b,  /* 153: VICE1_VIP0_INST2 RR 3703ns */
           0x80c7a035,  /* 154: VICE1_VIP1_INST2 RR 7400ns */
           0x80140077,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8009f078,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x005ea017,  /* 157: BVN_MAD4_PIX_FD 3511ns */
           0x0085302c,  /* 158: BVN_MAD4_QUANT 4938ns */
           0x00bd7031,  /* 159: BVN_MAD4_PIX_CAP 7023ns */
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
           0x8014204c,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x80000089,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104d,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008a,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000081,  /* 216: HVD0_PFRI RR 0ns */
           0x80000082,  /* 217: HVD1_PFRI RR 0ns */
           0x80000083,  /* 218: HVD2_PFRI RR 0ns */
           0x3ffff080,  /* 219: VICE_PFRI off */
           0x8000007f,  /* 220: VICE1_PFRI RR 0ns */
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
           0xbfffe06e,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x0069201f   /* 255: REFRESH 3900ns */
         };
static const uint32_t aulMemc2_20151125190323_7445_1u4t[] = {
           0x3ffff004,  /*   0: XPT_WR_RS off */
           0x3ffff052,  /*   1: XPT_WR_XC off */
           0x3ffff00b,  /*   2: XPT_WR_CDB off */
           0x3ffff05b,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff024,  /*   4: XPT_RD_RS off */
           0x3ffff03e,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff056,  /*   7: XPT_RD_PB off */
           0x80fd7061,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438069,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd009,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae905a,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617067,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e04b,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662054,  /*  17: SATA RR 4015.6862745098ns */
           0x80662055,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x3ffff046,  /*  19: MCIF2_RD off */
           0x3ffff048,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e04a,  /*  22: BSP RR 50000ns */
           0x80ad9058,  /*  23: SAGE RR 6820ns */
           0x8644906b,  /*  24: FLASH_DMA RR 63000ns */
           0x81617066,  /*  25: HIF_PCIe RR 13880ns */
           0x8644906d,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644906c,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff047,  /*  29: MCIF_RD off */
           0x3ffff049,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db062,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5065,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5064,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1059,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db063,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff00e,  /*  40: RAAGA off */
           0x3ffff002,  /*  41: RAAGA_1 off */
           0x3ffff02e,  /*  42: RAAGA1 off */
           0x3ffff001,  /*  43: RAAGA1_1 off */
           0x3ffff02f,  /*  44: AUD_AIO off */
           0x3ffff051,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff068,  /*  46: VICE_CME_CSC off */
           0x3ffff053,  /*  47: VICE_FME_CSC off */
           0x3ffff05d,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff05c,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff03b,  /*  50: VICE_SG off */
           0x3ffff07e,  /*  51: VICE_DBLK off */
           0x3ffff040,  /*  52: VICE_CABAC0 off */
           0x3ffff045,  /*  53: VICE_CABAC1 off */
           0x3ffff057,  /*  54: VICE_ARCSS0 off */
           0x3ffff01c,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff036,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff01d,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff037,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff06f,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff074,  /*  60: VICE1_CME_CSC off */
           0x3ffff070,  /*  61: VICE1_FME_CSC off */
           0x3ffff072,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff071,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff03a,  /*  64: VICE1_SG off */
           0x3ffff07d,  /*  65: VICE1_DBLK off */
           0x3ffff03f,  /*  66: VICE1_CABAC0 off */
           0x3ffff044,  /*  67: VICE1_CABAC1 off */
           0x3ffff073,  /*  68: VICE1_ARCSS0 off */
           0x3ffff019,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff033,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff01a,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff034,  /*  72: VICE1_VIP1_INST1 off */
           0x3ffff075,  /*  73: HVD0_DBLK_0 off */
           0x3ffff076,  /*  74: HVD0_DBLK_1 off */
           0x3ffff04e,  /*  75: HVD0_ILCPU off */
           0x3ffff05e,  /*  76: HVD0_OLCPU off */
           0x3ffff025,  /*  77: HVD0_CAB off */
           0x3ffff020,  /*  78: HVD0_ILSI off */
           0x82c9006a,  /*  79: HVD0_ILCPU_p2 RR 26413ns */
           0x3ffff023,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff079,  /*  81: HVD1_DBLK_0 off */
           0x3ffff07a,  /*  82: HVD1_DBLK_1 off */
           0x3ffff04f,  /*  83: HVD1_ILCPU off */
           0x3ffff05f,  /*  84: HVD1_OLCPU off */
           0x3ffff026,  /*  85: HVD1_CAB off */
           0x3ffff021,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff07b,  /*  88: HVD2_DBLK_0 off */
           0x3ffff07c,  /*  89: HVD2_DBLK_1 off */
           0x3ffff050,  /*  90: HVD2_ILCPU off */
           0x3ffff060,  /*  91: HVD2_OLCPU off */
           0x3ffff027,  /*  92: HVD2_CAB off */
           0x3ffff022,  /*  93: HVD2_ILSI off */
           0x3ffff018,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff02d,  /*  95: BVN_MAD_QUANT off */
           0x3ffff032,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff016,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff02b,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff030,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x3ffff010,  /* 106: BVN_MFD0 off */
           0x3ffff000,  /* 107: BVN_MFD0_1 off */
           0x3ffff028,  /* 108: BVN_MFD1 off */
           0x3ffff011,  /* 109: BVN_MFD1_1 off */
           0x3ffff029,  /* 110: BVN_MFD2 off */
           0x3ffff012,  /* 111: BVN_MFD2_1 off */
           0x3ffff02a,  /* 112: BVN_MFD3 off */
           0x3ffff013,  /* 113: BVN_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001fd005,  /* 118: BVN_VFD0 1185ns */
           0x001fd006,  /* 119: BVN_VFD1 1185ns */
           0x0359a043,  /* 120: BVN_VFD2 31770ns */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x004fc00c,  /* 125: BVN_VFD7 2960ns */
           0x001e5003,  /* 126: BVN_CAP0 1128.594ns */
           0x001fd007,  /* 127: BVN_CAP1 1185ns */
           0x0174703d,  /* 128: BVN_CAP2 13800ns */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x004fc00d,  /* 133: BVN_CAP7 2960ns */
           0x0031d008,  /* 134: BVN_GFD0 1851ns */
           0x0167c03c,  /* 135: BVN_GFD1 13330ns */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff015,  /* 141: BVN_MCVP0 off */
           0x3ffff014,  /* 142: BVN_MCVP1 off */
           0x3ffff039,  /* 143: BVN_MCVP2 off */
           0x0057100f,  /* 144: BVN_RDC 3230ns */
           0x3ffff041,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff042,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff01e,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff038,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff01b,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff035,  /* 154: VICE1_VIP1_INST2 off */
           0x3ffff077,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff078,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff017,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff02c,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff031,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x8014204c,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x80000089,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026104d,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000008a,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff081,  /* 216: HVD0_PFRI off */
           0x3ffff082,  /* 217: HVD1_PFRI off */
           0x3ffff083,  /* 218: HVD2_PFRI off */
           0x3ffff080,  /* 219: VICE_PFRI off */
           0x3ffff07f,  /* 220: VICE1_PFRI off */
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
           0xbfffe06e,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x0069201f   /* 255: REFRESH 3900ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20151125190323_7445_1u4t[] = {
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_CONFIG,      0x80850803}, /* VICE_PFRI (gVice) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80860802}, /* HVD0_PFRI (gHvd0) 448320.00 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000326}, /* d: 4; p: 806.975 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000905}, /* 2309 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000569}, /* 60% * 2309 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80870802}, /* HVD1_PFRI (gHvd1) 448320.00 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000326}, /* d: 4; p: 806.975 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x00000905}, /* 2309 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x00000569}, /* 60% * 2309 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_CONFIG,      0x80880802}, /* HVD2_PFRI (gHvd2) 483840.00 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_FILTER_CTRL, 0x40000366}, /* d: 4; p: 870.908333333333 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH0,     0x00000a6e}, /* 2670 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH1,     0x00000642}, /* 60% * 2670 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_CONFIG,      0x80840803}, /* VICE1_PFRI (gVice1) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH1,     0x000000f9}  /* 60% * 416 */
};

static const uint32_t* const paulMemc_box15[] = { &aulMemc0_20151125190323_7445_1u4t[0], &aulMemc1_20151125190323_7445_1u4t[0], &aulMemc2_20151125190323_7445_1u4t[0]};

const BBOX_Rts stBoxRts_7445_1u4t_box15 = {
  "20151125190323_7445_1u4t_box15",
  7445,
  15,
  3,
  256,
  (const uint32_t**)&paulMemc_box15[0],
  20,
  stBoxRts_PfriClient_20151125190323_7445_1u4t
};
