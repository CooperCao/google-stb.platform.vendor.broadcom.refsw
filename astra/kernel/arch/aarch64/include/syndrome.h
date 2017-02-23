/***************************************************************************
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
 ***************************************************************************/

#ifndef INCLUDE_SYNDROME_H
#define INCLUDE_SYNDROME_H

#define ESR_EL1_EC_SHIFT	(26)
#define ESR_EL1_IL		(1U << 25)

#define ESR_EL1_EC_UNKNOWN	(0x00)
#define ESR_EL1_EC_WFI		(0x01)
#define ESR_EL1_EC_CP15_32	(0x03)
#define ESR_EL1_EC_CP15_64	(0x04)
#define ESR_EL1_EC_CP14_MR	(0x05)
#define ESR_EL1_EC_CP14_LS	(0x06)
#define ESR_EL1_EC_FP_ASIMD	(0x07)
#define ESR_EL1_EC_CP10_ID	(0x08)
#define ESR_EL1_EC_CP14_64	(0x0C)
#define ESR_EL1_EC_ILL_ISS	(0x0E)
#define ESR_EL1_EC_SVC32	(0x11)
#define ESR_EL1_EC_SVC64	(0x15)
#define ESR_EL1_EC_SYS64	(0x18)
#define ESR_EL1_EC_IABT_EL0	(0x20)
#define ESR_EL1_EC_IABT_EL1	(0x21)
#define ESR_EL1_EC_PC_ALIGN	(0x22)
#define ESR_EL1_EC_DABT_EL0	(0x24)
#define ESR_EL1_EC_DABT_EL1	(0x25)
#define ESR_EL1_EC_SP_ALIGN	(0x26)
#define ESR_EL1_EC_FP_EXC32	(0x28)
#define ESR_EL1_EC_FP_EXC64	(0x2C)
#define ESR_EL1_EC_SERROR	(0x2F)
#define ESR_EL1_EC_BREAKPT_EL0	(0x30)
#define ESR_EL1_EC_BREAKPT_EL1	(0x31)
#define ESR_EL1_EC_SOFTSTP_EL0	(0x32)
#define ESR_EL1_EC_SOFTSTP_EL1	(0x33)
#define ESR_EL1_EC_WATCHPT_EL0	(0x34)
#define ESR_EL1_EC_WATCHPT_EL1	(0x35)
#define ESR_EL1_EC_BKPT32	(0x38)
#define ESR_EL1_EC_BRK64	(0x3C)

#endif