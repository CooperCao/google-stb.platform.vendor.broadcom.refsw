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
*   at: Tue Jun 23 23:20:11 2015 GMT
*   by: robinc
*   for: Box 7439_4Kstb_hitemp
*         MemC 0 (32-bit DDR3@1066MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1066MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20150623232011_7439_4Kstb_hitemp[] = {
           0x003ce007,  /*   0: XPT_WR_RS 2260ns */
           0x8074804e,  /*   1: XPT_WR_XC RR 4580ns */
           0x80923025,  /*   2: XPT_WR_CDB RR 5420ns */
           0x8172305f,  /*   3: XPT_WR_ITB_MSG RR 14540ns */
           0x80f02031,  /*   4: XPT_RD_RS RR 8900ns */
           0x833cc039,  /*   5: XPT_RD_XC_RMX_MSG RR 30700ns */
           0x80923024,  /*   6: XPT_RD_XC_RAVE RR 5420ns */
           0x8145e05a,  /*   7: XPT_RD_PB RR 12800ns */
           0x813b2059,  /*   8: XPT_WR_MEMDMA RR 12376ns */
           0x82bce063,  /*   9: XPT_RD_MEMDMA RR 27520ns */
           0x803fd008,  /*  10: GENET0_WR RR 2370ns */
           0x81026055,  /*  11: GENET0_RD RR 10150ns */
           0x8089d023,  /*  12: GENET1_WR RR 5110ns */
           0x81026056,  /*  13: GENET1_RD RR 10150ns */
           0x803fd009,  /*  14: GENET2_WR RR 2370ns */
           0x81026057,  /*  15: GENET2_RD RR 10150ns */
           0x8596e044,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066204b,  /*  17: SATA RR 4015ns */
           0x8066204c,  /*  18: SATA_1 RR 4015ns */
           0x03e6e03f,  /*  19: MCIF2_RD 37000ns */
           0x03e6e041,  /*  20: MCIF2_WR 37000ns */
           0x8161705e,  /*  21: HIF_PCIe1 RR 13880ns */
           0x8545e043,  /*  22: BSP RR 50000ns */
           0x80ad9051,  /*  23: SAGE RR 6820ns */
           0x86449065,  /*  24: FLASH_DMA RR 63000ns */
           0x8161705d,  /*  25: HIF_PCIe RR 13880ns */
           0x86449067,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449066,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e040,  /*  29: MCIF_RD 37000ns */
           0x03e6e042,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db058,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c505c,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c505b,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1052,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x8050e04a,  /*  40: RAAGA RR 3000ns */
           0x001ae000,  /*  41: RAAGA_1 1000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00bb402a,  /*  44: AUD_AIO 6940ns */
           0x8072804d,  /*  45: VICE_CME_RMB_CMB RR 4500ns */
           0x8394e064,  /*  46: VICE_CME_CSC RR 36000ns */
           0x80d41053,  /*  47: VICE_FME_CSC RR 8330ns */
           0x81a6b061,  /*  48: VICE_FME_Luma_CMB RR 16600ns */
           0x81a6b060,  /*  49: VICE_FME_Chroma_CMB RR 16600ns */
           0x82574036,  /*  50: VICE_SG RR 22200ns */
           0x80002069,  /*  51: VICE_DBLK RR 10ns */
           0x83dc103e,  /*  52: VICE_CABAC0 RR 36600ns */
           0x86f07045,  /*  53: VICE_CABAC1 RR 65800ns */
           0x80a2e050,  /*  54: VICE_ARCSS0 RR 6400ns */
           0x80c7d02e,  /*  55: VICE_VIP0_INST0 RR 7406ns */
           0x82574037,  /*  56: VICE_VIP1_INST0 RR 22200ns */
           0x80c7d02f,  /*  57: VICE_VIP0_INST1 RR 7406ns */
           0x82574038,  /*  58: VICE_VIP1_INST1 RR 22200ns */
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
           0x8000006e,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x8000006f,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x80270047,  /*  75: HVD0_ILCPU RR 1451ns */
           0x809ee04f,  /*  76: HVD0_OLCPU RR 6242ns */
           0x005a2014,  /*  77: HVD0_CAB 3343ns */
           0x0071a019,  /*  78: HVD0_ILSI 4214ns */
           0x81fea062,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x0072601b,  /*  80: HVD0_ILSI_p2 4242ns */
           0x80000072,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x80000073,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x80270048,  /*  83: HVD1_ILCPU RR 1451ns */
           0x80d9b054,  /*  84: HVD1_OLCPU RR 8553ns */
           0x007de01c,  /*  85: HVD1_CAB 4667ns */
           0x0071a01a,  /*  86: HVD1_ILSI 4214ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x005e2017,  /*  94: BVN_MAD_PIX_FD 3493ns */
           0x0084801f,  /*  95: BVN_MAD_QUANT 4914ns */
           0x00bc702d,  /*  96: BVN_MAD_PIX_CAP 6986ns */
           0x005e2015,  /*  97: BVN_MAD1_PIX_FD 3493ns */
           0x0084801d,  /*  98: BVN_MAD1_QUANT 4914ns */
           0x00bc702b,  /*  99: BVN_MAD1_PIX_CAP 6986ns */
           0x005e2016,  /* 100: BVN_MAD2_PIX_FD 3493ns */
           0x0084801e,  /* 101: BVN_MAD2_QUANT 4914ns */
           0x00bc702c,  /* 102: BVN_MAD2_PIX_CAP 6986ns */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x010a8032,  /* 106: BVN_MFD0_Ch 9876ns */
           0x004fd00c,  /* 107: BVN_MFD0_Ch_1 2961ns */
           0x00853020,  /* 108: BVN_MFD1 4938ns */
           0x0058c011,  /* 109: BVN_MFD1_1 3292ns */
           0x00853021,  /* 110: BVN_MFD2 4938ns */
           0x0058c012,  /* 111: BVN_MFD2_1 3292ns */
           0x00853022,  /* 112: BVN_MFD3 4938ns */
           0x0058c013,  /* 113: BVN_MFD3_1 3292ns */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff001,  /* 118: BVN_VFD0 off */
           0x001fb002,  /* 119: BVN_VFD1 1179.10447761194ns */
           0x3ffff03c,  /* 120: BVN_VFD2 off */
           0x3ffff03d,  /* 121: BVN_VFD3 off */
           0x3ffff026,  /* 122: BVN_VFD4 off */
           0x3ffff027,  /* 123: BVN_VFD5 off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff003,  /* 126: BVN_CAP0 off */
           0x001fb004,  /* 127: BVN_CAP1 1179.10447761194ns */
           0x3ffff034,  /* 128: BVN_CAP2 off */
           0x3ffff035,  /* 129: BVN_CAP3 off */
           0x3ffff028,  /* 130: BVN_CAP4 off */
           0x3ffff029,  /* 131: BVN_CAP5 off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff006,  /* 134: BVN_GFD0 off */
           0x3ffff033,  /* 135: BVN_GFD1 off */
           0x3ffff00d,  /* 136: BVN_GFD2 off */
           0x3ffff00e,  /* 137: BVN_GFD3 off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff00b,  /* 141: BVN_MCVP0 off */
           0x3ffff00a,  /* 142: BVN_MCVP1 off */
           0x3ffff030,  /* 143: BVN_MCVP2 off */
           0x0057100f,  /* 144: BVN_RDC 3230ns */
           0x0352603a,  /* 145: VEC_VBI_ENC0 31500ns */
           0x0352603b,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80000070,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x80000071,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
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
           0x801e4046,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x8000007c,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393049,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x8000007d,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000076,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x80000077,  /* 217: HVD1_PFRI RR 0ns */
           0x80002074,  /* 218: VICE_PFRI RR 10ns */
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
           0x3ffff0ff,  /* 249: UNASSIGNED off */
           0xbfffe068,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00692018   /* 255: REFRESH 3900ns */
         };
static const uint32_t aulMemc1_20150623232011_7439_4Kstb_hitemp[] = {
           0x3ffff007,  /*   0: XPT_WR_RS off */
           0x3ffff04e,  /*   1: XPT_WR_XC off */
           0x3ffff025,  /*   2: XPT_WR_CDB off */
           0x3ffff05f,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff031,  /*   4: XPT_RD_RS off */
           0x3ffff039,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff024,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff05a,  /*   7: XPT_RD_PB off */
           0x813b2059,  /*   8: XPT_WR_MEMDMA RR 12376ns */
           0x82bce063,  /*   9: XPT_RD_MEMDMA RR 27520ns */
           0x803fd008,  /*  10: GENET0_WR RR 2370ns */
           0x81026055,  /*  11: GENET0_RD RR 10150ns */
           0x8089d023,  /*  12: GENET1_WR RR 5110ns */
           0x81026056,  /*  13: GENET1_RD RR 10150ns */
           0x803fd009,  /*  14: GENET2_WR RR 2370ns */
           0x81026057,  /*  15: GENET2_RD RR 10150ns */
           0x8596e044,  /*  16: MOCA_MIPS RR 53000ns */
           0x8066204b,  /*  17: SATA RR 4015ns */
           0x8066204c,  /*  18: SATA_1 RR 4015ns */
           0x3ffff03f,  /*  19: MCIF2_RD off */
           0x3ffff041,  /*  20: MCIF2_WR off */
           0x8161705e,  /*  21: HIF_PCIe1 RR 13880ns */
           0x8545e043,  /*  22: BSP RR 50000ns */
           0x80ad9051,  /*  23: SAGE RR 6820ns */
           0x86449065,  /*  24: FLASH_DMA RR 63000ns */
           0x8161705d,  /*  25: HIF_PCIe RR 13880ns */
           0x86449067,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449066,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff040,  /*  29: MCIF_RD off */
           0x3ffff042,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db058,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c505c,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c505b,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1052,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x3ffff04a,  /*  40: RAAGA off */
           0x3ffff000,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff02a,  /*  44: AUD_AIO off */
           0x3ffff04d,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff064,  /*  46: VICE_CME_CSC off */
           0x3ffff053,  /*  47: VICE_FME_CSC off */
           0x3ffff061,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff060,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff036,  /*  50: VICE_SG off */
           0x3ffff069,  /*  51: VICE_DBLK off */
           0x3ffff03e,  /*  52: VICE_CABAC0 off */
           0x3ffff045,  /*  53: VICE_CABAC1 off */
           0x3ffff050,  /*  54: VICE_ARCSS0 off */
           0x3ffff02e,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff037,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff02f,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff038,  /*  58: VICE_VIP1_INST1 off */
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
           0x8000006a,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8000006b,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff047,  /*  75: HVD0_ILCPU off */
           0x3ffff04f,  /*  76: HVD0_OLCPU off */
           0x3ffff014,  /*  77: HVD0_CAB off */
           0x3ffff019,  /*  78: HVD0_ILSI off */
           0x81fea062,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x3ffff01b,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff072,  /*  81: HVD1_DBLK_0 off */
           0x3ffff073,  /*  82: HVD1_DBLK_1 off */
           0x3ffff048,  /*  83: HVD1_ILCPU off */
           0x3ffff054,  /*  84: HVD1_OLCPU off */
           0x3ffff01c,  /*  85: HVD1_CAB off */
           0x3ffff01a,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff017,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff01f,  /*  95: BVN_MAD_QUANT off */
           0x3ffff02d,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff015,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff01d,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff02b,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff016,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff01e,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff02c,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0058c010,  /* 106: BVN_MFD0 3292ns */
           0x0027d005,  /* 107: BVN_MFD0_1 1480.5ns */
           0x3ffff020,  /* 108: BVN_MFD1 off */
           0x3ffff011,  /* 109: BVN_MFD1_1 off */
           0x3ffff021,  /* 110: BVN_MFD2 off */
           0x3ffff012,  /* 111: BVN_MFD2_1 off */
           0x3ffff022,  /* 112: BVN_MFD3 off */
           0x3ffff013,  /* 113: BVN_MFD3_1 off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001fb001,  /* 118: BVN_VFD0 1179.10447761194ns */
           0x3ffff002,  /* 119: BVN_VFD1 off */
           0x0359a03c,  /* 120: BVN_VFD2 31770ns */
           0x0359a03d,  /* 121: BVN_VFD3 31770ns */
           0x00951026,  /* 122: BVN_VFD4 5527.3631840796ns */
           0x00951027,  /* 123: BVN_VFD5 5527.3631840796ns */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x001fb003,  /* 126: BVN_CAP0 1179.10447761194ns */
           0x3ffff004,  /* 127: BVN_CAP1 off */
           0x01747034,  /* 128: BVN_CAP2 13800ns */
           0x01747035,  /* 129: BVN_CAP3 13800ns */
           0x00951028,  /* 130: BVN_CAP4 5527.3631840796ns */
           0x00951029,  /* 131: BVN_CAP5 5527.3631840796ns */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0031d006,  /* 134: BVN_GFD0 1851ns */
           0x0167c033,  /* 135: BVN_GFD1 13330ns */
           0x0055900d,  /* 136: BVN_GFD2 3174ns */
           0x0055900e,  /* 137: BVN_GFD3 3174ns */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b500b,  /* 141: BVN_MCVP0 2794ns */
           0x004b500a,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d030,  /* 143: BVN_MCVP2 7850ns */
           0x3ffff00f,  /* 144: BVN_RDC off */
           0x3ffff03a,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff03b,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x8000006c,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x8000006d,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
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
           0x801e4046,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x8000007c,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393049,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x8000007d,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000075,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff077,  /* 217: HVD1_PFRI off */
           0x3ffff074,  /* 218: VICE_PFRI off */
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
           0x3ffff0ff,  /* 249: UNASSIGNED off */
           0xbfffe068,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00692018   /* 255: REFRESH 3900ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20150623232011_7439_4Kstb_hitemp[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x807a0802}, /* HVD0_PFRI_Ch (gHvdC0) 493440.00 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000029a}, /* d: 4; p: 666.14375 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00000808}, /* 2056 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x000004d1}, /* 60% * 2056 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x807b0802}, /* HVD1_PFRI (gHvd1) 980160.00 ns/40 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000a56}, /* d: 4; p: 2646.43125 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x000015e5}, /* 5605 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x00000d23}, /* 60% * 5605 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x80780803}, /* VICE_PFRI (gVice) 333333.33 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x400001c1}, /* d: 4; p: 449.996875 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80790803}, /* HVD0_PFRI (gHvd0) 493440.00 ns/80 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000029a}, /* d: 4; p: 666.14375 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000f90}, /* 3984 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000956}  /* 60% * 3984 */
};

static const uint32_t* const paulMemc_box10[] = { &aulMemc0_20150623232011_7439_4Kstb_hitemp[0], &aulMemc1_20150623232011_7439_4Kstb_hitemp[0]};

const BBOX_Rts stBoxRts_7439_4Kstb_hitemp_box10 = {
  "20150623232011_7439_4Kstb_hitemp_box10",
  7439,
  10,
  2,
  256,
  (const uint32_t**)&paulMemc_box10[0],
  16,
  stBoxRts_PfriClient_20150623232011_7439_4Kstb_hitemp
};
