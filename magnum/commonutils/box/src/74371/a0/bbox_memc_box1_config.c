/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * API Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************
 *
 *                            Do Not Edit Directly
 * Auto-Generated from RTS environment:
 *   at: Fri Sep  8 18:28:38 2017 GMT
 *   by: cm902429
 *   for: Box 74371_1stb
 *         MemC 0 (32-bit DDR3@800MHz) w/324MHz clock
 *
 *   Run from /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/rts_shell_v3.pl
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439A0/rev5/BCM7439A0.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439A0/rev5/BCM7439A0MemoryTiming.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/MEMC_b1r8_timing_CLwc.lib
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/VideoDecoder_rNb.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/library_tools.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/rts_code/timing_model/Vice_v2b.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439A0/rev5/BCM7439A0BvnLib.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439A0/rev5/BCM7439A0Client.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439A0/rev5/BCM7439A0ClientTime.cfg
 *     /projects/dvt_chip_arch/RealTimeScheduling/rts/7439A0/rev5/BCM7439A0Display.cfg
 *
 *****************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20170908182838_74371_1stb[] = {
           0x0079c015,  /*   0: XPT_WR_RS 6020ns */
           0x80f78032,  /*   1: XPT_WR_XC RR 12230ns */
           0x8114b01c,  /*   2: XPT_WR_CDB RR 13670ns */
           0x8359d035,  /*   3: XPT_WR_ITB_MSG RR 42370ns */
           0x8186401e,  /*   4: XPT_RD_RS RR 19280ns */
           0x8edee027,  /*   5: XPT_RD_XC_RMX_MSG RR 188000ns */
           0x8114b01b,  /*   6: XPT_RD_XC_RAVE RR 13670ns */
           0x83997036,  /*   7: XPT_RD_PB RR 45510ns */
           0x861ce03b,  /*   8: XPT_WR_MEMDMA RR 81920ns */
           0x861ce03a,  /*   9: XPT_RD_MEMDMA RR 81920ns */
           0x802a9002,  /*  10: GENET0_WR RR 2110ns */
           0x80871031,  /*  11: GENET0_RD RR 7079ns */
           0x80675014,  /*  12: GENET1_WR RR 5110ns */
           0x8168c034,  /*  13: GENET1_RD RR 18890ns */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x3ffff0ff,  /*  15: UNASSIGNED off */
           0x84312026,  /*  16: MOCA_MIPS RR 53000ns */
           0x3ffff0ff,  /*  17: SATA off */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x83f46025,  /*  22: BSP RR 50000ns */
           0x8082202f,  /*  23: SAGE RR 6820ns */
           0x84b36037,  /*  24: FLASH_DMA RR 63000ns */
           0x814e3033,  /*  25: HIF_PCIe RR 17500ns */
           0x84b36039,  /*  26: SDIO_EMMC RR 63000ns */
           0x84b36038,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x02ed2021,  /*  29: MCIF_RD_0 37000ns */
           0x02ed2023,  /*  30: MCIF_WR_0 37000ns */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x8037102d,  /*  33: USB_HI_0 RR 2890ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0xbffff0ff,  /*  35: USB_X_WRITE_0 RR */
           0xbffff0ff,  /*  36: USB_X_READ_0 RR */
           0xbffff0ff,  /*  37: USB_X_CTRL_0 RR */
           0x8037102e,  /*  38: USB_HI_1 RR 2890ns */
           0xbffff0ff,  /*  39: USB_LO_1 RR */
           0x02ed2022,  /*  40: MCIF_RD_1 37000ns */
           0x02ed2024,  /*  41: MCIF_WR_1 37000ns */
           0x00307003,  /*  42: RAAGA 2400ns */
           0x3ffff0ff,  /*  43: RAAGA_1 off */
           0x008c6017,  /*  44: AUD_AIO 6940ns */
           0x3ffff0ff,  /*  45: VICE_CME_RMB_CMB off */
           0x3ffff0ff,  /*  46: VICE_CME_CSC off */
           0x3ffff0ff,  /*  47: VICE_FME_CSC off */
           0x3ffff0ff,  /*  48: VICE_FME_Luma_CMB off */
           0x3ffff0ff,  /*  49: VICE_FME_Chroma_CMB off */
           0x3ffff0ff,  /*  50: VICE_SG off */
           0x3ffff0ff,  /*  51: VICE_DBLK off */
           0x3ffff0ff,  /*  52: VICE_CABAC0 off */
           0x3ffff0ff,  /*  53: VICE_CABAC1 off */
           0x3ffff0ff,  /*  54: VICE_ARCSS0 off */
           0x3ffff0ff,  /*  55: VICE_VIP0_INST0 off */
           0x3ffff0ff,  /*  56: VICE_VIP1_INST0 off */
           0x3ffff0ff,  /*  57: VICE_VIP0_INST1 off */
           0x3ffff0ff,  /*  58: VICE_VIP1_INST1 off */
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
           0x8010e03d,  /*  73: HVD_DBLK_0 RR 0ns */
           0x8008603c,  /*  74: HVD_DBLK_1 RR 0ns */
           0x801d402c,  /*  75: HVD_ILCPU RR 1451ns */
           0x80838030,  /*  76: HVD_OLCPU RR 6893ns */
           0x005e5012,  /*  77: HVD_CAB 4666ns */
           0x0055c011,  /*  78: HVD_ILSI 4242ns */
           0x3ffff0ff,  /*  79: UNASSIGNED off */
           0x3ffff0ff,  /*  80: UNASSIGNED off */
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
           0x0046f00e,  /*  94: MADR_RD 3511ns */
           0x0063d013,  /*  95: MADR_QM 4938ns */
           0x008e1018,  /*  96: MADR_WR 7023ns */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x004ac00f,  /* 106: BVNF_MFD0_0 3700ns */
           0x0031b004,  /* 107: BVNF_MFD0_1 2460ns */
           0x004ac010,  /* 108: BVNF_MFD1_0 3700ns */
           0x0031b005,  /* 109: BVNF_MFD1_1 2460ns */
           0x3ffff0ff,  /* 110: UNASSIGNED off */
           0x3ffff0ff,  /* 111: UNASSIGNED off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x0031d006,  /* 118: BVNF_VFD0 2469ns */
           0x0031d007,  /* 119: BVNF_VFD1 2469ns */
           0x0031d008,  /* 120: BVNF_VFD2 2469ns */
           0x02835020,  /* 121: BVNF_VFD3 31777ns */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x0031d009,  /* 126: BVNF_CAP0 2469ns */
           0x0031d00a,  /* 127: BVNF_CAP1 2469ns */
           0x0031d00b,  /* 128: BVNF_CAP2 2469ns */
           0x0151501d,  /* 129: BVNF_CAP3 16666ns */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x001dd001,  /* 134: BVNF_GFD0 1481ns */
           0x00806016,  /* 135: BVNF_GFD1 6348ns */
           0x3ffff0ff,  /* 136: BVNF_GFD2 off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x0046a00d,  /* 141: MCVP0 3494ns */
           0x0046a00c,  /* 142: MCVP1 3494ns */
           0x009ed01a,  /* 143: MCVP_QM 7850ns */
           0x001c3000,  /* 144: BVNF_RDC 1400ns */
           0x027dc01f,  /* 145: VEC_VBI_ENC 31500ns */
           0x3ffff0ff,  /* 146: UNASSIGNED off */
           0x8012f028,  /* 147: V3D_0 RR 1000ns */
           0x8012f029,  /* 148: V3D_1 RR 1000ns */
           0xbffff0ff,  /* 149: M2MC RR */
           0xbffff0ff,  /* 150: M2MC1 RR */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x3ffff0ff,  /* 155: UNASSIGNED off */
           0x3ffff0ff,  /* 156: UNASSIGNED off */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0x3ffff0ff,  /* 160: UNASSIGNED off */
           0x3ffff0ff,  /* 161: UNASSIGNED off */
           0x3ffff0ff,  /* 162: UNASSIGNED off */
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
           0x801c802a,  /* 200: MCP_RD_HIGH RR 1500ns */
           0x8000003f,  /* 201: MCP_RD_LOW RR */
           0x801c802b,  /* 202: MCP_WR_HIGH RR 1500ns */
           0x80000040,  /* 203: MCP_WR_LOW RR */
           0x3ffff0ff,  /* 204: UNASSIGNED off */
           0x3ffff0ff,  /* 205: UNASSIGNED off */
           0x3ffff0ff,  /* 206: UNASSIGNED off */
           0x3ffff0ff,  /* 207: UNASSIGNED off */
           0x3ffff0ff,  /* 208: UNASSIGNED off */
           0x3ffff0ff,  /* 209: UNASSIGNED off */
           0x3ffff0ff,  /* 210: UNASSIGNED off */
           0x3ffff0ff,  /* 211: UNASSIGNED off */
           0x3ffff0ff,  /* 212: UNASSIGNED off */
           0x3ffff0ff,  /* 213: UNASSIGNED off */
           0x3ffff0ff,  /* 214: UNASSIGNED off */
           0x3ffff0ff,  /* 215: UNASSIGNED off */
           0x8000003e,  /* 216: HVD_PFRI RR 0ns */
           0x3ffff0ff,  /* 217: VICE_PFRI off */
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
           0x3ffff0ff,  /* 250: UNASSIGNED off */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x009dd019   /* 255: REFRESH 7800ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20170908182838_74371_1stb[] = {
    {BCHP_MEMC_GEN_0_PFRI_0_PAGE_BRK_INTR_INFO_0,      0x80700905}
};

static const uint32_t* const paulMemc_box1[] = { &aulMemc0_20170908182838_74371_1stb[0]};

const BBOX_Rts stBoxRts_74371_1stb_box1 = {
  "20170908182838_74371_1stb_box1",
  74371,
  1,
  1,
  256,
  (const uint32_t**)&paulMemc_box1[0],
  1,
  stBoxRts_PfriClient_20170908182838_74371_1stb
};
