/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/* CMP0 2084_to_1886_mosaic */

/* sel_nl2l = PQ, it is in ROM */

/* sel_l2nl = BT1886, it is in ROM */

/* LMR is bypassed */

static const BVDC_P_LRangeAdjTable s_LRangeAdj_CMP0_2084_to_1886_mosaic = BVDC_P_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.584351,    6,   /* PWL 0, 0x00000000, 0x00000000, 0x00004acc, 0x00000006 */
    0.005066, 0.189453, 0.576569,    5,   /* PWL 1, 0x000000a6, 0x00001840, 0x000049cd, 0x00000005 */
    0.024994, 0.557129, 0.582153,    4,   /* PWL 2, 0x00000333, 0x00004750, 0x00004a84, 0x00000004 */
    0.037506, 0.673676, 0.891815,    3,   /* PWL 3, 0x000004cd, 0x0000563b, 0x00007227, 0x00000003 */
    0.049988, 0.762726, 0.727142,    3,   /* PWL 4, 0x00000666, 0x000061a1, 0x00005d13, 0x00000003 */
    0.062500, 0.835510, 0.581207,    3,   /* PWL 5, 0x00000800, 0x00006af2, 0x00004a65, 0x00000003 */
    0.087494, 0.951721, 0.964630,    2,   /* PWL 6, 0x00000b33, 0x000079d2, 0x00007b79, 0x00000002 */
    0.100006, 1.000000, 0.000000,    0    /* PWL 7, 0x00000ccd, 0x00008000, 0x00000000, 0x00000000 */
);


/* end of file */
