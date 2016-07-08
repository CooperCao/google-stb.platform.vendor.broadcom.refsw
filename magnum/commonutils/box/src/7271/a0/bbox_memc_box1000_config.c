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
******************************************************************************
*
*                            Do Not Edit Directly
* Auto-Generated from RTS environment:
*   at: Fri Feb 12 21:10:41 2016 GMT
*   by: kranawet
*   for: Box 1000_LPDDR4_2667
*         MemC 0 (32-bit LPDDR4@1333MHz) w/432MHz clock
*
*   Run from /projects/bbvlsi_core1/rts/rev1_0/rts_shell2/rts_code/rts_shell_v3.pl
*     /projects/bbvlsi_core1/Architecture/Chips/7271/rts/sim/BCM7271.cfg
*     /projects/bbvlsi_core1/Architecture/Chips/7271/rts/sim/BCM7271boxes.cfg
*     /projects/bbvlsi_core1/Architecture/Chips/7271/rts/sim/MEMC_gk.lib
*     /projects/bbvlsi_core1/Architecture/Chips/7271/rts/sim/BCM7271BvnLib_NOTSURE.cfg
*     /projects/bbvlsi_core1/rts/rev1_0/rts_shell2/rts_code/timing_model/VideoDecoder_rS.cfg
*     /projects/bbvlsi_core1/rts/rev1_0/rts_shell2/rts_code/timing_model/library_tools.cfg
*     /projects/bbvlsi_core1/Architecture/Chips/7271/rts/sim/BCM7271Client.cfg
*     /projects/bbvlsi_core1/Architecture/Chips/7271/rts/sim/BCM7271ClientGroups.cfg
*
***************************************************************************/

#include "bchp_memc_gen_0.h"  /* located in /TBD directory */

#include "bbox.h"  /* located in /magnum/commonutils/box/include directory */

static const uint32_t aulMemc0_20160212211041_1000_LPDDR4_2667[] = {
           0x0144b00f,  /*   0: XPT_WR_RS 12032ns */
           0x81117023,  /*   1: XPT_WR_XC RR 10742ns */
           0x813aa00e,  /*   2: XPT_WR_CDB RR 11658ns */
           0x83e97027,  /*   3: XPT_WR_ITB_MSG RR 39320ns */
           0x83b03011,  /*   4: XPT_RD_RS RR 34976ns */
           0x9a6fd013,  /*   5: XPT_RD_XC_RMX_MSG RR 250666ns */
           0x813aa00d,  /*   6: XPT_RD_XC_RAVE RR 11658ns */
           0x80c6301f,  /*   7: XPT_RD_PB RR 7787ns */
           0x81094020,  /*   8: XPT_WR_MEMDMA RR 10418ns */
           0x3ffff0ff,  /*   9: UNASSIGNED off */
           0x81a26010,  /*  10: GENET0_WR RR 15500ns */
           0x87fbc02b,  /*  11: GENET0_RD RR 75700ns */
           0x3ffff0ff,  /*  12: UNASSIGNED off */
           0x3ffff0ff,  /*  13: UNASSIGNED off */
           0x3ffff0ff,  /*  14: UNASSIGNED off */
           0x3ffff0ff,  /*  15: UNASSIGNED off */
           0x3ffff0ff,  /*  16: UNASSIGNED off */
           0x83d49026,  /*  17: SATA RR 38502.4ns */
           0x3ffff0ff,  /*  18: UNASSIGNED off */
           0x3ffff0ff,  /*  19: UNASSIGNED off */
           0x3ffff0ff,  /*  20: UNASSIGNED off */
           0x3ffff0ff,  /*  21: UNASSIGNED off */
           0x8545e012,  /*  22: BSP RR 50000ns */
           0x80ad901b,  /*  23: SAGE RR 6820ns */
           0x86449028,  /*  24: FLASH_DMA RR 63000ns */
           0x3ffff0ff,  /*  25: UNASSIGNED off */
           0x8644902a,  /*  26: SDIO_EMMC RR 63000ns */
           0x86449029,  /*  27: SDIO_CARD RR 63000ns */
           0xbffff0ff,  /*  28: TPCAP RR */
           0x3ffff0ff,  /*  29: UNASSIGNED off */
           0x3ffff0ff,  /*  30: UNASSIGNED off */
           0xbffff0ff,  /*  31: UART_DMA_RD RR */
           0xbffff0ff,  /*  32: UART_DMA_WR RR */
           0x810db022,  /*  33: USB_HI_0 RR 10593ns */
           0xbffff0ff,  /*  34: USB_LO_0 RR */
           0x815c5025,  /*  35: USB_X_WRITE_0 RR 13680ns */
           0x815c5024,  /*  36: USB_X_READ_0 RR 13680ns */
           0x80ae101c,  /*  37: USB_X_CTRL_0 RR 6840ns */
           0x810db021,  /*  38: USB_BDC RR 10593ns */
           0x3ffff0ff,  /*  39: UNASSIGNED off */
           0x8040a017,  /*  40: RAAGA RR 2400ns */
           0x8a8be02c,  /*  41: RAAGA_1 RR 100000ns */
           0x3ffff0ff,  /*  42: UNASSIGNED off */
           0x3ffff0ff,  /*  43: UNASSIGNED off */
           0x80bb401d,  /*  44: AUD_AIO RR 6940ns */
           0x3ffff0ff,  /*  45: UNASSIGNED off */
           0x3ffff0ff,  /*  46: UNASSIGNED off */
           0x3ffff0ff,  /*  47: UNASSIGNED off */
           0x3ffff0ff,  /*  48: UNASSIGNED off */
           0x3ffff0ff,  /*  49: UNASSIGNED off */
           0x3ffff0ff,  /*  50: UNASSIGNED off */
           0x3ffff0ff,  /*  51: UNASSIGNED off */
           0x3ffff0ff,  /*  52: UNASSIGNED off */
           0x3ffff0ff,  /*  53: UNASSIGNED off */
           0x3ffff0ff,  /*  54: UNASSIGNED off */
           0xbffff0ff,  /*  55: VEC_VIP0 RR */
           0xbffff0ff,  /*  56: VEC_VIP1 RR */
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
           0x800a202e,  /*  73: HVD0_DBLK_0 RR 0ns */
           0x800a202f,  /*  74: HVD0_DBLK_1 RR 0ns */
           0x805c9018,  /*  75: HVD0_ILCPU RR 3433ns */
           0x801da014,  /*  76: HVD0_OLCPU RR 1169ns */
           0x00311003,  /*  77: HVD0_CAB 1823ns */
           0x0069f009,  /*  78: HVD0_ILSI 3929ns */
           0x805c9019,  /*  79: HVD0_ILCPU_p2 RR 3433ns */
           0x0069f00a,  /*  80: HVD0_ILSI_p2 3929ns */
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
           0x005ea007,  /*  94: BVN_MAD_PIX_FD 3511ns */
           0x0085300b,  /*  95: BVN_MAD_QUANT 4938ns */
           0x00bd700c,  /*  96: BVN_MAD_PIX_CAP 7023ns */
           0x3ffff0ff,  /*  97: UNASSIGNED off */
           0x3ffff0ff,  /*  98: UNASSIGNED off */
           0x3ffff0ff,  /*  99: UNASSIGNED off */
           0x3ffff0ff,  /* 100: UNASSIGNED off */
           0x3ffff0ff,  /* 101: UNASSIGNED off */
           0x3ffff0ff,  /* 102: UNASSIGNED off */
           0x3ffff0ff,  /* 103: UNASSIGNED off */
           0x3ffff0ff,  /* 104: UNASSIGNED off */
           0x3ffff0ff,  /* 105: UNASSIGNED off */
           0x0058c006,  /* 106: BVN_MFD0 3292ns */
           0x00181000,  /* 107: BVN_MFD0_1 897.272727272727ns */
           0x3ffff0ff,  /* 108: UNASSIGNED off */
           0x3ffff0ff,  /* 109: UNASSIGNED off */
           0x3ffff0ff,  /* 110: UNASSIGNED off */
           0x3ffff0ff,  /* 111: UNASSIGNED off */
           0x3ffff0ff,  /* 112: UNASSIGNED off */
           0x3ffff0ff,  /* 113: UNASSIGNED off */
           0x3ffff0ff,  /* 114: UNASSIGNED off */
           0x3ffff0ff,  /* 115: UNASSIGNED off */
           0x3ffff0ff,  /* 116: UNASSIGNED off */
           0x3ffff0ff,  /* 117: UNASSIGNED off */
           0x0018d001,  /* 118: BVN_VFD0 925ns */
           0x3ffff0ff,  /* 119: UNASSIGNED off */
           0x3ffff0ff,  /* 120: UNASSIGNED off */
           0x3ffff0ff,  /* 121: UNASSIGNED off */
           0x3ffff0ff,  /* 122: UNASSIGNED off */
           0x3ffff0ff,  /* 123: UNASSIGNED off */
           0x3ffff0ff,  /* 124: UNASSIGNED off */
           0x3ffff0ff,  /* 125: UNASSIGNED off */
           0x001fb002,  /* 126: BVN_CAP0 1185ns +HRT(0.5%) */
           0x3ffff0ff,  /* 127: UNASSIGNED off */
           0x3ffff0ff,  /* 128: UNASSIGNED off */
           0x3ffff0ff,  /* 129: UNASSIGNED off */
           0x3ffff0ff,  /* 130: UNASSIGNED off */
           0x3ffff0ff,  /* 131: UNASSIGNED off */
           0x3ffff0ff,  /* 132: UNASSIGNED off */
           0x3ffff0ff,  /* 133: UNASSIGNED off */
           0x0031d004,  /* 134: BVN_GFD0 1851ns */
           0x3ffff0ff,  /* 135: BVN_GFD1 off */
           0x3ffff0ff,  /* 136: UNASSIGNED off */
           0x3ffff0ff,  /* 137: UNASSIGNED off */
           0x3ffff0ff,  /* 138: UNASSIGNED off */
           0x3ffff0ff,  /* 139: UNASSIGNED off */
           0x3ffff0ff,  /* 140: UNASSIGNED off */
           0x3ffff0ff,  /* 141: UNASSIGNED off */
           0x3ffff0ff,  /* 142: UNASSIGNED off */
           0x3ffff0ff,  /* 143: UNASSIGNED off */
           0x00571005,  /* 144: BVN_RDC 3230ns */
           0x3ffff0ff,  /* 145: VEC_VBI_ENC0 off */
           0x3ffff0ff,  /* 146: UNASSIGNED off */
           0xbffff0ff,  /* 147: M2MC_0 RR */
           0xbffff0ff,  /* 148: M2MC_1 RR */
           0xbffff0ff,  /* 149: M2MC_2 RR */
           0xbffff0ff,  /* 150: PCIe_0 RR */
           0x3ffff0ff,  /* 151: UNASSIGNED off */
           0x3ffff0ff,  /* 152: UNASSIGNED off */
           0x3ffff0ff,  /* 153: UNASSIGNED off */
           0x3ffff0ff,  /* 154: UNASSIGNED off */
           0x800a2030,  /* 155: HVD0_DBLK_p2_0 RR 0ns */
           0x800a2031,  /* 156: HVD0_DBLK_p2_1 RR 0ns */
           0x3ffff0ff,  /* 157: UNASSIGNED off */
           0x3ffff0ff,  /* 158: UNASSIGNED off */
           0x3ffff0ff,  /* 159: UNASSIGNED off */
           0x80c4f01e,  /* 160: WIFI_CTRL_WR RR 7300ns */
           0x809c701a,  /* 161: WIFI_CTRL_RD RR 5800ns */
           0x3ffff0ff,  /* 162: WIFI_WR off */
           0x3ffff0ff,  /* 163: WIFI_RD_1 off */
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
           0x801e4015,  /* 200: CPU_MCP_RD_HIGH RR 1125ns */
           0x80000034,  /* 201: CPU_MCP_RD_LOW RR */
           0x80393016,  /* 202: CPU_MCP_WR_HIGH RR 2250ns */
           0x80000035,  /* 203: CPU_MCP_WR_LOW RR */
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
           0x80000032,  /* 216: HVD0_PFRI RR 0ns */
           0x3ffff0ff,  /* 217: UNASSIGNED off */
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
           0xbfffe02d,  /* 250: MEMC_ZQCS RR 1000000ns */
           0xbffff0ff,  /* 251: MEMC_MSA RR */
           0xbffff0ff,  /* 252: MEMC_DIS0 RR */
           0xbffff0ff,  /* 253: MEMC_DIS1 RR */
           0xbffff0ff,  /* 254: MEMC_DRAM_INIT_ZQCS RR */
           0x00694008   /* 255: REFRESH 3904ns */
         };


static const BBOX_Rts_PfriClient stBoxRts_PfriClient_20160212211041_1000_LPDDR4_2667[] = {
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_CONFIG,      0x80330d06}, /* HVD0_PFRI (gHVC_2k) 228120.00 ns/40 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_FILTER_CTRL, 0x40000267}, /* d: 4; p: 615.91875 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH0,     0x00001950}, /* 6480 */
  {BCHP_MEMC_GEN_0_PFRI_0_THROTTLE_THRESH1,     0x00000f30}  /* 60% * 6480 */
};

static const uint32_t* const paulMemc_box1000[] = { &aulMemc0_20160212211041_1000_LPDDR4_2667[0]};

const BBOX_Rts stBoxRts_1000_LPDDR4_2667_box1000 = {
  "20160212211041_1000_LPDDR4_2667_box1000",
  7271,
  1000,
  1,
  256,
  (const uint32_t**)&paulMemc_box1000[0],
  4,
  stBoxRts_PfriClient_20160212211041_1000_LPDDR4_2667
};

/* Checksum 0: 0x7ea136e */
/* Checksum 1: 0x0 */
