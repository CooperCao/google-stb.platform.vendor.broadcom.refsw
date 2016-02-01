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
*   at: Mon Sep 29 16:40:53 2014 GMT
*   by: kranawet
*   for: Box 3385_1u3t_box1
*         MemC 0 (32-bit DDRDDR3@1066MHz) w/432MHz clock
*         MemC 1 (32-bit DDRDDR3@1066MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20140929164053_3385_1u3t_box1[] = {
           0x001e6001,  /*   0: XPT_WR_RS 1130ns */
           0x803c105f,  /*   1: XPT_WR_XC RR 2230ns */
           0x8047200b,  /*   2: XPT_WR_CDB RR 2640ns */
           0x80c0a070,  /*   3: XPT_WR_ITB_MSG RR 7140ns */
           0x80780021,  /*   4: XPT_RD_RS RR 4450ns */
           0x819e503f,  /*   5: XPT_RD_XC_RMX_MSG RR 15350ns */
           0x8047200a,  /*   6: XPT_RD_XC_RAVE RR 2640ns */
           0x80a4d06e,  /*   7: XPT_RD_PB RR 6110ns */
           0x809d806a,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e607a,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x0a8be054,  /*  10: MCIF2_RD 100000ns */
           0x0a8be055,  /*  11: MCIF2_WR 100000ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x3ffff0ff,  /*  15: UNASSIGNED off */
           0x8596e051,  /*  16: MOCA_MIPS RR 53000ns */
           0x80fe9073,  /*  17: SATA RR 10000ns */
           0x80fe9076,  /*  18: SATA_1 RR 10000ns */
           0x80fe9074,  /*  19: SATA1 RR 10000ns */
           0x80fe9075,  /*  20: SATA1_1 RR 10000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e050,  /*  22: BSP RR 50000ns */
           0x80ad906f,  /*  23: SAGE RR 6820ns */
           0x86449083,  /*  24: FLASH_DMA RR 63000ns */
           0x3ffff0ff,  /*  25: UNASSIGNED off */
           0x86449085,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449084,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e04e,  /*  29: MCIF_RD 37000ns */
           0x03e6e04f,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db079,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x80931069,  /*  35: USB_X_WRITE_0 RR 5780ns */
           0x80931068,  /*  36: USB_X_READ_0 RR 5780ns */
           0x8166f03e,  /*  37: USB_X_CTRL_0 RR 13300ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x8050e061,  /*  40: RAAGA RR 3000ns */
           0x801ae05a,  /*  41: RAAGA_1 RR 1000ns */
           0x80a1e06b,  /*  42: RAAGA1 RR 6000ns */
           0x801ae059,  /*  43: RAAGA1_1 RR 1000ns */
           0x00bb4032,  /*  44: AUD_AIO 6940ns */
           0x80728066,  /*  45: VICE2_0_CME_RMB_CMB RR 4500ns */
           0x8394e081,  /*  46: VICE2_0_CME_CSC RR 36000ns */
           0x80d41071,  /*  47: VICE2_0_FME_CSC RR 8330ns */
           0x81a6b07d,  /*  48: VICE2_0_FME_Luma_CMB RR 16600ns */
           0x81a6b07c,  /*  49: VICE2_0_FME_Chroma_CMB RR 16600ns */
           0x82574044,  /*  50: VICE2_0_SG RR 22200ns */
           0x8016508c,  /*  51: VICE2_0_DBLK RR 0ns */
           0x83dc104c,  /*  52: VICE2_0_CABAC0 RR 36600ns */
           0x86f07052,  /*  53: VICE2_0_CABAC1 RR 65800ns */
           0x80a2e06c,  /*  54: VICE2_0_ARCSS0 RR 6400ns */
           0x80c7d037,  /*  55: VICE2_0_VIP0_INST0 RR 7406ns */
           0x82574045,  /*  56: VICE2_0_VIP1_INST0 RR 22200ns */
           0x80c7d038,  /*  57: VICE2_0_VIP0_INST1 RR 7406ns */
           0x82574046,  /*  58: VICE2_0_VIP1_INST1 RR 22200ns */
           0x80728067,  /*  59: VICE2_1_CME_RMB_CMB RR 4500ns */
           0x8394e082,  /*  60: VICE2_1_CME_CSC RR 36000ns */
           0x80d41072,  /*  61: VICE2_1_FME_CSC RR 8330ns */
           0x81a6b07f,  /*  62: VICE2_1_FME_Luma_CMB RR 16600ns */
           0x81a6b07e,  /*  63: VICE2_1_FME_Chroma_CMB RR 16600ns */
           0x82574047,  /*  64: VICE2_1_SG RR 22200ns */
           0x8016508d,  /*  65: VICE2_1_DBLK RR 0ns */
           0x83dc104d,  /*  66: VICE2_1_CABAC0 RR 36600ns */
           0x86f07053,  /*  67: VICE2_1_CABAC1 RR 65800ns */
           0x80a2e06d,  /*  68: VICE2_1_ARCSS0 RR 6400ns */
           0x80c7d039,  /*  69: VICE2_1_VIP0_INST0 RR 7406ns */
           0x82574048,  /*  70: VICE2_1_VIP1_INST0 RR 22200ns */
           0x80c7d03a,  /*  71: VICE2_1_VIP0_INST1 RR 7406ns */
           0x82574049,  /*  72: VICE2_1_VIP1_INST1 RR 22200ns */
           0x3ffff090,  /*  73: HVD0_DBLK_0 off */
           0x3ffff091,  /*  74: HVD0_DBLK_1 off */
           0x8027005d,  /*  75: HVD0_ILCPU RR 1451ns */
           0x8070a065,  /*  76: HVD0_OLCPU RR 4427ns */
           0x007dd023,  /*  77: HVD0_CAB 4666ns */
           0x0071a020,  /*  78: HVD0_ILSI 4214ns */
           0x3ffff0ff,  /*  79: UNASSIGNED off */
           0x3ffff0ff,  /*  80: UNASSIGNED off */
           0x3ffff08e,  /*  81: AVD1_DBLK_0 off */
           0x3ffff08f,  /*  82: AVD1_DBLK_1 off */
           0x8027005c,  /*  83: AVD1_ILCPU RR 1451ns */
           0x8070a064,  /*  84: AVD1_OLCPU RR 4427ns */
           0x007dd022,  /*  85: AVD1_CAB 4666ns */
           0x0071a01f,  /*  86: AVD1_ILSI 4214ns */
           0xbffff0ff,  /*  87: SID RR */
           0x8a8be057,  /*  88: USB1_HI_0 RR 100000ns */
           0xbffff0ff,  /*  89: USB1_LO_0 RR */
           0x80fe9078,  /*  90: USB1_X_WRITE_0 RR 10000ns */
           0x80fe9077,  /*  91: USB1_X_READ_0 RR 10000ns */
           0x810de03d,  /*  92: USB1_X_CTRL_0 RR 10000ns */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff017,  /*  94: MADR_RD off */
           0x3ffff02b,  /*  95: MADR_QM off */
           0x3ffff036,  /*  96: MADR_WR off */
           0x3ffff033,  /*  97: MADR1_RD off */
           0x3ffff028,  /*  98: MADR1_QM off */
           0x3ffff018,  /*  99: MADR1_WR off */
           0x3ffff034,  /* 100: MADR2_RD off */
           0x3ffff029,  /* 101: MADR2_QM off */
           0x3ffff019,  /* 102: MADR2_WR off */
           0x3ffff035,  /* 103: MADR3_RD off */
           0x3ffff02a,  /* 104: MADR3_QM off */
           0x3ffff01a,  /* 105: MADR3_WR off */
           0x3ffff01b,  /* 106: BVN_MFD0_0 off */
           0x3ffff006,  /* 107: BVN_MFD0_1 off */
           0x3ffff01c,  /* 108: BVN_MFD1_0 off */
           0x3ffff007,  /* 109: BVN_MFD1_1 off */
           0x3ffff01d,  /* 110: BVN_MFD2_0 off */
           0x3ffff008,  /* 111: BVN_MFD2_1 off */
           0x3ffff01e,  /* 112: BVN_MFD3_0 off */
           0x3ffff009,  /* 113: BVN_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff024,  /* 118: BVN_VFD0 off */
           0x3ffff025,  /* 119: BVN_VFD1 off */
           0x01aa7040,  /* 120: BVN_VFD2 15800ns */
           0x3ffff041,  /* 121: BVN_VFD3 off */
           0x3ffff00e,  /* 122: BVN_VFD4 off */
           0x3ffff00f,  /* 123: BVN_VFD5 off */
           0x3ffff010,  /* 124: BVN_VFD6 off */
           0x3ffff011,  /* 125: BVN_VFD7 off */
           0x3ffff026,  /* 126: BVN_CAP0 off */
           0x3ffff027,  /* 127: BVN_CAP1 off */
           0x01aa7042,  /* 128: BVN_CAP2 15800ns */
           0x3ffff043,  /* 129: BVN_CAP3 off */
           0x3ffff012,  /* 130: BVN_CAP4 off */
           0x3ffff013,  /* 131: BVN_CAP5 off */
           0x3ffff014,  /* 132: BVN_CAP6 off */
           0x3ffff015,  /* 133: BVN_CAP7 off */
           0x3ffff004,  /* 134: BVN_GFD0 off */
           0x3ffff02d,  /* 135: BVN_GFD1 off */
           0x3ffff02e,  /* 136: BVN_GFD2 off */
           0x3ffff02f,  /* 137: BVN_GFD3 off */
           0x3ffff030,  /* 138: BVN_GFD4 off */
           0x3ffff031,  /* 139: BVN_GFD5 off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff00d,  /* 141: MCVP0 off */
           0x3ffff00c,  /* 142: MCVP1 off */
           0x3ffff03c,  /* 143: MCVP_QM off */
           0x3ffff003,  /* 144: BVN_RDC off */
           0x0352604a,  /* 145: VEC_VBI_ENC0 31500ns */
           0x0352604b,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x802fb05e,  /* 151: PCIe_0 RR 1877ns */
           0x806b7062,  /* 152: PCIe_1 RR 4225ns */
           0xbffff0ff,  /* 153: M2MC1_0 RR */
           0xbffff0ff,  /* 154: M2MC1_1 RR */
           0xbffff0ff,  /* 155: M2MC1_2 RR */
           0x3ffff0ff,  /* 156: UNASSIGNED off */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0x80401060,  /* 160: GFAP_RD RR 2378ns */
           0x806da063,  /* 161: GFAP_W RR 4065ns */
           0x8a8be056,  /* 162: UNIMAC_LAN_W RR 100000ns */
           0x8a8be088,  /* 163: UNIMAC_LAN_R RR 100000ns */
           0x802dc005,  /* 164: UNIMAC_PORT0_W RR 1700ns */
           0x81bcd080,  /* 165: UNIMAC_PORT0_R RR 16480ns */
           0x80204002,  /* 166: UNIMAC_PORT1_W RR 1200ns */
           0x86675086,  /* 167: UNIMAC_PORT1_R RR 60720ns */
           0x8097102c,  /* 168: GMAC_WAN_W RR 5600ns */
           0x8a8be087,  /* 169: GMAC_WAN_R RR 100000ns */
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
           0x80142058,  /* 200: MCP_RD_HIGH RR 750ns */
           0x8000708a,  /* 201: MCP_RD_LOW RR 23ns */
           0x8026105b,  /* 202: MCP_WR_HIGH RR 1500ns */
           0x8000708b,  /* 203: MCP_WR_LOW RR 23ns */
           0xbffff0ff,  /* 204: V3D_MCP_RD_HIGH RR */
           0xbffff0ff,  /* 205: V3D_MCP_RD_LOW RR */
           0xbffff0ff,  /* 206: V3D_MCP_WR_HIGH RR */
           0xbffff0ff,  /* 207: V3D_MCP_WR_LOW RR */
           0x80564016,  /* 208: UBUS0_R RR 3200ns */
           0x8019f000,  /* 209: UBUS0_W RR 967ns */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x3ffff096,  /* 216: HVD0_PFRI off */
           0x3ffff095,  /* 217: AVD1_PFRI off */
           0x80000093,  /* 218: VICE2_0_PFRI RR 0ns */
           0x80000094,  /* 219: VICE2_1_PFRI RR 0ns */
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
           0x817df07b,  /* 234: CPU_LMB_HI RR 15000ns */
           0x80000092,  /* 235: CPU_LMB_LO RR 0ns */
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
           0xbfffe089,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2d03b,  /* 255: REFRESH 7812.5ns */
         };
static const uint32_t aulMemc1_20140929164053_3385_1u3t_box1[] = {
           0x3ffff001,  /*   0: XPT_WR_RS off */
           0x3ffff05f,  /*   1: XPT_WR_XC off */
           0x3ffff00b,  /*   2: XPT_WR_CDB off */
           0x3ffff070,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff021,  /*   4: XPT_RD_RS off */
           0x3ffff03f,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff06e,  /*   7: XPT_RD_PB off */
           0x809d806a,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e607a,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x0a8be054,  /*  10: MCIF2_RD 100000ns */
           0x0a8be055,  /*  11: MCIF2_WR 100000ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x3ffff0ff,  /*  15: UNASSIGNED off */
           0x8596e051,  /*  16: MOCA_MIPS RR 53000ns */
           0x80fe9073,  /*  17: SATA RR 10000ns */
           0x80fe9076,  /*  18: SATA_1 RR 10000ns */
           0x80fe9074,  /*  19: SATA1 RR 10000ns */
           0x80fe9075,  /*  20: SATA1_1 RR 10000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e050,  /*  22: BSP RR 50000ns */
           0x80ad906f,  /*  23: SAGE RR 6820ns */
           0x86449083,  /*  24: FLASH_DMA RR 63000ns */
           0x3ffff0ff,  /*  25: UNASSIGNED off */
           0x86449085,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449084,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff04e,  /*  29: MCIF_RD off */
           0x3ffff04f,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db079,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x80931069,  /*  35: USB_X_WRITE_0 RR 5780ns */
           0x80931068,  /*  36: USB_X_READ_0 RR 5780ns */
           0x8166f03e,  /*  37: USB_X_CTRL_0 RR 13300ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x3ffff061,  /*  40: RAAGA off */
           0x3ffff05a,  /*  41: RAAGA_1 off */
           0x3ffff06b,  /*  42: RAAGA1 off */
           0x3ffff059,  /*  43: RAAGA1_1 off */
           0x3ffff032,  /*  44: AUD_AIO off */
           0x3ffff066,  /*  45: VICE2_0_CME_RMB_CMB off */
           0x3ffff081,  /*  46: VICE2_0_CME_CSC off */
           0x3ffff071,  /*  47: VICE2_0_FME_CSC off */
           0x3ffff07d,  /*  48: VICE2_0_FME_Luma_CMB off */
           0x3ffff07c,  /*  49: VICE2_0_FME_Chroma_CMB off */
           0x3ffff044,  /*  50: VICE2_0_SG off */
           0x3ffff08c,  /*  51: VICE2_0_DBLK off */
           0x3ffff04c,  /*  52: VICE2_0_CABAC0 off */
           0x3ffff052,  /*  53: VICE2_0_CABAC1 off */
           0x3ffff06c,  /*  54: VICE2_0_ARCSS0 off */
           0x3ffff037,  /*  55: VICE2_0_VIP0_INST0 off */
           0x3ffff045,  /*  56: VICE2_0_VIP1_INST0 off */
           0x3ffff038,  /*  57: VICE2_0_VIP0_INST1 off */
           0x3ffff046,  /*  58: VICE2_0_VIP1_INST1 off */
           0x3ffff067,  /*  59: VICE2_1_CME_RMB_CMB off */
           0x3ffff082,  /*  60: VICE2_1_CME_CSC off */
           0x3ffff072,  /*  61: VICE2_1_FME_CSC off */
           0x3ffff07f,  /*  62: VICE2_1_FME_Luma_CMB off */
           0x3ffff07e,  /*  63: VICE2_1_FME_Chroma_CMB off */
           0x3ffff047,  /*  64: VICE2_1_SG off */
           0x3ffff08d,  /*  65: VICE2_1_DBLK off */
           0x3ffff04d,  /*  66: VICE2_1_CABAC0 off */
           0x3ffff053,  /*  67: VICE2_1_CABAC1 off */
           0x3ffff06d,  /*  68: VICE2_1_ARCSS0 off */
           0x3ffff039,  /*  69: VICE2_1_VIP0_INST0 off */
           0x3ffff048,  /*  70: VICE2_1_VIP1_INST0 off */
           0x3ffff03a,  /*  71: VICE2_1_VIP0_INST1 off */
           0x3ffff049,  /*  72: VICE2_1_VIP1_INST1 off */
           0x8009f090,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8009f091,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff05d,  /*  75: HVD0_ILCPU off */
           0x3ffff065,  /*  76: HVD0_OLCPU off */
           0x3ffff023,  /*  77: HVD0_CAB off */
           0x3ffff020,  /*  78: HVD0_ILSI off */
           0x3ffff0ff,  /*  79: UNASSIGNED off */
           0x3ffff0ff,  /*  80: UNASSIGNED off */
           0x8009f08e,  /*  81: AVD1_DBLK_0 RR 0ns */
           0x8009f08f,  /*  82: AVD1_DBLK_1 RR 0ns */
           0x3ffff05c,  /*  83: AVD1_ILCPU off */
           0x3ffff064,  /*  84: AVD1_OLCPU off */
           0x3ffff022,  /*  85: AVD1_CAB off */
           0x3ffff01f,  /*  86: AVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x8a8be057,  /*  88: USB1_HI_0 RR 100000ns */
           0xbffff0ff,  /*  89: USB1_LO_0 RR */
           0x80fe9078,  /*  90: USB1_X_WRITE_0 RR 10000ns */
           0x80fe9077,  /*  91: USB1_X_READ_0 RR 10000ns */
           0x810de03d,  /*  92: USB1_X_CTRL_0 RR 10000ns */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x005ea017,  /*  94: MADR_RD 3511ns */
           0x0085302b,  /*  95: MADR_QM 4938ns */
           0x00bd7036,  /*  96: MADR_WR 7023ns */
           0x00bd7033,  /*  97: MADR1_RD 7023ns */
           0x00853028,  /*  98: MADR1_QM 4938ns */
           0x005ea018,  /*  99: MADR1_WR 3511ns */
           0x00bd7034,  /* 100: MADR2_RD 7023ns */
           0x00853029,  /* 101: MADR2_QM 4938ns */
           0x005ea019,  /* 102: MADR2_WR 3511ns */
           0x00bd7035,  /* 103: MADR3_RD 7023ns */
           0x0085302a,  /* 104: MADR3_QM 4938ns */
           0x005ea01a,  /* 105: MADR3_WR 3511ns */
           0x0063d01b,  /* 106: BVN_MFD0_0 3703ns */
           0x00428006,  /* 107: BVN_MFD0_1 2469ns */
           0x0063d01c,  /* 108: BVN_MFD1_0 3703ns */
           0x00428007,  /* 109: BVN_MFD1_1 2469ns */
           0x0063d01d,  /* 110: BVN_MFD2_0 3703ns */
           0x00428008,  /* 111: BVN_MFD2_1 2469ns */
           0x0063d01e,  /* 112: BVN_MFD3_0 3703ns */
           0x00428009,  /* 113: BVN_MFD3_1 2469ns */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x007fe024,  /* 118: BVN_VFD0 4741ns */
           0x007fe025,  /* 119: BVN_VFD1 4741ns */
           0x3ffff040,  /* 120: BVN_VFD2 off */
           0x01aa7041,  /* 121: BVN_VFD3 15800ns */
           0x004fc00e,  /* 122: BVN_VFD4 2960ns */
           0x004fc00f,  /* 123: BVN_VFD5 2960ns */
           0x004fc010,  /* 124: BVN_VFD6 2960ns */
           0x004fc011,  /* 125: BVN_VFD7 2960ns */
           0x007fe026,  /* 126: BVN_CAP0 4741ns */
           0x007fe027,  /* 127: BVN_CAP1 4741ns */
           0x3ffff042,  /* 128: BVN_CAP2 off */
           0x01aa7043,  /* 129: BVN_CAP3 15800ns */
           0x004fc012,  /* 130: BVN_CAP4 2960ns */
           0x004fc013,  /* 131: BVN_CAP5 2960ns */
           0x004fc014,  /* 132: BVN_CAP6 2960ns */
           0x004fc015,  /* 133: BVN_CAP7 2960ns */
           0x0027d004,  /* 134: BVN_GFD0 1481ns */
           0x00ab402d,  /* 135: BVN_GFD1 6348ns */
           0x00ab402e,  /* 136: BVN_GFD2 6348ns */
           0x00ab402f,  /* 137: BVN_GFD3 6348ns */
           0x00ab4030,  /* 138: BVN_GFD4 6348ns */
           0x00ab4031,  /* 139: BVN_GFD5 6348ns */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b500d,  /* 141: MCVP0 2794ns */
           0x004b500c,  /* 142: MCVP1 2794ns */
           0x00d3d03c,  /* 143: MCVP_QM 7850ns */
           0x0025a003,  /* 144: BVN_RDC 1400ns */
           0x3ffff04a,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff04b,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x802fb05e,  /* 151: PCIe_0 RR 1877ns */
           0x806b7062,  /* 152: PCIe_1 RR 4225ns */
           0xbffff0ff,  /* 153: M2MC1_0 RR */
           0xbffff0ff,  /* 154: M2MC1_1 RR */
           0xbffff0ff,  /* 155: M2MC1_2 RR */
           0x3ffff0ff,  /* 156: UNASSIGNED off */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0x3ffff060,  /* 160: GFAP_RD off */
           0x3ffff063,  /* 161: GFAP_W off */
           0x3ffff056,  /* 162: UNIMAC_LAN_W off */
           0x3ffff088,  /* 163: UNIMAC_LAN_R off */
           0x3ffff005,  /* 164: UNIMAC_PORT0_W off */
           0x3ffff080,  /* 165: UNIMAC_PORT0_R off */
           0x3ffff002,  /* 166: UNIMAC_PORT1_W off */
           0x3ffff086,  /* 167: UNIMAC_PORT1_R off */
           0x3ffff02c,  /* 168: GMAC_WAN_W off */
           0x3ffff087,  /* 169: GMAC_WAN_R off */
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
           0x80142058,  /* 200: MCP_RD_HIGH RR 750ns */
           0x8000708a,  /* 201: MCP_RD_LOW RR 23ns */
           0x8026105b,  /* 202: MCP_WR_HIGH RR 1500ns */
           0x8000708b,  /* 203: MCP_WR_LOW RR 23ns */
           0xbffff0ff,  /* 204: V3D_MCP_RD_HIGH RR */
           0xbffff0ff,  /* 205: V3D_MCP_RD_LOW RR */
           0xbffff0ff,  /* 206: V3D_MCP_WR_HIGH RR */
           0xbffff0ff,  /* 207: V3D_MCP_WR_LOW RR */
           0x3ffff016,  /* 208: UBUS0_R off */
           0x3ffff000,  /* 209: UBUS0_W off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x80000096,  /* 216: HVD0_PFRI RR 0ns */
           0x80000095,  /* 217: AVD1_PFRI RR 0ns */
           0x3ffff093,  /* 218: VICE2_0_PFRI off */
           0x3ffff094,  /* 219: VICE2_1_PFRI off */
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
           0x3ffff07b,  /* 234: CPU_LMB_HI off */
           0x3ffff092,  /* 235: CPU_LMB_LO off */
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
           0xbfffe089,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2d03b,  /* 255: REFRESH 7812.5ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20140929164053_3385_1u3t_box1[] = {
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x80970803},
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x40000456},
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x000001a0},
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x000000f9},
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_CONFIG,      0x80980802},
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_FILTER_CTRL, 0x40000456},
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH0,     0x000001a8},
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH1,     0x000000fe},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x809a0802},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000430},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000a6e},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000642},
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80990803},
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000430},
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x00000a3a},
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x00000622},
};

static const uint32_t * const paulMemc_box1[] = { &aulMemc0_20140929164053_3385_1u3t_box1[0], &aulMemc1_20140929164053_3385_1u3t_box1[0],};

const BBOX_Rts stBoxRts_3385_1u3t_box1 = {
  "20140929164053_3385_1u3t_box1",
  7145,
  1,
  2,
  256,
  (const uint32_t **)&paulMemc_box1[0],
  16,
  stBoxRts_PfriClient_20140929164053_3385_1u3t_box1,
};
