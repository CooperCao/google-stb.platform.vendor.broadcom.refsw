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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      RF4CE Stack Components configuration. This file MUST be included as the first included file
 *      of each RF4CE network/profiles source files (and possibly header files). Because that file verifies
 *      the validity of the compilation definitions at compile time resolving nonsenses where possible or
 *      informing the user of an error in compilation definitions set. As well as builds proper definitions for
 *      inclusion/exclusion of the RF4CE modules into/from compilation set.
 *
*******************************************************************************/

#ifndef _RF4CE_CONFIG_H
#define _RF4CE_CONFIG_H


/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Checks on correct module inclusion by definitions.
 */
#if defined(USE_RF4CE_PROFILE_GDP)
#    undef USE_RF4CE_PROFILE_GDP
#    define USE_RF4CE_PROFILE_GDP 1
#else /* USE_RF4CE_PROFILE_GDP */
#    define USE_RF4CE_PROFILE_GDP 0
#endif /* USE_RF4CE_PROFILE_GDP */

#if defined(USE_RF4CE_PROFILE_MSO)
#    undef USE_RF4CE_PROFILE_MSO
#    define USE_RF4CE_PROFILE_MSO 1
#else /* USE_RF4CE_PROFILE_MSO */
#    define USE_RF4CE_PROFILE_MSO 0
#endif /* USE_RF4CE_PROFILE_MSO */

#if defined(USE_RF4CE_PROFILE_CC)
#    undef USE_RF4CE_PROFILE_CC
#    define USE_RF4CE_PROFILE_CC 1
#else /* USE_RF4CE_PROFILE_CC */
#    define USE_RF4CE_PROFILE_CC 0
#endif /* USE_RF4CE_PROFILE_CC */


#if defined(USE_RF4CE_PROFILE_ZRC) || defined(USE_RF4CE_PROFILE_ZRC1) || defined(USE_RF4CE_PROFILE_ZRC2)
#    if defined(USE_RF4CE_PROFILE_ZRC)
#        undef USE_RF4CE_PROFILE_ZRC
#        if !defined(USE_RF4CE_PROFILE_ZRC1)
#            define USE_RF4CE_PROFILE_ZRC1
#        endif /* !defined(USE_RF4CE_PROFILE_ZRC1) */
#        if !defined(USE_RF4CE_PROFILE_ZRC2)
#            define USE_RF4CE_PROFILE_ZRC2
#        endif /* !defined(USE_RF4CE_PROFILE_ZRC2) */
#    else /* defined(USE_RF4CE_PROFILE_ZRC) */
#        if defined(USE_RF4CE_PROFILE_ZRC2)
#            if !defined(USE_RF4CE_PROFILE_ZRC1)
#                define USE_RF4CE_PROFILE_ZRC1
#            endif /* !defined(USE_RF4CE_PROFILE_ZRC1) */
#        endif /* defined(USE_RF4CE_PROFILE_ZRC2) */
#    endif /* defined(USE_RF4CE_PROFILE_ZRC) */
#    define USE_RF4CE_PROFILE_ZRC 1
#else /* defined(USE_RF4CE_PROFILE_ZRC) || defined(USE_RF4CE_PROFILE_ZRC1) || defined(USE_RF4CE_PROFILE_ZRC2) */
#    define USE_RF4CE_PROFILE_ZRC 0
#endif /* defined(USE_RF4CE_PROFILE_ZRC) || defined(USE_RF4CE_PROFILE_ZRC1) || defined(USE_RF4CE_PROFILE_ZRC2) */

#ifdef RF4CE_CONTROLLER
#    ifdef RF4CE_TARGET
#        error Both RF4CE_CONTROLLER and RF4CE_TARGET are defined! Ambigious compilation directives detected!
#    endif /* RF4CE_TARGET */
#else /* RF4CE_CONTROLLER */
#    ifndef RF4CE_TARGET
#        define RF4CE_CONTROLLER
#    endif /* RF4CE_TARGET */
#endif /* RF4CE_CONTROLLER */

#ifdef USE_RF4CE_PROFILES
#    undef USE_RF4CE_PROFILES
#endif /* USE_RF4CE_PROFILES */
#define USE_RF4CE_PROFILES (USE_RF4CE_PROFILE_MSO + USE_RF4CE_PROFILE_ZRC + USE_RF4CE_PROFILE_GDP)

#if (USE_RF4CE_PROFILES != 0)
#    if (USE_RF4CE_PROFILES > 1)
#        ifdef RF4CE_CONTROLLER
#            error Controller must have only one profile enabled!
#        endif /* RF4CE_CONTROLLER */
#    endif /* (USE_RF4CE_PROFILES > 0) */
#endif /* (USE_RF4CE_PROFILES != 0) */

#ifdef MAILBOX_UNIT_TEST
# ifndef RF4CE_CONTROLLER
#  define RF4CE_CONTROLLER
# endif
# ifndef RF4CE_TARGET
#  define RF4CE_TARGET
# endif
#endif

#endif /* _RF4CE_CONFIG_H */

/* eof bbRF4CEConfig.h */