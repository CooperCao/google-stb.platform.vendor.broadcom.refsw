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

/* CMP0 V0 1886_to_2084_lmr_mosaic */

/* sel_nl2l = BT1886, it is in ROM */

/* sel_l2nl = PQ, it is in ROM */

static const uint32_t s_aulRamLut_CMP0_LMR_1886_to_2084_lmr_mosaic[] =
{
    0x00000000, /*    0:     0.000000000000000000000000000 */
    0x0003bb6d, /*    1:     0.233258247375488281250000000 */
    0x0004241c, /*    2:     0.258815765380859375000000000 */
    0x00046697, /*    3:     0.275046348571777343750000000 */
    0x00049844, /*    4:     0.287174224853515625000000000 */
    0x0004c04e, /*    5:     0.296949386596679687500000000 */
    0x0004e207, /*    6:     0.305182456970214843750000000 */
    0x0004ff45, /*    7:     0.312321662902832031250000000 */
    0x00051926, /*    8:     0.318639755249023437500000000 */
    0x00056afe, /*    9:     0.338621139526367187500000000 */
    0x0005a828, /*   10:     0.353553771972656250000000000 */
    0x0005d972, /*   11:     0.365587234497070312500000000 */
    0x000602f7, /*   12:     0.375723838806152343750000000 */
    0x000626f7, /*   13:     0.384512901306152343750000000 */
    0x000646d4, /*   14:     0.392292022705078125000000000 */
    0x0006f6e3, /*   15:     0.435275077819824218750000000 */
    0x000766b0, /*   16:     0.462570190429687500000000000 */
    0x0007ba3d, /*   17:     0.482968330383300781250000000 */
    0x0007fd93, /*   18:     0.499407768249511718750000000 */
    0x0008364a, /*   19:     0.513254165649414062500000000 */
    0x00086777, /*   20:     0.525259971618652343750000000 */
    0x000892fe, /*   21:     0.535886764526367187500000000 */
    0x0008ba1e, /*   22:     0.545438766479492187500000000 */
    0x0008ddb5, /*   23:     0.554127693176269531250000000 */
    0x0008fe63, /*   24:     0.562106132507324218750000000 */
    0x00091ca2, /*   25:     0.569490432739257812500000000 */
    0x000938cf, /*   26:     0.576369285583496093750000000 */
    0x00095333, /*   27:     0.582812309265136718750000000 */
    0x00096c08, /*   28:     0.588874816894531250000000000 */
    0x0009837f, /*   29:     0.594603538513183593750000000 */
    0x000a8e5a, /*   30:     0.659753799438476562500000000 */
    0x000b37cf, /*   31:     0.701125144958496093750000000 */
    0x000bb673, /*   32:     0.732043266296386718750000000 */
    0x000c1c82, /*   33:     0.756959915161132812500000000 */
    0x000c7279, /*   34:     0.777947425842285156250000000 */
    0x000cbd03, /*   35:     0.796145439147949218750000000 */
    0x000cfefc, /*   36:     0.812252044677734375000000000 */
    0x000d3a4a, /*   37:     0.826730728149414062500000000 */
    0x000d703b, /*   38:     0.839900016784667968750000000 */
    0x000da1c4, /*   39:     0.851993560791015625000000000 */
    0x000dcf9d, /*   40:     0.863186836242675781250000000 */
    0x000dfa51, /*   41:     0.873612403869628906250000000 */
    0x000e2251, /*   42:     0.883378028869628906250000000 */
    0x000e47f5, /*   43:     0.892567634582519531250000000 */
    0x000e6b86, /*   44:     0.901250839233398437500000000 */
    0x000e8d3f, /*   45:     0.909483909606933593750000000 */
    0x000ead52, /*   46:     0.917314529418945312500000000 */
    0x000ecbeb, /*   47:     0.924784660339355468750000000 */
    0x000ee92d, /*   48:     0.931927680969238281250000000 */
    0x000f0536, /*   49:     0.938772201538085937500000000 */
    0x000f2023, /*   50:     0.945345878601074218750000000 */
    0x000f3a0b, /*   51:     0.951670646667480468750000000 */
    0x000f5302, /*   52:     0.957765579223632812500000000 */
    0x000f6b1a, /*   53:     0.963647842407226562500000000 */
    0x000f8265, /*   54:     0.969334602355957031250000000 */
    0x000f98ef, /*   55:     0.974837303161621093750000000 */
    0x000faec6, /*   56:     0.980169296264648437500000000 */
    0x000fc3f6, /*   57:     0.985342025756835937500000000 */
    0x000fd88a, /*   58:     0.990365982055664062500000000 */
    0x000fec8a, /*   59:     0.995248794555664062500000000 */
    0x000fffff, /*   60:     0.999999046325683593750000000 */
    0x000fffff  /*   61:     0.999999046325683593750000000 */
};

static const BVDC_P_RamLut s_RamLutCtrl_CMP0_LMR_1886_to_2084_lmr_mosaic =
{
    4, 		/* numSeg */
    0x4, 	/* U1.2 xscl = 1.00000 */
    0, 		/* outIntBits */
    20, 		/* outFracBits */
    {      8,    14,    29,    61,     0,     0,     0,     0}, /* segEnd */
    {      0,     2,     1,     1,     0,     0,     0,     0}, /* segOffset */
    {     14,    12,     9,     5,     0,     0,     0,     0}, /* segIntBits */
    &s_aulRamLut_CMP0_LMR_1886_to_2084_lmr_mosaic[0]
};

static const uint32_t s_aulLmrAdj_CMP0_1886_to_2084_lmr_mosaic[] =
{
    0x100, /* S7.8 LMA.A =    1.000000000000000000000000000 */
    0x0, /* S7.8 LMA.B =    0.000000000000000000000000000 */
    0x0  /* S9.15 LMA.C =    0.000000000000000000000000000 */
};

static const BVDC_P_LRangeAdjTable s_LRangeAdj_CMP0_1886_to_2084_lmr_mosaic = BVDC_P_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.799805,   -4,   /* PWL 0, 0x00000000, 0x00000000, 0x00006660, 0xfffffffc */
    1.000000, 0.049988, 0.000000,    0,   /* PWL 1, 0x00008000, 0x00000666, 0x00000000, 0x00000000 */
    1.000000, 0.049988, 0.000000,    0,   /* PWL 2, 0x00008000, 0x00000666, 0x00000000, 0x00000000 */
    1.000000, 0.049988, 0.000000,    0,   /* PWL 3, 0x00008000, 0x00000666, 0x00000000, 0x00000000 */
    1.000000, 0.049988, 0.000000,    0,   /* PWL 4, 0x00008000, 0x00000666, 0x00000000, 0x00000000 */
    1.000000, 0.049988, 0.000000,    0,   /* PWL 5, 0x00008000, 0x00000666, 0x00000000, 0x00000000 */
    1.000000, 0.049988, 0.000000,    0,   /* PWL 6, 0x00008000, 0x00000666, 0x00000000, 0x00000000 */
    1.000000, 0.049988, 0.000000,    0    /* PWL 7, 0x00008000, 0x00000666, 0x00000000, 0x00000000 */
);


/* end of file */
