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
*   at: Tue Jun 23 23:44:14 2015 GMT
*   by: robinc
*   for: Box 7445D0
*         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 2 (32-bit DDR3@1067MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20150623234414_7445D0[] = {
           0x001e7005,  /*   0: XPT_WR_RS 1134.2383107089ns*/
           0x803b7057,  /*   1: XPT_WR_XC RR 2339.03576982893ns*/
           0x804b100d,  /*   2: XPT_WR_CDB RR 2785.18518518519ns*/
           0x80bc9067,  /*   3: XPT_WR_ITB_MSG RR 7408.86699507389ns*/
           0x8078001f,  /*   4: XPT_RD_RS RR 4449.70414201183ns*/
           0x819e303e,  /*   5: XPT_RD_XC_RMX_MSG RR 15346.9387755102ns*/
           0x804b100c,  /*   6: XPT_RD_XC_RAVE RR 2785.18518518519ns*/
           0x80aae060,  /*   7: XPT_RD_PB RR 6714.75409836066ns*/
           0x80fd706a,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns*/
           0x82438074,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns*/
           0x803fd00a,  /*  10: SYSPORT_WR RR 2370ns*/
           0x80ae9063,  /*  11: SYSPORT_RD RR 6860ns*/
           0x3ffff0ff,  /*  12: UNASSIGNED off*/
           0x3ffff0ff,  /*  13: UNASSIGNED off*/
           0x3ffff0ff,  /*  14: UNASSIGNED off*/
           0x81617070,  /*  15: HIF_PCIe1 RR 13880ns*/
           0x8596e04f,  /*  16: MOCA_MIPS RR 53000ns*/
           0x8066205a,  /*  17: SATA RR 4015.6862745098ns*/
           0x8066205b,  /*  18: SATA_1 RR 4015.6862745098ns*/
           0x03e6e04a,  /*  19: MCIF2_RD 37000ns*/
           0x03e6e04c,  /*  20: MCIF2_WR 37000ns*/
           0x3ffff0ff,  /*  21: UNASSIGNED off*/
           0x8545e04e,  /*  22: BSP RR 50000ns*/
           0x80ad9061,  /*  23: SAGE RR 6820ns*/
           0x86449077,  /*  24: FLASH_DMA RR 63000ns*/
           0x8161706f,  /*  25: HIF_PCIe RR 13880ns*/
           0x86449079,  /*  26: SDIO_EMMC RR 63000ns*/
           0x86449078,  /*  27: SDIO_CARD RR 63000ns*/
           0xbffff0ff,  /*  28: TPCAP RR*/
           0x03e6e04b,  /*  29: MCIF_RD 37000ns*/
           0x03e6e04d,  /*  30: MCIF_WR 37000ns*/
           0xbffff0ff,  /*  31: UART_DMA_RD RR*/
           0xbffff0ff,  /*  32: UART_DMA_WR RR*/
           0x810db06b,  /*  33: USB_HI_0 RR 10593ns*/
           0xbffff0ff,  /*  34: USB_LO_0 RR*/
           0x815c506e,  /*  35: USB_X_WRITE_0 RR 13680ns*/
           0x815c506d,  /*  36: USB_X_READ_0 RR 13680ns*/
           0x80ae1062,  /*  37: USB_X_CTRL_0 RR 6840ns*/
           0x810db06c,  /*  38: USB_HI_1 RR 10593ns*/
           0xbffff0ff,  /*  39: USB_LO_1 RR*/
           0x0050e010,  /*  40: RAAGA 3000ns*/
           0x001ae001,  /*  41: RAAGA_1 1000ns*/
           0x00a1e02b,  /*  42: RAAGA1 6000ns*/
           0x001ae000,  /*  43: RAAGA1_1 1000ns*/
           0x00bb402c,  /*  44: AUD_AIO 6940ns*/
           0x8072805d,  /*  45: VICE_CME_RMB_CMB RR 4500ns*/
           0x8394e076,  /*  46: VICE_CME_CSC RR 36000ns*/
           0x80d41068,  /*  47: VICE_FME_CSC RR 8330ns*/
           0x81a6b072,  /*  48: VICE_FME_Luma_CMB RR 16600ns*/
           0x81a6b071,  /*  49: VICE_FME_Chroma_CMB RR 16600ns*/
           0x82574040,  /*  50: VICE_SG RR 22200ns*/
           0x8000207c,  /*  51: VICE_DBLK RR 10ns*/
           0x83dc1049,  /*  52: VICE_CABAC0 RR 36600ns*/
           0x86f07051,  /*  53: VICE_CABAC1 RR 65800ns*/
           0x80a2e05f,  /*  54: VICE_ARCSS0 RR 6400ns*/
           0x80c7d031,  /*  55: VICE_VIP0_INST0 RR 7406ns*/
           0x82574041,  /*  56: VICE_VIP1_INST0 RR 22200ns*/
           0x80c7d032,  /*  57: VICE_VIP0_INST1 RR 7406ns*/
           0x82574042,  /*  58: VICE_VIP1_INST1 RR 22200ns*/
           0x8065c059,  /*  59: VICE1_CME_RMB_CMB RR 4000ns*/
           0x832ef075,  /*  60: VICE1_CME_CSC RR 32000ns*/
           0x80bb1064,  /*  61: VICE1_FME_CSC RR 7350ns*/
           0x80bb1066,  /*  62: VICE1_FME_Luma_CMB RR 7350ns*/
           0x80bb1065,  /*  63: VICE1_FME_Chroma_CMB RR 7350ns*/
           0x81b8d03f,  /*  64: VICE1_SG RR 16333.3333333333ns*/
           0x8000207b,  /*  65: VICE1_DBLK RR 10ns*/
           0x83dc1048,  /*  66: VICE1_CABAC0 RR 36600ns*/
           0x86f07050,  /*  67: VICE1_CABAC1 RR 65800ns*/
           0x80a2e05e,  /*  68: VICE1_ARCSS0 RR 6400ns*/
           0x8063d017,  /*  69: VICE1_VIP0_INST0 RR 3703ns*/
           0x80c7a02e,  /*  70: VICE1_VIP1_INST0 RR 7400ns*/
           0x8063d018,  /*  71: VICE1_VIP0_INST1 RR 3703ns*/
           0x80c7a02f,  /*  72: VICE1_VIP1_INST1 RR 7400ns*/
           0x3ffff07d,  /*  73: HVD0_DBLK_0 off*/
           0x3ffff07e,  /*  74: HVD0_DBLK_1 off*/
           0x801c2053,  /*  75: HVD0_ILCPU RR 1048ns*/
           0x804a7058,  /*  76: HVD0_OLCPU RR 2927ns*/
           0x00596015,  /*  77: HVD0_CAB 3317ns*/
           0x0071001a,  /*  78: HVD0_ILSI 4191ns*/
           0x81ea3073,  /*  79: HVD0_ILCPU_p2 RR 18162ns*/
           0x0071001b,  /*  80: HVD0_ILSI_p2 4191ns*/
           0x3ffff085,  /*  81: HVD1_DBLK_0 off*/
           0x3ffff086,  /*  82: HVD1_DBLK_1 off*/
           0x80270055,  /*  83: HVD1_ILCPU RR 1451ns*/
           0x80d9b069,  /*  84: HVD1_OLCPU RR 8553ns*/
           0x007de020,  /*  85: HVD1_CAB 4667ns*/
           0x0071a01d,  /*  86: HVD1_ILSI 4214ns*/
           0xbffff0ff,  /*  87: SID RR*/
           0x80000087,  /*  88: HVD2_DBLK_0 RR 0ns*/
           0x80000088,  /*  89: HVD2_DBLK_1 RR 0ns*/
           0x80270056,  /*  90: HVD2_ILCPU RR 1451ns*/
           0x8070a05c,  /*  91: HVD2_OLCPU RR 4427ns*/
           0x0071401c,  /*  92: HVD2_CAB 4200ns*/
           0x0072d01e,  /*  93: HVD2_ILSI 4258ns*/
           0x3ffff016,  /*  94: BVN_MAD_PIX_FD off*/
           0x3ffff024,  /*  95: BVN_MAD_QUANT off*/
           0x3ffff02d,  /*  96: BVN_MAD_PIX_CAP off*/
           0x3ffff0ff,  /*  97: UNASSIGNED off*/
           0x3ffff0ff,  /*  98: UNASSIGNED off*/
           0x3ffff0ff,  /*  99: UNASSIGNED off*/
           0x3ffff0ff,  /* 100: UNASSIGNED off*/
           0x3ffff0ff,  /* 101: UNASSIGNED off*/
           0x3ffff0ff,  /* 102: UNASSIGNED off*/
           0x008e1025,  /* 103: BVN_MAD3_PIX_FD 5266.5ns*/
           0x00c7d035,  /* 104: BVN_MAD3_QUANT 7407ns*/
           0x011c4039,  /* 105: BVN_MAD3_PIX_CAP 10534.5ns*/
           0x3ffff00b,  /* 106: BVN_MFD0 off*/
           0x3ffff002,  /* 107: BVN_MFD0_1 off*/
           0x3ffff021,  /* 108: BVN_MFD1 off*/
           0x3ffff012,  /* 109: BVN_MFD1_1 off*/
           0x3ffff0ff,  /* 110: UNASSIGNED off*/
           0x3ffff0ff,  /* 111: UNASSIGNED off*/
           0x3ffff0ff,  /* 112: UNASSIGNED off*/
           0x3ffff0ff,  /* 113: UNASSIGNED off*/
           0x00853022,  /* 114: BVN_MFD4 4938ns*/
           0x0058c013,  /* 115: BVN_MFD4_1 3292ns*/
           0x00853023,  /* 116: BVN_MFD5 4938ns*/
           0x0058c014,  /* 117: BVN_MFD5_1 3292ns*/
           0x3ffff006,  /* 118: BVN_VFD0 off*/
           0x3ffff007,  /* 119: BVN_VFD1 off*/
           0x3ffff046,  /* 120: BVN_VFD2 off*/
           0x3ffff047,  /* 121: BVN_VFD3 off*/
           0x3ffff0ff,  /* 122: UNASSIGNED off*/
           0x3ffff0ff,  /* 123: UNASSIGNED off*/
           0x0095d027,  /* 124: BVN_VFD6 5555ns*/
           0x0095d028,  /* 125: BVN_VFD7 5555ns*/
           0x3ffff003,  /* 126: BVN_CAP0 off*/
           0x3ffff004,  /* 127: BVN_CAP1 off*/
           0x3ffff03c,  /* 128: BVN_CAP2 off*/
           0x3ffff03d,  /* 129: BVN_CAP3 off*/
           0x3ffff0ff,  /* 130: UNASSIGNED off*/
           0x3ffff0ff,  /* 131: UNASSIGNED off*/
           0x0095d029,  /* 132: BVN_CAP6 5555ns*/
           0x0095d02a,  /* 133: BVN_CAP7 5555ns*/
           0x3ffff008,  /* 134: BVN_GFD0 off*/
           0x3ffff03b,  /* 135: BVN_GFD1 off*/
           0x3ffff0ff,  /* 136: UNASSIGNED off*/
           0x3ffff0ff,  /* 137: UNASSIGNED off*/
           0x3ffff0ff,  /* 138: UNASSIGNED off*/
           0x3ffff0ff,  /* 139: UNASSIGNED off*/
           0x3ffff0ff,  /* 140: UNASSIGNED off*/
           0x3ffff00f,  /* 141: BVN_MCVP0 off*/
           0x3ffff00e,  /* 142: BVN_MCVP1 off*/
           0x3ffff038,  /* 143: BVN_MCVP2 off*/
           0x3ffff011,  /* 144: BVN_RDC off*/
           0x03526044,  /* 145: VEC_VBI_ENC0 31500ns*/
           0x03526045,  /* 146: VEC_VBI_ENC1 31500ns*/
           0xbffff0ff,  /* 147: M2MC_0 RR*/
           0xbffff0ff,  /* 148: M2MC_1 RR*/
           0xbffff0ff,  /* 149: M2MC_2 RR*/
           0x3ffff0ff,  /* 150: UNASSIGNED off*/
           0x80c7d033,  /* 151: VICE_VIP0_INST2 RR 7406ns*/
           0x82574043,  /* 152: VICE_VIP1_INST2 RR 22200ns*/
           0x8063d019,  /* 153: VICE1_VIP0_INST2 RR 3703ns*/
           0x80c7a030,  /* 154: VICE1_VIP1_INST2 RR 7400ns*/
           0x3ffff081,  /* 155: HVD0_DBLK_p2_0 off*/
           0x3ffff082,  /* 156: HVD0_DBLK_p2_1 off*/
           0x008e1026,  /* 157: BVN_MAD4_PIX_FD 5266.5ns*/
           0x00c7d036,  /* 158: BVN_MAD4_QUANT 7407ns*/
           0x011c403a,  /* 159: BVN_MAD4_PIX_CAP 10534.5ns*/
           0xbffff0ff,  /* 160: M2MC1_0 RR*/
           0xbffff0ff,  /* 161: M2MC1_1 RR*/
           0xbffff0ff,  /* 162: M2MC1_2 RR*/
           0x3ffff0ff,  /* 163: UNASSIGNED off*/
           0x3ffff0ff,  /* 164: UNASSIGNED off*/
           0x3ffff0ff,  /* 165: UNASSIGNED off*/
           0x3ffff0ff,  /* 166: UNASSIGNED off*/
           0x3ffff0ff,  /* 167: UNASSIGNED off*/
           0x3ffff0ff,  /* 168: UNASSIGNED off*/
           0x3ffff0ff,  /* 169: UNASSIGNED off*/
           0x3ffff0ff,  /* 170: UNASSIGNED off*/
           0x3ffff0ff,  /* 171: UNASSIGNED off*/
           0x3ffff0ff,  /* 172: UNASSIGNED off*/
           0x3ffff0ff,  /* 173: UNASSIGNED off*/
           0x3ffff0ff,  /* 174: UNASSIGNED off*/
           0x3ffff0ff,  /* 175: UNASSIGNED off*/
           0x3ffff0ff,  /* 176: UNASSIGNED off*/
           0x3ffff0ff,  /* 177: UNASSIGNED off*/
           0x3ffff0ff,  /* 178: UNASSIGNED off*/
           0x3ffff0ff,  /* 179: UNASSIGNED off*/
           0x3ffff0ff,  /* 180: UNASSIGNED off*/
           0x3ffff0ff,  /* 181: UNASSIGNED off*/
           0x3ffff0ff,  /* 182: UNASSIGNED off*/
           0x3ffff0ff,  /* 183: UNASSIGNED off*/
           0x3ffff0ff,  /* 184: UNASSIGNED off*/
           0x3ffff0ff,  /* 185: UNASSIGNED off*/
           0x3ffff0ff,  /* 186: UNASSIGNED off*/
           0x3ffff0ff,  /* 187: UNASSIGNED off*/
           0x3ffff0ff,  /* 188: UNASSIGNED off*/
           0x3ffff0ff,  /* 189: UNASSIGNED off*/
           0x3ffff0ff,  /* 190: UNASSIGNED off*/
           0x3ffff0ff,  /* 191: UNASSIGNED off*/
           0x3ffff0ff,  /* 192: UNASSIGNED off*/
           0x3ffff0ff,  /* 193: UNASSIGNED off*/
           0x3ffff0ff,  /* 194: UNASSIGNED off*/
           0x3ffff0ff,  /* 195: UNASSIGNED off*/
           0x3ffff0ff,  /* 196: UNASSIGNED off*/
           0x3ffff0ff,  /* 197: UNASSIGNED off*/
           0x3ffff0ff,  /* 198: UNASSIGNED off*/
           0x3ffff0ff,  /* 199: UNASSIGNED off*/
           0x80142052,  /* 200: CPU_MCP_RD_HIGH RR 750ns*/
           0x80000095,  /* 201: CPU_MCP_RD_LOW RR*/
           0x80261054,  /* 202: CPU_MCP_WR_HIGH RR 1500ns*/
           0x80000096,  /* 203: CPU_MCP_WR_LOW RR*/
           0xbffff0ff,  /* 204: V3D_MCP_RD_HIGH RR*/
           0xbffff0ff,  /* 205: V3D_MCP_RD_LOW RR*/
           0xbffff0ff,  /* 206: V3D_MCP_WR_HIGH RR*/
           0xbffff0ff,  /* 207: V3D_MCP_WR_LOW RR*/
           0x3ffff0ff,  /* 208: UNASSIGNED off*/
           0x3ffff0ff,  /* 209: UNASSIGNED off*/
           0x3ffff0ff,  /* 210: UNASSIGNED off*/
           0x3ffff0ff,  /* 211: UNASSIGNED off*/
           0x3ffff0ff,  /* 212: UNASSIGNED off*/
           0x3ffff0ff,  /* 213: UNASSIGNED off*/
           0x3ffff0ff,  /* 214: UNASSIGNED off*/
           0x3ffff0ff,  /* 215: UNASSIGNED off*/
           0x3ffff089,  /* 216: HVD0_PFRI off*/
           0x3ffff08e,  /* 217: HVD1_PFRI off*/
           0x8000008b,  /* 218: HVD2_PFRI RR 0ns*/
           0x8000208c,  /* 219: VICE_PFRI RR 10ns*/
           0x8000208d,  /* 220: VICE1_PFRI RR 10ns*/
           0x3ffff0ff,  /* 221: UNASSIGNED off*/
           0x3ffff0ff,  /* 222: UNASSIGNED off*/
           0x3ffff0ff,  /* 223: UNASSIGNED off*/
           0x3ffff0ff,  /* 224: UNASSIGNED off*/
           0x3ffff0ff,  /* 225: UNASSIGNED off*/
           0x3ffff0ff,  /* 226: UNASSIGNED off*/
           0x3ffff0ff,  /* 227: UNASSIGNED off*/
           0x3ffff0ff,  /* 228: UNASSIGNED off*/
           0x3ffff0ff,  /* 229: UNASSIGNED off*/
           0x3ffff0ff,  /* 230: UNASSIGNED off*/
           0x3ffff0ff,  /* 231: UNASSIGNED off*/
           0x3ffff0ff,  /* 232: UNASSIGNED off*/
           0x3ffff0ff,  /* 233: UNASSIGNED off*/
           0x3ffff0ff,  /* 234: UNASSIGNED off*/
           0x3ffff0ff,  /* 235: UNASSIGNED off*/
           0x3ffff0ff,  /* 236: UNASSIGNED off*/
           0x3ffff0ff,  /* 237: UNASSIGNED off*/
           0x3ffff0ff,  /* 238: UNASSIGNED off*/
           0x3ffff0ff,  /* 239: UNASSIGNED off*/
           0x3ffff0ff,  /* 240: UNASSIGNED off*/
           0x3ffff0ff,  /* 241: UNASSIGNED off*/
           0x3ffff0ff,  /* 242: UNASSIGNED off*/
           0x3ffff0ff,  /* 243: UNASSIGNED off*/
           0x3ffff0ff,  /* 244: UNASSIGNED off*/
           0x3ffff0ff,  /* 245: UNASSIGNED off*/
           0x3ffff0ff,  /* 246: UNASSIGNED off*/
           0x3ffff0ff,  /* 247: UNASSIGNED off*/
           0xbffff0ff,  /* 248: MEMC_TRACELOG RR*/
           0x3ffff0ff,  /* 249: UNASSIGNED off*/
           0xbfffe07a,  /* 250: MEMC_ZQCS RR 1000000ns*/
           0xbffff0ff,  /* 251: MEMC_MSA RR*/
           0xbffff0ff,  /* 252: MEMC_DIS0 RR*/
           0xbffff0ff,  /* 253: MEMC_DIS1 RR*/
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR*/
           0x00d2d037,  /* 255: REFRESH 7812.5ns*/
         };
static const uint32_t aulMemc1_20150623234414_7445D0[] = {
           0x3ffff005,  /*   0: XPT_WR_RS off*/
           0x3ffff057,  /*   1: XPT_WR_XC off*/
           0x3ffff00d,  /*   2: XPT_WR_CDB off*/
           0x3ffff067,  /*   3: XPT_WR_ITB_MSG off*/
           0x3ffff01f,  /*   4: XPT_RD_RS off*/
           0x3ffff03e,  /*   5: XPT_RD_XC_RMX_MSG off*/
           0x3ffff00c,  /*   6: XPT_RD_XC_RAVE off*/
           0x3ffff060,  /*   7: XPT_RD_PB off*/
           0x80fd706a,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns*/
           0x82438074,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns*/
           0x803fd00a,  /*  10: SYSPORT_WR RR 2370ns*/
           0x80ae9063,  /*  11: SYSPORT_RD RR 6860ns*/
           0x3ffff0ff,  /*  12: UNASSIGNED off*/
           0x3ffff0ff,  /*  13: UNASSIGNED off*/
           0x3ffff0ff,  /*  14: UNASSIGNED off*/
           0x81617070,  /*  15: HIF_PCIe1 RR 13880ns*/
           0x8596e04f,  /*  16: MOCA_MIPS RR 53000ns*/
           0x8066205a,  /*  17: SATA RR 4015.6862745098ns*/
           0x8066205b,  /*  18: SATA_1 RR 4015.6862745098ns*/
           0x3ffff04a,  /*  19: MCIF2_RD off*/
           0x3ffff04c,  /*  20: MCIF2_WR off*/
           0x3ffff0ff,  /*  21: UNASSIGNED off*/
           0x8545e04e,  /*  22: BSP RR 50000ns*/
           0x80ad9061,  /*  23: SAGE RR 6820ns*/
           0x86449077,  /*  24: FLASH_DMA RR 63000ns*/
           0x8161706f,  /*  25: HIF_PCIe RR 13880ns*/
           0x86449079,  /*  26: SDIO_EMMC RR 63000ns*/
           0x86449078,  /*  27: SDIO_CARD RR 63000ns*/
           0x3ffff0ff,  /*  28: TPCAP off*/
           0x3ffff04b,  /*  29: MCIF_RD off*/
           0x3ffff04d,  /*  30: MCIF_WR off*/
           0x3ffff0ff,  /*  31: UART_DMA_RD off*/
           0x3ffff0ff,  /*  32: UART_DMA_WR off*/
           0x810db06b,  /*  33: USB_HI_0 RR 10593ns*/
           0xbffff0ff,  /*  34: USB_LO_0 RR*/
           0x815c506e,  /*  35: USB_X_WRITE_0 RR 13680ns*/
           0x815c506d,  /*  36: USB_X_READ_0 RR 13680ns*/
           0x80ae1062,  /*  37: USB_X_CTRL_0 RR 6840ns*/
           0x810db06c,  /*  38: USB_HI_1 RR 10593ns*/
           0xbffff0ff,  /*  39: USB_LO_1 RR*/
           0x3ffff010,  /*  40: RAAGA off*/
           0x3ffff001,  /*  41: RAAGA_1 off*/
           0x3ffff02b,  /*  42: RAAGA1 off*/
           0x3ffff000,  /*  43: RAAGA1_1 off*/
           0x3ffff02c,  /*  44: AUD_AIO off*/
           0x3ffff05d,  /*  45: VICE_CME_RMB_CMB off*/
           0x3ffff076,  /*  46: VICE_CME_CSC off*/
           0x3ffff068,  /*  47: VICE_FME_CSC off*/
           0x3ffff072,  /*  48: VICE_FME_Luma_CMB off*/
           0x3ffff071,  /*  49: VICE_FME_Chroma_CMB off*/
           0x3ffff040,  /*  50: VICE_SG off*/
           0x3ffff07c,  /*  51: VICE_DBLK off*/
           0x3ffff049,  /*  52: VICE_CABAC0 off*/
           0x3ffff051,  /*  53: VICE_CABAC1 off*/
           0x3ffff05f,  /*  54: VICE_ARCSS0 off*/
           0x3ffff031,  /*  55: VICE_VIP0_INST0 off*/
           0x3ffff041,  /*  56: VICE_VIP1_INST0 off*/
           0x3ffff032,  /*  57: VICE_VIP0_INST1 off*/
           0x3ffff042,  /*  58: VICE_VIP1_INST1 off*/
           0x3ffff059,  /*  59: VICE1_CME_RMB_CMB off*/
           0x3ffff075,  /*  60: VICE1_CME_CSC off*/
           0x3ffff064,  /*  61: VICE1_FME_CSC off*/
           0x3ffff066,  /*  62: VICE1_FME_Luma_CMB off*/
           0x3ffff065,  /*  63: VICE1_FME_Chroma_CMB off*/
           0x3ffff03f,  /*  64: VICE1_SG off*/
           0x3ffff07b,  /*  65: VICE1_DBLK off*/
           0x3ffff048,  /*  66: VICE1_CABAC0 off*/
           0x3ffff050,  /*  67: VICE1_CABAC1 off*/
           0x3ffff05e,  /*  68: VICE1_ARCSS0 off*/
           0x3ffff017,  /*  69: VICE1_VIP0_INST0 off*/
           0x3ffff02e,  /*  70: VICE1_VIP1_INST0 off*/
           0x3ffff018,  /*  71: VICE1_VIP0_INST1 off*/
           0x3ffff02f,  /*  72: VICE1_VIP1_INST1 off*/
           0x8000007f,  /*  73: HVD0_DBLK_Ch_0 RR 0ns*/
           0x80000080,  /*  74: HVD0_DBLK_Ch_1 RR 0ns*/
           0x3ffff053,  /*  75: HVD0_ILCPU off*/
           0x3ffff058,  /*  76: HVD0_OLCPU off*/
           0x3ffff015,  /*  77: HVD0_CAB off*/
           0x3ffff01a,  /*  78: HVD0_ILSI off*/
           0x81ea3073,  /*  79: HVD0_ILCPU_p2 RR 18162ns*/
           0x3ffff01b,  /*  80: HVD0_ILSI_p2 off*/
           0x80000085,  /*  81: HVD1_DBLK_0 RR 0ns*/
           0x80000086,  /*  82: HVD1_DBLK_1 RR 0ns*/
           0x3ffff055,  /*  83: HVD1_ILCPU off*/
           0x3ffff069,  /*  84: HVD1_OLCPU off*/
           0x3ffff020,  /*  85: HVD1_CAB off*/
           0x3ffff01d,  /*  86: HVD1_ILSI off*/
           0xbffff0ff,  /*  87: SID RR*/
           0x3ffff087,  /*  88: HVD2_DBLK_0 off*/
           0x3ffff088,  /*  89: HVD2_DBLK_1 off*/
           0x3ffff056,  /*  90: HVD2_ILCPU off*/
           0x3ffff05c,  /*  91: HVD2_OLCPU off*/
           0x3ffff01c,  /*  92: HVD2_CAB off*/
           0x3ffff01e,  /*  93: HVD2_ILSI off*/
           0x005ea016,  /*  94: BVN_MAD_PIX_FD 3511ns*/
           0x00853024,  /*  95: BVN_MAD_QUANT 4938ns*/
           0x00bd702d,  /*  96: BVN_MAD_PIX_CAP 7023ns*/
           0x3ffff0ff,  /*  97: UNASSIGNED off*/
           0x3ffff0ff,  /*  98: UNASSIGNED off*/
           0x3ffff0ff,  /*  99: UNASSIGNED off*/
           0x3ffff0ff,  /* 100: UNASSIGNED off*/
           0x3ffff0ff,  /* 101: UNASSIGNED off*/
           0x3ffff0ff,  /* 102: UNASSIGNED off*/
           0x3ffff025,  /* 103: BVN_MAD3_PIX_FD off*/
           0x3ffff035,  /* 104: BVN_MAD3_QUANT off*/
           0x3ffff039,  /* 105: BVN_MAD3_PIX_CAP off*/
           0x00c7d034,  /* 106: BVN_MFD0_Ch 7407ns*/
           0x0038f009,  /* 107: BVN_MFD0_Ch_1 2115ns*/
           0x00853021,  /* 108: BVN_MFD1 4938ns*/
           0x0058c012,  /* 109: BVN_MFD1_1 3292ns*/
           0x3ffff0ff,  /* 110: UNASSIGNED off*/
           0x3ffff0ff,  /* 111: UNASSIGNED off*/
           0x3ffff0ff,  /* 112: UNASSIGNED off*/
           0x3ffff0ff,  /* 113: UNASSIGNED off*/
           0x3ffff022,  /* 114: BVN_MFD4 off*/
           0x3ffff013,  /* 115: BVN_MFD4_1 off*/
           0x3ffff023,  /* 116: BVN_MFD5 off*/
           0x3ffff014,  /* 117: BVN_MFD5_1 off*/
           0x001fd006,  /* 118: BVN_VFD0 1185ns*/
           0x001fd007,  /* 119: BVN_VFD1 1185ns*/
           0x3ffff046,  /* 120: BVN_VFD2 off*/
           0x3ffff047,  /* 121: BVN_VFD3 off*/
           0x3ffff0ff,  /* 122: UNASSIGNED off*/
           0x3ffff0ff,  /* 123: UNASSIGNED off*/
           0x3ffff027,  /* 124: BVN_VFD6 off*/
           0x3ffff028,  /* 125: BVN_VFD7 off*/
           0x001e5003,  /* 126: BVN_CAP0 1128.594ns*/
           0x001e5004,  /* 127: BVN_CAP1 1128.594ns*/
           0x3ffff03c,  /* 128: BVN_CAP2 off*/
           0x3ffff03d,  /* 129: BVN_CAP3 off*/
           0x3ffff0ff,  /* 130: UNASSIGNED off*/
           0x3ffff0ff,  /* 131: UNASSIGNED off*/
           0x3ffff029,  /* 132: BVN_CAP6 off*/
           0x3ffff02a,  /* 133: BVN_CAP7 off*/
           0x3ffff008,  /* 134: BVN_GFD0 off*/
           0x3ffff03b,  /* 135: BVN_GFD1 off*/
           0x3ffff0ff,  /* 136: UNASSIGNED off*/
           0x3ffff0ff,  /* 137: UNASSIGNED off*/
           0x3ffff0ff,  /* 138: UNASSIGNED off*/
           0x3ffff0ff,  /* 139: UNASSIGNED off*/
           0x3ffff0ff,  /* 140: UNASSIGNED off*/
           0x004b500f,  /* 141: BVN_MCVP0 2794ns*/
           0x004b500e,  /* 142: BVN_MCVP1 2794ns*/
           0x00d3d038,  /* 143: BVN_MCVP2 7850ns*/
           0x3ffff011,  /* 144: BVN_RDC off*/
           0x3ffff044,  /* 145: VEC_VBI_ENC0 off*/
           0x3ffff045,  /* 146: VEC_VBI_ENC1 off*/
           0xbffff0ff,  /* 147: M2MC_0 RR*/
           0xbffff0ff,  /* 148: M2MC_1 RR*/
           0xbffff0ff,  /* 149: M2MC_2 RR*/
           0x3ffff0ff,  /* 150: UNASSIGNED off*/
           0x3ffff033,  /* 151: VICE_VIP0_INST2 off*/
           0x3ffff043,  /* 152: VICE_VIP1_INST2 off*/
           0x3ffff019,  /* 153: VICE1_VIP0_INST2 off*/
           0x3ffff030,  /* 154: VICE1_VIP1_INST2 off*/
           0x80000083,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns*/
           0x80000084,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns*/
           0x3ffff026,  /* 157: BVN_MAD4_PIX_FD off*/
           0x3ffff036,  /* 158: BVN_MAD4_QUANT off*/
           0x3ffff03a,  /* 159: BVN_MAD4_PIX_CAP off*/
           0xbffff0ff,  /* 160: M2MC1_0 RR*/
           0xbffff0ff,  /* 161: M2MC1_1 RR*/
           0xbffff0ff,  /* 162: M2MC1_2 RR*/
           0x3ffff0ff,  /* 163: UNASSIGNED off*/
           0x3ffff0ff,  /* 164: UNASSIGNED off*/
           0x3ffff0ff,  /* 165: UNASSIGNED off*/
           0x3ffff0ff,  /* 166: UNASSIGNED off*/
           0x3ffff0ff,  /* 167: UNASSIGNED off*/
           0x3ffff0ff,  /* 168: UNASSIGNED off*/
           0x3ffff0ff,  /* 169: UNASSIGNED off*/
           0x3ffff0ff,  /* 170: UNASSIGNED off*/
           0x3ffff0ff,  /* 171: UNASSIGNED off*/
           0x3ffff0ff,  /* 172: UNASSIGNED off*/
           0x3ffff0ff,  /* 173: UNASSIGNED off*/
           0x3ffff0ff,  /* 174: UNASSIGNED off*/
           0x3ffff0ff,  /* 175: UNASSIGNED off*/
           0x3ffff0ff,  /* 176: UNASSIGNED off*/
           0x3ffff0ff,  /* 177: UNASSIGNED off*/
           0x3ffff0ff,  /* 178: UNASSIGNED off*/
           0x3ffff0ff,  /* 179: UNASSIGNED off*/
           0x3ffff0ff,  /* 180: UNASSIGNED off*/
           0x3ffff0ff,  /* 181: UNASSIGNED off*/
           0x3ffff0ff,  /* 182: UNASSIGNED off*/
           0x3ffff0ff,  /* 183: UNASSIGNED off*/
           0x3ffff0ff,  /* 184: UNASSIGNED off*/
           0x3ffff0ff,  /* 185: UNASSIGNED off*/
           0x3ffff0ff,  /* 186: UNASSIGNED off*/
           0x3ffff0ff,  /* 187: UNASSIGNED off*/
           0x3ffff0ff,  /* 188: UNASSIGNED off*/
           0x3ffff0ff,  /* 189: UNASSIGNED off*/
           0x3ffff0ff,  /* 190: UNASSIGNED off*/
           0x3ffff0ff,  /* 191: UNASSIGNED off*/
           0x3ffff0ff,  /* 192: UNASSIGNED off*/
           0x3ffff0ff,  /* 193: UNASSIGNED off*/
           0x3ffff0ff,  /* 194: UNASSIGNED off*/
           0x3ffff0ff,  /* 195: UNASSIGNED off*/
           0x3ffff0ff,  /* 196: UNASSIGNED off*/
           0x3ffff0ff,  /* 197: UNASSIGNED off*/
           0x3ffff0ff,  /* 198: UNASSIGNED off*/
           0x3ffff0ff,  /* 199: UNASSIGNED off*/
           0x80142052,  /* 200: CPU_MCP_RD_HIGH RR 750ns*/
           0x80000095,  /* 201: CPU_MCP_RD_LOW RR*/
           0x80261054,  /* 202: CPU_MCP_WR_HIGH RR 1500ns*/
           0x80000096,  /* 203: CPU_MCP_WR_LOW RR*/
           0xbffff0ff,  /* 204: V3D_MCP_RD_HIGH RR*/
           0xbffff0ff,  /* 205: V3D_MCP_RD_LOW RR*/
           0xbffff0ff,  /* 206: V3D_MCP_WR_HIGH RR*/
           0xbffff0ff,  /* 207: V3D_MCP_WR_LOW RR*/
           0x3ffff0ff,  /* 208: UNASSIGNED off*/
           0x3ffff0ff,  /* 209: UNASSIGNED off*/
           0x3ffff0ff,  /* 210: UNASSIGNED off*/
           0x3ffff0ff,  /* 211: UNASSIGNED off*/
           0x3ffff0ff,  /* 212: UNASSIGNED off*/
           0x3ffff0ff,  /* 213: UNASSIGNED off*/
           0x3ffff0ff,  /* 214: UNASSIGNED off*/
           0x3ffff0ff,  /* 215: UNASSIGNED off*/
           0x8000008a,  /* 216: HVD0_PFRI_Ch RR 0ns*/
           0x8000008e,  /* 217: HVD1_PFRI RR 0ns*/
           0x3ffff08b,  /* 218: HVD2_PFRI off*/
           0x3ffff08c,  /* 219: VICE_PFRI off*/
           0x3ffff08d,  /* 220: VICE1_PFRI off*/
           0x3ffff0ff,  /* 221: UNASSIGNED off*/
           0x3ffff0ff,  /* 222: UNASSIGNED off*/
           0x3ffff0ff,  /* 223: UNASSIGNED off*/
           0x3ffff0ff,  /* 224: UNASSIGNED off*/
           0x3ffff0ff,  /* 225: UNASSIGNED off*/
           0x3ffff0ff,  /* 226: UNASSIGNED off*/
           0x3ffff0ff,  /* 227: UNASSIGNED off*/
           0x3ffff0ff,  /* 228: UNASSIGNED off*/
           0x3ffff0ff,  /* 229: UNASSIGNED off*/
           0x3ffff0ff,  /* 230: UNASSIGNED off*/
           0x3ffff0ff,  /* 231: UNASSIGNED off*/
           0x3ffff0ff,  /* 232: UNASSIGNED off*/
           0x3ffff0ff,  /* 233: UNASSIGNED off*/
           0x3ffff0ff,  /* 234: UNASSIGNED off*/
           0x3ffff0ff,  /* 235: UNASSIGNED off*/
           0x3ffff0ff,  /* 236: UNASSIGNED off*/
           0x3ffff0ff,  /* 237: UNASSIGNED off*/
           0x3ffff0ff,  /* 238: UNASSIGNED off*/
           0x3ffff0ff,  /* 239: UNASSIGNED off*/
           0x3ffff0ff,  /* 240: UNASSIGNED off*/
           0x3ffff0ff,  /* 241: UNASSIGNED off*/
           0x3ffff0ff,  /* 242: UNASSIGNED off*/
           0x3ffff0ff,  /* 243: UNASSIGNED off*/
           0x3ffff0ff,  /* 244: UNASSIGNED off*/
           0x3ffff0ff,  /* 245: UNASSIGNED off*/
           0x3ffff0ff,  /* 246: UNASSIGNED off*/
           0x3ffff0ff,  /* 247: UNASSIGNED off*/
           0xbffff0ff,  /* 248: MEMC_TRACELOG RR*/
           0x3ffff0ff,  /* 249: UNASSIGNED off*/
           0xbfffe07a,  /* 250: MEMC_ZQCS RR 1000000ns*/
           0xbffff0ff,  /* 251: MEMC_MSA RR*/
           0xbffff0ff,  /* 252: MEMC_DIS0 RR*/
           0xbffff0ff,  /* 253: MEMC_DIS1 RR*/
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR*/
           0x00d2d037,  /* 255: REFRESH 7812.5ns*/
         };
static const uint32_t aulMemc2_20150623234414_7445D0[] = {
            0x3ffff005,  /*   0: XPT_WR_RS off*/
           0x3ffff057,  /*   1: XPT_WR_XC off*/
           0x3ffff00d,  /*   2: XPT_WR_CDB off*/
           0x3ffff067,  /*   3: XPT_WR_ITB_MSG off*/
           0x3ffff01f,  /*   4: XPT_RD_RS off*/
           0x3ffff03e,  /*   5: XPT_RD_XC_RMX_MSG off*/
           0x3ffff00c,  /*   6: XPT_RD_XC_RAVE off*/
           0x3ffff060,  /*   7: XPT_RD_PB off*/
           0x80fd706a,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns*/
           0x82438074,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns*/
           0x803fd00a,  /*  10: SYSPORT_WR RR 2370ns*/
           0x80ae9063,  /*  11: SYSPORT_RD RR 6860ns*/
           0x3ffff0ff,  /*  12: UNASSIGNED off*/
           0x3ffff0ff,  /*  13: UNASSIGNED off*/
           0x3ffff0ff,  /*  14: UNASSIGNED off*/
           0x81617070,  /*  15: HIF_PCIe1 RR 13880ns*/
           0x8596e04f,  /*  16: MOCA_MIPS RR 53000ns*/
           0x8066205a,  /*  17: SATA RR 4015.6862745098ns*/
           0x8066205b,  /*  18: SATA_1 RR 4015.6862745098ns*/
           0x3ffff04a,  /*  19: MCIF2_RD off*/
           0x3ffff04c,  /*  20: MCIF2_WR off*/
           0x3ffff0ff,  /*  21: UNASSIGNED off*/
           0x8545e04e,  /*  22: BSP RR 50000ns*/
           0x80ad9061,  /*  23: SAGE RR 6820ns*/
           0x86449077,  /*  24: FLASH_DMA RR 63000ns*/
           0x8161706f,  /*  25: HIF_PCIe RR 13880ns*/
           0x86449079,  /*  26: SDIO_EMMC RR 63000ns*/
           0x86449078,  /*  27: SDIO_CARD RR 63000ns*/
           0x3ffff0ff,  /*  28: TPCAP off*/
           0x3ffff04b,  /*  29: MCIF_RD off*/
           0x3ffff04d,  /*  30: MCIF_WR off*/
           0x3ffff0ff,  /*  31: UART_DMA_RD off*/
           0x3ffff0ff,  /*  32: UART_DMA_WR off*/
           0x810db06b,  /*  33: USB_HI_0 RR 10593ns*/
           0xbffff0ff,  /*  34: USB_LO_0 RR*/
           0x815c506e,  /*  35: USB_X_WRITE_0 RR 13680ns*/
           0x815c506d,  /*  36: USB_X_READ_0 RR 13680ns*/
           0x80ae1062,  /*  37: USB_X_CTRL_0 RR 6840ns*/
           0x810db06c,  /*  38: USB_HI_1 RR 10593ns*/
           0xbffff0ff,  /*  39: USB_LO_1 RR*/
           0x3ffff010,  /*  40: RAAGA off*/
           0x3ffff001,  /*  41: RAAGA_1 off*/
           0x3ffff02b,  /*  42: RAAGA1 off*/
           0x3ffff000,  /*  43: RAAGA1_1 off*/
           0x3ffff02c,  /*  44: AUD_AIO off*/
           0x3ffff05d,  /*  45: VICE_CME_RMB_CMB off*/
           0x3ffff076,  /*  46: VICE_CME_CSC off*/
           0x3ffff068,  /*  47: VICE_FME_CSC off*/
           0x3ffff072,  /*  48: VICE_FME_Luma_CMB off*/
           0x3ffff071,  /*  49: VICE_FME_Chroma_CMB off*/
           0x3ffff040,  /*  50: VICE_SG off*/
           0x3ffff07c,  /*  51: VICE_DBLK off*/
           0x3ffff049,  /*  52: VICE_CABAC0 off*/
           0x3ffff051,  /*  53: VICE_CABAC1 off*/
           0x3ffff05f,  /*  54: VICE_ARCSS0 off*/
           0x3ffff031,  /*  55: VICE_VIP0_INST0 off*/
           0x3ffff041,  /*  56: VICE_VIP1_INST0 off*/
           0x3ffff032,  /*  57: VICE_VIP0_INST1 off*/
           0x3ffff042,  /*  58: VICE_VIP1_INST1 off*/
           0x3ffff059,  /*  59: VICE1_CME_RMB_CMB off*/
           0x3ffff075,  /*  60: VICE1_CME_CSC off*/
           0x3ffff064,  /*  61: VICE1_FME_CSC off*/
           0x3ffff066,  /*  62: VICE1_FME_Luma_CMB off*/
           0x3ffff065,  /*  63: VICE1_FME_Chroma_CMB off*/
           0x3ffff03f,  /*  64: VICE1_SG off*/
           0x3ffff07b,  /*  65: VICE1_DBLK off*/
           0x3ffff048,  /*  66: VICE1_CABAC0 off*/
           0x3ffff050,  /*  67: VICE1_CABAC1 off*/
           0x3ffff05e,  /*  68: VICE1_ARCSS0 off*/
           0x3ffff017,  /*  69: VICE1_VIP0_INST0 off*/
           0x3ffff02e,  /*  70: VICE1_VIP1_INST0 off*/
           0x3ffff018,  /*  71: VICE1_VIP0_INST1 off*/
           0x3ffff02f,  /*  72: VICE1_VIP1_INST1 off*/
           0x8000007d,  /*  73: HVD0_DBLK_0 RR 0ns*/
           0x8000007e,  /*  74: HVD0_DBLK_1 RR 0ns*/
           0x3ffff053,  /*  75: HVD0_ILCPU off*/
           0x3ffff058,  /*  76: HVD0_OLCPU off*/
           0x3ffff015,  /*  77: HVD0_CAB off*/
           0x3ffff01a,  /*  78: HVD0_ILSI off*/
           0x81ea3073,  /*  79: HVD0_ILCPU_p2 RR 18162ns*/
           0x3ffff01b,  /*  80: HVD0_ILSI_p2 off*/
           0x3ffff085,  /*  81: HVD1_DBLK_0 off*/
           0x3ffff086,  /*  82: HVD1_DBLK_1 off*/
           0x3ffff055,  /*  83: HVD1_ILCPU off*/
           0x3ffff069,  /*  84: HVD1_OLCPU off*/
           0x3ffff020,  /*  85: HVD1_CAB off*/
           0x3ffff01d,  /*  86: HVD1_ILSI off*/
           0xbffff0ff,  /*  87: SID RR*/
           0x3ffff087,  /*  88: HVD2_DBLK_0 off*/
           0x3ffff088,  /*  89: HVD2_DBLK_1 off*/
           0x3ffff056,  /*  90: HVD2_ILCPU off*/
           0x3ffff05c,  /*  91: HVD2_OLCPU off*/
           0x3ffff01c,  /*  92: HVD2_CAB off*/
           0x3ffff01e,  /*  93: HVD2_ILSI off*/
           0x3ffff016,  /*  94: BVN_MAD_PIX_FD off*/
           0x3ffff024,  /*  95: BVN_MAD_QUANT off*/
           0x3ffff02d,  /*  96: BVN_MAD_PIX_CAP off*/
           0x3ffff0ff,  /*  97: UNASSIGNED off*/
           0x3ffff0ff,  /*  98: UNASSIGNED off*/
           0x3ffff0ff,  /*  99: UNASSIGNED off*/
           0x3ffff0ff,  /* 100: UNASSIGNED off*/
           0x3ffff0ff,  /* 101: UNASSIGNED off*/
           0x3ffff0ff,  /* 102: UNASSIGNED off*/
           0x3ffff025,  /* 103: BVN_MAD3_PIX_FD off*/
           0x3ffff035,  /* 104: BVN_MAD3_QUANT off*/
           0x3ffff039,  /* 105: BVN_MAD3_PIX_CAP off*/
           0x0042800b,  /* 106: BVN_MFD0 2469ns*/
           0x001c6002,  /* 107: BVN_MFD0_1 1057.5ns*/
           0x3ffff021,  /* 108: BVN_MFD1 off*/
           0x3ffff012,  /* 109: BVN_MFD1_1 off*/
           0x3ffff0ff,  /* 110: UNASSIGNED off*/
           0x3ffff0ff,  /* 111: UNASSIGNED off*/
           0x3ffff0ff,  /* 112: UNASSIGNED off*/
           0x3ffff0ff,  /* 113: UNASSIGNED off*/
           0x3ffff022,  /* 114: BVN_MFD4 off*/
           0x3ffff013,  /* 115: BVN_MFD4_1 off*/
           0x3ffff023,  /* 116: BVN_MFD5 off*/
           0x3ffff014,  /* 117: BVN_MFD5_1 off*/
           0x3ffff006,  /* 118: BVN_VFD0 off*/
           0x3ffff007,  /* 119: BVN_VFD1 off*/
           0x0359a046,  /* 120: BVN_VFD2 31770ns*/
           0x0359a047,  /* 121: BVN_VFD3 31770ns*/
           0x3ffff0ff,  /* 122: UNASSIGNED off*/
           0x3ffff0ff,  /* 123: UNASSIGNED off*/
           0x3ffff027,  /* 124: BVN_VFD6 off*/
           0x3ffff028,  /* 125: BVN_VFD7 off*/
           0x3ffff003,  /* 126: BVN_CAP0 off*/
           0x3ffff004,  /* 127: BVN_CAP1 off*/
           0x0174703c,  /* 128: BVN_CAP2 13800ns*/
           0x0174703d,  /* 129: BVN_CAP3 13800ns*/
           0x3ffff0ff,  /* 130: UNASSIGNED off*/
           0x3ffff0ff,  /* 131: UNASSIGNED off*/
           0x3ffff029,  /* 132: BVN_CAP6 off*/
           0x3ffff02a,  /* 133: BVN_CAP7 off*/
           0x0031d008,  /* 134: BVN_GFD0 1851ns*/
           0x0167c03b,  /* 135: BVN_GFD1 13330ns*/
           0x3ffff0ff,  /* 136: UNASSIGNED off*/
           0x3ffff0ff,  /* 137: UNASSIGNED off*/
           0x3ffff0ff,  /* 138: UNASSIGNED off*/
           0x3ffff0ff,  /* 139: UNASSIGNED off*/
           0x3ffff0ff,  /* 140: UNASSIGNED off*/
           0x3ffff00f,  /* 141: BVN_MCVP0 off*/
           0x3ffff00e,  /* 142: BVN_MCVP1 off*/
           0x3ffff038,  /* 143: BVN_MCVP2 off*/
           0x00571011,  /* 144: BVN_RDC 3230ns*/
           0x3ffff044,  /* 145: VEC_VBI_ENC0 off*/
           0x3ffff045,  /* 146: VEC_VBI_ENC1 off*/
           0xbffff0ff,  /* 147: M2MC_0 RR*/
           0xbffff0ff,  /* 148: M2MC_1 RR*/
           0xbffff0ff,  /* 149: M2MC_2 RR*/
           0x3ffff0ff,  /* 150: UNASSIGNED off*/
           0x3ffff033,  /* 151: VICE_VIP0_INST2 off*/
           0x3ffff043,  /* 152: VICE_VIP1_INST2 off*/
           0x3ffff019,  /* 153: VICE1_VIP0_INST2 off*/
           0x3ffff030,  /* 154: VICE1_VIP1_INST2 off*/
           0x80000081,  /* 155: HVD0_DBLK_p2_0 RR 0ns*/
           0x80000082,  /* 156: HVD0_DBLK_p2_1 RR 0ns*/
           0x3ffff026,  /* 157: BVN_MAD4_PIX_FD off*/
           0x3ffff036,  /* 158: BVN_MAD4_QUANT off*/
           0x3ffff03a,  /* 159: BVN_MAD4_PIX_CAP off*/
           0xbffff0ff,  /* 160: M2MC1_0 RR*/
           0xbffff0ff,  /* 161: M2MC1_1 RR*/
           0xbffff0ff,  /* 162: M2MC1_2 RR*/
           0x3ffff0ff,  /* 163: UNASSIGNED off*/
           0x3ffff0ff,  /* 164: UNASSIGNED off*/
           0x3ffff0ff,  /* 165: UNASSIGNED off*/
           0x3ffff0ff,  /* 166: UNASSIGNED off*/
           0x3ffff0ff,  /* 167: UNASSIGNED off*/
           0x3ffff0ff,  /* 168: UNASSIGNED off*/
           0x3ffff0ff,  /* 169: UNASSIGNED off*/
           0x3ffff0ff,  /* 170: UNASSIGNED off*/
           0x3ffff0ff,  /* 171: UNASSIGNED off*/
           0x3ffff0ff,  /* 172: UNASSIGNED off*/
           0x3ffff0ff,  /* 173: UNASSIGNED off*/
           0x3ffff0ff,  /* 174: UNASSIGNED off*/
           0x3ffff0ff,  /* 175: UNASSIGNED off*/
           0x3ffff0ff,  /* 176: UNASSIGNED off*/
           0x3ffff0ff,  /* 177: UNASSIGNED off*/
           0x3ffff0ff,  /* 178: UNASSIGNED off*/
           0x3ffff0ff,  /* 179: UNASSIGNED off*/
           0x3ffff0ff,  /* 180: UNASSIGNED off*/
           0x3ffff0ff,  /* 181: UNASSIGNED off*/
           0x3ffff0ff,  /* 182: UNASSIGNED off*/
           0x3ffff0ff,  /* 183: UNASSIGNED off*/
           0x3ffff0ff,  /* 184: UNASSIGNED off*/
           0x3ffff0ff,  /* 185: UNASSIGNED off*/
           0x3ffff0ff,  /* 186: UNASSIGNED off*/
           0x3ffff0ff,  /* 187: UNASSIGNED off*/
           0x3ffff0ff,  /* 188: UNASSIGNED off*/
           0x3ffff0ff,  /* 189: UNASSIGNED off*/
           0x3ffff0ff,  /* 190: UNASSIGNED off*/
           0x3ffff0ff,  /* 191: UNASSIGNED off*/
           0x3ffff0ff,  /* 192: UNASSIGNED off*/
           0x3ffff0ff,  /* 193: UNASSIGNED off*/
           0x3ffff0ff,  /* 194: UNASSIGNED off*/
           0x3ffff0ff,  /* 195: UNASSIGNED off*/
           0x3ffff0ff,  /* 196: UNASSIGNED off*/
           0x3ffff0ff,  /* 197: UNASSIGNED off*/
           0x3ffff0ff,  /* 198: UNASSIGNED off*/
           0x3ffff0ff,  /* 199: UNASSIGNED off*/
           0x80142052,  /* 200: CPU_MCP_RD_HIGH RR 750ns*/
           0x80000095,  /* 201: CPU_MCP_RD_LOW RR*/
           0x80261054,  /* 202: CPU_MCP_WR_HIGH RR 1500ns*/
           0x80000096,  /* 203: CPU_MCP_WR_LOW RR*/
           0xbffff0ff,  /* 204: V3D_MCP_RD_HIGH RR*/
           0xbffff0ff,  /* 205: V3D_MCP_RD_LOW RR*/
           0xbffff0ff,  /* 206: V3D_MCP_WR_HIGH RR*/
           0xbffff0ff,  /* 207: V3D_MCP_WR_LOW RR*/
           0x3ffff0ff,  /* 208: UNASSIGNED off*/
           0x3ffff0ff,  /* 209: UNASSIGNED off*/
           0x3ffff0ff,  /* 210: UNASSIGNED off*/
           0x3ffff0ff,  /* 211: UNASSIGNED off*/
           0x3ffff0ff,  /* 212: UNASSIGNED off*/
           0x3ffff0ff,  /* 213: UNASSIGNED off*/
           0x3ffff0ff,  /* 214: UNASSIGNED off*/
           0x3ffff0ff,  /* 215: UNASSIGNED off*/
           0x80000089,  /* 216: HVD0_PFRI RR 0ns*/
           0x3ffff08e,  /* 217: HVD1_PFRI off*/
           0x3ffff08b,  /* 218: HVD2_PFRI off*/
           0x3ffff08c,  /* 219: VICE_PFRI off*/
           0x3ffff08d,  /* 220: VICE1_PFRI off*/
           0x3ffff0ff,  /* 221: UNASSIGNED off*/
           0x3ffff0ff,  /* 222: UNASSIGNED off*/
           0x3ffff0ff,  /* 223: UNASSIGNED off*/
           0x3ffff0ff,  /* 224: UNASSIGNED off*/
           0x3ffff0ff,  /* 225: UNASSIGNED off*/
           0x3ffff0ff,  /* 226: UNASSIGNED off*/
           0x3ffff0ff,  /* 227: UNASSIGNED off*/
           0x3ffff0ff,  /* 228: UNASSIGNED off*/
           0x3ffff0ff,  /* 229: UNASSIGNED off*/
           0x3ffff0ff,  /* 230: UNASSIGNED off*/
           0x3ffff0ff,  /* 231: UNASSIGNED off*/
           0x3ffff0ff,  /* 232: UNASSIGNED off*/
           0x3ffff0ff,  /* 233: UNASSIGNED off*/
           0x3ffff0ff,  /* 234: UNASSIGNED off*/
           0x3ffff0ff,  /* 235: UNASSIGNED off*/
           0x3ffff0ff,  /* 236: UNASSIGNED off*/
           0x3ffff0ff,  /* 237: UNASSIGNED off*/
           0x3ffff0ff,  /* 238: UNASSIGNED off*/
           0x3ffff0ff,  /* 239: UNASSIGNED off*/
           0x3ffff0ff,  /* 240: UNASSIGNED off*/
           0x3ffff0ff,  /* 241: UNASSIGNED off*/
           0x3ffff0ff,  /* 242: UNASSIGNED off*/
           0x3ffff0ff,  /* 243: UNASSIGNED off*/
           0x3ffff0ff,  /* 244: UNASSIGNED off*/
           0x3ffff0ff,  /* 245: UNASSIGNED off*/
           0x3ffff0ff,  /* 246: UNASSIGNED off*/
           0x3ffff0ff,  /* 247: UNASSIGNED off*/
           0xbffff0ff,  /* 248: MEMC_TRACELOG RR*/
           0x3ffff0ff,  /* 249: UNASSIGNED off*/
           0xbfffe07a,  /* 250: MEMC_ZQCS RR 1000000ns*/
           0xbffff0ff,  /* 251: MEMC_MSA RR*/
           0xbffff0ff,  /* 252: MEMC_DIS0 RR*/
           0xbffff0ff,  /* 253: MEMC_DIS1 RR*/
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR*/
           0x00d2d037,  /* 255: REFRESH 7812.5ns*/
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20150623234414_7445D0[] = {
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x80910803}, /* HVD2_PFRI (gHvd2) 265333.33 ns/60 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x400001dd}, /* d: 4; p: 477.595833333333 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x00000a3a}, /* 2618 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x00000622}, /* 60% * 2618 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_CONFIG,      0x80920802}, /* VICE_PFRI (gVice) 333333.33 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_FILTER_CTRL, 0x400001c1}, /* d: 4; p: 449.996875 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH0,     0x000001a8}, /* 424 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH1,     0x000000fe}, /* 60% * 424 */
  {BCHP_MEMC_GEN_0_PFRI_4_THROTTLE_CONFIG,      0x80930802}, /* VICE1_PFRI (gVice1) 441176.47 ns/120 */
  {BCHP_MEMC_GEN_0_PFRI_4_THROTTLE_FILTER_CTRL, 0x4000018d}, /* d: 4; p: 397.058333333333 */
  {BCHP_MEMC_GEN_0_PFRI_4_THROTTLE_THRESH0,     0x000001a8}, /* 424 */
  {BCHP_MEMC_GEN_0_PFRI_4_THROTTLE_THRESH1,     0x000000fe}, /* 60% * 424 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80900803}, /* HVD0_PFRI_Ch (gHvdC0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000cfe}, /* 3326 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x000007cb}, /* 60% * 3326 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_CONFIG,      0x80940802}, /* HVD1_PFRI (gHvd1) 483840.00 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000366}, /* d: 4; p: 870.908333333333 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH0,     0x00000a6e}, /* 2670 */
  {BCHP_MEMC_GEN_1_PFRI_1_THROTTLE_THRESH1,     0x00000642}, /* 60% * 2670 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_CONFIG,      0x808f0803}, /* HVD0_PFRI (gHvd0) 233760.00 ns/40 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.15 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH0,     0x00001642}, /* 5698 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH1,     0x00000d5a}  /* 60% * 5698 */
};

static const uint32_t* const paulMemc_box1[] = { &aulMemc0_20150623234414_7445D0[0], &aulMemc1_20150623234414_7445D0[0], &aulMemc2_20150623234414_7445D0[0]};

const BBOX_Rts stBoxRts_7445D0_box1 = {
  "20150623234414_7445D0_box1",
  7445,
  1,
  3,
  256,
  (const uint32_t**)&paulMemc_box1[0],
  24,
  stBoxRts_PfriClient_20150623234414_7445D0
};
