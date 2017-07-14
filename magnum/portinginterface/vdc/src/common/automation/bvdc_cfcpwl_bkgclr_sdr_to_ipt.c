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

/* SW conv bkg colr sdr_to_ipt */
#if 0 /* duplicate */
static const BVDC_P_NL_PwlSegment s_Nl2l_Pwl_1886 = BVDC_P_MAKE_NL_PWL
(   8, /* number of PWL points */
    /*     x         y         m     e */
    0.000000, 0.000000, 0.000000,    0,
    0.170000, 0.014226, 0.669451,   -3,
    0.320000, 0.064917, 0.675885,   -1,
    0.465000, 0.159179, 0.650084,    0,
    0.610000, 0.305346, 0.504021,    1,
    0.740000, 0.485463, 0.692759,    1,
    0.870000, 0.715890, 0.886257,    1,
    1.000000, 1.000000, 0.546366,    2
);
#endif
static const BVDC_P_NL_PwlSegments s_L2nl_Pwl_2084 = BVDC_P_MAKE_NL_PWL
(   8, /* number of PWL points */
    /*     x         y         m     e */
    0.000000, 0.000001, 0.000000,    0,
    0.001000, 0.299698, 0.585354,    9,
    0.003000, 0.392877, 0.727947,    6,
    0.007000, 0.472733, 0.623874,    5,
    0.013000, 0.534636, 0.644830,    4,
    0.023000, 0.593779, 0.739279,    3,
    0.030000, 0.621863, 0.501505,    3,
    0.050000, 0.676585, 0.684025,    2
);

/* LMR is bypassed */

/* rng_adj is bypassed */

/* end of file */