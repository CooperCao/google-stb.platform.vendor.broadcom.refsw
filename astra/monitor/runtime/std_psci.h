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

#ifndef _STD_PSCI_H_
#define _STD_PSCI_H_

#define STD_SVC_PSCI_VERSION_MAJOR      0
#define STD_SVC_PSCI_VERSION_MINOR      2

/* Standard service PSCI module bit[15:5] and func bit[4:0] */
#define STD_SVC_PSCI_MOD_BITS           0x000

#define STD_SVC_PSCI_MOD_SHIFT          5
#define STD_SVC_PSCI_MOD_MASK           0x7FF

#define STD_SVC_PSCI_FUNC_SHIFT         0
#define STD_SVC_PSCI_FUNC_MASK          0x1F

#define STD_SVC_PSCI_MOD(fx)            (((fx) >> STD_SVC_PSCI_MOD_SHIFT) & \
                                         STD_SVC_PSCI_MOD_MASK)
#define STD_SVC_PSCI_FUNC(fx)           (((fx) >> STD_SVC_PSCI_FUNC_SHIFT) & \
                                         STD_SVC_PSCI_FUNC_MASK)

/* Standard service PSCI functions */
#define STD_SVC_PSCI_VERSION            0x00
#define STD_SVC_PSCI_CPU_SUSPEND        0x01
#define STD_SVC_PSCI_CPU_OFF            0x02
#define STD_SVC_PSCI_CPU_ON             0x03
#define STD_SVC_PSCI_AFFINITY_INFO      0x04
#define STD_SVC_PSCI_MIGRATE            0x05
#define STD_SVC_PSCI_MIGRATE_INFO_TYPE  0x06
#define STD_SVC_PSCI_MIGRATE_INFO_UP_CPU 0x07
#define STD_SVC_PSCI_SYSTEM_OFF         0x08
#define STD_SVC_PSCI_SYSTEM_RESET       0x09
#define STD_SVC_PSCI_PSCI_FEATURES      0x0A
#define STD_SVC_PSCI_CPU_FREEZE         0x0B
#define STD_SVC_PSCI_CPU_DEFAULT_SUSPEND 0x0C
#define STD_SVC_PSCI_NODE_HW_STATE      0x0D
#define STD_SVC_PSCI_SYSTEM_SUSPEND     0x0E
#define STD_SVC_PSCI_SET_SUSPEND_MODE   0x0F
#define STD_SVC_PSCI_STAT_RESIDENCY     0x10
#define STD_SVC_PSCI_STAT_COUNT         0x11
#define STD_SVC_PSCI_MAX                0x12

/* Standard service PSCI return error codes */
#define STD_SVC_PSCI_SUCCESS             0
#define STD_SVC_PSCI_NOT_SUPPORTED      -1
#define STD_SVC_PSCI_INVALID_PARAMETERS -2
#define STD_SVC_PSCI_DENIED             -3
#define STB_SVC_PSCI_ALREADY_ON         -4
#define STB_SVC_PSCI_ON_PENDING         -5
#define STB_SVC_PSCI_INTERNAL_FAILURE   -6
#define STB_SVC_PSCI_NOT_PRESENT        -7
#define STB_SVC_PSCI_DISABLED           -8
#define STB_SVC_PSCI_INVALID_ADDRESS    -9

#endif /* _STD_PSCI_H_ */
