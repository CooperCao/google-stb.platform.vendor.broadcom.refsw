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
*   at: Fri Nov 20 18:30:10 2015 GMT
*   by: robinc
*   for: Box 7445_1u3t
*         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 2 (32-bit DDR3@1067MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20151120183010_7445_1u3t[] = {
           0x001e7002,  /*   0: XPT_WR_RS 1134.2383107089ns */
           0x803a1062,  /*   1: XPT_WR_XC RR 2285.71428571429ns */
           0x80490009,  /*   2: XPT_WR_CDB RR 2709.90990990991ns */
           0x80b9206a,  /*   3: XPT_WR_ITB_MSG RR 7274.48609431681ns */
           0x8078002c,  /*   4: XPT_RD_RS RR 4449.70414201183ns */
           0x819e304f,  /*   5: XPT_RD_XC_RMX_MSG RR 15346.9387755102ns */
           0x80490008,  /*   6: XPT_RD_XC_RAVE RR 2709.90990990991ns */
           0x80a2e065,  /*   7: XPT_RD_PB RR 6400ns */
           0x80fd706d,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438074,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd007,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9068,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617073,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05c,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662063,  /*  17: SATA RR 4015.6862745098ns */
           0x80662064,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x03e6e057,  /*  19: MCIF2_RD 37000ns */
           0x03e6e059,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05b,  /*  22: BSP RR 50000ns */
           0x80ad9066,  /*  23: SAGE RR 6820ns */
           0x86449076,  /*  24: FLASH_DMA RR 63000ns */
           0x81617072,  /*  25: HIF_PCIe RR 13880ns */
           0x86449078,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449077,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e058,  /*  29: MCIF_RD 37000ns */
           0x03e6e05a,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db06e,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5071,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5070,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1067,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db06f,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x0050e012,  /*  40: RAAGA 3000ns */
           0x001ae001,  /*  41: RAAGA_1 1000ns */
           0x0050e013,  /*  42: RAAGA1 3000ns */
           0x001ae000,  /*  43: RAAGA1_1 1000ns */
           0x00bb403d,  /*  44: AUD_AIO 6940ns */
           0x3ffff07e,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff085,  /*  46: VICE_CME_CSC off */
           0x3ffff07f,  /*  47: VICE_FME_CSC off */
           0x3ffff081,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff080,  /*  49: VICE_FME_Chroma_CMB off */
           0x80dca04c,  /*  50: VICE_SG RR 8176.66666666667ns */
           0x3ffff08f,  /*  51: VICE_DBLK off */
           0x81edf050,  /*  52: VICE_CABAC0 RR 18300ns */
           0x83782055,  /*  53: VICE_CABAC1 RR 32900ns */
           0x3ffff084,  /*  54: VICE_ARCSS0 off */
           0x3ffff026,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff046,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff027,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff047,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff07a,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff083,  /*  60: VICE1_CME_CSC off */
           0x3ffff07b,  /*  61: VICE1_FME_CSC off */
           0x3ffff07d,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff07c,  /*  63: VICE1_FME_Chroma_CMB off */
           0x80dca04b,  /*  64: VICE1_SG RR 8176.66666666667ns */
           0x3ffff08e,  /*  65: VICE1_DBLK off */
           0x82317051,  /*  66: VICE1_CABAC0 RR 20800ns */
           0x83e17056,  /*  67: VICE1_CABAC1 RR 36800ns */
           0x3ffff082,  /*  68: VICE1_ARCSS0 off */
           0x3ffff023,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff043,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff024,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff044,  /*  72: VICE1_VIP1_INST1 off */
           0x3ffff086,  /*  73: HVD0_DBLK_0 off */
           0x3ffff087,  /*  74: HVD0_DBLK_1 off */
           0x8027005f,  /*  75: HVD0_ILCPU RR 1451ns */
           0x80d9b06b,  /*  76: HVD0_OLCPU RR 8553ns */
           0x007de02f,  /*  77: HVD0_CAB 4667ns */
           0x0071a029,  /*  78: HVD0_ILSI 4214ns */
           0x82c90075,  /*  79: HVD0_ILCPU_p2 RR 26413ns */
           0x0073f02b,  /*  80: HVD0_ILSI_p2 4299ns */
           0x3ffff08a,  /*  81: HVD1_DBLK_0 off */
           0x3ffff08b,  /*  82: HVD1_DBLK_1 off */
           0x80270060,  /*  83: HVD1_ILCPU RR 1451ns */
           0x80d9b06c,  /*  84: HVD1_OLCPU RR 8553ns */
           0x007de030,  /*  85: HVD1_CAB 4667ns */
           0x0071a02a,  /*  86: HVD1_ILSI 4214ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff08c,  /*  88: HVD2_DBLK_0 off */
           0x3ffff08d,  /*  89: HVD2_DBLK_1 off */
           0x80270061,  /*  90: HVD2_ILCPU RR 1451ns */
           0x80af7069,  /*  91: HVD2_OLCPU RR 6893ns */
           0x007dd02e,  /*  92: HVD2_CAB 4666ns */
           0x00b6e03c,  /*  93: HVD2_ILSI 6779ns */
           0x3ffff022,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff03b,  /*  95: BVN_MAD_QUANT off */
           0x3ffff042,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff01e,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff037,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff03e,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff01f,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff038,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff03f,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff020,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff039,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff040,  /* 105: BVN_MAD3_PIX_CAP off */
           0x3ffff015,  /* 106: BVN_MFD0 off */
           0x3ffff016,  /* 107: BVN_MFD0_1 off */
           0x3ffff032,  /* 108: BVN_MFD1 off */
           0x3ffff017,  /* 109: BVN_MFD1_1 off */
           0x3ffff033,  /* 110: BVN_MFD2 off */
           0x3ffff018,  /* 111: BVN_MFD2_1 off */
           0x3ffff034,  /* 112: BVN_MFD3 off */
           0x3ffff019,  /* 113: BVN_MFD3_1 off */
           0x3ffff035,  /* 114: BVN_MFD4 off */
           0x3ffff01a,  /* 115: BVN_MFD4_1 off */
           0x3ffff036,  /* 116: BVN_MFD5 off */
           0x3ffff01b,  /* 117: BVN_MFD5_1 off */
           0x007fd031,  /* 118: BVN_VFD0 4740ns */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff054,  /* 120: BVN_VFD2 off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff00a,  /* 122: BVN_VFD4 off */
           0x3ffff00b,  /* 123: BVN_VFD5 off */
           0x3ffff00c,  /* 124: BVN_VFD6 off */
           0x3ffff00d,  /* 125: BVN_VFD7 off */
           0x0079c02d,  /* 126: BVN_CAP0 4514.376ns */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff04e,  /* 128: BVN_CAP2 off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff00e,  /* 130: BVN_CAP4 off */
           0x3ffff00f,  /* 131: BVN_CAP5 off */
           0x3ffff010,  /* 132: BVN_CAP6 off */
           0x3ffff011,  /* 133: BVN_CAP7 off */
           0x0031d006,  /* 134: BVN_GFD0 1851ns */
           0x0167c04d,  /* 135: BVN_GFD1 13330ns */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x0027d003,  /* 138: BVN_GFD4 1481ns */
           0x0027d004,  /* 139: BVN_GFD5 1481ns */
           0x0027d005,  /* 140: BVN_GFD6 1481ns */
           0x3ffff01d,  /* 141: BVN_MCVP0 off */
           0x3ffff01c,  /* 142: BVN_MCVP1 off */
           0x3ffff049,  /* 143: BVN_MCVP2 off */
           0x3ffff014,  /* 144: BVN_RDC off */
           0x03526052,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526053,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff028,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff048,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff025,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff045,  /* 154: VICE1_VIP1_INST2 off */
           0x3ffff088,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff089,  /* 156: HVD0_DBLK_p2_1 off */
           0x3ffff021,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff03a,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff041,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x8014205d,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000009a,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026105e,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000009b,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff092,  /* 216: HVD0_PFRI off */
           0x3ffff093,  /* 217: HVD1_PFRI off */
           0x3ffff094,  /* 218: HVD2_PFRI off */
           0x3ffff091,  /* 219: VICE_PFRI off */
           0x3ffff090,  /* 220: VICE1_PFRI off */
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
           0xbfffe079,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2704a   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20151120183010_7445_1u3t[] = {
           0x3ffff002,  /*   0: XPT_WR_RS off */
           0x3ffff062,  /*   1: XPT_WR_XC off */
           0x3ffff009,  /*   2: XPT_WR_CDB off */
           0x3ffff06a,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff02c,  /*   4: XPT_RD_RS off */
           0x3ffff04f,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff008,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff065,  /*   7: XPT_RD_PB off */
           0x80fd706d,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438074,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd007,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9068,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617073,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05c,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662063,  /*  17: SATA RR 4015.6862745098ns */
           0x80662064,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x3ffff057,  /*  19: MCIF2_RD off */
           0x3ffff059,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05b,  /*  22: BSP RR 50000ns */
           0x80ad9066,  /*  23: SAGE RR 6820ns */
           0x86449076,  /*  24: FLASH_DMA RR 63000ns */
           0x81617072,  /*  25: HIF_PCIe RR 13880ns */
           0x86449078,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449077,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff058,  /*  29: MCIF_RD off */
           0x3ffff05a,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db06e,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5071,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5070,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1067,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db06f,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff012,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff013,  /*  42: RAAGA1 off */
           0x3ffff000,  /*  43: RAAGA1_1 off */
           0x3ffff03d,  /*  44: AUD_AIO off */
           0x8032d07e,  /*  45: VICE_CME_RMB_CMB RR 2000ns */
           0x81977085,  /*  46: VICE_CME_CSC RR 16000ns */
           0x805d607f,  /*  47: VICE_FME_CSC RR 3670ns */
           0x805d6081,  /*  48: VICE_FME_Luma_CMB RR 3670ns */
           0x805d6080,  /*  49: VICE_FME_Chroma_CMB RR 3670ns */
           0x3ffff04c,  /*  50: VICE_SG off */
           0x8000008f,  /*  51: VICE_DBLK RR 0ns */
           0x3ffff050,  /*  52: VICE_CABAC0 off */
           0x3ffff055,  /*  53: VICE_CABAC1 off */
           0x804d9084,  /*  54: VICE_ARCSS0 RR 3050ns */
           0x8063d026,  /*  55: VICE_VIP0_INST0 RR 3703ns */
           0x80c7a046,  /*  56: VICE_VIP1_INST0 RR 7400ns */
           0x8063d027,  /*  57: VICE_VIP0_INST1 RR 3703ns */
           0x80c7a047,  /*  58: VICE_VIP1_INST1 RR 7400ns */
           0x8032d07a,  /*  59: VICE1_CME_RMB_CMB RR 2000ns */
           0x81977083,  /*  60: VICE1_CME_CSC RR 16000ns */
           0x805d607b,  /*  61: VICE1_FME_CSC RR 3670ns */
           0x80bb107d,  /*  62: VICE1_FME_Luma_CMB RR 7350ns */
           0x80bb107c,  /*  63: VICE1_FME_Chroma_CMB RR 7350ns */
           0x3ffff04b,  /*  64: VICE1_SG off */
           0x8000008e,  /*  65: VICE1_DBLK RR 0ns */
           0x3ffff051,  /*  66: VICE1_CABAC0 off */
           0x3ffff056,  /*  67: VICE1_CABAC1 off */
           0x80a2e082,  /*  68: VICE1_ARCSS0 RR 6400ns */
           0x8063d023,  /*  69: VICE1_VIP0_INST0 RR 3703ns */
           0x80c7a043,  /*  70: VICE1_VIP1_INST0 RR 7400ns */
           0x8063d024,  /*  71: VICE1_VIP0_INST1 RR 3703ns */
           0x80c7a044,  /*  72: VICE1_VIP1_INST1 RR 7400ns */
           0x3ffff086,  /*  73: HVD0_DBLK_0 off */
           0x3ffff087,  /*  74: HVD0_DBLK_1 off */
           0x3ffff05f,  /*  75: HVD0_ILCPU off */
           0x3ffff06b,  /*  76: HVD0_OLCPU off */
           0x3ffff02f,  /*  77: HVD0_CAB off */
           0x3ffff029,  /*  78: HVD0_ILSI off */
           0x82c90075,  /*  79: HVD0_ILCPU_p2 RR 26413ns */
           0x3ffff02b,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff08a,  /*  81: HVD1_DBLK_0 off */
           0x3ffff08b,  /*  82: HVD1_DBLK_1 off */
           0x3ffff060,  /*  83: HVD1_ILCPU off */
           0x3ffff06c,  /*  84: HVD1_OLCPU off */
           0x3ffff030,  /*  85: HVD1_CAB off */
           0x3ffff02a,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff08c,  /*  88: HVD2_DBLK_0 off */
           0x3ffff08d,  /*  89: HVD2_DBLK_1 off */
           0x3ffff061,  /*  90: HVD2_ILCPU off */
           0x3ffff069,  /*  91: HVD2_OLCPU off */
           0x3ffff02e,  /*  92: HVD2_CAB off */
           0x3ffff03c,  /*  93: HVD2_ILSI off */
           0x005ea022,  /*  94: BVN_MAD_PIX_FD 3511ns */
           0x0085303b,  /*  95: BVN_MAD_QUANT 4938ns */
           0x00bd7042,  /*  96: BVN_MAD_PIX_CAP 7023ns */
           0x005ea01e,  /*  97: BVN_MAD1_PIX_FD 3511ns */
           0x00853037,  /*  98: BVN_MAD1_QUANT 4938ns */
           0x00bd703e,  /*  99: BVN_MAD1_PIX_CAP 7023ns */
           0x005ea01f,  /* 100: BVN_MAD2_PIX_FD 3511ns */
           0x00853038,  /* 101: BVN_MAD2_QUANT 4938ns */
           0x00bd703f,  /* 102: BVN_MAD2_PIX_CAP 7023ns */
           0x005ea020,  /* 103: BVN_MAD3_PIX_FD 3511ns */
           0x00853039,  /* 104: BVN_MAD3_QUANT 4938ns */
           0x00bd7040,  /* 105: BVN_MAD3_PIX_CAP 7023ns */
           0x3ffff015,  /* 106: BVN_MFD0 off */
           0x3ffff016,  /* 107: BVN_MFD0_1 off */
           0x3ffff032,  /* 108: BVN_MFD1 off */
           0x3ffff017,  /* 109: BVN_MFD1_1 off */
           0x3ffff033,  /* 110: BVN_MFD2 off */
           0x3ffff018,  /* 111: BVN_MFD2_1 off */
           0x3ffff034,  /* 112: BVN_MFD3 off */
           0x3ffff019,  /* 113: BVN_MFD3_1 off */
           0x3ffff035,  /* 114: BVN_MFD4 off */
           0x3ffff01a,  /* 115: BVN_MFD4_1 off */
           0x3ffff036,  /* 116: BVN_MFD5 off */
           0x3ffff01b,  /* 117: BVN_MFD5_1 off */
           0x3ffff031,  /* 118: BVN_VFD0 off */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff054,  /* 120: BVN_VFD2 off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x004fc00a,  /* 122: BVN_VFD4 2960ns */
           0x004fc00b,  /* 123: BVN_VFD5 2960ns */
           0x004fc00c,  /* 124: BVN_VFD6 2960ns */
           0x004fc00d,  /* 125: BVN_VFD7 2960ns */
           0x3ffff02d,  /* 126: BVN_CAP0 off */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff04e,  /* 128: BVN_CAP2 off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x004fc00e,  /* 130: BVN_CAP4 2960ns */
           0x004fc00f,  /* 131: BVN_CAP5 2960ns */
           0x004fc010,  /* 132: BVN_CAP6 2960ns */
           0x004fc011,  /* 133: BVN_CAP7 2960ns */
           0x3ffff006,  /* 134: BVN_GFD0 off */
           0x3ffff04d,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff003,  /* 138: BVN_GFD4 off */
           0x3ffff004,  /* 139: BVN_GFD5 off */
           0x3ffff005,  /* 140: BVN_GFD6 off */
           0x005c101d,  /* 141: BVN_MCVP0 3415ns */
           0x005c101c,  /* 142: BVN_MCVP1 3415ns */
           0x00cf2049,  /* 143: BVN_MCVP2 7676ns */
           0x3ffff014,  /* 144: BVN_RDC off */
           0x3ffff052,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff053,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x8063d028,  /* 151: VICE_VIP0_INST2 RR 3703ns */
           0x80c7a048,  /* 152: VICE_VIP1_INST2 RR 7400ns */
           0x8063d025,  /* 153: VICE1_VIP0_INST2 RR 3703ns */
           0x80c7a045,  /* 154: VICE1_VIP1_INST2 RR 7400ns */
           0x3ffff088,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff089,  /* 156: HVD0_DBLK_p2_1 off */
           0x005ea021,  /* 157: BVN_MAD4_PIX_FD 3511ns */
           0x0085303a,  /* 158: BVN_MAD4_QUANT 4938ns */
           0x00bd7041,  /* 159: BVN_MAD4_PIX_CAP 7023ns */
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
           0x8014205d,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000009a,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026105e,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000009b,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff092,  /* 216: HVD0_PFRI off */
           0x3ffff093,  /* 217: HVD1_PFRI off */
           0x3ffff094,  /* 218: HVD2_PFRI off */
           0x80000091,  /* 219: VICE_PFRI RR 0ns */
           0x80000090,  /* 220: VICE1_PFRI RR 0ns */
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
           0xbfffe079,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2704a   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc2_20151120183010_7445_1u3t[] = {
           0x3ffff002,  /*   0: XPT_WR_RS off */
           0x3ffff062,  /*   1: XPT_WR_XC off */
           0x3ffff009,  /*   2: XPT_WR_CDB off */
           0x3ffff06a,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff02c,  /*   4: XPT_RD_RS off */
           0x3ffff04f,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff008,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff065,  /*   7: XPT_RD_PB off */
           0x80fd706d,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns */
           0x82438074,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns */
           0x803fd007,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9068,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617073,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05c,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662063,  /*  17: SATA RR 4015.6862745098ns */
           0x80662064,  /*  18: SATA_1 RR 4015.6862745098ns */
           0x3ffff057,  /*  19: MCIF2_RD off */
           0x3ffff059,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05b,  /*  22: BSP RR 50000ns */
           0x80ad9066,  /*  23: SAGE RR 6820ns */
           0x86449076,  /*  24: FLASH_DMA RR 63000ns */
           0x81617072,  /*  25: HIF_PCIe RR 13880ns */
           0x86449078,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449077,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff058,  /*  29: MCIF_RD off */
           0x3ffff05a,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db06e,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5071,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5070,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1067,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db06f,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff012,  /*  40: RAAGA off */
           0x3ffff001,  /*  41: RAAGA_1 off */
           0x3ffff013,  /*  42: RAAGA1 off */
           0x3ffff000,  /*  43: RAAGA1_1 off */
           0x3ffff03d,  /*  44: AUD_AIO off */
           0x3ffff07e,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff085,  /*  46: VICE_CME_CSC off */
           0x3ffff07f,  /*  47: VICE_FME_CSC off */
           0x3ffff081,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff080,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff04c,  /*  50: VICE_SG off */
           0x3ffff08f,  /*  51: VICE_DBLK off */
           0x3ffff050,  /*  52: VICE_CABAC0 off */
           0x3ffff055,  /*  53: VICE_CABAC1 off */
           0x3ffff084,  /*  54: VICE_ARCSS0 off */
           0x3ffff026,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff046,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff027,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff047,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff07a,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff083,  /*  60: VICE1_CME_CSC off */
           0x3ffff07b,  /*  61: VICE1_FME_CSC off */
           0x3ffff07d,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff07c,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff04b,  /*  64: VICE1_SG off */
           0x3ffff08e,  /*  65: VICE1_DBLK off */
           0x3ffff051,  /*  66: VICE1_CABAC0 off */
           0x3ffff056,  /*  67: VICE1_CABAC1 off */
           0x3ffff082,  /*  68: VICE1_ARCSS0 off */
           0x3ffff023,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff043,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff024,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff044,  /*  72: VICE1_VIP1_INST1 off */
           0x80140086,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8009f087,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff05f,  /*  75: HVD0_ILCPU off */
           0x3ffff06b,  /*  76: HVD0_OLCPU off */
           0x3ffff02f,  /*  77: HVD0_CAB off */
           0x3ffff029,  /*  78: HVD0_ILSI off */
           0x82c90075,  /*  79: HVD0_ILCPU_p2 RR 26413ns */
           0x3ffff02b,  /*  80: HVD0_ILSI_p2 off */
           0x8014008a,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8009f08b,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff060,  /*  83: HVD1_ILCPU off */
           0x3ffff06c,  /*  84: HVD1_OLCPU off */
           0x3ffff030,  /*  85: HVD1_CAB off */
           0x3ffff02a,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x8014008c,  /*  88: HVD2_DBLK_0 RR 0ns */
           0x8009f08d,  /*  89: HVD2_DBLK_1 RR 0ns */
           0x3ffff061,  /*  90: HVD2_ILCPU off */
           0x3ffff069,  /*  91: HVD2_OLCPU off */
           0x3ffff02e,  /*  92: HVD2_CAB off */
           0x3ffff03c,  /*  93: HVD2_ILSI off */
           0x3ffff022,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff03b,  /*  95: BVN_MAD_QUANT off */
           0x3ffff042,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff01e,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff037,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff03e,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff01f,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff038,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff03f,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff020,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff039,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff040,  /* 105: BVN_MAD3_PIX_CAP off */
           0x0058c015,  /* 106: BVN_MFD0 3292ns */
           0x0058c016,  /* 107: BVN_MFD0_1 3292ns */
           0x00853032,  /* 108: BVN_MFD1 4938ns */
           0x0058c017,  /* 109: BVN_MFD1_1 3292ns */
           0x00853033,  /* 110: BVN_MFD2 4938ns */
           0x0058c018,  /* 111: BVN_MFD2_1 3292ns */
           0x00853034,  /* 112: BVN_MFD3 4938ns */
           0x0058c019,  /* 113: BVN_MFD3_1 3292ns */
           0x00853035,  /* 114: BVN_MFD4 4938ns */
           0x0058c01a,  /* 115: BVN_MFD4_1 3292ns */
           0x00853036,  /* 116: BVN_MFD5 4938ns */
           0x0058c01b,  /* 117: BVN_MFD5_1 3292ns */
           0x3ffff031,  /* 118: BVN_VFD0 off */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x0359a054,  /* 120: BVN_VFD2 31770ns */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff00a,  /* 122: BVN_VFD4 off */
           0x3ffff00b,  /* 123: BVN_VFD5 off */
           0x3ffff00c,  /* 124: BVN_VFD6 off */
           0x3ffff00d,  /* 125: BVN_VFD7 off */
           0x3ffff02d,  /* 126: BVN_CAP0 off */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x0174704e,  /* 128: BVN_CAP2 13800ns */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff00e,  /* 130: BVN_CAP4 off */
           0x3ffff00f,  /* 131: BVN_CAP5 off */
           0x3ffff010,  /* 132: BVN_CAP6 off */
           0x3ffff011,  /* 133: BVN_CAP7 off */
           0x3ffff006,  /* 134: BVN_GFD0 off */
           0x3ffff04d,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff003,  /* 138: BVN_GFD4 off */
           0x3ffff004,  /* 139: BVN_GFD5 off */
           0x3ffff005,  /* 140: BVN_GFD6 off */
           0x3ffff01d,  /* 141: BVN_MCVP0 off */
           0x3ffff01c,  /* 142: BVN_MCVP1 off */
           0x3ffff049,  /* 143: BVN_MCVP2 off */
           0x00571014,  /* 144: BVN_RDC 3230ns */
           0x3ffff052,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff053,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff028,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff048,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff025,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff045,  /* 154: VICE1_VIP1_INST2 off */
           0x80140088,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8009f089,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff021,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff03a,  /* 158: BVN_MAD4_QUANT off */
           0x3ffff041,  /* 159: BVN_MAD4_PIX_CAP off */
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
           0x8014205d,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x8000009a,  /* 201: CPU_MCP_RD_LOW RR */
           0x8026105e,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x8000009b,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000092,  /* 216: HVD0_PFRI RR 0ns */
           0x80000093,  /* 217: HVD1_PFRI RR 0ns */
           0x80000094,  /* 218: HVD2_PFRI RR 0ns */
           0x3ffff091,  /* 219: VICE_PFRI off */
           0x3ffff090,  /* 220: VICE1_PFRI off */
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
           0xbfffe079,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2704a   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20151120183010_7445_1u3t[] = {
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_CONFIG,      0x80960802}, /* VICE_PFRI (gVice) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_THRESH0,     0x000001a8}, /* 424 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_THRESH1,     0x000000fe}, /* 60% * 424 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_CONFIG,      0x80950803}, /* VICE1_PFRI (gVice1) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_CONFIG,      0x80970803}, /* HVD0_PFRI (gHvd0) 448320.00 ns/60 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000326}, /* d: 4; p: 806.975 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH0,     0x000008d8}, /* 2264 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH1,     0x0000054e}, /* 60% * 2264 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_CONFIG,      0x80980802}, /* HVD1_PFRI (gHvd1) 448320.00 ns/60 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000326}, /* d: 4; p: 806.975 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_THRESH0,     0x00000905}, /* 2309 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_THRESH1,     0x00000569}, /* 60% * 2309 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_CONFIG,      0x80990802}, /* HVD2_PFRI (gHvd2) 497333.33 ns/60 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_FILTER_CTRL, 0x4000037f}, /* d: 4; p: 895.2 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_THRESH0,     0x00000905}, /* 2309 */
  {BCHP_MEMC_GEN_2_PFRI_2_THROTTLE_THRESH1,     0x00000569}  /* 60% * 2309 */
};

static const uint32_t* const paulMemc_box14[] = { &aulMemc0_20151120183010_7445_1u3t[0], &aulMemc1_20151120183010_7445_1u3t[0], &aulMemc2_20151120183010_7445_1u3t[0]};

const BBOX_Rts stBoxRts_7445_1u3t_box14 = {
  "20151120183010_7445_1u3t_box14",
  7445,
  14,
  3,
  256,
  (const uint32_t**)&paulMemc_box14[0],
  20,
  stBoxRts_PfriClient_20151120183010_7445_1u3t
};
