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
*   at: Wed Nov 12 01:36:07 2014 GMT
*   by: chuck
*   for: Box 1stb1t
*         MemC 0 (32-bit DDRDDR3@1067MHz) w/324MHz clock
*         MemC 1 (32-bit DDRDDR3@1067MHz) w/324MHz clock
*******************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */
#include "bchp_memc_gen_1.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20141112013607_1stb1t[] = {
           0x00241001,  /*   0: XPT_WR_RS 1790ns */
           0x802dd046,  /*   1: XPT_WR_XC RR 2270ns */
           0x8031e010,  /*   2: XPT_WR_CDB RR 2470ns */
           0x8097c055,  /*   3: XPT_WR_ITB_MSG RR 7500ns */
           0x80375011,  /*   4: XPT_RD_RS RR 2740ns */
           0x827a4036,  /*   5: XPT_RD_XC_RMX_MSG RR 31330ns */
           0x8031e00f,  /*   6: XPT_RD_XC_RAVE RR 2470ns */
           0x80838053,  /*   7: XPT_RD_PB RR 6500ns */
           0x8044f04b,  /*   8: XPT_WR_MEMDMA RR 3413ns */
           0x8044f04a,  /*   9: XPT_RD_MEMDMA RR 3413ns */
           0x802fa008,  /*  10: GENET0_WR RR 2360ns */
           0x80871054,  /*  11: GENET0_RD RR 7079ns */
           0x8052d01b,  /*  12: GENET1_WR RR 4096ns */
           0x81a1c059,  /*  13: GENET1_RD RR 21875ns */
           0x8052d01c,  /*  14: GENET2_WR RR 4096ns */
           0x81a1c05a,  /*  15: GENET2_RD RR 21875ns */
           0x8431203e,  /*  16: MOCA_MIPS RR 53000ns */
           0x8029b043,  /*  17: SATA RR 2190ns */
           0x8029b044,  /*  18: SATA_1 RR 2190ns */
           0x814e3058,  /*  19: HIF_PCIe1 RR 17500ns */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0xbffff0ff,  /*  21: LEAP RR */
           0x83f4603d,  /*  22: BSP RR 50000ns */
           0x80822052,  /*  23: SAGE RR 6820ns */
           0x84b3605b,  /*  24: FLASH_DMA RR 63000ns */
           0x814e3057,  /*  25: HIF_PCIe RR 17500ns */
           0x84b3605d,  /*  26: SDIO_EMMC RR 63000ns */
           0x84b3605c,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x02ed2038,  /*  29: MCIF_RD_0 37000ns */
           0x02ed203a,  /*  30: MCIF_WR_0 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x80371047,  /*  33: USB_HI_0 RR 2890ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x806e4051,  /*  35: USB_X_WRITE_0 RR 5780ns */
           0x806e4050,  /*  36: USB_X_READ_0 RR 5780ns */
           0x810d3031,  /*  37: USB_X_CTRL_0 RR 13300ns */
           0x80371048,  /*  38: USB_HI_1 RR 2890ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x02ed2039,  /*  40: MCIF_RD_1 37000ns */
           0x02ed203b,  /*  41: MCIF_WR_1 37000ns */
           0x0030700b,  /*  42: RAAGA 2400ns */
           0x00142000,  /*  43: RAAGA_1 1000ns */
           0x008c6026,  /*  44: AUD_AIO 6940ns */
           0x80261041,  /*  45: VICE_CME_RMB_CMB RR 2000ns */
           0x81318056,  /*  46: VICE_CME_CSC RR 16000ns */
           0x8046004c,  /*  47: VICE_FME_CSC RR 3670ns */
           0x8046004e,  /*  48: VICE_FME_Luma_CMB RR 3670ns */
           0x8046004d,  /*  49: VICE_FME_Chroma_CMB RR 3670ns */
           0x80a57030,  /*  50: VICE_SG RR 8176.66666666667ns */
           0x80075063,  /*  51: VICE_DBLK RR 0ns */
           0x81d5a035,  /*  52: VICE_CABAC0 RR 23200ns */
           0x833e203c,  /*  53: VICE_CABAC1 RR 41000ns */
           0x803a2049,  /*  54: VICE_ARCSS0 RR 3050ns */
           0x80255002,  /*  55: VICE_VIP0_INST0 RR 1850ns */
           0x8095e02c,  /*  56: VICE_VIP1_INST0 RR 7410ns */
           0x80255003,  /*  57: VICE_VIP0_INST1 RR 1850ns */
           0x8095e02d,  /*  58: VICE_VIP1_INST1 RR 7410ns */
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
           0x8002905f,  /*  73: HVD_DBLK_Ch_0 RR 0ns */
           0x80029060,  /*  74: HVD_DBLK_Ch_1 RR 0ns */
           0x801d4040,  /*  75: HVD_ILCPU RR 1451ns */
           0x806cc04f,  /*  76: HVD_OLCPU RR 5700ns */
           0x003ee015,  /*  77: HVD_CAB 3113.33333333333ns */
           0x0055c01d,  /*  78: HVD_ILSI 4242ns */
           0x80270042,  /*  79: HVD_ILCPU_p2 RR 1933.33333333333ns */
           0x00718024,  /*  80: HVD_ILSI_p2 5613.33333333333ns */
           0x3ffff0ff,  /*  81: UNASSIGNED off */
           0x3ffff0ff,  /*  82: UNASSIGNED off */
           0x3ffff0ff,  /*  83: UNASSIGNED off */
           0x3ffff0ff,  /*  84: UNASSIGNED off */
           0x3ffff0ff,  /*  85: UNASSIGNED off */
           0x3ffff0ff,  /*  86: UNASSIGNED off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x3ffff017,  /*  94: MADR_RD off */
           0x3ffff021,  /*  95: MADR_QM off */
           0x3ffff028,  /*  96: MADR_WR off */
           0x0046f016,  /*  97: MADR1_RD 3511ns */
           0x0063d020,  /*  98: MADR1_QM 4938ns */
           0x008e1027,  /*  99: MADR1_WR 7023ns */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0095d029,  /* 106: BVNF_MFD0_0 7406ns */
           0x004ad018,  /* 107: BVNF_MFD0_1 3702ns */
           0x0095d02a,  /* 108: BVNF_MFD1_0 7406ns */
           0x004ad019,  /* 109: BVNF_MFD1_1 3702ns */
           0x0095d02b,  /* 110: BVNF_MFD2_0 7406ns */
           0x004ad01a,  /* 111: BVNF_MFD2_1 3702ns */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x3ffff009,  /* 118: BVNF_VFD0 off */
           0x3ffff00a,  /* 119: BVNF_VFD1 off */
           0x013fd032,  /* 120: BVNF_VFD2 15800ns */
           0x00707022,  /* 121: BVNF_VFD3 5560ns */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x3ffff01e,  /* 126: BVNF_CAP0 off */
           0x3ffff01f,  /* 127: BVNF_CAP1 off */
           0x013fd033,  /* 128: BVNF_CAP2 15800ns */
           0x00707023,  /* 129: BVNF_CAP3 5560ns */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x3ffff004,  /* 134: BVNF_GFD0 off */
           0x3ffff034,  /* 135: BVNF_GFD1 off */
           0x00806025,  /* 136: BVNF_GFD2 6348ns */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: BVNF_GFD0_1 off */
           0x3ffff0ff,  /* 140: BVNF_GFD1_1 off */
           0x3ffff013,  /* 141: MCVP0 off */
           0x3ffff012,  /* 142: MCVP1 off */
           0x3ffff02f,  /* 143: MCVP_QM off */
           0x00389014,  /* 144: BVNF_RDC 2800ns */
           0x027dc037,  /* 145: VEC_VBI_ENC 31500ns */
           0x3ffff0ff,  /* 146: UNASSIGNED off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80029061,  /* 155: HVD_DBLK_p2_Ch_0 RR 0ns */
           0x80029062,  /* 156: HVD_DBLK_p2_Ch_1 RR 0ns */
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
           0x801c803f,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1500ns */
           0x8000006e,  /* 201: HOST_CPU_MCP_RD_LOW RR */
           0x802ad045,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x8000006f,  /* 203: HOST_CPU_MCP_WR_LOW RR */
           0xbffff0ff,  /* 204: V3D_MCP_R_HI RR */
           0xbffff0ff,  /* 205: V3D_MCP_W_HI RR */
           0xbffff0ff,  /* 206: V3D_MCP_R_LO RR */
           0xbffff0ff,  /* 207: V3D_MCP_W_LO RR */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x80000069,  /* 216: HVD_PFRI_Ch RR 0ns */
           0x80000068,  /* 217: VICE_PFRI RR 0ns */
           0x3ffff0ff,  /* 218: UNASSIGNED off */
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
           0xbfffe05e,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x009e102e,  /* 255: REFRESH 7812.5ns */
         };
static const uint32_t aulMemc1_20141112013607_1stb1t[] = {
           0x3ffff001,  /*   0: XPT_WR_RS off */
           0x3ffff046,  /*   1: XPT_WR_XC off */
           0x3ffff010,  /*   2: XPT_WR_CDB off */
           0x3ffff055,  /*   3: XPT_WR_ITB_MSG off */
           0x3ffff011,  /*   4: XPT_RD_RS off */
           0x3ffff036,  /*   5: XPT_RD_XC_RMX_MSG off */
           0x3ffff00f,  /*   6: XPT_RD_XC_RAVE off */
           0x3ffff053,  /*   7: XPT_RD_PB off */
           0x8044f04b,  /*   8: XPT_WR_MEMDMA RR 3413ns */
           0x8044f04a,  /*   9: XPT_RD_MEMDMA RR 3413ns */
           0x802fa008,  /*  10: GENET0_WR RR 2360ns */
           0x80871054,  /*  11: GENET0_RD RR 7079ns */
           0x8052d01b,  /*  12: GENET1_WR RR 4096ns */
           0x81a1c059,  /*  13: GENET1_RD RR 21875ns */
           0x8052d01c,  /*  14: GENET2_WR RR 4096ns */
           0x81a1c05a,  /*  15: GENET2_RD RR 21875ns */
           0x8431203e,  /*  16: MOCA_MIPS RR 53000ns */
           0x8029b043,  /*  17: SATA RR 2190ns */
           0x8029b044,  /*  18: SATA_1 RR 2190ns */
           0x814e3058,  /*  19: HIF_PCIe1 RR 17500ns */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0xbffff0ff,  /*  21: LEAP RR */
           0x83f4603d,  /*  22: BSP RR 50000ns */
           0x80822052,  /*  23: SAGE RR 6820ns */
           0x84b3605b,  /*  24: FLASH_DMA RR 63000ns */
           0x814e3057,  /*  25: HIF_PCIe RR 17500ns */
           0x84b3605d,  /*  26: SDIO_EMMC RR 63000ns */
           0x84b3605c,  /*  27: SDIO_CARD RR 63000ns */
           0x3ffff0ff,  /*  28: TPCAP off */
           0x3ffff038,  /*  29: MCIF_RD_0 off */
           0x3ffff03a,  /*  30: MCIF_WR_0 off */
           0x3ffff0ff,  /*  31: UART_DMA_RD off */
           0x3ffff0ff,  /*  32: UART_DMA_WR off */
           0x80371047,  /*  33: USB_HI_0 RR 2890ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x806e4051,  /*  35: USB_X_WRITE_0 RR 5780ns */
           0x806e4050,  /*  36: USB_X_READ_0 RR 5780ns */
           0x810d3031,  /*  37: USB_X_CTRL_0 RR 13300ns */
           0x80371048,  /*  38: USB_HI_1 RR 2890ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x3ffff039,  /*  40: MCIF_RD_1 off */
           0x3ffff03b,  /*  41: MCIF_WR_1 off */
           0x3ffff00b,  /*  42: RAAGA off */
           0x3ffff000,  /*  43: RAAGA_1 off */
           0x008c6026,  /*  44: AUD_AIO 6940ns */
           0x3ffff041,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff056,  /*  46: VICE_CME_CSC off */
           0x3ffff04c,  /*  47: VICE_FME_CSC off */
           0x3ffff04e,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff04d,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff030,  /*  50: VICE_SG off */
           0x3ffff063,  /*  51: VICE_DBLK off */
           0x3ffff035,  /*  52: VICE_CABAC0 off */
           0x3ffff03c,  /*  53: VICE_CABAC1 off */
           0x3ffff049,  /*  54: VICE_ARCSS0 off */
           0x3ffff002,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff02c,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff003,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff02d,  /*  58: VICE_VIP1_INST1 off */
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
           0x80029064,  /*  73: HVD_DBLK_0 RR 0ns */
           0x80029065,  /*  74: HVD_DBLK_1 RR 0ns */
           0x3ffff040,  /*  75: HVD_ILCPU off */
           0x3ffff04f,  /*  76: HVD_OLCPU off */
           0x3ffff015,  /*  77: HVD_CAB off */
           0x3ffff01d,  /*  78: HVD_ILSI off */
           0x3ffff042,  /*  79: HVD_ILCPU_p2 off */
           0x3ffff024,  /*  80: HVD_ILSI_p2 off */
           0x3ffff0ff,  /*  81: UNASSIGNED off */
           0x3ffff0ff,  /*  82: UNASSIGNED off */
           0x3ffff0ff,  /*  83: UNASSIGNED off */
           0x3ffff0ff,  /*  84: UNASSIGNED off */
           0x3ffff0ff,  /*  85: UNASSIGNED off */
           0x3ffff0ff,  /*  86: UNASSIGNED off */
           0xbffff0ff,  /*  87: SID RR */
           0x3ffff0ff,  /*  88: UNASSIGNED off */
           0x3ffff0ff,  /*  89: UNASSIGNED off */
           0x3ffff0ff,  /*  90: UNASSIGNED off */
           0x3ffff0ff,  /*  91: UNASSIGNED off */
           0x3ffff0ff,  /*  92: UNASSIGNED off */
           0x3ffff0ff,  /*  93: UNASSIGNED off */
           0x0046f017,  /*  94: MADR_RD 3511ns */
           0x0063d021,  /*  95: MADR_QM 4938ns */
           0x008e1028,  /*  96: MADR_WR 7023ns */
           0x3ffff016,  /*  97: MADR1_RD off */
           0x3ffff020,  /*  98: MADR1_QM off */
           0x3ffff027,  /*  99: MADR1_WR off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0031d00c,  /* 106: BVNF_MFD0_0Y 2469ns */
           0x00255005,  /* 107: BVNF_MFD0_1Y 1851ns */
           0x0031d00d,  /* 108: BVNF_MFD1_0Y 2469ns */
           0x00255006,  /* 109: BVNF_MFD1_1Y 1851ns */
           0x0031d00e,  /* 110: BVNF_MFD2_0Y 2469ns */
           0x00255007,  /* 111: BVNF_MFD2_1Y 1851ns */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x002fd009,  /* 118: BVNF_VFD0 2370ns */
           0x002fd00a,  /* 119: BVNF_VFD1 2370ns */
           0x3ffff032,  /* 120: BVNF_VFD2 off */
           0x3ffff022,  /* 121: BVNF_VFD3 off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x005fe01e,  /* 126: BVNF_CAP0 4741ns */
           0x005fe01f,  /* 127: BVNF_CAP1 4741ns */
           0x3ffff033,  /* 128: BVNF_CAP2 off */
           0x3ffff023,  /* 129: BVNF_CAP3 off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x00255004,  /* 134: BVNF_GFD0 1851ns */
           0x01414034,  /* 135: BVNF_GFD1 15873ns */
           0x3ffff025,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: BVNF_GFD0_1 off */
           0x3ffff0ff,  /* 140: BVNF_GFD1_1 off */
           0x00387013,  /* 141: MCVP0 2794ns */
           0x00387012,  /* 142: MCVP1 2794ns */
           0x009ed02f,  /* 143: MCVP_QM 7850ns */
           0x3ffff014,  /* 144: BVNF_RDC off */
           0x3ffff037,  /* 145: VEC_VBI_ENC off */
           0x3ffff0ff,  /* 146: UNASSIGNED off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0x3ffff0ff,  /* 150: UNASSIGNED off */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x80029066,  /* 155: HVD_DBLK_p2_0 RR 0ns */
           0x80029067,  /* 156: HVD_DBLK_p2_1 RR 0ns */
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
           0x801c803f,  /* 200: HOST_CPU_MCP_RD_HIGH RR 1500ns */
           0x8000006e,  /* 201: HOST_CPU_MCP_RD_LOW RR */
           0x802ad045,  /* 202: HOST_CPU_MCP_WR_HIGH RR 2250ns */
           0x8000006f,  /* 203: HOST_CPU_MCP_WR_LOW RR */
           0xbffff0ff,  /* 204: V3D_MCP_R_HI RR */
           0xbffff0ff,  /* 205: V3D_MCP_W_HI RR */
           0xbffff0ff,  /* 206: V3D_MCP_R_LO RR */
           0xbffff0ff,  /* 207: V3D_MCP_W_LO RR */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x8000006a,  /* 216: HVD_PFRI RR 0ns */
           0x3ffff068,  /* 217: VICE_PFRI off */
           0x3ffff0ff,  /* 218: UNASSIGNED off */
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
           0xbfffe05e,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x009e102e,  /* 255: REFRESH 7812.5ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20141112013607_1stb1t[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x806c0802},
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000060d},
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00000d40},
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x000007f3},
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_CONFIG,      0x806b0803},
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_FILTER_CTRL, 0x400001ea},
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH0,     0x000001a0},
  {BCHP_MEMC_GEN_0_PFRI_1_THROTTLE_THRESH1,     0x000000f9},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_CONFIG,      0x806d0803},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_FILTER_CTRL, 0x4000060d},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH0,     0x00001642},
  {BCHP_MEMC_GEN_1_PFRI_0_THROTTLE_THRESH1,     0x00000d5a},
};

static const uint32_t * const paulMemc_box1[] = { &aulMemc0_20141112013607_1stb1t[0], &aulMemc1_20141112013607_1stb1t[0],};

const BBOX_Rts stBoxRts_1stb1t_box1 = {
  "20141112013607_1stb1t_box1",
  7366,
  1,
  2,
  256,
  (const uint32_t **)&paulMemc_box1[0],
  12,
  stBoxRts_PfriClient_20141112013607_1stb1t,
};
