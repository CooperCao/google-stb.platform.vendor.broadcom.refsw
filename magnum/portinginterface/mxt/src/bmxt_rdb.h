/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#ifndef BMXT_RDB__
#define BMXT_RDB__

/* special case */
const unsigned BMXT_NUMELEM_NULL[]  = { 0,  0,  0,  0,  0,  0, };
/* 3128- family */
const unsigned BMXT_NUMELEM_3128[]  = {16,  9,  2,  0,  1,  0, }; /* for Cx */
const unsigned BMXT_NUMELEM_3383[]  = {16,  9,  8,  0,  1,  0, }; /* for Bx. Ax has fewer resources */
const unsigned BMXT_NUMELEM_4528[]  = {16, 16,  8,  1,  2,  0, }; /* for Bx. Ax has fewer resources */
const unsigned BMXT_NUMELEM_3472[]  = {16, 16,  8,  1,  2,  0, };
/* 4538-family */
const unsigned BMXT_NUMELEM_4538[]  = {16, 32, 10,  1,  2, 10, }; /* for Cx. Bx has fewer resources */
const unsigned BMXT_NUMELEM_3384[]  = {18, 32, 16,  1,  2, 0, };
const unsigned BMXT_NUMELEM_7366[]  = {26, 32, 16,  1,  2, 24, }; /* for Bx. Ax has fewer resources */
const unsigned BMXT_NUMELEM_7145[]  = {26, 32, 16,  1,  2, 24, }; /* for Bx. Ax has NO DEMOD_XPT_FE */
const unsigned BMXT_NUMELEM_45216[] = {26, 32, 16,  1,  2, 24, };
/* 45308-family */
const unsigned BMXT_NUMELEM_45308[] = {26, 32, 16,  1,  3, 24, };
const unsigned BMXT_NUMELEM_45402[] = { 4,  4,  2,  0,  1,  0, };

/* 3158-family */
const unsigned BMXT_NUMELEM_3158[]  = {26, 32, 16,  1,  2, 24, };

const unsigned BMXT_STEPSIZE_3128[]  = { 8, 16, 16,  8, 16,  0, }; /* taken from 4528 */
const unsigned BMXT_STEPSIZE_4538[]  = { 8,  8, 16, 80, 80,  8, };
const unsigned BMXT_STEPSIZE_45308[] = { 8, 12, 16, 80, 80,  8, };
const unsigned BMXT_STEPSIZE_3158[]  = {12,  8, 16, 80, 80,  8, };

const uint32_t BMXT_REGOFFSETS_3128[] = { /* taken from 4528 */
0x00000000, 0x00000020, 0x00000030, 0x00000280, 0x00000010, 0x00000014, 0xffffffff, 0x00000018,
0x0000001c, 0xffffffff, 0x00000480, 0x00000484, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff, 0xffffffff, 0x00000600, 0x00000604, 0x00000608, 0x00000504, 0xffffffff, \
0x00000100, 0xffffffff, 0x00000104, 0x00000180, 0x00000184, 0xffffffff, 0x00000580, 0x00000400, \
0x00000404, 0x00000408, 0x0000040c, 0x00000680, 0x0000068c, 0xffffffff, 0xffffffff, 0xffffffff, \
0x000006c0, 0x000006c4, 0x000006c8, 0x000006cc, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000800, 0x00000c00, \
};

const uint32_t BMXT_REGOFFSETS_4538[] = { \
0x00000000, 0x00000004, 0x00000008, 0x0000000c, 0x00000010, 0x00000014, 0x00000018, 0x00000024, \
0x00000028, 0x0000002c, 0x00000038, 0x0000003c, 0x00000040, 0x00000048, 0x00000050, 0x00000058, \
0x00000060, 0x00000068, 0x00000070, 0x00000078, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0x00000080, 0x00000088, 0x0000008c, 0x000000a0, 0x000000a4, 0x000000a8, 0x000000b0, 0x000000d4, \
0x00000100, 0xffffffff, 0x00000104, 0x00000200, 0x00000204, 0xffffffff, 0x00000300, 0x00000320, \
0x00000324, 0x00000328, 0x0000032c, 0x00000420, 0x00000424, 0x00000428, 0x0000042c, 0x0000046c, \
0x00000560, 0x00000564, 0x00000568, 0x0000056c, 0x00000570, 0x00000600, 0x00000604, 0x00000700, \
0x00000704, 0x00000708, 0x0000070c, 0x00000710, 0x00000800, 0x00001000, \
};

const uint32_t BMXT_REGOFFSETS_45308[] = { \
0x00000000, 0x00000004, 0x00000008, 0x0000000c, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000010, 0x00000014, 0x00000018, 0x0000001c, \
0x00000080, 0x00000088, 0x0000008c, 0x000000a0, 0x000000a4, 0x000000a8, 0x000000b0, 0x000000d4, \
0x00000100, 0xffffffff, 0x00000104, 0x00000200, 0x00000204, 0x00000208, 0x00000380, 0x000003a0, \
0x000003a4, 0x000003a8, 0x000003ac, 0x000004a0, 0x000004a4, 0x000004a8, 0x000004ac, 0x000004ec, \
0x000005e0, 0x000005e4, 0x000005e8, 0x000005ec, 0x000005f0, 0x00000720, 0x00000724, 0x00000820, \
0x00000824, 0x00000828, 0x0000082c, 0x00000830, 0x00002000, 0x00003000, \
};

const uint32_t BMXT_REGOFFSETS_3158[] = { \
0x00000000, 0x00000004, 0x00000008, 0x0000000c, 0x00000010, 0x00000014, 0x00000018, 0x00000024, \
0x00000028, 0x0000002c, 0x00000038, 0x0000003c, 0x00000040, 0x00000048, 0x00000050, 0x00000058, \
0x00000060, 0x00000068, 0x00000070, 0x00000078, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, \
0x00000080, 0x00000088, 0x0000008c, 0x000000a0, 0x000000a4, 0x000000a8, 0x000000b0, 0x000000d4, \
0x00000100, 0x00000104, 0x00000108, 0x00000280, 0x00000284, 0xffffffff, 0x00000380, 0x00000400, \
0x00000404, 0x00000408, 0x0000040c, 0x00000500, 0x00000504, 0x00000508, 0x0000050c, 0x0000054c, \
0x00000560, 0x00000564, 0x00000568, 0x0000056c, 0x00000570, 0x00000600, 0x00000604, 0x00000700, \
0x00000704, 0x00000708, 0x0000070c, 0x00000710, 0x00000800, 0x00001000, \
};

const uint32_t BMXT_REGBASE_3128  = 0x000c5000;
const uint32_t BMXT_REGBASE_3383  = 0x00000000; /* not needed since RPC-based platform */
const uint32_t BMXT_REGBASE_4528  = 0x00095000;
const uint32_t BMXT_REGBASE_3472  = 0x00105000;

const uint32_t BMXT_REGBASE_4538  = 0x00096000;
const uint32_t BMXT_REGBASE_3384  = 0x00000000; /* not needed since RPC-based platform */
const uint32_t BMXT_REGBASE_7366  = 0x01272000;
const uint32_t BMXT_REGBASE_7145  = 0x00000000; /* 0x20a82000; TODO: not certain if this is an RPC-based platform */
const uint32_t BMXT_REGBASE_45216 = 0x06b02000;

const uint32_t BMXT_REGBASE_45308 = 0x07100000;
const uint32_t BMXT_REGBASE_3158  = 0x04402000;
const uint32_t BMXT_REGBASE_3466  = 0x04b02000;

#endif
