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
*   at: Thu Oct  8 23:04:08 2015 GMT
*   by: robinc
*   for: Box 7251_4kp251t
*         MemC 0 (32-bit DDR3@1067MHz) w/432MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20151008230408_7251_4kp251t[] = {
           0x003ce004,  /*   0: XPT_WR_RS 2260ns */
           0x8074804a,  /*   1: XPT_WR_XC RR 4580ns */
           0x80923020,  /*   2: XPT_WR_CDB RR 5420ns */
           0x81723058,  /*   3: XPT_WR_ITB_MSG RR 14540ns */
           0x80f0202e,  /*   4: XPT_RD_RS RR 8900ns */
           0x833cc036,  /*   5: XPT_RD_XC_RMX_MSG RR 30700ns */
           0x8092301f,  /*   6: XPT_RD_XC_RAVE RR 5420ns */
           0x8145e054,  /*   7: XPT_RD_PB RR 12800ns */
           0x813b2053,  /*   8: XPT_WR_MEMDMA RR 12376ns */
           0x82bce05c,  /*   9: XPT_RD_MEMDMA RR 27520ns */
           0x803fd006,  /*  10: GENET0_WR RR 2370ns */
           0x8102604e,  /*  11: GENET0_RD RR 10150ns */
           0x8089d01e,  /*  12: GENET1_WR RR 5110ns */
           0x8102604f,  /*  13: GENET1_RD RR 10150ns */
           0x803fd007,  /*  14: GENET2_WR RR 2370ns */
           0x81026050,  /*  15: GENET2_RD RR 10150ns */
           0x8596e040,  /*  16: MOCA_MIPS RR 53000ns */
           0x80662049,  /*  17: SATA RR 4015ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x03e6e03b,  /*  19: MCIF2_RD 37000ns */
           0x03e6e03d,  /*  20: MCIF2_WR 37000ns */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e03f,  /*  22: BSP RR 50000ns */
           0x80ad904b,  /*  23: SAGE RR 6820ns */
           0x86449060,  /*  24: FLASH_DMA RR 63000ns */
           0x81617057,  /*  25: HIF_PCIe RR 13880ns */
           0x86449062,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449061,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x03e6e03c,  /*  29: MCIF_RD 37000ns */
           0x03e6e03e,  /*  30: MCIF_WR 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db051,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5056,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5055,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae104c,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x3ffff0ff,  /*  38: UNASSIGNED off */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x00a1e022,  /*  40: RAAGA 6000ns */
           0x3fffe044,  /*  41: RAAGA_1 1000000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x0236e033,  /*  44: AUD_AIO 21000ns */
           0x8112f052,  /*  45: VICE_CME_RMB_CMB RR 10800ns */
           0x8898a063,  /*  46: VICE_CME_CSC RR 86400ns */
           0x81fb405b,  /*  47: VICE_FME_CSC RR 19920ns */
           0x83f9b05e,  /*  48: VICE_FME_Luma_CMB RR 39960ns */
           0x83f9b05d,  /*  49: VICE_FME_Chroma_CMB RR 39960ns */
           0x859c4041,  /*  50: VICE_SG RR 53200ns */
           0x80002065,  /*  51: VICE_DBLK RR 10ns */
           0x89438042,  /*  52: VICE_CABAC0 RR 87840ns */
           0x90a7b043,  /*  53: VICE_CABAC1 RR 157920ns */
           0x8196605a,  /*  54: VICE_ARCSS0 RR 15960ns */
           0x80efd02c,  /*  55: VICE_VIP0_INST0 RR 8887.2ns */
           0x82cf2034,  /*  56: VICE_VIP1_INST0 RR 26640ns */
           0x80efd02d,  /*  57: VICE_VIP0_INST1 RR 8887.2ns */
           0x82cf2035,  /*  58: VICE_VIP1_INST1 RR 26640ns */
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
           0x80000066,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x80000067,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x80270046,  /*  75: HVD0_ILCPU RR 1451ns */
           0x80d9b04d,  /*  76: HVD0_OLCPU RR 8553ns */
           0x007de016,  /*  77: HVD0_CAB 4667ns */
           0x0071a014,  /*  78: HVD0_ILSI 4214ns */
           0x8409205f,  /*  79: HVD0_ILCPU_p2 RR 38270ns */
           0x0073f015,  /*  80: HVD0_ILSI_p2 4299ns */
           0x8000006a,  /*  81: HVD1_DBLK_0 RR 0ns */
           0x8000006b,  /*  82: HVD1_DBLK_1 RR 0ns */
           0x803b3048,  /*  83: HVD1_ILCPU RR 2198ns */
           0x8188e059,  /*  84: HVD1_OLCPU RR 15429ns */
           0x00973021,  /*  85: HVD1_CAB 5606ns */
           0x01b7f032,  /*  86: HVD1_ILSI 16299ns */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x005e2013,  /*  94: BVN_MAD_PIX_FD 3493ns */
           0x0084801a,  /*  95: BVN_MAD_QUANT 4914ns */
           0x00bc7029,  /*  96: BVN_MAD_PIX_CAP 6986ns */
           0x005e2011,  /*  97: BVN_MAD1_PIX_FD 3493ns */
           0x00848018,  /*  98: BVN_MAD1_QUANT 4914ns */
           0x00bc7027,  /*  99: BVN_MAD1_PIX_CAP 6986ns */
           0x005e2012,  /* 100: BVN_MAD2_PIX_FD 3493ns */
           0x00848019,  /* 101: BVN_MAD2_QUANT 4914ns */
           0x00bc7028,  /* 102: BVN_MAD2_PIX_CAP 6986ns */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0058c00d,  /* 106: BVN_MFD0 3292ns */
           0x001a8000,  /* 107: BVN_MFD0_1 987ns */
           0x0085301b,  /* 108: BVN_MFD1 4938ns */
           0x0058c00e,  /* 109: BVN_MFD1_1 3292ns */
           0x0085301c,  /* 110: BVN_MFD2 4938ns */
           0x0058c00f,  /* 111: BVN_MFD2_1 3292ns */
           0x0085301d,  /* 112: BVN_MFD3 4938ns */
           0x0058c010,  /* 113: BVN_MFD3_1 3292ns */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x00261001,  /* 118: BVN_VFD0 1414.92537313433ns */
           0x003f8005,  /* 119: BVN_VFD1 2358.20895522388ns */
           0x0359a039,  /* 120: BVN_VFD2 31770ns */
           0x0359a03a,  /* 121: BVN_VFD3 31770ns */
           0x00b2f023,  /* 122: BVN_VFD4 6632.83582089552ns */
           0x00b2f024,  /* 123: BVN_VFD5 6632.83582089552ns */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x00261002,  /* 126: BVN_CAP0 1414.92537313433ns */
           0x007f3017,  /* 127: BVN_CAP1 4716.41791044776ns */
           0x01747030,  /* 128: BVN_CAP2 13800ns */
           0x01747031,  /* 129: BVN_CAP3 13800ns */
           0x00b2f025,  /* 130: BVN_CAP4 6632.83582089552ns */
           0x00b2f026,  /* 131: BVN_CAP5 6632.83582089552ns */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x003bd003,  /* 134: BVN_GFD0 2221.2ns */
           0x0167c02f,  /* 135: BVN_GFD1 13330ns */
           0x0055900a,  /* 136: BVN_GFD2 3174ns */
           0x0055900b,  /* 137: BVN_GFD3 3174ns */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x004b5009,  /* 141: BVN_MCVP0 2794ns */
           0x004b5008,  /* 142: BVN_MCVP1 2794ns */
           0x00d3d02b,  /* 143: BVN_MCVP2 7850ns */
           0x0057100c,  /* 144: BVN_RDC 3230ns */
           0x03526037,  /* 145: VEC_VBI_ENC0 31500ns */
           0x03526038,  /* 146: VEC_VBI_ENC1 31500ns */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80000068,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x80000069,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
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
           0x801e4045,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000072,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393047,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000073,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x8000006d,  /* 216: HVD0_PFRI RR 0ns */
           0x8000006e,  /* 217: HVD1_PFRI RR 0ns */
           0x8000206c,  /* 218: VICE_PFRI RR 10ns */
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
           0xbfffe064,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00d2702a   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20151008230408_7251_4kp251t[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80700802}, /* HVD0_PFRI (gHvd0) 1184640.00 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000063f}, /* d: 4; p: 1599.2625 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x000017e8}, /* 6120 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00000e58}, /* 60% * 6120 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x80710802}, /* HVD1_PFRI (gHvd1) 1112880.00 ns/60 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x400007d3}, /* d: 4; p: 2003.18333333333 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x00000905}, /* 2309 */
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x00000569}, /* 60% * 2309 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_CONFIG,      0x806f0803}, /* VICE_PFRI (gVice) 800000.00 ns/80 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_FILTER_CTRL, 0x40000438}, /* d: 4; p: 1080 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH0,     0x000001a0}, /* 416 */
  {BCHP_MEMC_GEN_0_PFRI_2_THROTTLE_THRESH1,     0x000000f9}  /* 60% * 416 */
};

static const uint32_t* const paulMemc_box19[] = { &aulMemc0_20151008230408_7251_4kp251t[0]};

const BBOX_Rts stBoxRts_7251_4kp251t_box19 = {
  "20151008230408_7251_4kp251t_box19",
  7439,
  19,
  1,
  256,
  (const uint32_t**)&paulMemc_box19[0],
  12,
  stBoxRts_PfriClient_20151008230408_7251_4kp251t
};
