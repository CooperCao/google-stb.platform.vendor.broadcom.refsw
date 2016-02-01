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
*   at: Mon Oct 12 21:57:28 2015 GMT
*   by: robinc
*   for: Box 7252_4K1t
*         MemC 0 (32-bit DDR3@1066MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1066MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20151012215728_7252_4K1t[] = {
           0x001e6002,  /*   0: XPT_WR_RS 1130ns */
           0x803a3035,  /*   1: XPT_WR_XC RR 2290ns */
           0x8049000b,  /*   2: XPT_WR_CDB RR 2710ns */
           0x80b9103e,  /*   3: XPT_WR_ITB_MSG RR 7270ns */
           0x80780017,  /*   4: XPT_RD_RS RR 4450ns */
           0x819e5022,  /*   5: XPT_RD_XC_RMX_MSG RR 15350ns */
           0x8049000a,  /*   6: XPT_RD_XC_RAVE RR 2710ns */
           0x80a2e03b,  /*   7: XPT_RD_PB RR 6400ns */
           0x809d8039,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e6045,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd007,  /*  10: SYSPORT_WR RR 2370ns */
           0x81026040,  /*  11: SYSPORT_RD RR 10150ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617047,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e030,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662037,  /*  17: SATA RR 4015ns */
           0x80662038,  /*  18: SATA_1 RR 4015ns */
           0x03e6e02b,  /*  19: MCIF2_RD 37000ns */
           0x03e6e02d,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e02f,  /*  22: BSP RR 50000ns */
           0x80ad903c,  /*  23: SAGE RR 6820ns */
           0x86449049,  /*  24: FLASH_DMA RR 63000ns */
           0x81617046,  /*  25: HIF_PCIe RR 13880ns */
           0x8644904b,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644904a,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e02c,  /*  29: MCIF_RD 37000ns */
           0x03e6e02e,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db041,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5044,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5043,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae103d,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db042,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x0050e00f,  /*  40: RAAGA 3000ns */
           0x001ae000,  /*  41: RAAGA_1 1000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x00bb401a,  /*  44: AUD_AIO 6940ns */
           0x3ffff04d,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff052,  /*  46: VICE_CME_CSC off */
           0x3ffff04e,  /*  47: VICE_FME_CSC off */
           0x3ffff050,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff04f,  /*  49: VICE_FME_Chroma_CMB off */
           0x82574024,  /*  50: VICE_SG RR 22200ns */
           0x3ffff057,  /*  51: VICE_DBLK off */
           0x83dc102a,  /*  52: VICE_CABAC0 RR 36600ns */
           0x86f07031,  /*  53: VICE_CABAC1 RR 65800ns */
           0x3ffff051,  /*  54: VICE_ARCSS0 off */
           0x3ffff01b,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff025,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff0ff,  /*  57: UNASSIGNED off */
           0x3ffff0ff,  /*  58: UNASSIGNED off */
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
           0x80000058,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x80000059,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x80270033,  /*  75: HVD0_ILCPU RR 1451ns */
           0x809ee03a,  /*  76: HVD0_OLCPU RR 6242ns */
           0x005a2012,  /*  77: HVD0_CAB 3343ns */
           0x0071a014,  /*  78: HVD0_ILSI 4214ns */
           0x81fea048,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x00726015,  /*  80: HVD0_ILSI_p2 4242ns */
           0x8000005c,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8000005d,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x803b3036,  /*  83: HVD1_ILCPU RR 2198ns */
           0x80e3303f,  /*  84: HVD1_OLCPU RR 8925ns */
           0x00916018,  /*  85: HVD1_CAB 5390ns */
           0x00743016,  /*  86: HVD1_ILSI 4308ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff0ff,  /*  94: UNASSIGNED off */
           0x3ffff0ff,  /*  95: UNASSIGNED off */
           0x3ffff0ff,  /*  96: UNASSIGNED off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x00f2401f,  /* 100: BVN_MAD2_PIX_FD 8978.507ns */
           0x025de026,  /* 101: BVN_MAD2_QUANT 22446.741ns */
           0x01e4b023,  /* 102: BVN_MAD2_PIX_CAP 17957.014ns */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x00c7d01c,  /* 106: BVN_MFD0_Ch 7407ns */
           0x0038f006,  /* 107: BVN_MFD0_Ch_1 2115ns */
           0x3ffff0ff,  /* 108: UNASSIGNED off */
           0x3ffff0ff,  /* 109: UNASSIGNED off */
           0x0063d013,  /* 110: BVN_MFD2 3703ns */
           0x00428009,  /* 111: BVN_MFD2_1 2469ns */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x001fb003,  /* 118: BVN_VFD0 1179.10447761194ns */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff029,  /* 120: BVN_VFD2 off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x00951019,  /* 123: BVN_VFD5 5527.3631840796ns */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x001fb004,  /* 126: BVN_CAP0 1179.10447761194ns */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff021,  /* 128: BVN_CAP2 off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x004a700c,  /* 131: BVN_CAP5 2763.6815920398ns */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff005,  /* 134: BVN_GFD0 off */
           0x3ffff020,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff010,  /* 138: BVN_GFD4 off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b500e,  /* 141: BVN_MCVP0 2794ns */
           0x004b500d,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d01e,  /* 143: BVN_MCVP2 7850ns */
           0x00571011,  /* 144: BVN_RDC 3230ns */
           0x03526027,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526028,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x8000005a,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x8000005b,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
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
           0x801e4032,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000066,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393034,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000067,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000005f,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x80000061,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff0ff,  /* 218: UNASSIGNED off */
           0x3ffff060,  /* 219: VICE_PFRI off */
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
           0xbfffe04c,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2701d   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20151012215728_7252_4K1t[] = {
           0x3ffff002,  /*   0: XPT_WR_RS off */
           0x3ffff035,  /*   1: XPT_WR_XC off */
           0x3ffff00b,  /*   2: XPT_WR_CDB off */
           0x3ffff03e,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff017,  /*   4: XPT_RD_RS off */
           0x3ffff022,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff03b,  /*   7: XPT_RD_PB off */
           0x809d8039,  /*   8: XPT_WR_MEMDMA RR 6188ns */
           0x815e6045,  /*   9: XPT_RD_MEMDMA RR 13760ns */
           0x803fd007,  /*  10: SYSPORT_WR RR 2370ns */
           0x81026040,  /*  11: SYSPORT_RD RR 10150ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x81617047,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e030,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662037,  /*  17: SATA RR 4015ns */
           0x80662038,  /*  18: SATA_1 RR 4015ns */
           0x3ffff02b,  /*  19: MCIF2_RD off */
           0x3ffff02d,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e02f,  /*  22: BSP RR 50000ns */
           0x80ad903c,  /*  23: SAGE RR 6820ns */
           0x86449049,  /*  24: FLASH_DMA RR 63000ns */
           0x81617046,  /*  25: HIF_PCIe RR 13880ns */
           0x8644904b,  /*  26: SDIO_EMMC RR 63000ns */
           0x8644904a,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff02c,  /*  29: MCIF_RD off */
           0x3ffff02e,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db041,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5044,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5043,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae103d,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db042,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff00f,  /*  40: RAAGA off */
           0x3ffff000,  /*  41: RAAGA_1 off */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x3ffff01a,  /*  44: AUD_AIO off */
           0x8072804d,  /*  45: VICE_CME_RMB_CMB RR 4500ns */
           0x8394e052,  /*  46: VICE_CME_CSC RR 36000ns */
           0x80d4104e,  /*  47: VICE_FME_CSC RR 8330ns */
           0x81a6b050,  /*  48: VICE_FME_Luma_CMB RR 16600ns */
           0x81a6b04f,  /*  49: VICE_FME_Chroma_CMB RR 16600ns */
           0x3ffff024,  /*  50: VICE_SG off */
           0x80002057,  /*  51: VICE_DBLK RR 10ns */
           0x3ffff02a,  /*  52: VICE_CABAC0 off */
           0x3ffff031,  /*  53: VICE_CABAC1 off */
           0x80a2e051,  /*  54: VICE_ARCSS0 RR 6400ns */
           0x80c7d01b,  /*  55: VICE_VIP0_INST0 RR 7406ns */
           0x82574025,  /*  56: VICE_VIP1_INST0 RR 22200ns */
           0x3ffff0ff,  /*  57: UNASSIGNED off */
           0x3ffff0ff,  /*  58: UNASSIGNED off */
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
           0x80000053,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x80000054,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff033,  /*  75: HVD0_ILCPU off */
           0x3ffff03a,  /*  76: HVD0_OLCPU off */
           0x3ffff012,  /*  77: HVD0_CAB off */
           0x3ffff014,  /*  78: HVD0_ILSI off */
           0x81fea048,  /*  79: HVD0_ILCPU_p2 RR 18917ns */
           0x3ffff015,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff05c,  /*  81: HVD1_DBLK_0 off */
           0x3ffff05d,  /*  82: HVD1_DBLK_1 off */
           0x3ffff036,  /*  83: HVD1_ILCPU off */
           0x3ffff03f,  /*  84: HVD1_OLCPU off */
           0x3ffff018,  /*  85: HVD1_CAB off */
           0x3ffff016,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff0ff,  /*  94: UNASSIGNED off */
           0x3ffff0ff,  /*  95: UNASSIGNED off */
           0x3ffff0ff,  /*  96: UNASSIGNED off */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff01f,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff026,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff023,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x00428008,  /* 106: BVN_MFD0 2469ns */
           0x001c6001,  /* 107: BVN_MFD0_1 1057.5ns */
           0x3ffff0ff,  /* 108: UNASSIGNED off */
           0x3ffff0ff,  /* 109: UNASSIGNED off */
           0x3ffff013,  /* 110: BVN_MFD2 off */
           0x3ffff009,  /* 111: BVN_MFD2_1 off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff003,  /* 118: BVN_VFD0 off */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x0359a029,  /* 120: BVN_VFD2 31770ns */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff019,  /* 123: BVN_VFD5 off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff004,  /* 126: BVN_CAP0 off */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x01747021,  /* 128: BVN_CAP2 13800ns */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff00c,  /* 131: BVN_CAP5 off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0031d005,  /* 134: BVN_GFD0 1851ns */
           0x0167c020,  /* 135: BVN_GFD1 13330ns */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x00559010,  /* 138: BVN_GFD4 3174ns */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff00e,  /* 141: BVN_MCVP0 off */
           0x3ffff00d,  /* 142: BVN_MCVP1 off */
           0x3ffff01e,  /* 143: BVN_MCVP2 off */
           0x3ffff011,  /* 144: BVN_RDC off */
           0x3ffff027,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff028,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80000055,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x80000056,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
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
           0x801e4032,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000066,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393034,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000067,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000005e,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff061,  /* 217: HVD1_PFRI off */
           0x3ffff0ff,  /* 218: UNASSIGNED off */
           0x80002060,  /* 219: VICE_PFRI RR 10ns */
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
           0xbfffe04c,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2701d   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20151012215728_7252_4K1t[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80630803}, /* HVD0_PFRI_Ch (gHvdC0) 935040.00 ns/160 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.1515625 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00000cfe}, /* 3326 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x000007cb}, /* 60% * 3326 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x80650802}, /* HVD1_PFRI (gHvd1) 3871680.00 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x4000146a}, /* d: 4; p: 5226.765625 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x000023f4}, /* 9204 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x00001592}, /* 60% * 9204 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80620803}, /* HVD0_PFRI (gHvd0) 935040.00 ns/160 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000277}, /* d: 4; p: 631.1515625 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00001642}, /* 5698 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000d5a}, /* 60% * 5698 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_CONFIG,      0x80640802}, /* VICE_PFRI (gVice) 1333333.33 ns/160 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_FILTER_CTRL, 0x40000383}, /* d: 4; p: 899.9984375 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_THRESH0,     0x0000034e}, /* 846 */
  {BCHP_MEMC_GEN_1_PFRI_3_THROTTLE_THRESH1,     0x000001fb}  /* 60% * 846 */
};

static const uint32_t* const paulMemc_box5[] = { &aulMemc0_20151012215728_7252_4K1t[0], &aulMemc1_20151012215728_7252_4K1t[0]};

const BBOX_Rts stBoxRts_7252_4K1t_box5 = {
  "20151012215728_7252_4K1t_box5",
  7445,
  5,
  2,
  256,
  (const uint32_t**)&paulMemc_box5[0],
  16,
  stBoxRts_PfriClient_20151012215728_7252_4K1t
};
