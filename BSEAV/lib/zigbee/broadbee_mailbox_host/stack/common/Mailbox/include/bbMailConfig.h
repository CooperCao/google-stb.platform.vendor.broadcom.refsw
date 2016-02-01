/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/Mailbox/include/bbMailConfig.h $
*
* DESCRIPTION:
*       The mailbox configuration constants.
*
* $Revision: 3591 $
* $Date: 2014-09-16 07:32:27Z $
*
*****************************************************************************************/
#include "bbRF4CEConfig.h"

#ifndef _MAIL_CONFIG_H
#define _MAIL_CONFIG_H

/************************* DEFINITIONS **************************************************/
#undef MAILBOX_STACK_SIDE
#if !defined(_TEST_HARNESS_) || defined(MAILBOX_UNIT_TEST)
# define MAILBOX_STACK_SIDE
#endif

#undef MAILBOX_HOST_SIDE
#if defined(_TEST_HARNESS_) || defined(MAILBOX_UNIT_TEST)
# define MAILBOX_HOST_SIDE
#endif

#ifndef WRAPPERS_OFF
# define WRAPPERS_OFF 0
#endif

#ifndef WRAPPERS_BASE
# define WRAPPERS_BASE 1
#endif

#ifndef WRAPPERS_ALL
# define WRAPPERS_ALL 2
#endif

#ifndef _MAILBOX_WRAPPERS_TEST_ENGINE_
# define _MAILBOX_WRAPPERS_TEST_ENGINE_ WRAPPERS_OFF
#endif

#ifndef _MAILBOX_WRAPPERS_PROFILING_ENGINE_
# define _MAILBOX_WRAPPERS_PROFILING_ENGINE_ WRAPPERS_OFF
#endif

#ifndef _MAILBOX_WRAPPERS_MAC_
# define _MAILBOX_WRAPPERS_MAC_ WRAPPERS_OFF
#endif

#ifndef _MAILBOX_WRAPPERS_NWK_
# define _MAILBOX_WRAPPERS_NWK_ WRAPPERS_OFF
#endif

#ifndef _MAILBOX_WRAPPERS_APS_
# define _MAILBOX_WRAPPERS_APS_ WRAPPERS_OFF
#endif

#ifndef _MAILBOX_WRAPPERS_ZDO_
# define _MAILBOX_WRAPPERS_ZDO_ WRAPPERS_OFF
#endif

#ifndef _MAILBOX_WRAPPERS_TC_
# define _MAILBOX_WRAPPERS_TC_ WRAPPERS_OFF
#endif

#ifndef _MAILBOX_WRAPPERS_ZCL_
# define _MAILBOX_WRAPPERS_ZCL_ WRAPPERS_OFF
#endif

#ifndef FIRMWARE_STACKS
#
# define FIRMWARE_STACKS_BIT_00  0 /* reserved */
#
# if defined(_ZBPRO_)
#  define FIRMWARE_STACKS_BIT_01  1
# else /* ! _ZBPRO_ */
#  define FIRMWARE_STACKS_BIT_01  0
# endif
#
# if defined(_RF4CE_)
#  if defined(RF4CE_CONTROLLER)
#   define FIRMWARE_STACKS_BIT_02  1
#  else
#   define FIRMWARE_STACKS_BIT_02  0
#  endif
#  if defined(RF4CE_TARGET)
#   define FIRMWARE_STACKS_BIT_03  1
#  else
#   define FIRMWARE_STACKS_BIT_03  0
#  endif
# else
#  define FIRMWARE_STACKS_BIT_02  0
#  define FIRMWARE_STACKS_BIT_03  0
# endif
#
# define FIRMWARE_STACKS (                  \
        ((FIRMWARE_STACKS_BIT_00)   << 0)   \
        | ((FIRMWARE_STACKS_BIT_01) << 1)   \
        | ((FIRMWARE_STACKS_BIT_02) << 2)   \
        | ((FIRMWARE_STACKS_BIT_03) << 3))

#
#endif /* ifndef FIRMWARE_STACKS */

#ifndef FIRMWARE_LEVEL
#
# define FIRMWARE_LEVEL_BIT_00 0  // Reserved for HAL and PHY
# define FIRMWARE_LEVEL_BIT_01 0  // RESERVED for MAC-LE incomplete
# define FIRMWARE_LEVEL_BIT_02 0  // RESERVED for MAC-LE complete
#
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_MAC_)
#  define FIRMWARE_LEVEL_BIT_03 1
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_MAC_)
#   define FIRMWARE_LEVEL_BIT_04 1
#  else
#   define FIRMWARE_LEVEL_BIT_04 0
#  endif
# else
#  define FIRMWARE_LEVEL_BIT_03 0
#  define FIRMWARE_LEVEL_BIT_04 0
# endif
#
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_NWK_)
#  define FIRMWARE_LEVEL_BIT_05 1
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_NWK_)
#   define FIRMWARE_LEVEL_BIT_06 1
#  else
#   define FIRMWARE_LEVEL_BIT_06 0
#  endif
# else
#  define FIRMWARE_LEVEL_BIT_05 0
#  define FIRMWARE_LEVEL_BIT_06 0
# endif
#
# define FIRMWARE_LEVEL_BIT_07 0  // RESERVED
#
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_APS_)
#  define FIRMWARE_LEVEL_BIT_08 1
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_APS_)
#   define FIRMWARE_LEVEL_BIT_09 1
#  else
#   define FIRMWARE_LEVEL_BIT_09 0
#  endif
# else
#  define FIRMWARE_LEVEL_BIT_08 0
#  define FIRMWARE_LEVEL_BIT_09 0
# endif
#
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZDO_)
#  define FIRMWARE_LEVEL_BIT_10 1
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZDO_)
#   define FIRMWARE_LEVEL_BIT_11 1
#  else
#   define FIRMWARE_LEVEL_BIT_11 0
#  endif
# else
#  define FIRMWARE_LEVEL_BIT_10 0
#  define FIRMWARE_LEVEL_BIT_11 0
# endif
#
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_ZCL_)
#  define FIRMWARE_LEVEL_BIT_12 1
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_ZCL_)
#   define FIRMWARE_LEVEL_BIT_13 1
#  else
#   define FIRMWARE_LEVEL_BIT_13 0
#  endif
# else
#  define FIRMWARE_LEVEL_BIT_12 0
#  define FIRMWARE_LEVEL_BIT_13 0
# endif
#
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_PROFILE_)
#  define FIRMWARE_LEVEL_BIT_14 1
#  if (WRAPPERS_ALL == _MAILBOX_WRAPPERS_PROFILE_)
#   define FIRMWARE_LEVEL_BIT_15 1
#  else
#   define FIRMWARE_LEVEL_BIT_15 0
#  endif
# else
#  define FIRMWARE_LEVEL_BIT_14 0
#  define FIRMWARE_LEVEL_BIT_15 0
# endif
#
# define FIRMWARE_LEVEL (                   \
        ((FIRMWARE_LEVEL_BIT_00)   << 0)    \
        | ((FIRMWARE_LEVEL_BIT_01) << 1)    \
        | ((FIRMWARE_LEVEL_BIT_02) << 2)    \
        | ((FIRMWARE_LEVEL_BIT_03) << 3)    \
        | ((FIRMWARE_LEVEL_BIT_04) << 4)    \
        | ((FIRMWARE_LEVEL_BIT_05) << 5)    \
        | ((FIRMWARE_LEVEL_BIT_06) << 6)    \
        | ((FIRMWARE_LEVEL_BIT_07) << 7)    \
        | ((FIRMWARE_LEVEL_BIT_08) << 8)    \
        | ((FIRMWARE_LEVEL_BIT_09) << 9)    \
        | ((FIRMWARE_LEVEL_BIT_10) << 10)   \
        | ((FIRMWARE_LEVEL_BIT_11) << 11)   \
        | ((FIRMWARE_LEVEL_BIT_12) << 12)   \
        | ((FIRMWARE_LEVEL_BIT_13) << 13)   \
        | ((FIRMWARE_LEVEL_BIT_14) << 14)   \
        | ((FIRMWARE_LEVEL_BIT_15) << 15))
#
#endif /* ifndef FIRMWARE_LEVEL */


#ifndef FIRMWARE_FEATURES
# if (defined(__SoC__))
#  define FIRMWARE_FEATURES_BIT_00 1
# else
#  define FIRMWARE_FEATURES_BIT_00 0
# endif
# if (defined(RF4CE_NWK_GU_DISCOVERY)) /* Act as GU for RF4CE NWK discovery/pair */
#  define FIRMWARE_FEATURES_BIT_02 1
# else
#  define FIRMWARE_FEATURES_BIT_02 0
# endif
#
# if (WRAPPERS_OFF != _MAILBOX_WRAPPERS_TC_)
#  define FIRMWARE_FEATURES_BIT_03 1
# else
#  define FIRMWARE_FEATURES_BIT_03 0
# endif
#
# define FIRMWARE_FEATURES (                 \
        ((FIRMWARE_FEATURES_BIT_00) << 0)    \
        | ((FIRMWARE_FEATURES_BIT_02) << 2)  \
        | ((FIRMWARE_FEATURES_BIT_03) << 2))
// Already only SoC identification is used
#endif /* ifndef FIRMWARE_FEATURES*/


#ifndef FIRMWARE_PROFILES
#
// ZigBee PRO - Home Automation.
# if DEFINED_OR_ONE(USE_ZBPRO_PROFILE_ZHA)
#  define FIRMWARE_PROFILES_BIT_00 1
# else
#  define FIRMWARE_PROFILES_BIT_00 0
# endif
#
# define FIRMWARE_PROFILES_BIT_01  0  // ZigBee PRO - Smart Energy.
# define FIRMWARE_PROFILES_BIT_02  0  // ZigBee PRO - Light Link.
# define FIRMWARE_PROFILES_BIT_03  0
# define FIRMWARE_PROFILES_BIT_04  0
# define FIRMWARE_PROFILES_BIT_05  0
# define FIRMWARE_PROFILES_BIT_06  0
# define FIRMWARE_PROFILES_BIT_07  0
#
# if DEFINED_OR_ONE(USE_RF4CE_PROFILE_GDP)
#  define FIRMWARE_PROFILES_BIT_08 1
# else
#  define FIRMWARE_PROFILES_BIT_08 0
# endif
#
# if DEFINED_OR_ONE(USE_RF4CE_PROFILE_ZRC1)
#  if DEFINED_OR_ONE(USE_RF4CE_PROFILE_ZRC2)
#   define FIRMWARE_PROFILES_BIT_09 0
#  else
#   define FIRMWARE_PROFILES_BIT_09 1
#  endif
# else
#  define FIRMWARE_PROFILES_BIT_09 0
# endif
#
# if DEFINED_OR_ONE(USE_RF4CE_PROFILE_ZRC2)
#  define FIRMWARE_PROFILES_BIT_10 1
# else
#  define FIRMWARE_PROFILES_BIT_10 0
# endif
#
# if DEFINED_OR_ONE(USE_RF4CE_PROFILE_MSO)
#  define FIRMWARE_PROFILES_BIT_11  1
# else
#  define FIRMWARE_PROFILES_BIT_11  0
# endif
#
# define FIRMWARE_PROFILES_BIT_12  0
# define FIRMWARE_PROFILES_BIT_13  0
# define FIRMWARE_PROFILES_BIT_14  0
# define FIRMWARE_PROFILES_BIT_15  0
#
# define FIRMWARE_PROFILES (                    \
        ((FIRMWARE_PROFILES_BIT_00)   <<  0)    \
        | ((FIRMWARE_PROFILES_BIT_01) <<  1)    \
        | ((FIRMWARE_PROFILES_BIT_02) <<  2)    \
        | ((FIRMWARE_PROFILES_BIT_03) <<  3)    \
        | ((FIRMWARE_PROFILES_BIT_04) <<  4)    \
        | ((FIRMWARE_PROFILES_BIT_05) <<  5)    \
        | ((FIRMWARE_PROFILES_BIT_06) <<  6)    \
        | ((FIRMWARE_PROFILES_BIT_07) <<  7)    \
        | ((FIRMWARE_PROFILES_BIT_08) <<  8)    \
        | ((FIRMWARE_PROFILES_BIT_09) <<  9)    \
        | ((FIRMWARE_PROFILES_BIT_10) <<  10)   \
        | ((FIRMWARE_PROFILES_BIT_11) <<  11)   \
        | ((FIRMWARE_PROFILES_BIT_12) <<  12)   \
        | ((FIRMWARE_PROFILES_BIT_13) <<  13)   \
        | ((FIRMWARE_PROFILES_BIT_14) <<  14)   \
        | ((FIRMWARE_PROFILES_BIT_15) << 15))
#
#endif /* ifndef FIRMWARE_PROFILES */

#endif /* _MAIL_CONFIG_H */