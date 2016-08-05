/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BMXT_RDB_DCBG__
#define BMXT_RDB_DCBG__

/* REGBASE and REGOFFSETS are relative to DEMOD_XPT_FE_DCB_DEBUG0 */

const unsigned BMXT_NUMELEM_DCBG_45308[]  = {32, 32, 28, 32, 32, 32,  8, 32, 32,  8, 32, };
const unsigned BMXT_STEPSIZE_DCBG_45308[] = {16, 28,  8,  8, 12, 36, 32,  8,  8,  8,  8, };

const uint32_t BMXT_REGOFFSETS_DCBG_45308[] = { \
0x00000000, 0x00000050, 0x000000d0, 0x000000d4, 0x000000e0, 0x000000e4, 0x000000e8, 0x000000ec, \
0x000002e0, 0x000002e4, 0x000002e8, 0x000002ec, 0x000002f0, 0x000002f4, 0x000002f8, 0x00000660, \
0x00000664, 0x00000668, 0x0000066c, 0x00000670, 0x00000750, 0x00000754, 0x00000758, 0x0000075c, \
0x00000760, 0x00000764, 0x00000768, 0x0000076c, 0x00000770, 0x00000774, 0x00000778, 0x0000077c, \
0x00000780, 0x00000784, 0x00000788, 0x0000078c, 0x00000790, 0x00000810, 0x00000890, 0x00000910, \
0x00000990, 0x00000a10, 0x00000a90, 0x00000a94, 0x00000b90, 0x00000b94, 0x00000b98, 0x00000d10, \
0x00000d14, 0x00000d18, 0x00000d1c, 0x00000d20, 0x00000d24, 0x00000d28, 0x00000d2c, 0x00000d30, \
0x00001190, 0x00001194, 0x00001198, 0x0000119c, 0x000011a0, 0x000011a4, 0x000011a8, 0x000011ac, \
0x00001290, 0x00001294, 0x00001390, 0x00001394, 0x00001490, 0x00001494, 0x00001650, 0x00001654, \
};

const uint32_t BMXT_REGBASE_DCBG_45308 = 0x071008b0;

#endif
