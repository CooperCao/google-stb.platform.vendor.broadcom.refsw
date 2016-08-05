/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 *****************************************************************************/
#ifndef BCHP_MEMC_OFFSETS_PRIV_H__
#define BCHP_MEMC_OFFSETS_PRIV_H__

/* Table of Physical addresses per memory controller per SOC. */

/* All SOCs have the same MEMC_0 offset. */
#define BCHP_P_MEMC_0_OFFSET               0x00000000

#if BCHP_CHIP == 7422 || BCHP_CHIP == 7425 || BCHP_CHIP == 7435
#define BCHP_P_MEMC_1_OFFSET               0x90000000
#endif

#if ((BCHP_CHIP == 7366) && (BCHP_VER >= BCHP_VER_B0))
/* MEMC0 addr space: 0x0000_0000 ~ 0x7FFF_FFFF */
/* MEMC1 addr space: 0x8000_0000 ~ 0xbFFF_FFFF */
#define BCHP_P_MEMC_1_OFFSET               0x80000000
#endif

#if ((BCHP_CHIP == 7439) && (BCHP_VER >= BCHP_VER_B0))
/* MEMC0 addr space: 0x0000_0000 ~ 0x7FFF_FFFF */
/* MEMC1 addr space: 0x8000_0000 ~ 0xbFFF_FFFF */
#define BCHP_P_MEMC_1_OFFSET               0x80000000
#endif

#if BCHP_CHIP == 7445
/* MEMC0 addr space: 0x0000_0000 ~ 0x3FFF_FFFF */
/* MEMC1 addr space: 0x4000_0000 ~ 0x7FFF_FFFF */
/* MEMC2 addr space: 0x8000_0000 ~ 0xbFFF_FFFF */
#define BCHP_P_MEMC_1_OFFSET               0x40000000
#define BCHP_P_MEMC_2_OFFSET               0x80000000
#endif

#endif /* BCHP_MEMC_OFFSETS_PRIV_H__ */
