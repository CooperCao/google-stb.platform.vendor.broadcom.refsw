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
*   at: Wed Jun 24 00:57:25 2015 GMT
*   by: robinc
*   for: Box 7445_4k2t
*         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 1 (32-bit DDR3@1067MHz) w/432MHz clock
*         MemC 2 (32-bit DDR3@1067MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20150624005725_7445_4k2t[] = {
           0x001f3005,  /*   0: XPT_WR_RS 1160.49382716049ns */
           0x802ec065,  /*   1: XPT_WR_XC RR 1840.88127294982ns */
           0x8035900b,  /*   2: XPT_WR_CDB RR 1989.41798941799ns */
           0x809c4073,  /*   3: XPT_WR_ITB_MSG RR 6138.77551020408ns */
           0x8070a028,  /*   4: XPT_RD_RS RR 4177.77777777778ns */
           0x82d50053,  /*   5: XPT_RD_XC_RMX_MSG RR 26857.1428571429ns */
           0x8035900a,  /*   6: XPT_RD_XC_RAVE RR 1989.41798941799ns */
           0x80720071,  /*   7: XPT_RD_PB RR 4481.40043763676ns */
           0x8033c067,  /*   8: XPT_WR_MEMDMA_0 RR 2036.36363636364ns */
           0x80767072,  /*   9: XPT_RD_MEMDMA_0 RR 4654.54545454545ns */
           0x803fd00c,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9076,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161707c,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05f,  /*  16: MOCA_MIPS RR 53000ns */
           0x80411068,  /*  17: SATA RR 2560ns */
           0x80411069,  /*  18: SATA_1 RR 2560ns */
           0x03e6e05a,  /*  19: MCIF2_RD 37000ns */
           0x03e6e05c,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05e,  /*  22: BSP RR 50000ns */
           0x80ad9074,  /*  23: SAGE RR 6820ns */
           0x8644907f,  /*  24: FLASH_DMA RR 63000ns */
           0x8161707b,  /*  25: HIF_PCIe RR 13880ns */
           0x86449081,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449080,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e05b,  /*  29: MCIF_RD 37000ns */
           0x03e6e05d,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db077,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c507a,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5079,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1075,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db078,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x0050e017,  /*  40: RAAGA 3000ns */
           0x001ae003,  /*  41: RAAGA_1 1000ns */
           0x00a1e03c,  /*  42: RAAGA1 6000ns */
           0x001ae002,  /*  43: RAAGA1_1 1000ns */
           0x00bb403d,  /*  44: AUD_AIO 6940ns */
           0x8032d066,  /*  45: VICE_CME_RMB_CMB RR 2000ns */
           0x8197707d,  /*  46: VICE_CME_CSC RR 16000ns */
           0x805d606d,  /*  47: VICE_FME_CSC RR 3670ns */
           0x805d606f,  /*  48: VICE_FME_Luma_CMB RR 3670ns */
           0x805d606e,  /*  49: VICE_FME_Chroma_CMB RR 3670ns */
           0x80dca04d,  /*  50: VICE_SG RR 8176.66666666667ns */
           0x8000009a,  /*  51: VICE_DBLK RR 0ns */
           0x81edf052,  /*  52: VICE_CABAC0 RR 18300ns */
           0x83782059,  /*  53: VICE_CABAC1 RR 32900ns */
           0x804d906c,  /*  54: VICE_ARCSS0 RR 3050ns */
           0x8063d024,  /*  55: VICE_VIP0_INST0 RR 3703ns */
           0x80c7a046,  /*  56: VICE_VIP1_INST0 RR 7400ns */
           0x8063d025,  /*  57: VICE_VIP0_INST1 RR 3703ns */
           0x80c7a047,  /*  58: VICE_VIP1_INST1 RR 7400ns */
           0x3ffff083,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff08a,  /*  60: VICE1_CME_CSC off */
           0x3ffff084,  /*  61: VICE1_FME_CSC off */
           0x3ffff086,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff085,  /*  63: VICE1_FME_Chroma_CMB off */
           0x80dca04c,  /*  64: VICE1_SG RR 8176.66666666667ns */
           0x3ffff099,  /*  65: VICE1_DBLK off */
           0x81edf051,  /*  66: VICE1_CABAC0 RR 18300ns */
           0x83782058,  /*  67: VICE1_CABAC1 RR 32900ns */
           0x3ffff089,  /*  68: VICE1_ARCSS0 off */
           0x3ffff021,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff043,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff022,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff044,  /*  72: VICE1_VIP1_INST1 off */
           0x3ffff08d,  /*  73: HVD0_DBLK_0 off */
           0x3ffff08e,  /*  74: HVD0_DBLK_1 off */
           0x801c2061,  /*  75: HVD0_ILCPU RR 1048ns */
           0x804a706a,  /*  76: HVD0_OLCPU RR 2927ns */
           0x00596019,  /*  77: HVD0_CAB 3317ns */
           0x00710029,  /*  78: HVD0_ILSI 4191ns */
           0x81ea307e,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x0071002a,  /*  80: HVD0_ILSI_p2 4191ns */
           0x3ffff095,  /*  81: HVD1_DBLK_0 off */
           0x3ffff096,  /*  82: HVD1_DBLK_1 off */
           0x801c2062,  /*  83: HVD1_ILCPU RR 1048ns */
           0x804a706b,  /*  84: HVD1_OLCPU RR 2927ns */
           0x0059601a,  /*  85: HVD1_CAB 3317ns */
           0x006f4027,  /*  86: HVD1_ILSI 4125ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff097,  /*  88: HVD2_DBLK_0 off */
           0x3ffff098,  /*  89: HVD2_DBLK_1 off */
           0x80270064,  /*  90: HVD2_ILCPU RR 1451ns */
           0x8070a070,  /*  91: HVD2_OLCPU RR 4427ns */
           0x007dd02c,  /*  92: HVD2_CAB 4666ns */
           0x0073d02b,  /*  93: HVD2_ILSI 4296ns */
           0x3ffff01f,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff031,  /*  95: BVN_MAD_QUANT off */
           0x3ffff042,  /*  96: BVN_MAD_PIX_CAP off */
           0x005e301b,  /*  97: BVN_MAD1_PIX_FD 3493.445ns */
           0x0084802d,  /*  98: BVN_MAD1_QUANT 4913.31ns */
           0x00bc803e,  /*  99: BVN_MAD1_PIX_CAP 6987.885ns */
           0x005e301c,  /* 100: BVN_MAD2_PIX_FD 3493.445ns */
           0x0084802e,  /* 101: BVN_MAD2_QUANT 4913.31ns */
           0x00bc803f,  /* 102: BVN_MAD2_PIX_CAP 6987.885ns */
           0x005e301d,  /* 103: BVN_MAD3_PIX_FD 3493.445ns */
           0x0084802f,  /* 104: BVN_MAD3_QUANT 4913.31ns */
           0x00bc8040,  /* 105: BVN_MAD3_PIX_CAP 6987.885ns */
           0x3ffff020,  /* 106: BVN_MFD0 off */
           0x3ffff000,  /* 107: BVN_MFD0_1 off */
           0x3ffff032,  /* 108: BVN_MFD1 off */
           0x3ffff033,  /* 109: BVN_MFD1_1 off */
           0x3ffff034,  /* 110: BVN_MFD2 off */
           0x3ffff035,  /* 111: BVN_MFD2_1 off */
           0x3ffff036,  /* 112: BVN_MFD3 off */
           0x3ffff037,  /* 113: BVN_MFD3_1 off */
           0x3ffff038,  /* 114: BVN_MFD4 off */
           0x3ffff039,  /* 115: BVN_MFD4_1 off */
           0x3ffff03a,  /* 116: BVN_MFD5 off */
           0x3ffff03b,  /* 117: BVN_MFD5_1 off */
           0x3ffff006,  /* 118: BVN_VFD0 off */
           0x3ffff007,  /* 119: BVN_VFD1 off */
           0x3ffff056,  /* 120: BVN_VFD2 off */
           0x3ffff057,  /* 121: BVN_VFD3 off */
           0x004fc00f,  /* 122: BVN_VFD4 2960ns */
           0x004fc010,  /* 123: BVN_VFD5 2960ns */
           0x004fc011,  /* 124: BVN_VFD6 2960ns */
           0x004fc012,  /* 125: BVN_VFD7 2960ns */
           0x3ffff001,  /* 126: BVN_CAP0 off */
           0x3ffff004,  /* 127: BVN_CAP1 off */
           0x3ffff04f,  /* 128: BVN_CAP2 off */
           0x3ffff050,  /* 129: BVN_CAP3 off */
           0x004fc013,  /* 130: BVN_CAP4 2960ns */
           0x004fc014,  /* 131: BVN_CAP5 2960ns */
           0x004fc015,  /* 132: BVN_CAP6 2960ns */
           0x004fc016,  /* 133: BVN_CAP7 2960ns */
           0x3ffff009,  /* 134: BVN_GFD0 off */
           0x3ffff04e,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff00e,  /* 141: BVN_MCVP0 off */
           0x3ffff00d,  /* 142: BVN_MCVP1 off */
           0x3ffff04a,  /* 143: BVN_MCVP2 off */
           0x3ffff018,  /* 144: BVN_RDC off */
           0x03526054,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526055,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x8063d026,  /* 151: VICE_VIP0_INST2 RR 3703ns */
           0x80c7a048,  /* 152: VICE_VIP1_INST2 RR 7400ns */
           0x3ffff023,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff045,  /* 154: VICE1_VIP1_INST2 off */
           0x3ffff091,  /* 155: HVD0_DBLK_p2_0 off */
           0x3ffff092,  /* 156: HVD0_DBLK_p2_1 off */
           0x005e301e,  /* 157: BVN_MAD4_PIX_FD 3493.445ns */
           0x00848030,  /* 158: BVN_MAD4_QUANT 4913.31ns */
           0x00bc8041,  /* 159: BVN_MAD4_PIX_CAP 6987.885ns */
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
           0x80142060,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x800000a7,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261063,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x800000a8,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x3ffff09e,  /* 216: HVD0_PFRI off */
           0x3ffff09f,  /* 217: HVD1_PFRI off */
           0x3ffff0a0,  /* 218: HVD2_PFRI off */
           0x8000009c,  /* 219: VICE_PFRI RR 0ns */
           0x3ffff09b,  /* 220: VICE1_PFRI off */
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
           0xbfffe082,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2704b   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc1_20150624005725_7445_4k2t[] = {
           0x3ffff005,  /*   0: XPT_WR_RS off */
           0x3ffff065,  /*   1: XPT_WR_XC off */
           0x3ffff00b,  /*   2: XPT_WR_CDB off */
           0x3ffff073,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff028,  /*   4: XPT_RD_RS off */
           0x3ffff053,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff071,  /*   7: XPT_RD_PB off */
           0x8033c08b,  /*   8: XPT_WR_MEMDMA_1 RR 2036.36363636364ns */
           0x80767087,  /*   9: XPT_RD_MEMDMA_1 RR 4654.54545454545ns */
           0x803fd00c,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9076,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161707c,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05f,  /*  16: MOCA_MIPS RR 53000ns */
           0x80411068,  /*  17: SATA RR 2560ns */
           0x80411069,  /*  18: SATA_1 RR 2560ns */
           0x3ffff05a,  /*  19: MCIF2_RD off */
           0x3ffff05c,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05e,  /*  22: BSP RR 50000ns */
           0x80ad9074,  /*  23: SAGE RR 6820ns */
           0x8644907f,  /*  24: FLASH_DMA RR 63000ns */
           0x8161707b,  /*  25: HIF_PCIe RR 13880ns */
           0x86449081,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449080,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff05b,  /*  29: MCIF_RD off */
           0x3ffff05d,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db077,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c507a,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5079,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1075,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db078,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff017,  /*  40: RAAGA off */
           0x3ffff003,  /*  41: RAAGA_1 off */
           0x3ffff03c,  /*  42: RAAGA1 off */
           0x3ffff002,  /*  43: RAAGA1_1 off */
           0x3ffff03d,  /*  44: AUD_AIO off */
           0x3ffff066,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff07d,  /*  46: VICE_CME_CSC off */
           0x3ffff06d,  /*  47: VICE_FME_CSC off */
           0x3ffff06f,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff06e,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff04d,  /*  50: VICE_SG off */
           0x3ffff09a,  /*  51: VICE_DBLK off */
           0x3ffff052,  /*  52: VICE_CABAC0 off */
           0x3ffff059,  /*  53: VICE_CABAC1 off */
           0x3ffff06c,  /*  54: VICE_ARCSS0 off */
           0x3ffff024,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff046,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff025,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff047,  /*  58: VICE_VIP1_INST1 off */
           0x3ffff083,  /*  59: VICE1_CME_RMB_CMB off */
           0x3ffff08a,  /*  60: VICE1_CME_CSC off */
           0x3ffff084,  /*  61: VICE1_FME_CSC off */
           0x3ffff086,  /*  62: VICE1_FME_Luma_CMB off */
           0x3ffff085,  /*  63: VICE1_FME_Chroma_CMB off */
           0x3ffff04c,  /*  64: VICE1_SG off */
           0x3ffff099,  /*  65: VICE1_DBLK off */
           0x3ffff051,  /*  66: VICE1_CABAC0 off */
           0x3ffff058,  /*  67: VICE1_CABAC1 off */
           0x3ffff089,  /*  68: VICE1_ARCSS0 off */
           0x3ffff021,  /*  69: VICE1_VIP0_INST0 off */
           0x3ffff043,  /*  70: VICE1_VIP1_INST0 off */
           0x3ffff022,  /*  71: VICE1_VIP0_INST1 off */
           0x3ffff044,  /*  72: VICE1_VIP1_INST1 off */
           0x800a608f,  /*  73: HVD0_DBLK_Ch_0 RR 0ns */
           0x80052090,  /*  74: HVD0_DBLK_Ch_1 RR 0ns */
           0x3ffff061,  /*  75: HVD0_ILCPU off */
           0x3ffff06a,  /*  76: HVD0_OLCPU off */
           0x3ffff019,  /*  77: HVD0_CAB off */
           0x3ffff029,  /*  78: HVD0_ILSI off */
           0x81ea307e,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff02a,  /*  80: HVD0_ILSI_p2 off */
           0x3ffff095,  /*  81: HVD1_DBLK_0 off */
           0x3ffff096,  /*  82: HVD1_DBLK_1 off */
           0x3ffff062,  /*  83: HVD1_ILCPU off */
           0x3ffff06b,  /*  84: HVD1_OLCPU off */
           0x3ffff01a,  /*  85: HVD1_CAB off */
           0x3ffff027,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x80140097,  /*  88: HVD2_DBLK_0 RR 0ns */
           0x8009f098,  /*  89: HVD2_DBLK_1 RR 0ns */
           0x3ffff064,  /*  90: HVD2_ILCPU off */
           0x3ffff070,  /*  91: HVD2_OLCPU off */
           0x3ffff02c,  /*  92: HVD2_CAB off */
           0x3ffff02b,  /*  93: HVD2_ILSI off */
           0x005e301f,  /*  94: BVN_MAD_PIX_FD 3493.445ns */
           0x00848031,  /*  95: BVN_MAD_QUANT 4913.31ns */
           0x00bc8042,  /*  96: BVN_MAD_PIX_CAP 6987.885ns */
           0x3ffff01b,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff02d,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff03e,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff01c,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff02e,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff03f,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff01d,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff02f,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff040,  /* 105: BVN_MAD3_PIX_CAP off */
           0x00c7d049,  /* 106: BVN_MFD0_Ch 7407ns */
           0x002b6008,  /* 107: BVN_MFD0_Ch_1 1612ns */
           0x3ffff032,  /* 108: BVN_MFD1 off */
           0x3ffff033,  /* 109: BVN_MFD1_1 off */
           0x00853034,  /* 110: BVN_MFD2 4938ns */
           0x00853035,  /* 111: BVN_MFD2_1 4938ns */
           0x00853036,  /* 112: BVN_MFD3 4938ns */
           0x00853037,  /* 113: BVN_MFD3_1 4938ns */
           0x3ffff038,  /* 114: BVN_MFD4 off */
           0x3ffff039,  /* 115: BVN_MFD4_1 off */
           0x3ffff03a,  /* 116: BVN_MFD5 off */
           0x3ffff03b,  /* 117: BVN_MFD5_1 off */
           0x001fd006,  /* 118: BVN_VFD0 1185ns */
           0x001fd007,  /* 119: BVN_VFD1 1185ns */
           0x0359a056,  /* 120: BVN_VFD2 31770ns */
           0x0359a057,  /* 121: BVN_VFD3 31770ns */
           0x3ffff00f,  /* 122: BVN_VFD4 off */
           0x3ffff010,  /* 123: BVN_VFD5 off */
           0x3ffff011,  /* 124: BVN_VFD6 off */
           0x3ffff012,  /* 125: BVN_VFD7 off */
           0x00171001,  /* 126: BVN_CAP0 859ns */
           0x001e5004,  /* 127: BVN_CAP1 1128.594ns */
           0x0174704f,  /* 128: BVN_CAP2 13800ns */
           0x01747050,  /* 129: BVN_CAP3 13800ns */
           0x3ffff013,  /* 130: BVN_CAP4 off */
           0x3ffff014,  /* 131: BVN_CAP5 off */
           0x3ffff015,  /* 132: BVN_CAP6 off */
           0x3ffff016,  /* 133: BVN_CAP7 off */
           0x3ffff009,  /* 134: BVN_GFD0 off */
           0x0167c04e,  /* 135: BVN_GFD1 13330ns */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004a900e,  /* 141: BVN_MCVP0 2767.095ns */
           0x004a900d,  /* 142: BVN_MCVP1 2767.095ns */
           0x00d1b04a,  /* 143: BVN_MCVP2 7771.945ns */
           0x3ffff018,  /* 144: BVN_RDC off */
           0x3ffff054,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff055,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff026,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff048,  /* 152: VICE_VIP1_INST2 off */
           0x3ffff023,  /* 153: VICE1_VIP0_INST2 off */
           0x3ffff045,  /* 154: VICE1_VIP1_INST2 off */
           0x800a6093,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns */
           0x80052094,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns */
           0x3ffff01e,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff030,  /* 158: BVN_MAD4_QUANT off */
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
           0x80142060,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x800000a7,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261063,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x800000a8,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000009d,  /* 216: HVD0_PFRI_Ch RR 0ns */
           0x3ffff09f,  /* 217: HVD1_PFRI off */
           0x800000a0,  /* 218: HVD2_PFRI RR 0ns */
           0x3ffff09c,  /* 219: VICE_PFRI off */
           0x3ffff09b,  /* 220: VICE1_PFRI off */
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
           0xbfffe082,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2704b   /* 255: REFRESH 7800ns */
         };
static const uint32_t aulMemc2_20150624005725_7445_4k2t[] = {
           0x3ffff005,  /*   0: XPT_WR_RS off */
           0x3ffff065,  /*   1: XPT_WR_XC off */
           0x3ffff00b,  /*   2: XPT_WR_CDB off */
           0x3ffff073,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff028,  /*   4: XPT_RD_RS off */
           0x3ffff053,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff071,  /*   7: XPT_RD_PB off */
           0x8033c08c,  /*   8: XPT_WR_MEMDMA_2 RR 2036.36363636364ns */
           0x80767088,  /*   9: XPT_RD_MEMDMA_2 RR 4654.54545454545ns */
           0x803fd00c,  /*  10: SYSPORT_WR RR 2370ns */
           0x80ae9076,  /*  11: SYSPORT_RD RR 6860ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x8161707c,  /*  15: HIF_PCIe1 RR 13880ns */
           0x8596e05f,  /*  16: MOCA_MIPS RR 53000ns */
           0x80411068,  /*  17: SATA RR 2560ns */
           0x80411069,  /*  18: SATA_1 RR 2560ns */
           0x3ffff05a,  /*  19: MCIF2_RD off */
           0x3ffff05c,  /*  20: MCIF2_WR off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e05e,  /*  22: BSP RR 50000ns */
           0x80ad9074,  /*  23: SAGE RR 6820ns */
           0x8644907f,  /*  24: FLASH_DMA RR 63000ns */
           0x8161707b,  /*  25: HIF_PCIe RR 13880ns */
           0x86449081,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449080,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff05b,  /*  29: MCIF_RD off */
           0x3ffff05d,  /*  30: MCIF_WR off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x810db077,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c507a,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5079,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae1075,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db078,  /*  38: USB_HI_1 RR 10593ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff017,  /*  40: RAAGA off */
           0x3ffff003,  /*  41: RAAGA_1 off */
           0x3ffff03c,  /*  42: RAAGA1 off */
           0x3ffff002,  /*  43: RAAGA1_1 off */
           0x3ffff03d,  /*  44: AUD_AIO off */
           0x3ffff066,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff07d,  /*  46: VICE_CME_CSC off */
           0x3ffff06d,  /*  47: VICE_FME_CSC off */
           0x3ffff06f,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff06e,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff04d,  /*  50: VICE_SG off */
           0x3ffff09a,  /*  51: VICE_DBLK off */
           0x3ffff052,  /*  52: VICE_CABAC0 off */
           0x3ffff059,  /*  53: VICE_CABAC1 off */
           0x3ffff06c,  /*  54: VICE_ARCSS0 off */
           0x3ffff024,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff046,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff025,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff047,  /*  58: VICE_VIP1_INST1 off */
           0x8032d083,  /*  59: VICE1_CME_RMB_CMB RR 2000ns */
           0x8197708a,  /*  60: VICE1_CME_CSC RR 16000ns */
           0x805d6084,  /*  61: VICE1_FME_CSC RR 3670ns */
           0x805d6086,  /*  62: VICE1_FME_Luma_CMB RR 3670ns */
           0x805d6085,  /*  63: VICE1_FME_Chroma_CMB RR 3670ns */
           0x3ffff04c,  /*  64: VICE1_SG off */
           0x80000099,  /*  65: VICE1_DBLK RR 0ns */
           0x3ffff051,  /*  66: VICE1_CABAC0 off */
           0x3ffff058,  /*  67: VICE1_CABAC1 off */
           0x804d9089,  /*  68: VICE1_ARCSS0 RR 3050ns */
           0x8063d021,  /*  69: VICE1_VIP0_INST0 RR 3703ns */
           0x80c7a043,  /*  70: VICE1_VIP1_INST0 RR 7400ns */
           0x8063d022,  /*  71: VICE1_VIP0_INST1 RR 3703ns */
           0x80c7a044,  /*  72: VICE1_VIP1_INST1 RR 7400ns */
           0x800a608d,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x8005208e,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x3ffff061,  /*  75: HVD0_ILCPU off */
           0x3ffff06a,  /*  76: HVD0_OLCPU off */
           0x3ffff019,  /*  77: HVD0_CAB off */
           0x3ffff029,  /*  78: HVD0_ILSI off */
           0x81ea307e,  /*  79: HVD0_ILCPU_p2 RR 18162ns */
           0x3ffff02a,  /*  80: HVD0_ILSI_p2 off */
           0x800d3095,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x80068096,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x3ffff062,  /*  83: HVD1_ILCPU off */
           0x3ffff06b,  /*  84: HVD1_OLCPU off */
           0x3ffff01a,  /*  85: HVD1_CAB off */
           0x3ffff027,  /*  86: HVD1_ILSI off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff097,  /*  88: HVD2_DBLK_0 off */
           0x3ffff098,  /*  89: HVD2_DBLK_1 off */
           0x3ffff064,  /*  90: HVD2_ILCPU off */
           0x3ffff070,  /*  91: HVD2_OLCPU off */
           0x3ffff02c,  /*  92: HVD2_CAB off */
           0x3ffff02b,  /*  93: HVD2_ILSI off */
           0x3ffff01f,  /*  94: BVN_MAD_PIX_FD off */
           0x3ffff031,  /*  95: BVN_MAD_QUANT off */
           0x3ffff042,  /*  96: BVN_MAD_PIX_CAP off */
           0x3ffff01b,  /*  97: BVN_MAD1_PIX_FD off */
           0x3ffff02d,  /*  98: BVN_MAD1_QUANT off */
           0x3ffff03e,  /*  99: BVN_MAD1_PIX_CAP off */
           0x3ffff01c,  /* 100: BVN_MAD2_PIX_FD off */
           0x3ffff02e,  /* 101: BVN_MAD2_QUANT off */
           0x3ffff03f,  /* 102: BVN_MAD2_PIX_CAP off */
           0x3ffff01d,  /* 103: BVN_MAD3_PIX_FD off */
           0x3ffff02f,  /* 104: BVN_MAD3_QUANT off */
           0x3ffff040,  /* 105: BVN_MAD3_PIX_CAP off */
           0x0063d020,  /* 106: BVN_MFD0 3703ns */
           0x0015a000,  /* 107: BVN_MFD0_1 806ns */
           0x00853032,  /* 108: BVN_MFD1 4938ns */
           0x00853033,  /* 109: BVN_MFD1_1 4938ns */
           0x3ffff034,  /* 110: BVN_MFD2 off */
           0x3ffff035,  /* 111: BVN_MFD2_1 off */
           0x3ffff036,  /* 112: BVN_MFD3 off */
           0x3ffff037,  /* 113: BVN_MFD3_1 off */
           0x00853038,  /* 114: BVN_MFD4 4938ns */
           0x00853039,  /* 115: BVN_MFD4_1 4938ns */
           0x0085303a,  /* 116: BVN_MFD5 4938ns */
           0x0085303b,  /* 117: BVN_MFD5_1 4938ns */
           0x3ffff006,  /* 118: BVN_VFD0 off */
           0x3ffff007,  /* 119: BVN_VFD1 off */
           0x3ffff056,  /* 120: BVN_VFD2 off */
           0x3ffff057,  /* 121: BVN_VFD3 off */
           0x3ffff00f,  /* 122: BVN_VFD4 off */
           0x3ffff010,  /* 123: BVN_VFD5 off */
           0x3ffff011,  /* 124: BVN_VFD6 off */
           0x3ffff012,  /* 125: BVN_VFD7 off */
           0x3ffff001,  /* 126: BVN_CAP0 off */
           0x3ffff004,  /* 127: BVN_CAP1 off */
           0x3ffff04f,  /* 128: BVN_CAP2 off */
           0x3ffff050,  /* 129: BVN_CAP3 off */
           0x3ffff013,  /* 130: BVN_CAP4 off */
           0x3ffff014,  /* 131: BVN_CAP5 off */
           0x3ffff015,  /* 132: BVN_CAP6 off */
           0x3ffff016,  /* 133: BVN_CAP7 off */
           0x0031d009,  /* 134: BVN_GFD0 1851ns */
           0x3ffff04e,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff00e,  /* 141: BVN_MCVP0 off */
           0x3ffff00d,  /* 142: BVN_MCVP1 off */
           0x3ffff04a,  /* 143: BVN_MCVP2 off */
           0x00571018,  /* 144: BVN_RDC 3230ns */
           0x3ffff054,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff055,  /* 146: VEC_VBI_ENC1 off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff026,  /* 151: VICE_VIP0_INST2 off */
           0x3ffff048,  /* 152: VICE_VIP1_INST2 off */
           0x8063d023,  /* 153: VICE1_VIP0_INST2 RR 3703ns */
           0x80c7a045,  /* 154: VICE1_VIP1_INST2 RR 7400ns */
           0x800a6091,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x80052092,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff01e,  /* 157: BVN_MAD4_PIX_FD off */
           0x3ffff030,  /* 158: BVN_MAD4_QUANT off */
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
           0x80142060,  /* 200: CPU_MCP_RD_HIGH RR 750ns */
           0x800000a7,  /* 201: CPU_MCP_RD_LOW RR */
           0x80261063,  /* 202: CPU_MCP_WR_HIGH RR 1500ns */
           0x800000a8,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000009e,  /* 216: HVD0_PFRI RR 0ns */
           0x8000009f,  /* 217: HVD1_PFRI RR 0ns */
           0x3ffff0a0,  /* 218: HVD2_PFRI off */
           0x3ffff09c,  /* 219: VICE_PFRI off */
           0x8000009b,  /* 220: VICE1_PFRI RR 0ns */
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
           0xbfffe082,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2704b   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20150624005725_7445_4k2t[] = {
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_CONFIG,      0x80a20803}, /* VICE_PFRI (gVice) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH1,     0x000000f9}, /* 60% * 416 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80a30803}, /* HVD0_PFRI_Ch (gHvdC0) 259733.33 ns/120 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x400000e9}, /* d: 4; p: 233.758333333333 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000072}, /* 114 */
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000044}, /* 60% * 114 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_CONFIG,      0x80a60802}, /* HVD2_PFRI (gHvd2) 530666.67 ns/60 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_FILTER_CTRL, 0x400003bb}, /* d: 4; p: 955.195833333333 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH0,     0x00000a6e}, /* 2670 */
  {BCHP_MEMC_GEN_1_PFRI_2_THROTTLE_THRESH1,     0x00000642}, /* 60% * 2670 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_CONFIG,      0x80a40802}, /* HVD0_PFRI (gHvd0) 343733.33 ns/60 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000026a}, /* d: 4; p: 618.716666666667 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH0,     0x00000a6e}, /* 2670 */
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH1,     0x00000642}, /* 60% * 2670 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_CONFIG,      0x80a50802}, /* HVD1_PFRI (gHvd1) 349066.67 ns/60 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000274}, /* d: 4; p: 628.316666666667 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_THRESH0,     0x00000a6e}, /* 2670 */
  {BCHP_MEMC_GEN_2_PFRI_1_THROTTLE_THRESH1,     0x00000642}, /* 60% * 2670 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_CONFIG,      0x80a10803}, /* VICE1_PFRI (gVice1) 220588.24 ns/120 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_FILTER_CTRL, 0x400000c6}, /* d: 4; p: 198.529166666667 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_2_PFRI_4_THROTTLE_THRESH1,     0x000000f9}  /* 60% * 416 */
};

static const uint32_t* const paulMemc_box9[] = { &aulMemc0_20150624005725_7445_4k2t[0], &aulMemc1_20150624005725_7445_4k2t[0], &aulMemc2_20150624005725_7445_4k2t[0]};

const BBOX_Rts stBoxRts_7445_4k2t_box9 = {
  "20150624005725_7445_4k2t_box9",
  7445,
  9,
  3,
  256,
  (const uint32_t**)&paulMemc_box9[0],
  24,
  stBoxRts_PfriClient_20150624005725_7445_4k2t
};
