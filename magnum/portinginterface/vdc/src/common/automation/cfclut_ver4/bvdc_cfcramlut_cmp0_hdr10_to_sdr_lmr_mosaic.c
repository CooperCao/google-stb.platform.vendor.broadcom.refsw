 /******************************************************************************
  *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
  *
  *  This program is the proprietary software of Broadcom and/or its licensors,
  *  and may only be used, duplicated, modified or distributed pursuant to the terms and
  *  conditions of a separate, written license agreement executed between you and Broadcom
  *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
  *  no license (express or implied), right to use, or waiver of any kind with respect to the
  *  Software, and Broadcom expressly reserves all rights in and to the Software and all
  *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
  *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
  *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  *
  *  Except as expressly set forth in the Authorized License,
  *
  *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
  *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
  *  and to use this information only in connection with your use of Broadcom integrated circuit products.
  *
  *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
  *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
  *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
  *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
  *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
  *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
  *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
  *  USE OR PERFORMANCE OF THE SOFTWARE.
  *
  *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
  *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
  *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
  *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
  *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
  *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
  *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
  *  ANY LIMITED REMEDY.
  ******************************************************************************/

/* CMP0 V0 2084_to_1886_lmr_mosaic */

/* sel_nl2l = PQ, it is in ROM */

/* sel_l2nl = BT1886, it is in ROM */

static const uint32_t s_aulRamLut_CMP0_LMR_2084_to_1886_lmr_mosaic[] =
{
    0x000ef312, /*    0:     0.934343338012695312500000000 */
    0x000ef312, /*    1:     0.934343338012695312500000000 */
    0x000ef312, /*    2:     0.934343338012695312500000000 */
    0x000ef312, /*    3:     0.934343338012695312500000000 */
    0x000ed738, /*    4:     0.927543640136718750000000000 */
    0x000e86a4, /*    5:     0.907871246337890625000000000 */
    0x000e2082, /*    6:     0.882936477661132812500000000 */
    0x000db3b6, /*    7:     0.856374740600585937500000000 */
    0x000d472c, /*    8:     0.829875946044921875000000000 */
    0x000cde1c, /*    9:     0.804225921630859375000000000 */
    0x000c79f4, /*   10:     0.779773712158203125000000000 */
    0x000bc1f4, /*   11:     0.734851837158203125000000000 */
    0x000b1f05, /*   12:     0.695073127746582031250000000 */
    0x000a8ecb, /*   13:     0.659861564636230468750000000 */
    0x000a0e9c, /*   14:     0.628566741943359375000000000 */
    0x00099c12, /*   15:     0.600603103637695312500000000 */
    0x00093522, /*   16:     0.575471878051757812500000000 */
    0x0008d81a, /*   17:     0.552759170532226562500000000 */
    0x00088394, /*   18:     0.532123565673828125000000000 */
    0x00083669, /*   19:     0.513283729553222656250000000 */
    0x0007efa2, /*   20:     0.496004104614257812500000000 */
    0x00077233, /*   21:     0.465380668640136718750000000 */
    0x00070648, /*   22:     0.439033508300781250000000000 */
    0x0006a847, /*   23:     0.416083335876464843750000000 */
    0x00065584, /*   24:     0.395877838134765625000000000 */
    0x00060bfd, /*   25:     0.377926826477050781250000000 */
    0x0005ca26, /*   26:     0.361852645874023437500000000 */
    0x00058ec8, /*   27:     0.347358703613281250000000000 */
    0x000558ec, /*   28:     0.334209442138671875000000000 */
    0x000527cb, /*   29:     0.322215080261230468750000000 */
    0x0004fac3, /*   30:     0.311221122741699218750000000 */
    0x0004d14e, /*   31:     0.301099777221679687500000000 */
    0x0004aafd, /*   32:     0.291745185852050781250000000 */
    0x00048773, /*   33:     0.283068656921386718750000000 */
    0x0004665f, /*   34:     0.274992942810058593750000000 */
    0x00044780, /*   35:     0.267456054687500000000000000 */
    0x00042a9a, /*   36:     0.260400772094726562500000000 */
    0x00040f7c, /*   37:     0.253780364990234375000000000 */
    0x00040000, /*   38:     0.250000000000000000000000000 */
    0x00040000, /*   39:     0.250000000000000000000000000 */
    0x00040000, /*   40:     0.250000000000000000000000000 */
    0x00040000, /*   41:     0.250000000000000000000000000 */
    0x00040000, /*   42:     0.250000000000000000000000000 */
    0x00040000, /*   43:     0.250000000000000000000000000 */
    0x00040000, /*   44:     0.250000000000000000000000000 */
    0x00040000, /*   45:     0.250000000000000000000000000 */
    0x00040000, /*   46:     0.250000000000000000000000000 */
    0x00040000, /*   47:     0.250000000000000000000000000 */
    0x00040000, /*   48:     0.250000000000000000000000000 */
    0x00040000, /*   49:     0.250000000000000000000000000 */
    0x00040000, /*   50:     0.250000000000000000000000000 */
    0x00040000, /*   51:     0.250000000000000000000000000 */
    0x00040000  /*   52:     0.250000000000000000000000000 */
};

static const BVDC_P_RamLut s_RamLutCtrl_CMP0_LMR_2084_to_1886_lmr_mosaic =
{
    5, 		/* numSeg */
    0x4, 	/* U1.2 xscl = 1.00000 */
    0, 		/* outIntBits */
    20, 		/* outFracBits */
    {      2,    10,    20,    44,    52,     0,     0,     0}, /* segEnd */
    {      0,     4,     6,     8,     1,     0,     0,     0}, /* segOffset */
    {      9,    10,     9,     8,     3,     0,     0,     0}, /* segIntBits */
    &s_aulRamLut_CMP0_LMR_2084_to_1886_lmr_mosaic[0]
};

static const uint32_t s_aulLmrAdj_CMP0_2084_to_1886_lmr_mosaic[] =
{
    0x400, /* S7.8 LMA.A =    4.000000000000000000000000000 */
    0x0, /* S7.8 LMA.B =    0.000000000000000000000000000 */
    0x0  /* S9.15 LMA.C =    0.000000000000000000000000000 */
};

static const BVDC_P_LRangeAdjTable s_LRangeAdj_CMP0_2084_to_1886_lmr_mosaic = BVDC_P_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.624969,    4,   /* PWL 0, 0x00000000, 0x00000000, 0x00004fff, 0x00000004 */
    0.100006, 1.000000, 0.000000,    0,   /* PWL 1, 0x00000ccd, 0x00008000, 0x00000000, 0x00000000 */
    1.000000, 1.000000, 0.000000,    0,   /* PWL 2, 0x00008000, 0x00008000, 0x00000000, 0x00000000 */
    1.000000, 1.000000, 0.000000,    0,   /* PWL 3, 0x00008000, 0x00008000, 0x00000000, 0x00000000 */
    1.000000, 1.000000, 0.000000,    0,   /* PWL 4, 0x00008000, 0x00008000, 0x00000000, 0x00000000 */
    1.000000, 1.000000, 0.500000,    1,   /* PWL 5, 0x00008000, 0x00008000, 0x00004000, 0x00000001 */
    0.000000, 0.000000, 0.000000,    0,   /* PWL 6, 0x00000000, 0x00000000, 0x00000000, 0x00000000 */
    0.000000, 0.000000, 0.000000,    0    /* PWL 7, 0x00000000, 0x00000000, 0x00000000, 0x00000000 */
);


/* end of file */
