/******************************************************************************
 * (c) 2005-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *****************************************************************************/


#include "bstd.h"
#include "brsp.h"

#ifndef VEC_GRPD_RFM
#include "bchp_rfm_clk27.h"
#include "bchp_rfm_sysclk.h"
#include "brfm.h"
#include "brfm_priv.h"
#include "brfm_scripts.h"
#else

#endif

/*******************************************************************************
Sub RFM_Audio_Trap_95Taps_DEFAULT_NTSC()
' Audio Trap filter: symmetric FIR (95-tap design for TRAP00-TRAP23).
'    Designed for Rev. 3.0/4.0 RFM core (e.g. Bcm74xx).
' NTSC M/N version (4-MHz bandwidth, audio carrier at 4.5 MHz).
*******************************************************************************/
static const uint32_t BRFM_P_AudioTrapScrNtsc[] =
{
#if (BCHP_CHIP==7118)
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP00, 0x004cff38,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP01, 0x00ecffb0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP02, 0xff2001ac,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP03, 0xfea80004,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP04, 0x013cfeb0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP05, 0x00380104,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP06, 0xfeb4008c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP07, 0x0068ff38,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP08, 0x0088ffec,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP09, 0xffb400a4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP10, 0xff300058,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP11, 0x00d4fe14,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP12, 0x016800dc,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP13, 0xfd1c0244,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP14, 0x010cfc08,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP15, 0x0324015c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP16, 0xfae003e0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP17, 0x01c8f9cc,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP18, 0x046c0248,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP19, 0xf8d404d0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP20, 0x02d0f814,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP21, 0x04fc0338,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP22, 0xf7ac052c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP23, 0x02c4791c,
#elif (BRFM_REVID>=51)
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP00, 0xfff40014, /* -0.0003662109375/ 0.0006103515625 (   -3/    5) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP01, 0xfff8fff4, /* -0.0002441406250/-0.0003662109375 (   -2/   -3) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP02, 0x00180000, /*  0.0007324218750/ 0.0             (    6/    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP03, 0xffe4001c, /* -0.0008544921875/ 0.0008544921875 (   -7/    7) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP04, 0x0010ffd0, /*  0.0004882812500/-0.0014648437500 (    4/  -12) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP05, 0x00140030, /*  0.0006103515625/ 0.0014648437500 (    5/   12) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP06, 0xffc0fff8, /* -0.0019531250000/-0.0002441406250 (  -16/   -2) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP07, 0x0058ffc0, /*  0.0026855468750/-0.0019531250000 (   22/  -16) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP08, 0xffc8007c, /* -0.0017089843750/ 0.0037841796875 (  -14/   31) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP09, 0xffe0ff80, /* -0.0009765625000/-0.0039062500000 (   -8/  -32) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP10, 0x00900028, /*  0.0043945312500/ 0.0012207031250 (   36/   10) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP11, 0xff300074, /* -0.0063476562500/ 0.0035400390625 (  -52/   29) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP12, 0x00a0fef4, /*  0.0048828125000/-0.0081787109375 (   40/  -67) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP13, 0x00180134, /*  0.0007324218750/ 0.0093994140625 (    6/   77) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP14, 0xfef0ff68, /* -0.0083007812500/-0.0046386718750 (  -68/  -38) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP15, 0x01c4ff50, /*  0.0137939453125/-0.0053710937500 (  113/  -44) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP16, 0xfe68021c, /* -0.0124511718750/ 0.0164794921875 ( -102/  135) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP17, 0x0044fd30, /*  0.0020751953125/-0.0219726562500 (   17/ -180) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP18, 0x01f401f4, /*  0.0152587890625/ 0.0152587890625 (  125/  125) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP19, 0xfbe800dc, /* -0.0319824218750/ 0.0067138671875 ( -262/   55) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP20, 0x04b8fac8, /*  0.0368652343750/-0.0407714843750 (  302/ -334) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP21, 0xfdd009fc, /* -0.0170898437500/ 0.0780029296875 ( -140/  639) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP22, 0xfa04f254, /* -0.0467529296875/-0.1068115234375 ( -383/ -875) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP23, 0x25f44f14, /*  0.2965087890625/ 0.6177978515625 ( 2429/ 5061) */
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP00, 0x0004fffc,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP01, 0xfffc0008,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP02, 0x0000fff4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP03, 0x00080008,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP04, 0xffec0004,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP05, 0x0018ffe4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP06, 0xfff00030,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP07, 0xfff4ffc8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP08, 0x003c001c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP09, 0xff9c0020,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP10, 0x0068ff88,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP11, 0xffd000b8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP12, 0xffb8ff4c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP13, 0x00dc0044,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP14, 0xfebc0094,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP15, 0x012cfe74,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP16, 0xffa8022c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP17, 0xfee0fe14,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP18, 0x02d00068,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP19, 0xfc0c0260,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP20, 0x038cfa04,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP21, 0xff8c099c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP22, 0xf880f3b8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP23, 0x268c4d48,
#endif
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

/*******************************************************************************
Sub RFM_Audio_Trap_95Taps_PAL_D()
' Audio Trap filter: symmetric FIR (95-tap design for TRAP00-TRAP23).
'    Designed for Rev. 3.0/4.0 RFM core.
' PAL-D version (6-MHz bandwidth, audio carrier at 6.5 MHz).
*******************************************************************************/
static const uint32_t BRFM_P_AudioTrapScrPalD[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP00, 0xffd80018,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP01, 0xffe00024,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP02, 0xffdc0020,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP03, 0xffec0004,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP04, 0x0010ffd8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP05, 0x004cff90,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP06, 0x0094ff48,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP07, 0x00d4ff18,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP08, 0x00f0ff18,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP09, 0x00ccff64,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP10, 0x00580004,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP11, 0xff9000e4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP12, 0xfea001dc,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP13, 0xfdbc029c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP14, 0xfd3402d4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP15, 0xfd5c0238,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP16, 0xfe7400a0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP17, 0x008cfe18,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP18, 0x036cfb00,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP19, 0x0690f808,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP20, 0x08fcf668,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP21, 0x0998f7c4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP22, 0x06d4fa90,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP23, 0x042c7c00,
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

/*******************************************************************************
Sub RFM_Audio_Trap_95Taps_PAL_B()
' Audio Trap filter: symmetric FIR (95-tap design for TRAP00-TRAP23).
'    Designed for Rev. 3.0/4.0 RFM core (e.g. Bcm74xx).
' PAL-B/G/H version (5-MHz bandwidth, audio carrier at 5.5 MHz).
*******************************************************************************/
static const uint32_t BRFM_P_AudioTrapScrPalBG[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP00, 0x000c0004, /*  0.0003662109375 / 0.0001220703125 (   3 /    1) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP01, 0xffec0028, /* -0.0006103515625 / 0.0012207031250 (  -5 /   10) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP02, 0xffc4003c, /* -0.0018310546875 / 0.0018310546875 ( -15 /   15) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP03, 0xffd80000, /* -0.0012207031250 / 0.0             ( -10 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP04, 0x002cffb8, /*  0.0013427734375 /-0.0021972656250 (  11 /  -18) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP05, 0x0044ffe8, /*  0.0020751953125 /-0.0007324218750 (  17 /   -6) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP06, 0xffd80064, /* -0.0012207031250 / 0.0030517578125 ( -10 /   25) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP07, 0xff8c004c, /* -0.0035400390625 / 0.0023193359375 ( -29 /   19) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP08, 0x000cff90, /*  0.0003662109375 /-0.0034179687500 (   3 /  -28) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP09, 0x00b0ff64, /*  0.0053710937500 /-0.0047607421875 (  44 /  -39) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP10, 0x00340064, /*  0.0015869140625 / 0.0030517578125 (  13 /   25) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP11, 0xff1c0100, /* -0.0069580078125 / 0.0078125000000 ( -57 /   64) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP12, 0xff64ffd4, /* -0.0047607421875 /-0.0013427734375 ( -39 /  -11) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP13, 0x0100fe88, /*  0.0078125000000 /-0.0114746093750 (  64 /  -94) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP14, 0x013cffb0, /*  0.0096435546875 /-0.0024414062500 (  79 /  -20) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP15, 0xff0c01f4, /* -0.0074462890625 / 0.0152587890625 ( -61 /  125) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP16, 0xfde4012c, /* -0.0164794921875 / 0.0091552734375 (-135 /   75) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP17, 0x0098fd98, /*  0.0046386718750 /-0.0187988281250 (  38 / -154) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP18, 0x035cfd4c, /*  0.0262451171875 /-0.0211181640625 ( 215 / -173) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP19, 0x006402cc, /*  0.0030517578125 / 0.0218505859375 (  25 /  179) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP20, 0xfa9405e8, /* -0.0423583984375 / 0.0461425781250 (-347 /  378) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP21, 0xfce0fcf4, /* -0.0244140625000 /-0.0238037109375 (-200 / -195) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP22, 0x0b74ebf0, /*  0.0894775390625 /-0.1567382812500 ( 733 /-1284) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP23, 0x1a806320, /*  0.2070312500000 / 0.7744140625000 (1696 / 6344) */
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

/*******************************************************************************
Sub RFM_Audio_Trap_95Taps_PAL_I()
' Audio Trap filter: symmetric FIR (95-tap design for TRAP00-TRAP23).
'    Designed for Rev. 3.0/4.0 RFM core (e.g. Bcm74xx).
' PAL-I version (5.5-MHz bandwidth, audio carrier at 6.0 MHz).
*******************************************************************************/
static const uint32_t BRFM_P_AudioTrapScrPalI[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP00, 0x0000fff4, /*  0.0              -0.0003662109375 (   0 /   -3) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP01, 0x0010ffe8, /*  0.0004882812500 /-0.0007324218750 (   4 /   -6) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP02, 0x0020ffe0, /*  0.0009765625000 /-0.0009765625000 (   8 /   -8) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP03, 0x001cfff4, /*  0.0008544921875 /-0.0003662109375 (   7 /   -3) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP04, 0xfff40028, /* -0.0003662109375 / 0.0012207031250 (  -3 /   10) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP05, 0xffbc0050, /* -0.0020751953125 / 0.0024414062500 ( -17 /   20) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP06, 0xffb00038, /* -0.0024414062500 / 0.0017089843750 ( -20 /   14) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP07, 0xfff8ffcc, /* -0.0002441406250 /-0.0015869140625 (  -2 /  -13) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP08, 0x0070ff60, /*  0.0034179687500 /-0.0048828125000 (  28 /  -40) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP09, 0x00b0ff6c, /*  0.0053710937500 /-0.0045166015625 (  44 /  -37) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP10, 0x0050001c, /*  0.0024414062500 / 0.0008544921875 (  20 /    7) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP11, 0xff680104, /* -0.0046386718750 / 0.0079345703125 ( -38 /   65) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP12, 0xfebc0140, /* -0.0098876953125 / 0.0097656250000 ( -81 /   80) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP13, 0xff1c003c, /* -0.0069580078125 / 0.0018310546875 ( -57 /   15) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP14, 0x009cfe88, /*  0.0047607421875 /-0.0114746093750 (  39 /  -94) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP15, 0x0220fda4, /*  0.0166015625000 /-0.0184326171875 ( 136 / -151) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP16, 0x0208fee8, /*  0.0158691406250 /-0.0085449218750 ( 130 /  -70) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP17, 0xffac01f8, /* -0.0025634765625 / 0.0153808593750 ( -21 /  126) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP18, 0xfc84046c, /* -0.0272216796875 / 0.0345458984375 (-223 /  283) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP19, 0xfb9c0328, /* -0.0343017578125 / 0.0246582031250 (-281 /  202) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP20, 0xff3cfd44, /* -0.0059814453125 /-0.0213623046875 ( -49 / -175) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP21, 0x06d4f5c8, /*  0.0533447265625 /-0.0798339843750 ( 437 / -654) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP22, 0x0d8cefd0, /*  0.1058349609375 /-0.1264648437500 ( 867 /-1036) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TRAP23, 0x11a06d9c, /*  0.1376953125000 / 0.8563232421875 (1128 / 7015) */
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

/*******************************************************************************
Sub RFM_Group_Delay_FCC_M()
' Group-delay filter: symmetric FIR (40-tap design for GRP00-GRP19).
'    Originally designed for Rev. 2.0 RFM core, also works for 3.0 and 4.0.
' NTSC-M version (audio carrier at 4.5 MHz).
'
' ** RFM Group Delay DC gain = 0.9990234375 = 2046/2^11 = 0x7FE/2^11
'
' Designed to "FCC, EIA 1977" specification:
'
'           Nominal
'    Freq.  G.Delay  Tolerance
'    (MHz)   (ns)      (ns)
'    -----  -------- ---------
'     0         0      +/-25
'     0.1       0    Reference
'    >0.1       0      +/-25
'     1.0       0      +/-25
'     2.0       0      +/-25
'     3.0       0      +/-25
'     3.58   -170      +/-25
'     4.0    -293      +/-50
'     4.18   -346      +/-100
*******************************************************************************/
static const uint32_t BRFM_P_GroupDelayFCCScrNtsc[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP00, 0x00000180,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP01, 0xf8d010e0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP02, 0xe2202920,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP03, 0xd6601650,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP04, 0x0a80e140,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP05, 0x10c011a0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP06, 0xe1e0fd80,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP07, 0x3e904110,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP08, 0x1600fbb0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP09, 0xfbc00110,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP10, 0x01a0ffc0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP11, 0xff200000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP12, 0x00800010,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP13, 0xffc00010,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP14, 0x00500020,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP15, 0xfff0ffe0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP16, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP17, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP18, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP19, 0x00000000,
    BRSP_ScriptType_eEndOfScript, 0x00,             0x00000000
};

/*******************************************************************************
Sub RFM_Group_Delay_PAL_D__CCIR_Rpt_308()
' Group-delay filter: symmetric FIR (40-tap design for GRP00-GRP19).
'    Designed for Rev. 3.0/4.0 RFM core (also works with 2.x cores).
' PAL-D version (audio carrier at 6.5 MHz).
'
'  ** RFM Group Delay DC gain = 0.9951171875 = 2038/2^11 = 0x7F6/2^11
'
' Designed to "CCIR Report 308" specification:
'
'           Nominal
'    Freq.  G.Delay  Tolerance
'    (MHz)   (ns)      (ns)
'    -----  -------- ---------
'     0        0    Reference
'     0.25   + 5      +/-12
'     1.0    +53      +/-12
'     2.0    +87      +/-12
'     3.0    +85      +/-12
'     4.0    +50      +/-12
'     4.43     0      +/-12
'     0      -90      +/-30
*******************************************************************************/
static const uint32_t BRFM_P_GroupDelayScrPalD[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP00, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP01, 0xffe00020,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP02, 0xffd00050,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP03, 0xff7000e0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP04, 0xfeb00210,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP05, 0xfcd00520,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP06, 0xf8500c60,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP07, 0xee401a80,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP08, 0xe3102060,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP09, 0xf7f0fa80,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP10, 0x66d02940,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP11, 0xf610f3b0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP12, 0x0240fd00,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP13, 0x0100fea0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP14, 0x0120ff10,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP15, 0x0090ff90,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP16, 0x0040ffd0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP17, 0x0010fff0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP18, 0xfff00000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP19, 0xffe00010,
    BRSP_ScriptType_eEndOfScript, 0x00,             0x00000000
};

/*******************************************************************************
Sub RFM_Group_Delay_PAL_D__China_Flat()
' Group-delay filter: symmetric FIR (40-tap design for GRP00-GRP19).
'
' Impulse (delay-only) filter with DC gain matched to filter above
'    [RFM_Group_Delay_PAL_D__CCIR_Rpt_308()].
'
'  ** RFM Group Delay DC gain = 0.9951171875 = 2038/2^11 = 0x7F6/2^11
*******************************************************************************/
static const uint32_t BRFM_P_GroupDelayScrPalD_China[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP00, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP01, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP02, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP03, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP04, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP05, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP06, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP07, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP08, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP09, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP10, 0x7f600000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP11, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP12, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP13, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP14, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP15, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP16, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP17, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP18, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP19, 0x00000000,
    BRSP_ScriptType_eEndOfScript, 0x00,             0x00000000
};

/*******************************************************************************
Sub RFM_Group_Delay_PAL_B__General()
' Group-delay filter: symmetric FIR (40-tap design for GRP00-GRP19).
'    Designed for Rev. 3.0/4.0 RFM core (also works with 2.x cores).
' PAL-B version (audio carrier at 5.5 MHz).
'
' Designed to following specification:
' (see Rohde & Schwarz Sound and TV Broadcasting's "CCIR and FCC TV Standards"
'  document, "GROUP DELAY" section, "Standard B/G" table, "general"
'  pre-correction column of table)
'
'           Nominal
'    Freq.  G.Delay  Tolerance
'    (MHz)   (ns)      (ns)
'    -----  -------- ---------
'     0.1      0    Reference
'     0.25   + 5      +/-12
'     1.0    +53      +/-12
'     2.0    +90      +/-12
'     3.0    +75      +/-12
'     3.75     0      +/-12
'     4.43  -170      +/-25
'     4.8   -400      +/-90
*******************************************************************************/
static const uint32_t BRFM_P_GroupDelayScrPalBG[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP00, 0xffc0ffb0, /* -0.00195312500 /-0.00244140625 (  -4 /   -5) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP01, 0x01f0fa80, /*  0.01513671875 /-0.04296875000 (  31 /  -88) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP02, 0x0b10ee50, /*  0.08642578125 /-0.13818359375 ( 177 / -283) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP03, 0x1710e7c0, /*  0.18017578125 /-0.18945312500 ( 369 / -388) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP04, 0x1310f820, /*  0.14892578125 /-0.06152343750 ( 305 / -126) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP05, 0xfab00e90, /* -0.04150390625 / 0.11376953125 ( -85 /  233) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP06, 0xf21003e0, /* -0.10888671875 / 0.03027343750 (-223 /   62) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP07, 0x0ba0eeb0, /*  0.09082031250 /-0.13525390625 ( 186 / -277) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP08, 0x09101480, /*  0.07080078125 / 0.16015625000 ( 145 /  328) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP09, 0xe9401180, /* -0.17773437500 / 0.13671875000 (-364 /  280) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP10, 0x5a602840, /*  0.70605468750 / 0.31445312500 (1446 /  644) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP11, 0xf9e0f530, /* -0.04785156250 /-0.08447265625 ( -98 / -173) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP12, 0xfcd00050, /* -0.02490234375 / 0.00244140625 ( -51 /    5) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP13, 0x0130fc60, /*  0.00927734375 /-0.02832031250 (  19 /  -58) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP14, 0x0560f930, /*  0.04199218750 /-0.05322265625 (  86 / -109) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP15, 0x0680fb20, /*  0.05078125000 /-0.03808593750 ( 104 /  -78) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP16, 0x02a0ff60, /*  0.02050781250 /-0.00488281250 (  42 /  -10) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP17, 0xff200180, /* -0.00683593750 / 0.01171875000 ( -14 /   24) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP18, 0xfe700110, /* -0.01220703125 / 0.00830078125 ( -25 /   17) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP19, 0xff500040, /* -0.00537109375 / 0.00195312500 ( -11 /    4) */
    BRSP_ScriptType_eEndOfScript, 0x00,             0x00000000
};

/*******************************************************************************
Sub RFM_Group_Delay_PAL_I__BBC_Flat()
'Sub RFM_Group_Delay_PAL_I__Flat()
' Group-delay filter: symmetric FIR (40-tap design for GRP00-GRP19).
'    Designed for Rev. 3.0/4.0 RFM core (also works with 2.x cores).
' PAL-I version (audio carrier at 6.0 MHz).
'
' Designed to following specification:
' (see Rohde & Schwarz Sound and TV Broadcasting's "CCIR and FCC TV Standards"
'  document, "GROUP DELAY" section, "Standard I" table, "BBC system without
'  receiver precorrection" column of table)
'
'           Nominal
'    Freq.  G.Delay  Tolerance
'    (MHz)   (ns)      (ns)
'    -----  -------- ---------
'     0.01     0     Reference
'     0.1      0      +/-40
'     0.2      0      +/-40
'    <3.6      0      +/-40
'    >3.6      0      +/-20
'     4.0      0      +/-20
'     4.43     0      +/-20
'     4.8      0      +/-20
'    <5.2      0      +/-20
'    >5.2      0      +/-80
'     5.5      0      +/-80
*******************************************************************************/
static const uint32_t BRFM_P_GroupDelayScrPalI[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP00, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP01, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP02, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP03, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP04, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP05, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP06, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP07, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP08, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP09, 0x00007ff0, /* 0.0            / 0.99951171875 (   0 / 2047) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP10, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP11, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP12, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP13, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP14, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP15, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP16, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP17, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP18, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP19, 0x00000000, /* 0.0            / 0.0           (   0 /    0) */
    BRSP_ScriptType_eEndOfScript, 0x00,             0x00000000
};

/*******************************************************************************
Sub RFM_Group_Delay_PAL_L__TDF()
' Group-delay filter: symmetric FIR (40-tap design for GRP00-GRP19).
'    Designed for Rev. 3.0/4.0 RFM core (also works with 2.x cores).
' PAL-L version (AM audio carrier at 6.5 MHz).
'
' Designed to following specification:
' (see Rohde & Schwarz Sound and TV Broadcasting's "CCIR and FCC TV Standards"
'  document, "GROUP DELAY" section, "Standard L, K1, K'" table, "TDF"
'  pre-correction column of table)
'
'           Nominal
'    Freq.  G.Delay  Tolerance
'    (MHz)   (ns)      (ns)
'    -----  -------- ---------
'     0.1      0     Reference
'     0.2      0      +/-15
'     2.0      0      +/-15
'     4.0      0      +/-15
'     4.43     0      +/-15
'     4.6      0      +/-15
'     4.8    -20      +/-35
'     5.0    -57.5    +/-42.5
'     5.25  -100      +/-42.5
'     5.5  <-100
*******************************************************************************/
static const uint32_t BRFM_P_GroupDelayScrPalL[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP00, 0x0030ffd0, /*  0.00146484375 /-0.00146484375 (   3/   -3) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP01, 0x0040ff80, /*  0.00195312500 /-0.00390625000 (   4/   -8) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP02, 0x00c0fef0, /*  0.00585937500 /-0.00830078125 (  12/  -17) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP03, 0x0160fe30, /*  0.01074218750 /-0.01416015625 (  22/  -29) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP04, 0x0240fd00, /*  0.01757812500 /-0.02343750000 (  36/  -48) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP05, 0x03f0fb10, /*  0.03076171875 /-0.03857421875 (  63/  -79) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP06, 0x0580fb00, /*  0.04296875000 /-0.03906250000 (  88/  -80) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP07, 0x02800280, /*  0.01953125000 / 0.01953125000 (  40/   40) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP08, 0xf59015b0, /* -0.08154296875 / 0.16943359375 (-167/  347) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP09, 0xd8d062c0, /* -0.30615234375 / 0.77148437500 (-627/ 1580) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP10, 0x3d40f940, /*  0.47851562500 /-0.05273437500 ( 980/ -108) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP11, 0xfdf00340, /* -0.01611328125 / 0.02539062500 ( -33/   52) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP12, 0xfdd000b0, /* -0.01708984375 / 0.00537109375 ( -35/   11) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP13, 0x0050ff40, /*  0.00244140625 /-0.00585937500 (   5/  -12) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP14, 0x00b0ff80, /*  0.00537109375 /-0.00390625000 (  11/   -8) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP15, 0x0060ffa0, /*  0.00292968750 /-0.00292968750 (   6/   -6) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP16, 0x0070ff90, /*  0.00341796875 /-0.00341796875 (   7/   -7) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP17, 0x0070ffb0, /*  0.00341796875 /-0.00244140625 (   7/   -5) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP18, 0x0040ffe0, /*  0.00195312500 /-0.00097656250 (   4/   -2) */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_GRP19, 0x0010ffe0, /*  0.00048828125 /-0.00097656250 (   1/   -2) */
    BRSP_ScriptType_eEndOfScript, 0x00,                 0x00000000
};

/*******************************************************************************
*******************************************************************************/
/* 15 kHz left-input/output lowpass filters */
static const uint32_t BRFM_P_BTSCSetup15kHzLeft[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF1A1, 0x000C6E58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF1B1, 0x00089C12,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF1B2, 0x0003989C,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF1C1, 0x000868F2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF1C2, 0x0003EA90,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF2A1, 0x0008C660,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF2A2, 0x000353A4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF2B1, 0x00087B92,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF2B2, 0x0003CD46,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF2C1, 0x00085DAE,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_LCF2C2, 0x0003FA34,
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

/* 15 kHz right-input/output lowpass filters (currently not in use) */
static const uint32_t BRFM_P_BTSCSetup15kHzRight[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1A1, 0x000c6e58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B1, 0x00089c12,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B2, 0x0003989c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C1, 0x000868f2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C2, 0x0003ea90,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A1, 0x0008c660,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A2, 0x000353a4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B1, 0x00087b92,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B2, 0x0003cd46,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C1, 0x00085dae,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C2, 0x0003fa34,
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

/*******************************************************************************
Sub RFM_7038_Audio_Setup_Stereo()
BTSC Stereo
*******************************************************************************/
static const uint32_t BRFM_P_AudioSetupStereo[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,      0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,      0x08cb9000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VOLCTL,     0x000000c5,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_AUDIOTONE, SEL_TPVID, 0),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MODPLT,     0x802630c4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLPLT,     0x00002312,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLPR,     0x000057AC,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLMR,     0x0000af58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLAM,      0x00010000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATLR,      0xaf5857ac,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATSUM,     0x0000af58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCB0,   0x000fffff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCB1,   0x00090001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCA1,   0x00000300,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0xb1810F0F,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0x91810F0F,
#if (BRFM_REVID>=50) /* override */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0x91812020,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL2,   0x004C1D81,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGROFFSAT, 0x005D34EE, /* +/-150 Hz saturation limit */
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1A1,     0x000c6e58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B1,     0x00089c12,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B2,     0x0003989c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C1,     0x000868f2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C2,     0x0003ea90,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A1,     0x0008c660,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A2,     0x000353a4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B1,     0x00087b92,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B2,     0x0003cd46,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C1,     0x00085dae,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C2,     0x0003fa34,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1,     0x0003f5c2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0,     0x0002f996,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1,     0x000d25e8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AUDIOBYP,   0x00800300,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREA1,     0x000b7678,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREA2,     0x0000d732,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB0,     0x0001b7f4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB1,     0x000ca668,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB2,     0x0001a1ca,
    BRSP_ScriptType_eEndOfScript, 0x00,                  0x00000000
};

static const uint32_t BRFM_P_AudioSetupStereoExtDeviation[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,      0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,      0x08cb9000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VOLCTL,     0x000000c5,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_AUDIOTONE, SEL_TPVID, 0),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MODPLT,     0x802630c4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLPLT,     0x00002312,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLPR,     0x000057AC,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLMR,     0x0000af58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLAM,      0x00010000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATLR,      0xffffffff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATSUM,     0x0000ffff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCB0,   0x000fffff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCB1,   0x00090001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCA1,   0x00000300,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0xb1810F0F,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0x91810F0F,
#if (BRFM_REVID>=50) /* override */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0x91812020,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL2,   0x004C1D81,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGROFFSAT, 0x005D34EE, /* +/-150 Hz saturation limit */
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1A1,     0x000c6e58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B1,     0x00089c12,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B2,     0x0003989c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C1,     0x000868f2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C2,     0x0003ea90,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A1,     0x0008c660,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A2,     0x000353a4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B1,     0x00087b92,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B2,     0x0003cd46,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C1,     0x00085dae,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C2,     0x0003fa34,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1,     0x0003f5c2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0,     0x0005f32c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1,     0x000a4bd0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AUDIOBYP,   0x00800300,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREA1,     0x000b7678,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREA2,     0x0000d732,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB0,     0x00036fe8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB1,     0x00094cd0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB2,     0x00034394,
    BRSP_ScriptType_eEndOfScript, 0x00,                  0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrStereo[] =
{
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioSetupStereo,   0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_BTSCSetup15kHzLeft, 0x00000000,
    BRSP_ScriptType_eEndOfScript,  0x00,                                0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrStereoExtDeviation[] =
{
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioSetupStereoExtDeviation, 0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_BTSCSetup15kHzLeft,           0x00000000,
    BRSP_ScriptType_eEndOfScript,  0x00,                                          0x00000000
};

/*******************************************************************************
Sub RFM_7038_Audio_Setup_Mono75us()
Mono (M,N): 75 us pre-emphasis, 25 kHz deviation
*******************************************************************************/
static const uint32_t BRFM_P_AudioEncodingScrMonoCommon[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VOLCTL,   0x000000c5,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_AUDIOTONE, SEL_TPVID, 0),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MODPLT,   0x002630c4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLPLT,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLPR,   0x000057AC,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLMR,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLAM,    0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATLR,    0xffffffff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATSUM,   0x0000ffff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1A1,   0x000c6e58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B1,   0x00089c12,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B2,   0x0003989c,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C1,   0x000868f2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C2,   0x0003ea90,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A1,   0x0008c660,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A2,   0x000353a4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B1,   0x00087b92,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B2,   0x0003cd46,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C1,   0x00085dae,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C2,   0x0003fa34,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AUDIOBYP, 0x00000300,
    BRSP_ScriptType_eEndOfScript, 0x00,                0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrMonoNtsc[] =
{
    /* NTSC Mono Script, uses 75us Pre-emphasis Filter */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,  0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,  0x08cb9000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioEncodingScrMonoCommon, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1, 0x0003f5c2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0, 0x0002f996,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1, 0x000d25e8,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_BTSCSetup15kHzLeft, 0x00000000,
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrMonoPalSecam[] =
{
    /* Pal/Secam Mono Script, uses 50us Pre-emphasis Filter */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,  0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,  0x11973000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioEncodingScrMonoCommon, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1, 0x0003F5C2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0, 0x000200FA,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1, 0x000E1E84,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_BTSCSetup15kHzLeft, 0x00000000,
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrMonoPalD[] =
{
    /* PalD Mono Script, uses 50us Pre-emphasis Filter */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,  0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,  0x11973000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioEncodingScrMonoCommon, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1, 0x0003F5C2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0, 0x000200FA,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1, 0x000E1E84,
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrNicam[] =
{
    /* Nicam Stereo script, uses 50us Pre-emphasis Filter */
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioEncodingScrMonoCommon, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,  0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,  0x11973000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1, 0x0003F5C2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0, 0x000200FA,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1, 0x000E1E84,
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrNicamPalD[] =
{
    /* Nicam Stereo script, uses 50us Pre-emphasis Filter */
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioEncodingScrMonoCommon, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,  0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,  0x11973000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1, 0x0003F5C2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0, 0x000200FA,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1, 0x000E1E84,

    /* Need a lower-bandwidth audio trap because of NICAM at 5.85MHz
    (FM carrier at 6.5MHz). PAL-D filter has a 6.4MHz stopband edge, so use
    the PAL-B filter (5.4MHz stopband edge) instead of PAL-D filter  */
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioTrapScrPalBG, 0x00000000,
    BRSP_ScriptType_eEndOfScript, 0x00,              0x00000000
};

/*******************************************************************************
Sub RFM_7038_Audio_Setup_MonoAndSAP()
Mono and SAP (BTSC)
*******************************************************************************/
static const uint32_t BRFM_P_AudioEncodingScrSap[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,      0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,      0x08cb9000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VOLCTL,     0x0000018a,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_AUDIOTONE, SEL_TPVID, 0),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MODPLT,     0x002630c4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLPLT,     0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLPR,     0x000057AC,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLMR,     0x0000349A,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLAM,      0x00006117,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATLR,      0xaf58af58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATSUM,     0x0000af58,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0xb1810F0F,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0x91810F0F,
#if (BRFM_REVID>=50) /* override */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0x91812020,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL2,   0x004C1D81,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGROFFSAT, 0x005D34EE, /* +/-150 Hz saturation limit */
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1A1,     0x000C55F2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B1,     0x00086E6A,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1B2,     0x0003AE16,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C1,     0x00083DE0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF1C2,     0x0003EEC0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A1,     0x000896A0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2A2,     0x000377F8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B1,     0x00084F92,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2B2,     0x0003D776,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C1,     0x000833A4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RCF2C2,     0x0003FB48,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1,     0x0003f5c2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0,     0x0002f996,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1,     0x000d25e8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCB0,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCB1,   0x00020000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCA1,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AUDIOBYP,   0x01800380,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREA1,     0x000b7678,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREA2,     0x0000d732,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB0,     0x0001b7f4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB1,     0x000ca668,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SPREB2,     0x0001a1ca,
    BRSP_ScriptType_eEndOfScript, 0x00,                  0x00000000
};

static const uint32_t BRFM_P_AudioEncodingScrSapMono[] =
{
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioEncodingScrSap, 0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_BTSCSetup15kHzLeft,  0x00000000,
};

/*******************************************************************************
Sub RFM_7038_Audio_Setup_JMono1()
Single-input (left channel) mono for Japan: 75 us pre-emphasis, 25 kHz deviation
*******************************************************************************/
static const uint32_t BRFM_P_AudioEncodingScrJpMono1[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMCTL,      0x00010fff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMSCL,      0x08cb9000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VOLCTL,     0x0000018a,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_AUDIOTONE, SEL_TPVID, 0),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MODPLT,     0x002630c4,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLPLT,     0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLPR,     0x000057AC,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLLMR,     0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SCLAM,      0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATLR,      0xffffffff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_SATSUM,     0x0000ffff,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0xb1810F0F,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0x91810F0F,
#if (BRFM_REVID>=50) /* override */
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL1,   0x91812020,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGRCTL2,   0x004C1D81,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_RMGROFFSAT, 0x005D34EE, /* +/-150 Hz saturation limit */
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREA1,     0x0003f5c2,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB0,     0x0002f996,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_MPREB1,     0x000d25e8,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCB0,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCB1,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_OVRSPCA1,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AUDIOBYP,   0x00000380,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_BTSCSetup15kHzLeft, 0x00000000,
    BRSP_ScriptType_eEndOfScript, 0x00,                  0x00000000
};

/*******************************************************************************
*******************************************************************************/
static const uint32_t BRFM_P_ChScrNtscOpenCableRF[] =
{
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_FILSCL, AUDSCL, 0x300),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x30002000,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_ChScrNtscOpenCableCh3[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((61.25/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_ChScrNtscOpenCableRF, 0x00000000,
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_ChScrNtscOpenCableCh4[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((67.25/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_ChScrNtscOpenCableRF, 0x00000000,
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_ChScrNtscOpenCableBaseband[] =
{
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_FILSCL, AUDSCL, 0xfff),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x00002000,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 1),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 1),
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

/* Keep in mind that ChScrSIF and ChScrNtscOpenCableBaseband are not identical.
   BYP_FM=0x0 for SIF and 0x1 for Baseband, which is set in BRFM_SetAudioEncoding() */
static const uint32_t BRFM_P_ChScrSIF[] =
{
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_FILSCL, AUDSCL, 0xfff),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x00002000,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 1),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 1),
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_InitScrNtscOpenCableConfig[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMFREQ,     0x2AAAAAAB, /* 4.5MHz audio carrier*/
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_GroupDelayFCCScrNtsc, 0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioTrapScrNtsc,     0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    (0x11000000 | BRFM_PLL_MULT),
#if BRFM_SPUR_WORKAROUND
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    (0x11000000 | BRFM_PLL_MULT | 0x800000), /* bit 23, PLL_MN_OVERRIDE */
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIP1,      0x7ff88000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIP2,      0x7ffc8000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIPBYP,    0x00010001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL,   0x30002000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL1,     0x12E401B0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2,     0x70000001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MODTST,    0x00000504,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MODBYP,    0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL,    0x0000003f,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOBYP,   0x00000000,
    /* SW7429-1217 - RFM video SNR not complying customer spec */
#if (BRFM_REVID==51)
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_MISC3, KA, 3),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_MISC3, KI, 3),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_MISC3, KP, 10),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_MISC2, PLL_CTRL, 0x800000),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_CLKCTL, PLL_MN_OVERRIDE, 1),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_DIV1, NDIV_INT, 56),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_DIV1, PDIV, 1),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_CH0, M_DIV, 8),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_CH0, LOAD_EN, 1),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_CH0, LOAD_EN, 0),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_MISC1, ARESET, 1),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_DPLL_MISC1, ARESET, 0),
#endif
#if (BRFM_REVID==40 || BRFM_REVID==50)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_TSTCNTL,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x28408001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_PLL_MISC1, 0x00000020,
#elif (BRFM_REVID>=51)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_TSTCNTL,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x28408001,
    #if (BCHP_CHIP!=7445) && (BCHP_CHIP!=7145) && (BCHP_CHIP!=7439) && (BCHP_CHIP!=74371) && (BCHP_CHIP!=7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DPLL_MISC1, 0x00000020,
    #endif
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TSTCNTL,    0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x3f000001,
#endif
#if (BRFM_REVID>=50)
    #if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x00000001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
    #else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x28408009, /* override TC_OBB_2 = 1 */
    #endif
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MISC,      BRFM_PLL_MISC,

#if BRFM_SPUR_WORKAROUND
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_PLL_DIV1,  0x1c0101, /* reset value is 0x510102 */
#endif
    BRSP_ScriptType_eEndOfScript, 0x00,                  0x00000000
};

static const uint32_t BRFM_P_ChScrPalICh3[] =
{
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439 || BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT),
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((53.75/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eEndOfScript, 0x00,               0x00000000
};

static const uint32_t BRFM_P_ChScrPalICh4[] =
{
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371)  || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT),
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((61.75/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eEndOfScript, 0x00,               0x00000000
};

static const uint32_t BRFM_P_ChScrPalIBaseband[] =
{
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_InitScrPalIConfig[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMFREQ,   0x38E38E39, /* 6.0 MHz audio carrier*/
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL,   0x46D00000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioTrapScrPalI,  0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_GroupDelayScrPalI, 0x00000000,
#if (BRFM_REVID>=40)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x28408001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2,   0x70000001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x20002000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL,  0x0000003f,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x3f000001,
#endif
#if (BRFM_REVID>=50)
    #if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x00000001,
    #else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x28408009, /* override TC_OBB_2 = 1 */
    #endif
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MISC,    BRFM_PLL_MISC,
    BRSP_ScriptType_eEndOfScript, 0x00,                0x00000000
};

static const uint32_t BRFM_P_ChScrPalDCh3[] =
{
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT),
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((65.75/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eEndOfScript, 0x00,               0x00000000
};

static const uint32_t BRFM_P_ChScrPalDCh4[] =
{
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145)  || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT),
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((77.25/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eEndOfScript, 0x00,               0x00000000
};

static const uint32_t BRFM_P_ChScrPalDBaseband[] =
{
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_InitScrPalDConfig[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMFREQ,   0x3DA12F68, /* 6.5 MHz audio carrier*/
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL,   0x32200000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioTrapScrPalD,  0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_GroupDelayScrPalD, 0x00000000,
#if (BRFM_REVID>=40)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x28408001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2,   0x70000001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x20002000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL,  0x0000003f,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x3f000001,
#endif
#if (BRFM_REVID>=50)
    #if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x00000001,
    #else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x28408009, /* override TC_OBB_2 = 1 */
    #endif
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MISC,    BRFM_PLL_MISC,
    BRSP_ScriptType_eEndOfScript, 0x00,                0x00000000
};

static const uint32_t BRFM_P_ChScrPalDChinaCh3[] =
{
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145)|| (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT),
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((65.75/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eEndOfScript, 0x00,               0x00000000
};

static const uint32_t BRFM_P_ChScrPalDChinaCh4[] =
{
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT),
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((77.25/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eEndOfScript, 0x00,               0x00000000
};

static const uint32_t BRFM_P_ChScrPalDChinaBaseband[] =
{
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_InitScrPalDChinaConfig[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMFREQ,   0x3DA12F68, /* 6.5 MHz audio carrier*/
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL,   0x32200000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioTrapScrPalD,        0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_GroupDelayScrPalD_China, 0x00000000,
#if (BRFM_REVID>=40)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x28408001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2,   0x70000001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x20002000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL,  0x0000003f,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x3f000001,
#endif
#if (BRFM_REVID>=50)
    #if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x00000001,
    #else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x28408009, /* override TC_OBB_2 = 1 */
    #endif
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MISC,    BRFM_PLL_MISC,
    BRSP_ScriptType_eEndOfScript, 0x00,                0x00000000
};

static const uint32_t BRFM_P_ChScrPalBGCh3[] =
{
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT),
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((55.25/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eEndOfScript, 0x00,               0x00000000
};

static const uint32_t BRFM_P_ChScrPalBGCh4[] =
{
#if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL, (0x11000000 | BRFM_PLL_MULT),
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_AMFREQ, ((62.25/BRFM_PLL_FS)*0xFFFFFFFF),
    BRSP_ScriptType_eEndOfScript, 0x00,               0x00000000
};

static const uint32_t BRFM_P_ChScrPalBGBaseband[] =
{
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_InitScrPalBGConfig[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMFREQ,   0x3425ED09, /* 5.5 MHz audio carrier*/
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL,   0x32200000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioTrapScrPalBG,  0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_GroupDelayScrPalBG, 0x00000000,
#if (BRFM_REVID>=40)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x28408001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2,   0x70000001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL, 0x20002000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL,  0x0000003f,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, BYP_MIX, 0),
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_SYSCLK_MODBYP, SEL_MIX, 0),
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x3f000001,
#endif
#if (BRFM_REVID>=50)
    #if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP==7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x00000001,
    #else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,  0x28408009, /* override TC_OBB_2 = 1 */
    #endif
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MISC,    BRFM_PLL_MISC,
    BRSP_ScriptType_eEndOfScript, 0x00,                0x00000000
};

static const uint32_t BRFM_P_ChScrPalMCh3[] =
{
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_ChScrNtscOpenCableCh3, 0x00000000,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_FILSCL, AUDSCL, 0x3bc), /* override to -15dB A/V ratio */
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_ChScrPalMCh4[] =
{
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_ChScrNtscOpenCableCh4, 0x00000000,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_FILSCL, AUDSCL, 0x3bc),/* override to -15dB A/V ratio */
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_ChScrPalNCh3[] =
{
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_ChScrNtscOpenCableCh3, 0x00000000,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_FILSCL, AUDSCL, 0x3bc), /* override to -15dB A/V ratio */
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_ChScrPalNCh4[] =
{
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_ChScrNtscOpenCableCh4, 0x00000000,
    BRSP_ScriptType_eRdModWr32, BRSP_RD_MOD_WR_32(RFM_CLK27_FILSCL, AUDSCL, 0x3bc), /* override to -15dB A/V ratio */
    BRSP_ScriptType_eEndOfScript, 0x00, 0x00000000
};

static const uint32_t BRFM_P_InitScrPalNConfig[] =
{
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FMFREQ,     0x2AAAAAAB, /* 4.5MHz audio carrier*/
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_FILSCL,     0x3bc00000, /* override to -15dB A/V ratio */
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_AudioTrapScrNtsc,        0x00000000,
    BRSP_ScriptType_eNestedScript, (uint32_t)BRFM_P_GroupDelayScrPalD_China, 0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    (0x11000000 | BRFM_PLL_MULT),
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIP1,      0x7ff88000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIP2,      0x7ffc8000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_CLIPBYP,    0x00010001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOSCL,   0x30002000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL1,     0x12E401B0,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_AMCTL2,     0x70000001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MODTST,    0x00000504,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MODBYP,    0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_OUTSCL,    0x0000003f,
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_VIDEOBYP,   0x00000000,

#if (BRFM_REVID==40 || BRFM_REVID==50)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_TSTCNTL,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x28408001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_PLL_MISC1, 0x00000020,
#elif (BRFM_REVID>=51)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_TSTCNTL,   0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x28408001,
    #if (BCHP_CHIP!=7445) && (BCHP_CHIP!=7145)  && (BCHP_CHIP!=7439)  && (BCHP_CHIP!=74371) && (BCHP_CHIP!=7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DPLL_MISC1, 0x00000020,
    #endif
#else
    BRSP_ScriptType_eWrite32, BCHP_RFM_CLK27_TSTCNTL,    0x00000000,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x3f000001,
#endif
#if (BRFM_REVID>=50)
    #if (BCHP_CHIP==7445) || (BCHP_CHIP==7145) || (BCHP_CHIP==7439)  || (BCHP_CHIP==74371) && (BCHP_CHIP!=7364)
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x00000001,
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_CLKCTL,    0x0100000e,
    #else
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_DACCTL,    0x28408009, /* override TC_OBB_2 = 1 */
    #endif
#endif
    BRSP_ScriptType_eWrite32, BCHP_RFM_SYSCLK_MISC,      BRFM_PLL_MISC,
    BRSP_ScriptType_eEndOfScript, 0x00,                  0x00000000
};

#ifndef VEC_GRPD_RFM

static const BRFM_P_ModulationInfo BRFM_P_ModInfo[] =
{
    {
        BRFM_ModulationType_eNtscOpenCable,
        {
            BRFM_P_ChScrNtscOpenCableBaseband,
            BRFM_P_ChScrNtscOpenCableCh3,
            BRFM_P_ChScrNtscOpenCableCh4,
            BRFM_P_ChScrSIF,
        },
        {
            BRFM_P_AudioEncodingScrMonoNtsc,
            BRFM_P_AudioEncodingScrStereo,
            BRFM_P_AudioEncodingScrStereoExtDeviation,
            BRFM_P_AudioEncodingScrSap,      /* SAP only, same as Sap/Mono, but volume for Mono is diabled */
            BRFM_P_AudioEncodingScrSapMono,  /* Sap/Mono */
            BRFM_P_AudioEncodingScrJpMono1,  /* Japan Mono1 */
            BRFM_P_AudioEncodingScrMonoNtsc, /* Japan Mono2, Same as Mono */
            NULL,                            /* No NICAM support on NTSC */
            NULL,
            NULL
        },
        BRFM_P_InitScrNtscOpenCableConfig
    },
    {
        BRFM_ModulationType_ePalI,
        {
            BRFM_P_ChScrPalIBaseband,
            BRFM_P_ChScrPalICh3,
            BRFM_P_ChScrPalICh4,
            BRFM_P_ChScrSIF,
        },
        {
            BRFM_P_AudioEncodingScrMonoPalSecam, /* PAL only supports Mono */
            BRFM_P_AudioEncodingScrMonoPalSecam,
            NULL,
            BRFM_P_AudioEncodingScrMonoPalSecam,
            BRFM_P_AudioEncodingScrMonoPalSecam,
            BRFM_P_AudioEncodingScrMonoPalSecam,
            BRFM_P_AudioEncodingScrMonoPalSecam,
            BRFM_P_AudioEncodingScrNicam,        /* NICAM support */
            BRFM_P_AudioEncodingScrNicam,
            BRFM_P_AudioEncodingScrNicam
        },
        BRFM_P_InitScrPalIConfig
    },
    {
        BRFM_ModulationType_ePalD,
        {
            BRFM_P_ChScrPalDBaseband,
            BRFM_P_ChScrPalDCh3,
            BRFM_P_ChScrPalDCh4,
            BRFM_P_ChScrSIF,
        },
        {
            BRFM_P_AudioEncodingScrMonoPalD,  /* PAL-D only supports Mono */
            BRFM_P_AudioEncodingScrMonoPalD,
            NULL,
            BRFM_P_AudioEncodingScrMonoPalD,
            BRFM_P_AudioEncodingScrMonoPalD,
            BRFM_P_AudioEncodingScrMonoPalD,
            BRFM_P_AudioEncodingScrMonoPalD,
            BRFM_P_AudioEncodingScrNicamPalD, /* NICAM support */
            BRFM_P_AudioEncodingScrNicamPalD,
            BRFM_P_AudioEncodingScrNicamPalD
        },
        BRFM_P_InitScrPalDConfig
    },
    {
        BRFM_ModulationType_ePalDChina,
        {
            BRFM_P_ChScrPalDChinaBaseband,
            BRFM_P_ChScrPalDChinaCh3,
            BRFM_P_ChScrPalDChinaCh4,
            BRFM_P_ChScrSIF,
        },
        {
            BRFM_P_AudioEncodingScrMonoPalD,  /* PAL-D only supports Mono */
            BRFM_P_AudioEncodingScrMonoPalD,
            NULL,
            BRFM_P_AudioEncodingScrMonoPalD,
            BRFM_P_AudioEncodingScrMonoPalD,
            BRFM_P_AudioEncodingScrMonoPalD,
            BRFM_P_AudioEncodingScrMonoPalD,
            BRFM_P_AudioEncodingScrNicamPalD, /* NICAM support */
            BRFM_P_AudioEncodingScrNicamPalD,
            BRFM_P_AudioEncodingScrNicamPalD
        },
        BRFM_P_InitScrPalDChinaConfig
    },
    {
        BRFM_ModulationType_ePalBG,
        {
            BRFM_P_ChScrPalBGBaseband,
            BRFM_P_ChScrPalBGCh3,
            BRFM_P_ChScrPalBGCh4,
            BRFM_P_ChScrSIF,
        },
        {
            BRFM_P_AudioEncodingScrMonoPalSecam, /* PAL-D only supports Mono */
            BRFM_P_AudioEncodingScrMonoPalSecam,
            NULL,
            BRFM_P_AudioEncodingScrMonoPalSecam,
            BRFM_P_AudioEncodingScrMonoPalSecam,
            BRFM_P_AudioEncodingScrMonoPalSecam,
            BRFM_P_AudioEncodingScrMonoPalSecam,
            BRFM_P_AudioEncodingScrNicam,        /* NICAM support */
            BRFM_P_AudioEncodingScrNicam,
            BRFM_P_AudioEncodingScrNicam
        },
        BRFM_P_InitScrPalBGConfig
    },
    {
        BRFM_ModulationType_ePalM, /* Pal-M is very similar to NTSC, with one override for A/V ratio */
        {
            BRFM_P_ChScrNtscOpenCableBaseband,
            BRFM_P_ChScrPalMCh3,
            BRFM_P_ChScrPalMCh4,
            BRFM_P_ChScrSIF,
        },
        {
            BRFM_P_AudioEncodingScrMonoNtsc, /* PAL-M only supports Mono */
            BRFM_P_AudioEncodingScrMonoNtsc,
            NULL,
            BRFM_P_AudioEncodingScrMonoNtsc,
            BRFM_P_AudioEncodingScrMonoNtsc,
            BRFM_P_AudioEncodingScrMonoNtsc,
            BRFM_P_AudioEncodingScrMonoNtsc,
            NULL,
            NULL,
            NULL
        },
        BRFM_P_InitScrNtscOpenCableConfig /* Pal-M uses the NTSC open cable init script with some overrides */
    },
    {
        BRFM_ModulationType_ePalN,
        {
            BRFM_P_ChScrNtscOpenCableBaseband,
            BRFM_P_ChScrPalNCh3, /* Pal-N uses the NTSC open cable scripts with some overrides */
            BRFM_P_ChScrPalNCh4,
            BRFM_P_ChScrSIF,
        },
        {
            BRFM_P_AudioEncodingScrMonoNtsc, /* PAL-N only supports Mono */
            BRFM_P_AudioEncodingScrMonoNtsc,
            NULL,
            BRFM_P_AudioEncodingScrMonoNtsc,
            BRFM_P_AudioEncodingScrMonoNtsc,
            BRFM_P_AudioEncodingScrMonoNtsc,
            BRFM_P_AudioEncodingScrMonoNtsc,
            NULL,
            NULL,
            NULL
        },
        BRFM_P_InitScrPalNConfig
    }
};

const BRFM_P_ModulationInfo *BRFM_P_GetModInfoPtr(
    BRFM_ModulationType modType /* [in] Requested modulation type */
    )
{
    unsigned idx;
    const BRFM_P_ModulationInfo *retVal = NULL;

    for (idx=0; idx<(sizeof(BRFM_P_ModInfo)/sizeof(BRFM_P_ModulationInfo)); idx++) {
        if (BRFM_P_ModInfo[idx].modType == modType) {
            retVal = &BRFM_P_ModInfo[idx];
            break;
        }
    }
    return retVal;
}

#endif /* #ifndef VEC_GRPD_RFM */
