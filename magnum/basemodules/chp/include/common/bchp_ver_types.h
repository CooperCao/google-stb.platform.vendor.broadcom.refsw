/******************************************************************************
 *    (c)2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/
#ifndef BCHP_VER_TYPES_H__
#define BCHP_VER_TYPES_H__

/**
Summary:
BCHP_VER_XX values are used with the BCHP_VER and BCHP_<<secondary_chip>>_VER macros
to provide version-specific software support.

Description:
Every magnum software build must have BCHP_VER defined to one of the following
values. This determines the version of chip the software is intended to run on.

Magnum has limited support for binary compatibility for other minor versions
of the same chip. In most cases this is made possible by backward compatible
hardware interfaces. In a small number of cases, either compile time or
run time chip version tests are used to provide binary compatibility. See magnum
architecture documentation for more details.
**/
#define BCHP_VER_A0 (0x00000000)
#define BCHP_VER_A1 (0x00000001)
#define BCHP_VER_A2 (0x00000002)
#define BCHP_VER_A3 (0x00000003)
#define BCHP_VER_A4 (0x00000004)
#define BCHP_VER_A5 (0x00000005)

#define BCHP_VER_B0 (0x00010000)
#define BCHP_VER_B1 (0x00010001)
#define BCHP_VER_B2 (0x00010002)
#define BCHP_VER_B3 (0x00010003)
#define BCHP_VER_B4 (0x00010004)
#define BCHP_VER_B5 (0x00010005)

#define BCHP_VER_C0 (0x00020000)
#define BCHP_VER_C1 (0x00020001)
#define BCHP_VER_C2 (0x00020002)
#define BCHP_VER_C3 (0x00020003)
#define BCHP_VER_C4 (0x00020004)
#define BCHP_VER_C5 (0x00020005)

#define BCHP_VER_D0 (0x00030000)
#define BCHP_VER_D1 (0x00030001)
#define BCHP_VER_D2 (0x00030002)
#define BCHP_VER_D3 (0x00030003)
#define BCHP_VER_D4 (0x00030004)
#define BCHP_VER_D5 (0x00030005)

#define BCHP_VER_E0 (0x00040000)
#define BCHP_VER_E1 (0x00040001)
#define BCHP_VER_E2 (0x00040002)
#define BCHP_VER_E3 (0x00040003)
#define BCHP_VER_E4 (0x00040004)
#define BCHP_VER_E5 (0x00040005)

#endif
