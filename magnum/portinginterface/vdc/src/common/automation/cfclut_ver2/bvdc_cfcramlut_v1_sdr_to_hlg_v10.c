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

/* CMP0 V1 1886_to_1886_v10 */

/* sel_nl2l = BT1886, it is in ROM */

/* sel_l2nl = BT1886, it is in ROM */

/* LMR is bypassed */

static const BVDC_P_LRangeAdjTable s_LRangeAdj_PIP_1886_to_hlg_v10 = BVDC_P_MAKE_LR_ADJ
(   8, /* number of PWL points */
    /*     x         y         m     e              (U0.15)x,   (U0.15)y,   (S0.15)m,   (S4)e   */
    0.000000, 0.000000, 0.758606,    0,   /* PWL 0, 0x00000000, 0x00000000, 0x0000611a, 0x00000000 */
    0.200012, 0.151733, 0.732910,    0,   /* PWL 1, 0x0000199a, 0x0000136c, 0x00005dd0, 0x00000000 */
    0.299988, 0.225006, 0.590790,    0,   /* PWL 2, 0x00002666, 0x00001ccd, 0x00004b9f, 0x00000000 */
    0.399994, 0.284088, 0.974060,   -1,   /* PWL 3, 0x00003333, 0x0000245d, 0x00007cae, 0xffffffff */
    0.500000, 0.332794, 0.807739,   -1,   /* PWL 4, 0x00004000, 0x00002a99, 0x00006764, 0xffffffff */
    0.649994, 0.393372, 0.675476,   -1,   /* PWL 5, 0x00005333, 0x0000325a, 0x00005676, 0xffffffff */
    0.799988, 0.444031, 0.571259,   -1,   /* PWL 6, 0x00006666, 0x000038d6, 0x0000491f, 0xffffffff */
    1.000000, 0.501160, 0.000000,    0    /* PWL 7, 0x00008000, 0x00004026, 0x00000000, 0x00000000 */
);


/* end of file */
