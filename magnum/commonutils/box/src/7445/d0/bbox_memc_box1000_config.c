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
*   at: Tue Jul 29 21:20:47 2014 GMT
*   by: robinc
*   for: Box 7445_TEMP
*         MemC 0 (32-bit DDRDDR3@1067MHz) w/432MHz clock
*         MemC 1 (32-bit DDRDDR3@1067MHz) w/432MHz clock
*         MemC 2 (32-bit DDRDDR3@1067MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */
#include "bchp_memc_gen_2.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20140729212047_7445_TEMP[] = {
           0x001e7004,  /*   0: XPT_WR_RS 1134.2383107089ns*/
           0x8038d05d,  /*   1: XPT_WR_XC RR 2234.76968796434ns*/
           0x8047100b,  /*   2: XPT_WR_CDB RR 2638.59649122807ns*/
           0x80b5e06b,  /*   3: XPT_WR_ITB_MSG RR 7144.89311163895ns*/
           0x80780027,  /*   4: XPT_RD_RS RR 4449.70414201183ns*/
           0x819e3049,  /*   5: XPT_RD_XC_RMX_MSG RR 15346.9387755102ns*/
           0x8047100a,  /*   6: XPT_RD_XC_RAVE RR 2638.59649122807ns*/
           0x809b9067,  /*   7: XPT_RD_PB RR 6113.4328358209ns*/
           0x80fd706c,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns*/
           0x82438075,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns*/
           0x803fd008,  /*  10: SYSPORT_WR RR 2370ns*/
           0x80ae906a,  /*  11: SYSPORT_RD RR 6860ns*/
           0x3ffff0ff,  /*  12: UNASSIGNED off*/
           0x3ffff0ff,  /*  13: UNASSIGNED off*/
           0x3ffff0ff,  /*  14: UNASSIGNED off*/
           0x81617072,  /*  15: HIF_PCIe1 RR 13880ns*/
           0x8596e056,  /*  16: MOCA_MIPS RR 53000ns*/
           0x80662063,  /*  17: SATA RR 4015.6862745098ns*/
           0x80662064,  /*  18: SATA_1 RR 4015.6862745098ns*/
           0x03e6e051,  /*  19: MCIF2_RD 37000ns*/
           0x03e6e053,  /*  20: MCIF2_WR 37000ns*/
           0x3ffff0ff,  /*  21: UNASSIGNED off*/
           0x8545e055,  /*  22: BSP RR 50000ns*/
           0x80ad9068,  /*  23: SAGE RR 6820ns*/
           0x86449076,  /*  24: FLASH_DMA RR 63000ns*/
           0x81617071,  /*  25: HIF_PCIe RR 13880ns*/
           0x86449078,  /*  26: SDIO_EMMC RR 63000ns*/
           0x86449077,  /*  27: SDIO_CARD RR 63000ns*/
           0xbffff0ff,  /*  28: TPCAP RR*/
           0x03e6e052,  /*  29: MCIF_RD 37000ns*/
           0x03e6e054,  /*  30: MCIF_WR 37000ns*/
           0xbffff0ff,  /*  31: UART_DMA_RD RR*/
           0xbffff0ff,  /*  32: UART_DMA_WR RR*/
           0x810db06d,  /*  33: USB_HI_0 RR 10593ns*/
           0xbffff0ff,  /*  34: USB_LO_0 RR*/
           0x815c5070,  /*  35: USB_X_WRITE_0 RR 13680ns*/
           0x815c506f,  /*  36: USB_X_READ_0 RR 13680ns*/
           0x80ae1069,  /*  37: USB_X_CTRL_0 RR 6840ns*/
           0x810db06e,  /*  38: USB_HI_1 RR 10593ns*/
           0xbffff0ff,  /*  39: USB_LO_1 RR*/
           0x0050e016,  /*  40: RAAGA 3000ns*/
           0x001ae001,  /*  41: RAAGA_1 1000ns*/
           0x00a1e036,  /*  42: RAAGA1 6000ns*/
           0x001ae000,  /*  43: RAAGA1_1 1000ns*/
           0x00bb4037,  /*  44: AUD_AIO 6940ns*/
           0x8032d05c,  /*  45: VICE_CME_RMB_CMB RR 2000ns*/
           0x81977073,  /*  46: VICE_CME_CSC RR 16000ns*/
           0x805d6060,  /*  47: VICE_FME_CSC RR 3670ns*/
           0x805d6062,  /*  48: VICE_FME_Luma_CMB RR 3670ns*/
           0x805d6061,  /*  49: VICE_FME_Chroma_CMB RR 3670ns*/
           0x80dca046,  /*  50: VICE_SG RR 8176.66666666667ns*/
           0x80002080,  /*  51: VICE_DBLK RR 10ns*/
           0x81edf04b,  /*  52: VICE_CABAC0 RR 18300ns*/
           0x83782050,  /*  53: VICE_CABAC1 RR 32900ns*/
           0x804d905f,  /*  54: VICE_ARCSS0 RR 3050ns*/
           0x8063d020,  /*  55: VICE_VIP0_INST0 RR 3703ns*/
           0x80c7a03f,  /*  56: VICE_VIP1_INST0 RR 7400ns*/
           0x8063d021,  /*  57: VICE_VIP0_INST1 RR 3703ns*/
           0x80c7a040,  /*  58: VICE_VIP1_INST1 RR 7400ns*/
           0x3ffff07a,  /*  59: VICE1_CME_RMB_CMB off*/
           0x3ffff07f,  /*  60: VICE1_CME_CSC off*/
           0x3ffff07b,  /*  61: VICE1_FME_CSC off*/
           0x3ffff07d,  /*  62: VICE1_FME_Luma_CMB off*/
           0x3ffff07c,  /*  63: VICE1_FME_Chroma_CMB off*/
           0x80dca045,  /*  64: VICE1_SG RR 8176.66666666667ns*/
           0x3ffff089,  /*  65: VICE1_DBLK off*/
           0x81edf04a,  /*  66: VICE1_CABAC0 RR 18300ns*/
           0x8378204f,  /*  67: VICE1_CABAC1 RR 32900ns*/
           0x3ffff07e,  /*  68: VICE1_ARCSS0 off*/
           0x3ffff01d,  /*  69: VICE1_VIP0_INST0 off*/
           0x3ffff03c,  /*  70: VICE1_VIP1_INST0 off*/
           0x3ffff01e,  /*  71: VICE1_VIP0_INST1 off*/
           0x3ffff03d,  /*  72: VICE1_VIP1_INST1 off*/
           0x3ffff081,  /*  73: HVD0_DBLK_0 off*/
           0x3ffff082,  /*  74: HVD0_DBLK_1 off*/
           0x801c2058,  /*  75: HVD0_ILCPU RR 1048ns*/
           0x804a705e,  /*  76: HVD0_OLCPU RR 2927ns*/
           0x00596018,  /*  77: HVD0_CAB 3317ns*/
           0x00710023,  /*  78: HVD0_ILSI 4191ns*/
           0x81ea3074,  /*  79: HVD0_ILCPU_p2 RR 18162ns*/
           0x00710024,  /*  80: HVD0_ILSI_p2 4191ns*/
           0x8000008a,  /*  81: HVD1_DBLK_0 RR 0ns*/
           0x8000008b,  /*  82: HVD1_DBLK_1 RR 0ns*/
           0x8027005a,  /*  83: HVD1_ILCPU RR 1451ns*/
           0x8070a065,  /*  84: HVD1_OLCPU RR 4427ns*/
           0x007dd028,  /*  85: HVD1_CAB 4666ns*/
           0x0073d025,  /*  86: HVD1_ILSI 4296ns*/
           0xbffff0ff,  /*  87: SID RR*/
           0x8000008c,  /*  88: HVD2_DBLK_0 RR 0ns*/
           0x8000008d,  /*  89: HVD2_DBLK_1 RR 0ns*/
           0x8027005b,  /*  90: HVD2_ILCPU RR 1451ns*/
           0x8070a066,  /*  91: HVD2_OLCPU RR 4427ns*/
           0x007dd029,  /*  92: HVD2_CAB 4666ns*/
           0x0073d026,  /*  93: HVD2_ILSI 4296ns*/
           0x3ffff0ff,  /*  94: UNASSIGNED off*/
           0x3ffff0ff,  /*  95: UNASSIGNED off*/
           0x3ffff0ff,  /*  96: UNASSIGNED off*/
           0x3ffff019,  /*  97: BVN_MAD1_PIX_FD off*/
           0x3ffff032,  /*  98: BVN_MAD1_QUANT off*/
           0x3ffff038,  /*  99: BVN_MAD1_PIX_CAP off*/
           0x3ffff01a,  /* 100: BVN_MAD2_PIX_FD off*/
           0x3ffff033,  /* 101: BVN_MAD2_QUANT off*/
           0x3ffff039,  /* 102: BVN_MAD2_PIX_CAP off*/
           0x3ffff01b,  /* 103: BVN_MAD3_PIX_FD off*/
           0x3ffff034,  /* 104: BVN_MAD3_QUANT off*/
           0x3ffff03a,  /* 105: BVN_MAD3_PIX_CAP off*/
           0x3ffff009,  /* 106: BVN_MFD0 off*/
           0x3ffff002,  /* 107: BVN_MFD0_1 off*/
           0x3ffff0ff,  /* 108: UNASSIGNED off*/
           0x3ffff0ff,  /* 109: UNASSIGNED off*/
           0x0085302a,  /* 110: BVN_MFD2 4938ns*/
           0x0085302b,  /* 111: BVN_MFD2_1 4938ns*/
           0x0085302c,  /* 112: BVN_MFD3 4938ns*/
           0x0085302d,  /* 113: BVN_MFD3_1 4938ns*/
           0x0085302e,  /* 114: BVN_MFD4 4938ns*/
           0x0085302f,  /* 115: BVN_MFD4_1 4938ns*/
           0x00853030,  /* 116: BVN_MFD5 4938ns*/
           0x00853031,  /* 117: BVN_MFD5_1 4938ns*/
           0x3ffff005,  /* 118: BVN_VFD0 off*/
           0x3ffff0ff,  /* 119: UNASSIGNED off*/
           0x3ffff04e,  /* 120: BVN_VFD2 off*/
           0x3ffff0ff,  /* 121: UNASSIGNED off*/
           0x3ffff00e,  /* 122: BVN_VFD4 off*/
           0x3ffff00f,  /* 123: BVN_VFD5 off*/
           0x3ffff010,  /* 124: BVN_VFD6 off*/
           0x3ffff011,  /* 125: BVN_VFD7 off*/
           0x3ffff003,  /* 126: BVN_CAP0 off*/
           0x3ffff0ff,  /* 127: UNASSIGNED off*/
           0x3ffff048,  /* 128: BVN_CAP2 off*/
           0x3ffff0ff,  /* 129: UNASSIGNED off*/
           0x3ffff012,  /* 130: BVN_CAP4 off*/
           0x3ffff013,  /* 131: BVN_CAP5 off*/
           0x3ffff014,  /* 132: BVN_CAP6 off*/
           0x3ffff015,  /* 133: BVN_CAP7 off*/
           0x3ffff006,  /* 134: BVN_GFD0 off*/
           0x3ffff047,  /* 135: BVN_GFD1 off*/
           0x3ffff0ff,  /* 136: UNASSIGNED off*/
           0x3ffff0ff,  /* 137: UNASSIGNED off*/
           0x3ffff0ff,  /* 138: UNASSIGNED off*/
           0x3ffff0ff,  /* 139: UNASSIGNED off*/
           0x3ffff0ff,  /* 140: UNASSIGNED off*/
           0x3ffff00d,  /* 141: BVN_MCVP0 off*/
           0x3ffff00c,  /* 142: BVN_MCVP1 off*/
           0x3ffff044,  /* 143: BVN_MCVP2 off*/
           0x3ffff017,  /* 144: BVN_RDC off*/
           0x0352604c,  /* 145: VEC_VBI_ENC0 31500ns*/
           0x0352604d,  /* 146: VEC_VBI_ENC1 31500ns*/
           0xbffff0ff,  /* 147: M2MC_0 RR*/
           0xbffff0ff,  /* 148: M2MC_1 RR*/
           0xbffff0ff,  /* 149: M2MC_2 RR*/
           0x3ffff0ff,  /* 150: UNASSIGNED off*/
           0x8063d022,  /* 151: VICE_VIP0_INST2 RR 3703ns*/
           0x80c7a041,  /* 152: VICE_VIP1_INST2 RR 7400ns*/
           0x3ffff01f,  /* 153: VICE1_VIP0_INST2 off*/
           0x3ffff03e,  /* 154: VICE1_VIP1_INST2 off*/
           0x3ffff085,  /* 155: HVD0_DBLK_p2_0 off*/
           0x3ffff086,  /* 156: HVD0_DBLK_p2_1 off*/
           0x3ffff01c,  /* 157: BVN_MAD4_PIX_FD off*/
           0x3ffff035,  /* 158: BVN_MAD4_QUANT off*/
           0x3ffff03b,  /* 159: BVN_MAD4_PIX_CAP off*/
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
           0x80142057,  /* 200: CPU_MCP_RD_HIGH RR 750ns*/
           0x8000009a,  /* 201: CPU_MCP_RD_LOW RR*/
           0x80261059,  /* 202: CPU_MCP_WR_HIGH RR 1500ns*/
           0x8000009b,  /* 203: CPU_MCP_WR_LOW RR*/
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
           0x3ffff090,  /* 216: HVD0_PFRI off*/
           0x80000092,  /* 217: HVD1_PFRI RR 0ns*/
           0x80000093,  /* 218: HVD2_PFRI RR 0ns*/
           0x8000208f,  /* 219: VICE_PFRI RR 10ns*/
           0x3ffff08e,  /* 220: VICE1_PFRI off*/
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
           0xbfffe079,  /* 250: MEMC_ZQCS RR 1000000ns*/
           0xbffff0ff,  /* 251: MEMC_MSA RR*/
           0xbffff0ff,  /* 252: MEMC_DIS0 RR*/
           0xbffff0ff,  /* 253: MEMC_DIS1 RR*/
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR*/
           0x00d2d043,  /* 255: REFRESH 7812.5ns*/
         };
static const uint32_t aulMemc1_20140729212047_7445_TEMP[] = {
           0x3ffff004,  /*   0: XPT_WR_RS off*/
           0x3ffff05d,  /*   1: XPT_WR_XC off*/
           0x3ffff00b,  /*   2: XPT_WR_CDB off*/
           0x3ffff06b,  /*   3: XPT_WR_ITB_MSG off*/
           0x3ffff027,  /*   4: XPT_RD_RS off*/
           0x3ffff049,  /*   5: XPT_RD_XC_RMX_MSG off*/
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off*/
           0x3ffff067,  /*   7: XPT_RD_PB off*/
           0x80fd706c,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns*/
           0x82438075,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns*/
           0x803fd008,  /*  10: SYSPORT_WR RR 2370ns*/
           0x80ae906a,  /*  11: SYSPORT_RD RR 6860ns*/
           0x3ffff0ff,  /*  12: UNASSIGNED off*/
           0x3ffff0ff,  /*  13: UNASSIGNED off*/
           0x3ffff0ff,  /*  14: UNASSIGNED off*/
           0x81617072,  /*  15: HIF_PCIe1 RR 13880ns*/
           0x8596e056,  /*  16: MOCA_MIPS RR 53000ns*/
           0x80662063,  /*  17: SATA RR 4015.6862745098ns*/
           0x80662064,  /*  18: SATA_1 RR 4015.6862745098ns*/
           0x3ffff051,  /*  19: MCIF2_RD off*/
           0x3ffff053,  /*  20: MCIF2_WR off*/
           0x3ffff0ff,  /*  21: UNASSIGNED off*/
           0x8545e055,  /*  22: BSP RR 50000ns*/
           0x80ad9068,  /*  23: SAGE RR 6820ns*/
           0x86449076,  /*  24: FLASH_DMA RR 63000ns*/
           0x81617071,  /*  25: HIF_PCIe RR 13880ns*/
           0x86449078,  /*  26: SDIO_EMMC RR 63000ns*/
           0x86449077,  /*  27: SDIO_CARD RR 63000ns*/
           0x3ffff0ff,  /*  28: TPCAP off*/
           0x3ffff052,  /*  29: MCIF_RD off*/
           0x3ffff054,  /*  30: MCIF_WR off*/
           0x3ffff0ff,  /*  31: UART_DMA_RD off*/
           0x3ffff0ff,  /*  32: UART_DMA_WR off*/
           0x810db06d,  /*  33: USB_HI_0 RR 10593ns*/
           0xbffff0ff,  /*  34: USB_LO_0 RR*/
           0x815c5070,  /*  35: USB_X_WRITE_0 RR 13680ns*/
           0x815c506f,  /*  36: USB_X_READ_0 RR 13680ns*/
           0x80ae1069,  /*  37: USB_X_CTRL_0 RR 6840ns*/
           0x810db06e,  /*  38: USB_HI_1 RR 10593ns*/
           0xbffff0ff,  /*  39: USB_LO_1 RR*/
           0x3ffff016,  /*  40: RAAGA off*/
           0x3ffff001,  /*  41: RAAGA_1 off*/
           0x3ffff036,  /*  42: RAAGA1 off*/
           0x3ffff000,  /*  43: RAAGA1_1 off*/
           0x3ffff037,  /*  44: AUD_AIO off*/
           0x3ffff05c,  /*  45: VICE_CME_RMB_CMB off*/
           0x3ffff073,  /*  46: VICE_CME_CSC off*/
           0x3ffff060,  /*  47: VICE_FME_CSC off*/
           0x3ffff062,  /*  48: VICE_FME_Luma_CMB off*/
           0x3ffff061,  /*  49: VICE_FME_Chroma_CMB off*/
           0x3ffff046,  /*  50: VICE_SG off*/
           0x3ffff080,  /*  51: VICE_DBLK off*/
           0x3ffff04b,  /*  52: VICE_CABAC0 off*/
           0x3ffff050,  /*  53: VICE_CABAC1 off*/
           0x3ffff05f,  /*  54: VICE_ARCSS0 off*/
           0x3ffff020,  /*  55: VICE_VIP0_INST0 off*/
           0x3ffff03f,  /*  56: VICE_VIP1_INST0 off*/
           0x3ffff021,  /*  57: VICE_VIP0_INST1 off*/
           0x3ffff040,  /*  58: VICE_VIP1_INST1 off*/
           0x8032d07a,  /*  59: VICE1_CME_RMB_CMB RR 2000ns*/
           0x8197707f,  /*  60: VICE1_CME_CSC RR 16000ns*/
           0x805d607b,  /*  61: VICE1_FME_CSC RR 3670ns*/
           0x805d607d,  /*  62: VICE1_FME_Luma_CMB RR 3670ns*/
           0x805d607c,  /*  63: VICE1_FME_Chroma_CMB RR 3670ns*/
           0x3ffff045,  /*  64: VICE1_SG off*/
           0x80002089,  /*  65: VICE1_DBLK RR 10ns*/
           0x3ffff04a,  /*  66: VICE1_CABAC0 off*/
           0x3ffff04f,  /*  67: VICE1_CABAC1 off*/
           0x804d907e,  /*  68: VICE1_ARCSS0 RR 3050ns*/
           0x8063d01d,  /*  69: VICE1_VIP0_INST0 RR 3703ns*/
           0x80c7a03c,  /*  70: VICE1_VIP1_INST0 RR 7400ns*/
           0x8063d01e,  /*  71: VICE1_VIP0_INST1 RR 3703ns*/
           0x80c7a03d,  /*  72: VICE1_VIP1_INST1 RR 7400ns*/
           0x80000083,  /*  73: HVD0_DBLK_Ch_0 RR 0ns*/
           0x80000084,  /*  74: HVD0_DBLK_Ch_1 RR 0ns*/
           0x3ffff058,  /*  75: HVD0_ILCPU off*/
           0x3ffff05e,  /*  76: HVD0_OLCPU off*/
           0x3ffff018,  /*  77: HVD0_CAB off*/
           0x3ffff023,  /*  78: HVD0_ILSI off*/
           0x81ea3074,  /*  79: HVD0_ILCPU_p2 RR 18162ns*/
           0x3ffff024,  /*  80: HVD0_ILSI_p2 off*/
           0x3ffff08a,  /*  81: HVD1_DBLK_0 off*/
           0x3ffff08b,  /*  82: HVD1_DBLK_1 off*/
           0x3ffff05a,  /*  83: HVD1_ILCPU off*/
           0x3ffff065,  /*  84: HVD1_OLCPU off*/
           0x3ffff028,  /*  85: HVD1_CAB off*/
           0x3ffff025,  /*  86: HVD1_ILSI off*/
           0xbffff0ff,  /*  87: SID RR*/
           0x3ffff08c,  /*  88: HVD2_DBLK_0 off*/
           0x3ffff08d,  /*  89: HVD2_DBLK_1 off*/
           0x3ffff05b,  /*  90: HVD2_ILCPU off*/
           0x3ffff066,  /*  91: HVD2_OLCPU off*/
           0x3ffff029,  /*  92: HVD2_CAB off*/
           0x3ffff026,  /*  93: HVD2_ILSI off*/
           0x3ffff0ff,  /*  94: UNASSIGNED off*/
           0x3ffff0ff,  /*  95: UNASSIGNED off*/
           0x3ffff0ff,  /*  96: UNASSIGNED off*/
           0x005ea019,  /*  97: BVN_MAD1_PIX_FD 3511ns*/
           0x00853032,  /*  98: BVN_MAD1_QUANT 4938ns*/
           0x00bd7038,  /*  99: BVN_MAD1_PIX_CAP 7023ns*/
           0x005ea01a,  /* 100: BVN_MAD2_PIX_FD 3511ns*/
           0x00853033,  /* 101: BVN_MAD2_QUANT 4938ns*/
           0x00bd7039,  /* 102: BVN_MAD2_PIX_CAP 7023ns*/
           0x005ea01b,  /* 103: BVN_MAD3_PIX_FD 3511ns*/
           0x00853034,  /* 104: BVN_MAD3_QUANT 4938ns*/
           0x00bd703a,  /* 105: BVN_MAD3_PIX_CAP 7023ns*/
           0x00c7d042,  /* 106: BVN_MFD0_Ch 7407ns*/
           0x0038f007,  /* 107: BVN_MFD0_Ch_1 2115ns*/
           0x3ffff0ff,  /* 108: UNASSIGNED off*/
           0x3ffff0ff,  /* 109: UNASSIGNED off*/
           0x3ffff02a,  /* 110: BVN_MFD2 off*/
           0x3ffff02b,  /* 111: BVN_MFD2_1 off*/
           0x3ffff02c,  /* 112: BVN_MFD3 off*/
           0x3ffff02d,  /* 113: BVN_MFD3_1 off*/
           0x3ffff02e,  /* 114: BVN_MFD4 off*/
           0x3ffff02f,  /* 115: BVN_MFD4_1 off*/
           0x3ffff030,  /* 116: BVN_MFD5 off*/
           0x3ffff031,  /* 117: BVN_MFD5_1 off*/
           0x3ffff005,  /* 118: BVN_VFD0 off*/
           0x3ffff0ff,  /* 119: UNASSIGNED off*/
           0x3ffff04e,  /* 120: BVN_VFD2 off*/
           0x3ffff0ff,  /* 121: UNASSIGNED off*/
           0x004fc00e,  /* 122: BVN_VFD4 2960ns*/
           0x004fc00f,  /* 123: BVN_VFD5 2960ns*/
           0x004fc010,  /* 124: BVN_VFD6 2960ns*/
           0x004fc011,  /* 125: BVN_VFD7 2960ns*/
           0x3ffff003,  /* 126: BVN_CAP0 off*/
           0x3ffff0ff,  /* 127: UNASSIGNED off*/
           0x3ffff048,  /* 128: BVN_CAP2 off*/
           0x3ffff0ff,  /* 129: UNASSIGNED off*/
           0x004fc012,  /* 130: BVN_CAP4 2960ns*/
           0x004fc013,  /* 131: BVN_CAP5 2960ns*/
           0x004fc014,  /* 132: BVN_CAP6 2960ns*/
           0x004fc015,  /* 133: BVN_CAP7 2960ns*/
           0x0031d006,  /* 134: BVN_GFD0 1851ns*/
           0x3ffff047,  /* 135: BVN_GFD1 off*/
           0x3ffff0ff,  /* 136: UNASSIGNED off*/
           0x3ffff0ff,  /* 137: UNASSIGNED off*/
           0x3ffff0ff,  /* 138: UNASSIGNED off*/
           0x3ffff0ff,  /* 139: UNASSIGNED off*/
           0x3ffff0ff,  /* 140: UNASSIGNED off*/
           0x3ffff00d,  /* 141: BVN_MCVP0 off*/
           0x3ffff00c,  /* 142: BVN_MCVP1 off*/
           0x3ffff044,  /* 143: BVN_MCVP2 off*/
           0x3ffff017,  /* 144: BVN_RDC off*/
           0x3ffff04c,  /* 145: VEC_VBI_ENC0 off*/
           0x3ffff04d,  /* 146: VEC_VBI_ENC1 off*/
           0xbffff0ff,  /* 147: M2MC_0 RR*/
           0xbffff0ff,  /* 148: M2MC_1 RR*/
           0xbffff0ff,  /* 149: M2MC_2 RR*/
           0x3ffff0ff,  /* 150: UNASSIGNED off*/
           0x3ffff022,  /* 151: VICE_VIP0_INST2 off*/
           0x3ffff041,  /* 152: VICE_VIP1_INST2 off*/
           0x8063d01f,  /* 153: VICE1_VIP0_INST2 RR 3703ns*/
           0x80c7a03e,  /* 154: VICE1_VIP1_INST2 RR 7400ns*/
           0x80000087,  /* 155: HVD0_DBLK_p2_Ch_0 RR 0ns*/
           0x80000088,  /* 156: HVD0_DBLK_p2_Ch_1 RR 0ns*/
           0x005ea01c,  /* 157: BVN_MAD4_PIX_FD 3511ns*/
           0x00853035,  /* 158: BVN_MAD4_QUANT 4938ns*/
           0x00bd703b,  /* 159: BVN_MAD4_PIX_CAP 7023ns*/
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
           0x80142057,  /* 200: CPU_MCP_RD_HIGH RR 750ns*/
           0x8000009a,  /* 201: CPU_MCP_RD_LOW RR*/
           0x80261059,  /* 202: CPU_MCP_WR_HIGH RR 1500ns*/
           0x8000009b,  /* 203: CPU_MCP_WR_LOW RR*/
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
           0x80000091,  /* 216: HVD0_PFRI_Ch RR 0ns*/
           0x3ffff092,  /* 217: HVD1_PFRI off*/
           0x3ffff093,  /* 218: HVD2_PFRI off*/
           0x3ffff08f,  /* 219: VICE_PFRI off*/
           0x8000208e,  /* 220: VICE1_PFRI RR 10ns*/
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
           0xbfffe079,  /* 250: MEMC_ZQCS RR 1000000ns*/
           0xbffff0ff,  /* 251: MEMC_MSA RR*/
           0xbffff0ff,  /* 252: MEMC_DIS0 RR*/
           0xbffff0ff,  /* 253: MEMC_DIS1 RR*/
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR*/
           0x00d2d043,  /* 255: REFRESH 7812.5ns*/
         };
static const uint32_t aulMemc2_20140729212047_7445_TEMP[] = {
           0x3ffff004,  /*   0: XPT_WR_RS off*/
           0x3ffff05d,  /*   1: XPT_WR_XC off*/
           0x3ffff00b,  /*   2: XPT_WR_CDB off*/
           0x3ffff06b,  /*   3: XPT_WR_ITB_MSG off*/
           0x3ffff027,  /*   4: XPT_RD_RS off*/
           0x3ffff049,  /*   5: XPT_RD_XC_RMX_MSG off*/
           0x3ffff00a,  /*   6: XPT_RD_XC_RAVE off*/
           0x3ffff067,  /*   7: XPT_RD_PB off*/
           0x80fd706c,  /*   8: XPT_WR_MEMDMA RR 9955.55555555556ns*/
           0x82438075,  /*   9: XPT_RD_MEMDMA RR 22755.5555555556ns*/
           0x803fd008,  /*  10: SYSPORT_WR RR 2370ns*/
           0x80ae906a,  /*  11: SYSPORT_RD RR 6860ns*/
           0x3ffff0ff,  /*  12: UNASSIGNED off*/
           0x3ffff0ff,  /*  13: UNASSIGNED off*/
           0x3ffff0ff,  /*  14: UNASSIGNED off*/
           0x81617072,  /*  15: HIF_PCIe1 RR 13880ns*/
           0x8596e056,  /*  16: MOCA_MIPS RR 53000ns*/
           0x80662063,  /*  17: SATA RR 4015.6862745098ns*/
           0x80662064,  /*  18: SATA_1 RR 4015.6862745098ns*/
           0x3ffff051,  /*  19: MCIF2_RD off*/
           0x3ffff053,  /*  20: MCIF2_WR off*/
           0x3ffff0ff,  /*  21: UNASSIGNED off*/
           0x8545e055,  /*  22: BSP RR 50000ns*/
           0x80ad9068,  /*  23: SAGE RR 6820ns*/
           0x86449076,  /*  24: FLASH_DMA RR 63000ns*/
           0x81617071,  /*  25: HIF_PCIe RR 13880ns*/
           0x86449078,  /*  26: SDIO_EMMC RR 63000ns*/
           0x86449077,  /*  27: SDIO_CARD RR 63000ns*/
           0x3ffff0ff,  /*  28: TPCAP off*/
           0x3ffff052,  /*  29: MCIF_RD off*/
           0x3ffff054,  /*  30: MCIF_WR off*/
           0x3ffff0ff,  /*  31: UART_DMA_RD off*/
           0x3ffff0ff,  /*  32: UART_DMA_WR off*/
           0x810db06d,  /*  33: USB_HI_0 RR 10593ns*/
           0xbffff0ff,  /*  34: USB_LO_0 RR*/
           0x815c5070,  /*  35: USB_X_WRITE_0 RR 13680ns*/
           0x815c506f,  /*  36: USB_X_READ_0 RR 13680ns*/
           0x80ae1069,  /*  37: USB_X_CTRL_0 RR 6840ns*/
           0x810db06e,  /*  38: USB_HI_1 RR 10593ns*/
           0xbffff0ff,  /*  39: USB_LO_1 RR*/
           0x3ffff016,  /*  40: RAAGA off*/
           0x3ffff001,  /*  41: RAAGA_1 off*/
           0x3ffff036,  /*  42: RAAGA1 off*/
           0x3ffff000,  /*  43: RAAGA1_1 off*/
           0x3ffff037,  /*  44: AUD_AIO off*/
           0x3ffff05c,  /*  45: VICE_CME_RMB_CMB off*/
           0x3ffff073,  /*  46: VICE_CME_CSC off*/
           0x3ffff060,  /*  47: VICE_FME_CSC off*/
           0x3ffff062,  /*  48: VICE_FME_Luma_CMB off*/
           0x3ffff061,  /*  49: VICE_FME_Chroma_CMB off*/
           0x3ffff046,  /*  50: VICE_SG off*/
           0x3ffff080,  /*  51: VICE_DBLK off*/
           0x3ffff04b,  /*  52: VICE_CABAC0 off*/
           0x3ffff050,  /*  53: VICE_CABAC1 off*/
           0x3ffff05f,  /*  54: VICE_ARCSS0 off*/
           0x3ffff020,  /*  55: VICE_VIP0_INST0 off*/
           0x3ffff03f,  /*  56: VICE_VIP1_INST0 off*/
           0x3ffff021,  /*  57: VICE_VIP0_INST1 off*/
           0x3ffff040,  /*  58: VICE_VIP1_INST1 off*/
           0x3ffff07a,  /*  59: VICE1_CME_RMB_CMB off*/
           0x3ffff07f,  /*  60: VICE1_CME_CSC off*/
           0x3ffff07b,  /*  61: VICE1_FME_CSC off*/
           0x3ffff07d,  /*  62: VICE1_FME_Luma_CMB off*/
           0x3ffff07c,  /*  63: VICE1_FME_Chroma_CMB off*/
           0x3ffff045,  /*  64: VICE1_SG off*/
           0x3ffff089,  /*  65: VICE1_DBLK off*/
           0x3ffff04a,  /*  66: VICE1_CABAC0 off*/
           0x3ffff04f,  /*  67: VICE1_CABAC1 off*/
           0x3ffff07e,  /*  68: VICE1_ARCSS0 off*/
           0x3ffff01d,  /*  69: VICE1_VIP0_INST0 off*/
           0x3ffff03c,  /*  70: VICE1_VIP1_INST0 off*/
           0x3ffff01e,  /*  71: VICE1_VIP0_INST1 off*/
           0x3ffff03d,  /*  72: VICE1_VIP1_INST1 off*/
           0x80000081,  /*  73: HVD0_DBLK_0 RR 0ns*/
           0x80000082,  /*  74: HVD0_DBLK_1 RR 0ns*/
           0x3ffff058,  /*  75: HVD0_ILCPU off*/
           0x3ffff05e,  /*  76: HVD0_OLCPU off*/
           0x3ffff018,  /*  77: HVD0_CAB off*/
           0x3ffff023,  /*  78: HVD0_ILSI off*/
           0x81ea3074,  /*  79: HVD0_ILCPU_p2 RR 18162ns*/
           0x3ffff024,  /*  80: HVD0_ILSI_p2 off*/
           0x3ffff08a,  /*  81: HVD1_DBLK_0 off*/
           0x3ffff08b,  /*  82: HVD1_DBLK_1 off*/
           0x3ffff05a,  /*  83: HVD1_ILCPU off*/
           0x3ffff065,  /*  84: HVD1_OLCPU off*/
           0x3ffff028,  /*  85: HVD1_CAB off*/
           0x3ffff025,  /*  86: HVD1_ILSI off*/
           0xbffff0ff,  /*  87: SID RR*/
           0x3ffff08c,  /*  88: HVD2_DBLK_0 off*/
           0x3ffff08d,  /*  89: HVD2_DBLK_1 off*/
           0x3ffff05b,  /*  90: HVD2_ILCPU off*/
           0x3ffff066,  /*  91: HVD2_OLCPU off*/
           0x3ffff029,  /*  92: HVD2_CAB off*/
           0x3ffff026,  /*  93: HVD2_ILSI off*/
           0x3ffff0ff,  /*  94: UNASSIGNED off*/
           0x3ffff0ff,  /*  95: UNASSIGNED off*/
           0x3ffff0ff,  /*  96: UNASSIGNED off*/
           0x3ffff019,  /*  97: BVN_MAD1_PIX_FD off*/
           0x3ffff032,  /*  98: BVN_MAD1_QUANT off*/
           0x3ffff038,  /*  99: BVN_MAD1_PIX_CAP off*/
           0x3ffff01a,  /* 100: BVN_MAD2_PIX_FD off*/
           0x3ffff033,  /* 101: BVN_MAD2_QUANT off*/
           0x3ffff039,  /* 102: BVN_MAD2_PIX_CAP off*/
           0x3ffff01b,  /* 103: BVN_MAD3_PIX_FD off*/
           0x3ffff034,  /* 104: BVN_MAD3_QUANT off*/
           0x3ffff03a,  /* 105: BVN_MAD3_PIX_CAP off*/
           0x00428009,  /* 106: BVN_MFD0 2469ns*/
           0x001c6002,  /* 107: BVN_MFD0_1 1057.5ns*/
           0x3ffff0ff,  /* 108: UNASSIGNED off*/
           0x3ffff0ff,  /* 109: UNASSIGNED off*/
           0x3ffff02a,  /* 110: BVN_MFD2 off*/
           0x3ffff02b,  /* 111: BVN_MFD2_1 off*/
           0x3ffff02c,  /* 112: BVN_MFD3 off*/
           0x3ffff02d,  /* 113: BVN_MFD3_1 off*/
           0x3ffff02e,  /* 114: BVN_MFD4 off*/
           0x3ffff02f,  /* 115: BVN_MFD4_1 off*/
           0x3ffff030,  /* 116: BVN_MFD5 off*/
           0x3ffff031,  /* 117: BVN_MFD5_1 off*/
           0x001fd005,  /* 118: BVN_VFD0 1185ns*/
           0x3ffff0ff,  /* 119: UNASSIGNED off*/
           0x0359a04e,  /* 120: BVN_VFD2 31770ns*/
           0x3ffff0ff,  /* 121: UNASSIGNED off*/
           0x3ffff00e,  /* 122: BVN_VFD4 off*/
           0x3ffff00f,  /* 123: BVN_VFD5 off*/
           0x3ffff010,  /* 124: BVN_VFD6 off*/
           0x3ffff011,  /* 125: BVN_VFD7 off*/
           0x001e5003,  /* 126: BVN_CAP0 1128.594ns*/
           0x3ffff0ff,  /* 127: UNASSIGNED off*/
           0x01747048,  /* 128: BVN_CAP2 13800ns*/
           0x3ffff0ff,  /* 129: UNASSIGNED off*/
           0x3ffff012,  /* 130: BVN_CAP4 off*/
           0x3ffff013,  /* 131: BVN_CAP5 off*/
           0x3ffff014,  /* 132: BVN_CAP6 off*/
           0x3ffff015,  /* 133: BVN_CAP7 off*/
           0x3ffff006,  /* 134: BVN_GFD0 off*/
           0x0167c047,  /* 135: BVN_GFD1 13330ns*/
           0x3ffff0ff,  /* 136: UNASSIGNED off*/
           0x3ffff0ff,  /* 137: UNASSIGNED off*/
           0x3ffff0ff,  /* 138: UNASSIGNED off*/
           0x3ffff0ff,  /* 139: UNASSIGNED off*/
           0x3ffff0ff,  /* 140: UNASSIGNED off*/
           0x004b500d,  /* 141: BVN_MCVP0 2794ns*/
           0x004b500c,  /* 142: BVN_MCVP1 2794ns*/
           0x00d3d044,  /* 143: BVN_MCVP2 7850ns*/
           0x00571017,  /* 144: BVN_RDC 3230ns*/
           0x3ffff04c,  /* 145: VEC_VBI_ENC0 off*/
           0x3ffff04d,  /* 146: VEC_VBI_ENC1 off*/
           0xbffff0ff,  /* 147: M2MC_0 RR*/
           0xbffff0ff,  /* 148: M2MC_1 RR*/
           0xbffff0ff,  /* 149: M2MC_2 RR*/
           0x3ffff0ff,  /* 150: UNASSIGNED off*/
           0x3ffff022,  /* 151: VICE_VIP0_INST2 off*/
           0x3ffff041,  /* 152: VICE_VIP1_INST2 off*/
           0x3ffff01f,  /* 153: VICE1_VIP0_INST2 off*/
           0x3ffff03e,  /* 154: VICE1_VIP1_INST2 off*/
           0x80000085,  /* 155: HVD0_DBLK_p2_0 RR 0ns*/
           0x80000086,  /* 156: HVD0_DBLK_p2_1 RR 0ns*/
           0x3ffff01c,  /* 157: BVN_MAD4_PIX_FD off*/
           0x3ffff035,  /* 158: BVN_MAD4_QUANT off*/
           0x3ffff03b,  /* 159: BVN_MAD4_PIX_CAP off*/
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
           0x80142057,  /* 200: CPU_MCP_RD_HIGH RR 750ns*/
           0x8000009a,  /* 201: CPU_MCP_RD_LOW RR*/
           0x80261059,  /* 202: CPU_MCP_WR_HIGH RR 1500ns*/
           0x8000009b,  /* 203: CPU_MCP_WR_LOW RR*/
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
           0x80000090,  /* 216: HVD0_PFRI RR 0ns*/
           0x3ffff092,  /* 217: HVD1_PFRI off*/
           0x3ffff093,  /* 218: HVD2_PFRI off*/
           0x3ffff08f,  /* 219: VICE_PFRI off*/
           0x3ffff08e,  /* 220: VICE1_PFRI off*/
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
           0xbfffe079,  /* 250: MEMC_ZQCS RR 1000000ns*/
           0xbffff0ff,  /* 251: MEMC_MSA RR*/
           0xbffff0ff,  /* 252: MEMC_DIS0 RR*/
           0xbffff0ff,  /* 253: MEMC_DIS1 RR*/
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR*/
           0x00d2d043,  /* 255: REFRESH 7812.5ns*/
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20140729212047_7445_TEMP[] = {
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x80980802},
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x40000937},
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x00000a6e},
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x00000642},
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x80990802},
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x40000937},
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x00000a6e},
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x00000642},
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_CONFIG,      0x80950802},
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_FILTER_CTRL, 0x400001ea},
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH0,     0x000001a8},
  {BCHP_MEMC_GEN_0_PFRI_3_THROTTLE_THRESH1,     0x000000fe},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x80970802},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000616},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00000d40},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x000007f3},
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_CONFIG,      0x80940803},
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_FILTER_CTRL, 0x400001ea},
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH0,     0x000001a0},
  {BCHP_MEMC_GEN_1_PFRI_4_THROTTLE_THRESH1,     0x000000f9},
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_CONFIG,      0x80960802},
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000616},
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH0,     0x000016b3},
  {BCHP_MEMC_GEN_2_PFRI_0_THROTTLE_THRESH1,     0x00000d9f},
};

static const uint32_t * const paulMemc_box1000[] = { &aulMemc0_20140729212047_7445_TEMP[0], &aulMemc1_20140729212047_7445_TEMP[0], &aulMemc2_20140729212047_7445_TEMP[0],};

const BBOX_Rts stBoxRts_7445_TEMP_box1000 = {
  "20140729212047_7445_TEMP_box1000",
  7445,
  1000,
  3,
  256,
  (const uint32_t **)&paulMemc_box1000[0],
  24,
  stBoxRts_PfriClient_20140729212047_7445_TEMP,
};
